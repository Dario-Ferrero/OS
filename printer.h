#ifndef __PRINTER_H__
#define __PRINTER_H__

/*
 * Terminazione forzata : rilascia le risorse
 */
void terminate();

/*
 * Signal handler per il processo
 */
void handle_signal(int signum);

#endif /* __PRINTER_H__ */