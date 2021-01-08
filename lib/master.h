#ifndef __MASTER_H__
#define __MASTER_H__

/* Macro per la lettura dei parametri di configurazione */

#define PARAMS_FILE "conf/dense.conf"
#define READ_LEN (19 + 16)
#define N_PARAMS 10

/*
 * Minimo rapporto possibile tra il numero di celle totale
 * ed il numero di celle inaccessibili.
 * Poichè il numero di holes generabile dipende anche dalle proporzioni
 * della griglia, il valore corrente è una precauzione e garantisce il
 * funzionamento per ogni configurazione rispettante il vincolo
 * GRID_SIZE / SO_HOLES >= SIZE_HOLES_RATIO.
 */
#define SIZE_HOLES_RATIO 9

/* Macro per la generazione dei processi figli */

#define SRC_FILE "./out/sorgente"
#define TAXI_FILE "./out/taxi"
#define PRINTER_FILE "./out/printer"
#define BUF_SIZE 10

/*
 * Richieste generate da una sorgente per un certo intervallo di tempo.
 * Passato come argomento ad ogni processo sorgente creato.
 */
#define REQS_RATE (SO_TAXI / SO_SOURCES)

/* Parametri di configurazione, letti a run-time */

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
 * Legge il file PARAMS_FILE ed inizializza le variabili globali SO.
 */
void read_params();

/*
 * Controlla i valori dei parametri SO e termina
 * nel caso ve ne siano di non validi.
 */
void check_params();

/*
 * Inizializza le celle della city_grid in memoria condivisa.
 * SO_HOLES di queste sono marcate come inaccessibili.
 */
void init_city_grid();

/*
 * Ritorna TRUE se almeno una cella adiacente a city_grid[pos] è una cella inaccessibile.
 */
int check_adj_cells(long pos);

/*
 * Assegna SO_SOURCES celle in city_grid il ruolo di sorgente
 * e ne salva le posizioni nell'array globale sources_pos.
 */
void assign_sources();

/*
 * Crea ed inizializza l'array di semafori, salvandone l'id in sem_id.
 */
void init_sems();

/*
 * Crea SO_SOURCES processi sorgente, mantenendone i process id
 * nell'array globale sources.
 */
void create_sources();

/*
 * Crea n_taxis processi taxi, ognuno in una cella casuale
 * con almeno un posto libero. Salva i nuovi pid nell'array globale taxis.
 */
void create_taxis(int n_taxis);

/* 
 * Crea il processo printer che si occupa della stampa ad ogni secondo della simulazione.
 */
void create_printer();

/*
 * Signal handler per il processo.
 */
void handle_signal(int signum);

/*
 * Termina nkids processi figli in kids, per poi liberare
 * la memoria allocata dall'array.
 */
void term_kids(pid_t *kids, int nkids);

/*
 * Termina i processi figli, ne raccoglie le statistiche e
 * stampa ad output i risultati della simulazione.
 */
void end_simulation();

/*
 * Raccoglie nell'array globale tstats le statistiche inviate dai taxi terminati.
 * Se il valore di ntaxis è positivo, per ogni statistica raccolta
 * sono creati ntaxis nuovi processi taxi.
 */
void collect_taxi_stats(int ntaxis);

/*
 * Stampa i process id dei tre singoli taxi che hanno attraversato più celle,
 * viaggiato più a lungo e soddisfatto più richieste.
 */
void print_best_taxis();

/*
 * Salva nell'array puntato da top_cells le posizioni delle SO_TOP_CELLS celle
 * più attraversate (che non siano celle sorgenti).
 * Usa un adattamento dell'algoritmo di insertion_sort.
 */
void get_top_cells(int **top_cells);

/*
 * Rilascia le risorse IPC e termina.
 */
void terminate();

#endif /* __MASTER_H__ */