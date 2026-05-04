//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
    traffic6.c
    (株)ジョージ交通
	2000/06/02 T.Shibata

	$Id: traffic.c,v 1.2 2002/12/09 11:19:15 takaki Exp $
*/
#endif
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

//#include	"def_dma.h"
//#include	"utl_dma.h"

#include	"../util/ts_util.h"

#include "BP_Misc.h"

//片道４車線 計８車線 (テスト中 2 )
#define		TRAFFIC_LANE (4)

//橋の矩形(小さい順)
#if 1
#define		BOUND0_X	(-150000.0f)
#define		BOUND0_Y	(65500.0f)
#define		BOUND0_Z	(18500.0f)

#define		BOUND1_X	(50000.0f)
#define		BOUND1_Y	(65500.0f)
#define		BOUND1_Z	(45500.0f)
#else
#define		BOUND0_X	(-200000.0f)
#define		BOUND0_Y	(65500.0f)
#define		BOUND0_Z	(18500.0f)

#define		BOUND1_X	(100000.0f)
#define		BOUND1_Y	(65500.0f)
#define		BOUND1_Z	(45500.0f)
#endif

//中央線の範囲(中央分離体の厚さ/2)
#define		CENTER_LINE_W		(50.0f)
#define		CENTER_LINE_H		(100.0f)


#define		BOUND_W				((BOUND1_Z - BOUND0_Z)/4.0f)
#define		BOUND_L				(BOUND1_X - BOUND0_X)

#define		SHIFT0_Z		(BOUND0_Z + BOUND_W)
#define		SHIFT1_Z		(BOUND0_Z + BOUND_W*3.0f)
#define		SHIFT_Y			(BOUND0_Y)

#define		SWING_ONE_LANE_EX		(BOUND_W/4.0f)
#define		SWING_TWO_LANE_EX		(SWING_ONE_LANE_EX*2)
#define		SWING_FOUR_LANE_EX		(SWING_ONE_LANE_EX*4)

#define		SWING_ONE_LANE		(BOUND_W/4.0f * 0.6f)
#define		SWING_TWO_LANE		(SWING_ONE_LANE*2)
#define		SWING_THREE_LANE	(SWING_ONE_LANE*3)
#define		SWING_FOUR_LANE		(SWING_ONE_LANE*4)

#define		OFFSET_Z0_A			(BOUND0_Z + SWING_ONE_LANE_EX)
#define		OFFSET_Z0_BH		(BOUND0_Z + SWING_ONE_LANE_EX + SWING_TWO_LANE_EX)
#define		OFFSET_Z0_CI		(BOUND0_Z + SWING_ONE_LANE_EX + SWING_TWO_LANE_EX*2)
#define		OFFSET_Z0_D			(BOUND0_Z + SWING_ONE_LANE_EX + SWING_TWO_LANE_EX*3)
#define		OFFSET_Z0_E			(BOUND0_Z + SWING_TWO_LANE_EX)
#define		OFFSET_Z0_F			(BOUND0_Z + SWING_TWO_LANE_EX * 2)
#define		OFFSET_Z0_G			(BOUND0_Z + SWING_TWO_LANE_EX * 3)

#define		OFFSET_Z1_A			(OFFSET_Z0_A + SWING_FOUR_LANE_EX*2)
#define		OFFSET_Z1_BH		(OFFSET_Z0_BH + SWING_FOUR_LANE_EX*2)
#define		OFFSET_Z1_CI		(OFFSET_Z0_CI + SWING_FOUR_LANE_EX*2)
#define		OFFSET_Z1_D			(OFFSET_Z0_D + SWING_FOUR_LANE_EX*2)
#define		OFFSET_Z1_E			(OFFSET_Z0_E + SWING_FOUR_LANE_EX*2)
#define		OFFSET_Z1_F			(OFFSET_Z0_F + SWING_FOUR_LANE_EX*2)
#define		OFFSET_Z1_G			(OFFSET_Z0_G + SWING_FOUR_LANE_EX*2)

#define		LIGHT_HIGHT_MIN			(20.0f)
#define		LIGHT_HIGHT_MAX			(80.0f)
#define		LIGHT_HIGHT_SCALE		(LIGHT_HIGHT_MAX - LIGHT_HIGHT_MIN)

#define		LANE_NUM		(0x00000007)

#define		TRFC_FLAGS_INVSBL		(0x0001)
#define		TRFC_FLAGS_INVSBL1		(0x0002)
#define		TRFC_FLAGS_CHNGBRGHT	(0x0004)
#define		TRFC_FLAGS_CHNGBRGHT1	(0x0008)

#define		MAX_ALPHA		(32)
#define		MAX_ALPHA1		(16)

#define		TAILS		(128-16)
#define		N_MIN_TAIL 	(16)
#define		SUB_ALPHA0	(MAX_ALPHA/16)
#define		SUB_ALPHA1	((TAILS-16)/MAX_ALPHA)
#define		SUB_ALPHA_TST		(128/MAX_ALPHA)
#define		ALPHA_CHNG_FLAG		(0x80)

#define		N_TAILS		(TAILS*2)
#define		N_PRIMS		(N_TAILS/32)
#define		N_VERTS		(32)
#define		TAIL_SIZE	(800)


#define		PI2				(2.0f * PI)

#define		CAR_COLOR		(0x40)
#define		CAR_ALPHA		(0x20)

#define		SCREEN_Z		(51.0f)

