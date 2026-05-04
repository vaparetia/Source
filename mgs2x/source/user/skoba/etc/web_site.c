//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   web_site.c
   ウェブサイト
   
   2001/07/18	S.Kobayashi
   $Id: web_site.c,v 1.2 2002/12/05 18:41:16 takaki Exp $
*/

#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include    "font.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"
#include    "../test/etc.h"
#include    "../../mode/codec/cjimaku.h"
#include    "game.h"
#include "libfs.h"

#include "mode/menu/xtextscn.h"

#define 	STR_GAMEOVER (9221682) // 特殊GameOver tri file

#define IN_FADE_SPEED (6)
#define OUT_FADE_SPEED (12)
#define SK_R (128)
#define SK_G (128)
#define SK_B (128)
#define SK_A (128)
#define SCREEN_DEFAULT_POS_X  (33)
#define SCREEN_DEFAULT_POS_Y  (29)
#define SCREEN_DEFAULT_WIDTH  (203)
#define SCREEN_DEFAULT_HEIGHT (152)
#define FONT_MAX (2)
#define PLANT_BOMB_TEX (11726393)
#define PLANT_BOMB16 (13768267)

#define END_LIMIT ( DIRECT_TICK(120) )

enum {
	SK_NORMAL               = 0x00001,
	SK_SELECT_OK            = 0x00002,
	SK_JPEG_LOAD            = 0x00004,
	SK_PLANT_SPRITE_OK      = 0x00008,
	SK_MIST_DATA            = 0x00010,
	SK_END                  = 0x00020,
};

typedef struct {
	void *data; // jpeg date
	float x , y; // 始点
	float w , h; // 幅
	u_char r , g , b , a; // 色
} Photo;

typedef struct {
	void   *mini_screen;
	SPR_OBJ *menu_line;
	FVECTOR xy1;
	FVECTOR xy2;
	FVECTOR last_xy1;
	FVECTOR last_xy2;
	u_char r , g , b , a;
} Screen;

typedef struct {
	int tex_handle;
	int strcode_tex;
	SPR_OBJ *obj;
} SK_Sprite;

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	int                 position;
	SPR_OBJ             *special_view;
	Photo               photo[ 2 ];
	int                 tex_handle;
	int                 action;
	u_int               flag;
	int                 pad_status;
	int                 pad_check;
	int                 hold_time;
	int                 name;
	int 				timer;
	int                 mode;
	int                 strcode_tri;
	SK_Sprite           sprite[ 2 ]; // max 2
	int                 proc_next;
#ifndef PSX2
	DG_TEX_LIN			*tex ;
#endif
	void  ( *act )( struct _work * );
} Work ;

// プロトタイプ
// extern 
extern void SK_PrintfChengColor( SPR_OBJ *pObj , u_char r , u_char g , u_char b , u_char a );
extern void SK_Printf( u_char ascci , SPR_OBJ *pObj , int u , int v , int str_width , int str_height );

extern void *NewTextScreenControl( void );
extern void MENU_ClearTextTexture( void *work );
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
extern int ChangePhotoInfoStart( int , int , void * );
// 
extern void *NewReduceScreenEffect( void );
extern void TAKABE_SetReduceScreenSize( void *work_ptr, int x1, int y1, int x2, int y2, int color );
extern void *NewPictureDrawManager( int prio );
extern void MENU_DrawPicture32( int x, int y, int w, int h, int color, void *image, int image_w, int image_h, int flag );
extern int UTL_JpegDecode( void *image, int width, int height, void *code );


#ifndef PSX2
/*XBOX*/
extern void MENU_DrawPictureTex( int x, int y, int w, int h, int color, DG_TEX_LIN *tex, int flag );
extern DG_TEX_LIN *DG_MakeLinerTexture( int width, int height, int format );
extern void DG_FreeLinerTexture( DG_TEX_LIN *tex );

#endif


