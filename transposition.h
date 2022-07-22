#pragma once 

#include <functional>
#include <optional>

#include "zobrist.h"
#include "utils/types.h"

struct TTItem {
    ZobristKey key;
    NodeType type;
    int score;

    TTItem() : key(0), type(EXACT), score(0) {}
    TTItem(ZobristKey k, NodeType t, int s) : key(k), type(t), score(s) {}
};

struct Bucket {
    TTItem *item;
    bool empty { false };
};

class TT {
public:

    TT()
    {
        rehash(256);
    }
   
    void insert(ZobristKey key, NodeType type, int score);
    TTItem* get(ZobristKey hash) const;

private:
    Bucket* buckets { NULL };
    // The number of non empty buckets
    size_t num_items { 0 };
    // The number of available spaces for buckets
    size_t m_capacity { 0 };

    void rehash(size_t new_capacity);
};
