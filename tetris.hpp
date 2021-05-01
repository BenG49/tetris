#include "libs.hpp"
#include "font.hpp"

constexpr float centerFont(int len, int fontSize, int winWidth)
{
    return (winWidth - fontSize * len) / 2;
}

const Vector2f winSize(480, 700);
const Vector2f boardDim(10, 20);

const int displayRot = 1;
const int tileSize = 22;
const int fontSize = 32;

const int startLevel = 0;

const Color gridColor(255, 255, 255, 80);
const Vector2f boardSize = boardDim * (float)tileSize;
const Vector2f boardPos((winSize.x - boardSize.x) / 2, fontSize * 3);

// positions
const Vector2f pausePos(centerFont(6, fontSize, winSize.x),  // len(PAUSED) = 6
                        centerFont(1, fontSize, winSize.y));
const Vector2f linesPos(centerFont(10, fontSize, winSize.x), // len(LINES: 000) = 10
                        fontSize / 2);
const Vector2f scorePos(centerFont(13, fontSize, winSize.x), // len(SCORE: 000000) = 13
                        winSize.y - fontSize * 2);
const Vector2f levelPos(centerFont(9, fontSize, winSize.x), // len(LEVEL: 00) = 9
                        winSize.y - fontSize * 4);

const float lostX = centerFont(8, fontSize, winSize.x);     // x pos for "YOU LOST"
const float pressX = centerFont(14, fontSize, winSize.x);   // x pos for "PRESS ENTER TO"
const int lostFontMul = -7; // how many fontSizes to offset from center of screen for top of lost text

const Vector2f nextTetPos(11, boardDim.y - 3);
const Vector2f heldTetPos(-5, boardDim.y - 3);
const Vector2f spawnPos(3, boardDim.y - 4);

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
    array<uint16_t, 4>({0x0f00, 0x4444, 0x00f0, 0x2222}), // I
    array<uint16_t, 4>({0x0660, 0x0660, 0x0660, 0x0660}), // O
    array<uint16_t, 4>({0x3600, 0x4620, 0x0360, 0x2310}), // Z
    array<uint16_t, 4>({0x6300, 0x2640, 0x0630, 0x1320}), // S
    array<uint16_t, 4>({0x4700, 0x2260, 0x0710, 0x3220}), // L
    array<uint16_t, 4>({0x1700, 0x6220, 0x0740, 0x2230}), // J
    array<uint16_t, 4>({0x2700, 0x2620, 0x0720, 0x2320})  // T
});

// https://gamedev.stackexchange.com/questions/159835/understanding-tetris-speed-curve
const array<int, 20> frames({48, 43, 38, 33, 28, 23, 18, 13, 8, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2});
const array<int, 20> softFrames({3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});

RectangleShape tile(Vector2f(tileSize, tileSize));

// game vars
int frameTimer, rotation, heldTet, score, level, lines;
bool usedHeld, softDrop, paused, lost;

Vector2f tetPos = spawnPos;

RenderWindow win(VideoMode(winSize.x, winSize.y), "Tetris", Style::Titlebar);
vector<vector<int>> tiles;
int nextTet;
int currentTet;

bool collisionCheck(int rotOffset, int moveOffset)
{
    uint16_t current = tetrominos[currentTet][(rotation + rotOffset) % 4];
    for (int i = 0; i < 16; ++i)
        if (current >> i & 1 == 1)
        {
            int x = i % 4 + tetPos.x + moveOffset;
            int y = floor(i / 4) + tetPos.y + ((!rotOffset && !moveOffset) ? -1 : 0);

            if (y == -1 || x < 0 || x >= boardDim.x || (y < 20 && tiles[y][x] != N))
                return false;
        }

    return true;
}

void placeTet()
{
    // add tetrominos to tile vector
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

    // lose check for when tetromino spawns with y < 20
    // if tetromino spawns inside of tile, lose
    current = tetrominos[currentTet][rotation % 4];
    for (int i = 0; i < 16; ++i)
        if (current >> i & 1 == 1 && tiles[floor(i / 4) + tetPos.y][i % 4 + tetPos.x] != N)
        {
            lost = true;
            return;
        }

    int rows = 0;
    // check for full row
    for (int y = boardDim.y-1; y >= 0; --y)
    {
        vector<int> temp;

        for (int x = 0; x < boardDim.x; ++x)
            if (tiles[y][x] == N)
                goto cnt; // apparently this is the only way to break in a nested loop

        // erase row and insert empty row at the top
        tiles.erase(tiles.begin() + y);
        for (int x = 0; x < boardDim.x; ++x)
            temp.push_back(N);

        tiles.push_back(temp);
        ++rows;
        ++lines;

    cnt:;
    }

    // check to increase level
    if (level == startLevel)
    {
        if (lines >= min(startLevel * 10 + 10, max(100, startLevel * 10 - 50)))
            ++level;
    }
    else if (lines >= min(startLevel * 10 + 10, max(100, startLevel * 10 - 50)) + 10 * level)
        ++level;

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

void drawTet(int xOffset, int yOffset, int tet, int rot, bool topCheck)
{
    uint16_t current = tetrominos[tet][rot % 4];
    for (int i = 0; i < 16; ++i)
    {
        if (current >> i & 1 == 1) // if bit is set
        {
            int x = i % 4 + xOffset;
            int y = floor(i / 4) + yOffset;
            int ypos = boardDim.y - y - 1;

            if (!topCheck || y < 20)
            {
                tile.setFillColor(COLORS[tet]);
                tile.setPosition(boardPos + Vector2f(tileSize * x, tileSize * ypos));
                win.draw(tile);
            }
        }
    }
}
