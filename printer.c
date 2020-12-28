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

    bzero(&sa, sizeof(sa));
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

    print_grid_state();

    /* Finito inizializzazione, può partire la simulazione */

    SEMOP(sem_id, SEM_KIDS, 1, 0);
    SEMOP(sem_id, SEM_START, 0, 0);
    
    shmdt(city_grid);
}


void print_grid_state()
{
    int x, y, n_taxi;

    fprintf(stderr, "\n\n\n     ");
    for (x = 0; x < SO_WIDTH; x++) {
        fprintf(stderr, "%d ", x % 10);
    }
    fprintf(stderr, "\n    ");
    for (x = 0; x < SO_WIDTH; x++) {
        fprintf(stderr, "--");
    }
    fprintf(stderr, "-\n");

    for (y = 0; y < SO_HEIGHT; y++) {
        fprintf(stderr, " %d | ", y % 10);
        for (x = 0; x < SO_WIDTH; x++) {
            n_taxi = city_grid[INDEX(x, y)].capacity -
                     semctl(sem_id, INDEX(x, y), GETVAL);
            TEST_ERROR;
            if (IS_HOLE(city_grid[INDEX(x, y)])) {
                fprintf(stderr, ANSI_RED"H "ANSI_RESET);
            } else if (n_taxi) {
                if (IS_SOURCE(city_grid[INDEX(x, y)])) {
                    fprintf(stderr, ANSI_YELLOW"%d "ANSI_RESET, n_taxi);
                } else {
                    fprintf(stderr, "%d ", n_taxi);
                }
            } else if (IS_SOURCE(city_grid[INDEX(x, y)])) {
                fprintf(stderr, ANSI_YELLOW"S "ANSI_RESET);
            } else {
                fprintf(stderr, "%c ", (char)96);
            }
        }
        fprintf(stderr, "|\n");
    }

    fprintf(stderr, "    ");
    for (x = 0; x < SO_WIDTH; x++) {
        fprintf(stderr, "--");
    }
    fprintf(stderr, "-\n\n\n");
}


void handle_signal(int signum)
{
    switch (signum)
    {
    case SIGALRM:
        break;
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
    exit(EXIT_FAILURE);
}