#ifndef _util_h_
#define _util_h_


#define INIT_POOL_SIZE  4096
#define POOL_ADD  1024

#ifndef _util_c_
#define EXT extern
#define INIT(_n...)
#else
#define EXT
#define INIT(_n...)  = _n
#endif /* _util_c_ */


EXT char *poolbuf INIT(NULL);
EXT int  pool_siz INIT(0);
EXT int  poolcnt INIT(0);

#undef EXT
#undef INIT(_n...)

int pool_init(void);
int pool(char * str);

#endif /* _util_h_ */
