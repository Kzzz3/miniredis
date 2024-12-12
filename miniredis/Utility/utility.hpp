#pragma once
#include <chrono>
#include <optional>
#include <charconv>

using std::optional;

template <typename T>
constexpr std::optional<T> str2num(const char* str, size_t len) {
    static_assert(std::is_arithmetic_v<T>, "T must be a numeric type");

    if (str == nullptr || len == 0) {
        return std::nullopt;
    }

    T value;
    std::from_chars_result result = std::from_chars(str, str + len, value);

    if (result.ec == std::errc{} && result.ptr == str + len) {
        return value;
    }

    return std::nullopt;
}


inline uint32_t GetSecTimestamp() {
    return std::time(nullptr);
}
