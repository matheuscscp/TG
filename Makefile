CC      = g++ -std=c++0x
SRCS    = $(shell find src -name '*.cpp')
HEADERS = $(shell find src -name '*.hpp')
OBJS    = $(addprefix obj/,$(notdir $(SRCS:%.cpp=%.o)))
EXE     = knapsack_cnf

$(EXE): $(OBJS)
	$(CC) $(OBJS) -o $@

obj/%.o: src/%.cpp $(HEADERS)
	mkdir -p obj
	$(CC) -c $< -o $@

.PHONY: clean

clean:
	rm -rf $(EXE) obj
