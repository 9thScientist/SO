CFLAGS := -O2 -Wall -Wextra -Wunreachable-code -Wunused-parameter

all: client server

server: server.c backup.c message.c restore.c
	$(CC) $(CFLAGS) -o $@ $^

client: client.c message.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clear
clear:
	-@rm -f client
	-@rm -f server

.PHONY: stop
stop:
	pkill -f sobuserv

.PHONY: install
install:
	-@if [ ! -d $(HOME)/bin ] ; \
	  then \
		  mkdir $(HOME)/bin; \
	  fi;
	-@mv server $(HOME)/bin/sobuserv
	-@mv client $(HOME)/bin/sobucli

.PHONY: uninstall
uninstall:
	-@rm $(HOME)/bin/sobuserv
	-@rm $(HOME)/bin/sobucli
