CC=gcc

all: utils.o main.o
	$(CC) utils.o main.o -o game -lpthread

%.o: %.c
	$(CC) -c $< -g -o $@

clean:
	rm -f main.o utils.o game
