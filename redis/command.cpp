#include "server.h"
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
	{ "zadd", CmdZAdd },
	{ "zrange", CmdZRange }
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
	if (cmd.size() != 3)
		return;
	
	RedisDb* db = server.selectDb(cmd[1]);
	db->kvstore[cmd[1]] = CreateStringObject(cmd[2]->buf, cmd[2]->length());
}

void CmdGet(Connection& conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto obj = db->kvstore[cmd[1]];
		if (obj->type == ObjType::REDIS_STRING)
		{
			Sds* value = nullptr;
			std::vector<Sds*> result;
			if (obj->encoding == ObjEncoding::REDIS_ENCODING_INT)
			{
				std::string temp = std::to_string(obj->data.num);
				value = Sds::create(temp.c_str(), temp.length(), temp.length());
			}
			else
			{
				value = reinterpret_cast<Sds*>(obj->data.ptr);
			}
			result.push_back(value);

			Sds* reply = GenerateReply(result);
			conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code &error,size_t length) {});
		}
		else
		{
			Sds* reply = GenerateErrorReply(Sds::create("WRONGTYPE Operation against a key holding the wrong kind of value", 128, 128));
			conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
		}
	}
	else
	{
		Sds* reply = GenerateErrorReply(Sds::create("nil", 3, 3));
		conn.socket.async_send(asio::buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
	}
}

void CmdHSet(Connection& conn, Command& cmd)
{
	if (cmd.size() != 4)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (!db->kvstore.contains(cmd[1]))
	{
		db->kvstore[cmd[1]] = CreateHashObject();
	}

	Dict& dict = *reinterpret_cast<Dict*>(db->kvstore[cmd[1]]->data.ptr);
	dict[cmd[2]] = CreateStringObject(cmd[3]->buf, cmd[3]->length());
}

void CmdHGet(Connection& conn, Command& cmd)
{
	if (cmd.size() != 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto obj = db->kvstore[cmd[1]];
		if (obj->type == ObjType::REDIS_HASH)
		{
			Dict& dict = *reinterpret_cast<Dict*>(obj->data.ptr);
			if (dict.contains(cmd[2]))
			{
				auto field = dict[cmd[2]];
				Sds* value = nullptr;
				std::vector<Sds*> result;
				if (field->encoding == ObjEncoding::REDIS_ENCODING_INT)
				{
					std::string temp = std::to_string(field->data.num);
					value = Sds::create(temp.c_str(), temp.length(), temp.length());
				}
				else
				{
					value = reinterpret_cast<Sds*>(field->data.ptr);
				}
				result.push_back(value);
				Sds* reply = GenerateReply(result);
				conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
			}
			else
			{
				Sds* reply = GenerateErrorReply(Sds::create("nil", 3, 3));
				conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
			}
		}
		else
		{
			Sds* reply = GenerateErrorReply(Sds::create("WRONGTYPE Operation against a key holding the wrong kind of value", 128, 128));
			conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
		}
	}
	else
	{
		Sds* reply = GenerateErrorReply(Sds::create("nil", 3, 3));
		conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
	}
}

void CmdLPush(Connection& conn, Command& cmd)
{
	if (cmd.size() != 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (!db->kvstore.contains(cmd[1]))
	{
		db->kvstore[cmd[1]] = CreateListObject();
	}

	LinkedList& list = *reinterpret_cast<LinkedList*>(db->kvstore[cmd[1]]->data.ptr);
	list.push_front(cmd[2]);
}

void CmdRPush(Connection& conn, Command& cmd)
{
	if (cmd.size() != 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (!db->kvstore.contains(cmd[1]))
	{
		db->kvstore[cmd[1]] = CreateListObject();
	}

	LinkedList& list = *reinterpret_cast<LinkedList*>(db->kvstore[cmd[1]]->data.ptr);
	list.push_back(cmd[2]);
}

void CmdLPop(Connection& conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto obj = db->kvstore[cmd[1]];
		if (obj->type == ObjType::REDIS_LIST)
		{
			LinkedList& list = *reinterpret_cast<LinkedList*>(obj->data.ptr);
			if (list.size() > 0)
			{
				Sds* value = reinterpret_cast<Sds*>(list.front());
				list.pop_front();

				std::vector<Sds*> result;
				result.push_back(value);

				Sds* reply = GenerateReply(result);
				conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
			}
			else
			{
				Sds* reply = GenerateErrorReply(Sds::create("nil", 3, 3));
				conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
			}
		}
		else
		{
			Sds* reply = GenerateErrorReply(Sds::create("WRONGTYPE Operation against a key holding the wrong kind of value", 128, 128));
			conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
		}
	}
	else
	{
		Sds* reply = GenerateErrorReply(Sds::create("nil", 3, 3));
		conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
	}
}

void CmdRPop(Connection& conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;
	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto obj = db->kvstore[cmd[1]];
		if (obj->type == ObjType::REDIS_LIST)
		{
			LinkedList& list = *reinterpret_cast<LinkedList*>(obj->data.ptr);
			if (list.size() > 0)
			{
				Sds* value = reinterpret_cast<Sds*>(list.back());
				list.pop_back();
				std::vector<Sds*> result;
				result.push_back(value);
				Sds* reply = GenerateReply(result);
				conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
			}
			else
			{
				Sds* reply = GenerateErrorReply(Sds::create("nil", 3, 3));
				conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
			}
		}
		else
		{
			Sds* reply = GenerateErrorReply(Sds::create("WRONGTYPE Operation against a key holding the wrong kind of value", 128, 128));
			conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
		}
	}
	else
	{
		Sds* reply = GenerateErrorReply(Sds::create("nil", 3, 3));
		conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
	}
}

