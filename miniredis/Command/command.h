#pragma once
#include <array>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_set>
#include <unordered_map>

#include "db.h"
#include "DataStruct/sds.h"
#include "DataType/setobj.hpp"
#include "DataType/hashobj.hpp"
#include "DataType/listobj.hpp"
#include "DataType/zsetobj.hpp"
#include "DataType/stringobj.hpp"
#include "Networking/connection.h"

class Server;
extern Server server;

using std::array;
using std::weak_ptr;
using std::shared_ptr;
using std::unordered_set;
using Command = std::vector<Sds*>;
using CommandMap =
    std::unordered_map<std::string, std::function<void(shared_ptr<Connection> conn, Command&)>>;

// string command
bool CmdSet(shared_ptr<Connection> conn, Command& cmd);
bool CmdGet(shared_ptr<Connection> conn, Command& cmd);
bool CmdIncr(shared_ptr<Connection> conn, Command& cmd);
bool CmdDecr(shared_ptr<Connection> conn, Command& cmd);
bool CmdAppend(shared_ptr<Connection> conn, Command& cmd);

// hash command
bool CmdHSet(shared_ptr<Connection> conn, Command& cmd);
bool CmdHGet(shared_ptr<Connection> conn, Command& cmd);
bool CmdHDel(shared_ptr<Connection> conn, Command& cmd);
bool CmdHKeys(shared_ptr<Connection> conn, Command& cmd);
bool CmdHGetAll(shared_ptr<Connection> conn, Command& cmd);

// list command
bool CmdLPush(shared_ptr<Connection> conn, Command& cmd);
bool CmdRPush(shared_ptr<Connection> conn, Command& cmd);
bool CmdLPop(shared_ptr<Connection> conn, Command& cmd);
bool CmdRPop(shared_ptr<Connection> conn, Command& cmd);
bool CmdLRange(shared_ptr<Connection> conn, Command& cmd);

// set command
bool CmdSAdd(shared_ptr<Connection> conn, Command& cmd);
bool CmdSRem(shared_ptr<Connection> conn, Command& cmd);
bool CmdSMembers(shared_ptr<Connection> conn, Command& cmd);
bool CmdSisMember(shared_ptr<Connection> conn, Command& cmd);

// zset command
bool CmdZAdd(shared_ptr<Connection> conn, Command& cmd);
bool CmdZRem(shared_ptr<Connection> conn, Command& cmd);
bool CmdZRange(shared_ptr<Connection> conn, Command& cmd);
bool CmdZRevRange(shared_ptr<Connection> conn, Command& cmd);

// free command
bool CmdDel(shared_ptr<Connection> conn, Command& cmd);
bool CmdTTL(shared_ptr<Connection> conn, Command& cmd);
bool CmdExpire(shared_ptr<Connection> conn, Command& cmd);
bool CmdKeyNum(shared_ptr<Connection> conn, Command& cmd);
bool CmdFlushDB(shared_ptr<Connection> conn, Command& cmd);
bool CmdFlushAll(shared_ptr<Connection> conn, Command& cmd);

// assist function
std::function<bool(shared_ptr<Connection> conn, Command&)> GetCommandHandler(Sds* cmdtype);

unique_ptr<Sds, decltype(&Sds::destroy)> GenerateErrorReply(const char* errmsg);
unique_ptr<Sds, decltype(&Sds::destroy)> GenerateReply(unique_ptr<ValueRef>& result);
unique_ptr<Sds, decltype(&Sds::destroy)> GenerateReply(unique_ptr<ValueRef>&& result);
unique_ptr<Sds, decltype(&Sds::destroy)> GenerateReply(vector<unique_ptr<ValueRef>>& result);
unique_ptr<Sds, decltype(&Sds::destroy)> GenerateReply(vector<unique_ptr<ValueRef>>&& result);