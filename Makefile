UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CFLAGS=$(shell ncursesw5-config --cflags) -Wall -std=c99 -I.
	LDLIBS=$(shell ncursesw5-config --libs)
endif
ifeq ($(UNAME_S),Darwin)
	CFLAGS=-I/usr/include/ncurses -Wall -std=c99 -I.
	LDLIBS=-lncurses
endif


.PHONY: all clean

all: fast

clean:
	$(RM) fast

