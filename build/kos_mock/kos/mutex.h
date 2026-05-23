#pragma once
#define MUTEX_TYPE_NORMAL     0
#define MUTEX_TYPE_RECURSIVE  2
typedef struct { int type; int locked; } mutex_t;
static inline int mutex_init(mutex_t *m, int t)  { m->type=t; m->locked=0; return 0; }
static inline int mutex_destroy(mutex_t *m)       { (void)m; return 0; }
static inline int mutex_lock(mutex_t *m)           { (void)m; return 0; }
static inline int mutex_trylock(mutex_t *m)        { (void)m; return 0; }
static inline int mutex_unlock(mutex_t *m)         { (void)m; return 0; }
