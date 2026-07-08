# 搜索业务能力汇总 — 业务应用视角

> 从 QU → 召回 → 排序 → 算法模型 串联搜索全链路
> 基于 `/home/jeff/code/`, `/home/jeff/github/kumose/`, `/home/jeff/github/kumo-pub/cppdev/` 汇总

---

## 一、QU — 查询理解

### 1.1 分词

| 组件 | 位置 | 说明 |
|------|------|------|
| **SegNet** | `co3/SegNet` | 多粒度中文分词引擎 — 小/中/短语/全四种粒度，200万+ token/s，POS 标注，IDF，term weight，tightness，多语言(EN/JP/KR/VN/ID)，干预层，新词发现 |
| **jieba** | `c2/jieba_tools`, `co3/jieba_wrapper`, `github/kumose/jieba` | 结巴分词 C++ 版，基础分词能力 |
| **qqseg_new** | `co3/nlp_third_party_depend/qqseg_new` | QQ 分词器 |

### 1.2 实体识别 (NER) & 实体链接

| 组件 | 位置 | 说明 |
|------|------|------|
| **实体识别服务** | `c2/search_entity_recognition` | tRPC 服务，TF 模型驱动的 NER |
| **实体链接服务** | `c2/search_entity_linking` | tRPC 服务，查询 → 知识库实体匹配 |
| **NER 插件** | `co3/videosearch/ner` | 视频搜索 NER 组件 |

### 1.3 查询改写 & 纠错

| 组件 | 位置 | 说明 |
|------|------|------|
| **QRW (查询改写)** | `co3/qrw` | DAG 算子框架，同义/拼写/前缀改写 |
| **查询纠错** | `co3/query_correct` | FST + 缓存 + DAG 流水线，拼写纠错 |
| **rewrite_engine** | `c2/qu/rewrite_engine` | QU 平台内改写引擎 |

### 1.4 查询优化 (Query Optimizer)

| 组件 | 位置 | 说明 |
|------|------|------|
| **查询优化服务** | `co3/query_optimizer` | 分词 + term weight + intent + tightness DAG 流水线 |
| **意图分类 (旧版)** | `co3/sg_old_qo_local_intent` | 搜狗融合版 — term/hint/model/pattern/offline 多分类器，医疗实体 |
| **hot_event** | `c2/hot_event_qu_link_event` | 热点事件 + QU 联动 |

### 1.5 QU 平台

| 组件 | 位置 | 说明 |
|------|------|------|
| **QU 平台服务** | `c1/qu_platform_trpc` | tRPC 服务，Processor DAG 流水线，CallProcessor/CallSearchProcessor API |
| **QU 完整服务** | `c2/qu` | 包含: qu_platform_trpc / qrw / rewrite_engine / sug_leaf / sug_proxy / sug_root / sugas |
| **QU API 定义** | `c2/qu_api` | 算子接口: intent / ner / normalize / rewrite / search / segment / term |
| **QU 算子 (c1)** | `c1/qu_platform_trpc/proc` | 处理器工厂 + DAG 节点实现 |
| **QU 算子 (co3 视频)** | `co3/videosearch/vsquproxy` | 视频搜索 QU 代理 |

---

## 二、召回

### 2.1 搜索引擎核心

| 组件 | 位置 | 说明 |
|------|------|------|
| **搜索入口代理** | `c2/search_engine_proxy_trpc` | 查询策略构建，平台功能装配 |
| **搜索引擎 API** | `c2/search_engine_api` | 外部 RPC API (Proto) |
| **搜索引擎协议** | `c2/search_engine_proto` | document.proto(正排/倒排/filter) / index_config(索引库schema) / recall_forward(召回正排) / index_meta(元数据) / kafka_msg |
| **Search Kit** | `co3/search_engine_skit` | 检索核心库 — query/matcher/forward/collector/storage/bloom_filter |
| **XSearch 叶子节点** | `c2/xsearch_leaf` | 索引管理 + 检索 + 粗排，scorer 插件化，COS 增量同步 |
| **搜索引擎** | `tcode/归档/search_engine/` | 归档版引擎代码 |

