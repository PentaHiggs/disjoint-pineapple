BASE_DIR = $(abspath $(dir $(lastword $$PWD))/..)
EXEC_DIR = $(BASE_DIR)/bin
INCL_DIR = $(BASE_DIR)/include
# Compiler
CC = g++
CFLAGS = -g -Wall -Wextra -std=c++14 -Wl,--as-needed -I$(BASE_DIR)/include

# Libraries needed
TEST_LDFLAGS = -lboost_unit_test_framework

util_test: util_test.cpp
	cd $(BASE_DIR); make bin/util.o
	$(CC) $(CFLAGS) -I/usr/include -o $(EXEC_DIR)/util_test $(EXEC_DIR)/util.o $(INCL_DIR)/util.hpp util_test.cpp $(TEST_LDFLAGS)
	
clean:
	\rm -f bin/*