#define		BAN_MDL_CODE 	(5320215)//(GV_StrCode("car_ban"))
#define		ODS_MDL_CODE 	(2125632)//(GV_StrCode("car_odessey"))
#define		SKY_MDL_CODE 	(6080859)//(GV_StrCode("car_skyline"))
#define		WGN_MDL_CODE 	(544660)//(GV_StrCode("car_wagon"))
// extern
extern void *NewTraffic_Flush( FVECTOR *m_pos, CVECTOR *rgba );
//----------------------------------------------------
//		呼出の方
//----------------------------------------------------

//#define		MEM_SCR_UNIT 	((void*)(SCRPAD_ADDR))
//#define		MEM_SCR_UNIT	((void*)(MEM_SCR_TEMP + sizeof(FVECTOR)*N_VERTS))

#define			MAKE_TAIL
//#define			MAKE_CARS
#define			MAKE_FLUSH
#define			MAKE_OBJCT

#ifdef MAKE_TAIL
#ifdef BP_PS2
#define			MEM_SCR_POS		((void*)(SCRPAD_ADDR))
#define			MEM_SCR_UV		((void*)(MEM_SCR_POS + sizeof(FVECTOR)*N_TAILS))
#define			MEM_SCR_BPOS	((void*)(MEM_SCR_POS + sizeof(FVECTOR)*N_TAILS))
#define			MEM_SCR_NEXT0	((void*)(MEM_SCR_UV + sizeof(DG_PRIM2_UVRGBWH)*N_TAILS))
#else
#define			MEM_SCR_POS		((void*)(SCRPAD_ADDR))
#define			MEM_SCR_UV		((void*)((char *)MEM_SCR_POS + sizeof(FVECTOR)*N_TAILS))
#define			MEM_SCR_BPOS	((void*)((char *)MEM_SCR_POS + sizeof(FVECTOR)*N_TAILS))
#define			MEM_SCR_NEXT0	((void*)((char *)MEM_SCR_UV + sizeof(DG_PRIM2_UVRGBWH)*N_TAILS))
#endif
#else
#define			MEM_SCR_NEXT0	((void*)(SCRPAD_ADDR))
#endif

#ifdef MAKE_BURN
#ifdef BP_PS2
#define			MEM_SCR_POS1	MEM_SCR_NEXT0
#define			MEM_SCR_UV1		((void*)(MEM_SCR_POS1 + sizeof(FVECTOR)*N_BURNS*4))
#define			MEM_SCR_NEXT1	((void*)(MEM_SCR_UV1 + sizeof(DG_PRIM2_UVRGB)*N_BURNS*4))
#else
#define			MEM_SCR_POS1	MEM_SCR_NEXT0
#define			MEM_SCR_UV1		((void*)((char *)MEM_SCR_POS1 + sizeof(FVECTOR)*N_BURNS*4))
#define			MEM_SCR_NEXT1	((void*)((char *)MEM_SCR_UV1 + sizeof(DG_PRIM2_UVRGB)*N_BURNS*4))
#endif
#else
#define			MEM_SCR_NEXT1	MEM_SCR_NEXT0
#endif

#ifdef BP_PS2
#define			MEM_SCR_UNIT		MEM_SCR_NEXT1
#define			MEM_SCR_POS_ACT		((void*)(MEM_SCR_UNIT - 0x00001000))
#define			MEM_SCR_POS_ACT2 	((void*)(MEM_SCR_UNIT - 0x00001500))
#else
#define			MEM_SCR_UNIT		MEM_SCR_NEXT1
#define			MEM_SCR_POS_ACT		((void*)((char *)MEM_SCR_UNIT - 0x00001000))
#define			MEM_SCR_POS_ACT2 	((void*)((char *)MEM_SCR_UNIT - 0x00001500))
#endif

//#define			MEM_SCR_UNIT	((void*)0x70002c00)
#define			MAX_FLUSH		(12)
typedef struct
{
	FVECTOR		m_pos;			//車
//	FVECTOR		burn_size;		//
#ifdef MAKE_OBJCT
	DG_OBJS		*objs;
#endif
	float		dx;				//
	float		hight;			//
	float		az;				//
	
	float		wz;				//
	float		tz;
	float		z_off;
	CVECTOR		rgba;			//
	
	float		size;			//はなれっぷり
#ifdef MAKE_OBJCT
	float		sclx,scly,sclz;
#endif
} Unit;

typedef struct
{
	GV_ACT_EX		actor;

	int				name;
	DG_PRIM2		*prim;
	DG_PRIM2		*prim1;
	float			z_crip;
	
	int				max_car;
	int				f_speed;
	unsigned int 	timer;
	int				flush_num;

	int				flags;
	int				bright;
	int				pad[2];
	float			x_off;

#ifdef KP_WINDOWS
	int				pre_vcount ;
#endif

	Unit			unit[0];
} Work ;

#ifdef KP_WINDOWS
extern	int		DG_TickCount ;
#endif

