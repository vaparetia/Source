//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   question.c
   アンケート
   
   2001/06/05	S.Kobayashi
   $Id: encute.c,v 1.1.1.3 2002/11/19 11:50:20 Yoshizawa1 Exp $
*/

/* メモリーカードがやばいかも */

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
#include "libfs.h"

#include "mode/menu/xtextscn.h"

#define		LAYOUT		   (8695473) // node_enc.l2d 
#define     STR_CURSOR_DOT (2739206)
#define     STR_DEFAULT    (566267)
#define     STR_ROOT       (2770484)
#define     DOT_POS_X      ( 25.0f )
#define     DOT_POS_Y      ( 78.0f )
#define		ENCUTE_POS_X   (54)
#define 	ENCUTE_POS_Y   (84)
#define		ENCUTE_HEIGHT  (28)
#define 	POSITION_MAX   (5)
#define 	VOICE_MAX      (2)

#define     SK_LOW_A       (52)
#define ENCUTE_BASE_X ( 59 )
//#ifdef JAPANESE_BP_IGNORE()
	#define 	FONT_WIDTH_JP     ( 24 )
	#define 	FONT_HEIGHT_JP    ( 28 )
	#define 	ENCUTE_BASE_Y_JP  ( 86 )
	#define 	FONT_PITCH_JP     ( 5 )
//#else 
	#define 	FONT_WIDTH_NOJP     ( 22 )
	#define 	FONT_HEIGHT_NOJP    ( 26 )
	#define 	ENCUTE_BASE_Y_NOJP  ( 76 )
	#define 	FONT_PITCH_NOJP     ( 2 )
//#endif

#define FONT_WIDTH ( BP_Area_JP() ? ( FONT_WIDTH_JP ) : ( FONT_WIDTH_NOJP ) )
#define FONT_HEIGHT ( BP_Area_JP() ? ( FONT_HEIGHT_JP ) : ( FONT_HEIGHT_NOJP ) )
#define ENCUTE_BASE_Y ( BP_Area_JP() ? ( ENCUTE_BASE_Y_JP ) : ( ENCUTE_BASE_Y_NOJP ) )
#define FONT_PITCH ( BP_Area_JP() ? ( FONT_PITCH_JP ) : ( FONT_PITCH_NOJP ) )

#define SYS_FONT_WIDTH ( 24 )
#define SYS_FONT_HEIGHT ( 24 )

//#ifdef JAPANESE_BP_IGNORE()
#define LINE_1_JP ( SYS_FONT_WIDTH * 24 )
#define LINE_2_JP ( SYS_FONT_WIDTH * 24 )
#define LINE_3_JP ( SYS_FONT_WIDTH * 23 )
#define LINE_4_JP ( SYS_FONT_WIDTH * 21 )
#define LINE_5_JP ( SYS_FONT_WIDTH * 22 )

#define DISP_LINE_1_JP ( FONT_WIDTH * 24 )
#define DISP_LINE_2_JP ( FONT_WIDTH * 24 )
#define DISP_LINE_3_JP ( FONT_WIDTH * 23 )
#define DISP_LINE_4_JP ( FONT_WIDTH * 21 )
#define DISP_LINE_5_JP ( FONT_WIDTH * 22 )

//#else

#define LINE_1_NOJP ( SYS_FONT_WIDTH * 35 )
#define LINE_2_NOJP ( SYS_FONT_WIDTH * 35 )
#define LINE_3_NOJP ( SYS_FONT_WIDTH * 35 )
#define LINE_4_NOJP ( SYS_FONT_WIDTH * 35 )
#define LINE_5_NOJP ( SYS_FONT_WIDTH * 35 )

#define DISP_LINE_1_NOJP ( FONT_WIDTH * 28 )
#define DISP_LINE_2_NOJP ( FONT_WIDTH * 28 )
#define DISP_LINE_3_NOJP ( FONT_WIDTH * 28 )
#define DISP_LINE_4_NOJP ( FONT_WIDTH * 28 )
#define DISP_LINE_5_NOJP ( FONT_WIDTH * 28 )

