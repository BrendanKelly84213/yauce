#pragma once

#include <bits/stdc++.h>
#include "utils/types.h"


struct Zobrist {
    Bitboard table[12][64];

    void init_table();
    Bitboard compute_hash(); 
};
