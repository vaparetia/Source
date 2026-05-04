//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	モーションストリーミングドライバ

	2000/09/26 K.Takabe
	$Id: mtstream.c,v 1.1.1.3 2002/11/19 11:41:53 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#ifndef KP_XBOX
#include	<eekernel.h>
#include	<eeregs.h>
#include	<libgraph.h>
#include 	<libvu0.h>
#endif

#include	"mgs_type.h"
#include	"libgv.h"
#include	"libgv.cnf"
#include	"libfs.h"
#include	"libdg.h"
#include	"libmt.h"
#include	"stream.h"
#include	"g_define.h"

#include	"strctrl.h"
#include	"fmt_demo.h"

#include "bp_matrix.h"

#include "BP_EndianSupport.h"

#define MAX_MOTIONS		(8)

extern void DM_EndianSwapDemoMotion( DEMO_MOTION *packet );

typedef struct {
	int			id ;			/* 割り当てオブジェクトＩＤ */
	short		type ;			/* 0:旧形式 1:新形式 */
	short		frame ;			/* 補間フレーム */
	DEMO_MOTION	*motion ;
	DEMO_MOTION	*org_motion ;	/* 補間先 */
} MOTION_WORK ;

typedef struct {
	GV_ACT_EX actor;
	int type;
	GM_STREAM_CONTROL *ctrl;

	/* ドライバ固有ワーク */
	int			which;
	int			n_motion ;
	MOTION_WORK	motion_list[MAX_MOTIONS] ;
} Work;

#define MAX_WORK	2

static Work	*work_ptr[ MAX_WORK ] = { NULL, NULL } ;

static void set_work_ptr( Work *work )
{
	int i;
	for( i = 0; i < MAX_WORK; i++ ){
		if( work_ptr[ i ] == NULL ){
			work_ptr[ i ] = work;
			work->which = i;
			return;
		}
	}
	ASSERT( FALSE );
}

static void reset_work_ptr( Work *work )
{
	ASSERT( work_ptr[ work->which ] == work );
	work_ptr[ work->which ] = NULL;
}

/* ---------------------------------------------------------------- */
static void ExpandMotion( Work *work, DEMO_MOTION *packet );
static float MFtoF( unsigned short mf );

