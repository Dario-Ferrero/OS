#include "common.h"
#include "master.h"

/*
 * La griglia di Cell rappresentante la città.
 * 
 * Impostata come var. globale per potervi accedere nel signal handler
 * e ridurre le signature delle funzioni
 */
Cell *city_grid;

int main(int argc, char *argv[])
{
    pid_t *taxis, *sources, child_pid;
    int i, status, shm_id, *src_pos;

    fprintf(stderr, "Inizio lettura parametri... ");
    read_params();
    fprintf(stderr, "Parametri letti.\n");

    fprintf(stderr, "Inizializzazione griglia... \n\n");
    shm_id = init_city_grid();
    fprintf(stderr, "Griglia inizializzata\n");

    print_grid();

    assign_sources(&src_pos);

    print_grid();

    /* print_grid_values(); */

    free(src_pos);
    shmdt(city_grid);
}


void read_params()
{
    FILE *in;
    char buf[READ_LEN], *token;
    int *params, cnt;

    if ((in = fopen(PARAMS_FILE, "r")) == NULL) {
        TEST_ERROR;
        fprintf(stderr, "Errore nell'apertura del file %s\n", PARAMS_FILE);
        exit(EXIT_FAILURE);
    }

    params = calloc(N_PARAMS, sizeof(*params));
    cnt = 0;
    while (fgets(buf, READ_LEN, in) != NULL) {
        strtok(buf, "=\n");
        token = strtok(NULL, "=\n");
        params[cnt++] = atoi(token);
    }

    SO_HOLES        = params[0];
    SO_TOP_CELLS    = params[1];
    SO_SOURCES      = params[2];
    SO_CAP_MIN      = params[3];
    SO_CAP_MAX      = params[4];
    SO_TAXI         = params[5];
    SO_TIMENSEC_MIN = params[6];
    SO_TIMENSEC_MAX = params[7];
    SO_TIMEOUT      = params[8];
    SO_DURATION     = params[9];

    free(params);
    fclose(in);
}


int init_city_grid()
{
    int i, pos, shm_id;

    if ((shm_id = shmget(getpid(),
                  GRID_SIZE * sizeof(*city_grid),
                  IPC_CREAT | IPC_EXCL | 0666)) == -1) {
        TEST_ERROR;
        fprintf(stderr, "Oggetto IPC (memoria condivisa) già esistente con id %d\n", getpid());
        exit(EXIT_FAILURE);
    }

    city_grid = shmat(shm_id, NULL, 0);
    TEST_ERROR;
    shmctl(shm_id, IPC_RMID, NULL);

    fprintf(stderr, "Assegnamento dei %d HOLE... ", SO_HOLES);

    srand(getpid());
    i = SO_HOLES;
    while (i > 0) {
        pos = rand() % GRID_SIZE;
        if (!check_adj_cells(pos)) {
            city_grid[pos].flags |= HOLE_CELL;
            i--;
        }
    }
    fprintf(stderr, "assegnati (%d rimanenti).\n", i);

    fprintf(stderr, "Inizializzazione delle celle... ");
    
    for (i = 0; i < GRID_SIZE; i++) {
        if (!IS_HOLE(city_grid[i])) {
            city_grid[i].cross_time = (long)RAND_RNG(SO_TIMENSEC_MIN, SO_TIMENSEC_MAX);
            city_grid[i].capacity = (long)RAND_RNG(SO_CAP_MIN, SO_CAP_MAX);
            city_grid[i].cross_n = 0;
            city_grid[i].flags = 0;
        }
    }
    fprintf(stderr, "Celle inizializzate.\n\n");

    return shm_id;
}


int check_adj_cells(long pos)
{
    int x, x_beg, x_end, y, y_beg, y_end;
    int res;
    
    x = pos % SO_WIDTH;
    y = (pos - x) / SO_WIDTH;
    x_beg = x ? x-1 : x;
    y_beg = y ? y-1 : y;
    x_end = (x == SO_WIDTH-1)  ? x : x+1;
    y_end = (y == SO_HEIGHT-1) ? y : y+1;

    res = FALSE;
    for (y = y_beg; !res && y <= y_end; y++) {
        for (x = x_beg; !res && x <= x_end; x++) {
            res = city_grid[INDEX(x, y)].flags & HOLE_CELL;
        }
    }

    return res;
}


void print_grid()
{
    int x, y;

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
            if (IS_HOLE(city_grid[INDEX(x, y)])) {
                fprintf(stderr, "H ");
            } else if (IS_SOURCE(city_grid[INDEX(x, y)])) {
                fprintf(stderr, "S ");
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


void print_grid_values()
{
    long i;

    for (i = 0; i < GRID_SIZE; i++) {
        fprintf(stderr, "city_grid[%3ld].cross_time = %ld\n", i, city_grid[i].cross_time);
        fprintf(stderr, "city_grid[%3ld].capacity = %ld\n", i, city_grid[i].capacity);
        fprintf(stderr, "city_grid[%3ld].cross_n = %d\n", i, city_grid[i].cross_n);
        fprintf(stderr, "city_grid[%3ld].flags = %u\n\n", i, city_grid[i].flags);
    }
}


void assign_sources(int **sources)
{
    int cnt, i, pos;

    *sources = (int *)calloc(SO_SOURCES, sizeof(*sources));
    cnt = SO_SOURCES;
    i = 0;
    do {
        pos = RAND_RNG(0, GRID_SIZE-1);
        if (!IS_HOLE(city_grid[pos]) && !IS_SOURCE(city_grid[pos])) {
            city_grid[pos].flags |= SRC_CELL;
            (*sources)[i++] = pos;
            cnt--;
        }
    } while (cnt > 0);
}

void handle_signal(int signum)
{
    switch (signum) {
    case SIGINT:
        break;
    
    case SIGALRM:
        break;
    
    default:
        break;
    }
}