static Work *traffic = NULL;
#if 0
static FVECTOR OffSetPos[14]={
	{ 0.0f, 3000.0f,OFFSET_Z0_A,0.0f },
	{ 0.0f, 3000.0f,OFFSET_Z0_BH,0.0f },
	{ 0.0f, 3000.0f,OFFSET_Z0_CI,0.0f },
	{ 0.0f, 3000.0f,OFFSET_Z0_D,0.0f },
	{ 0.0f, 3000.0f,OFFSET_Z0_E,0.0f },
	{ 0.0f, 3000.0f,OFFSET_Z0_F,0.0f },
	{ 0.0f, 3000.0f,OFFSET_Z0_G,0.0f },
	{ 0.0f, 3000.0f,OFFSET_Z1_A,0.0f },
	{ 0.0f, 3000.0f,OFFSET_Z1_BH,0.0f },
	{ 0.0f, 3000.0f,OFFSET_Z1_CI,0.0f },
	{ 0.0f, 3000.0f,OFFSET_Z1_D,0.0f },
	{ 0.0f, 3000.0f,OFFSET_Z1_E,0.0f },
	{ 0.0f, 3000.0f,OFFSET_Z1_F,0.0f },
	{ 0.0f, 3000.0f,OFFSET_Z1_G,0.0f },
};
#endif
static float Offset_z_Table[18] = {
	OFFSET_Z0_A,
	OFFSET_Z0_BH,
	OFFSET_Z0_CI,
	OFFSET_Z0_D,
	
	OFFSET_Z0_E,
	OFFSET_Z0_F,
	OFFSET_Z0_G,
	OFFSET_Z0_BH,
	OFFSET_Z0_CI,
	
	OFFSET_Z1_D,
	OFFSET_Z1_CI,
	OFFSET_Z1_BH,
	OFFSET_Z1_A,
	
	OFFSET_Z1_E,
	OFFSET_Z1_F,
	OFFSET_Z1_G,
	OFFSET_Z1_BH,
	OFFSET_Z1_CI,
};

static float Swing_Table[] = {
	SWING_ONE_LANE,
	SWING_TWO_LANE,
	SWING_THREE_LANE,
};

//メッセージ[0]
enum {
	TRFC_MSG_ON = 0,		//可視
	TRFC_MSG_OFF,			//不可視
	TRFC_MSG_SPEED,			//スピード変更	[1]端から端までのフレーム数（平均  × 1.0 <-> 0.6 ）
	TRFC_MSG_BRIGHT,		//非対応
	TRFC_MSG_KILL,			//こーろーすー
	TRFC_MSG_XOFF,
};

extern int DM_FrameSkip ;

#if 0
static void WriteLane()
{	
	//センターライン
//	float ftemp = LANE_SIZE;
	FVECTOR 	b0,b1;
	FVECTOR		fvtemp[2];
	int i,r = 0x80,g = 0x00;
	
	b0.vx = BOUND0_X;
	b0.vy = BOUND0_Y;
	b0.vz = BOUND0_Z;
	b1.vx = BOUND1_X;
	b1.vy = BOUND0_Y + 3000.0f;//CENTER_LINE_H;
	b1.vz = BOUND1_Z;
	
	NewBoundingBoxView( &b0, &b1, 0x80, 0x80, 0x80 );
#if 0
	//車線表示
	ftemp /= 2;
	fvtemp[0].vx = BOUND0_X;
	fvtemp[1].vx = BOUND1_X;
	fvtemp[0].vy = BOUND0_Y + 10.0f;
	fvtemp[1].vy = BOUND0_Y + 10.0f;
	
	for( i = 0; i < 8; i++ ){
		fvtemp[0].vz = fvtemp[1].vz = BOUND0_Z + ftemp*i;
		NewLineView( fvtemp, 2, r, g, 0x00 );
		fvtemp[0].vz = fvtemp[1].vz = BOUND1_Z - ftemp*i;
		NewLineView( fvtemp, 2, r, g, 0x00 );
		
		r ^= 0x80;
		g ^= 0x80;
	}
#endif
}
#endif

static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		case TRFC_MSG_ON:				//可視
			if(traffic->prim)  DG_VisiblePrim2(work->prim) ;
			if(traffic->prim1) DG_VisiblePrim2(work->prim1) ;
			GV_CallChildSignalFunc( work, 0, 0 );
			work->flags &= ~(TRFC_FLAGS_INVSBL|TRFC_FLAGS_INVSBL1);
			{
				int i;
				Unit *unit = work->unit;
				
				for( i = 0; i < work->max_car; i++ ){
					if(unit->objs) DG_VisibleObjs( unit->objs );
					unit++;
				}	
			}	
			printf("Trfc Msg Visible\n");
			break;
		case TRFC_MSG_OFF:			//不可視
			if(traffic->prim)  DG_InvisiblePrim2(work->prim) ;
			if(traffic->prim1) DG_InvisiblePrim2(work->prim1) ;
			work->flags |= TRFC_FLAGS_INVSBL;
			printf("Trfc Msg Invisible\n");
			break;
		case TRFC_MSG_SPEED:			//スピード変更	[1]端から端までのフレーム数（平均  × 1.0 <-> 0.6 ）

         if ( BP_IsPAL()==TRUE )
			   work->f_speed = msg->message[1] * 50 / 60;
         else
			   work->f_speed = msg->message[1];

         //全体のスピード変更
			{
			    int i;
				Unit *unit = work->unit;
				float	dx;

				dx = (BOUND1_X - BOUND0_X) / work->f_speed;
				for( i = 0; i < work->max_car; i++ ){
					if( work->max_car / 2 > i ){
						unit->dx = -dx * (1.0f - 0.4f * rnd());
					}else{
						unit->dx = dx * (1.0f - 0.4f * rnd());
					}
					unit++;
				}
			}
			printf("Trfc Msg Change Speed\n");
			break;
		case TRFC_MSG_BRIGHT:			//非対応
			printf("Trfc Msg Change Bright\n");
			if(!(work->flags & TRFC_FLAGS_CHNGBRGHT)){
			    work->flags |= TRFC_FLAGS_CHNGBRGHT;
			    work->bright = msg->message[1];
			}else{
				printf("変えてる途中\n");
			}
			break;
		  case TRFC_MSG_KILL:			//こーろーすー
			if(traffic->prim)  DG_InvisiblePrim2(work->prim) ;
			if(traffic->prim1) DG_InvisiblePrim2(work->prim1) ;
			GV_DestroyActor(work) ;
			printf("Trfc Msg Kill\n");
			return 1;
			break;
		case TRFC_MSG_XOFF:
			{
			    float temp = (float)msg->message[1];
			    int i;
				Unit *unit = work->unit;

			    for( i = 0; i < work->max_car; i++ ){
			        unit->m_pos.vx -= work->x_off;
			        unit->m_pos.vx += temp;

			        unit++;
		        }
			    work->x_off = temp;
		    }
			printf("x_off Change[%f]\n",work->x_off);
			break;
		  default:
			printf("Trfc Msg Err!!\n");
			break;
		}
		msg--;
	}

	return 0;
}


