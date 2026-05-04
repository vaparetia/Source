#ifndef _voice_h_
#define _voice_h_

#include "analyze.h"

#ifdef _voice_c_
#define EXT
#define INIT(n)  = n
#else
#define EXT extern
#define INIT(n)
#endif /* _voice_c_ */



#undef EXT
#undef INIT

int voiAutoAdjustLevel(anaParam **p_list, int f_nums);

#endif /* _voice_h_ */
