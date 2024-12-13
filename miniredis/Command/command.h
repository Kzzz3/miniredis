#pragma once
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "DataStruct/sds.h"
#include "DataType/hashobj.hpp"
#include "DataType/listobj.hpp"
#include "DataType/setobj.hpp"
#include "DataType/stringobj.hpp"
#include "DataType/zsetobj.hpp"
#include "Networking/connection.h"
#include "db.h"

class Server;
extern Server server;

using std::shared_ptr;
using std::weak_ptr;
using Command = std::vector<Sds*>;
using CommandMap =
    std::unordered_map<std::string, std::function<void(shared_ptr<Connection> conn, Command&)>>;

std::function<void(shared_ptr<Connection> conn, Command&)> GetCommandHandler(Sds* cmdtype);

// string command
void CmdSet(shared_ptr<Connection> conn, Command& cmd);
void CmdGet(shared_ptr<Connection> conn, Command& cmd);
void CmdIncr(shared_ptr<Connection> conn, Command& cmd);
void CmdDecr(shared_ptr<Connection> conn, Command& cmd);
void CmdAppend(shared_ptr<Connection> conn, Command& cmd);

// hash command
void CmdHSet(shared_ptr<Connection> conn, Command& cmd);
void CmdHGet(shared_ptr<Connection> conn, Command& cmd);
void CmdHDel(shared_ptr<Connection> conn, Command& cmd);
void CmdHKeys(shared_ptr<Connection> conn, Command& cmd);
void CmdHGetAll(shared_ptr<Connection> conn, Command& cmd);

// list command
void CmdLPush(shared_ptr<Connection> conn, Command& cmd);
void CmdRPush(shared_ptr<Connection> conn, Command& cmd);
void CmdLPop(shared_ptr<Connection> conn, Command& cmd);
void CmdRPop(shared_ptr<Connection> conn, Command& cmd);
void CmdLRange(shared_ptr<Connection> conn, Command& cmd);

// set command
void CmdSAdd(shared_ptr<Connection> conn, Command& cmd);
void CmdSRem(shared_ptr<Connection> conn, Command& cmd);
void CmdSMembers(shared_ptr<Connection> conn, Command& cmd);
void CmdSisMember(shared_ptr<Connection> conn, Command& cmd);

// zset command
void CmdZAdd(shared_ptr<Connection> conn, Command& cmd);
void CmdZRem(shared_ptr<Connection> conn, Command& cmd);
void CmdZRange(shared_ptr<Connection> conn, Command& cmd);
void CmdZRevRange(shared_ptr<Connection> conn, Command& cmd);

// free command
void CmdDel(shared_ptr<Connection> conn, Command& cmd);
void CmdTTL(shared_ptr<Connection> conn, Command& cmd);
void CmdExpire(shared_ptr<Connection> conn, Command& cmd);
void CmdKeyNum(shared_ptr<Connection> conn, Command& cmd);
void CmdFlushDB(shared_ptr<Connection> conn, Command& cmd);
void CmdFlushAll(shared_ptr<Connection> conn, Command& cmd);

unique_ptr<Sds, decltype(&Sds::destroy)> GenerateErrorReply(const char* errmsg);
unique_ptr<Sds, decltype(&Sds::destroy)> GenerateReply(unique_ptr<ValueRef>& result);
unique_ptr<Sds, decltype(&Sds::destroy)> GenerateReply(unique_ptr<ValueRef>&& result);
unique_ptr<Sds, decltype(&Sds::destroy)> GenerateReply(vector<unique_ptr<ValueRef>>& result);
unique_ptr<Sds, decltype(&Sds::destroy)> GenerateReply(vector<unique_ptr<ValueRef>>&& result);