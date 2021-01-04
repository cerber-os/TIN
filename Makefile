C=clang
CFLAGS=-I include/ -Wall

all: out/logger.o out/server

out/logger.o: utils/logger.c
	$(C) $(CFLAGS) -o out/logger.o -c utils/logger.c

out/server: server/server.c out/logger.o
	$(C) $(CFLAGS) -o out/server out/logger.o server/server.c
