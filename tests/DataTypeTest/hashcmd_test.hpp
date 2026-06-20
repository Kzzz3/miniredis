#pragma once

#include <gtest/gtest.h>

#include <asio.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "utility.hpp"

// ============================================================================
// Hash Command Integration Tests
// ============================================================================

class HashCommandTest : public ::testing::Test {
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
        for (const auto& [key, _] : test_keys) {
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
    std::unordered_map<std::string, std::vector<std::string>> test_keys;
};

TEST_F(HashCommandTest, HSetHGet) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"HSET", "testhash", "field1", "value1"});

    sendCommand({"HGET", "testhash", "field1"});
    std::string response = readResponse();

    EXPECT_NE(response.find("value1"), std::string::npos);
    test_keys["testhash"] = {"field1"};
}

TEST_F(HashCommandTest, HGetNonExistent) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"HGET", "nonexistent", "field"});
    std::string response = readResponse();

    EXPECT_TRUE(response.find("$-1") != std::string::npos ||
                response.find("nil") != std::string::npos);
}

TEST_F(HashCommandTest, HSetMultipleFields) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"HSET", "testhash", "f1", "v1"});
    sendCommand({"HSET", "testhash", "f2", "v2"});
    sendCommand({"HSET", "testhash", "f3", "v3"});

    sendCommand({"HGET", "testhash", "f1"});
    EXPECT_NE(readResponse().find("v1"), std::string::npos);

    sendCommand({"HGET", "testhash", "f2"});
    EXPECT_NE(readResponse().find("v2"), std::string::npos);

    sendCommand({"HGET", "testhash", "f3"});
    EXPECT_NE(readResponse().find("v3"), std::string::npos);

    test_keys["testhash"] = {"f1", "f2", "f3"};
}

TEST_F(HashCommandTest, HGetAll) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"HSET", "testhash", "f1", "v1"});
    sendCommand({"HSET", "testhash", "f2", "v2"});

    sendCommand({"HGETALL", "testhash"});
    std::string response = readResponse();

    EXPECT_NE(response.find("f1"), std::string::npos);
    EXPECT_NE(response.find("v1"), std::string::npos);
    EXPECT_NE(response.find("f2"), std::string::npos);
    EXPECT_NE(response.find("v2"), std::string::npos);

    test_keys["testhash"] = {"f1", "f2"};
}

TEST_F(HashCommandTest, HSetOverwrite) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"HSET", "testhash", "field", "value1"});
    sendCommand({"HSET", "testhash", "field", "value2"});

    sendCommand({"HGET", "testhash", "field"});
    std::string response = readResponse();

    EXPECT_NE(response.find("value2"), std::string::npos);
    test_keys["testhash"] = {"field"};
}
