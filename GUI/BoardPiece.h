#ifndef PIECE_H
#define PIECE_H

#include <string>
#include "../utils/types.h"

const std::string svg_path[] = {
    "assets/piece/png/cburnett/bQ.png",
    "assets/piece/png/cburnett/bK.png",
    "assets/piece/png/cburnett/bR.png",
    "assets/piece/png/cburnett/bN.png",
    "assets/piece/png/cburnett/bB.png",
    "assets/piece/png/cburnett/bP.png",
    "assets/piece/png/cburnett/wQ.png",
    "assets/piece/png/cburnett/wK.png",
    "assets/piece/png/cburnett/wR.png",
    "assets/piece/png/cburnett/wN.png",
    "assets/piece/png/cburnett/wB.png",
    "assets/piece/png/cburnett/wP.png"
};

class BoardPiece {
public:

    BoardPiece() : dragging(false), p(WP), s(a1), texture(NULL), rect({0,0,0,0}) 
    {
    }

    void init(Piece _p, Square _s, int w, SDL_Renderer * renderer);
    void update(Square s, int w);
    void update(int x, int y, int w);
    void update(int w);

    bool is_being_dragged() const { return dragging; }
    bool in_piece(int x, int y) const { return x > rect.x && x < (rect.x + rect.w) && y > rect.y && y < (rect.y + rect.w); }
    Piece get_piece() const { return p; } 
    Square get_square() const { return s; }
    SDL_Texture * get_texture() const { return texture; }
    SDL_Rect get_rect() const { return rect; }

    void toggle_dragging() { dragging = !dragging; }
    void set_dragging(bool _dragging) { dragging = _dragging; }

private:

    bool dragging;
    Piece p;
    Square s;
    SDL_Texture * texture;
    SDL_Rect rect;

    void make_piece_texture(SDL_Renderer * renderer);
};

#endif