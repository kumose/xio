# xio

[中文版](./README_CN.md)

C++ asynchronous I/O and networking library, with integrated Raft consensus support.

xio is derived from Boost.Asio: the public API lives in the `xio` namespace and provides the familiar async model (`io_context`, completion tokens, coroutines, SSL, timers, and more). The Raft module (`xio/raft/`) embeds NuRaft on top of the xio transport layer for replicated state machines.

## Features

- **Async networking** — TCP/UDP sockets, acceptors, resolvers, local sockets, SSL
- **Async I/O utilities** — timers, signals, files, pipes, serial ports (platform-dependent)
- **Modern C++** — completion tokens, `co_await` / `co_spawn` (C++20), thread pools, strands
- **Raft consensus** — leader election, log replication, snapshots, custom state machines
- **Unified logging** — Raft and runtime logs go through [xlog](https://github.com/kumose/xlog) / TLOG
- **Build system** — [kmcmake](https://github.com/kumose/kmcmake) + [kmpkg](https://github.com/kumose/kmpkg) integration

## Dependencies

Runtime dependencies are intentionally minimal:

| Library | Purpose |
|---------|---------|
| [xlog](https://github.com/kumose/xlog) | Unified logging (TLOG) |
| OpenSSL | TLS/SSL support |

Optional:

| Library | When needed |
|---------|-------------|
| liburing | Only when `XIO_ENABLE_URING=ON` (Linux disk async I/O) |

Build-only (not required to link against xio in your app):

| Library | When needed |
|---------|-------------|
| GoogleTest | `KMCMAKE_BUILD_TEST=ON` |
| benchmark | `KMCMAKE_BUILD_BENCHMARK=ON` |

Compression libraries (zlib, lz4, snappy, zstd) are not used by xio.

## Requirements

- Linux (Ubuntu 20.04+ / CentOS 7+ recommended; other Unix-like platforms may work)
- CMake >= 3.31
- GCC >= 9.4 or Clang >= 12
- C++14 or later (C++17 recommended for most examples; C++20 for coroutine examples)

## Quick Start

### With kmpkg (recommended)

Install [kmpkg](https://kumo-pub.github.io/docs/category/%E6%8C%81%E7%BB%AD%E9%9B%86%E6%88%90----kmpkg) first, then from the project root:

```bash
cmake --preset=default
cmake --build build -j$(nproc)
```

Dependencies are resolved automatically via [`kmpkg.json`](kmpkg.json). To pin or update the dependency baseline, edit `default-registry.baseline` in [`kmpkg-configuration.json`](kmpkg-configuration.json).

### Manual build

If you manage dependencies yourself, make sure CMake can find **xlog** and **OpenSSL** (the two runtime dependencies):

```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

## Build Options

Common kmcmake switches (set via `-D` on the cmake command line or in a preset):

| Option | Default | Description |
|--------|---------|-------------|
| `KMCMAKE_BUILD_TEST` | ON | Build unit tests under `tests/` |
| `KMCMAKE_BUILD_EXAMPLES` | ON | Build examples under `examples/` |
| `KMCMAKE_BUILD_BENCHMARK` | OFF | Build benchmarks under `benchmark/` |
| `XIO_ENABLE_URING` | OFF | Enable io_uring for disk async I/O (Linux only) |

Project-specific overrides can go in [`cmake/xio_user_option.cmake`](cmake/xio_user_option.cmake).

## Tests

```bash
ctest --test-dir build
```

Unit tests cover the xio core (sockets, timers, executors, SSL, etc.) and live under `tests/unit/`. Raft integration tests are under `tests/raft/`.

## Examples

Examples are built as separate targets when `KMCMAKE_BUILD_EXAMPLES=ON`. Each demo has its own `CMakeLists.txt`.

| Directory | Description |
|-----------|-------------|
| [`examples/cpp14/`](examples/cpp14/) | C++14 async I/O demos (echo, deferred, executors, …) |
| [`examples/cpp17/`](examples/cpp17/) | C++17 demos (HTTP servers, SSL, chat, serialization, …) |
| [`examples/raft/`](examples/raft/) | Raft quick start, calculator, echo server |

Build a single example, for example:

```bash
cmake --build build --target cpp17_echo_async_tcp_echo_server
cmake --build build --target quick_start
```

Source for C++20 coroutine demos is under `examples/cpp20/` (reference; not wired into the default example build).

## Raft Documentation

Raft usage guides are in [`docs/raft/`](docs/raft/):

- [Quick start guide](docs/raft/quick_start_guide.md)
- [How to use](docs/raft/how_to_use.md)
- [Basic operations](docs/raft/basic_operations.md)
- [Enabling SSL](docs/raft/enabling_ssl.md)
- [Replication flow](docs/raft/replication_flow.md)

## Using xio in Your Project

After installing or adding xio as a subproject:

```cmake
find_package(xio REQUIRED)
target_link_libraries(my_app PRIVATE xio::xio_static)
```

Include headers as:

```cpp
#include <xio/xio.h>           // core I/O
#include <xio/raft/launcher.h> // Raft (optional)
```

## Project Layout

```
xio/           Library implementation (networking + raft/)
examples/      Runnable demos (cpp14, cpp17, raft)
tests/         Unit and integration tests
docs/raft/     Raft documentation
cmake/         Project CMake modules and options
kmcmake/       Build-system helpers (kmcmake)
```

## License

Apache License 2.0. See [LICENSE](LICENSE) and per-file copyright headers.

xio incorporates code originally from Boost.Asio (Boost Software License 1.0) and NuRaft.
