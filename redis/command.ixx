module;
export module command;

import db;
import sds;
import std;
import redisobj;
import connection;

export using Command = std::vector<sds*>;
using CommandMap = std::unordered_map<std::string, std::function<void(Connection& conn, Command&)>>;

export std::function<void(Connection& conn, Command&)> GetCommandHandler(sds*& cmd);

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