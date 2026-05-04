//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   scope_layout.c
   スコープサイト
   
   2001/05/17	S.Kobayashi
   $Id: scope_layout.c,v 1.1.1.3 2002/11/19 11:50:30 Yoshizawa1 Exp $
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

#define	LAYOUT	     	     (6507116)	/* scope.o2d */
#define STR_ZOOM_GRAPH_PROG  (14951237)
#define STR_GRAPH_LR_R       (8190261)
#define STR_GRAPH_LR_L       (8190255)
#define STR_CENTER_DOT_B     (11588776)
#define STR_CENTER_DOT_T     (11588794)
#define STR_8BAR_PROG_1      (16634276)
#define STR_8BAR_PROG_2      (16634277)
#define STR_8BAR_PROG_3      (16634278)
#define STR_8BAR_PROG_4      (16634279)
#define STR_8BAR_PROG_5      (16634280)
#define STR_8BAR_PROG_6      (16634281)
#define STR_8BAR_PROG_7      (16634282)
#define STR_8BAR_PROG_8      (16634283)
#define STR_CENTER_BOX_DUMMY (11144365)
#define STR_CENTER_LINE      (3763688)
#define STR_DEFAULT          (566267)
#define STR_ROOT             (2770484)

#define GRAPH_MAX  ( 4 )
#define NUMBER_MAX ( 16 )
#define NUM_WIDTH  ( 60 )
#define BAR_MAX    ( 8 )
#define ANGLE_MAX (21.5f)
#define DEFAULT_Y ( DIRECT_SCREEN_Y( -10.0f ) )
#define ZOOM_DW ( 81 )

enum {
	SK_UN_INITIALIZE    = 0x1,
	SK_INITIALIZE_START = 0x2,
	SK_INITIALIZE_OK    = 0x4,
	SK_INVISIBLE        = 0x8,
};

typedef struct {
	int                 number;
	FVECTOR             number_pos;
	FVECTOR             parent_pos;
} Number;

typedef struct {
	int graph_l;
	int graph_r;
} DupCount;

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	int                 action_num;
	int                 action;
	int                 time;
	SPR_OBJ             *zoom_graph_prog;
	SPR_OBJ             *graph_l[ GRAPH_MAX ];
	SPR_OBJ             *graph_r[ GRAPH_MAX ];
	SPR_OBJ             *center_dot_b;
	SPR_OBJ             *center_dot_t;
	SPR_OBJ             *bar_prog[ BAR_MAX ];
	SPR_OBJ             *center_box_dummy;
	SPR_OBJ             *center_line;
	float                cam_scale_bak;
	float                center_box_scale;
	float                zoom_raute; // のびちぢむ比率
	GM_CameraSet		 *pCam;
	FVECTOR              bar_pos[ BAR_MAX ];
	Number               Num[ NUMBER_MAX ];
	SVECTOR              cam_rot;
	DupCount             dup_count;
	float                raute;
	int                  flag;
	void                 *menuprint_work_ptr;
	void  ( *act )( struct _work * );
} Work ;

// プロトタイプ
static int AnimetionAct( Work * ); // アニメーション

