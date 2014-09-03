CFLAGS=$(shell pkg-config --cflags ncurses) -Wall -std=c99
LDLIBS=$(shell pkg-config --libs ncurses)

.PHONY: all clean

all: fast

clean:
	$(RM) fast
