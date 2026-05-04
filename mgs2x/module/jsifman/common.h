/*
	common.h
	KCEJ SIF MANAGER 用のヘッダファイル
	EE, IOP共通

	1999/11/17	K.Uehara
	$Id: common.h,v 1.2 2000/06/21 12:45:59 usr01475 Exp $
*/


/*
	GPの復帰と退避を行なうマクロ
*/

#ifdef IOP

#if 0	// -G0 オプションをつけない時は必要
#define FUNC_INIT() \
	unsigned long oldgp; \
	asm volatile( "  move %0, $gp; la  $gp, _gp" : "=r" (oldgp));

#define FUNC_EXIT() \
    asm volatile( "  move $gp, %0"  : : "r" (oldgp));
#else	// EE
#define FUNC_INIT()
#define FUNC_EXIT()
#endif

#define INT_CTRL	int old_is_EI
#define IOP_EI()	CpuResumeIntr( old_is_EI )
#define IOP_DI()	CpuSuspendIntr( &old_is_EI )
#define EI()		CpuResumeIntr( old_is_EI )
#define DI()		CpuSuspendIntr( &old_is_EI )
#define iPrintf		Kprintf

extern inline void copy128( void *dst, void *src )
{
	typedef struct { unsigned long d1, d2, d3, d4 ; } Long4;

	*( Long4 * )dst = *( Long4 * )src;
}

#else if defined( EE )

#define FUNC_INIT()
#define FUNC_EXIT()
#define IOP_DI()
#define IOP_EI()
#define INT_CTRL
#define iPrintf		scePrintf

extern inline void copy128( void *dst, void *src )
{
	*( u_long128 * )dst = *( u_long128 * )src;
}

#endif

