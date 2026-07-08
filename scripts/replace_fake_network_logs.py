#!/usr/bin/env python3
"""Replace fake_network SimpleLogger _log_* calls with TLOG."""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

MACRO_MAP = [
    ("_log_trace(", "TLOG(TRACE, "),
    ("_log_debug(", "TLOG(DEBUG, "),
    ("_log_info(", "TLOG(INFO, "),
    ("_log_warn(", "TLOG(WARNING, "),
    ("_log_err(", "TLOG(ERROR, "),
    ("_log_fatal(", "TLOG(FATAL, "),
    ("_log_sys(", "TLOG(INFO, "),
]

FMT_REPLACEMENTS = [
    (r'%" PRIu64', '{}"'),
    (r'%" PRId64', '{}"'),
    (r'%" PRIi64', '{}"'),
    (r'%" PRIx64', '{}"'),
    (r"%zu", "{}"),
    (r"%lld", "{}"),
    (r"%ld", "{}"),
    (r"%lu", "{}"),
    (r"%d", "{}"),
    (r"%u", "{}"),
    (r"%s", "{}"),
    (r"%p", "{}"),
    (r"%x", "{}"),
]


def convert_printf_fmt(text: str) -> str:
    for pattern, repl in FMT_REPLACEMENTS:
        text = re.sub(pattern, repl, text)
    return text


def strip_logger_first_arg(text: str) -> str:
    for old, new in MACRO_MAP:
        idx = 0
        out = []
        while True:
            pos = text.find(old, idx)
            if pos == -1:
                out.append(text[idx:])
                break
            out.append(text[idx:pos])
            out.append(new)
            i = pos + len(old)
            # skip first argument (logger pointer)
            depth = 0
            in_string = False
            escape = False
            while i < len(text):
                ch = text[i]
                if in_string:
                    if escape:
                        escape = False
                    elif ch == "\\":
                        escape = True
                    elif ch == '"':
                        in_string = False
                    i += 1
                    continue
                if ch == '"':
                    in_string = True
                elif ch == "(":
                    depth += 1
                elif ch == ")":
                    depth -= 1
                elif ch == "," and depth == 0:
                    i += 1
                    while i < len(text) and text[i].isspace():
                        i += 1
                    break
                i += 1
            idx = i
        text = "".join(out)
    return text


def main() -> None:
    path = ROOT / "tests" / "raft" / "fake_network.cc"
    text = path.read_text(encoding="utf-8")
    text = text.replace('#include "logger.h"\n', '#include <xio/logging.h>\n')
    text = re.sub(r"^\s*SimpleLogger\* ll = .*?;\s*\n", "", text, flags=re.MULTILINE)
    text = strip_logger_first_arg(text)
    text = convert_printf_fmt(text)
    text = re.sub(
        r"FakeNetworkBase::FakeNetworkBase\(\) \{[^}]+\}\n\n",
        "FakeNetworkBase::FakeNetworkBase() {}\n\n",
        text,
        flags=re.DOTALL,
    )
    text = text.replace("    SimpleLogger::shutdown();\n", "")
    text = re.sub(
        r"FakeTimer::FakeTimer\(const std::string& endpoint,\s*SimpleLogger\* logger\)\s*:\s*myEndpoint\(endpoint\)\s*,\s*myLog\(logger\)\s*\{\s*\}",
        "FakeTimer::FakeTimer(const std::string& endpoint)\n    : myEndpoint(endpoint)\n{}",
        text,
    )
    text = text.replace("myLog,", "")
    text = text.replace("myLog)", ")")
    path.write_text(text, encoding="utf-8")
    print(f"updated {path.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
