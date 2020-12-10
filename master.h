#ifndef __MASTER_H__
#define __MASTER_H__

#define PARAMS_FILE "params.conf"
#define READ_LEN (19 + 16)
#define N_PARAMS 10

/*
 * Signal handler per il processo
 */
void handle_signal(int signum);

/*
 * Legge il file specificato ed inizializza le variabili globali SO
 */
void read_params();

/*
 * Inizializza la city_grid in memoria condivisa e ne ritorna l'id
 */
int init_city_grid();

/*
 * Ritorna TRUE se almeno una cella adiacente a city_grid[pos] è una HOLE_CELL
 */
int check_adj_cells(long pos);

/*
 * Stampa a terminale, per ogni cella di city_grid, il valore dei suoi campi
 */
void print_grid_values();

/*
 * Stampa a terminale la city_grid
 */
void print_grid();


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