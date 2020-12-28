#include "common.h"
#include "taxi.h"

int sem_id, shm_id, reqsq_id, SO_TIMEOUT;
Cell *city_grid;
struct sembuf sops;
TaxiStats stats;

int main(int argc, char *argv[])
{
    int i, pos, cnt, dest_pos, SO_SOURCES, *src_pos;
    Request req;
    struct sigaction sa;

    /* Creato dal master : leggere parametri (posizione, SO_SOURCES, ???) */

    pos = atoi(argv[1]);
    SO_SOURCES = atoi(argv[2]);
    SO_TIMEOUT = atoi(argv[3]);
    /* fprintf(stderr, "Taxi creato! La mia posizione è %4d, SO_SOURCES è %d\n", pos, SO_SOURCES); */

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

    i = closest_source(pos, src_pos, SO_SOURCES);
    fprintf(stderr, "Taxi #%5d:  pos: (%3d, %3d)  goal: (%3d, %3d)   SO_SOURCES = %d\n",
            getpid(), GET_X(pos), GET_Y(pos), GET_X(i), GET_Y(i), SO_SOURCES);

    SEMOP(sem_id, SEM_KIDS, 1, 0);
    TEST_ERROR;

    SEMOP(sem_id, SEM_START, 0, 0);
    TEST_ERROR;

#if 0
    /* Simulazione iniziata (ciclo abbastanza contorto, da pensare più in dettaglio) */
    while (1) {
        
        /* GESTIRE MASCHERAZIONE SEGNALI E TIMER (qui o in drive() ?) */
        
        /* Trovare la sorgente più vicina tramite manhattan distance */

        dest_pos = closest_source(pos, src_pos, SO_SOURCES);

        /* Spostarsi su quella cella */

        pos = drive(pos, dest_pos);

        /* IF coda non vuota : prelevare una richiesta. ELSE continue; */

        reqsq_id = city_grid[dest_pos].msq_id;
        msgrcv(reqsq_id, &req, sizeof(req) - sizeof(long), 0, IPC_NOWAIT);
        if (errno == ENOMSG) {
            continue;
        }

        /* Spostarsi sulla cella di destinazione */

        pos = drive(pos, req.dest_cell);

        stats.reqs_compl += 1;
    }
#endif /* #if 0 per il loop */

    free(src_pos);
}


int closest_source(int cur_pos, int *src_pos, int n_src)
{
    int i, res, min_dist, dist;

    min_dist = SO_WIDTH + SO_HEIGHT;
    res = -1;
    for (i = 0; i < n_src; i++) {
        dist = MANH_DIST(cur_pos, src_pos[i]);
        if (dist < min_dist) { /* Nel caso sia già su una sorgente ? */
            min_dist = dist;
            res = src_pos[i];
            if (res == cur_pos) {
                break;
            }
        }
    }
    return res;
}


/*
 * Occhio alla gestione sync per print / (dis)attivazione timer
 * - Il check sul SEM_START (o SEM_PRINT) deve avvenire quando il timer è stato disattivato
 *      - però : se il taxi non si è spostato ?
 *      - fermo alarm, salvo i secondi che rimanevano, faccio il check e lo faccio ripartire
 */
int drive(int start, int goal)
{
    char i;
    int pos,
        roads[2] = {-1, -1};
    
    pos = start;
    while (pos != goal) {
        SEMOP(sem_id, SEM_PRINT, 0, 0);
        

        if (UP(pos) >= 0 && !IS_HOLE(city_grid[UP(pos)]) && GET_Y(goal) < GET_Y(pos)) {
            single_move(pos, UP(pos));
        } else if (LEFT(pos) >= 0 && !IS_HOLE(city_grid[LEFT(pos)]) && GET_X(goal) < GET_X(pos)) {
            return 0;
        }
        
    }
        
    return pos;
}


int single_move(int from, int dest)
{
    struct timespec cross_time;

    SEMOP(sem_id, dest, -1, IPC_NOWAIT);
    if (errno != EAGAIN) {
        SEMOP(sem_id, from, 1, 0);
        cross_time.tv_nsec = city_grid[dest].cross_time;
        nanosleep(&cross_time, NULL);
        stats.route_time += cross_time.tv_nsec;
        stats.cells_crossed += 1;
        city_grid[dest].cross_n += 1;
        return dest;
    }

    return from;
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