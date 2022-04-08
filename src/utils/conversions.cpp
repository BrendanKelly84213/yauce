#include "conversions.h"

std::string square_to_str(int idx)
{
    std::string strs[64] = {  
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", 
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", 
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", 
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", 
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", 
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", 
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", 
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8" 
    };
    if(idx >= a1 && idx <= h8) {
        return strs[idx];
    } 
    if(idx == None)
        return "None";
    return "Invalid Square Input" + std::to_string(idx);
}

std::string piece_to_str(int idx)
{
    std::string strs[12] = {
        "BQ", "BK", "BR", "BN", "BB", "BP", 
        "WQ", "WK", "WR", "WN", "WB", "WP",
    };
    return idx >= 0 ? strs[idx] : "None" ;
}

std::string piecetype_to_str(int idx)
{
    std::string strs[6] = {
        "Queen", "King", "Rook", "Knight", "Bishop", "Pawn"
    };
    return idx >= 0 ? strs[idx] : "None";
}

int piece_to_piecetype(int idx)
{
    int piecetypes[12] = { 
        Queen, King, Rook, Knight, Bishop, Pawn, 
        Queen, King, Rook, Knight, Bishop, Pawn
    };
    return piecetypes[idx];
}

std::string flag_to_str(int idx)
{
    std::string strs[] = {
        "QUIET", "DOUBLE_PAWN_PUSH", "OO", "OOO", "EN_PASSANT" 
    };
    return idx >= 0 ? strs[idx] : "None";
}

