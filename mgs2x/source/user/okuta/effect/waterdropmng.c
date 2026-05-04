//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    waterdropmng.c
    水滴:ピチョン君マネージャ
    2001/07/27 Masafumi Okuta
    $Id: waterdropmng.c,v 1.1.1.3 2002/11/19 11:47:54 Yoshizawa1 Exp $
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
#include <string.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include	"../conv/algfunc.h"
#include	"../conv/maoutil.h"
#include	"waterdrop.h"


// プレイヤーの所属しているバウンドのインデックスを取得
static int GetPlayerBoundIndex( WATERDROP_MNG* work)
{
    int i;

    for ( i =0 ; i < work->nBoundNum; i++){
	if ( MAO_BoundCheck( &GM_PlayerPosition, &work->vecMinTbl[i], &work->vecMaxTbl[i]) ){
	    return (i); // 入っていた
	}
    }

    return (-1); // どこにも入っていない
}

// 動作関数
static void Act( WATERDROP_MNG* work )
{
    int i, j;
    // プレイヤーの属しているバウンドのインデックス取得
    work->nPlayerIndex = GetPlayerBoundIndex( work);
    if ( GM_GetDGGroupID(work->map) & GM_GetDGGroupID(GM_CurrentStageMap)){
	work->primAdd->flag &= ~DG_PRIM2_INVISIBLE;
	work->primSub->flag &= ~DG_PRIM2_INVISIBLE;
    }else{
	work->primAdd->flag |= DG_PRIM2_INVISIBLE;
	work->primSub->flag |= DG_PRIM2_INVISIBLE;
    }

    GM_GroupPrim2( work->primAdd, work->map );
    GM_GroupPrim2( work->primSub, work->map );

    // 生成
    for ( i = 0; i < work->nBoundNum; i++){
	if ( i == work->nPlayerIndex ) continue;

	work->nCntr[i]++;
	if ( work->nCntr[i] >= work->nTime[i]){
	    FVECTOR vecSize;
	    FVECTOR vecPos;

	    _sceVu0SubVector( &vecSize, &work->vecMaxTbl[i], &work->vecMinTbl[i]);
	    vecPos.vx = work->vecMinTbl[i].vx + rnd() * vecSize.vx;
	    vecPos.vy = work->vecMinTbl[i].vy + rnd() * vecSize.vy;
	    vecPos.vz = work->vecMinTbl[i].vz + rnd() * vecSize.vz;
	    vecPos.vw = 1.f;
	    {	
		extern void* NewPipeWaterDrop( WATERDROP_MNG*, FVECTOR* );	
		GV_SetActorChild( work, NewPipeWaterDrop( work, &vecPos));
	    }

	    work->nCntr[i] = 0;
	    work->nTime[i] = MAO_GetRandom( work->nIntvMin[i], work->nIntvMax[i]);
	}
    }

    // 描画更新
    {
	FVECTOR* 		pvecPrimAdd;
	FVECTOR* 		pvecPrimSub;
	FVECTOR** 		ppvecPos;
	DG_PRIM2_UVRGBWH*	uvrgbwhAdd;
	DG_PRIM2_UVRGBWH*	uvrgbwhSub;

	pvecPrimAdd = work->primAdd->pos[ work->primAdd->buffer_clock];
	pvecPrimSub = work->primSub->pos[ work->primSub->buffer_clock];
	uvrgbwhAdd  = work->primAdd->uvrgb[ work->primAdd->buffer_clock];
	uvrgbwhSub  = work->primSub->uvrgb[ work->primSub->buffer_clock];
	ppvecPos    = work->vecDrop;
	for ( i = 0; i < N_PRIMS; i++){
	    for ( j = 0; j < work->nDropNum; j++){
		_sceVu0CopyVector( pvecPrimAdd, (ppvecPos[j]));
		_sceVu0CopyVector( pvecPrimSub, (ppvecPos[j]));

		pvecPrimAdd++;
		pvecPrimSub++;
	    }
	}
	for ( i = 0; i < N_PRIMS; i++){
	    for ( j = 0; j < N_VERTS; j++){
		if ( j < work->nDropNum ){
		    uvrgbwhAdd->a = 0x50;
		    uvrgbwhSub->a = 0x45;
		}else{
		    uvrgbwhAdd->a = 0x00;
		    uvrgbwhSub->a = 0x00;
		}
		uvrgbwhAdd++;
		uvrgbwhSub++;
	    }
	}
    }

    work->nDropNum = 0;
}

