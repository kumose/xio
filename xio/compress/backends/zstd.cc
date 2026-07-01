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

#include <zstd.h>

#include <string>

namespace xio::compress::detail {
namespace {

int zstd_level(CompressOptions opts) {
    return opts.level == 0 ? 3 : opts.level;
}

} // namespace

size_t zstd_compress_bound(size_t plain_len) {
    return ZSTD_compressBound(plain_len);
}

turbo::Status zstd_compress_handler(const Compressor &, CompressOptions opts, const xio::Buffer &in, Output *out) {
    OutputTail tail(out);

    const std::string plain = in.flatten<std::string>();
    std::string compressed;
    compressed.assign(ZSTD_compressBound(plain.size()), '\0');
    const size_t r = ZSTD_compress(compressed.data(), compressed.size(), plain.data(), plain.size(), zstd_level(opts));
    if (ZSTD_isError(r)) {
        return turbo::internal_error("zstd compress failed");
    }
    compressed.resize(r);

    turbo::Status st = out->buf->append(compressed.data(), compressed.size());
    if (!st.ok()) {
        return st;
    }
    tail.success();
    return turbo::OkStatus();
}

turbo::Status zstd_decompress_handler(const Compressor &, DecompressOptions opts, const xio::Buffer &in, Output *out) {
    OutputTail tail(out);

    const std::string wire = in.flatten<std::string>();
    unsigned long long out_size = ZSTD_getFrameContentSize(wire.data(), wire.size());
    switch (out_size) {
        case ZSTD_CONTENTSIZE_UNKNOWN:
            out_size = wire.size() * 2;
            break;
        case ZSTD_CONTENTSIZE_ERROR:
            return turbo::data_loss_error("zstd content size error");
        default:
            break;
    }
    if (opts.max_plain != 0 && out_size > opts.max_plain) {
        return turbo::out_of_range_error("zstd plain exceeds max_plain");
    }

    while (true) {
        std::string plain;
        plain.assign(static_cast<size_t>(out_size), '\0');
        const size_t r = ZSTD_decompress(plain.data(), plain.size(), wire.data(), wire.size());
        if (!ZSTD_isError(r)) {
            plain.resize(r);
            turbo::Status st = out->buf->append(plain.data(), plain.size());
            if (!st.ok()) {
                return st;
            }
            tail.success();
            return turbo::OkStatus();
        }
        if (ZSTD_getErrorCode(r) != ZSTD_error_dstSize_tooSmall) {
            return turbo::data_loss_error("zstd decompress failed");
        }
        out_size *= 2;
        if (out_size > wire.size() * 256) {
            return turbo::data_loss_error("zstd output too large");
        }
    }
}

} // namespace xio::compress::detail
