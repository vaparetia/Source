//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dg_cam.c 
   デジタルカメラ 
   
   2000/12/05 M.Sonoyama 
   $Id: dg_cam.c,v 1.1.1.3 2002/11/19 11:50:15 Yoshizawa1 Exp $ 
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

#include "bp_matrix.h"

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
#define	PAD_SHUTTER	(PL_PAD_WEAPON)

#define	PROC_TIME	(90)

/*------------------------------------------------------------------*/
// こば４追加 
enum {
	SK_VISIBLE = 0,
	SK_INVISIBLE,
};

enum {
    STATE_NORMAL = 0,
    STATE_SHUTTER_ON,
	STATE_SAVE ,
	STATE_READY ,
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
	FVECTOR		*checkadd ;
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

typedef	struct _work {
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
} Work ;

/*------------------------------------------------------------------*/

static	float	CPERS_X, CPERS_Y, PERS_SIZE ;

int	PL_GetDGCamSubject( void )
{
	GCL_VAR_REF ref; //配列への参照データ

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)CPERS_X );

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)CPERS_Y );

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)PERS_SIZE );

	return 1 ;
}
/*------------------------------------------------------------------*/
/* 面積計算 */
static	float	ExprSize( FVECTOR *v )
{
	float		s1, s2 ;

	s1 = ( v[ 2 ].vx - v[ 1 ].vx ) * ( v[ 1 ].vy - v[ 0 ].vy ) -
		 ( v[ 1 ].vx - v[ 0 ].vx ) * ( v[ 2 ].vy - v[ 1 ].vy ) ;
	s2 = ( v[ 3 ].vx - v[ 2 ].vx ) * ( v[ 2 ].vy - v[ 0 ].vy ) -
		 ( v[ 2 ].vx - v[ 0 ].vx ) * ( v[ 3 ].vy - v[ 2 ].vy ) ;
	return DG_FABS( ( s1 + s2 ) / 2.0F ) ;
}

/* 面にある点が含まれるかチェック */
static	int		CheckPointInside( FVECTOR *v, float px, float py )
{
	FVECTOR		p1, p2 ;
	float		opz ;

	p1.vx = px - v[ 0 ].vx ;
	p1.vz = py - v[ 0 ].vy ;
	p2.vx = v[ 1 ].vx - v[ 0 ].vx ;
	p2.vz = v[ 1 ].vy - v[ 0 ].vy ;
	opz = p1.vz * p2.vx - p1.vx * p2.vz ;
	if ( opz < 0.0F ) return 0 ;
	p1.vx = px - v[ 1 ].vx ;
	p1.vz = py - v[ 1 ].vy ;
	p2.vx = v[ 2 ].vx - v[ 1 ].vx ;
	p2.vz = v[ 2 ].vy - v[ 1 ].vy ;
	opz = p1.vz * p2.vx - p1.vx * p2.vz ;
	if ( opz < 0.0F ) return 0 ;
	p1.vx = px - v[ 2 ].vx ;
	p1.vz = py - v[ 2 ].vy ;
	p2.vx = v[ 3 ].vx - v[ 2 ].vx ;
	p2.vz = v[ 3 ].vy - v[ 2 ].vy ;
	opz = p1.vz * p2.vx - p1.vx * p2.vz ;
	if ( opz < 0.0F ) return 0 ;
	p1.vx = px - v[ 3 ].vx ;
	p1.vz = py - v[ 3 ].vy ;
	p2.vx = v[ 0 ].vx - v[ 3 ].vx ;
	p2.vz = v[ 0 ].vy - v[ 3 ].vy ;
	opz = p1.vz * p2.vx - p1.vx * p2.vz ;
	if ( opz < 0.0F ) return 0 ;
	return 1 ;
}

/* キャラ、ターゲットチェック用コールバック */
static	void	Hit( TARGET *off, TARGET *def, void *ptr )
{	
	CHK_CH		*ch ;
	FVECTOR		pos ;
	FMATRIX		*m ;

	ch = ( CHK_CH * )ptr ;
	m = &ch->body->objs->objs[ ch->joint ].world ;
	GV_MatToVec( m, &pos ) ;
	//printf( "chara : target target0!!\n" ) ;
	if ( GV_VecLen3F2( &off->hit, &pos ) > 1000.0F ) {
		//printf( "chara : target target1!!\n" ) ;
		ch->flag = 1 ;
	}
}

