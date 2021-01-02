#include "common.h"
#include "taxi.h"

int sem_id, shm_id, taxi_pos, SO_TIMEOUT, *sources_pos;
Cell *city_grid;
struct sembuf sops;
TaxiStats stats;

int main(int argc, char *argv[])
{
    int i, cnt, dest_pos, exc_pos, SO_SOURCES;
    struct sigaction sa;
    Request req;
    
    /* Creato dal master : leggere parametri (posizione, SO_SOURCES, ???) */

    taxi_pos = atoi(argv[1]);
    SO_SOURCES = atoi(argv[2]);
    SO_TIMEOUT = atoi(argv[3]);

    /* Assegnare handle_signal come handler per i segnali che gestisce */

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

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

    if ((sem_id = semget(getppid(), NSEMS, 0666)) == -1) {
        TEST_ERROR;
        free(sources_pos);
        exit(EXIT_FAILURE);
    }

    /* Inizializzare le proprie variabili (es. TaxiStat) */

    bzero(&stats, sizeof(stats));
    stats.mtype = REQ_SUCC_MTYPE;
    stats.taxi_pid = getpid();

    exc_pos = -1;
    srand(getpid() + time(NULL));

    /* Pronto ? Allora signal su SEM_KIDS e wait for zero su SEM_START */

    SEMOP(sem_id, SEM_KIDS, 1, 0);
    TEST_ERROR;

    SEMOP(sem_id, SEM_START, 0, 0);
    TEST_ERROR;

    /* Simulazione iniziata (ciclo abbastanza contorto, da pensare più in dettaglio) */

    alarm(SO_TIMEOUT);

    while (1) {
        
        /* Trovare la sorgente più vicina tramite manhattan distance */

        dest_pos = closest_source(SO_SOURCES, exc_pos);

        /* Spostarsi su quella cella */
        while (taxi_pos != dest_pos) {
            taxi_pos = drive_diagonal(dest_pos);
            taxi_pos = drive_straight(dest_pos);
        }

        /* IF coda non vuota : prelevare una richiesta. ELSE continue; */

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
            taxi_pos = drive_diagonal(req.dest_cell);
            taxi_pos = drive_straight(req.dest_cell);
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


int drive_diagonal(int goal)
{
    int8_t valid, roads[2];
    int next;

    roads[0] = GET_Y(taxi_pos) > GET_Y(goal) ? GO_UP : GO_DOWN;
    roads[1] = GET_X(taxi_pos) > GET_X(goal) ? GO_LEFT : GO_RIGHT;

    while (!ALIGNED(taxi_pos, goal)) {
        switch (roads[RAND_RNG(0, 1)]) { /* Scelgo tra una delle due strade che mi avvicinano a goal */
        case GO_UP:
            next = UP(taxi_pos);
            valid = !IS_BORDER(next) && GET_Y(goal) < GET_Y(taxi_pos) && !IS_HOLE(city_grid[next]);
            break;
        case GO_DOWN:
            next = DOWN(taxi_pos);
            valid = !IS_BORDER(next) && GET_Y(goal) > GET_Y(taxi_pos) && !IS_HOLE(city_grid[next]);
            break;
        case GO_LEFT:
            next = LEFT(taxi_pos);
            valid = !IS_BORDER(next) && GET_X(goal) < GET_X(taxi_pos) && !IS_HOLE(city_grid[next]);
            break;
        case GO_RIGHT:
            next = RIGHT(taxi_pos);
            valid = !IS_BORDER(next) && GET_X(goal) > GET_X(taxi_pos) && !IS_HOLE(city_grid[next]);
            break;
        }

        if (valid) {
            taxi_pos = access_cell(next);
        }
    }

    return taxi_pos;
}


int drive_straight(int goal)
{
    int8_t old_dir, new_dir;
    int next;

    new_dir = SAME_COLUMN(taxi_pos, goal) ? GO_UP : GO_LEFT;
    if (GET_Y(taxi_pos) < GET_Y(goal) || GET_X(taxi_pos) < GET_X(goal)) {
        new_dir++;
    }
    while (taxi_pos != goal) {
        next = get_road(new_dir);
        if (!IS_HOLE(city_grid[next])) {
            taxi_pos = access_cell(next);
        } else { /* Circumnavigarlo */
            taxi_pos = circle_hole(new_dir, goal);
            return taxi_pos;
        }
    }

    return taxi_pos;    
}


int circle_hole(int8_t dir, int goal)
{
    int8_t old_dir;
    int next;

    do {
        /* Scelgo una strada perpendicolare a new_dir */
        old_dir = dir;
        dir += (dir == GO_UP || dir == GO_LEFT) ? RAND_RNG(2, 3) : RAND_RNG(1, 2);
        dir %= N_ROADS;
        next = get_road(dir);
        if (!IS_BORDER(next)) {
            taxi_pos = access_cell(next);
        }
        dir = old_dir;
    } while (taxi_pos != next);

    next = get_road(dir);
    do {
        taxi_pos = access_cell(next);
    } while (taxi_pos != next);

    return taxi_pos;
}

int get_road(int8_t dir)
{
    switch (dir) {
        case GO_UP:
            return UP(taxi_pos);
        case GO_DOWN:
            return DOWN(taxi_pos);
        case GO_LEFT:
            return LEFT(taxi_pos);
        case GO_RIGHT:
            return RIGHT(taxi_pos);
        default:
            return -1;
    }
}


int access_cell(int dest)
{
    int time_left;
    struct timespec cross_time;
    sigset_t sig_mask;

    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGALRM);

    time_left = alarm(0);
    SEMOP(sem_id, SEM_PRINT, 0, 0);
    alarm(time_left);

    SEMOP(sem_id, dest, -1, IPC_NOWAIT);
    if (errno != EAGAIN) {
        SEMOP(sem_id, taxi_pos, 1, 0);
        cross_time.tv_sec = 0;
        cross_time.tv_nsec = city_grid[dest].cross_time;
        nanosleep(&cross_time, NULL);

        sigprocmask(SIG_BLOCK, &sig_mask, NULL);
        stats.route_time += cross_time.tv_nsec;
        stats.cells_crossed++;
        city_grid[dest].cross_n++;
        taxi_pos = dest;
        SEMOP(sem_id, SEM_PRINT, 0, 0);
        alarm(SO_TIMEOUT);
        sigprocmask(SIG_UNBLOCK, &sig_mask, NULL);
    }
    
    return taxi_pos;
}


void handle_signal(int signum)
{
    SEMOP(sem_id, taxi_pos, 1, 0);
    free(sources_pos);

    switch (signum) {
    case SIGINT:
    case SIGTERM:
        exit(EXIT_FAILURE);
        /* Terminazione forzata : free(), shmdt() */
        break;
    case SIGALRM:
        exit(EXIT_SUCCESS);
        /* Timeout : chiudi tutto come sopra, ma exit status diverso e invia stats al master */
        break;
    }
}