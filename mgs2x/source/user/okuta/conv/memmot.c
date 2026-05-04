//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    memmot.c
    メモリモーション:メインメモリ上にモーションデータを先計算して展開,それを参照するシステム
    2001/05/09 Masafumi Okuta
    $Id: memmot.c,v 1.1.1.3 2002/11/19 11:47:48 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <libscf.h>
#include <libpkt.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include	"maoutil.h"
#include	"memmot.h"

#define	MMTT_INTERP_DEF		8*5	// 1/300単位 

inline int BP_AdjustTick(int);
inline int BP_AdjustTick2(int);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))

/*----------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	MMT_JOINT	(16)					/* モデル関節数:今回はカモメオンリー */

/*----------------------------------------------------------------*/
// クォータニオン
typedef struct {
    FVECTOR	prev, c_prev;	// 補間元クォータニオン＆補正用クォータニオン 
    FVECTOR	next, c_next;	// 補間先クォータニオン＆補正用クォータニオン 
    FVECTOR	t;
} SQUAD_WORK ;
// クォータニオン補間ワーク
typedef struct {
    FVECTOR	from[4];
    FVECTOR	to[4];
    float	t[4];
    float	tmp[4], param_x[4], param_y[4];
    float	scale_from[4], scale_to[4];
    float	omega[4];
    float	cosom[4];
    float	sinom[4];
} SLERP4_WORK;
// スクラッチパッドワーク
typedef struct {
    FVECTOR	root ;
    FVECTOR	joints[64] ;
    FVECTOR	quat_buffer[4] ;
    FVECTOR	vec ;
    FMATRIX	mat ;
    FMATRIX	root_mat ;
    FMATRIX	mats[64] ;
    SLERP4_WORK	slerp4_work ;
    SQUAD_WORK	squad_work[64] ;
} ScrPadWork ;

static	FVECTOR* MEMMOT_ActMotion( MEMMOT_CTRL* mmt_ctrl);

// モーションデータの中から使用する回転データの先頭を返す
FVECTOR* MEMMOT_GetAbsRot( MEMMOT_CTRL* mmt_ctrl )
{
    int mas, m_time ;
    FVECTOR	*abs ;
    MEMMOT_DATA*	pmmtData;

    pmmtData = mmt_ctrl->pmmtData;

    mas = mmt_ctrl->current_mot;

    // フレームを取得
    if ( pmmtData->m_len[mas] != 0 )	m_time = (int)mmt_ctrl->m_time % pmmtData->m_len[mas];
    else				m_time = 0;

    // 反転チェック
    if ( mmt_ctrl->reverse_flag ) 	m_time = pmmtData->m_len[mas] - (int)m_time;

    abs = pmmtData->m_buff + (MMT_JOINT*(pmmtData->m_ptr[mas]+m_time));

    return abs ;
}
// モーション切替え
void	MEMMOT_SetMotion( MEMMOT_CTRL *mmt_ctrl, int num, int intrp )
{
    MEMMOT_DATA*	pmmtData;

    pmmtData = mmt_ctrl->pmmtData;

    mmt_ctrl->nLoop	   	= 0;
    mmt_ctrl->current_mot  	= num ; 
    mmt_ctrl->m_time 	   	= 0.f;
    mmt_ctrl->last_chk_time	= 0;
    mmt_ctrl->interp_count 	= intrp;
    mmt_ctrl->interp_inc   	= intrp;
    mmt_ctrl->interp_time  	= 0.f ;
    mmt_ctrl->reverse_flag 	= 0;
    mmt_ctrl->nPlayFlag    	= MEMMOT_PLAY_ACT;

    mmt_ctrl->old_height   = pmmtData->m_height[pmmtData->m_ptr[mmt_ctrl->current_mot]];
#if 0
    abs = MEMMOT_GetAbsRot( mmt_ctrl ) ;
    abs = MEMMOT_ActMotion( mmt_ctrl ) ;

    KR_MemCopy( &pmmtData->abs_rots[0], abs, sizeof(FVECTOR), MMT_JOINT ) ;	
    mmt_ctrl->current_abs = &pmmtData->abs_rots[0];
#endif
}

