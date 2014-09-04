CFLAGS=$(shell ncursesw5-config --cflags) -Wall -std=c99 -I.
LDLIBS=$(shell ncursesw5-config --libs)

.PHONY: all clean

all: fast

clean:
	$(RM) fast