// 初期化関数
static int Initialize( Work *pWork )
{
	GM_CameraSet *pCam;
	SPR_OBJ *spr;
	float tmp_width , tmp_height;
	int strcode , i;

#if 0 /*yano 2002.03.19*/
	tmp_width = ( float )( DRAW_WIDTH / 2 );
	tmp_height = ( float )( DRAW_HEIGHT / 2 );
#else
	tmp_width = ( float )( SPR_SCRN_WIDTH / 2 );
	tmp_height = ( float )( SPR_SCRN_HEIGHT / 2 );
#endif

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得

	pWork->zoom_graph_prog = L2D_GetObject( pWork->handle_2d, STR_ZOOM_GRAPH_PROG );
	if ( pWork->zoom_graph_prog == NULL ){
		SK_Err( "zoom_graph_prog\0" );
		return ( -1 );
	}
	// graph_r
	spr = L2D_GetObject( pWork->handle_2d, STR_GRAPH_LR_R );
	if ( spr == NULL ){
		SK_Err( "graph_r\0" );
		return ( -1 );
	}
	for ( i = pWork->dup_count.graph_r ; i < GRAPH_MAX ; i ++ ){
		pWork->graph_r[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->graph_r[ i ] == NULL ){
#if 0
			SK_FreeMemory( pWork->graph_r , i );
#endif
			return ( -1 );			
		} else {
			pWork->dup_count.graph_r++;
		}
	}
	SPR_HIDE( spr );
	// graph l
	spr = L2D_GetObject( pWork->handle_2d, STR_GRAPH_LR_L );
	if ( spr == NULL ){
		SK_Err( "graph_l\0" );
		return ( -1 );
	}
	for ( i = pWork->dup_count.graph_l ; i < GRAPH_MAX ; i ++ ){
		pWork->graph_l[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->graph_l[ i ] == NULL ){
#if 0
			SK_FreeMemory( pWork->graph_r , GRAPH_MAX );
			SK_FreeMemory( pWork->graph_l , i );
#endif
			return ( -1 );			
		} else {
			pWork->dup_count.graph_l++;
		}
	}
	SPR_HIDE( spr );
	pWork->center_dot_b = L2D_GetObject( pWork->handle_2d, STR_CENTER_DOT_B );
	if ( pWork->center_dot_b == NULL ){
		SK_Err( "center_dot_b\0" );
		return ( -1 );
	}
	pWork->center_dot_t = L2D_GetObject( pWork->handle_2d, STR_CENTER_DOT_T );
	if ( pWork->center_dot_t == NULL ){
		SK_Err( "center_dot_t\0" );
		return ( -1 );
	}
	pWork->center_box_dummy = L2D_GetObject( pWork->handle_2d, STR_CENTER_BOX_DUMMY );
	if ( pWork->center_box_dummy == NULL ){
		SK_Err( "center_box_dummy\0" );
		return ( -1 );
	}
	pWork->center_line = L2D_GetObject( pWork->handle_2d, STR_CENTER_LINE );
	if ( pWork->center_line == NULL ){
		SK_Err( "center_line\0" );
		return ( -1 );
	}
	strcode = STR_8BAR_PROG_1;
	for ( i = 0 ; i < BAR_MAX ; i++ ){
		pWork->bar_prog[ i ] = L2D_GetObject( pWork->handle_2d, strcode );
		if ( pWork->bar_prog[ i ] == NULL ){
			SK_Err( "8bar_prog\0" );
			return ( -1 );
		}
		strcode++;
	}
	// initialize
	for ( i = 0 ; i < BAR_MAX ; i++ ){
		pWork->bar_pos[ i ].vx = 0.f;
		pWork->bar_pos[ i ].vy = pWork->bar_prog[ i ]->box.rect.begin.y;
		pWork->bar_pos[ i ].vz = 0.f;
		pWork->bar_pos[ i ].vw = 0.f;
	}
	// lr
	for ( i = 0 ; i < GRAPH_MAX ; i++ ){
		// l
		pWork->graph_l[ i ]->line.pos[ 0 ].x += ( 13 * ( i - 1 ) ); 
		pWork->graph_l[ i ]->line.pos[ 1 ].x += ( 13 * ( i - 1 ) ); 
		pWork->graph_l[ i ]->line.pos[ 0 ].y +=  ( 6 * ( i - 1 ) ); 
		pWork->graph_l[ i ]->line.pos[ 1 ].y -=  ( 6 * ( i - 1 ) ); 
		// r
		pWork->graph_r[ i ]->line.pos[ 0 ].x -= ( 13 * ( i - 1 ) ); 
		pWork->graph_r[ i ]->line.pos[ 1 ].x -= ( 13 * ( i - 1 ) ); 
		pWork->graph_r[ i ]->line.pos[ 0 ].y +=  ( 6 * ( i - 1 ) ); 
		pWork->graph_r[ i ]->line.pos[ 1 ].y -=  ( 6 * ( i - 1 ) ); 
	}
	// 数字
	{
		int itmp;

		itmp = ( NUMBER_MAX / 2 ) * 30;
		for ( i = 0 ; i < NUMBER_MAX ; i++ ){
			pWork->Num[ i ].number_pos.vx = ( tmp_width - itmp - 8 ) + 30 * i;
		}
	}
	// 比率
	pWork->zoom_raute = ( ZOOM_DW / ( ANGLE_MAX * pWork->raute ) );
	// rotの初期化
	pWork->cam_rot = pCam->rotate;
	pWork->cam_scale_bak = pCam->angle;
#if 0
	// default下駄はかせ
	{
		SPR_OBJ *pSpr;
		
		pSpr = L2D_GetObject( pWork->handle_2d, STR_ROOT );
		if ( pSpr == NULL ){
			SK_Err( "root\0" );
		}
		pSpr->empty.pos.y += DEFAULT_Y;
	}
#endif
	return ( 0 );	
}

