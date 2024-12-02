module command;

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

std::function<void(Connection& conn, Command&)> GetCommandHandler(sds*& cmd)
{
	std::string strcmd = std::string(cmd->buf, cmd->length());
	if (commands_map.find(strcmd) != commands_map.end())
	{
		return commands_map[strcmd];
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

	sds* str = reinterpret_cast<sds*>(reinterpret_cast<RedisObj*>(db->kvstore[cmd[1]].val)->ptr);
	conn.buffer->sputn(str->buf, str->length());
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