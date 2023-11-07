CC := gcc
CFLAGS := -g -Wall
INCFLAGS := -Iinclude

.PHONY: test clean

helper.o: src/helper.c include/helper.h
	$(CC) $(CFLAGS) -c $< $(INCFLAGS)

apager.o: src/apager.c include/helper.h
	$(CC) $(CFLAGS) -c $< $(INCFLAGS)

dpager.o: src/dpager.c include/helper.h
	$(CC) $(CFLAGS) -c $< $(INCFLAGS)

hpager.o: src/hpager.c include/helper.h
	$(CC) $(CFLAGS) -c $< $(INCFLAGS)

apager: helper.o apager.o
	$(CC) $(CFLAGS) -o $@ $^

dpager: helper.o dpager.o
	$(CC) $(CFLAGS) -o $@ $^

hpager: helper.o hpager.o
	$(CC) $(CFLAGS) -o $@ $^

test: test/*.c
	$(CC) -static -o test/simple test/simple.c
	$(CC) -static -o test/static_array test/static_array.c
	$(CC) -static -o test/random_access test/random_access.c

clean:
	rm -r *.o || \
	rm apager dpager hpager || \
	rm test/random_access test/simple test/static_array
