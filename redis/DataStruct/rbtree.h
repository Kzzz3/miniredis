#pragma once
#include <unordered_map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <string_view>

#include "../DataStruct/sds.h"

struct Compare {
    bool operator()(const std::pair<double, Sds*>& a,
        const std::pair<double, Sds*>& b) const {
        return a.first < b.first || (a.first == b.first && a.second == nullptr) ||
                (a.first == b.first && b.second != nullptr&&
                std::string_view(a.second->buf, a.second->length()) < 
                std::string_view(b.second->buf, b.second->length()));
    }
};

class RBTree {
public:

	bool contains(Sds* member);
    void remove(Sds* member);
    void add(double score, Sds* member);
    std::multiset<std::pair<double, Sds*>, Compare>::const_iterator find(Sds* member);

    std::optional<size_t> rank(Sds* member);
    std::optional<std::pair<double, Sds*>> getByRank(int rank);
    std::vector<std::pair<double, Sds*>> rangeByScore(double minScore, double maxScore);

public:
    std::unordered_map<Sds*, double> scoremap;
    std::multiset<std::pair<double, Sds*>, Compare> rbt;
};
