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
#include <xio/compress/fwd.h>

#include <gtest/gtest.h>
#include <string>

namespace xio::compress {

class CompressTest : public ::testing::Test {
protected:
    xio::Buffer compressed;
    xio::Buffer decompressed;

    Output make_output(xio::Buffer *buf) {
        return Output{buf, 0};
    }

    std::string roundtrip(const Compressor &compressor, const std::string &plain) {
        compressed.clear();
        decompressed.clear();

        CompressOptions copts;
        Output cout = make_output(&compressed);
        auto st = compressor.compress(copts, plain.data(), plain.size(), &cout);
        EXPECT_TRUE(st.ok()) << st.message();
        if (!st.ok()) return "";

        DecompressOptions dopts;
        if (compressor.decompress_needs_max_plain()) {
            dopts.max_plain = plain.size() * 2;
        }
        Output dout = make_output(&decompressed);
        st = compressor.decompress(dopts, compressed, &dout);
        EXPECT_TRUE(st.ok()) << st.message();
        if (!st.ok()) return "";

        return decompressed.flatten<std::string>();
    }
};

TEST_F(CompressTest, None) {
    std::string result = roundtrip(kNoneCompressor, "hello world");
    EXPECT_EQ(result, "hello world");
}

TEST_F(CompressTest, Gzip) {
    Compressor c = Compressor::make_gzip();
    std::string result = roundtrip(c, "The quick brown fox jumps over the lazy dog");
    EXPECT_EQ(result, "The quick brown fox jumps over the lazy dog");
}

TEST_F(CompressTest, Snappy) {
    Compressor c = Compressor::make_snappy();
    std::string result = roundtrip(c, "snappy compression test data");
    EXPECT_EQ(result, "snappy compression test data");
}

TEST_F(CompressTest, Lz4) {
    Compressor c = Compressor::make_lz4();
    std::string result = roundtrip(c, "lz4 compression test data");
    EXPECT_EQ(result, "lz4 compression test data");
}

TEST_F(CompressTest, Zstd) {
    Compressor c = Compressor::make_zstd();
    std::string result = roundtrip(c, "zstd compression test data");
    EXPECT_EQ(result, "zstd compression test data");
}

TEST_F(CompressTest, FromName) {
    EXPECT_EQ(Compressor::from_name("gzip").name(), std::string("gzip"));
    EXPECT_EQ(Compressor::from_name("snappy").name(), std::string("snappy"));
    EXPECT_EQ(Compressor::from_name("lz4").name(), std::string("lz4"));
    EXPECT_EQ(Compressor::from_name("zstd").name(), std::string("zstd"));
    EXPECT_EQ(Compressor::from_name("none").name(), std::string("none"));
    EXPECT_EQ(Compressor::from_name("").name(), std::string("none"));
}

TEST_F(CompressTest, LargeData) {
    Compressor c = Compressor::make_zstd();
    std::string large(100000, 'A');
    std::string result = roundtrip(c, large);
    EXPECT_EQ(result, large);
}

TEST_F(CompressTest, CompressBound) {
    Compressor c = Compressor::make_zstd();
    EXPECT_GT(c.compress_bound(1000), 0);
    EXPECT_EQ(kNoneCompressor.compress_bound(1000), 1000);
}

TEST_F(CompressTest, EmptyInput) {
    Compressor c = Compressor::make_gzip();
    std::string result = roundtrip(c, "");
    EXPECT_EQ(result, "");
}

} // namespace xio::compress
