#include "server.h"
#include "command.h"

void CmdDel(shared_ptr<Connection> conn, Command& cmd)
{
	if (cmd.size() != 2)
		return;

	RedisDb* db = server.selectDb(cmd[1]);
	if (db->kvstore.contains(cmd[1]))
	{
		auto entry = db->kvstore.find(cmd[1]);

		//recording the key-value pair to be deleted
		Sds* key = entry->first;
		RedisObj* obj = entry->second;

		//delete the key-value pair
		db->kvstore.erase(key);

		//destroy the key and value(object)
		Sds::destroy(key);
		switch (obj->type)
		{
		case ObjType::REDIS_STRING:
			StringObjectDestroy(obj);
			break;
		case ObjType::REDIS_LIST:
			ListObjectDestroy(obj);
			break;
		case ObjType::REDIS_HASH:
			HashObjectDestroy(obj);
			break;
		case ObjType::REDIS_SET:
			SetObjectDestroy(obj);
			break;
		case ObjType::REDIS_ZSET:
			ZsetObjectDestroy(obj);
			break;
		default:
			assert(false);
		}
	}
}