//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mt3_effct.c
	モーション加工ユーティリティ

	1999/07/07 K.Takabe
	$Id: mt_effct.c,v 1.1.1.3 2002/11/19 11:42:53 Yoshizawa1 Exp $

*/
/*
	＜ＰＳＹ＞メタルギアソリッド２

	モーション加工ユーティリティ

	1999.6.14	K.Takabe

*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libgv.h"
#include	"libdg.h"
#include	"libmt.h"


/*----------------------------------------------------------------*/






/*----------------------------------------------------------------*/



/*----------------------------------------------------------------*/
void MT_ReversalMotion( FVECTOR *rots, int flag )
{
	int		i, max_joints, type ;
	u_long64	mask = 0 ;
	volatile FVECTOR	*quat ;
	volatile u_long128	*rots_data ;
	u_long128	tmp ;

	if ( !( flag & ( MT_FLAG_REVERSAL1 | MT_FLAG_REVERSAL2 ) ) ) return ;

	switch ( flag & MT_FLAG_TYPE_MASK ){
	  case MT_FLAG_HUMAN1:
		type = 0 ;
		max_joints = 16 ;
		if ( flag & MT_FLAG_REVERSAL1 ) mask |= I64(0x03fe) ;
		if ( flag & MT_FLAG_REVERSAL2 ) mask |= I64(0xfc01) ;
		break ;
	  case MT_FLAG_HUMAN2:
		type = 1 ;
		max_joints = 21 ;
		if ( flag & MT_FLAG_REVERSAL1 ) mask |= I64(0x001ffe) ;
		if ( flag & MT_FLAG_REVERSAL2 ) mask |= I64(0x1fe001) ;
		break ;
	  case MT_FLAG_PDRAY:
		type = 2 ;
		max_joints = 33 ;
		if ( flag & MT_FLAG_REVERSAL1 ) mask |= I64(0x0001ffffe) ;
		if ( flag & MT_FLAG_REVERSAL2 ) mask |= I64(0x1ffe00001) ;
		break ;
	  default:
		return ;
		break ;
	}
#ifdef BP_PSX2_ASM
	asm ("vmulx.xyzw vf2,vf2,vf0");
#endif	
	quat = rots ;
	for ( i = max_joints ; i > 0 ; i-- ){
		if ( mask & 0x01 ){
#ifdef BP_PSX2_ASM
			asm ("
				lqc2	vf1,0(%0)
				vsub.xw	vf1,vf2,vf1
				sqc2	vf1,0(%0)
			"::"r"(quat));
#else
			quat->x = -quat->x;
			quat->w = -quat->w;
#endif			
		}
		mask >>= 1 ;
		quat++ ;
	}

	rots_data = (u_long128*)rots ;
	switch( type ){
		int	a, b ;
	  case 0:
		/* 人型１・上半身対象関節同士の入れ替え */
		if ( flag & MT_FLAG_REVERSAL1 ){
			static int swap_data[3] = {0x0207, 0x0308, 0x0409 };
			for ( i = 0 ; i < 3 ; i++ ){
				a = swap_data[ i ] & 0xff ;
				b = swap_data[ i ] >> 8 ;
				tmp = rots_data[a] ; rots_data[a] = rots_data[b] ; rots_data[b] = tmp ;
			}
		}
		/* 人型１・下半身対象関節同士の入れ替え */
		if ( flag & MT_FLAG_REVERSAL2 ){
			static int swap_data[3] = {0x0a0d, 0x0b0e, 0x0c0f };
			for ( i = 0 ; i < 3 ; i++ ){
				a = swap_data[ i ] & 0xff ;
				b = swap_data[ i ] >> 8 ;
				tmp = rots_data[a] ; rots_data[a] = rots_data[b] ; rots_data[b] = tmp ;
			}
		}
		break ;
	  case 1:
		/* 人型２・上半身対象関節同士の入れ替え */
		if ( flag & MT_FLAG_REVERSAL1 ){
			static int swap_data[4] = {0x0307, 0x0408, 0x0509, 0x060a };
			for ( i = 0 ; i < 4 ; i++ ){
				a = swap_data[ i ] & 0xff ;
				b = swap_data[ i ] >> 8 ;
				tmp = rots_data[a] ; rots_data[a] = rots_data[b] ; rots_data[b] = tmp ;
			}
		}
		/* 人型２・下半身対象関節同士の入れ替え */
		if ( flag & MT_FLAG_REVERSAL2 ){
			static int swap_data[4] = {0x0d11, 0x0e12, 0x0f13, 0x1014 };
			for ( i = 0 ; i < 4 ; i++ ){
				a = swap_data[ i ] & 0xff ;
				b = swap_data[ i ] >> 8 ;
				tmp = rots_data[a] ; rots_data[a] = rots_data[b] ; rots_data[b] = tmp ;
			}
		}
		break ;
	  case 2:
		/* 量産型ＲＡＹ・上半身対象関節同士の入れ替え */
		if ( flag & MT_FLAG_REVERSAL1 ){
			static int swap_data[7] = {(3<<8)|8, (4<<8)|9, (5<<8)|10, (6<<8)|11, (7<<8)|12, (17<<8)|19, (18<<8)|20 };
			for ( i = 0 ; i < 7 ; i++ ){
				a = swap_data[ i ] & 0xff ;
				b = swap_data[ i ] >> 8 ;
				tmp = rots_data[a] ; rots_data[a] = rots_data[b] ; rots_data[b] = tmp ;
			}
		}
		/* 量産型ＲＡＹ・下半身対象関節同士の入れ替え */
		if ( flag & MT_FLAG_REVERSAL2 ){
			static int swap_data[6] = {(21<<8)|27, (22<<8)|28, (23<<8)|29, (24<<8)|30, (25<<8)|31, (26<<8)|32 };
			for ( i = 0 ; i < 6 ; i++ ){
				a = swap_data[ i ] & 0xff ;
				b = swap_data[ i ] >> 8 ;
				tmp = rots_data[a] ; rots_data[a] = rots_data[b] ; rots_data[b] = tmp ;
			}
		}
		break ;
	}

}
