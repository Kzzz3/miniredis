module;

#include <asio.hpp>

export module connection;

import db;
import std;

using std::uint64_t;
using asio::ip::tcp;
using asio::streambuf;
using std::unique_ptr;

export constexpr uint64_t BUFFER_MAX_SIZE = 1024 * 1024;

export enum class ConnectionState
{
    CONN_STATE_NONE,
    CONN_STATE_CONNECTING,
    CONN_STATE_ACCEPTING,
    CONN_STATE_CONNECTED,
    CONN_STATE_CLOSED,
    CONN_STATE_ERROR
};

export class Connection
{
public:
    uint64_t id;
	tcp::socket socket;
	ConnectionState state;
	unique_ptr<streambuf> buffer;

    RedisDb* db;


public:
    Connection(asio::io_context& io_context, uint64_t id): 
        id(id), 
        socket(io_context), 
        state(ConnectionState::CONN_STATE_NONE), 
        buffer(std::make_unique<streambuf>(BUFFER_MAX_SIZE))
    {
    }
};