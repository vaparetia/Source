//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	obj_act.c
	オブジェクト表示アクター

	2000/07/26	K.Takabe
	$Id: obj_act.c,v 1.3 2002/11/23 12:46:54 Yoshizawa1 Exp $

*/
/*

	void *NewDemoObject( int id, int name, int cache_id, int flag )
	int		id ;		デモ認識ＩＤ
	int		name ;		キャラクタ名
	int		cache_id ;	モデルデータキャッシュＩＤ（モデル名ＩＤ｜拡張子ＩＤ）
	int		flag ;		デモモデル用フラグ
						0x0001	単一オブジェクト

	デモオブジェクト表示用アクター

*/
#endif

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
#include <def_dma.h>
#include <libfs.h>
#include <stream.h>
#include <gameheader.h>
#include <camera.h>

#include "libdemo.h"
#include "libutl.h"


/* ---------------------------------------------------------------- */
typedef struct _obj_act_work{
	GV_ACT_EX	actor ;
	FMATRIX		light[2] ;		/* 光源マトリクス */
	CONTROL		control ;		/* コントロール */
	OBJECT		object ;		/* オブジェクト */
	int			id ;		/* デモ管理ＩＤ */
	int			name ;		/* キャラクタ名（メッセージやコントロール検索用） */
	int			onepiece_flag ;	/* 0:関節付きモデル  1:関節なしモデル */
	int			evm_flag ;		/* 0:kms, 1:evm */
	void			*skeleton ;		/* モーションが参照するスケルトン */
	void (*set_motion)( struct _obj_act_work *work, OBJECT *body, DEMO_MOTION *motion, void *skeleton );
} Work ;

/* ---------------------------------------------------------------- */
typedef struct _scrpad_kms{
	FMATRIX		world ;
	FMATRIX		matrix[128] ;
	FVECTOR		rots[128] ;
	FVECTOR		trans[128] ;
	FVECTOR		vec ;
} Scrpad ;

/* ---------------------------------------------------------------- */
	/*
		各種インライン関数
	*/
static inline void InitVector( FVECTOR *vec )
{
#ifndef BP_PSX2_ASM
	*vec = DG_ZeroVector ;
#else
	asm volatile ( "sqc2	vf00,0(%0)"::"r"(vec) );
#endif
}

