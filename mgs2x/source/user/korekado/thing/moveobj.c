//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	moveobj.c
	移動オブジェクト設置

	2000/12/27 Y.Korekado
	$Id: moveobj.c,v 1.1.1.3 2002/11/19 11:44:28 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------
void	*NewMoveObject( name, where )

シナリオオプション
		-m	オブジェクト名
		-p	場所
		-r	向き
		-s  ステータス

----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"camera.h"

#include	"korekado/conv/korekado.x"
/*----------------------------------------------------------------*/
#define	BODY_FLAG			(DG_FLAG_TEXT|DG_FLAG_PAINT|DG_FLAG_ONEPIECE)
#define	BODY_FLAG_SHADOW	(DG_FLAG_TEXT|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)

#define	PO_STATUS_SHADE		0x00000001
#define	PO_STATUS_SPOTLIGHT	0x00000002
#define	PO_STATUS_LOD		0x00000004
#define	PO_STATUS_D_HAZARD	0x00000008	/* モデルのバウンディングに動的壁セット */
#define	PO_STATUS_TRANS_SCALE	0x00000010	/* トランスにスケールをかける */
#define	PO_STATUS_NO_PRESHADE	0x00000020	/* プレイシェードしない */
#define	PO_STATUS_AUTOREPAINT	0x00000040	/* オートリペイント */
#define	PO_STATUS_OBJ_OFF	0x10000000

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT		actor ;
	OBJECT		body ;

	FMATRIX		lights[2] ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	FVECTOR		scale ;
	int			status ;
	int			name ;
	int			lit ;
	int			scene ;
	int			count ;
} Work ;

enum {
	OBJ_OFF=0,
	OBJ_ON,
	OBJ_TURN,
	OBJ_MOVE,
	OBJ_PRESHADE,
	OBJ_KILL
};

enum {
	SCENE_WAIT,
	SCENE_MOVE,
	SCENE_STOP,
} ;

/*----------------------------------------------------------------*/
extern void *NewMakeObjectBoundHazard( HZX_GROUP_ID	hzx_id, DG_OBJS *objs, u_int seg_atr, u_int flr_atr ) ;
/*----------------------------------------------------------------*/

static void SetObject( work )
Work *work ;
{
	FMATRIX	mat ;

	_RotMatrixZYX( &mat, &work->rot ) ;
	_ScaleMatrix( &mat, &work->scale ) ;
	KR_FvecToMat( &work->pos, &mat ) ;
	DG_SetPos( &mat ) ;

	DG_PutObjs( work->body.objs ) ;
}

/*----------------------------------------------------------------*/
static void SceneWait( Work *work )
{
	if( GV_PadData[ 1 ].press & PAD_L2 ){
		work->scene = SCENE_MOVE ;
		work->count  = 0 ;
		return ;
	}
}
static void SceneMove( Work *work )
{
	work->rot.vz += 128 ;
	SetObject( work ) ;

	if ( work->count >= 7 ) {

GM_SeSetMode( SD_W_BOUND02, &work->pos, GM_SEMODE_BOMB ) ;
GM_SetNoise( NOISE_S , &work->pos, 0 ) ;

		work->scene = SCENE_STOP ;
		work->count  = 0 ;
		return ;
	}

	work->count ++ ;
}

static void SceneStop( Work *work )
{
	if( GV_PadData[ 1 ].press & PAD_L1 ){
		work->rot.vz -= 1024 ;
		SetObject( work ) ;
		work->scene = SCENE_WAIT ;
		work->count  = 0 ;
		return ;
	}

	work->count ++ ;
}
static void MoveObject( Work *work )
{
	switch( work->scene ) {
		case SCENE_WAIT :
			SceneWait( work ) ;
		break ;
		case SCENE_MOVE :
			SceneMove( work ) ;
		break ;
		case SCENE_STOP :
			SceneStop( work ) ;
		break ;
	}
}
/*----------------------------------------------------------------*/
static void CheckMessage( work )
Work	*work ;
{
	GV_MSG *msg;
	int mes_num ;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		switch( msg->message[0] ){
		  case OBJ_OFF:
		  	work->status |= PO_STATUS_OBJ_OFF ;
			work->body.objs->flag |= DG_FLAG_INVISIBLE ;
//printf("putobj: model[%d] OFF \n",work->name ) ;
			break;
		  case OBJ_ON:
		  	work->status &= ~PO_STATUS_OBJ_OFF ;
			work->body.objs->flag &= ~DG_FLAG_INVISIBLE ;
			break;
		  case OBJ_TURN:
			if ( msg->message_len >= 4 ) {
				work->rot.vx = (short)msg->message[ 1 ] ;
				work->rot.vy = (short)msg->message[ 2 ] ;
				work->rot.vz = (short)msg->message[ 3 ] ;
			}
			SetObject( work ) ;
			break;
		  case OBJ_PRESHADE:
			ENE_PreShade( work->body.objs, work->lit ) ;
			break;
		  case OBJ_MOVE:	/* 同マップ内限定 */
			if ( msg->message_len >= 4 ) {
				work->pos.vx = (float)msg->message[ 1 ] ;
				work->pos.vy = (float)msg->message[ 2 ] ;
				work->pos.vz = (float)msg->message[ 3 ] ;
			}
			SetObject( work ) ;
			break;
		  case OBJ_KILL:
			GV_DestroyActor( work ) ;
			break;
		}
		msg--;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	CheckMessage( work ) ;

	MoveObject( work ) ;

//AN_Test_Eye2( &work->pos, 1 );

	if ( work->status & PO_STATUS_SHADE ) {
		DG_GetLightMatrix( &work->pos, (FMATRIX *)work->lights );
	}
}

