//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	campose.c
	カメラ判定管理

	2002/05/16 Y.Korekado
	$Id: campose.c,v 1.1.1.3 2002/11/19 11:44:06 Yoshizawa1 Exp $
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"libutl.h"

#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"enemy.h"


/* ---------------------------------------------------------------- */
#define MAX_EROGECHU (8)
#define MAX_PIYOPIYO (4)
#define MAX_WBIKKURI (8)

#define CPE_ERO_GECHU	0x00000001		/* エロゲッチュー*/ 
#define CPE_PIYOPIYO	0x00000002		/* ぴよぴよ★ */
#define CPE_W_BIKKURI	0x00000004		/* 白びっくり！ */

/* ---------------------------------------------------------------- */

typedef struct {
	GV_ACT		actor ;

	int			joint ;
	int			proc ;
	int			status ;
	float		far_x ;
	int			xrange ;
	int			yrange ;
	COMMANDER	*com ;
} Work	;

/* エロゲッチュー */
typedef struct {
	int set_camcheck ;
} EROGECHU ;
static 	EROGECHU	EroGechu[MAX_EROGECHU] ;

/* ピヨピヨ★ */
typedef struct {
	DG_PRIM2	*prim;
	FMATRIX		*world ;

	FVECTOR piyo1[ 4 ] ;
	FVECTOR piyo2[ 4 ] ;
	FVECTOR piyo3[ 4 ] ;
	int set_camcheck ;
} PIYOPIYO ;
static 	PIYOPIYO	PiyoPiyo[MAX_PIYOPIYO] ;
static int N_Piyopiyo ;

/* 白！ */
#define BIKKURI_TIME	(57)
#define BIKKURI_DISP_TIME	(48)
typedef struct {
	int time ;
	FMATRIX	*head ;
	FVECTOR wbikkuri[ 4 ] ;
	int set_camcheck ;
} WBIKKURI ;
static 	WBIKKURI	WBikkuri[MAX_WBIKKURI] ;

/* ---------------------------------------------------------------- */
extern int	PL_SetDGCameraCheckChara( OBJECT *object, int joint, int proc, float far_x ) ;
extern int	PL_DeleteDGCameraCheckChara( OBJECT *object ) ;
extern int	PL_SetCheckSquareOfCamera( FVECTOR *checkadd, int proc, float far_x, int xrange, int yrange,
				FVECTOR *v1, FVECTOR *v2, FVECTOR *v3, FVECTOR *v4 ) ;
extern int	PL_DeleteDGCameraCheckSquare( FVECTOR *checkadd ) ;

/* ---------------------------------------------------------------- */
void	CPE_CamPoseInit( void )
{
	int i;

	for ( i=0; i<MAX_EROGECHU; i++ ) {
		EroGechu[ i ].set_camcheck = 0 ;
	}

	N_Piyopiyo = 0 ;
	for ( i=0; i<MAX_PIYOPIYO; i++ ) {
		PiyoPiyo[ i ].set_camcheck = 0 ;
	}
}

/* ---------------------------------------------------------------- */
void	CPE_SetCamPoseSquare( DG_PRIM2 *prim, FMATRIX *world )
{
	PIYOPIYO	*piyo ;

	if ( N_Piyopiyo >= MAX_PIYOPIYO ) return ;

	piyo = &PiyoPiyo[N_Piyopiyo] ;
	piyo->prim = prim ;
	piyo->world = world ;

	N_Piyopiyo ++ ;
}

void	CPE_DeletCamPoseSquare( DG_PRIM2 *prim )
{
	int	i ;
	PIYOPIYO	*piyo ;

	piyo = PiyoPiyo ;
	for( i=0; i<MAX_PIYOPIYO; i++, piyo++ ) {
		if ( piyo->prim == prim ) {
			if ( piyo->set_camcheck ) {
				PL_DeleteDGCameraCheckSquare( piyo->piyo1 ) ;
				if ( piyo->set_camcheck > 1 ) {
					PL_DeleteDGCameraCheckSquare( piyo->piyo2 ) ;
				}
				if ( piyo->set_camcheck > 2 ) {
					PL_DeleteDGCameraCheckSquare( piyo->piyo3 ) ;
				}
			}
			for( ; i<MAX_PIYOPIYO-1; i++, piyo++ ) {
				*piyo = *(piyo+1) ;
				(piyo+1)->set_camcheck = 0 ;
			}
			N_Piyopiyo -- ;
			break ;
		}
	}
}

