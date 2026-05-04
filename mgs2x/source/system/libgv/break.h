/*
	break.h
		ハードウエアブレークポイント設定
	2000/02/08	K.Uehara
	$Id: break.h,v 1.1.1.3 2002/11/19 11:42:43 Yoshizawa1 Exp $
*/

/* ----------------------------------------------------------- */

#define HWB_IAE		0x80000000	// 命令アドレス
#define HWB_DRE		0x40000000	// データリード
#define HWB_DWE		0x20000000	// データライト
#define HWB_DVE		0x10000000	// データバリュー (R/Wと同時指定)
#define HWB_IUE		0x04000000	//   命令 / USER
#define HWB_ISE		0x02000000	//   命令 / SYSTEM
#define HWB_IKE		0x01000000	//   命令 / KERNEL
#define HWB_IXE		0x00800000	//   命令 / EXCEPTION
#define HWB_DUE		0x00200000	//   データ / USER
#define HWB_DSE		0x00100000	//   データ / SYSTEM
#define HWB_DKE		0x00080000	//   データ / KERNEL
#define HWB_DXE		0x00040000	//   データ / EXCEPTION

#define HWB_ITE		0x00020000
#define HWB_DTE		0x00010000
#define HWB_BED		0x00008000

#define HWB_DWB		0x00000004
#define HWB_DRB		0x00000002
#define HWB_IAB		0x00000001

//#define HWB_D_ALL	( HWB_DUE | HWB_DKE | HWB_DXE | HWB_DSE )
#define HWB_D_ALL	( HWB_DUE | HWB_DKE | HWB_DXE )

#define GV_BREAK_CONTROL_WRITE		( HWB_DWE | HWB_D_ALL )
#define GV_BREAK_CONTROL_READ		( HWB_DRE | HWB_D_ALL )
#define GV_BREAK_CONTROL_VALUE		( HWB_DWE | HWB_DRE | HWB_DVE | HWB_D_ALL )

/* ----------------------------------------------------------- */

#define GV_BREAK()			{ asm volatile( "break" ); }
#define GV_BREAK_SET_END()	{ asm volatile( "sync.p" ); }

extern inline void GV_BREAK_SET_CONTROL( int flag )
{
	asm volatile( "mtbpc  %0" :: "r"( flag ) );
}

extern inline void GV_BREAK_SET_ADDRESS( void *ptr )
{
	asm volatile( "mtdab  %0" :: "r"( ptr ) );
}

extern inline void GV_BREAK_SET_ADDRESS_MASK( int mask )
{
	asm volatile( "mtdabm %0" :: "r"( mask ) );
}

extern inline void GV_BREAK_SET_VALUE( int value )
{
	asm volatile( "mtdvb  %0" :: "r"( value ) );
}

extern inline void GV_BREAK_SET_VALUE_MASK( int mask )
{
	asm volatile( "mtdvbm %0" :: "r"( mask ) );
}

/* ----------------------------------------------------------- */

extern inline void GV_BREAK_WRITE_CHECK( void *ptr )
{
	GV_BREAK_SET_CONTROL( GV_BREAK_CONTROL_WRITE );
	GV_BREAK_SET_ADDRESS( ptr );
	GV_BREAK_SET_ADDRESS_MASK( 0xFFFFFFFF );
	GV_BREAK_SET_END();
}

extern inline void GV_BREAK_VALUE_CHECK( void *ptr, int value )
{
	GV_BREAK_SET_CONTROL( GV_BREAK_CONTROL_VALUE );
	GV_BREAK_SET_ADDRESS( ptr );
	GV_BREAK_SET_ADDRESS_MASK( 0xFFFFFFFF );
	GV_BREAK_SET_VALUE( value );
	GV_BREAK_SET_VALUE_MASK( 0xFFFFFFFF );
	GV_BREAK_SET_END();
}

extern inline void GV_BREAK_CLEAR( void )
{
	GV_BREAK_SET_CONTROL( 0 );
	GV_BREAK_SET_END();
}


