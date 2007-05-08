DEBUG = no

CC = gcc
CFLAGS = $(CFLAGS_std) $(CFLAGS_opt) $(CFLAGS_def) $(CFLAGS_lib)
ifeq ($(DEBUG),yes)
  CFLAGS_def := $(CFLAGS_def) -DDEBUG
endif

CFLAGS_opt = -O3 -s
CFLAGS_std = -std=gnu99 -pedantic -Wall -Winline
CFLAGS_lib = -lm

all: rhotate

rhotate: rhotate.c color.h pi.h
	$(CC) $(CFLAGS) rhotate.c -o rhotate

test: rhotate
	./rhotate

stats:
	@echo $(shell cat *.c *.h | wc -l) lines.
	@echo $(shell cat *.c *.h | wc -c) bytes.

clean:
	rm -f rhotate

.PHONY: all test stats clean
