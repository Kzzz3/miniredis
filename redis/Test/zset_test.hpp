#pragma once
#include <iostream>
#include <cassert>
#include <chrono>
#include "../DataStruct/zset.h"

// 创建辅助函数
Sds* createSds(const std::string& str) {
    return Sds::create(str.c_str(), str.size(), str.size());
}

inline void testZSetFunctionality() {
    ZSet zset;

    auto member1 = createSds("member1");
    auto member2 = createSds("member2");
    auto member3 = createSds("member3");

    // 测试插入和排名
    zset.add(10, member1);
    zset.add(20, member2);
    assert(zset.rank(member1).value() == 0);
    assert(zset.rank(member2).value() == 1);

    // 测试更新分数
    zset.add(30, member1);
    assert(zset.rank(member1).value() == 1);

    // 测试删除成员
    assert(zset.remove(member1) == true);
    assert(zset.rank(member1) == std::nullopt);

    // 测试根据排名查找
    auto result = zset.getByRank(0);
    assert(result.has_value());
    assert(result->second == member2);

    // 测试范围查找
    zset.add(10, member1);
    zset.add(25, member3);
    auto range = zset.rangeByScore(15, 30);
    assert(range.size() == 2); // member2, member3

}

inline void testZSetEdgeCases() {
    ZSet zset;

    auto member1 = createSds("member1");

    // 空集合操作
    assert(zset.rank(member1) == std::nullopt);
    assert(zset.getByRank(0) == std::nullopt);

    // 删除不存在的成员
    assert(zset.remove(member1) == false);

    // 插入重复成员
    zset.add(10, member1);
    zset.add(10, member1); // 更新
    assert(zset.rank(member1).value() == 0);

    // 范围查找边界
    auto range = zset.rangeByScore(5, 10);
    assert(range.size() == 1); // 包含 member1

}

inline void testZSetPerformance() {
    ZSet zset;

    constexpr size_t NUM_ELEMENTS = 10000;
    std::vector<Sds*> members;

    // 插入大量数据
    for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
        auto member = createSds("member" + std::to_string(i));
        members.push_back(member);
        zset.add(i, member);
    }

    // 测试排名性能
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
        assert(zset.rank(members[i]).value() == i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Rank test completed in "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << " ms\n";

    // 测试范围查找性能
    start = std::chrono::high_resolution_clock::now();
    auto range = zset.rangeByScore(100, 200);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "RangeByScore test completed in "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << " ms, found " << range.size() << " elements\n";

    // 删除数据
    start = std::chrono::high_resolution_clock::now();
    for (auto member : members) {
        zset.remove(member);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Delete test completed in "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << " ms\n";
}

inline void zset_test() {
    std::cout << "Testing ZSet functionality...\n";
    testZSetFunctionality();
    std::cout << "Functionality test passed!\n";

    std::cout << "Testing ZSet edge cases...\n";
    testZSetEdgeCases();
    std::cout << "Edge cases test passed!\n";

    std::cout << "Testing ZSet performance...\n";
    testZSetPerformance();
    std::cout << "Performance test completed!\n";

}