// 破棄関数
static void Die( WATERDROP_MNG* work )
{
    work->primAdd = MAO_FreePrim2( work->primAdd );
    work->primSub = MAO_FreePrim2( work->primSub );
}
static int InitPacket( WATERDROP_MNG* 		work, 		// 
		       DG_PRIM2*	prim,		// 
		       DG_TEX* 		tex,		// 
		       long64		tagAlpha,	// 
		       u_int		nRGBA,		// 
		       int		nRaise)
{
    u_short		r,g,b,a;
    int			i, j;
    FVECTOR*		pvecVert1;
    FVECTOR*		pvecVert2;
    DG_PRIM2_UVRGBWH*	uvrgbwh1;
    DG_PRIM2_UVRGBWH*	uvrgbwh2;

    // 優先設定
    prim->raise = nRaise;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );	// プリミティブにテクスチャを設定
    DG_SetPrim2Alpha( prim, tagAlpha );	// α設定

    // RGBA値
    r = ((nRGBA & 0xff000000) >> 24);
    g = ((nRGBA & 0x00ff0000) >> 16);
    b = ((nRGBA & 0x0000ff00) >> 8);
    a =  (nRGBA & 0x000000ff);

    pvecVert1 = prim->pos[0];
    pvecVert2 = prim->pos[1];
    uvrgbwh1  = prim->uvrgb[0];
    uvrgbwh2  = prim->uvrgb[1];
    for ( i = 0; i < N_PRIMS; i++){
	for ( j = 0; j < N_VERTS; j++){
	    _sceVu0CopyVector( pvecVert1, &DG_ZeroVector);
	    _sceVu0CopyVector( pvecVert2, &DG_ZeroVector);

	    uvrgbwh2->w  = uvrgbwh1->w  = SIZE;
	    uvrgbwh2->h  = uvrgbwh1->h  = (int)(SIZE * 1.5f);

	    uvrgbwh2->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );/* 左上 */
	    uvrgbwh2->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );/* 左上 */
	    uvrgbwh2->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset );/* 右下 */
	    uvrgbwh2->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset );/* 右下 */
	    uvrgbwh2->q0 = uvrgbwh1->q0 = 4096;
	    uvrgbwh2->q1 = uvrgbwh1->q1 = 4096;
	    uvrgbwh2->f0 = uvrgbwh1->f0 = 0x0fff;
	    uvrgbwh2->f1 = uvrgbwh1->f1 = 0x0fff;

	    uvrgbwh2->r  = uvrgbwh1->r  = r;
	    uvrgbwh2->g  = uvrgbwh1->g  = g;
	    uvrgbwh2->b  = uvrgbwh1->b  = b;
	    uvrgbwh2->a  = uvrgbwh1->a  = 0x00;

	    pvecVert1++;
	    pvecVert2++;
	    uvrgbwh1++;
	    uvrgbwh2++;
	}
    }
    return 1;
}

