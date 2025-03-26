CC = gcc
CFLAGS = -Wall



all: clean ndn

ndn: 
	
	$(CC) $(CFLAGS) -o ndn main.c node.c network.c utils.c object.c cache.c utilsForObject.c 
	
clean:
	rm -f *.o ndn
