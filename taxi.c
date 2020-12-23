#include "common.h"

void handle_signal(int signum);

int main(int argc, char *argv[])
{
    /* Creato dal master : leggere parametri (posizione, ???) */

    /*
     * Assegnare handle_signal come handler per i segnali che gestisce
     */


    /*
     * Accedere alla griglia
     * shm_id = shmget(getppid(), GRID_SIZE * sizeof(Cell), 0666);
     * grid = (Cell *)shmat(shm_id);
     * 
     * Per ogni cella, se sorgente, salvarne la posizione in un array
     */


    /*
     * Accedere all'array di semafori per sincronizzarsi col master
     * 
     * sem_id = semget(getppid(), NSEMS, 0666);
     */


    /*
     * Inizializzare le proprie variabili (es. TaxiStat)
     */

    
    /*
     * SE coda condivisa : accedere alla coda per l'invio delle richieste
     * 
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
        /* Terminazione forzata : free(), shmdt(), e SE la simulazione è iniziata invia stats al master */
        break;
    case SIGALRM:
        /* Timeout : chiudi tutto come sopra, ma exit status diverso */
        break;
    }
}