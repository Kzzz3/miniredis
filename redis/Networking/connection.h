#pragma once 

#include <asio.hpp>

using asio::ip::tcp;
using asio::streambuf;
using std::unique_ptr;

constexpr uint64_t BUFFER_MAX_SIZE = 1024 * 1024;

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
	tcp::socket socket;
	ConnectionState state;
	unique_ptr<streambuf> read_buffer;
	unique_ptr<streambuf> write_buffer;

public:
    Connection(uint64_t id, tcp::socket&& socket): 
		id(id),
		socket(std::move(socket)),
        state(ConnectionState::CONN_STATE_NONE), 
        read_buffer(std::make_unique<streambuf>(BUFFER_MAX_SIZE)),
		write_buffer(std::make_unique<streambuf>(BUFFER_MAX_SIZE))
    {
    }

    void AsyncSend(unique_ptr<Sds,decltype(&Sds::destroy)> &str)
    {
		socket.async_send(asio::const_buffer(str->buf, str->length()), [](const asio::error_code& error, size_t length) {});
    }

    void AsyncSend(unique_ptr<Sds, decltype(&Sds::destroy)> &&str)
    {
        socket.async_send(asio::const_buffer(str->buf, str->length()), [](const asio::error_code& error, size_t length) {});
    }
};