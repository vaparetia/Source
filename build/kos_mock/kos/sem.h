#pragma once
typedef struct { int count; } semaphore_t;
static inline int sem_init(semaphore_t *s, int c) { s->count=c; return 0; }
static inline int sem_destroy(semaphore_t *s)     { (void)s; return 0; }
static inline int sem_signal(semaphore_t *s)      { (void)s; return 0; }
static inline int sem_wait(semaphore_t *s)        { (void)s; return 0; }
static inline int sem_trywait(semaphore_t *s)     { (void)s; return 0; }
