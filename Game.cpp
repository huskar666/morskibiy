#include "Game.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <optional>
#include <algorithm>

using namespace sf;
using namespace std;

enum Klitka { porozhno = 0, korabl = 1, mimo = 2, vlupleno = 3 };

Game::Game() 
    : window(VideoMode({1000, 600}), "Sea Battle"), 
      rozmirKlitky(45),
      text(font), 
      shadow(font)
{
    srand(static_cast<unsigned int>(time(nullptr)));

    if (!font.openFromFile("Fonts/ArialRegular.ttf")) {
        cerr << "Error: ArialRegular.ttf not found!\n";
        exit(1);
    }

    text.setCharacterSize(60);
    shadow.setCharacterSize(60);
    shadow.setFillColor(Color(30, 30, 30));

    zgenPole(poleGrav);
    zgenPole(poleVor);
}

void Game::run() {
    while (window.isOpen()) {
        obrobkaPodiy();
        onovlennya();
        render();
    }
}

void Game::obrobkaPodiy() {
    while (const optional event = window.pollEvent()) {
        if (event->is<Event::Closed>()) {
            window.close();
        }

        if (!kinetsHry && hidGrav) {
            if (const auto* mouse = event->getIf<Event::MouseButtonPressed>()) {
                if (mouse->button == Mouse::Button::Left) {
                    int x = (mouse->position.x - 517) / rozmirKlitky;
                    int y = (mouse->position.y - 100) / rozmirKlitky;

                    if (x >= 0 && x < SIZE && y >= 0 && y < SIZE) {
                        if (poleVor[x][y] == porozhno || poleVor[x][y] == korabl) {
                            postril(x, y, poleVor);
                            hidGrav = false;
                            botTimer.restart();
                        }
                    }
                }
            }
        }

        if (kinetsHry) {
            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (key->code == Keyboard::Key::R) {
                    zgenPole(poleGrav);
                    zgenPole(poleVor);
                    botCili.clear();
                    kinetsHry = false;
                    hidGrav = true;
                }
            }
        }
    }
}

void Game::onovlennya() {
    if (!hidGrav && !kinetsHry) {
        if (botTimer.getElapsedTime().asSeconds() >= 2.0f) {
            hidBota();
            hidGrav = true;
        }
    }
    perevirkaPeremohy();
}

void Game::postril(int x, int y, unsigned char pole[SIZE][SIZE]) {
    if (pole[x][y] == korabl) pole[x][y] = vlupleno;
    else if (pole[x][y] == porozhno) pole[x][y] = mimo;
}

void Game::perevirkaPeremohy() {
    bool gravZhiv = false;
    bool vorZhiv = false;

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (poleGrav[i][j] == korabl) gravZhiv = true;
            if (poleVor[i][j] == korabl) vorZhiv = true;
        }
    }

    if (!vorZhiv) { kinetsHry = true; gravPeremih = true; }
    else if (!gravZhiv) { kinetsHry = true; gravPeremih = false; }
}

void Game::hidBota() {
    int x = -1, y = -1;
    bool valid = false;

    while (!botCili.empty() && !valid) {
        Vector2i t = botCili.back();
        botCili.pop_back();

        if (t.x >= 0 && t.x < SIZE && t.y >= 0 && t.y < SIZE) {
            if (poleGrav[t.x][t.y] == porozhno || poleGrav[t.x][t.y] == korabl) {
                x = t.x;
                y = t.y;
                valid = true;
            }
        }
    }

    if (!valid) {
        do {
            x = rand() % SIZE;
            y = rand() % SIZE;
        } while (poleGrav[x][y] == mimo || poleGrav[x][y] == vlupleno);
    }

    bool hit = (poleGrav[x][y] == korabl);
    postril(x, y, poleGrav);

    if (hit) {
        vector<Vector2i> hits;
        for (int i = 0; i < SIZE; i++)
            for (int j = 0; j < SIZE; j++)
                if (poleGrav[i][j] == vlupleno)
                    hits.push_back({i, j});

        if (hits.size() == 1) {
            botCili.push_back({x + 1, y});
            botCili.push_back({x - 1, y});
            botCili.push_back({x, y + 1});
            botCili.push_back({x, y - 1});
        } 
        else {
            botCili.clear();
            bool horiz = hits[0].y == hits[1].y;
            int minX = 10, maxX = -1, minY = 10, maxY = -1;
            for (auto& h : hits) {
                minX = min(minX, h.x);
                maxX = max(maxX, h.x);
                minY = min(minY, h.y);
                maxY = max(maxY, h.y);
            }
            if (horiz) {
                botCili.push_back({minX - 1, minY});
                botCili.push_back({maxX + 1, minY});
            } else {
                botCili.push_back({minX, minY - 1});
                botCili.push_back({minX, maxY + 1});
            }
        }
    }
}

