#include "common.h"
#include "sorgente.h"

int sem_id;
struct sembuf sops;

int main(int argc, char *argv[])
{
    int pos;

    pos = atoi(argv[1]);
    
    fprintf(stderr, "Sorgente creata! La mia posizione è : %d\n", pos);

    if ((sem_id = semget(getppid(), NSEMS, 0666)) == -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }

    sops.sem_num = SEM_KIDS;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    semop(sem_id, &sops, 1);

    exit(EXIT_SUCCESS);


    /* 
     * Processo creato dal master, leggere i parametri passati tramite execve
     * - posizione della propria cella
     * - altri (?)
     */

    /* Gestire maschere / stabilire l'handler per i 3/4 segnali */

    /*
     * Accedere all'array di semafori per sincronizzarmi col master.
     * 
     * sem_id = semget(getppid(), NULL, SOMETHING);
     */



    /*
     * Accedere ad almeno una coda di messaggi :
     * * se una condivisa : msgget(getppid(), 0666);
     * * se mia personale : msgget(getpid(), 0666);
     * 
     * #define N_REQS ((SO_TAXI / SO_SOURCES) > 0 ? (SO_TAXI / SO_SOURCES) : 1)
     * N_REQS dipendente dal numero di taxi e dal numero di sorgenti
     * - ad ogni burst, SO_TAXI / SO_SOURCES richieste create da ogni sorgente
     * - se SO_TAXI < SO_SOURCES, un valore di default maggiore di 1 può andare bene (e.g. capacity della cella?)
     */


    /*
     * Quando sono pronto faccio signal su SEM_KIDS e wait for zero su SEM_START
     * 
     * semop(sem_id, plus, SEM_KIDS, 1);
     * semop(sem_id, wait0, SEM_START, 1);
     */


    /* Simulazione iniziata, posso entrare nel ciclo infinito di generazione di richieste */

    while (1) {
        break;

        /*
         * * Faccio partire l'alarm(1)
         * * Attesa attiva o pause() ? (di fatto non ho nulla da fare...)
         */
    }
}

void handle_signal(int signum)
{
    switch (signum)
    {
    case SIGINT: /* Ctrl-C da terminale */
    case SIGTERM: /* "End Process" nel manager di sistema (NO KILL) */
        /* Terminazione forzata : free(), shmdt(), invia qualcosa al master (?)
         * se coda propria della sorgente, inviare al master il n di richieste ancora inevase
         * (exit status o messaggio sulla coda delle statistiche), poi rimuovi la coda
         */
        break;
    
    /*
     * PROBLEMA :
     * Sorgente in attesa per inserimento di 1+ richieste --> arrivo di altri segnali
     * * quali mascherare (e quali no) nell'handler ?
     */
    case SIGALRM:
        /* Genera N_REQS richieste ed inseriscile nella coda */
        break;
    case SIGUSR1:
        /* Genera una richiesta ed inseriscila nella coda */
        break;
    default:
        break;
    }
}