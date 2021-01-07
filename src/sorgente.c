#include "../lib/common.h"
#include "../lib/sorgente.h"

int sem_id, msq_id, source_pos;
struct sembuf sops;
Cell *city_grid;

int main(int argc, char *argv[])
{
    int shm_id, reqs_rate;
    struct sigaction sa;
    sigset_t sig_mask;
    struct timespec slp_time, time_left, tmp;
    Request req;

    /* 
     * Processo creato dal master, leggere i parametri passati tramite execve
     * - posizione della propria cella
     * - numero di richieste da inviare ogni intervallo di tempo
     */

    source_pos = atoi(argv[1]);
    reqs_rate = atoi(argv[2]);

    /* Gestire maschere / stabilire l'handler per i 3/4 segnali */

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handle_signal;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);

    /* Accedere all'array di semafori per sincronizzarmi col master. */

    if ((sem_id = semget(getppid(), NSEMS, 0666)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }

    /* Accedere alla mia coda di messaggi per le richieste */

    if ((msq_id = msgget(IPC_PRIVATE, 0666)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }

    /* Devo scrivere l'id della coda nella sua cella, così che i taxi vi possano accedere */

    if ((shm_id = shmget(getppid(), GRID_SIZE * sizeof(*city_grid), 0600)) == -1) {
        TEST_ERROR;
        terminate();
    }
    city_grid = (Cell *)shmat(shm_id, NULL, 0);
    TEST_ERROR;
    city_grid[source_pos].msq_id = msq_id;

    /* Invio reqs_rate richieste iniziali */

    srand(getpid() + time(NULL));
    create_requests(reqs_rate);

    /* Quando sono pronto per la simulazione faccio signal su SEM_KIDS e wait for zero su SEM_START */

    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGUSR1);

    SEMOP(sem_id, SEM_KIDS, 1, 0);
    TEST_ERROR;

    SEMOP(sem_id, SEM_START, 0, 0);
    TEST_ERROR;

    /* Simulazione iniziata, posso entrare nel ciclo infinito di generazione di richieste */

    while (1) {
        slp_time.tv_sec = BURST_INTERVAL;
        slp_time.tv_nsec = 0;
        while (nanosleep(&slp_time, &time_left) && errno == EINTR) {
            errno = 0;
            tmp = slp_time;
            slp_time = time_left;
            time_left = tmp;
        }
        create_requests(reqs_rate);
    }
}

void handle_signal(int signum)
{
    switch (signum) {
    case SIGINT: /* Ctrl-C da terminale */
    case SIGTERM: /* "End Process" nel manager di sistema (NO SIGKILL) */
        terminate();
        break;
    case SIGUSR1:
        /* Genera una richiesta ed inseriscila nella coda */
        create_requests(1);
        break;
    case SIGALRM:
        send_stats();
        terminate();
        break;
    default:
        break;
    }
}


void create_requests(int nreqs)
{
    int i;
    Request req;

    req.mtype = source_pos + 1;
    for (i = 0; i < nreqs; i++) {
        do {
            req.dest_cell = RAND_RNG(0, GRID_SIZE-1);
        } while (IS_HOLE(city_grid[req.dest_cell]) || req.dest_cell == source_pos);
        msgsnd(msq_id, &req, MSG_LEN(req), 0);
    }
}


void send_stats()
{
    int statsq_id;
    SourceStats stats;
    struct msqid_ds buf;

    if ((statsq_id = msgget(getppid(), 0600)) == -1) {
        TEST_ERROR;
        fprintf(stderr, "%s: PID #%5d: Errore nell'apertura della coda per le statistiche.\n", __FILE__, getpid());
        exit(EXIT_FAILURE);
    }
    if (msgctl(msq_id, IPC_STAT, &buf) == -1) {
        TEST_ERROR;
        fprintf(stderr, "%s: PID #%5d: Errore nella msgctl con IPC_STAT.\n", __FILE__, getpid());
        exit(EXIT_FAILURE);
    }
    stats.mtype = SOURCE_MTYPE;
    stats.reqs_unpicked = buf.msg_qnum;
    msgsnd(statsq_id, &stats, MSG_LEN(stats), 0);
}


void terminate()
{
    shmdt(city_grid);
    msgctl(msq_id, IPC_RMID, NULL);
    exit(EXIT_FAILURE);
}