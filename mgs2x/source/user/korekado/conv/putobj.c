//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	putobj.c
	オブジェクト設置

	1997/07/14 Y.Korekado
	$Id: putobj.c,v 1.1.1.3 2002/11/19 11:44:04 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewPutObject( name, where )

シナリオオプション
		-m	オブジェクト名
		-p	場所
		-r	向き
		-s  ステータス

----------------------------------------------------------------*/

#ifdef PSX2
#include <eekernel.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
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
#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_PAINT|DG_FLAG_ONEPIECE)
#define	BODY_FLAG_SHADOW	(DG_FLAG_TEXT|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
#define MAX_OBJECT	4	/* ＬＯＤ */

#define	PO_STATUS_SHADE			0x00000001
#define	PO_STATUS_SPOTLIGHT		0x00000002
#define	PO_STATUS_LOD			0x00000004
#define	PO_STATUS_D_HAZARD		0x00000008	/* モデルのバウンディングに動的壁セット */
#define	PO_STATUS_TRANS_SCALE	0x00000010	/* トランスにスケールをかける */
#define	PO_STATUS_NO_PRESHADE	0x00000020	/* プレイシェードしない */
#define	PO_STATUS_AUTOREPAINT	0x00000040	/* オートリペイント */
#define	PO_STATUS_NOFOG			0x00000080	/* フォグ効かない */
#define	PO_STATUS_IR			0x00000100	/* 赤外線ゴーグル反応 */
#define	PO_STATUS_STAGE_ROTATE	0x00000200	/* ステージモデル（トランス付き）を回転 */
#define	PO_STATUS_NV			0x00000400	/* 暗視ゴーグルに反応 */
#define	PO_STATUS_SEMITRANS		0x00000800	/* 強制半透明 */

#define	PO_STATUS_OBJ_OFF		0x10000000
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT		actor ;
	OBJECT		body[ MAX_OBJECT ] ;

	FMATRIX		lights[2] ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	FVECTOR		scale ;
	int			status ;
	int			name ;
	int			objs_num ;
	int			lod_th[ MAX_OBJECT-1 ] ;
	int			lit ;
	int			inv_channel ;
} Work ;

enum {
	OBJ_OFF=0,
	OBJ_ON,
	OBJ_TURN,
	OBJ_MOVE,
	OBJ_PRESHADE,
	OBJ_KILL
};

/*----------------------------------------------------------------*/
extern void *NewMakeObjectBoundHazard( HZX_GROUP_ID	hzx_id, DG_OBJS *objs, u_int seg_atr, u_int flr_atr ) ;
void		*NewPutObject( int name, int where ) ;
/*----------------------------------------------------------------*/

static void SetObject( Work *work )
{
	FMATRIX	mat ;
	int		i ;

	_RotMatrixZYX( &mat, &work->rot ) ;
	_ScaleMatrix( &mat, &work->scale ) ;
	KR_FvecToMat( &work->pos, &mat ) ;
	DG_SetPos( &mat ) ;

	for ( i=0; i<work->objs_num; i++ ) {
		DG_PutObjs( work->body[i].objs ) ;
	}
}