#ifdef MAKE_TAIL
static void Act_Tail( Unit *unit, FVECTOR *pos, FVECTOR *b_pos )
{
	FVECTOR	*m_pos = &unit->m_pos,*scr_pos = MEM_SCR_POS_ACT;
	float	size = unit->size;
#if 0
	memmove( &pos[2], b_pos, sizeof(FVECTOR) * (N_TAILS-2) );
	
	pos[0].vx = m_pos->vx;
	pos[0].vy = m_pos->vy;
	pos[0].vz = m_pos->vz + size;
	pos[1].vx = m_pos->vx;
	pos[1].vy = m_pos->vy;
	pos[1].vz = m_pos->vz - size;
#else
	
	TS_Mem_Scr( &scr_pos[2],b_pos,sizeof(FVECTOR),N_TAILS-2);
	
	scr_pos[0].vx = m_pos->vx;
	scr_pos[0].vy = m_pos->vy;
	scr_pos[0].vz = m_pos->vz + size;
	scr_pos[1].vx = m_pos->vx;
	scr_pos[1].vy = m_pos->vy;
	scr_pos[1].vz = m_pos->vz - size;

	TS_Mem_Scr( pos,MEM_SCR_POS_ACT,sizeof(FVECTOR),N_TAILS);	
#endif
}
#endif

void ChangeBright( DG_PRIM2 *prim, int max_car, int max_alpha )
{
	int		i,j;
	float alpha = (float)max_alpha;
	float sub_alpha0 = max_alpha/16.0f;
	float sub_alpha1 = max_alpha/112.0f;
	
	DG_PRIM2_UVRGBWH	*uvrgbwh = prim->uvrgb[prim->buffer_clock];
	
	for( i = 0; i < max_car; i++ ){
		for( j = 0; j < N_TAILS; j++ ){
			if(j < 32){
				uvrgbwh->a = (short)(alpha - (float)(j/2)*sub_alpha0);//2;
			}else{
				uvrgbwh->a = (short)(alpha - (float)((j-32)/2)*sub_alpha1);//2;
			}
			uvrgbwh++;
		}
	}
	
}

