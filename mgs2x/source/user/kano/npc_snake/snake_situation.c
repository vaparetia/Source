//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	snake_situation.c
		NPCスネークメインルーチン(w14a,w17a,w32a,w44a,w45a)

	2001/04/21 K.Kano
	$Id: snake_situation.c,v 1.1.1.3 2002/11/19 11:43:23 Yoshizawa1 Exp $
 */


#include "npc_snake.h"


#define SEARCH_DIS					((int)CVM2N(20.0f))
#define SEARCH_RANGE				((int)(60.0f/360.0f*4096.0f))
#define SEARCH_RANGE_AHEAD			((int)(70.0f/360.0f*4096.0f))
#define SEARCH_RANGE_BEHIND			((int)(100.0f/360.0f*4096.0f))
#define SEARCH_RANGE_PEEPING		((int)(60.0f/360.0f*4096.0f))

#define SEARCH_ALLRANGE				0x1000
#define SEARCH_UDRANGE				((int)(50.0f/360.0f*4096.0f))

#define SEARCH_DEG_AHEAD			0x0800
#define SEARCH_DEG_BEHIND			0x0000
#define SEARCH_DEG_PEEPRIGHT		0x0a00
#define SEARCH_DEG_PEEPLEFT			0x0600

#define SHOOT_INTERVAL_COUNT(work)		(DIRECT_TICK(shoot_interval_count[(work)->support_level]))


/* 狙う位置を決める、割合 */
#define SHOOT_POINT_RATE(work,index)	(shoot_rate[(work)->support_level2][(index)])
#define SHOOT_POINT_RATE2(work,index)	(shoot_rate[8+(work)->support_level/2][(index)])


/* ライデンとスネークの間の距離を判定 */
#define LINE_OVER_RAIDEN			CVM2N(1.0f)
#define LINE_OVER_RAIDEN0			CVM2N(0.5f)
#define LINE0_BEHIND_RAIDEN			CVM2N(0.0f)
#define LINE1_BEHIND_RAIDEN			CVM2N(-2.0f)
#define LINE2_BEHIND_RAIDEN			CVM2N(-3.0f)
#define LINE3_BEHIND_RAIDEN			CVM2N(-5.0f)


/* ライデンの周囲をチェックする際にチェックする範囲の指定 */
#define CHECK_ENEMY_DIS_RAIDEN_AHEAD		(CVM2N(5.0f)*CVM2N(5.0f))
#define CHECK_ENEMY_DIS_RAIDEN_BEHIND		(CVM2N(12.0f)*CVM2N(12.0f))


#define SHOOT_RATE(c)		((unsigned int)((c)*ULONG_MAX))

static const unsigned int shoot_rate[NPC_SNAKE_SUPPORT_LEVEL_MAX*2][4]={
	/* ATAMA,MUNE,ASHI,UDE2 ( OTHER ) */
	/* WEEK */

#if 0
	/* 10%, 40%, 20%, 20%, (10%) */
	{ SHOOT_RATE(0.10f),SHOOT_RATE(0.40f),SHOOT_RATE(0.20f),SHOOT_RATE(0.20f), },
	/* 17%, 45%, 15%, 15%, ( 8%) */
	{ SHOOT_RATE(0.17f),SHOOT_RATE(0.45f),SHOOT_RATE(0.15f),SHOOT_RATE(0.15f), },
	/* 25%, 45%, 12%, 12%, ( 6%) */
	{ SHOOT_RATE(0.25f),SHOOT_RATE(0.45f),SHOOT_RATE(0.12f),SHOOT_RATE(0.12f), },
	/* 26%, 50%, 10%, 10%, ( 4%) */
	{ SHOOT_RATE(0.26f),SHOOT_RATE(0.50f),SHOOT_RATE(0.10f),SHOOT_RATE(0.10f), },

	/* 34%, 50%,  7%,  7%, ( 2%) */
	{ SHOOT_RATE(0.34f),SHOOT_RATE(0.50f),SHOOT_RATE(0.07f),SHOOT_RATE(0.07f), },
	/* 40%, 50%,  5%,  5%, ( 0%) */
	{ SHOOT_RATE(0.40f),SHOOT_RATE(0.50f),SHOOT_RATE(0.05f),SHOOT_RATE(0.05f), },
	/* 50%, 45%, 2.5%,2.5%, ( 0%) */
	{ SHOOT_RATE(0.50f),SHOOT_RATE(0.45f),SHOOT_RATE(0.025f),SHOOT_RATE(0.025f), },
	/* 60%, 35%, 2.5%,2.5%, ( 0%) */
	{ SHOOT_RATE(0.60f),SHOOT_RATE(0.35f),SHOOT_RATE(0.025f),SHOOT_RATE(0.025f), },
#elif 0
	/* 5%, 20%, 20%, 20%, (35%) */
	{ SHOOT_RATE(0.05f),SHOOT_RATE(0.20f),SHOOT_RATE(0.20f),SHOOT_RATE(0.20f), },
	/* 8.5%, 22.5%, 17%, 17%, (35%) */
	{ SHOOT_RATE(0.085f),SHOOT_RATE(0.225f),SHOOT_RATE(0.17f),SHOOT_RATE(0.17f), },
	/* 12.5%, 22.5%, 15%, 15%, (35%) */
	{ SHOOT_RATE(0.125f),SHOOT_RATE(0.225f),SHOOT_RATE(0.15f),SHOOT_RATE(0.15f), },
	/* 13%, 25%, 13.5%, 13.5%, (35%) */
	{ SHOOT_RATE(0.13f),SHOOT_RATE(0.25f),SHOOT_RATE(0.135f),SHOOT_RATE(0.135f), },

	/* 17%, 25%, 11.5%, 11.5%, (35%) */
	{ SHOOT_RATE(0.17f),SHOOT_RATE(0.25f),SHOOT_RATE(0.115f),SHOOT_RATE(0.115f), },
	/* 20%, 25%,  10%,  10%, (35%) */
	{ SHOOT_RATE(0.20f),SHOOT_RATE(0.25f),SHOOT_RATE(0.10f),SHOOT_RATE(0.10f), },
	/* 25%, 22.5%, 8.75%,8.75%, (35%) */
	{ SHOOT_RATE(0.25f),SHOOT_RATE(0.225f),SHOOT_RATE(0.875f),SHOOT_RATE(0.875f), },
	/* 30%, 17.5%, 8.75%,8.75%, (35%) */
	{ SHOOT_RATE(0.30f),SHOOT_RATE(0.175f),SHOOT_RATE(0.875f),SHOOT_RATE(0.875f), },
#else
	/* 0.05%, 0.1%, 1.0%, 1.0%, (97.9%) */
	{ SHOOT_RATE(0.0005f),SHOOT_RATE(0.001f),SHOOT_RATE(0.01f),SHOOT_RATE(0.01f), },
	/* 0.2%, 0.5%, 1.5%, 1.5%, (95.8%) */
	{ SHOOT_RATE(0.002f),SHOOT_RATE(0.005f),SHOOT_RATE(0.015f),SHOOT_RATE(0.015f), },
	/* 0.5%, 0.8%, 1.5%, 1.5%, (95.7%) */
	{ SHOOT_RATE(0.005f),SHOOT_RATE(0.008f),SHOOT_RATE(0.015f),SHOOT_RATE(0.015f), },
	/* 0.8%, 1.2%, 2.0%, 2.0%, (94.0%) */
	{ SHOOT_RATE(0.008f),SHOOT_RATE(0.012f),SHOOT_RATE(0.02f),SHOOT_RATE(0.02f), },

	/* 1.0%, 1.5%, 5.0%, 5.0%, (87.5%) */
	{ SHOOT_RATE(0.01f),SHOOT_RATE(0.015f),SHOOT_RATE(0.05f),SHOOT_RATE(0.05f), },
	/* 1.2%, 4.0%, 10.0%, 10.0%, (74.8%) */
	{ SHOOT_RATE(0.012f),SHOOT_RATE(0.04f),SHOOT_RATE(0.10f),SHOOT_RATE(0.10f), },
	/* 1.5%, 10.0%, 20.0%, 20.0%, (48.5%) */
	{ SHOOT_RATE(0.015f),SHOOT_RATE(0.10f),SHOOT_RATE(0.20f),SHOOT_RATE(0.20f), },
	/* 2.0%, 15.0%, 25.0%, 25.0%, (33%) */
	{ SHOOT_RATE(0.02f),SHOOT_RATE(0.15f),SHOOT_RATE(0.25f),SHOOT_RATE(0.25f), },

#if 1
	/* 10%, 40%, 20%, 20%, (10%) */
	{ SHOOT_RATE(0.10f),SHOOT_RATE(0.40f),SHOOT_RATE(0.20f),SHOOT_RATE(0.20f), },
	/* 17%, 45%, 15%, 15%, ( 8%) */
	{ SHOOT_RATE(0.17f),SHOOT_RATE(0.45f),SHOOT_RATE(0.15f),SHOOT_RATE(0.15f), },
	/* 25%, 45%, 12%, 12%, ( 6%) */
	{ SHOOT_RATE(0.25f),SHOOT_RATE(0.45f),SHOOT_RATE(0.12f),SHOOT_RATE(0.12f), },
	/* 26%, 50%, 10%, 10%, ( 4%) */
	{ SHOOT_RATE(0.26f),SHOOT_RATE(0.50f),SHOOT_RATE(0.10f),SHOOT_RATE(0.10f), },

	/* 34%, 50%,  7%,  7%, ( 2%) */
	{ SHOOT_RATE(0.34f),SHOOT_RATE(0.50f),SHOOT_RATE(0.07f),SHOOT_RATE(0.07f), },
	/* 40%, 50%,  5%,  5%, ( 0%) */
	{ SHOOT_RATE(0.40f),SHOOT_RATE(0.50f),SHOOT_RATE(0.05f),SHOOT_RATE(0.05f), },
	/* 50%, 45%, 2.5%,2.5%, ( 0%) */
	{ SHOOT_RATE(0.50f),SHOOT_RATE(0.45f),SHOOT_RATE(0.025f),SHOOT_RATE(0.025f), },
	/* 60%, 35%, 2.5%,2.5%, ( 0%) */
	{ SHOOT_RATE(0.60f),SHOOT_RATE(0.35f),SHOOT_RATE(0.025f),SHOOT_RATE(0.025f), },
#endif

#endif

 	/* STRONG */
};