#define LINE_1 ( BP_Area_JP() ? ( LINE_1_JP ) : ( LINE_1_NOJP ) )
#define LINE_2 ( BP_Area_JP() ? ( LINE_2_JP ) : ( LINE_2_NOJP ) )
#define LINE_3 ( BP_Area_JP() ? ( LINE_3_JP ) : ( LINE_3_NOJP ) )
#define LINE_4 ( BP_Area_JP() ? ( LINE_4_JP ) : ( LINE_4_NOJP ) )
#define LINE_5 ( BP_Area_JP() ? ( LINE_5_JP ) : ( LINE_5_NOJP ) )

#define DISP_LINE_1 ( BP_Area_JP() ? ( DISP_LINE_1_JP ) : ( DISP_LINE_1_NOJP ) )
#define DISP_LINE_2 ( BP_Area_JP() ? ( DISP_LINE_2_JP ) : ( DISP_LINE_2_NOJP ) )
#define DISP_LINE_3 ( BP_Area_JP() ? ( DISP_LINE_3_JP ) : ( DISP_LINE_3_NOJP ) )
#define DISP_LINE_4 ( BP_Area_JP() ? ( DISP_LINE_4_JP ) : ( DISP_LINE_4_NOJP ) )
#define DISP_LINE_5 ( BP_Area_JP() ? ( DISP_LINE_5_JP ) : ( DISP_LINE_5_NOJP ) )

//#endif
// l2d parts
#define STR_LINE1 (16139479)
#define STR_LINE2 (16139480)
#define STR_LINE3 (16139481)
#define STR_LINE4 (16139482)
#define STR_LINE5 (16139483)

#define MESG_PANEL_W		(512-64+FONT_SIZE_W)
#define MESG_PANEL_H		((FONT_SIZE_H+FONT_SIZE_H/2)*3+FONT_SIZE_H/2)

// etc

enum {
	SK_NORMAL        = 0x0 ,
	SK_END           = 0x1 ,
	SK_STREAM_NORMAL = 0x2 ,
	SK_STREAM_END    = 0x4 ,
	SK_CHANCEL       = 0x8 ,
	SK_SPRITE_INIT   = 0x10 ,
	SK_MOVE_OK       = 0x20 , 
};

typedef struct {
	u_char   r;
	u_char   g;
	u_char   b;
	u_char   a;
	char    *font;
} Code;

typedef struct {
	void *data_le;
	int handle;
	int point;
	char flag;
} Voice;

typedef struct {
	float p;
	int   count;
	int   code1;
	int   code2;
	int   morf_parts;
	int   flag;
} Hokan;

typedef struct {
	int     count;
	int     end_y;
	u_long64  flag;
} Hokan_Program;

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	SPR_OBJ             *cursor_dot; // sprite
	void           		*font_work;
	Code                code[ POSITION_MAX ];
	Voice              	voice[ VOICE_MAX ];
	int                 voice_switch;
	int                 position;
	int                 old_position;
	int                 pad_status;
	int                 pad_check;
	int                 hold_time;
	int                 proc_prev;
	int                 proc_next;
	int                 name;
	int                 flag;

	int                 parent_name;
	int                 chilid_name;
	int             	ans;
	int                 mode;
	int                 encute_resource;
	Hokan_Program       dot_hokan;
	void  ( *act )( struct _work * );
} Work ;

// プロトタイプ
// static
static void EscapeAct( Work * );
static void NormalAct( Work * );
static void EncountAct( Work * );
static void MorfL2d( int handle , int strcode , int code1 , int code2 , Hokan * );
static int StringHeightLocal( void );
static inline int MorfAct( Work *pWork , Hokan *pHokan );
static inline void DotMorfUpdate( Work *pWork ); // update
// extern 
extern void SK_Printf( int ascci , SPR_OBJ *pObj , int u , int v );
extern void SK_MenuPrintf( char *str , int x , int y , CVECTOR *color , char mode );
extern void *NewTextScreenControl( void );
extern void MENU_ClearTextTexture( void *work );
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
extern void *NewTextScreenControlEx( int text_vram_width, int text_vram_height, int prio, int flag, int bufferedTextFlag );


void *GetLocalResource( int ref_id, int offset );


/*----------------------------------------------------------------*/
static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork;

	pWork = ( Work * )workp;

	switch ( signal ){
	case 0x2 :
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return 1;
	}

	return ( 0 );
}