#if 0//BP
static inline void CopyVector( FVECTOR *dst, FVECTOR *src )
{
#ifndef PSX2
	*dst = *src ;
#else
	asm volatile ( "
		lq		$5,0(%1)
		sq		$5,0(%0)
	"::"r"(dst),"r"(src):"$5","memory" );
#endif
}
#endif

/* ---------------------------------------------------------------- */
/* スクラッチパッド上のモーションデータを初期化する */
static void InitMotion( int max_joint )
{
	Scrpad	*scrpad = SCRPAD_ADDR ;
	FVECTOR	*rot, *trans ;
	int		i ;

	rot = scrpad->rots ;
	trans = scrpad->trans ;
	for ( i = max_joint ; i > 0 ; i-- ){
		InitVector( rot++ );
		InitVector( trans++ );
	}

}
/* モーションデータをスクラッチパッドに読み込む */
static void LoadMotion( DEMO_MOTION *motion )
{
	Scrpad	*scrpad = SCRPAD_ADDR ;
	FVECTOR	*rot, *trans, *vec ;
	int		i ;

	/* スクラッチパッドに転送 */
	vec = motion->motion ;
	rot = &scrpad->rots[ motion->start_joint ] ;
	trans = &scrpad->trans[ motion->start_joint ] ;
	switch ( motion->motion_type ){
	  case DEMO_MOTION_ROT:
		for ( i = motion->n_joints ; i > 0 ; i-- )
      {
         bp_math_assert( BP_Vec4_Check( vec ) );
			*rot++ = *vec++ ;
		}
		break ;
	  case DEMO_MOTION_TRANS:
		for ( i = motion->n_joints ; i > 0 ; i-- )
      {
         bp_math_assert( BP_Vec4_Check( vec ) );
			*trans++ = *vec++ ;
		}
		break ;
	  case DEMO_MOTION_ROTTRANS:
		for ( i = motion->n_joints ; i > 0 ; i-- )
      {
         bp_math_assert( BP_Vec4_Check( &vec[0] ) );
         bp_math_assert( BP_Vec4_Check( &vec[1] ) );
			*rot++ = *vec++ ;
			*trans++ = *vec++ ;
		}
		break ;
	}
}

/* ＫＭＳモーション設定 */
static void SetMotionKms( Work *work, OBJECT *body, DEMO_MOTION *motion, void *skeleton )
{
	DG_OBJS		*objs ;
	DG_OBJ		*obj ;
	DG_DEF		*def ;
	DG_MDL		*mdl ;
	Scrpad		*scrpad = SCRPAD_ADDR ;
	int			i ;

	objs = body->objs ;

	/* 定数回転マトリクス位置に配置 */
	DG_PutObjs( objs );

	/* マップ変更処理 */
	if ( body->map_name != GM_CurrentMap ) {
	    body->map_name = GM_CurrentMap ;
	}

	/* オブジェクトのグループ変更 */
	GM_GroupObjs( objs, GM_CurrentMap ) ;

	/* 関節が無ければここで終了 */
	if ( work->onepiece_flag & 1 ) return ;

	DG_GetPos( &scrpad->world );

	/* 関節のモーションを設定（モーションパケットが複数に別れている事も考慮する） */
	def = objs->def ;
   
   InitMotion( DG_MAX( def->n_models, objs->n_models ) );//BP_ANIM - initialize all joints

   if ( motion ) {
	    //InitMotion( def->n_models );  //BP_ANIM - initialized all joints above
	    while ( motion != NULL ){
		LoadMotion( motion );
		motion = DM_GetMotionDataNext();
	    }
	}

	{
		FMATRIX		*mats ;
		FVECTOR		*rots ;
		FVECTOR		*trans ;
		int			parent ;

		obj = objs->objs ;
		mdl = def->models ;
		mats = scrpad->matrix ;
		rots = scrpad->rots ;
		scrpad->vec.vw = 1.0f ;
		trans = scrpad->trans ;

		for ( i = objs->n_models ; i > 0 ; i-- ){
			/* クォータニオンからマトリクスへ */
			MT_QuatToMat( mats, rots );
			/* 親からの相対回転マトリクスからワールドでの絶対回転マトリクスへ */
			mats->m[3][0] = obj->trans.vx + trans->vx ;
			mats->m[3][1] = obj->trans.vy + trans->vy ;
			mats->m[3][2] = obj->trans.vz + trans->vz ;
			mats->m[3][3] = 1.0f;
			parent = mdl->parent ;
			_sceVu0MulMatrix( mats, &scrpad->matrix[ parent ], mats );
			/* 計算したマトリクスをメモリに書き戻す */
			obj->world = *mats ;
			if ( i == objs->n_models )
			    objs->world = *mats ;
			/* 次の処理へ */
			obj++ ;
			mdl++ ;
			mats++ ;
			rots++ ;
			trans++ ;
		}

	}
}

static void MakeMatrixFromSkeleton( int from, int to,
				    DG_EVMOBJ *evmobj, EVM_SKEL *skel )
{
    Scrpad *scrpad = SCRPAD_ADDR   ;
    FMATRIX *mats = &scrpad->matrix[from] ;
    FVECTOR *rots = &scrpad->rots[from]   ;  
    FVECTOR *trans= &scrpad->trans[from]  ;  
    int parent, i ;

    scrpad->vec.vw = 1.0f ;
    skel += from ;
    for ( i=from ; i<to ; i++ )
    {
	/* クォータニオンからマトリクスへ */
	MT_QuatToMat( mats, rots );

	/* 親からの相対回転マトリクスからワールドでの絶対回転マトリクスへ */
	parent = skel->parent ;
	_sceVu0MulMatrix( mats, &scrpad->matrix[ parent ], mats );

	/* マトリクスの位置を関節間の距離から計算 */
	scrpad->vec.vx = skel->rt_tx + trans->vx ;
	scrpad->vec.vy = skel->rt_ty + trans->vy ;
	scrpad->vec.vz = skel->rt_tz + trans->vz ;
	_sceVu0ApplyMatrix( (FVECTOR*)&mats->m[3][0],
			    &scrpad->matrix[ parent ],
			    &scrpad->vec );

	/* 腰の位置に戻す */
	scrpad->vec.vx = -skel->rt_tx ;
	scrpad->vec.vy = -skel->rt_ty ;
	scrpad->vec.vz = -skel->rt_tz ;
	_sceVu0ApplyMatrix( (FVECTOR*)&mats->m[3][0], mats, &scrpad->vec );

	/* 次の処理へ */
	skel++ ;
	mats++ ;
	rots++ ;
	trans++ ;
    }
}

static void SetKmsMatrixFromEvm( DG_EVMOBJ *evmobj, OBJECT *body )
{
    Scrpad   *scrpad = SCRPAD_ADDR ;
    DG_OBJ   *obj  ;
    FMATRIX  *mats ;
    EVM_SKEL *skel ;
    int       i    ;

    DG_StartSprToMem( evmobj->matrix[ evmobj->use_buffer ], scrpad->matrix,
		      evmobj->n_skeleton * 4 ) ;
    if ( body->objs )
    {
	obj  = body->objs->objs ;
	skel = evmobj->def->skeleton ;
	mats = scrpad->matrix;

	for ( i=body->objs->n_models ; i>0 ; i-- )
	{
	    scrpad->vec.vx = skel->rt_tx ;
	    scrpad->vec.vy = skel->rt_ty ;
	    scrpad->vec.vz = skel->rt_tz ;
	    fpu_CopyMatrix( &obj->world, mats ) ;
	    _sceVu0ApplyMatrix( (FVECTOR*)&(obj->world.m[3][0]),
				mats,
				&scrpad->vec );
	    skel++;
	    mats++;
	    obj++;
	}

	body->objs->world = *scrpad->matrix ;
    }
    DG_EndSprToMem();
}

static void SetEvmMatrixForLowPoly( int from, int to,
				    DG_EVMOBJ *evmobj, EVM_SKEL *skel )
{
    Scrpad   *scrpad = SCRPAD_ADDR ;
    FMATRIX  *mats = &scrpad->matrix[from] ;
    EVM_SKEL *skel_org = evmobj->def->skeleton ;
    int      i ;

    /* 関節長さの差分を今のマトリックスに掛けることでオリジナルに転写できる*/
    scrpad->vec.vw = 1.0f ;
    skel_org += from ;
    skel     += from ;
    for ( i=from ; i<to ; i++ )
    {
	/* マトリクスの位置を関節間の距離から計算 */
	scrpad->vec.vx = skel->rt_tx - skel_org->rt_tx ;
	scrpad->vec.vy = skel->rt_ty - skel_org->rt_ty ;
	scrpad->vec.vz = skel->rt_tz - skel_org->rt_tz ;
	_sceVu0ApplyMatrix( (FVECTOR*)&mats->m[3][0], mats, &scrpad->vec ) ;
	/* 次の処理へ */
	skel++ ;
	skel_org++ ;
	mats++ ;
    }
}


/* ＥＶＭモーション設定 */
static void SetMotionEvm( Work *work, OBJECT *body, DEMO_MOTION *motion, void *skeleton )
{
	DG_EVMOBJ  *evmobj ;
	Scrpad     *scrpad = SCRPAD_ADDR ;

	evmobj = body->evmobj ;

	/* 定数回転マトリクス位置に配置 */
	DG_GetPos( &evmobj->world );
	DG_GetPos( &scrpad->world );

	/* マップ変更処理 */
	if ( body->map_name != GM_CurrentMap )
	    body->map_name = GM_CurrentMap ;

	/* オブジェクトのグループ変更 */
	GM_GroupObject( evmobj, GM_CurrentMap );

	/* 関節のモーションを設定（モーションパケットが複数に別れている事も考慮する） */
	InitMotion( evmobj->n_skeleton );
	while ( motion != NULL ){
		LoadMotion( motion );
		motion = DM_GetMotionDataNext();
	}

	/*  */
	MakeMatrixFromSkeleton( 0, evmobj->n_skeleton,
				evmobj,
				evmobj->def->skeleton ) ;
	evmobj->world = *scrpad->matrix ;

	/* 計算したマトリクスをメモリに書き戻す */
   DG_Arm_SwitchEvmBuffer(evmobj);
	FlushCache( 0 );

	/* 修正 : EVMの関節位置にKMSを持っていく
	   2001/5/1  Ken Kano                     */
	SetKmsMatrixFromEvm( evmobj, body ) ;
}



/* ローポリ劇場用 ＥＶＭモーション設定 */
static void SetMotionLowPoly( Work *work, OBJECT *body, DEMO_MOTION *motion, void *skeleton )
{
	DG_EVMOBJ *evmobj ;
	Scrpad	  *scrpad = SCRPAD_ADDR ;

	evmobj = body->evmobj ;

	/* 定数回転マトリクス位置に配置 */
	DG_GetPos( &evmobj->world );
	DG_GetPos( &scrpad->world );

	/* マップ変更処理 */
	if ( body->map_name != GM_CurrentMap ) {
	    body->map_name = GM_CurrentMap ;
	}

	/* オブジェクトのグループ変更 */
	GM_GroupObject( evmobj, GM_CurrentMap );

	/* 関節のモーションを設定 (モーションパケットが複数に別れてい
           る事も考慮する) */
	InitMotion( evmobj->n_skeleton );
	while ( motion != NULL ){
		LoadMotion( motion );
		motion = DM_GetMotionDataNext();
	}

	/* 顔以外の関節を変換する */
	MakeMatrixFromSkeleton( 0, 21,
				evmobj, ((EVM_DEF *)skeleton)->skeleton ) ;
	/* 各関節の補正(元の関節の差をマトリックスに掛け直す) */
	SetEvmMatrixForLowPoly( 0, 21,
				evmobj, ((EVM_DEF *)skeleton)->skeleton ) ;
	/* 顔の関節は最後にする(頭の位置が補正されてから) */
	MakeMatrixFromSkeleton( 21, evmobj->n_skeleton,
				evmobj, evmobj->def->skeleton ) ;
	evmobj->world = *scrpad->matrix ;

	/* 計算したマトリクスをメモリに書き戻す */
   DG_Arm_SwitchEvmBuffer(evmobj);
	FlushCache( 0 );

	/* 修正 : EVMの関節位置にKMSを持っていく
	   2001/5/1  Ken Kano                     */
	SetKmsMatrixFromEvm( evmobj, body ) ;
}

/* ---------------------------------------------------------------- */
	/*
		メイン処理
	*/
static void Act( Work *work )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;
	DEMO_MOTION	*motion ;

	ctrl = &work->control ;
	body = &work->object ;

	/* 自分用のモーションを取得する */
	motion = DM_GetMotionData( work->id );
	if ( motion != NULL )
	{
	    /* コントロールの更新 */
	    ctrl->mov = motion->pos ;
       ctrl->mov.vw = 0.f; //BP
	    ctrl->rot = ctrl->turn = motion->rot ;
	    GM_ActControl( ctrl );

	   ctrl->mov.vy+=300.0f ;
	    ctrl->level_found = HZX_LevelHazardCheck( ctrl->hzx_id,
						      &ctrl->mov,
						      ctrl->hzx_check_type,
						      ctrl->flr_flag ) ;
	   ctrl->mov.vy-=300.0f ;
	    if ( ctrl->level_found )
	    {
		HZX_GetLevelHeight( ctrl->levels ) ;
		HZX_GetLevelHazard( ctrl->level[0], ctrl->flr_atrs ) ;
		ctrl->grounded = 1 ;
	    }
	    else
		ctrl->grounded = 0 ;
	    if ( !(ctrl->level_found & 1) )
		ctrl->levels[0] = GM_WORLD_LIMIT_BOTTOM ;
	    if ( !(ctrl->level_found & 2) )
		ctrl->levels[1] = GM_WORLD_LIMIT_UPPER ;

	    /* モーションの設定 */
	    (*work->set_motion)( work, body, motion, work->skeleton ) ;

		/* 腰の高さを入れる */
		body->height = body->objs->objs[0].world.m[3][1] - ctrl->levels[0] ;
	}
	else
	    GM_ActControl( ctrl );

	/* 光源反映させる */
	DG_GetLightMatrix( &ctrl->mov, work->light );
}

