#include "rbtree.h"

bool RBTree::contains(Sds* member)
{
	return scoremap.contains(member);
}

void RBTree::remove(Sds* member)
{
	if (!scoremap.contains(member))
		return;

	rbt.erase({ scoremap[member], member });
	scoremap.erase(member);
}

void RBTree::add(double score, Sds* member)
{

	if (scoremap.contains(member))
		rbt.erase({ scoremap[member], member });

	scoremap[member] = score;
	rbt.emplace(score, member);
}

std::optional<size_t> RBTree::rank(Sds* member)
{
	if (!scoremap.contains(member))
		return std::nullopt;

	auto it = rbt.find({ scoremap[member], member });
	return std::distance(rbt.begin(), it);
}

std::optional<std::pair<double, Sds*>> RBTree::getByRank(size_t rank)
{
	if (rank >= rbt.size())
		return std::nullopt;

	auto it = std::next(rbt.begin(), rank);
	return *it;
}

std::vector<std::pair<double, Sds*>> RBTree::rangeByScore(double minScore, double maxScore)
{
	std::vector<std::pair<double, Sds*>> result;

	for (auto it = rbt.lower_bound({ minScore, nullptr });
		it != rbt.end() && it->first <= maxScore; ++it) {
		result.push_back(*it);
	}
	return result;
}