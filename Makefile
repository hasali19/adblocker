CC      = gcc
CFLAGS  = -Wall -g -std=c11 -Werror -pedantic
LDLIBS  = -lcurl

.SUFFIXES: .c .o

.PHONY: all clean

dnsblocker_headers = $(wildcard ./src/*.h)
dnsblocker_objects = $(patsubst %.c,%.o,$(wildcard ./src/*.c))

all: dnsblocker

dnsblocker: $(dnsblocker_objects) $(dnsblocker_headers)
	$(CC) $(CFLAGS) $(dnsblocker_objects) $(LDLIBS) -o dnsblocker

clean:
	rm -f src/*.o
	rm -f dnsblocker
