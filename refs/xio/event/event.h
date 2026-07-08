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

#include <cstdint>

namespace xio {
    enum class EventType : int32_t {
        kEventNone = 0x00,
        kEventRead = 1 << 0,
        kEventWrite = 1 << 1,
        kEventError = 1 << 2,
        kEventHangUp = 1 << 3,
        kEventReadHup = 1 << 4,
        kEventPriority = 1 << 5,
    };

    constexpr int32_t kEventMask =
            static_cast<int32_t>(EventType::kEventRead) |
            static_cast<int32_t>(EventType::kEventWrite) |
            static_cast<int32_t>(EventType::kEventError) |
            static_cast<int32_t>(EventType::kEventHangUp) |
            static_cast<int32_t>(EventType::kEventReadHup) |
            static_cast<int32_t>(EventType::kEventPriority);

    /// Returns the bitwise NOT of an EventType, masking out undefined bits.
    inline EventType operator~(EventType e) {
        return static_cast<EventType>(~static_cast<int32_t>(e) & kEventMask);
    }

    /// Returns the bitwise OR of two EventType values.
    inline EventType operator|(EventType lhs, EventType rhs) {
        return static_cast<EventType>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
    }

    /// Returns the bitwise AND of two EventType values.
    inline EventType operator&(EventType lhs, EventType rhs) {
        return static_cast<EventType>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
    }

    /// Returns the bitwise XOR of two EventType values.
    inline EventType operator^(EventType lhs, EventType rhs) {
        return static_cast<EventType>(static_cast<int32_t>(lhs) ^ static_cast<int32_t>(rhs));
    }

    /// Performs bitwise OR assignment on an EventType.
    inline EventType& operator|=(EventType& lhs, EventType rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    /// Performs bitwise AND assignment on an EventType.
    inline EventType& operator&=(EventType& lhs, EventType rhs) {
        lhs = lhs & rhs;
        return lhs;
    }

    /// Performs bitwise XOR assignment on an EventType.
    inline EventType& operator^=(EventType& lhs, EventType rhs) {
        lhs = lhs ^ rhs;
        return lhs;
    }

} // namespace xio
