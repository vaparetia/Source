//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    pee.c
    w25c放尿エフェクト
    2001/04/25 Yuuta Kunibe	
    $Id: pee.c,v 1.1.1.3 2002/11/19 11:44:44 Yoshizawa1 Exp $
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../okajima/etc/ok_util.h"
#include	"../../shibata/util/ts_util.h"




#define N_PRIMS		(64)
#define	N_VERTS		(32)
#define	N_SPRTS		(2048)

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)

#define PEE_FLAG	(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE)
#define PEE_SHADOW_FLAG	(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_NOBUFFER)
#define LIGHT_FLAG	(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE)

/*--- 放尿スプライトパラメータ ---*/
#define	PEE_TEX		( GV_StrCode( "drop01_msk" ) )
#define COL_R		(128)
#define COL_G		(128)
#define COL_B		(96)
#define ALPHA		(4)
#define SIZE		(20)

#define COL_SHADOW_R	(COL_R/4)
#define COL_SHADOW_G	(COL_G/4)
#define COL_SHADOW_B	(COL_B/4)

/*--- 反射キラキラパラメータ ---*/
#define	LIGHT_TEX	( GV_StrCode( "light05_msk" ) )
#define	LIGHT_ALPHA	(128)
#define	LIGHT_SIZE	(30)


#define	N_NEW			(16)

#define	PEE_INIT_SPEED		(20.0f)
#define PEE_SPEED 		(16.0f)
#define GRAVITY			(1.0f)

#define FLOOR_HEIGHT		(0.0f)




#define	BODYWORLD(_body,_index) (((OBJECT*)_body)->objs->objs[(_index)].world)


/*------------ ターゲット情報 ------------*/
#define	PEE_TARGET_NAME		(137921)

#define PARENT_TARGET_FLAG	(TARGET_ROTATE|TARGET_DEFENSE|TARGET_POWER|TARGET_SEEK|TARGET_CHILD|TARGET_HIT_SAMENAME)
#define	CHILD_TARGET_NUM	(11)
#define CHILD_TRG_FLAG		(TARGET_ROTATE|TARGET_DEFENSE|TARGET_POWER|TARGET_SEEK|TARGET_HIT_SAMENAME)

#define PEE_TRG_FLAG		(TARGET_ONLINE|TARGET_OFFENSE|TARGET_POWER|TARGET_SEEK|TARGET_CHILD|TARGET_HIT_SAMENAME)
#define	PEE_HEIGHT		(3000.0f)

#define	BOX_INDEX		(11)


/*--- 親ターゲットサイズ ---*/
static FVECTOR Target_Size[] = {
	{ 500.0F, 1000.0F, 500.0F },	/* 立ち */
	{ 1000.0F, 200.0F, 1000.0F },	/* ダウン */
	{ 700.0F, 650.0F, 700.0F },	/* しゃがみ */
};

/*--- 各レベルの子ターゲット数 ---*/
static int	Child_Level_Num[] = { 
	1,	// Level 0 のターゲット数
	5,	// Level 1 のターゲット数
	1,	// Level 2 のターゲット数
	4	// Level 3 のターゲット数
};

/*--- 各子ターゲットの関節 ---*/
static int Child_Target_Connect[] = { 
	// level 0
	HUMAN21_ATAMA,				// 頭
	// level 1
	HUMAN21_MIGI_UDE1,			// 右腕１
	HUMAN21_MIGI_UDE2,			// 右腕２
	HUMAN21_HIDARI_UDE1,			// 左腕１
	HUMAN21_HIDARI_UDE2,			// 左腕２
	HUMAN21_MUNE,				// 胸
	// level 2
	HUMAN21_KOSHI,				// 胴体
	// level 3
	HUMAN21_MIGI_ASHI1,			// 右足１
	HUMAN21_MIGI_ASHI2,			// 右足２
	HUMAN21_HIDARI_ASHI1,			// 左足１
	HUMAN21_HIDARI_ASHI2,			// 左足２

} ;

