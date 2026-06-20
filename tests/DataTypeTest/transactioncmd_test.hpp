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
// Transaction Command Integration Tests
// ============================================================================

class TransactionCommandTest : public ::testing::Test {
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

TEST_F(TransactionCommandTest, MultiBasic) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Start transaction
    sendCommand({"MULTI"});
    std::string response = readResponse();

    // Should return OK
    EXPECT_NE(response.find("OK"), std::string::npos);
}

TEST_F(TransactionCommandTest, ExecBasic) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Start transaction
    sendCommand({"MULTI"});
    readResponse();

    // Queue some commands
    sendCommand({"SET", "txkey1", "value1"});
    std::string response = readResponse();
    EXPECT_NE(response.find("QUEUED"), std::string::npos);

    sendCommand({"SET", "txkey2", "value2"});
    response = readResponse();
    EXPECT_NE(response.find("QUEUED"), std::string::npos);

    // Execute transaction
    sendCommand({"EXEC"});
    response = readResponse();

    // Should return OK
    EXPECT_NE(response.find("OK"), std::string::npos);

    // Verify keys were set
    sendCommand({"GET", "txkey1"});
    response = readResponse();
    EXPECT_NE(response.find("value1"), std::string::npos);

    sendCommand({"GET", "txkey2"});
    response = readResponse();
    EXPECT_NE(response.find("value2"), std::string::npos);

    test_keys.push_back("txkey1");
    test_keys.push_back("txkey2");
}

TEST_F(TransactionCommandTest, DiscardBasic) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Start transaction
    sendCommand({"MULTI"});
    readResponse();

    // Queue some commands
    sendCommand({"SET", "discardkey", "value"});
    readResponse();

    // Discard transaction
    sendCommand({"DISCARD"});
    std::string response = readResponse();

    // Should return OK
    EXPECT_NE(response.find("OK"), std::string::npos);

    // Verify key was NOT set
    sendCommand({"GET", "discardkey"});
    response = readResponse();
    EXPECT_NE(response.find("nil"), std::string::npos);
}

TEST_F(TransactionCommandTest, WatchBasic) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Set a key
    sendCommand({"SET", "watchkey", "value"});
    readResponse();
    test_keys.push_back("watchkey");

    // Watch the key
    sendCommand({"WATCH", "watchkey"});
    std::string response = readResponse();

    // Should return OK
    EXPECT_NE(response.find("OK"), std::string::npos);
}

TEST_F(TransactionCommandTest, UnwatchBasic) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Watch a key
    sendCommand({"WATCH", "unwatchkey"});
    readResponse();

    // Unwatch
    sendCommand({"UNWATCH"});
    std::string response = readResponse();

    // Should return OK
    EXPECT_NE(response.find("OK"), std::string::npos);
}

TEST_F(TransactionCommandTest, NestedMultiError) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Start transaction
    sendCommand({"MULTI"});
    readResponse();

    // Try to start another transaction
    sendCommand({"MULTI"});
    std::string response = readResponse();

    // Should return error
    EXPECT_NE(response.find("ERR"), std::string::npos);

    // Cleanup
    sendCommand({"DISCARD"});
    readResponse();
}

TEST_F(TransactionCommandTest, ExecWithoutMultiError) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Try to execute without MULTI
    sendCommand({"EXEC"});
    std::string response = readResponse();

    // Should return error
    EXPECT_NE(response.find("ERR"), std::string::npos);
}

TEST_F(TransactionCommandTest, DiscardWithoutMultiError) {
    if (!connected) {
        GTEST_SKIP() << "Server not available";
    }

    // Try to discard without MULTI
    sendCommand({"DISCARD"});
    std::string response = readResponse();

    // Should return error
    EXPECT_NE(response.find("ERR"), std::string::npos);
}
