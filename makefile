CC=cc
CFLAGS= -std=c11 -Wall


all: piupiu.app

piupiu.app: structures.o tweet.o vector.o list.o refcnt.o str.o htable.o fman.o main.o
	$(CC) $(CFLAGS) *.o -o piupiu.app

fman.o: src/structures/fman.c
	$(CC) $(CFLAGS) -c src/structures/fman.c

list.o: src/structures/list.c
	$(CC) $(CFLAGS) -c src/structures/list.c

htable.o: src/structures/htable.c
	$(CC) $(CFLAGS) -c src/structures/htable.c

refcnt.o: src/structures/refcnt.c
	$(CC) $(CFLAGS) -c src/structures/refcnt.c

str.o: src/structures/str.c
	$(CC) $(CFLAGS) -c src/structures/str.c

vector.o: src/structures/vector.c
	$(CC) $(CFLAGS) -c src/structures/vector.c

structures.o: src/structures/structures.c
	$(CC) $(CFLAGS) -c src/structures/structures.c

tweet.o: src/tweet.c
	$(CC) $(CFLAGS) -c src/tweet.c

main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c

clean:
	rm -rf *.o
