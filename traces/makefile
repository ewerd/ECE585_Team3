CC=gcc
CFLAGS= -Wall -std=c99

all: writeTrace

help:
	@echo all, writeTrace, clean

writeTrace:writeTrace.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	@rm -v -f writeTrace
