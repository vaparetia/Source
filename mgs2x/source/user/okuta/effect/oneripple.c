//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    oneripple.c
    固定生成バンプ波紋
    2001/08/08 Masafumi Okuta
    $Id: oneripple.c,v 1.1.1.3 2002/11/19 11:47:52 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../../okajima/etc/ok_util.h"
#include	"../../okajima/effect2/bubble.h"
#include	"../../mode/demo/eft_con.h"
#include	"../conv/algfunc.h"
#include	"../conv/maoutil.h"

extern int BP_AdjustTick(int);
extern int BP_AdjustTick2(int);
extern float BP_AdjustTick3(float);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))
#define STEP_VMODEF(_a) (BP_AdjustTick3(_a))

#define	SET_FLAG( a,b )		(a) |= (b)
#define	UNSET_FLAG( a,b )	(a) &= ~(b)
#define	CLEAR_FLAG( a ) 	(a) = 0
#define	BODYWORLD(a,b) 		(a)->objs->objs[(b)].world
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

/*----------------------------------------------------------------*/

extern float GM_WaterLevel; // 水面の高さ
extern float _TS_Sin( int s );

/*----------------------------------------------------------------*/
#define	RAISE_SUB	(-3000)
#define	RAISE_ADD	( 3000)

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS_ADD		(SCRPAD_ADDR)
#define	SCR_POS_SUB		(SCRPAD_ADDR + 0x1000)
#define	SCR_UVR		    	(SCRPAD_ADDR + 0x2000)

#define	N_PRIMS		(1)
#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_LOOPS		(1)

#define	COL_R		(96)
#define	COL_G		(96)
#define	COL_B		(96)
#define	MAX_ALPHA	(32.0f)	/* 寿命に影響 */

#define	MINIMUM_SIZE	(10.0f)

#define	DECAY_VEC		(1.00f)
#define	VEC_RATIO_TOP	(0.10f)
#define	VEC_RATIO_DOWN	(0.06f)

#define	INITIAL_SCALE	(6.0f)

typedef	struct	{
    // 基本システム
    GV_ACT_EX	actor;

    // 拡張システム
    FVECTOR	vecPos;				// 発生位置
    float	fRadius;			// 発生半径
    int		nTime;				// 時間
    int		nIntvMin;			// 発生間隔（最小）
    int		nIntvMax;			// 発生間隔（最大）
    float	fSizeMin;			// サイズ（最小）
    float	fSizeMax;			// サイズ（最大）

    u_short	nInterval;			// 発生間隔
    u_short	nAddNum;			// 同時発生数	
    int		act_clock;
    int		activate_num;

    CONTROL*	control;			// 付随するキャラのコントロール
    
    FVECTOR	pre_pos[2];			// 履歴
    FVECTOR	vec[N_PRIMS*N_LOOPS*N_VERTS];

    // 描画系
    DG_PRIM2*	primAdd ;
    DG_PRIM2*	primSub ;
} Work ;

