#include "libs.hpp"
#include "font.hpp"

#define TILE_SIZE 22
#define WIN_X 480
#define WIN_Y 640

const int boardWidth = 10;
const int boardHeight = 20;
const int tetDisplayRot = 1;

const Color gridColor(255, 255, 255, 80);
const Vector2f nextTetPos(10, 1);
const Vector2f heldTetPos(-5, 1);
const Vector2f spawnPos(3, boardHeight);
const Vector2f boardSize(TILE_SIZE * boardWidth, TILE_SIZE * boardHeight);
const Vector2f boardPos(
    (WIN_X - boardSize.x) / 2,
    (WIN_Y - boardSize.y) / 2);
const Vector2f fontPos(
    TILE_SIZE * 0.7,
    TILE_SIZE * 9);
const Vector2f scorePos(
    TILE_SIZE * -5,
    TILE_SIZE * -4);

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
    array<uint16_t, 4>({0xf0, 0x4444, 0xf00, 0x2222}),   // I
    array<uint16_t, 4>({0x660, 0x660, 0x660, 0x660}),    // O
    array<uint16_t, 4>({0x630, 0x2640, 0x63, 0x4c80}),   // Z
    array<uint16_t, 4>({0x6c0, 0x4620, 0x6c, 0x8c40}),   // S
    array<uint16_t, 4>({0x2e00, 0x4460, 0xe80, 0xc440}), // L
    array<uint16_t, 4>({0x8e00, 0x6440, 0xe20, 0x44c0}), // J
    array<uint16_t, 4>({0x4e00, 0x4640, 0xe40, 0x4c40})  // T
});

// https://gamedev.stackexchange.com/questions/159835/understanding-tetris-speed-curve
const array<int, 20> frames({48, 43, 38, 33, 28, 23, 18, 13, 8, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2});
const array<int, 20> softFrames({3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});

RectangleShape tile(Vector2f(TILE_SIZE, TILE_SIZE));

// game vars
int heldTet = N;
int rotation = 0;
Vector2f tetPos = spawnPos;
int score = 0;
int level = 0;
bool usedHeld = false;

int nextTet;
int currentTet;
vector<vector<int>> tiles; // 20x10

bool collisionCheck(int rotOffset, int moveOffset)
{
    uint16_t current = tetrominos[currentTet][(rotation + rotOffset) % 4];
    for (int i = 0; i < 16; ++i)
        if (current >> i & 1 == 1)
        {
            int x = i % 4 + tetPos.x + moveOffset;
            int y = floor(i / 4) + tetPos.y + ((!rotOffset && !moveOffset) ? -1 : 0);
            int ypos = boardHeight - y - 1;

            if (y == 0 || x < 0 || x >= boardWidth || tiles[y][x] != N)
            {
                printf("%d, %d, %d\n", x, y, ypos);
                return false;
            }
        }

    return true;
}

void placeTet()
{
    // add tetrominos to tile
    uint16_t current = tetrominos[currentTet][rotation % 4];
    for (int i = 0; i < 16; ++i)
        if (current >> i & 1 == 1)
            tiles[floor(i / 4) + tetPos.y][i % 4 + tetPos.x] = currentTet;

    // reset tetromino
    currentTet = nextTet;
    nextTet = rand() % N;
    tetPos = spawnPos;
    rotation = 0;
    usedHeld = false;

    int rows = 0;
    // check for full row
    for (int y = 0; y < boardHeight; ++y)
    {
        vector<int> temp;

        for (int x = 0; x < boardWidth; ++x)
            if (tiles[y][x] == N)
                goto cnt; // apparently this is the only way to break in a nested loop

        // erase row and insert empty row at the top
        tiles.erase(tiles.begin() + y);
        for (int x = 0; x < boardWidth; ++x)
            temp.push_back(N);

        tiles.insert(tiles.begin(), temp);
        ++rows;

        cnt:;
    }

    if (rows == 0)
        return;
    else if (rows == 1)
        score += 40 * (level + 1);
    else if (rows == 2)
        score += 100 * (level + 1);
    else if (rows == 3)
        score += 300 * (level + 1);
    else if (rows == 4)
        score += 1200 * (level + 1);
}

void drawTet(int xOffset, int yOffset, int tet, int rot, RenderWindow *win)
{
    uint16_t current = tetrominos[tet][rot % 4];
    for (int i = 0; i < 16; ++i)
    {
        if (current >> i & 1 == 1) // if bit is set
        {
            int x = i % 4 + xOffset;
            int y = floor(i / 4) + yOffset;
            int ypos = boardHeight - y - 1;

            tile.setFillColor(COLORS[tet]);
            tile.setPosition(boardPos + Vector2f(TILE_SIZE * x, TILE_SIZE * ypos));
            win->draw(tile);
        }
    }
}