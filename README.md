# Connect N Chess Bot

A UCI chess engine.

## Features

- Search
    - NegaMax
    - Iterative deepening
    - Quiescent search
    - Null move pruning
- Move Ordering
    - Hash move
    - MVV-LVA
    - Killer moves
- Eval
    - Piece square tables
    - King tropism
    - Tapered eval 
    - Huge reward for forming a pawn chain

## Build on *nix

` make `

## Non UCI Commands

` perft <depth> `

` printboard `