static void Act(Work *work)
{
	int 				i,max_car = work->max_car,clock0;//,clock2;
	Unit 				*unit;// = work->unit;
	FVECTOR				dvec,car;
	FVECTOR				*pos,*b_pos;

	//FVECTOR				*car_pos;
	//FVECTOR				*carprim_pos;

	FMATRIX				fmtemp;
//	int					check = 0;
	
//	char				*ppos10,*puvrgb10;
//	char				*ppos11,*puvrgb11;
//	FVECTOR				*l_pos0,*l_pos1,fvtemp0,fvtemp1;
#ifdef KP_WINDOWS
	int					vcount_diff ;
	float				spd_coef ;
#endif

#ifdef KP_WINDOWS
	/* Windowsの場合、フレームに合わせて動かないと困る */
	vcount_diff = DG_TickCount - work->pre_vcount ;
	work->pre_vcount = DG_TickCount ;

	if( vcount_diff < 0 ){ vcount_diff = 0 ; }	// 安全策
	spd_coef = (float)vcount_diff * (1.0f/3.0f) ;
#endif

	work->timer++;
	
	if(CheckMesgParam( work )) return;
	
	if( work->flags & TRFC_FLAGS_INVSBL ){
		if(work->flags & TRFC_FLAGS_INVSBL1) return;
		
		if(traffic->prim)  DG_InvisiblePrim2(work->prim) ;
		if(traffic->prim1) DG_InvisiblePrim2(work->prim1) ;
		GV_CallChildSignalFunc( work, 1, 0 );
		{
			int k;
			Unit *in_unit = work->unit;
			
			for( k = 0; k < work->max_car; k++ ){
				if(in_unit->objs) DG_InvisibleObjs( in_unit->objs );
				in_unit++;
			}
		}
		work->flags |= TRFC_FLAGS_INVSBL1;
	}

#ifdef MAKE_TAIL
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock0 = work->prim->buffer_clock;
	pos = work->prim->pos[clock0];
	b_pos = work->prim->pos[1-clock0];

#endif

#ifdef MAKE_CARS
	DG_SwitchBuffPrim2( work->prim1 );
	clock2 = work->prim1->buffer_clock;
	carprim_pos = work->prim1->pos[clock2];
	car_pos = MEM_SCR_POS_ACT;
	car_pos+=12;
#endif
	
#if 1
	if( work->flags & TRFC_FLAGS_CHNGBRGHT ){
		ChangeBright( work->prim, work->max_car, work->bright);
		if( work->flags & TRFC_FLAGS_CHNGBRGHT1 ){
			work->flags &= ~(TRFC_FLAGS_CHNGBRGHT1|TRFC_FLAGS_CHNGBRGHT);
		}else{
			work->flags |= TRFC_FLAGS_CHNGBRGHT1;
		}
	}
#endif
	
	unit = MEM_SCR_UNIT;
	
	TS_Mem_Scr(MEM_SCR_UNIT,work->unit,sizeof(Unit),max_car);
	
	for( i = 0; i < max_car; i++ ){

		DG_COPY_VEC( &dvec, &unit->m_pos );
#ifndef KP_WINDOWS
		unit->m_pos.vx += unit->dx;//*(float)(DM_FrameSkip+1);
#else
		unit->m_pos.vx += unit->dx * spd_coef ;
#endif
		if( unit->m_pos.vx < BOUND0_X + work->x_off) unit->m_pos.vx = BOUND1_X + work->x_off;
		if( unit->m_pos.vx > BOUND1_X + work->x_off) unit->m_pos.vx = BOUND0_X + work->x_off;

		unit->m_pos.vy = unit->hight + SHIFT_Y;
		unit->m_pos.vz = unit->az * sinf(unit->wz*unit->m_pos.vx+unit->tz) + unit->z_off;
		DG_COPY_VEC( &car, &unit->m_pos );
		car.vy = SHIFT_Y;
		_sceVu0SubVector( &dvec, &unit->m_pos, &dvec );
//AN_Test_Eye2( &unit->m_pos, 1);
#ifdef MAKE_FLUSH
		if(max_car/2 <= i && !(irnd()%64)){
				unit->rgba.cd = work->bright;
			if(work->flush_num < MAX_FLUSH){
				void *child;
				unit->rgba.cd = work->bright;
				child = NewTraffic_Flush(&work->unit[i].m_pos, &unit->rgba);
				if(child){
					GV_SetActorChild(work,child);
					work->flush_num++;
				}	
			}	
		}
#endif
#ifdef MAKE_TAIL

		Act_Tail( unit, pos, b_pos );
	/*	
		if( i == 8 ){
			AN_Test_Eye2(&pos[0],2);
			AN_Test_Eye2(&pos[1],2);
			AN_Test_Eye2(&pos[2],2);
			AN_Test_Eye2(&pos[3],2);
			AN_Test_Eye2(&pos[4],2);
			AN_Test_Eye2(&pos[5],2);
			AN_Test_Eye2(&pos[6],2);
			AN_Test_Eye2(&pos[7],2);
		}
	*/
		pos += N_TAILS;
		b_pos += N_TAILS;

#endif


#ifdef MAKE_CARS
		TS_Scr_Mem( carprim_pos,car_pos,sizeof(FVECTOR),8);

		carprim_pos+=8;
#endif
#ifdef MAKE_OBJCT
		
		if(unit->objs){
			FVECTOR		scale;
			scale.vx = unit->sclx;
			scale.vy = unit->scly;
			scale.vz = unit->sclz;
			scale.vw = 1.0f;
			
			TS_MakeMatrix( &fmtemp, &dvec, &car );
			TS_ScaleMatrix( &unit->objs->world, &fmtemp, &scale);
		}
		//DG_COPY_VEC((FVECTOR*)&unit->objs->world.m[3],&unit->m_pos);
#endif
		unit++;
	}
	TS_Scr_Mem( work->unit,MEM_SCR_UNIT,sizeof(Unit),max_car);

#if 0
	//WriteLane();
	{
		FVECTOR bound0,bound1;

		bound0.vx = BOUND0_X;
		bound0.vy = BOUND0_Y;
		bound0.vz = BOUND0_Z;
		
		bound1.vx = BOUND1_X;
		bound1.vy = BOUND1_Y + 2000.0f;
		bound1.vz = BOUND1_Z;
		//�橋のバンダリ表示
		NewBoundingBoxView( &bound0, &bound1, 0x80, 0x80, 0x80 );
	}
#endif
//	for(i=0;i<14;i++){
//		AN_Test_Eye2(&OffSetPos[i],2);
//	}
}

static void Die(Work *work)
{
	int i;
	Unit	*unit = work->unit;
	if(work->prim) GM_FreePrim2(work->prim);
	if(work->prim1) GM_FreePrim2(work->prim1);
#ifdef MAKE_OBJCT
	for(i = 0; i < work->max_car; i++,unit++ ){
		if(unit->objs) DG_DequeueObjs( unit->objs ), DG_FreeObjs( unit->objs );
	}
#endif
	traffic = NULL;
}

