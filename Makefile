CFLAGS := -O2 -Wall -Wextra -Wunreachable-code -Wunused-parameter

all: client server

server: server.c
	$(CC) $(CFLAGS) -o $@ $^

client: client.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clear
clear:
	-@rm -f client
	-@rm -f server
