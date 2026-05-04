//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ゴルルコ兵ライデン主観2Dエフェクト
    goruchan.c
    
        2001/07/13 Y.YANO

        $Id: goruruko.c,v 1.1.1.3 2002/11/19 11:51:57 Yoshizawa1 Exp $
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


#define TEX_STRCODE 16492416/*raiden_gbssub_alp_ovl*/
#define TRI_CODE    6061498 /*goruuuu*/

#define U0	0.3F
#define V0	0.25F
#define U1	1.0F
#define V1	1.0F
#define WIDTH	(U1-U0)
#define HEIGHT	(V1-V0)

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))

typedef struct {
    GV_ACT_EX               actor;
    SPR_OBJ                 *sprite[4];
} Work;

static Work *GoruRaiWorkMemory = NULL ;

static void Act( Work *work )
{
    if ( GM_CheckSightStatus( SGT_Invisible ) ) {
	/* このときは強制的に非表示 */
	SPR_HIDE( work->sprite[0] );
	SPR_HIDE( work->sprite[1] );
	SPR_HIDE( work->sprite[2] );
	SPR_HIDE( work->sprite[3] );
    }
}

static void Die( Work *work )
{
    int i;
    for( i = 0 ; i < 4 ; i++ ){
	if(work->sprite[i]){
	    SPR_Destroy_2D_Object( work->sprite[i] );
	}
    }
    GoruRaiWorkMemory = NULL ;/*死んでいる時はNULL にしておく*/
}

static void     SetSpriteUV( SPR_SPRITE *prim , FVECTOR *data )
{
    SPR_TEX         *tex = &prim->head.tex ;
    float           u , v , h , w ;
    
#if 0 //BP_PS2 def PSX2  //yano 2002.02.22	
    u = (tex->u >> 4) + (tex->u << 12 >> 12) * 0.0001 ;/*実数で考えるために、float に*/
    v = (tex->v >> 4) + (tex->v << 12 >> 12) * 0.0001 ;
    w = (tex->w >> 4) + (tex->w << 12 >> 12) * 0.0001 ;
    h = (tex->h >> 4) + (tex->h << 12 >> 12) * 0.0001 ;
    tex->u = ( (int)( u + w * data->vx ) <<4 );
    tex->v = ( (int)( v + h * data->vy ) <<4 );
    tex->w = ( (int)( w * data->vz ) << 4);
    tex->h = ( (int)( h * data->vw ) << 4);
#else
    u = tex->u;/*実数で考えるために、float に*/
    v = tex->v; 
    w = tex->w; 
    h = tex->h; 
    tex->u = SPR_FIXED( u + w * data->vx );
    tex->v = SPR_FIXED( v + h * data->vy );
    tex->w = SPR_FIXED( w * data->vz );
    tex->h = SPR_FIXED( h * data->vw );
#endif
}

static int GetResources( Work *work , int mode , int camera_chanl )
{
    /* 横512  縦384 で設定 */
    FVECTOR SpriteData[4] = {/*vx-->U ,vy-->V ,vz-->幅 , vw-->高さ*/
	{ 0.0F   , 0.0F   , 256.0F , 192.0F },
	{ 256.0F , 0.0F   , 256.0F , 192.0F },
	{ 0.0F   , 192.0F , 256.0F , 192.0F },
	{ 256.0F , 192.0F , 256.0F , 192.0F },
    };
#if 0
    FVECTOR TexData[4] = {/* uv値を自由に設定するためのデータ */
		{ 0.0F , 0.0F , 1.0F , 1.0F },
		{ 1.0F , 0.0F ,-1.0F , 1.0F },
		{ 0.0F , 1.0F , 1.0F ,-1.0F },
		{ 1.0F , 1.0F ,-1.0F ,-1.0F },
    };
#else
    FVECTOR TexData[4] = {/* uv値を自由に設定するためのデータ */
		{ U0 , V0 , WIDTH , HEIGHT },
		{ U1 , V0 ,-WIDTH , HEIGHT },
		{ U0 , V1 , WIDTH ,-HEIGHT },
		{ U1 , V1 ,-WIDTH ,-HEIGHT },
    };
#endif
    
    FVECTOR *data = SpriteData ;
    FVECTOR *TData = TexData ;
    SPR_OBJ *sprite ;
    int             i ;
    int             handle;
    handle = SPR_LoadTexture( TRI_CODE ) ;/* tri を選択 */
    if( handle == -1 ){
	return -1 ;
    }
    for( i = 0 ; i < 4 ; i++ ){
	sprite = work->sprite[i] = SPR_Create_2D_Object( SP_SPRITE , DG_CHANL_MAIN , NULL );
	if( sprite == NULL ){
	    printf("ERR!! Make2DObj[%d] 'yano/effect/goruruko.c' \n" , i );
	    return -1 ;
	}
	SPR_ObjSetTexture( sprite , TEX_STRCODE , handle );/* テクスチャの uvtwth の設定行なわれる */
	SetSpriteUV( (SPR_SPRITE*)sprite , (FVECTOR*)TData );/*上で設定されたuvwhの値を変更*/
	SPR_SetPosSprite( sprite , (SPR_POS*)data );/* 開始点設定 */
	SPR_SetSizeSprite( sprite , data->vz , data->vw );/* 幅、高さの設定 */
	sprite->head.alpha = SCE_GS_SET_ALPHA( 0 , 1 , 0 , 1 , 0 );
	sprite->head.flags |= SPR_FLAG_ALPHA ;
	//SET_COLOR_2DPRIM( sprite , 0x80808080 );/* 頂点カラーの設定 */
	SET_COLOR_2DPRIM( sprite , 0x60808080 );/* 頂点カラーの設定 */
	
	data ++ ;
	TData ++ ;
    }
    
    SPR_SHOW( work->sprite[0] );
    SPR_SHOW( work->sprite[1] );
    SPR_SHOW( work->sprite[2] );
    SPR_SHOW( work->sprite[3] );
    
    return 0;
}

/*
  mode: 1(非表示) 0（起動,表示） -1（殺）
*/
void *NewGoruRaiSub2D( int mode , int camera_chanl )
{
    Work            *work ;

    if( mode < 0 ){/*殺*/
	if( GoruRaiWorkMemory != NULL ){
	    GV_DestroyActor( GoruRaiWorkMemory ) ;
	}
	GoruRaiWorkMemory = NULL ;
	return NULL ;
    }else if( mode > 0 ){/*非表示*/
	if( GoruRaiWorkMemory != NULL ){
	    SPR_HIDE( GoruRaiWorkMemory->sprite[0] );
	    SPR_HIDE( GoruRaiWorkMemory->sprite[1] );
	    SPR_HIDE( GoruRaiWorkMemory->sprite[2] );
	    SPR_HIDE( GoruRaiWorkMemory->sprite[3] );
	}
	return NULL;
    }
    /*以下、起動または表示の場合*/
    if( GoruRaiWorkMemory != NULL ){/*表示*/
	SPR_SHOW( GoruRaiWorkMemory->sprite[0] );
	SPR_SHOW( GoruRaiWorkMemory->sprite[1] );
	SPR_SHOW( GoruRaiWorkMemory->sprite[2] );
	SPR_SHOW( GoruRaiWorkMemory->sprite[3] );
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
	GoruRaiWorkMemory = work ;
    }

    return ( void * )work ;
}