#if 1//BP
//#ifdef KP_WINDOWS
static void ActDemoDummy( Work *work )
{
	OBJECT	*body ;

	/* 本体処理 */
	Act(work) ;

	/* モデルがdemo_dummyだった場合は強制的に非表示に設定する */
	body = &work->object ;
	body->objs->flag |= DG_FLAG_INVISIBLE ;
}
#endif
/* ---------------------------------------------------------------- */
	/*
		終了処理
	*/
static void Die( Work *work )
{
	/* オブジェクトの開放 */
	GM_FreeControl( &work->control );
	GM_FreeObject( &work->object );
}

/* ---------------------------------------------------------------- */

static inline void ChangeCostume( Work *work, int name,
				  int evm_id, int kms_id, CONTROL *ctrl,
				  int mode, int type, int from_to )
{
    int   flag, id ;
    int   i     ;
    void *child ;
    extern void *NewDemoAllEquip( int name, CONTROL *ctrl,
				  int type, int flag ) ;
    extern void *NewDemoAllChange( int name, OBJECT *object,
				   int mode, int type, int flag ) ;
    DM_WORK *dm_work ;

    dm_work = DM_GetDemoWork() ;
    if ( dm_work->low_poly_demo )
    {
	/* 上位8ビットは伏せてしまう */
	evm_id &= 0x00ffffff ;
	kms_id &= 0x00ffffff ;

	/* ローポリ劇場 EMV を入れ換えた */
	if ( (from_to == ((('e'-'a')<<8) | ('k'-'a'))) ||
	     (from_to == ((('e'-'a')<<8) | ('a'-'a'))) )
	    flag = 1, work->evm_flag = 0 ;
	else	    
	    flag = 0 ;

#if DEBUG_MODE
	if ( flag )
	    printf( "Change %d %d to Low ARKMS md%d ty%d %d\n", 
		    evm_id, kms_id, mode, type, flag ) ;
#endif

	/* ARKMS系のローポリに入れ換え(EMVINVISIBLERとしても起動する) */
	child = NewDemoAllChange( name, ctrl->object, mode, type, flag );
	if ( child )
	    GV_SetActorChild( work, child ) ;

	/* 装備IDに渡すものを決める */
	if ( /*from_to == 0 ||*/
	     from_to == ((('e'-'a')<<8) | ('e'-'a')) )
	    id = evm_id ;
	else if ( from_to == ((('e'-'a')<<8) | ('k'-'a')) )
	    id = kms_id ;
	else
	    id = evm_id ? evm_id : kms_id ;

#if 1
	printf( "Costume id=%d frmto%x %x ->",
		id, from_to,
		((('e'-'a')<<8) | ('e'-'a')),
		((('e'-'a')<<8) | ('k'-'a'))  ) ;
#endif

	/* 装備フラグ */
	flag = 0x80000000 ;/* デフォルトの装備にする */
	for( i=DM_n_EquipList ; --i>=0 ; )
	{
	    if ( mode && DM_EquipTo[i] == mode )
	    {
		flag = DM_EquipFlag[i] ;
		id = mode ;
		break ;
	    }
	    else if ( id && DM_EquipTo[i] == id )
	    {
		flag = DM_EquipFlag[i] ;
		break ;
	    }
	}

	if ( id && !(flag & 0x40000000) )
	{
	    child = NewDemoAllEquip( name, ctrl, id, flag ) ;
	    if ( child )
		GV_SetActorChild( work, child ) ;
	}
    }
}


	/*
		初期化処理
	*/
