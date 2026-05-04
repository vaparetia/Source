#ifndef _debug_h_
#define _debug_h_

#ifdef DEBUG
#define DBG(args...)   { fprintf(stderr, args); fflush(stderr); }
#else
#define DBG(args...)
#endif /* DEBUG */

#endif /* _debug_h_ */
