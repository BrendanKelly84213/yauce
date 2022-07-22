#include "transposition.h"

void TT::insert(ZobristKey key, NodeType type, int score, size_t depth)
{
    num_items++;
    if(num_items / m_capacity * 100 >= 60) {
       size_t new_capacity = (m_capacity) * 2;
       rehash(new_capacity);
    }

    size_t index = key % m_capacity;
    buckets[index].item = new TTItem(key, type, score, depth );
    buckets[index].empty = false;
}

TTItem* TT::get(ZobristKey hash) const 
{
    size_t index = hash % m_capacity;
    Bucket bucket = buckets[index];
    if(!bucket.empty && bucket.item != NULL && hash == bucket.item->key)
        return bucket.item;
    return NULL;
}

void TT::rehash(size_t new_capacity)
{
    Bucket* old_buckets = buckets;
    buckets = (Bucket*)malloc(sizeof(Bucket) * new_capacity);
    for(size_t i = 0; i < m_capacity; ++i) {
        buckets[i] = old_buckets[i];
    }
    m_capacity = new_capacity;
}

