//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	putspot.c
	静止スポットライト設置

	1999/10/25 K.Takabe
	$Id: putspot.c,v 1.1.1.3 2002/11/19 11:51:15 Yoshizawa1 Exp $

*/
/*

chara 静止スポットライト設置[NewPutStaticSpotLight] $s:名前 \
	-pos $v:設置位置 \
	-dir $v:光源方向（自動的に正規化される） \
	-range $w:有効距離 \
	-angle $b:円錐角度（１度単位で、最大は１８０未満） \
	-color $b:光源色（赤） $b:光源色（緑） $b:光源色（青） \
	-fadeproc $p:フェードイン・アウト用コールバック \
	-option $i:オプションフラグ
mesg 静止スポットライト設置 $s:名前 on[1]
mesg 静止スポットライト設置 $s:名前 off[0]
mesg 静止スポットライト設置 $s:名前 fadein[2] $w:カウント（１／３００秒単位）
mesg 静止スポットライト設置 $s:名前 fadeout[3] $w:カウント（１／３００秒単位）
// スポットライトを設置する
// . . . .
// 同じ名前の「スポットライト投影モデル」を予め置いていないと効果がない
// フェードイン・アウトには終了検出用のコールバック用procを設定可能
// オプションフラグ詳細
//  0x0001:-dir指定を方向ベクトルではなく回転ベクトルとして認識
//  0x0002:OFF状態で起動する（デフォルトはONで起動）
//  0x0004:キャラに反映するライトを置かない（スポットライト投影のみに）
//  0x0010:高速画角切替え（おそらく使用しない）
//  0x0020:レイブン影用Y軸回転アニメーション
//  0x8000:デバッグ機能ＯＮ（２コンＬ１を押しながら操作）

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"g_other.h"

#define CLAMP( v, max, min ) {(v)=((v)>min)?(v):min;(v)=((v)<max)?(v):max;}

#define HEAD_JOINT	(12)

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT		actor ;
	DG_SPOT		spot ;
	FMATRIX		world ;
	FVECTOR		pos ;
	FVECTOR		dir ;
	int			name ;
	int			where ;
	float		umbra ;
	float		penumbra ;
	float		range ;
	float		angle ;
	float		angle_cos ;
	float		angle_sin ;
	u_int		color ;
	SVECTOR		rot ;
	int			option ;
	int			enable ;
	int			mode ;
	int			fade_count ;
	int			current_fade_count ;
	int			fade_proc ;
	/* スポットライト揺らしパラメータ */
	int			count ;
#ifdef DEBUG_MODE //マスターでは消える
	int			debug_flag ;
	int			debug_r, debug_g, debug_b ;
#endif
} Work ;

/* ---------------------------------------------------------------- */
#define MAX_SPOT_CHECK_WORK	(8)

static int	LastGVTime = 0 ;
static int	MaxSpotList = 0 ;
static GCT_ProjectionSpot_SpotParam	SpotList[ MAX_SPOT_CHECK_WORK ];
//extern int							GC_ProjectionSpot_HitLights ;
//extern GCT_ProjectionSpot_SpotParam	*GC_ProjectionSpot_SnakeCheckList ;

