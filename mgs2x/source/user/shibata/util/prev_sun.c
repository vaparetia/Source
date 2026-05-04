//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    plant_sun.c
	ゲーム中の太陽
	2000/12/14 T.Shibata
	
	$Id: prev_sun.c,v 1.1.1.3 2002/11/19 11:48:54 Yoshizawa1 Exp $

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

#define		N_FLARES			(16)
#define		N_FLARE_VERTS		(4)

#define	   	MEM_SCR_UV	((void*)(SCRPAD_ADDR))
#define	   	MEM_SCR_POS	((void*)(MEM_SCR_UV + sizeof(DG_PRIM2_UVRGB)*N_ALLVERTS_F))
#define	   	MEM_SCR_TMP	((void*)(MEM_SCR_POS + sizeof(FVECTOR)*N_ALLVERTS_F))

#define		ALPHA		(18.0f)
#define				SUN_SCALE	(32.0f)
//#define			SUN_SCALE	(1.0f)


typedef struct {
	GV_ACT_EX		actor;
	FVECTOR			center;
	DG_PRIM2		*sun;
	DG_PRIM2		*flare[N_FLARES];
	int				now_tex;
	FMATRIX			*root;
	int				*flags;
	float			*w,*h;
} Work;



static int InitSunPrim( Work *work, int code );
/* -------------------------------------------------------------- */
//	flare_b1_msk	14071822	真中明るい
//	flare_b2_msk	15120398 	べつに明るくない
//	flare_g1_msk	14071832 	真中明るい
//	flare_g2_msk	15120408   	べつに明るくない
//	flare_r1_msk	14071854 	真中明るい
//	flare_r2_msk	15120430  	べつに明るくない

#define N_TEXKIND	(sizeof(FlareTexCode)/sizeof(FlareTexCode[0]))
static int FlareTexCode[N_FLARES]={	
	14071822,
	14071854,
	14071832,
//	15120430,
	15120398,

//	14071854,
	14071832,
//	14071822,
	14071822,
	14071832,
//	14071854,
	
	15120408,
	14071822,
//	14071832,
	15120408,

//	15120430,
	15120408,
	15120398,
	15120398,
	15120408,
//	15120430,

	14071832,
//	15120398,
	14071854,
};

#define	TRAN_SCR_SIZE(_size) ((_size)*(float)(DRAW_WIDTH)/(float)(DRAW_HEIGHT))
//ＶＸ、ＶＹ：サイズ
//ＶＷ：光源から中心を1.0fとした時の光源からの距離
static FVECTOR FlareData[N_FLARES]={
	{ 1.6f, TRAN_SCR_SIZE(1.6f), 0.0f, -2.5f },
	{ 1.3f, TRAN_SCR_SIZE(1.3f), 0.0f, -1.6f },
	{ 0.4f, TRAN_SCR_SIZE(0.4f), 0.0f, -0.7f },
//	{ 0.2f, TRAN_SCR_SIZE(0.2f), 0.0f, -0.5f },
	{ 0.3f, TRAN_SCR_SIZE(0.3f), 0.0f, -0.3f },
	
//	{ 0.5f, TRAN_SCR_SIZE(0.5f), 0.0f, -0.0005f },		//太陽付近
	{ 0.8f, TRAN_SCR_SIZE(0.8f), 0.0f, -0.0001f },		//太陽付近
//	{ 0.4f, TRAN_SCR_SIZE(0.4f), 0.0f, -0.0f },			//太陽付近
	{ 0.4f, TRAN_SCR_SIZE(0.4f), 0.0f,  0.0f },			//太陽付近
	{ 0.8f, TRAN_SCR_SIZE(0.8f), 0.0f,  0.0001f },		//太陽付近
//	{ 0.5f, TRAN_SCR_SIZE(0.5f), 0.0f,  0.0005f },		//太陽付近
	
	{ 0.2f, TRAN_SCR_SIZE(0.2f), 0.0f,  1.2f },
	{ 0.3f, TRAN_SCR_SIZE(0.3f), 0.0f,  1.4f },
//	{ 0.8f, TRAN_SCR_SIZE(0.8f), 0.0f,  1.5f },
	{ 0.3f, TRAN_SCR_SIZE(0.3f), 0.0f,  1.8f },
	
//	{ 1.2f, TRAN_SCR_SIZE(1.2f), 0.0f,  1.99998f },		//逆太陽付近
	{ 0.8f, TRAN_SCR_SIZE(0.8f), 0.0f,  1.99999f },		//逆太陽付近
	{ 0.4f, TRAN_SCR_SIZE(0.4f), 0.0f,  2.0f },			//逆太陽付近
	{ 0.4f, TRAN_SCR_SIZE(0.4f), 0.0f,  2.0f },			//逆太陽付近
	{ 0.8f, TRAN_SCR_SIZE(0.8f), 0.0f,  2.00001f },		//逆太陽付近
//	{ 1.2f, TRAN_SCR_SIZE(1.2f), 0.0f,  2.00002f },		//逆太陽付近
	
	{ 0.7f, TRAN_SCR_SIZE(0.7f), 0.0f,  3.0f },
//	{ 0.3f, TRAN_SCR_SIZE(0.3f), 0.0f,  6.0f },
	{ 1.7f, TRAN_SCR_SIZE(1.7f), 0.0f,  10.0f }
};

