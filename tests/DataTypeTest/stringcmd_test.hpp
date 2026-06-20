#pragma once

#include <gtest/gtest.h>

#include <asio.hpp>
#include <memory>
#include <string>
#include <vector>

#include "utility.hpp"

// ============================================================================
// String Command Integration Tests
//
// These tests require a running miniredis server on 127.0.0.1:10087
// ============================================================================

class StringCommandTest : public ::testing::Test {
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
        if (connected && socket) {
            try {
                socket->shutdown(asio::ip::tcp::socket::shutdown_both);
                socket->close();
            } catch (...) {
                // Ignore shutdown errors
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
};

TEST_F(StringCommandTest, SetGet) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // SET key value
    sendCommand({"SET", "testkey", "testvalue"});

    // GET key
    sendCommand({"GET", "testkey"});
    std::string response = readResponse();

    // Response should contain "testvalue"
    EXPECT_NE(response.find("testvalue"), std::string::npos);

    // Cleanup
    sendCommand({"DEL", "testkey"});
}

TEST_F(StringCommandTest, GetNonExistent) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"GET", "nonexistentkey"});
    std::string response = readResponse();

    // Should return nil or empty
    EXPECT_TRUE(response.find("$-1") != std::string::npos ||
                response.find("nil") != std::string::npos);
}

TEST_F(StringCommandTest, SetOverwrite) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"SET", "overwritekey", "value1"});
    sendCommand({"SET", "overwritekey", "value2"});

    sendCommand({"GET", "overwritekey"});
    std::string response = readResponse();

    EXPECT_NE(response.find("value2"), std::string::npos);
    EXPECT_EQ(response.find("value1"), std::string::npos);

    // Cleanup
    sendCommand({"DEL", "overwritekey"});
}

TEST_F(StringCommandTest, MultipleOperations) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    const int NUM_KEYS = 100;
    std::vector<std::string> keys;

    // Set multiple keys
    for (int i = 0; i < NUM_KEYS; ++i) {
        std::string key = "multikey" + std::to_string(i);
        std::string value = "value" + std::to_string(i);
        sendCommand({"SET", key, value});
        keys.push_back(key);
    }

    // Get and verify
    for (int i = 0; i < NUM_KEYS; ++i) {
        sendCommand({"GET", keys[i]});
        std::string response = readResponse();
        std::string expected = "value" + std::to_string(i);
        EXPECT_NE(response.find(expected), std::string::npos)
            << "Failed for key: " << keys[i];
    }

    // Cleanup
    for (const auto& key : keys) {
        sendCommand({"DEL", key});
    }
}

TEST_F(StringCommandTest, LargeValue) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    std::string large_value(100000, 'X');
    sendCommand({"SET", "largekey", large_value});

    sendCommand({"GET", "largekey"});
    std::string response = readResponse();

    EXPECT_NE(response.find(large_value), std::string::npos);

    // Cleanup
    sendCommand({"DEL", "largekey"});
}
