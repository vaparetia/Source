//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	複数赤外線センサー  main.c
	1999/??/?? TANAKA
	2001/03/14 S.Kobayashi
	$Id: Senser.c,v 1.1.1.3 2002/11/19 11:50:23 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2 
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"
#include    "../../kano/attachment/attachment_called.h"
#include    "newirs.h"

#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))

enum {
	SK_NORMAL = 0x0,
	SK_COLD = 0x1,
};

typedef struct {
    OBJECT    body[N_IRS_SENSOR];
    FMATRIX   lights[N_IRS_SENSOR][2];

    TARGET    target[N_IRS_SENSOR];
    FVECTOR   pos[N_IRS_SENSOR];
    SVECTOR   rot[N_IRS_SENSOR];
	int       map[N_IRS_SENSOR];
} Senser;

typedef struct
{
    DG_PRIM2     *prim;
    DG_TEX       *tex ;                   
    float        patern[N_IRS_SENSOR_RAY];   // テクスチャパターン 
    float        speed[N_IRS_SENSOR_RAY];    // テクスチャ間の移動スピード 
	int          count ;               // 表示時間カウント数 
	int          a_flag ;              // アルファ値の増減をチェック
	int          a_flag2 ;              // アルファ値の増減をチェック 
	float        bright ;              // 赤外線の明るさ 
	// ターゲットを見る
	TARGET        target;
	FVECTOR       center;               // 当たり判定の中心
	FVECTOR         size;               // 大きさ 
} SenserRay;

typedef struct _Senser_Work {
	GV_ACT_EX actor;

	Senser senser;
	SenserRay senser_ray;
	FMATRIX world[ 2 ];
	FVECTOR sensor_pos;

	FVECTOR before_player_pos;

	int name;
	int map;
	int mode;
	int tabac_count;
	int cold_count;
	int exec_once;
	int proc;
	int flag;
	int cold_flag;
	char se_flag;
} Work;

enum {
	SK_SENSER_SE   = 0x01 , 
	SK_PARENT_CALL = 0x02 ,
	SK_SENSER_DIE  = 0x04 ,
	SK_ATTACH_DIE  = 0x08 , 
	SK_ALL_DIE     = 0x10 , 
	SK_SINGLE_DIE  = 0x20 , 
	SK_SENSER_SE_ON = 0x40 ,
};

// プロトタイプ
extern void *NewSensorLampComodel( FMATRIX * );

static FVECTOR   Irs_Sensor_Hole[] =
{
	{ 0.0F, 2280.0F, 75.0F, 1.0F},{ 0.0F, 2075.0F, -60.0F, 1.0F},
	{ 0.0F, 1870.0F, 75.0F, 1.0F},{ 0.0F, 1675.0F, -60.0F, 1.0F},
	{ 0.0F, 1470.0F, 75.0F, 1.0F},
	{ 0.0F, 1150.0F, 75.0F, 1.0F},{ 0.0F, 935.0F, -60.0F, 1.0F},
	{ 0.0F,  730.0F, 75.0F, 1.0F},{ 0.0F, 525.0F, -60.0F, 1.0F},
	{ 0.0F,  310.0F, 75.0F, 1.0F}
} ;

