#include "linkedlist.h"

void LinkedList::serialize_to(ofstream& ofs, LinkedList* list)
{
    int size = list->size();
    struct_pack::serialize_to(ofs, size);
    for (auto& item : *list)
    {
        Sds::serialize_to(ofs, item);
    }
}

LinkedList* LinkedList::deserialize_from(ifstream& ifs)
{
    auto expect_size = struct_pack::deserialize<int>(ifs);
    if (!expect_size.has_value())
        throw std::runtime_error("deserialize failed");
    int size = expect_size.value();

    LinkedList* list = Allocator::create<LinkedList>();
    for (int i = 0; i < size; i++)
    {
        list->push_back(Sds::deserialize_from(ifs));
    }
    return list;
}