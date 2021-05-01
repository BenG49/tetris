/*
    https://tetris.fandom.com/wiki/TGM_legend
    https://tetris.fandom.com/wiki/Drop
    https://strategywiki.org/wiki/Tetris/Controls
    https://www.sfml-dev.org/tutorials/2.5/window-inputs.php
*/

#include "libs.hpp"
#include "tetris.hpp"

const bool useGrid = false;

void updateGame(int *timer, bool softDrop)
{
    // if timer > frame count (depending on if soft dropping)
    if (*timer >= ((softDrop) ? softFrames[level] : frames[level]))
    {
        if (collisionCheck(0, 0))
            tetPos = Vector2f(tetPos.x, tetPos.y - 1);
        else
            placeTet();

        *timer = 0;
    }
}

void drawGame(RenderWindow *win, int *frameTimer, bool softDrop, bool paused)
{
    if (!paused)
        updateGame(frameTimer, softDrop);

    // draw board
    RectangleShape board(boardSize);
    board.setFillColor(Color::Black);
    board.setOutlineThickness(2);
    board.setOutlineColor(Color::White);
    board.setPosition(boardPos);

    win->draw(board);

    // draw grid
    if (useGrid)
    {
        RectangleShape line(Vector2f(1, TILE_SIZE * boardHeight));
        line.setFillColor(gridColor);

        for (int x = 1; x < boardWidth; ++x)
        {
            line.setPosition(Vector2f(boardPos.x + x * TILE_SIZE, boardPos.y));
            win->draw(line);
        }

        line.setSize(Vector2f(TILE_SIZE * boardWidth, 1));
        for (int y = 1; y < 20; ++y)
        {
            line.setPosition(Vector2f(boardPos.x, boardPos.y + y * TILE_SIZE));
            win->draw(line);
        }
    }

    // draw score
    drawText(to_string(score), 32, boardPos+scorePos, Color::White, win);

    if (paused)
        drawText("PAUSED", 32, boardPos+fontPos, Color::White, win);
    else
    {
        // draw tiles
        for (int y = 0; y < boardHeight; ++y)
            for (int x = 0; x < boardWidth; ++x)
                if (tiles[y][x] != N)
                {
                    int ypos = boardHeight - y - 1;

                    tile.setFillColor(COLORS[tiles[y][x]]);
                    tile.setPosition(boardPos + Vector2f(TILE_SIZE * x, TILE_SIZE * ypos));
                    win->draw(tile);
                }

        // draw falling tet
        drawTet(tetPos.x, tetPos.y, currentTet, rotation, win);

        // draw next
        drawTet(nextTetPos.x, nextTetPos.y, nextTet, tetDisplayRot, win);

        // draw held
        if (heldTet != N)
            drawTet(heldTetPos.x, heldTetPos.y, heldTet, tetDisplayRot, win);
    }
}

void input(int code, bool *paused, bool *softDrop)
{
    if (!*paused)
    {
        if (code == Keyboard::Up || code == Keyboard::W || code == Keyboard::X)
        { // clockwise
            if (collisionCheck(1, 0))
                ++rotation;
        }
        else if (code == Keyboard::Z)
        { // counterclockwise
            if (collisionCheck(-1, 0))
                rotation += 3;
        }
        else if (code == Keyboard::Left || code == Keyboard::A)
        { // move left
            if (collisionCheck(0, -1))
                tetPos = Vector2f(tetPos.x - 1, tetPos.y);
        }
        else if (code == Keyboard::Right || code == Keyboard::D)
        { // move right
            if (collisionCheck(0, 1))
                tetPos = Vector2f(tetPos.x + 1, tetPos.y);
        }
        else if (code == Keyboard::Space)
        {
            // hard drop (drop until collision)
            while (collisionCheck(0, 0))
                tetPos = Vector2f(tetPos.x, tetPos.y - 1);

            placeTet();
        }
        else if (code == Keyboard::Down || code == Keyboard::S)
            *softDrop = true;
        else if (code == Keyboard::C && !usedHeld)
        { // hold piece
            if (heldTet != N)
            {
                int temp = heldTet;
                heldTet = currentTet;
                currentTet = temp;
            }
            else
            {
                heldTet = currentTet;
                currentTet = nextTet;
                nextTet = rand() % N;
            }
            tetPos = spawnPos;
            rotation = 0;
            usedHeld = true;
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
    for (int y = 0; y < boardHeight; ++y)
    {
        vector<int> temp;
        for (int x = 0; x < boardWidth; ++x)
            temp.push_back(N);
        tiles.push_back(temp);
    }

    for (int i = 0; i < N; ++i)
        tiles[0][i] = i;

    nextTet = rand() % N;
    currentTet = rand() % N;

    bool softDrop = false;
    bool paused = false;
    int frameTimer = 0;

    while (window.isOpen())
    {
        softDrop = false;

        while (window.pollEvent(event))
        {
            if (event.type == Event::KeyPressed)
                input(event.key.code, &paused, &softDrop);
            else if (event.type == Event::Closed)
                window.close();
        }

        window.clear();

        drawGame(&window, &frameTimer, softDrop, paused);
        // drawText("1", 64, Vector2f(0, 100), Color::White, &window);

        window.display();
        ++frameTimer;
    }

    return 0;
}
