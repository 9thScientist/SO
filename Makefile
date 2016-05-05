CFLAGS := -O2 -Wall -Wextra -Wunreachable-code -Wunused-parameter

all: client

client: client.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clear
clear:
	-@rm -f client