#define STR_TRICODE       (3081455)
#define STR_WEB00_ALP_OVL (7077977)
#define STR_WEB01_ALP_OVL (7143513)
#define STR_WEB02_ALP_OVL (7209049)
#define DIE_FLAG (0x1)
// メッセージを受けとると殺す敵が死んだ時に利用
static int MsgDie( Work *pWork )
{
	GV_MSG		*msg;
	int      msg_num;
	int         name;

	msg_num = GV_ReceiveMessage( pWork->name , &msg );
	msg += msg_num - 1;
	name = 0;
	while( --msg_num >= 0 ){
		if ( msg->message[ 0 ] == DIE_FLAG ){
			GV_DestroyActor( pWork );
		}
		msg--;
	}
	return ( 0 );
}

#define DEFAULT_X (0.0f)
#define DEFAULT_Y (22.0f)

static void SpriteInit( Work *pWork )
{
	int   strcode;
	SPR_POS   pos;
 	// 特殊
    pWork->special_view = SPR_Create_2D_Object( SP_SPRITE , 4 , NULL );

	pos.x = DEFAULT_X;
	pos.y = DEFAULT_Y;
    /* 表示座標の設定 */
    SPR_SetPosSprite( pWork->special_view , &pos );

    /* 表示サイズの設定 */
//    SPR_SetSizeSprite( pWork->special_view , 512.0F, 280.0F);  // BP JG - fixed to match the other set of black bars.
    SPR_SetSizeSprite( pWork->special_view , 512.0F, 270.0F);  // BP JG - fixed to match the other set of black bars.

    /* テクスチャの設定 */
    pWork->tex_handle = SPR_LoadTexture( pWork->strcode_tri );
	if ( pWork->tex_handle < 0 ){
		SK_Err("non tri\0");
		ASSERT( 0 )
	}
	strcode = 0;
	switch ( pWork->mode ){
	case 0 : 
		strcode = STR_WEB00_ALP_OVL;
		break;
	case 1 : 
		strcode = STR_WEB01_ALP_OVL;
		break;
	case 2 : 
		strcode = STR_WEB02_ALP_OVL;
		break;
	}
	if ( SPR_ObjSetTexture( pWork->special_view , strcode , pWork->tex_handle ) < 0 ){
		SK_Err("non tex\0");
		ASSERT( 0 )
	}
    /* アルファブレンディングの設定 */
    pWork->special_view->head.alpha = SCE_GS_SET_ALPHA(1, 0, 1, 0, 64);
	pWork->special_view->sprite.col.a = 64;
	// プライオリティ 
	SPR_SetPriority( pWork->special_view , 0 );
}

static void PlantSpriteInit( Work *pWork )
{
	SPR_POS spr_pos;
	int           i;

	if ( pWork->flag & SK_PLANT_SPRITE_OK ){
		return;
	}
	for ( i = 0 ; i < ( pWork->mode == 2 ? 2 : 1 ) ; i ++ ){
	 	// 特殊
	    pWork->sprite[ i ].obj = SPR_Create_2D_Object( SP_SPRITE , 4 , NULL );
	    /* 表示座標の設定 */
		spr_pos.x = pWork->photo[ i ].x;
		spr_pos.y = pWork->photo[ i ].y;
	    SPR_SetPosSprite( pWork->sprite[ i ].obj , &spr_pos );
	    /* 表示サイズの設定 */
	    SPR_SetSizeSprite( pWork->sprite[ i ].obj , pWork->photo[ i ].w , pWork->photo[ i ].h );
	    /* テクスチャの設定 */
	    pWork->sprite[ i ].tex_handle = SPR_LoadTexture( pWork->strcode_tri );
		if ( pWork->sprite[ i ].tex_handle < 0 ){
			SK_Err("non tri\0");
			ASSERT( 0 )
		}
		if ( SPR_ObjSetTexture( pWork->sprite[ i ].obj , pWork->sprite[ i ].strcode_tex , pWork->tex_handle ) < 0 ){
			SK_Err("non tex\0");
			ASSERT( 0 )
		}
	    /* アルファブレンディングの設定 */
	    pWork->sprite[ i ].obj->head.alpha = SCE_GS_SET_ALPHA(1, 0, 1, 0, 64);
		// プライオリティ 
		SPR_SetPriority( pWork->sprite[ i ].obj , 2 );
		SPR_SHOW( pWork->sprite[ i ].obj );
	}
	pWork->flag |= SK_PLANT_SPRITE_OK;
}

