#include "../Generator.h"
#include "../BoardState.h"
#include "../utils/types.h"
#include "../utils/conversions.h"
#include <iostream>
#include <cassert>
/* #include <bprinter/table_printer.h> */

struct Stats {
    uint64_t nodes = 0;
    uint64_t captures = 0;
    uint64_t ep = 0;
    uint64_t castles = 0;
    uint64_t checks = 0;

    int bad_captures = 0;
    int bad_ep = 0;
    int bad_check = 0;
    int bad_castles = 0;
};

int perft(int depth, BoardState board_state, Stats &stats);
void print_perft(int depth, BoardState board_state);
bool test_in_check(BoardState board_state, Colour us);
