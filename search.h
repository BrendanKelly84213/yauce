#ifndef BOARD_STATE_H
#define BOARD_STATE_H

#include <chrono>
#include "BoardState.h"

typedef std::chrono::duration<double> Duration;
typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

// typedef std::vector<BMove> Line; // Refactor 
                                 //
                                 
struct Line {
    std::vector<BMove> line;
    bool is_mating;

    Line() : is_mating(true) {}
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
    std::vector<size_t> d_times; // times per depth
             
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

    int search(BoardState board, size_t current_depth);
    void iterative_search(BoardState board);
    void print_pv();
    void print_info();
    void stop_search() { searching = false; }

    size_t get_depth_searched() const { return depth_searched; }
    size_t get_nodes_searched() const { return nodes_searched; }

    int alphabeta(
        BoardState board,
        int alpha, 
        int beta, 
        size_t current_depth
    );

    int quiescence(BoardState board, int alpha, int beta);
};

#endif
