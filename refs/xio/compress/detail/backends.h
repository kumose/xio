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

#include <xio/compress/compress.h>

namespace xio::compress::detail {

    turbo::Status gzip_compress_handler(const Compressor &c, CompressOptions opts, const xio::Buffer &in, Output *out);

    turbo::Status gzip_decompress_handler(const Compressor &c, DecompressOptions opts, const xio::Buffer &in,
                                          Output *out);

    turbo::Status snappy_compress_handler(const Compressor &c, CompressOptions opts, const xio::Buffer &in,
                                          Output *out);

    turbo::Status snappy_decompress_handler(const Compressor &c, DecompressOptions opts, const xio::Buffer &in,
                                            Output *out);

    turbo::Status lz4_compress_handler(const Compressor &c, CompressOptions opts, const xio::Buffer &in, Output *out);

    turbo::Status lz4_decompress_handler(const Compressor &c, DecompressOptions opts, const xio::Buffer &in,
                                         Output *out);

    turbo::Status zstd_compress_handler(const Compressor &c, CompressOptions opts, const xio::Buffer &in, Output *out);

    turbo::Status zstd_decompress_handler(const Compressor &c, DecompressOptions opts, const xio::Buffer &in,
                                          Output *out);

    size_t gzip_compress_bound(size_t plain_len);

    size_t snappy_compress_bound(size_t plain_len);

    size_t lz4_compress_bound(size_t plain_len);

    size_t zstd_compress_bound(size_t plain_len);

} // namespace xio::compress::detail
