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

#include <xlog/logging.h>

// NuRaft legacy log levels used by rate-limited logging switches.
constexpr int L_FATAL = 1;
constexpr int L_ERROR = 2;
constexpr int L_WARN = 3;
constexpr int L_INFO = 4;
constexpr int L_DEBUG = 5;
constexpr int L_TRACE = 6;