#define JPEG_SIZE_LIMIT	 ( DRAW_WIDTH * DRAW_HEIGHT * 2 * sizeof(short) ) //( 24*1024 * sizeof(short) )
#define PHOTO_WIDTH ( 512 )
#define PHOTO_HEIGHT ( 128 )
#define PHOTO_MAIN_WIDTH ( 512 )
#define PHOTO_MAIN_HEIGHT ( 128 )

#ifdef PSX2

static void PhotoDisp( Work *pWork ) // 写真の描画
{
	u_char        *pTmp;
	int       mem_count;
	int          height;
	int     trance_size;
	int               i;
	int               j;
	int           add_y;
	int         photo_y;
	float         raute;
	u_int         color;

	if ( ( pWork->photo[ 0 ].data == NULL ) ){	// err chack
		return;
	}
	// main
	for ( j = 0 ; j < ( pWork->mode == 2 ? 2 : 1 ) ; j ++ ){
		pTmp = ( u_char * )pWork->photo[ 0 ].data;
		mem_count = 0;
		photo_y = pWork->photo[ j ].y;
		height = PHOTO_MAIN_HEIGHT;
		trance_size = ( int )( ( DRAW_HEIGHT + 127 ) / height );
		add_y = ( int )( pWork->photo[ j ].h ) / ( trance_size );
		color = ( SK_A << 24 ) | ( SK_B << 16 ) | ( SK_G << 8 ) | SK_R;
      if ( BP_Area_EU() )
      {
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
   		raute = 0;
      }
      else
      {
//#else
		   raute = ( pWork->photo[ j ].h - add_y * ( trance_size - 1 ) );
		   raute /= ( float )( trance_size );
      }
//#endif
		for( i = 0 ; i < trance_size ; i ++ ){
			MENU_DrawPicture32( ( int )pWork->photo[ j ].x , photo_y ,
								 pWork->photo[ j ].w , add_y + raute , color , pTmp , PHOTO_MAIN_WIDTH , height , 0 );
			mem_count += height;
			photo_y += add_y + raute;
			pTmp += ( PHOTO_MAIN_WIDTH * height * 4 );
			if ( DRAW_HEIGHT - mem_count < height ){
				add_y = ( pWork->photo[ j ].h - ( ( add_y + raute ) * ( trance_size - 1 ) ) );
				height = DRAW_HEIGHT - mem_count;
			}
		}
	}
}
#else
//XBOX
static void PhotoDisp( Work *pWork ) // 写真の描画
{
	int j ;
	u_int         color;
	if ( ( pWork->tex == NULL ) ){	// err chack
		return;
	}
	color = ( SK_A << 24 ) | ( SK_B << 16 ) | ( SK_G << 8 ) | SK_R;
	for ( j = 0 ; j < ( pWork->mode == 2 ? 2 : 1 ) ; j ++ ){
		MENU_DrawPictureTex( (int)pWork->photo[j].x,(int)pWork->photo[j].y,(int)pWork->photo[j].w,
			(int)pWork->photo[j].h,color,pWork->tex,0);
	}
}
#endif

static void JpegDisp( Work *pWork ) // JPEGを展開して表示
{
	if ( !( pWork->flag & SK_JPEG_LOAD ) ){
#if 0 // BP/Armature - Fix for web_site crash with corrupt save data.
      if ( ( GM_Configuration & GM_CONFIG_TANKER_CLEARED ) && ( pWork->mode == 0 ) && !( pWork->flag & SK_MIST_DATA ) ){
			if ( GM_TankerPicture != NULL ){
#ifdef PSX2
				UTL_JpegDecode( pWork->photo[ 0 ].data , DRAW_WIDTH , DRAW_HEIGHT , GM_TankerPicture ); // decode
#else
//XBOX
				UTL_JpegDecode( pWork->tex->image , DRAW_WIDTH , DRAW_HEIGHT , GM_TankerPicture ); // decode
#endif


				pWork->flag |= SK_JPEG_LOAD;
			}
		} 
      else 
#endif
      { // Plant Start
			PlantSpriteInit( pWork );
		}
	} else {
		PhotoDisp( pWork );
	}
}

