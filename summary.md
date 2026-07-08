# 项目能力总览

> 汇总范围: `/home/jeff/code/`, `/home/jeff/github/kumose/`, `/home/jeff/github/kumo-pub/cppdev/docs/`
> 汇总时间: 2026-07-03

---

## 一、xio — 当前项目

**位置**: `/home/jeff/github/kumose/xio/`
**构建**: kmcmake + kmpkg 包管理
**状态**: 迁移完成（从 kthread + qrpc），测试通过

### 模块

| 模块 | 功能 |
|------|------|
| `xio/event/` | 跨平台事件循环: epoll(Linux) / kqueue(macOS) / iocp(Windows) / uring(Linux, 选项开关), Waker, EventData, ThreadId |
| `xio/io/` | TcpSocket, Listener, Connector, PairWare, SocketWare |
| `xio/timer/` | WheelTimer, BtreeTimer |
| `xio/compress/` | 压缩统一接口: gzip / snappy / lz4 / zstd 后端 |

### 关键决策

- liburing 通过 `cmake/xio_user_option.cmake` 的 `XIO_ENABLE_URING` 选项控制（默认 OFF）
- 所有模块共用单一 kmcmake_cc_library 目标（因 event 与 timer 有循环依赖）
- 文件 IO (fio) 暂缓实现，等明确指示

### 依赖
- **核心**: turbo (基础库), fermat (高性能容器), tally (指标)
- **压缩**: zlib, lz4, snappy, zstd
- **可选**: liburing (Linux, 默认关)
- **测试**: gtest, benchmark

---

## 二、/home/jeff/github/kumose/ — 基础设施与核心库

### 2.1 基础库层

| 项目 | 说明 |
|------|------|
| **turbo** | C++ 基础库 (Algo/Container/Crypto/Debug/File/Flag/Functional/Hash/Log/Memory/Num/Profiling/Rand/String/Sync/Thread/Time/Utility) — 零外部依赖 |
| **fermat** | 高性能 C++ 容器库（KString/Buffer/Vector/Deque/Cord/Bitset/LruCache/ObjectPool/PriorityQueue/RadixSort/IO），对标 std 2-10x 加速 |
| **melon** | Facebook folly 生态移植: fibers/futures/executors/small_vector/ProducerConsumerQueue/JSON/IO/Crypto/SSL |
| **taco** | 多线程并发工具 (高并发容器/工具) |
| **typesafe** | 零开销强类型包装，用类型系统预防 bug |
| **vamos** | Unicode 验证与转码 (UTF-8/UTF-16), 数十亿字符/秒 |
| **xsort** | 排序算法: vergesort / ska_sort / pdqsort |
| **xdistance** | 头文件仅距离/相似度计算库 |

### 2.2 I/O 与网络层

| 项目 | 说明 |
|------|------|
| **xio** | 跨平台异步 IO: 事件循环/ TCP Sockets/ 压缩/ 定时器 (本仓库) |
| **kthread** | I/O 调度 + M:N Worker 协程栈，~1M HTTP QPS |
| **phttp** | C++ HTTP/1.x 请求/响应解析器 |
| **chimera** | 多协议编解码库: HTTP/1.1, HTTP/2, WebSocket, Redis RESP2/3, Kafka(16 ApiKeys), MySQL Classic, Memcached, gRPC |
| **qrpc** | RPC 框架（基于 chimera + fermat + protobuf + tally），支持加密(mbedtls/openssl)和压缩 |
| **h2o** | H2O HTTP 服务器（C 语言） |

### 2.3 RPC 框架（生态）

| 项目 | 说明 |
|------|------|
| **gRPC** | 多语言 gRPC (文档) |
| **brpc** | Baidu RPC，高吞吐，Raft 兼容 (文档) |
| **krpc** | Kumo 增强版 brpc (文档，Kumo 首选的内部后端 RPC) |
| **httplib** | 头文件仅轻量 HTTP (文档) |
| **acl** | 高质量 C++ 后端服务库 (文档) |
| **Thrift** | Apache Thrift RPC (文档) |
| **Seastar** | Seastar 框架 (文档) |

### 2.4 存储层

