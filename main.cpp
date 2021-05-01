/*
    https://tetris.fandom.com/wiki/TGM_legend
    https://tetris.fandom.com/wiki/Drop
    https://strategywiki.org/wiki/Tetris/Controls
    https://www.sfml-dev.org/tutorials/2.5/window-inputs.php
*/

#include "libs.hpp"
#include "tetris.hpp"

const bool useGrid = false;

void pad0(string *s, int len)
{
    while (s->size() < len)
        s->insert(s->begin(), '0');
}

void updateGame()
{
    if (lost)
    {
        drawText("YOU LOST", fontSize, Vector2f(lostX, pausePos.y + fontSize * lostFontMul), Color::White, &win);

        // draw score
        string s = to_string(score);
        pad0(&s, 6);
        drawText("SCORE: "+s, fontSize, Vector2f(scorePos.x, pausePos.y + fontSize * (lostFontMul + 2)), Color::White, &win);

        // draw lines
        s = to_string(lines);
        pad0(&s, 3);
        drawText("LINES: "+s, fontSize, Vector2f(linesPos.x, pausePos.y + fontSize * (lostFontMul + 4)), Color::White, &win);

        // draw prompt
        drawText("PRESS ENTER TO", fontSize, Vector2f(pressX, pausePos.y + fontSize * (lostFontMul + 7)), Color::White, &win);
        drawText("PLAY AGAIN", fontSize, Vector2f(linesPos.x, pausePos.y + fontSize * (lostFontMul + 9)), Color::White, &win);

        return;
    }

    // update game if timer > frame count (depending on if soft dropping)
    if (!paused && frameTimer >= (softDrop ? softFrames[level] : frames[level]))
    {
        if (collisionCheck(0, 0))
            tetPos = Vector2f(tetPos.x, tetPos.y - 1);
        else
            placeTet();

        frameTimer = 0;
    }

    // draw board outline
    RectangleShape board(boardSize);
    board.setFillColor(Color::Black);
    board.setOutlineThickness(2);
    board.setOutlineColor(Color::White);
    board.setPosition(boardPos);

    win.draw(board);

    // draw grid
    if (useGrid)
    {
        RectangleShape line(Vector2f(1, tileSize * boardDim.y));
        line.setFillColor(gridColor);

        for (int x = 1; x < boardDim.x; ++x)
        {
            line.setPosition(Vector2f(boardPos.x + x * tileSize, boardPos.y));
            win.draw(line);
        }

        line.setSize(Vector2f(tileSize * boardDim.x, 1));
        for (int y = 1; y < boardDim.y; ++y)
        {
            line.setPosition(Vector2f(boardPos.x, boardPos.y + y * tileSize));
            win.draw(line);
        }
    }

    // draw score
    string s = to_string(score);
    pad0(&s, 6);
    drawText("SCORE: "+s, fontSize, scorePos, Color::White, &win);

    // draw lines
    s = to_string(lines);
    pad0(&s, 3);
    drawText("LINES: "+s, fontSize, linesPos, Color::White, &win);

    if (paused)
        drawText("PAUSED", fontSize, pausePos, Color::White, &win);
    else
    {
        // draw tiles
        for (int y = 0; y < boardDim.y; ++y)
            for (int x = 0; x < boardDim.x; ++x)
                if (tiles[y][x] != N)
                {
                    int ypos = boardDim.y - y - 1;

                    tile.setFillColor(COLORS[tiles[y][x]]);
                    tile.setPosition(boardPos + Vector2f(tileSize * x, tileSize * ypos));
                    win.draw(tile);
                }

        // draw falling tet
        drawTet(tetPos.x, tetPos.y, currentTet, rotation, true);

        // draw next
        drawTet(nextTetPos.x, nextTetPos.y, nextTet, displayRot, false);

        // draw held
        if (heldTet != N)
            drawTet(heldTetPos.x, heldTetPos.y, heldTet, displayRot, false);
    }
}

void init()
{
    frameTimer = 0;
    rotation = 0;
    heldTet = N;
    score = 0;
    level = 0;
    lines = 0;

    usedHeld = false;
    softDrop = false;
    paused = false;
    lost = false;

    tetPos = spawnPos;

    tile.setOutlineThickness(0);
    win.setFramerateLimit(60);

    // init array
    tiles.clear();
    for (int y = 0; y < boardDim.y; ++y)
    {
        vector<int> temp;
        for (int x = 0; x < boardDim.x; ++x)
            temp.push_back(N);
        tiles.push_back(temp);
    }

    nextTet = rand() % N;
    currentTet = rand() % N;
}

void input(int code)
{
    if (lost)
    {
        if (code == Keyboard::Enter)
            init();

        return;
    }

    if (!paused)
    {
        if ((code == Keyboard::Up || code == Keyboard::W || code == Keyboard::X) && collisionCheck(1, 0))
            // clockwise
            ++rotation;
        else if (code == Keyboard::Z && collisionCheck(-1, 0))
            // counterclockwise
            rotation += 3;
        else if ((code == Keyboard::Left || code == Keyboard::A) && collisionCheck(0, -1))
            // move left
            tetPos = Vector2f(tetPos.x - 1, tetPos.y);
        else if ((code == Keyboard::Right || code == Keyboard::D) && collisionCheck(0, 1))
            // move right
            tetPos = Vector2f(tetPos.x + 1, tetPos.y);
        else if (code == Keyboard::Space)
        {
            // hard drop (drop until collision)
            while (collisionCheck(0, 0))
                tetPos = Vector2f(tetPos.x, tetPos.y - 1);

            placeTet();
        }
        else if (code == Keyboard::Down || code == Keyboard::S)
            softDrop = true;
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
        paused = !paused;
}

int main()
{
    srand(time(NULL)); // seed generator with time

    Event event;
    init();

    while (win.isOpen())
    {
        softDrop = false;

        while (win.pollEvent(event))
        {
            if (event.type == Event::KeyPressed)
                input(event.key.code);
            else if (event.type == Event::Closed)
                win.close();
        }

        win.clear();

        updateGame();

        win.display();
        ++frameTimer;
    }

    return 0;
}
