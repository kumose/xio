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

#include <snappy-c.h>

#include <string>

namespace xio::compress::detail {

size_t snappy_compress_bound(size_t plain_len) {
    return ::snappy_max_compressed_length(plain_len);
}

turbo::Status snappy_compress_handler(const Compressor &, CompressOptions, const xio::Buffer &in, Output *out) {
    OutputTail tail(out);

    const std::string plain = in.flatten<std::string>();
    std::string compressed;
    compressed.assign(snappy_compress_bound(plain.size()), '\0');
    size_t compressed_len = compressed.size();
    if (::snappy_compress(plain.data(), plain.size(), compressed.data(), &compressed_len) != SNAPPY_OK) {
        return turbo::internal_error("snappy compress failed");
    }
    compressed.resize(compressed_len);

    turbo::Status st = out->buf->append(compressed.data(), compressed.size());
    if (!st.ok()) {
        return st;
    }
    tail.success();
    return turbo::OkStatus();
}

turbo::Status snappy_decompress_handler(const Compressor &, DecompressOptions, const xio::Buffer &in, Output *out) {
    OutputTail tail(out);

    const std::string wire = in.flatten<std::string>();
    size_t plain_len = 0;
    if (::snappy_uncompressed_length(wire.data(), wire.size(), &plain_len) != SNAPPY_OK) {
        return turbo::data_loss_error("snappy length probe failed");
    }

    std::string plain;
    plain.assign(plain_len, '\0');
    size_t n = plain_len;
    if (::snappy_uncompress(wire.data(), wire.size(), plain.data(), &n) != SNAPPY_OK) {
        return turbo::data_loss_error("snappy uncompress failed");
    }
    plain.resize(n);

    turbo::Status st = out->buf->append(plain.data(), plain.size());
    if (!st.ok()) {
        return st;
    }
    tail.success();
    return turbo::OkStatus();
}

} // namespace xio::compress::detail
