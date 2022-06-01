#ifndef BOARD_STATE_H
#define BOARD_STATE_H

#include <chrono>
#include "BoardState.h"

typedef std::chrono::duration<double> Duration;
typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
/* typedef std::vector<MoveInfo> Line; // Refactor */ 

MoveInfo get_moveinfo(BMove m, BoardState board);

struct MoreMoveInfo : MoveInfo {
    BoardState board; 

    MoreMoveInfo() {}

    MoreMoveInfo(BMove m, BoardState _board)
        : MoveInfo(get_moveinfo(m, _board)), board(_board)
    {
    }
};

struct Line {
    size_t num_moves;
    MoreMoveInfo line[16]; 

    Line() : num_moves(0) {}
};

struct ScoredMove {
    BMove m;
    int score;
    std::string alg;
    MoveList movelist;
};


class Search {
public: 

    Search(double _allotted)
        : allotted(_allotted), searching(false), depth_searched(0), nodes_searched(0)
    {
    }

    int search(BoardState board, size_t depth, Line * pline);
    ScoredMove best_move(BoardState board, size_t depth, Line * pline);
    std::vector<ScoredMove> iterative_search(BoardState board);

    size_t get_depth_searched() const { return depth_searched; }
    size_t get_nodes_searched() const { return nodes_searched; }

private:

    double allotted;
    TimePoint search_start;
    bool searching;
    size_t depth_searched;
    size_t nodes_searched;

    int alphabeta_max(
            BoardState board,
            int alpha, 
            int beta, 
            size_t depth,
            Line * pline
    );

    int alphabeta_min(
            BoardState board,
            int alpha, 
            int beta, 
            size_t depth, 
            Line * pline
    );
};

#endif
