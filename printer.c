#include "common.h"
#include "printer.h"

Cell *city_grid;
int sem_id, shm_id;
struct sembuf sops;

int main(int argc, char *argv[])
{
    struct sigaction sa;

    /* Creato dal master, deve leggere parametri (?) */

    fprintf(stderr, "Processo printer creato!\n");

    /* Gestire maschere e segnali */

    sa.sa_handler = handle_signal;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    /* Accedere all'array di semafori */

    if ((sem_id = semget(getppid(), NSEMS, 0666)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }

    /* Accedere alla griglia di celle */

    if ((shm_id = shmget(getppid(), GRID_SIZE * sizeof(*city_grid), 0600)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }
    city_grid = (Cell *)shmat(shm_id, NULL, 0);
    TEST_ERROR;


    /* Finito inizializzazione, può partire la simulazione */

    SEMOP(sem_id, SEM_KIDS, 1, 0);
    SEMOP(sem_id, SEM_START, 0, 0);
    
    shmdt(city_grid);
}


void handle_signal(int signum)
{
    switch (signum)
    {
    case SIGALRM:
        break;
    case SIGINT:
    case SIGTERM:
        break;
    default:
        break;
    }
}


void terminate()
{
    shmdt(city_grid);
    exit(EXIT_FAILURE);
}