/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
static void ExpandMotion2( Work *work, MST_PACKET *packet, MST_INFO *info, void *data_addr )
{
	int			i, init_flag = 0  ;
	const FVECTOR	zero_vector = {0,0,0,1};
	DEMO_MOTION	*expand_motion = NULL ;
	MOTION_WORK	*motion = NULL ;

	/* モーション展開ワークの検索 */
	for ( i = 0 ; i < work->n_motion ; i++ ){
		if ( work->motion_list[ i ].id == info->object_id ){
			motion = &work->motion_list[ i ] ;
			expand_motion = work->motion_list[ i ].org_motion ;
			break ;
		}
	}

	/* 展開ワークの新規作成 */
	if ( motion == NULL ){
		int		size ;

		/* モーション管理構造体取得 */
		ASSERT( work->n_motion < MAX_MOTIONS );
		motion = &work->motion_list[ work->n_motion ] ;
		init_flag = 1 ;

		size = sizeof(DEMO_MOTION) ;
		size += sizeof(FVECTOR) * info->n_joints ;
		size += sizeof(FVECTOR) * info->n_joints ;
		motion->motion = GV_Malloc( size * 2 ) ;
		if ( motion->motion == NULL ) return ;
		motion->org_motion = (void*)( (int)motion->motion + size ) ;
		work->n_motion++ ;	/* 管理数増加 */

		expand_motion = motion->org_motion ;
		expand_motion->object_id = info->object_id ;
		DEMO_PACKET_SIZE_SET( expand_motion->header, size );
		expand_motion->motion_type = DEMO_MOTION_ROTTRANS ;	/* タイプを強制的に変更 */
		expand_motion->start_joint = info->start_joint ;
		expand_motion->n_joints = info->n_joints ;
		expand_motion->pos = DG_ZeroVector ;
		expand_motion->rot = DG_ZeroSVector ;
		motion->motion->pos = DG_ZeroVector ;
		motion->motion->rot = DG_ZeroSVector ;
		for ( i = 0 ; i < info->n_joints * 2 ; i++ ){
			expand_motion->motion[i] = zero_vector ;
			motion->motion->motion[i] = zero_vector ;
		}

		/* 検索に必要なデータと実体へのポインタをコピー */
		motion->id = expand_motion->object_id ;
		motion->type = 1 ;
		*(motion->motion) = *(motion->org_motion) ;
		//printf("Create motion expand work\n");
	}

	{/* モーション展開処理 */
		FVECTOR		*dst ;
		short		*unswap_src, flag, *unswap_data ;
		int			count ;

		/* 補間時間の記録 */
		motion->frame = info->frame ;
		if ( init_flag ) motion->frame = 1 ;

		/* モーション以外のデータをコピー */
		//expand_motion->pos = packet->pos ;
		//expand_motion->rot = packet->rot ;

		unswap_src = (short*)data_addr ;
		dst = expand_motion->motion ;

		flag = BP_LE_SwapSShort( *unswap_src );			/* 展開フラグ */
		count = BP_LE_SwapSShort( *unswap_src ) & 0xff ;	/* 展開数 */
		for ( i = 0 ; i < info->n_joints ; i++ ){
			unswap_data = &unswap_src[ 1 ];
			if ( !( flag & MOTIONDIET_ROT_X ) ){
				dst->vx = ( (float)BP_LE_SwapSShort( *unswap_data++ ) ) / 32767.0f ;
			}
			if ( !( flag & MOTIONDIET_ROT_Y ) ){
				dst->vy = ( (float)BP_LE_SwapSShort( *unswap_data++ ) ) / 32767.0f ;
			}
			if ( !( flag & MOTIONDIET_ROT_Z ) ){
				dst->vz = ( (float)BP_LE_SwapSShort( *unswap_data++ ) ) / 32767.0f ;
			}
			if ( !( flag & MOTIONDIET_ROT_W ) ){
				dst->vw = ( (float)BP_LE_SwapSShort( *unswap_data++ ) ) / 32767.0f ;
			}
			//MT_QuatNormalize( dst, dst );
			//printf("%f %f %f %f\n", dst->vx, dst->vy, dst->vz, dst->vw );
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
			//printf("%f %f %f\n", dst->vx, dst->vy, dst->vz );
			dst++ ;
			if ( --count <= 0 ){
				unswap_src = unswap_data ;
				flag = BP_LE_SwapSShort( *unswap_src );			/* 展開フラグ */
				count = BP_LE_SwapSShort( *unswap_src ) & 0xff ;	/* 展開数 */
			}
		}
	}

}

