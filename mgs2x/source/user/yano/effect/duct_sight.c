//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ダクト1内2Dエフェクト
    duct_sight.c
    
    2001/07/13 Y.YANO
    
    $Id: duct_sight.c,v 1.1.1.3 2002/11/19 11:51:56 Yoshizawa1 Exp $
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
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

//#include	"camera.h"
#include 	"sprite_2d.h"

#include	"../etc/yn_utl.h"

#define TEX_STRCODE 16492416/*raiden_gbssub_alp_ovl*/
#define TRI_CODE    6061498 /*goruuuu*/

#if 0

#define UUU0 (0.0F)
#define VVV0 (0.2F)
#define UUU1 (0.2F)
#define VVV1 (1.0F)
#else
#define UUU0 (0.0F)
#define VVV0 (0.0F)
#define UUU1 (1.0F)
#define VVV1 (1.0F)

#endif

#define WWW  (UUU1-UUU0)
#define HHH  (VVV1-VVV0)

#define FADE_TIME 30
#define FADE_U0_RATE ((float)WWW-0.05F) / (float)FADE_TIME
#define FADE_V0_RATE ((float)HHH-0.2F) / (float)FADE_TIME

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))

typedef	struct {
	GV_ACT_EX		actor;
	SPR_OBJ   		*sprite[4];

	int fade_flag ;/* 1:フェードイン -1:フェードアウト 0:通常表示 */
	int act_cnt ;

	float 		u , v , h , w ;/* おおもとのテクスチャ(ゴル)の大きさ */

} Work;

static Work *DuctSight2DWorkMemory = NULL ;

static void	SetSpriteUV2( Work * work , SPR_SPRITE *prim , FVECTOR *data )
{
	SPR_TEX		*tex = &prim->head.tex ;

	tex->u = SPR_FIXED( work->u + work->w * data->vx );
	tex->v = SPR_FIXED( work->u + work->h * data->vy );
	tex->w = SPR_FIXED( work->w * data->vz );
	tex->h = SPR_FIXED( work->h * data->vw );
}

static void Act( Work *work )
{
	FVECTOR modify_uv[4] ;
	int i ;
	float uuu0 =0 , vvv0 =0 , www =0 , hhh =0 ;

	//printf("u0  %f , v0 %f \n" , FADE_U0_RATE , FADE_V0_RATE );
	
	if ( GM_CheckSightStatus( SGT_Invisible ) ) {
		/* このときは強制的に非表示 */
		SPR_HIDE( work->sprite[0] );
		SPR_HIDE( work->sprite[1] );
		SPR_HIDE( work->sprite[2] );
		SPR_HIDE( work->sprite[3] );
	}

	switch( work->fade_flag ){
	  case 1:
		uuu0 = FADE_U0_RATE * (float)( FADE_TIME - work->act_cnt ) + UUU0 ;
		vvv0 = FADE_V0_RATE * (float)( FADE_TIME - work->act_cnt ) + VVV0 ;
		//printf("uuu0 = %f , vvv0 = %f \n" , uuu0 , vvv0 );
		www  = UUU1 - uuu0 ;
		hhh  = VVV1 - vvv0 ;
		if( work->act_cnt >= FADE_TIME ){
			work->act_cnt = 0 ;
			work->fade_flag = 0 ;
		}
		break ;
		
	  case -1:
		uuu0 = FADE_U0_RATE * (float)work->act_cnt + UUU0 ;
		vvv0 = FADE_V0_RATE * (float)work->act_cnt + VVV0 ;
		//printf("uuu0 = %f , vvv0 = %f \n" , uuu0 , vvv0 );
		www  = UUU1 - uuu0 ;
		hhh  = VVV1 - vvv0 ;
		if( work->act_cnt >= FADE_TIME ){
			GV_DestroyActor( work );
		}
		break ;

	  case 0:
		uuu0 = UUU0 ;
		vvv0 = VVV0 ;
		www  = UUU1 - uuu0 ;
		hhh  = VVV1 - vvv0 ;
		//printf("uuu0 = %f , vvv0 = %f \n" , uuu0 , vvv0 );
		work->act_cnt = 0 ;
		break ;

	  default:
		printf("ERR!! switch in duct_sight.c(yano)\n ");
		break ;
	}

	modify_uv[0].vx = uuu0 ;
	modify_uv[0].vy = vvv0 ;
	modify_uv[0].vz = www ;
	modify_uv[0].vw = hhh ;
	modify_uv[1].vx = UUU1 ;
	modify_uv[1].vy = vvv0 ;
	modify_uv[1].vz = -www ;
	modify_uv[1].vw = hhh ;
	modify_uv[2].vx = uuu0 ;
	modify_uv[2].vy = VVV1 ;
	modify_uv[2].vz = www ;
	modify_uv[2].vw = -hhh ;
	modify_uv[3].vx = UUU1 ;
	modify_uv[3].vy = VVV1 ;
	modify_uv[3].vz = -www ;
	modify_uv[3].vw = -hhh ;

	for( i = 0 ; i < 4 ; i ++ ){
		SetSpriteUV2( work , (SPR_SPRITE*)work->sprite[i] , (FVECTOR*)(&modify_uv[i]) );
	}
	work->act_cnt ++ ;
}

static void Die( Work *work )
{
	int i;
	for( i = 0 ; i < 4 ; i++ ){
		if(work->sprite[i]){
			SPR_Destroy_2D_Object( work->sprite[i] );
		}
	}
	DuctSight2DWorkMemory = NULL ;/*死んでいる時はNULL にしておく*/
}

