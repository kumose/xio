// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <xio/compress/detail/backends.h>
#include <xio/compress/detail/output_tail.h>

#include <zlib.h>

#include <array>
#include <cstdint>

namespace xio::compress::detail {
namespace {

turbo::Status append_chunk(xio::Buffer *buf, const void *data, size_t size) {
    if (size == 0) {
        return turbo::OkStatus();
    }
    return buf->append(data, size);
}

int gzip_level(CompressOptions opts) {
    return opts.level == 0 ? 6 : opts.level;
}

} // namespace

size_t gzip_compress_bound(size_t plain_len) {
    return static_cast<size_t>(::compressBound(static_cast<uLong>(plain_len)));
}

turbo::Status gzip_compress_handler(const Compressor &, CompressOptions opts, const xio::Buffer &in, Output *out) {
    OutputTail tail(out);

    z_stream strm{};
    if (::deflateInit2(&strm, gzip_level(opts), Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return turbo::internal_error("gzip deflate init failed");
    }

    std::array<char, xio::kBlockSize> scratch{};
    for (auto it = in.buffer_begin(); it != in.buffer_end(); ++it) {
        strm.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(it->data()));
        strm.avail_in = static_cast<uInt>(it->size());
        while (strm.avail_in > 0) {
            strm.next_out = reinterpret_cast<Bytef *>(scratch.data());
            strm.avail_out = static_cast<uInt>(scratch.size());
            const int r = ::deflate(&strm, Z_NO_FLUSH);
            const size_t produced = scratch.size() - strm.avail_out;
            turbo::Status st = append_chunk(out->buf, scratch.data(), produced);
            if (!st.ok()) {
                ::deflateEnd(&strm);
                return st;
            }
            if (r != Z_OK) {
                ::deflateEnd(&strm);
                return turbo::internal_error("gzip deflate failed");
            }
        }
    }

    while (true) {
        strm.next_out = reinterpret_cast<Bytef *>(scratch.data());
        strm.avail_out = static_cast<uInt>(scratch.size());
        const int r = ::deflate(&strm, Z_FINISH);
        const size_t produced = scratch.size() - strm.avail_out;
        turbo::Status st = append_chunk(out->buf, scratch.data(), produced);
        if (!st.ok()) {
            ::deflateEnd(&strm);
            return st;
        }
        if (r == Z_STREAM_END) {
            break;
        }
        if (r != Z_OK) {
            ::deflateEnd(&strm);
            return turbo::internal_error("gzip deflate finish failed");
        }
    }

    ::deflateEnd(&strm);
    tail.success();
    return turbo::OkStatus();
}

turbo::Status gzip_decompress_handler(const Compressor &, DecompressOptions, const xio::Buffer &in, Output *out) {
    OutputTail tail(out);

    z_stream strm{};
    if (::inflateInit2(&strm, 15 | 32) != Z_OK) {
        return turbo::internal_error("gzip inflate init failed");
    }

    std::array<char, xio::kBlockSize> scratch{};
    for (auto it = in.buffer_begin(); it != in.buffer_end(); ++it) {
        strm.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(it->data()));
        strm.avail_in = static_cast<uInt>(it->size());
        while (strm.avail_in > 0) {
            strm.next_out = reinterpret_cast<Bytef *>(scratch.data());
            strm.avail_out = static_cast<uInt>(scratch.size());
            const int r = ::inflate(&strm, Z_NO_FLUSH);
            const size_t produced = scratch.size() - strm.avail_out;
            turbo::Status st = append_chunk(out->buf, scratch.data(), produced);
            if (!st.ok()) {
                ::inflateEnd(&strm);
                return st;
            }
            if (r == Z_STREAM_ERROR || r == Z_NEED_DICT || r == Z_DATA_ERROR || r == Z_MEM_ERROR) {
                ::inflateEnd(&strm);
                return turbo::data_loss_error("gzip inflate failed");
            }
            if (r == Z_STREAM_END) {
                ::inflateEnd(&strm);
                tail.success();
                return turbo::OkStatus();
            }
            if (r != Z_OK) {
                ::inflateEnd(&strm);
                return turbo::data_loss_error("gzip inflate failed");
            }
        }
    }

    while (true) {
        strm.next_in = nullptr;
        strm.avail_in = 0;
        strm.next_out = reinterpret_cast<Bytef *>(scratch.data());
        strm.avail_out = static_cast<uInt>(scratch.size());
        const int r = ::inflate(&strm, Z_NO_FLUSH);
        const size_t produced = scratch.size() - strm.avail_out;
        turbo::Status st = append_chunk(out->buf, scratch.data(), produced);
        if (!st.ok()) {
            ::inflateEnd(&strm);
            return st;
        }
        if (r == Z_STREAM_END) {
            ::inflateEnd(&strm);
            tail.success();
            return turbo::OkStatus();
        }
        if (r == Z_STREAM_ERROR || r == Z_NEED_DICT || r == Z_DATA_ERROR || r == Z_MEM_ERROR) {
            ::inflateEnd(&strm);
            return turbo::data_loss_error("gzip inflate finish failed");
        }
        if (r != Z_OK && r != Z_BUF_ERROR) {
            ::inflateEnd(&strm);
            return turbo::data_loss_error("gzip inflate finish failed");
        }
        if (produced == 0 && r == Z_BUF_ERROR) {
            ::inflateEnd(&strm);
            return turbo::data_loss_error("gzip stream incomplete");
        }
    }
}

} // namespace xio::compress::detail
