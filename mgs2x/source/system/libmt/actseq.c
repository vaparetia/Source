//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	actseq.c
	シーケンスデータ再生処理関連

	1999/12/06 K.Takabe
	$Id: actseq.c,v 1.4 2002/11/23 11:49:51 Yoshizawa1 Exp $
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

#include "BP_EndianSupport.h"

#include	"g_define.h"
#include	"g_sound.h"
#include	"gameheader.h"

#include "bp_matrix.h"

//#define LOCAL_DEBUG
#ifdef LOCAL_DEBUG
#define DEBUG_MES(str,param)	printf( str,param )
#else
#define DEBUG_MES(str,param)	/**/
#endif

/* ---------------------------------------------------------------- */

static int DefaultSevAction( SAR_INST *inst, int layer, int param1 );

/* ---------------------------------------------------------------- */
void* MT_InitSequence( int n_layer, int id, int flag )
{
	SAR_CONTROL *sar_ctrl ;
	SAR_HEADER	*header ;
	int			size, i ;

	header = GV_GetCache( GV_CacheID( id, 's' ) );
	if ( header == NULL ) return( NULL ) ;

	size = sizeof(SAR_CONTROL) + sizeof(SAR_INST) * n_layer ;
	sar_ctrl = GV_Malloc( size );
	if ( sar_ctrl == NULL ) return ( NULL );
	GV_ZeroMemory( sar_ctrl, size );

	sar_ctrl->header = header ;
	sar_ctrl->n_instance = n_layer ;

	/* データタイプチェック */
	if ( header->format_id == SAR_FORMATID_SEV ) sar_ctrl->type = 1 ;

	for ( i = 0 ; i < n_layer ; i++ ) sar_ctrl->instance[i].base_tick = TIME_BASE ;

	return ( sar_ctrl );
}

void MT_FreeSequence( SAR_CONTROL *sar_ctrl )
{
	if ( sar_ctrl == NULL ) return ;
	GV_Free( sar_ctrl );
}

/* ---------------------------------------------------------------- */
	/*
		シーケンスデータの再生処理
	*/
void MT_ActSequence( SAR_CONTROL *sar_ctrl )
{
	SAR_INST	*inst ;
	SAR_ELEMENT	*element ;
	int			exec_flag, i ;

	if ( sar_ctrl == NULL ) return ;
	if ( sar_ctrl->header == NULL ) return ;

	inst = sar_ctrl->instance ;
	for ( i = 0 ; i < sar_ctrl->n_instance ; i++, inst++ ){

		if ( !( inst->play_flag & SAR_FLAG_PLAY ) ) continue ;
		if ( inst->play_flag & SAR_FLAG_PAUSE ) continue ;

		inst->time += inst->base_tick ;

		/* シーケンス再生処理 */
		element = inst->data_top ;
		//printf("%d %d %d\n", element->time, element->next_offset, element->pad );
		while ( ( inst->time >= BP_LE_SwapUShort( element->time_le ) ) && ( BP_LE_SwapUShort( element->time_le ) != 0xffff ) ){

			if ( !( inst->play_flag & SAR_FLAG_MASK ) ){

				/* コールバックが登録されている場合にはそれを実行する */
				exec_flag = 0 ;
				if ( sar_ctrl->callback != NULL ){
					exec_flag = (sar_ctrl->callback)( inst, i, sar_ctrl->param1 );
				}

				/* コールバックが登録されていない場合にはデフォルトの処理を行なう */
				if ( exec_flag == 0 ){
					switch( sar_ctrl->type ){
					  case 1:/* ＳＥ再生データ */
						DefaultSevAction( inst, i, sar_ctrl->param1 );
						break ;
					  default:
						break ;
					}
				}

			}

			/* ポインタを次に移す */
			element = (SAR_ELEMENT*)( (int)element + element->next_offset );
			inst->data_top = element ;
		}

		/* ループ動作チェック */
		if ( inst->play_flag & SAR_FLAG_LOOP ){
			if ( inst->time >= inst->loop_time ){
				inst->time -= inst->loop_time ;
				inst->data_top = (void*)( sar_ctrl->header->list[ inst->play_num ].offset +
										 sar_ctrl->header->list[ inst->play_num ].local_header_size );
			}
		}

		/* 終端チェック */
		if ( BP_LE_SwapUShort( element->time_le ) == 0xffff && !( inst->play_flag & SAR_FLAG_LOOP ) ){
			inst->play_flag = 0 ;
			DEBUG_MES( "actseq.c: sequence end\n", 0 );
			break ;
		}

	}

}

