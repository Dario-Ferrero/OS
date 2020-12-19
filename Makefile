CFLAGS = -pedantic -std=c89

master : master.h common.h master.c
	$(CC) $(CFLAGS) master.c -o master

clear :
	rm -f *.o master
