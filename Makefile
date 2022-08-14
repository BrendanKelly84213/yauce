OBJS = main.cpp transposition.cpp zobrist.cpp test/perft.cpp utils/conversions.cpp utils/bits.cpp BoardState.cpp Generator.cpp eval.cpp search.cpp

CC = g++

COMPILER_FLAGS = -g -Wall -O3

LINKER_FLAGS = -pthread -lpthread

OBJ_NAME = run 

all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

