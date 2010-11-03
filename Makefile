CC      	= gcc

CFLAGS  	= -g -pthread -ansi -std=c99 -D_SVID_SOURCE -pedantic-errors -Wall
CCLIENT 	= $(CFLAGS) `pkg-config --cflags gtk+-2.0`
CSERVER 	= $(CFLAGS)

LDFLAGS 	=
LDCLIENT	= $(LDFLAGS) `pkg-config --libs gtk+-2.0` -lgthread-2.0
LDSERVER	= $(LDFLAGS)

# Build everything
all: client server logger

# Build the server
server: build/server.o build/signal_handler.o build/mq.o build/utils.o \
	build/blackboard.o build/login_thread.o build/broadcasting.o
	$(CC) $(CSERVER) $(LDSERVER) -o build/server build/server.o \
		build/signal_handler.o build/mq.o build/blackboard.o \
		build/login_thread.o build/client_thread.o build/net_message.o \
		build/client_list.o build/message_handler.o build/utils.o \
		build/broadcasting.o build/message_builder.o

build/server.o:
	$(CC) $(CSERVER) -c -o build/server.o src/server/server.c

build/signal_handler.o:
	$(CC) $(CSERVER) -c -o build/signal_handler.o src/server/signal_handler.c

build/mq.o:
	$(CC) $(CSERVER) -c -o build/mq.o src/server/mq.c

build/utils.o:
	$(CC) $(CSERVER) -c -o build/utils.o src/server/utils.c

build/blackboard.o:
	$(CC) $(CSERVER) -c -o build/blackboard.o src/server/blackboard.c

build/login_thread.o: build/client_thread.o
	$(CC) $(CSERVER) -c -o build/login_thread.o src/server/login_thread.c

build/client_thread.o: build/net_message.o build/client_list.o \
	build/message_handler.o build/broadcasting.o
	$(CC) $(CSERVER) -c -o build/client_thread.o src/server/client_thread.c

build/net_message.o:
	$(CC) $(CSERVER) -c -o build/net_message.o src/net_message.c

build/client_list.o:
	$(CC) $(CSERVER) -c -o build/client_list.o src/server/client_list.c

build/message_builder.o:
	$(CC) $(CSERVER) -c -o build/message_builder.o src/server/message_builder.c

build/message_handler.o:
	$(CC) $(CSERVER) -c -o build/message_handler.o src/server/message_handler.c

build/broadcasting.o: build/net_message.o build/message_builder.o
	$(CC) $(CSERVER) -c -o build/broadcasting.o src/server/broadcasting.c

# Build the logger
logger: build/logger.o build/mq.o
	$(CC) $(CSERVER) $(LDSERVER) -o build/logger build/logger.o build/mq.o
build/logger.o:
	$(CC) $(CSERVER) -c -o build/logger.o src/server/logger.c

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
	rm -f build/*.o build/client build/server build/logger
