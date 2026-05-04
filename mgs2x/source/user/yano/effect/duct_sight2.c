//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ダクト内2Dエフェクト(ポリゴン作成)
    duct_sight2.c
    
    2001/07/17 Y.YANO
    
    $Id: duct_sight2.c,v 1.1.1.3 2002/11/19 11:51:56 Yoshizawa1 Exp $
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

#include        "libutl.h"
#include        "libdg.h"
#include        "libdg.cnf"
#include        "libmt.h"
#include        "gameheader.h"

//#include      "camera.h"
#include        "sprite_2d.h"

#include        "../etc/yn_utl.h"

#define FADE 1
#define SUB_FADE 0/*減算 αフラグは自分で変えて*/

#define SET  ( 2 )
#define POLY ( 32 )

#define DIFF_SCALE (150.0F)
#define SCALE_X 20.0F
#define SCALE_Y 15.0F

#define INNER1_X (220.0F)
#define INNER1_Y (165.0F)
#define INNER2_X (140.0F)/* 4 : 3 */
#define INNER2_Y (105.0F)/*       */
#define OFFSET_X (256.0F)
#define OFFSET_Y (192.0F)

#define MAX_ALPHA  ( 0x50 )
#define MAX_ALPHA2 ( 0x80 )
#define MAX_ALPHA3 ( 0x80 )

#define FADE_TIME ( 13 )
//#define FADE_UNIT ( 10 )/*変数で処理*/
#define FADE_U0_RATE ((float)WWW-0.05F) / (float)FADE_TIME
#define FADE_V0_RATE ((float)HHH-0.2F) / (float)FADE_TIME

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))

typedef struct {
    GV_ACT_EX               actor;
    SPR_OBJ                 *poly[ POLY * SET ];

    int fade_flag ;/* 1:フェードイン -1:フェードアウト 0:通常表示 */
    int act_cnt ;

    FVECTOR scale ;

} Work;

static Work *DuctSight2DWorkMemory2 = NULL ;

static void SetVertsPos( SPR_POS *pos , FVECTOR *scale )
{
    SPR_POS point[3][ SET * POLY ];
    int i , k ;
    float angle ;
    SPR_POS * _pos = pos ;

    angle = TPI / (float)POLY ;

    /* 一番外、、 *//* 一辺が 640 で考える */
    /* 中 */        /* ( 256*cos , 192*sin )ぐらい */
    /* 中心近く*/   /* テキトー */
    for( i = 0 ; i < POLY ; i ++ ){
	float c , s ;
	c = cos( i * angle );
	s = sin( i * angle ); 
	point[0][ i ].x = 480.0F * c + OFFSET_X ;
	point[0][ i ].y = 360.0F * s + OFFSET_Y ;
	//point[0][ i ].x = 360.0F * c + OFFSET_X ;
	//point[0][ i ].y = 270.0F * s + OFFSET_Y ;
	//point[0][ i ].x = 320.0F * c + OFFSET_X ;
	//point[0][ i ].y = 240.0F * s + OFFSET_Y ;
	point[1][ i ].x = scale->vx * c + OFFSET_X ;
	point[1][ i ].y = scale->vy * s + OFFSET_Y ;
	point[2][ i ].x = scale->vz * c + OFFSET_X ;
	point[2][ i ].y = scale->vw * s + OFFSET_Y ;
    }

    /* 実際に 4 頂点に組み込む */
    for( i = 0 ; i < POLY ; i ++ ){/*内側*/
	k = i + 1 ;
	if( k >= POLY ){
	    k = 0 ;
	}
	pos[0] = point[2][ i ];
	pos[1] = point[2][ k ];
	pos[2] = point[1][ k ];
	pos[3] = point[1][ i ];

	pos += 4 ;
    }
    for( i = 0 ; i < POLY ; i ++ ){/*外側*/
	k = i + 1 ;
	if( k >= POLY ){
	    k = 0 ;
	}
	pos[0] = point[1][ i ];
	pos[1] = point[1][ k ];
	pos[2] = point[0][ k ];
	pos[3] = point[0][ i ];

	pos += 4 ;
    }

    pos = _pos ;
}       

