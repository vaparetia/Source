/*
		header file for system link variable
*/

#define GM_State1		(*( short * )( linkvarbuf + 0 ))
#define GM_State2		(*( short * )( linkvarbuf + 2 ))
#define GM_State5		(( int *)( linkvarbuf + 4 ))
#define GM_State3		(( short * )( linkvarbuf + 12 ))
#define GM_State4		(*( short * )( linkvarbuf + 22 ))

#define MAX_LINKVARBUF 24

#ifdef __VARIABLE_C__
char linkvarbuf[ MAX_LINKVARBUF ];
#else
extern char linkvarbuf[ MAX_LINKVARBUF ];
#endif