// Japanease or Usa or Pal
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
#if 0

#define SELECT PAD_OK //元はPAD_A yano 2002.05.10
#define CANCEL PAD_DEMO_CANCEL //元はPAD_B

#else

#define SELECT PAD_OK //元はPAD_A yano 2002.05.10
#define CANCEL PAD_DEMO_CANCEL //元はPAD_B

#endif

static void PadAct( Work *pWork )
{
#if 0
	// 取得
	pContinue = pWork->menu_null->head.child;
	pExit = pWork->menu_null->head.child->head.next;
	if ( GV_PadDataDirect[ 0 ].press & PAD_U ){
		if ( pWork->position > 0 ){
			// 現在選択されているものを選ぶ
			if ( pWork->position == 0 ){
				pNow = pContinue;
				pNoSelect = pExit;
			} else {
				pNow = pExit;
				pNoSelect = pContinue;
			}
			pNoSelect->sprite.col.a = ( u_char )68;
			pWork->position--;
			GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_CUR01 );
		}
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_D ){
		if ( pWork->position < 1 ){
			// 現在選択されているものを選ぶ
			if ( pWork->position == 0 ){
				pNow = pContinue;
				pNoSelect = pExit;
			} else {
				pNow = pExit;
				pNoSelect = pContinue;
			}
			pNoSelect->sprite.col.a = ( u_char )68;
			pWork->position++;
			GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_CUR01 );
		}
	}
#endif
	if ( ( GV_PadDataDirect[ 0 ].press & CANCEL ) || ( GV_PadDataDirect[ 0 ].press & PAD_STA ) ){
		if ( pWork->proc_next != 0 ){
			GM_ExecProc( pWork->proc_next , NULL );
			pWork->proc_next = 0;
		}
		return;
	}
}

#define PIC_SPEED_LIMIT ( 0.2f )
static void ScreenDisp( Work *pWork )
{
    SPR_SHOW( pWork->special_view );
}

static void NormalAct( Work *pWork )
{
	MsgDie( pWork );
	PadAct( pWork );
	ScreenDisp( pWork );
	JpegDisp( pWork );
}

static void Act( Work *pWork )
{
 	pWork->act( pWork );
}

