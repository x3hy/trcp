CC := cc
CFLAGS := -g
PREFIX := /usr/local
PORT := 6060

# Copy the config over
src/config.h: src/config.def.h
	cat $^ > $@

# Build the chat client
trchat: trchat.c src/config.h
	$(CC) $(CFLAGS) $(firstword $^) -o $@

# Build the server
trcp: trcp.c src/config.h
	$(CC) $(CFLAGS) $(firstword $^) -o $@

# Test the connection
conn: src/testconn
	./$^ $(PORT) POST test "hello_world"
	./$^ $(PORT) GET test

# Remove builds
clean:
	-rm trcp
	-rm trchat

# Install systemwide
install: trchat trcp
	mv $(firstword $^) $(PREFIX)/bin/trchat
	mv $(lastword $^) $(PREFIX)/bin/trcp

# Start the server
start: trcp
	./$^ --port=$(PORT) --id=$(UID)

# Start the chat
chat: trchat
	./$^ --port=$(PORT) --addr=http://localhost --id=$(UID)