/*--- 各子ターゲットの関節からのオフセット値 ---*/
static FVECTOR	Child_Target_Shift[] = { 
	// level 0
	{ 0.0F, 0.0F, 10.0F },			// 頭
	// level 1
	{ -30.0F, -130.0F, -10.0F },		// 右腕１
	{ 0.0F, -130.0F, 0.0F },		// 右腕２
	{ 30.0F, -130.0F, -10.0F },		// 左腕１
	{ 0.0F, -130.0F, 0.0F },		// 左腕２
	{ 0.0F, 100.0F, 0.0F },			// 胸
	// level 2
	{ 0.0F, 0.0F, 0.0F },			// 胴体
	// level 3
	{ -10.0F, -200.0F, -10.0F },		// 右足１
	{  20.0F, -250.0F, -20.0F },		// 右足２
	{ -10.0F, -200.0F, -10.0F },		// 左足１
	{ -20.0F, -250.0F, -20.0F },		// 左足２

	{ 0.0F, 190.0F, 300.0F },		// ダンボール

} ;

/*--- 各子ターゲットのサイズ ---*/
static FVECTOR	Child_Target_Size[] = { 
	// level 0
	{ 90.0F, 120.0F, 90.0F },		// 頭
	// level 1
	{ 45.0F, 150.0F, 65.0F },		// 右腕１
	{ 75.0F, 150.0F, 75.0F },		// 右腕２
	{ 40.0F, 150.0F, 65.0F },		// 左腕１
	{ 75.0F, 150.0F, 75.0F },		// 左腕２
	{ 150.0F, 120.0F, 100.0F },		// 胸
	// level 2
	{ 120.0F, 160.0F, 80.0F },		// 胴体
	// level 3
	{ 65.0F, 200.0F, 90.0F },		// 右足１
	{ 60.0F, 260.0F, 60.0F },		// 右足２
	{ 65.0F, 200.0F, 90.0F },		// 左足１
	{ 60.0F, 260.0F, 60.0F },		// 左足２

	{ 350.0F, 400.0F, 450.0F },		// ダンボール
	
};




/*--- 放尿制御フラグ ---*/
enum {
    PEE_HOLD		= 0x00,
    PEE_SPLASH		= 0x01,
    PEE_HOMING		= 0x02,
    PEE_FINISH		= 0x04,
};



extern void *NewRainbow( FVECTOR *pos );
extern void GM_SeSetEx( int, FVECTOR*, int, int );





typedef	struct	{

    GV_ACT_EX 	actor;

    DG_PRIM2	*prim;
    DG_PRIM2	*prim_shadow;

    DG_PRIM2	*prim_light;

    OBJECT	*body;
    FVECTOR	pre_pos;

    FVECTOR	vec[N_SPRTS];
    int		ref_flag[N_SPRTS];

    int 	id;
    int 	pre_id;

    float	splash_speed;
    SVECTOR	rot;
    FVECTOR	vec_pre;

    FVECTOR	vecbody;
    
    int 	step;

    int		cnt;
    int 	off_flag;

    int		*flag;
    int		pre_flag;


    float	wind;
    float	local_wind;
    int		wind_cnt;

    int		scr_flag;

    /* 当たり判定用 */
    TARGET	target;
    TARGET	def_child[CHILD_TARGET_NUM];

    TARGET		target_pee;
    POWER_TARGET	power;
    FVECTOR		from;
    int			from_flag;
    int			to_flag;

    int			pee_se_cnt;

    FVECTOR		to;

    ALIGN16_PRE DG_PRIM2_UVRGBWH	uvrgbwh_shadow[2][N_SPRTS] ALIGN16_POST;

    
} Work;




/*-------- PeeTargetMove : ターゲット更新関数 --------*/  
static void PeeTargetMove( Work *work )
{

    int		i;
    TARGET	*deftrg;
    TARGET	*child;
    int		*connect;    
    FMATRIX	mat;


    deftrg = &work->target;
    child = work->def_child;
    connect = Child_Target_Connect;


    /* 親ターゲット */
    DG_SetPos2( &GM_PlayerControl->mov, &GM_PlayerControl->rot );
    DG_GetPos( &mat );
    GM_MoveTarget2( deftrg, &mat );



    /* 子供ターゲット */
    for ( i = 0 ; i < CHILD_TARGET_NUM ; i++ ) {
	GM_MoveTarget2( child, &( BODYWORLD( GM_PlayerBody, *connect ) ) );
	child++;
	connect++;
    }

    /* ダンボール特殊処理 */
    if ( GM_CheckPlayerStatus( PLAYER_CB_BOX ) ) {
	DG_COPY_VEC( &work->def_child[6].size, &Child_Target_Size[BOX_INDEX] );
	DG_COPY_VEC( &work->def_child[6].offset, &Child_Target_Shift[BOX_INDEX] );
    }
    /* 通常胴体ターゲット */
    else {
	DG_COPY_VEC( &work->def_child[6].size, &Child_Target_Size[6] );
	DG_COPY_VEC( &work->def_child[6].offset, &Child_Target_Shift[6] );
    }



    /* 放尿ターゲット */
    if ( work->from_flag && work->to_flag ) {
	GM_MoveOnlineTarget( &work->target_pee, &work->from, &work->to );
	//AN_Test_Eye2( &work->from, 500.f );
	//AN_Test_Eye2( &work->to, 500.f );
	GM_PutTarget( &work->target_pee );
    }


}




