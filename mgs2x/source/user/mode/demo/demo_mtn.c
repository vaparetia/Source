//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	demo_mtn.c
	モーションパケット処理

	2000/07/26	K.Takabe
	$Id: demo_mtn.c,v 1.1.1.3 2002/11/19 11:45:09 Yoshizawa1 Exp $

*/
/*

	DEMO_MOTION*	DM_GetMotionData( int object_id )
	int		object_id ;		対象となるオブジェクトのユニークＩＤ

	指定したオブジェクトへのモーションがある場合にモーションを取得する


	DEMO_MOTION*	DM_GetMotionDataNext( void )

	指定したオブジェクトへのモーションが複数ある場合に次のモーションを取得する


	--------------------------------
	＜パケット別処理ルーチン用関数＞

	void DM_Packet_Motion( DM_WORK *work, DEMO_MOTION *packet )
	モーションパケット処理

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <mgs_type.h>
#include <libgv.h>
#include <libdg.h>
#include <libfs.h>
#include <stream.h>
#include <gameheader.h>
#include <camera.h>

#include "libdemo.h"

/* ---------------------------------------------------------------- */
static int			GetMotionData_Id ;		/* 再検索用モーションＩＤ */
static int			GetMotionData_Count ;	/* 再検索用残りカウント */
static DM_MOTION	*GetMotionData_Ptr ;	/* 再検索用ポインタ */


static void ExpandMotion( DM_WORK *work, DEMO_MOTION *packet );
static void ExpandMotion2( DM_WORK *work, DEMO_MOTION_INFO *info, u_short *data );
static float MFtoF( unsigned short mf );


void DM_EndianSwapDemoMotion( DEMO_MOTION *packet )
{
   if ( !DEMO_PACKET_IS_ENDIAN_SWAP( packet->header ) )
   {
      int remainingSize;

      DEMO_PACKET_SET_ENDIAN_SWAP( packet->header );

      BP_LE_SwapSInt_Inp( &packet->object_id );
      BP_LE_SwapSInt_Inp( &packet->motion_type );
      BP_LE_SwapSInt_Inp( &packet->start_joint );
      BP_LE_SwapSInt_Inp( &packet->n_joints );
      BP_LE_SwapFloatArray_Inp( &packet->pos.vx, 4 );
      BP_LE_SwapSShortArray_Inp( &packet->rot.vx, 4 );
      BP_LE_SwapSInt_Inp( &( packet->pad[0] ) );
      BP_LE_SwapSInt_Inp( &( packet->pad[1] ) );

      remainingSize = DEMO_PACKET_SIZE(packet->header) - sizeof( *packet );

      if ( packet->motion_type == DEMO_MOTION_COMPRESS )
      {
         // If it's compressed, it's a stream of shorts
         BP_LE_SwapSShortArray_Inp( (short *) packet->motion, remainingSize / sizeof( short ) );
      }
      else
      {
         // Otherwise it's an array of floats
         BP_LE_SwapFloatArray_Inp( (float *) packet->motion, remainingSize / sizeof( float ) );
      }
   }

}
/* ---------------------------------------------------------------- */
/* モーションパケット処理 */
void DM_Packet_Motion( DM_WORK *work, DEMO_MOTION *packet )
{
   DM_EndianSwapDemoMotion( packet );
   
   /* モーションデータの登録 */
	ExpandMotion( work, packet );
}

/* モーションパケット処理 */
void DM_Packet_MotionPack( DM_WORK *work, DEMO_MOTION_PACK *packet )
{
	int		i ;
	DEMO_MOTION_INFO	*info ;
	void	*addr ;

   if ( !DEMO_PACKET_IS_ENDIAN_SWAP( packet->header ) )
   {
      DEMO_PACKET_SET_ENDIAN_SWAP( packet->header );

      BP_LE_SwapSInt_Inp( &packet->n_lists );
      BP_LE_SwapSInt_Inp( &packet->data_offset );
      BP_LE_SwapSInt_Inp( &( packet->pad[0] ) );
      BP_LE_SwapSInt_Inp( &( packet->pad[1] ) );

      for ( i = 0; i < packet->n_lists; ++i )
      {
         DEMO_MOTION_INFO *pList = packet->lists + i;

         BP_LE_SwapSInt_Inp( &pList->object_id );
         BP_LE_SwapUShort_Inp( &pList->flag );
         BP_LE_SwapUShort_Inp( &pList->offset );
         BP_LE_SwapUShort_Inp( &pList->frame );
      }
   }

	info = packet->lists ;
	for ( i = 0 ; i < packet->n_lists ; i++ ){
		/* モーションデータの登録 */
		addr = (void*)( (int)packet + info->offset );
		ExpandMotion2( work, info, addr );
		info++ ;
	}
}

