#ifndef __GRIDPRINT_H__
#define __GRIDPRINT_H__

#define PRINT_INTERVAL 1

#define PRINT_HEADER printf("\n\n\n       ");           \
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