bool Game::mozhnaKorabl(unsigned char pole[SIZE][SIZE], int x, int y, int dovzh, bool horiz) {
    if (horiz) {
        if (x + dovzh > SIZE) return false;
    } else {
        if (y + dovzh > SIZE) return false;
    }

    int sx = max(0, x - 1);
    int sy = max(0, y - 1);
    int ex = min(SIZE - 1, x + (horiz ? dovzh : 1));
    int ey = min(SIZE - 1, y + (horiz ? 1 : dovzh));

    for (int i = sx; i <= ex; i++)
        for (int j = sy; j <= ey; j++)
            if (pole[i][j] != porozhno)
                return false;

    return true;
}

void Game::postavytyKorabl(unsigned char pole[SIZE][SIZE], int dovzh) {
    bool ok = false;
    while (!ok) {
        int x = rand() % SIZE;
        int y = rand() % SIZE;
        bool horiz = rand() % 2;
        if (mozhnaKorabl(pole, x, y, dovzh, horiz)) {
            for (int i = 0; i < dovzh; i++) {
                if (horiz) pole[x + i][y] = korabl;
                else pole[x][y + i] = korabl;
            }
            ok = true;
        }
    }
}

void Game::zgenPole(unsigned char pole[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            pole[i][j] = porozhno;

    int rozmiary[] = {4,3,3,2,2,2,1,1,1,1};
    for (int d : rozmiary)
        postavytyKorabl(pole, d);
}

void Game::namalyuvatyPole(unsigned char pole[SIZE][SIZE], int zsuVx, bool pryhovaty) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            float posX = static_cast<float>(zsuVx + i * rozmirKlitky);
            float posY = static_cast<float>(100 + j * rozmirKlitky);

            RectangleShape klitka({
                static_cast<float>(rozmirKlitky - 2),
                static_cast<float>(rozmirKlitky - 2)
            });

            klitka.setPosition({posX, posY});

            if (pole[i][j] == porozhno || (pole[i][j] == korabl && pryhovaty)) {
                klitka.setFillColor(Color(255, 255, 255, 200));
            }
            else if (pole[i][j] == korabl && !pryhovaty) {
                klitka.setFillColor(Color(46, 204, 113));
            }
            else if (pole[i][j] == mimo) {
                klitka.setFillColor(Color(52, 152, 219));
            }
            else if (pole[i][j] == vlupleno) {
                klitka.setFillColor(Color(255, 255, 255, 150));
            }

            window.draw(klitka);

            if (pole[i][j] == vlupleno) {
                RectangleShape line1(Vector2f(static_cast<float>(rozmirKlitky) * 1.1f, 6.f));
                line1.setFillColor(Color(231, 76, 60));
                line1.setOrigin({line1.getSize().x / 2.f, line1.getSize().y / 2.f});
                line1.setPosition({posX + (rozmirKlitky - 2) / 2.f, posY + (rozmirKlitky - 2) / 2.f});
                line1.setRotation(degrees(45));

                RectangleShape line2(Vector2f(static_cast<float>(rozmirKlitky) * 1.1f, 6.f));
                line2.setFillColor(Color(231, 76, 60));
                line2.setOrigin({line2.getSize().x / 2.f, line2.getSize().y / 2.f});
                line2.setPosition({posX + (rozmirKlitky - 2) / 2.f, posY + (rozmirKlitky - 2) / 2.f});
                line2.setRotation(degrees(-45));

                window.draw(line1);
                window.draw(line2);
            }
        }
    }
}

void Game::render() {
    window.clear(Color(45, 52, 54));

    namalyuvatyPole(poleGrav, 33, false);
    namalyuvatyPole(poleVor, 517, true);

    if (kinetsHry) {
        text.setString(gravPeremih ? "YOU WIN! Press R" : "GAME OVER! Press R");
        shadow.setString(text.getString());

        Vector2f pos = {
            (window.getSize().x - text.getLocalBounds().size.x) / 2.f,
            250.f
        };

        text.setPosition(pos);
        shadow.setPosition(pos + Vector2f{3.f, 3.f});

        window.draw(shadow);
        window.draw(text);
    }

    window.display();
}