//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   photo_view.c
   タンカーカメラでセーブされた写真を表示
   web_sight.cから切り出し 苦情は重野まで。
   2002/07/24 Sigeno
   $Id: photo_view.c,v 1.2 2002/12/05 18:42:01 takaki Exp $
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

#include	"../../mode/menu/menu.h"
#include "libfs.h"

#ifdef PSX2
#define PAD_OKU (PAD_R | PAD_A)
#define PAD_MOD (PAD_L | PAD_B)
#define PAD_EXIT PAD_Y
#else /* KP_XBOX */
#define PAD_OKU (PAD_R | PAD_B)
#define PAD_MOD (PAD_L | PAD_A)
#define PAD_EXIT PAD_Y
#endif

enum {
	OKURU=1,
	MODORU ,
};

#define IN_FADE_SPEED (6)
#define OUT_FADE_SPEED (12)
#define SK_R (128)
#define SK_G (128)
#define SK_B (128)
#define SK_A (128)

#define FADE_IN_TIME	DIRECT_TICK(60)
#define SUSTIN_TIME		DIRECT_TICK(180)
#define FADE_OUT_TIME	DIRECT_TICK(60)

#define TOTAL_TIME	(FADE_IN_TIME + SUSTIN_TIME + FADE_OUT_TIME)

enum {
	SK_NORMAL               = 0x00001,
	SK_SELECT_OK            = 0x00002,
	SK_JPEG_LOAD            = 0x00004,
	SK_PLANT_SPRITE_OK      = 0x00008,
	SK_MIST_DATA            = 0x00010,
	SK_END                  = 0x00020,
};


enum{
	EXE_PROG = 0,
	EXE_GCL = 1,
} ;

