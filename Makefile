CC = gcc
CFLAGS = -Wall -Wextra

all: ndn

ndn: 
	$(CC) $(CFLAGS) -o ndn main.c node.c network.c utils.c
	
clean:
	rm -f *.o ndn