| 项目 | 说明 |
|------|------|
| **granite** | 存储引擎内核（基于 RocksDB，兼容 Redis 数据结构），多层存储: 内存/RocksDB/RPC/冷热分离 |
| **potti** | KV 存储服务 (RocksDB + brpc + braft + hnswlib) |
| **kns** | 分布式名称/KV 服务 (braft + brpc + granite + shark) |
| **xhdfs** | C++ HDFS 客户端 (libhdfs3) |
| **braft** | Baidu Raft 共识算法实现（工业级 C++） |
| **xraft** | Raft 共识库（基于 braft + brpc + rocksdb + granite） |

### 2.5 SQL / 查询层

| 项目 | 说明 |
|------|------|
| **ksql** | SQL 解析器 + Binder + Planner (bison 59 个语法分片，MySQL 方言 + Kumo 扩展) → AST → 语义绑定 → 逻辑计划 (protobuf) |
| **goose** | 分析型进程内 SQL IR 数据库（基于 DuckDB），完整 SQL 数据库管理系统 |

### 2.6 搜索 / 检索层

| 项目 | 说明 |
|------|------|
| **ksearch** | 分布式搜索引擎 — 整合 SQL + 向量 + 全文，基于 Arrow/Faiss/RocksDB/brpc/braft |
| **faiss** | Meta FAISS 向量索引（CPU + GPU） |
| **xann** | 近似最近邻搜索 (xsimd 加速) |
| **goose-vss** | Goose 向量相似搜索扩展 |
| **darts** | 静态双数组 Trie (DAWG) |
| **cedar** | 动态双数组 Trie |

### 2.7 NLP / 文本处理层

| 项目 | 说明 |
|------|------|
| **hadar** | 文本处理库（分词+查找，依赖 darts/marisa-trie/rapidjson） |
| **jieba** | C++ 结巴中文分词 |
| **xpinyin** | 中文拼音转换库 |
| **snowball** | 词干提取 (libstemmer) |
| **gnef** | SQL NLP IR — "表达式即开发"，基于 fasttext/goose/hadar/jieba/merak/taco/xpinyin |
| **xre2** | RE2 正则封装 |
| **xtokenizer** | 跨平台 Tokenizer 绑定（HuggingFace + sentencepiece） |
| **xicu** | ICU 66 国际化库 |
| **xfsst** | FSST 短字符串压缩 |

### 2.8 ML / 推理层

| 项目 | 说明 |
|------|------|
| **xinference** | 多后端 ML 推理 (MNN / ncnn / OpenVINO) |
| **fasttext** | fastText 文本分类和表示学习 |
| **xann** | ANN 搜索 (xsimd) |
| **goose-vss** | Goose 向量搜索扩展 |

### 2.9 序列化与配置

| 项目 | 说明 |
|------|------|
| **merak** | Protobuf ↔ JSON ↔ FlatKV 互转，全套 DOM/SAX JSON + JSON Pointer/Schema |
| **xconfig** | 无锁热更新配置框架 — TOML/JSON/Protobuf 双向转换，双缓冲读写分离，版本管理回滚 |
| **xtoml** | TOML 解析器 (基于 toml11) |
| **shark** | Proto 驱动代码生成 + 列存适配 |
| **pollux** | Proto 数据结构 (row/type/value/vector) |
| **nlpproto** | NLP 领域 Protobuf 定义 |

### 2.10 可观测性

| 项目 | 说明 |
|------|------|
| **tally** | Prometheus 指标库 — Counter/Gauge/Histogram/Window/Percentile，线程本地写入，读取时聚合，零竞争。对比 bvar 1.6-10x 快 |
| **heaton** | 日志管理统一 — GLog/Abseil/Turbo 日志统一收集调度，零拷贝分发，自愈轮转 |
| **glog** | Google 日志库 (C++14) |

### 2.11 工具与构建

| 项目 | 说明 |
|------|------|
| **kmpkg** | Kumo 包管理器 |
| **kmcmake** | Kumo CMake 构建系统模板 |
| **kmdo** | Go 构建工具链 |
| **kmgen** | 代码生成工具 |
| **xxd** | 静态资源嵌入式 C++ 代码生成 (CMake 工具, `std::string_view` 零拷贝) |
| **tclap** | 命令行参数解析 |
| **qtest** | 基于 gtest + turbo 扩展的测试框架 |
| **cppast** | C++ AST 库 |
| **cantor** | PostgreSQL 查询解析器封装 |
| **xthrift** | Apache Thrift RPC 框架轻量版 |

