CC = gcc
CFLAGS = -Wall -std=gnu99 -pedantic -g
MAIN = mush
OBJS = mush.o mush_functions.o parseline_functions.o

all: $(MAIN)

$(MAIN): $(OBJS) mush.h
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

mush.o: mush.h
	$(CC) $(CFLAGS) -c mush.c

mush_functions.o: mush.h
	$(CC) $(CFLAGS) -c mush_functions.c

parseline_functions.o: parseline.h
	$(CC) $(CFLAGS) -c parseline_functions.c

clean:
	rm -f *.o $(OBJS)
