#include "Game.h"
#include "Pawn.h"
#include "Queen.h"
#include "Rook.h"
#include "Knight.h"
#include "Bishop.h"
#include <optional>    // Graphic needed for SFML 3 event polling
using namespace std;

Game::Game()
    : currentTurn(WHITE),
    gameOver(false),
    winner(WHITE),
    selectedRow(-1),              // -1 = nothing selected yet
    selectedCol(-1),
    isInCheck(false),
    awaitingPromotion(false),
    promotionRow(-1),
    promotionCol(-1),
    promotionColor(WHITE),

    // Create the OS window.

    window(sf::VideoMode(sf::Vector2u(BOARD_PIXELS, BOARD_PIXELS)),
        "Chess - CS1004 OOP Project"),

    fontLoaded(false)
{

    window.setFramerateLimit(60);

    if (font.openFromFile("C:/Windows/Fonts/seguisym.ttf")) fontLoaded = true;
    else if (font.openFromFile("C:/Windows/Fonts/arial.ttf"))    fontLoaded = true;
    else if (font.openFromFile("arial.ttf"))                     fontLoaded = true;
}

void Game::drawSquare(int row, int col) {
    bool isLightSquare = ((row + col) % 2 == 0);
    sf::Color tileColor = isLightSquare
        ? sf::Color(232, 220, 202)   // warm beige  (light squares)
        : sf::Color(120, 90, 70);  // dark brown  (dark squares)

    sf::RectangleShape tileShape(sf::Vector2f((float)TILE_SIZE, (float)TILE_SIZE));
    tileShape.setPosition(sf::Vector2f((float)(col * TILE_SIZE),
        (float)(row * TILE_SIZE)));
    tileShape.setFillColor(tileColor);
    window.draw(tileShape);

    sf::ConvexShape shineTriangle;
    shineTriangle.setPointCount(3);
    shineTriangle.setPoint(0, sf::Vector2f((float)(col * TILE_SIZE),
        (float)(row * TILE_SIZE)));
    shineTriangle.setPoint(1, sf::Vector2f((float)(col * TILE_SIZE + TILE_SIZE),
        (float)(row * TILE_SIZE)));
    shineTriangle.setPoint(2, sf::Vector2f((float)(col * TILE_SIZE),
        (float)(row * TILE_SIZE + TILE_SIZE)));
    shineTriangle.setFillColor(sf::Color(255, 255, 255, isLightSquare ? 35 : 22));
    window.draw(shineTriangle);  // [GRAPHIC]

    if (row == selectedRow && col == selectedCol) {
        sf::RectangleShape selectionBorder(
            sf::Vector2f((float)TILE_SIZE - 6, (float)TILE_SIZE - 6));
        selectionBorder.setPosition(sf::Vector2f((float)(col * TILE_SIZE + 3),
            (float)(row * TILE_SIZE + 3)));
        selectionBorder.setFillColor(sf::Color::Transparent); // hollow inside
        selectionBorder.setOutlineColor(sf::Color(255, 215, 0)); // gold
        selectionBorder.setOutlineThickness(4.0f);
        window.draw(selectionBorder);  // [GRAPHIC]
    }
}