/*----------------------------------------------------------------*/
static void InitVectors( Work*		work,
			 FVECTOR*	center,
			 FVECTOR*	force,	// XZ平面投影後の方向と大きさを使用
			 float 		fRadius,
			 int 		clock)
{
    DG_PRIM2*		primAdd;
    DG_PRIM2*		primSub;
    FVECTOR*		pos_add;
    FVECTOR*		pos_sub;
    FVECTOR*		vec;
    DG_PRIM2_UVRGB*	uvrgb_add;
    DG_PRIM2_UVRGB*	uvrgb_sub;
    FVECTOR		local_force;
    FVECTOR		fvtemp0;
    FVECTOR		fvtemp1;
    FVECTOR		fvtemp2;
    FVECTOR		fvtemp3;
    FVECTOR		calc_vec;
    int			num;
    int			alpha;
    float		param;

    
    DG_COPY_VEC( &calc_vec, center );
    
    // 水面判定
    _sceVu0CopyVector( &local_force, force);

    // 半径チェック
    if( fRadius > work->fSizeMax ){
	fRadius = work->fSizeMax;
    }

    primAdd = work->primAdd;
    primSub = work->primSub;

    /* ＸＺ平面投影 */
    param = frnd();
    fvtemp0.vx = local_force.vx  - param*local_force.vz;
    fvtemp0.vy = 0.0f;
    fvtemp0.vz = local_force.vz  + param*local_force.vx;
    fvtemp0.vw = 1.0f;
    _sceVu0Normalize( &fvtemp0, &fvtemp0 );
    _sceVu0ScaleVector( &fvtemp0, &fvtemp0, fRadius*0.75f );

    /* 直交 */
    fvtemp1.vx =-fvtemp0.vz * 0.5f;
    fvtemp1.vy = 0.0f;
    fvtemp1.vz = fvtemp0.vx * 0.5f;

    fvtemp2.vx = fvtemp0.vx + fvtemp1.vx;
    fvtemp2.vy = 0.0f;
    fvtemp2.vz = fvtemp0.vz + fvtemp1.vz;

    fvtemp3.vx = fvtemp0.vx - fvtemp1.vx;
    fvtemp3.vy = 0.0f;
    fvtemp3.vz = fvtemp0.vz - fvtemp1.vz;

    _sceVu0ScaleVector( &fvtemp1, &fvtemp1, 0.25f );

    vec          = work->vec;
    pos_add      = primAdd->pos[clock];
    pos_sub      = primSub->pos[clock];
    uvrgb_add    = primAdd->uvrgb[clock];
    uvrgb_sub    = primSub->uvrgb[clock];
    num = work->activate_num*4;
    work->activate_num++;
    work->activate_num = (work->activate_num < N_LOOPS*N_PRIMS*N_POLYS)? work->activate_num: 0;
    vec         += num;
    pos_add     += num;
    pos_sub     += num;
    uvrgb_add   += num;
    uvrgb_sub   += num;

    param = 1.0f;
    _sceVu0ScaleVector( vec++, &fvtemp2, VEC_RATIO_DOWN );
    _sceVu0ScaleVector( vec++, &fvtemp2, VEC_RATIO_TOP );
    _sceVu0ScaleVector( vec++, &fvtemp3, VEC_RATIO_DOWN );
    _sceVu0ScaleVector( vec++, &fvtemp3, VEC_RATIO_TOP );

    // α値設定
    alpha = (int)(MAX_ALPHA * (0.5f + fRadius * 0.5f / work->fSizeMax) );

    (uvrgb_add++)->a = alpha;
    (uvrgb_add++)->a = alpha;
    (uvrgb_add++)->a = alpha;
    (uvrgb_add++)->a = alpha;

    (uvrgb_sub++)->a = alpha;
    (uvrgb_sub++)->a = alpha;
    (uvrgb_sub++)->a = alpha;
    (uvrgb_sub++)->a = alpha;

    fvtemp0.vx = calc_vec.vx;
    fvtemp0.vy = GM_WaterLevel;
    fvtemp0.vz = calc_vec.vz;

    // 頂点位置設定
    _sceVu0AddVector( pos_add, &fvtemp0, &fvtemp1 ) ;
    _sceVu0AddVector( pos_add, pos_add, &fvtemp2 ) ;
    DG_COPY_VEC( pos_sub++, pos_add++ ) ;

    _sceVu0AddVector( pos_add, &fvtemp0, &fvtemp1 ) ;
    _sceVu0AddVector( pos_add, pos_add, &fvtemp2 ) ;
    DG_COPY_VEC( pos_sub++, pos_add++ ) ;

    _sceVu0SubVector( pos_add, &fvtemp0, &fvtemp1 ) ;
    _sceVu0AddVector( pos_add, pos_add, &fvtemp3 ) ;
    DG_COPY_VEC( pos_sub++, pos_add++ ) ;

    _sceVu0SubVector( pos_add, &fvtemp0, &fvtemp1 ) ;
    _sceVu0AddVector( pos_add, pos_add, &fvtemp3 ) ;
    DG_COPY_VEC( pos_sub++, pos_add++ ) ;
}

