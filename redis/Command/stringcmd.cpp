#include "command.h"
#include "../server.h"

void CmdSet(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
		db->kvstore[cmd[1]] = UpdateStringObject(db->kvstore[cmd[1]], cmd[2]);
	else
		db->kvstore[cmd[1]] = CreateStringObject(cmd[2]);
}

void CmdGet(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	auto reply = db->kvstore.contains(cmd[1]) ?
		GenerateReply(StringObjGet(db->kvstore[cmd[1]])) : GenerateErrorReply("nil");

	server.response_queue.push_back({ std::move(reply), conn });
}

void CmdDel(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
		db->kvstore.erase(cmd[1]);
}

void CmdIncr(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto obj = db->kvstore[cmd[1]];
		if (obj->encoding == ObjEncoding::REDIS_ENCODING_INT)
			obj->data.num++;
		else
			server.response_queue.push_back({ GenerateErrorReply("value is not an integer"), conn });
	}
	else
	{
		server.response_queue.push_back({ GenerateErrorReply("nil"), conn });
	}
}

void CmdDecr(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto obj = db->kvstore[cmd[1]];
		if (obj->encoding == ObjEncoding::REDIS_ENCODING_INT)
			obj->data.num--;
		else
			server.response_queue.push_back({ GenerateErrorReply("value is not an integer"), conn });
	}
	else
	{
		server.response_queue.push_back({ GenerateErrorReply("nil"), conn });
	}
}

void CmdAppend(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto obj = db->kvstore[cmd[1]];
		if (obj->encoding == ObjEncoding::REDIS_ENCODING_RAW)
		{
			auto value = reinterpret_cast<Sds*>(obj->data.ptr);
			obj->data.ptr = value->append(cmd[2]);
		}
		else
		{
			server.response_queue.push_back({ GenerateErrorReply("WRONGTYPE Operation against a key holding the wrong kind of value"), conn });
		}
	}
	else
	{
		server.response_queue.push_back({ GenerateErrorReply("nil"), conn });
	}
}