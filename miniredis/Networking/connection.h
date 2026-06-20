#pragma once
#include <mutex>
#include <atomic>
#include <vector>
#include <unordered_set>
#include <asio.hpp>

using asio::streambuf;
using asio::ip::tcp;
using std::lock_guard;
using std::mutex;
using std::unique_ptr;
using std::atomic;
using std::vector;
using std::unordered_set;

constexpr uint64_t BUFFER_MAX_SIZE = 1024 * 1024 * 15;

// Forward declaration
class Sds;
using Command = vector<Sds*>;

enum class ConnectionState : uint8_t
{
    CONN_STATE_NONE,
    CONN_STATE_CONNECTING,
    CONN_STATE_ACCEPTING,
    CONN_STATE_CONNECTED,
    CONN_STATE_CLOSED,
    CONN_STATE_ERROR
};

class Connection
{
public:
    uint64_t id;
    atomic<ConnectionState> state;  // Use atomic for lock-free state check
    tcp::socket socket;
    streambuf read_buffer;

    // Transaction state
    bool in_transaction = false;  // Whether we are in a transaction
    vector<Command> command_queue;  // Queued commands for transaction
    unordered_set<Sds*> watched_keys;  // Keys being watched for optimistic locking

public:
    Connection(uint64_t id, tcp::socket&& socket)
        : id(id), state(ConnectionState::CONN_STATE_NONE),
          socket(std::move(socket)), read_buffer(BUFFER_MAX_SIZE)
    {
    }

    void Close()
    {
        // Use atomic exchange to ensure only one thread closes
        ConnectionState expected = ConnectionState::CONN_STATE_CONNECTED;
        if (state.compare_exchange_strong(expected, ConnectionState::CONN_STATE_CLOSED))
        {
            asio::error_code ec;
            socket.shutdown(tcp::socket::shutdown_both, ec);
            socket.close(ec);
        }
    }

    void Send(unique_ptr<Sds, decltype(&Sds::destroy)>&& str)
    {
        if (state.load() == ConnectionState::CONN_STATE_CLOSED)
            return;

        asio::error_code ec;
        auto buffer = asio::const_buffer(str->buf, str->length());
        socket.send(buffer, asio::socket_base::message_flags(0), ec);
    }

    void AsyncSend(unique_ptr<Sds, decltype(&Sds::destroy)>&& str)
    {
        if (state.load() == ConnectionState::CONN_STATE_CLOSED)
            return;

        auto buffer = asio::const_buffer(str->buf, str->length());
        socket.async_send(buffer, [sds = std::move(str)](const asio::error_code&, size_t) {});
    }

    // Transaction methods
    void multi()
    {
        in_transaction = true;
        command_queue.clear();
    }

    void discard()
    {
        in_transaction = false;
        // Clear command queue
        for (auto& cmd : command_queue)
        {
            for (auto& sds : cmd)
                Sds::destroy(sds);
        }
        command_queue.clear();
        watched_keys.clear();
    }

    void watch(Sds* key)
    {
        watched_keys.insert(key);
    }

    void unwatch()
    {
        watched_keys.clear();
    }
};