static int SunTexCode[] = {
	6241701,			//GV_StrCode("sky_sun_m_alp")
	13581732,			//GV_StrCode("sky_sun_d_alp")
	14630308,			//GV_StrCode("sky_sun_e_alp")
	7290277,			//GV_StrCode("sky_sun_n_alp")
	12533157,			//GV_StrCode("sky_sun_s_alp")
};

/* -------------------------------------------------------------- */


#if 0
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
		default:
			printf("morn_sky.c Msg Err!![%d]\n",num);
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

static void SetSunPos( Work *work )
{
	FVECTOR vec = { 
		DG_LightMatrix.m[ 0 ][ 0 ],
		DG_LightMatrix.m[ 1 ][ 0 ],
		DG_LightMatrix.m[ 2 ][ 0 ],
		0.0f
	};
	_sceVu0ScaleVector(&work->center,&vec,-9000.0f*SUN_SCALE);
	DG_SetPos( work->root );
	DG_PutVector(&work->center,&work->center,1);
}

static void SetSunPrimPos( DG_PRIM2 *prim, FVECTOR *set_pos, float w, float h )
{
	int		clock;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	uvrgbwh = prim->uvrgb[clock];

	DG_COPY_VEC( &prim->pos[clock][0], set_pos );

	uvrgbwh->w = (short)(w * SUN_SCALE);
	uvrgbwh->h = (short)(h * SUN_SCALE);
	
}

static void Flare_Act(Work *work)
{
	FVECTOR				*pos,center,m_pos;
	//FVECTOR				fvtemp,*eye_pos = (FVECTOR*)DG_Chanls[0].eye.m[3];
	//FVECTOR				*eye_for = (FVECTOR*)DG_Chanls[0].eye.m[2];
	DG_PRIM2_UVRGB		*uvrgb;
	int 				i,clock;
	float				screen_near_x,screen_near_y,scr_pos_x,scr_pos_y;
	float				max_alpha;
	float				alpha,temp,temp_x,temp_y;
	
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
	max_alpha = (float)(((*work->flags)>>16)&0xff);
	alpha = max_alpha;
		
    //-1.0f<=>1.0f
	temp = temp_x - 1.1f;

	if(temp > 0.0f){
		alpha = max_alpha - temp*max_alpha/0.3f;
	}
	temp = temp_y - 1.1f;
	if(temp > 0.0f){
		alpha = max_alpha - temp*max_alpha/0.3f;
	}
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

		uvrgb[0].a = (int)alpha;
		uvrgb[1].a = (int)alpha;
		uvrgb[2].a = (int)alpha;
		uvrgb[3].a = (int)alpha;

	}
	
	//printf("You can see\n");
		
}

