#ifndef __MASTER_H__
#define __MASTER_H__

#define PARAMS_FILE "test.conf"
#define READ_LEN (19 + 16)
#define N_PARAMS 10

#define SRC_FILE "./sorgente"
#define TAXI_FILE "./taxi"
#define PRINTER_FILE "./printer"
#define BUF_SIZE 10


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
 * e ne salva le posizioni nell'array sources_pos
 */
void assign_sources();

/*
 * Crea ed inizializza l'array di semafori, salvandone l'id in sem_id.
 */
void init_sems();

/*
 * Crea SO_SOURCES processi sorgente, passando ad ognuno una posizione nella
 * griglia tra quelle in sources_pos.
 */
void create_sources();

/*
 * Crea n_taxis processi taxi, ognuno in una cella casuale
 * e con almeno un posto libero. Salva i nuovi pid nell'array globale taxis.
 */
void create_taxis(int n_taxis);

/* 
 * Crea il processo che si occupa della stampa ad ogni secondo della simulazione
 */
void create_printer();

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
 * Termina i primi nkids processi figli in kids, per poi liberare
 * la memoria allocata dall'array
 */
void term_kids(pid_t *kids, int nkids);

/*
 * Termina i processi figli, ne raccoglie le statistiche e
 * stampa a terminale i risultati della simulazione
 */
void end_simulation();

/*
 * Stampa i process id dei tre singoli taxi che hanno attraversato più celle,
 * viaggiato più a lungo e soddisfatto più richieste.
 */
void print_best_taxis();

/*
 * Rilascia le risorse IPC e termina
 */
void terminate();


#endif /* __MASTER_H__ */