static void Act( Work *work )
{
    int i ;
    //SPR_OBJ **poly = (SPR_OBJ **)work->poly ;
    SPR_POS pos_s[ SET * POLY * 4 ];
    SPR_POS *pos ;

    pos = pos_s ;
        
    if ( GM_CheckSightStatus( SGT_Invisible ) ) {
	/* このときは強制的に非表示 */
	for( i = 0 ; i < POLY*SET ; i ++ ){
	    SPR_HIDE( work->poly[i] );
	}
    }

    switch( work->fade_flag ){
      case 1:/* FADE_IN */
	if( work->act_cnt <= FADE_TIME ){
	    work->scale.vx -= SCALE_X ;
	    work->scale.vy -= SCALE_Y ;
	    work->scale.vz -= SCALE_X ;
	    work->scale.vw -= SCALE_Y ;
	    SetVertsPos( pos , &work->scale );

	    for( i = 0 ; i < POLY*SET ; i ++ ){
		SPR_SetPosPoly( work->poly[i] , pos );/* pos は 4 頂点分必要 */
		pos += 4 ;
	    }
	}else{
	    work->act_cnt = 0 ;
	    work->fade_flag = 0 ;
	}
	break ;
#if 1
      case -1:/* FADE_OUT */
	if( work->act_cnt <= FADE_TIME ){
	    work->scale.vx += SCALE_X ;
	    work->scale.vy += SCALE_Y ;
	    work->scale.vz += SCALE_X ;
	    work->scale.vw += SCALE_Y ;
	    SetVertsPos(  pos , &work->scale );
	    for( i = 0 ; i < POLY*SET ; i ++ ){
		SPR_SetPosPoly( work->poly[i] , pos );/* pos は 4 頂点分必要 */
		pos += 4 ;
	    }
	}else{
	    GV_DestroyActor( work );
	}
	break ;

#endif

      case 0:/* NORMAL */
	work->act_cnt = 0 ;
	break ;

      default:/* ERR */
	printf("ERR!! switch in duct_sight.c(yano)\n ");
	break ;
    }

    work->act_cnt ++ ;
}

static void Die( Work *work )
{
    int i;
    for( i = 0 ; i < POLY*SET ; i++ ){
	if(work->poly[i]){
	    SPR_Destroy_2D_Object( work->poly[i] );
	}
    }
    DuctSight2DWorkMemory2 = NULL ;/*死んでいる時はNULL にしておく*/
}