static int GetResources( Work *work, int id, int name, int evm_id, int kms_id, int flag )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;
	int			body_flag ;
	int  i ;
	extern int DG_MakePreshade( DG_OBJS *objs, LIT_DEF *lit_def ) ;
	int  from_to ;
	int  type, mode ;

	/* デモデーモンではありあえないモデルははじいいてしまう */
	if ( (kms_id & 0x00ffffff) == 7584084 ) {
		return -1 ;
	}

	work->id = id ;
	work->name = name ;
	ctrl = &work->control ;
	body = &work->object ;

	if ( evm_id )
	    work->skeleton = GV_GetCache( evm_id ) ;

	from_to   = 0 ;
	type      = 0 ;
	mode      = 0 ;
	body_flag = 0 ;
	for( i=DM_n_RenameList ; --i>=0 ; )
	{
	    /* ローポリ劇場用 */
	    if ( DM_RenameFrom[i] & 0x80000000 )
	    {
		if ( (DM_RenameFrom[i] & 0x7fffffff) == evm_id ||
		     (DM_RenameFrom[i] & 0x7fffffff) == kms_id )
		{
		    from_to = (DM_RenameTo[i] & 0x7f000000) >> 24 ;
		    if ( from_to == 'k'-'a' )
			kms_id = DM_RenameTo[i] & 0x7fffffff ;
		    else if ( from_to == 'e'-'a' )
			evm_id = DM_RenameTo[i] & 0x7fffffff ;
		    else
		    {
			mode =  DM_RenameTo[i]       & 0x00ff ;
			type = (DM_RenameTo[i] >> 8) & 0xffff ;
		    }
		    from_to |= (DM_RenameFrom[i] & 0x7f000000) >> 16 ;
		}
		else
		    continue ;
	    }
	    /* IDチェンジ 兵装スゲ変え用 */
	    else if ( DM_RenameFrom[i] == kms_id )
		kms_id = DM_RenameTo[i] ;
	    else if ( DM_RenameFrom[i] == evm_id )
		evm_id = DM_RenameTo[i] ;
	    else
		continue ;

		/* 描画フラグの変更 */
		if ( DM_RenameDispFlag[i] ) {
			flag |= 0x10 ;
		}
	    break ;
	}

	/* モデルを非表示で初期化 */
	if ( kms_id ){
		/* IDチェンジ 兵装スゲ変え用 */
		printf("start kms-object(%d)flag %x\n", kms_id & 0x00ffffff, flag );

		/* ＫＭＳモデルの初期化 */
		if ( flag & 0x02 ) {
			body_flag = DG_FLAG_PAINT | DG_FLAG_FINISHCALC ;
			work->onepiece_flag = 0 ;
		} else if ( flag & 0x01 ){
			body_flag = DG_FLAG_SHADE | DG_FLAG_FINISHCALC ;
			work->onepiece_flag = 0 ;
		} else {
			body_flag = DG_FLAG_SHADE | DG_FLAG_ONEPIECE ;
			work->onepiece_flag = 1 ;
		}
		if ( flag & 4 ){
		    body_flag |= DG_FLAG_SEMITRANS ;
          body_flag |= DG_FLAG_FORCEMSAA ;
		}
		if ( flag & 0x10 ) {
		    body_flag |= DG_FLAG_LATTERDRAW ;
		}

      // AS(JA) -- force the MO disk entirely into the main scene, so the disc and metal bits z-occlude the shell properly
      if ( (kms_id & 0x00ffffff) == 2346735 ) // mo_disk
      {
         body_flag |= DG_FLAG_NOMSAA ;
      }
		GM_InitObject( body, kms_id & 0x00ffffff, body_flag );
		body->objs->flag |= DG_FLAG_INVISIBLE ;
		/* ライトマトリクスの設定 */
		DG_SetLightMatrix( body->objs, work->light );

		/* モーション設定関数の設定 */
		work->set_motion = SetMotionKms ;

#if 1 //BP - unclear what this object is they're hiding in the windows version.
      //It's found in stage d00t (intro cinematic).
//#ifdef KP_WINDOWS
		/* モデルがdemo_dummyだった場合は強制的に非表示に設定する */
		if ( (kms_id & 0x00ffffff) == 6604458 ) {
         BP_TRIVIAL_BREAK;
			//BP body->objs->flag |= DG_FLAG_INVISIBLE ; //
		}
#endif
		printf( "init kms-object(%d)\n",kms_id & 0x00ffffff );
	}

	if ( evm_id )
	{
		printf("start evm-object(%d)\n", evm_id & 0x00ffffff );

		/* ＥＶＭモデルの初期化 */
		body_flag = 0 ;
		if ( flag & 0x04 )
		    body_flag |= DG_EVMOBJ_SEMITRANS  ;
		if ( flag & 0x10 )
		    body_flag |= DG_EVMOBJ_LATTERDRAW ;
		GM_ConfigObjectEvm( body, evm_id & 0x00ffffff, body_flag );

		body->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;

		/* ライトマトリクスの設定 */
		body->evmobj->light = work->light ;
		/* 体験版の対処としてEVMの場合にはデフォルトで影をONにする */
		body->objs->flag |= DG_FLAG_SHADOWMAKE ;

		/* モーション設定関数の設定 */
		if ( from_to )
		    work->set_motion = SetMotionLowPoly ;
		else
		    work->set_motion = SetMotionEvm ;

		printf("init evm-object(%d)\n", evm_id & 0x00ffffff );
	}

	/* コントロール構造体の初期化 */
	GM_InitControl( ctrl, name, GM_CurrentMap );
	GM_ConfigControlHazard( ctrl, 1, 1, 1 );
	GM_ConfigControlObject( ctrl, body );

	ctrl->flr_flag = HZX_FLOOR_ALL ;
	//ctrl->skip_flag |= CTRL_SKIP_HZX ;	/* ハザードチェック無視 */

	/* ARKMS系のローポリ劇場用着せ変え ＆ 装備変え */
	ChangeCostume( work, name, evm_id, kms_id, ctrl,
		       mode, type, from_to ) ;

	/* モーションの設定(位置の初期化) */
	Act( work ) ;

	/* プリシェード処理(KMSでかつフラグが立っていたら 無加重状態のみ) */
	if ( !evm_id && (flag & 2) )
	    DG_MakePreshade( body->objs, GM_GetMap( GM_CurrentMap )->light ) ;

	return ( 0 );
}
/* ---------------------------------------------------------------- */
	/*
		シグナルレシーブ関数
	*/
