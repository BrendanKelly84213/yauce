#pragma once 

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
    TTItem item;
    bool empty;

    Bucket() : item(), empty(true) {}
    Bucket(TTItem i) : item(i), empty(false) {}

    void put(TTItem i)
    {
        item = i;
        empty = false;
    }
};

class TT {
public:

    std::vector<Bucket> buckets;

    TT() : num_items(0), m_capacity(0) 
    {
    }
   
    void insert(ZobristKey key, TTItem item);
    TTItem get(ZobristKey hash);

private:
    // The number of non empty buckets
    size_t num_items;
    // The number of available spaces for buckets
    size_t m_capacity;
};
