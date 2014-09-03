CFLAGS=$(shell pkg-config --flags ncurses)
LDLIBS=$(shell pkg-config --libs ncurses)

.PHONY: all

all: fast