void CmdSAdd(Connection& conn, Command& cmd)
{
	if (cmd.size() < 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (!db->kvstore.contains(cmd[1]))
	{
		db->kvstore[cmd[1]] = CreateSetObject();
	}
	Dict& dict = *reinterpret_cast<Dict*>(db->kvstore[cmd[1]]->data.ptr);
	for (size_t i = 2; i < cmd.size(); i++)
	{
		dict[cmd[i]] = nullptr;
	}
}

void CmdSMembers(Connection& conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;
	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto obj = db->kvstore[cmd[1]];
		if (obj->type == ObjType::REDIS_SET)
		{
			Dict& dict = *reinterpret_cast<Dict*>(obj->data.ptr);
			std::vector<Sds*> result;
			for (auto& [key, _] : dict)
			{
				result.push_back(key);
			}
			Sds* reply = GenerateReply(result);
			conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
		}
		else
		{
			Sds* reply = GenerateErrorReply(Sds::create("WRONGTYPE Operation against a key holding the wrong kind of value", 128, 128));
			conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
		}
	}
	else
	{
		Sds* reply = GenerateErrorReply(Sds::create("nil", 3, 3));
		conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
	}
}

void CmdSisMember(Connection& conn, Command& cmd)
{
	if (cmd.size() != 3)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto obj = db->kvstore[cmd[1]];
		if (obj->type == ObjType::REDIS_SET)
		{
			Sds* exsist = nullptr;
			std::vector<Sds*> result;
			Dict& dict = *reinterpret_cast<Dict*>(obj->data.ptr);
			if (dict.contains(cmd[2]))
			{
				exsist = Sds::create("true", 4, 4);
			}
			else
			{
				exsist = Sds::create("false", 5, 5);
			}
			result.push_back(exsist);
			Sds* reply = GenerateReply(result);
			conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
		}
		else
		{
			Sds* reply = GenerateErrorReply(Sds::create("WRONGTYPE Operation against a key holding the wrong kind of value", 128, 128));
			conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
		}
	}
	else
	{
		Sds* reply = GenerateErrorReply(Sds::create("nil", 3, 3));
		conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
	}
}

void CmdZAdd(Connection& conn, Command& cmd)
{
	if (cmd.size() < 4 || cmd.size() % 2 != 0)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (!db->kvstore.contains(cmd[1]))
	{
		db->kvstore[cmd[1]] = CreateZsetObject();
	}
	ZSet& zset = *reinterpret_cast<ZSet*>(db->kvstore[cmd[1]]->data.ptr);
	for (size_t i = 2; i < cmd.size(); i += 2)
	{
		double score = str2num<double>(cmd[i]->buf, cmd[i]->length()).value();
		zset.add(score, cmd[i + 1]);
	}
}

void CmdZRange(Connection& conn, Command& cmd)
{
	if (cmd.size() != 4)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto obj = db->kvstore[cmd[1]];
		if (obj->type == ObjType::REDIS_ZSET)
		{
			ZSet& zset = *reinterpret_cast<ZSet*>(obj->data.ptr);
			double minScore = str2num<double>(cmd[2]->buf, cmd[2]->length()).value();
			double maxScore = str2num<double>(cmd[3]->buf, cmd[3]->length()).value();
			std::vector<Sds*> result;
			for (auto& [score, member] : zset.rangeByScore(minScore, maxScore))
			{
				std::string temp = std::to_string(score);
				result.push_back(Sds::create(temp.c_str(), temp.length(), temp.length()));
				result.push_back(member);
			}
			Sds* reply = GenerateReply(result);
			conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
		}
		else
		{
			Sds* reply = GenerateErrorReply(Sds::create("WRONGTYPE Operation against a key holding the wrong kind of value", 128, 128));
			conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
		}
	}
	else
	{
		Sds* reply = GenerateErrorReply(Sds::create("nil", 3, 3));
		conn.socket.async_send(asio::const_buffer(reply->buf, reply->length()), [](const asio::error_code& error, size_t length) {});
	}
}

Sds* GenerateReply(std::vector<Sds*>& result)
{
	int size = result.size();
	std::string temp = std::to_string(size);

	Sds* reply = Sds::create("*", 1, 1);
	reply = reply->append(temp.c_str(), temp.length());
	reply = reply->append("\r\n", 2);
	
	for (auto& sds : result)
	{
		temp = std::to_string(sds->length());
		reply = reply->append("$", 1);
		reply = reply->append(temp.c_str(), temp.length());
		reply = reply->append("\r\n", 2);
		reply = reply->append(sds);
		reply = reply->append("\r\n", 2);
	}

	return reply;
}

Sds* GenerateErrorReply(Sds* errmsg)
{
	Sds* reply = Sds::create("-ERR ", 5, 5);
	reply = reply->append(errmsg);
	reply = reply->append("\r\n", 2);
	return reply;
}
