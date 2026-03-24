#include "Game.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <optional>

enum Cell { EMPTY = 0, SHIP = 1, MISS = 2, HIT = 3 };

Game::Game() 
    : window(sf::VideoMode({1000, 600}), "Sea Battle"), 
      cellSize(45),
      text(font), 
      shadow(font)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    if (!font.openFromFile("Fonts/ArialRegular.ttf")) {
        std::cerr << "Error: ArialRegular.ttf not found!\n";
        exit(1);
    }

    text.setCharacterSize(60);
    shadow.setCharacterSize(60);
    shadow.setFillColor(sf::Color::Black);

    generateField(playerField);
    generateField(enemyField);
}

void Game::run() {
    while (window.isOpen()) {
        processEvents();
        update();
        render();
    }
}

void Game::processEvents() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (!gameOver && playerTurn) {
            if (const auto* mousePress = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePress->button == sf::Mouse::Button::Left) {
                    int x = (mousePress->position.x - 550) / cellSize;
                    int y = (mousePress->position.y - 100) / cellSize;
                    
                    if (x >= 0 && x < SIZE && y >= 0 && y < SIZE) {
                        if (enemyField[x][y] == EMPTY || enemyField[x][y] == SHIP) {
                            handleShot(x, y, enemyField);
                            playerTurn = false;
                        }
                    }
                }
            }
        }

        if (gameOver) {
            if (const auto* keyPress = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPress->code == sf::Keyboard::Key::R) {
                    generateField(playerField);
                    generateField(enemyField);
                    gameOver = false;
                    playerTurn = true;
                }
            }
        }
    }
}

void Game::update() {
    if (!playerTurn && !gameOver) {
        computerMove();
        playerTurn = true;
    }
    checkWin();
}

void Game::handleShot(int x, int y, unsigned char field[SIZE][SIZE]) {
    if (field[x][y] == SHIP) field[x][y] = HIT;
    else if (field[x][y] == EMPTY) field[x][y] = MISS;
}

void Game::checkWin() {
    bool playerAlive = false;
    bool enemyAlive = false;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (playerField[i][j] == SHIP) playerAlive = true;
            if (enemyField[i][j] == SHIP) enemyAlive = true;
        }
    }
    if (!enemyAlive) { gameOver = true; playerWon = true; }
    else if (!playerAlive) { gameOver = true; playerWon = false; }
}

void Game::computerMove() {
    int x, y;
    do {
        x = std::rand() % SIZE;
        y = std::rand() % SIZE;
    } while (playerField[x][y] == MISS || playerField[x][y] == HIT);
    handleShot(x, y, playerField);
}

void Game::generateField(unsigned char field[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            field[i][j] = EMPTY;

    for (int i = 0; i < 15; i++) {
        int x = std::rand() % SIZE;
        int y = std::rand() % SIZE;
        field[x][y] = SHIP;
    }
}

void Game::drawField(unsigned char field[SIZE][SIZE], int offsetX, bool hideShips) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            sf::RectangleShape cell({static_cast<float>(cellSize - 2), static_cast<float>(cellSize - 2)});
            cell.setPosition({static_cast<float>(offsetX + i * cellSize), static_cast<float>(100 + j * cellSize)});

            if (field[i][j] == EMPTY) cell.setFillColor(sf::Color::White);
            else if (field[i][j] == SHIP && !hideShips) cell.setFillColor(sf::Color::Green);
            else if (field[i][j] == MISS) cell.setFillColor(sf::Color::Blue);
            else if (field[i][j] == HIT) cell.setFillColor(sf::Color::Red);

            window.draw(cell);
        }
    }
}

void Game::render() {
    window.clear();

    drawField(playerField, 50, false);

    drawField(enemyField, 550, true);

    if (gameOver) {
        text.setString(playerWon ? "YOU WIN! Press R" : "GAME OVER! Press R");
        shadow.setString(text.getString());

        sf::Vector2f textPos = { (window.getSize().x - text.getLocalBounds().size.x) / 2.f, 250.f };
        text.setPosition(textPos);
        shadow.setPosition(textPos + sf::Vector2f{3.f, 3.f});

        window.draw(shadow);
        window.draw(text);
    }
    window.display();
}