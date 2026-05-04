//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dg_cam.c
   デジタルカメラマネージャ
   
   2001/05/11 S.Kobayashi
   $Id: dg_cam_mng.c,v 1.1.1.3 2002/11/19 11:50:15 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"camera.h"

#include	"memlist.h"
#include	"jpeg.h"
#include	"capture.h"

/*------------------------------------------------------------------*/

#define	MAX_CHK_SQS	(16)
#define	MAX_CHK_CHS	(8)

#define	ANGLE_MAX	(24.00F)
#define	ANGLE_MIN	(2.00F)
#define	ANGLE_STEP	(0.15F)

extern int PL_PAD_ZOOMIN        ;
extern int PL_PAD_PRESS_ZOOMIN  ;
extern int PL_PAD_ZOOMOUT       ;
extern int PL_PAD_PRESS_ZOOMOUT ;
extern int PL_PAD_WEAPON        ;

#define	PAD_ANGLE_INC	(PL_PAD_ZOOMOUT)
#define	PAD_ANGLE_DEC	(PL_PAD_ZOOMIN)
#define	PAD_SHUTTER	 (0)
#define	PAD_SHUTTER2 (PL_PAD_WEAPON)

#define	PROC_TIME	(90)

#define CAPTURE_PAUSE_TIME	50

/* ズームカメラ (sonoyama/etc/zoomcam.c) */
extern	void		*NewCameraZoomControl( GM_CameraSet **res, GM_CameraSet *parent, GV_PAD *pad, 
										  float angle_min, float angle_max,
										  float angle_step, int mode, int name ) ;
extern  void	*NewFarFocusControl( GM_CameraSet *camera );

/*------------------------------------------------------------------*/
// こば４追加
enum {
	SK_UN_INITIALIZE    = 0x1,
	SK_INITIALIZE_START = 0x2,
	SK_INITIALIZE_OK    = 0x4,
	SK_INVISIBLE        = 0x8,
	SK_VISIBLE          = 0x10,
};

enum {
	SK_ANDER_STAY       = 0x1,
	SK_TANKER_FRAME     = 0x2,
	SK_SHUTER_ON        = 0x8,
	SK_SHUTER_OPEN      = 0x10,
	SK_SHUTER_FULL_OPEN = 0x20,
	SK_SHUTER_READY = 0x40,
	SK_ANDER_MOVING = 0x80,
};

enum {
    STATE_NORMAL = 0,
    STATE_SHUTTER_ON,
	STATE_SAVE ,
	STATE_READY ,
} ;

enum {
	SK_SHUTER = 0x1 ,
	SK_MOVE_STAY = 0x2 ,
} ;

/*------------------------------------------------------------------*/
typedef	struct	{
	FVECTOR		p[ 4 ] ;
	FVECTOR		normal ;
	FVECTOR		center ;
	TARGET		target ;
	float		far_x ;
	int			proc ;
	int			xrange ;
	int			yrange ;
	int			flag ;
} CHK_SQ ;

static	CHK_SQ	*CheckSquare = NULL ;

typedef	struct	{	
	TARGET		target ;
	OBJECT		*body ;
	u_short		flag ;
	short		joint ;	
	float		far_x ;
	int			proc ;
} CHK_CH ;

static	CHK_CH	*CheckChara = NULL ;

/*------------------------------------------------------------------*/

typedef	struct _dg_cam_mng_work {
    GV_ACT_EX	actor ;
    OBJECT		equip ;

    CONTROL		*ctrl ;
    OBJECT		**body ;
    int			*unit ;
    u_int		*trigger ;

    GM_CameraSet	*subject ;
    GM_CameraSet	*camera ;
    GV_PAD		*pad ;

    u_short		state ;
    u_short		time ;
	int			chanl ;
	int         child_status;
	int         frame_number; // タンカー格納場所
	void        ( * act )( struct _work * );

	int			this ;
	int         push_flag;
	int         frame_count;
	void		*zctrl ;
	void		*focus ;
} Work ;