/* キャラ、ターゲットチェック */
static	void	Chara_CheckTarget( Work *work )
{
	CHK_CH		*chkch ;
	TARGET		*t ;
	DG_OBJ		*obj ;
	FMATRIX		*m ;
	FVECTOR		pos ;
	int			i ;

	chkch = CheckChara ;
	for ( i = 0; i < PL_DGCam_N_CheckCharas; i ++, chkch ++ ) {
		obj = &chkch->body->objs->objs[ chkch->joint ] ;
		m = &obj->world ;
		GV_MatToVec( m, &pos ) ;
		
		chkch->flag = 0 ;
		t = &chkch->target ;
		GM_SetTarget( t, TARGET_OFFENSE | TARGET_SEEK | TARGET_ONLINE | TARGET_THROUGH |
					  TARGET_ONLINE_MIN | TARGET_CHILD, 0, ENEMY_SIDE, 
					  &DG_ZeroVector, &DG_ZeroVector ) ;
		GM_SetTargetWeaponType( t, WP_LASERSIGHT ) ;
		GM_SetTargetCallBack( t, Hit, chkch ) ;

		GM_MoveOnlineTargetMap( t, &work->camera->position, &pos, GM_PlayerMap ) ;
		GM_PutTarget( t ) ;
	}
}

/* 面、ターゲットチェック用コールバック */
static	void	SQ_Hit( TARGET *off, TARGET *def, void *ptr )
{	
	CHK_SQ		*ch ;

	//printf( "nogenoge %f %f %f\n", def->size.vx, def->size.vy, def->size.vz ) ;
	if ( def->size.vx > 120.0F ||	
		 def->size.vy > 120.0F ||
		 def->size.vz > 120.0F ) {
		ch = ( CHK_SQ * )ptr ;
		ch->flag = 1 ;
	}
}

/* 面、ターゲットチェック */
static	void	Square_CheckTarget( Work *work )
{
	CHK_SQ		*chksq ;
	TARGET		*t ;
	int			i ;

	chksq = CheckSquare ;
	for ( i = 0; i < PL_DGCam_N_CheckSquares; i ++, chksq ++ ) {
		chksq->flag = 0 ;
		t = &chksq->target ;
		GM_SetTarget( t, TARGET_OFFENSE | TARGET_SEEK | TARGET_ONLINE | TARGET_THROUGH |
					  TARGET_ONLINE_MIN | TARGET_CHILD, 0, ENEMY_SIDE, 
					  &DG_ZeroVector, &DG_ZeroVector ) ;
		GM_SetTargetWeaponType( t, WP_LASERSIGHT ) ;
		GM_SetTargetCallBack( t, SQ_Hit, chksq ) ;
		GM_MoveOnlineTargetMap( t, &work->camera->position, &chksq->center, GM_PlayerMap ) ;
		GM_PutTarget( t ) ;
	}
}