/* ---------------------------------------------------------------- */
	/*
		指定したオブジェクトへのモーションがある場合にモーションを取得する
	*/
DEMO_MOTION*	DM_GetMotionData( int object_id )
{
	DM_WORK		*work ;
	DM_MOTION	*motion ;
	int			i ;

	/* デモ管理構造体へのポインタを取得 */
	if ( ( work = DM_GetDemoWork() ) == NULL ) return NULL ;

	/* モーション対象オブジェクトＩＤで検索 */
	motion = work->mtn_list ;
	for ( i = work->n_motion ; i > 0 ; motion++, i-- ){
		if ( motion->object_id == object_id ){
			/* 再検索用に値を保持 */
			GetMotionData_Id = object_id ;
			GetMotionData_Count = i - 1 ;
			GetMotionData_Ptr = motion + 1 ;
			return ( motion->motion_ptr );
		}
	}

	return NULL ;
}
	/*
		指定したオブジェクトへのモーションが複数ある場合に次のモーションを取得する
	*/
DEMO_MOTION*	DM_GetMotionDataNext( void )
{
	DM_MOTION	*motion ;
	int			i, object_id ;

	/* モーション対象オブジェクトＩＤで検索 */
	object_id = GetMotionData_Id ;
	motion = GetMotionData_Ptr ;
	for ( i = GetMotionData_Count ; i > 0 ; motion++, i-- ){
		if ( motion->object_id == object_id ){
			/* 再検索用に値を保持 */
			GetMotionData_Id = object_id ;
			GetMotionData_Count = i - 1 ;
			GetMotionData_Ptr = motion + 1 ;
			return ( motion->motion_ptr );
		}
	}

	return ( NULL );
}