/* ---------------------------------------------------------------- */
	/*
		再生スピードの変更
	*/
void MT_SetSequenceSpeed( SAR_CONTROL *sar_ctrl, float time )
{
	int			i ; 
	if ( sar_ctrl == NULL ) return ;
	for ( i = 0 ; i < sar_ctrl->n_instance ; i++ ){
		sar_ctrl->instance[i].base_tick = time ;
	}
}

/* ---------------------------------------------------------------- */
	/*
		指定データの再生開始
	*/
void MT_PlaySequence( SAR_CONTROL *sar_ctrl, int layer, int num, int flag, int loop_time )
{
	SAR_INST	*inst ;
	SAR_HEADER	*header ;

	if ( sar_ctrl == NULL ) return ;

	sar_ctrl->instance[ layer ].play_flag = 0 ;
	if ( ( header = sar_ctrl->header ) == NULL ){
		DEBUG_MES( "actsar.c: no initialize\n", 0 );
		return ;
	}
	if ( num >= header->n_datas ){
		DEBUG_MES( "actsar.c: index over (%d)\n", num );
		return ;
	}
	if ( header->list[ num ].id == 0 ){
		DEBUG_MES( "actsar.c: no entry(%d)\n", num );
		return ;
	}

	inst = &sar_ctrl->instance[ layer ] ;
	inst->local_header = (void*)header->list[ num ].offset ;
	inst->data_top = (void*)( header->list[ num ].offset + header->list[ num ].local_header_size );
	inst->play_num = num ;
	inst->time = 0 ;
	inst->loop_time = loop_time ;
	inst->play_flag = flag ;

}

void MT_StopSequence( SAR_CONTROL *sar_ctrl, int layer )
{
	if ( sar_ctrl == NULL ) return ;

	sar_ctrl->instance[ layer ].play_flag = 0 ;
}


/* ---------------------------------------------------------------- */

static int DefaultSevAction( SAR_INST *inst, int layer, int param1 )
{
	SEV_LOCAL_HEADER	*l_header ;
	SEV_ELEMENT			*element ;
	SEV_PARAM			*se_param ;
	FVECTOR				*pos ;
	DG_OBJS				*objs ;
	int					id ;
   unsigned short    se_code;

	l_header = inst->local_header ;
	element = inst->data_top ;
	id = element->id ;
	se_param = &l_header->params[ id ] ;
   se_code = BP_LE_SwapUShort(se_param->se_code_le);

	objs = (DG_OBJS*)param1 ;
	pos = (FVECTOR*)objs->objs[ se_param->joint ].world.m[3] ;

	switch ( se_param->mode ){
	  case 0:
		GM_SeSetMode( se_code, pos, GM_SEMODE_NORMAL );
		break ;
	  case 1:
		GM_SeSetMode( se_code, pos, GM_SEMODE_BOMB );
		break ;
	  case 2:
		GM_SeSetMode( se_code, pos, GM_SEMODE_REAL );
		break ;
	  case 3:
		GM_SeSetMode( se_code, pos, GM_SEMODE_NORMAL );
		break ;
	  case 4:
		GM_SeSetMode( se_code, pos, GM_SEMODE_CAMPOS );
		break ;
	  case 5:
		GM_SeSetMode( se_code, pos, GM_SEMODE_CAMTRG );
		break ;
	}
#ifdef LOCAL_DEBUG
	printf( "actseq.c: GM_SeSet called (%d %d) time:%d\n",
		   se_code, se_param->mode, inst->time );
#endif
	return ( 1 );
}

/* ---------------------------------------------------------------- */

	/*
		ＳＥＶ固有関数
	*/