// クォータニオンの線形補間（正規化しないので注意）
static	void MEMMOT_QuatSlerp( FVECTOR *res, FVECTOR *from, FVECTOR *to, float t )
{
    float	to1[4] ;
    float	cosom  ;	/* 本当はdouble型の方がいい */
    float	scale0, scale1 ;		/* 本当はdouble型の方がいい */

    /* 内積を求める */
    cosom = from->vx * to->vx + from->vy * to->vy + from->vz * to->vz + from->vw * to->vw ;

    /* 符号をそろえる */
    if ( cosom < 0.0F ){
	cosom = -cosom ;
	to1[0] = - to->vx ;
	to1[1] = - to->vy ;
	to1[2] = - to->vz ;
	to1[3] = - to->vw ;
    } else  {
	to1[0] = to->vx ;
	to1[1] = to->vy ;
	to1[2] = to->vz ;
	to1[3] = to->vw ;
    }

    /* 係数を求める */
    /* ２つの角度の差が小さすぎるときには線形補間で求める */
    scale0 = 1.0F - t ;
    scale1 = t ;
    
    /* 係数を使って計算 */
    to1[0] = scale0 * from->vx + scale1 * to1[0] ;
    to1[1] = scale0 * from->vy + scale1 * to1[1] ;
    to1[2] = scale0 * from->vz + scale1 * to1[2] ;
    to1[3] = scale0 * from->vw + scale1 * to1[3] ;
    res->vx = to1[0] ;
    res->vy = to1[1] ;
    res->vz = to1[2] ;
    res->vw = to1[3] ;
}

// モーション再生
static	FVECTOR* MEMMOT_ActMotion( MEMMOT_CTRL* mmt_ctrl)
{
    int			i;
    FVECTOR*		old_abs;
    FVECTOR*		abs;
    MEMMOT_DATA*	pmmtData;
    
    pmmtData = mmt_ctrl->pmmtData;
    abs = mmt_ctrl->current_abs;

    if ( mmt_ctrl->interp_count <= 0 ) return abs;

    /* 補間処理 */
    old_abs = &pmmtData->abs_rots[0];
    for ( i = MMT_JOINT ; i > 0 ; i-- ){
	MEMMOT_QuatSlerp( old_abs, old_abs, abs, mmt_ctrl->interp_time );
	abs++;
	old_abs++;
    }

    // 補間時間
    if ( mmt_ctrl->interp_count ){
	mmt_ctrl->interp_time += (float)TIME_BASE / (float)mmt_ctrl->interp_inc;
	if ( ( mmt_ctrl->interp_count -= TIME_BASE ) <= 0 ){
	    mmt_ctrl->interp_count = 0;
	}
    }

    return abs;
//    return &pmmtData->abs_rots[0];
}

// アジャスト機能
static FVECTOR	*MEMMOT_AdjustMotion( MEMMOT_CTRL *mmt_ctrl)
{
    FVECTOR 		quat;
    FVECTOR*		old_abs;
    FVECTOR*		abs;
    SVECTOR 		rot;
    MEMMOT_DATA*	pmmtData;

    pmmtData = mmt_ctrl->pmmtData;
    abs = mmt_ctrl->current_abs;

    if ( (mmt_ctrl->adj_y == 0) && (mmt_ctrl->adj_turn_y == 0) 
	 && (mmt_ctrl->adj_x == 0) && (mmt_ctrl->adj_turn_x == 0) ) return abs ;

    if ( mmt_ctrl->adj_turn_y != mmt_ctrl->adj_y ) {
	mmt_ctrl->adj_y = GV_NearExp8P ( mmt_ctrl->adj_y, mmt_ctrl->adj_turn_y ) ;
    } 
	
    if ( mmt_ctrl->adj_turn_x != mmt_ctrl->adj_x ) {
	mmt_ctrl->adj_x = GV_NearExp8P ( mmt_ctrl->adj_x, mmt_ctrl->adj_turn_x ) ;
    }

    /* アジャスト処理 */
    old_abs = &pmmtData->abs_rots[ 0 ] ;
    old_abs += 3 ;
    abs += 3 ;

    rot.vx = mmt_ctrl->adj_x ;
    rot.vy = mmt_ctrl->adj_y ;
    rot.vz = 0 ;

    GM_RotToQuat( &rot, &quat ) ;
    MT_QuatMul( old_abs, &quat, abs );

    return &pmmtData->abs_rots[0] ;
}

