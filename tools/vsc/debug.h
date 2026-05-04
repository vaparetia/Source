#ifndef _debug_h_
#define _debug_h_

#ifdef _GCC_

#ifdef _DEBUG_
#define DEBUG(args...)   fprintf(stderr, args);  fflush(stderr)
#define DBG(args...)   fprintf(stderr, args);  fflush(stderr)
#else
#define DEBUG(args...)
#define DBG(args...)
#endif /* _DEBUG_ */

#else /* _GCC_ */

int DEBUG(char *format, ...);
int DBG(char *format, ...);

#endif /* _GCC_ */

#endif /* _debug_h_ */