#define CENTER_BOX_MAX ( 10 )
#define CENTER_BOX_SPEED_X (1)
#define CENTER_BOX_SPEED_Y (1)
static void CenterBoxMove( Work *pWork )
{
	FVECTOR       stmp;
	FVECTOR       fvec;
	FVECTOR      ftmp3;
	float         ftmp;
	float        ftmp2;
	float    tmp_width;
	float   tmp_height;
	float            r;
	int           sign;

	tmp_width  = ( float )( SPR_SCRN_WIDTH / 2 );
	tmp_height = ( float )( SPR_SCRN_HEIGHT / 2 );
	// カメラの取得
	stmp.vx = pWork->pCam->rotate.vx - pWork->cam_rot.vx; // 移動した角度を入力
	stmp.vy = pWork->pCam->rotate.vy - pWork->cam_rot.vy; // 移動した角度を入力
	stmp.vx *= ( 360.0f / 4096.0f );
	stmp.vy *= ( 360.0f / 4096.0f );

	r = 60.f * pWork->pCam->angle;
	ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vz  = ( ( ftmp * stmp.vx ) / 360 );
	stmp.vw  = ( ( ftmp * stmp.vy ) / 360 );
	// スピード補正
	sign = stmp.vz < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vz < -CENTER_BOX_SPEED_X ){
			stmp.vz = -CENTER_BOX_SPEED_X;
		}
	} else {
		if ( stmp.vz > CENTER_BOX_SPEED_X ){
			stmp.vz = CENTER_BOX_SPEED_X;
		}
	}
	// スピード補正
	sign = stmp.vw < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vw < -CENTER_BOX_SPEED_Y ){
			stmp.vw = -CENTER_BOX_SPEED_Y;
		}
	} else {
		if ( stmp.vw > CENTER_BOX_SPEED_Y ){
			stmp.vw = CENTER_BOX_SPEED_Y;
		}
	}
	pWork->center_box_dummy->empty.pos.x -= ( int )( stmp.vw );
	pWork->center_box_dummy->empty.pos.y += ( int )( stmp.vz );
	// 範囲ない制御
	ftmp2 = stmp.vz < 0 ? -stmp.vz : stmp.vz;
	if ( stmp.vz != 0 ){
		if ( pWork->center_box_dummy->empty.pos.y < -CENTER_BOX_MAX + tmp_height + DEFAULT_Y ){
			pWork->center_box_dummy->empty.pos.y = -CENTER_BOX_MAX + tmp_height + DEFAULT_Y ;
		}
		if ( pWork->center_box_dummy->empty.pos.y > CENTER_BOX_MAX + tmp_height + DEFAULT_Y ){
			pWork->center_box_dummy->empty.pos.y = CENTER_BOX_MAX + tmp_height + DEFAULT_Y;
		}
	}
	ftmp2 = stmp.vw < 0 ? -stmp.vw : stmp.vw;
	if ( stmp.vw != 0 ){
		if ( pWork->center_box_dummy->empty.pos.x < -CENTER_BOX_MAX + tmp_width ){
			pWork->center_box_dummy->empty.pos.x = -CENTER_BOX_MAX + tmp_width;
		}
		if ( pWork->center_box_dummy->empty.pos.x > CENTER_BOX_MAX + tmp_width ){
			pWork->center_box_dummy->empty.pos.x = CENTER_BOX_MAX + tmp_width;
		}
	}
	// 戻す
	if ( stmp.vz == 0 && stmp.vw == 0 ){
		stmp.vx = pWork->center_box_dummy->empty.pos.x;
		stmp.vy = pWork->center_box_dummy->empty.pos.y;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3.vx = tmp_width;
		ftmp3.vy = tmp_height + DEFAULT_Y;
		ftmp3.vz = 0;
		ftmp3.vw = 0;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 4 );
		pWork->center_box_dummy->empty.pos.x += fvec.vx;
		pWork->center_box_dummy->empty.pos.y += fvec.vy;
	}
	// 拡縮
	{ 	
	 	float scale;
		// 補間
		if ( pWork->cam_scale_bak == pWork->pCam->angle ){ // 元に戻す
			scale = ( float )( 1.0f - pWork->center_box_scale ) / 4.f;
			pWork->center_box_scale += scale;
		} else {
			scale = pWork->pCam->angle - pWork->cam_scale_bak; // default 2.0だから
			scale = scale < 0 ? 1.125f : 0.906f;
			pWork->center_box_scale += ( float )( scale - pWork->center_box_scale ) / 10.f;
			// 拡大
			if ( pWork->center_box_scale < 0.906F ){
				pWork->center_box_scale = 0.906F;
			}
			if ( pWork->center_box_scale > 1.125F ){
				pWork->center_box_scale = 1.125F;
			}	
		}
		// 更新
		SPR_MAG( pWork->center_box_dummy , pWork->center_box_scale );
	}
}

