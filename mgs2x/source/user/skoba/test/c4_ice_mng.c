//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	冷却エフェクト管理  main.c
	2001/04/05 S.Kobayashi
	$Id: c4_ice_mng.c,v 1.1.1.3 2002/11/19 11:50:24 Yoshizawa1 Exp $
*/

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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"
#include    "etc.h"

#define ICE_DUST    (15638496)
#define ICE_LOST    (6684577)
#define SPRAY_ON_FLAG (0x1)
#define DESTROY_FLAG (0x2)

typedef	struct _work {
	GV_ACT_EX	actor;

	u_int        trigger ;
	FMATRIX      *pWorld;
	FVECTOR      *pShift;
	FVECTOR      shift;
	char         effect_flag[ 2 ];
	void         *effect[ 2 ];
	char         flag;
	int          effect_plus;
	int          ice_eff_count;
	int          name;

	void ( *act )(struct _work *);
} Work;

typedef struct {
	int *num;
	int strcode;
	int alpha;
} SK_TEX_INFO;

enum {
	SK_SOUND_OFF = 0x0 ,
	SK_SOUND_ON  = 0x1 ,
	SK_SOUND_START  = 0x2 ,
};

enum {
	SK_EFFECT_NORMAL  = 0x1,
	SK_EFFECT_ADD     = 0x2,
	SK_EFFECT_KILL_OK = 0x4,
	SK_EFFECT_KILL_NO = 0x8,
	SK_EFFECT_DESTROY = 0x10,
	SK_EFFECT_HOKAN   = 0x20,
	SK_EFFECT_ADD_END  = 0x40,
};

// プロトタイプ
extern  void    *NewC4_Ice( FMATRIX * , FVECTOR * , SK_TEX_INFO * , int * , char * );

static int MsgChack( Work *pWork )
{
	GV_MSG		*msg;
	int      msg_num;
	int          num;
	int         name;

	msg_num = GV_ReceiveMessage( pWork->name , &msg );
	msg += msg_num - 1;
	name = 0;
	while( --msg_num >= 0 ){
		num = msg->message[ 0 ];
		if( num == SPRAY_ON_FLAG ){
			pWork->flag |= SK_EFFECT_ADD;
//			printf("SPRAY MESG Receive!!!!!!!!!!!!!\n");
		}          
		if( num == DESTROY_FLAG ){
			pWork->flag |= SK_EFFECT_DESTROY;
			pWork->flag &= ~SK_EFFECT_ADD;
		}
		msg--;
	}
	return ( 0 );
}
extern void PosBox(FVECTOR	*,float ,SVECTOR *) ;