static int GetResources( Work *work , int mode , int camera_chanl )
{
    SPR_OBJ *poly ;
    int             i , j ;
    SPR_POS pos_s[ SET * POLY * 4 ];
    SPR_POS *pos = pos_s ;

    /* 横512  縦384 で設定 */

    /* 座標計算 */
    work->scale.vx = 360.0F + DIFF_SCALE ;
    work->scale.vy = 270.0F + DIFF_SCALE*3.0F/4.0F ;
    work->scale.vz = 360.0F ;
    work->scale.vw = 270.0F ;
    SetVertsPos( pos , &work->scale );

    /* 初期化 */
    for( j = 0 ; j < POLY ; j ++ ){
	poly = work->poly[ j ] = SPR_Create_2D_Object( SP_POLY , DG_CHANL_MAIN , NULL );
	if( poly == NULL ){
	    printf("ERR!! Make2DObj[%d] 'yano/effect/duct_sight.c' \n" , j );
	    return -1 ;
	}
	/* 座標 */
	SPR_SetPosPoly(poly, pos );/* pos は 4 頂点分必要 */
	pos += 4 ;
	/* 色 */
	SPR_SetColorPoly( poly , 0 , 0  , 0 , 0 , 0 );
	SPR_SetColorPoly( poly , 1 , 0  , 0 , 0 , 0 );
	SPR_SetColorPoly( poly , 2 , 0  , 0 , 0 , MAX_ALPHA );
	SPR_SetColorPoly( poly , 3 , 0  , 0 , 0 , MAX_ALPHA );
	poly->head.alpha = SCE_GS_SET_ALPHA( 0 , 1 , 0 , 1 , 0 ); /* 減算の方がいいかも */
	poly->head.flags |= SPR_FLAG_ALPHA ;
    }
    for( j = 0 ; j < POLY ; j ++ ){
	poly = work->poly[ j + POLY ] = SPR_Create_2D_Object( SP_POLY , DG_CHANL_MAIN , NULL );
	if( poly == NULL ){
	    printf("ERR!! Make2DObj[%d] 'yano/effect/duct_sight.c' \n" , j + 16 );
	    return -1 ;
	}
	/* 座標 */
	SPR_SetPosPoly( poly, pos );/* pos は 4 頂点分必要 */
	pos += 4 ;
	/* 色  */
	SPR_SetColorPoly( poly , 0 , 0  , 0 , 0 , MAX_ALPHA );
	SPR_SetColorPoly( poly , 1 , 0  , 0 , 0 , MAX_ALPHA );
	SPR_SetColorPoly( poly , 2 , 0  , 0 , 0 , MAX_ALPHA2 );
	SPR_SetColorPoly( poly , 3 , 0  , 0 , 0 , MAX_ALPHA2 );
	poly->head.alpha = SCE_GS_SET_ALPHA( 0 , 1 , 0 , 1 , 0 ); /* 減算の方がいいかも */
	poly->head.flags |= SPR_FLAG_ALPHA ;
    }
        
    for( i = 0 ; i < SET ; i ++ ){/* 表示開始 */
	for( j = 0 ; j < POLY ; j ++ ){
	    SPR_SHOW( work->poly[ j + i*POLY ] );
	}
    }

    work->fade_flag = 1 ;
        
    return 0;
}

/*
  mode: 1(非表示) 0（起動,表示） -1（殺）
*/
void *NewIntrudeInDuct2D_2( int mode , int camera_chanl )
{
    Work            *work ;
    int i ;

    if( mode < 0 ){/*殺*/
	if( DuctSight2DWorkMemory2 != NULL ){
	    //printf("D 殺す\n");
	    //GV_DestroyActor( DuctSight2DWorkMemory2 );
	    DuctSight2DWorkMemory2->fade_flag = -1 ;
	    DuctSight2DWorkMemory2->act_cnt = 0 ;
	}
	return NULL ; 
    }else if( mode > 0 ){/*非表示*/
	if( DuctSight2DWorkMemory2 != NULL ){
	    //printf("D 非表示\n");
	    for( i = 0 ; i < SET*POLY ; i ++ ){
		SPR_HIDE( DuctSight2DWorkMemory2->poly[i] );
	    }
	}
	return NULL;
    }
    /*以下、起動または表示の場合*/
    if( DuctSight2DWorkMemory2 != NULL ){/*表示*/
	//printf("D 表示\n");
	for( i = 0 ; i < SET*POLY ; i ++ ){
	    SPR_SHOW( DuctSight2DWorkMemory2->poly[i] );
	}
	return NULL;
    }
        
    work = (Work *)GV_NewActor( GV_ACTOR_EFFECT , sizeof( Work ) ) ;
    if ( work != NULL ) {
	//printf("D 起動\n");
	GV_SetActor( &( work->actor ), Act, Die );
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, mode, camera_chanl ) < 0 ) {
	    GV_DestroyActor( work );
	    return NULL;
	}
	DuctSight2DWorkMemory2 = work ;
    }

    return ( void * )work ;
}
