/*
    https://tetris.fandom.com/wiki/TGM_legend
    https://tetris.fandom.com/wiki/Drop
    https://strategywiki.org/wiki/Tetris/Controls
    https://www.sfml-dev.org/tutorials/2.5/window-inputs.php
*/

#include <iostream>
#include <vector>
#include <random>

#include "tetris.hpp"

using namespace sf;
using namespace std;

void updateGame(game *g, int *timer, bool softDrop)
{
    // if timer > frame count (depending on if soft dropping)
    if (*timer >= ((softDrop) ? softFrames[g->level] : frames[g->level]))
    {
        if (collisionCheck(0, 0, g))
            g->tetPos = Vector2f(g->tetPos.x, g->tetPos.y + 1);
        else
            placeTet(g);

        *timer = 0;
    }
}

void drawGame(game *g, RenderWindow *win, int *timer, bool softDrop, bool paused)
{
    if (!paused)
        updateGame(g, timer, softDrop);

    // draw board
    RectangleShape board(boardSize);
    board.setFillColor(Color::Black);
    board.setOutlineThickness(2);
    board.setOutlineColor(Color::White);
    board.setPosition(boardPos);

    win->draw(board);

    // draw grid
    RectangleShape line(Vector2f(1, TILE_SIZE * 20));
    line.setFillColor(gridColor);

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

    // draw score
    drawText(to_string(g->score), 32, boardPos+scorePos, Color::White, win);

    if (paused)
        drawText("PAUSED", 32, boardPos+fontPos, Color::White, win);
    else
    {
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
        drawTet(g->tetPos.x, g->tetPos.y, g->currentTet, g->rotation, g, win);

        // draw next
        drawTet(nextTetPos.x, nextTetPos.y, g->nextTet, tetDisplayRot, g, win);

        // draw held
        if (g->heldTet != N)
            drawTet(heldTetPos.x, heldTetPos.y, g->heldTet, tetDisplayRot, g, win);
    }
}

void input(int code, game *g, bool *paused, bool *softDrop)
{
    if (!*paused)
    {
        if (code == Keyboard::Up || code == Keyboard::W || code == Keyboard::X)
        { // clockwise
            if (collisionCheck(1, 0, g))
                ++g->rotation;
        }
        else if (code == Keyboard::Z)
        { // counterclockwise
            if (collisionCheck(-1, 0, g))
                g->rotation += 3;
        }
        else if (code == Keyboard::Left || code == Keyboard::A)
        { // move left
            if (collisionCheck(0, -1, g))
                g->tetPos = Vector2f(g->tetPos.x - 1, g->tetPos.y);
        }
        else if (code == Keyboard::Right || code == Keyboard::D)
        { // move right
            if (collisionCheck(0, 1, g))
                g->tetPos = Vector2f(g->tetPos.x + 1, g->tetPos.y);
        }
        else if (code == Keyboard::Space)
        {
            // hard drop (drop until collision)
            while (collisionCheck(0, 0, g))
                g->tetPos = Vector2f(g->tetPos.x, g->tetPos.y + 1);

            placeTet(g);
        }
        else if (code == Keyboard::Down || code == Keyboard::S)
            *softDrop = true;
        else if (code == Keyboard::C && !g->usedHeld)
        { // hold piece
            if (g->heldTet != N)
            {
                int temp = g->heldTet;
                g->heldTet = g->currentTet;
                g->currentTet = temp;
            }
            else
            {
                g->heldTet = g->currentTet;
                g->currentTet = g->nextTet;
                g->nextTet = rand() % N;
            }
            g->tetPos = spawnVec;
            g->rotation = 0;
            g->usedHeld = true;
        }
    }

    if (code == Keyboard::P || code == Keyboard::Escape)
        *paused = !*paused;
}

int main()
{
    srand(time(NULL)); // seed generator with time

    RenderWindow window(VideoMode(480, 640), "win", Style::Titlebar);
    Vector2u size = window.getSize();
    Event event;

    tile.setOutlineThickness(0);
    window.setFramerateLimit(60);

    // init array
    vector<vector<int>> tiles;
    for (int y = 0; y < 20; ++y)
    {
        vector<int> temp;
        for (int x = 0; x < 10; ++x)
            temp.push_back(N);
        tiles.push_back(temp);
    }

    game g = {rand() % N, rand() % N, N, 0, Vector2f(3, 0), tiles, 0, 0, false};

    bool softDrop = false;
    bool paused = false;
    int timer = 0;

    while (window.isOpen())
    {
        softDrop = false;

        while (window.pollEvent(event))
        {
            if (event.type == Event::KeyPressed)
                input(event.key.code, &g, &paused, &softDrop);
            else if (event.type == Event::Closed)
                window.close();
        }

        window.clear();

        drawGame(&g, &window, &timer, softDrop, paused);
        // drawText("1", 64, Vector2f(0, 100), Color::White, &window);

        window.display();
        ++timer;
    }

    return 0;
}