### 2.2 插件系统

| 组件 | 位置 | 说明 |
|------|------|------|
| **插件协议** | `c2/search_plugin_proto` | C++ / Lua / RPC 三种插件类型 |
| **Scorer 插件** | `c2/xsearch_leaf/plugin/scorer` | demo_scorer / vs_rough_sort_scorer (视频) |
| **plugblue** | `co3/plugblue` | 插件库: kv_trie / ronda_client / segnet / lightgbm / redis / onnx / xgboost / serving |

### 2.3 向量召回

| 组件 | 位置 | 说明 |
|------|------|------|
| **ElasticFaiss** | `co2/elasticfaiss` | 分布式 FAISS 向量检索服务，容器化，插件配置，监控 |
| **XSearch ANN** | `c2/xsearch_ann` | 向量索引聚合 — FAISS + HNSW 索引管理 |
| **增量 Embedding 引擎** | `co3/incr_embedding_engine` | 实时 Embedding 更新检索 — Bloom 过滤 + 多版本索引 + Kafka 流 |
| **FAISS** | `github/kumose/faiss` + `c2/xsearch_third_party` | Meta FAISS CPU + GPU 向量索引 |
| **xann** | `github/kumose/xann` | ANN 搜索 (xsimd) |
| **xdistance** | `github/kumose/xdistance` | 距离/相似度计算 |
| **goose-vss** | `github/kumose/goose-vss` | SQL 内向量搜索集成 (usearch + duckdb) |
| **ksearch** | `github/kumose/ksearch` | 分布式搜索引擎（向量 + SQL + 全文） |

### 2.4 词典/索引召回

| 组件 | 位置 | 说明 |
|------|------|------|
| **Darts** | `github/kumose/darts` | 静态双数组 Trie (DAWG) — 词典匹配 |
| **Cedar** | `github/kumose/cedar` | 动态双数组 Trie |
| **Roaring Bitmap** | `c2/xsearch_third_party/roaring` | 压缩位图，SIMD 加速 |
| **倒排索引** | `co3/search_engine_skit/matcher` | 倒排匹配 |
| **Trie 插件** | `co3/plugblue/kv_trie` | KV Trie 检索插件 |

### 2.5 联想词召回 (Suggestion)

| 组件 | 位置 | 说明 |
|------|------|------|
| **SUG 代理** | `co1/sug_proxy` | 入口，查询归一化/过滤/插件管理 |
| **SUG 根节点** | `co1/sug_root` | 聚合叶子结果，合并排序 |
| **SUG 叶子** | `co1/sug_leaf` | 索引分片查询，recall/sort/cache，Cedar Trie |
| **SUG 离线数据处理** | `co1/sug_data_process` | DAG 调度多数据源处理 |
| **SUG 离线预处理** | `co1/sug_offline` | 联想词离线数据生成 |
| **SUG 监控** | `co1/sug_monitor` | 白屏率监控 |
| **SUG API** | `co1/sug_api` | Proto 接口定义 |

### 2.6 知识/实体召回

| 组件 | 位置 | 说明 |
|------|------|------|
| **知识图谱** | `c2/search/knowledge` | 搜索知识图谱 |
| **OneBox** | `c2/one_box` + `tcode/归档/one_box/` | 富媒体结果协议 (天气/股票等) |

---

## 三、排序

### 3.1 粗排 (Coarse Ranking)

| 组件 | 位置 | 说明 |
|------|------|------|
| **粗排服务** | `c1/rel_server` / `co1/rel_server` | XGBoost / Bimpm / LTR 多模型，QT Pair 相关性打分 |
| **粗排平台** | `c2/basic_rank_platform` / `co3/basic_rank_platform` | 多目标加权粗排，按 app_id/module_id 路由 |
| **粗排协议** | `c2/rel_proto` | RelScoreRequest/Reply Proto |
| **IR 基础权重** | `c2/irbw_component` | BM25-like: CQR/CDR/SOR/Offset 细粒度特征 |
| **相关性组件库** | `c2/rele_component` | 相关性模型 + 源码 |
| **相关性协议** | `c2/rel_proto` | Proto 定义 |
| **Scorer 插件** | `c2/xsearch_leaf/plugin/scorer/vs_rough_sort_scorer` | 视频粗排 scorer |

