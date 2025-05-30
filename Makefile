CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude

SRC = src/main.c src/graph.c src/node.c src/edge.c
OBJ = $(SRC:.c=.o)
EXEC = test_graph

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(EXEC)

clean:
	rm -f src/*.o $(EXEC)

.PHONY: all clean run