/* ---------------------------------------------------------------- */
static void ExpandMotion( DM_WORK *work, DEMO_MOTION *packet )
{
   int			i ;
   const FVECTOR	zero_vector = {0,0,0,1};
   DEMO_MOTION	*expand_motion = NULL ;
   DM_MOTION	*motion ;

   /* モーション展開ワークの検索 */
   for ( i = 0 ; i < work->n_motion ; i++ ){
      if ( work->mtn_list[ i ].object_id == packet->object_id ){
         expand_motion = work->mtn_list[ i ].motion_ptr ;
         break ;
      }
   }

   /* 展開ワークの新規作成 */
   if ( expand_motion == NULL ){
      int		size ;
      size = sizeof(DEMO_MOTION) ;
      size += sizeof(FVECTOR) * packet->n_joints ;
      size += sizeof(FVECTOR) * packet->n_joints ;
      expand_motion = GV_Malloc( size );
      *expand_motion = *packet ;
      DEMO_PACKET_SIZE_SET( expand_motion->header, size );
      expand_motion->motion_type = DEMO_MOTION_ROTTRANS ;	/* タイプを強制的に変更 */
      for ( i = 0 ; i < packet->n_joints * 2 ; i++ ){
         expand_motion->motion[i] = zero_vector ;
      }

      /* モーション管理構造体取得 */
      motion = &work->mtn_list[ work->n_motion++ ] ;
      ASSERT( work->n_motion < DM_MAX_MOTION );

      /* 検索に必要なデータと実体へのポインタをコピー */
      motion->object_id = expand_motion->object_id ;
      motion->motion_ptr = expand_motion ;
      printf("Create motion expand work\n");
   }

   /* 無圧縮用モーション用処理 */
   if ( packet->motion_type != DEMO_MOTION_COMPRESS ){
      FVECTOR		*src, *dst ;

      /* モーション以外のデータをコピー */
      expand_motion->pos = packet->pos ;
      expand_motion->rot = packet->rot ;

      src = packet->motion ;
      dst = expand_motion->motion ;
      for ( i = 0 ; i < packet->n_joints ; i++ ){
         if ( packet->motion_type & DEMO_MOTION_ROT ){
            *dst++ = *src++ ;
         }
         if ( packet->motion_type & DEMO_MOTION_TRANS ){
            *dst++ = *src++ ;
         }
      }
      return ;
   }

   {/* モーション展開処理 */
      FVECTOR		*dst ;
      short		*src, flag, *data ;
      int			count ;

      /* モーション以外のデータをコピー */
      expand_motion->pos = packet->pos ;
      expand_motion->rot = packet->rot ;

      src = (short*)packet->motion ;
      dst = expand_motion->motion ;

      flag = *src ;			/* 展開フラグ */
      count = *src & 0xff ;	/* 展開数 */
      for ( i = 0 ; i < packet->n_joints ; i++ ){
         data = &src[ 1 ];
         if ( !( flag & MOTIONDIET_ROT_X ) ){
            dst->vx = ( (float)*data++ ) / 16383.0f ;
         }
         if ( !( flag & MOTIONDIET_ROT_Y ) ){
            dst->vy = ( (float)*data++ ) / 16383.0f ;
         }
         if ( !( flag & MOTIONDIET_ROT_Z ) ){
            dst->vz = ( (float)*data++ ) / 16383.0f ;
         }
         if ( !( flag & MOTIONDIET_ROT_W ) ){
            dst->vw = ( (float)*data++ ) / 16383.0f ;
         }
         //MT_QuatNormalize( dst, dst );
         dst++ ;
         if ( !( flag & MOTIONDIET_TRANS_X ) ){
            dst->vx = MFtoF( *data ) ;
            data++ ;
         }
         if ( !( flag & MOTIONDIET_TRANS_Y ) ){
            dst->vy = MFtoF( *data ) ;
            data++ ;
         }
         if ( !( flag & MOTIONDIET_TRANS_Z ) ){
            dst->vz = MFtoF( *data ) ;
            data++ ;
         }
         dst++ ;
         if ( --count <= 0 ){
            src = data ;
            flag = *src ;			/* 展開フラグ */
            count = *src & 0xff ;	/* 展開数 */
         }
      }
   }

}
/* ---------------------------------------------------------------- */
/* データ展開専用 */
static inline void *SVecEndianSwapAndCopy( void *d, void *s )
{
	u_short		*dst = d ;
	u_short		*src = s ;

   BP_LE_Swap2Bytes( dst + 0, src + 0 );
   BP_LE_Swap2Bytes( dst + 1, src + 1 );
   BP_LE_Swap2Bytes( dst + 2, src + 2 );
   BP_LE_Swap2Bytes( dst + 3, src + 3 );

	return ( &src[4] );
}
static inline void *FVecEndianSwapAndCopy( void *d, void *s )
{
   float *dst = d;
   float *src = s;

   BP_LE_Swap4Bytes( dst + 0, src + 0 );
   BP_LE_Swap4Bytes( dst + 1, src + 1 );
   BP_LE_Swap4Bytes( dst + 2, src + 2 );
   BP_LE_Swap4Bytes( dst + 3, src + 3 );

	return ( &src[4] );
}
/* モーションパックパケット用モーション展開ルーチン */
static void ExpandMotion2( DM_WORK *work, DEMO_MOTION_INFO *info, u_short *buff )
{
	int			i ;
	const FVECTOR	zero_vector = {0,0,0,1};
	DEMO_MOTION *expand_motion = NULL ;
	DM_MOTION	*motion ;

	/* モーション展開ワークの検索 */
	for ( i = 0 ; i < work->n_motion ; i++ ){
		if ( work->mtn_list[ i ].object_id == info->object_id ){
			expand_motion = work->mtn_list[ i ].motion_ptr ;
			break ;
		}
	}

	/* 展開ワークの新規作成 */
	if ( expand_motion == NULL ){
		int		size ;

		/* モーション管理構造体取得 */
		ASSERT( work->n_motion < DM_MAX_MOTION );
		motion = &work->mtn_list[ work->n_motion++ ] ;

		size = sizeof(DEMO_MOTION) ;
		size += sizeof(FVECTOR) * info->n_joints ;
		size += sizeof(FVECTOR) * info->n_joints ;

		expand_motion = GV_Malloc( size );
		expand_motion->object_id = info->object_id ;
		expand_motion->start_joint = info->start_joint ;
		expand_motion->n_joints = info->n_joints ;
		DEMO_PACKET_SIZE_SET(expand_motion->header, size );
		expand_motion->motion_type = DEMO_MOTION_ROTTRANS ;	/* タイプを強制的に変更 */
		for ( i = 0 ; i < info->n_joints * 2 ; i++ ){
			expand_motion->motion[i] = zero_vector ;
		}

		/* 検索に必要なデータと実体へのポインタをコピー */
		motion->object_id = expand_motion->object_id ;
		motion->motion_ptr = expand_motion ;
		printf("Create motion expand work (%d)\n", motion->object_id );
	}

	{/* モーション展開処理 */
		FVECTOR		*dst ;
		short		*unswap_src, flag, *unswap_data ;
		int			count ;

		/* モーション以外のデータをコピー */
		if ( info->flag & DEMO_MOTIONPACK_MOV ){
			buff = FVecEndianSwapAndCopy( &expand_motion->pos, buff );
			//printf("get move %f %f %f\n", expand_motion->pos.vx, expand_motion->pos.vy, expand_motion->pos.vz );
		}
		if ( info->flag & DEMO_MOTIONPACK_ROT ){
			buff = SVecEndianSwapAndCopy( &expand_motion->rot, buff );
			//printf("get rot %d %d %d\n", expand_motion->rot.vx, expand_motion->rot.vy, expand_motion->rot.vz );
		}
		unswap_src = buff ;
		dst = expand_motion->motion ;

		flag = BP_LE_SwapSShort( *unswap_src );			/* 展開フラグ */
		count = BP_LE_SwapSShort( *unswap_src ) & 0xff ;	/* 展開数 */
		for ( i = 0 ; i < info->n_joints ; i++ ){
			unswap_data = &unswap_src[ 1 ];
			if ( !( flag & MOTIONDIET_ROT_X ) ){
				dst->vx = ( (float)BP_LE_SwapSShort( *unswap_data++ ) ) / 16383.0f ;
			}
			if ( !( flag & MOTIONDIET_ROT_Y ) ){
				dst->vy = ( (float)BP_LE_SwapSShort( *unswap_data++ ) ) / 16383.0f ;
			}
			if ( !( flag & MOTIONDIET_ROT_Z ) ){
				dst->vz = ( (float)BP_LE_SwapSShort( *unswap_data++ ) ) / 16383.0f ;
			}
			if ( !( flag & MOTIONDIET_ROT_W ) ){
				dst->vw = ( (float)BP_LE_SwapSShort( *unswap_data++ ) ) / 16383.0f ;
			}
			//MT_QuatNormalize( dst, dst );
			//printf("rot(%d):%f %f %f %f\n", i, dst->vx, dst->vy, dst->vz, dst->vw );
			dst++ ;
			if ( !( flag & MOTIONDIET_TRANS_X ) ){
				dst->vx = MFtoF( BP_LE_SwapSShort( *unswap_data ) ) ;
				unswap_data++ ;
			}
			if ( !( flag & MOTIONDIET_TRANS_Y ) ){
				dst->vy = MFtoF( BP_LE_SwapSShort( *unswap_data ) ) ;
				unswap_data++ ;
			}
			if ( !( flag & MOTIONDIET_TRANS_Z ) ){
				dst->vz = MFtoF( BP_LE_SwapSShort( *unswap_data ) ) ;
				unswap_data++ ;
			}
			//printf("trans(%d):%f %f %f\n", i, dst->vx, dst->vy, dst->vz );
			dst++ ;
			if ( --count <= 0 ){
				unswap_src = unswap_data ;
				flag = BP_LE_SwapSShort( *unswap_src );			/* 展開フラグ */
				count = BP_LE_SwapSShort( *unswap_src ) & 0xff ;	/* 展開数 */
			}
		}
	}

}





