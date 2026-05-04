
#ifndef _MY_H_
#define _MY_H_

/*********************************************************************

	共通ヘッダファイル
	
	1999/06/16		NISINO Motoaki

**********************************************************************/

#if 0

typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef char	BYTE;

#endif


#if 1
typedef char	BYTE;
#endif


#if 0
typedef short			WORD;
typedef long			DWORD;
#endif

#define ON        1
#define OFF       0

#ifndef TRUE
#define TRUE      1
#endif

#ifndef FALSE
#define FALSE     0
#endif

#ifndef NULL
#define NULL      0
#endif

#define HANI_SEIGEN(x,l,h)		((x)=((x)<(l)?(l):(x)>(h)?(h):(x)))
#define SYUUSOKU(cx,ax,count)	(cx=(cx>ax+count)?(cx-count):(cx<ax-count)?(cx+count):ax)	
#define MIN_(min,x)				((x)=(min)<(x)?(x):(min))
#define ON_OFF(x)				(x=x?0:1)

#ifndef _RELEASE_
#define dprint(expr) printf(#expr " = %d\n",(int)expr)
#define dprintx(expr) printf(#expr " = %x\n",expr)
#define dprints(expr) printf(#expr " = %s\n",expr)
#define dprintf(expr...) printf(##expr)
#else
#define dprint(expr) 
#define dprintx(expr) 
#define dprints(expr) 

#define dprintf(expr...) 
#endif


extern void my_assert_disp(char *file_name, int line, const char *fmt, ...);


#if 0
#define ASSERT(a) { if(!(a)) printf("ASSERT!!  " #a "  %s(%d)\n",__FILE__,__LINE__); }

#define ASSERT2(x, fmt, arg...) \
    {	if (!(x)){ printf("ASSERT!!  " #x );\
			my_assert_disp(__FILE__, __LINE__, (fmt), ## arg);}	} exit(1);

#else

#define ASSERT(a) \
	{ if(!(a)){ \
		printf("ASSERT!!  %s(%d) ",__FILE__,__LINE__ );\
		printf(#a"\n"); \
		}}


#define ASSERT2(x, fmt, arg...) \
    {	if (!(x)){ \
			printf("ASSERT!!  ");\
			printf("%s(%d) ",__FILE__,__LINE__);\
			printf(#x"\n");\
			my_assert_disp2( (fmt), ## arg);\
		}\
	} exit(1);
#endif


/* ex.
	ASSERT(!"Message");
	ASSERT(pointer!=NULL && "Message");

	int a=-1;
	ASSERT(a>0);
	ASSERT2(a>0  , "invalid value a=%d\n",a);

*/		




#define _MAKE_ONE_BMP_

typedef struct _HL {
	short	l,h;		/* little endian */
} HL;

typedef struct _UHL {
	unsigned short	l,h;	/* little endian */
} UHL;

typedef union _ENDIAN {
	long    d;
	HL      m;
	long	l;
	unsigned long	ul;
	HL	s;
	UHL	us;
} ENDIAN;





#endif
