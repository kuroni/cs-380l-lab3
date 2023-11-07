CC := gcc
CFLAGS := -g -Wall
INCFLAGS := -Iinclude

helper.o: src/helper.c include/helper.h
	$(CC) $(CFLAGS) -c $< $(INCFLAGS)

apager.o: src/apager.c include/helper.h
	$(CC) $(CFLAGS) -c $< $(INCFLAGS)

dpager.o: src/dpager.c include/helper.h
	$(CC) $(CFLAGS) -c $< $(INCFLAGS)

hpager.o: src/dpager.c include/helper.h
	$(CC) $(CFLAGS) -c $< $(INCFLAGS)

apager: helper.o apager.o
	$(CC) $(CFLAGS) -o $@ $^

dpager: helper.o apager.o
	$(CC) $(CFLAGS) -o $@ $^

hpager: helper.o apager.o
	$(CC) $(CFLAGS) -o $@ $^

test/*: test/*.c
	$(CC) -static -o $@ $^
