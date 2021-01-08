#ifndef __GRIDPRINT_H__
#define __GRIDPRINT_H__

/*
 * Codici per stampa colorata a terminale
 */

#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_RED 	   "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_RESET 	"\x1b[0m"

#define PRINT_INTERVAL 1

#define PRINT_LEGEND printf("\n\n\n");                \
                     printf("Chiave di lettura:\n");  \
                     printf(ANSI_YELLOW"S"ANSI_RESET" : cella sorgente\n");      \
                     printf(ANSI_RED"H"ANSI_RESET" : cella inaccessibile\n");    \
                     printf(ANSI_CYAN"n"ANSI_RESET" : taxi sulla cella\n");      \
                     printf(ANSI_GREEN"T"ANSI_RESET" : top cell\n\n");


#define PRINT_HEADER printf("       ");                 \
                     for (x = 0; x < SO_WIDTH; x++) {   \
                        printf("%d ", x % 10);          \
                     }                                  \
                     printf("\n      ");                \
                     for (x = 0; x < SO_WIDTH; x++) {   \
                        printf("--");                   \
                     }                                  \
                     printf("-\n");

#define PRINT_FOOTER printf("      ");                  \
                     for (x = 0; x < SO_WIDTH; x++) {   \
                        printf("--");                   \
                     }                                  \
                     printf("-\n\n\n");

/*
 * Stampa a terminale la city_grid mostrandone le celle inaccessibili
 * e le celle sorgente.
 */
void print_initial_grid(Cell *city_grid);

/*
 * Stampa lo stato di occupazione della city_grid da parte dei taxi
 * durante la simulazione, rappresentando anche celle sorgenti ed inaccessibili.
 */
void print_grid_state(int sem_id, Cell *city_grid);

/*
 * Stampa a terminale la city_grid con evidenziate le celle più
 * attraversate assieme alle celle sorgenti.
 */
void print_final_grid(Cell *city_grid, int *top_cells, int ntops);

#endif /* __GRIDPRINT_H__ */