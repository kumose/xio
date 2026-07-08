/************************************************************************
Modifications Copyright 2017-2019 eBay Inc.
Author/Developer(s): Jung-Sang Ahn

Original Copyright:
See URL: https://github.com/datatechnology/cornerstone

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**************************************************************************/

#include <xio/raft/global_mgr.h>
#include <xio/raft/pp_util.h>
#include <xio/raft/raft_params.h>
#include <xio/raft/raft_server.h>

#include <xio/raft/cluster_config.h>
#include <xio/raft/error_code.h>
#include <xio/raft/event_awaiter.h>
#include <xio/raft/exit_handler.h>
#include <xio/raft/handle_custom_notification.h>
#include <xio/raft/peer.h>
#include <xio/raft/snapshot.h>
#include <xio/raft/state_machine.h>
#include <xio/raft/state_mgr.h>
#include <xio/logging.h>

#include <algorithm>
#include <cassert>
#include <sstream>

namespace nuraft {
    /**
 * Additional information in addition to `append_entries_response`.
 */
    struct resp_appendix {
        enum extra_order : uint8_t {
            NONE = 0,
            DO_NOT_REWIND = 1,
            RECEIVING_SNAPSHOT = 2,
            NOTIFYING_SM_COMMITTED_INDEX = 3,
        };

        resp_appendix()
            : extra_order_(NONE)
              , sm_committed_idx_(0) {
        }

        ptr<buffer> serialize() const {
            const static uint8_t CUR_VERSION = 0;
            size_t buf_len = sizeof(CUR_VERSION) + sizeof(extra_order_);
            if (extra_order_ == NOTIFYING_SM_COMMITTED_INDEX) {
                buf_len += sizeof(sm_committed_idx_);
            }

            //  << Format >>
            // Format version       1 byte
            // Extra order          1 byte

            //  << Format (if order == NOTIFYING_SM_COMMITTED_INDEX) >>
            // Format version       1 byte
            // Extra order          1 byte
            // SM committed index   8 bytes

            ptr<buffer> result = buffer::alloc(buf_len);
            buffer_serializer bs(*result);
            bs.put_u8(CUR_VERSION);
            bs.put_u8(extra_order_);
            if (extra_order_ == NOTIFYING_SM_COMMITTED_INDEX) {
                bs.put_u64(sm_committed_idx_);
            }

            return result;
        }

        static ptr<resp_appendix> deserialize(buffer &buf) {
            buffer_serializer bs(buf);
            ptr<resp_appendix> res = cs_new<resp_appendix>();

            uint8_t cur_ver = bs.get_u8();
            if (cur_ver != 0) {
                // Not supported version.
                return res;
            }

            res->extra_order_ = static_cast<extra_order>(bs.get_u8());
            if (res->extra_order_ == NOTIFYING_SM_COMMITTED_INDEX &&
                bs.pos() + sizeof(uint64_t) <= buf.size()) {
                res->sm_committed_idx_ = bs.get_u64();
            }
            return res;
        }

        static const char *extra_order_msg(extra_order v) {
            switch (v) {
                case NONE:
                    return "NONE";
                case DO_NOT_REWIND:
                    return "DO_NOT_REWIND";
                case RECEIVING_SNAPSHOT:
                    return "RECEIVING_SNAPSHOT";
                case NOTIFYING_SM_COMMITTED_INDEX:
                    return "NOTIFYING_SM_COMMITTED_INDEX";
                default:
                    return "UNKNOWN";
            }
        };

        /**
     * Extra order for the response.
     */
        extra_order extra_order_;

        /**
     * If non-zero, it indicates the committed log index of
     * the state machine of the follower who sent this response.
     */
        uint64_t sm_committed_idx_;
    };

    void raft_server::append_entries_in_bg() {
        std::string thread_name = "nuraft_append";
#ifdef __linux__
        pthread_setname_np(pthread_self(), thread_name.c_str());
#elif __APPLE__
        pthread_setname_np(thread_name.c_str());
#endif

        TLOG(INFO, "bg append_entries thread initiated");
        do {
            bg_append_ea_->wait();
            bg_append_ea_->reset();
            if (stopping_) break;

            append_entries_in_bg_exec();
        } while (!stopping_);
        append_bg_stopped_ = true;
        TLOG(INFO, "bg append_entries thread terminated");
    }

    void raft_server::append_entries_in_bg_exec() {
        recur_lock(lock_);
        request_append_entries();
    }

    void raft_server::request_append_entries() {
        // Special case:
        //   1) one-node cluster, OR
        //   2) quorum size == 1 (including leader).
        //
        // In those cases, we may not enter `handle_append_entries_resp`,
        // which calls `commit()` function.
        // We should call it here.
        if (peers_.size() == 0 ||
            get_quorum_for_commit() == 0) {
            uint64_t leader_index = get_current_leader_index();
            commit(leader_index);
            return;
        }

        for (peer_itor it = peers_.begin(); it != peers_.end(); ++it) {
            request_append_entries(it->second);
        }
    }

