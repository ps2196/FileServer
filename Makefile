#!bin/bash
CC=g++
LINK_FLAGS=-lboost_system -lboost_filesystem
FLAGS=-std=c++11
main.o:
	$(CC) main.cpp $(FLAGS) $(LINK_FLAGS) -o server 
all:
	$(CC) main.cpp $(FLAGS) $(LINK_FLAGS) -o server