#define CENTER_DOT_MAX (10)
#define CENTER_DOT_SPEED_X (1)
#define CENTER_DOT_SPEED_Y (1)
#define CENTER_DOT_T_DEFAULT_X (254)
#define CENTER_DOT_T_DEFAULT_Y (182)
#define CENTER_DOT_B_DEFAULT_X (254)
#define CENTER_DOT_B_DEFAULT_Y (193) // 192
#define CENTER_DOT_T_HEIGHT    (10)
#define CENTER_DOT_B_HEIGHT    (3)
static void CenterDotMove( Work *pWork ) // 真中の処理をすべて統括
{
	FVECTOR       stmp;
	FVECTOR       fvec;
	FVECTOR      ftmp3;
	float         ftmp;
	float        ftmp2;
	float    tmp_width;
	float   tmp_height;
	float            r;
	int           sign;

#if 0 /*yano 2002.03.19*/
	tmp_width  = ( float )( DRAW_WIDTH / 2 );
	tmp_height = ( float )( DRAW_HEIGHT / 2 ) - DIRECT_SCREEN_Y( 32 );
#else
	tmp_width  = ( float )( SPR_SCRN_WIDTH / 2 );
	tmp_height = ( float )( SPR_SCRN_HEIGHT / 2 );
#endif
	stmp.vx = pWork->pCam->rotate.vx - pWork->cam_rot.vx; // 移動した角度を入力
	stmp.vy = pWork->pCam->rotate.vy - pWork->cam_rot.vy; // 移動した角度を入力
	stmp.vx *= ( 360.f / 4096.f );
	stmp.vy *= ( 360.f / 4096.f );

	r = 50.f * pWork->pCam->angle;;
	ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vz  = ( ( ftmp * stmp.vy ) / 360 );
	stmp.vw  = ( ( ftmp * stmp.vx ) / 360 );
	// スピード補正
	sign = stmp.vz < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vz < -CENTER_DOT_SPEED_X ){
			stmp.vz = -CENTER_DOT_SPEED_X;
		}
	} else {
		if ( stmp.vz > CENTER_DOT_SPEED_X ){
			stmp.vz = CENTER_DOT_SPEED_X;
		}
	}
	sign = stmp.vw < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vw < -CENTER_DOT_SPEED_Y ){
			stmp.vw = -CENTER_DOT_SPEED_Y;
		}
	} else {
		if ( stmp.vw > CENTER_DOT_SPEED_Y ){
			stmp.vw = CENTER_DOT_SPEED_Y;
		}
	}
	// dot_t
	pWork->center_dot_t->sprite.pos.x -= stmp.vz;
	pWork->center_dot_t->sprite.pos.y -= stmp.vw;
	// dot_b
	pWork->center_dot_b->sprite.pos.x += stmp.vz;
	pWork->center_dot_b->sprite.pos.y -= stmp.vw;
	// 中心のbarはY軸にのみ同期
	pWork->center_line->line.pos[ 0 ].y -= stmp.vw;  
	pWork->center_line->line.pos[ 1 ].y -= stmp.vw;  
	// 範囲ない制御
	ftmp2 = stmp.vz < 0 ? -stmp.vz : stmp.vz;
	if ( stmp.vz != 0 ){
		// dot_t
		if ( pWork->center_dot_t->sprite.pos.x < -CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_X ){
			pWork->center_dot_t->sprite.pos.x = -CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_X ;
		}
		if ( pWork->center_dot_t->sprite.pos.x > CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_X ){
			pWork->center_dot_t->sprite.pos.x = CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_X;
		}
		// dot_b
		if ( pWork->center_dot_b->sprite.pos.x < -CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_X ){
			pWork->center_dot_b->sprite.pos.x = -CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_X ;
		}
		if ( pWork->center_dot_b->sprite.pos.x > CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_X ){
			pWork->center_dot_b->sprite.pos.x = CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_X;
		}
	}
	// 範囲ない制御
	ftmp2 = stmp.vw < 0 ? -stmp.vw : stmp.vw;
	if ( stmp.vw != 0 ){
		// dot_t
		if ( pWork->center_dot_t->sprite.pos.y < -CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_Y + DEFAULT_Y ){
			pWork->center_dot_t->sprite.pos.y = -CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_Y + DEFAULT_Y;
		}
		if ( pWork->center_dot_t->sprite.pos.y > CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_Y + DEFAULT_Y ){
			pWork->center_dot_t->sprite.pos.y = CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_Y + DEFAULT_Y;
		}
		// dot_b
		if ( pWork->center_dot_b->sprite.pos.y < -CENTER_DOT_MAX + CENTER_DOT_B_DEFAULT_Y + DEFAULT_Y ){
			pWork->center_dot_b->sprite.pos.y = -CENTER_DOT_MAX + CENTER_DOT_B_DEFAULT_Y + DEFAULT_Y;
		}
		if ( pWork->center_dot_b->sprite.pos.y > CENTER_DOT_MAX + CENTER_DOT_B_DEFAULT_Y + DEFAULT_Y ){
			pWork->center_dot_b->sprite.pos.y = CENTER_DOT_MAX + CENTER_DOT_B_DEFAULT_Y + DEFAULT_Y;
		}
		// bar
		if ( pWork->center_line->line.pos[ 0 ].y < -CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_Y + CENTER_DOT_T_HEIGHT + DEFAULT_Y ){
			pWork->center_line->line.pos[ 0 ].y = -CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_Y + CENTER_DOT_T_HEIGHT + DEFAULT_Y;
			pWork->center_line->line.pos[ 1 ].y = -CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_Y + CENTER_DOT_T_HEIGHT + DEFAULT_Y;
		}
		if ( pWork->center_line->line.pos[ 0 ].y > CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_Y + CENTER_DOT_T_HEIGHT + DEFAULT_Y ){
			pWork->center_line->line.pos[ 0 ].y = CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_Y + CENTER_DOT_T_HEIGHT + DEFAULT_Y;
			pWork->center_line->line.pos[ 1 ].y = CENTER_DOT_MAX + CENTER_DOT_T_DEFAULT_Y + CENTER_DOT_T_HEIGHT + DEFAULT_Y;
		}
	}
	// 戻す
	if ( stmp.vz == 0 && stmp.vw == 0 ){
		stmp.vx = pWork->center_dot_t->sprite.pos.x;
		stmp.vy = pWork->center_dot_t->sprite.pos.y;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3.vx = CENTER_DOT_T_DEFAULT_X;
		ftmp3.vy = CENTER_DOT_T_DEFAULT_Y + DEFAULT_Y;
		ftmp3.vz = 0;
		ftmp3.vw = 0;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 6 );
		pWork->center_dot_t->sprite.pos.x += fvec.vx;
		pWork->center_dot_t->sprite.pos.y += fvec.vy;
		pWork->center_line->line.pos[ 0 ].y += fvec.vy;
		pWork->center_line->line.pos[ 1 ].y += fvec.vy;
		// dot_b
		stmp.vx = pWork->center_dot_b->sprite.pos.x;
		stmp.vy = pWork->center_dot_b->sprite.pos.y;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3.vx = CENTER_DOT_B_DEFAULT_X;
		ftmp3.vy = CENTER_DOT_B_DEFAULT_Y + DEFAULT_Y;
		ftmp3.vz = 0;
		ftmp3.vw = 0;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 6 );
		pWork->center_dot_b->sprite.pos.x += fvec.vx;
		pWork->center_dot_b->sprite.pos.y += fvec.vy;
	}
}

