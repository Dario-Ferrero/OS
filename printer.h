#ifndef __PRINTER_H__
#define __PRINTER_H__

/*
 * Stampa lo stato della city_grid durante la simulazione
 */
void print_grid_state();

/*
 * Terminazione forzata : rilascia le risorse
 */
void terminate();

/*
 * Signal handler per il processo
 */
void handle_signal(int signum);

#endif /* __PRINTER_H__ */