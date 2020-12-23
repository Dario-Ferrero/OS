CFLAGS = -pedantic -std=c89

master : master.c master.h common.h sorgente
	$(CC) $(CFLAGS) master.c -o master

sorgente : sorgente.c sorgente.h common.h
	$(CC) $(CFLAGS) sorgente.c -o sorgente

clear :
	rm -f *.o master
