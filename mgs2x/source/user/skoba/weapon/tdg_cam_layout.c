//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   tdg_cam_layout_sight.c
   タンカーカメラ
   
   2001/04/19	S.Kobayashi
   $Id: tdg_cam_layout.c,v 1.5 2002/12/29 13:39:16 takaki Exp $
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
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"
#include    "../test/etc.h"
#include    "../equip/capture.h"
#include    <string.h>

#include "bp_vector.h"
#include "BP_EndianSupport.h"

extern void BP_CaptureScreenShot_Tanker_FromJPEG_MemBuffer(int photoNum, void* pMemory);

#define	LAYOUT	     	 (3906765)	/* tdcm.o2d */
#define STR_CENTER       (3367603)
#define STR_CENTER_CURS  (3481206)
#define STR_CENTER_CURS1 (5989410)
#define STR_S_CURS       (2319263)
#define STR_DEFAULT      (566267)
#define STR_HORIZON      (13039795)
#define STR_ROOT         (2770484)
#define STR_SUB_FRAME    (181869)
#define STR_SUB1         (3891313)
#define STR_SUBWINBG     (11138736)

#define SUB_HARF_WIDTH  (34)
#define SUB_HARF_HEIGHT (25)
#define SUB_WIDTH  (68)
#define SUB_HEIGHT (50)
#define SUB_FRAME_MAX   (6)
#define WIDTH ( 82.0f )
#define LOW_WIDTH ( 8.0f )
#define HIGH_WIDTH ( 500.0f )
#define SUB_FRAME_CENTER ( 254.0f )
#define SUB_FRAME_LEFT ( 8.0f )
#define SUB_FRAME_RIGHT ( 500.0f )
#define SUB_FRAME_BUF ( 500.0f )

#define EMPTY_PIC (7308044)

// 後するかも
typedef struct {
	float               sub_frame_next; // 次の場所
	int                 sub_frame_this; // 自分は誰なのか
} SUB_INFO ;

typedef struct {
	int sub_frame;
} DupCount;

typedef	struct _tdg_cam_layout_work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	int                 action_num;
	int                 action;
	int                 time;
	SPR_OBJ             *horizon;      // 枠エフェクト
	SPR_OBJ             *center;      // 真中親オブジェクト
	SPR_OBJ             *center_s_curs;      // 真中オブジェクト
	SPR_OBJ             *center_curs; // 回転オブジェクト
	SPR_OBJ             *sub_frame[ SUB_FRAME_MAX ];
//	SUB_INFO            sub_info[ SUB_FRAME_MAX ];
	float               sub_frame_next[ SUB_FRAME_MAX ]; // 次の場所
	int                 sub_frame_this[ SUB_FRAME_MAX ]; // 自分は誰なのか
	int                 sub_frame_color[ SUB_FRAME_MAX ]; // 写真の色
	int                 sub_frame_sign; // 進行方向
	int                 *sub_frame_center_this;
	int                 next_position; // 次の場所
	float               center_curs1_scale; // 回転しない方の拡縮
	float               cam_scale_bak; // 1フレーム前のカメラのスケール値
	SVECTOR             cam_rot;
	float               angle_max;
	float               angle_min;
	char                pad_info;
	char                pad_info2;
	char                auto_mode;
	char                visible_flag; // 表示のON OFF
	char                sub_frame_togle; // mode cheng
	char                hold_flag;
	int                 flag;
	char                initialize_flag;
	DupCount            dup_count;
	void                *menuprint_work_ptr;
	void                *empty_raw;
	void  ( *act )( struct _work * );
#if 1 //BP_RENDER //#ifdef KP_XBOX
	DG_TEX_LIN* emptyThumbTex;
#endif

} Work ;

enum {
	SK_UN_INITIALIZE    = 0x1,
	SK_INITIALIZE_START = 0x2,
	SK_INITIALIZE_OK    = 0x4,
	SK_INVISIBLE        = 0x8,
	SK_VISIBLE          = 0x10,
};

enum {
	SK_ANDER_STAY = 0x1 ,
	SK_TANKER_FRAME = 0x2,
	SK_SHUTER_ON        = 0x8,
	SK_SHUTER_OPEN      = 0x10,
	SK_SHUTER_FULL_OPEN = 0x20,
	SK_SHUTER_READY = 0x40,
	SK_ANDER_MOVING = 0x80,
};

enum {
	SK_SPACE_FREE = 0,
	SK_SPACE_USE ,
};

enum {
	SK_RIGHT_MODE = 0 ,
	SK_LEFT_MODE,
};

enum {
	SK_PAD_NORMAL = 0,
	SK_PAD_HOLD ,
};

enum {
	SK_HOLD_OFF = 0 ,
	SK_HOLD_ON ,
};

enum {
	SUB_FRAME_0 = 0x1,
	SUB_FRAME_1 = 0x2,
	SUB_FRAME_2 = 0x4,
	SUB_FRAME_3 = 0x8,
	SUB_FRAME_4 = 0x10,
	SUB_FRAME_5 = 0x20,
};

#define SK_FRAME_THIS_GET ( ( GM_TnkerCamStatus & 0xff00 ) >> 8 ) // 現在のframeを出す
#define SK_FRAME_THIS_STORE( c1 ) ( GM_TnkerCamStatus = ( GM_TnkerCamStatus & 0xff ) | ( c1 << 8 ) ) // 現在のframeを格納

// あってはならないglobal
static GCL_VAR_REF *SK_SubFrameRef; // 真中デフォルト
//static int SK_FrameThis = 0;
//static char sub_frame_flag[ SUB_FRAME_MAX ]; // 状態