static int ReceiveSignal( void *work, int signal, int value )
{
	Work *pWork;

	pWork = ( Work * )work;
	switch ( signal ){
	case SK_SENSER_DIE : // 装置破壊
		pWork->exec_once = 1;
		pWork->tabac_count /= 6;
		pWork->flag = 1; 
		pWork->senser_ray.a_flag  = 0;
		pWork->senser_ray.a_flag2 = 0;
		pWork->senser_ray.count = 0;
		GV_CallChildSignalFunc( pWork , 0x04 , 0 );
		break;
	case SK_SENSER_SE : // se 呼ぶ
		GV_CallChildSignalFunc( pWork , SK_SENSER_SE , 0 );
		break;
	case SK_SENSER_SE_ON :
		if ( pWork->se_flag == 0 ){
			GM_SeSetMode( SD_A_OFFSENS1 , &pWork->sensor_pos , GM_SEMODE_BOMB );
			pWork->se_flag = 0x1;
		}
		break;
	case GV_SIGNAL_KILL :
		((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( work );
		return ( -1 );
	}
	return ( 0 );
}

static int InitSenser( Work *pWork , int model , int name , int where )
{
	int i;
	// 位置,方向の設定
	for ( i = 0 ; i < N_IRS_SENSOR ; i++ ){
		GM_InitObject( &pWork->senser.body[ i ] , model , BODY_FLAG2 );          // モデルの初期化
		GM_ConfigObjectLight( &pWork->senser.body[ i ] , pWork->senser.lights[ i ] ); // ライト設定

		DG_SetPos2( &pWork->senser.pos[ i ] , &pWork->senser.rot[ i ] );

		if ( !GM_IsOneID( where ) ){
			where = GM_GetMapIDfromPos( where , &pWork->senser.pos[ i ] ); // 場所から今自分のいる場所を割出す
		}
		GM_SetCurrentMap( where );    // カレントマップの設定
		GM_ActObject( &pWork->senser.body[ i ] ); // 配置

		DG_GetLightMatrix( &pWork->senser.pos[ i ] , pWork->senser.lights[ i ] );            // 位置からライトマトリックスの計算
		pWork->senser.map[ i ] = where;
	}

	return 0;
}

static void Hit( TARGET *pOff , TARGET *pDef , void *pWork )
{
	Work *work;

	work = ( Work * )pWork;
	if ( ( pDef->weapon_type & WP_COLDSPRAY ) && ( work->flag == 0 ) ){ // 赤外線ON
		work->cold_count++;
		if ( work->cold_count > MAX_BRIGHT ){
			work->cold_count = MAX_BRIGHT;
		}
		work->cold_flag |= SK_COLD;
	}
}

static void InitTarget( Work *pWork )
{
	GM_SetTarget( &pWork->senser_ray.target , TARGET_DEFENSE | TARGET_POWER | TARGET_THROUGH , pWork->senser.map[ 0 ] , BOTH_SIDE , &pWork->senser_ray.size , &DG_ZeroVector );
	GM_MoveTarget( &pWork->senser_ray.target , &pWork->senser_ray.center );             // ターゲットのトランス
	GM_SetTargetCallBack( &pWork->senser_ray.target , Hit , pWork ); // コールバックの設定
	GM_PutTarget( &pWork->senser_ray.target );						  // ターゲットの配置
}

static void Init_RayUvrgbScrPad( SenserRay *pRay ) // センサー初期化
{
	DG_PRIM2_UVRGB *uvrgb;
	float       *p_patern;
	float        *p_speed;
	int			        i;

	p_patern = pRay->patern;
	p_speed  = pRay->speed;
	uvrgb = SCR_UVS;

	for(i = 0; i < N_IRS_SENSOR_RAY; i++) {
		*p_patern = rnd() * 1.0F;
		*p_speed  = rnd() * 3.0F + 1.0F;
		uvrgb[0].u = FTOI12( pRay->tex->u_offset );
		uvrgb[0].v = FTOI12( pRay->tex->v_offset );
		uvrgb[0].q = 4096;
		uvrgb[0].f = 0x8fff;

		uvrgb[0].r = 128;
		uvrgb[0].g = 0;
		uvrgb[0].b = 0;
		uvrgb[0].a = 128; // アンチエイリアスを使用するときには１２８にする 

		uvrgb[1].u = FTOI12( pRay->tex->u_scale );
		uvrgb[1].v = FTOI12( pRay->tex->v_scale + pRay->tex->v_offset );
		uvrgb[1].q = 4096;
		uvrgb[1].f = 0x0fff;

		uvrgb[1].r = 128;
		uvrgb[1].g = 0;
		uvrgb[1].b = 0;
		uvrgb[1].a = 128; // アンチエイリアスを使用するときには１２８にする
		uvrgb += N_RAY_VERTS;
		p_patern++;
		p_speed++;
	}

	OK_Scr_Mem( pRay->prim->uvrgb[ 0 ] , SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_IRS_SENSOR_RAY * N_RAY_VERTS );
	OK_Scr_Mem( pRay->prim->uvrgb[ 1 ] , SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_IRS_SENSOR_RAY * N_RAY_VERTS );

	pRay->prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
}

static void Init_RayPosScrPad( Work *pWork )
{
	FVECTOR			 *pos;
	FVECTOR			 *local_pos1 , *local_pos2;
	int				 tmp[ 3 ];
	int				 r;
	int				 i , j;

	// 位置決め
	pos = SCR_POS;
	local_pos1 = (void *)(SCRPAD_ADDR + 0x2000);
	local_pos2 = (void *)(SCRPAD_ADDR + 0x2200);

	// センサーホールの位置を決める
	DG_SetPos2( &pWork->senser.pos[ 0 ] , &pWork->senser.rot[ 0 ] );
	DG_PutVector( Irs_Sensor_Hole , local_pos1 , N_IRS_SENSOR_RAY );
	
	DG_SetPos2( &pWork->senser.pos[ 1 ] , &pWork->senser.rot[ 1 ] );
	DG_PutVector( Irs_Sensor_Hole , local_pos2 , N_IRS_SENSOR_RAY );

	if(pWork->mode){
		DG_COPY_VEC( &local_pos1[8], &local_pos2[8] );
		DG_COPY_VEC( &local_pos1[9], &local_pos2[9] );
	}
	// 1列目はそろえる
	*pos = *local_pos1;
	pos++;
	local_pos1++;
	*pos = *local_pos2;
	pos++;
	local_pos2++;

	// 2列目～4列目まではランダム
	for( i = 0 ; i < 3 ; i++ ){
		tmp[ i ] = i;
	}
	for( i = 3 ; i > 0 ; i-- ){
		*pos = *local_pos1;  pos ++; local_pos1 ++;
		r = irnd() % i;
		*pos = local_pos2[ tmp[ r ] ];  pos ++;
		for(j = r ; j < i ; j++){
			tmp[ j ] = tmp[ j + 1 ];
		}
	}
	local_pos2 += 3;
	
	/* 5,6列目は同じ */
	*pos = *local_pos1;   pos++; local_pos1++;
	*pos = *local_pos2;   pos++; local_pos2++;
	*pos = *local_pos1;   pos++; local_pos1++;
	*pos = *local_pos2;   pos++; local_pos2++;
	
	/* 7列目から9列目はランダム */
	for( i = 0 ; i < 3 ; i++ ){
		tmp[i] = i ;
	}
	for( i = 3 ; i > 0 ; i-- ){
		*pos = *local_pos1;  pos++; local_pos1++;	
		r = irnd() % i;
		*pos = local_pos2[tmp[r]]; pos++;
		for( j = r ; j < i ; j++ ){
			tmp[j] = tmp[j + 1];
		}
	}
	local_pos2 += 3; 

	/* 10列目は同じ */
	*pos = *local_pos1;   pos++; local_pos1++;
	*pos = *local_pos2;   pos++; local_pos2++;

	// スクラッチから転送
	OK_Scr_Mem( pWork->senser_ray.prim->pos[ 0 ] , SCR_POS , sizeof( FVECTOR ) , N_IRS_SENSOR_RAY * N_RAY_VERTS );
	OK_Scr_Mem( pWork->senser_ray.prim->pos[ 1 ] , SCR_POS , sizeof( FVECTOR ) , N_IRS_SENSOR_RAY * N_RAY_VERTS );
}


// 赤外線の当たりの範囲を決定 
static void InitSensorRay_Bound(FVECTOR *base , SenserRay  *pRay)
{
	float	 min,max;
	FVECTOR   *pos;
	FVECTOR   *bound;
	int	   i;

	bound = SCR_WORK;
	pos = MEM_ADDR1;
	min = max = pos->vx;
	pos++;
	for(i = 1 ; i < 4 ; i++){
		if( min > pos->vx ){
			min = pos->vx;
		}
		if( max < pos->vx ){
			max = pos->vx;
		}
		pos++;
	}

	pos = SCR_POS;
	bound[0].vx = min;
	bound[1].vx = max;
	bound[0].vy = base->vy;
	bound[1].vy = pos->vy;

	min = max = pos->vz;
	pos++;
	for( i = 1 ; i < 4 ; i++ ){
		if( min > pos->vz ){
			min = pos->vz;
		}
		if( max < pos->vz ){
			max = pos->vz;
		}
		pos++;
	}

	bound[0].vz = min;
	bound[1].vz = max;

	pRay->center.vx = ( bound[0].vx + bound[1].vx ) * 0.5f;
	pRay->center.vy = ( bound[0].vy + bound[1].vy ) * 0.5f;
	pRay->center.vz = ( bound[0].vz + bound[1].vz ) * 0.5f;
	pRay->size.vx = ( bound[1].vx - bound[0].vx ) * 0.5f;
	pRay->size.vy = ( bound[1].vy - bound[0].vy ) * 0.5f;
	pRay->size.vz = ( bound[1].vz - bound[0].vz ) * 0.5f;

// その場対応、小さい方をちょい大きくする

	if( pRay->size.vx > pRay->size.vz ){
		pRay->size.vz*= 4.0f;
	}else{
		pRay->size.vx*= 4.0f;
	}


}

#define STR_LIGHT_MSK (6792560) // lsight_msk
static int InitSensorRay( Work *pWork )
{
	SenserRay    *ray;
	DG_PRIM2	*prim;
	DG_TEX	     *tex;

	ray = &pWork->senser_ray;
	// プリミティブをセット
	prim = ray->prim = GM_MakePrim2(PRIM_TYPE , N_IRS_SENSOR_RAY , N_RAY_VERTS);
	if( prim == NULL ){
		return -1;
	}
	// テクスチャの取得
	tex = ray->tex = DG_GetTexture( STR_LIGHT_MSK );
	if( tex == NULL ){
		return -1;
	}

	DG_ConfigPrim2Tex(prim,tex);
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ));

	// uvrgb 決め
	Init_RayUvrgbScrPad( ray );
	// 位置決め
	Init_RayPosScrPad( pWork );
	// ターゲット用のバウンディングボックスを求める
	InitSensorRay_Bound( &pWork->senser.pos[ 0 ] , ray );
	InitTarget( pWork );
	// カウントセット
	ray->count = 0;
	ray->a_flag  = 0;
	ray->a_flag2 = 0;
	ray->bright = 0.0F;

	return 0;
}

