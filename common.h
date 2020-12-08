#ifndef __COMMON_H__
#define __COMMON_H__

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define TEST_ERROR    if (errno) {fprintf(stderr,			\
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}

#define PRINT_INT(n) fprintf(stderr, "%s:%i: %s = %i\n", __FILE__, __LINE__, #n, n)
#define PRINT_LONG_I(n) fprintf(stderr, "%s:%i: %s = %li\n", __FILE__, __LINE__, #n, n)

#define SO_WIDTH  20
#define SO_HEIGHT 10

#endif /* __COMMON_H__ */