// staticプロトタイプ
static int NormalAct( Work * ); // 通常
static int AnimetionAct( Work * ); // アニメーション
static int AnderAct( Work * ); // 空領域に移動
static int AnderSearch( Work * );
void SK_PicFileOut( void );
// extern
extern void *SK_PhotoFramePointer( int ); // frameの格納領域のポインタを返す
#if 0 //BP_RENDER //#ifdef PSX2
extern void MENU_DrawPicture16( int x, int y, int w, int h, int color, void *image, int image_w, int image_h, int flag );
#else  /*XBOX*/
extern void MENU_DrawPictureTexEX( int x, int y, int w, int h, int color,
						   DG_TEX_LIN *tex, int u0, int v0, int u1, int v1, int flag );
extern void BP_MENU_DrawPictureTexEX( int x, int y, int w, int h, int color,
                                  int photoNum, int u0, int v0, int u1, int v1, int flag );
#endif /*XBOX*/

static int ImaginaryToReal( Work *pWork , int point )
{
	int i;

	for ( i = 0 ; i < SUB_FRAME_MAX ; i++ ){
		if ( pWork->sub_frame_this[ i ] == point ){
			return ( i );
		}
	}
	return ( 0xff );
}

static void AnderSort( Work *pWork ) // ならびかえ
{
	int   point;
	int    itmp;
	int       i;

	// default
	itmp = SK_FRAME_THIS_GET - 2;
//	printf(" itmp = %d \n" , itmp );
	if ( itmp < 0 ){
		itmp += 6;
	}
	// main
	for ( i = 0 ; i < SUB_FRAME_MAX ; i++ ){
		point = itmp + i;
		point %= 6;
		pWork->sub_frame_this[ i ] = point; // 自分はだれなのか
	}
	if ( pWork->sub_frame_center_this ){
		*pWork->sub_frame_center_this = SK_FRAME_THIS_GET;
	}
}

static int PositionToEnum( int position )
{
	switch( position ){
	case 0 : return ( SUB_FRAME_0 );
	case 1 : return ( SUB_FRAME_1 );
	case 2 : return ( SUB_FRAME_2 );
	case 3 : return ( SUB_FRAME_3 );
	case 4 : return ( SUB_FRAME_4 );
	case 5 : return ( SUB_FRAME_5 );
	default : return 0xff;
	}
}

static int AnderUse( Work *pWork )
{
	int position;

	position = ImaginaryToReal( pWork , SK_FRAME_THIS_GET );
	GM_TnkerCamStatus |= PositionToEnum( position );
	pWork->sub_frame_color[ position ] = 0x80808080; // 色
	return ( position );
}

// 子アクター
static int ReceiveSignal( void *workp, int signal, int value )
{
	Work    *pWork;
	SPR_OBJ *pRoot;
	int        tmp;
	int          i;

	pWork = ( Work * )workp;

	switch ( signal ){
	case 0x2 :
		if ( value == SK_INVISIBLE ){
			if ( !( pWork->flag & SK_INVISIBLE ) ){
				pRoot = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , STR_ROOT );
				if ( pRoot == NULL ){
					SK_Err( "root\0" );
					return -1;
				}
				SPR_HIDE( pRoot );
			}
			pWork->visible_flag = SK_INVISIBLE;
		} else {
			if ( !( pWork->flag & SK_INVISIBLE ) ){
				pRoot = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , STR_ROOT );
				if ( pRoot == NULL ){
					SK_Err( "root\0" );
					return -1;
				}
				SPR_SHOW( pRoot );
			}
			pWork->visible_flag = SK_VISIBLE;
			pWork->hold_flag = SK_HOLD_OFF;
		}
		break;
	case 0x4 :
		printf("now %d\n" , SK_FRAME_THIS_GET);
		tmp = AnderUse( pWork );
		// 親に送信
		GV_CallParentSignalFunc( pWork , SK_ANDER_MOVING , 0 );
		if ( tmp == 0xff ){
			// 親に送信
			GV_CallParentSignalFunc( pWork , SK_SHUTER_READY , 0 );
			return ( 0 );
		}
		pWork->pad_info = SK_PAD_HOLD;
		// あいてない
		pWork->next_position = AnderSearch( pWork );
		if ( pWork->next_position != 0xff && tmp != pWork->next_position ){
			pWork->sub_frame_togle = SK_LEFT_MODE;
			*pWork->sub_frame_center_this = pWork->sub_frame_this[ tmp ]; // 現実から仮へ
			SK_FRAME_THIS_STORE( pWork->sub_frame_this[ pWork->next_position ] ); // 現実から仮へ
			// 強制移動
			for ( i = 0 ; i < SUB_FRAME_MAX ; i ++ ){
				if ( pWork->sub_frame[ i ]->sprite.pos.x == SUB_FRAME_LEFT ){
					pWork->sub_frame[ i ]->sprite.pos.x = HIGH_WIDTH;
					pWork->sub_frame_next[ i ] = HIGH_WIDTH;
					SPR_HIDE( pWork->sub_frame[ i ] );
				}
			}
			pWork->auto_mode = 0x1;
			pWork->act = ( void * )AnderAct;
		} else {
			// 親に送信
			GV_CallParentSignalFunc( pWork , SK_SHUTER_READY , 0 );
		}
		break;
	case SK_SHUTER_ON : // ホールド状態に持ち込む
		pWork->hold_flag = SK_HOLD_ON;
		break;
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return 1;
	}

	return ( 0 );
}