typedef struct {
	void *data; // jpeg date
//	float x , y; // 始点
//	float w , h; // 幅
	int x , y; // 始点
	int w , h; // 幅
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

typedef	struct _photo_view_work {
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
	int 				timer;
	int                 mode;
	int                 strcode_tri;
//	SK_Sprite           sprite[ 2 ]; // max 2
	int                 proc_id;
	int					r;
	int					g;
	int					b;
	int					a;
	int					pad_buf ;
	int					exe_mode ;
	void	*child_act ;
	void  ( *act )( struct _work * );

   int               photoNum;
} Work ;

// プロトタイプ
// extern 

// 
extern void *NewPictureDrawManager( int prio );
extern void BP_MENU_DrawPictureTexEX( int x, int y, int w, int h, int color, int photoNum, int u0, int v0, int u1, int v1, int flag );

#ifndef PSX2
/*XBOX*/
extern void MENU_DrawPictureTex( int x, int y, int w, int h, int color, DG_TEX_LIN *tex, int flag );
extern DG_TEX_LIN *DG_MakeLinerTexture( int width, int height, int format );
extern void DG_FreeLinerTexture( DG_TEX_LIN *tex );

#endif

#define JPEG_SIZE_LIMIT	 ( DRAW_WIDTH * DRAW_HEIGHT * 2 * sizeof(short) ) //( 24*1024 * sizeof(short) )
#define PHOTO_MAIN_WIDTH ( 1280 )
#define PHOTO_MAIN_HEIGHT ( 720 )

static void PhotoDisp( Work *work ) // 写真の描画
{
   u_int color;
   color = ( work->a << 24 ) | ( work->b << 16 ) | ( work->g << 8 ) | work->r ;

   BP_MENU_DrawPictureTexEX( (int)work->photo[ 0 ].x, (int)work->photo[ 0 ].y,
      work->photo[ 0 ].w, work->photo[ 0 ].h, color, 
      work->photoNum,
      0, 0, PHOTO_MAIN_WIDTH, PHOTO_MAIN_HEIGHT,
      0 );
}

static void JpegDisp( Work *work ) // JPEGを展開して表示
{
   PhotoDisp( work );
}

// Japanease or Usa or Pal
#if 0
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL

#define SELECT PAD_OK //元はPAD_A yano 2002.05.10
#define CANCEL PAD_CANCEL //元はPAD_B

#else

#define SELECT PAD_OK //元はPAD_A yano 2002.05.10
#define CANCEL PAD_CANCEL //元はPAD_B

#endif


#define PIC_SPEED_LIMIT ( 0.2f )

static void Act( Work *work )
{

	if(work->timer> SUSTIN_TIME+FADE_OUT_TIME){
		work->a = SK_A * (FADE_IN_TIME - (work->timer - (SUSTIN_TIME+FADE_OUT_TIME) ) ) / FADE_IN_TIME ;
//printf("work->a[%d]\n",work->a);
	}else if(work->timer> FADE_OUT_TIME){
		work->a = SK_A  ;
#if 1
		work->timer = SUSTIN_TIME+FADE_OUT_TIME ;
		if(work->pad_buf == 0){
			if( GV_PadDataDirect[0].press & PAD_OKU ){ /* 次ページへ */			
				work->pad_buf = OKURU ;
			} else if( GV_PadDataDirect[0].press & PAD_MOD ){ /* 前ページへ */
				work->pad_buf = MODORU ;
			} else if( GV_PadDataDirect[0].press & PAD_EXIT ){ /* 終了 */
				work->pad_buf = OKURU ;
			}
			if(work->pad_buf != 0){
				work->timer = FADE_OUT_TIME + 1 ;
			} ;
		}
#endif
	}else {
		work->a = SK_A * work->timer / FADE_OUT_TIME ;
	}

#if 0
	if( GV_PadDataDirect[0].status & PAD_U ){
		work->photo[ 0 ].h -= 1;
	}else if(GV_PadDataDirect[0].status & PAD_D ){
		work->photo[ 0 ].h += 1;
	}

	if( GV_PadDataDirect[0].status & PAD_R ){
		work->photo[ 0 ].w += 1 ;
	}else if(GV_PadDataDirect[0].status & PAD_L ){
		work->photo[ 0 ].w -= 1 ;
	}

	if( GV_PadDataDirect[0].status & PAD_B ){
		work->photo[ 0 ].x += 1 ;
	}else if(GV_PadDataDirect[0].status & PAD_Y ){
		work->photo[ 0 ].x -= 1 ;
	}

	if( GV_PadDataDirect[0].status & PAD_A ){
		work->photo[ 0 ].y += 1 ;
	}else if(GV_PadDataDirect[0].status & PAD_X ){
		work->photo[ 0 ].y -= 1 ;
	}

printf(" X[%d] Y [%d] W[%d] H[%d] \n",work->photo[ 0 ].x,work->photo[ 0 ].y,work->photo[ 0 ].w,work->photo[ 0 ].h);

#endif
	if(work->a < 0 ){
		work->a = 0 ;
	}
	JpegDisp( work );
	if(work->timer >= 0){
		work->timer-- ;
	}else {
		if( work->actor.actor.class & GV_CLASS_CHILD ){
			GV_CallParentSignalFunc(work, work->pad_buf, 0);
		}
		GV_DestroyActor( work ) ;
	}
}

static	void	Die( Work *work )
{
#ifdef PSX2
   if( work->photo[ 0 ].data )
	   GV_DelayedFree( work->photo[ 0 ].data );
#endif
#ifndef PSX2
//XBOX
	DG_FreeLinerTexture( work->tex );
#endif

	if(work->exe_mode == EXE_GCL ){
		if(work->proc_id != 0){
			GM_ExecProc( work->proc_id, NULL );
			work->proc_id= 0;
		}
	}
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	u_int *pTmp;
	float ftmp;
	/*何らかの理由でキャラ起動失敗してもproc実行だけは行うようにする*/
	if(work->exe_mode == EXE_GCL ){
		if ( GCL_GetOption( 'p' ) != NULL ){
			if( GCL_NextStr() != NULL ){
				work->proc_id  = GCL_GetNextInt();
			}else {
				work->proc_id  = 0;
			}
		}
	}


	work->action = 0;
	work->timer = TOTAL_TIME ;
	work->mode = 0 ;

   pTmp = ( u_int * )GM_TankerPicture;
	if( pTmp == NULL )
   {
		printf("GM_TankerPicture Not Found!!!\n");
		return ( -1 );
	}
	work->photoNum = *pTmp;

	if( work->photoNum  < 0 || work->photoNum  > 5 )
   { 
		printf("Get GM_TankerPicture failed!!\n");
		return ( -1 );
	}
//512*384
	work->photo[ 0 ].x = 0 ;
	work->photo[ 0 ].y = 0 ;
	work->photo[ 0 ].w = VR_WIDTH ;
	work->photo[ 0 ].h = VR_HEIGHT ;

printf(" PHOTO X[%d]Y[%d]W[%d]H[%d]\n",work->photo[ 0 ].x,work->photo[ 0 ].y,work->photo[ 0 ].w,work->photo[ 0 ].h);

	work->r = 128;
	work->g = 128;
	work->b = 128;
	work->a = 128;
	work->pad_buf = 0 ;
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
	if(work->tex == NULL) {
		return ( -1 );
	}
#endif

	GV_SetActorChild( work , work->child_act = NewPictureDrawManager( 145 ) );

#ifdef DEBUG
	printf("GM = %x mode = %d\n" , GM_Configuration , work->mode );
#endif

	JpegDisp( work );

	return 0 ;
}

/*----------------------------------------------------------------*/
#define	ACTOR_PRIO		(254)

void *NewTnkPhotoView( void ) 
{
	Work		*work ;
printf("NewTnkPhotoView CALLED!!!\n");
	work = (Work*)GV_CreateActor( GV_ACTOR_ASSIST , GV_CLASS_OBJECT , sizeof( Work ), ACTOR_PRIO ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->exe_mode = EXE_PROG ;
//	work->exe_mode = EXE_GCL ;

	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}

	return work ;
}
void *NewTnkPhotoViewGcl( void ) 
{
	Work		*work ;
printf("NewTnkPhotoView CALLED!!!\n");
	work = (Work*)GV_CreateActor( GV_ACTOR_ASSIST , GV_CLASS_OBJECT , sizeof( Work ), ACTOR_PRIO ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
//	work->exe_mode = EXE_PROG ;
	work->exe_mode = EXE_GCL ;

	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}

	return work ;
}