#ifdef MAKE_TAIL
static void InitTailPrim( DG_TEX *tex, CVECTOR *rgba, float size, FVECTOR *m_pos, float dx, float wide, int mode )
{
	int					j;
	short 				u0,v0,u1,v1;
	FVECTOR				*pos,fvtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	
	DG_COPY_VEC( &fvtemp, m_pos );
	fvtemp.vz += wide;
	wide *= 2.0f;
	dx /= 2.0f;
	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	
	pos = MEM_SCR_POS;
	uvrgbwh = MEM_SCR_UV;

	for( j = 0; j < N_TAILS; j++ ){
		
		DG_COPY_VEC(pos,&fvtemp);
		wide *= -1.0f;
		fvtemp.vz += wide;
		fvtemp.vx -= dx;
		
		uvrgbwh->u0 = u0;
		uvrgbwh->v0 = v0;
		uvrgbwh->u1 = u1;
		uvrgbwh->v1 = v1;
		uvrgbwh->r = rgba->r;
		uvrgbwh->g = rgba->g;
		uvrgbwh->b = rgba->b;
//		uvrgbwh->a = MAX_ALPHA - j/(2*SUB_ALPHA);//2;
		
		uvrgbwh->q0 = 4096;
		uvrgbwh->q1 = 4096;
		uvrgbwh->f0 = 0;
		uvrgbwh->f1 = 0;
		uvrgbwh->w = uvrgbwh->h = size;
#if 0
		if(j < 32){
			uvrgbwh->a = MAX_ALPHA - (j/2)*2;//2;
		}else{
			uvrgbwh->a = MAX_ALPHA - (j-32)/(2*3);//2;
		}
#else
#define	MAX_ALPHA_TEST	(24.0f)
#define	SUB_ALPHA_TEST0	(MAX_ALPHA_TEST/16.0f)
#define	SUB_ALPHA_TEST1	(MAX_ALPHA_TEST/112.0f)
		
		if(j < 32){
			uvrgbwh->a = (short)(MAX_ALPHA_TEST - (float)(j/2)*SUB_ALPHA_TEST0);//2;
		}else{
			uvrgbwh->a = (short)(MAX_ALPHA_TEST - (float)((j-32)/2)*SUB_ALPHA_TEST1);//2;
		}
//		if(!mode) uvrgbwh->a = 0;
#endif
		pos++;
		uvrgbwh++;
	}
	//ASSERT(0)
//	printf("\n");
}
#endif

#ifdef MAKE_CARS
static void InitCarsPrim( DG_TEX *tex, float size )
{
	int 				i;
	short 				u0,v0,u1,v1;
	DG_PRIM2_UVRGBWH 	*uvrgbwh = MEM_SCR_UV;

	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	for( i = 0; i < 8; i++ ){

		uvrgbwh->u0 = u0;
		uvrgbwh->v0 = v0;
		uvrgbwh->u1 = u1;
		uvrgbwh->v1 = v1;

		uvrgbwh->r = DG_FogColor.r;//CAR_COLOR;
		uvrgbwh->g = DG_FogColor.g;//CAR_COLOR;
		uvrgbwh->b = DG_FogColor.b;//CAR_COLOR;
		
		uvrgbwh->a = CAR_ALPHA;

		uvrgbwh->q0 = 4096;
		uvrgbwh->q1 = 4096;
		uvrgbwh->f0 = 0;
		uvrgbwh->f1 = 0;

		uvrgbwh->w = uvrgbwh->h = size;
		
		uvrgbwh++;
	}
	
}
#endif

#ifdef MAKE_OBJCT
static int GetObjsData( Unit *unit, int a, int i )
{
	DG_DEF		*def = NULL;
	DG_OBJS		*objs = NULL;
	int			index = (irnd()>>16)%3;
	
	int			Modele_Code[] = {
		WGN_MDL_CODE,
		ODS_MDL_CODE,
		SKY_MDL_CODE,
		BAN_MDL_CODE,
	};
	if( (a == 0 && i == 0) || (a == 0 && i == 32) ) index = 3;
	def = (DG_DEF*)GV_GetCache( GV_CacheID( Modele_Code[index], 'k' ) ) ;
	if(!def){
		printf("ERR!! NO MODEL!!\n");
		return -1;
	}
//	printf("index %d\n",index);
	objs = unit->objs = DG_MakeObjs( def, DG_FLAG_SHADE, 0 );
	if(!objs) return -1;

	if(DG_QueueObjs( unit->objs )<0) return -1;
//	DG_SetLightMatrix( work->objs, work->lights );
	
	unit->sclx = 1.0f + frnd()*0.1f;
	unit->scly = 1.0f + frnd()*0.1f;
	unit->sclz = 1.0f + frnd()*0.1f;

	return 0;
}

static void InitObjsMat( Unit *unit, FVECTOR *pos )
{
	FVECTOR		vec = { 1.0f, 0.0f, 0.0f};
	FVECTOR		scale = { 1.0f, 1.0f, 1.0f, 1.0f};
	FMATRIX		mat;
	
	TS_MakeMatrix( &mat, &vec ,pos );
	TS_ScaleMatrix( &unit->objs->world, &mat, &scale);
}