void Game::drawPiece(Piece* piece) {
    if (piece == nullptr) return;   // [LOGIC] empty square — nothing to draw
    if (!fontLoaded)      return;   // [GRAPHIC] no font → can't draw text

    float centerX = piece->getCol() * TILE_SIZE + TILE_SIZE / 2.0f;
    float centerY = piece->getRow() * TILE_SIZE + TILE_SIZE / 2.0f;

    sf::String glyphSymbol = piece->getSymbol();

    sf::Text shadowText(font, glyphSymbol, (unsigned int)(TILE_SIZE * 0.75f));
    shadowText.setFillColor(sf::Color(0, 0, 0, 100)); // black, semi-transparent
    sf::FloatRect shadowBounds = shadowText.getLocalBounds();
    shadowText.setOrigin(sf::Vector2f(
        shadowBounds.position.x + shadowBounds.size.x / 2.0f,
        shadowBounds.position.y + shadowBounds.size.y / 2.0f));
    shadowText.setPosition(sf::Vector2f(centerX + 2, centerY + 3)); // offset = shadow
    window.draw(shadowText);

    sf::Text pieceText(font, glyphSymbol, (unsigned int)(TILE_SIZE * 0.75f));

    if (piece->getColor() == WHITE) {
        pieceText.setFillColor(sf::Color(250, 248, 240));    // near-white fill
        pieceText.setOutlineColor(sf::Color(40, 30, 20)); // dark outline
    }
    else {
        pieceText.setFillColor(sf::Color(35, 25, 20));    // near-black fill
        pieceText.setOutlineColor(sf::Color(220, 215, 200)); // light outline
    }
    pieceText.setOutlineThickness(1.5f);

    sf::FloatRect pieceBounds = pieceText.getLocalBounds();
    pieceText.setOrigin(sf::Vector2f(
        pieceBounds.position.x + pieceBounds.size.x / 2.0f,
        pieceBounds.position.y + pieceBounds.size.y / 2.0f));
    pieceText.setPosition(sf::Vector2f(centerX, centerY));
    window.draw(pieceText);
}