---

## 三、/home/jeff/github/kumo-pub/cppdev/ — 开发者文档体系

**位置**: `/home/jeff/github/kumo-pub/cppdev/docs/docs/`
**框架**: Docusaurus v3

### 文档目录结构

| 章节 | 内容 |
|------|------|
| **foundamentals/** | 14 个基础专题 — testing/ log/ time/ status/ strings/ json/ map/ codec/ compress/ fibers/ filesystem/ flags/ format/ monitor |
| **advance/** | 高级专题 — gpu(CUDA/cuDNN) / ml(PyTorch/TF/ONNX/TVM) / simd / tensor / raft / compute(Arrow/Velox/Pollux) |
| **rpc/** | 7 种 RPC 框架场景化对比: gRPC / brpc / krpc / httplib / acl / Thrift / Seastar |
| **store/** | 存储 — kv(LevelDB/LMDB/RocksDB) / format(Parquet/Arrow/Avro/HDF5) / cloud(S3/Azure/GCS/HDFS) |
| **retrieve/** | 检索 — trie(cedar/darts/marisa) / bitmap(Roaring) / post / regex(PCRE/RE2) / nlp(hadar/jieba/sentencepiece/stem) / vector(FAISS/HNSW/DiskANN/NGT/ScaNN/SPTAG) |
| **ir/** | IR — pegtl / bison-flex / antlr / ast / llvm |
| **integration/** | 30+ 第三方库集成指南，从 kmpkg.json 到 CMake 配置一站搞定 |
| **lives/** | 在线演示 |

### 特点
- 每个专题都有**多库场景对比和选型指南**（如 7 种 RPC 对比、6 种 JSON 对比、4 种日志对比）
- 所有库通过 kmpkg 包管理，文档与代码集成一致

---

## 四、/home/jeff/code/ — 搜索业务项目

### 按业务分类

#### 4.1 查询理解 (QU) — Query Understanding

| 项目 | 说明 |
|------|------|
| **c1/qu_platform_trpc** | QU 平台 — tRPC 服务，Processor DAG 流水线，CallProcessor/CallSearchProcessor API |
| **c2/qu** | QU 完整服务 — 包含子服务: qu_platform_trpc, qrw(改写), rewrite_engine, sug_data_process, sug_leaf, sug_proxy, sug_root, sugas |
| **c2/qu_api** | QU 接口定义 — intent/ner/normalize/rewrite/search/segment/term 各类算子 |

#### 4.2 查询改写与纠错

| 项目 | 说明 |
|------|------|
| **co3/qrw** | Query Rewrite — DAG 算子框架 |
| **co3/query_correct** | 拼写纠错 — FST + 缓存 + DAG 流水线 |
| **co3/query_optimizer** | 查询优化 — 分词 + 权重 + intent + tightness |

#### 4.3 相关性排序 (Coarse Ranking)

| 项目 | 说明 |
|------|------|
| **c1/rel_server** | 粗排服务 — XGBoost/Bimpm/LTR 多模型，QT Pair 相关性打分 |
| **c2/basic_rank_platform** | 基础排序平台 — 多目标加权粗排，按 app_id/module_id 路由 |
| **c2/irbw_component** | IR 基础权重 — BM25-like 细粒度特征 (CQR/CDR/SOR/Offset) |
| **c2/rele_component** | 相关性组件库 — 含模型文件 |
| **c2/rel_proto** | 相关性协议 (RelScoreRequest/Reply) |
| **co3/basic_rank_platform** | 同 c2 版 |

#### 4.4 搜索引擎核心

| 项目 | 说明 |
|------|------|
| **c2/search_engine_proxy_trpc** | 搜索引擎代理 — 搜索入口，用户查询策略构建 |
| **c2/search_engine_api** | 搜索引擎外部 API (Proto RPC 定义) |
| **c2/search_engine_proto** | 引擎协议 — document/inverted index/filter/kafka/index_config/recall_forward/index_meta|
| **c2/xsearch_leaf** | XSearch 叶子节点 — 索引管理 + 检索 + 粗排，支持 scorer 插件 |
| **c2/search_plugin_proto** | 插件协议 — C++ / Lua / RPC 三种插件类型 |
| **co3/search_engine_skit** | 搜索引擎 Search Kit — query/matcher/forward/collector/storage/bloom_filter |

#### 4.5 搜索工具与公共库

| 项目 | 说明 |
|------|------|
| **c2/tsvt_common_util** | 15 个头文件: base85/bloom_filter/dedup/diff/format/jce/report/string/task_executor/time_logger/xlog/yaml |
| **c2/xsearch_util** | arena/container/string/system/time/tokenizer/rapidjson/mock |
| **c2/debug_trace_lib** | 调试日志打印 (TAF + tRPC) |
| **c2/debugsdk** | tRPC 插件级调试信息收集 SDK |
| **c2/xsearch_third_party** | 23 个三方库: faiss/cppjieba/roaring/librdkafka/snappy/exprtk/pugixml/jsoncons/cedar/darts/luajit/cos_sdk/hbase_api/taskflow |

#### 4.6 A/B 测试与对比

| 项目 | 说明 |
|------|------|
| **c2/xsearch_diff_filter** | Diff 过滤插件 — 客户端/服务端双端 diff |
| **c2/online_diff_monitor_platform_proto** | 在线 diff 监控平台协议 |
| **co3/tab-cpp-sdk** | 腾讯 Tab A/B 测试 + 特征标记 C++ SDK |
| **co3/attaapi_cplus** | 腾讯 Atta 数据上报 C++ SDK |

#### 4.7 实体识别与知识

| 项目 | 说明 |
|------|------|
| **c2/search_entity_linking** | 实体链接 — 查询 → 知识库实体 |
| **c2/search_entity_recognition** | 实体识别 (NER) — TF 模型 |
| **c2/search/knowledge** | 搜索知识图谱 |

#### 4.8 联想词 (Suggestion)

| 项目 | 说明 |
|------|------|
| **co1/sug_proxy** | SUG 代理 — 入口，查询归一化/过滤 |
| **co1/sug_root** | SUG 根节点 — 聚合叶子结果，合并排序 |
| **co1/sug_leaf** | SUG 叶子 — 索引分片查询，recall/sort/cache |
| **co1/sug_data_process** | SUG 离线数据处理 — DAG 调度，多数据源 |
| **co1/sug_offline** | SUG 离线预处理 |
| **co1/sug_monitor** | SUG 白屏监控 |
| **co1/sug_api** | SUG Proto API 定义 |

#### 4.9 向量搜索与 Embedding

| 项目 | 说明 |
|------|------|
| **co2/elasticfaiss** | 分布式向量检索服务 — 基于 FAISS，容器化，插件配置 |
| **c2/xsearch_ann** | 向量索引聚合 — FAISS + HNSW |
| **co3/incr_embedding_engine** | 增量 Embedding 引擎 — 实时索引更新，Bloom 过滤，Kafka 流，多版本索引 |

#### 4.10 NLP / 分词

| 项目 | 说明 |
|------|------|
| **co3/SegNet** | 多粒度中文分词引擎 — 小/中/短语/全四种模式，200万+ token/s，POS/IDF/权重/tightness，多语言，新词发现 |
| **co3/jieba_wrapper** | 结巴分词封装 (cppjieba + 词典) |
| **co3/nlp_third_party_depend** | NLP 三方依赖: base/da_trie/qqseg_new |
| **co3/sg_old_qo_local_intent** | 搜狗旧版查询意图分类 — 多种分类器 (term/hint/model/pattern/offline) + 医疗实体 |

#### 4.11 视频搜索

| 项目 | 说明 |
|------|------|
| **co3/videosearch** | 视频搜索平台 — 40 个微服务: basicsort/clicksim/embedding/ner/mixranking/queryexpand/termweight/tf/torch/sugrank |
| **co3/vscore** | 视频搜索 C++ 公共库（可依赖三方库）: trpc_ext/cache/bloom_filter |
| **co3/vssak** | 视频搜索 C++ 公共库（零三方依赖）: async/codec/concurrency/cache/string |
| **c2/videosearch** | 视频搜索组件 |

#### 4.12 搜狗融合

| 项目 | 说明 |
|------|------|
| **co3/sg_third_party** | 搜狗三方依赖: dicmap/shared_hash/ssplatform_encoding/web_base/web_segmentor/word_segmentor |
| **co3/sg_old_qo_local_intent** | 搜狗旧意图分类 — 含预编译 lib64_debug/lib64_release |

#### 4.13 服务框架与基础设施

| 项目 | 说明 |
|------|------|
| **co3/ispine** | 在线服务开发框架 — DAG + DI + 插件 + 自动监控 + 字典热更新 |
| **co3/frameblue** | 轻量应用框架 — dict/plugin/context/app 管理 |
| **co3/kcfg** | C++ JSON 配置 → struct 映射（头文件仅，编译器反射宏） |
| **co3/plugblue** | 插件管理库 — kv_trie/ronda_client/segnet/lightgbm/redis/onnx/xgboost/serving 插件 |
| **c2/braft** | Baidu Raft 共识（工业级 C++） |
| **tcode/dag_np** | Go DAG 节点处理器框架 |

#### 4.14 存档与杂项

| 项目 | 说明 |
|------|------|
| **tcode** | 归档代码快照: ElasticFaiss/FAISS/HNSW/ScaNN/trpc-dag/kafka/flare/union_kv/one_box/qu/rel_server/search_engine/NLP 框架/RTC |
| **tcode/guidang** | 个人文档: 竞品分析/QU 平台文档/Union 重构/SUMERU/TRPC Fiber/绩效等 |
| **c2/Pigeons** | C++ → Python 桥接 |
| **c2/mvp** | MVP 项目（Go 模板/实验） |

### 业务依赖关系

```
搜索入口 (search_engine_proxy_trpc)
  → 查询理解 (qu_platform_trpc)
    → 分词 (SegNet / jieba)
    → 实体识别 (search_entity_recognition/linking)
    → 改写 (qrw / query_correct)
    → 意图分类 (sg_old_qo_local_intent / query_optimizer)
  → 检索 (xsearch_leaf / search_engine_skit)
    → 向量搜索 (elasticfaiss / xsearch_ann / incr_embedding_engine)
    → Trie 索引 (cedar/darts)
  → 粗排 (rel_server / basic_rank_platform / irbw_component)
    → 模型 (XGBoost / LightGBM / BERT / ONNX / LibTorch)
  → 联想词 (sug_proxy → sug_root → sug_leaf)
  → A/B 测试 (xsearch_diff_filter / tab-cpp-sdk)
  → 数据上报 (attaapi_cplus / paas-trpc-sdk)
```

---

## 五、技术栈全景

```
语言: C++17 (主力) / Go (工具/部分服务) / Python (ML 训练)
构建: Bazel (搜索业务) / cmake + kmcmake (基础设施库) / kmpkg (包管理)
协议: tRPC / trpc-cpp / Protobuf / Thrift / HTTP(S)/2 / WebSocket / Redis / Kafka / MySQL
容器: turbo (0外链) / fermat (高性能) / melon (folly 生态)
搜索: FAISS / HNSW / BM25 / Trie (cedar/darts) / Roaring Bitmap
SQL: ksql (解析→规划) → goose (执行引擎)
存储: RocksDB / LevelDB / LMDB / HDFS / COS
ML: XGBoost / LightGBM / ONNX / LibTorch / MNN / ncnn / OpenVINO
AI: BERT / fastText / SegNet / SentencePiece / HuggingFace Tokenizers
监控: tally (Prometheus) / heaton (日志) / diff monitor (A/B)
RPC: brpc / krpc / gRPC / tRPC / qrpc
```

---

## 六、文档体系

| 来源 | 内容 |
|------|------|
| **kumo-pub/cppdev/docs/** | 开发者文档站（Docusaurus）— 14 个基础专题 + 高级/存储/RPC/检索/IR/30+ 集成指南 |
| **各项目 README.md** | 中英文双语 README（turbo/fermat/tally/xconfig/shark 等有详细 README） |
| **各项目 docs/** | 额外文档（turbo/melon/ksearch/ksql/chimera/pollux 等） |
