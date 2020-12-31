#include "common.h"
#include "taxi.h"

int sem_id, shm_id, taxi_pos, timeout, SO_TIMEOUT;
Cell *city_grid;
struct sembuf sops;
TaxiStats stats;

int main(int argc, char *argv[])
{
    int i, cnt, dest_pos, SO_SOURCES, *src_pos;
    Request req;
    struct sigaction sa;

    /* Creato dal master : leggere parametri (posizione, SO_SOURCES, ???) */

    taxi_pos = atoi(argv[1]);
    SO_SOURCES = atoi(argv[2]);
    SO_TIMEOUT = atoi(argv[3]);
    /* fprintf(stderr, "Taxi creato! La mia posizione è %4d, SO_SOURCES è %d\n", taxi_pos, SO_SOURCES); */

    /*
     * Assegnare handle_signal come handler per i segnali che gestisce
     */
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
    src_pos = (int *)calloc(SO_SOURCES, sizeof(*src_pos));
    for (i = 0, cnt = 0; i < GRID_SIZE; i++) {
        if (IS_SOURCE(city_grid[i])) {
            src_pos[cnt++] = i;
        }
    }

    /* Accedere all'array di semafori per sincronizzarsi col master */

    if ((sem_id = semget(getppid(), NSEMS, 0666)) == -1) {
        TEST_ERROR;
        free(src_pos);
        exit(EXIT_FAILURE);
    }

    /* Inizializzare le proprie variabili (es. TaxiStat) */

    bzero(&stats, sizeof(stats));
    stats.taxi_pid = getpid();

    /* Accedere alla coda per l'invio delle statistiche : adesso o prima di terminare ??? */


    /*
     * Pronto ? Allora signal su SEM_KIDS e wait for zero su SEM_START
     */

    i = closest_source(src_pos, SO_SOURCES);
    fprintf(stderr, "Taxi #%5d:  taxi_pos: (%3d, %3d)  goal: (%3d, %3d)   SO_SOURCES = %d\n",
            getpid(), GET_X(taxi_pos), GET_Y(taxi_pos), GET_X(i), GET_Y(i), SO_SOURCES);

    SEMOP(sem_id, SEM_KIDS, 1, 0);
    TEST_ERROR;

    SEMOP(sem_id, SEM_START, 0, 0);
    TEST_ERROR;

#if 1
    /* Simulazione iniziata (ciclo abbastanza contorto, da pensare più in dettaglio) */

    srand(getpid() + time(NULL));
    alarm(timeout = SO_TIMEOUT);

    while (1) {
        
        /* GESTIRE MASCHERAZIONE SEGNALI E TIMER (qui o in drive() ?) */

        /* Trovare la sorgente più vicina tramite manhattan distance */

        dest_pos = closest_source(src_pos, SO_SOURCES);

        /* Spostarsi su quella cella */
        while (taxi_pos != dest_pos) {
            taxi_pos = drive_diagonal(dest_pos);
            taxi_pos = drive_straight(dest_pos);
        }
        fprintf(stderr, "Taxi ha raggiunto SORGENTE (%3d, %3d)\n", GET_X(taxi_pos), GET_Y(taxi_pos));

#if 1
        /* IF coda non vuota : prelevare una richiesta. ELSE continue; */

        msgrcv(city_grid[dest_pos].msq_id, &req, sizeof(req) - sizeof(long), 0, IPC_NOWAIT);
        if (errno == ENOMSG) {
            continue;
        } else if (errno == EIDRM) {
            raise(SIGALRM);
        }
        fprintf(stderr, "Prossima fermata: (%3d, %3d)\n", GET_X(req.dest_cell), GET_Y(req.dest_cell));

        /* Spostarsi sulla cella di destinazione */

        while (taxi_pos != req.dest_cell) {
            taxi_pos = drive_diagonal(req.dest_cell);
            taxi_pos = drive_straight(req.dest_cell);
        }
        stats.reqs_compl += 1;
        fprintf(stderr, "Taxi ha raggiunto DESTINAZIONE (%3d, %3d)\n", GET_X(taxi_pos), GET_Y(taxi_pos));
        break;
#endif
    }
#endif /* #if 0 per il loop */

    free(src_pos);
}