static	void	Die( work )
Work		*work ;
{
	extern	void	DG_FreePreshade( DG_OBJS * ) ;

    DG_FreePreshade( work->body.objs ) ;
    GM_FreeObject( &work->body ) ;
}

/*----------------------------------------------------------------*/
static	int	GetResources( work, name, where )
Work	*work ;
int		name ;
int		where ;
{
	char		*opt ;
	int			model ;

	work->name = name ;

	work->status = GCL_GetOptionValue( 'f', 0 ) ;
printf("PutObj Status [%d]\n",work->status );

	GM_CurrentMap = where ;	
	if ( ( opt = GCL_GetOption( 'm' ) ) != NULL ){
		if ( ( ENE_GclGetInt( opt, &model )) != 1 ) return -1 ;

		if( work->status & (PO_STATUS_SHADE|PO_STATUS_NO_PRESHADE) ) {
			GM_InitObject( &work->body, model, BODY_FLAG_SHADOW );
			GM_ConfigObjectLight( &work->body, (FMATRIX *)work->lights );
		} else {
			GM_InitObject( &work->body, model, BODY_FLAG );
		}
	    GM_GroupObjs( work->body.objs, GM_CurrentMap ) ;
	} else {
		return -1 ;
	}

	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		ENE_GCL_GetFV( opt, &work->pos ) ;
	} else {
		work->pos = DG_ZeroVector ;
	}

	if ( ( opt = GCL_GetOption( 'r' ) ) != NULL ){
		GCL_GetSV( opt, (short *)&work->rot ) ;
	} else {
		work->rot = DG_ZeroSVector ;
	}
	
	if ( ( opt = GCL_GetOption( 's' ) ) != NULL ){
		ENE_GCL_GetFV( opt, &work->scale ) ;
		work->scale.vx /= 100.0F ;
		work->scale.vy /= 100.0F ;
		work->scale.vz /= 100.0F ;
	} else {
		work->scale.vx = work->scale.vy = work->scale.vz = 1.0F ;
	}

	if ( work->status & (PO_STATUS_SHADE|PO_STATUS_SPOTLIGHT) ) {
		work->body.objs->flag |= DG_FLAG_SHADOWMAKE ;
	}

//printf("scale [%f][%f][%f]\n",work->scale.vx,work->scale.vy,work->scale.vz );

	if( work->status & PO_STATUS_D_HAZARD ) {
		NewMakeObjectBoundHazard( GM_GetHzxGroupID( where ), work->body.objs, 0, 0 ) ;
	}

	/* スケーリング前に座標移動 */
	DG_SetPos2( &work->pos, &work->rot ) ;
	DG_PutObjs( work->body.objs ) ;

	/* プリシェードはスケーリング前に */
	if( !(work->status & (PO_STATUS_SHADE|PO_STATUS_NO_PRESHADE)) ) {
		if ( (work->lit = GCL_GetOptionValue( 'l', 0 )) != 0 ) {
			ENE_PreShade( work->body.objs, work->lit ) ;
			if ( work->status & PO_STATUS_AUTOREPAINT ) {
				work->body.objs->flag |= DG_FLAG_AUTOREPAINT ;
			}
		}
	}

	if( work->status & PO_STATUS_NO_PRESHADE ) {
		work->lights[1].m[3][0] = 128 ;
		work->lights[1].m[3][1] = 128 ;
		work->lights[1].m[3][2] = 128 ;
	}

	if( work->status & PO_STATUS_TRANS_SCALE ) {
		FVECTOR	ofset ;
		DG_DEF	*def ;

		def = work->body.objs->def ;
		ofset.vx = def->tx * (work->scale.vx-1.0f) ;
		ofset.vy = def->ty * (work->scale.vy-1.0f) ;
		ofset.vz = def->tz * (work->scale.vz-1.0f) ;
		_sceVu0AddVector( &work->pos, &work->pos, &ofset ) ;
printf("scale [%f][%f][%f] ",work->scale.vx,work->scale.vy,work->scale.vz) ;
printf("trans [%f][%f][%f] ",def->tx,def->ty,def->tz) ;
printf("> pos [%f][%f][%f] ",work->pos.vx,work->pos.vy,work->pos.vz) ;
	}

	SetObject( work ) ;

	work->scene = SCENE_WAIT ;
	work->count  = 0 ;

printf( " status[%x] work->body.objs->flag=[%x] \n",work->status, work->body.objs->flag ) ;
	return 0 ;
}

/*
	シナリオ呼び出し
	*/
void		*NewMoveObject( name, where )
int		name ;
int		where ;
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