/* アクト関数 */
static void Act( Work *work )
{

    int			i;
    int	  		clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FVECTOR		*vec;

    DG_PRIM2_UVRGBWH 	*uvrgbwh_shadow;
    
    FMATRIX		mat;
    FVECTOR		fvtmp;
    float		random;

    FVECTOR  		*pos_light;
    DG_PRIM2_UVRGBWH 	*uvrgb_light;
    int 		light_id;

    int			size_cnt;
    int			alpha_cnt;
    int 		cnt;

    FVECTOR		vecbody_now;

    FVECTOR		swing;

    FVECTOR		*hit;
    int			hit_flag;

    FVECTOR		*floor_hit_pos;
    int			floor_hit_flag;

    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    /* グループ更新 */
    GM_GroupPrim2( work->prim, work->body->map_name );
    GM_GroupPrim2( work->prim_shadow, work->body->map_name );

    /* 減算ずらす */
    work->prim_shadow->flag |= DG_PRIM_AS_CUSTOMWORLD;
    DG_COPY_MAT( &work->prim_shadow->as_world, &DG_UnitMatrix );
    work->prim_shadow->as_world.m[3][0] = 10.0f;


    /* 操作する頂点バッファ取得 */
    DG_SwitchBuffPrim2( work->prim_shadow );
    DG_SwitchBuffPrim2( work->prim_light );

    clock = work->prim->buffer_clock;
    
    pos		= work->prim->pos[clock];
    pos_pre	= work->prim->pos[1-clock];
    uvrgbwh	= work->prim->uvrgb[clock];
    uvrgbwh_pre = work->prim->uvrgb[1-clock];
    vec 	= work->vec;
    pos_light   = work->prim_light->pos[work->prim_light->buffer_clock];
    uvrgb_light = work->prim_light->uvrgb[work->prim_light->buffer_clock];
    uvrgbwh_shadow = work->prim_shadow->uvrgb[work->prim_shadow->buffer_clock];



    alpha_cnt = 0;
    light_id = irnd()%N_SPRTS;

    work->from_flag = 0;
    work->to_flag   = 0;


    /* あとで見直すこと！ */
    hit = NULL;
    floor_hit_pos = NULL;


    /* 当たり判定 */
    hit_flag = 0;
    floor_hit_flag = 0;
    for ( i = 0 ; i < CHILD_TARGET_NUM; i++ ) {
	if ( TARGET_POWER & work->def_child[i].damaged ) {
	    hit_flag = 1;
	    hit = &work->target_pee.hit;
	    work->def_child[i].damaged &= ~TARGET_POWER;
	    GM_PlayerStateFlag |= PL_SHOWERED;
	}
    }

    

    /* 放尿スピード操作 */
    if ( ( work->pre_flag == PEE_HOLD ) && ( *work->flag & PEE_SPLASH ) ) {
	work->splash_speed = PEE_INIT_SPEED;
	work->off_flag = 0;
	work->step = 0;
    }    
    work->pre_flag = *work->flag;


    switch ( *work->flag ) {
    case PEE_HOLD:
	work->splash_speed = 0.0f;
	break;
    case PEE_SPLASH:
    case PEE_HOMING:
	if ( work->splash_speed > PEE_SPEED ) {
	    work->splash_speed *= 0.996f;
	}
	break;
    case PEE_FINISH:
	switch ( work->step ) {
	case 0:
	    work->splash_speed *= 0.996f;
	    if ( work->splash_speed <= 8.0f ) {
		work->splash_speed = PEE_SPEED * 0.80f;
		work->step = 1;
	    }
	    break;
	case 1:
	    work->splash_speed *= 0.996f;
	    if ( work->splash_speed <= 9.0f ) {
		work->splash_speed = 0.0f;
		work->step = 2;
	    }
	    break;
	case 2:
	    break;
	}
	break;
    }




    /* 揺れ計算 */
    if ( --work->wind_cnt == 0 ) {
	work->wind = 0.30f * frnd();
	work->wind_cnt = 15 + irnd()%30;
    }

    /* ローカル揺れ算出 */
    work->local_wind = work->local_wind * 0.92f + work->wind * 0.08f;
    _sceVu0ScaleVector( &swing, work->body->objs->objs[0].world.m[0], work->local_wind );

    


    size_cnt = work->cnt % 4;



    
    /* 通常処理 */    
    if ( work->off_flag != 0 ) {	

	/* 放尿してない時はスプライトをインビジブルにして更新処理しない */
	DG_InvisiblePrim2( work->prim );
	DG_InvisiblePrim2( work->prim_shadow );
	DG_InvisiblePrim2( work->prim_light );

    }
    else {	/* 放尿中 */

	DG_VisiblePrim2( work->prim );
	DG_VisiblePrim2( work->prim_shadow );
	DG_VisiblePrim2( work->prim_light );

	/* スプライト更新 */
	for ( i = 0 ; i < N_SPRTS ; i++ ) {

	    /* スプライト位置更新 */
	    _sceVu0AddVector( pos, pos_pre, vec );

	    /* 速度更新 */
	    vec->vx *= 0.99f;
	    vec->vy -= GRAVITY;
	    vec->vz *= 0.99f;

	    /* 揺れ反映 */
	    _sceVu0AddVector( vec, vec, &swing );


	    /* 放尿ターゲット始点 */
	    if ( work->from_flag == 0 ) {
		if ( pos_pre->vy > PEE_HEIGHT && pos->vy <= PEE_HEIGHT ) {
		    DG_COPY_VEC( &work->from, pos );
		    work->from_flag = 1;
		}
	    }

	    /* 体との当たり判定 */
	    if ( uvrgbwh_pre->a > 0 ) {
		if ( hit_flag && work->ref_flag[i] == 0 && pos_pre->vy > hit->vy && pos->vy <= hit->vy ) {
		
		    /* 放尿ターゲット終点 */
		    if ( work->to_flag == 0 ) {
			DG_COPY_VEC( &work->to, pos );
			work->to_flag = 1;
		    }

		    /* 跳ね返り初期化 */
		    vec->vx = frnd()*100.0f;
		    vec->vy = 50.0f + frnd()*40.0f;
		    vec->vz = frnd()*100.0f;

		    DG_COPY_VEC( pos, hit );
		    pos->vy += vec->vy;

		    uvrgbwh->w = uvrgbwh->h = 10 + irnd()%30;
		    uvrgbwh->a = 16+irnd()%8;

		    work->ref_flag[i] = 1;

		}	    
		/* 床との当たり判定 */
		else if ( work->ref_flag[i] == 0 && pos->vy < FLOOR_HEIGHT ) {

		    /* 放尿ターゲット終点 */
		    if ( work->to_flag == 0 ) {
			DG_COPY_VEC( &work->to, pos );
			work->to.vy = 0.0f;
			work->to_flag = 1;
		    }

		    if ( floor_hit_flag == 0 ) {
			floor_hit_pos = pos;
			floor_hit_flag = 1;
		    }

		    /* 跳ね返り初期化 */
		    vec->vx = frnd()*100.0f;
		    vec->vy = 60.0f + frnd()*40.0f;
		    vec->vz = frnd()*100.0f;

		    pos->vy += vec->vy;

		    uvrgbwh->w = uvrgbwh->h = 10 + irnd()%30;
		    uvrgbwh->a = 16+irnd()%8;

		    work->ref_flag[i] = 1;
		    
		}
		/* 通常処理 */
		else {

		    if ( work->ref_flag[i] == 1 ) {
			if ( uvrgbwh_pre->a > 0 ) {
			    uvrgbwh->a = uvrgbwh_pre->a - 1;
			}
			else {
			    uvrgbwh->a = 0;
			}
		    }
		    else {
			uvrgbwh->a = uvrgbwh_pre->a;
		    }

		    if ( size_cnt == 0 ) {
			uvrgbwh->w = uvrgbwh_pre->w + 1;
			uvrgbwh->h = uvrgbwh_pre->h + 1;
		    }
		    else if ( size_cnt == 1 ) {
			uvrgbwh->w = uvrgbwh_pre->w;
			uvrgbwh->h = uvrgbwh_pre->h;
		    }

		}

	    }
	    else {
		uvrgbwh->a = 0;
		alpha_cnt++;
	    }


	    /* キラキラ更新 */
	    if ( i == light_id ) {
		if ( uvrgbwh->a > 0 ) {
		    DG_COPY_VEC( pos_light, pos );
		    uvrgb_light->a = 128;
		}
		else {
		    uvrgb_light->a = 0;
		}
	    }	    



	    /* 疑似バンプマッピング */
	    uvrgbwh_shadow->a = uvrgbwh->a;
	    uvrgbwh_shadow->w = uvrgbwh->w;
	    uvrgbwh_shadow->h = uvrgbwh->h;

	    
	    /* ポインタ更新 */
	    pos++;
	    pos_pre++;
	    uvrgbwh++;
	    uvrgbwh_pre++;
	    vec++;

	    uvrgbwh_shadow++;


	}




	/*------------------------------------------*/
	/* idで指定されたスプライトを初期化し再利用 */
	/*------------------------------------------*/
	pos = work->prim->pos[clock];
	uvrgbwh = work->prim->uvrgb[clock];

	/* 本体マトリクスをランダムに回転させて動きつける */
	work->rot.vx = irnd()%40 - 20;
	work->rot.vy = irnd()%40 - 20;
	work->rot.vz = 0;
	
	DG_SetPos( &work->body->objs->objs[0].world );
	DG_RotatePos( &work->rot );
	DG_GetPos( &mat );

	
	/* 現在の本体速度ベクトル取得 */
	_sceVu0SubVector( &vecbody_now, (FVECTOR *)work->body->objs->objs[0].world.m[3], &work->pre_pos );
	
	cnt = 0;
    
	/* パーティクル初期化 */
	for ( i = 0 ; i < N_NEW ; i++ ) {

	    /* 初期位置 */
	    DG_COPY_VEC( &pos[work->id], &DG_ZeroVector );

	    pos[work->id].vx = frnd()*5.f;
	    pos[work->id].vy = frnd()*5.f;
	    pos[work->id].vz = frnd()*5.f;

	    DG_COPY_VEC( &fvtmp, (FVECTOR *)(work->body->objs->objs[0].world.m[3]) );
	    fvtmp.vy -= 120.f;
	    _sceVu0ScaleVector( &fvtmp, &fvtmp, (float)( N_NEW - i )/(float)N_NEW );
	    _sceVu0AddVector( &pos[work->id], &pos[work->id], &fvtmp );
	    _sceVu0ScaleVector( &fvtmp, &pos[work->pre_id], (float)( i )/ (float)N_NEW );
	    _sceVu0AddVector( &pos[work->id], &pos[work->id], &fvtmp );
	

	    /* 初速度 */
	    DG_COPY_VEC( &work->vec[work->id], &DG_ZeroVector );

	    if ( !( irnd()%3 ) ) {
		random = ( 1.0f * frnd() - 0.5f ) * 0.5f;/**/
	    }
	    else {
		random = 0.f;
	    }
	    _sceVu0ScaleVector( &fvtmp, (FVECTOR *)(mat.m[0]), random );
	    _sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &fvtmp );
	
	    if ( !( irnd()%3 ) ) {
		random = ( 1.0f * frnd() - 0.5f ) - 1.0f * (float)i / (float)N_NEW;
	    }
	    else {
		random = - 1.0f * (float)i / (float)N_NEW;
	    }
	    DG_COPY_VEC( &fvtmp, &DG_ZeroVector );
	    fvtmp.vy = 1.0f;
	    _sceVu0ScaleVector( &fvtmp, &fvtmp, random );
	    _sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &fvtmp );

	    random = (float)i / (float)N_NEW;
	    _sceVu0ScaleVector( &fvtmp, (FVECTOR*)(&mat.m[2]), work->splash_speed * ( 1.0f - random ) );
	    _sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &fvtmp );
	    _sceVu0ScaleVector( &fvtmp, &work->vec_pre, work->splash_speed * random );
	    _sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &fvtmp );

	    /* 本体ベクトル更新 ->> 補間処理入れて滑らかに動かす */
	    work->vecbody.vx = work->vecbody.vx * 0.2f + vecbody_now.vx * 0.8f;
	    work->vecbody.vy = work->vecbody.vy * 0.2f + vecbody_now.vy * 0.8f;
	    work->vecbody.vz = work->vecbody.vz * 0.2f + vecbody_now.vz * 0.8f;

	    /* 本体自体の移動速度プラス */
	    _sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &work->vecbody );
	    
	    /* アルファ初期化 */	
	    if ( work->splash_speed <= 0.0f ) {
		uvrgbwh[work->id].a = 0;
	    }
	    else if ( work->splash_speed < 13.f ) {
		if ( !( irnd()%( (int)( 15.f - work->splash_speed ) ) ) ) {
		    uvrgbwh[work->id].a = ALPHA;
		    cnt++;
		}
		else {
		    uvrgbwh[work->id].a = 0;
		}	    		
	    }
	    else {
		uvrgbwh[work->id].a = ALPHA;
		cnt++;
	    }	    
	    uvrgbwh[work->id].w = SIZE;
	    uvrgbwh[work->id].h = SIZE;

	    work->ref_flag[work->id] = 0;

	    /* ID更新 */
	    work->id++;

	}

	if ( work->id >= N_SPRTS ) work->id = 0;


	/* 速度ベクトル確保 */
	work->pre_id = work->id - N_NEW;
	if ( work->pre_id < 0 ) work->pre_id += N_SPRTS;
	DG_COPY_VEC( &work->vec_pre, (FVECTOR *)(mat.m[2]) );


	/* 終了条件 */
	if ( ( ( *work->flag == PEE_FINISH ) || ( *work->flag == PEE_HOLD ) ) && alpha_cnt >= N_SPRTS ) {
	    work->off_flag = 1;
	    *work->flag = PEE_HOLD;
	}



	/* SE呼び出し */
	if ( hit_flag ) {

	    /* 振動呼びだし */
	    if ( (work->cnt%5) ) {
		GM_SetVibration2( 0, 127 );
	    }
	    
	    if ( work->pee_se_cnt == 0 ) {
		if ( GM_CheckPlayerStatus( PLAYER_CB_BOX ) ) {
		    GM_SeSetMode( SD_A_SYONDM01, hit, GM_SEMODE_NORMAL );
		}
		else {
		    GM_SeSetMode( SD_A_SYONBE01, hit, GM_SEMODE_NORMAL );
		}
		work->pee_se_cnt = DIRECT_TICK( 6 + irnd()%9 );
	    }
	    else {
		work->pee_se_cnt--;
	    }
	}
	else if ( floor_hit_flag ) {
	    if ( work->pee_se_cnt == 0 ) {
		GM_SeSetMode( SD_A_SYONMT01, floor_hit_pos, GM_SEMODE_NORMAL );
		work->pee_se_cnt = DIRECT_TICK( 6 + irnd()%9 );
	    }
	    else {
		work->pee_se_cnt--;
	    }
	}
	else {
	    work->pee_se_cnt = 0;
	}


	/* モデル水滴呼び出し
	   条件 : 2ｍ以内でしゃがみ、匍匐、エルード中の主観 */
	if ( ( GM_PlayerStatus & ( PLAYER_BEYOND | PLAYER_SQUAT | PLAYER_GROUND ) )
	     && ( GM_PlayerStatus & PLAYER_WATCH )
	     && ( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ) ) {

	    if ( work->scr_flag == 0 ) {

		if ( floor_hit_pos != NULL ) {

		    _sceVu0SubVector( &fvtmp, &GM_PlayerPosition, floor_hit_pos );

		    if ( fvtmp.vx * fvtmp.vx + fvtmp.vz * fvtmp.vz < 2000.0f * 2000.0f ) {	    
			extern void *NewScrDrop( int life );
			NewScrDrop( 120 );
			work->scr_flag = 1;
		    }

		}

	    }
	    else if ( !(work->cnt%3) ) {
		if ( floor_hit_pos != NULL ) {

		    _sceVu0SubVector( &fvtmp, &GM_PlayerPosition, floor_hit_pos );

		    if ( fvtmp.vx * fvtmp.vx + fvtmp.vz * fvtmp.vz < 1000.0f * 1000.0f ) {	    
			extern void *NewScrDrop( int life );
			NewScrDrop( 120 );
			work->scr_flag = 1;
		    }
		}
	    }

	}
	else {
	    if ( work->scr_flag == 1 ) {
		work->scr_flag = 0;
	    }
	}

	    

	/* ターゲット更新 */
	PeeTargetMove(work);


    }

    DG_COPY_VEC( &work->pre_pos, (FVECTOR *)work->body->objs->objs[0].world.m[3] );
    
    work->cnt++;
    
}



