CFLAGS := -O2 -Wall -Wextra -Wunreachable-code -Wunused-parameter

all: client server

server: server.c backup.c message.c
	$(CC) $(CFLAGS) -o $@ $^

client: client.c message.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clear
clear:
	-@rm -f client
	-@rm -f server

.PHONY: install
install:
	-@mv server /usr/bin/sobuserv
	-@mv client /usr/bin/sobucli
	-@if [ -a /usr/share/sobuserv ] ; \
	  then \
		  rm -rf /usr/share/sobuserv ; \
	  fi;
	-@mkdir /usr/share/sobuserv 
	-@touch /usr/share/sobuserv/running_user 
	-@chmod 666 /usr/share/sobuserv/running_user 

.PHONY: uninstall
uninstall:
	-@rm -rf /usr/share/sobuserv
	-@rm /usr/bin/sobuserv
	-@rm /usr/bin/sobucli
