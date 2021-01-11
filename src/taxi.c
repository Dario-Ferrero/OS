#include "../lib/common.h"
#include "../lib/taxi.h"

int sem_id, taxi_pos, SO_TIMEOUT, *sources_pos;
Cell *city_grid;
struct sembuf sops;
TaxiStats stats;
struct itimerval timer_init, time_left, timer_block;

int main(int argc, char *argv[])
{
    int i, cnt, dest_pos, exc_pos, shm_id, SO_SOURCES;
    struct sigaction sa;
    sigset_t sig_mask;
    Request req;

    /* Inizializzare le variabili */

    taxi_pos = atoi(argv[1]);
    SO_SOURCES = atoi(argv[2]);
    SO_TIMEOUT = atoi(argv[3]);

    bzero(&stats, sizeof(stats));
    stats.mtype = REQ_SUCC_MTYPE;
    stats.taxi_pid = getpid();

    bzero(&timer_init, sizeof(timer_init));
    timer_init.it_value.tv_sec = SO_TIMEOUT;
    bzero(&timer_block, sizeof(timer_block));

    exc_pos = -1;
    srand(getpid() + time(NULL));

    /* Impostare il signal handler, sbloccare SIGALRM (per i taxi di seconda generazione) */

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);
    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGALRM);
    sigprocmask(SIG_UNBLOCK, &sig_mask, NULL);

    /* Accedere alla griglia, e per ogni cella sorgente salvarne la posizione in un array */

    if ((shm_id = shmget(getppid(), GRID_SIZE * sizeof(*city_grid), 0600)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }
    city_grid = (Cell *)shmat(shm_id, NULL, 0);
    TEST_ERROR;
    sources_pos = (int *)calloc(SO_SOURCES, sizeof(*sources_pos));
    for (i = 0, cnt = 0; i < GRID_SIZE; i++) {
        if (IS_SOURCE(city_grid[i])) {
            sources_pos[cnt++] = i;
        }
    }

    /* Accedere all'array di semafori per sincronizzarsi col master */

    if ((sem_id = semget(getppid(), NSEMS, 0600)) == -1) {
        TEST_ERROR;
        free(sources_pos);
        exit(EXIT_FAILURE);
    }

    /* Il processo è pronto : incrementare SEM_KIDS e wait for zero su SEM_START */

    SEMOP(sem_id, SEM_KIDS, 1, 0);
    TEST_ERROR;

    SEMOP(sem_id, SEM_START, 0, 0);
    TEST_ERROR;

    /* Simulazione iniziata */

    setitimer(ITIMER_REAL, &timer_init, NULL);

    while (1) {
        
        /* Trovare la sorgente più vicina */

        dest_pos = closest_source(SO_SOURCES, exc_pos);

        /* Spostarsi su quella cella */

        while (taxi_pos != dest_pos) {
            drive_diagonal(dest_pos);
            drive_straight(dest_pos);
        }

        /* Se la coda non è vuota : prelevare una richiesta. Altrimenti cercare di nuovo */

        msgrcv(city_grid[taxi_pos].msq_id, &req, MSG_LEN(req), 0, IPC_NOWAIT);
        if (errno == ENOMSG) {
            exc_pos = dest_pos;
            continue;
        } else if (errno == EIDRM) {
            raise(SIGALRM);
        }
        stats.mtype = REQ_ABRT_MTYPE;

        /* Spostarsi sulla cella di destinazione */

        while (taxi_pos != req.dest_cell) {
            drive_diagonal(req.dest_cell);
            drive_straight(req.dest_cell);
        }
        stats.reqs_compl++;
        stats.mtype = REQ_SUCC_MTYPE;
    }
}


int closest_source(int n_src, int except)
{
    int i, res, min_dist, dist;

    min_dist = SO_WIDTH + SO_HEIGHT;
    res = -1;
    for (i = 0; i < n_src; i++) {
        dist = MANH_DIST(taxi_pos, sources_pos[i]);
        if (sources_pos[i] != except && dist < min_dist) {
            min_dist = dist;
            res = sources_pos[i];
            if (res == taxi_pos) {
                break;
            }
        }
    }
    return res;
}


