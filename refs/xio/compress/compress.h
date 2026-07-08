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

#include <xio/compress/fwd.h>

#include <fermat/container/string.h>
#include <turbo/utility/status.h>

#include <cstddef>

namespace xio::compress {

    enum class CompressAlgo {
        kCompressNone,
        kCompressGzip,
        kCompressSnappy,
        kCompressLz4,
        kCompressZstd,
    };

    struct CompressOptions {
        int level{0};
    };

    struct DecompressOptions {
        size_t max_plain{0};
    };

    struct Output {
        xio::Buffer *buf{nullptr};
        size_t bytes_written{0};
    };

    struct Compressor;

    using CompressHandler = turbo::Status (*)(const Compressor &c, CompressOptions opts, const xio::Buffer &in,
                                              Output *out);

    using DecompressHandler = turbo::Status (*)(const Compressor &c, DecompressOptions opts, const xio::Buffer &in,
                                                Output *out);

    struct Compressor {
        CompressAlgo algo{CompressAlgo::kCompressNone};
        CompressHandler compress_handler{nullptr};
        DecompressHandler decompress_handler{nullptr};

        static Compressor make_gzip();
        static Compressor make_snappy();
        static Compressor make_lz4();
        static Compressor make_zstd();
        static Compressor from_name(const fermat::KString &name);

        turbo::Status compress(CompressOptions opts, const void *data, size_t size, Output *out) const;

        turbo::Status compress(CompressOptions opts, const xio::Buffer &in, Output *out) const;

        turbo::Status decompress(DecompressOptions opts, const void *data, size_t size, Output *out) const;

        turbo::Status decompress(DecompressOptions opts, const xio::Buffer &in, Output *out) const;

        const char *name() const;

        size_t compress_bound(size_t plain_len) const;

        size_t max_plain_size() const;

        bool decompress_needs_max_plain() const;
    };

    inline constexpr Compressor kNoneCompressor{};

} // namespace xio::compress