/* ---------------------------------------------------------------- */
/* ================================================================ */
/*

	１６ビット浮動小数点フォーマット

	seeeeeffffffffff
	||    |
	||    +- 10bit 仮数部
	||
	|+- 5bit 指数（バイアス値 N の下駄履き指数）
	|
	+- 1bit 符合

*/

/* 指数のバイアス値（2^0となる指数値） */
#define E_BIAS	(8)		/* メタルギアでは大きい値を重視する */


#if 0

/* 現在(2001/07/30)は,使っていないので コメントアウトしておく オブジェ
   がもったいないから */

/* 単精度浮動小数点から１６ビット浮動小数点へ変換 */

static unsigned short FtoMF( float f )
{
	unsigned int	data, s_dat, e_dat, f_dat ;
	unsigned short	mf ;

	data = *(unsigned int*)&f ;
	/* 符合取りだし */
	s_dat = ( data & 0x80000000 ) >> 16 ;
	/* 指数取りだし */
	e_dat = ( data & 0x7f800000 ) ;
	if ( e_dat != 0 ){
		if ( e_dat <= ( 127 - E_BIAS ) << 23 ){
			//printf("convert error!!\n");
			return ( 0x0000 | s_dat );
		}
		if ( e_dat > ( 127 - E_BIAS + 31 ) << 23 ){
			//printf("convert error!!\n");
			return ( 0x7fff | s_dat );
		}
		e_dat -= ( 127 - E_BIAS ) << 23 ;
		e_dat >>= 13 ;
	}
	/* 仮数部取りだし */
	f_dat = ( data & 0x007fffff ) >> 13 ;

	/* ビット合成 */
	mf = s_dat | e_dat | f_dat ;

	return ( mf );
}

#endif


/* １６ビット浮動小数点から単精度浮動小数点へ変換 */
static float MFtoF( unsigned short mf )
{
	unsigned int	s_dat, e_dat, f_dat ;
	float			f ;

	/* 符合取りだし */
	s_dat = ( mf & 0x8000 ) << 16 ;
	/* 指数取りだし */
	e_dat = ( mf & 0x7c00 ) ;
	if ( e_dat != 0 ){
		e_dat += ( 127 - E_BIAS ) << 10 ;
		e_dat <<= 13 ;
	}
	/* 仮数部取りだし */
	f_dat = ( mf & 0x03ff ) << 13 ;
	/* ビット合成 */
	*(unsigned int*)&f = s_dat | e_dat | f_dat ;

	return ( f );
}


