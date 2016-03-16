CC   = g++ -std=c++1y
SRCS = $(shell find src -name '*.cpp')
OBJS = $(addprefix obj/,$(notdir $(SRCS:%.cpp=%.o)))

bin/tg: $(OBJS) bin
	$(CC) $(OBJS) -o $@

obj/%.o: src/%.cpp
	mkdir -p obj
	$(CC) -c $< -o $@

bin:
	mkdir -p bin

.PHONY: clean

clean:
	rm -rf obj bin