static void Act(Work *work)
{
	if((*work->flags) & 0x0001){
		Flare_Invisible(work);
		DG_InvisiblePrim2(work->sun);
		return;
	}

	if( (*work->flags) & 0x0004 ){
		printf("%x\n",*work->flags);
		(*work->flags) &= ~0x0004;
		if( work->sun ) GM_FreePrim2(work->sun);
		InitSunPrim( work, SunTexCode[ (((*work->flags)&0xff00)>>8)%5 ] );
		printf("%d\n",(((*work->flags)&0xff00)>>8)%3);
	}

	Flare_Visible(work);
	DG_VisiblePrim2(work->sun);
	SetSunPos( work );
	SetSunPrimPos( work->sun, &work->center, *work->w, *work->h );
	Flare_Act(work);
}

static void Die(Work *work)
{
	int i;
	if(work->sun)GM_FreePrim2(work->sun);
	for( i = 0; i < N_FLARES; i++)
		if(work->flare[i])GM_FreePrim2(work->flare[i]);
	
//	if(work->halation)GM_FreePrim2(work->halation);
}



//太陽
static int InitSunPrim( Work *work, int code )
{
	DG_PRIM2			*prim;
	DG_TEX				*tex;
	DG_PRIM2_UVRGBWH	*uvrgbwh0,*uvrgbwh1;
	FVECTOR				*pos0,*pos1;
	
	tex = DG_GetTexture(code);
	if(!tex) { printf("ERR!! NO TEX!! <gnrl_sprt.c>\n"); return(-1); }
	prim = work->sun = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,1,1 );
	if(!prim){ printf("ERR!! MAKE PRIM!! <plant_sun.c>\n"); return (-1); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	prim->raise = 1000*(int)SUN_SCALE;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgbwh0 = prim->uvrgb[0];
	uvrgbwh1 = prim->uvrgb[1];

	DG_COPY_VEC(pos0,&work->center);
	DG_COPY_VEC(pos1,&work->center);

	uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	uvrgbwh0->f0 = uvrgbwh1->f0 = DRAW_KICK_CODE;
	uvrgbwh0->f1 = uvrgbwh1->f1 = DRAW_KICK_CODE;
	uvrgbwh0->q0 = uvrgbwh1->q0 = 4096;
	uvrgbwh0->q1 = uvrgbwh1->q1 = 4096;
	uvrgbwh0->r = uvrgbwh1->r = 128;
	uvrgbwh0->g = uvrgbwh1->g = 128;
	uvrgbwh0->b = uvrgbwh1->b = 128;
	uvrgbwh0->a = uvrgbwh1->a = 128;
	uvrgbwh0->w = uvrgbwh1->w = (int)(240.0f * SUN_SCALE);
	uvrgbwh0->h = uvrgbwh1->h = (int)(240.0f * SUN_SCALE);

	return (0);
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
		tex = DG_GetTexture(FlareTexCode[i]);
		//tex = DG_GetTexture(6715088);
		if(!tex) { printf("ERR!! NO TEX!! <plant_sun.c>\n"); return(-1); }
		prim = work->flare[i] = GM_MakePrim2( FLARE_FLAG,N_PRIMS_F,N_VERTS_F );
		if(!prim){ printf("ERR!! MAKE PRIM!! <plant_sun.c>\n"); return (-1); }
		prim->flag |= DG_PRIM2_INVISIBLE;
		DG_ConfigPrim2Tex( prim, tex );
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

		prim->raise = 1000000;
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


static int GetResources( Work *work )
{
	SetSunPos( work );
	
	if(InitSunPrim( work, 1126305 ))return -1;
	if(InitFlarePrim( work ))return -1;

	return 0;
}

void *NewPrevSun( FMATRIX* root, int *flags, float *w, float *h )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		
		work->root = root;
		work->flags = flags;
		work->w = w;
		work->h = h;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		printf("new plant_sun.c\n");
	}

	return (void *)work ;
}