/* 指定ポイントがカメラに写ってるか */
void SK_DGcamCheck( Work *work, int mode )
{	
	//int			time ;
    int			i, j ;
    FVECTOR		*p, dir ;
    FVECTOR		pers[ 4 ], cpers = { 0 };
    GCL_ARGS	args ;
    int			buf[ 1 ], res, pchk, hchk ;
	CHK_SQ		*chksq ;
	CHK_CH		*chkch ;
	float		ip, size = 0;

	if ( mode == 0 ) {
		/* キャラチェック用に
		   ターゲットを飛ばす */
		Chara_CheckTarget( work ) ;
		/* 面がターゲットに遮られてないか
		   ターゲットを飛ばす */
		Square_CheckTarget( work ) ;
	} else if ( mode == 1 ) {
printf( "[%d] shutter\n",  GV_Time ) ;
		_sceVu0SubVector( &dir, &work->camera->position, &work->camera->target ) ;
		chksq = CheckSquare ;
		for ( i = 0; i < PL_DGCam_N_CheckSquares; i ++, chksq ++ ) {
			ip = _sceVu0InnerProduct( &dir, &chksq->normal ) ;
			res = 0 ;
			if ( ip <= 0.0F ) {
				/* 裏面 */
				printf( "reverse\n" ) ;
				goto result_out ;
			}
			if ( GV_DiffDirAbs( GV_VecDir2( &dir ), GV_VecDir2( &chksq->normal ) ) > chksq->yrange ) {
				/* Ｙ角度違いすぎ */
				printf( "y difference\n" ) ;
				goto result_out ;
			}
			if ( GV_DiffDirAbs( GV_VecDir2X( &dir ), GV_VecDir2X( &chksq->normal ) ) > chksq->xrange ) {
				/* Ｘ角度違いすぎ */
				printf( "x difference\n" ) ;
				goto result_out ;
			}
			if ( chksq->flag != 0 ) {
				/* ターゲットがある */
				printf( "sq target exist\n" ) ;
				goto result_out ;
			}
			pchk = hchk = 0 ;
			for ( j = 0; j < 4; j ++ ) {
				p = &chksq->p[ j ] ;
				DG_TransPersOneChanl( &pers[ j ], p, 0 ) ;
				pers[ j ].vx /= pers[ j ].vw ;
				pers[ j ].vy /= pers[ j ].vw ;
//				DumpVec( &pers[ j ] ) ;
				if ( pers[ j ].vz < pers[ j ].vw ) {
					if ( ( DG_FABS( pers[ j ].vx ) < 1.0F ) &&
						 ( DG_FABS( pers[ j ].vy ) < 1.0F ) ) {
						/* 画面内 */
						if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &work->camera->position,
												    p, HZX_CHK_ALL, 
												    HZX_SEG_NO_ENEMY_EYES | HZX_SEG_RECOIL_TYPE,
												    HZX_FLOOR_NO_ENEMY_EYES | HZX_FLOOR_RECOIL_TYPE ) ) {
							/* 壁がある */
							hchk |= ( 1 << j ) ;
						}
					} else {
						/* 画面外 */
						pchk |= ( 1 << j ) ;
					}
				} else {
					/* 画面外 */
					pchk |= ( 1 << j ) ;
				}
			}
			DG_TransPersOneChanl( &cpers, &chksq->center, 0 ) ;
			cpers.vx /= cpers.vw ;
			cpers.vy /= cpers.vw ;
printf( "pchk %x\n", pchk ) ;
			if ( pchk == 0x0f ) {
				/* 全点が画面外 */
				/* 画面が全部チェック面なら「近すぎ」 */
				if ( CheckPointInside( pers, -1.0F, -1.0F ) &&
					 CheckPointInside( pers, 1.0F, -1.0F ) &&
					 CheckPointInside( pers, -1.0F, 1.0F ) &&
					 CheckPointInside( pers, 1.0F, 1.0F ) ) {
					printf( "near near near\n" ) ;
					res = 1 ;
					goto result_out ;
				}
			}
			if ( pchk != 0 ) {
				/* 画面外の点がある */ 
				if ( cpers.vz > cpers.vw || 
					 DG_FABS( cpers.vx ) > 0.30F ||
					 DG_FABS( cpers.vy ) > 0.30F ) {
					/* 面中心が画面中心からはずれていたらＮＧ */
					printf( "center hazure\n" ) ;
					res = 0 ;
					goto result_out ;
				}
				/* 近すぎる */
				printf( "near near\n" ) ;
				res = 1 ;
				goto result_out ;
			} else {
				/* 全て画面内 */
				if ( hchk != 0 ) {
					/* 当たりで見えない点があるとＮＧ */
					printf( "hazard exist %x\n", hchk ) ;
					res = 2 ;
					goto result_out ;
				}
				size = ExprSize( pers ) ;
				printf( "size %f(%.2f)\n", size, size / 4.0F * 100.0F ) ;
				if ( chksq->far_x > size / 4.0F * 100.0F ) {
					printf( "far far\n" ) ;
					res = 3 ;
					goto result_out ;
				}
				res = 4 ;
			}
result_out :
			{
				/* 結果パラメータ */
				CPERS_X = cpers.vx*100.0 ;
				CPERS_Y = cpers.vy*100.0 ;
				PERS_SIZE = size / 4.0F * 10000.0F ;
printf(" CPERS_X[%f] CPERS_Y[%f] PERS_SIZE[%f]\n",CPERS_X, CPERS_Y, PERS_SIZE ) ;
			}

	        if ( chksq->proc != 0 ) {
				args.argc = 1 ; args.argv = buf ; buf[ 0 ] = res ;
				GM_ExecProc( chksq->proc, &args ) ;
			}
		}
		/* キャラ */
		res = 0 ;
		chkch = CheckChara ;
		for ( i = 0; i < PL_DGCam_N_CheckCharas; i ++, chkch ++ ) {
			if ( ( !( chkch->body->objs->flag & DG_FLAG_INVISIBLE0 ) ||
				  ( chkch->body->evmobj != NULL && !( chkch->body->evmobj->flag & DG_EVMOBJ_INVISIBLE0 ) ) ) &&
				 !DG_ObjBoundCheckChanl( chkch->body->objs, chkch->joint, 0 ) ) {
				DG_MDL		*mdl ;
				DG_OBJ		*obj ;
				FMATRIX		*m ;
				FVECTOR		pos, low, up, max, min, cur, curp ;
				
				obj = &chkch->body->objs->objs[ chkch->joint ] ;
				mdl = &chkch->body->objs->def->models[ chkch->joint ] ;
				m = &obj->world ;
				GV_MatToVec( m, &pos ) ;

				DG_TransPersOneChanl( &cpers, &pos, 0 ) ;
				cpers.vx /= cpers.vw ;
				cpers.vy /= cpers.vw ;
				if ( cpers.vz > cpers.vw || 
					 DG_FABS( cpers.vx ) > 0.85F ||
					 DG_FABS( cpers.vy ) > 0.85F ) {
					res = 0 ;
					printf( "chara center hazure\n" ) ;
					goto chara_result_out ;
				}
				if ( chkch->flag != 0 ) {
					/* ターゲットがある */
					printf( "chara target exist\n" ) ;
					res = 0 ;
					goto chara_result_out ;					
				}
				if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &work->camera->position,
										   &pos, HZX_CHK_ALL, 
										   HZX_SEG_NO_ENEMY_EYES | HZX_SEG_RECOIL_TYPE,
										   HZX_FLOOR_NO_ENEMY_EYES | HZX_FLOOR_RECOIL_TYPE ) ) {
					/* 壁がある */
					printf( "chara hazard exist\n" ) ;
					res = 0 ;
					goto chara_result_out ;
				}
				GV_SetVec3( &max, -1000000.0F, -1000000.0F, -1000000.0F ) ;
				GV_SetVec3( &min, 1000000.0F, 1000000.0F, 1000000.0F ) ;
				GV_SetVec3( &low, ( float )mdl->lx, ( float )mdl->ly, ( float )mdl->lz ) ;
				GV_SetVec3( &up, ( float )mdl->ux, ( float )mdl->uy, ( float )mdl->uz ) ;
				DG_SetPos( m ) ;
				for ( j = 0; j < 8; j ++ ) {
					cur.vx = ( j & 1 ) ? low.vx : up.vx ;
					cur.vy = ( j & 2 ) ? low.vy : up.vy ;
					cur.vz = ( j & 4 ) ? low.vz : up.vz ;
					cur.vw = 1.0F ;
					DG_PutVector( &cur, &cur, 1 ) ;
					DG_TransPersOneChanl( &curp, &cur, 0 ) ;
					curp.vx /= curp.vw ;
					curp.vy /= curp.vw ;
					if ( curp.vx > max.vx ) {
						max.vx = curp.vx ;
					}
					if ( curp.vx < min.vx ) {
						min.vx = curp.vx ;
					}
					if ( curp.vy > max.vy ) {
						max.vy = curp.vy ;
					}
					if ( curp.vy < min.vy ) {
						min.vy = curp.vy ;
					}
				}
				DG_COPY_VEC( &pers[ 0 ], &min ) ;
				DG_COPY_VEC( &pers[ 2 ], &max ) ;
				DG_COPY_VEC( &pers[ 1 ], &pers[ 0 ] ) ;
				pers[ 1 ].vy = pers[ 2 ].vy ;
				DG_COPY_VEC( &pers[ 3 ], &pers[ 2 ] ) ;
				pers[ 3 ].vy = pers[ 0 ].vy ;
				size = ExprSize( pers ) ;
				printf( "size %f(%.2f)\n", size, size / 4.0F * 100.0F ) ;
				if ( chkch->far_x > size / 4.0F * 100.0F ) {
					printf( "chara far far\n" ) ;
					res = 0 ;
					goto chara_result_out ;
				}				
				res = 1 ;
			}