static	void	Die( Work *pWork )
{
#ifdef PSX2
	GV_DelayedFree( pWork->photo[ 0 ].data );
#endif

	if ( pWork->flag & SK_PLANT_SPRITE_OK ){
		if ( pWork->sprite[ 0 ].obj != NULL ){
			SPR_Destroy_2D_Object( pWork->sprite[ 0 ].obj );	
		}
		if ( pWork->sprite[ 1 ].obj != NULL ){
			SPR_Destroy_2D_Object( pWork->sprite[ 1 ].obj );
		}
	}
	SPR_Destroy_2D_Object( pWork->special_view );
#ifndef PSX2
//XBOX
	DG_FreeLinerTexture( pWork->tex );
#endif


}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	u_int *pTmp;
	work->action = 0;

	// シナリオ
	work->mode = GCL_GetOptionValue( 'm' , 0 );
	work->strcode_tri = GCL_GetOptionValue( 't' , 0 );
	work->sprite[ 0 ].strcode_tex = work->sprite[ 1 ].strcode_tex = GCL_GetOptionValue( 'E' , 0 );

	pTmp = ( u_int * )GM_TankerPicture;

	if ( ( pTmp[ 0 ] + pTmp[ 1 ] == 0 ) || ( pTmp[ 0 ] + pTmp[ 1 ] > 16 * 1024 ) ){ // データが無いまたは不正
		work->flag |= SK_MIST_DATA;
		printf("DataSize = %d + %d = %d\n" , pTmp[ 0 ] , pTmp[ 1 ] , pTmp[ 0 ] + pTmp[ 1 ] );
	}
	if ( !( ( GM_Configuration & GM_CONFIG_TANKER_CLEARED ) && ( work->mode == 0 ) && !( work->flag & SK_MIST_DATA ) ) ){
		if ( work->strcode_tri == 0 || work->sprite[ 0 ].strcode_tex == 0 ){
			ASSERT( 0 );
		}
	}
	work->act = ( void * )NormalAct;
	work->position = 0; // default continue
	// gcl 
	work->proc_next = GCL_GetOptionValue( 'c' , 0 ); // procの読み込み
	// etc
	work->timer = 0;
	SpriteInit( work );
	// photo
	work->sprite[ 0 ].obj = NULL;
	work->sprite[ 1 ].obj = NULL;

	switch( work->mode ){
	case 0 : //web00
		work->photo[ 0 ].x = 115.0f;
		work->photo[ 0 ].y = 92.0f-2;
		work->photo[ 0 ].w = 217.0f;
		work->photo[ 0 ].h = 160.0f-7;
		// 2枚目
		work->photo[ 1 ].x = 0.0f;
		work->photo[ 1 ].y = 0.0f;
		work->photo[ 1 ].w = 0.0f;
		work->photo[ 1 ].h = 0.0f;
		break;
	case 1 : //web01
		work->photo[ 0 ].x = 43.0f;
		work->photo[ 0 ].y = 166.0f-6;
		work->photo[ 0 ].w = 151.0f;
		work->photo[ 0 ].h = 112.0f*0.964f;
		// 2枚目
		work->photo[ 1 ].x = 0.0f;
		work->photo[ 1 ].y = 0.0f;
		work->photo[ 1 ].w = 0.0f;
		work->photo[ 1 ].h = 0.0f;
		break;
	case 2 : //web02
		work->photo[ 0 ].x = 128.0f;
		work->photo[ 0 ].y = 118.0f-4;
		work->photo[ 0 ].w = 191.0f;
		work->photo[ 0 ].h = 142.0f-7;
		// 2枚目
		work->photo[ 1 ].x = 17.0f;
		work->photo[ 1 ].y = 166.0f-4;
		work->photo[ 1 ].w = 78.0f;
		work->photo[ 1 ].h = 58.0f-4;
		break;
	}

#ifdef PSX2
//JPEG_SIZE_LIMIT	 ( DRAW_WIDTH * DRAW_HEIGHT * 2 * sizeof(short) ) //( 24*1024 * sizeof(short) )
	work->photo[ 0 ].data = GV_Malloc( sizeof( u_char ) * JPEG_SIZE_LIMIT );
	if ( work->photo[ 0 ].data == NULL ){
		return ( -1 );
	}
	GV_ZeroMemory( work->photo[ 0 ].data , sizeof( u_char ) * JPEG_SIZE_LIMIT );
#else 
//XBOX
//TEX_format
//DG_TEXLIN_FORMAT_A8R8G8B8,DG_TEXLIN_FORMAT_A1R5G5B5,DG_TEXLIN_FORMAT_A4R4G4B4）

	work->tex = DG_MakeLinerTexture( DRAW_WIDTH, DRAW_HEIGHT, DG_TEXLIN_FORMAT_A8R8G8B8);
	ASSERT(work->tex != NULL) ;
#endif


	GV_SetActorChild( work , NewPictureDrawManager( 145 ) );
#ifdef DEBUG
	printf("GM = %x mode = %d\n" , GM_Configuration , work->mode );
#endif
	JpegDisp( work );

	return 0 ;
}

/*----------------------------------------------------------------*/
#define	ACTOR_PRIO		(254)

void *NewWebSite( int name ) 
{
	Work		*work ;

	work = (Work*)GV_CreateActor( GV_ACTOR_ASSIST , GV_CLASS_OBJECT , sizeof( Work ), ACTOR_PRIO ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->name = name;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}

	return work ;
}