// 赤外線テクスチャデータ更新
static void Move_Ray( Work *pWork )
{
	DG_PRIM2	*prim;
	DG_TEX		*tex;
	SenserRay   *ray;
	float		*p_patern,*p_speed;
	DG_PRIM2_UVRGB	*uvrgb;
	GV_MSG		*msg;
	int			mes_num;
	int			num;
	int			visible_flag;
	int			item_id;
	FVECTOR		fvtemp;
	float		length;
	float		max_alpha0;
	float		max_alpha1;
	float       alpha_tmp;
	int			i;

	ray = &pWork->senser_ray;
	prim = ray->prim;
	tex  = ray->tex;

	p_patern = ray->patern;
	p_speed  = ray->speed;
	uvrgb	 = prim->uvrgb[prim->buffer_clock];

	_sceVu0SubVector( &fvtemp , &pWork->senser.pos[0] , &GM_PlayerPosition );
	length = GV_VecLen3F( &fvtemp );
	visible_flag = 0;
	if ( pWork->flag == 0 ){
		if( length < SENSE_LENGTH ){
			// たばこを持っているかどうかをチェック
			item_id = PL_GetPlayerItem();
			if(item_id == IT_Tabacco){
				visible_flag = 1 ;
			}
		}
		// メッセージをチェックし カラーを決める
		mes_num = GV_ReceiveMessage( pWork->name , &msg );
		msg += mes_num - 1;
		while( --mes_num >= 0 ){
			num = msg->message[ 0 ];
			if(num == IRS_VISIBLE_FLAG){
				ray->a_flag  = 1;
				ray->a_flag2 = 1;
			} else if ( num == IRS_NORMAL_FLAG ){
				ray->a_flag  = 0;
				ray->a_flag2 = 0;
				ray->count = 0;
			}			
			msg--;
		}
	}
	// フラグが立ったとき
	if( ray->a_flag ){
		ray->count++;
		if(ray->count > MAX_COUNT){
			ray->count = 0;
			ray->a_flag = 0;
		}
	}
	max_alpha0 = 0.0f;
	max_alpha1 = 0.0f;

	if( ray->count > 0 ){
		if( ray->count < INIT_COUNT ){
			max_alpha0 = MAX_BRIGHT * (float)ray->count / (float)INIT_COUNT;
		}else if( ray->count > FINISH_COUNT ){
			max_alpha0 = MAX_BRIGHT * (float)(MAX_COUNT - ray->count) / (float)(MAX_COUNT - FINISH_COUNT);
		}else{
			max_alpha0 = MAX_BRIGHT;
		}
	}

	if( visible_flag ){
		pWork->tabac_count++ ;
		/*if( pWork->tabac_count > MAX_COUNT ) {
			pWork->tabac_count = MAX_COUNT;
		}*/
		if( pWork->tabac_count > INIT_COUNT ) {
			pWork->tabac_count = INIT_COUNT;
		}
	}else{
		pWork->tabac_count-- ;
		if( pWork->tabac_count < 0 ){
			pWork->tabac_count = 0;
		}
	}
	if ( PL_GetPlayerItem() == IT_Thermal && pWork->flag == 0 ){
		ray->bright = MAX_BRIGHT;
	} else {
		alpha_tmp = 0.0f;
		if ( pWork->cold_count != 0 ) {
			alpha_tmp = ( float )pWork->cold_count;
		}
		if( pWork->tabac_count < INIT_COUNT ){
			max_alpha1 = MAX_BRIGHT * (float)pWork->tabac_count / (float)INIT_COUNT;			
		//}else if( pWork->tabac_count > FINISH_COUNT ){
			//max_alpha1 = MAX_BRIGHT * (float)(FINISH_COUNT - pWork->tabac_count) / (float)FINISH_COUNT;
		}else{
			max_alpha1 = MAX_BRIGHT;
		}
		max_alpha1 *= (SENSE_LENGTH-length) / SENSE_LENGTH;
		ray->bright = (max_alpha0 > max_alpha1)? max_alpha0: max_alpha1;

		// cold or tabaco
		ray->bright = ray->bright < alpha_tmp ? alpha_tmp : ray->bright;
	}
	if ( PL_GetPlayerItem() == IT_Thermal && pWork->flag == 0 ){
		//prim->flag &= ~(DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING) ;
		prim->flag &= ~DG_PRIM2_TEX ;
		ray->bright = 255 ;
	} else {
		prim->flag |= PRIM_TYPE ;
	}

	/* 赤外線のパターンをチェック */
	for(i = 0 ; i < N_IRS_SENSOR_RAY ; i++){
		*p_patern = *p_patern + 1.0F / 256.0F * (*p_speed) ;
		if(*p_patern >= 1.0F){
			*p_patern = 0.0F ;
			*p_speed = rnd() * 2.0F + 1.0F;
		}
		uvrgb[0].u = FTOI12(tex->u_offset) ;
		uvrgb[0].v = FTOI12(*p_patern * tex->v_scale + tex->v_offset) ;
		uvrgb[1].u = FTOI12(tex->u_scale + tex->u_offset) ;
		uvrgb[1].v = FTOI12(*p_patern * tex->v_scale + tex->v_offset) ;
		uvrgb[0].r = uvrgb[0].g = uvrgb[0].b = (u_short)ray->bright ;
		uvrgb[1].r = uvrgb[1].g = uvrgb[1].b = (u_short)ray->bright ;
		uvrgb += N_RAY_VERTS ;
		p_patern ++ ;
		p_speed ++ ;
	}
}
// cross 1
// no cross 0

