//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	冷却エフェクトbox  main.c
	2001/03/23 S.Kobayashi
	$Id: c4_effct.c,v 1.1.1.3 2002/11/19 11:44:05 Yoshizawa1 Exp $
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

#include "libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#define	BODY_FLAG_SHADOW	(DG_FLAG_TEXT|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
#define ICE_WAIT (256)
#define FLUSH_COLD (0x10)

/* state */
#define C4_STATE_LIKE_CEILING	(0x00000001)

/* bul_c4.h */
#define FLAG_LIKE_CEILING (0x10000) /* 自分の高さよりカメラが上になると非表示 */

typedef	struct _work {
	GV_ACT_EX	actor;
	OBJECT		body ;

	FMATRIX   *pWorld;
	FMATRIX   *ltp;
	FMATRIX   lights[2] ;
	FVECTOR       pos;	
	SVECTOR       rot;
	float       color;
	int		      map;
	int			count;
	int           num;
	int          time;
	int			state ;
	DG_OBJS		*c4objs ;
	void ( *act )(struct _work *);
} Work;

static void MemCopy( void *dst, void *src, int size, int num )
{
	extern void UTL_StartSprToMem( void *dst, void *src, int size );
	extern void UTL_EndSprToMem( void );

	UTL_StartSprToMem( dst, src, size * num / sizeof(u_long128) );
	UTL_EndSprToMem() ;
}


static void EffectAct( Work *pWork )
{
	DG_SetPos( pWork->pWorld ) ;

if(0){
	FVECTOR s ;
	s.vx=0 ;
	s.vy=100 ;
	s.vz=100 ;
DG_MovePos( &s ) ;
}
	DG_PutObjs( pWork->body.objs ) ;
	MemCopy( (void *)pWork->lights, (void *)pWork->ltp, sizeof( FMATRIX ), 2 ) ;

{
	float k ;
	
	k = pWork->color / 255.0 ;

	pWork->lights[1].m[0][0] *= k ;
	pWork->lights[1].m[0][1] *= k ;
	pWork->lights[1].m[0][2] *= k ;
	pWork->lights[1].m[1][0] *= k ;
	pWork->lights[1].m[1][1] *= k ;
	pWork->lights[1].m[1][2] *= k ;
	pWork->lights[1].m[2][0] *= k ;
	pWork->lights[1].m[2][1] *= k ;
	pWork->lights[1].m[2][2] *= k ;
}

	pWork->lights[1].m[3][0] = pWork->color ;
	pWork->lights[1].m[3][1] = pWork->color ;
	pWork->lights[1].m[3][2] = pWork->color ;

    pWork->body.objs->group_id = pWork->c4objs->group_id ;

	if( pWork->state & C4_STATE_LIKE_CEILING ) {
		extern int	KR_CeilingCheck( float height ) ;
		pWork->body.objs->flag = KR_CeilingCheck( pWork->pWorld->m[3][1] ) ;
		pWork->body.objs->flag |= DG_FLAG_ONEPIECE ;
	} else {
		if ( pWork->c4objs->flag & DG_FLAG_INVISIBLE ) {
		    pWork->body.objs->flag |= DG_FLAG_INVISIBLE ;
		} else {
		    pWork->body.objs->flag &= ~DG_FLAG_INVISIBLE ;
		}
	}
}

static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork = ( Work * )workp;
	float tmp;
	
	switch ( signal ){
	case 0x2 :
		if ( value < DIRECT_TICK( 1500 ) ){
			tmp = ( 120.f - pWork->color ) / DIRECT_TICK( 150.f );
		} else {
			tmp = ( 255.f - pWork->color ) / DIRECT_TICK( 30.f );
		} 
		pWork->color += tmp;
#if 0
		pWork->comdl->pos[0].color.vw = ( int )pWork->color;
		pWork->comdl->pos[0].color.vw &= 0xff;
#endif
		// サウンド
		if ( !(pWork->time % DIRECT_TICK(16)) ){
			GM_SeSetMode( SD_A_FREEZE01, (FVECTOR *)pWork->pWorld->m[ 3 ] , GM_SEMODE_BOMB ) ;
			pWork->time = 1;
		} else {
			pWork->time++;
		}
		break;
	case 0x4 : // 完全凍結
		pWork->color = 255.0;
		break;
	case FLUSH_COLD : // 瞬殺
printf("koreeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee21\n");
		pWork->color = 255.0;
#if 0
		pWork->comdl->pos[ 0 ].color.vw = ( int )0xff;
#endif
		break;
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return ( 1 );
	}
	return ( 0 );
}

static void Act( Work *pWork )
{
	pWork->act( pWork );
}

static void Die( Work *pWork )
{

    GM_FreeObject( &pWork->body ) ;
}

static int GetResources( Work *pWork , FMATRIX *pWorld , FMATRIX *light, int str_code, int flag, DG_OBJS *objs )
{
	int            i ;

	i = 0;

	pWork->map = GM_CurrentMap;
	pWork->num = 1;                 // 将来をかんがえて
	pWork->time = 0;
	pWork->state = 0;
	pWork->c4objs = objs;
	pWork->ltp = light;
    GM_SetCurrentMap( pWork->map ); // 表示設定

	GV_SetActorSignalFunc( pWork , ReceiveSignal );
	GM_InitObject( &pWork->body, str_code, BODY_FLAG_SHADOW );
	GM_GroupObject( pWork->body.objs , pWork->map );
	pWork->pWorld = pWorld;
	DG_SetPos( pWork->pWorld ) ;
	DG_PutObjs( pWork->body.objs ) ;

	GM_ConfigObjectLight( &pWork->body, (FMATRIX *)pWork->lights );

#if 0
	/* 整数型なので注意！ */
	comdl->pos[i].color.vx = 128;
	comdl->pos[i].color.vy = 128;
	comdl->pos[i].color.vz = 128;
	comdl->pos[i].color.vw = 0;
#endif

	pWork->pos.vx = 0;
	pWork->pos.vy = 0;
	pWork->pos.vz = 0;
	pWork->pos.vw = 0;
	pWork->rot.vx = 0;
	pWork->rot.vy = 0;
	pWork->rot.vz = 0;

	pWork->color = 0.0f;
	pWork->act = (void *)EffectAct;

	if ( flag & FLAG_LIKE_CEILING ) pWork->state |= C4_STATE_LIKE_CEILING ;

	return ( 0 );
}

void *NewIceEffect( FMATRIX *pWorld , FMATRIX *light, int str_code, int flag, DG_OBJS *objs )
{
	Work		*pWork ;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_EFFECT , sizeof( Work ) ) ;
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act, Die ) ;
		GV_ActorEX( &pWork->actor );
		if ( GetResources( pWork, pWorld , light, str_code, flag, objs ) < 0 ) {
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}
