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

#include <fermat/container/cord_buffer.h>

#ifndef DEFAULT_BUFFER_BLOCK
#define DEFAULT_BUFFER_BLOCK (16 * 1024)
#endif

#ifndef DEFAULT_BUFFER_ALIGNMENT
#define DEFAULT_BUFFER_ALIGNMENT 64
#endif

namespace xio {

    static constexpr size_t kBufferBlockSize{DEFAULT_BUFFER_BLOCK};
    static constexpr size_t kAlignment{DEFAULT_BUFFER_ALIGNMENT};

    /// Buffer<char, Alignment>
    using BufferType = fermat::CordBufferBase<kAlignment, kBufferBlockSize>::buffer_type;

    using CordBufferType = fermat::CordBufferBase<kAlignment, kBufferBlockSize>;

    enum class IOState {
        /// may have more data
        kNone,
        /// reach eagin
        kNoMoreData,
        /// reach read 0 or write 0
        kPeerClose,
        kError,
    };
    struct IoResult {
        int64_t size{0};
        IOState state{IOState::kNone};
        int     error{0};
    };

    inline std::ostream& operator<<(std::ostream& os, IoResult r) {
        os <<"size:"<< r.size << " state:" << static_cast<int>(r.state) << " error:" << r.error;
        return os;
    }
}  // namespace xio