static void AlphaAnime( Work *pWork ) // 色が128になるまで繰り返す
{
	int position;
	int i;

	for ( i = 0 ; i < SUB_FRAME_MAX ; i ++ ){
		position = ImaginaryToReal( pWork , pWork->sub_frame_this[ i ] );
		if ( GM_TnkerCamStatus & PositionToEnum( position ) ){
			if ( !( pWork->sub_frame_color[ position ] & 0x80000000 ) ){
				if ( ( ( pWork->sub_frame_color[ position ] & 0xff000000 ) >> 24 ) < 128 ){
					pWork->sub_frame_color[ position ] += 0x08000000; // それぞれ+8 
				} else {
					pWork->sub_frame_color[ position ] -= 0x08000000; // それぞれ+8 
				}
			}
		} else {
			if ( !( pWork->sub_frame_color[ position ] & 0x40000000 ) ){ // 空わく
				if ( ( ( pWork->sub_frame_color[ position ] & 0xff000000 ) >> 24 ) < 64 ){
					pWork->sub_frame_color[ position ] += 0x04000000; // +4
				} else {
					pWork->sub_frame_color[ position ] -= 0x04000000; // +4
				}
			}
		}
	}
}

// 初期化関数
static int Initialize( Work *pWork )
{
	GM_CameraSet *pCam;
	float tmp_width , tmp_height;

	tmp_width = ( float )( DRAW_WIDTH / 2 );
	tmp_height = ( float )( DRAW_HEIGHT / 2 );

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得

	pWork->center = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_CENTER );
	if ( pWork->center == NULL ){
		SK_Err( "center_parent\0" );
		return ( -1 );
	}

	pWork->center_curs = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_CENTER_CURS );
	if ( pWork->center_curs == NULL ){
		SK_Err( "center_curs\0" );
		return ( -1 );
	}
   // BP FIX: These are correct - no need to break them!
   //pWork->center_curs->spin.center.x = 4;
   //pWork->center_curs->spin.center.y = 44;
	pWork->center_curs->spin.pos.x = 0;
	pWork->center_curs->spin.pos.y = 0;
	pWork->center_s_curs = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_S_CURS );
	if ( pWork->center_s_curs == NULL ){
		SK_Err( "center_s_curs\0" );
		return ( -1 );
	}
	pWork->horizon = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_HORIZON );
	if ( pWork->horizon == NULL ){
		SK_Err( "horizon\0" );
		return ( -1 );
	}
	{
	    // 下の枠
		SPR_OBJ *pTmp;
		SPR_OBJ *pTmp2;
		int strcode;
		int i;

		pTmp = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_SUB_FRAME );
		if ( pTmp == NULL ){
			SK_Err( "sub_frame\0" );
			return ( -1 );
		}
		for ( i = pWork->dup_count.sub_frame ; i < SUB_FRAME_MAX ; i++ ){
			pWork->sub_frame[ i ] = ( SPR_OBJ * )SPR_DuplicateTree( pTmp );
			if ( pWork->sub_frame[ i ] == NULL ){
				SK_Err("Non Memory");
//				SPR_Destroy_2D_Object( pWork->sub_frame[ 1 ] );
				return ( -1 );
			} else {
				pWork->dup_count.sub_frame++;
			}
		}
		// initialize phase
		strcode = STR_SUB1;
		for ( i = 0 ; i < SUB_FRAME_MAX - 1 ; i++ ){
			pTmp2 = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , strcode++ );
			pWork->sub_frame[ i ]->sprite.pos.x = pTmp2->box.rect.begin.x + SUB_HARF_WIDTH;
			pWork->sub_frame[ i ]->sprite.pos.y = pTmp2->box.rect.begin.y + SUB_HARF_HEIGHT;
			pWork->sub_frame_next[ i ] = pWork->sub_frame[ i ]->sprite.pos.x;
			pWork->sub_frame_color[ i ] = 0x00808080;
		}
		strcode--;			 
		pTmp2 = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , strcode );
		pWork->sub_frame[ SUB_FRAME_MAX - 1 ]->sprite.pos.x = pTmp2->box.rect.begin.x + SUB_HARF_WIDTH + 82;
		pWork->sub_frame[ SUB_FRAME_MAX - 1 ]->sprite.pos.y = pTmp2->box.rect.begin.y + SUB_HARF_HEIGHT;
		pWork->sub_frame_next[ SUB_FRAME_MAX - 1 ] = pWork->sub_frame[ SUB_FRAME_MAX - 1 ]->sprite.pos.x;
		pWork->sub_frame_color[ SUB_FRAME_MAX - 1 ] = 0x00808080;
		pWork->sub_frame_sign = 0;
		SPR_HIDE( pWork->sub_frame[ SUB_FRAME_MAX - 1 ] );
		// sort
		AnderSort( pWork );
		// invisible
		pTmp = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_SUBWINBG );
		SPR_HIDE( pTmp );
		pWork->sub_frame_togle = SK_LEFT_MODE;
	}
	pWork->cam_rot = pCam->rotate;

	return ( 0 );	
}