static void Act( Work *pWork )
{
	FVECTOR fvtmp;
	FVECTOR test ;
	char    sound_flag;


	GV_MatToVec(pWork->pWorld, &test ) ;
//	PosBox( &test ,250.0f ,NULL );
//	printf("ICE X[%f] Y[%f] Z[%f]\n",test.vx,test.vy,test.vz);


	MsgChack( pWork );

	sound_flag = 0;
	if ( pWork->flag & SK_EFFECT_ADD ){
		DG_SetPos( pWork->pWorld );
		DG_RotVector( pWork->pShift , &fvtmp , 1 );
		_sceVu0AddVector( &fvtmp , &fvtmp , ( FVECTOR * )pWork->pWorld->m[ 3 ] );
		// サウンド
		if ( pWork->ice_eff_count == 0 ){
			sound_flag = SK_SOUND_START;
		} else if ( !( pWork->ice_eff_count % 9 ) ){
			sound_flag = SK_SOUND_ON;
		}
		// エフェクト
		pWork->effect_plus = 5; // 6段階
		if ( 1 ){ // pWork->ice_eff_count < DIRECT_TICK( 600 ) ){ // 10秒
			if ( sound_flag & SK_SOUND_START ){
				GM_SeSetMode( SD_I_SPRAY01 , &fvtmp , GM_SEMODE_BOMB );
			}
			if ( sound_flag & SK_SOUND_ON ){
				GM_SeSetMode( SD_I_SPRAY02 , &fvtmp , GM_SEMODE_BOMB );
			}
			pWork->effect_flag[ 0 ] = SK_EFFECT_ADD;
			pWork->effect_flag[ 1 ] = SK_EFFECT_ADD;
			pWork->ice_eff_count++;
		} else if ( pWork->ice_eff_count < DIRECT_TICK( 700 ) ){
			{ 
				pWork->effect_plus = 5;
				if ( ( pWork->ice_eff_count % 5 ) == 0 ){
					if ( sound_flag & SK_SOUND_ON ){
						GM_SeSetMode( SD_I_SPRAY02 , &fvtmp , GM_SEMODE_BOMB );
					}
					pWork->effect_flag[ 0 ] = SK_EFFECT_ADD;
					pWork->effect_flag[ 1 ] = SK_EFFECT_ADD;
				}
			}
			pWork->ice_eff_count++;
		}
	}
	if ( pWork->flag & SK_EFFECT_DESTROY ){
		GV_DestroyChild( pWork ); // 絶縁
		GV_DestroyActor( pWork );
	}
}

static void Die( Work *pWork )
{
	// effect に kill を transe
	pWork->effect_flag[ 0 ] = 0x1;
	pWork->effect_flag[ 1 ] = 0x1;
}

//static int GetResources( Work *pWork , FMATRIX *pWorld , FVECTOR *pShift )
static int GetResources( Work *pWork , int name, int where )
{
	int buf[3] ;
	SK_TEX_INFO tex;
	int	parts ;
	CONTROL	*ctrl ;
	FMATRIX *pWorld ;

	GCL_GetOption( 'p' ) ;
	parts = GCL_GetNextInt()  ;

	GCL_GetOption( 'n' ) ;
	ctrl = GM_SearchWhere( GCL_GetNextInt() ) ;
	pWorld =  &ctrl->object->objs->objs[parts].world ;

	if ( GCL_GetOption( 's' ) ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &pWork->shift ) ;
	}else {
		pWork->shift = DG_ZeroVector;
	}

	pWork->trigger = TRIG_FIRE;
	pWork->ice_eff_count = 0;
	pWork->pWorld = pWorld;
//	pWork->pShift = pShift;
	pWork->pShift = &pWork->shift;
	// 氷Effect
	tex.alpha = 3;
	tex.num = &pWork->effect_plus;
	tex.strcode = ICE_DUST;
	pWork->effect_flag[ 0 ] = SK_EFFECT_NORMAL;
	pWork->effect[ 0 ] = NewC4_Ice( pWorld , pWork->pShift , &tex , &pWork->trigger , &pWork->effect_flag[ 0 ] );
	GV_SetActorChild( pWork , pWork->effect[ 0 ] );
	// 氷Effect2
	tex.alpha = 5;
	tex.num = &pWork->effect_plus;
	tex.strcode = ICE_LOST;
	pWork->effect_flag[ 1 ] = SK_EFFECT_NORMAL;
	pWork->effect[ 1 ] = NewC4_Ice( pWorld , pWork->pShift , &tex , &pWork->trigger , &pWork->effect_flag[ 1 ] );
	GV_SetActorChild( pWork , pWork->effect[ 1 ] );

	return (0);
}

//void *NewIceSpray_mng( FMATRIX *pWorld , FVECTOR *pShift , int name )
void *NewIceSpray_mng( int name , int where  )
{
	Work		*pWork;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) );
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act, Die );
		GV_ActorEX( &pWork->actor );
		pWork->name = name;
		if ( GetResources( pWork, name, where ) < 0 ) {
			GV_DestroyActor( pWork );
			return NULL;
		}
	}
	return (void *)pWork;
}
