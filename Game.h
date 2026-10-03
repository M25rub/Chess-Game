#pragma once
// @@@@@@@@@@@@@@@@@@@@@@==========================================
// 
//  OOP Concept: Composition
//    Game HAS-A Board  (board is a member object, not inherited)
//    Game HAS-A sf::RenderWindow  [GRAPHIC]
//    Game HAS-A sf::Font          [GRAPHIC]
//
//  This class is responsible for:
//    - Opening the window                [GRAPHIC]
//    - Drawing the board and pieces      [GRAPHIC]
//    - Handling mouse clicks             [GRAPHIC]
//    - Applying move logic (calls Board) [LOGIC]
//    - Detecting win condition           [LOGIC]
//    - Detecting check/checkmate         [LOGIC]
//    - Handling pawn promotion           [LOGIC]
// 
// ==========================================@@@@@@@@@@@@@@@@@@@@@@

#include "Board.h"
#include <SFML/Graphics.hpp>   // the entire SFML library for Graphics

class Game {
private:
    // Constants
    static const int TILE_SIZE = 90;              // pixels per square
    static const int BOARD_PIXELS = TILE_SIZE * 8; // 720 px total
    Board board;        // the 8x8 board with all pieces
    PieceColor currentTurn;  // whose turn it is: WHITE or BLACK
    bool gameOver;     // true when a King has been captured or checkmate
    PieceColor winner;       // who won (only meaningful if gameOver)

    int selectedRow;  // row of the piece the player clicked first
    int selectedCol;  // col of that piece  (-1 = nothing selected)

    sf::RenderWindow window;  // the OS window that appears on screen
    sf::Font font;    // font used to draw chess piece glyphs
    bool fontLoaded; // true if a usable font was found

    bool isInCheck;  // Track if current player is in check

    // Pawn promotion variables
    bool awaitingPromotion;  // true when waiting for player to choose promotion piece
    int promotionRow;        // row where pawn is being promoted
    int promotionCol;        // col where pawn is being promoted
    PieceColor promotionColor; // color of the pawn being promoted

    void drawSquare(int row, int col);   // draw one board tile
    void drawPiece(Piece* piece);       // draw one chess piece glyph
    void drawWinnerOverlay();            // dark veil + "WHITE WINS" text
    void drawCheckWarning();             // Visual warning for check
    void drawPromotionMenu();            // Draw pawn promotion selection menu
    void render();                       // calls all draw functions

    void handleClick(int row, int col);  // process a player's mouse click
    void handlePromotionClick(int mouseX, int mouseY); // handle promotion menu clicks

    // Check/Checkmate detection methods
    bool isSquareAttacked(int row, int col, PieceColor kingColor);
    bool hasAnyLegalMove(PieceColor playerColor);
    bool isCheckmate(PieceColor kingColor);
    void updateCheckStatus();            // Update check status after each move
    bool wouldBeInCheck(int fromRow, int fromCol, int toRow, int toCol, PieceColor playerColor);

    // Pawn promotion methods
    void checkAndPromotePawn(int row, int col);  // Check if pawn needs promotion
    void promotePawn(int row, int col, int pieceType); // Promote pawn to selected piece

public:
    Game();
    void run();
};