static void Die( Work *work )
{

    if ( work->prim ) work->prim        = OK_FreePrim2( work->prim );
    if ( work->prim_shadow ) work->prim_shadow = OK_FreePrim2( work->prim_shadow );
    if ( work->prim_light ) work->prim_light  = OK_FreePrim2( work->prim_light );
    GM_FreeTarget( &work->target );
    GM_FreeTarget( &work->target_pee );

}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work )
{

	int			i;
	DG_PRIM2		*prim;
	DG_PRIM2		*prim_shadow;
	DG_PRIM2		*prim_light;

	DG_TEX			*tex;

	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	DG_PRIM2_UVRGBWH	*uvrgbwh_shadow;



	prim        = work->prim;
	prim_shadow = work->prim_shadow;
	prim_light  = work->prim_light;



	tex = DG_GetTexture( PEE_TEX );

	prim->raise = 0;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	
	DG_SetPrim2Buffer( prim_shadow,
			   prim->pos[0], prim->pos[1],
			   (void *)work->uvrgbwh_shadow[0], (void *)work->uvrgbwh_shadow[1] );
	DG_ConfigPrim2Tex( prim_shadow, tex );
	DG_SetPrim2Alpha( prim_shadow, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );



	pos       = prim->pos[0];
	uvrgbwh   = prim->uvrgb[0];
	vec	  = work->vec;

	uvrgbwh_shadow = work->uvrgbwh_shadow[0];

	for ( i = 0 ; i < N_SPRTS ; i++ ) {

	    DG_COPY_VEC( pos, &DG_ZeroVector );

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    uvrgbwh->r  = COL_R;
	    uvrgbwh->g  = COL_G;
	    uvrgbwh->b  = COL_B;
	    uvrgbwh->a  = 0;
	    
	    uvrgbwh->w  = SIZE;
	    uvrgbwh->h  = SIZE;

	    DG_COPY_VEC( vec, &DG_ZeroVector );

	    work->ref_flag[i] = 0;


	    uvrgbwh_shadow->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh_shadow->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh_shadow->q0 = 4096;
	    uvrgbwh_shadow->f0 = 0x0fff;

	    uvrgbwh_shadow->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh_shadow->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh_shadow->q1 = 4096;
	    uvrgbwh_shadow->f1 = 0x0fff;

	    uvrgbwh_shadow->r  = COL_SHADOW_R;
	    uvrgbwh_shadow->g  = COL_SHADOW_G;
	    uvrgbwh_shadow->b  = COL_SHADOW_B;
	    uvrgbwh_shadow->a  = 0;
	    
	    uvrgbwh_shadow->w  = SIZE;
	    uvrgbwh_shadow->h  = SIZE;


	    pos++;
	    uvrgbwh++;
	    vec++;
	    uvrgbwh_shadow++;

	}


	OK_Scr_Mem( prim->pos[ 1 ],   prim->pos[ 0 ],   sizeof(FVECTOR),          N_SPRTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], prim->uvrgb[ 0 ], sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );

	OK_Scr_Mem( work->uvrgbwh_shadow[1], work->uvrgbwh_shadow[0], sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	prim_shadow->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );





	/* 反射キラキラ初期化 */
	prim_light->raise = 0;

	tex = DG_GetTexture( LIGHT_TEX );

	DG_ConfigPrim2Tex( work->prim_light, tex );
	DG_SetPrim2Alpha( work->prim_light, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos     = work->prim_light->pos[ 0 ];
	uvrgbwh = work->prim_light->uvrgb[ 0 ];

	DG_COPY_VEC( pos, &DG_ZeroVector );
	
	uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	uvrgbwh->q0 = 4096;
	uvrgbwh->f0 = 0x0fff;
	
	uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	uvrgbwh->q1 = 4096;
	uvrgbwh->f1 = 0x0fff;
	
	uvrgbwh->r  = COL_R;
	uvrgbwh->g  = COL_G;
	uvrgbwh->b  = COL_B;
	uvrgbwh->a  = LIGHT_ALPHA;
	
	uvrgbwh->w  = LIGHT_SIZE;
	uvrgbwh->h  = LIGHT_SIZE;

	OK_Scr_Mem( prim_light->pos[ 1 ],   prim_light->pos[ 0 ],   sizeof(FVECTOR),          1 );
	OK_Scr_Mem( prim_light->uvrgb[ 1 ], prim_light->uvrgb[ 0 ], sizeof(DG_PRIM2_UVRGBWH), 1 );

	prim_light->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;


}



