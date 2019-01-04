#!bin/bash
main.o:
	g++ main.cpp -std=c++11 -o server
all:
	g++ main.cpp -std=c++11 -o server