static const int shoot_point_index[4]={
	HUMAN21_ATAMA,HUMAN21_MUNE,HUMAN21_HIDARI_ASHI1,HUMAN21_MIGI_UDE2,
//	HUMAN21_ATAMA,HUMAN21_MIGI_ASHI1,HUMAN21_HIDARI_ASHI1,HUMAN21_MIGI_UDE2,
//	HUMAN21_ATAMA,HUMAN21_HIDARI_UDE1,HUMAN21_HIDARI_ASHI2,HUMAN21_MIGI_UDE2,
};


static const int shoot_interval_count[NPC_SNAKE_SUPPORT_LEVEL_MAX]={
	/* WEEK */
	/*DIRECT_TICK*/((int)(60.0f*1.0f)),
	/*DIRECT_TICK*/((int)(60.0f*1.0f)),
	/*DIRECT_TICK*/((int)(60.0f*0.8f)),
	/*DIRECT_TICK*/((int)(60.0f*0.8f)),
	/*DIRECT_TICK*/((int)(60.0f*0.6f)),
	/*DIRECT_TICK*/((int)(60.0f*0.6f)),
	/*DIRECT_TICK*/((int)(60.0f*0.4f)),
	/*DIRECT_TICK*/((int)(60.0f*0.4f)),
 	/* STRONG */
};


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

static void GetSupportPos(Work *work)
{
	NPCSNAKE_SUPPORT_POINT *support_points=work->support_points;
	int n_support_points=work->n_support_points;

	float len;
	float minlen_1=FLT_MAX,minlen_2=FLT_MAX;
	int support_index_1=work->support_pos_index,support_index_2=work->support_pos_index;
	int i;

	FVECTOR *pos;


	/* スネークが次にいるべき位置を決定 */
	for(i=0;i<n_support_points;i++){
		if(support_points[i].posflag & (POS_FLAG_HIDEPOS|POS_FLAG_SITTING_HIDEPOS)){
			continue;
		}

		len=support_points[i].pos.vz-work->ctrl.mov.vz;

		if(len<-CVM2N(1.0f)){
			len=fpu_Abs(len)+fpu_Abs(support_points[i].pos.vx-work->ctrl.mov.vx);

			if(len<minlen_1){
				minlen_2=minlen_1;
				support_index_2=support_index_1;

				minlen_1=len;
				support_index_1=i;
			}
			else if(len<minlen_2){
				minlen_2=len;
				support_index_2=i;
			}
		}
	}

	minlen_1=fpu_Abs(GM_PlayerPosition.vx-support_points[support_index_1].pos.vx);
	minlen_2=fpu_Abs(GM_PlayerPosition.vx-support_points[support_index_2].pos.vx);

	if(minlen_1<minlen_2){
		work->support_pos_index=support_index_2;
		pos=&(support_points[support_index_2].pos);
	}
	else{
		work->support_pos_index=support_index_1;
		pos=&(support_points[support_index_1].pos);
	}

	work->support_pos=pos;


	minlen_1=FLT_MAX;
	support_index_1=-1;

	/* 隠れポイントも同時に算出 */
	for(i=0;i<n_support_points;i++){
		if(!(support_points[i].posflag & (POS_FLAG_HIDEPOS|POS_FLAG_SITTING_HIDEPOS))){
			continue;
		}

		len=fpu_Abs(support_points[i].pos.vz-pos->vz)+fpu_Abs(support_points[i].pos.vx-pos->vx);

		if(len<minlen_1){
			minlen_1=len;
			support_index_1=i;
		}
	}

	work->hide_pos_index=support_index_1;
	work->hide_pos=&(support_points[work->hide_pos_index].pos);
	work->hide_dir=0;
	work->hide_posflag=support_points[work->hide_pos_index].posflag;
}

static void GetHidePos(Work *work)
{
	NPCSNAKE_SUPPORT_POINT *support_points=work->support_points;
	int n_support_points=work->n_support_points;

	float len;
	float minlen_1;
	int support_index_1;
	int i;

	minlen_1=FLT_MAX;
	support_index_1=-1;

	/* 隠れポイントも同時に算出 */
	for(i=0;i<n_support_points;i++){
		if(!(support_points[i].posflag & (POS_FLAG_HIDEPOS|POS_FLAG_SITTING_HIDEPOS))){
			continue;
		}

		len=fpu_Abs(support_points[i].pos.vx-GM_PlayerPosition.vx);
		if(len<CVM2N(2.0f)) continue;

		len=fpu_Abs(support_points[i].pos.vz-GM_PlayerPosition.vz);
		if(len<minlen_1){
			minlen_1=len;
			support_index_1=i;
		}
	}

	work->hide_pos_index=support_index_1;
	work->hide_pos=&(support_points[work->hide_pos_index].pos);
	work->hide_dir=0;
	work->hide_posflag=support_points[work->hide_pos_index].posflag;
}

static inline void GetDefaultNeighborPos(Work *work)
{
	FVECTOR pos;
	int hzx_addr;

	fpu_CopyVector(&pos,&(GM_PlayerPosition));
	pos.vx=work->ctrl.mov.vx;
	pos.vz+=CVM2N(1.5f);			/* ライデンから1.5m後方へ */

	hzx_addr=HZX_GetAddress(work->ctrl.hzx_id,&pos,work->ctrl.addr);

	if(HZX_Zone2(hzx_addr)!=0xff){
		fpu_CopyVector(&(work->neighbor_zone),&pos);
		return;
	}

	pos.vz-=CVM2N(1.5f);			/* ライデンと同じライン */

	hzx_addr=HZX_GetAddress(work->ctrl.hzx_id,&pos,work->ctrl.addr);

	if(HZX_Zone2(hzx_addr)!=0xff){
		fpu_CopyVector(&(work->neighbor_zone),&pos);
		return;
	}

	fpu_CopyVector(&(work->neighbor_zone),&(work->ctrl.mov));
}


#define POS_CHECK_HEIGHT	CVC2N(30.0f)		/* 30cm */
#define POS_SHIFT			CVC2N(30.0f)		/* 30cm */


