#include "rbtree.h"

void RBTree::serialize_to(ofstream& ofs, RBTree* rbt)
{
    int size = rbt->scoremap.size();
    struct_pack::serialize_to(ofs, size);
    for (auto& [key, value] : rbt->scoremap)
    {
        Sds::serialize_to(ofs, key);
        struct_pack::serialize_to(ofs, value);
    }
}

RBTree* RBTree::deserialize_from(ifstream& ifs)
{
    RBTree* rbt = Allocator::create<RBTree>();
    auto expect_size = struct_pack::deserialize<int>(ifs);
    if (!expect_size.has_value())
        throw std::runtime_error("deserialize failed");

    int size = expect_size.value();
    for (int i = 0; i < size; i++)
    {
        Sds* key = Sds::deserialize_from(ifs);

        auto expect_score = struct_pack::deserialize<double>(ifs);
        if (!expect_score.has_value())
            throw std::runtime_error("deserialize failed");

        double score = expect_score.value();
        rbt->add(score, key);
    }

    return rbt;
}

bool RBTree::contains(Sds* member)
{
    return scoremap.contains(member);
}

void RBTree::remove(Sds* member)
{
    if (!scoremap.contains(member))
        return;

    rbt.erase({scoremap[member], member});
    scoremap.erase(member);
}

std::multiset<std::pair<double, Sds*>, Compare>::iterator RBTree::find(Sds* member)
{
    return rbt.find({scoremap[member], member});
}

void RBTree::add(double score, Sds* member)
{

    if (scoremap.contains(member))
        rbt.erase({scoremap[member], member});

    scoremap[member] = score;
    rbt.emplace(score, member);
}

std::optional<size_t> RBTree::rank(Sds* member)
{
    if (!scoremap.contains(member))
        return std::nullopt;

    auto it = rbt.find({scoremap[member], member});
    return std::distance(rbt.begin(), it);
}

std::optional<std::pair<double, Sds*>> RBTree::getByRank(int rank)
{
    if (rbt.size() == 0)
        return std::nullopt;

    int size = rbt.size();
    rank = (rank % size + size) % size;
    auto it = std::next(rbt.begin(), rank);
    return *it;
}

std::vector<std::pair<double, Sds*>> RBTree::rangeByScore(double minScore, double maxScore)
{
    std::vector<std::pair<double, Sds*>> result;

    for (auto it = rbt.lower_bound({minScore, nullptr}); it != rbt.end() && it->first <= maxScore;
         ++it)
    {
        result.push_back(*it);
    }
    return result;
}