#ifndef BOARD_STATE_H
#define BOARD_STATE_H

#include <chrono>
#include "BoardState.h"

typedef std::chrono::duration<double> Duration;
typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
/* typedef std::vector<MoveInfo> Line; // Refactor */ 

struct Line {
    size_t num_moves;
    BMove line[16]; 

    Line() : num_moves(0) {}
};

struct ScoredMove {
    BMove m;
    int score;
};


class Search {
public: 

    Search(double _allotted)
        : allotted(_allotted), searching(false), depth_searched(0), nodes_searched(0)
    {
    }

    int search(BoardState board, size_t depth, Line * pline);
    void iterative_search(BoardState board);

    size_t get_depth_searched() const { return depth_searched; }
    size_t get_nodes_searched() const { return nodes_searched; }

private:

    double allotted;
    TimePoint search_start;
    bool searching;
    size_t depth_searched;
    size_t nodes_searched;

    int alphabeta(
        BoardState board,
        int alpha, 
        int beta, 
        size_t depth,
        Line * pline,
        bool max
    );
};

#endif
