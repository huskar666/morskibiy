#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <vector>

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update();
    void render();
    
    static const int SIZE = 10;
    const int cellSize;
    
    sf::RenderWindow window;
    sf::Font font;
    sf::Text text;
    sf::Text shadow;

    unsigned char playerField[SIZE][SIZE];
    unsigned char enemyField[SIZE][SIZE];

    bool playerTurn = true;
    bool gameOver = false;
    bool playerWon = false;

    void generateField(unsigned char field[SIZE][SIZE]);
    void drawField(unsigned char field[SIZE][SIZE], int offsetX, bool hideShips);
    void handleShot(int x, int y, unsigned char field[SIZE][SIZE]);
    void computerMove();
    void checkWin();
};
#endif