/* ---------------------------------------------------------------- */
static void CheckSnakeShadow( Work *work )
{
	FVECTOR		vec ;
	float		c, lenlen, d ;

	/* バッファ更新チェック */
	if ( LastGVTime != GV_Time ){
		LastGVTime = GV_Time ;
		MaxSpotList = 0 ;
		GC_ProjectionSpot_HitLights = 0 ;
	}

	if ( work->enable == 0 ) return ;

	/* プレイヤー用ワークの存在チェック */
	if ( GM_PlayerBody == NULL ) return ;

	/* 最大数チェック */
	if ( MaxSpotList >= MAX_SPOT_CHECK_WORK ) return ;
	/* 角度によるチェック */
	_sceVu0SubVector( &vec, (FVECTOR*)&GM_PlayerBody->objs->objs[HEAD_JOINT].world.m[3][0], &work->pos );
	lenlen = vec.vx * vec.vx + vec.vy * vec.vy + vec.vz * vec.vz ;
	d = DG_RSQRT( lenlen );
	_sceVu0ScaleVectorXYZ( &vec, &vec, d );
	c = _sceVu0InnerProduct( &vec, &work->dir );
	if ( c < work->angle_cos ) return ;
	/* 距離によるチェック */
	if ( lenlen > ( work->range * work->range * 4 ) ) return ;
	//printf("%d %f %f\n", MaxSpotList, c, work->angle_cos );

	SpotList[ MaxSpotList ].pos = work->pos ;
	SpotList[ MaxSpotList ].dir = work->dir ;
	SpotList[ MaxSpotList ].name = work->name ;
	SpotList[ MaxSpotList ].flag = 0 ;
	SpotList[ MaxSpotList ].angle = work->angle ;
	SpotList[ MaxSpotList ].range = work->range ;

	MaxSpotList++ ;
	GC_ProjectionSpot_HitLights = MaxSpotList ;
}
/* フェードイン・アウト用proc呼び出し */
static void ExecFadeProc( Work *work, int mode )
{
	GCL_ARGS	arg ;
	int			data[4] ;

	if ( work->fade_proc == 0 ) return ;
	arg.argc = 1 ;
	arg.argv = data ;
	data[0] = mode ;
	GCL_ExecProc( work->fade_proc, &arg );
}
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	u_int	current_color ;

	{/* メッセージチェック */
		GV_MSG *msg;
		int n;
		if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
			for ( ; n > 0 ; n--, msg++ ){
				switch ( msg->message[0] ){
				  case 0:/* OFF */
					work->enable = 0 ;
					work->mode = 0 ;		/* normal mode */
					break ;
				  case 1:/* ON */
					work->enable = 1 ;
					work->mode = 0 ;		/* normal mode */
					break ;
				  case 2:/* FADEIN */
					work->enable = 1 ;
					work->mode = 1 ;		/* FADEIN mode */
					work->fade_count = msg->message[1] ;
					work->current_fade_count = 0 ;
					break ;
				  case 3:/* FADEOUT */
					work->enable = 1 ;
					work->mode = 2 ;		/* FADEOUT mode */
					work->fade_count = msg->message[1] ;
					work->current_fade_count = work->fade_count ;
					break ;
				}
			}
		}
	}

	/* 描画色の設定（フェードイン・アウト対応） */
	switch ( work->mode ){
		int		r, g, b ;
	  default:
	  case 0:	/* normal mode */
		current_color = work->color ;
		break;
	  case 1:	/* FADEIN mode */
		r = work->color & 0xff ;
		g = ( work->color >> 8 ) & 0xff ;
		b = ( work->color >> 16 ) & 0xff ;
		r = r * work->current_fade_count / work->fade_count ;
		g = g * work->current_fade_count / work->fade_count ;
		b = b * work->current_fade_count / work->fade_count ;
		current_color = r | ( g << 8 ) | ( b << 16 ) | ( 128 << 24 ) ;
		/* 終了チェック */
		work->current_fade_count += TIME_BASE ;
		if ( work->current_fade_count >= work->fade_count ){
			work->mode = 0 ;
			current_color = work->color ;
			/* コールバック呼び出し */
			ExecFadeProc( work, 0 );
		}
		break;
	  case 2:	/* FADEOUT mode */
		r = work->color & 0xff ;
		g = ( work->color >> 8 ) & 0xff ;
		b = ( work->color >> 16 ) & 0xff ;
		r = r * work->current_fade_count / work->fade_count ;
		g = g * work->current_fade_count / work->fade_count ;
		b = b * work->current_fade_count / work->fade_count ;
		/* 終了チェック */
		current_color = r | ( g << 8 ) | ( b << 16 ) | ( 128 << 24 ) ;
		work->current_fade_count -= TIME_BASE ;
		if ( work->current_fade_count <= 0 ){
			current_color = 128 << 24 ;
			work->mode = 0 ;
			work->enable = 0 ;
			/* コールバック呼び出し */
			ExecFadeProc( work, 1 );
		}
		break;
	}
	work->spot.color = current_color ;

	/* テンポラリライトの設置＆ＯＮ・ＯＦＦ制御処理 */
	if ( work->enable ){
		if ( !( work->option & 0x0004 ) ){
			DG_SetTmpSpotLight( &work->pos, &work->dir,
							   work->range, work->umbra, work->penumbra, current_color, LIT_FLAG_CHARAONLY );
		}
		work->spot.flag &= ~DG_FLAG_INVISIBLE ;
	} else {
		work->spot.flag |= DG_FLAG_INVISIBLE ;
	}

	/* スポットライト揺らし */
	if ( work->option & 0x0030 ){
		float	f ;
		if ( work->option & 0x0010 ){
			f = 1.5f / 180.0f * 3.14159265f ;
			if ( work->count & 1 ){
				work->spot.angle = work->angle_cos / work->angle_sin ;
			} else {
				work->spot.angle = cosf( work->angle+f ) / sinf( work->angle+f ) ;
			}
		}
		if ( work->option & 0x0020 ){
			f = sinf( (float)( work->count & 511 ) / 256.0f * 3.14159265f );
			f = f * ( 0.02f / 180.0f * 3.14159265f ) + ( 0.5f / 180.0f * 3.14159265f ) ;
			_sceVu0RotMatrixY( &work->spot.world, &work->world, f );
		}
		work->count++ ;
	}


