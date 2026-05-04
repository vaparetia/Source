
#ifndef _MY_H_
#define _MY_H_

/*********************************************************************

	共通ヘッダファイル
	
	1999/06/16		NISINO Motoaki

**********************************************************************/

#if 1
#ifndef u_char
typedef	unsigned char	u_char;
#endif

#ifndef u_short
typedef	unsigned short	u_short;
#endif

#ifndef u_int
typedef	unsigned int	u_int;
#endif

#ifndef u_long
typedef	unsigned long	u_long;
#endif

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


#define dprint(expr) printf(#expr " = %d\n",(int)expr)
#define dprintx(expr) printf(#expr " = %x\n",expr)
#define dprints(expr) printf(#expr " = %s\n",expr)
#define dprintf(expr) printf(#expr " = %f\n",expr)


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
	u_long	ul;
	HL	s;
	UHL	us;
} ENDIAN;



#endif
