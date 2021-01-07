#include "../lib/common.h"
#include "../lib/printer.h"
#include "../lib/gridprint.h"

Cell *city_grid;

int main(int argc, char *argv[])
{
    int shm_id, sem_id;
    struct sigaction sa;
    struct sembuf sops;

    /* Gestire maschere e segnali */

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    setvbuf(stdout, NULL, _IOFBF, 0);

    /* Accedere all'array di semafori */

    if ((sem_id = semget(getppid(), NSEMS, 0666)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }

    /* Accedere alla griglia di simulazione */

    if ((shm_id = shmget(getppid(), GRID_SIZE * sizeof(*city_grid), 0600)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }
    city_grid = (Cell *)shmat(shm_id, NULL, 0);
    TEST_ERROR;

    print_grid_state(sem_id, city_grid);

    /* Il processo è pronto : incrementare SEM_KIDS e wait for zero su SEM_START */

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
    switch (signum) {
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