static void SendFinishMsg( Work *pWork )
{
	if( pWork->parent_name != 0 && pWork->parent_name != 1){
		GV_MSG msg;
		int message[ 3 ];

		msg.address = pWork->parent_name;
		msg.message = message;
		msg.message_len = sizeof( message ) / sizeof( message[ 0 ] );

		message[ 0 ] = pWork->name;
		message[ 1 ] = pWork->ans;

		GV_SendMessage(&msg);

#if 0
		printf("Portsel : Send Message\n");
#endif
	}
}

static int SprInit( Work *pWork ) // default を a
{
	SPR_OBJ		*spr;
	float       ftmp;
	int status;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( ( pWork->flag & SK_SPRITE_INIT ) || ( status == L2D_STAT_BUSY ) ){
		return ( 1 );
	}
	// dotの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_CURSOR_DOT ) ;
	if ( spr == NULL ){
		SK_Err("cursor_dot\0");
		return -1 ;
	}
	pWork->cursor_dot = spr;
	ftmp = 384.0f / 448.0f;
//	spr->sprite.pos.x = DOT_POS_X;
	spr->sprite.pos.y = ( ENCUTE_BASE_Y + 4 + ( ENCUTE_HEIGHT * pWork->position ) ) * ftmp;
	spr->sprite.col.a = 0;

    pWork->dot_hokan.flag &= ~SK_MOVE_OK;
	pWork->dot_hokan.end_y = ( int )( ( ENCUTE_BASE_Y + 4 ) * ftmp );
	pWork->dot_hokan.count = 5;

	pWork->flag |= SK_SPRITE_INIT;

	return( 0 );
}

static void DotUpdate( Work *pWork )
{
	float ftmp;
	int   tmp;

	if ( ( pWork->position == pWork->old_position ) ){
		pWork->old_position = pWork->position;
		return;
	}

	ftmp = 384.0f / 448.0f;
	tmp = FONT_PITCH * pWork->position;
    pWork->dot_hokan.flag &= ~SK_MOVE_OK;
	pWork->dot_hokan.end_y = ( int )( ( ENCUTE_BASE_Y + 4 + ( StringHeightLocal() * pWork->position ) + tmp ) * ftmp );
	pWork->dot_hokan.count = 5;
#if 0
	switch( pWork->position ){
	case 0 : // name
		pWork->dot_hokan.code1 = STR_LINE1;
		pWork->dot_hokan.p = 0.0f;
		pWork->dot_hokan.count = 10;
		break;
	case 1 : // sex
		pWork->dot_hokan.code1 = STR_LINE2;
		pWork->dot_hokan.p = 0.0f;
		pWork->dot_hokan.count = 10;
		break;
	case 2 : // birthday
		pWork->dot_hokan.code1 = STR_LINE3;
		pWork->dot_hokan.p = 0.0f;
		pWork->dot_hokan.count = 10;
		break;
	case 3 : // blood type
		pWork->dot_hokan.code1 = STR_LINE4;
		pWork->dot_hokan.p = 0.0f;
		pWork->dot_hokan.count = 10;
		break;
	case 4 : // region
		pWork->dot_hokan.code1 = STR_LINE5;
		pWork->dot_hokan.p = 0.0f;
		pWork->dot_hokan.count = 10;
		break;
	}
	switch( pWork->old_position ){
	case 0 : // name
		pWork->dot_hokan.code2 = STR_LINE1;
		break;
	case 1 : // sex
		pWork->dot_hokan.code2 = STR_LINE2;
		break;
	case 2 : // birthday
		pWork->dot_hokan.code2 = STR_LINE3;
		break;
	case 3 : // blood type
		pWork->dot_hokan.code2 = STR_LINE4;
		break;
	case 4 : // region
		pWork->dot_hokan.code2 = STR_LINE5;
		break;
	}
#endif
	pWork->old_position = pWork->position;
}

static void Update( Work *pWork ) // update
{
	int    alpha;
	int        i;

	for ( i = 0 ; i < POSITION_MAX ; i ++ ){
		if ( pWork->position == i ){
		  if ( pWork->code[ i ].a < 0x80 ){ // 選択させているもの
		  		pWork->code[ i ].a += 0x02;
				if ( pWork->code[ i ].a > 0x80 ){
					pWork->code[ i ].a = 0x80;
				}
			}
		} else {
			alpha = ( int )pWork->code[ i ].a;
			alpha -= 0x4;
			if ( alpha <= SK_LOW_A ){
				alpha = SK_LOW_A;
			}
			pWork->code[ i ].a = ( u_char )alpha;
		}
	}
	// dot position update
	DotUpdate( pWork );
	SPR_SHOW( pWork->cursor_dot );
}

