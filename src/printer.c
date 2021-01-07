#include "../lib/common.h"
#include "../lib/printer.h"
#include "../lib/gridprint.h"

Cell *city_grid;
int sem_id, shm_id;
struct sembuf sops;

int main(int argc, char *argv[])
{
    struct sigaction sa;

    /* Gestire maschere e segnali */

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    setvbuf(stdout, NULL, _IOFBF, 0);

    /* Accesso all'array di semafori */

    if ((sem_id = semget(getppid(), NSEMS, 0666)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }

    /* Accesso alla griglia di celle */

    if ((shm_id = shmget(getppid(), GRID_SIZE * sizeof(*city_grid), 0600)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }
    city_grid = (Cell *)shmat(shm_id, NULL, 0);
    TEST_ERROR;

    print_grid_state(sem_id, city_grid);

    /* Finito inizializzazione, può partire la simulazione */

    SEMOP(sem_id, SEM_KIDS, 1, 0);
    TEST_ERROR;

    SEMOP(sem_id, SEM_START, 0, 0);
    TEST_ERROR;

    while (1) {
        sleep(PRINT_INTERVAL);
        SEMOP(sem_id, SEM_PRINT, 1, 0);
        print_grid_state(sem_id, city_grid);
        SEMOP(sem_id, SEM_PRINT, -1, 0);
    }
}


void handle_signal(int signum)
{
    switch (signum)
    {
    case SIGINT:
    case SIGTERM:
        terminate();
        break;
    default:
        break;
    }
}


void terminate()
{
    shmdt(city_grid);
    exit(EXIT_SUCCESS);
}