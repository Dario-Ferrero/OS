#ifndef __SORGENTE_H__
#define __SORGENTE_H__

/*
 * Genera nreqs richieste con destinazione casuale (ma accessibile)
 * e le invia sulla propria coda di messaggi
 */
void create_requests(int nreqs);

/*
 * Signal handler per il processo
 */
void handle_signal(int signum);

/*
 * Terminazione forzata : rilascia le risorse
 */
void terminate();

#endif /* __SORGENTE_H__ */