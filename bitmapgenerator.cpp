#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <math.h>
#include <array>
#include <sstream>

using namespace sf;
using namespace std;

const int X_TILES = 8;
const int Y_TILES = 8;
const int WIDTH = 400;
const int HEIGHT = 400;
const Vector2f tileSize(WIDTH/X_TILES, HEIGHT/Y_TILES);
const Color gridColor(255, 255, 255, 80);

// state vars
RectangleShape rect(tileSize);
RenderWindow win(VideoMode(WIDTH, HEIGHT), "win", Style::Titlebar);
array<array<bool, X_TILES>, Y_TILES> selection;
vector<array<uint8_t, Y_TILES>> output;
char current = '!';
Font font;

void fill()
{
    for (int y = 0; y < selection.size(); ++y)
        for (int x = 0; x < selection[y].size(); ++x)
            selection[y][x] = false;
}

void processMouse(bool left, Vector2i pos)
{
    if (pos.x >= 0 && pos.y >= 0 && pos.x < WIDTH && pos.y < HEIGHT)
    {
        int x = floor(pos.x/tileSize.x);
        int y = floor(pos.y/tileSize.y);
        selection[y][x] = left;
    }
}

array<uint8_t, Y_TILES> getVal()
{
    array<uint8_t, Y_TILES> out;

    for (int y = 0; y < out.size(); ++y)
    {
        out[Y_TILES-y-1] = 0;
        for (int x = 0; x < X_TILES; ++x)
            if (selection[y][x])
                out[Y_TILES-y-1] += pow(2, x);
    }
    
    return out;
}

void draw()
{
    RectangleShape lineH(Vector2f(WIDTH, 2));
    RectangleShape lineV(Vector2f(2, HEIGHT));

    lineH.setFillColor(gridColor);
    lineV.setFillColor(gridColor);

    for (int y = 0;  y < Y_TILES; ++y)
    {
        // grid lines
        if (y > 0)
        {
            lineH.setPosition(Vector2f(0, y * tileSize.x - 1));
            lineV.setPosition(Vector2f(y * tileSize.y - 1, 0));
            win.draw(lineH);
            win.draw(lineV);
        }

        // tiles
        for (int x = 0; x < X_TILES; ++x)
            if (selection[y][x])
            {
                rect.setPosition(Vector2f(x*tileSize.x, y*tileSize.y));
                win.draw(rect);
            }
    }

    // draw char
    Text t;
    t.setFont(font);
    t.setString(current);
    t.setCharacterSize(24);
    t.setPosition(10, 10);
    win.draw(t);
}

string toHex(uint8_t val)
{
    stringstream ss;
    ss << hex << unsigned(val);

    string hex = ss.str();
    if (hex.size() == 1)
        return "0x0"+hex;
    else
        return "0x"+hex;
}

void writeToFile()
{
    ofstream File("output.txt");

    File << "unsigned char bitmap[" << (output.size()+1) << "][" << Y_TILES << "] = {" << endl;

    // hardcode space
    File << "    {0x00";
    for (int i = 1; i < Y_TILES; ++i)
        File << ", 0x00";
    File << "}," << endl;

    // rest of characters
    for (int i = 0; i < output.size(); ++i)
    {
        File << "    {" << toHex(output[i][0]);
        for (int n = 1; n < Y_TILES; ++n)
            File << ", " << toHex(output[i][n]);

        File << "}," << endl;
    }

    File << "};" << endl;

    File.close();
}

int main()
{
    Event event;
    if (!font.loadFromFile("/usr/share/fonts/CascadiaCode.ttf"))
    {
        printf("Error loading font!");
        return -1;
    }

    fill();
    
    rect.setOutlineThickness(0);
    rect.setFillColor(Color::White);

    while (win.isOpen())
    {
        while (win.pollEvent(event))
        {
            if (event.type == Event::KeyPressed)
            {
                int code = event.key.code;

                if (code == Keyboard::Enter)
                {
                    output.push_back(getVal());
                    ++current;
                    fill();
                }
                else if (code == Keyboard::S && event.key.control)
                {
                    writeToFile();
                    return 0;
                }
                else if (code == Keyboard::Z && event.key.control)
                {
                    fill();
                    --current;
                    output.pop_back();
                }
            }
            else if (event.type == Event::Closed)
                win.close();
        }

        if (Mouse::isButtonPressed(Mouse::Left))
            processMouse(true, Mouse::getPosition(win));

        if (Mouse::isButtonPressed(Mouse::Right))
            processMouse(false, Mouse::getPosition(win));
            
        win.clear();
        draw();
        win.display();
    }
}
