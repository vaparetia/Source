//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	lens_flare.c
	デモ中レンズフレア
	2000/12/14 T.Shibata
	
	$Id: lens_flare.c,v 1.1.1.3 2002/11/19 11:48:32 Yoshizawa1 Exp $
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

#include	"../util/ts_util.h"

#define		SCREEN_Z	(51.0f)

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)
#define		GET_SCREEN_TRAN_X(_cam) (SCREEN_Z / (ASPECT_X() ) / DG_Chanls[(_cam)].screen)
#define		GET_SCREEN_TRAN_Y(_cam) (SCREEN_Z / (ASPECT_Y() * DG_Chanls[(_cam)].width / \
												 DG_Chanls[(_cam)].height) / DG_Chanls[(_cam)].screen)

//11*12 == 132
//44 * 3
//(N_PRIMS*N_VERTS)==(N_STRIP_VERTS*N_STRIPS)

#define		N_PRIMS_F			(1)
#define		N_VERTS_F			(4)
#define		N_ALLVERTS_F		(N_VERTS_F*N_PRIMS_F)

#define		N_FLARES			(24)
#define		N_FLARE_VERTS		(4)

#define	   	MEM_SCR_UV	((void*)(SCRPAD_ADDR))
#define	   	MEM_SCR_POS	((void*)(MEM_SCR_UV + sizeof(DG_PRIM2_UVRGB)*N_ALLVERTS_F))
#define	   	MEM_SCR_TMP	((void*)(MEM_SCR_POS + sizeof(FVECTOR)*N_ALLVERTS_F))

#define		ALPHA		(16.0f)

enum {
	MSG_INV = 0,
	MSG_VSB,
};

typedef struct {
	GV_ACT_EX		actor;
	FVECTOR			center;
	DG_PRIM2		*flare[N_FLARES];
//	DG_PRIM2		*halation;
	int				name;
	int				flags;
	float			alpha_ratio;
	float			ratio_deluta;
} Work;

/* -------------------------------------------------------------- */
//	flare_b1_msk	14071822	真中明るい
//	flare_b2_msk	15120398 	べつに明るくない
//	flare_g1_msk	14071832 	真中明るい
//	flare_g2_msk	15120408   	べつに明るくない
//	flare_r1_msk	14071854 	真中明るい
//	flare_r2_msk	15120430  	べつに明るくない

#define N_TEXKIND	(sizeof(FlareTexCode)/sizeof(FlareTexCode[0]))
static int FlareTexCode[]={	
	14071822,
	14071854,
	14071832,
	15120430,
	15120398,

//	14071854,
	14071832,
	14071822,
	14071822,
	14071832,
//	14071854,
	
	15120408,
	14071822,
	14071832,
	15120408,

	15120430,
	15120408,
	15120398,
	15120398,
	15120408,
	15120430,

	14071832,
	15120398,
	14071854,
};

#define	TRAN_SCR_SIZE(_size) ((_size)*(float)(DRAW_WIDTH)/(float)(DRAW_HEIGHT))
//ＶＸ、ＶＹ：サイズ
//ＶＷ：光源から中心を1.0fとした時の光源からの距離
static FVECTOR FlareData[N_FLARES]={
	{ 1.8f, TRAN_SCR_SIZE(1.8f), 0.0f, -2.5f },
	{ 1.3f, TRAN_SCR_SIZE(1.3f), 0.0f, -1.6f },
	{ 0.4f, TRAN_SCR_SIZE(0.4f), 0.0f, -0.7f },
	{ 0.2f, TRAN_SCR_SIZE(0.2f), 0.0f, -0.5f },
	{ 0.3f, TRAN_SCR_SIZE(0.3f), 0.0f, -0.3f },
	
	{ 0.5f, TRAN_SCR_SIZE(0.5f), 0.0f, -0.0005f },		//太陽付近
	{ 0.8f, TRAN_SCR_SIZE(0.8f), 0.0f, -0.0001f },		//太陽付近
	{ 0.4f, TRAN_SCR_SIZE(0.4f), 0.0f, -0.0f },			//太陽付近
	{ 0.4f, TRAN_SCR_SIZE(0.4f), 0.0f,  0.0f },			//太陽付近
	{ 0.8f, TRAN_SCR_SIZE(0.8f), 0.0f,  0.0001f },		//太陽付近
	{ 0.5f, TRAN_SCR_SIZE(0.5f), 0.0f,  0.0005f },		//太陽付近
	
	{ 0.2f, TRAN_SCR_SIZE(0.2f), 0.0f,  1.2f },
	{ 0.3f, TRAN_SCR_SIZE(0.3f), 0.0f,  1.4f },
	{ 0.8f, TRAN_SCR_SIZE(0.8f), 0.0f,  1.5f },
	{ 0.3f, TRAN_SCR_SIZE(0.3f), 0.0f,  1.8f },
	
	{ 1.2f, TRAN_SCR_SIZE(1.2f), 0.0f,  1.99998f },		//逆太陽付近
	{ 0.8f, TRAN_SCR_SIZE(0.8f), 0.0f,  1.99999f },		//逆太陽付近
	{ 0.4f, TRAN_SCR_SIZE(0.4f), 0.0f,  2.0f },			//逆太陽付近
	{ 0.4f, TRAN_SCR_SIZE(0.4f), 0.0f,  2.0f },			//逆太陽付近
	{ 0.8f, TRAN_SCR_SIZE(0.8f), 0.0f,  2.00001f },		//逆太陽付近
	{ 1.2f, TRAN_SCR_SIZE(1.2f), 0.0f,  2.00002f },		//逆太陽付近
	
	{ 0.7f, TRAN_SCR_SIZE(0.7f), 0.0f,  3.0f },
	{ 0.3f, TRAN_SCR_SIZE(0.3f), 0.0f,  6.0f },
	{ 1.7f, TRAN_SCR_SIZE(1.7f), 0.0f,  10.0f }
};