static int CheckLinePlaneCross( FVECTOR *a, FVECTOR *b, FVECTOR *center, FVECTOR *size )
{
	FVECTOR		norm;
	FVECTOR		wide;
	FVECTOR		bound[5];
	FVECTOR		vec[6];
	FVECTOR		fvtemp;
	FVECTOR		cross;

	float		l,m;
	int			i,j;
	wide.vx = DG_FABS(size->vx);
	wide.vy = DG_FABS(size->vy);
	wide.vz = DG_FABS(size->vz);
	wide.vw = 0.0f;

	_sceVu0SubVector( &bound[0], center, &wide );
	_sceVu0AddVector( &bound[2], center, &wide );

	vec[0].vx = 0.0f;
	vec[0].vy = wide.vy*2.0f;
	vec[0].vz = 0.0f;
	vec[0].vw = 0.0f;
	
	vec[1].vx = wide.vx*2.0f;
	vec[1].vy = 0.0f;
	vec[1].vz = wide.vz*2.0f;
	vec[1].vw = 0.0f;

	_sceVu0ScaleVector( &vec[2], &wide, -2.0f );
	
	vec[3].vx = 0.0f;
	vec[3].vy = -wide.vy*2.0f;
	vec[3].vz = 0.0f;
	vec[3].vw = 0.0f;
	
	vec[4].vx = -wide.vx*2.0f;
	vec[4].vy = 0.0f;
	vec[4].vz = -wide.vz*2.0f;
	vec[4].vw = 0.0f;

	_sceVu0ScaleVector( &vec[5], &wide, 2.0f );

	
	_sceVu0AddVector( &bound[1], &vec[0], &bound[0] );
	_sceVu0AddVector( &bound[3], &vec[1], &bound[0] );
#if 0
	//デバッグ平面表示
	{
		DG_COPY_VEC( &bound[4], &bound[0] );
/*		FVECTOR tri_verts[6];
		DG_COPY_VEC( &tri_verts[0], &bound[0] );
		_sceVu0AddVector( &tri_verts[1], &vec[0], &bound[0] );
		_sceVu0AddVector( &tri_verts[2], &vec[1], &bound[0] );

		DG_COPY_VEC( &tri_verts[3], &bound[2] );
		_sceVu0AddVector( &tri_verts[4], &vec[3], &bound[2] );
		_sceVu0AddVector( &tri_verts[5], &vec[4], &bound[2] );
*/
		NewTriangleView( &bound[0], 1, 128,0,0 );
		NewTriangleView( &bound[2], 1, 0,128,0 );
	}
#endif
#if 0
	//デバッグ線分表示
	{
		FVECTOR line[2];
		DG_COPY_VEC( &line[0], a );
		DG_COPY_VEC( &line[1], b );
		
		NewLineView( line, 1, 128,0,128 );
	}
#endif
	_sceVu0OuterProduct( &norm, &vec[0], &vec[1] );
	_sceVu0Normalize( &norm, &norm );
#if 0
	//デバッグ用平面情報表示
	_sceVu0ScaleVector( &fvtemp, &norm, 1000.0f );
	_sceVu0AddVector( &fvtemp, &fvtemp, center );
	AN_Test_Eye2( &bound[0], 2 );
	AN_Test_Eye2( &bound[2], 2 );
	AN_Test_Eye2( center, 2 );
	AN_Test_Eye2( &fvtemp, 2 );
#endif

	_sceVu0SubVector( &fvtemp, &bound[0], a );
	l = _sceVu0InnerProduct( &norm, &fvtemp );

	_sceVu0SubVector( &fvtemp, b, a );
	m = _sceVu0InnerProduct( &norm, &fvtemp );

	if( m == 0.0F || l/m < 0.0f || 1.0f < l/m ) return 0;	// 2002/02/21 K.Uehara 0.0check
	_sceVu0ScaleVector( &fvtemp, &fvtemp, l/m );
	_sceVu0AddVector( &cross, &fvtemp, a );
	
	for( i = 0; i < 2; i++ ){		
		float inner[3];
		for( j = 0; j < 3; j++ ){
			FVECTOR	outer;
			_sceVu0SubVector( &fvtemp, &cross, &bound[(2*i + j)%4] );
			_sceVu0OuterProduct( &outer, &fvtemp, &vec[i*3 + j] );
			inner[j] = _sceVu0InnerProduct( &norm, &outer );
		}
		if( (inner[0] >= 0.0f && inner[1] >= 0.0f && inner[2] >= 0.0f) ||
			(inner[0] <= 0.0f && inner[1] <= 0.0f && inner[2] <= 0.0f) ){
			//AN_Test_Eye2( &cross, 2 );
			return 1;
		}
	}
	return 0;
}

