CC      	= gcc

CFLAGS  	= -g -pthread -ansi -std=c99 -D_SVID_SOURCE -pedantic-errors -Wall
CCLIENT 	= $(CFLAGS) `pkg-config --cflags gtk+-2.0`
CSERVER 	= $(CFLAGS)

LDFLAGS 	=
LDCLIENT	= $(LDFLAGS) `pkg-config --libs gtk+-2.0` -lgthread-2.0
LDSERVER	= $(LDFLAGS)

# Build everything
all: client server

# Build the server
server: build/server.o
	$(CC) $(CSERVER) $(LDSERVER) -o build/server build/server.o

build/server.o:
	$(CC) $(CSERVER) -c -o build/server.o src/server/server.c

# Build the logger
# logger:

# Build the archiver
# archiver:

# Build the client
client: build/gui.o build/client.o
	$(CC) $(CCLIENT) $(LDCLIENT) -o build/client build/client.o build/gui.o

build/client.o: src/client/client.c src/client/client.h src/commons.h src/client/gui.h
	$(CC) $(CCLIENT) -c -o build/client.o src/client/client.c

build/gui.o: src/client/gui.c src/client/gui.h src/commons.h
	$(CC) $(CCLIENT) -c -o build/gui.o src/client/gui.c

# Clean up the build directory
clean:
	rm -f build/*.o build/client build/server