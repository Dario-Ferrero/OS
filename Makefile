CFLAGS = -pedantic -std=c89

all : master sorgente taxi printer gridprint.o

run : all
	./master

master : master.c gridprint.o master.h common.h Makefile
	$(CC) $(CFLAGS) master.c gridprint.o -o master

sorgente : sorgente.c sorgente.h common.h Makefile
	$(CC) $(CFLAGS) sorgente.c -o sorgente

taxi : taxi.c taxi.h common.h Makefile
	$(CC) $(CFLAGS) taxi.c -o taxi

printer : printer.c gridprint.o printer.h common.h Makefile
	$(CC) $(CFLAGS) printer.c gridprint.o -o printer

gridprint.o : gridprint.c gridprint.h common.h Makefile
	$(CC) $(CFLAGS) -c gridprint.c -o gridprint.o

clear :
	rm -f *.o master sorgente taxi printer