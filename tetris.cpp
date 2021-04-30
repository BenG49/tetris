/*
https://tetris.fandom.com/wiki/TGM_legend
https://tetris.fandom.com/wiki/Drop
https://strategywiki.org/wiki/Tetris/Controls
https://www.sfml-dev.org/tutorials/2.5/window-inputs.php
*/

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <math.h>

#define TILE_SIZE 25
#define WIN_X 480
#define WIN_Y 640

using namespace sf;
using namespace std;

const int keyRepMillis = 100;
const Vector2f spawnVec(3, -1);
const Vector2f boardSize(TILE_SIZE * 10, TILE_SIZE * 20);
const Vector2f boardPos(
    (WIN_X - boardSize.x) / 2,
    (WIN_Y - boardSize.y) / 2);

RectangleShape tile(Vector2f(TILE_SIZE, TILE_SIZE));

enum tetromino { I, O, S, Z, L, J, T, N };

const array<Color, N> COLORS({
    Color(0, 255, 255), // cyan
    Color(255, 255, 0), // yellow
    Color(255, 0, 0),   // red
    Color(0, 255, 0),   // green
    Color(255, 128, 0), // orange
    Color(0, 0, 255),   // blue
    Color(128, 0, 128), // purple
});

// Stores each tetromino in a 4x4 array of pixels, gap on right and bottom if 3x3
const array<array<uint16_t, 4>, N> tetrominos({
    array<uint16_t, 4>({0xf0,   0x4444, 0xf00, 0x2222}), // I
    array<uint16_t, 4>({0x660,  0x660,  0x660, 0x660}),  // O
    array<uint16_t, 4>({0x630,  0x2640, 0x63,  0x4c80}), // Z
    array<uint16_t, 4>({0x6c0,  0x4620, 0x6c,  0x8c40}), // S
    array<uint16_t, 4>({0x2e00, 0x4460, 0xe80, 0xc440}), // L
    array<uint16_t, 4>({0x8e00, 0x6440, 0xe20, 0x44c0}), // J
    array<uint16_t, 4>({0x4e00, 0x4640, 0xe40, 0x4c40})  // T
});

// https://gamedev.stackexchange.com/questions/159835/understanding-tetris-speed-curve
const array<int, 20> frames({48, 43, 38, 33, 28, 23, 18, 13, 8, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2});
const array<int, 20> softFrames({3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});

struct game
{
    int nextTet;
    int currentTet;
    int rotation;
    Vector2f tetPos;

    array<array<int, 10>, 20> tiles;

    int score;
    int level;
};

int parseRot(int rot)
{
    return (rot < 0) ? (4 + rot % 4) : (rot % 4);
}

bool collisionCheck(int rotOffset, int moveOffset, game *g)
{
    uint16_t current = tetrominos[g->currentTet][parseRot(g->rotation + rotOffset)];
    for (int i = 0; i < 16; ++i)
    {
        if (current >> i & 1 == 1)
        {
            int x = i % 4 + g->tetPos.x + moveOffset;
            int y = floor(i / 4) + g->tetPos.y + ((!rotOffset && !moveOffset)?1:0);

            if (y > 20 || x < 0 || x >= 10 || g->tiles[y][x] != N)
                return false;
        }
    }

    return true;
}

void placeTet(game *g)
{
    uint16_t current = tetrominos[g->currentTet][g->rotation];
    for (int i = 0; i < 16; ++i)
        if (current >> i & 1 == 1)
            g->tiles[floor(i / 4) + g->tetPos.y][i % 4 + g->tetPos.x] = g->currentTet;

    g->currentTet = rand() % N;
    g->tetPos = spawnVec;
}

void hardDrop(game *g)
{
    while (collisionCheck(0, 0, g))
        g->tetPos = Vector2f(g->tetPos.x, g->tetPos.y + 1);
    
    placeTet(g);
}

void updateGame(game *g, int *timer, bool softDrop)
{
    if (*timer >= ((softDrop)?softFrames[g->level]:frames[g->level]))
    {
        if (collisionCheck(0, 0, g))
            g->tetPos = Vector2f(g->tetPos.x, g->tetPos.y + 1);
        else
            placeTet(g);

        *timer = 0;
    }
}

