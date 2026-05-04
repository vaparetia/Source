/*
	tst_evm.c
	マルチウェイトエンベロープモデルオブジェクト表示実験プログラム

	2000/01/27 K.Takabe
	$Id: tst_evm.c,v 1.1.1.3 2002/11/19 11:51:31 Yoshizawa1 Exp $

*/

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
#include <stdarg.h>

#include	"libdg.h"
#include	"utl_dma.h"
#include	"gameheader.h"

#define MODEL_FLAG	(0)



/* セーブロード用ヘッダ */
typedef struct {
	int			flags ;			/* 各種フラグ */
	int			motion_length ;	/* モーションの有効フレーム数 */
	int			motion_joints ;	/* モーションの関節数 */
	int			motion_tick ;	/* ベースクロック値（１／３００単位） */
	int			move_size ;		/* 移動量のサイズ((motion_length+1)*16) */
	int			rots_size ;		/* 関節回転データのサイズ(((motion_length+1)*motion_joints)*16) */
	int			trans_size ;	/* 関節移動量データへのサイズ(((motion_length+1)*motion_joints)*16) */
	int			minfo_size ;	/* 移動キー情報のサイズ((motion_length+1)*4) */
	int			jinfo_size ;	/* 関節キー情報のサイズ((motion_length+1)*motion_joints)*16) */
	int			sound_size ;	/* サウンド情報のサイズ（将来の拡張用） */
	int			anime_size ;	/* アニメーションのサイズ（将来の拡張用） */
	int			pad[4] ;
	int			error_angle ;	/* 圧縮時のエラー許容角度（ＰＳ固定小数点角度） */
} MTN_FILE_HEADER ;

typedef struct {
	int			length ;
	int			joints ;
	FVECTOR		*move ;
	FVECTOR		*rots ;
	FVECTOR		*trans ;
} MTN_DATA ;

#define BENCH

#ifdef BENCH
#define MAX_OBJS_X	(3)
#define MAX_OBJS_Z	(3)
#define MAX_OBJS	(MAX_OBJS_X*MAX_OBJS_Z)
typedef struct _benchobj{
	DG_OBJS		*objs ;
	DG_EVMOBJ	*evmobj ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	int			mode ;
} BENCHOBJ ;
#endif

typedef	struct	{
	GV_ACT		actor ;
	DG_EVMOBJ	*evmobj ;
	MTN_DATA	face_motion ;
	int			face_count ;
	FVECTOR		face_skel_trans[ 50 ] ;
	FVECTOR		face_skel_rot[ 50 ] ;
#ifdef BENCH
	/* Benchmark */
	BENCHOBJ	objlist[MAX_OBJS] ;
#endif
} Work ;


extern void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color );

typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[64] ;
	FVECTOR	quat_buffer[4] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[64] ;
} ScrPadWork ;