#ifdef DEBUG_MODE
	if ( work->debug_flag ){
		FMATRIX	mat ;
		FVECTOR	vec1, vec2, offset ;
		int		pad ;
		float	add = 1.0f/180.0f*(float)M_PI ;
		pad = GV_PadData[1].status ;
		if ( pad & PAD_L1 ){
			DEBUG_Locate( 30, 60, 0 );
			DEBUG_Color( 255, 255, 255, 128 );
			DEBUG_Printf( "PUTSPOT.C DEBUG\n" );
			DEBUG_Printf("POS:%5d %5d %5d\n", (int)work->pos.vx, (int)work->pos.vy, (int)work->pos.vz );
			DEBUG_Printf("ROT:%5d %5d %5d\n", work->rot.vx, work->rot.vy, work->rot.vz );
			DEBUG_Printf("ANGLE:%5d \n", (int)( work->angle * 180.0f / (float)M_PI + 0.5f ) );
			DEBUG_Printf("COLOR:%3d %3d %3d \n", work->debug_r, work->debug_g, work->debug_b );
			DEBUG_Printf("DEF:ROT & POS & ANGLE CONTROL\n");
			DEBUG_Printf("R1 :COLOR CONTROL\n");
			if ( !( pad & PAD_R1 ) ){
				if ( !( pad & PAD_B ) && !( pad & PAD_A ) ){
					/* スポットライト方向＆角度指定 */
					if ( pad & PAD_U ) work->rot.vx -= 10 ;
					if ( pad & PAD_D ) work->rot.vx += 10 ;
					if ( pad & PAD_L ) work->rot.vy += 10 ;
					if ( pad & PAD_R ) work->rot.vy -= 10 ;
				}
				if ( ( pad & PAD_B ) && !( pad & PAD_A ) ){
					/**/
					offset = DG_ZeroVector ;
					if ( pad & PAD_L ) offset.vx = 20.0f ;
					if ( pad & PAD_R ) offset.vx = -20.0f ;
					if ( pad & PAD_U ) offset.vy = 20.0f ;
					if ( pad & PAD_D ) offset.vy = -20.0f ;
					_sceVu0ApplyMatrix( &work->pos, &work->spot.world, &offset );
				}
				if ( ( pad & PAD_B ) && ( pad & PAD_A ) ){
					offset = DG_ZeroVector ;
					if ( pad & PAD_L ) offset.vx = 20.0f ;
					if ( pad & PAD_R ) offset.vx = -20.0f ;
					if ( pad & PAD_U ) offset.vz = 20.0f ;
					if ( pad & PAD_D ) offset.vz = -20.0f ;
					_sceVu0ApplyMatrix( &work->pos, &work->spot.world, &offset );
				}
				if ( pad & PAD_X ) work->angle += add ;
				if ( pad & PAD_Y ) work->angle -= add ;
				if ( work->angle < add ) work->angle = add ;
				work->angle_cos = cosf( work->angle );
				work->angle_sin = sinf( work->angle );
			} else {
				int		a = 0 ;
				if ( pad & PAD_U ) a += 1 ;
				if ( pad & PAD_D ) a -= 1 ;
				if ( pad & PAD_A ) work->debug_r += a ;
				if ( pad & PAD_X ) work->debug_g += a ;
				if ( pad & PAD_B ) work->debug_b += a ;
				CLAMP( work->debug_r, 255, 0 );
				CLAMP( work->debug_g, 255, 0 );
				CLAMP( work->debug_b, 255, 0 );
				work->color = work->debug_r | ( work->debug_g << 8 ) | ( work->debug_b << 16 ) | ( (unsigned int)128 << 24 ) ;
			}
			DG_SetPos2( &work->pos, &work->rot );
			DG_GetPos( &mat );
			mat.m[0][0] = -mat.m[0][0] ;
			mat.m[0][1] = -mat.m[0][1] ;
			mat.m[0][2] = -mat.m[0][2] ;
			mat.m[1][0] = -mat.m[1][0] ;
			mat.m[1][1] = -mat.m[1][1] ;
			mat.m[1][2] = -mat.m[1][2] ;
			work->dir = *(FVECTOR*)mat.m[2] ;
			work->spot.world = mat ;
			_sceVu0AddVector( &vec2, &work->pos, &work->dir );
			vec1 = work->pos ;
			vec1.vw = 0.0f ;
			//DG_SetCamera2( DG_Chanl(0), &vec1, &vec2, 1.0 );
         BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
			DG_SetCamera2( DG_Chanl(0), &vec1, &vec2, work->angle_cos / sinf( work->angle ) );
			work->spot.angle = work->angle_cos / sinf( work->angle ) ;
			work->spot.color = work->color ;
		}
	}
