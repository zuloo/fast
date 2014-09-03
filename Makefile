CFLAGS=$(shell pkg-config --cflags ncurses) -Wall
LDLIBS=$(shell pkg-config --libs ncurses)

.PHONY: all clean

all: fast

clean:
	$(RM) fast