/* ---------------------------------------------------------------- */
static void Message( int name, int len, ... )
{
	GV_MSG	msg ;
	int		mes[16], *mes_data ;
	va_list	ap ;

	msg.address = name ;
	msg.message = mes ;
	msg.message_len = len ;

	va_start( ap, len );
	for ( mes_data = mes ; len > 0 ; len-- ){
		*mes_data++ = va_arg( ap, int ) ;
	}
	va_end( ap );
	GV_SendMessage( &msg );
}
/* ---------------------------------------------------------------- */
void EvmActMotion( MOTION_CONTROL *m_ctrl, DG_EVMOBJ *evmobj )
{
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;
	FMATRIX		*skel_mats ;

	if ( evmobj->root != NULL )	scrpad->root_mat = *( evmobj->root );
	else						scrpad->root_mat = evmobj->world ;
	skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;

	UTL_StartMemToSpr( scrpad->joints, m_ctrl->abs_rots, m_ctrl->n_joints );
	UTL_EndMemToSpr();

	{/* オブジェクトにマトリクスを設定する */
		FVECTOR			*joints = scrpad->joints ;
		FMATRIX			*mats = scrpad->mats ;
		EVM_SKEL		*skel ;
		int		i ;

		skel = evmobj->def->skeleton ;
		scrpad->vec.vw = 1.0F ;
		{/* モデル情報から親子関係を取得して求める */
			for ( i = 0 ; i < evmobj->n_skeleton ; i++ ){
				FMATRIX	*parent ;
				if ( i < m_ctrl->n_joints ){
					MT_QuatToMat( mats, joints );
					_sceVu0MulMatrix( mats, &scrpad->root_mat, mats ) ;
					scrpad->vec.vx = skel->rt_tx + 0.0f ;
					scrpad->vec.vy = skel->rt_ty + 0.0f ;
					scrpad->vec.vz = skel->rt_tz + 0.0f ;
					parent = &scrpad->mats[ skel->parent ] ;
					_sceVu0ApplyMatrix( &mats->m[3][0], parent, &scrpad->vec );
					scrpad->vec.vx = -skel->rt_tx ;
					scrpad->vec.vy = -skel->rt_ty ;
					scrpad->vec.vz = -skel->rt_tz ;
					_sceVu0ApplyMatrix( &mats->m[3][0], mats, &scrpad->vec );
					*skel_mats = *mats ;
				} else {
					*mats = *skel_mats = scrpad->mats[ skel->parent ] ;
				}
				skel++ ;
				mats++ ;
				skel_mats++ ;
				joints++ ;
			}
		}
	}

}

#if 0
/* ＥＶＭオブジェクトの一部に直接モーションを設定 */
static void EvmActMotionImmediate( DG_EVMOBJ *evmobj, int first, int n_joints, FVECTOR *skel_trans, FVECTOR *skel_rot )
{
	int			i ;
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;
	FMATRIX		*skel_mats ;
	FVECTOR		vec ;
	FMATRIX		*mats = scrpad->mats ;
	EVM_SKEL	*skel ;

	skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;

	/* オブジェクトにマトリクスを設定する */
	skel = &evmobj->def->skeleton[ first ] ;
	vec.vw = 1.0F ;
	/* モデル情報から親子関係を取得して求める */
	for ( i = first ; i < ( first + n_joints ) ; i++ ){
		FMATRIX	*parent ;
		MT_QuatToMat( mats, skel_rot );
		mats->m[3][0] = skel->rt_tx + skel_trans->vx ;
		mats->m[3][1] = skel->rt_ty + skel_trans->vy ;
		mats->m[3][2] = skel->rt_tz + skel_trans->vz ;
		parent = &skel_mats[ skel->parent ] ;
		_sceVu0MulMatrix( mats, parent, mats ) ;
		vec.vx = -skel->rt_tx ;
		vec.vy = -skel->rt_ty ;
		vec.vz = -skel->rt_tz ;
		_sceVu0ApplyMatrix( &mats->m[3][0], mats, &vec );
		skel_mats[i] = *mats ;

		skel_rot++ ;
		skel_trans++ ;
		skel++ ;
		mats++ ;
	}
}
#endif

