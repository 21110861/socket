# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -std=c++20

# Libraries
LIBS = -lws2_32 -lmswsock -ladvapi32

all: server client autoclient

server: server.cpp
	$(CC) $(CFLAGS) -o server server.cpp $(LIBS)

client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp $(LIBS)
autoclient: autoclient.cpp
	$(CC) $(CFLAGS) -o autoclient autoclient.cpp $(LIBS)
clean:
	rm -f server client autoclient