### 3.2 精排 (Fine Ranking)

| 组件 | 位置 | 说明 |
|------|------|------|
| **视频精排** | `co3/videosearch/preciserankserver` | 视频精排服务 |
| **视频 mixranking** | `co3/videosearch/mixrankingsvr` | 视频混排服务 |
| **basicsort** | `co3/videosearch/basicsort` | 基础排序 |
| **rankaccess** | `co3/videosearch/rankaccess` | 排序接入层 |

### 3.3 LTR (Learning to Rank)

| 组件 | 位置 | 说明 |
|------|------|------|
| **LTR 模型** | `c2/ltr_model` | 模型训练 + 预测 |
| **XGBoost** | `c1/rel_server/model/XgbModel.cc` | XGBoost 粗排 |
| **LightGBM** | `co3/plugblue/lightgbm` | LightGBM 推理插件 |

---

## 四、算法模型

### 4.1 传统 ML 模型

| 模型 | 位置 | 使用场景 |
|------|------|---------|
| **XGBoost** | `c1/rel_server/model/` (48+ 模型) | 粗排相关性 |
| **LightGBM** | `co3/plugblue/lightgbm` | 排序/分类插件 |
| **BM25 / IR 特征** | `c2/irbw_component` | 基础相关性评分 |
| **fastText** | `github/kumose/fasttext` | 文本分类 / 表示学习 |

### 4.2 深度学习模型

| 模型 | 位置 | 使用场景 |
|------|------|---------|
| **BERT** | `c2/bert_tools` | BERT 工具链，视频相似度 (`tcode/归档/goneat_video_sim_title_bert`) |
| **Bimpm** | `c1/rel_server/model/` | 文本匹配粗排 |
| **SegNet (TF)** | `co3/SegNet/source/Tf/` | TensorFlow 版分词 |
| **TF Server** | `co3/videosearch/tfserver` | TF 模型推理服务 |
| **Torch Server** | `co3/videosearch/torchserver` | LibTorch 模型推理服务 |
| **ONNX** | `co3/plugblue/onnx` + `github/kumose/onnxruntime` | ONNX 模型推理 |

### 4.3 向量 & Embedding 模型

| 模型 | 位置 | 使用场景 |
|------|------|---------|
| **FAISS** | `co2/elasticfaiss`, `c2/xsearch_ann`, `github/kumose/faiss` | ANN 向量检索 |
| **HNSW** | `c2/xsearch_ann/hnsw_lib` | 图结构 ANN |
| **ScaNN** | `tcode/归档/scann/` | Google 向量检索 |
| **Embedding 引擎** | `co3/incr_embedding_engine` | 实时增量 Embedding |
| **goose-vss** | `github/kumose/goose-vss` | SQL 向量搜索 |
| **xann** | `github/kumose/xann` | xsimd 加速 ANN |

### 4.4 NLP 模型

| 模型 | 位置 | 使用场景 |
|------|------|---------|
| **SegNet** | `co3/SegNet` | 中文分词 + POS + IDF + weight |
| **jieba** | `c2/jieba_tools` + `github/kumose/jieba` | 中文分词 |
| **SentencePiece** | `github/kumose/xtokenizer` | 子词分词 (跨平台绑定 HF tokenizers) |
| **fastText** | `github/kumose/fasttext` | 文本分类 |
| **Hadar** | `github/kumose/hadar` | 文本分段 + 查找 |
| **Snowball** | `github/kumose/snowball` | 多语言词干提取 |
| **xpinyin** | `github/kumose/xpinyin` | 拼音转换 |
| **xre2** | `github/kumose/xre2` | RE2 正则 |
| **PCRE** | (文档) | PCRE 正则 |

### 4.5 ML 推理框架

