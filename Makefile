CFLAGS = -pedantic -std=c89

master : master.c master.h common.h sorgente taxi
	$(CC) $(CFLAGS) master.c -o master

sorgente : sorgente.c sorgente.h common.h
	$(CC) $(CFLAGS) sorgente.c -o sorgente

taxi : taxi.c taxi.h common.h
	$(CC) $(CFLAGS) taxi.c -o taxi

clear :
	rm -f *.o master