/* 赤外線に触れているかの判別 */
static int Check_Touch_Laser( Work *work, FVECTOR *size )
{
	FVECTOR bound_min, bound_max ;
	FVECTOR center;
	CONTROL	**list ;
	int		i, n ;
//	int		num;

	_sceVu0SubVector(&bound_min, &work->senser_ray.center , size);
	_sceVu0AddVector(&bound_max, &work->senser_ray.center , size);

	if( work->mode ){
		bound_min.vy += 500.0f;
	}

#if 1
	list = GM_WhereList ;
	n = GM_N_WhereList ;
//printf("num:%d\n",n);
	for ( i = n ; i > 0 ; -- i, list++ ) {
		if( (*list)!=NULL ){
//NewBoundingBoxView( &bound_min, &bound_max, 64, 32, 16 );
			if( vu0_CheckBoundingBox( &(*list)->mov, &bound_min, &bound_max ) ){
				return 1;
			}
		}
	}
	// 主観専用
	if ( GM_PlayerControl != NULL ){
		if( vu0_CheckBoundingBox( &GM_PlayerFindPos , &bound_min, &bound_max ) ){
			return 1;
		}
	}
#endif


#if 1
	// 2001.9/3
	center.vx = work->senser_ray.center.vx;
	center.vy = work->senser_ray.center.vy;
	center.vz = work->senser_ray.center.vz;
	if( work->mode ){
		center.vy+= 500.0f;
	}
	if( CheckLinePlaneCross( &GM_PlayerPosition, &work->before_player_pos, &center, size ) ){
printf("ok:COME!!\n");
		return 1;
	}

#else
// プレイヤー専用詳細チェック
	if( (GM_PlayerPosition.vy > bound_min.vy)
	 && (GM_PlayerPosition.vy < bound_max.vy) ){
		FVECTOR	fvtemp0;
		FVECTOR	fvtemp1;
		FVECTOR	fvtemp2;
		FVECTOR	fvtemp3;
		float	f0;
		float	f1;
		float	f2;
		float	f3;

		_sceVu0SubVector( &fvtemp0, &GM_PlayerPosition,       &work->before_player_pos );
		fvtemp0.vy = 0.0f;
		f1 = GV_VecLen3F( &fvtemp0 );
		if( f1 > 1.0f ){
			// 移動ベクトルとセンサー中心との距離を求める
			_sceVu0SubVector( &fvtemp1, &work->senser_ray.center, &work->before_player_pos );
			fvtemp1.vy = 0.0f;

			_sceVu0OuterProduct( &fvtemp2, &fvtemp0, &fvtemp1 );
			_sceVu0OuterProduct( &fvtemp2, &fvtemp0, &fvtemp2 );
			_sceVu0Normalize( &fvtemp2, &fvtemp2 );

			f0 = _sceVu0InnerProduct( &fvtemp1, &fvtemp2 );
			// センサーとの距離を調べる
			f2 = DG_FABS( f0 );
			if( (f2 < size->vx)
			 || (f2 < size->vz) ){
//printf("ok:%f::%f %f\n",f1,size->vx,size->vz);
				// 最短地点は移動距離内か調べる
				_sceVu0ScaleVector( &fvtemp2, &fvtemp2,-f0 );
				_sceVu0AddVector( &fvtemp2, &fvtemp2, &work->senser_ray.center );

//AN_Test_Eye2( &fvtemp2, 2 );

				_sceVu0SubVector( &fvtemp0, &fvtemp2, &work->before_player_pos );
				_sceVu0SubVector( &fvtemp1, &fvtemp2, &GM_PlayerPosition );

				fvtemp0.vy = 0.0f;
				fvtemp1.vy = 0.0f;

				f2 = GV_VecLen3F( &fvtemp0 );
				f3 = GV_VecLen3F( &fvtemp1 );

//printf("ok:%f %f %f\n",f1, f2, f3);

				if( f2+f3 < f1+1.0f ){
					printf("ok:--------------------------RARE CASE OCCASION!\n");
					return 1;
				}
			}
		}
	}
#endif

	return 0;

}