chara_result_out :
			{
				/* 結果パラメータ */
				CPERS_X = cpers.vx*100.0 ;
				CPERS_Y = cpers.vy*100.0 ;
				PERS_SIZE = size / 4.0F * 10000.0F ;
printf(" CPERS_X[%f] CPERS_Y[%f] PERS_SIZE[%f]\n",CPERS_X, CPERS_Y, PERS_SIZE ) ;
			}
	        if ( chkch->proc != 0 ) {
				args.argc = 1 ; args.argv = buf ; buf[ 0 ] = res ;
				GM_ExecProc( chkch->proc, &args ) ;
			}			
		}
#if 0
		GM_ResetPlayerStatus( PLAYER_MENU_DISABLE | PLAYER_PAD_OFF ) ;
		work->state = STATE_READY ;
		GV_PauseOffActorSystem( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ;
		GM_ResetMenuStatus( MENU_MENU_OFF | MENU_GAGE_OFF ) ;
#endif
        if ( PL_DGCam_ShutterProc != 0 ) {
			GM_ExecProc( PL_DGCam_ShutterProc, NULL ) ;
		}
	}
}

/*------------------------------------------------------------------*/

/* チェック面領域の確保 */
typedef	struct	{
	GV_ACT		actor ;
	CHK_SQ		chksq[ MAX_CHK_SQS ] ;
} CSW ;

