#include "command.h"

std::unordered_map<std::string, std::function<void(Connection& conn, Command&)>> commands_map = {
	{"set", CmdSet },
	{ "get", CmdGet },
	{ "hset", CmdHSet },
	{ "hget", CmdHGet },
	{ "lpush", CmdLPush },
	{ "rpush", CmdRPush },
	{ "lpop", CmdLPop },
	{ "rpop", CmdRPop },
	{ "sadd", CmdSAdd },
	{ "smembers", CmdSMembers },
	{ "sismember", CmdSisMember },
	{ "zadd", CmdZAdd }
};

std::function<void(Connection& conn, Command&)> GetCommandHandler(Sds*& cmd)
{
	std::string command = std::string(cmd->buf, cmd->length());
	std::transform(command.begin(), command.end(), command.begin(), ::tolower);
	auto it = commands_map.find(command);
	if (it != commands_map.end())
	{
		return it->second;
	}
	return nullptr;
}

void CmdSet(Connection& conn, Command& cmd)
{
	RedisDb* db = conn.db;
	if (cmd.size() != 3)
	{
		return;
	}

	db->kvstore[cmd[1]].val = CreateStringObject(cmd[2]->buf, cmd[2]->length());
}

void CmdGet(Connection& conn, Command& cmd)
{
	RedisDb* db = conn.db;
	if (cmd.size() != 2)
	{
		return;
	}

	Sds* str = reinterpret_cast<Sds*>(reinterpret_cast<RedisObj*>(db->kvstore[cmd[1]].val)->ptr);
}

void CmdHSet(Connection& conn, Command& cmd)
{
}

void CmdHGet(Connection& conn, Command& cmd)
{
}

void CmdLPush(Connection& conn, Command& cmd)
{
}

void CmdRPush(Connection& conn, Command& cmd)
{
}

void CmdLPop(Connection& conn, Command& cmd)
{
}

void CmdRPop(Connection& conn, Command& cmd)
{
}

void CmdSAdd(Connection& conn, Command& cmd)
{
}

void CmdSMembers(Connection& conn, Command& cmd)
{
}

void CmdSisMember(Connection& conn, Command& cmd)
{
}

void CmdZAdd(Connection& conn, Command& cmd)
{
}