/* -------------------------------------------------------------- */


#if 1
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
		  case MSG_INV:
			work->flags = 1;
			work->ratio_deluta = msg->message[1]/10000;
			if( work->ratio_deluta <= 0.0f ){
				work->ratio_deluta = 0.0625;
			}
			break;
		  case MSG_VSB:
			work->flags = 0;
			work->ratio_deluta = msg->message[1]/10000;
			if( work->ratio_deluta <= 0.0f ){
				work->ratio_deluta = 0.0625;
			}
			break;
		  default:
			printf("lens_flare.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}
	return 0;
}
#endif

static void Flare_Invisible(Work *work)
{
	int i;	
	for( i = 0; i < N_FLARES; i++ ){
		if(work->flare[i]) DG_InvisiblePrim2(work->flare[i]);
	}
}

static void Flare_Visible(Work *work)
{
	int i;
	for( i = 0; i < N_FLARES; i++ ){
		if(work->flare[i]) DG_VisiblePrim2(work->flare[i]);
	}
}
extern int DM_FrameSkip ;

static void Act(Work *work)
{
	FVECTOR					*pos,center,m_pos;
	DG_PRIM2_UVRGB			*uvrgb;
	int 					i,clock;
	float					screen_near_x,screen_near_y,scr_pos_x,scr_pos_y;
	float					alpha = ALPHA,temp,temp_x,temp_y;


	CheckMesgParam( work );

	if(work->flags){
		work->alpha_ratio -= (work->ratio_deluta*(float)(DM_FrameSkip+1));
		if(work->alpha_ratio < 0.0f){
			work->alpha_ratio = 0.0f;
		}
	}else{
		work->alpha_ratio += (work->ratio_deluta*(float)(DM_FrameSkip+1));
		if(work->alpha_ratio > 1.0f){
			work->alpha_ratio = 1.0f;
		}
	}
	
	DG_SetPos( &DG_Chanls[0].eye_pers );
	DG_PutVector( &work->center, &m_pos, 1 );

	//カメラに入ってる？
	//座標で計算
	scr_pos_x = m_pos.vx / m_pos.vw;
	scr_pos_y = m_pos.vy / m_pos.vw;

	temp_x = fpu_Abs(scr_pos_x);
	temp_y = fpu_Abs(scr_pos_y);
	
	if( m_pos.vz >= m_pos.vw || temp_x > 1.4f || temp_y > 1.4f){
		Flare_Invisible(work);
		//printf("out of camera\n");
		return ;
	}

	screen_near_x = GET_SCREEN_TRAN_X(0);
	screen_near_y = GET_SCREEN_TRAN_Y(0);
    //-1.0f<=>1.0f
	temp = temp_x - 1.1f;
	if(temp > 0.0f){
		alpha = ALPHA - temp*ALPHA/0.3f;
	}
	temp = temp_y - 1.1f;
	if(temp > 0.0f){
		alpha = ALPHA - temp*ALPHA/0.3f;
	}

	if( alpha <= 0.0f ){
		Flare_Invisible(work);
		printf("cannot see\n");
		return;
	}
	Flare_Visible(work);
	for( i = 0; i < N_FLARES; i++ ){

		DG_SwitchBuffPrim2( work->flare[i] );
		clock = work->flare[i]->buffer_clock;
			
		pos = work->flare[i]->pos[clock];
		uvrgb = work->flare[i]->uvrgb[clock];
		
		center.vx = scr_pos_x + (FlareData[i].vw * -scr_pos_x);
		center.vy = scr_pos_y + (FlareData[i].vw * -scr_pos_y);
		
		pos[0].vx = screen_near_x * (center.vx - FlareData[i].vx);
		pos[0].vy = screen_near_y * (center.vy - FlareData[i].vy);
		pos[0].vz = SCREEN_Z;
		pos[1].vx = screen_near_x * (center.vx + FlareData[i].vx);
		pos[1].vy = screen_near_y * (center.vy - FlareData[i].vy);
		pos[1].vz = SCREEN_Z;
		pos[2].vx = screen_near_x * (center.vx - FlareData[i].vx);
		pos[2].vy = screen_near_y * (center.vy + FlareData[i].vy);
		pos[2].vz = SCREEN_Z;
		pos[3].vx = screen_near_x * (center.vx + FlareData[i].vx);
		pos[3].vy = screen_near_y * (center.vy + FlareData[i].vy);
		pos[3].vz = SCREEN_Z;

		uvrgb[0].a = uvrgb[1].a = uvrgb[2].a = uvrgb[3].a = (int)(alpha*work->alpha_ratio);
//		uvrgb[1].a = (int)alpha*ratio;
//		uvrgb[2].a = (int)alpha*ratio;
//		uvrgb[3].a = (int)alpha*ratio;

	}
	
	//printf("You can see\n");
	
}