static	void	DieCSW( CSW *work )
{

}

int	NewMallocCameraSquareWork( void )
{
	CSW			*work ;

	work = ( CSW * )GV_NewActor( GV_ACTOR_PLAYER, sizeof( CSW ) ) ;
	ASSERT( work != NULL ) ;
	GV_SetActor( &work->actor, NULL, DieCSW ) ;
	CheckSquare = &work->chksq[ 0 ] ;
	return 0 ;
}

/* チェック面の設定 */
int	NewSetCheckSquareOfCamera( void )
{
	int			nsqs ;
	FVECTOR		v1, v2, v3, v4 ;
	FVECTOR		normal, center ;
	CHK_SQ		*chksq ;

	nsqs = PL_DGCam_N_CheckSquares ;
	ASSERT( nsqs < MAX_CHK_SQS ) ;
	chksq = &CheckSquare[ nsqs ] ;

	GCL_GetOption( 'p' ) ;
	PL_GetNextFV( &v1 ) ;
	PL_GetNextFV( &v2 ) ;
	PL_GetNextFV( &v3 ) ;
	PL_GetNextFV( &v4 ) ;

    normal.vx = ( v3.vy - v2.vy ) * ( v2.vz - v1.vz ) -
		( v2.vy - v1.vy ) * ( v3.vz - v2.vz ) ;
    normal.vy = ( v3.vz - v2.vz ) * ( v2.vx - v1.vx ) -
		( v2.vz - v1.vz ) * ( v3.vx - v2.vx ) ;
    normal.vz = ( v3.vx - v2.vx ) * ( v2.vy - v1.vy ) -
		( v2.vx - v1.vx ) * ( v3.vy - v2.vy ) ;

	center.vx = ( v1.vx + v2.vx + v3.vx + v4.vx ) / 4.0F ;
	center.vy = ( v1.vy + v2.vy + v3.vy + v4.vy ) / 4.0F ;
	center.vz = ( v1.vz + v2.vz + v3.vz + v4.vz ) / 4.0F ;
	center.vw = 1.0F ;
	
	DG_COPY_VEC( &chksq->p[ 0 ], &v1 ) ;
	DG_COPY_VEC( &chksq->p[ 1 ], &v2 ) ;
	DG_COPY_VEC( &chksq->p[ 2 ], &v3 ) ;
	DG_COPY_VEC( &chksq->p[ 3 ], &v4 ) ;
	DG_COPY_VEC( &chksq->normal, &normal ) ;
	DG_COPY_VEC( &chksq->center, &center ) ;
	
	chksq->proc = GCL_GetOptionValue( 'R', 0 ) ;
	chksq->far_x = ( float )GCL_GetOptionValue( 'f', 0 ) ;

	chksq->xrange = GCL_GetOptionValue( 'x', 640 ) ;
	chksq->yrange = GCL_GetOptionValue( 'y', 320 ) ;

	chksq->flag = 0 ;
	chksq->checkadd = NULL;

	PL_DGCam_N_CheckSquares ++ ;
	return 0 ;
}

