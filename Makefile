CC=g++
SRC=S3_FIFO.cpp test.cpp
SRC_TEST = test.cpp
HEADERS=FIFO_.hpp generic_cache.hpp
BIN=test s3_fifo all

all:                   $(SRC)
			$(CC) $(CFLAGS) $^ -o $@
test:                   $(SRC_TEST)
			$(CC) $(CFLAGS) $^ -o $@
clean:
			rm -f *.o $(BIN)
