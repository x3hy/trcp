CC := cc
CFLAGS := -g
PREFIX := /usr/local

all: 0bchat

config.h: config.def.h
	cat config.def.h > config.h

clean:
	-rm config.h
	-rm 0bchat
	-rm $(PREFIX)/bin/0bchat

0bchat: 0bchat.c
	$(CC) $(CFLAGS) $^ -o $@

install: 0bchat
	mv 0bchat $(PREFIX)/bin/0bchat

0brelay: server.c
	$(CC) $(CFLAGS) $^ -o $@
