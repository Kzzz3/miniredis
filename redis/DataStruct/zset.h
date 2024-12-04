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

class ZSet {
public:
    void add(double score, Sds* member) {

        if (scoremap.contains(member))
            zset.erase({ scoremap[member], member });

        scoremap[member] = score;
        zset.emplace(score, member);
    }

    bool remove(Sds* member) {
        if (!scoremap.contains(member)) 
            return false;

		zset.erase({ scoremap[member], member });
		scoremap.erase(member);
		return true;
    }

    std::optional<size_t> rank(Sds* member) {
        if(!scoremap.contains(member))
			return std::nullopt;

        auto it = zset.find({ scoremap[member], member});
        return std::distance(zset.begin(), it);
    }

    std::optional<std::pair<double, Sds*>> getByRank(size_t rank) {
        if (rank >= zset.size()) 
            return std::nullopt;

        auto it = std::next(zset.begin(), rank);
        return *it;
    }

    std::vector<std::pair<double, Sds*>> rangeByScore(double minScore, double maxScore) {
        std::vector<std::pair<double, Sds*>> result;

        for (auto it = zset.lower_bound({ minScore, nullptr });
            it != zset.end() && it->first <= maxScore; ++it) {
            result.push_back(*it);
        }
        return result;
    }

private:
    std::unordered_map<Sds*, double> scoremap;

    std::multiset<std::pair<double, Sds*>, Compare> zset;
};