#if 0
/* 旧形式モーションデータ（オイラー角配列データ）をＥＶＭモデルに割り当てる */
static void EvmActMotionFromOldMotion( FMATRIX *world, int n_joints, FVECTOR *rots, DG_EVMOBJ *evmobj )
{
	int			i ;
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;
	FMATRIX		*skel_mats ;
	FVECTOR		vec ;
	FMATRIX		*mats = scrpad->mats ;
	EVM_SKEL	*skel ;

	if ( evmobj->root != NULL )	evmobj->world = *( evmobj->root );
	else						evmobj->world = *world ;

	evmobj->use_buffer = 1 - evmobj->use_buffer ;
	skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;

	/* オブジェクトにマトリクスを設定する */
	skel = evmobj->def->skeleton ;
	vec.vw = 1.0F ;
	/* モデル情報から親子関係を取得して求める */
	for ( i = 0 ; i < evmobj->n_skeleton ; i++ ){
		FMATRIX	*parent ;
		if ( i < n_joints ){
			_sceVu0UnitMatrix( mats );
			if ( rots->vx != 0.0f ) _sceVu0RotMatrixX( mats, mats, rots->vx );
			if ( rots->vy != 0.0f ) _sceVu0RotMatrixY( mats, mats, rots->vy );
			if ( rots->vz != 0.0f ) _sceVu0RotMatrixZ( mats, mats, rots->vz );
			mats->m[3][0] = skel->rt_tx ;
			mats->m[3][1] = skel->rt_ty ;
			mats->m[3][2] = skel->rt_tz ;
			if ( skel->parent != -1 ){
				parent = &scrpad->mats[ skel->parent ] ;
				_sceVu0MulMatrix( mats, parent, mats ) ;
			} else {
				_sceVu0MulMatrix( mats, &evmobj->world, mats ) ;
			}
			vec.vx = -skel->rt_tx ;
			vec.vy = -skel->rt_ty ;
			vec.vz = -skel->rt_tz ;
			_sceVu0ApplyMatrix( &mats->m[3][0], mats, &vec );
			*skel_mats = *mats ;
			rots++ ;
		} else {
			*mats = *skel_mats = scrpad->mats[ skel->parent ] ;
		}
		skel++ ;
		mats++ ;
		skel_mats++ ;
	}
}
#endif


/* ---------------------------------------------------------------- */
#if 0
/* ＭＴＮデータのセットアップ */
static void SetMtnMotionData( MTN_DATA *mtn_data, int name_id )
{
	MTN_FILE_HEADER	*mtn_file_header ;
	int				size ;
	mtn_file_header = GV_GetCache( GV_CacheID( name_id, 'r' ) );
	if ( mtn_file_header != NULL ){
		mtn_data->length = mtn_file_header->motion_length ;
		mtn_data->joints = mtn_file_header->motion_joints ;
		size = 0 ;
		mtn_data->move = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
		size = mtn_file_header->move_size ;
		mtn_data->rots = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
		size += mtn_file_header->rots_size ;
		mtn_data->trans = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
		size += mtn_file_header->trans_size ;
	}

}
#endif
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	//int			i ;
	DG_EVMOBJ	*evmobj ;
	//EVM_SKEL	*skel ;
	//FMATRIX		*skel_mats ;
	//FVECTOR		vec ;

	evmobj = work->evmobj ;

#if 1
	evmobj->light = GM_PlayerBody->objs->light ;
	evmobj->world = GM_PlayerBody->objs->world ;
	evmobj->world.m[3][2] += 1500.0f ;
	/* マトリクスバッファ切り替え */
	evmobj->use_buffer = 1 - evmobj->use_buffer ;
	/* モーションの割り当て */
	EvmActMotion( GM_PlayerBody->m_ctrl, evmobj );
	return ;

#if 0
	/* フェイスモーション実験 */
	work->face_count++ ;
	if ( work->face_count > work->face_motion.length + 1 ) work->face_count = 1 ;
	for ( i = 0 ; i < work->face_motion.joints ; i++ ){
		if ( i >= 50 ) break ;
		work->face_skel_rot[i] =
		  work->face_motion.rots[ ( work->face_motion.length + 1 ) * i + work->face_count ];
		work->face_skel_trans[i] =
		  work->face_motion.trans[ ( work->face_motion.length + 1 ) * i + work->face_count ];
	}

	/* 顔モーション設定 */
	EvmActMotionImmediate( evmobj, 21, work->face_motion.joints, work->face_skel_trans, work->face_skel_rot );
#else
	if ( GV_PadData[1].press & PAD_A ){
		Message( 123, 3,
				0, GV_StrCode("face"), 5*4 );
#if 0
		Message( 123, 6,
				1, 0, 10*5,
				(int)( DG_Chanl(0)->eye.m[3][0] ),
				(int)( DG_Chanl(0)->eye.m[3][1] ),
				(int)( DG_Chanl(0)->eye.m[3][2] ) );
#endif
	}