extern void	*NewDgCamSight( float , float  );
extern void	*NewTDgCamSight( float , float , int * );
extern void	*NewDgCamShuterSight( void );
extern void SK_DGcamCheck( Work *, int );
extern void *NewPictureDrawManager( int prio );
extern void GM_JimakuHide( void );
extern void TS_SubWindowWake();
extern void TS_SubWindowSleep();
extern void CODEC_Block(int sw);

static inline void StatusReset( Work *pWork )
{
	int tmp;

	tmp = GM_CheckMenuStatus( MENU_STREAM_CH_0 );
	tmp |= GM_CheckMenuStatus( MENU_STREAM_CH_1 );
	DG_SetPrivilegeMode(0);
	if ( pWork->push_flag & SK_SHUTER ){
		GM_PopGameStatus();
		GM_PopMenuStatus();
		// 特殊
		if ( tmp & MENU_STREAM_CH_0 ){
			GM_SetMenuStatus( MENU_STREAM_CH_0 );
		} else {
			GM_ResetMenuStatus( MENU_STREAM_CH_0 );
		}
		if ( tmp & MENU_STREAM_CH_1 ){
			GM_SetMenuStatus( MENU_STREAM_CH_1 );
		} else {
			GM_ResetMenuStatus( MENU_STREAM_CH_1 );
		}
		pWork->push_flag &= ~SK_SHUTER;
	}
	CODEC_Block( 0 ); // 強制無線禁止解除
	TS_SubWindowWake();
}

