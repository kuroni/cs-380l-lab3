CC := gcc
CFLAGS := -g -Wall
INCFLAGS := -Iinclude

.PHONY: clean

bin/%.o: src/%.c include/helper.h
	$(CC) $(CFLAGS) -c -o $@ $< $(INCFLAGS)

%: bin/%.o bin/helper.o
	$(CC) $(CFLAGS) -o $@ $^

test/%: test/%.c
	$(CC) -static -o $@ $^

clean:
	rm $(patsubst src/%.c, bin/%.o, $(wildcard src/*.c)) || \
	rm $(patsubst src/%.c, ./%, $(wildcard src/*.c)) || \
	rm $(patsubst test/%.c, test/%, $(wildcard test/*.c)) || \
	true
