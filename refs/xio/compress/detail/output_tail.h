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

#pragma once

#include <xio/compress/compress.h>

namespace xio::compress::detail {

    class OutputTail {
    public:
        explicit OutputTail(Output *out) : out_(out), mark_(out != nullptr && out->buf != nullptr ? out->buf->size() : 0) {}

        OutputTail(const OutputTail &) = delete;

        OutputTail &operator=(const OutputTail &) = delete;

        ~OutputTail() {
            if (!ok_ && out_ != nullptr && out_->buf != nullptr) {
                const size_t cur = out_->buf->size();
                if (cur > mark_) {
                    out_->buf->pop_back(cur - mark_);
                }
                out_->bytes_written = 0;
            }
        }

        void success() {
            ok_ = true;
            if (out_ != nullptr && out_->buf != nullptr) {
                out_->bytes_written = out_->buf->size() - mark_;
            }
        }

    private:
        Output *out_;
        size_t mark_;
        bool ok_{false};
    };

} // namespace xio::compress::detail