/*------------------------------------------------------------------*/
// 受信
static int SignalFunc( void *work, int signal, int value )
{
	Work *pWork;

	pWork = ( Work * )work;
	switch ( signal ){
	case SK_SHUTER_OPEN :
		GV_CallChildSignalFunc( pWork , 0x02 , SK_INVISIBLE ); // 非表示
		GM_SeSet3D( 0x00, GM_MAX_VOL, SD_I_CAMERA02, -M_PI/2 ) ;	
		break;
	case SK_SHUTER_FULL_OPEN :
		GV_PauseOnActorSystem( GV_PAUSE_MENU ) ;
		pWork->state = STATE_SAVE ;
		pWork->time = 0 ;
		pWork->push_flag = SK_SHUTER;
		TS_SubWindowSleep();
		GM_PushGameStatus(); // 退避
		GM_PushMenuStatus();
		// 字幕消し
		GM_JimakuHide();
		CODEC_Block( 1 ); // 強制無線禁止
		GM_SetMenuStatus( MENU_GAGE_OFF );
		GM_SetMenuStatus( MENU_RADIO_DISABLE );
		GM_SetMenuStatus( MENU_RADAR_OFF );
		GM_SetMenuStatus( MENU_SUBWIN_OFF );
		GM_SetMenuStatus( MENU_MENU_OFF );

		DG_SetPrivilegeMode(1);
		pWork->frame_count = DIRECT_TICK(6);
//		printf( "JPEG save memory \n"); // <- 保存の関数を入れる予定
		break;
	case SK_SHUTER_READY : // 強制解除
		GM_ResetPlayerStatus( PLAYER_MENU_DISABLE | PLAYER_PAD_OFF ) ;
		GV_PauseOffActorSystem( GV_PAUSE_MENU ) ;
//		GM_ResetMenuStatus( MENU_MENU_OFF | MENU_GAGE_OFF ) ;
		pWork->state = STATE_NORMAL ;
		break;
	case SK_ANDER_MOVING : // タンカーかめら
		pWork->child_status = SK_ANDER_MOVING ;
		StatusReset( pWork );
		break;
	case SK_ANDER_STAY :
		pWork->child_status = 0 ;
		break;
	case SK_TANKER_FRAME :
		pWork->frame_number = value ;
		break;
	case GV_SIGNAL_KILL :
		((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( work );
		return ( -1 );
	}
	return ( 0 );
}

/* ズームカメラ起動 */
static	void	StartZoomCamera( Work *work )
{
	int			name ;

	if ( work->zctrl == NULL ) {
		name = ( work->this == IT_Camera ) ? PL_CAMERA_ZOOMCAMERA_NAME : PL_TNKCAM_ZOOMCAMERA_NAME ;
		work->zctrl = NewCameraZoomControl( &work->camera, GM_PlayerSubjectCamera[ 0 ], GV_PadData,
										    ANGLE_MIN, ANGLE_MAX, ANGLE_STEP, 0,
										    name ) ;
		GV_SetActorChild( work, work->zctrl ) ;
	}
	if ( work->focus == NULL ) {
		work->focus = NewFarFocusControl( NULL ) ;
		GV_SetActorChild( work, work->focus );		
	}
}

/* カメラ操作 */
static	void	CameraControl( work )
Work		*work ;
{
#if 0
    GM_CameraSet	*cam ;
    int				status ;
	int             itmp;
    float			angle, adj, step ;
#endif
    GV_PAD			*pad ;

    if ( GV_PauseLevel != 0 ) return ;

    pad = work->pad ;

    if ( ++ work->time > DIRECT_TICK( 30 ) && 
		( pad->press & PAD_SHUTTER || pad->press & PAD_SHUTTER2 ) && 
		( work->child_status != SK_ANDER_MOVING ) && ( work->frame_count <= 0 ) && 
		( GM_IsGameOver() != 1 ) ){
		GM_SetPlayerStatus( PLAYER_MENU_DISABLE | PLAYER_PAD_OFF ) ;
//		GM_SetMenuStatus( MENU_MENU_OFF | MENU_GAGE_OFF ) ;
		GV_PauseOnActorSystem( GV_PAUSE_MENU ) ;
		// koba4 add
		GV_CallChildSignalFunc( work , SK_SHUTER_ON , 0 );
		GV_SetActorChild( work , NewDgCamShuterSight() );
		work->state = STATE_SHUTTER_ON ;
		return ;
    }
	if ( work->this == IT_TnkCamera &&
		 work->time > DIRECT_TICK( 20 ) ) {
		SK_DGcamCheck( work, 0 ) ;
	}

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		/* VR等ではIT_Cameraは写真撮影モードでしか使わないと断定 */
		if ( work->this == IT_Camera &&
			 work->time > DIRECT_TICK( 20 ) ) {
			SK_DGcamCheck( work, 0 ) ;
		}
	}

#if 0
    cam = work->camera ;
    pad = work->pad ;
    status = pad->status ;

    angle = cam->angle ;
    adj = ANGLE_MAX - angle ; 
	itmp = DIRECT_TICK( 6 );
    if ( adj == 0.0F ){
		adj = 1.0F ;
	}
    adj = ANGLE_STEP * ( ANGLE_MAX - ANGLE_MIN ) / adj ;
    if ( status & PAD_ANGLE_INC ) {
		angle += adj ;
		if ( angle < ANGLE_MAX && ( GV_Time & itmp ) == 0 ) {
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_ZOOM01 ) ;
		}
    } else if ( status & PAD_ANGLE_DEC ) {
		angle -= adj ;
		if ( angle > ANGLE_MIN && ( GV_Time & itmp ) == 0 ) {
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_ZOOM01 ) ;
		}
    }
    if ( angle > ANGLE_MAX ) angle = ANGLE_MAX ;
    if ( angle < ANGLE_MIN ) angle = ANGLE_MIN ;

    cam->angle = angle ;
    _sceVu0CopyVector( &cam->position, &GM_CameraTarget ) ;

    cam->rotate.vx = GM_CameraDir.vx ;
    cam->rotate.vy = GM_CameraDir.vy ;
    cam->rotate.vz = 0 ;

    step = 16.0F - 16.0F * ( ( angle - ANGLE_MIN ) / ( ANGLE_MAX - ANGLE_MIN ) ) ;
    if ( angle > 20.0F ){
		step /= 3.0F ;
	} else if ( angle > 10.0F ){
		step /= 2.0F ;
	}
    if ( status & PAD_L1 ){
		step /= 2.0F ;
	}
    if ( step < 1.0F ) {
		if ( GV_Time & 1 ){
			GM_SubjectVStep = 1 ;
		} else {
		   GM_SubjectVStep = 0 ;
		}
    } else {
		GM_SubjectVStep = ( int )step ;
    }
    GM_SubjectHStep = GM_SubjectVStep ;
