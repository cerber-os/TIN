C=clang
Cpp=clang++
CFLAGS=-I include/ -Wall -g -pedantic -Wno-zero-length-array -fsanitize=address -fno-omit-frame-pointer

all: out/logger.o out/list.o out/server out/fileOperations.o out/handleSockets.o out/client out/libTestServer

out/logger.o: utils/logger.c
	$(C) $(CFLAGS) -o out/logger.o -c utils/logger.c

out/list.o: utils/list.c
	$(C) $(CFLAGS) -o out/list.o -c utils/list.c

out/config_parser.o: utils/config_parser.c
	$(C) $(CFLAGS) -o out/config_parser.o -c utils/config_parser.c

out/server: server/server.c out/logger.o out/list.o out/config_parser.o server/client_handle.c
	$(C) $(CFLAGS) -lpam -o out/server out/logger.o out/list.o out/config_parser.o server/client_handle.c server/server.c

# Pliki klienta
out/fileOperations.o: library/fileOperations.cpp
	$(Cpp) $(CFLAGS) -o out/fileOperations.o -c library/fileOperations.cpp

out/handleSockets.o: library/handleSockets.cpp
	$(Cpp) $(CFLAGS) -o out/handleSockets.o -c library/handleSockets.cpp

out/client: cli/client.cpp out/fileOperations.o out/handleSockets.o
	$(Cpp) $(CFLAGS) -o out/client out/fileOperations.o out/handleSockets.o cli/client.cpp

out/libTestServer: library/libTestServer.cpp
	$(Cpp) $(CFLAGS) -o out/libTestServer library/libTestServer.cpp

clean:
	rm out/*