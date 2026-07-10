# xio

[English](./README.md)

C++ 异步 I/O 与网络库，内置 Raft 共识支持。

xio 源自 Boost.Asio，公开 API 位于 `xio` 命名空间，提供熟悉的异步模型（`io_context`、completion token、协程、SSL、定时器等）。Raft 模块（`xio/raft/`）在 xio 传输层之上集成 NuRaft，用于复制状态机。

## 特性

- **异步网络** — TCP/UDP socket、acceptor、resolver、本地 socket、SSL
- **异步 I/O 工具** — 定时器、信号、文件、管道、串口（视平台而定）
- **现代 C++** — completion token、`co_await` / `co_spawn`（C++20）、线程池、strand
- **Raft 共识** — 选主、日志复制、快照、自定义状态机
- **统一日志** — Raft 与运行时日志走 [xlog](https://github.com/kumose/xlog) / TLOG
- **构建系统** — 集成 [kmcmake](https://github.com/kumose/kmcmake) 与 [kmpkg](https://github.com/kumose/kmpkg)

## 依赖

运行时依赖刻意保持最小：

| 库 | 用途 |
|----|------|
| [xlog](https://github.com/kumose/xlog) | 统一日志（TLOG） |
| OpenSSL | TLS/SSL 支持 |

仅构建时需要（链接 xio 库本身不依赖）：

| 库 | 何时需要 |
|----|----------|
| GoogleTest | `KMCMAKE_BUILD_TEST=ON` |
| benchmark | `KMCMAKE_BUILD_BENCHMARK=ON` |

xio 不使用压缩库（zlib、lz4、snappy、zstd 等）。

磁盘与 socket I/O 默认走传统异步模型（epoll / kqueue / IOCP）。**不需要 liburing**，一般保持 `XIO_ENABLE_URING=OFF` 即可。

## 环境要求

- Linux（推荐 Ubuntu 20.04+ / CentOS 7+；其他类 Unix 平台可能可用）
- CMake >= 3.31
- GCC >= 9.4 或 Clang >= 12
- C++14 及以上（多数示例建议 C++17；协程示例需 C++20）

## 快速开始

### 使用 kmpkg（推荐）

先安装 [kmpkg](https://kumo-pub.github.io/docs/category/%E6%8C%81%E7%BB%AD%E9%9B%86%E6%88%90----kmpkg)，在项目根目录执行：

```bash
cmake --preset=default
cmake --build build -j$(nproc)
```

依赖由 [`kmpkg.json`](kmpkg.json) 自动解析。如需固定或更新依赖基线，修改 [`kmpkg-configuration.json`](kmpkg-configuration.json) 中的 `default-registry.baseline`。

### 手动构建

自行管理依赖时，需确保 CMake 能找到 **xlog** 和 **OpenSSL**（两个运行时依赖）：

```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

## 构建选项

常用 kmcmake 开关（通过 cmake `-D` 或 preset 设置）：

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `KMCMAKE_BUILD_TEST` | ON | 构建 `tests/` 下的单元测试 |
| `KMCMAKE_BUILD_EXAMPLES` | ON | 构建 `examples/` 下的示例 |
| `KMCMAKE_BUILD_BENCHMARK` | OFF | 构建 `benchmark/` 下的基准测试 |
| `XIO_ENABLE_URING` | OFF | 可选 io_uring 后端（仅 Linux；保持关闭即可，传统 I/O 够用） |

项目级覆盖可写在 [`cmake/xio_user_option.cmake`](cmake/xio_user_option.cmake)。

## 测试

```bash
ctest --test-dir build
```

单元测试覆盖 xio 核心（socket、定时器、executor、SSL 等），位于 `tests/unit/`。Raft 集成测试位于 `tests/raft/`。

## 示例

`KMCMAKE_BUILD_EXAMPLES=ON` 时会构建示例。每个 demo 有独立的 `CMakeLists.txt`。

| 目录 | 说明 |
|------|------|
| [`examples/cpp14/`](examples/cpp14/) | C++14 异步 I/O 示例（echo、deferred、executors 等） |
| [`examples/cpp17/`](examples/cpp17/) | C++17 示例（HTTP 服务器、SSL、chat、序列化等） |
| [`examples/raft/`](examples/raft/) | Raft 快速入门、计算器、echo 服务 |

单独编译某个示例，例如：

```bash
cmake --build build --target cpp17_echo_async_tcp_echo_server
cmake --build build --target quick_start
```

C++20 协程示例源码在 `examples/cpp20/`（参考用，默认不参与 example 构建）。

## Raft 文档

使用说明见 [`docs/raft/`](docs/raft/)：

- [快速入门](docs/raft/quick_start_guide.md)
- [如何使用](docs/raft/how_to_use.md)
- [基本操作](docs/raft/basic_operations.md)
- [启用 SSL](docs/raft/enabling_ssl.md)
- [复制流程](docs/raft/replication_flow.md)

## 在项目中使用 xio

安装或将 xio 作为子项目引入后：

```cmake
find_package(xio REQUIRED)
target_link_libraries(my_app PRIVATE xio::xio_static)
```

头文件引用示例：

```cpp
#include <xio/xio.h>           // 核心 I/O
#include <xio/raft/launcher.h> // Raft（可选）
```

## 目录结构

```
xio/           库实现（网络 + raft/）
examples/      可运行示例（cpp14、cpp17、raft）
tests/         单元测试与集成测试
docs/raft/     Raft 文档
cmake/         项目 CMake 模块与选项
kmcmake/       构建系统辅助（kmcmake）
```

## 许可证

Apache License 2.0，详见 [LICENSE](LICENSE) 及各文件版权声明。

xio 包含源自 Boost.Asio（Boost Software License 1.0）与 NuRaft 的代码。
