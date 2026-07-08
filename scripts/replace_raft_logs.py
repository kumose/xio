#!/usr/bin/env python3
"""Batch replace Raft p_* logging macros with TLOG."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

MACRO_MAP = [
    ("p_tr(", "TLOG(TRACE, "),
    ("p_dv(", "TLOG(DEBUG, "),
    ("p_db(", "TLOG(DEBUG, "),
    ("p_in(", "TLOG(INFO, "),
    ("p_wn(", "TLOG(WARNING, "),
    ("p_er(", "TLOG(ERROR, "),
    ("p_ft(", "TLOG(FATAL, "),
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

INCLUDE_FROM = "#include <xio/raft/tracer.h>"
INCLUDE_TO = "#include <xio/logging.h>"

P_LV_LEVEL_MAP = {
    "L_FATAL": "FATAL",
    "L_ERROR": "ERROR",
    "L_WARN": "WARNING",
    "L_INFO": "INFO",
    "L_DEBUG": "DEBUG",
    "L_TRACE": "TRACE",
}


def convert_printf_fmt(text: str) -> str:
    for pattern, repl in FMT_REPLACEMENTS:
        text = re.sub(pattern, repl, text)
    return text


def replace_macros(text: str) -> str:
    for old, new in MACRO_MAP:
        text = text.replace(old, new)
    return text


def find_matching_paren(text: str, open_idx: int) -> int:
    depth = 0
    i = open_idx
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
            if depth == 0:
                return i
        i += 1
    raise ValueError("unmatched parenthesis")


def split_top_level_args(args: str) -> list[str]:
    parts: list[str] = []
    start = 0
    depth = 0
    in_string = False
    escape = False
    for i, ch in enumerate(args):
        if in_string:
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == '"':
                in_string = False
            continue
        if ch == '"':
            in_string = True
        elif ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
        elif ch == "," and depth == 0:
            parts.append(args[start:i].strip())
            start = i + 1
    tail = args[start:].strip()
    if tail:
        parts.append(tail)
    return parts


def convert_format_args(args: list[str]) -> str:
    converted: list[str] = []
    for arg in args:
        stripped = arg.lstrip()
        if stripped.startswith('"'):
            converted.append(convert_printf_fmt(arg))
        else:
            converted.append(arg)
    return ", ".join(converted)


def emit_tlog(severity: str, body_args: list[str]) -> str:
    return f"TLOG({severity}, {convert_format_args(body_args)})"


def replace_p_lv_calls(text: str) -> str:
    needle = "p_lv("
    out: list[str] = []
    idx = 0
    while True:
        pos = text.find(needle, idx)
        if pos == -1:
            out.append(text[idx:])
            break
        out.append(text[idx:pos])
        open_paren = pos + len("p_lv")
        close_paren = find_matching_paren(text, open_paren)
        args = text[open_paren + 1 : close_paren]
        parts = split_top_level_args(args)
        if len(parts) < 2:
            raise ValueError(f"invalid p_lv args: {args!r}")
        level_expr = parts[0].strip()
        body_args = parts[1:]
        if level_expr in P_LV_LEVEL_MAP:
            out.append(emit_tlog(P_LV_LEVEL_MAP[level_expr], body_args))
        else:
            switch_lines = [f"switch ({level_expr}) {{"]
            for lv, sev in P_LV_LEVEL_MAP.items():
                switch_lines.append(f"case {lv}:")
                switch_lines.append(f"    {emit_tlog(sev, body_args)};")
                switch_lines.append("    break;")
            switch_lines.append("default:")
            switch_lines.append("    break;")
            switch_lines.append("}")
            out.append("\n".join(switch_lines))
        idx = close_paren + 1
    return "".join(out)


def process_file(path: Path) -> bool:
    original = path.read_text(encoding="utf-8")
    text = original
    text = text.replace(INCLUDE_FROM, INCLUDE_TO)
    text = replace_p_lv_calls(text)
    text = replace_macros(text)
    text = convert_printf_fmt(text)
    text = re.sub(
        r"^\s*logger \*l_ = .*?;\s*\n",
        "",
        text,
        flags=re.MULTILINE,
    )
    if text != original:
        path.write_text(text, encoding="utf-8")
        return True
    return False


def main() -> int:
    targets: list[Path] = []
    targets.extend(sorted((ROOT / "xio" / "raft").glob("*.cc")))
    targets.append(ROOT / "tests" / "raft" / "logger_test.cc")

    changed = 0
    for path in targets:
        if path.exists() and process_file(path):
            print(f"updated {path.relative_to(ROOT)}")
            changed += 1

    tracer = ROOT / "xio" / "raft" / "tracer.h"
    if tracer.exists():
        tracer.unlink()
        print("deleted xio/raft/tracer.h")

    print(f"done, {changed} files updated")
    return 0


if __name__ == "__main__":
    sys.exit(main())
