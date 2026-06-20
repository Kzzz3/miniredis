#pragma once

#include <gtest/gtest.h>

#include <random>
#include <sstream>
#include <string>
#include <vector>

// ============================================================================
// Helper Functions
// ============================================================================

inline std::string GetRandomString(int length) {
    static thread_local std::mt19937 rng{std::random_device{}()};
    const std::string chars =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string result;
    result.reserve(length);

    std::uniform_int_distribution<> dist(0, chars.size() - 1);
    while (result.size() < static_cast<size_t>(length)) {
        result += chars[dist(rng)];
    }
    return result;
}

inline std::string ConvertToResp(const std::vector<std::string>& args) {
    std::stringstream ss;
    ss << "*" << args.size() << "\r\n";
    for (const auto& arg : args) {
        ss << "$" << arg.length() << "\r\n" << arg << "\r\n";
    }
    return ss.str();
}

// ============================================================================
// RESP Protocol Tests
// ============================================================================

TEST(RespProtocolTest, ConvertSimpleCommand) {
    std::vector<std::string> args = {"PING"};
    std::string resp = ConvertToResp(args);
    EXPECT_EQ(resp, "*1\r\n$4\r\nPING\r\n");
}

TEST(RespProtocolTest, ConvertSetCommand) {
    std::vector<std::string> args = {"SET", "key", "value"};
    std::string resp = ConvertToResp(args);
    EXPECT_EQ(resp, "*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n");
}

TEST(RespProtocolTest, ConvertEmptyArgs) {
    std::vector<std::string> args = {};
    std::string resp = ConvertToResp(args);
    EXPECT_EQ(resp, "*0\r\n");
}

TEST(RespProtocolTest, ConvertSpecialCharacters) {
    std::vector<std::string> args = {"SET", "key with spaces", "value\r\nwith\r\nnewlines"};
    std::string resp = ConvertToResp(args);
    EXPECT_EQ(resp,
              "*3\r\n$3\r\nSET\r\n$15\r\nkey with spaces\r\n$20\r\nvalue\r\nwith\r\nnewlines"
              "\r\n");
}

// ============================================================================
// Random String Generation Tests
// ============================================================================

TEST(RandomStringTest, CorrectLength) {
    for (int len : {0, 1, 10, 100, 1000}) {
        std::string s = GetRandomString(len);
        EXPECT_EQ(s.length(), static_cast<size_t>(len)) << "Length mismatch for " << len;
    }
}

TEST(RandomStringTest, ValidCharacters) {
    const std::string valid_chars =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::string s = GetRandomString(10000);
    for (char c : s) {
        EXPECT_NE(valid_chars.find(c), std::string::npos)
            << "Invalid character: " << c;
    }
}

TEST(RandomStringTest, DifferentOnEachCall) {
    // Statistically, two random strings should be different
    std::string s1 = GetRandomString(100);
    std::string s2 = GetRandomString(100);
    EXPECT_NE(s1, s2);
}