#ifdef KP_XBOX /*yano 2002.03.19*/
#define TO_XBOX640(_a) ((_a)/512.0f*640.0f)
#endif
#define NUMBER_SPEED_X 4
static void NumberMove( Work *pWork )
{
	float parent_x , parent_y;
	float tmp ;
	float tmp_width;
	int i;
	int sign;
	char text_buffer[ 8 ];
	float tmp2 , r;
	float diff;

   float const vitaCenterOffset = 3.95f;

	if ( pWork->flag & SK_INVISIBLE ){
		return;
	}
#if 0 /*yano 2002.03.19*/
	tmp_width = ( ( float )DRAW_WIDTH / 2.0f );
#else
	tmp_width = ( ( float )SPR_SCRN_WIDTH / 2.0f );
#endif

	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		tmp2 = 0.0f;
	} else {
		tmp = pWork->pCam->rotate.vy - pWork->cam_rot.vy; // 移動した角度を入力
		tmp *= ( 360.f / 4096.f );

		r = 100.f;
		tmp2 = 2 * M_PI * r; // 31半径のこと(円周をだす)
		tmp2  = ( ( tmp2 * tmp ) / 360 );
		// スピード補正
		sign = tmp2 < 0.f ? -1 : 1;
		if ( sign < 0 ){
			if ( tmp2 < -NUMBER_SPEED_X ){
				tmp2 = -NUMBER_SPEED_X;
			}
		} else {
			if ( tmp2 > NUMBER_SPEED_X ){
				tmp2 = NUMBER_SPEED_X;
			}
		}
	}
	{
		int itmp;
		int alpha;

		tmp = ( 384.f / 448.f );
		itmp = ( NUMBER_MAX / 2 ) * 30;
		MENU_SetAlphaMode( 0, 2, 0, 1, 0 ) ;
		for ( i = 0 ; i < NUMBER_MAX ; i++ ){ 
			// 移動
			pWork->Num[ i ].number_pos.vx += tmp2;
			if ( pWork->Num[ i ].number_pos.vx > tmp_width + 240 ){
				diff = pWork->Num[ i ].number_pos.vx - ( 240 + tmp_width );
				pWork->Num[ i ].number_pos.vx = diff + tmp_width - 240;
			}
			if ( pWork->Num[ i ].number_pos.vx < tmp_width - 240 ){
				diff = pWork->Num[ i ].number_pos.vx - ( tmp_width - 240 );
				pWork->Num[ i ].number_pos.vx = diff + tmp_width + 240;
			}
			// 表示
			alpha = 48 - (( int )DG_FABS( ( pWork->Num[ i ].number_pos.vx - tmp_width ) ) / 2 );
			if ( alpha <= 0 ){
				alpha = 0;
				continue;
			}
#ifdef PSX2 /*yano 2002.03.19*/
			parent_x = pWork->Num[ i ].number_pos.vx + vitaCenterOffset;
#else
			parent_x = TO_XBOX640( pWork->Num[ i ].number_pos.vx );
#endif
			parent_y = DIRECT_SCREEN_Y( ( 348.0f ) ) + DEFAULT_Y;
			sprintf( text_buffer , "%02d" , ( ( i + 8 ) % 16 ) );
			___MENU_Locate( pWork->menuprint_work_ptr, 2, parent_x , parent_y  , 0 );
			___MENU_Color( pWork->menuprint_work_ptr, 2, 180, 250, 90, alpha );
			___MENU_PrintMini( pWork->menuprint_work_ptr, 2, text_buffer );
#if 0
			parent_x = pWork->Num[ i ].number_pos.vx;
			parent_y = DIRECT_SCREEN_Y( ( 402.0f + DEFAULT_Y ) ) * tmp;
			MENU_S_Locate( parent_x , parent_y , 0 );
			MENU_S_Color( 50 , 110 , 50 , alpha );
			MENU_S_Printf( "%02d" , ( ( i + 8 ) % 16 ) );
#endif
		}
	}
}

