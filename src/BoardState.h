#ifndef BOARDSTATE_H
#define BOARDSTATE_H 

#include <string> 

#include "types.h"


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
        int ply_count; 

        void init(std::string fen);

    private:
        bool is_piece_ch(char ch);
        Piece fen_to_piece(char ch); 
        char piece_to_char(Piece p);
};

#endif
