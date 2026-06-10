CC := cc
CFLAGS := -g
PREFIX := /usr/local
PORT := 5050

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

conn: testconn
	./$^ $(PORT) POST test "hello_world"
	./$^ $(PORT) GET test

start: server
	./$^ --port=$(PORT)