static void ZoomControl( Work *pWork )
{
	pWork->zoom_graph_prog->sprite.dw = ( ( pWork->pCam->angle - 2.0f ) * pWork->zoom_raute );
	pWork->zoom_graph_prog->sprite.dw = pWork->zoom_graph_prog->sprite.dw > ZOOM_DW ? ZOOM_DW : pWork->zoom_graph_prog->sprite.dw;
}

static void AngleDisp( Work *pWork )
{
	float tmp;
	float parent_x , parent_y;
	char text_buffer[ 16 ];
	int high , low;

	tmp = ( 384.f / 448.f );

	if ( pWork->flag & SK_INVISIBLE ){
		return;
	}
#ifdef PSX2 /*yano 2002.03.19*/
 	parent_x = 378;
#else
 	parent_x = 472.5;
#endif
	parent_y = DIRECT_SCREEN_Y( ( 90.f ) ) + DEFAULT_Y;
	high = ( int )pWork->pCam->rotate.vx & 0x7ff;
	low = ( int )pWork->pCam->rotate.vy & 0x7ff;
	sprintf( text_buffer , "%04d %04d" , high , low );
	___MENU_Locate( pWork->menuprint_work_ptr, 2, parent_x , parent_y  , 0 );
	___MENU_Color( pWork->menuprint_work_ptr, 2, 180, 250, 90, 80 );
	___MENU_PrintMini( pWork->menuprint_work_ptr, 2, text_buffer );
#if 0
	MENU_S_Locate( parent_x , parent_y , 0 );
	MENU_S_Color( 180 , 250 , 90 , 20 );
	MENU_S_Printf( "%04d %04d" , high , low );
#endif
}