    bool raft_server::request_append_entries(ptr<peer> p) {
        static timer_helper chk_timer(1000 * 1000);

        // Checking the validity of role first.
        if (role_ != srv_role::leader) {
            // WARNING: We should allow `write_paused_` state for
            //          graceful resignation.
            return false;
        }

        cb_func::Param cb_param(id_, leader_, p->get_id());
        CbReturnCode rc = ctx_->cb_func_.call(cb_func::RequestAppendEntries, &cb_param);
        if (rc == CbReturnCode::ReturnNull) {
            TLOG(WARNING, "by callback, abort request_append_entries");
            return true;
        }

        ptr<raft_params> params = ctx_->get_params();

        if (params->auto_adjust_quorum_for_small_cluster_ &&
            get_num_voting_members() == 2 &&
            chk_timer.timeout_and_reset()) {
            // If auto adjust mode is on for 2-node cluster, and
            // the follower is not responding, adjust the quorum.
            size_t num_not_responding_peers = get_not_responding_peers_count();
            size_t cur_quorum_size = get_quorum_for_commit();
            size_t num_stale_peers = get_num_stale_peers();
            if (cur_quorum_size >= 1) {
                bool do_adjustment = false;
                if (num_not_responding_peers) {
                    TLOG(WARNING, "2-node cluster's follower is not responding long time, "
                        "adjust quorum to 1");
                    do_adjustment = true;
                } else if (num_stale_peers) {
                    TLOG(WARNING, "2-node cluster's follower is lagging behind, "
                        "adjust quorum to 1");
                    do_adjustment = true;
                }
                if (do_adjustment) {
                    cb_func::Param cb_param(id_, leader_, p->get_id());
                    CbReturnCode rc =
                            ctx_->cb_func_.call(cb_func::AutoAdjustQuorum, &cb_param);
                    if (rc == CbReturnCode::ReturnNull) {
                        // Callback function rejected the adjustment.
                        TLOG(WARNING, "quorum size adjustment was declined by callback");
                    } else {
                        ptr<raft_params> clone = cs_new<raft_params>(*params);
                        clone->custom_commit_quorum_size_ = 1;
                        clone->custom_election_quorum_size_ = 1;
                        ctx_->set_params(clone);
                        // When the quorum size is 1 and the server is idle, there is no
                        // opportunity to commit pending logs
                        uint64_t leader_index = get_current_leader_index();
                        commit(leader_index);
                    }
                }
            } else if (num_not_responding_peers == 0 &&
                       num_stale_peers == 0 &&
                       params->custom_commit_quorum_size_ == 1) {
                // Recovered, both cases should be clear.
                TLOG(WARNING, "2-node cluster's follower is responding now, "
                    "restore quorum with default value");
                ptr<raft_params> clone = cs_new<raft_params>(*params);
                clone->custom_commit_quorum_size_ = 0;
                clone->custom_election_quorum_size_ = 0;
                ctx_->set_params(clone);
            }
        }

        bool need_to_reconnect = p->need_to_reconnect();
        int32 last_active_time_ms = p->get_active_timer_us() / 1000;
        if (last_active_time_ms >
            params->heart_beat_interval_ *
            raft_server::raft_limits_.reconnect_limit_) {
            if (srv_to_leave_ &&srv_to_leave_
            ->
            get_id() == p->get_id()
            )
            {
                // We should not re-establish the connection to
                // to-be-removed server, as it will block removing it
                // from `peers_` list.
                TLOG(WARNING, "connection to peer {} is not active long time: {} ms, "
                     "but this peer should be removed. do nothing",
                     p->get_id(),
                     last_active_time_ms);
            }
            else
            {
                TLOG(WARNING, "connection to peer {} is not active long time: {} ms, "
                     "force re-connect",
                     p->get_id(),
                     last_active_time_ms);
                need_to_reconnect = true;
                p->reset_active_timer();
            }
        }
        if (need_to_reconnect) {
            bool reconnected = reconnect_client(*p);
            uint64_t p_next_log_idx = p->get_next_log_idx();
            if (reconnected && p_next_log_idx) {
                // NOTE:
                //   Discussions in https://github.com/eBay/NuRaft/issues/181
                //
                //   The leader keeps the last sent log info for each peer
                //   and sends the next `append_entries` request based on
                //   that data. This process disrupts offline data change,
                //   such as snapshot install, as the leader's peer info
                //   remains unchanged and results in wrong decisions. The
                //   right way to do this is to remove the node, install
                //   the snapshot, and then add the node back.
                //
                //   To avoid those hassles, we can reset the peer info that
                //   the leader has. Once any disconnection happens to a
                //   peer, we reset the last sent log info. The log info will
                //   be re-adjusted by the first `append_entries`
                //   communication between the leader and the peer. It will
                //   eventually have the same impact of removing and then
                //   re-adding the peer.
                //
                TLOG(TRACE, "new rpc for peer {} is created, "
                     "reset next log idx ({}" ") and matched log idx ({}" ")",
                     p->get_id(), p_next_log_idx, p->get_matched_idx());
                p->set_next_log_idx(0);
                p->set_matched_idx(0);
            }
            p->clear_reconnection();
        }

        if (params->use_bg_thread_for_snapshot_io_) {
            // Check the current queue if previous request exists.
            if (snapshot_io_mgr::instance().has_pending_request(this, p->get_id())) {
                TLOG(TRACE, "previous snapshot request for peer {} already exists",
                     p->get_id());
                return true;
            }
        }

        // If reserved message exists, process it first.
        ptr<req_msg> msg = p->get_rsv_msg();
        rpc_handler m_handler = p->get_rsv_msg_handler();
        if (msg) {
            if (p->make_busy()) {
                // Clear the reserved message.
                p->set_rsv_msg(nullptr, nullptr);
                TLOG(INFO, "found reserved message to peer {}, type {}",
                     p->get_id(), msg_type_to_string(msg->get_type()));
                return send_request(p, msg, m_handler);
            }
        } else {
            ulong last_streamed_log_idx = p->get_last_streamed_log_idx();
            int32 max_gap_in_stream = params->max_log_gap_in_stream_;
            if (last_streamed_log_idx > 0 && max_gap_in_stream == 0) {
                TLOG(INFO, "disable stream mode for peer {} at runtime, "
                     "current streamed log: {}" "", p->get_id(),
                     last_streamed_log_idx);
                last_streamed_log_idx = 0;
                p->reset_stream();
            }
            bool streaming = last_streamed_log_idx > 0;

            if (streaming || p->make_busy()) {
                TLOG(TRACE, "send request to {}, streaming: {}, is_busy: {}\n", (int) p->get_id(),
                     streaming, p->is_busy());
                msg = create_append_entries_req(p, last_streamed_log_idx);
                m_handler = resp_handler_;

                if (msg) {
                    streaming = streaming &&
                                msg->get_type() == msg_type::append_entries_request;
                    bool make_busy_result = p->is_busy();
                    if (streaming) {
                        // throttling
                        int64_t max_stream_bytes =
                                params->max_bytes_in_flight_in_stream_ > 0
                                    ? params->max_bytes_in_flight_in_stream_
                                    : 0;

                        if (max_gap_in_stream + p->get_next_log_idx() <=
                            (last_streamed_log_idx + 1) ||
                            (max_stream_bytes &&
                             p->get_bytes_in_flight() > max_stream_bytes)) {
                            streaming = false;
                        } else {
                            TLOG(TRACE, "send following request to {} in stream mode, "
                                 "start idx: {}" "", (int) p->get_id(),
                                 msg->get_last_log_idx());
                            p->set_last_streamed_log_idx(
                                last_streamed_log_idx,
                                last_streamed_log_idx + msg->log_entries().size());
                        }
                    } else if (!make_busy_result) {
                        make_busy_result = p->make_busy();
                    }

                    if (streaming || make_busy_result) {
                        return send_request(p, msg, m_handler, streaming);
                    }
                } else {
                    if (!streaming) {
                        p->set_free();
                    }

                    if (params->use_bg_thread_for_snapshot_io_ &&
                        p->get_snapshot_sync_ctx()) {
                        // If this is an async snapshot request, invoke IO thread.
                        snapshot_io_mgr::instance().invoke();
                    }
                    return true;
                }
            }
        }

        TLOG(DEBUG, "Server {} is busy, skip the request", p->get_id());
        check_snapshot_timeout(p);

        int32 last_ts_ms = p->get_ls_timer_us() / 1000;
        if (last_ts_ms > params->heart_beat_interval_) {
            // Waiting time becomes longer than HB interval, warning.
            p->inc_long_pause_warnings();
            if (p->get_long_puase_warnings() < raft_server::raft_limits_.warning_limit_) {
                TLOG(WARNING, "skipped sending msg to {} too long time, "
                     "last streamed idx: {}" ", "
                     "next log idx: {}" ", "
                     "in-flight: {}" " bytes, "
                     "last msg sent {} ms ago",
                     p->get_id(), p->get_last_streamed_log_idx(),
                     p->get_next_log_idx(), p->get_bytes_in_flight(), last_ts_ms);
            } else if (p->get_long_puase_warnings() ==
                       raft_server::raft_limits_.warning_limit_) {
                TLOG(WARNING, "long pause warning to {} is too verbose, "
                     "will suppress it from now", p->get_id());
            }
        }
        return false;
    }

    bool raft_server::send_request(ptr<peer> &p,
                                   ptr<req_msg> &msg,
                                   rpc_handler &m_handler,
                                   bool streaming) {
        if (!p->is_manual_free()) {
            // Actual recovery.
            if (p->get_long_puase_warnings() >=
                raft_server::raft_limits_.warning_limit_) {
                int32 last_ts_ms = p->get_ls_timer_us() / 1000;
                p->inc_recovery_cnt();
                TLOG(WARNING, "recovered from long pause to peer {}, {} warnings, "
                     "{} ms, {} times",
                     p->get_id(),
                     p->get_long_puase_warnings(),
                     last_ts_ms,
                     p->get_recovery_cnt());

                if (p->get_recovery_cnt() >= 10) {
                    // Re-connect client, just in case.
                    //reconnect_client(*p);
                    p->reset_recovery_cnt();
                }
            }
            p->reset_long_pause_warnings();
        } else {
            // FIXME: `manual_free` is deprecated, need to get rid of it.

            // It means that this is not an actual recovery,
            // but just temporarily freed busy flag.
            p->reset_manual_free();
        }

        p->send_req(p, msg, m_handler, streaming);
        p->reset_ls_timer();

        cb_func::Param param(id_, leader_, p->get_id(), msg.get());
        ctx_->cb_func_.call(cb_func::SentAppendEntriesReq, &param);

        if (srv_to_leave_ &&
            srv_to_leave_
        ->
        get_id() == p->get_id() &&
                msg->get_commit_idx() >= srv_to_leave_target_idx_ &&
                !srv_to_leave_->is_stepping_down()
        )
        {
            // If this is the server to leave, AND
            // current request's commit index includes
            // the target log index number, step down and remove it
            // as soon as we get the corresponding response.
            srv_to_leave_->step_down();
            TLOG(INFO, "srv_to_leave_ {} is safe to be erased from peer list, "
                 "log idx {}" " commit idx {}" ", set flag",
                 srv_to_leave_->get_id(),
                 msg->get_last_log_idx(),
                 msg->get_commit_idx());
        }

        TLOG(TRACE, "sent\n");
        return true;
    }