static int SprtPosChange( FMATRIX *world, DG_PRIM2 *prim, FVECTOR *out, int n )
{
	FVECTOR sft[4], *center ;
	FMATRIX mat ;
	float	w, h ;

	center = prim->pos[prim->buffer_clock] + n ;

	if ( DG_FABS( world->m[ 3 ][ 1 ] - center->vy ) > 4000.0 ) return 0 ;

	w = (float)((DG_PRIM2_UVRGBWH *)prim->uvrgb[prim->buffer_clock])->w ;
	h = (float)((DG_PRIM2_UVRGBWH *)prim->uvrgb[prim->buffer_clock])->h ;

	sft[0].vx = -w ; sft[0].vy =  h ; sft[0].vz = 0.0 ; sft[0].vw = 1.0 ;
	sft[3].vx =  w ; sft[3].vy =  h ; sft[1].vz = 0.0 ; sft[1].vw = 1.0 ;
	sft[2].vx =  w ; sft[2].vy = -h ; sft[2].vz = 0.0 ; sft[2].vw = 1.0 ;
	sft[1].vx = -w ; sft[1].vy = -h ; sft[3].vz = 0.0 ; sft[3].vw = 1.0 ;

	mat = DG_Chanls[0].eye ;
	mat.m[ 3 ][ 0 ] = center->vx ;
	mat.m[ 3 ][ 1 ] = center->vy ;
	mat.m[ 3 ][ 2 ] = center->vz ;
	DG_SetPos( &mat ) ;
	DG_PutVector( sft, out, 4 ) ;

	return 1 ;
}
static	void	Piyopiyo( Work *work )
{
	PIYOPIYO	*piyo ;
	int i;

	if ( !(work->status & CPE_PIYOPIYO) ) return ;

	piyo = PiyoPiyo ;
	for( i=0; i<MAX_PIYOPIYO; i++, piyo++ ) {
		if ( piyo->set_camcheck ) {
			PL_DeleteDGCameraCheckSquare( piyo->piyo1 ) ;
			if ( piyo->set_camcheck > 1 ) {
				PL_DeleteDGCameraCheckSquare( piyo->piyo2 ) ;
			}
			if ( piyo->set_camcheck > 2 ) {
				PL_DeleteDGCameraCheckSquare( piyo->piyo3 ) ;
			}

			PiyoPiyo[ i ].set_camcheck = 0 ;
		}
	}

	piyo = PiyoPiyo ;
	for( i=0; i<N_Piyopiyo; i++, piyo++ ) {
		/* １つ目の★ */
		if ( SprtPosChange( piyo->world, piyo->prim, piyo->piyo1, 0 ) ) {
			PL_SetCheckSquareOfCamera( piyo->piyo1, work->proc, work->far_x, work->xrange, work->yrange,
				&piyo->piyo1[0], &piyo->piyo1[1], &piyo->piyo1[2], &piyo->piyo1[3] ) ;
			piyo->set_camcheck = 1 ;
if ( 0 ) {
extern void	AN_Test_Eye3( FVECTOR *mov ) ;
AN_Test_Eye3( &piyo->piyo1[0] );
AN_Test_Eye3( &piyo->piyo1[1] );
AN_Test_Eye3( &piyo->piyo1[2] );
AN_Test_Eye3( &piyo->piyo1[3] );
}
		} else {
			continue ;
		}
		/* ２つ目の★ */
		if ( SprtPosChange( piyo->world, piyo->prim, piyo->piyo2, 1 ) ) {
			PL_SetCheckSquareOfCamera( piyo->piyo2, work->proc, work->far_x, work->xrange, work->yrange,
				&piyo->piyo2[0], &piyo->piyo2[1], &piyo->piyo2[2], &piyo->piyo2[3] ) ;
			piyo->set_camcheck = 2 ;
		} else {
			continue ;
		}
		/* ３つ目の★ */
		if ( SprtPosChange( piyo->world, piyo->prim, piyo->piyo3, 2 ) ) {
			PL_SetCheckSquareOfCamera( piyo->piyo3, work->proc, work->far_x, work->xrange, work->yrange,
				&piyo->piyo3[0], &piyo->piyo3[1], &piyo->piyo3[2], &piyo->piyo3[3] ) ;
			piyo->set_camcheck = 3 ;
		}
	}
}

/* ---------------------------------------------------------------- */
static	void	PoseEnemy( Work *work )
{
	COMMANDER	*com ;
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int g, u, i, num ;

	if ( !(work->status & CPE_ERO_GECHU) ) return ;

	num = -1 ;
	com = work->com ;
	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				num ++;
				ASSERT( num < MAX_EROGECHU ) ;
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk->status & ENE_STATUS_EVER_UNREAL ) continue ;
				if ( entk->act->status_status & ACT_STST_ERO_GETCHU ) {
					if ( !(EroGechu[num].set_camcheck) ) {
						PL_SetDGCameraCheckChara( entk->act->body, work->joint, work->proc, work->far_x ) ;
						EroGechu[num].set_camcheck = 1 ;
					}
				} else {
					if ( EroGechu[num].set_camcheck ) {
						PL_DeleteDGCameraCheckChara( entk->act->body ) ;
						EroGechu[num].set_camcheck = 0 ;
					}
				}
			}
		}
	}
}
/* ---------------------------------------------------------------- */
#define WB_W (80.0)
#define WB_H (220.0)

