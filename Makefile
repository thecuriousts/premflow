CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
PREFIX ?= $(HOME)/.local

all: premflow

premflow: main.o core.o ui.o
	$(CC) $(CFLAGS) -o premflow main.o core.o ui.o

main.o: main.c premflow.h
	$(CC) $(CFLAGS) -c main.c

core.o: core.c premflow.h
	$(CC) $(CFLAGS) -c core.c

ui.o: ui.c premflow.h
	$(CC) $(CFLAGS) -c ui.c

test: test.o core.o
	$(CC) $(CFLAGS) -o test_runner test.o core.o
	./test_runner

install: premflow
	install -d $(PREFIX)/bin
	install -m 755 premflow $(PREFIX)/bin/premflow
	@echo "✅ premflow installed to $(PREFIX)/bin/premflow"

uninstall:
	rm -f $(PREFIX)/bin/premflow
	rm -f $(PREFIX)/share/man/man1/premflow.1
	rm -f $(PREFIX)/share/bash-completion/completions/premflow
	@echo "✅ premflow uninstalled"

clean:
	rm -f *.o premflow test_runner


.PHONY: all clean test install uninstall
