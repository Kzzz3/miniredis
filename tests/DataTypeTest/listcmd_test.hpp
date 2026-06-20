#pragma once

#include <gtest/gtest.h>

#include <asio.hpp>
#include <memory>
#include <string>
#include <vector>

#include "utility.hpp"

// ============================================================================
// List Command Integration Tests
// ============================================================================

class ListCommandTest : public ::testing::Test {
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

TEST_F(ListCommandTest, LPushLPop) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"LPUSH", "testlist", "value1"});
    sendCommand({"LPUSH", "testlist", "value2"});

    sendCommand({"LPOP", "testlist"});
    std::string response = readResponse();

    // LIFO - should get value2 first
    EXPECT_NE(response.find("value2"), std::string::npos);

    test_keys.push_back("testlist");
}

TEST_F(ListCommandTest, RPushRPop) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"RPUSH", "testlist", "value1"});
    sendCommand({"RPUSH", "testlist", "value2"});

    sendCommand({"RPOP", "testlist"});
    std::string response = readResponse();

    // FIFO - should get value2 first
    EXPECT_NE(response.find("value2"), std::string::npos);

    test_keys.push_back("testlist");
}

TEST_F(ListCommandTest, LPushMultiple) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"LPUSH", "testlist", "v1", "v2", "v3"});

    sendCommand({"LPOP", "testlist"});
    EXPECT_NE(readResponse().find("v3"), std::string::npos);

    sendCommand({"LPOP", "testlist"});
    EXPECT_NE(readResponse().find("v2"), std::string::npos);

    sendCommand({"LPOP", "testlist"});
    EXPECT_NE(readResponse().find("v1"), std::string::npos);

    test_keys.push_back("testlist");
}

TEST_F(ListCommandTest, PopEmptyList) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"LPOP", "emptylist"});
    std::string response = readResponse();

    // Should return nil or empty
    EXPECT_TRUE(response.find("$-1") != std::string::npos ||
                response.find("nil") != std::string::npos);
}

TEST_F(ListCommandTest, MixedPushPop) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"LPUSH", "testlist", "left"});
    sendCommand({"RPUSH", "testlist", "right"});

    sendCommand({"LPOP", "testlist"});
    EXPECT_NE(readResponse().find("left"), std::string::npos);

    sendCommand({"RPOP", "testlist"});
    EXPECT_NE(readResponse().find("right"), std::string::npos);

    test_keys.push_back("testlist");
}
