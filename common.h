#ifndef __COMMON_H__
#define __COMMON_H__

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>

/*
 * Cella della griglia dove si spostano i taxi
 */

typedef struct _Cell {
    long cross_time;
	int msq_id;
    sig_atomic_t cross_n;
	u_int8_t capacity;
    u_int8_t flags;
} Cell;

/*
 * Strutture dati da inviare nelle code di messaggi
 */

typedef struct _TaxiStats {
    long mtype;
	unsigned long route_time;
    pid_t taxi_pid;
    int16_t cells_crossed;
    int16_t reqs_compl;
} TaxiStats;

typedef struct _SourceStats {
	long mtype;
	u_int64_t reqs_unpicked;
} SourceStats;

typedef struct _Request {
    long mtype; 	/* posizione della cella di origine + 1 */
    int dest_cell;
} Request;

/*
 * Macro per debugging
 * 
 * TEST_ERROR presa dagli esempi di laboratorio
 */

#define TEST_ERROR    if (errno) {fprintf(stderr,			\
					  "%s:%d: PID=%5d: Errore %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}

#define PRINT_INT(n) fprintf(stderr, "%s:%i: %s = %i\n", __FILE__, __LINE__, #n, n)
#define PRINT_LONG_I(n) fprintf(stderr, "%s:%i: %s = %li\n", __FILE__, __LINE__, #n, n)


/*
 * Macro per l'accesso alla griglia
 */

#define SO_WIDTH  20
#define SO_HEIGHT 10
#define GRID_SIZE (SO_WIDTH * SO_HEIGHT)
#define INDEX(x, y) (x + y * SO_WIDTH)
#define GET_X(pos) (pos % SO_WIDTH)
#define GET_Y(pos) ((pos - GET_X(pos)) / SO_WIDTH)
#define IS_BORDER(pos) (pos < 0 || pos >= GRID_SIZE)


/* Valori di flag per una cella */

#define SRC_CELL  0x01
#define HOLE_CELL 0x02
#define IS_HOLE(cell)   (cell.flags & HOLE_CELL)
#define IS_SOURCE(cell) (cell.flags & SRC_CELL)

/* Valori di mtype per le struct di statistiche */

#define REQ_SUCC_MTYPE 1
#define REQ_ABRT_MTYPE 2
#define SOURCE_MTYPE 3
#define MSG_LEN(msg) (sizeof(msg) - sizeof((msg).mtype))

/* Valori possibili di exit status */

#define EXIT_TAXI 3

/*
 * Macro per la gestione dei semafori
 * Valori in [0, GRID_SIZE-1] usati per gestire l'accesso alle celle
 */

#define NSEMS  	  (GRID_SIZE + 3)
#define SEM_START  GRID_SIZE
#define SEM_KIDS  (GRID_SIZE + 1)
#define SEM_PRINT (GRID_SIZE + 2)
#define SEMOP(id, num, op, flg)		sops.sem_num = num;		\
									sops.sem_op = op;		\
									sops.sem_flg = flg;		\
									semop(id, &sops, 1);

/*
 * Altre macro
 */

#define TRUE  1
#define FALSE 0
#define SWAP(a, b, tmp)  tmp = a; 	\
						 a = b;	  	\
						 b = tmp;

/*
 * Genera un intero casualmente incluso tra a e b
 */
#define RAND_RNG(a, b) ((rand() % (b - a + 1)) + a)

#endif /* __COMMON_H__ */