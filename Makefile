CFLAGS := -O2 -Wall -Wextra -Wunreachable-code -Wunused-parameter

all: client server

server: server.c backup.c message.c restore.c
	$(CC) $(CFLAGS) -o $@ $^

client: client.c message.c vec.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clear
clear:
	-@rm -f client
	-@rm -f server

.PHONY: stop
stop:
	pkill -f sobusrv

.PHONY: install
install:
	-@sudo mv server /bin/sobusrv
	-@sudo mv client /bin/sobucli

.PHONY: uninstall
uninstall:
	-@rm /bin/sobusrv
	-@rm /bin/sobucli
