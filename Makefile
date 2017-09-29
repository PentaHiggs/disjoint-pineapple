CURR_DIR = $(shell pwd)

# The compiler
CC = g++
CFLAGS = -Wall -Wextra -std=c++14 -Wl,--as-needed -I$(CURR_DIR)/include
DEBUG = -g

# Libraries needed
LOG_LDFLAGS = -lboost_log -lboost_log_setup -lboost_thread -lpthread -lboost_filesystem -lboost_system -lstdc++
LOG_CFLAGS = $(CFLAGS) -c -I/usr/include/
TESS_CFLAGS = $(CFLAGS) -c
TESS_LIBS = -L/usr/local/lib/
TESS_LDFLAGS = -ltesseract

main: bin/main.o bin/logging.o bin/api_fetch.o bin/ocrWrapper.o bin/util.o
	$(CC) $(CFLAGS) -I/usr/include/ -o bin/main bin/main.o bin/logging.o bin/api_fetch.o bin/ocrwrapper.o bin/util.o -DBOOST_LOG_DYN_LINK $(LOG_LDFLAGS) $(TESS_LDFLAGS) -llept -lcurl -lboost_date_time

bin/main.o: src/main.cpp include/logging.hpp include/api_fetch.hpp include/ocrWrapper.hpp include/util.hpp
	$(CC) $(CFLAGS) -c -I/usr/include -o bin/main.o src/main.cpp -DBOOST_LOG_DYN_LINK

bin/logging.o: src/logging.cpp include/logging.hpp
	$(CC) $(LOG_CFLAGS) -o bin/logging.o src/logging.cpp -DBOOST_LOG_DYN_LINK

bin/api_fetch.o: src/api_fetch.cpp include/logging.hpp
	$(CC) $(LOG_CFLAGS) -o bin/api_fetch.o src/api_fetch.cpp -DBOOST_LOG_DYN_LINK

bin/ocrWrapper.o: src/ocrWrapper.cpp include/ocrWrapper.hpp include/logging.hpp include/util.hpp
	$(CC) $(LOG_CFLAGS) -o bin/ocrwrapper.o src/ocrWrapper.cpp -DBOOST_LOG_DYN_LINK

bin/util.o: include/util.hpp src/util.cpp
	$(CC) $(CFLAGS) -c -o bin/util.o src/util.cpp -DBOOST_LOG_DYN_LINK

test:
	cd tests; make

clean:
	\rm -f bin/*
