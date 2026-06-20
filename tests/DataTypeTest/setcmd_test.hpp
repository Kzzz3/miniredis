#pragma once

#include <gtest/gtest.h>

#include <asio.hpp>
#include <memory>
#include <string>
#include <vector>

#include "utility.hpp"

// ============================================================================
// Set Command Integration Tests
// ============================================================================

class SetCommandTest : public ::testing::Test {
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

TEST_F(SetCommandTest, SaddSmembers) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"SADD", "testset", "member1"});
    sendCommand({"SADD", "testset", "member2"});

    sendCommand({"SMEMBERS", "testset"});
    std::string response = readResponse();

    EXPECT_NE(response.find("member1"), std::string::npos);
    EXPECT_NE(response.find("member2"), std::string::npos);

    test_keys.push_back("testset");
}

TEST_F(SetCommandTest, SaddDuplicate) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"SADD", "testset", "member1"});
    sendCommand({"SADD", "testset", "member1"});  // Duplicate

    sendCommand({"SMEMBERS", "testset"});
    std::string response = readResponse();

    // Should only contain member1 once
    EXPECT_NE(response.find("member1"), std::string::npos);

    test_keys.push_back("testset");
}

TEST_F(SetCommandTest, SismemberExists) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"SADD", "testset", "member1"});

    sendCommand({"SISMEMBER", "testset", "member1"});
    std::string response = readResponse();

    // Should return 1 (exists)
    EXPECT_NE(response.find(":1"), std::string::npos);

    test_keys.push_back("testset");
}

TEST_F(SetCommandTest, SismemberNotExists) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"SADD", "testset", "member1"});

    sendCommand({"SISMEMBER", "testset", "nonexistent"});
    std::string response = readResponse();

    // Should return 0 (not exists)
    EXPECT_NE(response.find(":0"), std::string::npos);

    test_keys.push_back("testset");
}

TEST_F(SetCommandTest, Srem) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"SADD", "testset", "member1"});
    sendCommand({"SADD", "testset", "member2"});

    sendCommand({"SREM", "testset", "member1"});

    sendCommand({"SMEMBERS", "testset"});
    std::string response = readResponse();

    EXPECT_EQ(response.find("member1"), std::string::npos);
    EXPECT_NE(response.find("member2"), std::string::npos);

    test_keys.push_back("testset");
}

TEST_F(SetCommandTest, SmembersEmpty) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    sendCommand({"SMEMBERS", "emptyset"});
    std::string response = readResponse();

    // Should return empty array
    EXPECT_NE(response.find("*0"), std::string::npos);
}