static int SENSORRAY_Act( Work *pWork )
{
	SenserRay *pRay;
	FVECTOR	fvtemp;
	int  flag;	

	pRay = &pWork->senser_ray;
	flag = 0;
	// 判定チェック
	if( (GM_Item == IT_CBBox)
	 || (GM_Item == IT_CBBoxB)
	 || (GM_Item == IT_CBBoxC)
	 || (GM_Item == IT_CBBoxWet)
	  ){
		fvtemp.vx = 200.0f;
		fvtemp.vy = 0.0f;
		fvtemp.vz = 200.0f;
		_sceVu0AddVector( &fvtemp, &pRay->size, &fvtemp );
	}else{
		DG_COPY_VEC( &fvtemp, &pRay->size );
	}
	if( pWork->exec_once == 0 ){
		if ( GM_PlayerBody == NULL ){
			flag = 0 ;
		} else { 
			flag = Check_Touch_Laser( pWork , &fvtemp ) ;
		}
//printf("Check_Touch_Laser::%d\n",flag);
		if(flag){
			if(pWork->proc != 0){
				if ( !GM_IsGameOver() ){
					GCL_ExecProc(pWork->proc , NULL);
					pWork->exec_once = 1;
				}
			}
		}
	}
	// 赤外線の移動
	Move_Ray( pWork );

	return flag ;
}