#define STREAM_MAX (1)
static int VoiceMng( Work *pWork )
{
	int status;

	// 使用済かどうか
	if ( pWork->voice[ pWork->voice_switch ].flag & SK_STREAM_END ){
		return ( -1 );
	}
	// 設定
	status = GM_StreamStatus( pWork->voice[ pWork->voice_switch ].handle );
	if ( pWork->voice[ pWork->voice_switch ].handle < 0 ){ 
		if ( pWork->voice[ pWork->voice_switch ].point < STREAM_MAX && status != GM_STREAM_STATE_PLAY ){
			pWork->voice[ pWork->voice_switch ].handle = GM_VoxStream( GCL_GetLong( pWork->voice[ pWork->voice_switch ].data_le ), GM_STREAM_NO_PAUSE ); 
//			GM_VoxStreamSetPan( pWork->voice[ pWork->voice_switch ].handle , GM_MAX_VOL , GM_PAN_CENTER );
//			GM_StreamStart( pWork->voice[ pWork->voice_switch ].handle );
			if ( pWork->voice[ pWork->voice_switch ].handle < 0 ){
				pWork->voice[ pWork->voice_switch ].flag = SK_STREAM_END;
				return ( -1 );
			}
		} else {
			pWork->voice[ pWork->voice_switch ].flag = SK_STREAM_END;
		}
	}
	switch ( status ){
	case GM_STREAM_STATE_END :
		pWork->voice[ pWork->voice_switch ].handle = -1;
		pWork->voice[ pWork->voice_switch ].point++;
		break;
	}

	return ( 0 );
}

// Japanease or Usa or Pal
#define SELECT PAD_OK
#define CANCEL PAD_CANCEL

static void PadAct( Work *pWork )
{
	if ( !( pWork->dot_hokan.flag & SK_MOVE_OK ) ){
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_U || pWork->pad_status & PAD_U ){
		pWork->pad_status = 0;
		if ( pWork->position > 0 ){
			SE_SEL();
			pWork->old_position = pWork->position;
			pWork->position--;
		}
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_D || pWork->pad_status & PAD_D ){
		pWork->pad_status = 0;
		if ( pWork->position < POSITION_MAX - 1 ){
			SE_SEL();
			pWork->old_position = pWork->position;
			pWork->position++;
		}
	}
	if ( GV_PadDataDirect[ 0 ].press & SELECT ){
		SE_OK();
		pWork->ans = pWork->position;
		pWork->voice_switch = 1;
		GM_StreamStopAll() ;
		pWork->act = ( void * )EscapeAct;
	}
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
		SE_CANCEL();
		pWork->ans = -1;
		pWork->flag = SK_CHANCEL;
		GM_StreamStopAll() ;
		pWork->act = ( void * )EscapeAct;
	}
	// auto mode
	if ( pWork->hold_time >= DIRECT_TICK( 30 ) ){
		pWork->pad_status = GV_PadDataDirect[ 0 ].status;
		pWork->hold_time -= DIRECT_TICK( 4 );
	} else if ( pWork->pad_check & GV_PadDataDirect[ 0 ].status ){
		pWork->hold_time++;
	} else {
		pWork->pad_check = GV_PadDataDirect[ 0 ].status;
	} 
	if ( !( GV_PadDataDirect[ 0 ].status & ( PAD_U | PAD_D ) ) ){
		pWork->hold_time = 0;
		pWork->pad_status = 0;
	}
}

static int StringHeightSYS( void )
{
	int height;

//#ifdef JAPANESE_BP_IGNORE()
   if ( BP_Area_JP() )
   	height = ( SYS_FONT_HEIGHT * 3 );
//#else // 海外
   else
	   height = ( SYS_FONT_HEIGHT * 3 ) + 4;
//#endif 
	return ( height );
}

