#ifndef __TAXI_H__
#define __TAXI_H__

#define ABS(n) (n < 0 ? -(n) : n)
#define MANH_DIST(from, dest) (ABS(GET_X(dest) - GET_X(from)) + ABS(GET_Y(dest) - GET_Y(from)))

/*
 * Le quattro posizioni adiacenti alla data pos.
 * E' ritornata una posizione invalida (-1) se si è raggiunto
 * il bordo della griglia in quella direzione.
 */

#define N_ROADS 4
#define UP(pos)    (GET_Y(pos) ? pos - SO_WIDTH : -1)
#define DOWN(pos)  (GET_Y(pos) != SO_HEIGHT-1 ? pos + SO_WIDTH : -1)
#define LEFT(pos)  (GET_X(pos) ? pos - 1 : -1)
#define RIGHT(pos) (GET_X(pos) != SO_WIDTH-1 ? pos + 1 : -1)

#define GO_UP    0
#define GO_DOWN  1
#define GO_LEFT  2
#define GO_RIGHT 3

#define SAME_ROW(fst, snd)      (GET_Y(fst) == GET_Y(snd))
#define SAME_COLUMN(fst, snd)   (GET_X(fst) == GET_X(snd))
#define ALIGNED(fst, snd)       (SAME_ROW(fst, snd) || SAME_COLUMN(fst, snd))


/* 
 * Stima tramite la distanza di Manhattan la posizione della cella sorgente
 * più vicina a taxi_pos tra le n_src presenti in sources_pos (esclusa 'except').
 */
int closest_source(int n_src, int except);

/*
 * Sposta il taxi dalla posizione taxi_pos, portandolo su una posizione
 * allineata a goal. Ritorna la posizione raggiunta.
 */
int drive_diagonal(int goal);

/*
 * Sposta il taxi dalla posizione taxi_pos verso la posizione goal in linea retta.
 * Ritorna la posizione raggiunta :
 *  - goal : se il taxi ha raggiunto la destinazione.
 *  - altrimenti : una cella inaccessibile è stata incontrata ed evitata
 *                 tramite la funzione circle_hole() 
 */
int drive_straight(int goal);

/*
 * Evita una cella inaccessibile adiacente a taxi_pos in direzione dir,
 * muovendo il taxi in una cella in direzione perpendicolare a dir
 * ed in seguito in una cella in direzione dir.
 */
int circle_hole(int8_t dir, int goal);

/*
 * Ritorna la posizione rispetto a taxi_pos della cella in direzione dir.
 */
int get_road(int8_t dir);

/*
 * Sposta il taxi dalla cella taxi_pos alla cella adiacente dest di city_grid.
 * Ritorna dest se lo spostamento è avvenuto con successo, taxi_pos altrimenti.
 */
int access_cell(int dest);

/*
 * Signal handler per il processo
 */
void handle_signal(int signum);


#endif /* __TAXI_H__ */