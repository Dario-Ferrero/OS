#include "common.h"
#include "master.h"

/*
 * La griglia di Cell rappresentante la città.
 * 
 * Impostata come var. globale per potervi accedere nel signal handler
 * e ridurre le signature delle funzioni
 */
Cell *city_grid;
int sem_id, shm_id;
struct sembuf sops;

pid_t *taxis, *srcs;

int main(int argc, char *argv[])
{
    pid_t child_pid;
    int i, status, *src_pos;
    char *src_args[3],
         *taxi_args[3],
         args_buf[10];

    struct sigaction sa;

    fprintf(stderr, "Inizio lettura parametri... ");
    read_params();
    fprintf(stderr, "parametri letti.\n");

    fprintf(stderr, "Controllo parametri... ");
    check_params();
    fprintf(stderr, "parametri validi.\n");

    fprintf(stderr, "Inizializzazione griglia... \n\n");
    shm_id = init_city_grid();
    fprintf(stderr, "Griglia inizializzata\n");

    print_grid();

    fprintf(stderr, "Assegnamento celle sorgente... ");
    assign_sources(&src_pos);
    fprintf(stderr, "celle sorgente assegnate.\n");

    print_grid();

    if ((sem_id = semget(getpid(), NSEMS, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
        TEST_ERROR;
        fprintf(stderr, "Oggetto IPC (array di semafori) già esistente con chiave %d\n", getpid());
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < GRID_SIZE; i++) {
        semctl(sem_id, i, SETVAL, RAND_RNG(SO_CAP_MIN, SO_CAP_MAX));
    }
    semctl(sem_id, SEM_START, SETVAL, 1);
    semctl(sem_id, SEM_KIDS, SETVAL, 0);


    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);


    fprintf(stderr, "Creazione processi sorgente...\n");
    srcs = (pid_t *)calloc(SO_SOURCES, sizeof(*srcs));
    src_args[0] = SRC_FILE;
    src_args[2] = NULL;
    for (i = 0; i < SO_SOURCES; i++) {
        switch (child_pid = fork()) {
        case -1:
            fprintf(stderr, "Fork fallita.\n");
            TEST_ERROR;
            terminate();
        case 0:
            sprintf(args_buf, "%d", src_pos[i]);
            src_args[1] = args_buf;
            execvp(SRC_FILE, src_args);
            exit(EXIT_FAILURE);
            break;
        default:
            srcs[i] = child_pid;
        }
    }

    /* Aspetto che le sorgenti abbiano finito di inizializzarsi */

    SEMOP(sem_id, SEM_KIDS, -SO_SOURCES, 0);
    TEST_ERROR;

    fprintf(stderr, "Master sbloccato, le sorgenti si sono inizializzate.\n");

    /* print_grid_values(); */

    SEMOP(sem_id, SEM_START, -1, 0);
    TEST_ERROR;

    while ((child_pid = wait(&status)) != -1) {
        fprintf(stderr, "Child (src) #%d terminated with exit status %d\n", child_pid, WEXITSTATUS(status));
    }

    free(src_pos);
    free(srcs);
    terminate();
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


void check_params()
{
    if (SO_WIDTH <= 0 || SO_HEIGHT <= 0) {
        fprintf(stderr, "Valori di SO_WIDTH e SO_HEIGHT troppo bassi!\n");
        exit(EXIT_FAILURE);
    } else if (SO_SOURCES + SO_HOLES > GRID_SIZE) {
        fprintf(stderr, "Troppi SO_SOURCES e SO_HOLES per la dimensione della griglia!\n");
        exit(EXIT_FAILURE);
    } else if (SO_TOP_CELLS > GRID_SIZE - SO_HOLES) {
        fprintf(stderr, "Troppe SO_TOP_CELLS rispetto alle celle accessibili!\n");
        exit(EXIT_FAILURE);
    } else if (SO_TIMENSEC_MIN > SO_TIMENSEC_MAX) {
        fprintf(stderr, "Valori SO_TIMENSEC incoerenti! (MIN > MAX)\n");
        exit(EXIT_FAILURE);
    } else if (SO_CAP_MIN > SO_CAP_MAX) {
        fprintf(stderr, "Valori SO_CAP incoerenti! (MIN > MAX)\n");
        exit(EXIT_FAILURE);
    } else if (SO_TAXI > (GRID_SIZE - SO_HOLES) * SO_CAP_MIN) {
        fprintf(stderr, "I SO_TAXI potrebbero essere troppi per questa configurazione.\n");
    } else if (GRID_SIZE / SO_HOLES < 9) {
        fprintf(stderr, "Troppi SO_HOLES rispetto alle dimensioni della griglia.\n");
        exit(EXIT_FAILURE);
    }
}


int init_city_grid()
{
    int i, pos;

    if ((shm_id = shmget(getpid(),
                  GRID_SIZE * sizeof(*city_grid),
                  IPC_CREAT | IPC_EXCL | 0600)) == -1) {
        TEST_ERROR;
        fprintf(stderr, "Oggetto IPC (memoria condivisa) già esistente con chiave %d\n", getpid());
        exit(EXIT_FAILURE);
    }

    city_grid = (Cell *)shmat(shm_id, NULL, 0);
    TEST_ERROR;

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
            city_grid[i].msq_id = 0;
            city_grid[i].cross_n = 0;
            city_grid[i].flags = 0;
        }
    }
    fprintf(stderr, "celle inizializzate.\n\n");
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
        fprintf(stderr, "city_grid[%3ld].msq_id = %d\n", i, city_grid[i].msq_id);
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
    case SIGTERM:
    case SIGINT:
        terminate();
        break;
    
    case SIGALRM:
        break;
    
    default:
        break;
    }
}


void terminate()
{
    int i;

    /*
     * TODO:
     * - uccidere sorgenti
     * - uccidere taxi
     */

    shmdt(city_grid);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    exit(EXIT_FAILURE);
}