static int StringHeightLocal( void )
{
	int height;

//#ifdef JAPANESE_BP_IGNORE()
   if ( BP_Area_JP() )
   	height = ( ENCUTE_HEIGHT );
//#else // 海外
   else
      height = ( ENCUTE_HEIGHT * 2 );
//#endif 
	return ( height );
}

static void StringCreate( Work *pWork )
{
	int poly_width[ POSITION_MAX ] = { LINE_1 , LINE_2 , LINE_3 , LINE_4 , LINE_5 };
	int i;
	int v1;

	for ( i = 0 ; i < POSITION_MAX ; i ++ ){
		// trans
		v1 = StringHeightSYS();
		MENU_CreateTextTexture( pWork->font_work , 1 , 12 + ( v1 * i ), poly_width[ i ] , v1 + 12 ,
								1 , 0 , 0 , pWork->code[ i ].font );
	}
}

static void StringDisp( Work *pWork )
{
	int poly_width[ POSITION_MAX ] = { LINE_1 , LINE_2 , LINE_3 , LINE_4 , LINE_5 };
	int disp_width[ POSITION_MAX ] = { DISP_LINE_1 , DISP_LINE_2 , DISP_LINE_3 , DISP_LINE_4 , DISP_LINE_5 };
	float ftmp;
	u_int color;
	int   x1 , y1 , x2 , y2 , u1 , v1; 
	int tmp;
	int i;

	ftmp = 384.0f / 448.0f;
	tmp = 0;
	for ( i = 0 ; i < POSITION_MAX ; i ++ ){
		// color
		color = pWork->code[ i ].r | ( pWork->code[ i ].g << 8 ) | ( pWork->code[ i ].b << 16 ) | ( pWork->code[ i ].a << 24 ); 
		x1 = ENCUTE_BASE_X;
		y1 = ( int )( ( ENCUTE_BASE_Y + ( StringHeightLocal() * i ) + tmp ) * ftmp );
		x2 = ( ENCUTE_BASE_X + disp_width[ i ] ) * ( 512.0f / 640.0f );
		y2 = ( int )( ( ENCUTE_BASE_Y + ( StringHeightLocal() * i ) + ( ENCUTE_HEIGHT * 2 ) + tmp ) * ftmp );
		u1 = poly_width[ i ];
		v1 = StringHeightSYS();
		tmp += FONT_PITCH;

		MENU_PutTextScreen( pWork->font_work, x1 , y1 , x2 , y2 ,
							1 , 12 + ( v1 * i ) , u1 , ( v1 * ( i + 1 ) ) + 12 , color );
	}
}

static void VoiceAct( Work *pWork )
{
	if ( VoiceMng( pWork ) < 0 ){
		// layout initialize <-- default frame の 関係でここで initialize
		pWork->act = ( void * )NormalAct;
	}
	// 文字表示
	StringDisp( pWork );
	Update( pWork );
	PadAct( pWork );
}

static void NormalAct( Work *pWork )
{
	StringDisp( pWork );
	Update( pWork );
	PadAct( pWork );
}

static void EncountAct( Work *pWork ) // 登場
{
	int     alpha;
	int       tmp;
	int         i;


	tmp = 0;
	// alpha増強
	for ( i = 0 ; i < POSITION_MAX ; i ++ ){
		if ( pWork->code[ i ].a < SK_LOW_A ){ // 選択させているもの
			alpha = ( int )pWork->code[ i ].a;
			alpha += 0x04;
			if ( alpha >= SK_LOW_A ){
				pWork->code[ i ].a = SK_LOW_A;
				tmp++;
			} else {
				pWork->code[ i ].a = ( u_char )alpha;
			}
		} else if ( pWork->position == i && pWork->code[ i ].a < 0x80 ) {
			alpha = ( int )pWork->code[ i ].a;
			alpha += 0x04;
			if ( alpha > 0x80 ){
				pWork->code[ i ].a = 0x80;
				tmp++;
			} else {
			  pWork->code[ i ].a = ( u_char )alpha;
			}
		} else {			
			tmp++;
		}
	}
	// l2d ここが + 1
	if ( pWork->cursor_dot->sprite.col.a < 0x80 ){
		alpha = ( int )pWork->cursor_dot->sprite.col.a;
		alpha += 0x04;
		if ( alpha > 0x80 ){
			pWork->cursor_dot->sprite.col.a = 0x80;
			tmp++;
		} else {
			pWork->cursor_dot->sprite.col.a = ( u_char )alpha;
		}
	} else {
		tmp++;
	}
	if ( tmp >= POSITION_MAX + 1 ) {
		if ( pWork->mode == 0 ){
			pWork->act = ( void * )VoiceAct;
		} else {
			pWork->act = ( void * )NormalAct;
		}
	}
	// 文字表示
	StringDisp( pWork );
}

