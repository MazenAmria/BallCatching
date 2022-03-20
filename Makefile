CC=gcc

all: utils.o main.o
	$(CC) utils.o main.o -o game -lpthread

%.o: %.c
	$(CC) -c $< -g -o $@

clean:
	rm -f *.o game