static void ExpandMotion( Work *work, DEMO_MOTION *packet )
{
   int			i ;
   const FVECTOR	zero_vector = {0,0,0,1};
   DEMO_MOTION	*expand_motion = NULL ;
   MOTION_WORK	*motion ;

   /* モーション展開ワークの検索 */
   for ( i = 0 ; i < work->n_motion ; i++ ){
      if ( work->motion_list[ i ].id == packet->object_id ){
         expand_motion = work->motion_list[ i ].motion ;
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
      if ( expand_motion == NULL ) return ;
      *expand_motion = *packet ;
      DEMO_PACKET_SIZE_SET( expand_motion->header, size );
      expand_motion->motion_type = DEMO_MOTION_ROTTRANS ;	/* タイプを強制的に変更 */
      for ( i = 0 ; i < packet->n_joints * 2 ; i++ ){
         expand_motion->motion[i] = zero_vector ;
      }

      /* モーション管理構造体取得 */
      ASSERT( work->n_motion < MAX_MOTIONS );
      motion = &work->motion_list[ work->n_motion++ ] ;

      /* 検索に必要なデータと実体へのポインタをコピー */
      motion->id = expand_motion->object_id ;
      motion->motion = expand_motion ;
      motion->type = 0 ;
      motion->frame = 0 ;
      //printf("Create motion expand work\n");
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

/* ---------------------------------------------------------------- */
static void InterpAct( Work *work )
{
	int		i, j ;
	MOTION_WORK		*m_work ;
	DEMO_MOTION		*src_motion, *dst_motion ;
	FVECTOR			*src, *dst, tmp_vec ;
	float			t ;

	m_work = work->motion_list ;
	for ( i = 0 ; i < work->n_motion ; i++, m_work++ ){
		if ( m_work->type == 0 ) continue ;
		if ( m_work->frame == 0 ) continue ;
		t = 1.0f / (float)m_work->frame ;
		src_motion = m_work->org_motion ;
		dst_motion = m_work->motion ;
		src = src_motion->motion ;
		dst = dst_motion->motion ;
		for ( j = 0 ; j < src_motion->n_joints ; j++ ){
			MT_QuatSlerp( &tmp_vec, dst, src, t );
			MT_QuatNormalize( dst, &tmp_vec );
			src++ ;
			dst++ ;
			_sceVu0InterVector( dst, src, dst, t );
			src++ ;
			dst++ ;
		}
		m_work->frame-- ;
	}
}

//----------------------------------------------------------------------------

static void EndianSwapDemoMstPacket( MST_PACKET *packet )
{
   if ( !DEMO_PACKET_IS_ENDIAN_SWAP( *packet ) )
   {
      int i;
      
      DEMO_PACKET_SET_ENDIAN_SWAP( *packet );

      // BP - Note, the top three members of packet are DEMO_PACKET, even though they're not 
      // called as such.
      // The last (n_lists) member is the "reserved" member of the packet header, but we can safely 
      // swap that.
      BP_LE_SwapSInt_Inp( &packet->n_lists );

      for ( i = 0; i < packet->n_lists; ++i )
      {
         MST_INFO *pInfo = packet->info_list + i;

         BP_LE_SwapSInt_Inp( &pInfo->object_id );
         BP_LE_SwapUShort_Inp( &pInfo->flag );
         BP_LE_SwapUShort_Inp( &pInfo->offset );
         BP_LE_SwapUShort_Inp( &pInfo->frame );
      }
   }
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	/* ひたすらデータが来たら解放する */
	DEMO_PACKET	*packet ;
	void *p;

	for( ;; ){
		if( ( p = FS_StreamGetData( work->ctrl->stream_h, work->type ) ) != NULL ){
			if( FS_STREAM_GET_TIME( p ) > work->ctrl->tick ){
				FS_StreamUngetData( work->ctrl->stream_h, p );
				break;
			}
			packet = p ;
			while ( ( DEMO_PACKET_PACKET_TYPE_MASKED( *packet ) ) == DEMO_PACKET_MOTION ){
				/* モーションの展開＆登録 */
				ExpandMotion( work, (DEMO_MOTION*)packet );
				packet = (DEMO_PACKET*)( (int)packet + DEMO_PACKET_SIZE( *packet ) );
			}

         if ( ( DEMO_PACKET_PACKET_TYPE_MASKED( *packet ) ) == DEMO_PACKET_MST ){
				int		i ;
				MST_PACKET	*mst_packet ;
				MST_INFO	*mst_info ;
				void		*data_top ;

            mst_packet = (MST_PACKET*)packet ;

            EndianSwapDemoMstPacket( mst_packet );

				mst_info = mst_packet->info_list ;
				data_top = (void*)&mst_packet->info_list[ mst_packet->n_lists ] ;
				
            for ( i = 0 ; i < mst_packet->n_lists ; i++, mst_info++ ){
					ExpandMotion2( work, mst_packet, mst_info,
								  (void*)( (int)data_top + mst_info->offset ) );
				}
			}
			FS_StreamFreeData( work->ctrl->stream_h, p );

		} else {
			if( work->ctrl->state >= GM_STREAM_STATE_READ_END ){
				/* 読みきってデータがないので終了 */
				GV_DestroyActor( work );
			}
			break;
		}
	}
	InterpAct( work );
}

static void Die( Work *work )
{
	int			i ;
	for ( i = 0 ; i < work->n_motion ; i++ ){
		GV_Free( work->motion_list[i].motion );
	}
	reset_work_ptr( work );
}

void *NewStreamMotionDriver( GM_STREAM_CONTROL *ctrl, int type )
{
	Work *work = NULL ;

	if( ( work = GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_SYSTEM
								 , sizeof( Work ), 0xf2 ) ) != NULL ){
		GV_SetActor( &work->actor, Act, Die );
		GV_ActorEX( &work->actor );
		work->type = type;
		work->ctrl = ctrl;

//		work_ptr = work ;
		set_work_ptr( work );
	}
	return work;
}
/* ---------------------------------------------------------------------- */
/*
	ドライバ登録 
*/
static GM_STREAM_DRIVER driver = {
	NULL,                  /* next   */
	0,                     /* type   */
	NewStreamMotionDriver, /* driver */
};

int GM_StreamMtDriverInit( void )
{
	GM_StreamAddDriver( &driver, CHANK_TYPE_MOTION );
	return 0;
}


/* ---------------------------------------------------------------------- */
/*
	外部呼び出しインターフェイス
*/
	/*
		指定したＩＤのモーションがある場合にモーションを取得する
	*/
DEMO_MOTION*	GM_StreamGetMotion( int id )
{
	MOTION_WORK	*motion ;
	int			i, j ;

	/* デモ管理構造体へのポインタを取得 */

	for( j = 0; j < 2; j++ ){
		/* モーション対象オブジェクトＩＤで検索 */
		if( work_ptr[ j ] == NULL ) continue;
		motion = work_ptr[ j ]->motion_list ;
		for ( i = work_ptr[ j ]->n_motion ; i > 0 ; motion++, i-- ){
			if ( motion->id == id ){
				return ( motion->motion );
			}
		}
	}

	return ( NULL );
}

/*
	口パクアニメーション
	注！！ 必ず GM_ActObject2 の後に実行すること！！！
	*/

void GM_MouthAnimation( int talk_name, DG_EVMOBJ *evmobj  )
{
    DEMO_MOTION *mtn ;
    int       i ;
    FMATRIX  *skel_mats, *mats ;
    EVM_SKEL *skel ;
    FVECTOR  *rot, *trans, vec ;
    DEMO_MOTION * GM_StreamGetMotion( int id ) ;

    /* 現在のフレームのモーションを得る */
    if( (mtn = GM_StreamGetMotion( talk_name ) ) ) {
		/* 顔の全関節の値を設定する */
		skel_mats = evmobj->matrix[ evmobj->use_buffer ] ; 
		skel = evmobj->def->skeleton + 21 ;
		vec.vw = 1.0F;
		for( i=0 ; i<32 ; i++, skel++ ) {
		    mats = &skel_mats[i + 21] ;

		    /* クォータニオンとトランスレーションの取得 */
		    rot   = &mtn->motion[i*2  ] ;
		    trans = &mtn->motion[i*2+1] ;
		    /* ローカルマトリックスを作成 */
		    MT_QuatToMat( mats, rot ) ;
		    mats->m[3][0] = skel->rt_tx + trans->vx ;
		    mats->m[3][1] = skel->rt_ty + trans->vy ;
		    mats->m[3][2] = skel->rt_tz + trans->vz ;
		    /* 親を掛け ワールドマトリックスに変換 */
		    _sceVu0MulMatrix( mats, &skel_mats[ skel->parent ], mats ) ;

		    vec.vx = -skel->rt_tx;
		    vec.vy = -skel->rt_ty;
		    vec.vz = -skel->rt_tz;
		    _sceVu0ApplyMatrix( (FVECTOR *)mats->m[3], mats, &vec ) ;
		}
    }
}
