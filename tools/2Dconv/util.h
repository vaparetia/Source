#ifndef _util_h_
#define _util_h_

#ifndef _util_c_
#define EXT extern
#define INIT(_n...)
#else
#define EXT
#define INIT(_n...)   = _n
#endif



#undef EXT
#undef INIT(_n...)


int            search_char(int c, char *str);
int            str_to_int(int * num, char * str);
int            str_to_float(float * fnum, char * str);
int            strcode(char * str);

int            fputl(long l, FILE * wfp);
int            fputw(short s, FILE * wfp);

unsigned short short_float(float f);

#endif /* _util_h_ */
