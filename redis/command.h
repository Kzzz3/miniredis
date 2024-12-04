#pragma once
#include <vector>
#include <functional>
#include <unordered_map>

#include "db.h"
#include "DataStruct/sds.h"
#include "DataType/setobj.hpp"
#include "DataType/zsetobj.hpp"
#include "DataType/hashobj.hpp"
#include "DataType/listobj.hpp"
#include "DataType/stringobj.hpp"
#include "Networking/connection.h"

class Server;
extern Server server;

using Command = std::vector<Sds*>;
using CommandMap = std::unordered_map<std::string, std::function<void(Connection& conn, Command&)>>;

std::function<void(Connection& conn, Command&)> GetCommandHandler(Sds*& cmd);

void CmdSet(Connection& conn, Command& cmd);
void CmdGet(Connection& conn, Command& cmd);

void CmdHSet(Connection& conn, Command& cmd);
void CmdHGet(Connection& conn, Command& cmd);

void CmdLPush(Connection& conn, Command& cmd);
void CmdRPush(Connection& conn, Command& cmd);
void CmdLPop(Connection& conn, Command& cmd);
void CmdRPop(Connection& conn, Command& cmd);

void CmdSAdd(Connection& conn, Command& cmd);
void CmdSMembers(Connection& conn, Command& cmd);
void CmdSisMember(Connection& conn, Command& cmd);

void CmdZAdd(Connection& conn, Command& cmd);
void CmdZRange(Connection& conn, Command& cmd);

Sds* GenerateReply(std::vector<Sds*>& result);
Sds* GenerateErrorReply(Sds* errmsg);