static void GraphMove( Work *pWork )
{
	float sign;
	float ftmp;
	float default_raute;
	int  i;

	sign = 0.0f;
	default_raute = ( 384.0f / 448.0f );
	// 変化なし
	if ( pWork->pCam->angle == pWork->cam_scale_bak ){
		return;
	}
	// zoom中、中心ひきよせ
	if ( pWork->pCam->angle > pWork->cam_scale_bak ){
		sign = 1.0f;
	}
	// unzoom中、外心ひきよせ?
	if ( pWork->pCam->angle < pWork->cam_scale_bak ){
		sign = -1.0f;
	}
	for ( i = 0 ; i < GRAPH_MAX ; i++ ){
		// l
		pWork->graph_l[ i ]->line.pos[ 0 ].x += 1.0f * sign; 
		pWork->graph_l[ i ]->line.pos[ 1 ].x  = pWork->graph_l[ i ]->line.pos[ 0 ].x;
		// r
		pWork->graph_r[ i ]->line.pos[ 0 ].x -= 1.0f * sign; 
		pWork->graph_r[ i ]->line.pos[ 1 ].x  = pWork->graph_r[ i ]->line.pos[ 0 ].x;
		// スクロール
		if ( pWork->graph_l[ i ]->line.pos[ 0 ].x < -11.0f ){ // LRどっちか片方だけでよい(Lを採用)
			pWork->graph_l[ i ]->line.pos[ 0 ].x = 42.0f;
			pWork->graph_l[ i ]->line.pos[ 1 ].x = 42.0f;
			pWork->graph_r[ i ]->line.pos[ 0 ].x = 470.0f;
			pWork->graph_r[ i ]->line.pos[ 1 ].x = 470.0f;
		}
		if ( pWork->graph_l[ i ]->line.pos[ 0 ].x > 42.0f ){ // どっちか片方だけでよい
			pWork->graph_l[ i ]->line.pos[ 0 ].x = -11.0f;
			pWork->graph_l[ i ]->line.pos[ 1 ].x = -11.0f;
			pWork->graph_r[ i ]->line.pos[ 0 ].x = 521.0f;
			pWork->graph_r[ i ]->line.pos[ 1 ].x = 521.0f;
		}
		// 共通
		ftmp = ( 62.0F - 42.0F ) * ( ( pWork->graph_l[ i ]->line.pos[ 0 ].x - 13.0F ) / ( 43.0F + 13.0F ) ) ;
		pWork->graph_l[ i ]->line.pos[ 0 ].y = ( 62.0F + DEFAULT_Y + ftmp );
		pWork->graph_l[ i ]->line.pos[ 1 ].y = ( 328.0F + DEFAULT_Y - ftmp );
		pWork->graph_r[ i ]->line.pos[ 0 ].y = ( 62.0F + DEFAULT_Y + ftmp );
		pWork->graph_r[ i ]->line.pos[ 1 ].y = ( 328.0F + DEFAULT_Y - ftmp );
	}
}

