CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -pedantic
SOURCES = src/server.c src/http.c src/static.c src/cgi.c
.PHONY: all clean test
all: sws
sws: $(SOURCES) src/sws.h
	$(CC) $(CFLAGS) -o $@ $(SOURCES)
test: sws
	python3 tests/test_server.py
	python3 tests/test_more.py
clean:
	rm -f sws
