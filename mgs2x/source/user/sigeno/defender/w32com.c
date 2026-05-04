//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	w32com.c
	エマ狙撃イベント兵用コマンダー
	2000/03/02 K.Sigeno
	$Id: w32com.c,v 1.1.1.3 2002/11/19 11:49:11 Yoshizawa1 Exp $
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
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"korekado/enemy/enemy.h"

//#include	"at_thk.h"
#include	"eve_w32.h"
#include	"../../morita/emma/include/emma_com.h"
#define AT_NO_ACTIVE (ACT_STATUS_FAINT|ACT_STATUS_DOWN|ACT_STATUS_CAPTURE)

/*---------------------------------------------------------------*/


typedef	struct	{
	GV_ACT		actor ;
	int			name ;
	E_GROUP		group ;
	E_UNIT		unit[ ATUNIT_MAX ] ;
	CLEARING	clearing[ ATUNIT_MAX ] ;
	COMMANDER	*com ;
	AT_COM		at_com ;
	int			think1 ;
	CONTROL		*t_ctrl ;		/*目標キャラのCONTROL*/
} Work ;



extern int ENE_GetRouteDis( FVECTOR *,FVECTOR *,int,int,u_int);



#ifdef DEBUG_MODE
//#define COM_DEBUG_PRIM
#endif

#ifdef COM_DEBUG_PRIM

#endif

/*---------------------------------------------------------------*/

#define		MSG_LIGHT_BREAK		(1)
#define		MSG_CHANGE_RES_POS	(2)



static void DEF_ComInit(Work *work){
	int i,j ;
//	int addr1,addr2;
	ENETHINK *entk;
//	HZX_GROUP_ID	id1,id2;
//	ENTK_TYPE_A *eve_a ;
	EVENT_W32 *eve_w ;

//	DEF_COM	*def_com ;
//	CONTROL	ctrl ;

//	def_com = &work->def_com ;

	work->at_com.Pl_StayTime = 0; /**/



	for(i=0;i<ATUNIT_MAX;i++){
		for(j=0;j < ATENEMY_MAX;j++){
			if((i<work->group.unit_num)
			&&(j < work->unit[i].enemy_num)){
				/****/
				entk = work->unit[i].entk[j];
				entk->at_com = &work->at_com ;
//				at_thk = (AT_THK *) entk->character;
				eve_w = (EVENT_W32 *) entk->character ;
//				at_thk->dis_rank = entk->id;
				eve_w->dis_rank = entk->id;
			}else {
//				work->at_com.teamaddr[i][j] = NULL;
			}
		}
//		work->at_com.siege[ i ] = 1;
	}
	/* 連絡用 */
	/* 可変 */
}


//extern int ENE_ReadOnlinInfo(int ,int);
static CONTROL* SearchControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		return ( control ) ;
	}

	return ( NULL );
}

