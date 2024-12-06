#include "command.h"
#include "../server.h"

void CmdHSet(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size < 4 || size % 2 != 0)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (!db->kvstore.contains(cmd[1]))
	{
		db->kvstore[cmd[1]] = CreateHashObject();
	}

	RedisObj* hashobj = db->kvstore[cmd[1]];
	for (size_t i = 2; i < size; i += 2)
	{
		HashObjInsert(hashobj, cmd[i], cmd[i + 1]);
	}
}

void CmdHGet(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size < 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		vector<unique_ptr<ValueRef>> result;
		RedisObj* hashobj = db->kvstore[cmd[1]];

		for (size_t i = 2; i < size; i++)
			result.push_back(HashObjGet(hashobj, cmd[i]));

		server.response_queue.emplace_back(GenerateReply(result), conn);

	}
	else
	{
		server.response_queue.emplace_back(GenerateErrorReply("nil"), conn);
	}
}

void CmdHDel(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size < 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		RedisObj* hashobj = db->kvstore[cmd[1]];
		for (size_t i = 2; i < size; i++)
			HashObjRemove(hashobj, cmd[i]);
	}
}

void CmdHKeys(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		RedisObj* hashobj = db->kvstore[cmd[1]];
		vector <unique_ptr<ValueRef>> result = HashObjKeys(hashobj);
		server.response_queue.emplace_back(GenerateReply(result), conn);
	}
	else
	{
		server.response_queue.emplace_back(GenerateErrorReply("nil"), conn);
	}
}

void CmdHGetAll(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		RedisObj* hashobj = db->kvstore[cmd[1]];
		vector <unique_ptr<ValueRef>> result = HashObjKVs(hashobj);
		server.response_queue.emplace_back(GenerateReply(result), conn);
	}
	else
	{
		server.response_queue.emplace_back(GenerateErrorReply("nil"), conn);
	}
}