static void CheckMessage( Work *work )
{
	GV_MSG *msg;
	int mes_num, i ;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		switch( msg->message[0] ){
		  case OBJ_OFF:
		  	work->status |= PO_STATUS_OBJ_OFF ;
			work->body[0].objs->flag |= DG_FLAG_INVISIBLE ;
//printf("putobj: model[%d] OFF \n",work->name ) ;
			break;
		  case OBJ_ON:
		  	work->status &= ~PO_STATUS_OBJ_OFF ;
			work->body[0].objs->flag &= ~DG_FLAG_INVISIBLE ;
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
			for ( i=0; i<work->objs_num; i++ ) {
				ENE_PreShade( work->body[i].objs, work->lit ) ;
			}
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

static	int	CameraDis( FVECTOR *pos )
{
	GM_CameraSet	*cam ;
	FVECTOR		vec ;
	float dis ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
    dis = GV_VecLen3F( &vec ) ;
	dis /= cam->angle/2 ;
	
	return (int)dis ;
}

static	void	SetLod( Work *work )
{
	int cam_dis, i, lod ;
	

	cam_dis = CameraDis( &work->pos ) ;
	lod = 0 ;
	for ( i=0; i<work->objs_num-1; i++ ) {
		if ( cam_dis > work->lod_th[ i ] ) lod = i+1 ;
	}
//printf("cam dis = %d lod[%d]\n",cam_dis,lod ) ;
	for ( i=0; i<work->objs_num; i++ ) {
		if ( i == lod && !(work->status & PO_STATUS_OBJ_OFF) ) {
//			work->body[i].objs->flag &= ~DG_FLAG_INVISIBLE ;
			work->body[i].objs->flag &= ~(work->inv_channel) ;
		} else {
//			work->body[i].objs->flag |= DG_FLAG_INVISIBLE ;
			work->body[i].objs->flag |= (work->inv_channel) ;
		}
	}
}


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	CheckMessage( work ) ;

	if ( work->status & PO_STATUS_LOD ) {
		SetLod( work ) ;
	}
//AN_Test_Eye2( &work->pos, 1 );

	if ( work->status & PO_STATUS_SHADE ) {
		DG_GetLightMatrix( &work->pos, (FMATRIX *)work->lights );
	}

	if ( work->status & PO_STATUS_NV ) {
		if ( Ply_GetPlayerItem() == IT_NightVision ) {
			if ( !(work->status & PO_STATUS_SHADE) ) {
				DG_GetLightMatrix( &work->pos, (FMATRIX *)work->lights );
			}

			work->lights[1].m[3][0] += 42 ;
			work->lights[1].m[3][1] += 42 ;
			work->lights[1].m[3][2] += 42 ;
		}
	}
}

static	void	Die( Work *work )
{
	extern	void	DG_FreePreshade( DG_OBJS * ) ;

	int i ;
	for ( i=0; i<work->objs_num; i++ ) {
	    DG_FreePreshade( work->body[i].objs ) ;
	    GM_FreeObject( &work->body[ i ] ) ;
	}
}

/*----------------------------------------------------------------*/
static	int	GetResources( Work *work, int name, int where )
{
	char		*opt ;
	int			model[MAX_OBJECT], i, num ;

	work->name = name ;

	work->status = GCL_GetOptionValue( 'f', 0 ) ;
printf("PutObj Status [%d]\n",work->status );

	GM_CurrentMap = where ;	
	if ( ( opt = GCL_GetOption( 'm' ) ) != NULL ){
		if ( (work->objs_num = ENE_GclGetInt( opt, &model[0] )) == 0 ) return -1 ;

		for ( i=0; i<work->objs_num; i++ ) {
printf("putobj: model[%d] name[%d]\n",i,model[i]);
			if( work->status & (PO_STATUS_SHADE|PO_STATUS_NO_PRESHADE) ) {
				GM_InitObject( &work->body[i], model[i], BODY_FLAG_SHADOW );
				GM_ConfigObjectLight( &work->body[i], (FMATRIX *)work->lights );
			} else {
				GM_InitObject( &work->body[i], model[i], BODY_FLAG );
			}
		    GM_GroupObjs( work->body[i].objs, GM_CurrentMap ) ;
			if ( i != 0 ) work->body[i].objs->flag |= DG_FLAG_INVISIBLE ;
		}
	} else {
		return -1 ;
	}
	num = 0 ;
	if ( ( opt = GCL_GetOption( 't' ) ) != NULL ){
		num = ENE_GclGetInt( opt, &work->lod_th[0] ) ;
	}
	if ( num != work->objs_num-1 ) return -1 ;

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

	if ( (work->inv_channel = (float)GCL_GetOptionValue( 'i', 0 )) == 0 ) {
		work->inv_channel = DG_FLAG_INVISIBLE1|DG_FLAG_INVISIBLE0 ;
	}

	if ( work->status & (PO_STATUS_SHADE|PO_STATUS_SPOTLIGHT) ) {
		for ( i=0; i<work->objs_num; i++ ) {
			work->body[i].objs->flag |= DG_FLAG_SHADOWMAKE ;
		}
		if ( work->status & PO_STATUS_NOFOG ) {
			for ( i=0; i<work->objs_num; i++ ) {
				work->body[i].objs->flag |= DG_FLAG_NOFOG ;
			}
		}
		if ( work->status & PO_STATUS_IR ) {
			for ( i=0; i<work->objs_num; i++ ) {
				work->body[i].objs->flag |= DG_FLAG_IRREACTION ;
			}
		}
		if ( work->status & PO_STATUS_SEMITRANS ) {
			for ( i=0; i<work->objs_num; i++ ) {
				work->body[i].objs->flag |= DG_FLAG_SEMITRANS ;
			}
		}
	}

//printf("scale [%f][%f][%f]\n",work->scale.vx,work->scale.vy,work->scale.vz );

	if( work->status & PO_STATUS_D_HAZARD ) {
		for ( i=0; i<work->objs_num; i++ ) {	
			NewMakeObjectBoundHazard( GM_GetHzxGroupID( where ), 
							       work->body[ i ].objs, 0, 0 ) ;
		}
	}

	/* スケーリング前に座標移動 */
	if( work->status & PO_STATUS_STAGE_ROTATE ) {
		/* ステージ変換されたモデルはトランス値を回転させる */
		FVECTOR	*trans ;

		DG_SetPos2( &DG_ZeroVector, &work->rot ) ;
		for ( i=0; i<work->objs_num; i++ ) {
			trans = &(work->body[ i ].objs->trans) ;	/* ０のモデルから計算 */
			DG_PutVector( trans, trans, 1 ) ;
{
	DG_DEF	*def ;
	def = work->body[ i ].objs->def ;
printf("putobj: rotate trans [%f][%f][%f] > [%f][%f][%f]\n",def->tx,def->ty,def->tz,
			trans->vx,trans->vy,trans->vz) ;
}
		}
	}

	DG_SetPos2( &work->pos, &work->rot ) ;
	for ( i=0; i<work->objs_num; i++ ) {
		DG_PutObjs( work->body[i].objs ) ;
	}

	/* プリシェードはスケーリング前に */
	if( !(work->status & (PO_STATUS_SHADE|PO_STATUS_NO_PRESHADE)) ) {
		if ( (work->lit = GCL_GetOptionValue( 'l', 0 )) != 0 ) {
			for ( i=0; i<work->objs_num; i++ ) {
				ENE_PreShade( work->body[i].objs, work->lit ) ;
				if ( work->status & PO_STATUS_AUTOREPAINT ) {
					work->body[i].objs->flag |= DG_FLAG_AUTOREPAINT ;
				}
			}
		}
	}

	if( work->status & PO_STATUS_NO_PRESHADE ) {
		work->lights[1].m[3][0] = 128 ;
		work->lights[1].m[3][1] = 128 ;
		work->lights[1].m[3][2] = 128 ;
	}

	if( work->status & PO_STATUS_TRANS_SCALE ) {
#if 1
		FVECTOR	*trans ;
		for ( i=0; i<work->objs_num; i++ ) {
			trans = &(work->body[ i ].objs->trans) ;	/* ０のモデルから計算 */
			trans->vx *= (work->scale.vx) ;
			trans->vy *= (work->scale.vy) ;
			trans->vz *= (work->scale.vz) ;
		}
#else
		FVECTOR	ofset ;
		DG_DEF	*def ;
		for ( i=0; i<work->objs_num; i++ ) {
			def = work->body[ i ].objs->def ;
			ofset.vx = def->tx * (work->scale.vx-1.0f) ;
			ofset.vy = def->ty * (work->scale.vy-1.0f) ;
			ofset.vz = def->tz * (work->scale.vz-1.0f) ;
			_sceVu0AddVector( &work->pos, &work->pos, &ofset ) ;
printf("scale [%f][%f][%f] ",work->scale.vx,work->scale.vy,work->scale.vz) ;
printf("trans [%f][%f][%f] ",def->tx,def->ty,def->tz) ;
printf("> pos [%f][%f][%f] ",work->pos.vx,work->pos.vy,work->pos.vz) ;
		}
#endif
	}

	SetObject( work ) ;

printf( " status[%x] work->body[i].objs->flag=[%x] \n",work->status, work->body[0].objs->flag ) ;
	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void		*NewPutObject( int name, int where )
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
