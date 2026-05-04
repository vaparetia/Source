//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ts_subwin.c
		子画面

	2000/06/08 T.Shibata
	$Id: ts_subwin.c,v 1.1.1.3 2002/11/19 11:48:54 Yoshizawa1 Exp $
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
#include	"utl_dma.h"
#include	"camera.h"

#include	"../util/ts_util.h"

/*sigeno*/
//#include	"../../sigeno/display/radsize.h"
#include	"../../kira/radar/radsize.h"
#define	CAMERA_NUM	(1)

#define	CAMERA_ANGLE (2.0f)

#define	MAX_CAMERA_ANGLE (6.0f)
#define	ADD_CAMERA_ANGLE ((MAX_CAMERA_ANGLE-CAMERA_ANGLE)/20)

#define	WAIT_TIME	(40)

typedef struct {
	GV_ACT_EX 			actor;
	GM_CameraSet	*camera;
	int				mode;
	u_int			life;
	u_int			timer;
	float			angle;
	float			max_angle;
	float			add_angle;
	u_int			wait_time;
	FVECTOR			*p_to;
	FVECTOR			*p_from;
	
	FVECTOR			from;
	FVECTOR			to;
	
	int 			x0, y0, x1, y1;
	int				name;
	int				map;
	int				flag;
	CVECTOR			rgb;
	int				pre_flag;
} Work;

typedef struct {
	sceGsRgbaq rgba_box;
	sceGsPrim prim_box;
	//ボックス
	DG_PRIM_XY xy1;
	DG_PRIM_XY xy2;
	DG_PRIM_XY xy3;
	DG_PRIM_XY xy4;

	//枠

	sceGsRgbaq rgba_frm;
	sceGsPrim prim_frm;
	DG_PRIM_XY xy5;
	DG_PRIM_XY xy6;
	DG_PRIM_XY xy7;
	DG_PRIM_XY xy8;
	DG_PRIM_XY xy9;


} MENU_LINE_BOX;

//extern void DG_SetDrawEnv( DG_CHANL *cp, long x, long y, long w, long h );

static Work *g_work = NULL;

static void CameraPosTargetSet( FVECTOR *pos, FVECTOR *target, FVECTOR *from, FVECTOR *to )
{
	FVECTOR		dvec,shift = { 0.0f, 300.0f, -600.0f, 0.0f };
	FMATRIX		mat;
	
	fpu_SubVectors( &dvec, to, from );
	TS_MakeMatrix( &mat, &dvec , from );
	DG_SetPos(&mat);
	DG_RotVector( &shift, &shift, 1 );
	fpu_AddVectors( pos, from, &shift );
	DG_COPY_VEC( target, to);
	
	{
		int hzx_result;
		int map_id = GM_CurrentMap;
		FVECTOR		point;
		hzx_result = HZX_OnlineHazardCheck( GM_GetHzxGroupID( map_id ), target, pos,
											HZX_CHK_ALL, HZX_SEG_ALL, HZX_FLOOR_ALL );
		if(hzx_result & 3){
			HZX_GetOnlinePoint( &point );
			DG_COPY_VEC(pos,&point) ;
		}
	}
	//AN_Test_Eye2( pos, 1);
}

