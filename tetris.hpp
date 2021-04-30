#include <SFML/Graphics.hpp>
#include <math.h>

#define TILE_SIZE 22
#define WIN_X 480
#define WIN_Y 640

using namespace sf;
using namespace std;

const int tetDisplayRot = 1;
const Color gridColor(255, 255, 255, 80);
const Vector2f nextTetPos(10, 1);
const Vector2f heldTetPos(-5, 1);
const Vector2f spawnVec(3, 0);
const Vector2f boardSize(TILE_SIZE * 10, TILE_SIZE * 20);
const Vector2f boardPos(
    (WIN_X - boardSize.x) / 2,
    (WIN_Y - boardSize.y) / 2);
const Vector2f fontPos(
    TILE_SIZE * 3.1,
    TILE_SIZE * 8.8);

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
    array<uint16_t, 4>({0xf0,   0x4444, 0xf00, 0x2222}),   // I
    array<uint16_t, 4>({0x660,  0x660,  0x660, 0x660}),    // O
    array<uint16_t, 4>({0x630,  0x2640, 0x63,  0x4c80}),   // Z
    array<uint16_t, 4>({0x6c0,  0x4620, 0x6c,  0x8c40}),   // S
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
    int heldTet;
    int rotation;
    Vector2f tetPos;

    vector<vector<int>> tiles; // 20x10

    int score;
    int level;

    bool usedHeld;
};

bool collisionCheck(int rotOffset, int moveOffset, game *g)
{
    uint16_t current = tetrominos[g->currentTet][(g->rotation + rotOffset) % 4];
    for (int i = 0; i < 16; ++i)
    {
        if (current >> i & 1 == 1)
        {
            int x = i % 4 + g->tetPos.x + moveOffset;
            int y = floor(i / 4) + g->tetPos.y + ((!rotOffset && !moveOffset) ? 1 : 0);

            if (y >= 20 || x < 0 || x >= 10 || g->tiles[y][x] != N)
                return false;
        }
    }

    return true;
}

void resetTet(game *g)
{
    // reset tetromino
    g->currentTet = g->nextTet;
    g->nextTet = rand() % N;
    g->tetPos = spawnVec;
    g->rotation = 0;
    g->usedHeld = false;
}

void placeTet(game *g)
{
    // add tetrominos to tile
    uint16_t current = tetrominos[g->currentTet][g->rotation % 4];
    for (int i = 0; i < 16; ++i)
        if (current >> i & 1 == 1)
            g->tiles[floor(i / 4) + g->tetPos.y][i % 4 + g->tetPos.x] = g->currentTet;

    resetTet(g);

    // check for full row
    bool fullRow = true;
    for (int y = 0; y < 20; ++y)
    {
        fullRow = true;
        for (int x = 0; x < 10; ++x)
        {
            if (g->tiles[y][x] == N)
            {
                fullRow = false;
                break;
            }
        }

        if (fullRow)
        {
            // erase row and insert empty row at the top
            g->tiles.erase(g->tiles.begin() + y);
            vector<int> temp;
            for (int x = 0; x < 10; ++x)
                temp.push_back(N);
            g->tiles.insert(g->tiles.begin(), temp);
        }
    }
}

void hardDrop(game *g)
{
    // drop until collision
    while (collisionCheck(0, 0, g))
        g->tetPos = Vector2f(g->tetPos.x, g->tetPos.y + 1);

    placeTet(g);
}

void drawTet(int xOffset, int yOffset, int tet, int rot, game *g, RenderWindow *win)
{
    uint16_t current = tetrominos[tet][rot % 4];
    for (int i = 0; i < 16; ++i)
    {
        if (current >> i & 1 == 1) // if bit is set
        {
            int x = i % 4 + xOffset;
            int y = floor(i / 4) + yOffset;

            tile.setFillColor(COLORS[tet]);
            tile.setPosition(boardPos + Vector2f(TILE_SIZE * x, TILE_SIZE * y));
            win->draw(tile);
        }
    }
}