    ptr<req_msg> raft_server::create_append_entries_req(ptr<peer> &pp,
                                                        ulong custom_last_log_idx) {
        peer &p = *pp;
        ulong cur_nxt_idx(0L);
        ulong commit_idx(0L);
        ulong last_log_idx(0L);
        ulong term(0L);
        ulong starting_idx(1L);

        {
            recur_lock(lock_);
            starting_idx = log_store_->start_index();
            cur_nxt_idx = precommit_index_ + 1;
            commit_idx = quick_commit_index_;
            term = state_->get_term();
        }

        {
            std::lock_guard<std::mutex> guard(p.get_lock());
            if (p.get_next_log_idx() == 0L) {
                p.set_next_log_idx(cur_nxt_idx);
            }

            if (custom_last_log_idx > 0) {
                last_log_idx = custom_last_log_idx;
            } else {
                last_log_idx = p.get_next_log_idx() - 1;
            }
        }

        if (last_log_idx >= cur_nxt_idx) {
            // LCOV_EXCL_START
            TLOG(ERROR, "Peer's lastLogIndex is too large {}" " v.s. {}" ", ",
                 last_log_idx, cur_nxt_idx);
            ctx_->state_mgr_->system_exit(raft_err::N8_peer_last_log_idx_too_large);
            _sys_exit(-1);
            return ptr<req_msg>();
            // LCOV_EXCL_STOP
        }

        // cur_nxt_idx: last log index of myself (leader).
        // starting_idx: start log index of myself (leader).
        // last_log_idx: last log index of replica (follower).
        // end_idx: if (cur_nxt_idx - last_log_idx) > max_append_size, limit it.

        TLOG(TRACE, "last_log_idx: {}" ", starting_idx: {}"
             ", cur_nxt_idx: {}" "\n",
             last_log_idx, starting_idx, cur_nxt_idx);

        // Verify log index range.
        bool entries_valid = (last_log_idx + 1 >= starting_idx);
        if (entries_valid && pp->is_snapshot_sync_needed()) {
            entries_valid = false;
        }

        // Read log entries. The underlying log store may have removed some log entries
        // causing some of the requested entries to be unavailable. The log store should
        // return nullptr to indicate such errors.
        ptr<raft_params> params = ctx_->get_params();
        ulong end_idx = std::min(cur_nxt_idx,
                                 last_log_idx + 1 + params->max_append_size_);

        // NOTE: If this is a retry, probably the follower is down.
        //       Send just one log until it comes back
        //       (i.e., max_append_size_ = 1).
        //       Only when end_idx - start_idx > 1, and 5th try.
        ulong peer_last_sent_idx = p.get_last_sent_idx();
        if (last_log_idx + 1 == peer_last_sent_idx &&
            last_log_idx + 2 < end_idx) {
            int32 cur_cnt = p.inc_cnt_not_applied();
            TLOG(DEBUG, "last sent log ({}" ") to peer {} is not applied, cnt {}",
                 peer_last_sent_idx, p.get_id(), cur_cnt);
            if (cur_cnt >= 5) {
                ulong prev_end_idx = end_idx;
                end_idx = std::min(cur_nxt_idx, last_log_idx + 1 + 1);
                TLOG(DEBUG, "reduce end_idx {}" " -> {}", prev_end_idx, end_idx);
            }
        } else {
            p.reset_cnt_not_applied();
        }

        ptr<std::vector<ptr<log_entry> > > log_entries;
        if ((last_log_idx + 1) >= cur_nxt_idx) {
            log_entries = ptr<std::vector<ptr<log_entry> > >();
        } else if (entries_valid) {
            log_entries = log_store_->log_entries_ext(last_log_idx + 1, end_idx,
                                                      p.get_next_batch_size_hint_in_bytes());
            if (log_entries == nullptr) {
                TLOG(WARNING, "failed to retrieve log entries: {}" " - {}",
                     last_log_idx + 1, end_idx);
                entries_valid = false;
            }
        }

        if (!entries_valid) {
            // Required log entries are missing. First, we try to use snapshot to recover.
            // To avoid inconsistency due to smart pointer, should have local varaible
            // to increase its ref count.
            ptr<snapshot> snp_local = get_last_snapshot();

            // Modified by Jung-Sang Ahn (Oct 11 2017):
            // As `reserved_log` has been newly added, need to check snapshot
            // in addition to `starting_idx`.
            if (snp_local &&
                (pp->is_snapshot_sync_needed() ||
                 (last_log_idx < starting_idx &&
                  last_log_idx < snp_local->get_last_log_idx()))) {
                TLOG(DEBUG, "send snapshot peer {}, peer log idx: {}"
                     ", my starting idx: {}" ", "
                     "my log idx: {}" ", last_snapshot_log_idx: {}"
                     ", snapshot sync needed: {}",
                     p.get_id(),
                     last_log_idx, starting_idx, cur_nxt_idx,
                     snp_local->get_last_log_idx(),
                     pp->is_snapshot_sync_needed());

                bool succeeded_out = false;
                return create_sync_snapshot_req(pp, last_log_idx, term,
                                                commit_idx, succeeded_out);
            }

            // Cannot recover using snapshot. Return here to protect the leader.
            static timer_helper msg_timer(5000000);
            int log_lv = msg_timer.timeout_and_reset() ? L_ERROR : L_TRACE;
            switch (log_lv) {
case L_FATAL:
    TLOG(FATAL, "neither snapshot nor log exists, peer {}, last log {}" ", "
                 "leader's start log {}", p.get_id(), last_log_idx, starting_idx);
    break;
case L_ERROR:
    TLOG(ERROR, "neither snapshot nor log exists, peer {}, last log {}" ", "
                 "leader's start log {}", p.get_id(), last_log_idx, starting_idx);
    break;
case L_WARN:
    TLOG(WARNING, "neither snapshot nor log exists, peer {}, last log {}" ", "
                 "leader's start log {}", p.get_id(), last_log_idx, starting_idx);
    break;
case L_INFO:
    TLOG(INFO, "neither snapshot nor log exists, peer {}, last log {}" ", "
                 "leader's start log {}", p.get_id(), last_log_idx, starting_idx);
    break;
case L_DEBUG:
    TLOG(DEBUG, "neither snapshot nor log exists, peer {}, last log {}" ", "
                 "leader's start log {}", p.get_id(), last_log_idx, starting_idx);
    break;
case L_TRACE:
    TLOG(TRACE, "neither snapshot nor log exists, peer {}, last log {}" ", "
                 "leader's start log {}", p.get_id(), last_log_idx, starting_idx);
    break;
default:
    break;
};

            // Send out-of-log-range notification to this follower.
            ptr<req_msg> req = cs_new<req_msg>
            (term, msg_type::custom_notification_request,
             id_, p.get_id(), 0, last_log_idx, commit_idx);

            // Out-of-log message.
            ptr<out_of_log_msg> ool_msg = cs_new<out_of_log_msg>();
            ool_msg->start_idx_of_leader_ = starting_idx;

            // Create a notification containing OOL message.
            ptr<custom_notification_msg> custom_noti =
                    cs_new<custom_notification_msg>
                    (custom_notification_msg::out_of_log_range_warning);
            custom_noti->ctx_ = ool_msg->serialize();

            // Wrap it using log_entry.
            ptr<log_entry> custom_noti_le =
                    cs_new<log_entry>(0, custom_noti->serialize(), log_val_type::custom);

            req->log_entries().push_back(custom_noti_le);
            return req;
        }

        ulong last_log_term = term_for_log(last_log_idx);
        ulong adjusted_end_idx = end_idx;
        if (log_entries) adjusted_end_idx = last_log_idx + 1 + log_entries->size();
        if (adjusted_end_idx != end_idx) {
            TLOG(TRACE, "adjusted end_idx due to batch size hint: {}" " -> {}",
                 end_idx, adjusted_end_idx);
        }

        TLOG(DEBUG, "append_entries for {} with LastLogIndex={}" ", "
             "LastLogTerm={}" ", EntriesLength={}, CommitIndex={}" ", "
             "Term={}" ", peer_last_sent_idx {}",
             p.get_id(), last_log_idx, last_log_term,
             (log_entries ? log_entries->size() : 0), commit_idx, term,
             peer_last_sent_idx);
        if (last_log_idx + 1 == adjusted_end_idx) {
            TLOG(TRACE, "EMPTY PAYLOAD");
        } else if (last_log_idx + 1 + 1 == adjusted_end_idx) {
            TLOG(DEBUG, "idx: {}", last_log_idx + 1);
        } else {
            TLOG(DEBUG, "idx range: {}" "-{}", last_log_idx + 1, adjusted_end_idx - 1);
        }

        ptr<req_msg> req
        (cs_new<req_msg>
        (term, msg_type::append_entries_request, id_, p.get_id(),
         last_log_term, last_log_idx, commit_idx));
        std::vector<ptr<log_entry> > &v = req->log_entries();
        if (log_entries) {
            v.insert(v.end(), log_entries->begin(), log_entries->end());
        }
        p.set_last_sent_idx(last_log_idx + 1);

        if (params->use_full_consensus_among_healthy_members_) {
            // Full consensus mode: set flag indicating the member is excluded.
            uint64_t last_resp_time_ms = p.get_resp_timer_us() / 1000;
            uint64_t expiry = params->heart_beat_interval_ *
                              raft_server::raft_limits_.full_consensus_leader_limit_;
            uint64_t required_log_idx =
                    quick_commit_index_ > (uint64_t) params->max_append_size_
                        ? quick_commit_index_ - params->max_append_size_
                        : 0;
            if (is_excluded_from_quorum(p, last_resp_time_ms, expiry, required_log_idx,
                                        /* include_self_mark_down = */ false)) {
                req->set_extra_flags(
                    req->get_extra_flags() | req_msg::EXCLUDED_FROM_THE_QUORUM);
            }
        }

        return req;
    }