/* ライデンのわずか後方で８の字(無限記号形)を描かせる */
static void GetInfinityShapePos(Work *work)
{
	static const FVECTOR infinity_shape[INFINITY_MOVE_SIZE]={

#if 0
		{ 30000.0f, 0.0f,CVM2N(1.0f), 0.0f, },
		{ 30000.0f, 0.0f,CVM2N(3.0f), 0.0f, },
		{ 34250.0f, 0.0f,CVM2N(2.5f), 0.0f, },
		{ 38500.0f, 0.0f,CVM2N(1.0f), 0.0f, },
		{ 38500.0f, 0.0f,CVM2N(3.0f), 0.0f, },
#elif 0
		{ 30000.0f, 0.0f,CVM2N(1.0f), 0.0f, },
		{ 34250.0f, 0.0f,CVM2N(1.0f), 0.0f, },
		{ 38500.0f, 0.0f,CVM2N(2.0f), 0.0f, },

		{ 34250.0f, 0.0f,CVM2N(2.5f), 0.0f, },
		{ 30000.0f, 0.0f,CVM2N(3.0f), 0.0f, },
		{ 34250.0f, 0.0f,CVM2N(3.5f), 0.0f, },

		{ 38500.0f, 0.0f,CVM2N(3.0f), 0.0f, },
		{ 34250.0f, 0.0f,CVM2N(2.5f), 0.0f, },
		{ 30000.0f, 0.0f,CVM2N(2.0f), 0.0f, },

		{ 34250.0f, 0.0f,CVM2N(1.0f), 0.0f, },
		{ 38500.0f, 0.0f,CVM2N(1.0f), 0.0f, },
		{ 34250.0f, 0.0f,CVM2N(2.0f), 0.0f, },
#else
		{ 30000.0f, 0.0f,CVM2N(1.0f), 0.0f, },
		{ 38500.0f, 0.0f,CVM2N(2.0f), 0.0f, },

		{ 34250.0f, 0.0f,CVM2N(3.5f), 0.0f, },
		{ 30000.0f, 0.0f,CVM2N(3.0f), 0.0f, },

		{ 34250.0f, 0.0f,CVM2N(2.0f), 0.0f, },
		{ 38500.0f, 0.0f,CVM2N(3.0f), 0.0f, },

		{ 34250.0f, 0.0f,CVM2N(3.5f), 0.0f, },
		{ 30000.0f, 0.0f,CVM2N(2.0f), 0.0f, },

		{ 38500.0f, 0.0f,CVM2N(1.0f), 0.0f, },
		{ 34250.0f, 0.0f,CVM2N(2.5f), 0.0f, },
#endif

	};
	FVECTOR pos;
	int hzx_addr;
	int i=work->i_infinity;
	int cnt=INFINITY_MOVE_SIZE;
	float height=work->ctrl.levels[0];

#if 0
	fpu_CopyVector(&pos,&(infinity_shape[i]));

	pos.vy=GM_PlayerPosition.vy;
	pos.vz+=GM_PlayerPosition.vz;

	hzx_addr=HZX_GetAddress(work->ctrl.hzx_id,&pos,work->ctrl.addr);

	if(HZX_Zone2(hzx_addr)!=0xff){
		fpu_CopyVector(&(work->neighbor_zone),&pos);
		return;
	}

	pos.vz+=CVM2N(1.0f);

	hzx_addr=HZX_GetAddress(work->ctrl.hzx_id,&pos,work->ctrl.addr);

	if(HZX_Zone2(hzx_addr)!=0xff){
		fpu_CopyVector(&(work->neighbor_zone),&pos);
		return;
	}

	pos.vz+=CVM2N(2.0f);

	hzx_addr=HZX_GetAddress(work->ctrl.hzx_id,&pos,work->ctrl.addr);

	if(HZX_Zone2(hzx_addr)!=0xff){
		fpu_CopyVector(&(work->neighbor_zone),&pos);
		return;
	}
#else
	while(cnt>0){
		HZX_ZON *zon;

		fpu_CopyVector(&pos,&(infinity_shape[i]));

		pos.vy=GM_PlayerPosition.vy;
		pos.vz+=GM_PlayerPosition.vz;

		hzx_addr=HZX_GetAddress(work->ctrl.hzx_id,&pos,work->ctrl.addr);

		if(HZX_Zone2(hzx_addr)!=0xff){
			zon=HZX_GetZone(work->ctrl.hzx_id,HZX_Zone1(hzx_addr));
			if(height-POS_CHECK_HEIGHT<zon->y && zon->y<height+POS_CHECK_HEIGHT){

				/* 2001/9/17 K.Kano 追加 */
				/* ZがZONEの際であればずらす */
				if(zon->z-zon->h+POS_SHIFT>pos.vz){
					pos.vz+=POS_SHIFT;
				}
				else if(zon->z+zon->h-POS_SHIFT<pos.vz){
					pos.vz-=POS_SHIFT;
				}

				fpu_CopyVector(&(work->neighbor_zone),&pos);
				work->i_infinity=i;
				return;
			}
		}

		pos.vz+=CVM2N(1.0f);

		hzx_addr=HZX_GetAddress(work->ctrl.hzx_id,&pos,work->ctrl.addr);

		if(HZX_Zone2(hzx_addr)!=0xff){
			zon=HZX_GetZone(work->ctrl.hzx_id,HZX_Zone1(hzx_addr));
			if(height-POS_CHECK_HEIGHT<zon->y && zon->y<height+POS_CHECK_HEIGHT){

				/* 2001/9/17 K.Kano 追加 */
				/* ZがZONEの際であればずらす */
				if(zon->z-zon->h+POS_SHIFT>pos.vz){
					pos.vz+=POS_SHIFT;
				}
				else if(zon->z+zon->h-POS_SHIFT<pos.vz){
					pos.vz-=POS_SHIFT;
				}

				fpu_CopyVector(&(work->neighbor_zone),&pos);
				work->i_infinity=i;
				return;
			}
		}

		pos.vz+=CVM2N(2.0f);

		hzx_addr=HZX_GetAddress(work->ctrl.hzx_id,&pos,work->ctrl.addr);

		if(HZX_Zone2(hzx_addr)!=0xff){
			zon=HZX_GetZone(work->ctrl.hzx_id,HZX_Zone1(hzx_addr));
			if(height-POS_CHECK_HEIGHT<zon->y && zon->y<height+POS_CHECK_HEIGHT){

				/* 2001/9/17 K.Kano 追加 */
				/* ZがZONEの際であればずらす */
				if(zon->z-zon->h+POS_SHIFT>pos.vz){
					pos.vz+=POS_SHIFT;
				}
				else if(zon->z+zon->h-POS_SHIFT<pos.vz){
					pos.vz-=POS_SHIFT;
				}

				fpu_CopyVector(&(work->neighbor_zone),&pos);
				work->i_infinity=i;
				return;
			}
		}

		i++;
		if(i>=INFINITY_MOVE_SIZE) i=0;
		cnt--;
	}
#endif

	fpu_CopyVector(&(work->neighbor_zone),&(work->ctrl.mov));
}


#if 0

static void GetNeighborHzx(Work *work)
{
	HZX_ZONE_ADD zone=work->ctrl.addr;
	HZX_ZON *link;
	int sign;
	float len,maxlen=-FLT_MAX;
	int no;
	int i;

	/* ランダム要素を入れるため追加 */
	int skip;

	if(work->homing==NULL){
		GetDefaultNeighborPos(work);
		return;
	}

	link=HZX_GetZoneFromAdd(zone);
	no=HZX_ZoneMapNo(zone);

	sign=(work->homing_p->vx>work->ctrl.mov.vx);
	skip=irnd() & 1;

	for(i=0;i<5;i++){
		HZX_ZON *zon;
		int sign2;

		if(link->nears[i]==0xff) continue;
		if(link->near_flag[i] & HZX_ROOT_JUMP) continue;

		if(i==skip) continue;

		zon=HZX_GetZoneNo(no,link->nears[i]);
		sign2=(zon->x>work->ctrl.mov.vx);

		if(!(sign ^ sign2)){
			len=fpu_Abs(zon->x-work->ctrl.mov.vx);
			if(len>maxlen){
				maxlen=len;
				work->neighbor_zone.vx=zon->x;
				work->neighbor_zone.vy=work->ctrl.mov.vy;
				work->neighbor_zone.vz=zon->z;
			}
		}
	}

	if(maxlen<0.0f){
		GetDefaultNeighborPos(work);
	}
}

#endif


#define SIN45			0.707106781f
#define COS45			0.707106781f

#define COS30			0.866025404f

#define SIN60			0.866025404f
#define COS60			0.5f

#define DIS_LIMIT1		CVM2N(1.0f)
#define DIS_LIMIT2		CVM2N(2.0f)


static inline int CrashZone(Work *work)
{
	float dis,cos;

	vu0_Ldv0(&(work->ctrl.mov));
	vu0_Ldv1(&(GM_PlayerPosition));
	vu0_Ldv2((FVECTOR *)&(GM_PlayerBody->objs->world.m[2][0]));

	vu0_Subv0v1();

	cos=vu0_InnerProductv0v2();
	dis=vu0_VectorLength2v0();

	if(dis<DIS_LIMIT1*DIS_LIMIT1){
		return 1;
	}
	else if(dis<DIS_LIMIT2*DIS_LIMIT2){
		if(fpu_Rsqrt(dis,cos)>COS30){
			return 1;
		}
	}
	return 0;
}


#define DIS			CVM2N(1.0f)
#define DIS2		CVM2N(1.5f)