#endif
#ifdef DEBUG_MODE
	if ( PlayerDebugMenuStatus & PDMS_HAZARDVIEW )
	{
		int			i, j ;
		extern void	PL_GetCheckWork( CHK_SQ **, CHK_CH ** ) ;

		PL_GetCheckWork( &CheckSquare, &CheckChara ) ;
		for ( i = 0; i < PL_DGCam_N_CheckSquares; i ++ ) {
			NewSquareView( &( CheckSquare + i )->p[ 0 ], 1, 23, 232, 32 ) ;
			NewPointView( &( CheckSquare + i )->center, 32, 32, 232 ) ;
		}
		for ( i = 0; i < PL_DGCam_N_CheckCharas; i ++ ) {
			DG_MDL		*mdl ;
			DG_OBJ		*obj ;
			FMATRIX		*m ;
			FVECTOR		pos ;

			obj = &( ( CheckChara + i )->body->objs->objs[ ( CheckChara + i )->joint ] ) ;
			mdl = &( ( CheckChara + i )->body->objs->def->models[ ( CheckChara + i )->joint ] ) ;
			m = &obj->world ;
			GV_MatToVec( m, &pos ) ;
			NewPointView( &pos, 32, 32, 232 ) ;
			DG_SetPos( m ) ;
			for ( j = 0; j < 8; j ++ ) {
				pos.vx = ( j & 1 ) ? mdl->lx : mdl->ux ;
				pos.vy = ( j & 2 ) ? mdl->ly : mdl->uy ;
				pos.vz = ( j & 4 ) ? mdl->lz : mdl->uz ;
				DG_PutVector( &pos, &pos, 1 ) ;
				NewPointView( &pos, 32, 232, 32 ) ;
			}
		}
	}
#endif
}

void *SK_PhotoFramePointer( int frame_this )
{
	void *resident;

	resident = GV_GetCache( 0x1F6AF7A3 );	// tank_jpeg_cam | ( 0x7F << 24 )
#if 0
	printf( "camera_reg_work = %X\n", resident );
#endif
	ASSERT( resident != NULL );

#ifdef BP_PS2
	return ( resident + ( THUMB_W * THUMB_H * sizeof( short ) ) * frame_this );
#else
	return ( ( char* )resident + ( THUMB_W * THUMB_H * sizeof( short ) ) * frame_this );
#endif
}

/*------------------------------------------------------------------*/
static void TankerAct( Work *pWork ) // 通常カメラ(保存のみ)
{
	// ブラー等の影響を極力なくすため,PAUSE状態でCAPTURE_PAUSE_TIME待つ。
	// あたり情報のチェック等はここで行なえばいい
	if ( pWork->time == 0 ){
		SK_DGcamCheck( pWork, 1 );
	}
	pWork->time++;

	if( pWork->time == CAPTURE_PAUSE_TIME ){
		// ここにくると、画面はフリーズして GV_PAUSE_STOPがかかる。
		// タンカー編
		{
			printf("frame_number %d" ,pWork->frame_number);
			NewCaptureStart( CAPTURE_NO_SAVE, NULL, NULL, TANKER_MAX_CODE, pWork->frame_number );
		}
	} else if( pWork->time == CAPTURE_PAUSE_TIME + 1 ){
		// 出てくる時はメモリーカード等へのセーブが完了してGV_PAUSE_STOPが完了した時。
		GM_ResetPlayerStatus( PLAYER_MENU_DISABLE | PLAYER_PAD_OFF ) ;
		pWork->state = STATE_NORMAL ;
		GV_PauseOffActorSystem( GV_PAUSE_MENU ) ;
//		GM_ResetMenuStatus( MENU_MENU_OFF | MENU_GAGE_OFF ) ;
		GV_CallChildSignalFunc( pWork , 0x04 , 0 ); // 非表示
	}
}