| 框架 | 位置 | 说明 |
|------|------|------|
| **xinference** | `github/kumose/xinference` | 统一推理接口: MNN / ncnn / OpenVINO |
| **TorchServer** | `co3/videosearch/torchserver` | LibTorch 在线推理 |
| **TF Server** | `co3/videosearch/tfserver` | TensorFlow 在线推理 |
| **RondaServing** | `c2/Interface/rondaserving_interface` | 无量预测服务接口 (稀疏/稠密) |
| **xgboost 插件** | `co3/plugblue/xgboost` | 在线 XGBoost 推理 |

### 4.6 算法基础设施

| 组件 | 位置 | 说明 |
|------|------|------|
| **ispine 框架** | `co3/ispine` | DAG + DI + 插件 + 字典热更新 + 自动监控，算法与工程协作框架 |
| **plugblue** | `co3/plugblue` | 插件管理: kv_trie / ronda / segnet / lightgbm / redis / onnx / xgboost / serving |
| **tally** | `github/kumose/tally` | Prometheus 指标 (Counter/Gauge/Histogram) — 监控模型效果 |
| **xconfig** | `github/kumose/xconfig` | 热更新配置，算法参数动态调整 |
| **turbo/flags** | `github/kumose/turbo` | 命令行/运行时 Flag 控制 |

---

## 五、全链路数据流

```
用户查询
  │
  ▼
┌─────────────────────────────────────────────────────┐
│  搜索入口 (search_engine_proxy_trpc)                │
│  → 查询策略构造                                     │
└─────────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────────┐
│  QU — 查询理解                                      │
│  ├─ 分词 (SegNet / jieba)                           │
│  ├─ 实体识别 + 实体链接 (NER / Entity Linking)      │
│  ├─ 查询改写 (QRW) + 拼写纠错 (query_correct)       │
│  └─ 查询优化 (query_optimizer: weight+intent+tight)  │
└─────────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────────┐
│  召回                                               │
│  ├─ 倒排索引 (search_engine_skit / xsearch_leaf)   │
│  ├─ 词典匹配 (Trie: darts / cedar)                  │
│  ├─ 向量检索 (FAISS / HNSW / ElasticFaiss)          │
│  ├─ 联想词 (sug: proxy → root → leaf)              │
│  └─ 知识图谱 / OneBox                               │
└─────────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────────┐
│  粗排                                               │
│  ├─ IR 基础权重 (BM25 + CQR/CDR/SOR)               │
│  ├─ XGBoost / LightGBM / Bimpm 模型                │
│  └─ 多目标加权 (basic_rank_platform)                │
└─────────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────────┐
│  精排                                               │
│  ├─ 视频精排 / 混排                                │
│  ├─ LTR 模型                                       │
│  ├─ TF / Torch / ONNX 在线推理                     │
│  └─ RondaServing 预测                              │
└─────────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────────┐
│  输出                                               │
│  ├─ 联想词 (sug_proxy)                             │
│  ├─ OneBox 富媒体                                   │
│  └─ 搜索结果                                       │
└─────────────────────────────────────────────────────┘

横切关注:
  ├─ A/B 测试: xsearch_diff_filter / tab-cpp-sdk / online_diff_monitor
  ├─ 监控指标: tally (Prometheus) / heaton (日志) / attaapi (数据上报)
  └─ 配置热更: xconfig (TOML/JSON/PB 互转)
```

---

## 六、关键数据流通道

| 通道 | 技术栈 | 位置 |
|------|--------|------|
| RPC 通信 | tRPC / brpc / krpc / qrpc | 全业务 |
| 实时流 | Kafka | `co3/incr_embedding_engine/consumer`, `c2/search_engine_proto/kafka_msg` |
| 存储持久化 | RocksDB / LevelDB | `github/kumose/granite`, `c2/braft` |
| 分布式协同 | braft / xraft | `c2/braft`, `github/kumose/xraft` |
| 配置下发 | MDB / Wuji | `co3/plugblue/wuji_client` |
| 服务发现 | Polaris | `tcode/归档/polaris-cpp/` |
| 数据上报 | Atta / PaaS | `co3/attaapi_cplus`, `c2/paas-trpc-sdk` |
