#pragma once

#include <gtest/gtest.h>

#include <asio.hpp>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

#include "utility.hpp"

// ============================================================================
// Expiration Command Integration Tests
// ============================================================================

class ExpireCommandTest : public ::testing::Test {
   protected:
    void SetUp() override {
        try {
            io_context = std::make_unique<asio::io_context>();
            socket = std::make_unique<asio::ip::tcp::socket>(*io_context);
            asio::ip::tcp::resolver resolver(*io_context);
            asio::connect(*socket, resolver.resolve("127.0.0.1", "10087"));
            connected = true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to connect to server: " << e.what() << std::endl;
            connected = false;
        }
    }

    void TearDown() override {
        // Cleanup test keys
        for (const auto& key : test_keys) {
            sendCommand({"DEL", key});
        }

        if (connected && socket) {
            try {
                socket->shutdown(asio::ip::tcp::socket::shutdown_both);
                socket->close();
            } catch (...) {
            }
        }
    }

    void sendCommand(const std::vector<std::string>& args) {
        asio::write(*socket, asio::buffer(ConvertToResp(args)));
    }

    std::string readResponse() {
        char buffer[4096];
        size_t n = socket->read_some(asio::buffer(buffer, sizeof(buffer)));
        return std::string(buffer, n);
    }

    std::unique_ptr<asio::io_context> io_context;
    std::unique_ptr<asio::ip::tcp::socket> socket;
    bool connected = false;
    std::vector<std::string> test_keys;
};

TEST_F(ExpireCommandTest, ExpireBasic) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Set a key
    sendCommand({"SET", "testkey", "value"});
    test_keys.push_back("testkey");

    // Set expiration
    sendCommand({"EXPIRE", "testkey", "10"});
    std::string response = readResponse();

    // Should return 1 (success)
    EXPECT_NE(response.find(":1"), std::string::npos);
}

TEST_F(ExpireCommandTest, ExpireNonExistentKey) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Try to expire non-existent key
    sendCommand({"EXPIRE", "nonexistent", "10"});
    std::string response = readResponse();

    // Should return 0 (key doesn't exist)
    EXPECT_NE(response.find(":0"), std::string::npos);
}

TEST_F(ExpireCommandTest, TTLWithExpiration) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Set a key with expiration
    sendCommand({"SET", "testkey", "value"});
    sendCommand({"EXPIRE", "testkey", "100"});
    test_keys.push_back("testkey");

    // Get TTL
    sendCommand({"TTL", "testkey"});
    std::string response = readResponse();

    // Should return a positive number
    EXPECT_EQ(response.find("-1"), std::string::npos);
    EXPECT_EQ(response.find("-2"), std::string::npos);
}

TEST_F(ExpireCommandTest, TTLWithoutExpiration) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Set a key without expiration
    sendCommand({"SET", "testkey", "value"});
    test_keys.push_back("testkey");

    // Get TTL
    sendCommand({"TTL", "testkey"});
    std::string response = readResponse();

    // Should return -1 (no expiration)
    EXPECT_NE(response.find("-1"), std::string::npos);
}

TEST_F(ExpireCommandTest, TTLNonExistentKey) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Get TTL for non-existent key
    sendCommand({"TTL", "nonexistent"});
    std::string response = readResponse();

    // Should return -2 (key doesn't exist)
    EXPECT_NE(response.find("-2"), std::string::npos);
}

TEST_F(ExpireCommandTest, ExpireAndAccess) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Set a key with short expiration
    sendCommand({"SET", "testkey", "value"});
    sendCommand({"EXPIRE", "testkey", "1"});
    test_keys.push_back("testkey");

    // Wait for expiration
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Try to get the key
    sendCommand({"GET", "testkey"});
    std::string response = readResponse();

    // Should return nil (key expired)
    EXPECT_NE(response.find("nil"), std::string::npos);
}

TEST_F(ExpireCommandTest, MultipleKeysExpiration) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    const int NUM_KEYS = 10;
    std::vector<std::string> keys;

    // Set multiple keys with different expirations
    for (int i = 0; i < NUM_KEYS; ++i) {
        std::string key = "expirekey" + std::to_string(i);
        sendCommand({"SET", key, "value"});
        sendCommand({"EXPIRE", key, std::to_string(100 + i)});
        keys.push_back(key);
        test_keys.push_back(key);
    }

    // Verify all keys have TTL
    for (const auto& key : keys) {
        sendCommand({"TTL", key});
        std::string response = readResponse();
        EXPECT_EQ(response.find("-1"), std::string::npos);
        EXPECT_EQ(response.find("-2"), std::string::npos);
    }
}
