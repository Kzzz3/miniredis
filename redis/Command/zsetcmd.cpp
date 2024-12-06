#include "command.h"
#include "../server.h"

void CmdZAdd(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size < 4 || size % 2 != 0)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (!db->kvstore.contains(cmd[1]))
	{
		db->kvstore[cmd[1]] = CreateZsetObject();
	}

	RedisObj* zsetobj = db->kvstore[cmd[1]];
	for (size_t i = 2; i < size; i += 2)
	{
		ZsetObjAdd(zsetobj, sds2num<double>(cmd[i]).value(), cmd[i + 1]);
	}
}

void CmdZRem(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size < 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		RedisObj* zsetobj = db->kvstore[cmd[1]];
		for (size_t i = 2; i < size; i++)
			ZsetObjRemove(zsetobj, cmd[i]);
	}
	else
	{
		auto reply = GenerateErrorReply("nil");
		server.response_queue.push_back({ std::move(reply), conn });
	}
}

void CmdZRange(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size != 4)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto reply = GenerateReply(ZsetObjRange(db->kvstore[cmd[1]], sds2num<double>(cmd[2]).value(), sds2num<double>(cmd[3]).value()));
		server.response_queue.push_back({ std::move(reply), conn });
	}
	else
	{
		auto reply = GenerateErrorReply("nil");
		server.response_queue.push_back({ std::move(reply), conn });
	}
}

void CmdZRevRange(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size != 4)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto reply = GenerateReply(ZsetObjRevRange(db->kvstore[cmd[1]], sds2num<double>(cmd[2]).value(), sds2num<double>(cmd[3]).value()));
		server.response_queue.push_back({ std::move(reply), conn });
	}
	else
	{
		auto reply = GenerateErrorReply("nil");
		server.response_queue.push_back({ std::move(reply), conn });
	}
}
