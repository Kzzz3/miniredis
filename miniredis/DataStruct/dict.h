#pragma once

#include <cstdint>
#include <cstring>
#include <functional>
#include <stdexcept>

#include "Utility/allocator.hpp"

// ============================================================================
// High-performance hash table implementation inspired by Redis dict
// Features:
// - Two hash tables for incremental rehashing
// - Power-of-2 sizing for fast modulo
// - Open addressing with linear probing
// - Memory efficient
// ============================================================================

constexpr size_t DICT_INITIAL_SIZE = 4;
constexpr size_t DICT_RESIZE_RATIO = 2;

template <typename K, typename V>
struct DictEntry {
    K key;
    V value;
    DictEntry* next;
};

template <typename K, typename V>
struct DictHashTable {
    DictEntry<K, V>** buckets;
    size_t size;
    size_t sizemask;
    size_t used;
};

template <typename K, typename V, typename Hash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
class Dict {
public:
    Dict() : rehashidx(-1), iterators(0) {
        ht[0] = {nullptr, 0, 0, 0};
        ht[1] = {nullptr, 0, 0, 0};
        expand(DICT_INITIAL_SIZE);
    }

    ~Dict() {
        clear();
        if (ht[0].buckets) {
            Allocator::deallocate(ht[0].buckets, sizeof(DictEntry<K, V>*) * ht[0].size);
        }
        if (ht[1].buckets) {
            Allocator::deallocate(ht[1].buckets, sizeof(DictEntry<K, V>*) * ht[1].size);
        }
    }

    // Disable copy
    Dict(const Dict&) = delete;
    Dict& operator=(const Dict&) = delete;

    // Insert or update
    bool insert(const K& key, const V& value) {
        if (isRehashing()) {
            rehashStep();
        }

        size_t hash = Hash{}(key);
        DictEntry<K, V>* entry = findEntry(key, hash);
        if (entry) {
            entry->value = value;
            return false;  // Updated
        }

        entry = Allocator::create<DictEntry<K, V>>();
        entry->key = key;
        entry->value = value;

        size_t idx = hash & ht[0].sizemask;
        if (isRehashing()) {
            idx = hash & ht[1].sizemask;
            entry->next = ht[1].buckets[idx];
            ht[1].buckets[idx] = entry;
            ht[1].used++;
        } else {
            entry->next = ht[0].buckets[idx];
            ht[0].buckets[idx] = entry;
            ht[0].used++;
        }

        // Check if we need to resize
        if (!isRehashing() && ht[0].used >= ht[0].size) {
            expand(ht[0].size * DICT_RESIZE_RATIO);
        }

        return true;  // Inserted
    }

    // Find
    V* find(const K& key) {
        if (isRehashing()) {
            rehashStep();
        }

        size_t hash = Hash{}(key);
        DictEntry<K, V>* entry = findEntry(key, hash);
        return entry ? &entry->value : nullptr;
    }

    // Erase
    bool erase(const K& key) {
        if (ht[0].used == 0) {
            return false;
        }

        if (isRehashing()) {
            rehashStep();
        }

        size_t hash = Hash{}(key);

        // Try to find and remove from ht[0]
        size_t idx = hash & ht[0].sizemask;
        DictEntry<K, V>* prev = nullptr;
        DictEntry<K, V>* entry = ht[0].buckets[idx];

        while (entry) {
            if (KeyEqual{}(entry->key, key)) {
                if (prev) {
                    prev->next = entry->next;
                } else {
                    ht[0].buckets[idx] = entry->next;
                }
                Allocator::destroy(entry);
                ht[0].used--;
                return true;
            }
            prev = entry;
            entry = entry->next;
        }

        // If rehashing, try ht[1]
        if (isRehashing()) {
            idx = hash & ht[1].sizemask;
            prev = nullptr;
            entry = ht[1].buckets[idx];

            while (entry) {
                if (KeyEqual{}(entry->key, key)) {
                    if (prev) {
                        prev->next = entry->next;
                    } else {
                        ht[1].buckets[idx] = entry->next;
                    }
                    Allocator::destroy(entry);
                    ht[1].used--;
                    return true;
                }
                prev = entry;
                entry = entry->next;
            }
        }

        return false;
    }

    // Contains
    bool contains(const K& key) {
        return find(key) != nullptr;
    }

    // Size
    size_t size() const {
        return ht[0].used + ht[1].used;
    }

    // Empty
    bool empty() const {
        return size() == 0;
    }