/* チェック面の設定 */
int	PL_SetCheckSquareOfCamera( FVECTOR *checkadd, int proc, float far_x, int xrange, int yrange,
	FVECTOR *v1, FVECTOR *v2, FVECTOR *v3, FVECTOR *v4 )
{
	int			nsqs ;
	FVECTOR		normal, center ;
	CHK_SQ		*chksq ;

	nsqs = PL_DGCam_N_CheckSquares ;
	ASSERT( nsqs < MAX_CHK_SQS ) ;
	chksq = &CheckSquare[ nsqs ] ;

    normal.vx = ( v3->vy - v2->vy ) * ( v2->vz - v1->vz ) -
		( v2->vy - v1->vy ) * ( v3->vz - v2->vz ) ;
    normal.vy = ( v3->vz - v2->vz ) * ( v2->vx - v1->vx ) -
		( v2->vz - v1->vz ) * ( v3->vx - v2->vx ) ;
    normal.vz = ( v3->vx - v2->vx ) * ( v2->vy - v1->vy ) -
		( v2->vx - v1->vx ) * ( v3->vy - v2->vy ) ;

	center.vx = ( v1->vx + v2->vx + v3->vx + v4->vx ) / 4.0F ;
	center.vy = ( v1->vy + v2->vy + v3->vy + v4->vy ) / 4.0F ;
	center.vz = ( v1->vz + v2->vz + v3->vz + v4->vz ) / 4.0F ;
	center.vw = 1.0F ;
	
	DG_COPY_VEC( &chksq->p[ 0 ], v1 ) ;
	DG_COPY_VEC( &chksq->p[ 1 ], v2 ) ;
	DG_COPY_VEC( &chksq->p[ 2 ], v3 ) ;
	DG_COPY_VEC( &chksq->p[ 3 ], v4 ) ;
	DG_COPY_VEC( &chksq->normal, &normal ) ;
	DG_COPY_VEC( &chksq->center, &center ) ;
	
	chksq->proc = proc ;
	chksq->far_x = far_x ;

	chksq->xrange = xrange ;
	chksq->yrange = yrange ;

	chksq->flag = 0 ;
	chksq->checkadd = checkadd;

//printf("PL_DGCam_N_CheckSquares [%d] \n",PL_DGCam_N_CheckSquares);
	PL_DGCam_N_CheckSquares ++ ;
	return 0 ;
}

/* キャラ削除（プログラム呼び） */
int	PL_DeleteDGCameraCheckSquare( FVECTOR *checkadd )
{
	int			i, nsqs ;
	CHK_SQ		*chksq ;
	int			found = 0 ;

delete_check_again :	
	nsqs = PL_DGCam_N_CheckSquares ;
	chksq = CheckSquare ;
	for ( i = 0; i < nsqs; i ++, chksq ++ ) {
		if ( chksq->checkadd == checkadd ) {
			goto delete_checkadd_found ;
		}
	}
	return found ;
delete_checkadd_found :
	for ( ; i < nsqs - 1; i ++, chksq ++ ) {
		*chksq = *( chksq + 1 ) ;
	}
//printf("DELETE PL_DGCam_N_CheckSquares [%d] \n",PL_DGCam_N_CheckSquares);
	PL_DGCam_N_CheckSquares -- ;
	/* まだあるかもしれないので */
	found = 1 ;
	goto delete_check_again ;
	return 0 ;
}

/*------------------------------------------------------------------*/

/* チェックキャラ領域の確保 */

typedef	struct	{
	GV_ACT		actor ;
	CHK_CH		chkch[ MAX_CHK_CHS ] ;
} CHW ;

static	void	DieCHW( CHW *work )
{

}