static FVECTOR SftHead = { 0.0, 440.0, 0.0 } ;
static FVECTOR SftWB[4] = { 
	{ -WB_W, WB_H, 0.0, 1.0 },
	{ -WB_W, -WB_H, 0.0, 1.0 },
	{ WB_W, -WB_H, 0.0, 1.0 },
	{ WB_W, WB_H, 0.0, 1.0 },
 } ;

static void Sprt_WB_PosChange( FMATRIX *world, FVECTOR *out )
{
	FVECTOR center ;
	FMATRIX mat ;

	DG_SetPos( world ) ;
	DG_PutVector( &SftHead, &center, 1 ) ;

	mat = DG_Chanls[0].eye ;
	mat.m[ 3 ][ 0 ] = center.vx ;
	mat.m[ 3 ][ 1 ] = center.vy ;
	mat.m[ 3 ][ 2 ] = center.vz ;
	DG_SetPos( &mat ) ;
	DG_PutVector( SftWB, out, 4 ) ;
}

static	void	WhiteBikkuri( Work *work )
{
	WBIKKURI	*wb ;
	COMMANDER	*com ;
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int g, u, i, num ;

	if ( !(work->status & CPE_W_BIKKURI) ) return ;

	num = -1 ;
	com = work->com ;
	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				num ++;
				ASSERT( num < MAX_WBIKKURI ) ;
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk->status & ENE_STATUS_EVER_UNREAL ) continue ;

				if ( (entk->act->sw->headmark & 0xffff) == 0x8004 ) {
					WBikkuri[ num ].time = BIKKURI_TIME ;
					WBikkuri[ num ].head = &(entk->act->body->objs->objs[HUMAN21_ATAMA].world) ;
				}
			}
		}
	}

	wb = WBikkuri ;
	for( i=0; i<MAX_PIYOPIYO; i++, wb++ ) {
		if ( wb->set_camcheck ) {
			PL_DeleteDGCameraCheckSquare( wb->wbikkuri ) ;
			wb[ i ].set_camcheck = 0 ;
		}
	}

	wb = WBikkuri ;
	for( i=0; i<MAX_PIYOPIYO; i++, wb++ ) {
		if ( (wb->time > 0) && (wb->time < BIKKURI_DISP_TIME) ) {
			Sprt_WB_PosChange( wb->head, wb->wbikkuri ) ;
			PL_SetCheckSquareOfCamera( wb->wbikkuri, work->proc, work->far_x, work->xrange, work->yrange,
				&wb->wbikkuri[0], &wb->wbikkuri[1], &wb->wbikkuri[2], &wb->wbikkuri[3] ) ;
			wb[ i ].set_camcheck = 1 ;
if ( 0 ) {
extern void	AN_Test_Eye3( FVECTOR *mov ) ;
AN_Test_Eye3( &wb->wbikkuri[0] );
AN_Test_Eye3( &wb->wbikkuri[1] );
AN_Test_Eye3( &wb->wbikkuri[2] );
AN_Test_Eye3( &wb->wbikkuri[3] );
}
		}
		if ( --wb->time < 0 ) wb->time = 0 ;
	}
}
/* ---------------------------------------------------------------- */
static	void	Act( Work *work )
{
	PoseEnemy( work ) ;
	Piyopiyo( work ) ;
	WhiteBikkuri( work ) ;
}

static	void	Die( Work *work )
{
}
/* ---------------------------------------------------------------- */
static	int	GetResources( work, name, where )
Work	*work ;
int		name ;
int		where ;
{
	work->status = 0 ;

	work->joint = GCL_GetOptionValue( 'j', 0 ) ;
	work->proc = GCL_GetOptionValue( 'p', 0 ) ;
	work->status = GCL_GetOptionValue( 'c', 0 ) ;
	work->far_x = ( float )GCL_GetOptionValue( 'f', 0 ) ;
	work->xrange = GCL_GetOptionValue( 'x', 640 ) ;
	work->yrange = GCL_GetOptionValue( 'y', 320 ) ;
	work->com = COM_GetCommander() ;

	return 0 ;
}

void		*NewCameraPoseEnemyManager( name, where )
int		name ;
int		where ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_AFTER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0x60 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