void Game::drawPromotionMenu() {
    if (!awaitingPromotion || !fontLoaded) return;

    // Semi-transparent overlay
    sf::RectangleShape overlay(sf::Vector2f((float)BOARD_PIXELS, (float)BOARD_PIXELS));
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    window.draw(overlay);

    // Menu panel dimensions
    float menuWidth = 360;
    float menuHeight = 90;
    float menuX = (BOARD_PIXELS - menuWidth) / 2;
    float menuY = (BOARD_PIXELS - menuHeight) / 2;

    // Draw menu background
    sf::RectangleShape menuPanel(sf::Vector2f(menuWidth, menuHeight));
    menuPanel.setPosition(sf::Vector2f(menuX, menuY));
    menuPanel.setFillColor(sf::Color(50, 50, 50, 240));
    menuPanel.setOutlineColor(sf::Color(200, 200, 200));
    menuPanel.setOutlineThickness(2.0f);
    window.draw(menuPanel);

    // Title text
    sf::Text title(font, "Promote Pawn To:", 24);
    title.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(sf::Vector2f(titleBounds.size.x / 2, titleBounds.size.y / 2));
    title.setPosition(sf::Vector2f(BOARD_PIXELS / 2, menuY + 25));
    window.draw(title);

    // Draw promotion options (Queen, Rook, Bishop, Knight)
    float optionSize = 60;
    float startX = menuX + (menuWidth - 4 * optionSize) / 2;
    float optionY = menuY + 50;

    // Option 0: Queen
    sf::RectangleShape queenOption(sf::Vector2f(optionSize, optionSize));
    queenOption.setPosition(sf::Vector2f(startX, optionY));
    queenOption.setFillColor(sf::Color(80, 80, 80));
    queenOption.setOutlineColor(sf::Color::White);
    queenOption.setOutlineThickness(1.0f);
    window.draw(queenOption);

    sf::Text queenText(font, sf::String(L"\u2655"), 40);
    queenText.setFillColor(promotionColor == WHITE ? sf::Color(250, 248, 240) : sf::Color(35, 25, 20));
    queenText.setOutlineColor(sf::Color::Black);
    queenText.setOutlineThickness(1.0f);
    sf::FloatRect queenBounds = queenText.getLocalBounds();
    queenText.setOrigin(sf::Vector2f(queenBounds.size.x / 2, queenBounds.size.y / 2));
    queenText.setPosition(sf::Vector2f(startX + optionSize / 2, optionY + optionSize / 2));
    window.draw(queenText);

    // Option 1: Rook
    sf::RectangleShape rookOption(sf::Vector2f(optionSize, optionSize));
    rookOption.setPosition(sf::Vector2f(startX + optionSize, optionY));
    rookOption.setFillColor(sf::Color(80, 80, 80));
    rookOption.setOutlineColor(sf::Color::White);
    rookOption.setOutlineThickness(1.0f);
    window.draw(rookOption);

    sf::Text rookText(font, sf::String(L"\u2656"), 40);
    rookText.setFillColor(promotionColor == WHITE ? sf::Color(250, 248, 240) : sf::Color(35, 25, 20));
    rookText.setOutlineColor(sf::Color::Black);
    rookText.setOutlineThickness(1.0f);
    sf::FloatRect rookBounds = rookText.getLocalBounds();
    rookText.setOrigin(sf::Vector2f(rookBounds.size.x / 2, rookBounds.size.y / 2));
    rookText.setPosition(sf::Vector2f(startX + optionSize + optionSize / 2, optionY + optionSize / 2));
    window.draw(rookText);

    // Option 2: Bishop
    sf::RectangleShape bishopOption(sf::Vector2f(optionSize, optionSize));
    bishopOption.setPosition(sf::Vector2f(startX + 2 * optionSize, optionY));
    bishopOption.setFillColor(sf::Color(80, 80, 80));
    bishopOption.setOutlineColor(sf::Color::White);
    bishopOption.setOutlineThickness(1.0f);
    window.draw(bishopOption);

    sf::Text bishopText(font, sf::String(L"\u2657"), 40);
    bishopText.setFillColor(promotionColor == WHITE ? sf::Color(250, 248, 240) : sf::Color(35, 25, 20));
    bishopText.setOutlineColor(sf::Color::Black);
    bishopText.setOutlineThickness(1.0f);
    sf::FloatRect bishopBounds = bishopText.getLocalBounds();
    bishopText.setOrigin(sf::Vector2f(bishopBounds.size.x / 2, bishopBounds.size.y / 2));
    bishopText.setPosition(sf::Vector2f(startX + 2 * optionSize + optionSize / 2, optionY + optionSize / 2));
    window.draw(bishopText);

    // Option 3: Knight
    sf::RectangleShape knightOption(sf::Vector2f(optionSize, optionSize));
    knightOption.setPosition(sf::Vector2f(startX + 3 * optionSize, optionY));
    knightOption.setFillColor(sf::Color(80, 80, 80));
    knightOption.setOutlineColor(sf::Color::White);
    knightOption.setOutlineThickness(1.0f);
    window.draw(knightOption);

    sf::Text knightText(font, sf::String(L"\u2658"), 40);
    knightText.setFillColor(promotionColor == WHITE ? sf::Color(250, 248, 240) : sf::Color(35, 25, 20));
    knightText.setOutlineColor(sf::Color::Black);
    knightText.setOutlineThickness(1.0f);
    sf::FloatRect knightBounds = knightText.getLocalBounds();
    knightText.setOrigin(sf::Vector2f(knightBounds.size.x / 2, knightBounds.size.y / 2));
    knightText.setPosition(sf::Vector2f(startX + 3 * optionSize + optionSize / 2, optionY + optionSize / 2));
    window.draw(knightText);
}


void Game::drawCheckWarning() {
    if (!isInCheck || gameOver) return;

    // Find the current player's king
    int kingRow = -1, kingCol = -1;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Piece* piece = board.getPiece(row, col);
            if (piece != nullptr && Piece::isKing(piece) && piece->getColor() == currentTurn) {
                kingRow = row;
                kingCol = col;
                break;
            }
        }
    }

    if (kingRow == -1) return;

    static sf::Clock clock;
    float time = clock.getElapsedTime().asSeconds();
    float intensity = 0.5f + 0.5f * sin(time * 8.0f); // Pulsates between 0 and 1
    int alpha = 100 + (int)(100 * intensity); // Between 100 and 200

    sf::RectangleShape checkBorder(
        sf::Vector2f((float)TILE_SIZE - 4, (float)TILE_SIZE - 4));
    checkBorder.setPosition(sf::Vector2f((float)(kingCol * TILE_SIZE + 2),
        (float)(kingRow * TILE_SIZE + 2)));
    checkBorder.setFillColor(sf::Color::Transparent);
    checkBorder.setOutlineColor(sf::Color(255, 50, 50, alpha));
    checkBorder.setOutlineThickness(5.0f);
    window.draw(checkBorder);
}