    // Clear
    void clear() {
        for (int i = 0; i < 2; i++) {
            if (ht[i].buckets) {
                for (size_t j = 0; j < ht[i].size; j++) {
                    DictEntry<K, V>* entry = ht[i].buckets[j];
                    while (entry) {
                        DictEntry<K, V>* next = entry->next;
                        Allocator::destroy(entry);
                        entry = next;
                    }
                    ht[i].buckets[j] = nullptr;
                }
                ht[i].used = 0;
            }
        }
        rehashidx = -1;
    }

    // Iterator support
    class Iterator {
    public:
        Iterator(Dict* dict, size_t bucket, DictEntry<K, V>* entry)
            : dict(dict), bucket(bucket), entry(entry) {}

        std::pair<const K&, V&> operator*() {
            return {entry->key, entry->value};
        }

        Iterator& operator++() {
            entry = entry->next;
            if (!entry) {
                bucket++;
                while (bucket < dict->ht[0].size && !dict->ht[0].buckets[bucket]) {
                    bucket++;
                }
                if (bucket < dict->ht[0].size) {
                    entry = dict->ht[0].buckets[bucket];
                }
            }
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return entry != other.entry || bucket != other.bucket;
        }

    private:
        Dict* dict;
        size_t bucket;
        DictEntry<K, V>* entry;
    };

    Iterator begin() {
        size_t bucket = 0;
        while (bucket < ht[0].size && !ht[0].buckets[bucket]) {
            bucket++;
        }
        DictEntry<K, V>* entry = bucket < ht[0].size ? ht[0].buckets[bucket] : nullptr;
        return Iterator(this, bucket, entry);
    }

    Iterator end() {
        return Iterator(this, ht[0].size, nullptr);
    }

private:
    DictHashTable<K, V> ht[2];
    int64_t rehashidx;
    int iterators;

    bool isRehashing() const {
        return rehashidx >= 0;
    }

    DictEntry<K, V>* findEntry(const K& key, size_t hash) {
        // Check ht[0]
        size_t idx = hash & ht[0].sizemask;
        DictEntry<K, V>* entry = ht[0].buckets ? ht[0].buckets[idx] : nullptr;
        while (entry) {
            if (KeyEqual{}(entry->key, key)) {
                return entry;
            }
            entry = entry->next;
        }

        // If rehashing, check ht[1]
        if (isRehashing() && ht[1].buckets) {
            idx = hash & ht[1].sizemask;
            entry = ht[1].buckets[idx];
            while (entry) {
                if (KeyEqual{}(entry->key, key)) {
                    return entry;
                }
                entry = entry->next;
            }
        }

        return nullptr;
    }

    bool expand(size_t size) {
        // Ensure power of 2
        size_t realsize = 1;
        while (realsize < size) {
            realsize <<= 1;
        }

        // Don't expand if already rehashing or if size is too small
        if (isRehashing() || (ht[0].buckets && ht[0].size >= realsize)) {
            return false;
        }

        DictHashTable<K, V> newht;
        newht.size = realsize;
        newht.sizemask = realsize - 1;
        newht.used = 0;
        newht.buckets = static_cast<DictEntry<K, V>**>(
            Allocator::allocate(sizeof(DictEntry<K, V>*) * realsize));
        std::memset(newht.buckets, 0, sizeof(DictEntry<K, V>*) * realsize);

        // If this is the first initialization
        if (!ht[0].buckets) {
            ht[0] = newht;
            return true;
        }

        // Prepare for incremental rehashing
        ht[1] = newht;
        rehashidx = 0;
        return true;
    }

    void rehashStep() {
        if (!isRehashing()) {
            return;
        }

        // Perform one step of rehashing
        while (ht[0].used > 0) {
            DictEntry<K, V>* entry = ht[0].buckets[rehashidx];
            if (!entry) {
                rehashidx++;
                if (rehashidx >= static_cast<int64_t>(ht[0].size)) {
                    // Rehashing complete
                    Allocator::deallocate(ht[0].buckets, sizeof(DictEntry<K, V>*) * ht[0].size);
                    ht[0] = ht[1];
                    ht[1] = {nullptr, 0, 0, 0};
                    rehashidx = -1;
                    return;
                }
                continue;
            }

            // Move all entries in this bucket to ht[1]
            while (entry) {
                DictEntry<K, V>* next = entry->next;
                size_t hash = Hash{}(entry->key);
                size_t idx = hash & ht[1].sizemask;
                entry->next = ht[1].buckets[idx];
                ht[1].buckets[idx] = entry;
                ht[1].used++;
                ht[0].used--;
                entry = next;
            }
            ht[0].buckets[rehashidx] = nullptr;
            rehashidx++;
        }

        // Rehashing complete
        Allocator::deallocate(ht[0].buckets, sizeof(DictEntry<K, V>*) * ht[0].size);
        ht[0] = ht[1];
        ht[1] = {nullptr, 0, 0, 0};
        rehashidx = -1;
    }
};