typedef struct _sev_scrpad {
	int			total_se ;			/*  */
	MT_EXCHG_SE	se_data ;
	int			pad ;
	u_long64		enable_joint[64] ;	/*  */
	struct _sev_datas{
		int			se_code ;
		short		joint ;
		short		mode ;
		short		layer ;
		short		time ;
		int			pad ;
	} datas[32] ;
} SevScrPad ;

static int	SevAction( SAR_INST *inst, int layer, int param1 )
{
	SEV_LOCAL_HEADER	*l_header ;
	SEV_ELEMENT			*element ;
	SEV_PARAM			*se_param ;
	int					id ;
	SevScrPad			*scrpad = SCRPAD_ADDR ;
	struct _sev_datas	*data ;

	l_header = inst->local_header ;
	element = inst->data_top ;
	id = element->id ;
	se_param = &l_header->params[ id ] ;

	data = &scrpad->datas[ scrpad->total_se++ ];
	data->se_code = BP_LE_SwapUShort(se_param->se_code_le);
	data->joint = se_param->joint ;
	data->mode = se_param->mode ;
	data->layer = layer ;
	data->time = inst->time ;	/* debug data */

	return ( 1 );
}

	/*
		ＳＥＶ専用ActSequence関数
	*/
#ifdef KP_WINDOWS
static int	_act_seq_sev_tbl[] =
{
	GM_SEMODE_NORMAL,	// 0
	GM_SEMODE_BOMB,		// 1
	GM_SEMODE_REAL,		// 2
	GM_SEMODE_BOMB,		// 3
	GM_SEMODE_CAMPOS,	// 4
	GM_SEMODE_CAMTRG,	// 5
} ;
#define	ACT_SEQ_SEV_TBL_NUM	(sizeof(_act_seq_sev_tbl)/sizeof(_act_seq_sev_tbl[0]))
#endif	// KP_WINDOWS

