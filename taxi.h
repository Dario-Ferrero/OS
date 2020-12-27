#ifndef __TAXI_H__
#define __TAXI_H__

#define ABS(n) (n < 0 ? -(n) : n)
/* 
 * Stima la posizione della cella sorgente più vicina a cur_pos tra le
 * n_src presenti in src_pos tramite la distanza di Manhattan.
 */
int best_pos(int cur_pos, int *src_pos, int n_src);


/*
 * Signal handler per il processo
 */
void handle_signal(int signum);


#endif /* __TAXI_H__ */