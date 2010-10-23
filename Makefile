CC      = gcc
CFLAGS  = -g -pthread -ansi -std=c99 -D_SVID_SOURCE -pedantic-errors -Wall \
          `pkg-config --cflags gtk+-2.0`
LDFLAGS = `pkg-config --libs gtk+-2.0` -lgthread-2.0

# Build everything
all: client

# Build the server
# server: 

# Build the logger
# logger:

# Build the archiver
# archiver:

# Build the client
client: build/gui.o build/client.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o build/client build/client.o build/gui.o

build/client.o: src/client/client.c src/client/client.h src/commons.h src/client/gui.h
	$(CC) $(CFLAGS) -c -o build/client.o src/client/client.c

build/gui.o: src/client/gui.c src/client/gui.h src/commons.h
	$(CC) $(CFLAGS) -c -o build/gui.o src/client/gui.c

# Clean up the build directory
clean:
	rm -f build/*.o build/client
