#!/usr/bin/env sh
g++ -c -g $1.cpp
g++ -o $1 $1.o -lsfml-graphics -lsfml-window -lsfml-system
./$1