#define CENTER_MAX 20
#define CENTER_SPEED_X 10
static void HorizonMove( Work *pWork )
{
	GM_CameraSet *pCam;
	FVECTOR       stmp;
	FVECTOR       fvec;
	FVECTOR      ftmp3;
	float         ftmp;
	float        ftmp2;
	float    tmp_width;
	float   tmp_height;
	float            r;
	int           sign;

	tmp_width  = ( float )( DRAW_WIDTH / 2 );
	tmp_height = ( float )( DRAW_HEIGHT / 2 ) - 32;
	// カメラの取得
	pCam = GM_GetCurrentCameraSet( 0 );	
	stmp.vx = pCam->rotate.vx - pWork->cam_rot.vx; // 移動した角度を入力
	stmp.vx *= ( 360.f / 4096.f );

	r = DIRECT_TICK( 100.f );
	ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vw  = ( ( ftmp * stmp.vx ) / 360.0f ) * pCam->angle;
	// スピード補正
	sign = stmp.vw < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vw < -CENTER_SPEED_X ){
			stmp.vw = -CENTER_SPEED_X;
		}
	} else {
		if ( stmp.vw > CENTER_SPEED_X ){
			stmp.vw = CENTER_SPEED_X;
		}
	}
	pWork->horizon->empty.pos.y -= ( int )( stmp.vw );
	// 範囲ない制御
	ftmp2 = stmp.vw < 0 ? -stmp.vw : stmp.vw;
	if ( stmp.vw != 0 ){
		if ( pWork->horizon->empty.pos.y < -CENTER_MAX ){
			pWork->horizon->empty.pos.y = -CENTER_MAX;
		}
		if ( pWork->horizon->empty.pos.y > CENTER_MAX ){
			pWork->horizon->empty.pos.y = CENTER_MAX;
		}
	}
	// 戻す
	if ( stmp.vw == 0 ){
		stmp.vx = 0;
		stmp.vy = pWork->horizon->empty.pos.y;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3.vx = 0;
		ftmp3.vy = 0;
		ftmp3.vz = 0;
		ftmp3.vw = 0;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 20 );
		pWork->horizon->empty.pos.y += fvec.vy;
	}
}

static void CenterCursor1Move( Work *pWork )
{
	SPR_OBJ *pObj;
	GM_CameraSet *pCam;
	float scale;

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得
	pObj = pWork->center_s_curs;
	
	// 補間
	if ( pWork->cam_scale_bak == pCam->angle ){ // 元に戻す
		scale = ( float )( 1.0f - pWork->center_curs1_scale ) / 4.f;
		pWork->center_curs1_scale += scale;
	} else {
		scale = pCam->angle - pWork->cam_scale_bak; // default 2.0だから
		scale = scale < 0 ? 1.125f : 0.906f;
		pWork->center_curs1_scale += ( float )( scale - pWork->center_curs1_scale ) / 10.f;
		// 拡大
		if ( pWork->center_curs1_scale < 0.906F ){
			pWork->center_curs1_scale = 0.906F;
		}
		if ( pWork->center_curs1_scale > 1.125F ){
			pWork->center_curs1_scale = 1.125F;
		}	
	}
	// 更新
	SPR_MAG( pObj , pWork->center_curs1_scale );
	SPR_SHOW( pObj ) ;
}

#define ROT_MAX ( 30.f * PI / 360.f )
static void CenterCursMove( Work *pWork )
{
	GM_CameraSet *pCam;
	SPR_OBJ *pObj;
	FVECTOR       stmp;
	FVECTOR       fvec;
	FVECTOR      ftmp3;
	float         ftmp;
	float        ftmp2;
	float            r;
	int           sign;

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得
	pObj = pWork->center_curs;

	// カメラの取得
	pCam = GM_GetCurrentCameraSet( 0 );	
	stmp.vx = pCam->rotate.vy - pWork->cam_rot.vy; // 移動した角度を入力
	stmp.vx *= ( 360.f / 4096.f );

	r = DIRECT_TICK( 100.f );
	ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vx  = ( ( ftmp * stmp.vx ) / 360 ) * pCam->angle;
	// スピード補正
	sign = stmp.vx < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vx < -CENTER_SPEED_X ){
			stmp.vx = -CENTER_SPEED_X;
		}
	} else {
		if ( stmp.vx > CENTER_SPEED_X ){
			stmp.vx = CENTER_SPEED_X;
		}
	}
	pObj->spin.spin -= ( stmp.vx * PI / 360 );
	// 範囲ない制御
	ftmp2 = stmp.vx < 0 ? -stmp.vx : stmp.vx;
	if ( stmp.vx != 0 ){
		if ( pObj->spin.spin < -ROT_MAX ){
			pObj->spin.spin = -ROT_MAX;
		}
		if ( pObj->spin.spin > ROT_MAX ){
			pObj->spin.spin = ROT_MAX;
		}
	}
	// 戻す
	if ( stmp.vx == 0 ){
		stmp.vx = pObj->spin.spin;
		stmp.vy = 0;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3 = DG_ZeroVector;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 10 );
		pObj->spin.spin += fvec.vx;
	}
}

static void NumberMove( Work *pWork )
{
	GM_CameraSet *pCam;
	char text_buffer[ 10 ];
	float tmp;
	float parent_x , parent_y;

	// カメラの取得
	pCam = GM_GetCurrentCameraSet( 0 );	
	tmp = ( 384.f / 448.f );

#ifdef PSX2 /* PS2,XBOXのDRAW_WIDTHがちがうため Y.Yano 2002.08.23 */
	parent_x = 298.0f;
#else
	parent_x = 372.5f;
#endif
	parent_y = DIRECT_SCREEN_Y( 221.0f );
	sprintf( text_buffer , "%04d" , ( int )( ( pCam->angle - 1 ) * 100.f) );
	___MENU_Locate( pWork->menuprint_work_ptr, 2, parent_x , parent_y  , 0 );
	___MENU_Color( pWork->menuprint_work_ptr, 2, 180, 250, 90, 60 );
	___MENU_PrintMini( pWork->menuprint_work_ptr, 2, text_buffer );
#if 0
	parent_x = 292;
	parent_y = 214;
	MENU_S_Locate( parent_x , parent_y , 0 );
	MENU_S_Color( 180 , 250 , 90 , 60 );
	MENU_S_Printf( "%04d" , ( int )( ( pCam->angle - 1 ) * 100.f) );
#endif
}

