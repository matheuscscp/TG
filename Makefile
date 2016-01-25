all:
	g++-5 -g -D"DEBUG" -std=c++1y main.cpp
	./a.out < in.txt
