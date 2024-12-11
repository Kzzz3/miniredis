#include "server.h"
#include "command.h"

void CmdZAdd(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size < 4 || size % 2 != 0)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (!db->kvstore.contains(cmd[1]))
	{
		db->kvstore[Sds::create(cmd[1])] = ZsetObjectCreate();
	}

	RedisObj* zsetobj = db->kvstore[cmd[1]];
	for (size_t i = 2; i < size; i += 2)
	{
		ZsetObjectAdd(zsetobj, sds2num<double>(cmd[i]).value(), cmd[i + 1]);
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
			ZsetObjectRemove(zsetobj, cmd[i]);
	}
	else
	{
		auto reply = GenerateErrorReply("nil");
		conn->AsyncSend(std::move(reply));
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
		auto reply = GenerateReply(ZsetObjectRange(db->kvstore[cmd[1]], sds2num<double>(cmd[2]).value(), sds2num<double>(cmd[3]).value()));
		conn->AsyncSend(std::move(reply));
	}
	else
	{
		auto reply = GenerateErrorReply("nil");
		conn->AsyncSend(std::move(reply));
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
		auto reply = GenerateReply(ZsetObjectRevRange(db->kvstore[cmd[1]], sds2num<double>(cmd[2]).value(), sds2num<double>(cmd[3]).value()));
		conn->AsyncSend(std::move(reply));
	}
	else
	{
		auto reply = GenerateErrorReply("nil");
		conn->AsyncSend(std::move(reply));
	}
}