static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *work = workp;
	switch( signal ){

	  case DM_SIGNAL_OBJECT_VISIBLE:
		if ( work->evm_flag == 0 ){
#ifdef DEBUG_MODE
			/* 逆転再生時の動作を逆転させる */
#ifdef PSX2
			if ( (dm_debug.mode1 != 0) && (dm_debug.play_status == 2) )
			{
				if ( value == 0 ){
					work->object.objs->flag |= DG_FLAG_INVISIBLE ;		/* 非表示 */
				} else {
					work->object.objs->flag &= ~DG_FLAG_INVISIBLE ;		/* 表示 */
				}
				break;
			}
#endif
#endif
			/* ＫＭＳモデルの場合 */
			if ( value == 0 ){
				work->object.objs->flag &= ~DG_FLAG_INVISIBLE ;		/* 表示 */
			} else {
				work->object.objs->flag |= DG_FLAG_INVISIBLE ;		/* 非表示 */
			}
		} else {
#ifdef DEBUG_MODE
			/* 逆転再生時の動作を逆転させる */
#ifdef PSX2
			if ( (dm_debug.mode1 != 0) && (dm_debug.play_status == 2) )
			{
				if ( value == 0 ){
					work->object.evmobj->flag |= DG_EVMOBJ_INVISIBLE ;	/* 非表示 */
				} else {
					work->object.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;	/* 表示 */
				}
				break;
			}
#endif
#endif
			/* ＥＶＭモデルの場合 */
			if ( value == 0 ){
				work->object.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;	/* 表示 */
			} else {
				work->object.evmobj->flag |= DG_EVMOBJ_INVISIBLE ;	/* 非表示 */
			}
		}
		break ;

	  case DM_SIGNAL_OBJECT_GET_CONTROL:
		  printf( "LinkToCONTROL%x\n", &work->control );
		return ( (int)&work->control );

	  case DM_SIGNAL_OBJECT_GET_OBJECT:
		  printf( "LinkToOBJECT%x\n", &work->object );
		return ( (int)&work->object );

	  case DM_SIGNAL_OBJECT_GET_DGOBJS:
		  printf( "LinkToObjs%x\n", work->object.objs );
		return ( (int)work->object.objs );

	  case DM_SIGNAL_OBJECT_GET_MATRIX:
	      if ( work->evm_flag )	/* ＥＶＭモデルの場合 */
		  if ( value > 21 )
		      value = 0 ;

	      if ( value < 0 )	  /* オブジェクト中心のマトリクス */
		  printf( "LinkToWorld%x Root\n", &work->object.objs->world );
	      else		  /* 各関節のマトリクス */
		  printf( "LinkToWorld%x %d\n", &work->object.objs->objs[value].world, value );

	      if ( value < 0 )	  /* オブジェクト中心のマトリクス */
		  return ( (int)&work->object.objs->world );
	      else		  /* 各関節のマトリクス */
		  return ( (int)&work->object.objs->objs[value].world );
	      break ;

	  case DM_SIGNAL_OBJECT_GET_DGOBJ:
	      if ( work->evm_flag )	/* ＥＶＭモデルの場合 */
		  if ( value > 21 )
		      value = 0 ;

	      if ( value < 0 )	  /* オブジェクト中心のマトリクス */
		  printf( "LinkToObj%x Root\n",&work->object.objs->objs[0] );
	      else		  /* 各関節のマトリクス */
		  printf( "LinkToObj%x %d\n",&work->object.objs->objs[value], value );

	      if ( value < 0 )	/* オブジェクト中心のマトリクス */
		  return ( (int)&work->object.objs->objs[0] );
	      else 			/* 各関節のマトリクス */
		  return ( (int)&work->object.objs->objs[value] );
	      break ;

	case DM_SIGNAL_OBJECT_GET_LIGHTMTX:
		return ( (int)work->light ) ;
		break ;
	case DM_SIGNAL_OBJECT_GET_EVM:
		return ((int)work->object.evmobj);
		break;
	default:
		return ( GV_DefaultSignalFunc( workp, signal, value ) );
		break ;
	}
	return 0;
}
/* ---------------------------------------------------------------- */
	/*
		デモオブジェクト表示用アクター
	*/
