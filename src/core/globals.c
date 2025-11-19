#include "../../include/malloc_internal.h"

t_zone_manager g_manager = {0};
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
