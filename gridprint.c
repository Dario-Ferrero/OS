#include "common.h"
#include "gridprint.h"


void print_initial_grid(Cell *city_grid)
{
    int x, y;

    PRINT_LEGEND;

    PRINT_HEADER;

    for (y = 0; y < SO_HEIGHT; y++) {
        printf(" %3d | ", y);
        for (x = 0; x < SO_WIDTH; x++) {
            if (IS_HOLE(city_grid[INDEX(x, y)])) {
                printf("H ");
            } else if (IS_SOURCE(city_grid[INDEX(x, y)])) {
                printf("S ");
            } else {
                printf("` ");
            }
        }
        printf("|\n");
    }

    PRINT_FOOTER;

    fflush(stdout);
}


void print_grid_state(int sem_id, Cell *city_grid)
{
    int x, y, n_taxi;

    PRINT_LEGEND;

    PRINT_HEADER;

    for (y = 0; y < SO_HEIGHT; y++) {
        printf(" %3d | ", y);
        for (x = 0; x < SO_WIDTH; x++) {
            n_taxi = city_grid[INDEX(x, y)].capacity -
                     semctl(sem_id, INDEX(x, y), GETVAL);
            if (IS_HOLE(city_grid[INDEX(x, y)])) {
                printf(ANSI_RED"H "ANSI_RESET);
            } else if (n_taxi) {
                if (IS_SOURCE(city_grid[INDEX(x, y)])) {
                    printf(ANSI_YELLOW"%d "ANSI_RESET, n_taxi);
                } else {
                    printf(ANSI_CYAN"%d "ANSI_RESET, n_taxi);
                }
            } else if (IS_SOURCE(city_grid[INDEX(x, y)])) {
                printf(ANSI_YELLOW"S "ANSI_RESET);
            } else {
                printf("` ");
            }
        }
        printf("|\n");
    }
    
    PRINT_FOOTER;

    fflush(stdout);
}


void print_final_grid(Cell *city_grid, int *top_cells, int ntops)
{
    int i, x, y;
    int8_t top;

    PRINT_LEGEND;

    PRINT_HEADER;

    for (y = 0; y < SO_HEIGHT; y++) {
        printf(" %3d | ", y);
        for (x = 0; x < SO_WIDTH; x++) {
            top = FALSE;
            for (i = 0; i < ntops; i++) {
                if (top_cells[i] == INDEX(x, y)) {
                    top = TRUE;
                    break;
                }
            }
            if (top) {
                printf(ANSI_GREEN"T "ANSI_RESET);
            } else if (IS_SOURCE(city_grid[INDEX(x, y)])) {
                printf(ANSI_YELLOW"S "ANSI_RESET);
            } else if (IS_HOLE(city_grid[INDEX(x, y)])) {
                printf(ANSI_RED"H "ANSI_RESET);
            } else {
                printf("` ");
            }
        }
        printf("|\n");
    }

    PRINT_FOOTER;

    fflush(stdout);
}