    ptr<resp_msg> raft_server::handle_append_entries(req_msg &req) {
        ptr<raft_params> params = ctx_->get_params();
        uint64_t time_gap_ms = last_rcvd_append_entries_req_.get_ms();
        last_rcvd_append_entries_req_.reset();
        if (params->use_full_consensus_among_healthy_members_ &&
            time_gap_ms > (uint64_t) params->heart_beat_interval_ *
            raft_limits_.full_consensus_follower_limit_) {
            // If full consensus mode is on, and heartbeat or append_entries request
            // arrives after the limit. We cannot accept it due to the case as follows:
            //
            // [L] Sends heartbeat, still in-flight.
            // [L] At 4*H, the leader will exclude the follower and make another commit.
            // [F] Receives heartbeat after 5*H, at this moment,
            //     it thinks it is part of consensus and able to server reads.
            //     But it does not have the latest commit.
            //
            // To avoid this, we should accept the request only when its interval
            // is smaller than the limit.
            //
            // If all the request have longer interval than the limit,
            // it is reasonable to say this follower is not healthy and excluded.
            TLOG(WARNING, "heartbeat or append_entries request arrives after {}" " ms, "
                 "which is larger than the limit: {} ms, reject it",
                 time_gap_ms,
                 params->heart_beat_interval_ *
                 raft_limits_.full_consensus_follower_limit_);

            return nullptr;
        }

        bool supp_exp_warning = false;
        if (state_->is_catching_up()) {
            // WARNING:
            //   We should clear the `catching_up_` flag only after this node's
            //   config has been added to the cluster config. Otherwise, if we
            //   clear it before that, any membership change configs (which is
            //   already outdated but committed after the received snapshot)
            //   may cause stepping down of this node.
            ptr<cluster_config> cur_config = get_config();
            ptr<srv_config> my_config = cur_config->get_server(id_);
            if (my_config && !my_config->is_new_joiner()) {
                TLOG(INFO, "catch-up process is done, clearing the flag");
                state_->set_catching_up(false);
                ctx_->state_mgr_->save_state(*state_);
            }
            supp_exp_warning = true;
        }

        // To avoid election timer wakes up while we are in the middle
        // of this function, this structure sets the flag and automatically
        // clear it when we return from this function.
        struct ServingReq {
            ServingReq(std::atomic<bool> *_val) : val(_val) { val->store(true); }
            ~ServingReq() { val->store(false); }
            std::atomic<bool> *val;
        } _s_req(&serving_req_);
        timer_helper tt;

        TLOG(TRACE, "from peer {}, req type: {}, req term: {}" ", "
             "req l idx: {}" " ({}), req c idx: {}" ", "
             "my term: {}" ", my role: {}\n",
             req.get_src(), (int) req.get_type(), req.get_term(),
             req.get_last_log_idx(), req.log_entries().size(), req.get_commit_idx(),
             state_->get_term(), (int) role_);

        if (req.get_term() == state_->get_term()) {
            if (role_ == srv_role::candidate) {
                become_follower();
            } else if (role_ == srv_role::leader) {
                TLOG(WARNING, "Receive AppendEntriesRequest from another leader ({}) "
                     "with same term, there must be a bug. Invoking the callback.",
                     req.get_src());

                cb_func::Param param(id_, leader_, -1, &req);
                cb_func::ReqResp req_resp;
                req_resp.req = &req;

                ctx_->cb_func_.call(cb_func::ReceivedMisbehavingMessage, &param);
                if (!req_resp.resp.get()) {
                    TLOG(WARNING, "callback function didn't return response, ignore this request");
                } else {
                    TLOG(WARNING, "callback function returned response, send it back");
                }
                return req_resp.resp;
            } else {
                update_target_priority();
                // Modified by JungSang Ahn, Mar 28 2018:
                //   As we have `serving_req_` flag, restarting election timer
                //   should be move to the end of this function.
                // restart_election_timer();
            }
        }

        // After a snapshot the req.get_last_log_idx() may less than
        // log_store_->next_slot() but equals to log_store_->next_slot() -1
        //
        // In this case, log is Okay if
        //   req.get_last_log_idx() == lastSnapshot.get_last_log_idx() &&
        //   req.get_last_log_term() == lastSnapshot.get_last_log_term()
        //
        // In not accepted case, we will return log_store_->next_slot() for
        // the leader to quick jump to the index that might aligned.
        ptr<resp_msg> resp = cs_new<resp_msg>(state_->get_term(),
                                              msg_type::append_entries_response,
                                              id_,
                                              req.get_src(),
                                              log_store_->next_slot());

        ptr<snapshot> local_snp = get_last_snapshot();
        ulong log_term = 0;
        if (req.get_last_log_idx() < log_store_->next_slot()) {
            log_term = term_for_log(req.get_last_log_idx());
        }
        bool log_okay =
                req.get_last_log_idx() == 0 ||
                (log_term &&
                 req.get_last_log_term() == log_term) ||
                (local_snp &&
                 local_snp->get_last_log_idx() == req.get_last_log_idx() &&
                 local_snp->get_last_log_term() == req.get_last_log_term());

        int log_lv = log_okay ? L_TRACE : (supp_exp_warning ? L_INFO : L_WARN);
        static timer_helper log_timer(500 * 1000, true);
        if (log_lv == L_WARN) {
            // To avoid verbose logs.
            if (!log_timer.timeout_and_reset()) {
                log_lv = L_TRACE;
            }
        }
        switch (log_lv) {
case L_FATAL:
    TLOG(FATAL, "[LOG {}] req log idx: {}" ", req log term: {}"
             ", my last log idx: {}" ", "
             "my log ({}" ") term: {}", (log_okay ? "OK" : "XX"), req.get_last_log_idx(), req.get_last_log_term(), log_store_->next_slot() - 1, req.get_last_log_idx(), log_term);
    break;
case L_ERROR:
    TLOG(ERROR, "[LOG {}] req log idx: {}" ", req log term: {}"
             ", my last log idx: {}" ", "
             "my log ({}" ") term: {}", (log_okay ? "OK" : "XX"), req.get_last_log_idx(), req.get_last_log_term(), log_store_->next_slot() - 1, req.get_last_log_idx(), log_term);
    break;
case L_WARN:
    TLOG(WARNING, "[LOG {}] req log idx: {}" ", req log term: {}"
             ", my last log idx: {}" ", "
             "my log ({}" ") term: {}", (log_okay ? "OK" : "XX"), req.get_last_log_idx(), req.get_last_log_term(), log_store_->next_slot() - 1, req.get_last_log_idx(), log_term);
    break;
case L_INFO:
    TLOG(INFO, "[LOG {}] req log idx: {}" ", req log term: {}"
             ", my last log idx: {}" ", "
             "my log ({}" ") term: {}", (log_okay ? "OK" : "XX"), req.get_last_log_idx(), req.get_last_log_term(), log_store_->next_slot() - 1, req.get_last_log_idx(), log_term);
    break;
case L_DEBUG:
    TLOG(DEBUG, "[LOG {}] req log idx: {}" ", req log term: {}"
             ", my last log idx: {}" ", "
             "my log ({}" ") term: {}", (log_okay ? "OK" : "XX"), req.get_last_log_idx(), req.get_last_log_term(), log_store_->next_slot() - 1, req.get_last_log_idx(), log_term);
    break;
case L_TRACE:
    TLOG(TRACE, "[LOG {}] req log idx: {}" ", req log term: {}"
             ", my last log idx: {}" ", "
             "my log ({}" ") term: {}", (log_okay ? "OK" : "XX"), req.get_last_log_idx(), req.get_last_log_term(), log_store_->next_slot() - 1, req.get_last_log_idx(), log_term);
    break;
default:
    break;
};

        if (req.get_term() < state_->get_term() ||
            log_okay == false ||
            state_->is_receiving_snapshot()) {
            switch (log_lv) {
case L_FATAL:
    TLOG(FATAL, "deny, req term {}" ", my term {}"
                 ", req log idx {}" ", my log idx {}"
                 ", receiving snapshot {}", req.get_term(), state_->get_term(), req.get_last_log_idx(), log_store_->next_slot() - 1, (state_->is_receiving_snapshot() ? "TRUE" : "FALSE"));
    break;
case L_ERROR:
    TLOG(ERROR, "deny, req term {}" ", my term {}"
                 ", req log idx {}" ", my log idx {}"
                 ", receiving snapshot {}", req.get_term(), state_->get_term(), req.get_last_log_idx(), log_store_->next_slot() - 1, (state_->is_receiving_snapshot() ? "TRUE" : "FALSE"));
    break;
case L_WARN:
    TLOG(WARNING, "deny, req term {}" ", my term {}"
                 ", req log idx {}" ", my log idx {}"
                 ", receiving snapshot {}", req.get_term(), state_->get_term(), req.get_last_log_idx(), log_store_->next_slot() - 1, (state_->is_receiving_snapshot() ? "TRUE" : "FALSE"));
    break;
case L_INFO:
    TLOG(INFO, "deny, req term {}" ", my term {}"
                 ", req log idx {}" ", my log idx {}"
                 ", receiving snapshot {}", req.get_term(), state_->get_term(), req.get_last_log_idx(), log_store_->next_slot() - 1, (state_->is_receiving_snapshot() ? "TRUE" : "FALSE"));
    break;
case L_DEBUG:
    TLOG(DEBUG, "deny, req term {}" ", my term {}"
                 ", req log idx {}" ", my log idx {}"
                 ", receiving snapshot {}", req.get_term(), state_->get_term(), req.get_last_log_idx(), log_store_->next_slot() - 1, (state_->is_receiving_snapshot() ? "TRUE" : "FALSE"));
    break;
case L_TRACE:
    TLOG(TRACE, "deny, req term {}" ", my term {}"
                 ", req log idx {}" ", my log idx {}"
                 ", receiving snapshot {}", req.get_term(), state_->get_term(), req.get_last_log_idx(), log_store_->next_slot() - 1, (state_->is_receiving_snapshot() ? "TRUE" : "FALSE"));
    break;
default:
    break;
};
            if (local_snp) {
                switch (log_lv) {
case L_FATAL:
    TLOG(FATAL, "snp idx {}" " term {}", local_snp->get_last_log_idx(), local_snp->get_last_log_term());
    break;
case L_ERROR:
    TLOG(ERROR, "snp idx {}" " term {}", local_snp->get_last_log_idx(), local_snp->get_last_log_term());
    break;
case L_WARN:
    TLOG(WARNING, "snp idx {}" " term {}", local_snp->get_last_log_idx(), local_snp->get_last_log_term());
    break;
case L_INFO:
    TLOG(INFO, "snp idx {}" " term {}", local_snp->get_last_log_idx(), local_snp->get_last_log_term());
    break;
case L_DEBUG:
    TLOG(DEBUG, "snp idx {}" " term {}", local_snp->get_last_log_idx(), local_snp->get_last_log_term());
    break;
case L_TRACE:
    TLOG(TRACE, "snp idx {}" " term {}", local_snp->get_last_log_idx(), local_snp->get_last_log_term());
    break;
default:
    break;
};
            }
            if (state_->is_receiving_snapshot()) {
                // If it is in `receiving_snapshot` status but received a normal
                // `append_entries` request, that means the leader is not aware of
                // this node's status. We should send an additional hint.
                resp_appendix appendix;
                appendix.extra_order_ = resp_appendix::RECEIVING_SNAPSHOT;
                resp->set_ctx(appendix.serialize());
                switch (log_lv) {
case L_FATAL:
    TLOG(FATAL, "appended extra order {}", resp_appendix::extra_order_msg(appendix.extra_order_));
    break;
case L_ERROR:
    TLOG(ERROR, "appended extra order {}", resp_appendix::extra_order_msg(appendix.extra_order_));
    break;
case L_WARN:
    TLOG(WARNING, "appended extra order {}", resp_appendix::extra_order_msg(appendix.extra_order_));
    break;
case L_INFO:
    TLOG(INFO, "appended extra order {}", resp_appendix::extra_order_msg(appendix.extra_order_));
    break;
case L_DEBUG:
    TLOG(DEBUG, "appended extra order {}", resp_appendix::extra_order_msg(appendix.extra_order_));
    break;
case L_TRACE:
    TLOG(TRACE, "appended extra order {}", resp_appendix::extra_order_msg(appendix.extra_order_));
    break;
default:
    break;
};
            }
            resp->set_next_batch_size_hint_in_bytes(
                state_machine_->get_next_batch_size_hint_in_bytes());
            return resp;
        }

        // --- Now this node is a follower, and given log is okay. ---

        // set initialized flag
        if (!initialized_) initialized_ = true;

        bool old_excluded_from_the_quorum_ = excluded_from_the_quorum_;
        bool new_excluded_from_the_quorum_ =
                req.get_extra_flags() & req_msg::EXCLUDED_FROM_THE_QUORUM;
        if (old_excluded_from_the_quorum_ != new_excluded_from_the_quorum_) {
            TLOG(INFO, "excluded from the quorum changed from {} to {}",
                 old_excluded_from_the_quorum_ ? "true" : "false",
                 new_excluded_from_the_quorum_ ? "true" : "false");
            excluded_from_the_quorum_ = new_excluded_from_the_quorum_;
        }

        // Callback if necessary.
        cb_func::Param param(id_, leader_, -1, &req);
        cb_func::ReturnCode cb_ret =
                ctx_->cb_func_.call(cb_func::GotAppendEntryReqFromLeader, &param);
        // If callback function decided to refuse this request, return here.
        if (cb_ret != cb_func::Ok) {
            // If this request is declined by the application, not because of
            // term mismatch, we should request leader not to rewind the log.
            resp_appendix appendix;
            appendix.extra_order_ = resp_appendix::DO_NOT_REWIND;
            resp->set_ctx(appendix.serialize());

            // Also we should set the hint to a negative number,
            // to slow down the leader.
            resp->set_next_batch_size_hint_in_bytes(-1);

            static timer_helper log_timer(1000 * 1000);
            int log_lv = log_timer.timeout_and_reset() ? L_INFO : L_TRACE;
            switch (log_lv) {
case L_FATAL:
    TLOG(FATAL, "appended extra order {}", resp_appendix::extra_order_msg(appendix.extra_order_));
    break;
case L_ERROR:
    TLOG(ERROR, "appended extra order {}", resp_appendix::extra_order_msg(appendix.extra_order_));
    break;
case L_WARN:
    TLOG(WARNING, "appended extra order {}", resp_appendix::extra_order_msg(appendix.extra_order_));
    break;
case L_INFO:
    TLOG(INFO, "appended extra order {}", resp_appendix::extra_order_msg(appendix.extra_order_));
    break;
case L_DEBUG:
    TLOG(DEBUG, "appended extra order {}", resp_appendix::extra_order_msg(appendix.extra_order_));
    break;
case L_TRACE:
    TLOG(TRACE, "appended extra order {}", resp_appendix::extra_order_msg(appendix.extra_order_));
    break;
default:
    break;
};

            // Since this decline is on purpose, we should not let this server
            // initiaite leader election.
            restart_election_timer();

            return resp;
        }

        // Reset timer.
        last_rcvd_valid_append_entries_req_.reset();

        if (req.log_entries().size() > 0) {
            // Write logs to store, start from overlapped logs

            // Actual log number.
            ulong log_idx = req.get_last_log_idx() + 1;
            // Local counter for iterating req.log_entries().
            size_t cnt = 0;

            TLOG(DEBUG, "[INIT] log_idx: {}" ", count: {}, "
                 "log_store_->next_slot(): {}" ", "
                 "req.log_entries().size(): {}",
                 log_idx, cnt, log_store_->next_slot(), req.log_entries().size());

            // Skipping already existing (with the same term) logs.
            while (log_idx < log_store_->next_slot() &&
                   cnt < req.log_entries().size()) {
                if (log_store_->term_at(log_idx) ==
                    req.log_entries().at(cnt)->get_term()) {
                    log_idx++;
                    cnt++;
                } else {
                    break;
                }
            }
            TLOG(DEBUG, "[after SKIP] log_idx: {}" ", count: {}", log_idx, cnt);

            // Rollback (only if necessary).
            // WARNING:
            //   1) Rollback should be done separately before overwriting,
            //      and MUST BE in backward direction.
            //   2) Should do rollback ONLY WHEN we have at least one log
            //      to overwrite.
            ulong my_last_log_idx = log_store_->next_slot() - 1;
            bool rollback_in_progress = false;
            if (my_last_log_idx >= log_idx &&
                cnt < req.log_entries().size()) {
                TLOG(INFO, "rollback logs: {}" " - {}"
                     ", commit idx req {}" ", quick {}" ", sm {}" ", "
                     "num log entries {}, current count {}",
                     log_idx,
                     my_last_log_idx,
                     req.get_commit_idx(),
                     quick_commit_index_.load(),
                     sm_commit_index_.load(),
                     req.log_entries().size(),
                     cnt);
                rollback_in_progress = true;
                // If rollback point is smaller than commit index,
                // should rollback commit index as well
                // (should not happen in Raft though).
                if (quick_commit_index_ >= log_idx) {
                    TLOG(WARNING, "rollback quick commit index from {}" " to {}",
                         quick_commit_index_.load(),
                         log_idx - 1);
                    quick_commit_index_ = log_idx - 1;
                }
                if (sm_commit_index_ >= log_idx) {
                    TLOG(ERROR, "rollback sm commit index from {}" " to {}" ", "
                         "it shouldn't happen and may indicate data loss",
                         sm_commit_index_.load(),
                         log_idx - 1);
                    sm_commit_index_ = log_idx - 1;
                }

                for (uint64_t ii = 0; ii < my_last_log_idx - log_idx + 1; ++ii) {
                    uint64_t idx = my_last_log_idx - ii;
                    ptr<log_entry> old_entry = log_store_->entry_at(idx);
                    ptr<buffer> buf = old_entry->get_buf_ptr();
                    if (old_entry->get_val_type() == log_val_type::app_log) {
                        buf->pos(0);
                        state_machine_->rollback_ext
                                (state_machine::ext_op_params(idx, buf));
                        TLOG(INFO, "rollback log {}" ", term {}",
                             idx, old_entry->get_term());
                    } else if (old_entry->get_val_type() == log_val_type::conf) {
                        ptr<cluster_config> conf_to_rollback =
                                cluster_config::deserialize(*buf);
                        state_machine_->rollback_config(idx, conf_to_rollback);
                        TLOG(INFO, "revert from a prev config change to config at {}",
                             get_config()->get_log_idx());
                        config_changing_ = false;
                    }
                }
            }

            // Dealing with overwrites (logs with different term).
            while (log_idx < log_store_->next_slot() &&
                   cnt < req.log_entries().size()) {
                ptr<log_entry> entry = req.log_entries().at(cnt);
                TLOG(INFO, "overwrite at {}" ", term {}" ", timestamp {}" "\n",
                     log_idx, entry->get_term(), entry->get_timestamp());
                store_log_entry(entry, log_idx);

                if (entry->get_val_type() == log_val_type::app_log) {
                    ptr<buffer> buf = entry->get_buf_ptr();
                    buf->pos(0);
                    state_machine_->pre_commit_ext
                            (state_machine::ext_op_params(log_idx, buf));
                } else if (entry->get_val_type() == log_val_type::conf) {
                    TLOG(INFO, "receive a config change from leader at {}", log_idx);
                    config_changing_ = true;
                }

                log_idx += 1;
                cnt += 1;

                if (stopping_) return resp;
            }
            TLOG(DEBUG, "[after OVWR] log_idx: {}" ", count: {}", log_idx, cnt);

            if (rollback_in_progress) {
                TLOG(INFO, "last log index after rollback and overwrite: {}",
                     log_store_->next_slot() - 1);
            }

            // Append new log entries
            while (cnt < req.log_entries().size()) {
                ptr<log_entry> entry = req.log_entries().at(cnt++);
                TLOG(TRACE, "append at {}" ", term {}" ", timestamp {}" "\n",
                     log_store_->next_slot(), entry->get_term(), entry->get_timestamp());
                ulong idx_for_entry = store_log_entry(entry);
                if (entry->get_val_type() == log_val_type::conf) {
                    TLOG(INFO, "receive a config change from leader at {}",
                         idx_for_entry);
                    config_changing_ = true;
                } else if (entry->get_val_type() == log_val_type::app_log) {
                    ptr<buffer> buf = entry->get_buf_ptr();
                    buf->pos(0);
                    state_machine_->pre_commit_ext
                            (state_machine::ext_op_params(idx_for_entry, buf));
                }

                if (stopping_) return resp;
            }

            // End of batch.
            log_store_->end_of_append_batch(req.get_last_log_idx() + 1,
                                            req.log_entries().size());

            if (params->parallel_log_appending_) {
                uint64_t last_durable_index = log_store_->last_durable_index();
                while (last_durable_index <
                       req.get_last_log_idx() + req.log_entries().size()) {
                    // Some logs are not durable yet, wait here and block the thread.
                    TLOG(TRACE, "durable index {}"
                         ", sleep and wait for log appending completion",
                         last_durable_index);
                    ea_follower_log_append_->wait_ms(params->heart_beat_interval_);

                    // --- `notify_log_append_completion` API will wake it up. ---

                    ea_follower_log_append_->reset();
                    last_durable_index = log_store_->last_durable_index();
                    TLOG(TRACE, "wake up, durable index {}", last_durable_index);
                }
            }
        }

        leader_ = req.get_src();

        // WARNING:
        //   If this node was leader but now follower, and right after
        //   leader election, new leader's committed index can be
        //   smaller than this node's quick/sm commit index.
        //   But that doesn't mean that rollback can happen on
        //   already committed index. Committed log index should never go back,
        //   in the state machine's point of view.
        //
        // e.g.)
        //   1) All replicas have log 1, and also committed up to 1.
        //   2) Leader appends log 2 and 3, replicates them, reaches consensus,
        //      so that commits up to 3.
        //   3) Leader appends a new log 4, but before replicating log 4 with
        //      committed log index 3, leader election happens.
        //   4) New leader has logs up to 3, but its last committed index is still 1.
        //   5) In such case, the old leader's log 4 should be rolled back,
        //      and new leader's commit index (1) can be temporarily smaller
        //      than old leader's commit index (3), but that doesn't mean
        //      old leader's commit index is wrong. New leader will soon commit
        //      logs up to 3 that is identical to what old leader has, and make
        //      progress after that. Logs already reached consensus (1, 2, and 3)
        //      will remain unchanged.
        leader_commit_index_.store(req.get_commit_idx());

        // WARNING:
        //   If `commit_idx > next_slot()`, it may cause problem
        //   on next `append_entries()` call, due to racing
        //   between BG commit thread and appending logs.
        //   Hence, we always should take smaller one.
        ulong target_precommit_index = req.get_last_log_idx() + req.log_entries().size();

        // WARNING:
        //   Since `peer::set_free()` is called prior than response handler
        //   without acquiring `raft_server::lock_`, there can be an edge case
        //   that leader may send duplicate logs, and their last log index may not
        //   be greater than the last log index this server already has. We should
        //   always compare the target index with current precommit index, and take
        //   it only when it is greater than the previous one.
        bool pc_updated = try_update_precommit_index(target_precommit_index);
        if (!pc_updated) {
            // If updating `precommit_index_` failed, we SHOULD NOT update
            // commit index as well.
        } else {
            commit(std::min(req.get_commit_idx(), target_precommit_index));
        }

        resp->accept(target_precommit_index + 1);

        int32 time_ms = tt.get_us() / 1000;
        if (time_ms >= ctx_->get_params()->heart_beat_interval_) {
            // Append entries took longer than HB interval. Warning.
            TLOG(WARNING, "appending entries from peer {} took long time ({} ms)\n"
                 "req type: {}, req term: {}" ", "
                 "req l idx: {}" " ({}), req c idx: {}" ", "
                 "my term: {}" ", my role: {}",
                 req.get_src(), time_ms, (int) req.get_type(), req.get_term(),
                 req.get_last_log_idx(), req.log_entries().size(), req.get_commit_idx(),
                 state_->get_term(), (int) role_);
        }

        // Modified by Jung-Sang Ahn, Mar 28 2018.
        // Restart election timer here, as this function may take long time.
        if (req.get_term() == state_->get_term() &&
            role_ == srv_role::follower) {
            restart_election_timer();
        }

        int64 bs_hint = state_machine_->get_next_batch_size_hint_in_bytes();
        resp->set_next_batch_size_hint_in_bytes(bs_hint);
        if (self_mark_down_) {
            resp->set_extra_flags(
                resp->get_extra_flags() | resp_msg::SELF_MARK_DOWN
            );
        }

        if (ctx_->get_params()->track_peers_sm_commit_idx_) {
            // If peer track mode is enabled, we should send
            // the current SM committed index to the leader.
            resp_appendix appendix;
            appendix.extra_order_ = resp_appendix::NOTIFYING_SM_COMMITTED_INDEX;
            appendix.sm_committed_idx_ = sm_commit_index_.load();
            resp->set_ctx(appendix.serialize());
            TLOG(TRACE, "appended extra order {}, sm committed index: {}",
                 resp_appendix::extra_order_msg(appendix.extra_order_),
                 appendix.sm_committed_idx_);
        }

        TLOG(TRACE, "batch size hint: {}" " bytes, flags: {}",
             bs_hint, resp->get_extra_flags());

        out_of_log_range_ = false;

        return resp;
    }