static int GetResources( Work *pWork , FVECTOR *pPos , SVECTOR *pRot , int proc , int mode , int model , int name , int where )
{
	FVECTOR fvtmp;
	int i;

	DG_COPY_VEC( &pWork->before_player_pos, &GM_PlayerPosition );

	pWork->name = name;
	pWork->proc = proc; // [0]
	pWork->flag = 0;
	pWork->cold_count = 0;
	pWork->cold_flag = SK_NORMAL;
	pWork->se_flag = 0;
	// 下の赤外線をつけるかどうか
	pWork->mode = mode;
	// シグナルの登録
	GV_SetActorSignalFunc( pWork , ReceiveSignal );
	// センサーの初期化
	pWork->senser.pos[ 0 ] = pPos[ 0 ];
	pWork->senser.pos[ 1 ] = pPos[ 1 ];
	pWork->senser.rot[ 0 ] = pRot[ 0 ];
	pWork->senser.rot[ 1 ] = pRot[ 1 ];
	InitSenser( pWork , model , name , where );
	// 真中算出
	for ( i = 0 ; i < 2 ; i ++ ) {
		DG_SetPos2( &pWork->senser.pos[ i ] , &pWork->senser.rot[ i ] );
		DG_GetPos( &pWork->world[ i ] );
		GV_SetActorChild( pWork , NewSensorLampComodel( &pWork->world[ i ] ) );
	}
	// Sensorの中心を出す
	_sceVu0SubVector( &fvtmp , &pWork->senser.pos[ 1 ] , &pWork->senser.pos[ 0 ] );
	_sceVu0DivVector( &fvtmp , &fvtmp , 2.0f );
	fvtmp.vy += 2400.0f;
	_sceVu0AddVector( &pWork->sensor_pos , &pWork->senser.pos[ 0 ] , &fvtmp );
	// 赤外線の初期化
	if ( InitSensorRay( pWork ) < 0 ){
		return ( -1 );
	}
	return 0;
}

