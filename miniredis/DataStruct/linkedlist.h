#pragma once
#include <list>
#include <fstream>

#include <ylt/struct_pack.hpp>

#include "sds.h"

using std::list;
using std::ofstream;
using std::ifstream;

class LinkedList : public list<Sds*>
{
public:
    static void serialize_to(ofstream& ofs, LinkedList* list);
    static LinkedList* deserialize_from(ifstream& ifs);
};