// リソース初期化
static int GetResources( WATERDROP_MNG*	work,
			 int	name,
			 int	map)
{
    char* 	opt;
    int		i;
    int	  	nIntvNum  = 0;
    int	  	nBoundNum = 0;

    work->name = name;
    work->map  = map;

    work->nBoundNum = 0;
    work->nSleep = 0;

    //-------- GCLからデータ取得
    // 発生間隔
    if ( ( opt = GCL_GetOption( 'i' ) ) == NULL ){	
#ifdef DEBUG_MODE
	printf("WaterDrop GCL -intv Not found!!\n");
	ASSERT( 0 );
#endif
	for ( i = 0; i < MAX_BOUND; i++){ // バグ回避
	    work->nIntvMin[i] = 1000000;
	    work->nIntvMax[i] = 1000001;
	}
    }else{
	char* optData = opt;
	for ( i = 0; i < MAX_BOUND; i++){
	    // 最小
	    if ( optData != NULL ){
		GCL_SetArgTop( optData );
		work->nIntvMin[i] = GCL_GetNextInt();
		optData = GCL_NextStr();
	    }else{
		break;
	    }

	    // 最大
	    if ( optData != NULL ){
		GCL_SetArgTop( optData );
		work->nIntvMax[i] = GCL_GetNextInt();
		optData = GCL_NextStr();
	    }else{
		break;
	    }
	    nIntvNum++;
	}
    }
    
    // 発生バウンダリ
    if ( ( opt = GCL_GetOption( 'b' ) ) == NULL ){	
#ifdef DEBUG_MODE
	printf("WaterDrop GCL -boundry Not found!!\n");
	ASSERT( 0 );
#endif
	for ( i = 0; i < MAX_BOUND; i++){ // バグ回避
	    _sceVu0CopyVector( &work->vecMinTbl[i], &DG_ZeroVector);
	    _sceVu0CopyVector( &work->vecMaxTbl[i], &DG_ZeroVector);
	}
    }else{
	char* optPos = opt;
	for ( i = 0; i < MAX_BOUND; i++){
	    extern int	ENE_GCL_GetFV( char *ptr, FVECTOR *fvec );
	    if ( ENE_GCL_GetFV( optPos, &work->vecMinTbl[i] ) != 0 )	break;
	    if ( ENE_GCL_GetFV( optPos, &work->vecMaxTbl[i] ) != 0 )	break;
	    nBoundNum++;
	}
    }

    if ( nIntvNum != nBoundNum ){
#ifdef DEBUG_MODE
	printf("WaterDrop GCL -intv num != -boundry num ::: %d != %d\n", nIntvNum, nBoundNum);
	ASSERT( 0 );
#endif
	if ( nIntvNum < nBoundNum ) nIntvNum  = nBoundNum;
	else			    nBoundNum = nIntvNum;
    }
    
    // バウンディングボックスの数
    work->nBoundNum = nBoundNum;
    // プレイヤーの属しているバウンドのインデックス取得
    work->nPlayerIndex = GetPlayerBoundIndex( work);

    // カウンタ初期化
    for ( i = 0; i < work->nBoundNum; i++){
	work->nCntr[i] = 0;
	work->nTime[i] = MAO_GetRandom( work->nIntvMin[i], work->nIntvMax[i]);
    }

    {
	DG_TEX*	tex;

	// 描画用ワーク初期化
	// 加算プリミティブ本体の作成
	work->primAdd = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS);
	if ( work->primAdd == NULL){
	    MAO_PRINTF("Cannot MakePrim!!\n");
	    return -1;
	}
	// 減算プリミティブ本体の作成
	work->primSub = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS);
	if ( work->primSub == NULL){
	    MAO_PRINTF("Cannot MakePrim!!\n");
	    return -1;
	}
	
	// テクスチャ設定
	tex = DG_GetTexture( 4642619 ); // drop02_msk

	InitPacket( work, work->primAdd, tex, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ), 0x30303050, 1000);
	InitPacket( work, work->primSub, tex, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ), 0x20202045, -1000);
   work->primSub->flag |= DG_PRIM_AS_CUSTOMWORLD;
	work->primSub->as_world.m[3][0] = 5.f;
    }
    work->nDropNum = 0;

    return 0;
}

void* NewWaterDropManager( int name, int map )	
{
    WATERDROP_MNG*	work;

    OPERATOR();

    work = (WATERDROP_MNG *)GV_NewEffect( GV_ACTOR_USER, sizeof( WATERDROP_MNG ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );
	if ( GetResources( work, name, map ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void*)work ;
}


