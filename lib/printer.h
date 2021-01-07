#ifndef __PRINTER_H__
#define __PRINTER_H__

/*
 * Signal handler per il processo
 */
void handle_signal(int signum);

/*
 * Terminazione forzata : rilascia le risorse
 */
void terminate();

#endif /* __PRINTER_H__ */