// いんちきグラフ 
#define LIRE_MAX (190)
static void LireGraphMove( Work *pWork )
{
	float		y, py ;
	float       default_raute;
	int			i ;

	default_raute = ( 384.0f / 448.0f );
	for ( i = 0; i < BAR_MAX; i ++ ) {
		py = pWork->bar_prog[ i ]->box.rect.begin.y;
		y = ( float )308 - ( float )( ( ( pWork->cam_rot.vy & 4095 ) * i / ( 9 - i ) 
									   + pWork->cam_rot.vx ) % ( 308 - 192 ) ) ;
		if ( DG_FABS( ( pWork->center_box_scale - 1.0f ) ) >= 0.01f ) {
			y += BP_PS2_rand() % 64 ;
		}
		y = y * default_raute;
		y = GV_NearExp8F( py, y ) ;
		if ( y < 182 ) y = 182 ;
		if ( y >= 292.0F ) y = 292.0F ;
		pWork->bar_prog[ i ]->box.rect.begin.y = y;
	}
}

static void invisible( Work *pWork )
{
	SPR_OBJ *pRoot;

	pRoot = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , STR_ROOT );
	if ( pRoot == NULL ){
		SK_Err( "Root\0" );
		return;
	}
	SPR_HIDE( pRoot ); 
	pWork->action = 0;
	pWork->act = ( void *)AnimetionAct;
}

static int NormalAct( Work *pWork )
{
	pWork->pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得
	NumberMove( pWork );
	AngleDisp( pWork );
	// ポーズ中かどうか
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return ( 0 );
	}
	CenterBoxMove( pWork );
	CenterDotMove( pWork );
	ZoomControl( pWork );
	GraphMove( pWork );
	LireGraphMove( pWork );
	// 更新
	pWork->cam_scale_bak = pWork->pCam->angle;
	pWork->cam_rot = pWork->pCam->rotate;

	return ( 0 );
}

static int AnimetionAct( Work *pWork ) // 通常実行
{
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return ( -1 );
	}
	if ( pWork->action < pWork->action_num ){
		L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action );
		pWork->action++;
	} else if ( L2D_ActionStatus( pWork->handle_2d ) == L2D_STAT_ACK ){
		if ( Initialize( pWork ) >= 0 ){
			pWork->act = (void *)NormalAct;
		}
	}
	return ( 0 );
}

static int Act( Work *pWork )
{
	if ( GM_CheckSightStatus( SGT_Scope ) ){
		invisible( pWork );
		return ( -1 );
	}
	// デモ等のサイトの表示非表示管理
	if ( SightVisibleInvisible( pWork->handle_2d , STR_ROOT ) > 0){
		pWork->flag &= ~SK_INVISIBLE;
	} else {
		pWork->flag |= SK_INVISIBLE;
	}
 	pWork->act( pWork );

	return ( 0 );
}

static	void	Die( Work *work )
{
	if ( work->handle_2d >= 0 ){
		L2D_ReleaseLayout( work->handle_2d );
	}
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	int			handle;

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout( LAYOUT , DG_CHANL_MAIN , 0, 0 ) ;
	if ( handle < 0 ){
		return ( -1 );
	}
	work->handle_2d = handle ;

	// etc
	work->dup_count.graph_l = 0;
	work->dup_count.graph_r = 0;
	work->center_box_scale = 1.0f; // 拡大率
	work->action_num = L2D_GetActionNumber( work->handle_2d );
	work->action = 0;
	work->act = (void *)AnimetionAct;
	work->flag = SK_INITIALIZE_OK;

	/* MENU_Printfキャラ生成 */
	if ( GM_CheckGameStatus( STATE_PLAY_DEMO ) ) { // demo mode
		work->menuprint_work_ptr = NewMenuPrintManager( ( 8 * 1024 ) , DG_DMAPACK_NORMAL | DG_DMAPACK_INVISIBLE1 | DG_DMAPACK_INVISIBLE2 |
														DG_DMAPACK_INVISIBLE3 | DG_DMAPACK_PRIVILEGE , DG_DMAPACK_PHASE_AFTER , 144 );
	} else { // normal mode
		work->menuprint_work_ptr = NewMenuPrintManager( ( 8 * 1024 ) , DG_DMAPACK_MENU , DG_DMAPACK_PHASE_NORMAL , 0 );
	}
	if ( work->menuprint_work_ptr == NULL ){
		SK_Err("menu print\0");
		return ( -1 );
	}
	GV_SetActorChild( work , work->menuprint_work_ptr );

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewSK_ScopeSight( float raute )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->raute = raute;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}

	return work ;
}