static void Die(Work *work)
{
	int i;

	for( i = 0; i < N_FLARES; i++)
		if(work->flare[i])GM_FreePrim2(work->flare[i]);
	
//	if(work->halation)GM_FreePrim2(work->halation);
}

//レンズフレア
#define FLARE_FLAG (DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_ON_CAMERA)
static int InitFlarePrim( Work *work )
{
	DG_PRIM2			*prim;
	DG_TEX				*tex;
	DG_PRIM2_UVRGB		*uvrgb0,*uvrgb1;
	FVECTOR				*pos0,*pos1;
	int					i,j,u[2],v[2];

	for( i = 0; i < N_FLARES; i++ ){
		tex = DG_GetTexture(FlareTexCode[i%N_TEXKIND]);
		//tex = DG_GetTexture(6715088);
		if(!tex) { printf("ERR!! NO TEX!! <plant_sun.c>\n"); return(-1); }
		prim = work->flare[i] = GM_MakePrim2( FLARE_FLAG,N_PRIMS_F,N_VERTS_F );
		if(!prim){ printf("ERR!! MAKE PRIM!! <plant_sun.c>\n"); return (-1); }
		prim->flag |= DG_PRIM2_INVISIBLE;
		DG_ConfigPrim2Tex( prim, tex );
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

		u[0] = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		v[0] = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		u[1] = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		v[1] = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		
		pos0 = prim->pos[0];
		pos1 = prim->pos[1];
		uvrgb0 = prim->uvrgb[0];
		uvrgb1 = prim->uvrgb[1];

		for( j = 0; j < N_FLARE_VERTS; j++ ){
			DG_COPY_VEC(pos0,&DG_ZeroVector);
			DG_COPY_VEC(pos1,&DG_ZeroVector);
			
			uvrgb0->u = uvrgb1->u = u[j&1];
			uvrgb0->v = uvrgb1->v = v[j>>1];
			uvrgb0->f = uvrgb1->f = (j<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
			uvrgb0->q = uvrgb1->q = 4096;
			uvrgb0->r = uvrgb1->r = 128;
			uvrgb0->g = uvrgb1->g = 128;
			uvrgb0->b = uvrgb1->b = 128;
			uvrgb0->a = uvrgb1->a = (int)ALPHA;

			uvrgb0++; uvrgb1++;
			pos0++; pos1++;

		}

	}
	return 0;
}

static int GetResources( Work *work, FVECTOR *center )
{
	DG_COPY_VEC( &work->center, center );
	if(InitFlarePrim( work ))return -1;
	work->alpha_ratio = 1.0f;
	
	return 0;
}

void *NewLensFlare_Demo( int name, FVECTOR *center )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );

		work->name = name;
		if(GetResources( work, center ) < 0){
			GV_DestroyActor( work );
			return NULL;
		}
		printf("new lens_flare.c\n");
	}

	return (void *)work;
}
