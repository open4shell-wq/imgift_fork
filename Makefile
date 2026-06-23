# cut is necessary for Cygwin
PLATFORM_OS := $(shell uname | cut -d_ -f1)
CC ?= cc

all: bin2png png2bin

clean:
	@rm -rf *.o bin2png png2bin tests/*.o tests/test_common tests/test_common_options tests/test_imgify

CFLAGS += -D_XOPEN_SOURCE=600 -std=c99 -Wall -Wextra
LDFLAGS += -lpng

ifeq ($(PLATFORM_OS), Linux)
	LDFLAGS += -lm # required for sqrt()
endif

common.o: common.h
	echo $(PLATFORM_OS)
	$(CC) -o $@ -c common.c $(CFLAGS)

imgify.o: imgify.h
	$(CC) -o $@ -c imgify.c $(CFLAGS)

bin2png: common.o imgify.o
	$(CC) -o $@ bin2png.c $^ $(CFLAGS) $(LDFLAGS)

png2bin: common.o imgify.o
	$(CC) -o $@ png2bin.c $^ $(CFLAGS) $(LDFLAGS)

tests/test_common: tests/test_common.c common.o
	$(CC) -o $@ $^ $(CFLAGS)

tests/test_common_options: tests/test_common_options.c
	$(CC) -o $@ $^ $(CFLAGS)

tests/test_imgify: tests/test_imgify.c imgify.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test: tests/test_common tests/test_common_options tests/test_imgify
	./tests/test_common
	./tests/test_common_options
	./tests/test_imgify

.PHONY: all clean test