#if 0
static void W32DisRankCheck( Work *work )
{
	ENETHINK *entk;
	int i,j,k,dislist[ATENEMY_MAX],idlist[ATENEMY_MAX];
	int tmpdis,tmplen,nearid;
	int	addr1,addr2,trgaddr;
	int ene_num;
	FVECTOR	*pos1,*pos2 ;
	FVECTOR	trgpos;
	EVENT_W32 *eve_w ;
//	AT_THK *at_thk;
	long64 front_st[ATENEMY_MAX] ;
//	SVECTOR	rgb ;


	if(work->t_ctrl != NULL){
		trgpos = work->t_ctrl->mov ;
		trgaddr = work->t_ctrl->addr ;
	}else {
		trgpos = GM_PlayerPosition ;
		trgaddr = GM_PlayerAddress ;
	}
	entk = work->unit[0].entk[0];
	for(k=0;k<work->group.unit_num;k++){
		ene_num = work->unit[k].enemy_num;
		for(i=0;i < ene_num;i++){
			entk = work->unit[k].entk[i];
			eve_w = (EVENT_W32 *) entk->character ;
			/*最後尾フラグクリア*/
ASSERT(entk != NULL);
ASSERT(eve_w != NULL);
//ASSERT(entk->at_com != NULL);
//			at_thk->at_status &= ~AT_ST_LAST ;
			eve_w->zone_dis 
				= ENE_GetRouteDis(&entk->znavi->flore_pos,
				&trgpos,entk->ctrl->addr,trgaddr,200000);
		}
	}
	for(k=0;k<work->group.unit_num;k++){
		/*ソート準備*/
		ene_num = work->unit[k].enemy_num;
		for(i=0;i < ene_num;i++){
			entk = work->unit[k].entk[i];
			/*手前兵の状態チェック用にstセット*/
			front_st[i] = entk->act->status;
			eve_w = (EVENT_W32 *) entk->character ;
			/*目標までの距離*/
			dislist[i] =eve_w->zone_dis;
			idlist[i] =i;
/*行動不能者は順位外*/
/*死亡者は別扱い*/
/*相手が隠れてるときは追い越しなし*/
			if(1){
/*注 この時点ではfront_stには自分のstatusがセットされている*/
				if(
/*イントルード時以外のダメージ兵*/
				(( front_st[i] & AT_NO_ACTIVE ) 
				&&( entk->act->bodyp.life > 0  )
				)
/*眠り兵*/
				||((entk->act->bodyp.anesthesia == -1)&&(entk->act->status & ACT_STATUS_FAINT))
				){
//printf("DIS RANK SHIFT %d id %d\n",entk->think3,entk->id);
					dislist[i] += 5000 ;
				}
			}
		}
		/*プレイヤに近い順 挿入ソート*/
		for(i=1;i<ene_num;i++){
			j = i;
			while ( j >= 1 && dislist[j-1] > dislist[j]) {
				tmpdis = dislist[j]; dislist[j] =dislist[j-1];
				dislist[j-1] = tmpdis;
				tmpdis = idlist[j]; idlist[j] = idlist[j-1];
				idlist[j-1] = tmpdis;
				j--;
			}
		}
		{
			/*順位更新*/
			for(i=0;i<ene_num;i++){
				eve_w = 
				(EVENT_W32 *) work->unit[k].entk[idlist[i]]->character;
				eve_w->dis_rank = i;
			}
		}
		/*味方との距離*/
		for(i=0;i<ene_num;i++){
			if(i==0) {
				/*先頭兵士*/
	eve_w = (EVENT_W32 *) work->unit[k].entk[idlist[i]]->character ;
	eve_w->dis_dif = eve_w->zone_dis ;
	eve_w->front_st = 0 ;
	addr1 = work->unit[k].entk[idlist[i]]->ctrl->addr ;
			}else {
				/*自分より近い兵全てと距離を計算し一番近い物を採用*/
				/*初期値には目標距離を設定*/
				/*プレイヤのほうが近ければそれを採用*/
				eve_w = 
				(EVENT_W32 *) work->unit[k].entk[idlist[i]]->character ;
				eve_w->dis_dif = eve_w->zone_dis ;
				/*自分*/
				pos1 = &work->unit[k].entk[idlist[i]]->znavi->flore_pos ;

				addr1 = work->unit[k].entk[idlist[i]]->ctrl->addr ;
				nearid = 255;
				for(j=(i-1);j>=0;j--){
					/*相手*/
					pos2 = &work->unit[k].entk[idlist[j]]->znavi->flore_pos ;
					addr2 = work->unit[k].entk[idlist[j]]->ctrl->addr ;
					tmplen = ENE_GetRouteDis( pos1, pos2 ,
					addr1,addr2,10000 ) ;

					eve_w = 
					(EVENT_W32 *) work->unit[k].entk[idlist[i]]->character ;
			/*最近兵 更新*/
					if( eve_w->dis_dif > tmplen){
						eve_w->dis_dif = tmplen; 
						nearid = j;
					}
				}
				if(nearid==255) {
					nearid = 0;
				}
				/*手前兵の状態セット*/
				eve_w = 
				(EVENT_W32 *) work->unit[k].entk[idlist[i]]->character ;
				eve_w->front_st = front_st[idlist[nearid]] ;

				if(
				(front_st[idlist[nearid]] & AT_NO_ACTIVE)
				&&( entk->act->bodyp.life > 0  )
				)
				{
					eve_w->dis_dif = 25000; 
				}else {
					/*手前との距離判定*/
					addr1 = work->unit[k].entk[idlist[i]]->ctrl->addr ;
					addr2 = work->unit[k].entk[idlist[nearid]]->ctrl->addr ;
				}
			}
		}
	}
}