// 再生処理
void MEMMOT_ActMotion2( MOTION_CONTROL *m_ctrl, DG_OBJS *objs, FVECTOR *rots )
{
    ScrPadWork*		scrpad = (ScrPadWork*)SCRPAD_ADDR ;

    if ( objs->root != NULL )	scrpad->root_mat = *( objs->root );
    else			scrpad->root_mat = objs->world ;

    MT_StartMemToSpr( (u_long128*)scrpad->joints, (u_long128*)rots, m_ctrl->n_joints );
    MT_WaitMemToSpr();

    {/* オブジェクトにマトリクスを設定する */
	FVECTOR			*joints = scrpad->joints ;
	FMATRIX			*mats = scrpad->mats ;
	DG_DEF	*def ;
	DG_MDL	*mdl ;
	DG_OBJ	*obj = objs->objs ;
	int		i ;

	def = objs->def ;
	scrpad->vec.vw = 1.0F ;
	{/* モデル情報から親子関係を取得して求める */
	    for ( i = def->n_models ; i > 0 ; i-- ){	/* 拡張モデルは無視する */
		FMATRIX	*parent ;
		mdl = obj->model ;
		MT_QuatNormalize( joints, joints ); // 正規化 2001/08/30
		MT_QuatToMat( mats, joints );
		_sceVu0MulMatrix( mats, &scrpad->root_mat, mats ) ;
		scrpad->vec.vx = mdl->tx ;
		scrpad->vec.vy = mdl->ty ;
		scrpad->vec.vz = mdl->tz ;
		parent = &scrpad->mats[ mdl->parent ] ;
		_sceVu0ApplyMatrix( &mats->m[3][0], parent, &scrpad->vec );
		obj->world = *mats ;
		obj++ ;
		mats++ ;
		joints++ ;
	    }
	}
    }
}
// スピード変換
void MEMMOT_SetMotionSpeed( MEMMOT_CTRL* mmt_ctrl, float fSpeed)
{
    mmt_ctrl->m_time_base = fSpeed;
}
// 終了判定
int MEMMOT_CheckObject_IsEnd( MEMMOT_CTRL* mmt_ctrl, int nLayer)
{
    if ( mmt_ctrl->nPlayFlag == MEMMOT_PLAY_END_PREV ){
	return 1;
    }
    return 0;
}

// モーション再生
void MEMMOT_MakeMotion( MEMMOT_CTRL*	mmt_ctrl )
{
    MEMMOT_DATA*	pmmtData;

    pmmtData = mmt_ctrl->pmmtData;

    mmt_ctrl->current_abs = MEMMOT_GetAbsRot( mmt_ctrl );	// 絶対回転量
    mmt_ctrl->current_abs = MEMMOT_ActMotion( mmt_ctrl);	// モーション更新 	
    mmt_ctrl->current_abs = MEMMOT_AdjustMotion( mmt_ctrl);	// アジャスト

    // 移動量
    {
	int m_time;
	u_short* psHeight;

	if ( pmmtData->m_len[mmt_ctrl->current_mot] != 0 )
	    m_time = (int)mmt_ctrl->m_time % pmmtData->m_len[mmt_ctrl->current_mot];
	else
	    m_time = 0;

	_sceVu0CopyVector( mmt_ctrl->body->step, &pmmtData->m_step[( pmmtData->m_ptr[mmt_ctrl->current_mot] + (int)mmt_ctrl->m_time)]);
	psHeight = pmmtData->m_height + ( pmmtData->m_ptr[mmt_ctrl->current_mot] + (int)mmt_ctrl->m_time);
	mmt_ctrl->body->step->vy = (*psHeight);

	mmt_ctrl->old_height = (*psHeight);
// MAO_DbgDumpVector( mmt_ctrl->body->step );
    }

    // 時間更新
    mmt_ctrl->m_time += mmt_ctrl->m_time_base;
   
    mmt_ctrl->nPlayFlag    = 0;	    

    // モーションフラグ更新
    if ( (int)mmt_ctrl->m_time >= pmmtData->m_len[mmt_ctrl->current_mot]){
	mmt_ctrl->nPlayFlag = MEMMOT_PLAY_END;	    // 終了した
	mmt_ctrl->m_time = 0.f;
	mmt_ctrl->nLoop++;	// ループ回数を更新
    }else if ( (int)(mmt_ctrl->m_time) >= ( pmmtData->m_len[mmt_ctrl->current_mot] - (mmt_ctrl->m_time_base)) ){
	mmt_ctrl->nPlayFlag = MEMMOT_PLAY_END_PREV; // 終了
    }else {
	mmt_ctrl->nPlayFlag = MEMMOT_PLAY_ACT;	    // 実行中
    }
}
// モーション再生:画面外スキップ用
void MEMMOT_MakeMotionSkip( MEMMOT_CTRL*	mmt_ctrl )
{
    MEMMOT_DATA*	pmmtData;

    pmmtData = mmt_ctrl->pmmtData;

    // 時間更新
    mmt_ctrl->m_time += mmt_ctrl->m_time_base;
   
    // モーションフラグ更新
    if ( (int)mmt_ctrl->m_time >= pmmtData->m_len[mmt_ctrl->current_mot]){
	mmt_ctrl->nPlayFlag = MEMMOT_PLAY_END;	    // 終了した
	mmt_ctrl->m_time = 0.f;
	mmt_ctrl->nLoop++;	// ループ回数を更新
    }else if ( (int)(mmt_ctrl->m_time) >= ( pmmtData->m_len[mmt_ctrl->current_mot] - (mmt_ctrl->m_time_base)) ){
	mmt_ctrl->nPlayFlag = MEMMOT_PLAY_END_PREV; // 終了
    }else {
	mmt_ctrl->nPlayFlag = MEMMOT_PLAY_ACT;	    // 実行中
    }
}