void drive_diagonal(int goal)
{
    int8_t valid, dir, roads[2];
    int next;

    roads[0] = GET_Y(taxi_pos) > GET_Y(goal) ? GO_UP : GO_DOWN;
    roads[1] = GET_X(taxi_pos) > GET_X(goal) ? GO_LEFT : GO_RIGHT;

    while (!ALIGNED(taxi_pos, goal)) {
        dir = roads[RAND_RNG(0, 1)];
        GET_NEXT(next, dir);
        switch (dir) {
        case GO_UP:
            valid = !IS_BORDER(next) && GET_Y(goal) < GET_Y(taxi_pos) && !IS_HOLE(city_grid[next]);
            break;
        case GO_DOWN:
            valid = !IS_BORDER(next) && GET_Y(goal) > GET_Y(taxi_pos) && !IS_HOLE(city_grid[next]);
            break;
        case GO_LEFT:
            valid = !IS_BORDER(next) && GET_X(goal) < GET_X(taxi_pos) && !IS_HOLE(city_grid[next]);
            break;
        case GO_RIGHT:
            valid = !IS_BORDER(next) && GET_X(goal) > GET_X(taxi_pos) && !IS_HOLE(city_grid[next]);
            break;
        }

        if (valid) {
            access_cell(next, dir);
        }
    }
}


void drive_straight(int goal)
{
    int8_t new_dir;
    int next;

    new_dir = SAME_COLUMN(taxi_pos, goal) ? GO_UP : GO_LEFT;
    if (GET_Y(taxi_pos) < GET_Y(goal) || GET_X(taxi_pos) < GET_X(goal)) {
        new_dir++;
    }
    while (taxi_pos != goal) {
        GET_NEXT(next, new_dir);
        if (!IS_HOLE(city_grid[next])) {
            access_cell(next, new_dir);
        } else {
            circle_hole(new_dir, goal);
            return;
        }
    }   
}


void circle_hole(int8_t dir, int goal)
{
    int8_t old_dir;
    int next;

    do {
        old_dir = dir;
        dir += (dir == GO_UP || dir == GO_LEFT) ? RAND_RNG(2, 3) : RAND_RNG(1, 2);
        dir %= N_ROADS;
        GET_NEXT(next, dir);
        if (!IS_BORDER(next)) {
            access_cell(next, dir);
        }
        dir = old_dir;
    } while (taxi_pos != next);

    GET_NEXT(next, dir);
    do {
        access_cell(next, dir);
    } while (taxi_pos != next);
}


void access_cell(int dest, int8_t dir)
{
    int8_t back_dir;
    struct timespec cross_time, timeout;

    timeout.tv_sec = SO_TIMEOUT;
    timeout.tv_nsec = 0;

    setitimer(ITIMER_REAL, &timer_block, &time_left);
    SEMOP(sem_id, SEM_PRINT, 0, 0);
    setitimer(ITIMER_REAL, &time_left, NULL);

    SEMTIMEDOP(sem_id, dest, -1, 0, &timeout);
    if (errno != EAGAIN) {
        SEMOP(sem_id, taxi_pos, 1, 0);
        taxi_pos = dest;

        cross_time.tv_sec = 0;
        cross_time.tv_nsec = city_grid[dest].cross_time;
        nanosleep(&cross_time, NULL);

        setitimer(ITIMER_REAL, &timer_init, NULL);

        stats.route_time += cross_time.tv_nsec;
        stats.cells_crossed++;
        city_grid[dest].cross_n++;
    } else {
        setitimer(ITIMER_REAL, &timer_block, NULL);
        terminate();
    }
}


void handle_signal(int signum)
{
    switch (signum) {
    case SIGINT:
    case SIGTERM: /* Terminazione forzata */
        shmdt(city_grid);
        free(sources_pos);
        exit(EXIT_SUCCESS);
        break;
    case SIGALRM: /* Fine simulazione */
        terminate();
    }
}


void terminate()
{
    int statsq_id;

    SEMOP(sem_id, taxi_pos, 1, 0);
    shmdt(city_grid);
    free(sources_pos);

    if ((statsq_id = msgget(getppid(), 0600)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }
    if (msgsnd(statsq_id, &stats, MSG_LEN(stats), 0) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}