    bool raft_server::try_update_precommit_index(ulong desired, const size_t MAX_ATTEMPTS) {
        // If `MAX_ATTEMPTS == 0`, try forever.
        size_t num_attempts = 0;
        ulong prev_precommit_index = precommit_index_;
        while (prev_precommit_index < desired &&
               (num_attempts < MAX_ATTEMPTS || MAX_ATTEMPTS == 0)) {
            if (precommit_index_.compare_exchange_strong(prev_precommit_index,
                                                         desired)) {
                return true;
            }
            // Otherwise: retry until `precommit_index_` is equal to or greater than
            //            `desired`.
            num_attempts++;
        }
        if (precommit_index_ >= desired) {
            return true;
        }
        TLOG(ERROR, "updating precommit_index_ failed after {}/{} attempts, "
             "last seen precommit_index_ {}" ", target {}",
             num_attempts, MAX_ATTEMPTS, prev_precommit_index, desired);
        return false;
    }

    void raft_server::handle_append_entries_resp(resp_msg &resp) {
        peer_itor it = peers_.find(resp.get_src());
        if (it == peers_.end()) {
            TLOG(INFO, "the response is from an unknown peer {}", resp.get_src());
            return;
        }

        check_srv_to_leave_timeout();
        if (srv_to_leave_ &&
            srv_to_leave_
        ->
        get_id() == resp.get_src() &&
                srv_to_leave_->is_stepping_down() &&
                resp.get_next_idx() > srv_to_leave_target_idx_
        )
        {
            // Catch-up is done.
            TLOG(INFO, "server to be removed {} fully caught up the "
                 "target config log {}",
                 srv_to_leave_->get_id(),
                 srv_to_leave_target_idx_);
            remove_peer_from_peers(srv_to_leave_);
            reset_srv_to_leave();
            return;
        }

        // If there are pending logs to be synced or commit index need to be advanced,
        // continue to send appendEntries to this peer
        bool need_to_catchup = true;

        ptr<peer> p = it->second;
        TLOG(TRACE, "handle append entries resp (from {}), resp.get_next_idx(): {}",
             (int) p->get_id(), resp.get_next_idx());

        int64 bs_hint = resp.get_next_batch_size_hint_in_bytes();
        TLOG(TRACE, "peer {} batch size hint: {}" " bytes, in-flight: {}" " bytes",
             p->get_id(), bs_hint, p->get_bytes_in_flight());
        p->set_next_batch_size_hint_in_bytes(bs_hint);

        if (resp.get_accepted()) {
            bool new_mark_down_status = (resp.get_extra_flags() & resp_msg::SELF_MARK_DOWN);
            bool old_mark_down_status = p->set_self_mark_down(new_mark_down_status);
            if (old_mark_down_status != new_mark_down_status) {
                TLOG(INFO, "peer {} self mark down status changed from {} to {}",
                     p->get_id(),
                     (old_mark_down_status ? "true" : "false"),
                     (new_mark_down_status ? "true" : "false"));
            }

            uint64_t prev_matched_idx = 0;
            uint64_t new_matched_idx = 0;
            uint64_t prev_sm_committed_idx = p->get_sm_committed_idx();
            uint64_t new_sm_committed_idx = 0;

            {
                std::lock_guard<std::mutex> l(p->get_lock());
                p->set_next_log_idx(resp.get_next_idx());
                prev_matched_idx = p->get_matched_idx();
                new_matched_idx = resp.get_next_idx() - 1;
                TLOG(TRACE, "peer {}, prev matched idx: {}" ", new matched idx: {}",
                     p->get_id(), prev_matched_idx, new_matched_idx);
                p->set_matched_idx(new_matched_idx);
                p->set_last_accepted_log_idx(new_matched_idx);
            }

            bool sm_committed_idx_updated = false;
            if (resp.get_ctx() &&
                ctx_->get_params()->track_peers_sm_commit_idx_) {
                // If the response contains appendix, it should be
                // `resp_appendix` type.
                ptr<resp_appendix> appendix = resp_appendix::deserialize(*resp.get_ctx());
                if (appendix->extra_order_ == resp_appendix::NOTIFYING_SM_COMMITTED_INDEX) {
                    {
                        std::lock_guard<std::mutex> l(p->get_lock());
                        new_sm_committed_idx = appendix->sm_committed_idx_;
                        TLOG(TRACE, "sm committed index of peer {}: {}" " -> {}",
                             p->get_id(), prev_sm_committed_idx, new_sm_committed_idx);
                        p->set_sm_committed_idx(new_sm_committed_idx);
                        sm_committed_idx_updated = true;
                    }

                    if (sm_committed_idx_updated &&
                        prev_sm_committed_idx < new_sm_committed_idx) {
                        uint64_t target_idx = find_sm_commit_idx_to_notify();
                        target_idx = update_sm_commit_notifier_target_idx(target_idx);
                        TLOG(TRACE, "sm commit notify ready: {}" ", target idx: {}"
                             ", notified idx: {}",
                             new_sm_committed_idx, target_idx,
                             sm_commit_notifier_notified_idx_.load());
                        global_mgr *mgr = get_global_mgr();
                        if (mgr) {
                            // Global thread pool exists, request it.
                            mgr->request_commit(this->shared_from_this());
                        } else {
                            std::unique_lock<std::mutex> lock(commit_cv_lock_);
                            commit_cv_.notify_one();
                        }
                    }
                }
            }
            if (!sm_committed_idx_updated) {
                p->set_sm_committed_idx(0);
            }

            cb_func::Param param(id_, leader_, p->get_id());
            param.ctx = &new_matched_idx;
            CbReturnCode rc = ctx_->cb_func_.call
                    (cb_func::GotAppendEntryRespFromPeer, &param);
            (void) rc;

            // Try to enable stream
            int32 max_gap_in_stream = ctx_->get_params()->max_log_gap_in_stream_;
            ulong acceptable_precommit_idx = resp.get_next_idx() +
                                             max_gap_in_stream;
            ulong last_streamed_log_idx = p->get_last_streamed_log_idx();
            TLOG(TRACE, "peer {}, max gap: {}, acceptable_precommit_idx: {}" ", "
                 "last_streamed_log_idx: {}" ", "
                 "last_sent: {}" ", next_idx: {}" "", p->get_id(),
                 max_gap_in_stream, acceptable_precommit_idx, last_streamed_log_idx,
                 p->get_last_sent_idx(), resp.get_next_idx());
            if (max_gap_in_stream > 0 &&
                last_streamed_log_idx == 0 &&
                resp.get_next_idx() > 0 &&
                p->get_last_sent_idx() < resp.get_next_idx() &&
                precommit_index_ < acceptable_precommit_idx) {
                TLOG(INFO, "start stream mode for peer {} at idx: {}" "",
                     p->get_id(), resp.get_next_idx() - 1);
                p->set_last_streamed_log_idx(0, resp.get_next_idx() - 1);
            }

            // Try to commit with this response.
            ulong committed_index = get_expected_committed_log_idx();

            // NOTE:
            //   In full consensus mode, `committed_index` can move back when
            //   a non-responding peer is re-included in the quorum.
            //   However, such decreased `committed_index` should not
            //   cause any problem, as it will be gracefully handled in
            //   `commit()` function below.
            commit(committed_index);

            // As commit might send requests, so refresh streamed log idx here
            last_streamed_log_idx = p->get_last_streamed_log_idx();
            ulong next_idx_to_send = last_streamed_log_idx
                                         ? last_streamed_log_idx + 1
                                         : resp.get_next_idx();
            need_to_catchup = p->clear_pending_commit() ||
                              next_idx_to_send < log_store_->next_slot();
            if (ctx_->get_params()->track_peers_sm_commit_idx_ &&
                p->get_sm_committed_idx() &&
                p->get_sm_committed_idx() < quick_commit_index_) {
                // If peer's SM committed index is lagging behind the
                // quick commit index, we should keep sending messages
                // to listen to the peer's SM commit progress.

                // However, if stream mode is on, and if there are
                // requests already in flight, those requests will do the job.
                // So we don't need to set `need_to_catchup` true in that case.
                bool streaming = last_streamed_log_idx > 0;
                int64_t bytes_in_flight = p->get_bytes_in_flight();
                if (streaming && bytes_in_flight > 0) {
                    // No need to send.
                } else {
                    need_to_catchup = true;
                }
            }
            TLOG(TRACE, "need to catchup peer {}: {}", p->get_id(), need_to_catchup);
        } else {
            std::lock_guard<std::mutex> guard(p->get_lock());
            ulong prev_next_log = p->get_next_log_idx();
            if (resp.get_next_idx() > 0 && prev_next_log > resp.get_next_idx()) {
                // fast move for the peer to catch up
                p->set_next_log_idx(resp.get_next_idx());
            } else {
                bool do_log_rewind = true;
                // If not, check an extra order exists.
                if (resp.get_ctx()) {
                    ptr<resp_appendix> appendix = resp_appendix::deserialize(*resp.get_ctx());
                    if (appendix->extra_order_ == resp_appendix::DO_NOT_REWIND) {
                        do_log_rewind = false;
                    } else if (appendix->extra_order_ == resp_appendix::RECEIVING_SNAPSHOT) {
                        p->set_snapshot_sync_is_needed(true);
                        TLOG(INFO, "peer {} was in snapshot sync mode, re-sending a snapshot. "
                             "peers next log idx: {}" ", resp next idx: {}",
                             p->get_id(), prev_next_log, resp.get_next_idx());
                    }

                    static timer_helper extra_order_timer(1000 * 1000, true);
                    int log_lv = extra_order_timer.timeout_and_reset() ? L_INFO : L_TRACE;
                    switch (log_lv) {
case L_FATAL:
    TLOG(FATAL, "received extra order: {}", resp_appendix::extra_order_msg(appendix->extra_order_));
    break;
case L_ERROR:
    TLOG(ERROR, "received extra order: {}", resp_appendix::extra_order_msg(appendix->extra_order_));
    break;
case L_WARN:
    TLOG(WARNING, "received extra order: {}", resp_appendix::extra_order_msg(appendix->extra_order_));
    break;
case L_INFO:
    TLOG(INFO, "received extra order: {}", resp_appendix::extra_order_msg(appendix->extra_order_));
    break;
case L_DEBUG:
    TLOG(DEBUG, "received extra order: {}", resp_appendix::extra_order_msg(appendix->extra_order_));
    break;
case L_TRACE:
    TLOG(TRACE, "received extra order: {}", resp_appendix::extra_order_msg(appendix->extra_order_));
    break;
default:
    break;
};
                }
                // if not, move one log backward.
                // WARNING: Make sure that `next_log_idx_` shouldn't be smaller than 0.
                if (do_log_rewind && prev_next_log) {
                    p->set_next_log_idx(prev_next_log - 1);
                }
            }
            bool suppress = p->need_to_suppress_error();

            // To avoid verbose logs here.
            static timer_helper log_timer(500 * 1000, true);
            int log_lv = suppress ? L_INFO : L_WARN;
            if (log_lv == L_WARN) {
                if (!log_timer.timeout_and_reset()) {
                    log_lv = L_TRACE;
                }
            }
            switch (log_lv) {
case L_FATAL:
    TLOG(FATAL, "declined append: peer {}, prev next log idx {}" ", "
                 "resp next {}" ", new next log idx {}"
                 ", my start idx: {}" ", my last idx: {}", p->get_id(), prev_next_log, resp.get_next_idx(), p->get_next_log_idx(), log_store_->start_index(), log_store_->next_slot() - 1);
    break;
case L_ERROR:
    TLOG(ERROR, "declined append: peer {}, prev next log idx {}" ", "
                 "resp next {}" ", new next log idx {}"
                 ", my start idx: {}" ", my last idx: {}", p->get_id(), prev_next_log, resp.get_next_idx(), p->get_next_log_idx(), log_store_->start_index(), log_store_->next_slot() - 1);
    break;
case L_WARN:
    TLOG(WARNING, "declined append: peer {}, prev next log idx {}" ", "
                 "resp next {}" ", new next log idx {}"
                 ", my start idx: {}" ", my last idx: {}", p->get_id(), prev_next_log, resp.get_next_idx(), p->get_next_log_idx(), log_store_->start_index(), log_store_->next_slot() - 1);
    break;
case L_INFO:
    TLOG(INFO, "declined append: peer {}, prev next log idx {}" ", "
                 "resp next {}" ", new next log idx {}"
                 ", my start idx: {}" ", my last idx: {}", p->get_id(), prev_next_log, resp.get_next_idx(), p->get_next_log_idx(), log_store_->start_index(), log_store_->next_slot() - 1);
    break;
case L_DEBUG:
    TLOG(DEBUG, "declined append: peer {}, prev next log idx {}" ", "
                 "resp next {}" ", new next log idx {}"
                 ", my start idx: {}" ", my last idx: {}", p->get_id(), prev_next_log, resp.get_next_idx(), p->get_next_log_idx(), log_store_->start_index(), log_store_->next_slot() - 1);
    break;
case L_TRACE:
    TLOG(TRACE, "declined append: peer {}, prev next log idx {}" ", "
                 "resp next {}" ", new next log idx {}"
                 ", my start idx: {}" ", my last idx: {}", p->get_id(), prev_next_log, resp.get_next_idx(), p->get_next_log_idx(), log_store_->start_index(), log_store_->next_slot() - 1);
    break;
default:
    break;
};

            // disable stream
            uint64_t last_streamed_log_idx = p->get_last_streamed_log_idx();
            p->reset_stream();
            if (last_streamed_log_idx) {
                TLOG(INFO, "stop stream mode for peer {} at idx: {}" "",
                     p->get_id(), last_streamed_log_idx);
            }
        }

        if (!config_changing_ && p->get_config().is_new_joiner()) {
            auto params = ctx_->get_params();
            uint64_t log_sync_stop_gap =
                    params->log_sync_stop_gap_ ? params->log_sync_stop_gap_ : 1;
            uint64_t matched_idx = p->get_matched_idx();
            uint64_t next_slot = log_store_->next_slot();
            if (matched_idx + log_sync_stop_gap >= next_slot) {
                TLOG(INFO, "peer {} is no longer a new joiner, matched index: {}" ", "
                     "next slot: {}" ", sync stop gap: {}"
                     ", set new joiner flag to false",
                     p->get_id(), matched_idx, next_slot, log_sync_stop_gap);

                // Clone the current cluster config.
                ptr<cluster_config> cur_conf = get_config();
                ptr<buffer> enc_conf_buf = cur_conf->serialize();
                ptr<cluster_config> new_conf = cluster_config::deserialize(*enc_conf_buf);
                new_conf->set_log_idx(log_store_->next_slot());

                // Remove new joiner flag.
                for (auto &ss: new_conf->get_servers()) {
                    if (ss->get_id() == p->get_id()) {
                        ss->set_new_joiner(false);
                        break;
                    }
                }

                ptr<buffer> new_conf_buf(new_conf->serialize());
                ptr<log_entry> entry(cs_new<log_entry>(state_->get_term(),
                                                       new_conf_buf,
                                                       log_val_type::conf,
                                                       timer_helper::get_timeofday_us()));
                store_log_entry(entry);
                config_changing_ = true;
                uncommitted_config_ = new_conf;
                request_append_entries();
                return;
            }
        }

        // NOTE:
        //   If all other followers are not responding, we may not make
        //   below condition true. In that case, we check the timeout of
        //   re-election timer in heartbeat handler, and do force resign.
        ulong p_matched_idx = p->get_matched_idx();
        if (write_paused_ &&
            p
        ->
        get_id() == next_leader_candidate_ &&
                p_matched_idx &&
                p_matched_idx == log_store_->next_slot() - 1 &&
                p->make_busy()
        )
        {
            // NOTE:
            //   If `make_busy` fails (very unlikely to happen), next
            //   response handler (of heartbeat, append_entries ..) will
            //   retry this.
            TLOG(INFO, "ready to resign, server id {}, "
                 "latest log index {}" ", "
                 "{}" " us elapsed, resign now",
                 next_leader_candidate_.load(),
                 p_matched_idx,
                 reelection_timer_.get_us());
            leader_ = -1;

            // To avoid this node becomes next leader again, set timeout
            // value bigger than any others, just once at this time.
            rand_timeout_ = [this]() -> int32 {
                return this->ctx_->get_params()->election_timeout_upper_bound_ +
                       this->ctx_->get_params()->election_timeout_lower_bound_;
            };
            become_follower();
            update_rand_timeout();

            // Clear live flag to avoid pre-vote rejection.
            hb_alive_ = false;

            // Send leadership takeover request to this follower.
            ptr<req_msg> req = cs_new<req_msg>
            (state_->get_term(),
             msg_type::custom_notification_request,
             id_, p->get_id(),
             term_for_log(log_store_->next_slot() - 1),
             log_store_->next_slot() - 1,
             quick_commit_index_.load());

            // Create a notification.
            ptr<custom_notification_msg> custom_noti =
                    cs_new<custom_notification_msg>
                    (custom_notification_msg::leadership_takeover);

            // Wrap it using log_entry.
            ptr<log_entry> custom_noti_le =
                    cs_new<log_entry>(0, custom_noti->serialize(), log_val_type::custom);

            req->log_entries().push_back(custom_noti_le);
            p->send_req(p, req, resp_handler_);
            return;
        }

        if (bs_hint < 0) {
            // If hint is a negative number, we should set `need_to_catchup`
            // to `false` to avoid sending meaningless messages continuously
            // which eats up CPU. Then the leader will send heartbeats only.
            need_to_catchup = false;
        }

        // This may not be a leader anymore,
        // such as the response was sent out long time ago
        // and the role was updated by UpdateTerm call
        // Try to match up the logs for this peer
        if (role_ == srv_role::leader) {
            if (need_to_catchup) {
                TLOG(DEBUG, "reqeust append entries need to catchup, p {}\n",
                     (int) p->get_id());
                request_append_entries(p);
            }
            if (status_check_timer_.timeout_and_reset()) {
                check_overall_status();
            }
        }
    }

