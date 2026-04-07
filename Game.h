#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <vector>
using namespace std;

class Game {
public:
    Game();
    void run();

private:
    void obrobkaPodiy();
    void onovlennya();
    void render();
    
    static const int SIZE =10;
    const int rozmirKlitky;
    
    sf::RenderWindow window;
    sf::Font font;
    sf::Text text;
    sf::Text shadow;

    unsigned char poleGrav[SIZE][SIZE];
    unsigned char poleVor[SIZE][SIZE];

    bool hidGrav = true;
    bool kinetsHry = false;
    bool gravPeremih = false;

    sf::Clock botTimer;
    vector<sf::Vector2i>botCili; 

    void zgenPole(unsigned char pole[SIZE][SIZE]);
    bool mozhnaKorabl(unsigned char pole[SIZE][SIZE], int x, int y, int dovzh, bool horiz);
    void postavytyKorabl(unsigned char pole[SIZE][SIZE],int dovzh);
    void namalyuvatyPole(unsigned char pole[SIZE][SIZE], int zsuVx, bool pryhovaty);
    void postril(int x, int y, unsigned char pole[SIZE][SIZE]);
    void hidBota();
    void perevirkaPeremohy();
};

#endif