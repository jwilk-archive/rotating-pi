CFLAGS += -std=gnu99 -pedantic -Wall -Winline
LDFLAGS += -lm

all: rhotate

rhotate.o: rhotate.c color.h pi.h

rhotate: rhotate.o

test: rhotate
	./rhotate

clean:
	rm -f rhotate *.o

.PHONY: all test clean