static void GetAvoidPos(Work *work)
{
	FVECTOR *x=(FVECTOR *)&(GM_PlayerBody->objs->world.m[0][0]);
	FVECTOR *z=(FVECTOR *)&(GM_PlayerBody->objs->world.m[2][0]);
	FVECTOR tv45[4];
	float dis[4];
	int index[4];
	int i;


	if(CrashZone(work)){
		work->snake_status2|=SNAKE_STATUS2_CRASH_ZONE;
	}
	else{
		work->snake_status2&=~SNAKE_STATUS2_CRASH_ZONE;
	}

	vu0_Ldv1(x);
	vu0_Ldv0(z);

	vu0_Mulv2v1a(-1.0f);
	vu0_Mulv0a(COS60);
	vu0_Mulv1a(SIN60);
	vu0_Mulv2a(SIN60);

	vu0_Addv1v0();
	vu0_Addv2v0();

	vu0_Stv1(&(tv45[0]));
	vu0_Stv2(&(tv45[1]));

	vu0_Ldv1(x);
	vu0_Ldv0(z);

	vu0_Mulv2v1a(-1.0f);
	vu0_Mulv0a(-COS45);
	vu0_Mulv1a(SIN45);
	vu0_Mulv2a(SIN45);


	vu0_Addv1v0();
	vu0_Addv2v0();

	vu0_Stv1(&(tv45[2]));
	vu0_Stv2(&(tv45[3]));

	vu0_Ldv1(&(tv45[0]));
	vu0_Ldv2(&(tv45[1]));
	vu0_Ldv0(&(GM_PlayerPosition));

	vu0_Mulv1a(DIS);
	vu0_Mulv2a(DIS);

	vu0_Addv1v0();
	vu0_Addv2v0();

	vu0_Stv1(&(tv45[0]));
	vu0_Stv2(&(tv45[1]));

	vu0_Ldv1(&(tv45[2]));
	vu0_Ldv2(&(tv45[3]));

	vu0_Mulv1a(DIS2);
	vu0_Mulv2a(DIS2);

	vu0_Addv1v0();
	vu0_Addv2v0();

	vu0_Ldv0(&(work->ctrl.mov));
	vu0_Stv1(&(tv45[2]));
	vu0_Stv2(&(tv45[3]));

	vu0_Subv1v0();
	vu0_Subv2v0();

	dis[2]=vu0_VectorLength2v1();
	dis[3]=vu0_VectorLength2v2();

	vu0_Ldv1(&(tv45[2]));
	vu0_Ldv2(&(tv45[3]));

	vu0_Subv1v0();
	vu0_Subv2v0();

	dis[0]=vu0_VectorLength2v1();
	dis[1]=vu0_VectorLength2v2();

	for(i=0;i<4;i++) index[i]=i;
	for(i=0;i<4;i++){
		int j;
		for(j=i+1;j<4;j++){
			if(dis[index[i]]>dis[index[j]]){
				int t=index[i];
				index[i]=index[j];
				index[j]=t;
			}
		}
	}

	for(i=0;i<4;i++){
		int hzx_addr;

		hzx_addr=HZX_GetAddress(work->ctrl.hzx_id,&(tv45[index[i]]),work->ctrl.addr);

		if(HZX_Zone2(hzx_addr)==0xff) continue;

		if(HZX_NearHazardCheck(work->ctrl.hzx_id,&(tv45[index[i]]),
							   CVM2N(0.5f), /* 検出半径 */
							   HZX_CHK_ALL,0,
							   CVM2N(0.5f) /* 反発半径 */)) continue;

		fpu_CopyVector(&(work->neighbor_zone),&(tv45[index[i]]));
		return;
	}
}


int NPCSnake_SearchPart(Work *work)
{
	unsigned int r=irnd();
	unsigned int d=0;
	int i;

	for(i=0;i<4;i++){
		d+=SHOOT_POINT_RATE(work,i);
		if(r<d) return shoot_point_index[i];
	}

	return -1 /* (irnd()>>16) & 0x0f */ ;
}

int NPCSnake_SearchPart2(Work *work)
{
	unsigned int r=irnd();
	unsigned int d=0;
	int i;

	for(i=0;i<4;i++){
		d+=SHOOT_POINT_RATE2(work,i);
		if(r<d) return shoot_point_index[i];
	}

	return -1 /* (irnd()>>16) & 0x0f */ ;
}


/* 狙いのずれの計算 */
#if 0
// 5度
#define SINQ		0.043619387f
#define COSQ		0.999048222f
#elif 1
// 2度
#define SINQ		0.034899497f
#define COSQ		0.999390827f
#else
// 10度
#define SINQ		0.087155743f
#define COSQ		0.996194698f
#endif

void NPCSnake_CalcTarget(Work *work)
{
	FVECTOR *p=(FVECTOR *)&(work->body.objs->objs[HUMAN21_ATAMA].world.m[3][0]);
	static const FVECTOR iy={ 0.0f,1.0f,0.0f,0.0f, };
	float r1,r2;
	FVECTOR q,v;
	FMATRIX m;

	vu0_Ldv0(work->homing_p);
	vu0_Ldv1(p);
	vu0_Ldv2(&iy);

	vu0_Subv0v1();
	vu0_OuterProductv2v0();
	r1=vu0_VectorLength2v0();
	r2=vu0_VectorLength2v2();

	vu0_Stv0(&v);

	r1=fpu_Rsqrt(r1,1.0f);
	r2=fpu_Rsqrt(r2,1.0f);

	vu0_Mulv0a(r1);
	vu0_Mulv2a(r2);

	vu0_Stv0(&q);

	/* 回転をランダムで算出 */
	r1=frnd();
	r2=fpu_Sqrt(1.0f-r1*r1);

	q.vx*=r1;
	q.vy*=r1;
	q.vz*=r1;
	q.vw=r2;

	MT_QuatToMat(&m,&q);

	vu0_Ldm0(&m);
	vu0_Mulv2m0v2();

	vu0_Stv2(&q);

	q.vx*=SINQ;
	q.vy*=SINQ;
	q.vz*=SINQ;
	q.vw=COSQ;

	MT_QuatToMat(&m,&q);

	vu0_Ldv0(&v);
	vu0_Ldm0(&m);
	vu0_Mulv0m0v0();
	vu0_Addv0v1();
	vu0_Setv0w1();

#if 0
	printf("B -- %f %f %f %f\n",
		   work->homing_p->vx,work->homing_p->vy,work->homing_p->vz,work->homing_p->vw);
#endif

	vu0_Stv0(work->homing_p);

#if 0
	printf("A -- %f %f %f %f\n",
		   work->homing_p->vx,work->homing_p->vy,work->homing_p->vz,work->homing_p->vw);
#endif

}

static inline void MatToVec( w, v )
FMATRIX	*w ;
FVECTOR	*v ;
{
	v->vx = w->m[3][0] ;
	v->vy = w->m[3][1] ;
	v->vz = w->m[3][2] ;
}

static	int		FVecLen3( vec )
FVECTOR		*vec ;
{
	FVECTOR		tmp ;

	tmp.vx = vec->vx * vec->vx ;
	tmp.vy = vec->vy * vec->vy ;
	tmp.vz = vec->vz * vec->vz ;
	return (int)fpu_Sqrt( tmp.vx + tmp.vy + tmp.vz ) ;
}

static HOMING_TRG *GM_GetHomingTrgPart( pos, dir, length, range, ud_range,
										part_num, hzx_id, pre_homing, cmp_homing )