static void EscapeAct( Work *pWork )
{
	int     alpha;
	int       tmp;
	int         i;

	tmp = 0;
	// alpha減衰
	for ( i = 0 ; i < POSITION_MAX ; i ++ ){
		if ( pWork->code[ i ].a > 0 ){ // 選択させているもの
			alpha = ( int )pWork->code[ i ].a;
			alpha -= 0x04;
			if ( alpha < 0 ){
				pWork->code[ i ].a = 0;
				tmp++;
			} else {
			  pWork->code[ i ].a = ( u_char )alpha;
			}
		} else {
			tmp++;
		}
	}
	// l2d ここが + 1
	if ( pWork->cursor_dot->sprite.col.a > 0 ){
		alpha = ( int )pWork->cursor_dot->sprite.col.a;
		alpha -= 0x04;
		if ( alpha < 0 ){
			pWork->cursor_dot->sprite.col.a = 0;
			tmp++;
		} else {
			pWork->cursor_dot->sprite.col.a = alpha;
		}
	} else {
		tmp++;
	}
	// 文字表示
	StringDisp( pWork );
	if ( pWork->flag & SK_CHANCEL ){
		if ( tmp >= POSITION_MAX + 1 ){
			GV_DestroyActor( pWork );
		}
	} else 	if ( tmp >= POSITION_MAX + 1 ) {
		GV_DestroyActor( pWork );
	}
}

static void Act( Work *pWork )
{
	if ( SprInit( pWork ) < 0 ){
		return;
	}
	if ( pWork->flag & SK_SPRITE_INIT ){
	 	pWork->act( pWork );
		DotMorfUpdate( pWork );
//		MorfAct( pWork , &pWork->dot_hokan );
	}
}

static	void	Die( Work *pWork )
{
	SendFinishMsg( pWork );
	if ( pWork->handle_2d >= 0 ){
		L2D_ReleaseLayout( pWork->handle_2d );
	}
}

static inline void DotMorfUpdate( Work *pWork ) // update
{
	// dot position update
	if ( !( pWork->dot_hokan.flag & SK_MOVE_OK ) ){ 
		pWork->cursor_dot->sprite.pos.y += ( pWork->dot_hokan.end_y - pWork->cursor_dot->sprite.pos.y ) / pWork->dot_hokan.count;
		pWork->dot_hokan.count--;
		if ( pWork->dot_hokan.count <= 0 ){
			pWork->dot_hokan.count = 1;
		}
		if ( pWork->cursor_dot->sprite.pos.y == pWork->dot_hokan.end_y ){
			pWork->dot_hokan.flag |= SK_MOVE_OK;
		}
	}
}

// morf
static inline int MorfAct( Work *pWork , Hokan *pHokan ) // これのみ実行
{
	if ( !( pHokan->flag & SK_MOVE_OK ) ){
		MorfL2d( pWork->handle_2d , pHokan->morf_parts  , pHokan->code2  , pHokan->code1  , pHokan );
	} else {
		return ( 0 );
	}
	return( 1 );
}

// モーフィングを設定してくれる関数
static void MorfL2d( int handle , int strcode , int code1 , int code2 , Hokan *hokan )
{
	void *parts;

	parts = L2D_GetParts( handle , strcode );    /* パーツポインタの取得 */

	if ( parts == NULL ){
		SK_Err("morf failed\0");
		return;
	}
	hokan->p += ( 1.0f - hokan->p ) / hokan->count;
	hokan->count--;

	L2D_MorfObject( parts , code1 , code2 , hokan->p );
	if ( ( hokan->p == 1.0f ) || ( hokan->count == 0 ) ){
		hokan->flag = SK_MOVE_OK;
	}
}

/*----------------------------------------------------------------*/

#define FONT_R (98)
#define FONT_G (108)
#define FONT_B (104)

