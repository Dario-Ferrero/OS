CFLAGS = -pedantic -std=c89

taxi.o : taxi.c
	$(CC) $(CFLAGS) -c taxi.c -o taxi.o

sorgente.o : sorgente.c
	$(CC) $(CFLAGS) -c sorgente.c -o sorgente.o

master : master.c sorgente.o taxi.o
	$(CC) $(CFLAGS) master.c -o master