void Game::drawWinnerOverlay() {
    sf::RectangleShape veil(sf::Vector2f((float)BOARD_PIXELS,
        (float)BOARD_PIXELS));
    veil.setFillColor(sf::Color(0, 0, 0, 180)); // black, 70% opacity
    window.draw(veil);

    if (!fontLoaded) return;

    sf::String winMessage = (winner == WHITE)
        ? sf::String(L"WHITE WINS")
        : sf::String(L"BLACK WINS");

    sf::String subMessage = sf::String(L"CHECKMATE!");

    sf::Text winText(font, winMessage, 70);
    winText.setFillColor(sf::Color(255, 215, 0));    // gold
    winText.setOutlineColor(sf::Color::Black);
    winText.setOutlineThickness(3.0f);

    sf::FloatRect textBounds = winText.getLocalBounds();
    winText.setOrigin(sf::Vector2f(
        textBounds.position.x + textBounds.size.x / 2.0f,
        textBounds.position.y + textBounds.size.y / 2.0f));
    winText.setPosition(sf::Vector2f(BOARD_PIXELS / 2.0f,
        BOARD_PIXELS / 2.0f - 40));
    window.draw(winText);

    sf::Text mateText(font, subMessage, 40);
    mateText.setFillColor(sf::Color(255, 50, 50));    // bright red
    mateText.setOutlineColor(sf::Color::Black);
    mateText.setOutlineThickness(2.0f);

    sf::FloatRect mateBounds = mateText.getLocalBounds();
    mateText.setOrigin(sf::Vector2f(
        mateBounds.position.x + mateBounds.size.x / 2.0f,
        mateBounds.position.y + mateBounds.size.y / 2.0f));
    mateText.setPosition(sf::Vector2f(BOARD_PIXELS / 2.0f,
        BOARD_PIXELS / 2.0f + 40));
    window.draw(mateText);
}

void Game::render() {
    window.clear(sf::Color(20, 18, 16));

    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++)
            drawSquare(row, col);

    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++)
            drawPiece(board.getPiece(row, col));  // [OOP] calls getPiece() getter

    drawCheckWarning();

    if (awaitingPromotion) {
        drawPromotionMenu();
    }

    if (gameOver)
        drawWinnerOverlay();

    window.display();
}


bool Game::wouldBeInCheck(int fromRow, int fromCol, int toRow, int toCol, PieceColor playerColor) {
    Piece* simulatedBoard[8][8] = { nullptr };

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Piece* original = board.getPiece(row, col);
            if (original != nullptr) {
                simulatedBoard[row][col] = original;
            }
        }
    }

    Piece* movingPiece = simulatedBoard[fromRow][fromCol];

    if (movingPiece == nullptr) {
        return true; // No piece to move
    }

    simulatedBoard[toRow][toCol] = movingPiece;
    simulatedBoard[fromRow][fromCol] = nullptr;

    int kingRow = -1, kingCol = -1;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Piece* piece = simulatedBoard[row][col];
            if (piece != nullptr && Piece::isKing(piece) && piece->getColor() == playerColor) {
                kingRow = row;
                kingCol = col;
                break;
            }
        }
    }

    bool kingInCheck = false;
    PieceColor opponentColor = (playerColor == WHITE) ? BLACK : WHITE;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Piece* piece = simulatedBoard[row][col];
            if (piece != nullptr && piece->getColor() == opponentColor) {
                if (piece->isValidMove(kingRow, kingCol, simulatedBoard)) {
                    kingInCheck = true;
                    break;
                }
            }
        }
        if (kingInCheck) break;
    }

    return kingInCheck;
}

