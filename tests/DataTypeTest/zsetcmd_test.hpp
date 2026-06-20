#pragma once

#include <gtest/gtest.h>

#include <asio.hpp>
#include <memory>
#include <string>
#include <vector>

#include "utility.hpp"

// ============================================================================
// Sorted Set Command Integration Tests
// ============================================================================

class ZSetCommandTest : public ::testing::Test {
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

TEST_F(ZSetCommandTest, ZaddZrange) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"ZADD", "testzset", "1", "member1"});
    sendCommand({"ZADD", "testzset", "2", "member2"});
    sendCommand({"ZADD", "testzset", "3", "member3"});

    sendCommand({"ZRANGE", "testzset", "0", "-1"});
    std::string response = readResponse();

    EXPECT_NE(response.find("member1"), std::string::npos);
    EXPECT_NE(response.find("member2"), std::string::npos);
    EXPECT_NE(response.find("member3"), std::string::npos);

    test_keys.push_back("testzset");
}

TEST_F(ZSetCommandTest, ZaddZrevrange) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"ZADD", "testzset", "1", "member1"});
    sendCommand({"ZADD", "testzset", "2", "member2"});
    sendCommand({"ZADD", "testzset", "3", "member3"});

    sendCommand({"ZREVRANGE", "testzset", "0", "-1"});
    std::string response = readResponse();

    // Should be in reverse order
    size_t pos1 = response.find("member3");
    size_t pos2 = response.find("member2");
    size_t pos3 = response.find("member1");

    EXPECT_LT(pos1, pos2);
    EXPECT_LT(pos2, pos3);

    test_keys.push_back("testzset");
}

TEST_F(ZSetCommandTest, ZaddUpdateScore) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"ZADD", "testzset", "1", "member1"});
    sendCommand({"ZADD", "testzset", "10", "member1"});  // Update score

    sendCommand({"ZRANGE", "testzset", "0", "-1"});
    std::string response = readResponse();

    EXPECT_NE(response.find("member1"), std::string::npos);

    test_keys.push_back("testzset");
}

TEST_F(ZSetCommandTest, ZrangeWithRange) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"ZADD", "testzset", "1", "a"});
    sendCommand({"ZADD", "testzset", "2", "b"});
    sendCommand({"ZADD", "testzset", "3", "c"});
    sendCommand({"ZADD", "testzset", "4", "d"});

    // Get range [1, 2] (0-indexed)
    sendCommand({"ZRANGE", "testzset", "1", "2"});
    std::string response = readResponse();

    EXPECT_EQ(response.find("a"), std::string::npos);  // Should not include
    EXPECT_NE(response.find("b"), std::string::npos);
    EXPECT_NE(response.find("c"), std::string::npos);
    EXPECT_EQ(response.find("d"), std::string::npos);  // Should not include

    test_keys.push_back("testzset");
}

TEST_F(ZSetCommandTest, ZrangeEmpty) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"ZRANGE", "emptyzset", "0", "-1"});
    std::string response = readResponse();

    // Should return empty array
    EXPECT_NE(response.find("*0"), std::string::npos);
}

TEST_F(ZSetCommandTest, ZaddMultipleSameScore) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"ZADD", "testzset", "1", "a"});
    sendCommand({"ZADD", "testzset", "1", "b"});
    sendCommand({"ZADD", "testzset", "1", "c"});

    sendCommand({"ZRANGE", "testzset", "0", "-1"});
    std::string response = readResponse();

    EXPECT_NE(response.find("a"), std::string::npos);
    EXPECT_NE(response.find("b"), std::string::npos);
    EXPECT_NE(response.find("c"), std::string::npos);

    test_keys.push_back("testzset");
}