    uint64_t raft_server::get_current_leader_index() {
        uint64_t leader_index = precommit_index_;
        ptr<raft_params> params = ctx_->get_params();
        if (params->parallel_log_appending_) {
            // For parallel appending, take the smaller one.
            uint64_t durable_index = log_store_->last_durable_index();
            TLOG(TRACE, "last durable index {}" ", precommit index {}",
                 durable_index, precommit_index_.load());
            leader_index = std::min(precommit_index_.load(), durable_index);
        }
        return leader_index;
    }

    ulong raft_server::get_expected_committed_log_idx() {
        std::vector<ulong> matched_indexes;
        state_machine::adjust_commit_index_params aci_params;
        matched_indexes.reserve(16);
        aci_params.peer_index_map_.reserve(16);

        // Put the index of leader itself.
        uint64_t leader_index = get_current_leader_index();
        matched_indexes.push_back(leader_index);
        aci_params.peer_index_map_[id_] = leader_index;

        for (auto &entry: peers_) {
            ptr<peer> &p = entry.second;
            aci_params.peer_index_map_[p->get_id()] = p->get_matched_idx();

            if (!is_regular_member(p)) continue;
            matched_indexes.push_back(p->get_matched_idx());
        }
        int voting_members = get_num_voting_members();
        assert((int32) matched_indexes.size() == voting_members);

        // NOTE: Descending order.
        //       e.g.) 100 100 99 95 92
        //             => commit on 99 if `quorum_idx == 2`.
        std::sort(matched_indexes.begin(),
                  matched_indexes.end(),
                  std::greater<ulong>());

        size_t quorum_idx = get_quorum_for_commit();
        ptr<raft_params> params = ctx_->get_params();

        if (ctx_->get_params()->use_full_consensus_among_healthy_members_ &&
            params->custom_commit_quorum_size_ == 0) {
            // In full consensus mode, a peer is considered unhealthy when
            //   1) it is not responding for 3 times of heartbeat interval, or
            //   2) its last log index is smaller (older) than
            //      the current committed log index - max batch size.
            //
            // WARNING: If custom quorum size is set, we should prioritize
            //          the custom quorum size over full consensus mode.

            int32_t allowed_interval =
                    params->heart_beat_interval_ *
                    raft_server::raft_limits_.full_consensus_leader_limit_;;
            uint64_t allowed_log_index =
                    quick_commit_index_ > (uint64_t) params->max_append_size_
                        ? quick_commit_index_ - params->max_append_size_
                        : 0;

            size_t not_responding_peers =
                    get_not_responding_peers_count(allowed_interval, allowed_log_index);

            if (not_responding_peers < voting_members - quorum_idx) {
                // If full consensus option is on, commit should be
                // agreed by all healthy members, and the number of
                // aggreed members should be bigger than regular quorum size.
                size_t prev_quorum_idx = quorum_idx;
                quorum_idx = voting_members - not_responding_peers - 1;
                TLOG(TRACE, "full consensus mode: {} peers are not responding out of {}, "
                     "adjust quorum idx {} -> {}",
                     not_responding_peers, voting_members,
                     prev_quorum_idx, quorum_idx);
            } else {
                // Majority of voting members are not responding.
                // We should not commit anything (regardless of full consensus mode).
                TLOG(TRACE, "full consensus mode, but {} peers are not responding, "
                     "required quorum size {}/{}",
                     not_responding_peers, quorum_idx + 1, voting_members);
            }
        }

        {
            std::string tmp_str = "[";
            for (size_t ii = 0; ii < matched_indexes.size(); ++ii) {
                tmp_str += std::to_string(matched_indexes[ii]);
                if (ii == quorum_idx) {
                    tmp_str += "] ";
                } else {
                    tmp_str += " ";
                }
            }
            TLOG(TRACE, "quorum idx {}, {}", quorum_idx, tmp_str.c_str());
        }

        aci_params.current_commit_index_ = quick_commit_index_;
        aci_params.expected_commit_index_ = matched_indexes[quorum_idx];
        uint64_t adjusted_commit_index = state_machine_->adjust_commit_index(aci_params);
        if (aci_params.expected_commit_index_ != adjusted_commit_index) {
            TLOG(TRACE, "commit index adjusted: {}" " -> {}",
                 aci_params.expected_commit_index_, adjusted_commit_index);
        }
        return adjusted_commit_index;
    }

