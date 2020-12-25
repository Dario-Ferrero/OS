#ifndef __MASTER_H__
#define __MASTER_H__

#define PARAMS_FILE "dense.conf"
#define READ_LEN (19 + 16)
#define N_PARAMS 10

#define SRC_FILE "./sorgente"
#define TAXI_FILE "./taxi"


/*
 * Legge il file specificato ed inizializza le variabili globali SO
 */
void read_params();

/*
 * Controlla i valori dei parametri SO e termina
 * nel caso ve ne siano di non validi
 */
void check_params();

/*
 * Inizializza le celle della city_grid in memoria condivisa.
 * SO_HOLES di queste sono marcate come inaccessibili.
 */
void init_city_grid();

/*
 * Ritorna TRUE se almeno una cella adiacente a city_grid[pos] è una HOLE_CELL
 */
int check_adj_cells(long pos);

/*
 * Assegna SO_SOURCES celle in city_grid il ruolo di sorgente
 * e ne salva le posizioni nell'array puntato da sources
 */
void assign_sources(int **sources);

/*
 * Stampa a terminale, per ogni cella di city_grid, il valore dei suoi campi
 */
void print_grid_values();

/*
 * Stampa a terminale la city_grid
 */
void print_grid();

/*
 * Signal handler per il processo
 */
void handle_signal(int signum);

/*
 * Termina i processi figli
 */
void term_kids();

/*
 * Rilascia le risorse IPC e termina
 */
void terminate();

/*
 * Parametri di configurazione, letti a run-time
 */

int SO_HOLES;
int SO_TOP_CELLS;
int SO_SOURCES;
int SO_CAP_MIN;
int SO_CAP_MAX;
int SO_TAXI;
int SO_TIMENSEC_MIN;
int SO_TIMENSEC_MAX;
int SO_TIMEOUT;
int SO_DURATION;


#endif /* __MASTER_H__ */