FVECTOR	*pos ;
int		dir ;
int		length ;
int		range ;
int		ud_range ;
int		part_num ;
HZX_GROUP_ID hzx_id;
HOMING_TRG *pre_homing;
HOMING_TRG *cmp_homing;
{
	HOMING_TRG	*hom, *cur, *res ;
	FVECTOR	vec, sub_vec ;
	int		dis, min_dis, t_dir, sub_dir ;

	min_dis = length + 100 ;

	hom = GM_GetHoming() ;
	res = NULL ;

	/* 以前の敵より近い敵に照準するが、あんまりシビアに行うと照準をころころ変えるという
	   現象が起き、見栄えが悪いので少々手をいれてある。 */
	if(cmp_homing!=pre_homing && cmp_homing!=NULL){

#if 0
		if((!(cmp_homing->status & HOMING_SKIP) ||
			(cmp_homing->status & HOMING_TNG_FAINT)) &&
		   !(cmp_homing->status & HOMING_NPC)){

			MatToVec( &(cmp_homing->body->objs->objs[part_num].world), &vec ) ;
			GV_SubVec3F( &vec, pos, &sub_vec ) ;
			dis = FVecLen3( &sub_vec ) ;

			if((cmp_homing->status & HOMING_TNG_FAINT) && dis<CVM2N(2.0f)) ;
			else if(dis<min_dis){
				if(dis<CVM2N(3.0f)){
					min_dis=dis-CVM2N(0.3f);
					res=cmp_homing;
				}
				else{
					min_dis=dis-CVM2N(2.0f);
					res=cmp_homing;
				}
			}
		}
#else
		/* 気絶中の天狗兵攻撃仕様は無し */

		if(!(cmp_homing->status & HOMING_SKIP) &&
		   !(cmp_homing->status & HOMING_NPC)){

			MatToVec( &(cmp_homing->body->objs->objs[part_num].world), &vec ) ;
			GV_SubVec3F( &vec, pos, &sub_vec ) ;
			dis = FVecLen3( &sub_vec ) ;

			if(dis<min_dis){
				if(dis<CVM2N(3.0f)){
					min_dis=dis-CVM2N(0.3f);
					res=cmp_homing;
				}
				else{
					min_dis=dis-CVM2N(2.0f);
					res=cmp_homing;
				}
			}
		}
#endif

	}

	while( hom != NULL ) {
		cur = hom ;
		hom = hom->next ;

#if 0
		if(((cur->status & HOMING_SKIP) &&
			!(cur->status & HOMING_TNG_FAINT)) ||
		   (cur->status & HOMING_NPC)) continue ;
#else
		/* 気絶中の天狗兵攻撃仕様は無し */
		if((cur->status & HOMING_SKIP) || (cur->status & HOMING_NPC)) continue ;
#endif


#ifdef DEBUG
		if(cur->status & HOMING_TNG_FAINT){
			DEBUG_PRINT_NPCSNAKE("Found Faint !!\n");
		}
#endif

		if(res==cur){
			// DEBUG_PRINT_NPCSNAKE("Same Enemy\n");
			continue;
		}
		if(pre_homing==cur){
			// DEBUG_PRINT_NPCSNAKE("Same PreHoming Enemy\n");
			continue;
		}

		MatToVec( &(cur->body->objs->objs[part_num].world), &vec ) ;
		GV_SubVec3F( &vec, pos, &sub_vec ) ;
		dis = FVecLen3( &sub_vec ) ;
		if ( dis > min_dis ){
			continue ;
		}

#if 0
		/* 気絶中の天狗兵攻撃仕様は無し */
		if((cur->status & HOMING_TNG_FAINT) && dis<CVM2N(2.0f)) continue;
#endif

		t_dir = GV_VecDir2( &sub_vec ) ;
		sub_dir = GV_DiffDirAbs( dir, t_dir ) ;
		if ( sub_dir > range ){
			// DEBUG_PRINT_NPCSNAKE("Range Over\n");
			continue ;
		}

		t_dir = GV_VecDir2X( &sub_vec ) ;
		t_dir &= (0x1000-1);
		if(t_dir>=0x800) t_dir-=0x1000;
		if ( t_dir > ud_range || t_dir < -ud_range ){
			// DEBUG_PRINT_NPCSNAKE("UD-Range Over\n");
			continue ;
		}
#if 1
		if ( hzx_id != 0 ) {

#if 1
		    if ( HZX_OnlineHazardCheck( hzx_id, pos, &vec, HZX_CHK_ALL, 
					        HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
					        HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE |
					        HZX_FLOOR_IK ) ) {

				// DEBUG_PRINT_NPCSNAKE("Online Check\n");
				continue ;
		    }
#else
		    if(HZX_OnlineHazardCheck(hzx_id,pos,&vec,
									 HZX_CHK_ALL|HZX_CHK_RECOIL_TYPE,0,0)){

				// DEBUG_PRINT_NPCSNAKE("Online Check\n");
				continue ;
		    }
#endif

		}
#endif
		min_dis = dis ;
		res = cur ;
	}

	return res ;
}

static void CalcHomingDisAndArg(Work *work)
{
	float dis=FLT_MAX;

	if(work->homing!=NULL){
		FVECTOR *p=(FVECTOR *)&(work->body.objs->world.m[3][0]);
		FVECTOR d;

		vu0_Ldv0((FVECTOR *)&(work->homing->world->m[3][0]));
		vu0_Ldv1(p);

		vu0_Subv0v1();
		vu0_Stv0(&d);

		dis=fpu_Sqrt(vu0_VectorLength2v0());

		work->homing_dir=GV_VecDir2(&d);

		// DEBUG_PRINT_NPCSNAKE("ENEMY SET : %f\n",dis);
	}

	work->homing_dis=dis;

	work->cmp_homing=work->homing;
}

/* 敵兵を全方向にサーチ */
static void SearchEnemy(Work *work)
{
	FVECTOR *p=(FVECTOR *)&(work->body.objs->objs[HUMAN21_KOSHI].world.m[3][0]);
	int part_num=NPCSnake_SearchPart(work);
	int t_part_num;

	if(part_num==-1) t_part_num=HUMAN21_MUNE;
	else t_part_num=part_num;

#if 0
	work->homing=GM_GetHomingTrgPart(p,0,SEARCH_DIS,
									 SEARCH_ALLRANGE,SEARCH_UDRANGE,t_part_num,
									 work->ctrl.hzx_id,work->pre_homing,work->cmp_homing);
#else
	work->homing=GM_GetHomingTrgPart(p,0,SEARCH_DIS,
									 SEARCH_ALLRANGE,SEARCH_UDRANGE,HUMAN21_KUBI,
									 work->ctrl.hzx_id,work->pre_homing,work->cmp_homing);
#endif

	if(work->homing!=NULL){
		fpu_CopyVector(&(work->homing_pos),
					   (FVECTOR *)&(work->homing->body->objs->objs[t_part_num].world.m[3][0]));

		work->homing_p=&(work->homing_pos);

		if(part_num<0) NPCSnake_CalcTarget(work);

		work->homing_part=part_num;
	}
	else{
		work->homing_p=NULL;
	}
}

/* 敵兵を進行方向のみサーチ */
static void SearchEnemyToward(Work *work)
{
	FVECTOR *p=(FVECTOR *)&(work->body.objs->objs[HUMAN21_KOSHI].world.m[3][0]);
	int part_num=NPCSnake_SearchPart(work);
	int t_part_num;

	if(part_num==-1) t_part_num=HUMAN21_MUNE;
	else t_part_num=part_num;

#if 0
	work->homing=GM_GetHomingTrgPart(p,work->ctrl.rot.vy,SEARCH_DIS,
									 SEARCH_RANGE,SEARCH_UDRANGE,t_part_num,
									 work->ctrl.hzx_id,work->pre_homing,work->cmp_homing);
#else
	work->homing=GM_GetHomingTrgPart(p,work->ctrl.rot.vy,SEARCH_DIS,
									 SEARCH_RANGE,SEARCH_UDRANGE,HUMAN21_KUBI,
									 work->ctrl.hzx_id,work->pre_homing,work->cmp_homing);
#endif

	if(work->homing!=NULL){
		fpu_CopyVector(&(work->homing_pos),
					   (FVECTOR *)&(work->homing->body->objs->objs[t_part_num].world.m[3][0]));

		work->homing_p=&(work->homing_pos);

		if(part_num<0) NPCSnake_CalcTarget(work);

		work->homing_part=part_num;
	}
	else{
		work->homing_p=NULL;
	}
}

/* 覗き込みモーション時の視界 */
static void SearchEnemyPeeping(Work *work)
{
	FVECTOR *p=(FVECTOR *)&(work->body.objs->objs[HUMAN21_ATAMA].world.m[3][0]);
	int part_num=NPCSnake_SearchPart2(work);
	int t_part_num;

	if(part_num==-1) t_part_num=HUMAN21_MUNE;
	else t_part_num=part_num;

	if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
#if 0
		work->homing=GM_GetHomingTrgPart(p,SEARCH_DEG_PEEPRIGHT,SEARCH_DIS,
										 SEARCH_RANGE_PEEPING,SEARCH_UDRANGE,t_part_num,
										 work->ctrl.hzx_id,NULL,NULL);
#else
		work->homing=GM_GetHomingTrgPart(p,SEARCH_DEG_PEEPRIGHT,SEARCH_DIS,
										 SEARCH_RANGE_PEEPING,SEARCH_UDRANGE,HUMAN21_KUBI,
										 work->ctrl.hzx_id,NULL,NULL);
#endif

		// DEBUG_PRINT_NPCSNAKE("Peep right = 0x%08x\n",(int)(work->homing));
	}
	else{
#if 0
		work->homing=GM_GetHomingTrgPart(p,SEARCH_DEG_PEEPLEFT,SEARCH_DIS,
										 SEARCH_RANGE_PEEPING,SEARCH_UDRANGE,t_part_num,
										 work->ctrl.hzx_id,NULL,NULL);
#else
		work->homing=GM_GetHomingTrgPart(p,SEARCH_DEG_PEEPLEFT,SEARCH_DIS,
										 SEARCH_RANGE_PEEPING,SEARCH_UDRANGE,HUMAN21_KUBI,
										 work->ctrl.hzx_id,NULL,NULL);
#endif

		// DEBUG_PRINT_NPCSNAKE("Peep left = 0x%08x\n",(int)(work->homing));
	}

	if(work->homing!=NULL){
		fpu_CopyVector(&(work->homing_pos),
					   (FVECTOR *)&(work->homing->body->objs->objs[t_part_num].world.m[3][0]));

		work->homing_p=&(work->homing_pos);

		if(part_num<0) NPCSnake_CalcTarget(work);

		work->homing_part=part_num;
	}
	else{
		work->homing_p=NULL;
	}
}

