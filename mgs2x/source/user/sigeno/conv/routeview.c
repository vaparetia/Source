//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
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


typedef	struct _Work {
	GV_ACT	actor ;
	int routeviewmode ;
	int routeviewnum ;
	int routeview_pt ;
	int count ;
} Work ;
#define PRINT_X (64)
#define PRINT_Y (64)
#define PRINT_H (16)
#ifdef DEBUG_MODE
extern void SIG_NumPrint(FVECTOR *  ,int ) ;
extern void SIG_PosLine(FVECTOR * ) ;
extern void SIG_NumPrintformat(FVECTOR * ,int,int) ;
#endif

enum {
	view_num = 0,/*ポイント番号*/
	view_act , 	/* アクション番号 */
	view_time,		/* 継続時間 */
	view_dir,		/* 方向 ( 0 ～ 4095 ) */
	view_flag		/* 巡回ポイントフラグ */
};
enum {
	pat_mode = 0,
	cle_mode,
	off_mode
} ;
static void PatRouteView(HZX_PAT *patrols,int mode,int num){
#ifdef DEBUG_MODE
	FVECTOR		line[2] ;
	FVECTOR		v_pos;
	int i;
	SVECTOR	rgb;

	if(patrols == NULL ) return ;
	
	rgb.vx = 255 ;
	rgb.vy = 0 ;
	rgb.vz = 0 ;
//	for( i=0;i<patrols->n_points;i++ ){
	i = num ;
	{
		v_pos.vx = patrols->points[i].x;
		v_pos.vy = patrols->points[i].y;
		v_pos.vz = patrols->points[i].z;
		v_pos.vy += 50.0F ;
		SIG_PosLine( &v_pos );
		switch(mode){
			case view_num :
				SIG_NumPrint(&v_pos ,i) ;
				break;
			case view_act :
				SIG_NumPrint(&v_pos ,patrols->points[i].act) ;
				break;
			case view_time :
				SIG_NumPrint(&v_pos ,patrols->points[i].time) ;
				break;
			case view_dir :
				SIG_NumPrint(&v_pos ,patrols->points[i].dir) ;
				break;
			case view_flag :
				SIG_NumPrintformat(&v_pos ,patrols->points[i].flag,1) ;
				break;
		}
		if(
		(patrols->n_points > 1 )
		&&(i < (patrols->n_points - 1))
		){

			line[0].vx = patrols->points[i].x ;
			line[0].vy = patrols->points[i].y ;
			line[0].vz = patrols->points[i].z ;

			line[1].vx = patrols->points[i+1].x ;
			line[1].vy = patrols->points[i+1].y ;
			line[1].vz = patrols->points[i+1].z ;

			line[0].vy +=50.0F ;
			line[1].vy +=50.0F ;
			NewLineView( line ,1,0,0,255) ;
		}
	}
#endif
}
void ALL_PatRouteView(HZX_PAT *patrols,int mode){
#ifdef DEBUG_MODE
	FVECTOR		line[2] ;
	FVECTOR		v_pos;
	int i;
	SVECTOR	rgb;

	if(patrols == NULL ) return ;
	
	rgb.vx = 255 ;
	rgb.vy = 0 ;
	rgb.vz = 0 ;
	for( i=0;i<patrols->n_points;i++ ){
		v_pos.vx = patrols->points[i].x;
		v_pos.vy = patrols->points[i].y;
		v_pos.vz = patrols->points[i].z;
		v_pos.vy += 50.0F ;
		SIG_PosLine( &v_pos );
		switch(mode){
			case view_num :
				SIG_NumPrint(&v_pos ,i) ;
				break;
			case view_act :
				SIG_NumPrint(&v_pos ,patrols->points[i].act) ;
				break;
			case view_time :
				SIG_NumPrint(&v_pos ,patrols->points[i].time) ;
				break;
			case view_dir :
				SIG_NumPrint(&v_pos ,patrols->points[i].dir) ;
				break;
			case view_flag :
				SIG_NumPrintformat(&v_pos ,patrols->points[i].flag,1) ;
				break;
		}
		if(
		(patrols->n_points > 1 )
		&&(i < (patrols->n_points - 1))
		){

			line[0].vx = patrols->points[i].x ;
			line[0].vy = patrols->points[i].y ;
			line[0].vz = patrols->points[i].z ;

			line[1].vx = patrols->points[i+1].x ;
			line[1].vy = patrols->points[i+1].y ;
			line[1].vz = patrols->points[i+1].z ;

			line[0].vy +=50.0F ;
			line[1].vy +=50.0F ;
			NewLineView( line ,1,0,0,255) ;
		}
	}
#endif
}