static void NormalAct( Work *pWork )
{
	// save memory
	pWork->time++;

	if( pWork->time == CAPTURE_PAUSE_TIME ){
		// 普通のデジカメ
		NewCaptureStart( CAPTURE_SAVE, NULL, NULL, 0, CAPTURE_DIGITAL_PHOTO_NUM );
	} else if( pWork->time == CAPTURE_PAUSE_TIME + 1 ){
		// 出てくる時はメモリーカード等へのセーブが完了してGV_PAUSE_STOPが完了した時。
		GM_ResetPlayerStatus( PLAYER_MENU_DISABLE | PLAYER_PAD_OFF ) ;
		pWork->state = STATE_NORMAL ;
		GV_PauseOffActorSystem( GV_PAUSE_MENU ) ;
//		GM_ResetMenuStatus( MENU_MENU_OFF | MENU_GAGE_OFF ) ;
		StatusReset( pWork );
	}
}

static void VrAct( Work *pWork )
{
	if ( pWork->time == 0 ){
		SK_DGcamCheck( pWork, 1 );
	}
	pWork->time++;

	if( pWork->time == 120 ){
		// 出てくる時はＶＲクリアが管理を初めている時。
//		GV_PauseOffActorSystem( GV_PAUSE_STOP );
//		GV_PauseOffActorSystem( GV_PAUSE_MENU ) ;
	}
}

static	void	Act( work )
Work		*work ;
{
    u_int		trg ;

	StartZoomCamera( work ) ;

	if ( GM_GetNextCamera( work->camera->chanl ) != work->camera ) {
		GM_ChangeCamera( work->camera->chanl ) ;
	}

	if ( GM_PlayerBody != NULL && 
		!( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ) ) {
		/* まだ主観になりきっていない */
		GM_SetSightStatus( SGT_Camera | SGT_TankerCamera ) ;
		DG_COPY_VEC( &work->camera->position, &GM_CameraTarget ) ;
		GM_SetCameraRotate( work->camera, &GM_CameraDir ) ;
		return ;
	} else {
		/* プレイヤーがいない ＯＲ 主観状態 */
		GM_ResetSightStatus( SGT_Camera | SGT_TankerCamera ) ;		
	}

    trg = *( work->trigger ) ;
    switch ( work->state ) {
    case STATE_NORMAL :
		GV_CallChildSignalFunc( work , 0x02 , SK_VISIBLE );	 // 表示
		if ( work->frame_count > 0 ){
			work->frame_count--;
		}
		CameraControl( work ) ;
		break ;
    case STATE_SHUTTER_ON :
		break;
    case STATE_SAVE :
		work->act( work );
		break;
    case STATE_READY : // メッセージ受取まち
		GV_CallChildSignalFunc( work , 0x02 , SK_VISIBLE );	 // 表示
		break;
    }
}

static	void	Die( work )
Work		*work ;
{
    if ( work->camera != NULL ) GM_ChangeCamera( work->camera->chanl ) ;
    GM_DeleteCamera( work->camera ) ;
    GM_SubjectVStep = 16 ;
    GM_SubjectHStep = 16 ;
	GM_FreeObject( &work->equip ) ;
}

/*------------------------------------------------------------------*/