/* 敵兵を前方のみサーチ */
static void SearchEnemyByAhead(Work *work)
{
	FVECTOR *p=(FVECTOR *)&(work->body.objs->objs[HUMAN21_KOSHI].world.m[3][0]);
	int part_num=NPCSnake_SearchPart(work);
	int t_part_num;

	if(part_num==-1) t_part_num=HUMAN21_MUNE;
	else t_part_num=part_num;

#if 0
	work->homing=GM_GetHomingTrgPart(p,SEARCH_DEG_AHEAD,SEARCH_DIS,
									 SEARCH_RANGE_AHEAD,SEARCH_UDRANGE,t_part_num,
									 work->ctrl.hzx_id,work->pre_homing,work->cmp_homing);
#else
	work->homing=GM_GetHomingTrgPart(p,SEARCH_DEG_AHEAD,SEARCH_DIS,
									 SEARCH_RANGE_AHEAD,SEARCH_UDRANGE,HUMAN21_KUBI,
									 work->ctrl.hzx_id,work->pre_homing,work->cmp_homing);
#endif

	if(work->homing!=NULL){
		fpu_CopyVector(&(work->homing_pos),
					   (FVECTOR *)&(work->homing->body->objs->objs[t_part_num].world.m[3][0]));

		work->homing_p=&(work->homing_pos);

		if(part_num<0) NPCSnake_CalcTarget(work);

		work->homing_part=part_num;
	}
	else{
		work->homing_p=NULL;
	}
}

/* 敵兵を後方のみサーチ */
static void SearchEnemyBehind(Work *work)
{
	FVECTOR *p=(FVECTOR *)&(work->body.objs->objs[HUMAN21_KOSHI].world.m[3][0]);
	int part_num=NPCSnake_SearchPart(work);
	int t_part_num;

	if(part_num==-1) t_part_num=HUMAN21_MUNE;
	else t_part_num=part_num;

#if 0
	work->homing=GM_GetHomingTrgPart(p,SEARCH_DEG_BEHIND,SEARCH_DIS,
									 SEARCH_RANGE_BEHIND,SEARCH_UDRANGE,t_part_num,
									 work->ctrl.hzx_id,work->pre_homing,work->cmp_homing);
#else
	work->homing=GM_GetHomingTrgPart(p,SEARCH_DEG_BEHIND,SEARCH_DIS,
									 SEARCH_RANGE_BEHIND,SEARCH_UDRANGE,HUMAN21_KUBI,
									 work->ctrl.hzx_id,work->pre_homing,work->cmp_homing);
#endif

	if(work->homing!=NULL){
		fpu_CopyVector(&(work->homing_pos),
					   (FVECTOR *)&(work->homing->body->objs->objs[t_part_num].world.m[3][0]));

		work->homing_p=&(work->homing_pos);

		if(part_num<0) NPCSnake_CalcTarget(work);

		work->homing_part=part_num;
	}
	else{
		work->homing_p=NULL;
	}
}

/* ライデンの周りの敵を検索 */
static void SearchEnemyAroundRaiden(Work *work)
{
	HOMING_TRG	*hom, *cur;
	FVECTOR *pos=&GM_PlayerPosition;
	FVECTOR	*vec, sub_vec ;
	int	dis, t_dir;
	int count=0,count2=0;

	hom = GM_GetHoming();

	work->raiden_status
		&=~(NPCSNAKE_RAIDEN_ENEMY_AHEAD|NPCSNAKE_RAIDEN_ENEMY_BEHIND|
			NPCSNAKE_RAIDEN_2ENEMYS|NPCSNAKE_RAIDEN_MANY_ENEMYS|
			NPCSNAKE_RAIDEN_ENEMY_EXIST);

	while( hom != NULL ) {
		cur = hom ;
		hom = hom->next ;

		if ( cur->status & HOMING_SKIP ) continue ;

		vec=(FVECTOR *)&(cur->body->objs->world.m[3][0]);
		GV_SubVec3F(vec,pos,&sub_vec);
		dis=fpu_VectorLength2(&sub_vec);

		if(dis<CHECK_ENEMY_DIS_RAIDEN_AHEAD) count++;
		if(dis<CHECK_ENEMY_DIS_RAIDEN_BEHIND) count2++;

		t_dir = GV_VecDir2( &sub_vec ) & 0xfff ;

		if(0x400<=t_dir && t_dir<=0xc00 && dis<CHECK_ENEMY_DIS_RAIDEN_AHEAD){
			work->raiden_status|=NPCSNAKE_RAIDEN_ENEMY_AHEAD;
		}
		if((0xc00<t_dir || t_dir<0x400) && dis<CHECK_ENEMY_DIS_RAIDEN_BEHIND){
			work->raiden_status|=NPCSNAKE_RAIDEN_ENEMY_BEHIND;
		}
	}

	if(count>=2) work->raiden_status|=NPCSNAKE_RAIDEN_2ENEMYS;
	if(count2>=4) work->raiden_status|=NPCSNAKE_RAIDEN_MANY_ENEMYS;

	/* 2001/9/17 K.Kano 追加 */
	if(count2>0) work->raiden_status|=NPCSNAKE_RAIDEN_ENEMY_EXIST;
}

/* ライデンからスネークが見えるかどうか判定(w17a) */
static void RaidenLooksSnake(Work *work)
{
	work->raiden_status&=~NPCSNAKE_RAIDEN_LOOKS_SNAKE;

	if(NPCSNAKE_ACT_MODE(work)==NPCSNAKE_ACT_W17A_0 ||
	   NPCSNAKE_ACT_MODE(work)==NPCSNAKE_ACT_W17A_1){

		int flag;

		if((flag=DG_BoundCheck(&(work->body.objs->world),
							   &(work->body.objs->bound_max),
							   &(work->body.objs->bound_min)))!=-1){

			HZX_GROUP_ID hzx_id=work->ctrl.hzx_id;
			// HZX_GROUP_ID hzx_id=GM_PlayerControl->hzx_id;

#if 0
			DEBUG_PRINT_NPCSNAKE("BoundingCheck Hit\n");
			DEBUG_PRINT_NPCSNAKE("Eye = %f %f %f\n",
								 DG_Chanl(0)->eye.m[3][0],
								 DG_Chanl(0)->eye.m[3][1],
								 DG_Chanl(0)->eye.m[3][2]);
			DEBUG_PRINT_NPCSNAKE("Mov = %f %f %f\n",
								 work->ctrl.mov.vx,
								 work->ctrl.mov.vy,
								 work->ctrl.mov.vz);
#endif

			if(!(flag & 1)){
				if(GM_CheckPlayerStatus(PLAYER_BEHIND)){
					FVECTOR v1,v2;

					vu0_Ldv1((FVECTOR *)&(DG_Chanl(0)->eye.m[0][0]));
					vu0_Ldv2((FVECTOR *)&(DG_Chanl(0)->eye.m[2][0]));
					vu0_Ldv0((FVECTOR *)&(DG_Chanl(0)->eye.m[3][0]));

					/* 右に1m、前に2mずらす */
					vu0_Mulv1a(CVM2N(1.0f));
					vu0_Mulv2a(CVM2N(2.0f));

					vu0_Addv0v1();
					vu0_Mulv1a(-2.0f);

					vu0_Addv0v2();
					vu0_Addv1v0();

					vu0_Stv0(&v1);
					vu0_Stv1(&v2);

					if(!HZX_OnlineHazardCheck(hzx_id,&(work->ctrl.mov),&v1,
											  HZX_CHK_ALL,HZX_TYPE_PLAYER,HZX_FLOOR_ALL)){

						work->raiden_status|=NPCSNAKE_RAIDEN_LOOKS_SNAKE;

						// DEBUG_PRINT_NPCSNAKE("Hit!!!\n");
					}
					if(!HZX_OnlineHazardCheck(hzx_id,&(work->ctrl.mov),&v2,
											  HZX_CHK_ALL,HZX_TYPE_PLAYER,HZX_FLOOR_ALL)){

						work->raiden_status|=NPCSNAKE_RAIDEN_LOOKS_SNAKE;

						// DEBUG_PRINT_NPCSNAKE("Hit!!!\n");
					}
				}
				else{
					if(!HZX_OnlineHazardCheck(hzx_id,&(work->ctrl.mov),
											  (FVECTOR *)&(DG_Chanl(0)->eye.m[3][0]),
											  HZX_CHK_ALL,HZX_TYPE_PLAYER,HZX_FLOOR_ALL)){

						work->raiden_status|=NPCSNAKE_RAIDEN_LOOKS_SNAKE;

						// DEBUG_PRINT_NPCSNAKE("Hit!!!\n");
					}
				}
			}
			if(!(flag & 2)){
				if(GM_CheckPlayerStatus(PLAYER_BEHIND)){
					FVECTOR v1,v2;

					vu0_Ldv1((FVECTOR *)&(DG_Chanl(1)->eye.m[0][0]));
					vu0_Ldv2((FVECTOR *)&(DG_Chanl(1)->eye.m[2][0]));
					vu0_Ldv0((FVECTOR *)&(DG_Chanl(1)->eye.m[3][0]));

					/* 右に1m、前に2mずらす */
					vu0_Mulv1a(CVM2N(1.0f));
					vu0_Mulv2a(CVM2N(2.0f));

					vu0_Addv0v1();
					vu0_Mulv1a(-2.0f);

					vu0_Addv0v2();
					vu0_Addv1v0();

					vu0_Stv0(&v1);
					vu0_Stv1(&v2);

					if(!HZX_OnlineHazardCheck(hzx_id,&(work->ctrl.mov),&v1,
											  HZX_CHK_ALL,HZX_TYPE_PLAYER,HZX_FLOOR_ALL)){

						work->raiden_status|=NPCSNAKE_RAIDEN_LOOKS_SNAKE;

						// DEBUG_PRINT_NPCSNAKE("Hit!!!\n");
					}
					if(!HZX_OnlineHazardCheck(hzx_id,&(work->ctrl.mov),&v2,
											  HZX_CHK_ALL,HZX_TYPE_PLAYER,HZX_FLOOR_ALL)){

						work->raiden_status|=NPCSNAKE_RAIDEN_LOOKS_SNAKE;

						// DEBUG_PRINT_NPCSNAKE("Hit!!!\n");
					}
				}
				else{
					if(!HZX_OnlineHazardCheck(hzx_id,&(work->ctrl.mov),
											  (FVECTOR *)&(DG_Chanl(0)->eye.m[3][0]),
											  HZX_CHK_ALL,HZX_TYPE_PLAYER,HZX_FLOOR_ALL)){

						work->raiden_status|=NPCSNAKE_RAIDEN_LOOKS_SNAKE;

						// DEBUG_PRINT_NPCSNAKE("Hit!!!\n");
					}
				}
			}
		}
	}
}