#endif

static void Act( Work *work )
{
	if(work->at_com.OldPlAddr != GM_PlayerAddress) {
		work->at_com.Pl_StayTime = 0; 
	}else {
		if(work->at_com.Pl_StayTime < 0xffffffff) {
			work->at_com.Pl_StayTime++;
		}
	}
	work->at_com.OldPlAddr = GM_PlayerAddress ;

}
/*---------------------------------------------------------------*/
static void Die( Work *work )
{

}
/*---------------------------------------------------------------*/

static int AttackerUnitCall( Work *work, char *top )
{
	int i, id;
	char *p ;

//	for( i = 0; (p=GCL_NextStr()) != NULL; i++ ){
	for( i = 0; GCL_NextStr() != NULL; i++ ){
		if ( i >= ATUNIT_MAX ) {
			printf("ATUNIT_MAX OVER!!\n");
			return (-1) ;
		}
		if ( COM_SetUnit( &work->group, &work->unit[i] ) < 0 ) {
			printf("MAX_UNIT OVER!!\n");
			return (-1) ;
		}
		id = GCL_GetNextInt( ) ;
		p = GCL_NextStr() ;
		GCL_ExecProc( id, NULL );
		GCL_SetNextStr( p );
		work->clearing[i].unit = &work->unit[i] ;
		work->clearing[i].hzx = work->com->hzx ;
	}

	return i;
}

static int GetResources( Work *work, int where ,int name)
{
	char	*opt ;
//	int i,buf[3] ;
//	CONTROL		ctrl ;

	work->name = name ;
	work->com = COM_GetCommander( ) ;
	work->at_com.com = COM_GetCommander( ) ;
//	work->com->at_com = &work->at_com ;

	work->think1 = 0 ;

	if ( ( opt = GCL_GetOption( 't' ) ) != NULL ){
		work->t_ctrl = SearchControl( GCL_GetNextInt() ) ; 
	} else {
		work->t_ctrl = NULL ;
	}

	if ( COM_SetGroup( &work->com->enemys, &work->group ) < 0 ) return (-1) ;

	if ( ( opt = GCL_GetOption( 'e' ) ) != NULL ){
		if ( AttackerUnitCall( work, opt ) < 0 ) return (-1) ;
	}

	/*守備地点設定*/

	DEF_ComInit(work);


	return 0;
}

/*---------------------------------------------------------------*/
void *NewW32Commander( name, where )
int	name ;
int	where ;
{
	Work		*work ;
	
	OPERATOR() ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), SUBCOMM_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, where ,name) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/**
	group = com->enemys.group[ 0 ] ;
	for ( i=0; i<group->unit_num; i++ ) {
		unit = group->unit[ i ] ;
		for ( j=0; j<unit->enemy_num; j++ ) {
			entk = unit->entk[ j ] ;
			if ( !(entk->act->status & ACT_STATUS_DAMAGE) ) {
				dis = _FVecTrgDis( &entk->ctrl->mov, &GM_NoisePosition 
printf("GROUP %d\n,work->com->enemys.group_num");
printf("UNIT %d\n",work->com->enemys.group[0]->unit_num);

**/