static void ChildStatus( Work *pWork )
{
	if ( pWork->pad_info != SK_PAD_NORMAL ){
		// 親に送信
		GV_CallParentSignalFunc( pWork , SK_ANDER_MOVING , 0 );
	} else {
		// 親に送信
		GV_CallParentSignalFunc( pWork , SK_ANDER_STAY , 0 );
	}
}

static void AnderMove( Work *pWork )
{
	float x_position;
	float sign;
	int itmp;
	int pad_flag;
	int sub_frame_right , sub_frame_left ;
	int i;

	pad_flag = 0x0;
	x_position = 0.0f;
	sub_frame_right = sub_frame_left = 0xff;

	// パッド/* 2002.07.26 PadDirectに変更 -> PAD-Release doesn't work, that change to PadData again. */
#ifdef PSX2
	if ( ( GV_PadData[ 0 ].press & PAD_L1 ) && pWork->pad_info == SK_PAD_NORMAL ){
		pad_flag = 0x1;
		pWork->pad_info = SK_PAD_HOLD;
		SE_TYPE();
	}
	if ( ( GV_PadData[ 0 ].press & PAD_R1 ) && pWork->pad_info == SK_PAD_NORMAL ){
		pad_flag = 0x2;
		pWork->pad_info = SK_PAD_HOLD;
		SE_TYPE();
	}
#else
	/* XBOXでは右スティックで */
	if ( ( GV_PadData[ 0 ].right_dx > 216 ) && pWork->pad_info == SK_PAD_NORMAL && pWork->pad_info2 == 0 ){
		pad_flag = 0x1;
		pWork->pad_info = SK_PAD_HOLD;
		pWork->pad_info2 = 1;
		SE_TYPE();
	}
	if ( ( GV_PadData[ 0 ].right_dx < 40 ) && pWork->pad_info == SK_PAD_NORMAL && pWork->pad_info2 == 0 ){
		pad_flag = 0x2;
		pWork->pad_info = SK_PAD_HOLD;
		pWork->pad_info2 = 1;
		SE_TYPE();
	}
	if( GV_PadData[ 0 ].right_dx > 90 && GV_PadData[ 0 ].right_dx < 170 ){
		/* スティックが倒れたままの状態でも一回しか移動しない */
		pWork->pad_info2 = 0;
	}
#endif

	switch( pad_flag ){
	case 0x1 : // 左
	  	x_position += WIDTH;
		itmp = SK_FRAME_THIS_GET - 1;
		if ( itmp < 0 ){
			itmp += 6;
		}
		SK_FRAME_THIS_STORE( itmp );
		*pWork->sub_frame_center_this = itmp;
		pWork->sub_frame_sign = 2;
		GCL_SetVarRef( SK_SubFrameRef , 0 , *pWork->sub_frame_center_this );

		pWork->sub_frame_togle = SK_RIGHT_MODE;
		// 強制移動
		for ( i = 0 ; i < SUB_FRAME_MAX ; i ++ ){
			if ( pWork->sub_frame[ i ]->sprite.pos.x == SUB_FRAME_RIGHT ){
				pWork->sub_frame[ i ]->sprite.pos.x = LOW_WIDTH;
				pWork->sub_frame_next[ i ] = LOW_WIDTH;
				SPR_HIDE( pWork->sub_frame[ i ] );
			}
		}
		break;
	case 0x2 : // 右
		x_position -= WIDTH;
		itmp = SK_FRAME_THIS_GET + 1;
		itmp %= 6;
		SK_FRAME_THIS_STORE( itmp );
		*pWork->sub_frame_center_this = itmp;
		pWork->sub_frame_sign = 2;
		GCL_SetVarRef( SK_SubFrameRef , 0 , *pWork->sub_frame_center_this );

		pWork->sub_frame_togle = SK_LEFT_MODE;
		// 強制移動
		for ( i = 0 ; i < SUB_FRAME_MAX ; i ++ ){
			if ( pWork->sub_frame[ i ]->sprite.pos.x == SUB_FRAME_LEFT ){
				pWork->sub_frame[ i ]->sprite.pos.x = HIGH_WIDTH;
				pWork->sub_frame_next[ i ] = HIGH_WIDTH;
				SPR_HIDE( pWork->sub_frame[ i ] );
			}
		}
		break;
	default : 
		x_position = 0.0f;
	}
	// 移動仕切ったか判定
	if ( ( pWork->sub_frame[ 0 ]->sprite.pos.x == pWork->sub_frame_next[ 0 ] ) && ( x_position == 0.0f ) ){
		pWork->pad_info = SK_PAD_NORMAL;
	}
	// 目標に近付く
	for ( i = 0 ; i < SUB_FRAME_MAX ; i ++ ){
		pWork->sub_frame_next[ i ] += x_position;
		// 08/31
		if ( pWork->sub_frame[ i ]->sprite.pos.x == SUB_FRAME_CENTER ){
			if ( SK_SubFrameRef != NULL ){
				*pWork->sub_frame_center_this = pWork->sub_frame_this[ i ];
				GCL_SetVarRef( SK_SubFrameRef , 0 , *pWork->sub_frame_center_this );
			}
		}
		if ( pWork->sub_frame[ i ]->sprite.pos.x >= SUB_FRAME_BUF || pWork->sub_frame[ i ]->sprite.pos.x <= LOW_WIDTH ){
			SPR_HIDE( pWork->sub_frame[ i ] );
		} else {
			SPR_SHOW( pWork->sub_frame[ i ] );
		}
		if ( pWork->sub_frame[ i ]->sprite.pos.x != pWork->sub_frame_next[ i ] ){
			// 移動
			sign = pWork->sub_frame_next[ i ] < pWork->sub_frame[ i ]->sprite.pos.x ? -1.0f : 1.0f;	
			pWork->sub_frame[ i ]->sprite.pos.x += ( ( float )WIDTH / 4.0f ) * sign;
			if ( pWork->sub_frame[ i ]->sprite.pos.x <= LOW_WIDTH && pWork->sub_frame_togle == SK_LEFT_MODE ){ // 最小
				pWork->sub_frame[ i ]->sprite.pos.x = HIGH_WIDTH;
				pWork->sub_frame_next[ i ] = HIGH_WIDTH;
				SPR_HIDE( pWork->sub_frame[ i ] );
			} else if ( pWork->sub_frame[ i ]->sprite.pos.x >= HIGH_WIDTH && pWork->sub_frame_togle == SK_RIGHT_MODE ){ // 最大
				pWork->sub_frame[ i ]->sprite.pos.x = LOW_WIDTH;
				pWork->sub_frame_next[ i ] = LOW_WIDTH;
				SPR_HIDE( pWork->sub_frame[ i ] );
			}
		}
	}
}

