#include "common.h"

#define PARAMS_FILE "params.conf"
#define READ_LEN (19 + 16)
#define N_PARAMS 9

/*
 * Legge il file specificato ed inizializza le variabili globali SO
 */
void read_params();

static int SO_HOLES;
static int SO_SOURCES;
static int SO_CAP_MIN;
static int SO_CAP_MAX;
static int SO_TAXI;
static int SO_TIMENSEC_MIN;
static int SO_TIMENSEC_MAX;
static int SO_TIMEOUT;
static int SO_DURATION;

int main(int argc, char *argv[])
{
    pid_t *taxis, *sources, child_pid;
    long i;
    int status;

    read_params();
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
    SO_SOURCES      = params[1];
    SO_CAP_MIN      = params[2];
    SO_CAP_MAX      = params[3];
    SO_TAXI         = params[4];
    SO_TIMENSEC_MIN = params[5];
    SO_TIMENSEC_MAX = params[6];
    SO_TIMEOUT      = params[7];
    SO_DURATION     = params[8];

    free(params);
    fclose(in);
}