#endif

	/* スネーク影通知チェック */
	CheckSnakeShadow( work );
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	DG_DequeueSpotObjs( &work->spot );
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int name, int where )
{
	static FVECTOR		dir_base = {0.0f, -1.0f, 0.0f, 0.0f };
	//float	umbra = 0, penumbra = 0 ;
	float	range = 0, angle = 0 ;
   FVECTOR	pos = { 0 }, dir = { 0 };
   FMATRIX	mat = { { 0 } };
   SVECTOR	rot = { 0 };
	unsigned int		color = 0, option = 0 ;

	work->name = name ;
	work->where = where ;

	/* オプション指定 */
    if ( GCL_GetOption( 'o' ) != NULL ) {
		work->option = option = GCL_GetNextInt() ;
	}
	/* 設置座標 */
    if ( GCL_GetOption( 'p' ) != NULL ) {
		pos.vx = (float)GCL_GetNextInt() ;
		pos.vy = (float)GCL_GetNextInt() ;
		pos.vz = (float)GCL_GetNextInt() ;
		pos.vw = 1.0f ;
	}
	/* スポットライト方向 */
    if ( GCL_GetOption( 'd' ) != NULL ) {
		if ( !( option & 0x0001 ) ){
			dir.vx = (float)GCL_GetNextInt() ;
			dir.vy = (float)GCL_GetNextInt() ;
			dir.vz = (float)GCL_GetNextInt() ;
			dir.vw = 0.0f ;
			_sceVu0Normalize( &dir, &dir );
			rot = DG_ZeroSVector ;
		} else {
			rot.vx = GCL_GetNextInt() ;
			rot.vy = GCL_GetNextInt() ;
			rot.vz = GCL_GetNextInt() ;
			DG_SetPos2( &pos, &rot );
			DG_GetPos( &mat );
			mat.m[0][0] = -mat.m[0][0] ;
			mat.m[0][1] = -mat.m[0][1] ;
			mat.m[0][2] = -mat.m[0][2] ;
			mat.m[1][0] = -mat.m[1][0] ;
			mat.m[1][1] = -mat.m[1][1] ;
			mat.m[1][2] = -mat.m[1][2] ;
			dir = *(FVECTOR*)mat.m[2] ;
		}
	}
	/* スポットライト有効距離 */
    if ( GCL_GetOption( 'r' ) != NULL ) {
		range = (float)GCL_GetNextInt() ;
	}
	/* スポットライト有効角度 */
    if ( GCL_GetOption( 'a' ) != NULL ) {
		angle = (float)GCL_GetNextInt() ;
		angle = angle * 3.14159265f / 180.0f ;
	}
	/* フェードイン・アウト用コールバック */
    if ( GCL_GetOption( 'f' ) != NULL ) {
		work->fade_proc = GCL_GetNextInt() ;
	}
	/* スポットライト光源色 */
    if ( GCL_GetOption( 'c' ) != NULL ) {
		unsigned int	r, g, b ;
		r = GCL_GetNextInt() ;
		g = GCL_GetNextInt() ;
		b = GCL_GetNextInt() ;
		color = r | ( g << 8 ) | ( b << 16 ) | ( (unsigned int)128 << 24 ) ;
#ifdef DEBUG_MODE
		work->debug_r = r ;
		work->debug_g = g ;
		work->debug_b = b ;
#endif
	}
#ifdef DEBUG_MODE
	/* デバッグオプション */
    if ( option & 0x8000 ) {
		/* 1で２コンＬ１を押しながら操作 */
		work->debug_flag = 1 ;
	}
#if 0
	{
		static int a = 0 ;
		if ( a == 0 ){
			work->debug_flag = 1 ;
			a = 1 ;
		}
	}
#endif
#endif
	work->angle_cos = cosf( angle );
	work->angle_sin = sinf( angle );
	work->pos = pos ;
	work->dir = dir ;
	work->rot = rot ;
	work->umbra = 1.0f ;/*cosf( 0.0f )*/
	work->penumbra = work->angle_cos ; /*cosf( angle )*/
	work->range = range ;
	work->angle = angle ;
	work->color = color ;

	if ( !( option & 0x0001 ) ){
		*(FVECTOR*)work->spot.world.m[3] = pos ;
		*(FVECTOR*)work->spot.world.m[2] = dir ;
		_sceVu0OuterProduct( work->spot.world.m[0], work->spot.world.m[2], &dir_base );
		_sceVu0OuterProduct( work->spot.world.m[1], work->spot.world.m[2], work->spot.world.m[0] );
		work->spot.world.m[0][3] = 0.0f ;
		work->spot.world.m[1][3] = 0.0f ;
		work->spot.world.m[2][3] = 0.0f ;
		work->spot.world.m[3][3] = 1.0f ;
	} else {
		work->spot.world = mat ;
	}
	work->spot.shadow_id = name ;
	DG_QueueSpotObjs( &work->spot );

	work->spot.range = range ;
	work->spot.angle = work->angle_cos / work->angle_sin ;
	work->spot.color = color ;

	work->world = work->spot.world ;	/* 揺らし効果で使用するので保存 */
	{
		float f ;
		f = - ( 0.5f / 180.0f * 3.14159265f ) ;
		_sceVu0RotMatrixY( &work->world, &work->world, f );
	}
	work->enable = !( option & 0x0002 ) ;

	GC_ProjectionSpot_HitLights = 0 ;
	GC_ProjectionSpot_SnakeCheckList = SpotList ;

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewPutStaticSpotLight( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