static void	SetSpriteUV( Work *work , SPR_SPRITE *prim , FVECTOR *data )
{
	SPR_TEX		*tex = &prim->head.tex ;

#if 0 //BP_PS2 def PSX2 //yano 2001.02.22
	work->u = (tex->u >> 4) + (tex->u << 12 >> 12) * 0.0001 ;/*実数で考えるために、float に*/
	work->v = (tex->v >> 4) + (tex->v << 12 >> 12) * 0.0001 ;
	work->w = (tex->w >> 4) + (tex->w << 12 >> 12) * 0.0001 ;
	work->h = (tex->h >> 4) + (tex->h << 12 >> 12) * 0.0001 ;
	tex->u = SPR_FIXED( work->u + work->w * data->vx );
	tex->v = SPR_FIXED( work->u + work->h * data->vy );
	tex->w = SPR_FIXED( work->w * data->vz );
	tex->h = SPR_FIXED( work->h * data->vw );
#else
	work->u = tex->u ;
	work->v = tex->v ;
	work->w = tex->w ;
	work->h = tex->h ;
	tex->u = SPR_FIXED( work->u + work->w * data->vx );
	tex->v = SPR_FIXED( work->u + work->h * data->vy );
	tex->w = SPR_FIXED( work->w * data->vz );
	tex->h = SPR_FIXED( work->h * data->vw );
#endif
}

static int GetResources( Work *work , int mode , int camera_chanl )
{
	FVECTOR	*data ;
	FVECTOR *TData ;
	SPR_OBJ	*sprite ;
	int		i ;
	int		handle;

    /* 横512  縦384 で設定 */
    FVECTOR SpriteData[4] = {/*vx-->U ,vy-->V ,vz-->幅 , vw-->高さ*/
		{ 0.0F   , 0.0F   , 256.0F , 192.0F },
		{ 256.0F , 0.0F   , 256.0F , 192.0F },
		{ 0.0F   , 192.0F , 256.0F , 192.0F },
		{ 256.0F , 192.0F , 256.0F , 192.0F },
	};
	FVECTOR TexData[4] = {/*uv値を自由に設定するためのデータ*/
		{ UUU1 , VVV1 , 0.0F , 0.0F },
		{ UUU1 , VVV1 , 0.0F , 0.0F },
		{ UUU1 , VVV1 , 0.0F , 0.0F },
		{ UUU1 , VVV1 , 0.0F , 0.0F },	
    };
	data = SpriteData ;
	TData = TexData ;

	handle = SPR_LoadTexture( TRI_CODE ) ;/* tri を選択 */
	if( handle == -1 ){
		return -1 ;
	}
	for( i = 0 ; i < 4 ; i++ ){
		sprite = work->sprite[i] = SPR_Create_2D_Object( SP_SPRITE , DG_CHANL_MAIN , NULL );
		if( sprite == NULL ){
			printf("ERR!! Make2DObj[%d] 'yano/effect/duct_sight.c' \n" , i );
			return -1 ;
		}
		SPR_ObjSetTexture( sprite , TEX_STRCODE , handle );/* テクスチャの uvtwth の設定行なわれる */
		SetSpriteUV( work , (SPR_SPRITE*)sprite , (FVECTOR*)TData );/*上で設定されたuvwhの値を変更*/
		SPR_SetPosSprite( sprite , (SPR_POS*)data );/* 開始点設定 */
		SPR_SetSizeSprite( sprite , data->vz , data->vw );/* 幅、高さの設定 */
		sprite->head.alpha = SCE_GS_SET_ALPHA( 0 , 1 , 0 , 1 , 0 );
		sprite->head.flags |= SPR_FLAG_ALPHA ;
		SET_COLOR_2DPRIM( sprite , 0x60808080 );/* 頂点カラーの設定 */

		data ++ ;
		TData ++ ;
	}

	SPR_SHOW( work->sprite[0] );
	SPR_SHOW( work->sprite[1] );
	SPR_SHOW( work->sprite[2] );
	SPR_SHOW( work->sprite[3] );

	work->fade_flag = 1 ;

	return 0;
}

/*
  mode: 1(非表示) 0（起動,表示） -1（殺）
*/
void *NewIntrudeInDuct2D( int mode , int camera_chanl )
{
	Work		*work ;

	if( mode < 0 ){/*殺*/
		if( DuctSight2DWorkMemory != NULL ){
			DuctSight2DWorkMemory->fade_flag = -1 ;
			//DuctSight2DWorkMemory->act_cnt = 0 ;
		}
		return NULL ; 
	}else if( mode > 0 ){/*非表示*/
		if( DuctSight2DWorkMemory != NULL ){
			SPR_HIDE( DuctSight2DWorkMemory->sprite[0] );
			SPR_HIDE( DuctSight2DWorkMemory->sprite[1] );
			SPR_HIDE( DuctSight2DWorkMemory->sprite[2] );
			SPR_HIDE( DuctSight2DWorkMemory->sprite[3] );
		}
		return NULL;
	}
	/*以下、起動または表示の場合*/
	if( DuctSight2DWorkMemory != NULL ){/*表示*/
		SPR_SHOW( DuctSight2DWorkMemory->sprite[0] );
		SPR_SHOW( DuctSight2DWorkMemory->sprite[1] );
		SPR_SHOW( DuctSight2DWorkMemory->sprite[2] );
		SPR_SHOW( DuctSight2DWorkMemory->sprite[3] );
		return NULL;
	}
	
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT , sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, mode, camera_chanl ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		DuctSight2DWorkMemory = work ;
	}

	return ( void * )work ;
}