#define SK_ENCUTE_JAPAN (4658308) /* 日本アンケート */
#define SK_ENCUTE_ENGLISH (5842976) /* アメリカアンケート */
#define SK_ENCUTE_FRANCE (5892954) /* フランスアンケート */
#define SK_ENCUTE_GERMAN (4163198) /* ドイツアンケート */
#define SK_ENCUTE_ITARY (14232051) /* イタリアアンケート */
#define SK_ENCUTE_SPAIN (9756734) /* スペインアンケート */
#define STR_VOICE_DATA	0x00ba8b1b
#define STR_VOICE_START		0
#define STR_VOICE_END		1
static void LangChecge( Work *pWork ) // 言語切替関数
{
	pWork->encute_resource = SK_ENCUTE_JAPAN;
	if ( GM_Language == GM_LANG_JAPANESE ){
		pWork->encute_resource = SK_ENCUTE_JAPAN;
#ifdef DEBUG_MODE
		printf("Japanese\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_ENGLISH ){
		pWork->encute_resource = SK_ENCUTE_ENGLISH;
#ifdef DEBUG_MODE
		printf("English\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_FRENCH ){
		pWork->encute_resource = SK_ENCUTE_FRANCE;
#ifdef DEBUG_MODE
		printf("French\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_GERMANY ){
		pWork->encute_resource = SK_ENCUTE_GERMAN;
#ifdef DEBUG_MODE
		printf("Germany\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_ITALY ){
		pWork->encute_resource = SK_ENCUTE_ITARY;
#ifdef DEBUG_MODE
		printf("Italy\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_SPANISH ){
		pWork->encute_resource = SK_ENCUTE_SPAIN;
#ifdef DEBUG_MODE
		printf("Spanish\n");
#endif
		return;
	}
}

static	int	GetResources( Work *work )
{
	int          i;

	// シグナルの登録
	GV_SetActorSignalFunc( work , ReceiveSignal );

	work->handle_2d = -1 ;
	work->handle_2d = L2D_LoadLayout2( LAYOUT , DG_CHANL_MENU , 0, 0 , GV_PAUSE_STOP ) ;
	if ( work->handle_2d < 0 ){
		SK_Err("encute l2d\0");
		return -1 ;
	}
	// default action
	L2D_EvokeAction( work->handle_2d , STR_DEFAULT );
	work->old_position = 0;
	work->position = 0;
	work->pad_status = 0;
	work->pad_check = 0;
	LangChecge( work );
	for( i = 0 ; i < POSITION_MAX ; i++ ){
		work->code[ i ].font = ( char * )GetLocalResource( work->encute_resource , i );
		work->code[ i ].r = FONT_R;
		work->code[ i ].g = FONT_G;
		work->code[ i ].b = FONT_B;
		work->code[ i ].a = 0;
	}
	work->voice[ 0 ].data_le = GetLocalResource( STR_VOICE_DATA , STR_VOICE_START );
	work->voice[ 1 ].data_le = GetLocalResource( STR_VOICE_DATA , STR_VOICE_END );
	for( i = 0 ; i < VOICE_MAX ; i++ ){
		work->voice[ i ].flag = SK_STREAM_NORMAL;
	}
	work->code[ work->position ].r = FONT_R;
	work->code[ work->position ].g = FONT_G;
	work->code[ work->position ].b = FONT_B;
	work->code[ work->position ].a = 0;
	work->flag = SK_NORMAL;
	// demon
	work->font_work = NewTextScreenControlEx( 1024 , 400 , 128 , 0x4, 0/*bufferedTextFlag*/ );
	if ( work->font_work == NULL ){
		SK_Err("Non Memory\0");
		return ( -1 );
	}
	MENU_ClearTextTexture( work->font_work );
	GV_SetActorChild( work , work->font_work );

	StringCreate( work );
	// voice
	work->voice[ 0 ].handle = work->voice[ 1 ].handle = -1;
	work->voice[ 0 ].point = work->voice[ 1 ].point = 0;
	work->voice_switch = 0;
	GM_StreamStopAll() ;
	// morf
	work->dot_hokan.count = 1;
	work->dot_hokan.flag = 0;
	// etc
	work->act = ( void * )EncountAct;

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewQuestion( int parent_name , int child_name , int mode )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->parent_name = parent_name ;
	work->name = child_name ;
	work->mode = mode;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
