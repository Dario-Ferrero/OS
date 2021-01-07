#include "../lib/common.h"
#include "../lib/master.h"
#include "../lib/gridprint.h"

Cell *city_grid;
TaxiStats *tstats;
int sem_id, shm_id, statsq_id,
    taxis_size, taxis_i,
    tstats_size, tstats_i,
    *sources_pos;
pid_t *taxis, *sources, printer;
struct sembuf sops;

int main(int argc, char *argv[])
{
    int i;
    struct sigaction sa;

    dprintf(STDOUT_FILENO, "Lettura parametri... ");
    read_params();
    dprintf(STDOUT_FILENO, "parametri letti.\n");

    dprintf(STDOUT_FILENO, "Controllo parametri... ");
    check_params();
    dprintf(STDOUT_FILENO, "parametri validi.\n");

    dprintf(STDOUT_FILENO, "Inizializzazione griglia... \n\n");
    init_city_grid();
    dprintf(STDOUT_FILENO, "Griglia inizializzata\n");

    print_initial_grid(city_grid);

    dprintf(STDOUT_FILENO, "Assegnamento celle sorgente... ");
    assign_sources();
    dprintf(STDOUT_FILENO, "celle sorgente assegnate.\n");

    print_initial_grid(city_grid);

    dprintf(STDOUT_FILENO, "Inizializzazione semafori... ");
    init_sems();
    dprintf(STDOUT_FILENO, "semafori inizializzati.\n");

    dprintf(STDOUT_FILENO, "Creazione coda di messaggi per statistiche taxi... ");
    if ((statsq_id = msgget(getpid(), IPC_CREAT | IPC_EXCL | 0600)) == -1) {
        TEST_ERROR;
        terminate();
    }
    tstats_i = 0;
    tstats_size = SO_TAXI;
    tstats = (TaxiStats *)calloc(tstats_size, sizeof(*tstats));
    dprintf(STDOUT_FILENO, "coda creata.\n\n");

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    dprintf(STDOUT_FILENO, "Creazione processi sorgente... ");
    create_sources();

    /* Aspetto che le sorgenti abbiano finito di inizializzarsi */

    SEMOP(sem_id, SEM_KIDS, -SO_SOURCES, 0);
    TEST_ERROR;

    dprintf(STDOUT_FILENO, "processi sorgente creati.\n");
    dprintf(STDOUT_FILENO, "Master sbloccato, le sorgenti si sono inizializzate.\n");

    dprintf(STDOUT_FILENO, "\nCreazione processi taxi... ");
    taxis_i = 0;
    taxis_size = SO_TAXI;
    taxis = (int *)calloc(taxis_size, sizeof(*taxis));
    create_taxis(taxis_size);

    /* Aspetto che i taxi abbiano finito di inizializzarsi */

    SEMOP(sem_id, SEM_KIDS, -SO_TAXI, 0);
    TEST_ERROR;

    dprintf(STDOUT_FILENO, "processi taxi creati.\n");
    dprintf(STDOUT_FILENO, "Master sbloccato, i taxi si sono inizializzati.\n\n");

    dprintf(STDOUT_FILENO, "Creazione processo printer... ");
    create_printer();

    /* Aspetto che il printer abbia finito di inizializzarsi */

    SEMOP(sem_id, SEM_KIDS, -1, 0);
    TEST_ERROR;

    dprintf(STDOUT_FILENO, "Processo printer creato.\n");

    dprintf(STDOUT_FILENO, "Processi figli creati, può partire la simulazione!\n\n");
    dprintf(STDOUT_FILENO, "I Process ID delle sorgenti sono osservabili:\n");
    dprintf(STDOUT_FILENO, "- da terminale, tramite comandi \'top\' oppure \'ps -e | grep sorgente\'\n");
    dprintf(STDOUT_FILENO, "- da applicazione, tramite il Monitor di Sistema\n");
    dprintf(STDOUT_FILENO, "E' possibile generare una richiesta per una sorgente tramite il comando:\n\n");
    dprintf(STDOUT_FILENO, "\t\t\tkill -SIGUSR1 pid_sorgente\n\n");
    dprintf(STDOUT_FILENO, "Premere ENTER per iniziare la simulazione o Ctrl-c per annullarla.\n");
    scanf("%c", (char *)&i);

    SEMOP(sem_id, SEM_START, -1, 0);
    TEST_ERROR;

    /* Ciclo di simulazione */

    alarm(SO_DURATION);
    collect_taxi_stats(1);
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


void init_city_grid()
{
    int i, pos;

    if ((shm_id = shmget(getpid(),
                  GRID_SIZE * sizeof(*city_grid),
                  IPC_CREAT | IPC_EXCL | 0666)) == -1) {
        TEST_ERROR;
        fprintf(stderr, "Oggetto IPC (memoria condivisa) già esistente con chiave %d\n", getpid());
        exit(EXIT_FAILURE);
    }

    city_grid = (Cell *)shmat(shm_id, NULL, 0);
    TEST_ERROR;

    dprintf(STDOUT_FILENO, "Assegnamento dei %d HOLE... ", SO_HOLES);
    srand(getpid() + time(NULL));
    i = SO_HOLES;
    while (i > 0) {
        pos = rand() % GRID_SIZE;
        if (!check_adj_cells(pos)) {
            city_grid[pos].flags |= HOLE_CELL;
            i--;
        }
    }
    dprintf(STDOUT_FILENO, "assegnati (%d rimanenti).\n", i);

    dprintf(STDOUT_FILENO, "Inizializzazione delle celle... ");
    for (i = 0; i < GRID_SIZE; i++) {
        if (!IS_HOLE(city_grid[i])) {
            city_grid[i].cross_time = (long)RAND_RNG(SO_TIMENSEC_MIN, SO_TIMENSEC_MAX);
            city_grid[i].msq_id = 0;
            city_grid[i].cross_n = 0;
            city_grid[i].capacity = RAND_RNG(SO_CAP_MIN, SO_CAP_MAX);
            city_grid[i].flags = 0;
        }
    }
    dprintf(STDOUT_FILENO, "celle inizializzate.\n\n");
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


void assign_sources()
{
    int cnt, i, pos;

    sources_pos = (int *)calloc(SO_SOURCES, sizeof(*sources_pos));
    cnt = SO_SOURCES;
    i = 0;
    do {
        pos = RAND_RNG(0, GRID_SIZE-1);
        if (!IS_HOLE(city_grid[pos]) && !IS_SOURCE(city_grid[pos])) {
            city_grid[pos].flags |= SRC_CELL;
            sources_pos[i++] = pos;
            cnt--;
        }
    } while (cnt > 0);
}


void init_sems()
{
    int i;

    if ((sem_id = semget(getpid(), NSEMS, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
        TEST_ERROR;
        fprintf(stderr, "Oggetto IPC (array di semafori) già esistente con chiave %d\n", getpid());
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < GRID_SIZE; i++) {
        if (!IS_HOLE(city_grid[i])) {
            semctl(sem_id, i, SETVAL, city_grid[i].capacity);
        } else {
            semctl(sem_id, i, SETVAL, 0);
        }
    }
    semctl(sem_id, SEM_START, SETVAL, 1);
    semctl(sem_id, SEM_KIDS, SETVAL, 0);
    semctl(sem_id, SEM_PRINT, SETVAL, 0);
}


void create_sources()
{
    int i, child_pid;
    char pos_buf[BUF_SIZE],
         nreqs_buf[BUF_SIZE],
        *src_args[4];

    sources = (pid_t *)calloc(SO_SOURCES, sizeof(*sources));
    src_args[0] = SRC_FILE;
    sprintf(nreqs_buf, "%d", REQS_RATE ? REQS_RATE : 1);
    src_args[2] = nreqs_buf;
    src_args[3] = NULL;
    for (i = 0; i < SO_SOURCES; i++) {
        switch (child_pid = fork()) {
        case -1:
            fprintf(stderr, "Fork fallita.\n");
            TEST_ERROR;
            exit(EXIT_FAILURE);
        case 0:
            sprintf(pos_buf, "%d", sources_pos[i]);
            src_args[1] = pos_buf;
            execvp(SRC_FILE, src_args);
            exit(EXIT_FAILURE);
            break;
        default:
            sources[i] = child_pid;
        }
    }
}


void create_taxis(int n_taxis)
{
    int i, pos, child_pid;
    char *taxi_args[5],
         pos_buf[BUF_SIZE],
         srcs_buf[BUF_SIZE],
         timeout_buf[BUF_SIZE];

    taxi_args[0] = TAXI_FILE;
    sprintf(srcs_buf, "%d", SO_SOURCES);
    taxi_args[2] = srcs_buf;
    sprintf(timeout_buf, "%d", SO_TIMEOUT);
    taxi_args[3] = timeout_buf;
    taxi_args[4] = NULL;
    for (i = 0; i < n_taxis; i++) {
        switch (child_pid = fork()) {
        case -1:
            fprintf(stderr, "Fork fallita.\n");
            TEST_ERROR;
            exit(EXIT_FAILURE);
        case 0:
            pos = -1;
            srand(getpid() + time(NULL));
            while (pos < 0) {
                pos = RAND_RNG(0, GRID_SIZE-1);
                SEMOP(sem_id, pos, -1, IPC_NOWAIT);
                if (errno == EAGAIN) {
                    errno = 0;
                    pos = -1;
                }
            }
            sprintf(pos_buf, "%d", pos);
            taxi_args[1] = pos_buf;
            execvp(TAXI_FILE, taxi_args);
            exit(EXIT_FAILURE);
            break;
        default:
            if (taxis_i >= taxis_size-1) {
                taxis_size <<= 1;
                taxis = (pid_t *)realloc(taxis, taxis_size * sizeof(*taxis));
            }
            taxis[taxis_i++] = child_pid;
        }
    }
}


void create_printer()
{
    int child_pid;
    char *prt_args[2];

    prt_args[0] = PRINTER_FILE;
    prt_args[1] = NULL;
    switch (child_pid = fork()) {
    case -1:
        fprintf(stderr, "Fork fallita.\n");
        TEST_ERROR;
        exit(EXIT_FAILURE);
        break;
    case 0:
        execvp(PRINTER_FILE, prt_args);
        exit(EXIT_FAILURE);
        break;
    default:
        printer = child_pid;
    }
}


void end_simulation()
{
    int i, child_cnt, *top_cells;
    long req_succ, req_unpicked, req_abrt;
    SourceStats src_stat;


    SEMOP(sem_id, SEM_PRINT, 0, 0);
    kill(printer, SIGTERM);
    waitpid(printer, NULL, 0);
    SEMOP(sem_id, SEM_PRINT, 1, 0);

    dprintf(STDOUT_FILENO, "Simulazione terminata.\n");
    dprintf(STDOUT_FILENO, "Raccolta statistiche sorgenti... ");
    req_succ = req_unpicked = req_abrt = 0;
    for (i = 0; i < SO_SOURCES; i++) {
        kill(sources[i], SIGALRM);
    }
    child_cnt = SO_SOURCES;
    while (child_cnt-- && msgrcv(statsq_id, &src_stat, MSG_LEN(src_stat), SOURCE_MTYPE, 0) != -1) {
        req_unpicked += src_stat.reqs_unpicked;
    }
    for (i = 0; i < SO_SOURCES; i++) {
        waitpid(sources[i], NULL, 0);
    }
    dprintf(STDOUT_FILENO, "completata.\n\n");


    dprintf(STDOUT_FILENO, "Raccolta statistiche taxi... ");
    for (i = 0; i < taxis_i; i++) {
        kill(taxis[i], SIGALRM);
    }
    collect_taxi_stats(0);
    dprintf(STDOUT_FILENO, "completata.\n");
    child_cnt = semctl(sem_id, SEM_KIDS, GETVAL);
    dprintf(STDOUT_FILENO, "Taxi di prima generazione = %d\n", SO_TAXI);
    dprintf(STDOUT_FILENO, "Taxi di seconda generazione = %d\n\n", child_cnt);


    for (i = 0; i < taxis_i; i++) {
        if (tstats[i].mtype == REQ_ABRT_MTYPE) {
            req_abrt++;
        }
        req_succ += tstats[i].reqs_compl;
    }
    dprintf(STDOUT_FILENO, "# Richieste completate con successo: %ld\n", req_succ);
    dprintf(STDOUT_FILENO, "# Richieste inevase: %ld\n", req_unpicked);
    dprintf(STDOUT_FILENO, "# Richieste abortite: %ld\n\n", req_abrt);


    print_best_taxis();

    get_top_cells(&top_cells);

    print_final_grid(city_grid, top_cells, SO_TOP_CELLS);

    free(top_cells);
    free(taxis);
    free(sources);
    free(tstats);
    free(sources_pos);
}


void collect_taxi_stats(int ntaxis)
{
    sigset_t sig_mask;

    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGALRM);

    do {
        while (msgrcv(statsq_id, &(tstats[tstats_i]), MSG_LEN(*tstats),
                        SOURCE_MTYPE, MSG_EXCEPT | IPC_NOWAIT) != -1) {
            sigprocmask(SIG_BLOCK, &sig_mask, NULL);
            if (tstats_i >= tstats_size-1) {
                tstats_size <<= 1;
                tstats = (TaxiStats *)realloc(tstats, tstats_size * sizeof(*tstats));
            }
            tstats_i++;
            if (ntaxis > 0) {
                create_taxis(ntaxis);
            }
            sigprocmask(SIG_UNBLOCK, &sig_mask, NULL);
        }
    } while (wait(NULL) != -1);
}


void print_best_taxis()
{
    int i;
    pid_t *best_taxis;
    long max_cells, max_time, max_reqs;

    best_taxis = (pid_t *)calloc(3, sizeof(*best_taxis));
    max_cells = max_time = max_reqs = 0;
    for (i = 0; i < tstats_i; i++) {
        if (tstats[i].cells_crossed > max_cells) {
            max_cells = tstats[i].cells_crossed;
            best_taxis[0] = tstats[i].taxi_pid;
        }
        if (tstats[i].route_time > max_time) {
            max_time = tstats[i].route_time;
            best_taxis[1] = tstats[i].taxi_pid;
        }
        if (tstats[i].reqs_compl > max_reqs) {
            max_reqs = tstats[i].reqs_compl;
            best_taxis[2] = tstats[i].taxi_pid;
        }
    }

    dprintf(STDOUT_FILENO, "Taxi che ha attraversato più celle (%ld) : %5d\n", max_cells, best_taxis[0]);
    dprintf(STDOUT_FILENO, "Taxi che ha viaggiato più a lungo (%ld nsecs) : %5d\n", max_time, best_taxis[1]);
    dprintf(STDOUT_FILENO, "Taxi che ha soddisfatto più richieste (%ld) : %5d\n\n", max_reqs, best_taxis[2]);
    free(best_taxis);
}


void get_top_cells(int **top_cells)
{
    int i, j, tmp, *cell_cnt;

    *top_cells = (int *)calloc(SO_TOP_CELLS+1, sizeof(**top_cells));
    cell_cnt = (int *)calloc(SO_TOP_CELLS+1, sizeof(*cell_cnt));

    for (i = 0; i < SO_TOP_CELLS; i++) {
        (*top_cells)[i] = -1;
    }
    for (i = 0; i < GRID_SIZE; i++) {
        if (!IS_SOURCE(city_grid[i])) {
            (*top_cells)[SO_TOP_CELLS] = i;
            cell_cnt[SO_TOP_CELLS] = city_grid[i].cross_n;
            for (j = SO_TOP_CELLS; j >= 1 && cell_cnt[j-1] < cell_cnt[j]; j--) {
                SWAP(cell_cnt[j], cell_cnt[j-1], tmp);
                SWAP((*top_cells)[j], (*top_cells)[j-1], tmp);
            }
        }
    }

    free(cell_cnt);
}


void handle_signal(int signum)
{
    switch (signum) {
    case SIGTERM:
    case SIGINT: /* Terminazione forzata */
        free(tstats);
        free(sources_pos);
        kill(printer, SIGTERM);
        waitpid(printer, NULL, WNOHANG);
        term_kids(sources, SO_SOURCES);
        term_kids(taxis, taxis_i);
        terminate();
        break;
    case SIGALRM:
        end_simulation();
        terminate();
        break;
    default:
        break;
    }
}


void term_kids(pid_t *kids, int nkids)
{
    int i;

    for (i = 0; i < nkids; i++) {
        kill(kids[i], SIGTERM);
    }
    while (wait(NULL) != -1);
    free(kids);
    fprintf(stderr, "Processi figli terminati ed array di PID deallocato.\n");
}


void terminate()
{
    shmdt(city_grid);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    msgctl(statsq_id, IPC_RMID, NULL);
    fprintf(stderr, "Risorse IPC deallocate con successo.\n");
    exit(EXIT_SUCCESS);
}