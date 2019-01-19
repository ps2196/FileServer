#!bin/bash
CC=g++
LINK_FLAGS=-lboost_system -lboost_filesystem
FLAGS=-std=c++11
main: main.cpp
	$(CC) main.cpp $(FLAGS) $(LINK_FLAGS) -o server -I.
