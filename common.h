#ifndef __COMMON_H__
#define __COMMON_H__

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>


typedef struct _Cell {
    long cross_time;
	int msq_id;
    sig_atomic_t cross_n;
	u_int8_t capacity;
    u_int8_t flags;
} Cell;


typedef struct _TaxiStats {
    long mtype;
    pid_t taxi_pid;
    int cells_crossed;
    int reqs_compl;
    unsigned long route_time;
} TaxiStats;


typedef struct _Request {
    long mtype; /* src_cell */
    int dest_cell; /* basta mtype (?) */
} Request;

/*
 * Macro per debugging
 */

#define TEST_ERROR    if (errno) {fprintf(stderr,			\
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
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


/*
 * Valori di flag per una cella
 */

#define SRC_CELL  0x01
#define HOLE_CELL 0x02
#define IS_HOLE(cell)   (cell.flags & HOLE_CELL)
#define IS_SOURCE(cell) (cell.flags & SRC_CELL)


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

/*
 * Genera un intero casualmente incluso tra a e b
 */
#define RAND_RNG(a, b) ((rand() % (b - a + 1)) + a)

/*
 * Stampa colorata a terminale
 */

#define ANSI_YELLOW "\x1b[33m"
#define ANSI_RED "\x1b[31m"
#define ANSI_RESET "\x1b[0m"

#endif /* __COMMON_H__ */