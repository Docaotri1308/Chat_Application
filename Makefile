.PHONY: all clean

all:
	gcc -o chat main.c -pthread
clean:
	rm -rf chat