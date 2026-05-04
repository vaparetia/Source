//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bul_c4.c
   セットされたＣ４
   
   1999/12/02 M.Sonoyama
   $Id: demo_c4.c,v 1.1.1.3 2002/11/19 11:50:19 Yoshizawa1 Exp $
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

#ifdef PSX2
#include	"../../kano/attachment/attachment_called.h"
#endif

/*------------------------------------------------------------*/

extern	void	*NewBlast2( FMATRIX *, int, int, int, int, int, int ) ;
extern  void    *NewC4_Lamp( FMATRIX * , int , char * , char * , int );
extern  void    *NewIceComodel( FMATRIX * , int );

#define	BODY_NAME	(104754)	/* cfr */
#define BODY_NAME_SCN_1		(9723355)	/* c4_kaitai_a1 */
#define BODY_NAME_SCN_2		(9723387)	/* c4_kaitai_b1 */

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | \
					 DG_FLAG_ONEPIECE | DG_FLAG_IRREACTION)
#define	DELAY_BLAST	(8)
#define	COLD_TIME	(180)	/* ３秒（６０フレーム時） */

#define		TYPE_NONE		C4_TYPE_NONE
#define		TYPE_SEGMENT	C4_TYPE_SEGMENT
#define		TYPE_FLOOR		C4_TYPE_FLOOR
#define		TYPE_CHARA		C4_TYPE_CHARA

#define SK_COLD_START (0x1)
#define VISIBLE_FLAG    ( 0x2 )
#define INVISIBLE_FLAG  ( 0x4 )

enum {
	SK_NONE = 0x0 , 
	SK_FLAG_COLD_START = 0x1 ,
	SK_FREEZE          = 0x2 ,
};

enum {
	LAMP_STATE_FREEZING	= 		0x01,		/* 凍らされ中 */
	LAMP_STATE_SE_ENABLE =		0x02,		/* ランプＳＥ鳴らす */
	LAMP_SE_TYPE0 =				0x00,
	LAMP_SE_TYPE1 =				0x04,
	LAMP_SE_TYPE2 =				0x08,
	LAMP_SE_ALL =				0x0c,
} ;

enum {
    FLAG_PLAYER = 		0x0001,
    FLAG_SCENARIO = 	0x0002,
	FLAG_KAITAI =		0x0004,
    FLAG_FREEZE =		0x0010,
    FLAG_DESTROY =		0x0020,
    FLAG_FREE =			0x0040,
	FLAG_NO_SMELL =		0x0080,
	FLAG_SMELL =		0x0100,
	FLAG_SWITCH =		0x0200,	
	FLAG_FALL =			0x0400,	/* その場に落ちる */
	FLAG_COUNTDOWN =	0x0800,
	FLAG_FATMAN =		0x1000,	/* ファットマンがしかける奴 */
	FLAG_NO_SPRAY =		0x2000,	/* スプレーが効かない */
} ;

#define	LAMP_BLINK_COUNT	(15)					/* ランプエフェクトの点滅間隔 */
#define	LAMP_COLOR_MAX		(LAMP_BLINK_COUNT-1)	/* ランプエフェクトの明るさ最大 */


typedef struct _Work {
    GV_ACT_EX		actor ;
    OBJECT			body ;
    FMATRIX			world ;
	FMATRIX			lights[ 2 ] ;
	FVECTOR			shift ;
	RADAR_CTRL		radar ;
    TARGET			target ;
	void			*stick ;
	int				model ;
    int				type ;
    int				side ;
    int				map ;
    int				flag ;
    int				delay ;
	int				cold_count ;
	int				id ;
	int				n_incremental ;
	int				freeze_disp ;
	int				disp_x, disp_y ;
	int				proc ;
	int				name ;
	int				enemy_joint ;
	int				blast_count ;
    struct _Work	*next ;
	ENEFIND 		ef ;
	GM_BOMB			this ;
	HZX_D_CALLBACK	dhcb ;
	char			lamp_status ;
	char			lamp_count ;
} Work ;

/*----------------------------------------------------------------*/
/* プロック実行 */
static	void	ExecProc( Work *work )
{
	int			buf[ 2 ] ;
	GCL_ARGS	args ;

	if ( work->proc == 0 ) return ;

	args.argc = 2 ;
	args.argv = buf ;

	buf[ 0 ] = work->name ;
	buf[ 1 ] = 0;
	GM_ForceExecProc( work->proc, &args ) ;
}

/*------------------------------------------------------------*/


static void DumyHit( Work *pWork ) // ダミーのヒット関数
{
	if ( !( pWork->flag & SK_FLAG_COLD_START ) ){
		return;
	}
	pWork->lamp_status |= LAMP_STATE_FREEZING ;
	GV_CallChildSignalFunc( pWork , 0x02 , pWork->cold_count );
	if ( pWork->cold_count < 0 ) {
		pWork->body.objs->flag &= ~DG_FLAG_IRREACTION ;
	    GV_CallChildSignalFunc( pWork , 0x04 , pWork->cold_count );
		ExecProc( pWork ) ;
		pWork->flag |= SK_FREEZE;
		pWork->flag &= ~SK_FLAG_COLD_START;
	} else {
		pWork->cold_count--;
	}
    GV_CallChildSignalFunc( pWork , 0x08 , pWork->cold_count );
}

