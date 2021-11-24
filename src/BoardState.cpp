#include "BoardState.h"

bool BoardState::is_piece_ch(char ch) 
{
    return (
            ch == 'p'
         || ch == 'r'
         || ch == 'q'
         || ch == 'k'
         || ch == 'n'
         || ch == 'b'
         || ch == 'P'
         || ch == 'R'
         || ch == 'Q'
         || ch == 'K'
         || ch == 'N'
         || ch == 'B'
    );
}

Piece PlayerView::fen_to_piece(char ch) 
{
    switch(ch) {
        case 'p': 
            return BP;
        case 'r':
            return BR;
        case 'q':
            return BQ;
        case 'k':
            return BK;
        case 'n':
            return BN;
        case 'b':
            return BB;
        case 'P':
            return WP;
        case 'R':    
            return WR;
        case 'Q':    
            return WQ;
        case 'K':    
            return WK;
        case 'N':    
            return WN;
        case 'B':    
            return WB;
        default: break;
    }
    return None;
}
    
char PlayerView::piece_to_char(Piece p) 
{
    switch(p) {
        case BQ:
            return 'q';
        case BK:
            return 'k';
        case BR:
            return 'r';
        case BN:
            return 'n';
        case BB:
            return 'b';
        case BP:
            return 'p';

        case WQ:
            return 'Q';
        case WK:
            return 'K';
        case WR:
            return 'R';
        case WN:
            return 'N';
        case WB:
            return 'B';
        case WP:
            return 'P';

        default: break;

    }
    return ' ';
}

void BoardState::init(std::string fen)
{
    // Parse out pieces 
    int rank=8;
    int file=0;
    int i=0;

    while(fen[i] != ' ') {
        int sq = rank * file;

        if(fen[i] == '/') {
            rank--;
            file=0;
        }

        if(fen[i] >= '0' && fen[i] <= '8') {
            file += (fen[i] - '0');
        }

        if(is_piece_ch(fen[i])) {
            squares[sq] =  

            file++;
        }
        i++;
    }

}