/*-------- InitTarget : ターゲット初期化関数 --------*/  
static void InitTarget( Work *work ) 
{

    int			i,j;
    TARGET		*deftrg;
    TARGET		*child;
    TARGET		*level_child;
    TARGET		*pee;

    int			index;
    static FVECTOR	power = { 0.0f, -1.0f, 0.0f, 1.0f };



    /* 親ターゲット設定 */
    deftrg = &work->target;    
    
    GM_SetTarget( deftrg, PARENT_TARGET_FLAG,
		  0, ENEMY_SIDE, Target_Size, &DG_ZeroVector );
    GM_SetTargetWeaponType( deftrg, 0 );
    GM_SetTargetName( deftrg, PEE_TARGET_NAME );
    GM_PutTarget( deftrg );    
    //NewTargetView( deftrg, 255, 255, 0 );


    /* 子ターゲット設定 */
    child = work->def_child;
    level_child = work->def_child;
    index = 0;

    for ( j = 0 ; j < 4 ; j++ ) {
	level_child = child;
	for ( i = 0 ; i < Child_Level_Num[j] ; i++ ) {
    
	    GM_SetTarget( child, CHILD_TRG_FLAG, 0, ENEMY_SIDE,
			  &Child_Target_Size[index], &Child_Target_Shift[index] );
	    GM_SetTargetWeaponType( child, 0 );
	    GM_SetTargetName( child, PEE_TARGET_NAME );
	    //NewTargetView( child, 255, 255, 0 );
	    child++;
	    index++;

	}

	GM_SetTargetParts( deftrg, level_child, i, j );
    }



    /* 放尿ターゲット設定 */
    pee = &work->target_pee;
    GM_SetTarget( pee, PEE_TRG_FLAG, 0, ENEMY_SIDE,
		  &DG_ZeroVector, &DG_ZeroVector );

    GM_SetTargetWeaponType( pee, WP_NONE );
    GM_SetPowerTarget( pee, &work->power, POWER_CONST, 255, 0, 0, &power );
    GM_SetTargetName( pee, PEE_TARGET_NAME );

    GM_MoveOnlineTarget( pee, &DG_ZeroVector, &DG_ZeroVector );


}




