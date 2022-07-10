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


struct Search {

    TimePoint search_start;
    bool searching;
    bool ready;
    size_t depth_searched;
    size_t nodes_searched;
    size_t elapsed_time;
    size_t depth;
    size_t movetime;
    size_t nodes;
    bool infinite;

    BMove best_move;
    int score;

    void init(size_t d, size_t mt, size_t ns, bool inf)
    {
        depth = d;
        movetime = mt;
        nodes = ns;
        infinite = inf; 
    }

    void reset() 
    {
        depth_searched = 0;
        nodes_searched = 0;
        depth = 0;
        movetime = 0;
        nodes = 0;
        infinite = false;
    }

    int search(BoardState board, size_t depth, Line * pline);
    void iterative_search(BoardState board);
    void print_line(BoardState board, Line line);
    void print_info(Line pv);
    void stop_search() { searching = false; }

    size_t get_depth_searched() const { return depth_searched; }
    size_t get_nodes_searched() const { return nodes_searched; }

    int alphabeta(
        BoardState board,
        int alpha, 
        int beta, 
        size_t depth,
        Line * pline
    );

    int quiescence(BoardState board, int alpha, int beta, Line * pline);
};

#endif