#endif
static void InitWorkData( Work *work, DG_TEX *tex )
{
	int 				i, max_car = work->max_car,a;
	int					n_car_obj = 18;//work->max_car/3;
//	short 				u0,v0,u1,v1;
	Unit				*unit;// = wotk->unit;
	float				ftemp,dx,tail_size,swing;	
//	FVECTOR				*pos,*b_pos,*init_pos = MEM_SCR_POS;
	FVECTOR				*ppooss = MEM_SCR_POS;
//	DG_PRIM2_UVRGBWH	*uvrgbwh,*b_uvrgbwh;
	
	char				*ppos00,*puvrgb00;
	char				*ppos01,*puvrgb01;

#ifdef MAKE_TAIL
	ppos00 = (char*)work->prim->pos[0];
	puvrgb00 = (char*)work->prim->uvrgb[0];
	ppos01 = (char*)work->prim->pos[1];
	puvrgb01 = (char*)work->prim->uvrgb[1];
#endif
	
#ifdef MAKE_CARS
	init_pos += 12;
	pos = work->prim1->pos[0];
	uvrgbwh = work->prim1->uvrgb[0];
	b_pos = work->prim1->pos[1];
	b_uvrgbwh = work->prim1->uvrgb[1];
#endif
	ppooss += 16;
//	unit = work->unit;
	unit = MEM_SCR_UNIT;
	
	dx = (BOUND1_X - BOUND0_X) / work->f_speed;
	
	for( i = 0; i < max_car; i++ ){
		
		unit->hight = 800.0f + 800.0f*rnd();

	
//		unit->m_pos.vx = ftemp = BOUND0_X + dx * (irnd()%work->f_speed);
//		unit->m_pos.vy = unit->hight + SHIFT_Y;
//		unit->m_pos.vz = unit->az * sinf(unit->wz*ftemp+unit->tz);
		unit->size = rnd()*480.0f + 1000.0f;

//		unit->burn_size.vx = BURN_SIZE + TAIL_SIZE;
//		unit->burn_size.vy = BURN_SIZE + TAIL_SIZE;
//		unit->burn_size.vz = 0.0f;
//		unit->burn_size.vw = 1.0f;
		
//		unit->off_z;
		swing = frnd();
		if( i%(max_car / 2) < 24 ){
			unit->az = ( Swing_Table[0] - 500.0f ) * swing;
		}else if( i%(max_car / 2) < 29 ){
			unit->az = ( Swing_Table[0] - 500.0f ) * swing + ( swing>0.0f ? SWING_ONE_LANE: -SWING_ONE_LANE );
		}else{
			unit->az = ( Swing_Table[1] - 500.0f ) * swing + ( swing>0.0f ? SWING_ONE_LANE: -SWING_ONE_LANE );
		}
#if 0
		if( max_car / 2 > i ){
			a = i%(max_car / 2);
			//back light
			unit->dx = -dx * (1.0f - 0.4f * rnd());
			unit->rgba.r = 0x60;// + irnd() % 96;
			unit->rgba.g = unit->rgba.b = 0x10 + irnd() % 48;
			//unit->rgba.b = 0x20 + irnd() % 48;
			tail_size = TAIL_SIZE - 180.0f * frnd();

			if(a < 28){
				a /= 4;
				unit->z_off = Offset_z_Table[a];
			}else{
				if( a < 30 ){
					unit->z_off = Offset_z_Table[7];
				}else{
					unit->z_off = Offset_z_Table[8];
				}
			}
		}else{
			a = i%(max_car / 2);
			//head light
			unit->dx =  dx * (1.0f - 0.4f * rnd());
			unit->rgba.r = unit->rgba.g = 0x40 + irnd() % 64;
	//		unit->rgba.g = 0x10 + irnd() % 64;
			unit->rgba.b = unit->rgba.r - 0x20;//0x10 + irnd() % 64;
			tail_size = TAIL_SIZE + 240.0f * frnd();
			if(a < 28){
				a /= 4;
				unit->z_off = Offset_z_Table[a+9];
			}else{
				if( a < 30 ){
					unit->z_off = Offset_z_Table[16];
				}else{
					unit->z_off = Offset_z_Table[17];
				}
			}
		}
#else
		unit->rgba.cd = (int)MAX_ALPHA_TEST;
	
		if( max_car / 2 > i ){
			a = i%(max_car / 2);//(0->31)
			//back light
			unit->dx = -dx * (1.0f - 0.4f * rnd());
			
			if(i&1){
				unit->rgba.r = 0x60;// + irnd() % 96;
				unit->rgba.g = 0x20 + irnd() % 48;
				unit->rgba.b = 0x10 + irnd() % 48;
			}else{
				unit->rgba.r = unit->rgba.g = 0x40 + irnd() % 64;
				//unit->rgba.g = 0x10 + irnd() % 64;
				unit->rgba.b = unit->rgba.r - 0x20;//0x10 + irnd() % 64;
			}
		
			tail_size = TAIL_SIZE - 180.0f * frnd();
			
			if(a < 24){
				a %= 7;
				unit->z_off = Offset_z_Table[a];
			}else{
				if( a < 29 ){
					unit->z_off = Offset_z_Table[7];
				}else{
					unit->z_off = Offset_z_Table[8];
				}
			}
		}else{
			a = i%(max_car / 2);
			//head light
			unit->dx =  dx * (1.0f - 0.4f * rnd());
			unit->rgba.r = unit->rgba.g = 0x40 + irnd() % 64;
	//		unit->rgba.g = 0x10 + irnd() % 64;
			unit->rgba.b = unit->rgba.r - 0x20;//0x10 + irnd() % 64;
			tail_size = TAIL_SIZE + 240.0f * frnd();
			if(a < 24){
				a %= 7;
				unit->z_off = Offset_z_Table[a+9];
			}else{
				if( a < 29 ){
					unit->z_off = Offset_z_Table[16];
				}else{
					unit->z_off = Offset_z_Table[17];
				}
			}
		}
#endif
		unit->wz = PI2 / (( 1.5f * rnd() + 2.5f ) * BOUND_L );
		unit->tz = PI2 * rnd();
		
		unit->m_pos.vx = ftemp = BOUND0_X + dx * (irnd()%work->f_speed);
		unit->m_pos.vy = unit->hight + SHIFT_Y;
		unit->m_pos.vz = unit->az * sinf(unit->wz*ftemp+unit->tz) + unit->z_off;
		unit->size = rnd()*480.0f + 480.0f;
		
#ifdef MAKE_TAIL
		InitTailPrim( tex, &unit->rgba, tail_size, &unit->m_pos, unit->dx, unit->size, i/32 );
		TS_Scr_Mem( ppos00,MEM_SCR_POS,sizeof(FVECTOR),N_TAILS);
		TS_Scr_Mem( ppos01,MEM_SCR_POS,sizeof(FVECTOR),N_TAILS);
		TS_Scr_Mem( puvrgb00,MEM_SCR_UV,sizeof(DG_PRIM2_UVRGBWH),N_TAILS);
		TS_Scr_Mem( puvrgb01,MEM_SCR_UV,sizeof(DG_PRIM2_UVRGBWH),N_TAILS);
		ppos00 += sizeof(FVECTOR) * N_TAILS;
		puvrgb00 += sizeof(DG_PRIM2_UVRGBWH) * N_TAILS;
		ppos01 += sizeof(FVECTOR) * N_TAILS;
		puvrgb01 += sizeof(DG_PRIM2_UVRGBWH) * N_TAILS;
#endif

#ifdef MAKE_CARS
		InitCarsPrim( tex, tail_size*3.0f );
		TS_Scr_Mem( pos,init_pos,sizeof(FVECTOR),8);
		TS_Scr_Mem( b_pos,init_pos,sizeof(FVECTOR),8);
		TS_Scr_Mem( uvrgbwh,MEM_SCR_UV,sizeof(DG_PRIM2_UVRGBWH),8);
		TS_Scr_Mem( b_uvrgbwh,MEM_SCR_UV,sizeof(DG_PRIM2_UVRGBWH),8);
		
		pos+=8;
		b_pos+=8;
		uvrgbwh+=8;
		b_uvrgbwh+=8;
#endif
#ifdef MAKE_OBJCT
		if(n_car_obj && (i >= max_car/2 || n_car_obj > 9) && a < 4 ){
				GetObjsData( unit, a, i );
				InitObjsMat( unit, ppooss );
				n_car_obj--;
//				printf("i[%d]a[%d]n_car[%d]\n",i,a,n_car_obj);
		}else{
			unit->objs = NULL;
		}
#endif
		unit++;
	}
	TS_Scr_Mem( work->unit,MEM_SCR_UNIT,sizeof(Unit),work->max_car);
	
}

