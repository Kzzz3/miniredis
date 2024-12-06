#include "command.h"
#include "../server.h"

void CmdLPush(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size < 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (!db->kvstore.contains(cmd[1]))
		db->kvstore[cmd[1]] = CreateListObject();

	for (size_t i = 2; i < size; i++)
		ListObjLPush(db->kvstore[cmd[1]], cmd[i]);
}

void CmdRPush(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size < 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (!db->kvstore.contains(cmd[1]))
		db->kvstore[cmd[1]] = CreateListObject();
	for (size_t i = 2; i < size; i++)
		ListObjRPush(db->kvstore[cmd[1]], cmd[i]);
}

void CmdLPop(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	auto reply = db->kvstore.contains(cmd[1]) ?
		GenerateReply(ListObjLPop(db->kvstore[cmd[1]])) : GenerateErrorReply("nil");
	server.response_queue.push_back({ std::move(reply), conn });
}

void CmdRPop(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	auto reply = db->kvstore.contains(cmd[1]) ?
		GenerateReply(ListObjRPop(db->kvstore[cmd[1]])) : GenerateErrorReply("nil");
	server.response_queue.push_back({ std::move(reply), conn });
}

void CmdLRange(shared_ptr<Connection> conn, Command& cmd)
{
	size_t size = cmd.size();
	if (size != 4)
		return;
	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		RedisObj* obj = db->kvstore[cmd[1]];
		LinkedList& list = *reinterpret_cast<LinkedList*>(obj->data.ptr);

		int start = sds2num<int>(cmd[2]).value();
		int end = sds2num<int>(cmd[3]).value();

		auto reply = GenerateReply(ListObjRange(obj, start, end));
		server.response_queue.push_back({ std::move(reply), conn });
	}
	else
	{
		auto reply = GenerateErrorReply("nil");
		server.response_queue.push_back({ std::move(reply), conn });
	}
}