/* カメラ起動 */
static	void	InitCamera( work, ctrl, this )
Work		*work ;
CONTROL		*ctrl ;
int			this ;
{
	StartZoomCamera( work ) ;
#if 0
    GM_CameraSet	*cam ;
    int			name ;

    name = GV_StrCode( "デジタルカメラ" ) ;
    work->camera = cam = NewProgramCamera( name, GM_CurrentCameraChanl, 
										  GM_CAMERA_SUBJECT, SCOPE_CAMERA_PRIO ) ; 
    GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_ROTATE,
					 CAM_FLAG_PAD_ADJUST ) ;
    GM_SetCameraRotate( cam, &GM_CameraDir ) ;
    GM_SetCameraTrack( cam, 1000 ) ;
    GM_SetCameraAngle( cam, 2.0F ) ;
    cam->position = GM_CameraTarget ;
    GM_SetCameraInterpMode( cam, GM_CAM_INTERP_INTO_SUBJECT,
						   GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;
    cam->on = 1 ;
    GM_ChangeCamera( work->camera->chanl ) ;
#endif
}

static	int	GetResources( work, ctrl, body, unit , this )
Work		*work ;
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
int       this;
{
	void *sights;
	void *pict;

	work->this = this ;
    /* カメラ起動 */
    InitCamera( work, ctrl, this ) ;
	GM_SetSightStatus( SGT_Camera | SGT_TankerCamera ) ;

    work->pad = GV_PadData + GM_CurrentCameraChanl ;
    work->subject = GM_PlayerSubjectCamera[ GM_CurrentCameraChanl ] ;
    work->state = STATE_NORMAL ;
	work->child_status = SK_ANDER_MOVING;
	work->time = 0;
	work->frame_count = 0;
	// シグナルの登録
	GV_SetActorSignalFunc( work , SignalFunc );

	// サイトの起動
	switch( this ){
	case IT_Camera :
		sights = NewDgCamSight( ANGLE_MIN , ANGLE_MAX );
		if ( sights == NULL ){
			return ( -1 );
		}
		GV_SetActorChild( work , sights );
		work->act = ( GM_GameStatus & STATE_VR_ANOTHER ) ? ( void *)VrAct : ( void *)NormalAct;
		break;
	case IT_TnkCamera :
		sights = NewTDgCamSight( ANGLE_MIN , ANGLE_MAX , &work->frame_number );
		if ( sights == NULL ){
			return ( -1 );
		}
		GV_SetActorChild( work , sights );
		pict = NewPictureDrawManager( 64 );
		if ( pict == NULL ){
			return ( -1 );
		}
		GV_SetActorChild( work , pict );

		work->act = ( void * )TankerAct;
		break;
	}

	/* 子画面、鏡用モデル */
	GM_InitObject( &work->equip, MDL_CAMERA, DG_FLAG_ONEPIECE|DG_FLAG_SHADE ) ;
	if ( work->equip.objs == NULL ) return -1 ;
	GM_ConfigObjectRoot( &work->equip, GM_PlayerBody, HUMAN21_MIGI_TE ) ;
	DG_InvisibleObjs( work->equip.objs ) ;
	DG_VisibleObjsChanl( work->equip.objs, 1 ) ;

    return 0 ;
}

/*------------------------------------------------------------------*/

void		*NewDigitalCamera( ctrl, body, unit, trigger , this )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int      this ;
{
    Work	*work ;

    work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_WPMNG_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, ctrl, body, unit , this ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->trigger = trigger ;
		work->chanl = GM_CurrentCameraChanl ;
    }

    return work ;
}

void *NewTnkDigitalCamera( CONTROL *ctrl , OBJECT **body , int *unit , u_int *trigger )
{
	return ( NewDigitalCamera( ctrl , body , unit , trigger , IT_TnkCamera ) );
}

void *NewNormalDigitalCamera( CONTROL *ctrl , OBJECT **body , int *unit , u_int *trigger )
{
	return ( NewDigitalCamera( ctrl , body , unit , trigger , IT_Camera ) );
}