static int ReceiveSignal( void *pwork, int signal, int value )
{
	Work *work = pwork;

	switch(signal){
	  case 0:
		work->flush_num--;
		break;
	  default:
		return GV_DefaultSignalFunc( pwork, signal, value );
	}
	return 0;
	//printf("flush_num = %d\n",work->flush_num);
}

extern void init_rnd( int x );

static int GetResources( Work *work, int name, float z_crip, int max_car, int f_speed )
{
	
	DG_PRIM2	*prim = NULL;
	DG_TEX		*tex = NULL;

	work->prim = NULL;
	work->prim1 = NULL;
	
init_rnd( (int)z_crip );

	tex = DG_GetTexture(3594043);//GV_StrCode("drop01_msk"));
	//tex = DG_GetTexture(GV_StrCode("rcm_l_msk"));
	if(!tex){
		printf(" not texture \n");
		return(-1);
	}
#ifdef MAKE_TAIL
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
									  N_PRIMS * max_car,
									  N_VERTS );
	if(!prim){
		printf("null prim\n");
		return (-1);
	}
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
#endif
	
#ifdef MAKE_CARS
	prim = work->prim1 = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA | DG_PRIM2_FOG,
									  16,//64car*2lane*4verts
									  32 );
	if(!prim){
		printf("null prim\n");
		return (-1);
	}
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
#endif
	// DataInit
	work->name = name;
	work->timer = 0;
	work->max_car = max_car;

   if ( BP_IsPAL()==TRUE )
	   work->f_speed = f_speed * 50 / 60;
   else
	   work->f_speed = f_speed;

   work->z_crip = z_crip;
	work->flush_num = 0;
	work->flags = 0;
	work->bright = 24;
	work->x_off = 0.0f;

//	printf("SYS_SCR_BOTTOM = %x\n",SYS_SCR_BOTTOM);
//	printf("PRO_SCR_BOTTOM = %x\n",MEM_SCR_UNIT + sizeof(Unit)*max_car);
//	printf("MEM_SCR_UNIT   = %x\n",MEM_SCR_UNIT);
//	printf("sizeof(Unit) = %d\n",sizeof(Unit));	
	InitWorkData( work, tex );

	GV_SetActorSignalFunc( work, ReceiveSignal );

	//DG_InvisiblePrim2(work->prim);
	//DG_InvisiblePrim2(work->prim1);

#ifdef KP_WINDOWS
	work->pre_vcount = DG_TickCount ;
#endif

	traffic = work;

	return (0);
}

void KillTraffic6()
{
	if(!traffic) return;
	
	if(traffic->prim)  DG_InvisiblePrim2(traffic->prim) ;
	if(traffic->prim1) DG_InvisiblePrim2(traffic->prim1) ;

	GV_DestroyActor( traffic );
}

/*
    int			name:
	float		z_crip,				// 以上離れると焼き付かない
	int			max_car,			// 車の最大数
	int 		f_speed,			// 端から端までのフレーム数（平均  × 1.0 <-> 0.6 ）
*/
void *NewTraffic_Demo( int name, float z_crip, int max_car, int f_speed )
{
	Work *work = NULL;
	int	size;
	
	size = sizeof(Work) + sizeof(Unit) * 64;

	work = (Work*)GV_NewEffect( GV_ACTOR_USER, size );
	if( work ){
		GV_SetActor( &work->actor,Act,Die );
		GV_ActorEX( &work->actor )
		//if(GetResources( work, z_crip, max_car, f_speed ) < 0){
		if(GetResources( work, name, z_crip, max_car, f_speed ) < 0){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	printf("traffic set ok\n");
	return (void *)work ;
}
