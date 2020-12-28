#ifndef __TAXI_H__
#define __TAXI_H__

#include <time.h>

#define ABS(n) (n < 0 ? -(n) : n)
#define MANH_DIST(from, dest) (ABS(GET_X(dest) - GET_X(from)) + ABS(GET_Y(dest) - GET_Y(from)))

/*
 * Le quattro posizioni adiacenti alla data pos.
 * E' ritornata una posizione invalida (-1) se si è raggiunto
 * il bordo della griglia in quella direzione.
 */

#define UP(pos)    (GET_Y(pos) ? pos - SO_WIDTH : -1)
#define DOWN(pos)  (GET_Y(pos) != SO_HEIGHT-1 ? pos + SO_WIDTH : -1)
#define LEFT(pos)  (GET_X(pos) ? pos - 1 : -1)
#define RIGHT(pos) (GET_X(pos) != SO_WIDTH-1 ? pos + 1 : -1)

/* 
 * Stima la posizione della cella sorgente più vicina a cur_pos tra le
 * n_src presenti in src_pos tramite la distanza di Manhattan.
 */
int closest_source(int cur_pos, int *src_pos, int n_src);

/*
 * Sposta il taxi dalla posizione start alla posizione goal
 * e ritorna la posizione raggiunta
 */
int drive(int start, int goal);

/*
 * Sposta il taxi dalla cella from alla cella adiacente dest di city_grid.
 * Ritorna dest se lo spostamento è avvenuto con successo, from altrimenti.
 */
int single_move(int from, int dest);

/*
 * Signal handler per il processo
 */
void handle_signal(int signum);


#endif /* __TAXI_H__ */