static int GetResources( Work *work, OBJECT *body, FVECTOR *target_pos, int *flag )
{
	
    work->body = body;
    work->flag = flag;
    
    work->id           = 0;
    work->pre_id       = 0;
    work->splash_speed = 0.0f;
    work->step         = 0;
    work->cnt 	       = 0;
    work->off_flag     = 1;

    work->pre_flag = PEE_HOLD;

    work->rot.vx = 0;
    work->rot.vz = 0;
    work->rot.vy = 0;
    DG_COPY_VEC( &work->vec_pre, &DG_ZeroVector );

    work->local_wind = 0.0f;
    work->wind = 0.0f;
    work->wind_cnt = 30;

    work->pee_se_cnt  = 0;

    work->scr_flag = 0;
    
    work->prim        = GM_MakePrim2( PEE_FLAG,        N_PRIMS, N_VERTS );
	if ( work->prim == NULL ) {
		return -1;
	}
    work->prim_shadow = GM_MakePrim2( PEE_SHADOW_FLAG, N_PRIMS, N_VERTS );
	if ( work->prim_shadow == NULL ) {
		return -1;
	}
    work->prim_light  = GM_MakePrim2( LIGHT_FLAG,            1,       1 );
	if ( work->prim_light == NULL ) {
		return -1;
	}


    /* スプライト初期化 */
    InitPacket( work );    

    /* ターゲット初期化 */
    InitTarget( work );
    
    return 0;

}


/*-------- void *NewPeeShower( OBJECT *body, FVECTOR *target_pos, int *flag ) ----------
                             屋外放尿(w25c)呼び出し関数

     OBJECT *body        : 放尿敵兵
     FVECTOR *target_pos : ホーミングするターゲットの座標 (#ホーミングがなくなったので未使用)
     int     *flag       : 放尿制御フラグ
--------------------------------------------------------------------------------------*/
void *NewPeeShower( OBJECT *body, FVECTOR *target_pos, int *flag )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, body, target_pos, flag ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


