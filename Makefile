CFLAGS = -pedantic -std=c89
DEPS = lib/common.h Makefile

all : out/gridprint.o out/printer out/master out/sorgente out/taxi 

run : all
	./out/master

out/master : src/master.c out/gridprint.o lib/master.h $(DEPS)
	$(CC) $(CFLAGS) src/master.c out/gridprint.o -o out/master

out/sorgente : src/sorgente.c lib/sorgente.h $(DEPS)
	$(CC) $(CFLAGS) src/sorgente.c -o out/sorgente

out/taxi : src/taxi.c lib/taxi.h $(DEPS)
	$(CC) $(CFLAGS) src/taxi.c -o out/taxi

out/printer : src/printer.c out/gridprint.o lib/printer.h $(DEPS)
	$(CC) $(CFLAGS) src/printer.c out/gridprint.o -o out/printer

out/gridprint.o : src/gridprint.c lib/gridprint.h $(DEPS)
	if [ ! -d "out/" ]; then mkdir out/; fi
	$(CC) $(CFLAGS) -c src/gridprint.c -o out/gridprint.o

clear :
	rm -rf out/
