.PHONY: all clean

all:
	gcc -o chat main1.c -pthread
clean:
	rm -rf chat