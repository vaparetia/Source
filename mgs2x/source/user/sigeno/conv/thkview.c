//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkview.c
	敵兵思考 可視可
	2000/04/24 K.Sigeno
	$Id: thkview.c,v 1.1.1.3 2002/11/19 11:49:08 Yoshizawa1 Exp $
*/

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
#include <float.h>

#include "gameheader.h"
#include "libutl.h"
#include	"korekado/enemy/enemy.h"
#include "../attacker/at_thk.h"


#define TRI_SIZE (500.0F)

typedef	struct {
	GV_ACT		actor ;
	ENETHINK	*entk;
	int			count ;
	int			mode ;
} Work ;



//extern void *NewTriangleView(FVECTOR * ,int,u_char,u_char,u_char);
//extern void *NewBoundingBoxView(FVECTOR *,FVECTOR *,u_char,u_char,u_char);
//extern void *NewSquareView( FVECTOR *,int,u_char,u_char,u_char);
extern void SigPosView( FVECTOR * ,SVECTOR *,float );
extern void SigZoneView(int,SVECTOR *,float) ; 

//extern void *NewLineView(FVECTOR * ,int,u_char,u_char,u_char) ;
extern void ENE_InitRouteNavi( ROUTENAVI *,int,int);

#define BOX_WIDE	(50.0F)
static void Sig_ColSqu( FVECTOR *pos ,SVECTOR *rgb){
	FVECTOR ver[4];
	int i ;

	for(i=0;i<4;i++){
		ver[i] = *pos ;
	}
	ver[0].vx -= BOX_WIDE ;
	ver[0].vz -= BOX_WIDE ;

	ver[1].vx += BOX_WIDE ;
	ver[1].vz -= BOX_WIDE ;

	ver[2].vx += BOX_WIDE ;
	ver[2].vz += BOX_WIDE ;

	ver[3].vx -= BOX_WIDE ;
	ver[3].vz += BOX_WIDE ;

	NewSquareView( ver,1,rgb->vx,rgb->vy,rgb->vz);

}
#if 0
static void Sig_ColBox( ENETHINK *entk ,SVECTOR *rgb,float hight){
	AT_THK *at_thk;
	u_char r,g,b;
	FVECTOR	pos[2];
	int i;

	at_thk = (AT_THK *) entk->character ;
	
	for(i=0;i<2;i++){
		pos[i] = entk->ctrl->mov;
	}
	pos[0].vx -= BOX_WIDE ;
	pos[0].vz -= BOX_WIDE ;
	pos[1].vx += BOX_WIDE ;
	pos[1].vz += BOX_WIDE ;

	pos[1].vy += hight ;

	r = rgb->vx;
	g = rgb->vy;
	b = rgb->vz;

	NewBoundingBoxView(&pos[0],&pos[1],r,g,b);

}
#endif

static void RedSet(SVECTOR *rgb){
	rgb->vx = 255 ;
	rgb->vy = 0 ;
	rgb->vz = 0 ;
}
static void BlueSet(SVECTOR *rgb){
	rgb->vx = 0 ;
	rgb->vy = 0 ;
	rgb->vz = 255 ;
}
static void YellowSet(SVECTOR *rgb){
	rgb->vx = 255 ;
	rgb->vy = 255 ;
	rgb->vz = 0 ;
}
static void GreenSet(SVECTOR *rgb){
	rgb->vx = 0 ;
	rgb->vy = 255 ;
	rgb->vz = 0 ;
}
#if 0
	rnavi->p_action = rnavi->pa_action[(int)rnavi->next_node];
	rnavi->p_acttime = rnavi->pa_time[(int)rnavi->next_node] ;
	rnavi->p_dir = rnavi->pa_dir[(int)rnavi->next_node];
	rnavi->p_con = rnavi->pa_con[(int)rnavi->next_node];
	rnavi->p_actstatus = rnavi->pa_flag[(int)rnavi->next_node];
#endif


static void Sig_ColTri( ENETHINK *entk ,SVECTOR *rgb)
{
	FVECTOR pos[3];
	int i;


	for(i=0;i<3;i++){
		pos[i] = entk->ctrl->mov;
		pos[i].vy += TRI_SIZE ;
		switch (i){
			case 0 :
				pos[i].vz -= TRI_SIZE ;
				pos[i].vy += TRI_SIZE ;
				break;
			case 1 :
				pos[i].vx += TRI_SIZE ;
				pos[i].vz += TRI_SIZE ;
				break;
			case 2 :
				pos[i].vx -= TRI_SIZE ;
				pos[i].vz += TRI_SIZE ;
				break;
		}
	}
	NewTriangleView(pos,1,rgb->vx,rgb->vy,rgb->vz);
}

