CFLAGS += -std=c99 -pedantic -Wall -Wextra
LDFLAGS += -lm

all: rotating-pi

rotating-pi.o: rotating-pi.c color.h pi.h

rotating-pi: rotating-pi.o

test: rotating-pi
	./rotating-pi

clean:
	rm -f rotating-pi *.o

.PHONY: all test clean