    void raft_server::notify_log_append_completion(bool ok) {
        TLOG(TRACE, "got log append completion notification: {}", ok ? "OK" : "FAILED");

        if (role_ == srv_role::leader) {
            recur_lock(lock_);
            if (!ok) {
                // If log appending fails, leader should resign immediately.
                TLOG(ERROR, "log appending failed, resign immediately");
                leader_ = -1;
                become_follower();

                // Clear this flag to avoid pre-vote rejection.
                hb_alive_ = false;
                return;
            }

            // Leader: commit the log and send append_entries request, if needed.
            uint64_t prev_committed_index = quick_commit_index_.load();
            uint64_t committed_index = get_expected_committed_log_idx();
            commit(committed_index);

            if (quick_commit_index_ > prev_committed_index) {
                // Commit index has been changed as a result of log appending.
                // Send replication messages.
                request_append_entries_for_all();
            }
        } else {
            if (!ok) {
                // If log appending fails for follower, there is no way to proceed it.
                // We should stop the server immediately.
                recur_lock(lock_);
                TLOG(FATAL, "log appending failed, stop this server");
                ctx_->state_mgr_->system_exit(N21_log_flush_failed);
                return;
            }

            // Follower: wake up the waiting thread.
            ea_follower_log_append_->invoke();
        }
    }
} // namespace nuraft;

