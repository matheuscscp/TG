all:
	g++-5 -g -D"DEBUG" -std=c++1y src/*.cpp
	./a.out < in.txt
