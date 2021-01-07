CFLAGS = -pedantic -std=c89

all : master sorgente taxi printer gridprint.o

run : all
	./out/master

master : src/master.c gridprint.o lib/master.h lib/common.h Makefile
	$(CC) $(CFLAGS) src/master.c out/gridprint.o -o out/master

sorgente : src/sorgente.c lib/sorgente.h lib/common.h Makefile
	$(CC) $(CFLAGS) src/sorgente.c -o out/sorgente

taxi : src/taxi.c lib/taxi.h lib/common.h Makefile
	$(CC) $(CFLAGS) src/taxi.c -o out/taxi

printer : src/printer.c out/gridprint.o lib/printer.h lib/common.h Makefile
	$(CC) $(CFLAGS) src/printer.c out/gridprint.o -o out/printer

gridprint.o : src/gridprint.c lib/gridprint.h lib/common.h Makefile
	if [ ! -d "./out/" ]; then mkdir out/; fi
	$(CC) $(CFLAGS) -c src/gridprint.c -o out/gridprint.o

clear :
	rm -f out/*