bool Game::isSquareAttacked(int row, int col, PieceColor kingColor) {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) return false;

    PieceColor opponentColor = (kingColor == WHITE) ? BLACK : WHITE;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Piece* piece = board.getPiece(r, c);
            if (piece != nullptr && piece->getColor() == opponentColor) {
                if (piece->isValidMove(row, col, board.getGrid())) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Game::hasAnyLegalMove(PieceColor playerColor) {
    for (int fromRow = 0; fromRow < 8; fromRow++) {
        for (int fromCol = 0; fromCol < 8; fromCol++) {
            Piece* piece = board.getPiece(fromRow, fromCol);
            if (piece != nullptr && piece->getColor() == playerColor) {
                // Try all possible destination squares
                for (int toRow = 0; toRow < 8; toRow++) {
                    for (int toCol = 0; toCol < 8; toCol++) {
                        // Check if the move is valid according to piece rules
                        if (piece->isValidMove(toRow, toCol, board.getGrid())) {
                            // Check if this move would put our king in check
                            if (!wouldBeInCheck(fromRow, fromCol, toRow, toCol, playerColor)) {
                                return true; // Found at least one legal move
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool Game::isCheckmate(PieceColor kingColor) {
    int kingRow = -1, kingCol = -1;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Piece* piece = board.getPiece(row, col);
            if (piece != nullptr && Piece::isKing(piece) && piece->getColor() == kingColor) {
                kingRow = row;
                kingCol = col;
                break;
            }
        }
    }

    if (kingRow == -1) return true; // King not found (should not happen)

    if (!isSquareAttacked(kingRow, kingCol, kingColor)) {
        return false;
    }

    return !hasAnyLegalMove(kingColor);
}

void Game::updateCheckStatus() {
    int kingRow = -1, kingCol = -1;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Piece* piece = board.getPiece(row, col);
            if (piece != nullptr && Piece::isKing(piece) && piece->getColor() == currentTurn) {
                kingRow = row;
                kingCol = col;
                break;
            }
        }
    }

    if (kingRow != -1 && kingCol != -1) {
        isInCheck = isSquareAttacked(kingRow, kingCol, currentTurn);
    }
    else {
        isInCheck = false;
    }
}

void Game::checkAndPromotePawn(int row, int col) {
    Piece* piece = board.getPiece(row, col);
    if (piece == nullptr) return;

    Pawn* pawn = dynamic_cast<Pawn*>(piece);
    if (pawn == nullptr) return;

    bool reachedEnd = false;
    if (pawn->getColor() == WHITE && row == 0) {
        reachedEnd = true;
    }
    else if (pawn->getColor() == BLACK && row == 7) {
        reachedEnd = true;
    }

    if (reachedEnd) {
        awaitingPromotion = true;
        promotionRow = row;
        promotionCol = col;
        promotionColor = pawn->getColor();
    }
}

void Game::promotePawn(int row, int col, int pieceType) {
    Piece* oldPawn = board.getPiece(row, col);
    if (oldPawn == nullptr) return;

    Piece* newPiece = nullptr;
    switch (pieceType) {
    case 0: // Queen
        newPiece = new Queen(promotionColor, row, col);
        break;
    case 1: // Rook
        newPiece = new Rook(promotionColor, row, col);
        break;
    case 2: // Bishop
        newPiece = new Bishop(promotionColor, row, col);
        break;
    case 3: // Knight
        newPiece = new Knight(promotionColor, row, col);
        break;
    default: // Default to Queen
        newPiece = new Queen(promotionColor, row, col);
        break;
    }

    // FIX: place the new piece into the grid first, then free the old
    // pawn — the old code deleted oldPawn and *then* called
    // board.movePiece() on the same square, which dereferenced the
    // now-freed pointer (use-after-free / undefined behavior).
    Piece* (*grid)[8] = board.getGrid();
    grid[row][col] = newPiece;
    delete oldPawn;
}

void Game::handlePromotionClick(int mouseX, int mouseY) {
    if (!awaitingPromotion) return;

    float menuWidth = 360;
    float menuHeight = 90;
    float menuX = (BOARD_PIXELS - menuWidth) / 2;
    float menuY = (BOARD_PIXELS - menuHeight) / 2;
    float optionSize = 60;
    float startX = menuX + (menuWidth - 4 * optionSize) / 2;
    float optionY = menuY + 50;

    if (mouseX >= startX && mouseX <= startX + 4 * optionSize &&
        mouseY >= optionY && mouseY <= optionY + optionSize) {

        int optionIndex = (int)((mouseX - startX) / optionSize);
        if (optionIndex >= 0 && optionIndex <= 3) {
            promotePawn(promotionRow, promotionCol, optionIndex);
            awaitingPromotion = false;

            // FIX: the old code flipped currentTurn a second time after
            // the checkmate check, which undid the first flip and left
            // the same player to move again. Now it flips exactly once,
            // matching handleClick()'s normal-move flow.
            if (!gameOver) {
                currentTurn = (currentTurn == WHITE) ? BLACK : WHITE;
                updateCheckStatus();
                if (isCheckmate(currentTurn)) {
                    gameOver = true;
                    winner = (currentTurn == WHITE) ? BLACK : WHITE;
                }
            }
        }
    }
}

void Game::handleClick(int clickedRow, int clickedCol) {
    if (gameOver) return;

    if (selectedRow == -1) {
        Piece* clickedPiece = board.getPiece(clickedRow, clickedCol);

        if (clickedPiece != nullptr &&
            clickedPiece->getColor() == currentTurn) {
            selectedRow = clickedRow;
            selectedCol = clickedCol;
        }
    }
    else {
        if (clickedRow == selectedRow && clickedCol == selectedCol) {
            selectedRow = selectedCol = -1;
        }
        else {
            Piece* movingPiece = board.getPiece(selectedRow, selectedCol);

            if (movingPiece == nullptr) {
                selectedRow = selectedCol = -1;
                return;
            }

            bool moveIsValid = movingPiece->isValidMove(clickedRow, clickedCol, board.getGrid());

            bool wouldBeInCheckAfterMove = false;
            if (moveIsValid) {
                wouldBeInCheckAfterMove = wouldBeInCheck(selectedRow, selectedCol, clickedRow, clickedCol, currentTurn);
            }

            if (moveIsValid && !wouldBeInCheckAfterMove)
            {
                Piece* capturedPiece = board.movePiece(selectedRow, selectedCol,
                    clickedRow, clickedCol);

                if (capturedPiece != nullptr) {
                    if (Piece::isKing(capturedPiece)) {
                        gameOver = true;
                        winner = currentTurn;
                    }
                    delete capturedPiece;
                }

                if (!gameOver) {
                    checkAndPromotePawn(clickedRow, clickedCol);
                }

                if (!gameOver && !awaitingPromotion) {
                    currentTurn = (currentTurn == WHITE) ? BLACK : WHITE;
                    updateCheckStatus();

                    if (isCheckmate(currentTurn)) {
                        gameOver = true;
                        winner = (currentTurn == WHITE) ? BLACK : WHITE;
                    }
                }
            }

            selectedRow = selectedCol = -1;
        }
    }
}


void Game::run() {
    updateCheckStatus();

    while (window.isOpen()) {

        while (const std::optional event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            else if (const auto* mouseEvent =
                event->getIf<sf::Event::MouseButtonPressed>()) {

                if (mouseEvent->button == sf::Mouse::Button::Left) {

                    int mouseX = mouseEvent->position.x;
                    int mouseY = mouseEvent->position.y;

                    if (awaitingPromotion) {
                        handlePromotionClick(mouseX, mouseY);
                    }
                    else {
                        int boardCol = mouseX / TILE_SIZE;
                        int boardRow = mouseY / TILE_SIZE;

                        if (inside(boardRow, boardCol))
                            handleClick(boardRow, boardCol);
                    }
                }
            }
        }

        render();
    }
}
