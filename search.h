#ifndef BOARD_STATE_H
#define BOARD_STATE_H

#include <chrono>
#include "transposition.h"
#include "BoardState.h"
#include "eval.h"

typedef std::chrono::duration<double> Duration;
typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

struct ScoredMove {
    BMove m;
    int score;
};

enum SearchVal {
    PV = 35,
    Hash = 34,
    Promotion = 33,
    // Captures 3 ... 32
    KillerMove1 = 2,
    KillerMove2 = 1,
    NonCapture = 0
};

// Pieces in order of value
enum PieceOrder {
    OPawn,
    OKnight,
    OBishop,
    ORook,
    OQueen,
    OKing 
};

constexpr void operator++(PieceOrder& po)
{
    po = (PieceOrder)(static_cast<int>(po) + 1);
}

constexpr void operator--(PieceOrder& po)
{
    po = (PieceOrder)(static_cast<int>(po) - 1);
}

const std::string temp_debug_table[6] = {
    "Pawn",
    "Knight",
    "Bishop",
    "Rook",
    "Queen",
    "King"
};

const PieceOrder ordering_table[6] {
    OQueen,
    OKing,
    ORook,
    OKnight,
    OBishop,
    OPawn
};

struct Search {

    TimePoint search_start;
    bool searching;
    bool ready;
    size_t depth_searched;
    size_t nodes_searched;
    size_t num_transpositions;
    size_t elapsed_time;
    size_t movesleft;
    std::vector<size_t> d_times; // times per depth
    BMove killers[64][2];
    int mvvlva_table[6][6];
    int piece_vals[6]; 
    
             
    TT tt;
    Line pv;
    size_t depth;
    size_t movetime;
    size_t wtime;
    size_t btime;
    size_t winc;
    size_t binc;
    size_t nodes;
    bool infinite;

    BMove best_move;
    int score;

    Search() : movesleft(60) {}

    void update_killers(BMove m, size_t ply)
    {
        killers[ply][1] = killers[ply][0];
        killers[ply][0] = m;
    }

    void init_mvvlva()
    {
        int val = 3;
        for(PieceOrder v = OPawn; v <= OQueen; ++v) {
            if(v != OKing) {
                for(PieceOrder a = OKing; a >= OPawn; --a) {
                    mvvlva_table[v][a] = val++; 
                }
            }
        }
    }

    void init(size_t d, size_t mt, size_t ns, bool inf)
    {
        depth = d;
        movetime = mt;
        nodes = ns;
        infinite = inf; 
    }

    void reset() 
    {
        num_transpositions = 0;
        depth_searched = 0;
        nodes_searched = 0;
        depth = 0;
        d_times = {};
        pv = {};
        movetime = 0;
        wtime = 0;
        btime = 0;
        winc = 0;
        binc = 0;
        nodes = 0;
        infinite = false;
    }

    ScoredMove search(BoardState board, size_t current_depth);
    void iterative_search(const BoardState& board);
    void print_pv(Line line);
    void print_info(BoardState board);
    void stop_search() { searching = false; }

    size_t get_depth_searched() const { return depth_searched; }
    size_t get_nodes_searched() const { return nodes_searched; }

    int q_relative_value(BMove m, const BoardState & board) const;

    int relative_value(BMove move, const BoardState & board, size_t current_depth) const;
    void sort_moves(BMove moves[], size_t num_moves, const BoardState & board, size_t current_depth);

    void print_movelist_vals(BMove moves[], size_t num_moves, const BoardState & board, size_t current_depth) const;

    int alphabeta(
        BoardState board,
        int alpha, 
        int beta, 
        size_t current_depth
    );

    int quiescence(BoardState board, int alpha, int beta, size_t qdepth);
};

#endif