static void DrawFrame( Work *work )
{
#if 0 ///madakore
	/* 子画面のフレーム表示 */
	MENU_PRIM *prim;
	sceGifTag *giftag;
	MENU_LINE_BOX *box;
	
	MENU_SetAlphaMode( 0,1,0,1,0x80 ) ;
	
	prim = MENU_OpenPrim();

	MENU_NEW_TAG( giftag, prim );
	DG_SET_GIFTAG( giftag, .FLG = SCE_GIF_REGLIST, .NREG = 14, .NLOOP = 1,
				   .REGS0 = GS_REGS_RGBA, GS_REGS_PRIM,
				   GS_REGS_XYZ3, GS_REGS_XYZ3, GS_REGS_XYZ2,
				   GS_REGS_XYZ2,
				   GS_REGS_RGBA, GS_REGS_PRIM,
				   GS_REGS_XYZ3, GS_REGS_XYZ2, GS_REGS_XYZ2,
				   GS_REGS_XYZ2, GS_REGS_XYZ2 );
	MENU_NEW_PRIM( box, prim );

	DG_SET_GS_REG( &box->prim_box, .PRIM = SCE_GS_PRIM_TRISTRIP, .ABE = 1 );
	DG_SET_GS_REG( &box->prim_frm, .PRIM = SCE_GS_PRIM_LINESTRIP, .AA1 = 1 );
	if(work->flag&2){
		DG_SET_GS_REG( &box->rgba_box, 130, 189, 255, 18, 0 );
	}else{
		DG_SET_GS_REG( &box->rgba_box, 255, 64, 64, 18, 0 );
	}
	DG_SET_GS_REG( &box->rgba_frm, 00, 00, 00, 80, 0 );

	box->xy1.x = DG_POS_X( work->x0 );
	box->xy1.y = DG_POS_Y( work->y0 );
	box->xy2.x = DG_POS_X( work->x1 );
	box->xy2.y = DG_POS_Y( work->y0 );
	box->xy3.x = DG_POS_X( work->x0 );
	box->xy3.y = DG_POS_Y( work->y1 );
	box->xy4.x = DG_POS_X( work->x1 );
	box->xy4.y = DG_POS_Y( work->y1 );

	box->xy5.x = DG_POS_X( work->x0-1 );
	box->xy5.y = DG_POS_Y( work->y0-1 );
	box->xy6.x = DG_POS_X( work->x1 );
	box->xy6.y = DG_POS_Y( work->y0-1 );
	box->xy7.x = DG_POS_X( work->x1 );
	box->xy7.y = DG_POS_Y( work->y1 );
	box->xy8.x = DG_POS_X( work->x0-1 );
	box->xy8.y = DG_POS_Y( work->y1 );
	box->xy9.x = DG_POS_X( work->x0-1 );
	box->xy9.y = DG_POS_Y( work->y0-1 );

	MENU_ClosePrim();
#endif
}

static void SubWinProgAct( Work *work )
{
	DG_CHANL 	*cp;
	int flag = 0;
	cp = DG_Chanl( CAMERA_NUM );

	if( !(GV_PauseLevel & GV_PAUSE_PAUSE) ){
		if( work->timer > WAIT_TIME ){
			if( work->angle <= work->max_angle ){
				work->angle += work->add_angle;
				GM_SetCameraAngle( work->camera, work->angle ) ;
				flag = 1;
			}
		}
		if( work->mode ){
			CameraPosTargetSet( &work->from, &work->to, work->p_from, work->p_to );
			DG_COPY_VEC(&work->camera->target, &work->to);
			DG_COPY_VEC(&work->camera->position, &work->from);
			flag = 1;
		}
	}
	if( flag ){
		GM_ChangeCamera( CAMERA_NUM );
      BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
		DG_SetCamera2( cp, &work->from, &work->to, work->angle );
	}
	
	if( !(GV_PauseLevel & GV_PAUSE_PAUSE) && ++work->timer > work->life ){
		//カメラ起動しない
		//GV_DestroyActor( work );
		work->flag &= ~(1|8);
		//cp = DG_Chanl( CAMERA_NUM );
		//cp->flag = 0;
		GM_NoUseCamera( CAMERA_NUM );
		GM_ResetMenuStatus( MENU_SUBWIN_ON );
		
		work->flag &= ~0x80;
		GM_ChanlControl( 1, 0 );
		//printf("aheaheuhiha\n");
	}
	//printf("prog ene view\n");
}

