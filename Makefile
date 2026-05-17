CC = gcc
CFLAGS = -Wall -Wextra -g

SRC = allocator.c test.c
OBJ = $(SRC:.c=.o)

test: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean valgrind

clean:
	rm -f *.o test

valgrind:
	valgrind --leak-check=full --track-origins=yes ./test