CURR_DIR = $(shell pwd)

# The compiler
CC = gcc
CFLAGS = -Wall -Wextra -std=c++14 -Wl,--as-needed -I$(CURR_DIR)/include
DEBUG = -g

# Libraries needed
LOG_LDFLAGS = -lboost_log -lboost_log_setup -lboost_thread -lpthread -lboost_filesystem -lboost_system -lstdc++
LOG_CFLAGS = $(CFLAGS) -c -I/usr/include/
TESS_CFLAGS = $(CFLAGS) -c
TESS_LIBS = -L/usr/local/lib/
TESS_LDFLAGS = -ltesseract

main: include/logging.hpp bin/logging.o
	$(CC) $(CFLAGS) src/main.cpp bin/logging.o -o bin/main -DBOOST_LOG_DYN_LINK $(LOG_LDFLAGS) $(TESS_LDFLAGS)

logging_test: bin/logging.o bin/logging_test.o
	$(CC) $(CFLAGS) bin/logging.o bin/logging_test.o -o bin/logging_test -DBOOST_LOG_DYN_LINK $(LOG_LDFLAGS)

api_fetch: bin/api_fetch.o bin/logging.o
	$(CC) $(CFLAGS) bin/logging.o bin/api_fetch.o -o bin/api_fetch -DBOOST_LOG_DYN_LINK -lcurl $(LOG_LDFLAGS)


bin/logging_test.o: tests/logging_test.cpp include/logging.hpp
	$(CC) $(LOG_CFLAGS) -o bin/logging_test.o tests/logging_test.cpp -DBOOST_LOG_DYN_LINK $(LOG_LDFLAGS)

bin/logging.o: src/logging.cpp include/logging.hpp
	$(CC) $(LOG_CFLAGS) -o bin/logging.o src/logging.cpp -DBOOST_LOG_DYN_LINK $(LOG_LDFLAGS)

bin/api_fetch.o: src/api_fetch.cpp include/logging.hpp
	$(CC) $(TESS_CFLAGS) -o bin/api_fetch.o src/api_fetch.cpp $(TESS_LIBS) $(TESS_LDFLAGS)

clean:
	\rm -f bin/*