void GetDisToRaiden(Work *work)
{
	/* ライデンとの距離を算出 */
	vu0_Ldv1(&GM_PlayerPosition);
	vu0_Ldv0(&(work->ctrl.mov));

	vu0_Setv1y0();
	vu0_Setv0y0();

	vu0_Subv0v1();

	work->raiden_dis=fpu_Sqrt(vu0_VectorLength2v0());
}

static inline void JudgeRaidenAndSnakeLine(Work *work)
{
	/* ライデンとスネークの位置関係を判定。
	   w44aでは、Z軸方向が重要なので、Z軸のみ判定 */

	float raiden_line=GM_PlayerPosition.vz;
	float snake_line=work->ctrl.mov.vz;

		/* ライデンを先行しすぎた */
	if(raiden_line<snake_line+work->independ_to_search_dis){
		work->snake_status&=~SNAKE_STATUS_TOO_OVER_RAIDEN;
	}
	else{
		work->snake_status|=SNAKE_STATUS_TOO_OVER_RAIDEN;
	}

	if(raiden_line<snake_line+LINE_OVER_RAIDEN){
		work->snake_status&=~SNAKE_STATUS_OVER_RAIDEN;
	}
	else{
		work->snake_status|=SNAKE_STATUS_OVER_RAIDEN;
	}

	if(raiden_line<snake_line+LINE_OVER_RAIDEN0){
		work->snake_status&=~SNAKE_STATUS_OVER_RAIDEN0;
	}
	else{
		work->snake_status|=SNAKE_STATUS_OVER_RAIDEN0;
	}

	/* ライデンにおいてかれた */
	if(raiden_line<snake_line+LINE0_BEHIND_RAIDEN){
		work->snake_status|=SNAKE_STATUS_BEHIND_RAIDEN_L0;
	}
	else{
		work->snake_status&=~SNAKE_STATUS_BEHIND_RAIDEN_L0;
	}

	if(raiden_line<snake_line+LINE1_BEHIND_RAIDEN){
		work->snake_status|=SNAKE_STATUS_BEHIND_RAIDEN_L1;
	}
	else{
		work->snake_status&=~SNAKE_STATUS_BEHIND_RAIDEN_L1;
	}

	if(raiden_line<snake_line+LINE2_BEHIND_RAIDEN){
		work->snake_status|=SNAKE_STATUS_BEHIND_RAIDEN_L2;
	}
	else{
		work->snake_status&=~SNAKE_STATUS_BEHIND_RAIDEN_L2;
	}

	if(raiden_line<snake_line+LINE3_BEHIND_RAIDEN){
		work->snake_status|=SNAKE_STATUS_BEHIND_RAIDEN_L3;
	}
	else{
		work->snake_status&=~SNAKE_STATUS_BEHIND_RAIDEN_L3;
	}
}

static inline void IsRaidenMoving(Work *work)
{
	if(work->raiden_status & NPCSNAKE_RAIDEN_MOVING){
		/* 移動中 */
		if(HZX_Zone1(work->pre_raiden_zone)==HZX_Zone1(GM_PlayerControl->addr)){
			work->raiden_stay_count++;
			if(work->raiden_stay_count>10){
				work->raiden_status&=~NPCSNAKE_RAIDEN_MOVING;
				fpu_CopyVector(&(work->raiden_stay_pos),&GM_PlayerPosition);
			}
		}
		else{
			work->raiden_stay_count=0;
		}
	}
	else{
		/* 停止中 */
		if(work->stage==NPCSNAKE_STAGE_W44A){
			if(fpu_Abs(work->raiden_stay_pos.vz-GM_PlayerPosition.vz)>CVM2N(3.0f)){
				work->raiden_status|=NPCSNAKE_RAIDEN_MOVING;
				work->raiden_stay_count=0;
			}
		}
		else{
			vu0_Ldv0(&(work->raiden_stay_pos));
			vu0_Ldv1(&(GM_PlayerPosition));

			vu0_Subv0v1();

			if(vu0_VectorLength2v0()>CVM2N(3.0f)*CVM2N(3.0f)){
				work->raiden_status|=NPCSNAKE_RAIDEN_MOVING;
				work->raiden_stay_count=0;
			}
		}
	}
}

static inline void IncreaseCount(Work *work)
{
	/* カウンター系を計算 */
	if(NPCSNAKE_ACT_MODE(work)!=NPCSNAKE_ACT_STAY){
		work->movepos_count++;
		work->stay_count=0;
	}
	else{
		work->stay_count++;
		work->movepos_count=0;
	}

	if(NPCSNAKE_HIDE_MODE(work)!=NPCSNAKE_HIDE_NO_HIDING &&
	   NPCSNAKE_HIDE_MODE(work)!=NPCSNAKE_HIDE_MOVE_TO_HIDE){

		work->hide_count++;
	}

	if(work->mode==NPCSNAKE_MODE_SEARCHENEMY &&
	   NPCSNAKE_HIDE_MODE(work)!=NPCSNAKE_HIDE_NO_HIDING &&
	   NPCSNAKE_HIDE_MODE(work)!=NPCSNAKE_HIDE_MOVE_TO_HIDE){

		work->wait_raiden_count++;
		if(work->wait_raiden_count>work->search_to_pickup_interval){
			work->snake_status|=SNAKE_STATUS_WAIT_RAIDEN_OVERTIME;
		}
		else{
			work->snake_status&=~SNAKE_STATUS_WAIT_RAIDEN_OVERTIME;
		}
	}
	else{
		work->wait_raiden_count=0;
		work->snake_status&=~SNAKE_STATUS_WAIT_RAIDEN_OVERTIME;
	}
	if(work->mode==NPCSNAKE_MODE_BEHIND_RAIDEN){
		work->behind_raiden_count++;
	}
	else{
		work->behind_raiden_count=0;
	}
	if(work->snake_status2 & SNAKE_STATUS2_BE_CAPTURED_MODE){
		if(NPCSNAKE_HIDE_MODE(work)==NPCSNAKE_HIDE_NO_HIDING){
			work->snake_status2&=~SNAKE_STATUS2_BE_CAPTURED_MODE;
		}
		else{
			work->capture_count++;
			if(work->capture_count>=work->capture_wait_interval){
				work->snake_status2&=~SNAKE_STATUS2_BE_CAPTURED_MODE;
			}
		}
	}
	else{
		work->capture_count=0;
	}
}

static void JudgeRaidenLife(Work *work)
{
	/* 特殊ルール、レーションが０なら半分の時も危険状態 */
	if(GM_ItemNum(IT_Ration)==0){
		if(GM_Vitality<GM_VitalityMax/2){
			work->raiden_status|=NPCSNAKE_RAIDEN_LIFE_DANGER;

			work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);
			work->snake_status|=SNAKE_STATUS_USE_FMS;
		}
		else{
			work->raiden_status&=~NPCSNAKE_RAIDEN_LIFE_DANGER;
		}
		return;
	}

	/* ライデンのライフが1/3になったら、危険状態と判断 */
	if(GM_Vitality<GM_VitalityMax/3){
		work->raiden_status|=NPCSNAKE_RAIDEN_LIFE_DANGER;

		work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);
		work->snake_status|=SNAKE_STATUS_USE_FMS;
	}
	else{
		work->raiden_status&=~NPCSNAKE_RAIDEN_LIFE_DANGER;
	}
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


