#pragma once
#include <mutex>
#include <asio.hpp>

using asio::streambuf;
using asio::ip::tcp;
using std::lock_guard;
using std::mutex;
using std::unique_ptr;

constexpr uint64_t BUFFER_MAX_SIZE = 1024 * 1024 * 15;

enum class ConnectionState
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
    mutex close_mutex;
    tcp::socket socket;
    ConnectionState state;
    unique_ptr<streambuf> read_buffer;

public:
    Connection(uint64_t id, tcp::socket&& socket)
        : id(id), socket(std::move(socket)), state(ConnectionState::CONN_STATE_NONE),
          read_buffer(std::make_unique<streambuf>(BUFFER_MAX_SIZE))
    {
    }

    void Close()
    {
        lock_guard<mutex> lock(close_mutex);

        socket.close();
        state = ConnectionState::CONN_STATE_CLOSED;
    }

    void AsyncSend(unique_ptr<Sds, decltype(&Sds::destroy)>& str)
    {
        lock_guard<mutex> lock(close_mutex);
        if (state == ConnectionState::CONN_STATE_CLOSED)
            return;

        auto buffer = asio::const_buffer(str->buf, str->length());
        socket.async_send(buffer,
                          [sds = std::move(str)](const asio::error_code& error, size_t length) {});
    }

    void AsyncSend(unique_ptr<Sds, decltype(&Sds::destroy)>&& str)
    {
        lock_guard<mutex> lock(close_mutex);
        if (state == ConnectionState::CONN_STATE_CLOSED)
            return;

        auto buffer = asio::const_buffer(str->buf, str->length());
        socket.async_send(buffer,
                          [sds = std::move(str)](const asio::error_code& error, size_t length) {});
    }
};