/* メッセージチェック */
static	void	CheckMessage( Work *pWork )
{
	int			n_msg ;
	GV_MSG		*msg ;

	n_msg = GV_ReceiveMessage( pWork->name, &msg ) ;
	while( -- n_msg >= 0 ) {
		if ( msg->message[ 0 ] == SK_COLD_START ) {	// 凍るまでの時間
			pWork->flag |= SK_FLAG_COLD_START;
			pWork->cold_count = msg->message[ 1 ];
		}
		if( msg->message[ 0 ] == VISIBLE_FLAG ){
			// default invisible
			DG_VisibleObjs( pWork->body.objs );
		}
		if( msg->message[ 0 ] == INVISIBLE_FLAG ){
			DG_InvisibleObjs( pWork->body.objs );
		}
		msg ++ ;
	}
}

/*------------------------------------------------------------*/

static	void	Act( Work *pWork )
{
	/* 解体用はメッセージチェック */
	CheckMessage( pWork ) ;
	DumyHit( pWork );
	GM_SetCurrentMap( pWork->map ) ;
}

static	void	Die( pWork )
Work		*pWork ;
{
	if ( pWork->type != TYPE_CHARA ) {
		HZX_RemoveDynamicCallback( pWork->stick, &pWork->dhcb ) ;
		GM_FreeObject( &pWork->body ) ;
	}
}

/*------------------------------------------------------------*/
static	int	GetResourcesKaitai( Work *work, int name, int where, 
							    FMATRIX *world, int count, int scn )
{
	FVECTOR		mov ;
	SVECTOR		rot ;
	int			model, bodyflag ;

	work->name = name ;
	work->map = where ;

	if ( scn == 1 ) {
		PL_GetOptionFV( 'p', &mov ) ;
		PL_GetOptionSV( 'r', &rot ) ;
		DG_SetPos2( &mov, &rot ) ;
	} else {
		DG_SetPos( world ) ;
	}


	DG_GetPos( &work->world ) ;
    work->side = BOTH_SIDE ;
    work->type = TYPE_NONE ;
    work->flag = SK_NONE ;

	// ランプの追加
	if ( scn ) {
		GV_SetActorChild( work , NewC4_Lamp( &work->world , where , NULL , NULL , name ));
	} else {
		GV_SetActorChild( work, NewC4_Lamp( &work->world, where, 
										    &work->lamp_status, &work->lamp_count , name ) ) ;
		work->lamp_status = 0 ;
		work->lamp_count = LAMP_COLOR_MAX ;
	}

	if ( scn == 1 ) {
		if ( GCL_GetOption( 't' ) != NULL ) {
			if ( GCL_GetNextInt() != 0 ) {
				work->flag |= FLAG_NO_SMELL ;
			} else {
				work->flag |= FLAG_SMELL ;
			}
		} else {
			work->flag |= FLAG_SMELL ;
		}
	} else {
		work->flag |= FLAG_SMELL ;
	}

	if ( work->flag & FLAG_SMELL ) {
		/* においつき */
		model = BODY_NAME_SCN_1 ;
		GV_SetActorChild( work , NewIceComodel( &work->world , GV_StrCode("c4_kaitai_a1_frost_cm") ) );
	} else {
		/* においなし */
		model = BODY_NAME_SCN_2 ;
		GV_SetActorChild( work , NewIceComodel( &work->world , GV_StrCode("c4_kaitai_b1_frost_cm") ) );
	}
	
	work->model = model ;
	work->map = where ;
	work->id = -1 ;

	GV_SetVec3( &work->shift, 0.0F, 0.0F, 0.0F ) ;
	GM_SetCurrentMap( where ) ;
	bodyflag = BODY_FLAG ;
	work->proc = GCL_GetOptionValue( 'e', 0 ) ;

	work->type = TYPE_NONE ;

	{	/* 壁床タイプ */
		FVECTOR		from = { 0.0F, 0.0F, 50.0F } ;
		FVECTOR		to =   { 0.0F, 0.0F, -150.0F } ;

		GM_InitObject( &( work->body ), model, bodyflag ) ;
		DG_COPY_MAT( &work->body.objs->world, &work->world ) ;
		GM_ConfigObjectLight( &work->body, work->lights ) ;
		DG_GetLightMatrix( ( FVECTOR * )&work->body.objs->world.m[ 3 ], work->lights ) ;
		DG_SetPos( &work->world ) ;
		DG_PutVector( &from, &from, 1 ) ;
		DG_PutVector( &to, &to, 1 ) ;		
		work->stick = NULL ;
		work->id = -1 ;
	}
	work->blast_count = count ;

	// default invisible
	DG_InvisibleObjs( work->body.objs );

    return 0 ;
}

/* シナリオ起動 */
void	*NewDEMOSetKaitaiC4( int name, int where )
{
	Work		*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResourcesKaitai( work, name, where, NULL, -1, 1 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
