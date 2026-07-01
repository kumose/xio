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

#include <xio/compress/detail/backends.h>
#include <xio/compress/detail/output_tail.h>

#include <lz4frame.h>

#include <array>
#include <string>

namespace xio::compress::detail {
namespace {

int lz4_level(CompressOptions opts) {
    return opts.level == 0 ? 1 : opts.level;
}

LZ4F_preferences_t lz4_prefs(CompressOptions opts) {
    LZ4F_preferences_t prefs{};
    prefs.compressionLevel = lz4_level(opts);
    prefs.frameInfo.blockMode = LZ4F_blockIndependent;
    return prefs;
}

} // namespace

size_t lz4_compress_bound(size_t plain_len) {
    return LZ4F_compressFrameBound(plain_len, nullptr);
}

turbo::Status lz4_compress_handler(const Compressor &, CompressOptions opts, const xio::Buffer &in, Output *out) {
    OutputTail tail(out);

    const std::string plain = in.flatten<std::string>();
    const LZ4F_preferences_t prefs = lz4_prefs(opts);
    const size_t bound = LZ4F_compressFrameBound(plain.size(), &prefs);

    std::string compressed;
    compressed.assign(bound, '\0');
    const size_t r = LZ4F_compressFrame(compressed.data(), compressed.size(), plain.data(), plain.size(), &prefs);
    if (LZ4F_isError(r)) {
        return turbo::internal_error("lz4 compress failed");
    }
    compressed.resize(r);

    turbo::Status st = out->buf->append(compressed.data(), compressed.size());
    if (!st.ok()) {
        return st;
    }
    tail.success();
    return turbo::OkStatus();
}

turbo::Status lz4_decompress_handler(const Compressor &, DecompressOptions opts, const xio::Buffer &in, Output *out) {
    if (opts.max_plain == 0) {
        return turbo::invalid_argument_error("lz4 decompress requires max_plain");
    }

    OutputTail tail(out);

    const std::string wire = in.flatten<std::string>();
    LZ4F_decompressionContext_t dctx = nullptr;
    const LZ4F_errorCode_t create_rc = LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
    if (LZ4F_isError(create_rc)) {
        return turbo::internal_error("lz4 context create failed");
    }

    size_t in_off = 0;
    size_t plain_written = 0;
    std::array<char, xio::kBlockSize> scratch{};

    while (in_off < wire.size()) {
        size_t out_sz = scratch.size();
        size_t in_sz = wire.size() - in_off;
        const size_t r = LZ4F_decompress(dctx, scratch.data(), &out_sz, wire.data() + in_off, &in_sz, nullptr);
        if (LZ4F_isError(r)) {
            LZ4F_freeDecompressionContext(dctx);
            return turbo::data_loss_error("lz4 decompress failed");
        }

        plain_written += out_sz;
        if (plain_written > opts.max_plain) {
            LZ4F_freeDecompressionContext(dctx);
            return turbo::out_of_range_error("lz4 plain exceeds max_plain");
        }

        turbo::Status st = out->buf->append(scratch.data(), out_sz);
        if (!st.ok()) {
            LZ4F_freeDecompressionContext(dctx);
            return st;
        }

        in_off += in_sz;
        if (r == 0) {
            break;
        }
    }

    LZ4F_freeDecompressionContext(dctx);
    if (in_off < wire.size()) {
        return turbo::data_loss_error("lz4 trailing input");
    }

    tail.success();
    return turbo::OkStatus();
}

} // namespace xio::compress::detail