static int AnderSearch( Work *pWork )
{
	int position;
	int i;

//	position = ImaginaryToReal( pWork , pWork->sub_frame_center_this );
	position = ImaginaryToReal( pWork , SK_FRAME_THIS_GET );

	for ( i = 0 ; i < SUB_FRAME_MAX ; i++ ){
//		printf("%x \n", PositionToEnum( position ) );
		if ( !( GM_TnkerCamStatus & PositionToEnum( position ) ) ){
			return ( position );
		}
		position = ( position + 1 ) % SUB_FRAME_MAX;
	}
	return ( 0xff );
}

static void AnderSpaceMove( Work *pWork )
{
	float x_position;
	float       sign;
	char        flag;
	int            i;
	
	// 終了
	flag = 0;
	x_position = 0.0f;

	if ( *pWork->sub_frame_center_this == pWork->sub_frame_this[ pWork->next_position ] ){
		pWork->act = ( void *)NormalAct;
		pWork->auto_mode = 0x0;
		// 親に送信
		GV_CallParentSignalFunc( pWork , SK_SHUTER_READY , 0 );
		return;
	} else if ( pWork->auto_mode & 0x1 ) {
		x_position -= WIDTH;
		pWork->auto_mode = 0x0;
	}
	// 目標に近付く
	for ( i = 0 ; i < SUB_FRAME_MAX ; i ++ ){
		pWork->sub_frame_next[ i ] += x_position;
		if ( pWork->sub_frame[ i ]->sprite.pos.x >= SUB_FRAME_BUF ){
			SPR_HIDE( pWork->sub_frame[ i ] );
		} else {
			SPR_SHOW( pWork->sub_frame[ i ] );
		}
		if ( pWork->sub_frame[ i ]->sprite.pos.x != pWork->sub_frame_next[ i ] ){
			// 移動
			sign = pWork->sub_frame_next[ i ] < pWork->sub_frame[ i ]->sprite.pos.x ? -1.0f : 1.0f;
			pWork->sub_frame[ i ]->sprite.pos.x += ( ( float )( WIDTH / 4.0f ) * sign ); // いまだけ
			// limit check
			if ( pWork->sub_frame[ i ]->sprite.pos.x <= LOW_WIDTH ){ // 最小
				pWork->sub_frame[ i ]->sprite.pos.x = HIGH_WIDTH;
				pWork->sub_frame_next[ i ] = HIGH_WIDTH;
				SPR_HIDE( pWork->sub_frame[ i ] );
			}
		}
		if ( pWork->sub_frame[ i ]->sprite.pos.x == SUB_FRAME_CENTER ){
		    if ( SK_SubFrameRef != NULL ){
				*pWork->sub_frame_center_this = pWork->sub_frame_this[ i ];
//				SK_FRAME_THIS_STORE( pWork->sub_frame_this[ i ] );
				GCL_SetVarRef( SK_SubFrameRef , 0 , *pWork->sub_frame_center_this );
			}
		}
	}
	// 移動仕切ったか判定
	if ( pWork->sub_frame[ *pWork->sub_frame_center_this ]->sprite.pos.x == pWork->sub_frame_next[ *pWork->sub_frame_center_this ] ){
		pWork->auto_mode = 0x1;
	}
}

static void PicDisp( Work *pWork ) // サムネイル表示
{
	short *thumb;
	int position;
	int image_w;
	int image_h;
	int i;

	for ( i = 0 ; i < SUB_FRAME_MAX ; i ++ )
   {
		position = ImaginaryToReal( pWork , pWork->sub_frame_this[ i ] );
		if ( pWork->sub_frame[ i ]->sprite.pos.x >= SUB_FRAME_BUF || pWork->sub_frame[ i ]->sprite.pos.x <= LOW_WIDTH )
      {
			continue;
		} 
      else if (	( GM_TnkerCamStatus & PositionToEnum( position ) ) )
      {
			// 表示
			thumb = SK_PhotoFramePointer( pWork->sub_frame_this[ position ] );
			image_w = 1280;
			image_h = 720;

         BP_MENU_DrawPictureTexEX(( int )pWork->sub_frame[ i ]->sprite.pos.x - SUB_HARF_WIDTH+2,
            ( int )pWork->sub_frame[ i ]->sprite.pos.y - SUB_HARF_HEIGHT+1,
            SUB_WIDTH+1, SUB_HEIGHT+2, pWork->sub_frame_color[ position ], 
            pWork->sub_frame_this[ position ],
            0, 0, image_w, image_h,
            0 );
		} 
      else
      {
			thumb = pWork->empty_raw;
			image_w = 18;
			image_h = 12;

         {
            MENU_DrawPictureTexEX(( int )pWork->sub_frame[ i ]->sprite.pos.x - SUB_HARF_WIDTH+2,
               ( int )pWork->sub_frame[ i ]->sprite.pos.y - SUB_HARF_HEIGHT+1,
               SUB_WIDTH+1, SUB_HEIGHT+2, pWork->sub_frame_color[ position ], pWork->emptyThumbTex,
               0, 0, image_w, image_h,
               0 );
         }
		}
	}
	
}

