export module intset_test;

import std;
import intset;
import <cassert>;


void test_create_and_destroy() {
    intset* is = intset::create();
    assert(is->length == 0);
    assert(is->encoding == INTSET_ENC_INT16);
    intset::destroy(is);
    std::cout << "test_create_and_destroy passed.\n";
}

void test_insert_and_contains() {
    intset* is = intset::create();

    // 插入一些值并检查
    is = is->insert(10);
    is = is->insert(5);
    is = is->insert(20);

    assert(is->contains(10));
    assert(is->contains(5));
    assert(is->contains(20));
    assert(!is->contains(15)); // 不存在的值

    // 检查顺序
    assert(is->get(0) == 5);
    assert(is->get(1) == 10);
    assert(is->get(2) == 20);

    intset::destroy(is);
    std::cout << "test_insert_and_contains passed.\n";
}

void test_delete() {
    intset* is = intset::create();

    // 插入值
    is = is->insert(10);
    is = is->insert(5);
    is = is->insert(20);

    // 删除值并检查
    is = is->remove(10);
    assert(!is->contains(10));
    assert(is->contains(5));
    assert(is->contains(20));

    // 检查顺序
    assert(is->get(0) == 5);
    assert(is->get(1) == 20);

    // 删除不存在的值
    is = is->remove(15); // 不应发生错误
    assert(is->length == 2);

    intset::destroy(is);
    std::cout << "test_delete passed.\n";
}

void test_upgrade() {
    intset* is = intset::create();

    // 插入触发升级
    is = is->insert(std::numeric_limits<std::int16_t>::max());
    is = is->insert(std::numeric_limits<std::int32_t>::max()); // 应触发升级到 INT32
    assert(is->encoding == INTSET_ENC_INT32);

    is = is->insert(std::numeric_limits<std::int64_t>::max()); // 应触发升级到 INT64
    assert(is->encoding == INTSET_ENC_INT64);

    // 检查值是否正确
    assert(is->contains(std::numeric_limits<std::int16_t>::max()));
    assert(is->contains(std::numeric_limits<std::int32_t>::max()));
    assert(is->contains(std::numeric_limits<std::int64_t>::max()));

    intset::destroy(is);
    std::cout << "test_upgrade passed.\n";
}

void test_boundary_conditions() {
    intset* is = intset::create();

    // 测试插入边界值
    is = is->insert(std::numeric_limits<std::int16_t>::min());
    is = is->insert(std::numeric_limits<std::int16_t>::max());
    is = is->insert(std::numeric_limits<std::int32_t>::min());
    is = is->insert(std::numeric_limits<std::int32_t>::max());
    is = is->insert(std::numeric_limits<std::int64_t>::min());
    is = is->insert(std::numeric_limits<std::int64_t>::max());

    // 检查边界值
    assert(is->contains(std::numeric_limits<std::int16_t>::min()));
    assert(is->contains(std::numeric_limits<std::int16_t>::max()));
    assert(is->contains(std::numeric_limits<std::int32_t>::min()));
    assert(is->contains(std::numeric_limits<std::int32_t>::max()));
    assert(is->contains(std::numeric_limits<std::int64_t>::min()));
    assert(is->contains(std::numeric_limits<std::int64_t>::max()));

    intset::destroy(is);
    std::cout << "test_boundary_conditions passed.\n";
}

void test_performance() {
    intset* is = intset::create();
    constexpr std::size_t NUM_ELEMENTS = 100000;

    using namespace std::chrono;

    // 测试插入性能
    auto start_insert = high_resolution_clock::now();
    for (std::int64_t i = 0; i < NUM_ELEMENTS; ++i) {
        is = is->insert(i);
    }
    auto end_insert = high_resolution_clock::now();
    auto insert_duration = duration_cast<milliseconds>(end_insert - start_insert).count();
    std::cout << "Insertion of " << NUM_ELEMENTS << " elements took " << insert_duration << " ms.\n";

    // 测试查询性能
    auto start_query = high_resolution_clock::now();
    for (std::int64_t i = 0; i < NUM_ELEMENTS; ++i) {
        assert(is->contains(i));
    }
    auto end_query = high_resolution_clock::now();
    auto query_duration = duration_cast<milliseconds>(end_query - start_query).count();
    std::cout << "Query of " << NUM_ELEMENTS << " elements took " << query_duration << " ms.\n";

    // 测试删除性能
    auto start_remove = high_resolution_clock::now();
    for (int i = NUM_ELEMENTS; i >=0 ; i--) {
        is = is->remove(i);
    }
    auto end_remove = high_resolution_clock::now();
    auto remove_duration = duration_cast<milliseconds>(end_remove - start_remove).count();
    std::cout << "Removal of " << NUM_ELEMENTS << " elements took " << remove_duration << " ms.\n";

    assert(is->length == 0);

    intset::destroy(is);

    // 总耗时
    auto total_duration = insert_duration + query_duration + remove_duration;
    std::cout << "Total performance test took " << total_duration << " ms.\n";
    std::cout << "test_performance passed.\n";
}

export void intset_test() {
    test_create_and_destroy();
    test_insert_and_contains();
    test_delete();
    test_upgrade();
    test_boundary_conditions();
    test_performance();
}