// メモリモーション制御システム初期化
void MEMMOT_InitMotion( MEMMOT_CTRL*	mmt_ctrl,
			MEMMOT_DATA*	pmmtData,
			OBJECT*		pobj,
			int		nInitMot)
{

    mmt_ctrl->current_mot  = nInitMot; 
    mmt_ctrl->current_abs  = pmmtData->m_buff + (MMT_JOINT*(pmmtData->m_ptr[mmt_ctrl->current_mot]));
    mmt_ctrl->nPlayFlag    = 0;	    
    mmt_ctrl->nLoop	   = 0;
    mmt_ctrl->m_time 	   = 0.f;
    mmt_ctrl->m_time_base  = 1.f;
    mmt_ctrl->interp_count = MMTT_INTERP_DEF;
    mmt_ctrl->interp_inc   = MMTT_INTERP_DEF;
    mmt_ctrl->interp_time  = 0.f;
    mmt_ctrl->reverse_flag = 0;
    
    mmt_ctrl->adj_x 	   = 0;
    mmt_ctrl->adj_y 	   = 0;
    mmt_ctrl->adj_turn_x   = 0;
    mmt_ctrl->adj_turn_y   = 0;
    
    mmt_ctrl->body 	   = pobj;
    mmt_ctrl->pmmtData 	   = pmmtData;
}

// モーションの総フレーム数を算出
// -1で取得失敗
static int MEMMOT_GetMotionData( int motion, int* pnMotNum, int* pnFlame )
{
    int 		i;
    MAR_HEADER*		mar_header;
    MT3_FILE_HEADER*	mt3_header;
    
    mar_header = MT_GetMotionArchives( motion );
    if ( mar_header == NULL ){
	MAO_PRINTF("motion init faild!! -- no motions file %d", motion);
	return ( -1 ) ;
    }

    (*pnMotNum) = mar_header->n_motion; // モーション数

    // フレーム数
    (*pnFlame) = 0;
    for ( i = 0; i < (*pnMotNum); i++){
	mt3_header = mar_header->data_table[i].addr; // MT3ヘッダ
	(*pnFlame) += COUNT_VMODE(mt3_header->motion_length); // 長さ
    }

    return 0;
}