static void RankView(ENETHINK *entk ){
	SVECTOR	rgb;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	switch (at_thk->dis_rank){
		case 0 :
			RedSet(&rgb);
			break;
		case 1 :
			YellowSet(&rgb);
			break;
		case 2 :
			GreenSet(&rgb);
			break;
		case 3 :
			BlueSet(&rgb);
			break;
		default :
			rgb.vx = 0 ;
			rgb.vy = 0 ;
			rgb.vz = 0 ;
	}
	Sig_ColTri( entk,&rgb );

}
void SIG_RouteView(ROUTENAVI *rnavi){
extern void SIG_NumPrint(FVECTOR * ,int ) ;

//	FVECTOR		line[MAX_ROOT_NODE] ;
	FVECTOR		line[2] ;
	FVECTOR		v_pos;
	int i;
	SVECTOR	rgb;

	if(rnavi == NULL ) return ;
	
	rgb.vx = 255 ;
	rgb.vy = 0 ;
	rgb.vz = 0 ;
	for( i=0;i<rnavi->n_nodes;i++ ){
		v_pos = rnavi->nodes[i];
		v_pos.vy += 50.0F ;
		Sig_ColSqu( &v_pos ,&rgb);
		SIG_NumPrint(&v_pos ,rnavi->pa_action[i]) ;
//		PosBox(&rnavi->nodes[i],50.0F,&rgb);
		if(
		(rnavi->n_nodes > 1 )
		&&(i < (rnavi->n_nodes - 1))
		){
			line[0] =rnavi->nodes[i];
			line[1] =rnavi->nodes[i+1];
			line[0].vy +=50.0F ;
			line[1].vy +=50.0F ;
			NewLineView( line ,1,0,0,255) ;
		}
	}
}


static void SightView(Work *work){

	SVECTOR rgb ;
	switch (work->entk->pl_eyei.sight){
		case EYE_INFO_SIGHT_IN :
			RedSet(&rgb);
			break;
		case EYE_INFO_SIGHT_BLURR :
			YellowSet(&rgb);
			break;
		case EYE_INFO_SIGHT_OUT_HZD :
			GreenSet(&rgb);
			break;
		case EYE_INFO_SIGHT_OUT :
			BlueSet(&rgb);
			break;
		default :
			rgb.vx = 0 ;
			rgb.vy = 0 ;
			rgb.vz = 0 ;
	}
	Sig_ColTri( work->entk,&rgb );
}

/*プレイヤからの見られ時間*/
static void PlSightView(Work *work){
	int		col;
	SVECTOR rgb ;
	AT_THK *at_thk;
	
	at_thk = (AT_THK *) work->entk->character ;
	
	col = at_thk->sight_time ;
	if(col> 255) col = 255 ;
	if(col< 0) col = 0 ;

	rgb.vx = col;
	rgb.vy = 0;
	rgb.vz = 255-col ;
	Sig_ColTri( work->entk,&rgb );
}
static void Die(void){
}
enum {
	RANK,
	SIGHT,
	PL_SIGHT,
	POS_IN_ZONE,
	ZONE_IN_ZONE,
	FIND_POS,
	TRG_ZONE
};

static void Act(Work *work ){
	int viewmode = RANK ;
	SVECTOR rgb ;
	AT_THK *at_thk;

	at_thk = (AT_THK *) work->entk->character ;
	switch(viewmode){
		case RANK :
			RankView(work->entk);
			break;
		case SIGHT :
			SightView(work);
			break;
		case PL_SIGHT :
			PlSightView(work);
			break;
		case POS_IN_ZONE :
			RedSet(&rgb);
			SigPosView( &work->entk->com->plpos_in_zone[0],
				&rgb,100.0F);
			break;
		case ZONE_IN_ZONE :
			BlueSet(&rgb);
			SigZoneView(work->entk->com->plzone_in_zone[0],
				&rgb,100.0F) ; 
			break;
		case FIND_POS :
			YellowSet(&rgb);
			SigPosView( &GM_PlayerFindPos,&rgb,100.0F);
			break;
//		case TRG_ZONE :
//
//			BlueSet(&rgb);
//			SigZoneView(work->entk->trgpoint.addr,
//				&rgb,100.0F) ; 
//			break;
	}
}
static int GetResources( work,entk , mode )
Work *work;
ENETHINK *entk ;
int		mode ;
{
	work->entk  = entk ;
	work->mode  = mode; 
	work->count = 0;
	return 1;
}
void *NewSigThkView( entk , mode )
ENETHINK *entk ;
int		mode ;
{
	Work *work ;

	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
		   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		if ( GetResources( work,entk,mode ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
