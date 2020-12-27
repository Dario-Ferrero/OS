CFLAGS = -pedantic -std=c89

all : master sorgente taxi printer

run : all
	./master

master : master.c master.h common.h Makefile
	$(CC) $(CFLAGS) master.c -o master

sorgente : sorgente.c sorgente.h common.h Makefile
	$(CC) $(CFLAGS) sorgente.c -o sorgente

taxi : taxi.c taxi.h common.h Makefile
	$(CC) $(CFLAGS) taxi.c -o taxi

printer : printer.c printer.h common.h Makefile
	$(CC) $(CFLAGS) printer.c -o printer

clear :
	rm -f *.o master sorgente taxi printer