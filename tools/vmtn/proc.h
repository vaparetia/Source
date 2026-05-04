#ifndef _proc_h_
#define _proc_h_

#ifndef _PAL_
#define TIME_BASE  5
#else
#define TIME_BASE  6
#endif /* _PAL_ */

#ifndef _proc_c_
#define EXT extern
#define INIT(n)
#else
#define EXT
#define INIT(n)  = n
#endif /* _proc_c_ */



#undef EXT
#undef INIT(n)

int    procLoadAllData(int argc, char **argv);
char * procGetSeqName(void);
int    procMakeMotion(char * prefix);
void   procReleaseAllData(void);

#endif /* _proc_h_ */
