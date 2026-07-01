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

#include <xio/compress/compress.h>
#include <xio/compress/detail/backends.h>
#include <xio/compress/detail/output_tail.h>

#include <lz4.h>

#include <climits>
#include <cstdint>
#include <cstring>

namespace xio::compress {
namespace {

constexpr size_t kSnappyMaxPlain = static_cast<size_t>(UINT32_MAX) - 1;
constexpr size_t kLz4MaxPlain = static_cast<size_t>(LZ4_MAX_INPUT_SIZE);

turbo::Status validate_output(Output *out) {
    if (out == nullptr || out->buf == nullptr) {
        return turbo::invalid_argument_error("compress output is null");
    }
    return turbo::OkStatus();
}

turbo::Status validate_input(const void *data, size_t size) {
    if (size > 0 && data == nullptr) {
        return turbo::invalid_argument_error("compress input is null");
    }
    return turbo::OkStatus();
}

turbo::Status check_plain_len(const Compressor &c, size_t plain_len) {
    const size_t max_plain = c.max_plain_size();
    if (plain_len > max_plain) {
        return turbo::out_of_range_error("plain length exceeds limit");
    }
    return turbo::OkStatus();
}

turbo::Status copy_plain(const void *data, size_t size, Output *out) {
    detail::OutputTail tail(out);
    turbo::Status st = out->buf->append(data, size);
    if (!st.ok()) {
        return st;
    }
    tail.success();
    return turbo::OkStatus();
}

turbo::Status copy_plain(const xio::Buffer &in, Output *out) {
    detail::OutputTail tail(out);
    for (auto it = in.buffer_begin(); it != in.buffer_end(); ++it) {
        turbo::Status st = out->buf->append(it->data(), it->size());
        if (!st.ok()) {
            return st;
        }
    }
    tail.success();
    return turbo::OkStatus();
}

bool name_equals(const fermat::KString &name, const char *literal) {
    return name.size() == std::strlen(literal) && name.compare(literal) == 0;
}

} // namespace

Compressor Compressor::make_gzip() {
    return {CompressAlgo::kCompressGzip, detail::gzip_compress_handler, detail::gzip_decompress_handler};
}

Compressor Compressor::make_snappy() {
    return {CompressAlgo::kCompressSnappy, detail::snappy_compress_handler, detail::snappy_decompress_handler};
}

Compressor Compressor::make_lz4() {
    return {CompressAlgo::kCompressLz4, detail::lz4_compress_handler, detail::lz4_decompress_handler};
}

Compressor Compressor::make_zstd() {
    return {CompressAlgo::kCompressZstd, detail::zstd_compress_handler, detail::zstd_decompress_handler};
}

Compressor Compressor::from_name(const fermat::KString &name) {
    if (name.empty() || name_equals(name, "none") || name_equals(name, "identity")) {
        return kNoneCompressor;
    }
    if (name_equals(name, "gzip")) {
        return make_gzip();
    }
    if (name_equals(name, "snappy")) {
        return make_snappy();
    }
    if (name_equals(name, "lz4")) {
        return make_lz4();
    }
    if (name_equals(name, "zstd")) {
        return make_zstd();
    }
    return kNoneCompressor;
}

turbo::Status Compressor::compress(CompressOptions opts, const void *data, size_t size, Output *out) const {
    turbo::Status st = validate_output(out);
    if (!st.ok()) {
        return st;
    }
    st = validate_input(data, size);
    if (!st.ok()) {
        return st;
    }
    st = check_plain_len(*this, size);
    if (!st.ok()) {
        return st;
    }
    if (compress_handler == nullptr) {
        return copy_plain(data, size, out);
    }
    xio::Buffer in;
    if (size > 0) {
        st = in.append(data, size);
        if (!st.ok()) {
            return st;
        }
    }
    return compress(opts, in, out);
}

turbo::Status Compressor::compress(CompressOptions opts, const xio::Buffer &in, Output *out) const {
    turbo::Status st = validate_output(out);
    if (!st.ok()) {
        return st;
    }
    st = check_plain_len(*this, in.size());
    if (!st.ok()) {
        return st;
    }
    if (compress_handler == nullptr) {
        return copy_plain(in, out);
    }
    return compress_handler(*this, opts, in, out);
}

turbo::Status Compressor::decompress(DecompressOptions opts, const void *data, size_t size, Output *out) const {
    turbo::Status st = validate_output(out);
    if (!st.ok()) {
        return st;
    }
    st = validate_input(data, size);
    if (!st.ok()) {
        return st;
    }
    if (decompress_handler == nullptr) {
        return copy_plain(data, size, out);
    }
    xio::Buffer in;
    if (size > 0) {
        st = in.append(data, size);
        if (!st.ok()) {
            return st;
        }
    }
    return decompress(opts, in, out);
}

turbo::Status Compressor::decompress(DecompressOptions opts, const xio::Buffer &in, Output *out) const {
    turbo::Status st = validate_output(out);
    if (!st.ok()) {
        return st;
    }
    if (decompress_handler == nullptr) {
        return copy_plain(in, out);
    }
    return decompress_handler(*this, opts, in, out);
}

const char *Compressor::name() const {
    switch (algo) {
        case CompressAlgo::kCompressNone:
            return "none";
        case CompressAlgo::kCompressGzip:
            return "gzip";
        case CompressAlgo::kCompressSnappy:
            return "snappy";
        case CompressAlgo::kCompressLz4:
            return "lz4";
        case CompressAlgo::kCompressZstd:
            return "zstd";
    }
    return "none";
}

size_t Compressor::compress_bound(size_t plain_len) const {
    switch (algo) {
        case CompressAlgo::kCompressNone:
            return plain_len;
        case CompressAlgo::kCompressGzip:
            return detail::gzip_compress_bound(plain_len);
        case CompressAlgo::kCompressSnappy:
            return detail::snappy_compress_bound(plain_len);
        case CompressAlgo::kCompressLz4:
            return detail::lz4_compress_bound(plain_len);
        case CompressAlgo::kCompressZstd:
            return detail::zstd_compress_bound(plain_len);
    }
    return plain_len;
}

size_t Compressor::max_plain_size() const {
    switch (algo) {
        case CompressAlgo::kCompressNone:
        case CompressAlgo::kCompressGzip:
        case CompressAlgo::kCompressZstd:
            return SIZE_MAX;
        case CompressAlgo::kCompressSnappy:
            return kSnappyMaxPlain;
        case CompressAlgo::kCompressLz4:
            return kLz4MaxPlain;
    }
    return SIZE_MAX;
}

bool Compressor::decompress_needs_max_plain() const {
    return algo == CompressAlgo::kCompressLz4;
}

} // namespace xio::compress
