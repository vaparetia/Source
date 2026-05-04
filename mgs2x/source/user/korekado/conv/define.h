/*
	define.h

	2000/02/13 Y.Korekado
	$Id: define.h,v 1.1.1.3 2002/11/19 11:44:03 Yoshizawa1 Exp $
	
*/

#ifndef __DEFINE___
#define	__DEFINE___ 1

#define	SW_FLAG_VISIBLE	0x00000001
#define	SW_FLAG_SWITCH1	0x00000002
#define	SW_FLAG_SWITCH2	0x00000004
#define	SW_FLAG_SWITCH3	0x00000008
#define	SW_FLAG_SWITCH4	0x00000010
#define	SW_FLAG_SWITCH5	0x00000020
#define	SW_FLAG_SWITCH6	0x00000040
#define	SW_FLAG_BREAK	0x00010000

#define HZXADD_INZONE(a) ( ((a)&255) | ((a)&255) << 8 )
#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world
#define	SET_FLAG( a,b )		(a) |= (b)
#define	UNSET_FLAG( a,b )	(a) &= ~(b)
#define	CLEAR_FLAG( a ) 	(a) = 0
#define ENE_ABS(x) ((x>=0)?(x):(-(x)))
#define ENE_ABSf(x) ((x>=0.0f)?(x):(-(x)))
#define ANDFLAG_CHECK( a,b )	((a) & (b)) == (b) 


inline int BP_AdjustTick(int);
inline int BP_AdjustTick2(int);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) ((float)BP_AdjustTick2(_a))

enum {
	SIDE_D,
	SIDE_R,
	SIDE_U,
	SIDE_L
} ;
#endif
