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


typedef struct _Cell {
    long cross_time;
    u_int8_t capacity;
    sig_atomic_t cross_n;
    u_int8_t flags;
} Cell;


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
 * Valori in [0, GRID_SIZE-1] usati per l'accesso alle celle
 */

#define NSEMS  	  (GRID_SIZE + 2)
#define SEM_START  GRID_SIZE
#define SEM_KIDS  (GRID_SIZE + 1)



/*
 * Altre macro
 */

#define TRUE  1
#define FALSE 0
#define RAND_RNG(a, b) ((rand() % (b - a + 1)) + a)

#endif /* __COMMON_H__ */
