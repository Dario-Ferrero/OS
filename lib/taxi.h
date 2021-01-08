#ifndef __TAXI_H__
#define __TAXI_H__


/*
 * Distanza di Manhattan tra le posizioni di due celle.
 * Corrisponde al numero di celle che serve attraversare
 * per raggiungere dest a partire da from.
 */
#define MANH_DIST(from, dest) (ABS(GET_X(dest) - GET_X(from)) + ABS(GET_Y(dest) - GET_Y(from)))
#define ABS(n) (n < 0 ? -(n) : n)
#define SEMTIMEDOP(id, num, op, flg, tout)  sops.sem_num = num;		\
									        sops.sem_op = op;		\
									        sops.sem_flg = flg;		\
									        semtimedop(id, &sops, 1, tout);

/* Macro di rappresentazione delle direzioni per un taxi */

#define N_ROADS  4
#define GO_UP    0
#define GO_DOWN  1
#define GO_LEFT  2
#define GO_RIGHT 3

/*
 * Le quattro posizioni adiacenti alla data pos.
 * E' ritornata una posizione invalida (-1) se si è raggiunto
 * il bordo della griglia in quella direzione.
 */

#define UP(pos)    (GET_Y(pos) ? pos - SO_WIDTH : -1)
#define DOWN(pos)  (GET_Y(pos) != SO_HEIGHT-1 ? pos + SO_WIDTH : -1)
#define LEFT(pos)  (GET_X(pos) ? pos - 1 : -1)
#define RIGHT(pos) (GET_X(pos) != SO_WIDTH-1 ? pos + 1 : -1)

#define SAME_ROW(fst, snd)      (GET_Y(fst) == GET_Y(snd))
#define SAME_COLUMN(fst, snd)   (GET_X(fst) == GET_X(snd))
#define ALIGNED(fst, snd)       (SAME_ROW(fst, snd) || SAME_COLUMN(fst, snd))

/*
 * Assegna a next la posizione della cella adiacente a taxi_pos in direzione dir
 */
#define GET_NEXT(next, dir) switch (dir) {				\
        					case GO_UP:					\
            					next = UP(taxi_pos);	\
								break;					\
        					case GO_DOWN:				\
            					next = DOWN(taxi_pos);	\
								break;					\
        					case GO_LEFT:				\
            					next = LEFT(taxi_pos);	\
								break;					\
        					case GO_RIGHT:				\
            					next = RIGHT(taxi_pos);	\
								break;					\
        					default:					\
            					next = -1;				\
								break;					\
    						}

/* 
 * Stima tramite la distanza di Manhattan la posizione della cella sorgente
 * più vicina a taxi_pos tra le n_src presenti in sources_pos (esclusa 'except').
 */
int closest_source(int n_src, int except);

/*
 * Sposta il taxi dalla posizione taxi_pos ad una allineata con goal.
 * Ad ogni iterazione del ciclo, il taxi prova ad accedere ad una cella
 * adiacente in una delle due direzioni che lo avvicinano a goal. 
 */
void drive_diagonal(int goal);

/*
 * Sposta il taxi dalla posizione taxi_pos verso la posizione goal in linea retta.
 * La variabile globale taxi_pos è aggiornata al valore goal se questo è stato
 * raggiunto o ad un altro valore se sulla strada si è trovata
 * (ed evitata tramite 'circle_hole') una cella inaccessibile.
 */
void drive_straight(int goal);

/*
 * Evita una cella inaccessibile adiacente a taxi_pos in direzione dir,
 * muovendo il taxi di una cella in direzione perpendicolare a dir
 * ed in seguito di una cella in direzione dir.
 */
void circle_hole(int8_t dir, int goal);

/*
 * Sposta il taxi dalla cella taxi_pos alla cella adiacente dest situata
 * in direzione dir.
 * Se lo spostamento è avvenuto con successo, taxi_pos è aggiornata a dest.
 * Se il taxi resta fermo per più di SO_TIMEOUT secondi, il processo
 * invia le proprie statistiche al processo master e termina.
 */
void access_cell(int dest, int8_t dir);

/*
 * Invia le proprie statistiche al processo master
 * e termina rilasciando le risorse.
 */
void terminate();

/*
 * Signal handler per il processo.
 */
void handle_signal(int signum);


#endif /* __TAXI_H__ */