static void invisible( Work *pWork )
{
	SPR_OBJ *pRoot;

	pRoot = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , STR_ROOT );
	if ( pRoot == NULL ){
		SK_Err( "root\0" );
		return;
	}
	SPR_HIDE( pRoot ); 
	pWork->visible_flag = SK_INVISIBLE;
	pWork->action = 0;
	pWork->act = ( void *)AnimetionAct;
}

static int NormalAct( Work *pWork )
{
	GM_CameraSet *pCam;

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得
    if ( pWork->hold_flag == SK_HOLD_ON || pWork->flag & SK_INVISIBLE ){
		return ( 1 );
	}
	pWork->initialize_flag |= SK_INITIALIZE_OK;
	NumberMove( pWork );
	PicDisp( pWork );
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return( 1 );
	}
	CenterCursor1Move( pWork );
	CenterCursMove( pWork );
	HorizonMove( pWork );
	AnderMove( pWork );
	AlphaAnime( pWork );
	// 更新
	pWork->cam_scale_bak = pCam->angle;
	pWork->cam_rot = pCam->rotate;

#if 0
	// 場所送信
	GV_CallParentSignalFunc( pWork , SK_TANKER_FRAME , pWork->sub_frame_center_this );
#endif

	return ( 0 );
}

static int AnderAct( Work *pWork )
{
	GM_CameraSet *pCam;

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得
	NumberMove( pWork );
	PicDisp( pWork );
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return( 1 );
	}
	CenterCursor1Move( pWork );
	CenterCursMove( pWork );
	HorizonMove( pWork );
	AnderSpaceMove( pWork );
	// 更新
	pWork->cam_scale_bak = pCam->angle;
	pWork->cam_rot = pCam->rotate;

	return ( 0 );
}

static int AnimetionAct( Work *pWork ) // 通常実行
{
	int status;

	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return ( -1 );
	}
	status = L2D_ActionStatus( pWork->handle_2d );
	if ( pWork->action < pWork->action_num ){
		if ( status != L2D_STAT_BUSY && status == L2D_STAT_ACK ){
			L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action );
			pWork->action++;
		}
	} else if ( status != L2D_STAT_BUSY ){
		if ( Initialize( pWork ) < 0 ){
			return ( -1 );
		}
		pWork->act = (void *)NormalAct;
	}
	return ( 0 );
}

static void Act( Work *pWork )
{
	if ( GM_CheckSightStatus( SGT_Camera ) ){
		invisible( pWork );
		return;
	}
	// デモ等のサイトの表示非表示管理
	if ( SthingerSightVisibleInvisible( pWork->handle_2d , STR_ROOT , ( pWork->visible_flag & SK_INVISIBLE ? 1 : 0 ) ) > 0){
		pWork->flag &= ~SK_INVISIBLE;
	} else {
		pWork->flag |= SK_INVISIBLE;
	}
	if ( pWork->visible_flag == SK_INVISIBLE ){
		return;
	}
 	pWork->act( pWork );
	if ( pWork->initialize_flag & SK_INITIALIZE_OK ){ // 毎frame 更新
		ChildStatus( pWork );
	}
}

