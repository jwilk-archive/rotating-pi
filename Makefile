# Available directives:
#  . M_VERSION
#  . M_DEBUG: yes | no

M_VERSION = 0.3
M_DEBUG = no

FAKEROOT = $(shell command -v fakeroot 2>/dev/null)
CC = gcc
CFLAGS = $(CFLAGS_std) $(CFLAGS_opt) $(CFLAGS_def) $(CFLAGS_lib)
CFLAGS_def := -DVERSION="\"$(M_VERSION)\""
ifeq ($(M_DEBUG),yes)
  CFLAGS_def := $(CFLAGS_def) -DDEBUG
endif

CFLAGS_opt = -O3 -s
CFLAGS_std = -std=c99 -pedantic -Wall -Winline
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