static void CleRouteView(HZX_CLE_ROOT *patrols,int mode,int num){
#ifdef DEBUG_MODE
	FVECTOR		line[2] ;
	FVECTOR		v_pos;
	int i;
	SVECTOR	rgb;

	if(patrols == NULL ) return ;
	
	rgb.vx = 255 ;
	rgb.vy = 0 ;
	rgb.vz = 0 ;
//	for( i=0;i<patrols->n_points;i++ ){
	i = num ;
	{
		v_pos.vx = patrols->points[i].x;
		v_pos.vy = patrols->points[i].y;
		v_pos.vz = patrols->points[i].z;
		v_pos.vy += 50.0F ;
		SIG_PosLine( &v_pos );
		switch(mode){
			case view_num :
				SIG_NumPrint(&v_pos ,i) ;
				break;
			case view_act :
				SIG_NumPrint(&v_pos ,patrols->points[i].act) ;
				break;
			case view_time :
				SIG_NumPrint(&v_pos ,patrols->points[i].time) ;
				break;
			case view_dir :
				SIG_NumPrint(&v_pos ,patrols->points[i].dir) ;
				break;
			case view_flag :
				SIG_NumPrintformat(&v_pos ,patrols->points[i].pad,1) ;
				break;
		}
		if(
		(patrols->n_points > 1 )
		&&(i < (patrols->n_points - 1))
		){

//			line[0] =rnavi->nodes[i];
//			line[1] =rnavi->nodes[i+1];
			line[0].vx = patrols->points[i].x ;
			line[0].vy = patrols->points[i].y ;
			line[0].vz = patrols->points[i].z ;

			line[1].vx = patrols->points[i+1].x ;
			line[1].vy = patrols->points[i+1].y ;
			line[1].vz = patrols->points[i+1].z ;

			line[0].vy +=50.0F ;
			line[1].vy +=50.0F ;
			NewLineView( line ,1,0,0,255) ;
		}
	}
#endif
}


static void Die(void){
}
#define NUM_SELECT PAD_R1
#define MODE_SELECT PAD_L1
#define POINT_SELECT PAD_R2

static void Act(Work *work ){
#ifdef DEBUG_MODE
	HZX_HDL *hdl ;
	int i=0 ;
	hdl = HZX_GetCurrentHzx();
	work->count++ ;
	if ( GM_Debug2PMode == GM_DEBUG_MODE_PATROLS) {
	/*パッド受け付け*/
		if(GV_PadData[ 1 ].press & NUM_SELECT ){
			/*番号*/
			work->routeviewnum++;
		}
		if(GV_PadData[ 1 ].press & MODE_SELECT ){
			/*モード*/
			work->routeviewmode++;
		}
		if(GV_PadData[ 1 ].press & POINT_SELECT ){
			/*ポイント情報*/
			work->routeview_pt++;
		}
	/*パラメータチェック*/
		if((work->routeviewmode <pat_mode)||(off_mode < work->routeviewmode )){
			work->routeviewmode = pat_mode ;
		}
		if(work->routeview_pt > view_flag){
			work->routeview_pt = 0 ;
		}
	/*文字表示*/
	    DEBUG_Locate(PRINT_X,PRINT_Y+PRINT_H*i,0) ;
	    DEBUG_Color(200,200,200,0x80);
		switch (work->routeviewmode) {
			case pat_mode :
				if(hdl->def->n_patrols <= work->routeviewnum) {
					work->routeviewnum = 0;
				}
				DEBUG_Printf("PATROL VIEW MODE \n") ;
				i++;
			    DEBUG_Locate(PRINT_X,PRINT_Y+PRINT_H*i,0) ;
			    DEBUG_Printf("ALL ROUTE NUM %d\n",hdl->def->n_patrols) ;
				i++;
			break;
			case cle_mode :
				if(hdl->def->n_clears <= work->routeviewnum) {
					work->routeviewnum = 0;
				}
			    DEBUG_Printf("CLEARING VIEW MODE \n") ;
				i++;
			    DEBUG_Locate(PRINT_X,PRINT_Y+PRINT_H*i,0) ;
			    DEBUG_Printf("ALL ROUTE NUM %d\n",hdl->def->n_clears) ;
				i++;
			break;
			case off_mode :
				DEBUG_Printf("ROUTE VIEW OFF \n") ;
				return ;
			break;
		}
	    DEBUG_Printf("NOW VIEW No %d\n",work->routeviewnum) ;
		i++;
		DEBUG_Locate(PRINT_X,PRINT_Y+PRINT_H*i,0) ;
		switch(work->routeview_pt){
			case view_num :
			    DEBUG_Printf("POINT NUM VIEW\n") ;
			break ;
			case view_act :
				DEBUG_Printf("POINT ACT VIEW\n") ;
				break ;
			case view_time :
				DEBUG_Printf("POINT TIME VIEW\n") ;
				break ;
			case view_dir :
				DEBUG_Printf("POINT DIR VIEW\n") ;
			break ;
			case view_flag :
			    DEBUG_Printf("POINT FLAG VIEW\n") ;
			break ;
		}
	}
	if(work->routeviewmode == off_mode){
		return ;
	}
	/*ライン描画*/
	switch (work->routeviewmode) {
		case pat_mode :
			if((hdl->def->n_patrols > 0)&&(work->routeviewnum < hdl->def->n_patrols)) {
				if(hdl->def->patrols[work->routeviewnum].n_points >0){
					GM_CurrentMap = GM_CurrentStageMap ;
					work->count %= hdl->def->patrols[work->routeviewnum].n_points ;
					PatRouteView(&hdl->def->patrols[work->routeviewnum],work->routeview_pt,work->count) ;
				}
			}
		break;
		case cle_mode :
			if((hdl->def->n_clears >0)&&(work->routeviewnum<hdl->def->n_clears)) {
				if(hdl->def->cle_roots[work->routeviewnum].n_points >0){
					GM_CurrentMap = GM_CurrentStageMap ;
					work->count %= hdl->def->cle_roots[work->routeviewnum].n_points;
					CleRouteView(&hdl->def->cle_roots[work->routeviewnum],work->routeview_pt,work->count) ;
				}
			}
		break;
	}
#endif
}

void *NewDebugRouteViewer( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_CHARA, 
		   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		work->routeviewmode = off_mode;
		work->routeviewnum = 0;
		work->routeview_pt = 0;
		work->count = 0;
	}
	return (void *)work ;
}



