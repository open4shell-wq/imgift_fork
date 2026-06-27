CC = clang
BINARIES = bin2png png2bin
OBJECTS = common.o imgify.o

# cut is necessary for Cygwin
PLATFORM_OS := $(shell uname | cut -d_ -f1)

.PHONY: all clean test

all: $(BINARIES)

clean:
	@rm -rf *.o $(BINARIES)

test: all
	sh tests/run.sh

CFLAGS += -D_XOPEN_SOURCE=600 -std=c99 -Wall -Wextra
LDFLAGS += -lpng

ifeq ($(PLATFORM_OS), Linux)
	LDFLAGS += -lm # required for sqrt()
endif

common.o: common.c common.h
	$(CC) -o $@ -c common.c $(CFLAGS)

imgify.o: imgify.c imgify.h
	$(CC) -o $@ -c imgify.c $(CFLAGS)

bin2png: bin2png.c $(OBJECTS)
	$(CC) -o $@ bin2png.c $(OBJECTS) $(CFLAGS) $(LDFLAGS)

png2bin: png2bin.c $(OBJECTS)
	$(CC) -o $@ png2bin.c $(OBJECTS) $(CFLAGS) $(LDFLAGS)