// 動作関数
static	void	Act( Work *work )
{
    DG_PRIM2*		primAdd;
    DG_PRIM2*		primSub;
    FVECTOR*		pos_add;
    FVECTOR*		pos_sub;
    FVECTOR*		vec;
    FVECTOR*		dest_pos_add;
    FVECTOR*		dest_pos_sub;
    FVECTOR*		dest_pos_before;
    FVECTOR		fvtemp0;
    DG_PRIM2_UVRGB*	uvrgb;
    DG_PRIM2_UVRGB*	dest_uvrgb;
    DG_PRIM2_UVRGB*	dest_uvrgb_sub;
    DG_PRIM2_UVRGB*	dest_uvrgb_before;

    int		i, j ;
    int		count;
    int		clock;
    float	angle;

    primAdd = work->primAdd;
    primSub = work->primSub;

    // 所属マップ更新
    GM_GroupPrim2( primAdd, GM_CurrentStageMap ) ;
    GM_GroupPrim2( primSub, GM_CurrentStageMap ) ;
    
    // 可視化
    DG_VisiblePrim2( primAdd ) ;
    DG_VisiblePrim2( primSub ) ;

    // バッファ切替え
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( primAdd ) )
    {
       return;
    }
    DG_SwitchBuffPrim2( primSub );
    clock = primAdd->buffer_clock;

    count = work->act_clock;
	
    vec               = work->vec;
    dest_pos_add      = primAdd->pos[    clock];
    dest_pos_sub      = primSub->pos[    clock];
    dest_pos_before   = primAdd->pos[  1-clock];

    dest_uvrgb        = primAdd->uvrgb[  clock];
    dest_uvrgb_sub    = primSub->uvrgb[  clock];
    dest_uvrgb_before = primAdd->uvrgb[1-clock];
    
    for ( i = 0 ; i < N_LOOPS ; i++ ){
	Mao_CopyMemToScr( dest_pos_before,   SCR_POS_ADD, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	Mao_CopyMemToScr( dest_uvrgb_before, SCR_UVR,  	  sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	pos_add = SCR_POS_ADD;
	pos_sub = SCR_POS_SUB;
	uvrgb   = SCR_UVR;
	for ( j = 0 ; j < N_PRIMS*N_POLYS ; j++ ){
	    {
		// α値更新
		if ( uvrgb->a > 0 ){
		    if( (count&1)==0 ){
			(uvrgb++)->a--;
			(uvrgb++)->a--;
			(uvrgb++)->a--;
			(uvrgb++)->a--;
		    }else{
			uvrgb  += 4;
		    }
		    
		    // 拡大
		    _sceVu0ScaleVector( &fvtemp0, vec, 2.0f );
		
		    pos_add->vx += vec->vx;
		    pos_add->vy  = GM_WaterLevel;
		    pos_add->vz += vec->vz;
		    _sceVu0AddVector( pos_sub++, pos_add++, &fvtemp0 ) ;
		    vec++;

		    pos_add->vx += vec->vx;
		    pos_add->vy  = GM_WaterLevel;
		    pos_add->vz += vec->vz;
		    _sceVu0AddVector( pos_sub++, pos_add++, &fvtemp0 ) ;
		    vec++;

		    pos_add->vx += vec->vx;
		    pos_add->vy  = GM_WaterLevel;
		    pos_add->vz += vec->vz;
		    _sceVu0AddVector( pos_sub++, pos_add++, &fvtemp0 ) ;
		    vec++;

		    pos_add->vx += vec->vx;
		    pos_add->vy  = GM_WaterLevel;
		    pos_add->vz += vec->vz;
		    _sceVu0AddVector( pos_sub++, pos_add++, &fvtemp0 ) ;
		    vec++;
		}else{
		    // 座標をすべて同じにする
#ifdef BP_PSX2_ASM // yano

		    asm volatile ("
		    lqc2		vf11,0x00(%0)
		    sqc2		vf11,0x10(%0)
    		sqc2		vf11,0x20(%0)
		    sqc2		vf11,0x30(%0)
		    sqc2		vf11,0x00(%1)
		    sqc2		vf11,0x10(%1)
		    sqc2		vf11,0x20(%1)
		    sqc2		vf11,0x30(%1)
		    "::"r"(pos_add),"r"(pos_sub):"memory"
		    );
#else
			_sceVu0CopyVector( pos_add + 1, pos_add );
			_sceVu0CopyVector( pos_add + 2, pos_add );
			_sceVu0CopyVector( pos_add + 3, pos_add );
			_sceVu0CopyVector( pos_sub    , pos_add );
			_sceVu0CopyVector( pos_sub + 1, pos_add );
			_sceVu0CopyVector( pos_sub + 2, pos_add );
			_sceVu0CopyVector( pos_sub + 3, pos_add );
#endif
		    pos_add+= 4;
		    pos_sub+= 4;
		    uvrgb  += 4;
		    vec    += 4;
		}
	    }
	    count++;
	}
	// スクラッチパッド->メインメモリへ
	Mao_CopyScrToMem( dest_pos_add,   SCR_POS_ADD, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	Mao_CopyScrToMem( dest_pos_sub,   SCR_POS_SUB, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	Mao_CopyScrToMem( dest_uvrgb,     SCR_UVR,     sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	Mao_CopyScrToMem( dest_uvrgb_sub, SCR_UVR,     sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	dest_pos_add      += N_PRIMS*N_VERTS;
	dest_pos_sub      += N_PRIMS*N_VERTS;
	dest_pos_before   += N_PRIMS*N_VERTS;
	dest_uvrgb        += N_PRIMS*N_VERTS;
	dest_uvrgb_sub    += N_PRIMS*N_VERTS;
	dest_uvrgb_before += N_PRIMS*N_VERTS;
    }

    {
	FVECTOR vecPos;
	int	nAng = BP_PS2_rand() & 4095;
	float   fRad = rnd() * work->fRadius;

	vecPos.vx = work->vecPos.vx + _TS_Sin( nAng) * fRad;
	vecPos.vy = GM_WaterLevel;
	vecPos.vz = work->vecPos.vz + _TS_Sin( nAng + 1024) * fRad;
	vecPos.vw = 1.f;

	DG_COPY_VEC( &work->pre_pos[0], &vecPos ) ;
    }

    pos_add = &work->pre_pos[0];
    pos_sub = &work->pre_pos[1];

    // 時間
    work->act_clock++;
    if ( work->act_clock < work->nTime ){
	// 再利用
	for( i=0; i<work->nAddNum; i++ ){
	    _sceVu0SubVector( &fvtemp0, pos_add, pos_sub ) ;
	    DG_COPY_VEC( pos_sub, pos_add ) ;

	    if ( work->nInterval == 0 ){
		{
                    angle = rnd()*TPI;
		    fvtemp0.vx = sinf(angle);
		    fvtemp0.vy = 0;
		    fvtemp0.vz = cosf(angle);
		    InitVectors( work, pos_add, &fvtemp0, rnd()*work->fSizeMin*0.25f + work->fSizeMin*0.75f, clock );
		}
	    }
	}
    }else{
	if ( work->act_clock >= work->nTime + COUNT_VMODE(120) ){
	    GV_DestroyActor( work);
	}
    }

    // 発生間隔
    if ( work->nInterval == 0 ){
	work->nInterval = COUNT_VMODE(MAO_GetRandom( work->nIntvMin, work->nIntvMax));
    }else{
	work->nInterval--;
    }
//MAO_DbgDumpVector( &work->vecPos);
//AN_Test_Eye2( &work->vecPos, 1);
}

// 破棄関数
static void Die( Work *work )
{
    work->primAdd = OK_FreePrim2( work->primAdd );
    work->primSub = OK_FreePrim2( work->primSub );
}

/*----------------------------------------------------------------*/
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int add_sub )
{
    int			i, j ;
    FVECTOR*		pos;
    FVECTOR*		dest_pos0;
    FVECTOR*		dest_pos1;
    DG_PRIM2_UVRGB*	uvrgb;
    DG_PRIM2_UVRGB*	dest_uvrgb0;
    DG_PRIM2_UVRGB*	dest_uvrgb1;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );

    // αモード設定
    if( add_sub ){
	prim->raise = RAISE_SUB; // 減算
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
    }else{
	prim->raise = RAISE_ADD; // 加算
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
    }

    prim->buffer_clock = 0;

    dest_pos0   = prim->pos[ 0 ];
    dest_pos1   = prim->pos[ 1 ];
    dest_uvrgb0 = prim->uvrgb[ 0 ];
    dest_uvrgb1 = prim->uvrgb[ 1 ];
    for ( i = 0 ; i < N_LOOPS ; i++ ){
	pos   = SCR_POS_ADD;
	uvrgb = SCR_UVR;
	for ( j = 0 ; j < N_PRIMS*N_POLYS ; j++ ){
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;

	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096 ;
	    uvrgb->f = 0x8fff ;
	    uvrgb->r = COL_R ;
	    uvrgb->g = COL_G ;
	    uvrgb->b = COL_B ;
	    uvrgb->a = 0 ;
	    uvrgb++;

	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096 ;
	    uvrgb->f = 0x8fff ;
	    uvrgb->r = COL_R ;
	    uvrgb->g = COL_G ;
	    uvrgb->b = COL_B ;
	    uvrgb->a = 0 ;
	    uvrgb++;

	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096 ;
	    uvrgb->f = 0x0fff ;
	    uvrgb->r = COL_R ;
	    uvrgb->g = COL_G ;
	    uvrgb->b = COL_B ;
	    uvrgb->a = 0 ;
	    uvrgb++;

	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096 ;
	    uvrgb->f = 0x0fff ;
	    uvrgb->r = COL_R ;
	    uvrgb->g = COL_G ;
	    uvrgb->b = COL_B ;
	    uvrgb->a = 0 ;
	    uvrgb++;
	}
	// スクラッチパッドからメインメモリへ転送
	Mao_CopyScrToMem( dest_pos0,   SCR_POS_ADD, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	Mao_CopyScrToMem( dest_pos1,   SCR_POS_ADD, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	Mao_CopyScrToMem( dest_uvrgb0, SCR_UVR,     sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	Mao_CopyScrToMem( dest_uvrgb1, SCR_UVR,     sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	
	dest_pos0  += N_PRIMS*N_VERTS;
	dest_pos1  += N_PRIMS*N_VERTS;
	dest_uvrgb0+= N_PRIMS*N_VERTS;
	dest_uvrgb1+= N_PRIMS*N_VERTS;
    }
}

// リソース初期化
static int GetResources( Work *work )
{
    DG_PRIM2*	prim;
    DG_TEX*	tex;

    // テクスチャ取得
    tex = DG_GetTexture( 12895813 /*"ripple02_alp"*/ );

    // 加算プリミティブ作成
    prim = work->primAdd = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 
					  N_PRIMS*N_LOOPS, N_VERTS );
    if( prim==NULL ){
	printf("null prim\n");
	return -1;
    }
    InitPacket( work, prim, tex, 0 );

    // 減算プリミティブ作成
    prim = work->primSub = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 
					  N_PRIMS*N_LOOPS, N_VERTS );
    if(prim==NULL){
	printf("null prim\n");
	return -1;
    }
    InitPacket( work, prim, tex, 1 );

    work->act_clock = 0;
    work->activate_num = 0;
    work->nInterval = COUNT_VMODE(MAO_GetRandom( work->nIntvMin, work->nIntvMax));

    return 0 ;
}

// 固定生成波紋
void* NewOneRipple( FVECTOR*	pvecPos,	// 中心位置
		    float	fRadius,	// 半径
		    int		nTime,		// 時間
		    int		nAddNum,	// 同時生成数	
		    int		nIntvMin,	// 波紋発生間隔(最小)	
		    int		nIntvMax,	// 波紋発生間隔(最大)	
		    float	fSizeMin,	// 波紋サイズ(最小)	
		    float	fSizeMax)	// 波紋サイズ(最大)	
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor );

	// 引数取得
	_sceVu0CopyVector( &work->vecPos, pvecPos);
        work->vecPos.vy = GM_WaterLevel;
        work->fRadius	= fRadius;
        work->nTime	= COUNT_VMODE(nTime);
	work->nAddNum   = nAddNum;
	work->nIntvMin  = nIntvMin;
	work->nIntvMax  = nIntvMax;
	work->fSizeMin  = fSizeMin;
	work->fSizeMax  = fSizeMax;

	if ( GetResources( work ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}


