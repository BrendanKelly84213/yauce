#include "BoardState.h"

enum Section 
{ 
    Board, 
    SideToMove, 
    CastlingRights, 
    EPSquare, 
    HalfmoveClock, 
    FullmoveCounter 
};

bool is_piece_ch(char ch);
Piece fen_to_piece(char ch);

// TODO: check if fen is properly formatted 
//       add update function

void BoardState::init_squares(std::string fen)
{
    // Parse out pieces 
    int rank=7;
    int file=0;
    int i=0;
    int section=0;
    
    for(int i=a1; i<=h8; ++i) {
        squares[i] = None;
    }

    while(fen[i] != ' ') {
        int sq = (rank)*8 + file;

        if(fen[i] == '/') {
            rank--;
            file=0;
        }

        if(fen[i] >= '0' && fen[i] <= '8') {
            file += (fen[i] - 0x30);
        }

        if(is_piece_ch(fen[i])) {
            squares[sq] = fen_to_piece(fen[i]);
            file++;
        } 

        i++;
    }

    section++;

    // Parse out info
    std::string info = fen.substr(i + 1, fen.length());
    for(int i=0; i<info.length(); ++i) {
        if(info[i] == ' ') {
            section++;
            continue;
        }
        switch(section) {
            case SideToMove: 
                if(info[i] == 'w') {
                    side_to_move = White;
                } else {
                    side_to_move = Black;
                }
                break;
            case CastlingRights:
                switch(info[i]) {
                    case '-':
                        w_castle_ks = false; 
                        w_castle_qs = false; 
                        b_castle_ks = false; 
                        b_castle_qs = false; 
                        break;
                    case 'k':
                        b_castle_ks = true;
                        break;
                    case 'q':
                        b_castle_qs = true;
                        break;
                    case 'K': 
                        w_castle_ks = true;
                        break;
                    case 'Q':
                        w_castle_qs = true;
                        break;
                    default: break;
                }
                break;
            case EPSquare:
                if(info[i] == '-') {
                    ep_file = -1;
                } else if(info[i] >= 'a' && info[i] <= 'h') {
                    ep_file = static_cast<int>(info[i] - 0x61);
                } 
                break;
            case HalfmoveClock:
                halfmove_clock = static_cast<int>(info[i] - 0x30);
                break;
            case FullmoveCounter:
                ply_count = static_cast<int>(info[i] - 0x30);
                break;
            default: break;
        }
    }
}

int piece_to_piecetype[12] = { 
    Queen, King, Rook, Knight, Bishop, Pawn, 
    Queen, King, Rook, Knight, Bishop, Pawn
};

void BoardState::init_bbs() 
{
    for(int s = 0; s <= 64; ++s) {
        int p = (int)squares[s];
        if(p >= 0) { 
            int pt = piece_to_piecetype[p];
            piece_bbs[p] |= (1ULL << s);
            occ |= piece_bbs[p];
            if(p >= 6 && p <= 11) {
                white_piece_bbs[pt] |= (1ULL << s);
                white_occ |= (1ULL << s);
            } else {
                black_piece_bbs[pt] |= (1ULL << s);
                black_occ |= (1ULL << s);
            }
        }
    }
}

void BoardState::init(std::string fen)
{
    init_squares(fen);
    init_bbs(); 
}

void BoardState::make_move(BMove m) 
{
    int from = (m >> 6) & 0x3f;
    int to = m & 0x3f;
    Piece p = squares[from];
    // Squares
    squares[from] = None;
    squares[to] = p;
    // Bitboards
    piece_bbs[p] &= ~(1ULL << from); 
    piece_bbs[p] |= (1ULL << to); 
   
    // Specific occupations
    if(side_to_move == White) {
        white_piece_bbs[p] &= ~(1ULL << from); 
        white_piece_bbs[p] |= (1ULL << to); 
        white_occ &= ~(1ULL << from); 
        white_occ |= (1ULL << to); 
    } else {
        black_piece_bbs[p] &= ~(1ULL << from); 
        black_piece_bbs[p] |= (1ULL << to); 
        black_occ &= ~(1ULL << from); 
        black_occ |= (1ULL << to); 

    }

}

Bitboard BoardState::get_friend_occ()
{
    return side_to_move == White ? white_occ : black_occ;
}

Bitboard BoardState::get_op_occ()
{
    return side_to_move == Black ? white_occ : black_occ;
}

bool is_piece_ch(char ch) 
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

Piece fen_to_piece(char ch) 
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