static void Act( Work *pWork )
{
	int i;

	SENSORRAY_Act( pWork ); //  赤外線動作
	if ( !( pWork->cold_flag & SK_COLD ) ){
		pWork->cold_count--;
		if ( pWork->cold_count < 0 ){
			pWork->cold_count = 0;
		}
	} else {
		pWork->cold_flag &= ~SK_COLD;
	}
// このターゲットはコールドスプレイ用
//	NewTargetView2( &pWork->senser_ray.target , 128 , 128 , 160 ); // ターゲットの表示
	for ( i = 0 ; i < N_IRS_SENSOR ; i++ ){ 
		DG_GetLightMatrix( &pWork->senser.pos[ i ] , pWork->senser.lights[ i ] ); // 位置からライトマトリックスの計算
	}

	DG_COPY_VEC( &pWork->before_player_pos, &GM_PlayerPosition );
}

static void Die( Work *pWork )
{
	int i;

	for ( i = 0 ; i < N_IRS_SENSOR ; i++ ) { // センサー解放
		GM_FreeObject( &pWork->senser.body[ i ] ); // オブジェクトの解放
	}
	if(pWork->senser_ray.prim != NULL){    // 赤外線領域開放
		pWork->senser_ray.prim = OK_FreePrim2( pWork->senser_ray.prim );
	}
}


void *NewSenser( FVECTOR *pPos , SVECTOR *pRot , int proc , int mode , int model , int name , int where )
{
	Work *pWork;

	pWork = ( Work * )GV_NewActor( GV_ACTOR_USER , sizeof( Work ) );
	if ( pWork != NULL ) {
		GV_SetActor( &pWork->actor , Act , Die );
		GV_ActorEX( &pWork->actor );

		pWork->map = where;

		if ( GetResources( pWork , pPos , pRot , proc , mode , model , name , where ) != 0 ) {
			GV_DestroyActor( pWork );
			return NULL;
		}
	}
	return pWork;
}
