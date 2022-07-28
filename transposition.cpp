#include "transposition.h"

void TT::insert(ZobristKey key, NodeType type, int score, size_t depth, BMove m)
{
    TTItem item(key, type, score, depth, m);
    auto res = buckets.insert(std::make_pair(key, item));
    if(!res.second)
       res.first->second = item; 
}

const TTItem* TT::get(ZobristKey hash) const 
{
    auto res = buckets.find(hash);
    if(res != buckets.end()) 
        return &buckets.at(hash);
    return NULL;
}

void TT::reset()
{
    buckets.clear();
}
