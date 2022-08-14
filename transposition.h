#pragma once 
#include <unordered_map>

#include "zobrist.h"
#include "utils/types.h"

struct TTItem {
    ZobristKey key;
    NodeType type;
    int score;
    size_t depth;
    BMove move;

    // TTItem() : key(0), type(EXACT), score(0) {}
    TTItem(ZobristKey k, NodeType t, int s, size_t d, BMove m) 
        : key(k), type(t), score(s), depth(d), move(m) {}
};

struct Bucket {
    TTItem *item { NULL };
    bool empty { true };
};

class TT {
public:
   
    void insert(ZobristKey key, NodeType type, int score, size_t depth, BMove m);
    const TTItem* get(ZobristKey hash) const;
    size_t size() const { return buckets.size(); }
    size_t max_size() const { return buckets.max_size(); }

    void reset();
private:
    std::unordered_map<ZobristKey, TTItem> buckets;
};
