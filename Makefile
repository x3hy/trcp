CC := cc
PREFIX := /usr/local
PORT := 15805
UID := devtest
VER := \"$(shell git describe --tags --always --dirty 2>/dev/null)\"
CFLAGS := -g -DVERSION=$(VER)
HOST    := localhost
MESSAGE :=

# Copy the configuration over
src/config.h: src/config.def.h
	cat $^ > $@

# Build the server
trcp: trcp.o src/config.h
	$(CC) $(CFLAGS) $(firstword $^) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Test the connection
conn: src/testconn
	./$^ $(PORT) POST test "hello_world"
	./$^ $(PORT) GET test

# Remove builds
clean:
	rm -f trchat trcp *.o

# Install systemwide
install: trchat trcp
	mv $(firstword $^) $(PREFIX)/bin/trchat
	mv $(lastword $^) $(PREFIX)/bin/trcp

# Start the server
start: clean trcp
	@echo "" ./$(lastword $^)  --port=$(PORT) --verbose $(UID)

stream:
	curl -N $(HOST):$(PORT)/sock/$(UID) --output - -i

post: src/post
	./$^ $(PORT) $(UID) $(HOST)


.PHONY: clean commit src/config.h