void drawGame(game *g, RenderWindow *win, int *timer, bool softDrop)
{
    updateGame(g, timer, softDrop);
    Vector2u size = win->getSize();

    // draw board
    RectangleShape board(boardSize);
    board.setFillColor(Color(0, 0, 0));
    board.setOutlineThickness(2);
    board.setOutlineColor(Color(255, 255, 255));
    board.setPosition(boardPos);

    win->draw(board);

    // draw grid
    RectangleShape line(Vector2f(1, TILE_SIZE * 20));
    line.setFillColor(Color(255, 255, 255, 100));

    for (int x = 1; x < 10; ++x)
    {
        line.setPosition(Vector2f(boardPos.x + x * TILE_SIZE, boardPos.y));
        win->draw(line);
    }

    line.setSize(Vector2f(TILE_SIZE * 10, 1));
    for (int y = 1; y < 20; ++y)
    {
        line.setPosition(Vector2f(boardPos.x, boardPos.y + y * TILE_SIZE));
        win->draw(line);
    }

    // draw tiles
    for (int y = 0; y < g->tiles.size(); ++y)
        for (int x = 0; x < g->tiles[y].size(); ++x)
            if (g->tiles[y][x] != N)
            {
                tile.setFillColor(COLORS[g->tiles[y][x]]);
                tile.setPosition(boardPos + Vector2f(TILE_SIZE * x, TILE_SIZE * y));
                win->draw(tile);
            }

    // draw falling tet
    uint16_t current = tetrominos[g->currentTet][g->rotation];
    for (int i = 0; i < 16; ++i)
    {
        if (current >> i & 1 == 1) // if bit is set
        {
            int x = i % 4 + g->tetPos.x;
            int y = floor(i / 4) + g->tetPos.y;

            tile.setFillColor(COLORS[g->currentTet]);
            tile.setPosition(boardPos + Vector2f(TILE_SIZE * x, TILE_SIZE * y));
            win->draw(tile);
        }
    }
}

int main()
{
    srand(time(NULL)); // seed generator with time

    RenderWindow window(VideoMode(480, 640), "win", Style::Titlebar);
    Vector2u size = window.getSize();
    Clock keyClock;

    tile.setOutlineThickness(0);
    window.setFramerateLimit(60);

    // init array
    array<array<int, 10>, 20> tiles;
    for (int y = 0; y < tiles.size(); ++y)
        for (int x = 0; x < tiles[y].size(); ++x)
            tiles[y][x] = N;

    // game g = { I, L, 0, Vector2f(3, 10), tiles, 0, 0 };
    game g = {I, J, 0, Vector2f(3, 10), tiles, 0, 2};

    bool softDrop = false;
    int timer = 0;

    while (window.isOpen())
    {
        softDrop = false;

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::KeyPressed)
            {
                int code = event.key.code;

            }
            else if (event.type == Event::Closed)
                window.close();
        }

        if (keyClock.getElapsedTime().asMilliseconds() % keyRepMillis == 0)
        {
            if (Keyboard::isKeyPressed(Keyboard::Up) ||
                Keyboard::isKeyPressed(Keyboard::W) ||
                Keyboard::isKeyPressed(Keyboard::X))
            {   // clockwise
                if (collisionCheck(1, 0, &g))
                    g.rotation = parseRot(g.rotation + 1);
            } else if (code == Keyboard::Z || event.key.control)
            {   // counterclockwise
                if (collisionCheck(-1, 0, &g))
                    g.rotation = parseRot(g.rotation - 1);
            } else if (code == Keyboard::Left || code == Keyboard::A)
            {   // move left
                if (collisionCheck(0, -1, &g))
                    g.tetPos = Vector2f(g.tetPos.x - 1, g.tetPos.y);
            } else if (code == Keyboard::Right || code == Keyboard::D)
            {   // move right
                if (collisionCheck(0, 1, &g))
                    g.tetPos = Vector2f(g.tetPos.x + 1, g.tetPos.y);
            } else if (code == Keyboard::Space)
            // hard drop
                hardDrop(&g);
            else if (code == Keyboard::Down || code == Keyboard::S)
                softDrop = true;
        }

        window.clear();

        drawGame(&g, &window, &timer, softDrop);

        window.display();
        ++timer;
    }

    return 0;
}
