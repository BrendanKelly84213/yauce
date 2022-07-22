#pragma once 

#include <functional>
#include <optional>

#include "zobrist.h"
#include "utils/types.h"

struct TTItem {
    ZobristKey key;
    NodeType type;
    int score;
    size_t depth;

    TTItem() : key(0), type(EXACT), score(0) {}
    TTItem(ZobristKey k, NodeType t, int s, size_t d) 
        : key(k), type(t), score(s), depth(d) {}
};

struct Bucket {
    TTItem *item { NULL };
    bool empty { true };
};

class TT {
public:

    TT()
    {
        rehash(4096);
    }

    ~TT()
    {
        for(size_t i = 0; i < m_capacity; ++i) {
            free(buckets[i].item);
        }
        free(buckets);
        buckets = NULL;
    }
   
    void insert(ZobristKey key, NodeType type, int score, size_t depth);
    TTItem* get(ZobristKey hash) const;

private:
    Bucket* buckets { NULL };
    // The number of non empty buckets
    size_t num_items { 0 };
    // The number of available spaces for buckets
    size_t m_capacity { 0 };

    void rehash(size_t new_capacity);
};
