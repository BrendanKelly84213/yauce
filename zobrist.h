#pragma once

#include <bits/stdc++.h>
#include "utils/types.h"

void init_table();
Bitboard compute_hash(); 
Bitboard updated_hash(Bitboard hash, Piece p, Square s); 
