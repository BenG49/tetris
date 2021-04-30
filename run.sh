#!/usr/bin/env sh
g++ -c -g tetris.cpp
g++ tetris.o -lsfml-graphics -lsfml-window -lsfml-system
./a.out
