#include "common.h"
#include "sorgente.h"

int sem_id, msq_id;
struct sembuf sops;

int main(int argc, char *argv[])
{
    int pos, shm_id;
    struct sigaction sa;
    Cell *city_grid;

    /* 
     * Processo creato dal master, leggere i parametri passati tramite execve
     * - posizione della propria cella
     * - altri (?)
     */

    pos = atoi(argv[1]);
    fprintf(stderr, "Sorgente creata! La mia posizione è : %d\n", pos);

    /* Gestire maschere / stabilire l'handler per i 3/4 segnali */

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);


    /*
     * Accedere all'array di semafori per sincronizzarmi col master.
     */

    if ((sem_id = semget(getppid(), NSEMS, 0666)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }

    /*
     * Accedere alla mia coda di messaggi per le richieste
     */

    if ((msq_id = msgget(IPC_PRIVATE, 0666)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }

    /*
     * Devo scrivere l'id della coda nella sua cella, così che i taxi vi possano accedere 
     */

    if ((shm_id = shmget(getppid(), GRID_SIZE * sizeof(*city_grid), 0600)) == -1) {
        TEST_ERROR;
        terminate();
    }
    city_grid = (Cell *)shmat(shm_id, NULL, 0);
    TEST_ERROR;
    city_grid[pos].msq_id = msq_id;

    /* In teoria la memoria condivisa non serve più al processo */
    shmdt(city_grid);


    /*
     * Quando sono pronto per la simulazione faccio signal su SEM_KIDS e wait for zero su SEM_START
     */

    SEMOP(sem_id, SEM_KIDS, 1, 0);
    TEST_ERROR;

    SEMOP(sem_id, SEM_START, 0, 0);
    TEST_ERROR;

    terminate();


    /* Simulazione iniziata, posso entrare nel ciclo infinito di generazione di richieste */

    while (1) {
        break;

        /*
         * * Faccio partire l'alarm(1)
         * * Attesa attiva o pause() ? (di fatto non ho nulla da fare...)
         */
    }
}

void handle_signal(int signum)
{
    switch (signum)
    {
    case SIGINT: /* Ctrl-C da terminale */
    case SIGTERM: /* "End Process" nel manager di sistema (NO KILL) */
        /* Terminazione forzata : free(), shmdt(), invia qualcosa al master (?)
         * se coda propria della sorgente, inviare al master il n di richieste ancora inevase
         * (exit status o messaggio sulla coda delle statistiche), poi rimuovi la coda
         */
        terminate();
        break;
    
    /*
     * PROBLEMA :
     * Sorgente in attesa per inserimento di 1+ richieste --> arrivo di altri segnali
     * * quali mascherare (e quali no) nell'handler ?
     */
    case SIGALRM:
        /* Genera N_REQS richieste ed inseriscile nella coda */
        break;
    case SIGUSR1:
        /* Genera una richiesta ed inseriscila nella coda */
        break;
    default:
        break;
    }
}


void terminate()
{
    msgctl(msq_id, IPC_RMID, NULL);
    exit(EXIT_FAILURE);
}