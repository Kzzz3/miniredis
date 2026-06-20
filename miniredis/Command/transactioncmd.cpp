#include "command.h"
#include "server.h"

bool CmdMulti(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 1)
        return false;

    if (conn->in_transaction)
    {
        auto reply = GenerateErrorReply("ERR MULTI calls can not be nested");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    conn->multi();
    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("OK"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdExec(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 1)
        return false;

    if (!conn->in_transaction)
    {
        auto reply = GenerateErrorReply("ERR EXEC without MULTI");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    // Check if any watched keys have been modified
    if (!conn->watched_keys.empty())
    {
        for (auto& watched_key : conn->watched_keys)
        {
            // Check if the key has been modified since WATCH
            // For simplicity, we'll just check if the key still exists
            // In a real implementation, we'd track version numbers
            HashTable<RedisObj*>& kvstore = server.database.getKVStore(watched_key);
            if (!kvstore.contains(watched_key))
            {
                // Key was deleted, abort transaction
                conn->discard();
                auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("nil"), nullptr));
                conn->AsyncSend(std::move(reply));
                return true;
            }
        }
    }

    // Execute all queued commands
    vector<unique_ptr<ValueRef>> results;
    for (auto& queued_cmd : conn->command_queue)
    {
        // Get command handler
        auto handler = GetCommandHandler(queued_cmd[0]);
        if (handler)
        {
            // Execute command
            bool success = handler(conn, queued_cmd);
            // Note: In a real implementation, we'd collect the results
            // For simplicity, we'll just execute them
        }
    }

    // Clear transaction state
    conn->in_transaction = false;
    conn->command_queue.clear();
    conn->watched_keys.clear();

    // Return OK for now
    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("OK"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdDiscard(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 1)
        return false;

    if (!conn->in_transaction)
    {
        auto reply = GenerateErrorReply("ERR DISCARD without MULTI");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    conn->discard();
    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("OK"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdWatch(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() < 2)
        return false;

    if (conn->in_transaction)
    {
        auto reply = GenerateErrorReply("ERR WATCH inside MULTI is not allowed");
        conn->AsyncSend(std::move(reply));
        return false;
    }

    for (size_t i = 1; i < cmd.size(); ++i)
    {
        conn->watch(cmd[i]);
    }

    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("OK"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}

bool CmdUnwatch(shared_ptr<Connection> conn, Command& cmd)
{
    if (cmd.size() != 1)
        return false;

    conn->unwatch();
    auto reply = GenerateReply(make_unique<ValueRef>(Sds::create("OK"), nullptr));
    conn->AsyncSend(std::move(reply));
    return true;
}