// メモリモーション初期化
// 全モーションの全フレーム数を先計算する
int MEMMOT_MotionPreCalloc( MEMMOT_DATA*	pmmtData, 	// メモリモーションデータ
			    int 		motion, 	// モーションデータ名 strcode
			    int 		model) 	 	// モデルデータ名 strcode
{
    int		i, len, sum ;
    OBJECT	*body, *body_mtbuff ;
    FVECTOR	*p;
    FVECTOR	*s, *step_buff;
    u_short	*h;
    int 	mot_num  = 0; 	// モーション数
    int 	mot_flam = 0;	// トータルフレーム数
#ifdef DEBUG_MODE
    int		nDataSize = 0;
#endif

    // モーション数とフレーム数取得
    if ( MEMMOT_GetMotionData( motion, &mot_num, &mot_flam) == -1 ){
	MAO_PRINTF(" Cannot Read motion file!! %s %d", __FILE__, __LINE__ );
	MEMMOT_FreeWork( pmmtData); // 解放処理
	return -1 ;
    }

    pmmtData->mot_num = mot_num ;

    // オブジェクトバッファ確保 １モーションに１オブジェクト必要 
    if( (pmmtData->body_mtbuff = (OBJECT *)GV_Malloc(sizeof(OBJECT) * mot_num ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Memory Motion Object Buffer !!\n");
	MEMMOT_FreeWork( pmmtData); // 解放処理
	return -1 ;
    }
    GV_ZeroMemory(pmmtData->body_mtbuff, sizeof(OBJECT) * mot_num );
#ifdef DEBUG_MODE
    nDataSize += sizeof(OBJECT) * mot_num;
#endif
#if 1
    // 移動量バッファ確保 １モーションに１つ必要 
    if( (pmmtData->step_buff = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * mot_num ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Memory Motion Step Buffer !!\n");
	MEMMOT_FreeWork( pmmtData); // 解放処理
	return -1 ;
    }
#ifdef DEBUG_MODE
    nDataSize += sizeof(FVECTOR) * mot_num;
#endif
#endif
    // モーション保管用オブジェクト初期化 
    body_mtbuff = pmmtData->body_mtbuff ;
    step_buff = pmmtData->step_buff ;

    for( i = 0; i < mot_num; i++ ) {
	GM_InitObject( body_mtbuff, model, BODY_FLAG );

	GM_ConfigObjectMotion( body_mtbuff, 0, motion, MT_FLAG_HUMAN1 );

	GM_ConfigObjectStep( body_mtbuff, step_buff ) ;

	body_mtbuff->objs->flag |= DG_FLAG_INVISIBLE ;

	/* sar ファイルを使用しない プリ計算時にカメラ関係の初期化がまだなので落ちちゃう */
	if ( body_mtbuff->m_ctrl->sar_ctrl != NULL ) {
	    GV_Free( body_mtbuff->m_ctrl->sar_ctrl ) ;
	    body_mtbuff->m_ctrl->sar_ctrl = NULL ;
	}
	body_mtbuff++;
	step_buff++;
    }

    // ポインタバッファ 
    if( (pmmtData->m_ptr = (u_short *)GV_Malloc(sizeof(u_short) * mot_num))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Memory Motion Pointer !!\n");
	MEMMOT_FreeWork( pmmtData); // 解放処理
	return -1 ;
    }
#ifdef DEBUG_MODE
    nDataSize += sizeof(u_short) * mot_num;
#endif

    GV_ZeroMemory(pmmtData->m_ptr, sizeof(u_short) * mot_num );
    
    // 長さバッファ
    if( (pmmtData->m_len = (u_short *)GV_Malloc(sizeof(u_short) * mot_num))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Memory Motion Length !!\n");
	MEMMOT_FreeWork( pmmtData); // 解放処理
	return -1 ;
    }
    GV_ZeroMemory(pmmtData->m_len, sizeof(u_short) * mot_num );
#ifdef DEBUG_MODE
    nDataSize += sizeof(u_short) * mot_num;
#endif
#if 1
    // 高さバッファ
    if( (pmmtData->m_height = (u_short *)GV_Malloc(sizeof(u_short) * mot_flam))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Memory Motion Height Buffer !!\n");
	MEMMOT_FreeWork( pmmtData); // 解放処理
	return -1 ;
    }
    GV_ZeroMemory(pmmtData->m_height,sizeof(u_short) * mot_flam );
#ifdef DEBUG_MODE
    nDataSize += sizeof(u_short) * mot_flam;
#endif
#endif
    // モーションバッファ
    if( (pmmtData->m_buff = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * MMT_JOINT * mot_flam ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Memory Motion Motion Buffer !!\n");
	MEMMOT_FreeWork( pmmtData); // 解放処理
	return -1 ;
    }
    GV_ZeroMemory(pmmtData->m_buff,	sizeof(FVECTOR) * MMT_JOINT * mot_flam );
#ifdef DEBUG_MODE
    nDataSize += sizeof(FVECTOR) * MMT_JOINT * mot_flam;
#endif
#if 1
    // 移動量バッファ
    if( (pmmtData->m_step = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * mot_flam ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Memory Motion Step Buffer !!\n");
	MEMMOT_FreeWork( pmmtData); // 解放処理
	return -1 ;
    }
    GV_ZeroMemory(pmmtData->m_step,sizeof(FVECTOR)*mot_flam );
#ifdef DEBUG_MODE
    nDataSize += sizeof(FVECTOR) * mot_flam;
#endif
#endif
    // 絶対回転量バッファ
    if( (pmmtData->abs_rots = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * MMT_JOINT * mot_flam ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Memory Motion Motion Buffer !!\n");
	MEMMOT_FreeWork( pmmtData); // 解放処理
	return -1 ;
    }
    GV_ZeroMemory(pmmtData->abs_rots, sizeof(FVECTOR) * MMT_JOINT * mot_flam );
#ifdef DEBUG_MODE
    nDataSize += sizeof(FVECTOR) * MMT_JOINT * mot_flam;
printf("----------Memory motion----------\n");
printf("Memory motion Num 	  = %d\n", mot_num);
printf("Memory motion Flame 	  = %d\n", mot_flam);
printf("Memory motion malloc size = %d\n", nDataSize);
#endif

    // ダミーSEモーションテーブル
    //    MT_SetMotionSeTable( pmmtData->body_mtbuff->m_ctrl, GM_CurrentMap, 1, 0, 0 ) ;

    sum = 0 ;
    p = pmmtData->m_buff ;
    s = pmmtData->m_step ;
    h = pmmtData->m_height ;
    body = pmmtData->body_mtbuff ;
    DG_SetPos( &DG_UnitMatrix ) ;
    for( i=0; i < mot_num; i++ ) {
	pmmtData->m_ptr[i] = sum ;
#ifdef PAL
//	MT_SetMotionSpeed( body->m_ctrl, (float)(( float )TIME_BASE * 1.2f) ); // PAL版は1.2倍
#endif
        GM_ConfigObjectAction( body, 0, i, 0, 0xffff, 0 );
	len = COUNT_VMODE(body->m_ctrl->mt3_ctrl[0].file_header->motion_length) ;
	ASSERT( len >= 0 && len < 2048);
	pmmtData->m_len[i] = len ;
	sum += len;

	while( len-- > 0 ){
	    DG_SetPos( &DG_UnitMatrix ) ;
	    GM_ActObject( body ) ;
#if 0
	    KR_MemCopy( p, body->m_ctrl->abs_rots, sizeof(FVECTOR), MMT_JOINT ) ;
	    KR_MemCopy( s, body->step, sizeof(FVECTOR), 1 ) ;
#else
	    memcpy( p, body->m_ctrl->abs_rots, sizeof(FVECTOR) * MMT_JOINT ) ;
	    memcpy( s, body->step, sizeof(FVECTOR)) ;
#endif
	    *h = body->height ;

	    p += MMT_JOINT ;
	    s++;
	    h++;
	}
	body++;
    }
	
    return 0 ;
}
// 破棄
void MEMMOT_FreeWork(MEMMOT_DATA* pmmtData)
{
    int i;
    for( i=0; i < pmmtData->mot_num; i++ ) {
	GM_FreeObject( &pmmtData->body_mtbuff[i] );
    }
    if ( pmmtData->body_mtbuff != NULL) GV_Free( pmmtData->body_mtbuff);
    if ( pmmtData->step_buff   != NULL) GV_Free( pmmtData->step_buff);
    if ( pmmtData->m_ptr       != NULL) GV_Free( pmmtData->m_ptr);
    if ( pmmtData->m_len       != NULL) GV_Free( pmmtData->m_len);
    if ( pmmtData->m_height    != NULL) GV_Free( pmmtData->m_height);
    if ( pmmtData->m_buff      != NULL) GV_Free( pmmtData->m_buff);
    if ( pmmtData->m_step      != NULL) GV_Free( pmmtData->m_step);
    if ( pmmtData->abs_rots    != NULL) GV_Free( pmmtData->abs_rots);

}


/*----------------------------------------------------------------*/
/*
	モーションが指定時間を通過したかどうかをチェック
	（複数チェックする場合には小さい値から順番にチェックすること）
*/
int MEMMOT_CheckMotionTime( MEMMOT_CTRL* pmmCtrl, int time )
{
    if ( pmmCtrl->last_chk_time >= time ) return ( 0 );
    if ( DG_FTOI( pmmCtrl->m_time ) >= time ){
	pmmCtrl->last_chk_time = time ;
	return ( 1 );
    }
    return ( 0 );
}
