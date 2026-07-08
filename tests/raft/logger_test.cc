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

#include <xio/logging.h>

#include <tests/raft/test_common.h>

#include <string>

namespace logger_test {

int logger_basic_test() {
    TLOG(DEBUG, "hello {}", "world");
    return 0;
}

int logger_long_line_test() {
    for (int i = 1800; i < 4096; ++i) {
        const std::string str(i, 'a');
        TLOG(DEBUG, "long string: {}", str);
    }

    for (int i = 1; i < 1024; i *= 2) {
        const std::string str(1024UL * i, 'a');
        TLOG(DEBUG, "long string: {}", str);
    }

    return 0;
}

} // namespace logger_test
using namespace logger_test;

int main(int argc, char** argv) {
    TestSuite ts(argc, argv);

    ts.options.printTestMessage = true;

    ts.doTest("logger basic test", logger_basic_test);

    ts.doTest("logger long line test", logger_long_line_test);

    return 0;
}
