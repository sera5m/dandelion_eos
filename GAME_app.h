#ifndef GAME_APP_H
#define GAME_APP_H

typedef enum {
    EMPTY,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
} ChessPiece;

typedef struct __attribute__((packed)) {
    ChessPiece piece;
    bool isPlayerPiece;
    bool isCaptured;
    uint8_t posX;
    uint8_t posY;
} ChessPosition;

#define GOI_AIM_SUBDIVISIONS 32//is how many segments the sky is divided into from 128(screen width) aiming is GOI_AIM_PIXELS,automatically made via 
#define GOI_AIM_PIXELS SCREEN_WIDTH/GOI_AIM_SUBDIVISIONS 



#endif