static void Act( Work *work )
{
	DG_CHANL 	*cp;
	int pflag = 0,n,flag;
	GV_MSG *msg;

	if(work->pre_flag){
		if(!GM_CheckMenuStatus(MENU_SUBWIN_OFF)){
			work->pre_flag = 0;
			if( g_work->flag & 0x18 && !(work->flag&0x80) ){
				GM_ChanlControl( 1, 1 );
				GM_SetMenuStatus( MENU_SUBWIN_ON );
			}
		}else{
			return;
		}
	}else{
		if(GM_CheckMenuStatus(MENU_SUBWIN_OFF)){
			GM_ChanlControl( 1, 0 );
			GM_ResetMenuStatus( MENU_SUBWIN_ON );
			work->pre_flag = 1;
			//work->flag = 0;
			
			return;
		}
	}

	if( (GV_PauseLevel & GV_PAUSE_PAUSE) ){
		GM_ChanlControl( 1, 0 );
		GM_ResetMenuStatus( MENU_SUBWIN_ON );
		return;
	}else{
		if( g_work->flag & 0x18 && !(work->flag&0x80) ){
			GM_ChanlControl( 1, 1 );
			GM_SetMenuStatus( MENU_SUBWIN_ON );
		}
	}

	cp = DG_Chanl( CAMERA_NUM );
	
	pflag = flag = cp->flag ;
	
	if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
	    for( ; n > 0; n-- ){
			flag = msg->message[ 0 ];
			printf("Sub Wind[%d]\n",flag);
			if( flag ) work->flag |= 0x10;
			else work->flag &= ~0x10;
			msg++;
	    }
		//GM_ChanlControl( 1, flag );
	}
	

	if( ( GM_Item == IT_NightVision ) ||
		( GM_Item == IT_Thermal ) ){
		flag = 0;
		work->flag &= ~0x10;
	}
	if ( pflag != flag ) {
	    if ( flag ){
			GM_SetMenuStatus( MENU_SUBWIN_ON );
			GM_UseCamera( 1 );
			GM_ChangeCamera( CAMERA_NUM );
			work->flag |= 2;
			GM_ChanlControl( 1, 1 );
			//cp->flag = 1;
			//printf("aaa\n");
			/* ここにプログラム子画面消去処理 */
			work->flag &= ~1;
		} else {
			GM_ResetMenuStatus( MENU_SUBWIN_ON );
			GM_NoUseCamera( 1 );
			//cp->flag = 0;
			//printf("bbb\n");
			GM_ChanlControl( 1, 0 );
			work->flag &= ~0x82;
		}
	}


	//子画面ベース表示
	if( cp->flag ) DrawFrame( work );

	//プログラムカメラアクト
	if( work->flag&1 ) SubWinProgAct( work );

	//if( GV_PadData[0].status & PAD_X ) GM_ChanlControl( 1, 0 );
	//else if( GV_PadData[0].status & PAD_Y ) GM_ChanlControl( 1, 1 );

}

static void Die( Work *work )
{
	DG_CHANL *cp;

	cp = DG_Chanl( CAMERA_NUM );
	GM_ChanlControl( 1, 0 );
	GM_NoUseCamera( CAMERA_NUM );
    if(work->camera)GM_DeleteCamera( work->camera );

	GM_ResetMenuStatus( MENU_SUBWIN_ON );
	
	g_work = NULL;
}

static	void	InitCamera( Work *work, FVECTOR *from, FVECTOR *to )
{
    GM_CameraSet	*cam ;

    work->camera = cam = NewProgramCamera( 0, CAMERA_NUM, 
										   GM_CAMERA_PROG4, 253 ) ;

    GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_TARGET,
					 CAM_FLAG_FIX ) ;
    //GM_SetCameraRotate( cam, &GM_CameraDir ) ;
    //GM_SetCameraTrack( cam, 1000 ) ;
    GM_SetCameraAngle( cam, CAMERA_ANGLE ) ;

	DG_COPY_VEC(&cam->position, from);
	DG_COPY_VEC(&cam->target, to);
	//cam->position = ;
	//cam->target = ;
	
    GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP4,
							GM_CAM_INTERP_EXP4, 0, 0 ) ;
    //cam->on = 1 ;
    //GM_ChangeCamera( CAMERA_NUM ) ;
    //GM_SetCameraQuick( CAMERA_NUM ) ;
}


static void *SetCameraStatus( FVECTOR *from, FVECTOR *to,
							  float init_angle, float max_angle, float add_angle,
							  u_int life, u_int wait_time, int mode, int map )
{
	DG_CHANL *cp;
	cp = DG_Chanl( CAMERA_NUM );

	if(GM_AlertMode != ALERT_MODE_SNEAK) return NULL;
	if(!g_work) { printf("子画面を起動しておいて下さい\n"); return NULL;};
	if(g_work->flag&2) return NULL;
	if(!g_work->camera) InitCamera( g_work, &DG_ZeroVector, &DG_ZeroVector );

	g_work->life = life;
	g_work->wait_time = wait_time;
	g_work->mode = mode;
	g_work->angle = init_angle;
	g_work->max_angle = max_angle;
	g_work->add_angle = add_angle;
	DG_COPY_VEC(&g_work->from, from);
	DG_COPY_VEC(&g_work->to, to);		
	g_work->p_from = from;
	g_work->p_to = to;

	g_work->timer = 0;

	if(mode) CameraPosTargetSet( &g_work->from, &g_work->to, g_work->p_from, g_work->p_to );
	GM_SetCameraAngle( g_work->camera, init_angle ) ;

	DG_COPY_VEC(&g_work->camera->position, from);
	DG_COPY_VEC(&g_work->camera->target, to);
	
	GM_SetCameraInterpMode( g_work->camera, GM_CAM_INTERP_EXP4,
							GM_CAM_INTERP_EXP4, 0, 0 ) ;
	g_work->camera->on = 1 ;
	GM_ChangeCamera( CAMERA_NUM ) ;
	GM_SetCameraQuick( CAMERA_NUM ) ;

   BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
 	DG_SetCamera2( &DG_Chanls[CAMERA_NUM], &g_work->from, &g_work->to, g_work->angle );
	
	g_work->flag |= 1;
	g_work->flag |= 8;
	GM_SetChanlTargetMap( 1, map );
	GM_ChanlControl( 1, 1 );

	//cp->flag = 1;
	GM_UseCamera( 1 );
	GM_SetMenuStatus( MENU_SUBWIN_ON );

//	printf("ENE_VIEW_OPEN\n");
	return g_work;
	//GM_GameStatus |= STATE_RADAR_INVISIBLE;
}