int closest_source(int *src_pos, int n_src)
{
    int i, res, min_dist, dist;

    min_dist = SO_WIDTH + SO_HEIGHT;
    res = -1;
    for (i = 0; i < n_src; i++) {
        dist = MANH_DIST(taxi_pos, src_pos[i]);
        if (dist < min_dist) {
            /*
             * Nel caso sia già su una sorgente ?
             * - magari non ho trovato messaggi su di una coda e ne voglio cercare una diversa
             * - caso particolarmente raro, forse non vale la pena pensarci
             */
            min_dist = dist;
            res = src_pos[i];
            if (res == taxi_pos) {
                break;
            }
        }
    }
    return res;
}


/*
 * Occhio alla gestione sync per print / (dis)attivazione timer
 * - funzione apposita ? chiamata ad inizio cicli di drive_diagonal() e drive_straight()
 * - Il check sul SEM_START (o SEM_PRINT) deve avvenire quando il timer è stato disattivato
 *      - però : se il taxi non si è spostato ?
 *      - fermo alarm, salvo i secs che rimanevano, faccio il check e lo faccio ripartire
 */
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
            if (taxi_pos == next) {
                alarm(timeout = SO_TIMEOUT); /* Qui o in access_cell() ? */
                fprintf(stderr, "dd:  Taxi ha raggiunto posizione (%3d, %3d)\n", GET_X(taxi_pos), GET_Y(taxi_pos));
            }
        }
    }

    return taxi_pos;
}


int drive_straight(int goal)
{
    int8_t old_dir, new_dir;
    int next;

    new_dir = SAME_COLUMN(taxi_pos, goal) ? GO_UP : GO_LEFT;
    new_dir += GET_Y(taxi_pos) < GET_Y(goal) || GET_X(taxi_pos) < GET_X(goal) ? 1 : 0;
    while (taxi_pos != goal) {
        next = get_road(new_dir);
        if (!IS_HOLE(city_grid[next])) {
            taxi_pos = access_cell(next);
            if (taxi_pos == next) { /* Mi sono spostato */
                alarm(timeout = SO_TIMEOUT); /* Qui o in access_cell() ? */
                fprintf(stderr, "ds:  Taxi ha raggiunto posizione (%3d, %3d)\n", GET_X(taxi_pos), GET_Y(taxi_pos));
            }
        } else { /* Circumnavigarlo (funzione apposita ?) */
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
            if (taxi_pos == next) {
                alarm(timeout = SO_TIMEOUT); /* Qui o in access_cell() ? */
                fprintf(stderr, "ch1: Taxi ha raggiunto posizione (%3d, %3d)\n", GET_X(taxi_pos), GET_Y(taxi_pos));
            }
        }
        dir = old_dir;
    } while (taxi_pos != next);

    next = get_road(dir);
    do {
        taxi_pos = access_cell(next);
    } while (taxi_pos != next);
    alarm(timeout = SO_TIMEOUT); /* Qui o in access_cell() ? */
    fprintf(stderr, "ch2: Taxi ha raggiunto posizione (%3d, %3d)\n", GET_X(taxi_pos), GET_Y(taxi_pos));

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
    struct timespec cross_time;

    SEMOP(sem_id, dest, -1, IPC_NOWAIT);
    if (errno != EAGAIN) {
        SEMOP(sem_id, taxi_pos, 1, 0);
        cross_time.tv_nsec = city_grid[dest].cross_time;
        nanosleep(&cross_time, NULL);
        stats.route_time += cross_time.tv_nsec;
        stats.cells_crossed += 1;
        city_grid[dest].cross_n += 1;
        return dest;
    }

    return taxi_pos;
}


void handle_signal(int signum)
{
    SEMOP(sem_id, taxi_pos, 1, 0);

    switch (signum) {
    case SIGINT:
    case SIGTERM:
        
        exit(EXIT_FAILURE);
        /* Terminazione forzata : free(), shmdt() */
        break;
    case SIGALRM:
        /* Timeout : chiudi tutto come sopra, ma exit status diverso e invia stats al master */
        break;
    }
}