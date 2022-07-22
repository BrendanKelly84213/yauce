#include "transposition.h"

void TT::rehash(size_t new_capacity)
{
    Bucket* old_buckets = buckets;
    buckets = (Bucket*)malloc(sizeof(Bucket) * new_capacity);
    for(size_t i = 0; i < m_capacity; ++i) {
        buckets[i] = old_buckets[i];
    }
    m_capacity = new_capacity;
}

void TT::insert(ZobristKey key, NodeType type, int score)
{
    num_items++;
    size_t index = key % m_capacity;
    if((num_items + 1) >= m_capacity) {
       size_t new_capacity = (num_items + 1) * 2;
       rehash(new_capacity);
    }

    buckets[index].item = new TTItem(key, type, score);
    buckets[index].empty = false;
}

TTItem* TT::get(ZobristKey hash) const 
{
    Bucket bucket = buckets[hash % m_capacity];
    if(!bucket.empty && hash == bucket.item->key)
        return bucket.item;
    return NULL;
}