#endif


#else
	/* オイラー角配列モーションをＥＶＭに割り当てる場合 */
	EvmActMotionFromOldMotion( &GM_PlayerBody->objs->world, 21, rot_array, evmobj );
#endif

#if 0
	if ( GV_PadData[1].press & PAD_A ){
		NewOpticalCamouflageBreakDemo( GM_PlayerBody->objs, 0, 30000, 31800, 0x00808080 );
	}
	if ( GV_PadData[1].press & PAD_B ){
		Message( 9983938, 4,
				0, rand()%128, rand()%128, 60*5 );
	}
#endif

}

static void Die( Work *work )
{
	DG_DequeueEvmObj( work->evmobj );
	DG_FreeEvmObj( work->evmobj );
}

static int GetResources( Work *work, int name, int where )
{
	DG_EVMOBJ	*evmobj ;
	EVM_DEF		*def ;
	int			i, j, k ;

	def = GV_GetCache( GV_CacheID( GV_StrCode("org_def"), 'e' ) );
	//def = GV_GetCache( GV_CacheID( GV_StrCode("snh_def_mh_mt"), 'e' ) );
	//def = GV_GetCache( GV_CacheID( GV_StrCode("bmptest"), 'e' ) );

	work->evmobj = evmobj = DG_MakeEvmObj( def, MODEL_FLAG, 0 );
	DG_QueueEvmObj( evmobj );

	//GM_ConfigObjectEvm( GM_PlayerBody, GV_StrCode("human5"), 0 );
	//GM_ConfigObjectLight( GM_PlayerBody, GM_PlayerBody->objs->light );


#if 0
	/* フェイスモーション実験 */
	SetMtnMotionData( &work->face_motion, GV_StrCode("face") );
#else
	//NewE3FaceAnimation( 123, evmobj );
#endif


#ifdef BENCH
	for ( j = 0 ; j < MAX_OBJS_Z ; j++ ){
		for ( i = 0 ; i < MAX_OBJS_X ; i++ ){
			int		num ;
			num = i + j * MAX_OBJS_X ;
			work->objlist[num].pos.vx = i * 1500 - MAX_OBJS_X * 1500 / 2 ;
			work->objlist[num].pos.vz = j * 1500 - MAX_OBJS_Z * 1500 / 2 ;
			work->objlist[num].pos.vy = 0 ;
			DG_SetPos2( &work->objlist[num].pos, &work->objlist[num].rot );
#if 1
			work->objlist[num].evmobj = evmobj = DG_MakeEvmObj( def, MODEL_FLAG, 0 );
			DG_QueueEvmObj( evmobj );
			DG_GetPos( &evmobj->world );
			for ( k = 0 ; k < evmobj->n_skeleton ; k++ ){
				evmobj->matrix[0][k] = evmobj->world ;
				evmobj->matrix[1][k] = evmobj->world ;
			}
#else
			{
				DG_OBJS		*objs ;
				DG_DEF		*kms_def ;
				//kms_def = GV_GetCache( GV_CacheID( GV_StrCode("sna_def"), 'k' ) );
				kms_def = GV_GetCache( GV_CacheID( GV_StrCode("us_low_5_s"), 'k' ) );
				work->objlist[num].objs = objs = DG_MakeObjs( kms_def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC, 0 ) ;
				//work->objlist[num].objs = objs = DG_MakeObjs( kms_def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 ) ;
				DG_QueueObjs( objs );
				DG_GetPos( &objs->world );
				for ( k = 0 ; k < kms_def->n_models ; k++ ){
					DG_GetPos( &objs->objs[k].world );
					objs->objs[k].world.m[3][0] += def->skeleton[k].rt_tx ;
					objs->objs[k].world.m[3][1] += def->skeleton[k].rt_ty ;
					objs->objs[k].world.m[3][2] += def->skeleton[k].rt_tz ;
				}
			}
#endif
		}
	}

#endif


	return (0);
}


void *NewEvmObjectTest( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
