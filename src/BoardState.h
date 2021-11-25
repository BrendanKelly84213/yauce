#ifndef BOARDSTATE_H
#define BOARDSTATE_H 

#include <string> 

#include "types.h"
#include <iostream>


class BoardState
{
    public: 
        Piece squares[64];
        Colour side_to_move; 
        bool w_castle_ks = false; 
        bool w_castle_qs = false; 
        bool b_castle_ks = false; 
        bool b_castle_qs = false; 
        int ep_file;
        int halfmove_clock;
        int ply_count; 

        void init(std::string fen);
};

#endif