void *NewDemoObject( int id, int name, int evm_id, int kms_id, int flag )
{
	Work		*work ;
	int			evm_flag = 0 ;
#if 1//BP
//#ifdef KP_WINDOWS
	void		(*act_func)(Work *) ;
#endif

#if 1 //BP - unclear what this object is they're hiding in the windows version.
   //It's found in stage d00t (intro cinematic)
//#ifdef KP_WINDOWS
	if ( (kms_id & 0x00ffffff) == 6604458 ) {
      BP_TRIVIAL_BREAK;
		act_func = Act; //BP ActDemoDummy ;	// demo_dummy
	} else {
		act_func = Act ;
	}
#endif

	OPERATOR() ;
	/* モデルタイプ判別 */
	if ( ( ( ( evm_id & ~GV_CACHEID_RESIDENT ) >> 24 ) + 'a' ) == 'e' ){
		evm_flag = 1 ;
	} else {
		evm_flag = 0 ;
	}

	/* アクター生成 */
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	work->evm_flag = evm_flag ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die ) ;
#else
		GV_SetActor( &( work->actor ), act_func, Die ) ;
#endif
		GV_ActorEX( &work->actor );
		if ( GetResources( work, id, name, evm_id, kms_id, flag ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		/* シグナルレシーブ関数設定 */
		GV_SetActorSignalFunc( work, ReceiveSignal );
	}
	return ( work ) ;
}

