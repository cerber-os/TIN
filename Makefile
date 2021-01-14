C=clang
Cpp=clang++
CFLAGS=-I include/ -Wall -pedantic -Wno-zero-length-array -fno-omit-frame-pointer

ifeq ($(MAKECMDGOALS),asan_lsan)
    CFLAGS += -O3 -fsanitize=address -fsanitize=leak
else ifeq ($(MAKECMDGOALS),debug)
	CFLAGS += -O0 -g -fsanitize=address -fsanitize=leak
else
    CFLAGS += -O3
endif

all: out/logger.o out/list.o out/server out/fileOperations.o out/handleSockets.o out/client out/mynfs_library.so

debug: out/logger.o out/list.o out/server out/fileOperations.o out/handleSockets.o out/client out/mynfs_library.so

asan_lsan: out/logger.o out/list.o out/server out/fileOperations.o out/handleSockets.o out/client out/mynfs_library.so

tests: out/01-read-write out/02-flock

# Pliki serwera
out/logger.o: utils/logger.c
	$(C) $(CFLAGS) -o out/logger.o -c utils/logger.c

out/list.o: utils/list.c
	$(C) $(CFLAGS) -o out/list.o -c utils/list.c

out/config_parser.o: utils/config_parser.c
	$(C) $(CFLAGS) -o out/config_parser.o -c utils/config_parser.c

out/server: server/server.c out/logger.o out/list.o out/config_parser.o server/client_handle.c server/auth.c
	$(C) $(CFLAGS) -lpam -o out/server out/logger.o out/list.o out/config_parser.o server/auth.c server/client_handle.c server/server.c

# Pliki klienta
out/fileOperations.o: library/fileOperations.cpp
	$(Cpp) $(CFLAGS) -fPIC -o out/fileOperations.o -c library/fileOperations.cpp

out/handleSockets.o: library/handleSockets.cpp
	$(Cpp) $(CFLAGS) -fPIC -o out/handleSockets.o -c library/handleSockets.cpp

out/client: cli/client.cpp out/fileOperations.o out/handleSockets.o
	$(Cpp) $(CFLAGS) -o out/client out/fileOperations.o out/handleSockets.o cli/client.cpp

out/mynfs_library.so: out/fileOperations.o out/handleSockets.o
	$(Cpp) $(CFLAGS) --shared -o out/mynfs_library.so out/fileOperations.o out/handleSockets.o

# Testy
out/01-read-write: tests/01-read-write.c out/mynfs_library.so
	$(C) $(CFLAGS) -o out/01-read-write tests/01-read-write.c out/mynfs_library.so

out/02-flock: tests/02-flock.c out/mynfs_library.so
	$(C) $(CFLAGS) -o out/02-flock tests/02-flock.c out/mynfs_library.so

clean:
	rm out/*