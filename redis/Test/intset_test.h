#pragma once

#include <chrono>
#include <cassert>

#include "../DataStruct/intset.h"


inline void test_create_and_destroy() {
    IntSet* is = IntSet::create();
    assert(is->length == 0);
    assert(is->encoding == INTSET_ENC_INT16);
    IntSet::destroy(is);
    std::cout << "test_create_and_destroy passed.\n";
}

inline void test_insert_and_contains() {
    IntSet* is = IntSet::create();

    // ����һЩֵ�����
    is = is->insert(10);
    is = is->insert(5);
    is = is->insert(20);

    assert(is->contains(10));
    assert(is->contains(5));
    assert(is->contains(20));
    assert(!is->contains(15)); // �����ڵ�ֵ

    // ���˳��
    assert(is->get(0) == 5);
    assert(is->get(1) == 10);
    assert(is->get(2) == 20);

    IntSet::destroy(is);
    std::cout << "test_insert_and_contains passed.\n";
}

inline void test_delete() {
    IntSet* is = IntSet::create();

    // ����ֵ
    is = is->insert(10);
    is = is->insert(5);
    is = is->insert(20);

    // ɾ��ֵ�����
    is = is->remove(10);
    assert(!is->contains(10));
    assert(is->contains(5));
    assert(is->contains(20));

    // ���˳��
    assert(is->get(0) == 5);
    assert(is->get(1) == 20);

    // ɾ�������ڵ�ֵ
    is = is->remove(15); // ��Ӧ��������
    assert(is->length == 2);

    IntSet::destroy(is);
    std::cout << "test_delete passed.\n";
}

inline void test_upgrade() {
    IntSet* is = IntSet::create();

    // ���봥������
    is = is->insert(std::numeric_limits<std::int16_t>::max());
    is = is->insert(std::numeric_limits<std::int32_t>::max()); // Ӧ���������� INT32
    assert(is->encoding == INTSET_ENC_INT32);

    is = is->insert(std::numeric_limits<std::int64_t>::max()); // Ӧ���������� INT64
    assert(is->encoding == INTSET_ENC_INT64);

    // ���ֵ�Ƿ���ȷ
    assert(is->contains(std::numeric_limits<std::int16_t>::max()));
    assert(is->contains(std::numeric_limits<std::int32_t>::max()));
    assert(is->contains(std::numeric_limits<std::int64_t>::max()));

    IntSet::destroy(is);
    std::cout << "test_upgrade passed.\n";
}

inline void test_boundary_conditions() {
    IntSet* is = IntSet::create();

    // ���Բ���߽�ֵ
    is = is->insert(std::numeric_limits<std::int16_t>::min());
    is = is->insert(std::numeric_limits<std::int16_t>::max());
    is = is->insert(std::numeric_limits<std::int32_t>::min());
    is = is->insert(std::numeric_limits<std::int32_t>::max());
    is = is->insert(std::numeric_limits<std::int64_t>::min());
    is = is->insert(std::numeric_limits<std::int64_t>::max());

    // ���߽�ֵ
    assert(is->contains(std::numeric_limits<std::int16_t>::min()));
    assert(is->contains(std::numeric_limits<std::int16_t>::max()));
    assert(is->contains(std::numeric_limits<std::int32_t>::min()));
    assert(is->contains(std::numeric_limits<std::int32_t>::max()));
    assert(is->contains(std::numeric_limits<std::int64_t>::min()));
    assert(is->contains(std::numeric_limits<std::int64_t>::max()));

    IntSet::destroy(is);
    std::cout << "test_boundary_conditions passed.\n";
}

inline void test_performance() {
    IntSet* is = IntSet::create();
    constexpr std::size_t NUM_ELEMENTS = 100000;

    using namespace std::chrono;

    // ���Բ�������
    auto start_insert = high_resolution_clock::now();
    for (std::int64_t i = 0; i < NUM_ELEMENTS; ++i) {
        is = is->insert(i);
    }
    auto end_insert = high_resolution_clock::now();
    auto insert_duration = duration_cast<milliseconds>(end_insert - start_insert).count();
    std::cout << "Insertion of " << NUM_ELEMENTS << " elements took " << insert_duration << " ms.\n";

    // ���Բ�ѯ����
    auto start_query = high_resolution_clock::now();
    for (std::int64_t i = 0; i < NUM_ELEMENTS; ++i) {
        assert(is->contains(i));
    }
    auto end_query = high_resolution_clock::now();
    auto query_duration = duration_cast<milliseconds>(end_query - start_query).count();
    std::cout << "Query of " << NUM_ELEMENTS << " elements took " << query_duration << " ms.\n";

    // ����ɾ������
    auto start_remove = high_resolution_clock::now();
    for (int i = NUM_ELEMENTS; i >=0 ; i--) {
        is = is->remove(i);
    }
    auto end_remove = high_resolution_clock::now();
    auto remove_duration = duration_cast<milliseconds>(end_remove - start_remove).count();
    std::cout << "Removal of " << NUM_ELEMENTS << " elements took " << remove_duration << " ms.\n";

    assert(is->length == 0);

    IntSet::destroy(is);

    // �ܺ�ʱ
    auto total_duration = insert_duration + query_duration + remove_duration;
    std::cout << "Total performance test took " << total_duration << " ms.\n";
    std::cout << "test_performance passed.\n";
}

inline void intset_test() {
    test_create_and_destroy();
    test_insert_and_contains();
    test_delete();
    test_upgrade();
    test_boundary_conditions();
    test_performance();
}