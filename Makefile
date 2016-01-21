all:
	g++-5 -std=c++1y main.cpp
	./a.out < in.txt
