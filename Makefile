CFLAGS=$(shell pkg-config --flags ncurses) -Wall
LDLIBS=$(shell pkg-config --libs ncurses)

.PHONY: all clean

all: fast

clean:
	$(RM) fast
