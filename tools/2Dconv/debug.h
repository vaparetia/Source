#ifndef _debug_h_
#define _debug_h_

#ifndef _DEBUG_
#define DBG(args...)
#define ABORT(ex, args...)
#else
#define DBG(args...)   { fprintf(stderr, args);  fflush(stderr); }
#define ABORT(ex, args...) { fprintf(stderr, args); fflush(stderr); exit(ex); }
#endif /* _DEBUG_ */

#endif /* _debug_h_ */
