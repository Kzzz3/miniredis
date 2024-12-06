#include "command.h"
#include "../server.h"

void CmdSAdd(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size < 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (!db->kvstore.contains(cmd[1]))
	{
		db->kvstore[cmd[1]] = CreateSetObject();
	}

	for (size_t i = 2; i < size; i++)
		SetObjAdd(db->kvstore[cmd[1]], cmd[i]);
}

void CmdSRem(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size < 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		for (size_t i = 2; i < size; i++)
			SetObjRemove(db->kvstore[cmd[1]], cmd[i]);
	}
	else
	{
		auto reply = GenerateErrorReply("nil");
		server.response_queue.push_back({ std::move(reply), conn });
	}
}

void CmdSMembers(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size != 2)
		return;
	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto reply = GenerateReply(SetObjMembers(db->kvstore[cmd[1]]));
		server.response_queue.push_back({ std::move(reply), conn });
	}
	else
	{
		auto reply = GenerateErrorReply("nil");
		server.response_queue.push_back({ std::move(reply), conn });
	}
}

void CmdSisMember(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto reply = SetObjIsMember(db->kvstore[cmd[1]], cmd[2]) ?
			GenerateReply(make_unique<ValueRef>(Sds::create("true", 4), nullptr)) : 
			GenerateReply(make_unique<ValueRef>(Sds::create("false", 5), nullptr));
		server.response_queue.push_back({ std::move(reply), conn });
	}
	else
	{
		auto reply = GenerateErrorReply("nil");
		server.response_queue.push_back({ std::move(reply), conn });
	}
}