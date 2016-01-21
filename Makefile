all:
	g++-5 -g -std=c++1y main.cpp
	./a.out < in.txt
