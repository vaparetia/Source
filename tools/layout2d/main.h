#ifndef _main_h_
#define _main_h_

#ifndef _main_c_
#define EXT extern
#define INIT(_ini...)
#else
#define EXT
#define INIT(_ini...)   = _ini
#endif /* _main_c_ */

EXT  char * dirSearch INIT( NULL );

#undef EXT
#undef INIT


#endif /* _main_h_ */
