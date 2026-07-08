#!/usr/bin/env python3
"""Generate per-project CMakeLists.txt for cpp14/cpp17 examples."""

from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

HEADER = """\
# Copyright (C) Kumo inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
"""

# rel_dir under cpp17 -> (target_suffix, explicit_sources or None for all *.cpp)
CPP17_BUNDLES: dict[str, tuple[str, list[str] | None]] = {
    'http/server': ('http_server', None),
    'http/server2': ('http_server2', None),
    'http/server3': ('http_server3', None),
    'http/server4': ('http_server4', None),
    'type_erasure': ('type_erasure', None),
    'services': ('daytime_client', ['daytime_client.cpp', 'logger_service.cpp']),
}

CPP17_SKIP_SOURCES = {'logger_service.cpp'}
CPP17_SKIP_DIRS = {'windows'}
CPP17_SKIP_SUBDIRS = {'doc_root'}


def has_cpp(d: Path) -> bool:
    return any(d.glob('*.cpp'))


def is_leaf_dir(d: Path) -> bool:
    if not has_cpp(d):
        return False
    for sub in d.iterdir():
        if sub.is_dir() and has_cpp(sub):
            return False
    return True


def collect_leaf_dirs(std_root: Path) -> list[str]:
    out: list[str] = []
    for path in sorted(std_root.rglob('*')):
        if not path.is_dir():
            continue
        rel = path.relative_to(std_root).as_posix()
        if rel in CPP17_SKIP_SUBDIRS or rel.split('/')[0] in CPP17_SKIP_SUBDIRS:
            continue
        if is_leaf_dir(path):
            out.append(rel)
    return out


def bundle_sources(std_root: Path, rel_dir: str, sources: list[str] | None) -> list[str]:
    if sources is not None:
        return sources
    d = std_root / rel_dir
    return sorted(p.name for p in d.glob('*.cpp'))


def write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding='utf-8')


def cmake_binary(name: str, sources: list[str], std: str) -> str:
    src_lines = '\n'.join(f'        {s}' for s in sources)
    root_var = f'CPP{std}_EXAMPLE_ROOT'
    return f"""\
kmcmake_cc_binary(
        NAME {name}
        SOURCES
{src_lines}
        CXXOPTS ${{KMCMAKE_CXX_OPTIONS}} -std=c++{std}
        INCLUDES
        ${{CMAKE_CURRENT_SOURCE_DIR}}
        ${{{root_var}}}
        LINKS xio::xio_static ${{KMCMAKE_DEPS_LINK}}
)
"""


def gen_leaf(std: str, std_root: Path, rel_dir: str) -> str:
    prefix = f'cpp{std}'
    body: list[str] = []

    if std == '17' and rel_dir in CPP17_BUNDLES:
        target, srcs = CPP17_BUNDLES[rel_dir]
        sources = bundle_sources(std_root, rel_dir, srcs)
        body.append(cmake_binary(f'{prefix}_{target}', sources, std))
        return HEADER + body[0] + '\n'

    d = std_root / rel_dir
    skip = CPP17_SKIP_SOURCES if std == '17' else set()
    for cpp in sorted(d.glob('*.cpp')):
        if cpp.name in skip:
            continue
        target = f'{prefix}_{rel_dir.replace("/", "_")}_{cpp.stem}'
        body.append(cmake_binary(target, [cpp.name], std))
        body.append('')

    return HEADER + '\n'.join(body)


def gen_std(std: str) -> None:
    std_root = ROOT / 'examples' / f'cpp{std}'
    prefix = f'cpp{std}'
    leaf_dirs = collect_leaf_dirs(std_root)

    top_lines = [
        HEADER.rstrip(),
        f'set(CPP{std}_EXAMPLE_ROOT ${{CMAKE_CURRENT_SOURCE_DIR}})',
        '',
    ]

    for rel in sorted(leaf_dirs):
        parts = rel.split('/')
        if std == '17' and parts[0] in {'http', 'tutorial'}:
            continue
        if std == '17' and parts[0] in CPP17_SKIP_DIRS:
            continue
        top_lines.append(f'add_subdirectory({rel})')

    if std == '17':
        top_lines.extend(['', 'if(WIN32)', '    add_subdirectory(windows)', 'endif()', 'add_subdirectory(http)', 'add_subdirectory(tutorial)', ''])

    write(std_root / 'CMakeLists.txt', '\n'.join(top_lines))

    for rel in sorted(leaf_dirs):
        write(std_root / rel / 'CMakeLists.txt', gen_leaf(std, std_root, rel))

    if std == '17':
        http_lines = [HEADER.rstrip(), '']
        for rel in sorted(leaf_dirs):
            if rel.startswith('http/'):
                sub = rel.split('/', 1)[1]
                if sub in CPP17_SKIP_SUBDIRS:
                    continue
                http_lines.append(f'add_subdirectory({sub})')
        write(std_root / 'http' / 'CMakeLists.txt', '\n'.join(http_lines) + '\n')

        tut_lines = [HEADER.rstrip(), '']
        for rel in sorted(leaf_dirs):
            if rel.startswith('tutorial/'):
                tut_lines.append(f'add_subdirectory({rel.split("/", 1)[1]})')
        write(std_root / 'tutorial' / 'CMakeLists.txt', '\n'.join(tut_lines) + '\n')


def main() -> None:
    gen_std('14')
    gen_std('17')
    print('generated example CMakeLists.txt files')


if __name__ == '__main__':
    main()
