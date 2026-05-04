#ifndef _main_h_
#define _main_h_

#include "seq.h"

#ifdef _main_c_
#define EXT
#define INIT(n)   = n
#else
#define EXT extern
#define INIT(n)
#endif /* _main_c_ */

EXT int fips_mode   INIT(0);
EXT int big_endian  INIT(0);
EXT int max_size    INIT(MAX_SIZE);

#undef EXT
#undef INIT(n)

#endif /* _main_h_ */
