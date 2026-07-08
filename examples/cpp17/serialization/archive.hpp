//
// archive.hpp
// ~~~~~~~~~~~
//
// Simple length-prefixed binary serialization for the stock example.
//

#ifndef SERIALIZATION_ARCHIVE_HPP
#define SERIALIZATION_ARCHIVE_HPP

#include "stock.hpp"

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace s11n_example {

namespace detail {

inline void append(std::string& out, const void* data, std::size_t size) {
    out.append(static_cast<const char*>(data), size);
}

inline void read(const char*& p, const char* end, void* data, std::size_t size) {
    if (static_cast<std::size_t>(end - p) < size) {
        throw std::runtime_error("archive underflow");
    }
    std::memcpy(data, p, size);
    p += size;
}

template<typename T>
void append_pod(std::string& out, T value) {
    append(out, &value, sizeof(T));
}

template<typename T>
void read_pod(const char*& p, const char* end, T& value) {
    read(p, end, &value, sizeof(T));
}

inline void append_string(std::string& out, const std::string& s) {
    const auto len = static_cast<std::uint32_t>(s.size());
    append_pod(out, len);
    append(out, s.data(), s.size());
}

inline void read_string(const char*& p, const char* end, std::string& s) {
    std::uint32_t len = 0;
    read_pod(p, end, len);
    if (static_cast<std::size_t>(end - p) < len) {
        throw std::runtime_error("archive underflow");
    }
    s.assign(p, len);
    p += len;
}

} // namespace detail

inline void encode(std::string& out, const stock& s) {
    detail::append_string(out, s.code);
    detail::append_string(out, s.name);
    detail::append_pod(out, s.open_price);
    detail::append_pod(out, s.high_price);
    detail::append_pod(out, s.low_price);
    detail::append_pod(out, s.last_price);
    detail::append_pod(out, s.buy_price);
    detail::append_pod(out, s.buy_quantity);
    detail::append_pod(out, s.sell_price);
    detail::append_pod(out, s.sell_quantity);
}

inline void decode(const char*& p, const char* end, stock& s) {
    detail::read_string(p, end, s.code);
    detail::read_string(p, end, s.name);
    detail::read_pod(p, end, s.open_price);
    detail::read_pod(p, end, s.high_price);
    detail::read_pod(p, end, s.low_price);
    detail::read_pod(p, end, s.last_price);
    detail::read_pod(p, end, s.buy_price);
    detail::read_pod(p, end, s.buy_quantity);
    detail::read_pod(p, end, s.sell_price);
    detail::read_pod(p, end, s.sell_quantity);
}

inline void encode(std::string& out, const std::vector<stock>& stocks) {
    const auto count = static_cast<std::uint32_t>(stocks.size());
    detail::append_pod(out, count);
    for (const auto& s : stocks) {
        encode(out, s);
    }
}

inline void decode(const char*& p, const char* end, std::vector<stock>& stocks) {
    std::uint32_t count = 0;
    detail::read_pod(p, end, count);
    stocks.clear();
    stocks.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        stock s;
        decode(p, end, s);
        stocks.push_back(std::move(s));
    }
}

} // namespace s11n_example

#endif // SERIALIZATION_ARCHIVE_HPP
