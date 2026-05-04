#ifndef _fname_h_
#define _fname_h_

#ifndef _fname_c_
#define EXT extern
#define INIT(_n...)
#else
#define EXT
#define INIT(_n...)  = _n
#endif /* _fname_c_ */

/* このモジュールにグローバル変数は存在しないが、一応。 */

#undef EXT
#undef INIT(_n...)

int    fnmLengthSuffixExchange(char * name, char * suffix);
char * fnmExchangeSuffix(char * buf, int maxlen, char * name, char * suffix);
char * fnmMakeSuffixExchanged(char * name, char * suffix);

#endif /* _fname_h_ */
