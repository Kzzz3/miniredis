#pragma once

#include <gtest/gtest.h>

#include <asio.hpp>
#include <memory>
#include <string>
#include <vector>

#include "utility.hpp"

// ============================================================================
// General Command Integration Tests
// ============================================================================

class GeneralCommandTest : public ::testing::Test {
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

// ============================================================================
// PING Command
// ============================================================================

TEST_F(GeneralCommandTest, Ping) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"PING"});
    std::string response = readResponse();

    EXPECT_NE(response.find("PONG"), std::string::npos);
}

// ============================================================================
// DEL Command
// ============================================================================

TEST_F(GeneralCommandTest, DelExistingKey) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Create a key
    sendCommand({"SET", "delkey", "value"});
    readResponse();

    // Delete it
    sendCommand({"DEL", "delkey"});
    std::string response = readResponse();

    // Verify it's gone
    sendCommand({"GET", "delkey"});
    response = readResponse();
    EXPECT_NE(response.find("nil"), std::string::npos);
}

TEST_F(GeneralCommandTest, DelNonExistentKey) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"DEL", "nonexistent"});
    std::string response = readResponse();

    // Should return error or nil
    EXPECT_TRUE(response.find("nil") != std::string::npos ||
                response.find("-ERR") != std::string::npos);
}

// ============================================================================
// INCR/DECR Commands
// ============================================================================

TEST_F(GeneralCommandTest, IncrBasic) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Set initial value
    sendCommand({"SET", "incrkey", "10"});
    readResponse();
    test_keys.push_back("incrkey");

    // Increment
    sendCommand({"INCR", "incrkey"});
    std::string response = readResponse();

    // Should return 11
    EXPECT_NE(response.find("11"), std::string::npos);
}

TEST_F(GeneralCommandTest, IncrFromZero) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Set to 0
    sendCommand({"SET", "incrkey", "0"});
    readResponse();
    test_keys.push_back("incrkey");

    // Increment
    sendCommand({"INCR", "incrkey"});
    std::string response = readResponse();

    // Should return 1
    EXPECT_NE(response.find(":1"), std::string::npos);
}

TEST_F(GeneralCommandTest, DecrBasic) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Set initial value
    sendCommand({"SET", "decrkey", "10"});
    readResponse();
    test_keys.push_back("decrkey");

    // Decrement
    sendCommand({"DECR", "decrkey"});
    std::string response = readResponse();

    // Should return 9
    EXPECT_NE(response.find("9"), std::string::npos);
}

TEST_F(GeneralCommandTest, IncrNonExistentKey) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Increment non-existent key
    sendCommand({"INCR", "newincrkey"});
    std::string response = readResponse();
    test_keys.push_back("newincrkey");

    // Should return 1
    EXPECT_NE(response.find(":1"), std::string::npos);
}

// ============================================================================
// MSET Command
// ============================================================================

TEST_F(GeneralCommandTest, MsetBasic) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // MSET multiple keys
    sendCommand({"MSET", "mset1", "val1", "mset2", "val2", "mset3", "val3"});
    std::string response = readResponse();
    test_keys.insert(test_keys.end(), {"mset1", "mset2", "mset3"});

    // Verify all keys
    sendCommand({"GET", "mset1"});
    EXPECT_NE(readResponse().find("val1"), std::string::npos);

    sendCommand({"GET", "mset2"});
    EXPECT_NE(readResponse().find("val2"), std::string::npos);

    sendCommand({"GET", "mset3"});
    EXPECT_NE(readResponse().find("val3"), std::string::npos);
}

// ============================================================================
// LRANGE Command
// ============================================================================

TEST_F(GeneralCommandTest, LrangeBasic) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Create a list
    sendCommand({"LPUSH", "listkey", "c"});
    sendCommand({"LPUSH", "listkey", "b"});
    sendCommand({"LPUSH", "listkey", "a"});
    test_keys.push_back("listkey");

    // Get range
    sendCommand({"LRANGE", "listkey", "0", "-1"});
    std::string response = readResponse();

    // Should contain all elements
    EXPECT_NE(response.find("a"), std::string::npos);
    EXPECT_NE(response.find("b"), std::string::npos);
    EXPECT_NE(response.find("c"), std::string::npos);
}

TEST_F(GeneralCommandTest, LrangeWithRange) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Create a list
    sendCommand({"RPUSH", "listkey", "a"});
    sendCommand({"RPUSH", "listkey", "b"});
    sendCommand({"RPUSH", "listkey", "c"});
    sendCommand({"RPUSH", "listkey", "d"});
    test_keys.push_back("listkey");

    // Get range [1, 2]
    sendCommand({"LRANGE", "listkey", "1", "2"});
    std::string response = readResponse();

    // Should contain b and c, not a and d
    EXPECT_NE(response.find("b"), std::string::npos);
    EXPECT_NE(response.find("c"), std::string::npos);
}

// ============================================================================
// SPOP Command
// ============================================================================

TEST_F(GeneralCommandTest, SpopBasic) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Create a set
    sendCommand({"SADD", "setkey", "member1"});
    sendCommand({"SADD", "setkey", "member2"});
    test_keys.push_back("setkey");

    // Pop a member
    sendCommand({"SPOP", "setkey"});
    std::string response = readResponse();

    // Should return one of the members
    EXPECT_TRUE(response.find("member1") != std::string::npos ||
                response.find("member2") != std::string::npos);
}

// ============================================================================
// ZPOPMIN Command
// ============================================================================

TEST_F(GeneralCommandTest, ZpopminBasic) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Create a sorted set
    sendCommand({"ZADD", "zsetkey", "1", "a"});
    sendCommand({"ZADD", "zsetkey", "2", "b"});
    sendCommand({"ZADD", "zsetkey", "3", "c"});
    test_keys.push_back("zsetkey");

    // Pop min
    sendCommand({"ZPOPMIN", "zsetkey"});
    std::string response = readResponse();

    // Should return member with score 1
    EXPECT_NE(response.find("a"), std::string::npos);
    EXPECT_NE(response.find("1"), std::string::npos);
}

// ============================================================================
// Error Handling
// ============================================================================

TEST_F(GeneralCommandTest, WrongTypeOperation) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Create a string key
    sendCommand({"SET", "stringkey", "value"});
    readResponse();
    test_keys.push_back("stringkey");

    // Try to use it as a list
    sendCommand({"LPUSH", "stringkey", "item"});
    std::string response = readResponse();

    // Should return WRONGTYPE error
    EXPECT_NE(response.find("WRONGTYPE"), std::string::npos);
}

TEST_F(GeneralCommandTest, InvalidExpireTime) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"SET", "expirekey", "value"});
    readResponse();
    test_keys.push_back("expirekey");

    // Try to set invalid expire time
    sendCommand({"EXPIRE", "expirekey", "-1"});
    std::string response = readResponse();

    // Should return error
    EXPECT_NE(response.find("-ERR"), std::string::npos);
}
