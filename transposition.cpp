#include "transposition.h"

void TT::insert(ZobristKey key, const TTItem item)
{
    num_items++;
    size_t index = key % num_items;
    if(index >= m_capacity) {
       size_t new_capacity = index + 1;
       buckets.resize(new_capacity);
       m_capacity = new_capacity;
    }

    buckets[index].put(item);
}

TTItem TT::get(ZobristKey hash)
{
    return buckets[hash % num_items].item;
}
