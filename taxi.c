#include "common.h"
#include "taxi.h"

int sem_id, shm_id;
struct sembuf sops;

int main(int argc, char *argv[])
{
    int i, pos, cnt, SO_SOURCES, *src_pos;
    Cell *city_grid;
    struct sigaction sa;

    /* Creato dal master : leggere parametri (posizione, SO_SOURCES, ???) */

    pos = atoi(argv[1]);
    SO_SOURCES = atoi(argv[2]);
    fprintf(stderr, "Taxi creato! La mia posizione è %4d, SO_SOURCES è %d\n", pos, SO_SOURCES);

    /*
     * Assegnare handle_signal come handler per i segnali che gestisce
     */
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    /*
     * Accedere all'array di semafori per sincronizzarsi col master
     */

    if ((sem_id = semget(getppid(), NSEMS, 0666)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }

    SEMOP(sem_id, SEM_KIDS, 1, 0);
    TEST_ERROR;

    SEMOP(sem_id, SEM_START, 0, 0);
    TEST_ERROR;

#if 0

    /*
     * Accedere alla griglia, e per ogni cella sorgente salvarne la posizione in un array
     */

    if ((shm_id = shmget(getppid(), GRID_SIZE * sizeof(*city_grid), 0600)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }
    city_grid = (Cell *)shmat(shm_id, NULL, 0);
    TEST_ERROR;
    src_pos = (int *)calloc(SO_SOURCES, sizeof(*src_pos));
    for (i = 0, cnt = 0; i < GRID_SIZE; i++) {
        if (IS_SOURCE(city_grid[i])) {
            src_pos[cnt++] = i;
        }
    }
#endif

    /*
     * Inizializzare le proprie variabili (es. TaxiStat)
     */

    
    /*
     * Accedere alla coda per l'invio delle statistiche
     */


    /*
     * Pronto ? Allora signal su SEM_KIDS e wait for zero su SEM_START
     */

    /* Simulazione iniziata (ciclo abbastanza contorto, da pensare più in dettaglio) */
    while (1) {
        /*
         * Sequenza varia a seconda di 1 o più msgq per ogni sorgente, ma le azioni sono :
         * - trovare la posizione più vicina tramite manhattan distance
         * - prelevare una richiesta
         * - entrare nel ciclo di spostamento (funzione?) prima a src_cell, poi a dest_cell
         * - mano a mano : tenere traccia dei valori per la stampa finale
         */
        break;
    }

}


void handle_signal(int signum)
{
    switch (signum) {
    case SIGINT:
    case SIGTERM:
        exit(EXIT_FAILURE);
        /* Terminazione forzata : free(), shmdt(), e SE la simulazione è iniziata invia stats al master */
        break;
    case SIGALRM:
        /* Timeout : chiudi tutto come sopra, ma exit status diverso */
        break;
    }
}