static	void	Die( Work *work )
{
	int   position;
	int   point;
	int    itmp;
	int       i;
	int   back_up;

	/* これは、dg_cam_mng.cのワークであり、そいつはもう死んでいるので AnderSortで書き変わらないようにNULLにしておく。 */
	work->sub_frame_center_this = NULL ;

	// 死ぬまえにsortする
	if ( work->action < work->action_num || !( work->initialize_flag & SK_INITIALIZE_OK ) ){
		AnderSort( work );
	}
	itmp = SK_FRAME_THIS_GET - 2;
	if ( itmp < 0 ){
		itmp += 6;
	}
	back_up = 0;
	for ( i = 0 ; i < SUB_FRAME_MAX ; i++ ){
		point = itmp + i;
		point %= 6;
		position = ImaginaryToReal( work , point );
	   	if ( GM_TnkerCamStatus & PositionToEnum( position ) ){
			back_up |= ( 1 << i );
		}
	}
	back_up |= GM_TnkerCamStatus & 0xff00;
	GM_TnkerCamStatus = back_up;
	// l2d 解放
	if ( work->handle_2d >= 0 )
   {
		L2D_ReleaseLayout( work->handle_2d );
	}
	{
		DG_FreeLinerTexture( work->emptyThumbTex );
      free(work->empty_raw);
	}
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
 	void        *ptr;
	int			handle;

	// シグナルの登録
	GV_SetActorSignalFunc( work , ReceiveSignal );

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout( LAYOUT , DG_CHANL_MAIN , 0, 0 ) ;
	if ( handle < 0 ){
		return ( -1 );
	}
	work->handle_2d = handle ;

	// 全SHOW
	L2D_EvokeAction( work->handle_2d , STR_DEFAULT ) ;

	// etc
	work->dup_count.sub_frame = 0;
 	work->center_curs1_scale = 1.0f;
	work->cam_scale_bak = 2.0f;
	work->pad_info = SK_PAD_NORMAL;
	work->action_num = L2D_GetActionNumber( work->handle_2d );
	work->action = 1;
	work->act = (void *)AnimetionAct;
	work->visible_flag = SK_INVISIBLE;
	work->hold_flag = SK_HOLD_OFF;
	work->auto_mode = 0x0;
	work->flag = SK_INITIALIZE_OK;
	work->initialize_flag = 0;
	/* MENU_Printfキャラ生成 */
	if ( GM_CheckGameStatus( STATE_PLAY_DEMO ) ) { // demo mode
		work->menuprint_work_ptr = NewMenuPrintManager( ( 8 * 1024 ) , DG_DMAPACK_NORMAL | DG_DMAPACK_INVISIBLE1 | DG_DMAPACK_INVISIBLE2 |
														DG_DMAPACK_INVISIBLE3 | DG_DMAPACK_PRIVILEGE , DG_DMAPACK_PHASE_AFTER , 144 );
	} else { // normal mode
		work->menuprint_work_ptr = NewMenuPrintManager( ( 8 * 1024 ) , DG_DMAPACK_MENU, DG_DMAPACK_PHASE_NORMAL, 0 );
	}
	if ( work->menuprint_work_ptr == NULL ){
		return ( -1 );
	}
	GV_SetActorChild( work , work->menuprint_work_ptr );

	ptr = ( void * )GV_GetCache( GV_CacheID( EMPTY_PIC , 'r' ) );
	if ( ptr == NULL )
   {
		SK_Err("empty_raw no data\0");
	}
   work->empty_raw = malloc(18*12*sizeof(u_short));
   {
      int i;
      int r, g, b;
      u_short* dmdm = work->empty_raw;
      u_short* ptrShort = ptr;
      for( i = 0; i < 18*12; i ++ )
      {
         *dmdm = *ptrShort;
         BP_LE_SwapUShort_Inp(dmdm);
         dmdm ++ ; 
         ptrShort ++ ;
      }
   }
	{
		work->emptyThumbTex = DG_MakeLinerTexture( THUMB_W, THUMB_H, DG_TEXLIN_FORMAT_X1R5G5B5 );
	}
   {
      int image_w = 18;
      int image_h = 12;

      {
         int n;
         short * dmdm = work->emptyThumbTex->image;
         short * thumb = work->empty_raw;
         for( n = 0; n < image_h; n++ )
         {
            memcpy( dmdm, thumb, image_w * sizeof(short) );
            dmdm += THUMB_W;/* 64 */
            thumb += image_w;/* 18 */
         }
         work->emptyThumbTex->image_dirty = 1;// TODO: need a way to cache off if changes are necessary or not
      }
   }

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewTDgCamSight( float min , float max , int *frame )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 48 ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->angle_max = max;
	work->angle_min = min;
	work->sub_frame_center_this = frame;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

void NewTdgCameraStatusOff( void ) // シナリオから指定された場所をinitializeする
{
	int position;

	position = GCL_GetOptionValue( 'f' , 0 );
	GM_TnkerCamStatus &= ~PositionToEnum( position );
}

int NewTdgCameraFrameStatusGet( void ) // シナリオから指定された場所の情報を得る
{
	int position;

	position = GCL_GetOptionValue( 'f' , 0 );

	return ( GM_TnkerCamStatus & PositionToEnum( position ) );
}

#define STR_CELGEI_SNAPSHOT_BIGSIZE (8819669)
void NewTdgCameraInitialize( void ) // 全部黒初期化
{
	void *tmp;
	void *data;
	void *resident;
	int  frame;
	int  strcode;
	int  itmp;


	strcode = GCL_GetOptionValue( 's' , 0 );
	frame = GCL_GetOptionValue( 'f' , 0 );
	itmp = frame - 2;
	if ( itmp < 0 ){
		itmp += 6;
	}
	tmp = SK_PhotoFramePointer( itmp );
	GV_ZeroMemory( tmp , THUMB_W * THUMB_H * sizeof( short ) ); // せるげい転送
	// 真中にせるげい
	if ( strcode != 0 )
   {
		data = ( void * )GV_GetCache( GV_CacheID( strcode , 'r' ) );
		if ( data != NULL )
      {
			{
				int i;
				int r, g, b;
				u_short* dmdm = data;
				for( i = 0; i < THUMB_W*THUMB_H; i ++ )
            {
               BP_LE_SwapUShort_Inp(dmdm);
					r = *dmdm & 0x1F;
					g = ( *dmdm >> 5 ) & 0x1F;
					b = ( *dmdm >> 10 ) & 0x1F;
					 *dmdm = b | ( g << 5 ) | ( r << 10 ) | ( 1 << 15 );
					dmdm ++ ; 
				}
			}		
			memcpy( tmp , data , THUMB_W * THUMB_H * sizeof( short ) ); // せるげい転送
		}
		resident = GV_GetCache( 0x1F6AF7A3 );	// tank_jpeg_cam | ( 0x7F << 24 )
		data = ( void * )GV_GetCache( GV_CacheID( STR_CELGEI_SNAPSHOT_BIGSIZE , 'r' ) );
      BP_CaptureScreenShot_Tanker_FromJPEG_MemBuffer(5, data); // Load JPEG into tanker screenshot # 5
		tmp = (char *)resident + ( THUMB_W * THUMB_H * sizeof( short ) ) * TANKER_MAX_CAPTURE
			+ TANKER_MAX_CODE * itmp;
		if ( data != NULL && tmp != NULL )
      {
			memcpy( tmp , data , TANKER_MAX_CODE ); // jpegせるげい転送
		}
	}
	GM_TnkerCamStatus |= PositionToEnum( frame );
}

void NewTdgCameraStatus( void )
{
	static GCL_VAR_REF ref;

	GCL_GetOption( 'd' );  // 位置の取得
	GCL_GetNextVarRef( &ref );

	SK_SubFrameRef = &ref;
}