int	NewMallocCameraCharaWork( void )
{
	CHW			*work ;

	work = ( CHW * )GV_NewActor( GV_ACTOR_PLAYER, sizeof( CHW ) ) ;
	ASSERT( work != NULL ) ;
	GV_SetActor( &work->actor, NULL, DieCHW ) ;
	CheckChara = &work->chkch[ 0 ] ;
	return 0 ;
}

int	NewSetCheckCharaOfCamera( void )
{
	int			nchs ;
	CONTROL		*ctrl ;
	CHK_CH		*chkch ;

	nchs = PL_DGCam_N_CheckCharas ;
	ASSERT( nchs < MAX_CHK_CHS ) ;
	ctrl = GM_SearchWhere( GCL_GetOptionValue( 'n', 0 ) ) ;
	ASSERT( ctrl != NULL ) ;
	ASSERT( ctrl->object != NULL ) ;
	chkch = CheckChara + nchs ;
	chkch->body = ctrl->object ;
	chkch->joint = GCL_GetOptionValue( 'j', 0 ) ;
	ASSERT( chkch->joint >= 0 && 
		    chkch->joint < chkch->body->objs->n_models ) ;
	chkch->proc = GCL_GetOptionValue( 'p', 0 ) ;
	chkch->far_x = ( float )GCL_GetOptionValue( 'f', 0 ) ;
	chkch->flag = 0 ;
	PL_DGCam_N_CheckCharas ++ ;
	return 0 ;
}

/* キャラ登録（プログラム呼び） */
int	PL_SetDGCameraCheckChara( OBJECT *object, int joint, int proc, float far_x )
{
	int			nchs ;
	CHK_CH		*chkch ;

	nchs = PL_DGCam_N_CheckCharas ;
printf("PL_DGCam_N_CheckCharas [%d] \n",PL_DGCam_N_CheckCharas);
	ASSERT( nchs < MAX_CHK_CHS ) ;
	chkch = CheckChara + nchs ;
	chkch->body = object ;
	chkch->joint = joint ;
	ASSERT( chkch->joint >= 0 && 
		    chkch->joint < chkch->body->objs->n_models ) ;
	chkch->proc = proc ;
	chkch->far_x = far_x ;
	chkch->flag = 0 ;
	PL_DGCam_N_CheckCharas ++ ;
	return 0 ;
}

/* キャラ削除（プログラム呼び） */
int	PL_DeleteDGCameraCheckChara( OBJECT *object )
{
	int			i, nchs ;
	CHK_CH		*chkch ;
	int			found = 0 ;
delete_check_again :	
	nchs = PL_DGCam_N_CheckCharas ;
	chkch = CheckChara ;
	for ( i = 0; i < nchs; i ++, chkch ++ ) {
		if ( chkch->body == object ) {
			goto delete_object_found ;
		}
	}
	return found ;
delete_object_found :
	for ( ; i < nchs - 1; i ++, chkch ++ ) {
		*chkch = *( chkch + 1 ) ;
	}
printf("DELETE PL_DGCam_N_CheckCharas [%d] \n",PL_DGCam_N_CheckCharas);
	PL_DGCam_N_CheckCharas -- ;
	/* まだあるかもしれないので */
	found = 1 ;
	goto delete_check_again ;
	return 0 ;
}

/* キャラ削除（シナリオ） */
int	NewDeleteCheckCharaOfCamera( void )
{
	CONTROL		*ctrl ;

	ctrl = GM_SearchWhere( GCL_GetOptionValue( 'n', 0 ) ) ;
	if ( ctrl == NULL ) return 0 ;
	ASSERT( ctrl->object != NULL ) ;
	PL_DeleteDGCameraCheckChara( ctrl->object ) ;
	return 0 ;
}

int	NewSetCameraProc( void )
{
	PL_DGCam_ShutterProc = GCL_GetOptionValue( 'p', 0 ) ;
	return 0 ;
}

/*------------------------------------------------------------------*/

#ifdef DEBUG_MODE
/* デバッグ表示用 */
void	PL_GetCheckWork( CHK_SQ **chksq, CHK_CH	**chkch )
{
	*chksq = CheckSquare ;
	*chkch = CheckChara ;
}
#endif