void MT_ActSequenceSEV( SAR_CONTROL *sar_ctrl, MOTION_CONTROL *m_ctrl, DG_OBJS *objs, DG_EVMOBJ *evmobj )
{
	SevScrPad	*scrpad = SCRPAD_ADDR ;
	struct _sev_datas	*data ;
	int			i ;
	u_long64		joint, mask ;
	MT3_CONTROL	*mt3_ctrl ;
	FVECTOR		*pos, evm_tmp_pos ;
#ifdef KP_WINDOWS
	int			se_mode_tbl[ACT_SEQ_SEV_TBL_NUM] ;
#endif

	if ( sar_ctrl == NULL ) return ;

	/* モーション再生において有効な関節を調べる */
	mask = I64(0xffffffffffffffff) ;
	mt3_ctrl = &m_ctrl->mt3_ctrl[ m_ctrl->n_layer - 1 ];
	for ( i = m_ctrl->n_layer - 1 ; i >= 0 ; mt3_ctrl--, i-- ){
		if ( !( mt3_ctrl->flag & MT3_ACTIVE ) ) continue ;
		scrpad->enable_joint[ i ] = mask & mt3_ctrl->mask ;
		mask &= ~( mt3_ctrl->mask );

		/* 進行時間の同期処理 */
		sar_ctrl->instance[ i ].base_tick = m_ctrl->mt3_ctrl[ i ].play_time_base ;

		/* モーションのスリープフラグに対応 */
		if ( mt3_ctrl->flag & MT3_SLEEP )
		  sar_ctrl->instance[ i ].play_flag |= SAR_FLAG_PAUSE ;	/* モーションに応じて停止フラグを立てる */
		else 
		  sar_ctrl->instance[ i ].play_flag &= ~SAR_FLAG_PAUSE ;	/* ポーズ状態は解除しておく */

	}

	scrpad->total_se = 0 ;
	
	sar_ctrl->callback = SevAction ;
	MT_ActSequence( sar_ctrl );

	if ( scrpad->total_se == 0 ) return ;
#ifdef KP_WINDOWS
	memcpy(se_mode_tbl, _act_seq_sev_tbl, sizeof(se_mode_tbl)) ;
#endif

	data = scrpad->datas ;
#ifndef KP_WINDOWS
	for ( i = 0 ; i < scrpad->total_se ; i++, data++ ){
#else
	for ( i = scrpad->total_se ; i > 0 ; i--, data++ ){
#endif
		joint = I64(1) << ( data->joint ) ;
		if ( scrpad->enable_joint[ data->layer ] & joint ){
			/* 音源座標算出 */
			if ( objs != NULL ){
				pos = (FVECTOR*)objs->objs[ data->joint ].world.m[3] ;
			} else {
				/* evmobjにおいて関節スケルトンの中心座標を求める */
				FVECTOR		offset ;
				FMATRIX		*mat ;
				offset.vx = evmobj->def->skeleton[ data->joint ].rt_tx ;
				offset.vy = evmobj->def->skeleton[ data->joint ].rt_ty ;
				offset.vz = evmobj->def->skeleton[ data->joint ].rt_tz ;
				offset.vw = 1.0f ;
				mat = &evmobj->matrix[ evmobj->use_buffer ][ data->joint ] ;
				_sceVu0ApplyMatrix( &evm_tmp_pos, mat, &offset );
				pos = &evm_tmp_pos ;
			}

			/* 効果音変換テーブルチェック */
			if ( m_ctrl->se_table_id != -1 ){
				MT_GetExchangeSe( &scrpad->se_data, data->se_code,
								 m_ctrl->se_table_id, m_ctrl->se_hazard_type, m_ctrl->se_segment_type );
			} else {
				scrpad->se_data.se_code = data->se_code & 0x0fff ;
				scrpad->se_data.noise_vol = 0 ;
				scrpad->se_data.flag = 0 ;
			}
			if ( scrpad->se_data.se_code == 0xffff ) continue ;

			/* 再生処理 */
//printf("mot se: code[%d] mode[%d]\n",scrpad->se_data.se_code,data->mode );
#ifndef KP_WINDOWS
			switch ( data->mode ){
			  case 0:
				GM_SeSetMode( scrpad->se_data.se_code, pos, GM_SEMODE_NORMAL );
				break ;
			  case 1:
				GM_SeSetMode( scrpad->se_data.se_code, pos, GM_SEMODE_BOMB );
				break ;
			  case 2:
				GM_SeSetMode( scrpad->se_data.se_code, pos, GM_SEMODE_REAL );
				break ;
			  case 3:
				GM_SeSetMode( scrpad->se_data.se_code, pos, GM_SEMODE_BOMB );
//暫定7.18まで是角				GM_SeSetMode( scrpad->se_data.se_code, pos, GM_SEMODE_NORMAL );
				break ;
			  case 4:
				GM_SeSetMode( scrpad->se_data.se_code, pos, GM_SEMODE_CAMPOS );
				break ;
			  case 5:
				GM_SeSetMode( scrpad->se_data.se_code, pos, GM_SEMODE_CAMTRG );
				break ;
			}
#else
			if( data->mode < ACT_SEQ_SEV_TBL_NUM )
			{
				GM_SeSetMode( scrpad->se_data.se_code, pos, se_mode_tbl[data->mode] );
			}
#endif
			/* ノイズ発生 */
			GM_SetNoise( scrpad->se_data.noise_vol, pos, m_ctrl->map_name );
#ifdef LOCAL_DEBUG
	printf( "actseq.c: GM_SeSet called (code:0x%x mode:%d) time:%d , noise %d\n",
		   scrpad->se_data.se_code, data->mode, data->time, scrpad->se_data.noise_vol );
#endif
		} else {
#ifdef LOCAL_DEBUG
	printf( "actseq.c: GM_SeSet no called (code:0x%x mode:%d) time:%d\n",
		   data->se_code, data->mode, data->time );
#endif
		}
	}

}

#if 0
/* ---------------------------------------------------------------- */
	/*
		３Ｄサウンド実験
	*/

void _GM_SeSetMode( int code, FVECTOR *pos, int mode )
{
	FMATRIX		GM_ListenerPos ;
	FVECTOR	mic_cone[2] ;
	FVECTOR	vec, dir ;
	float	angle1, angle2, angle3 ;
	float	near, far ;
	float	len ;
	float	vol, left_vol, right_vol ;
	float	cone_min ;

	GM_ListenerPos = DG_Chanl(0)->eye ;

	/* 左右マイクの方向ベクトルを求める */
	angle1 = 30.0f / 180.0f * (float)M_PI ;	/* 左右の開き */
	angle2 = 40.0f / 180.0f * (float)M_PI ;	/* コーンの減衰開始角度 */
	angle3 = 70.0f / 180.0f * (float)M_PI ;	/* コーンの減衰終了角度 */
	vec = DG_ZeroVector ;
	vec.vx = cosf( angle1 );
	vec.vz = sinf( angle1 );
	vec.vw = 0.0f ;
	_sceVu0ApplyMatrix( &mic_cone[0], &GM_ListenerPos, &vec );/* right */
	vec.vx = -vec.vx ;
	_sceVu0ApplyMatrix( &mic_cone[1], &GM_ListenerPos, &vec );/* left */

	/* 音源の距離と方向ベクトルを求める */
	dir.vx = pos->vx - GM_ListenerPos.m[3][0] ;
	dir.vy = pos->vy - GM_ListenerPos.m[3][1] ;
	dir.vz = pos->vz - GM_ListenerPos.m[3][2] ;
	len = dir.vx * dir.vx + dir.vy * dir.vy + dir.vz * dir.vz ;
	len = bp_sqrtf( len ) ; //BP_MATH - emulate PS2 sqrtf
	dir.vx *= 1.0f / len ;
	dir.vy *= 1.0f / len ;
	dir.vz *= 1.0f / len ;

	/* 距離による音量を求める */
	near = 750.0f ;
	far = 14000.0f ;
	len = DG_MAX( len, near );
	len = DG_MIN( len, far );
	vol = ( far - len ) / ( far - near );

	/* 左右の音量を求める */
	cone_min = 0.5f ;	/* マイクコーン外における最小音量 */
	angle1 = dir.vx * mic_cone[0].vx + dir.vy * mic_cone[0].vy + dir.vz * mic_cone[0].vz ;
	angle1 = acos( angle1 );/* 遅い */
	angle1 = DG_MAX( angle1, angle2 );
	angle1 = DG_MIN( angle1, angle3 );
	right_vol = ( angle3 - angle1 ) / ( angle3 - angle2 ) * ( 1.0f - cone_min ) + cone_min ;

	angle1 = dir.vx * mic_cone[1].vx + dir.vy * mic_cone[1].vy + dir.vz * mic_cone[1].vz ;
	angle1 = acos( angle1 );/* 遅い */
	angle1 = DG_MAX( angle1, angle2 );
	angle1 = DG_MIN( angle1, angle3 );
	left_vol = ( angle3 - angle1 ) / ( angle3 - angle2 ) * ( 1.0f - cone_min ) + cone_min ;

#if 0
	GM_SeSet( (int)-32, (int)( vol * right_vol * 63 ), code );
	GM_SeSet( (int)31, (int)( vol * left_vol * 63 ), code );
	printf("vol %f, l %f, r %f\n", vol, left_vol, right_vol );
#else
	//printf("vol %f, l %f, r %f\n", vol, left_vol, right_vol );
	right_vol *= vol ;
	left_vol *= vol ;
	if ( right_vol > left_vol ){
		float	pan ;
		pan = 1.0f - left_vol / right_vol ;
		GM_SeSet( (int)( pan * 31 ), (int)( right_vol * 63 ), code );
		//printf("vol %d, pan %d \n", (int)( right_vol * 63 ), (int)(-pan * 31 ) );
	} else {
		float	pan ;
		pan = 1.0f - right_vol / left_vol ;
		GM_SeSet( (int)( -pan * 31 ), (int)( left_vol * 63 ), code );
		//printf("vol %d, pan %d \n", (int)( left_vol * 63 ), (int)( pan * 31 ) );
	}
#endif
}

#endif

/* ---------------------------------------------------------------- */