void GetSituation_SEARCHENEMY(Work *work)
{
	/* 周囲索敵モード */
	if(!(work->snake_status & SNAKE_STATUS_LOCKON_ENEMY)){
		work->homing=NULL;

		switch(NPCSNAKE_HIDE_MODE(work)){
		case NPCSNAKE_HIDE_NO_HIDING:
			SearchEnemy(work);
			break;
		case NPCSNAKE_HIDE_MOVE_TO_HIDE:
			SearchEnemyToward(work);
			break;
		case NPCSNAKE_HIDE_HIDING:
			SearchEnemyBehind(work);
			break;
		case NPCSNAKE_HIDE_SITDOWN_TO_HIDE:
		case NPCSNAKE_HIDE_BACK_TO_HIDE:
			break;
		case NPCSNAKE_HIDE_PEEPING:
		case NPCSNAKE_HIDE_ATTACK:
			SearchEnemyPeeping(work);
			break;
		}
	}

	GetInfinityShapePos(work);
	// GetNeighborHzx(work);

	work->shoot_interval_count++;
	if(work->shoot_interval_count>SHOOT_INTERVAL_COUNT(work)){
		work->shoot_count=work->shoot_limit;
	}

	CalcHomingDisAndArg(work);

	JudgeRaidenAndSnakeLine(work);

	IsRaidenMoving(work);

	IncreaseCount(work);

	JudgeRaidenLife(work);

	SearchEnemyAroundRaiden(work);
}

void GetSituation_INDEPEND(Work *work)
{
	/* 自己判断モード */
	if((work->snake_status & SNAKE_STATUS_ACTION_FINISHED) &&
	   NPCSNAKE_HIDE_MODE(work)==NPCSNAKE_HIDE_NO_HIDING){

		GetSupportPos(work);
	}

	GetInfinityShapePos(work);
	// GetNeighborHzx(work);

	if(!(work->snake_status & SNAKE_STATUS_LOCKON_ENEMY)){
		work->homing=NULL;

		switch(NPCSNAKE_HIDE_MODE(work)){
		case NPCSNAKE_HIDE_NO_HIDING:
			SearchEnemy(work);
			break;
		case NPCSNAKE_HIDE_MOVE_TO_HIDE:
			SearchEnemyToward(work);
			break;
		case NPCSNAKE_HIDE_HIDING:
			SearchEnemyBehind(work);
			break;
		case NPCSNAKE_HIDE_SITDOWN_TO_HIDE:
		case NPCSNAKE_HIDE_BACK_TO_HIDE:
			break;
		case NPCSNAKE_HIDE_PEEPING:
		case NPCSNAKE_HIDE_ATTACK:
			SearchEnemyPeeping(work);
			break;
		}
	}

	work->shoot_interval_count++;
	if(work->shoot_interval_count>SHOOT_INTERVAL_COUNT(work)){
		work->shoot_count=work->shoot_limit;
	}

	CalcHomingDisAndArg(work);

	JudgeRaidenAndSnakeLine(work);

	IsRaidenMoving(work);

	IncreaseCount(work);

	JudgeRaidenLife(work);

	SearchEnemyAroundRaiden(work);
}

void GetSituation_FOLLOW_RAIDEN(Work *work)
{
	/* ライデン追跡モード */
	GetSupportPos(work);
	if(!(work->snake_status & SNAKE_STATUS_LOCKON_ENEMY)){
		work->homing=NULL;
		SearchEnemyByAhead(work);
	}

	work->shoot_interval_count++;
	if(work->shoot_interval_count>SHOOT_INTERVAL_COUNT(work)){
		work->shoot_count=work->shoot_limit;
	}

	CalcHomingDisAndArg(work);

	JudgeRaidenAndSnakeLine(work);

	IsRaidenMoving(work);

	IncreaseCount(work);

	JudgeRaidenLife(work);

	SearchEnemyAroundRaiden(work);
}

void GetSituation_BEHIND_RAIDEN(Work *work)
{
	/* ライデン後方援護モード */
	GetHidePos(work);

	GetInfinityShapePos(work);
	// GetNeighborHzx(work);

	if(!(work->snake_status & SNAKE_STATUS_LOCKON_ENEMY)){
		work->homing=NULL;

		switch(NPCSNAKE_HIDE_MODE(work)){
		case NPCSNAKE_HIDE_NO_HIDING:
			SearchEnemy(work);
			break;
		case NPCSNAKE_HIDE_MOVE_TO_HIDE:
			SearchEnemyToward(work);
			break;
		case NPCSNAKE_HIDE_HIDING:
			SearchEnemyBehind(work);
			break;
		case NPCSNAKE_HIDE_SITDOWN_TO_HIDE:
		case NPCSNAKE_HIDE_BACK_TO_HIDE:
			break;
		case NPCSNAKE_HIDE_PEEPING:
		case NPCSNAKE_HIDE_ATTACK:
			SearchEnemyPeeping(work);
			break;
		}
	}

	work->shoot_interval_count++;
	if(work->shoot_interval_count>SHOOT_INTERVAL_COUNT(work)){
		work->shoot_count=work->shoot_limit;
	}

	CalcHomingDisAndArg(work);

	JudgeRaidenAndSnakeLine(work);

	IsRaidenMoving(work);

	IncreaseCount(work);

	JudgeRaidenLife(work);

	SearchEnemyAroundRaiden(work);
}

void GetSituation_PICKUP_RAIDEN(Work *work)
{
	/* ライデンお迎えモード */
	GetInfinityShapePos(work);
	// GetNeighborHzx(work);

	if(!(work->snake_status & SNAKE_STATUS_LOCKON_ENEMY)){
		work->homing=NULL;
		SearchEnemyBehind(work);
	}

	work->shoot_interval_count++;
	if(work->shoot_interval_count>SHOOT_INTERVAL_COUNT(work)){
		work->shoot_count=work->shoot_limit;
	}

	CalcHomingDisAndArg(work);

	JudgeRaidenAndSnakeLine(work);

	IsRaidenMoving(work);

	IncreaseCount(work);

	JudgeRaidenLife(work);

	SearchEnemyAroundRaiden(work);
}


/* 2001/9/18  K.Kano
   w44aでの最後の山場でのスネークの動きを改善
   新モードを追加 */

void GetSituation_W44AFinal(Work *work)
{
	GetSupportPosW44AFinal(work);
	GetNeighborHzxW45A(work);
	if(!(work->snake_status & SNAKE_STATUS_LOCKON_ENEMY)){
		work->homing=NULL;
		SearchEnemy(work);
	}

	work->shoot_interval_count++;
	if(work->shoot_interval_count>SHOOT_INTERVAL_COUNT(work)){
		work->shoot_count=work->shoot_limit;
	}

	CalcHomingDisAndArg(work);

	IsRaidenMoving(work);

	IncreaseCount(work);

	JudgeRaidenLife(work);

	SearchEnemyAroundRaiden(work);
}



void GetSituation_W45A(Work *work)
{
	GetSupportPosW45A(work);
	GetNeighborHzxW45A(work);
	if(!(work->snake_status & SNAKE_STATUS_LOCKON_ENEMY)){
		work->homing=NULL;
		SearchEnemy(work);
	}

	work->shoot_interval_count++;
	if(work->shoot_interval_count>SHOOT_INTERVAL_COUNT(work)){
		work->shoot_count=work->shoot_limit;
	}

	CalcHomingDisAndArg(work);

	IsRaidenMoving(work);

	IncreaseCount(work);

	JudgeRaidenLife(work);

	SearchEnemyAroundRaiden(work);
}

void GetSituation_BETWEEN_STAGES(Work *work)
{
	work->snake_status&=~SNAKE_STATUS_LOCKON_ENEMY;

	GetAvoidPos(work);

	IsRaidenMoving(work);

	IncreaseCount(work);

	// JudgeRaidenLife(work,0);

	SearchEnemyAroundRaiden(work);

	if(GM_CheckPlayerStatus(PLAYER_DAMAGED|PLAYER_DOWNED)){
		work->raiden_status|=NPCSNAKE_RAIDEN_DAMAGED;
	}
}

void GetSituation_W14A(Work *work)
{
	IsRaidenMoving(work);

	IncreaseCount(work);

	// JudgeRaidenLife(work,0);

	SearchEnemyAroundRaiden(work);
}

void GetSituation_W17A(Work *work)
{
	RaidenLooksSnake(work);

	IsRaidenMoving(work);

	IncreaseCount(work);

	// JudgeRaidenLife(work,0);

	SearchEnemyAroundRaiden(work);
}

void GetSituation_W32A(Work *work)
{
	if((work->snake_status & SNAKE_STATUS_ORDER_TO_SNIPE) &&
	   !(work->snake_status & SNAKE_STATUS_LOCKON_ENEMY)){

		NPCSnake_CheckInSight(work);
	}

	// CalcHomingDisAndArg(work);

	IsRaidenMoving(work);

	IncreaseCount(work);

	// JudgeRaidenLife(work,0);

	SearchEnemyAroundRaiden(work);
}

void GetSituation_DUMMY(Work *work)
{
	work->snake_status&=~SNAKE_STATUS_LOCKON_ENEMY;
	work->homing=NULL;
}
