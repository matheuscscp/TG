CC   = g++ -std=c++1y
SRCS = $(shell find src -name '*.cpp')
OBJS = $(addprefix obj/,$(notdir $(SRCS:%.cpp=%.o)))

.PRECIOUS: $(OBJS)

obj/%.o: src/%.cpp
	mkdir -p obj
	$(CC) -c $< -o $@

bin/tg: $(OBJS) bin
	$(CC) $(OBJS) -o $@

bin:
	mkdir -p bin

.PHONY: clean

clean:
	rm -rf obj bin
