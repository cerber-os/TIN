C=clang
CFLAGS=-I include/ -Wall -g -pedantic -Wno-zero-length-array

all: out/logger.o out/server

out/logger.o: utils/logger.c
	$(C) $(CFLAGS) -o out/logger.o -c utils/logger.c

out/config_parser.o: utils/config_parser.c
	$(C) $(CFLAGS) -o out/config_parser.o -c utils/config_parser.c

out/server: server/server.c out/logger.o out/config_parser.o server/client_handle.c
	$(C) $(CFLAGS) -lpam -o out/server out/logger.o out/config_parser.o server/client_handle.c server/server.c

clean:
	rm out/*