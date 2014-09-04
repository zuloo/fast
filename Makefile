CFLAGS=$(shell pkg-config --cflags ncursesw) -std=c99 -I.
LDLIBS=$(shell pkg-config --libs ncursesw)

.PHONY: all clean

all: fast

clean:
	$(RM) fast
