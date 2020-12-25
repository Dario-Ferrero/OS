#ifndef __SORGENTE_H__
#define __SORGENTE_H__

/*
 * Signal handler per il processo
 */
void handle_signal(int signum);

/*
 * Terminazione forzata : rilascia le risorse
 */
void terminate();

#endif /* __SORGENTE_H__ */