static int GetResources_S( Work *work )
{
	int x0, y0, x1, y1;
	int xofs, yofs, width, height;
	DG_CHANL *cp;

	cp = DG_Chanl( 1 );
	
	work->flag = 0;

#if 0
	if( GCL_GetOption( 'w' ) != NULL ){
		work->x0 = x0 = GCL_GetNextInt();
		work->y0 = y0 = GCL_GetNextInt();
		work->x1 = x1 = GCL_GetNextInt();
		work->y1 = y1 = GCL_GetNextInt();
	} else {
		printf( "subwin:unsetWindow\n" );
		return -1;
	}
#else

	work->x0 = x0 = RADAR_WINDOW_X0;
	work->y0 = y0 = RADAR_WINDOW_Y0+1;
	work->x1 = x1 = RADAR_WINDOW_X1;
	work->y1 = y1 = RADAR_WINDOW_Y1-1; /*クリアリング表示との合わせのため枠線の分縮める */
#endif

	width = x1 - x0;
	height = y1 - y0;
	xofs = ( x0 - DRAW_WIDTH / 2 ) + width / 2;
	yofs = ( y0 - DRAW_HEIGHT / 2 ) + height / 2;

	DG_SetDrawEnv( cp, xofs, yofs, width, height );
   BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
	DG_SetCamera2( cp, &DG_ZeroVector, &DG_ZeroVector, CAMERA_ANGLE );

	//printf("flag!!!!!!!!!!!!!!! %d\n",cp->flag);
	return 0;
}

void *NewTs_SubWindow_S( int name, int map )
{
	Work *work;

	if(g_work){ return NULL; }

	work = ( Work * )GV_NewActor( GV_ACTOR_PREV2, sizeof( Work ) );
	if( work == NULL ){ return NULL; }

	GV_SetActor( &( work->actor ), Act, Die );
	GV_ActorEX( &work->actor );

	work->name = name;
	work->map = map;
	work->pre_flag = (GM_CheckMenuStatus(MENU_SUBWIN_OFF))?1:0;

	if( GetResources_S( work ) < 0 ){
		GV_DestroyActor( work );
		return NULL;
	}
	g_work = work;
	
//	printf("Ts SubCamera on\n");
	return work;
}

void TS_SubWindowSleep()
{
	if( !g_work ) return;

	//off
	GM_ChanlControl( 1, 0 );
	GM_ResetMenuStatus( MENU_SUBWIN_ON );
	g_work->flag |= 0x80;
}

void TS_SubWindowWake()
{
	if( !g_work ) return;
	//printf("aaaaaaaa\n");
	if( g_work->flag & 0x18 ){
		GM_ChanlControl( 1, 1 );//,	printf("bbbbbbbbbbbbbb\n");
		GM_SetMenuStatus( MENU_SUBWIN_ON );
		g_work->flag &= ~0x80;
	}
}

//修正
void KillEnemyView()
{
	DG_CHANL 	*cp;

	if(!g_work) { printf("子画面を起動しておいて下さい\n"); return;};
	if(!g_work->camera) return;

	cp = DG_Chanl( CAMERA_NUM );
	
	g_work->flag = 0;
	GM_ChanlControl( 1, 0 );
	GM_NoUseCamera( CAMERA_NUM );
	GM_ResetMenuStatus( MENU_SUBWIN_ON );
}

void *NewSubCameraControl( FVECTOR *pos, FVECTOR *target,
						   float init_angle, float max_angle, float add_angle,
						   u_int life, u_int wait_time, int map  )
{
	if( GM_CheckGameStatus( STATE_PLAY_DEMO ) || GM_CheckMenuStatus(MENU_SUBWIN_OFF) ) return NULL;
	return (SetCameraStatus( pos, target, init_angle, max_angle, add_angle, life, wait_time, 0, map ));
}
