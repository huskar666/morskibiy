#include "Game.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <optional>
#include <algorithm>

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
                            botTimer.restart();
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
                    botTargets.clear();
                    gameOver = false;
                    playerTurn = true;
                }
            }
        }
    }
}

void Game::update() {
    if (!playerTurn && !gameOver) {
        if (botTimer.getElapsedTime().asSeconds() >= 2.0f) {
            computerMove();
            playerTurn = true;
        }
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
    int x = -1, y = -1;
    bool validShot = false;

    while (!botTargets.empty() && !validShot) {
        sf::Vector2i target = botTargets.back();
        botTargets.pop_back();

        if (target.x >= 0 && target.x < SIZE && target.y >= 0 && target.y < SIZE) {
            if (playerField[target.x][target.y] == EMPTY || playerField[target.x][target.y] == SHIP) {
                x = target.x;
                y = target.y;
                validShot = true;
            }
        }
    }

    if (!validShot) {
        do {
            x = std::rand() % SIZE;
            y = std::rand() % SIZE;
        } while (playerField[x][y] == MISS || playerField[x][y] == HIT);
    }

    bool wasHit = (playerField[x][y] == SHIP);
    handleShot(x, y, playerField);

    if (wasHit) {
        std::vector<sf::Vector2i> hits;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (playerField[i][j] == HIT) {
                    hits.push_back({i, j});
                }
            }
        }
                if (hits.size() == 1) {
            botTargets.push_back({x + 1, y});
            botTargets.push_back({x - 1, y});
            botTargets.push_back({x, y + 1});
            botTargets.push_back({x, y - 1});
        } 
        else if (hits.size() > 1) {
            botTargets.clear();
            bool horizontal = hits[0].y == hits[1].y;
            
            int minX = 10, maxX = -1, minY = 10, maxY = -1;
            for(auto& h : hits) {
                minX = std::min(minX, h.x); maxX = std::max(maxX, h.x);
                minY = std::min(minY, h.y); maxY = std::max(maxY, h.y);
            }

            if (horizontal) {
                botTargets.push_back({minX - 1, minY});
                botTargets.push_back({maxX + 1, minY});
            } else {
                botTargets.push_back({minX, minY - 1});
                botTargets.push_back({minX, maxY + 1});
            }
        }
    }
}

bool Game::canPlaceShip(unsigned char field[SIZE][SIZE], int x, int y, int length, bool horizontal) {
    if (horizontal) {
        if (x + length > SIZE) return false;
    } else {
        if (y + length > SIZE) return false;
    }

    int startX = std::max(0, x - 1);
    int startY = std::max(0, y - 1);
    int endX = std::min(SIZE - 1, x + (horizontal ? length : 1));
    int endY = std::min(SIZE - 1, y + (horizontal ? 1 : length));

    for (int i = startX; i <= endX; i++) {
        for (int j = startY; j <= endY; j++) {
            if (field[i][j] != EMPTY) return false;
        }
    }
    return true;
}

void Game::placeShip(unsigned char field[SIZE][SIZE], int length) {
    bool placed = false;
    while (!placed) {
        int x = std::rand() % SIZE;
        int y = std::rand() % SIZE;
        bool horizontal = std::rand() % 2;

        if (canPlaceShip(field, x, y, length, horizontal)) {
            for (int i = 0; i < length; i++) {
                if (horizontal) field[x + i][y] = SHIP;
                else field[x][y + i] = SHIP;
            }
            placed = true;
        }
    }
}

void Game::generateField(unsigned char field[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            field[i][j] = EMPTY;

    int shipSizes[] = {4, 3, 3, 2, 2, 2, 1, 1, 1, 1};
    for (int length : shipSizes) {
        placeShip(field, length);
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