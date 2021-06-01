CC=gcc

all: main utils
	$(CC) main utils -o game

%: %.c
	$(CC) -c $< -g -o $@

clean:
	rm -f main utils game
