//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    plant_sky.c
	ゲーム中の昼空
	2000/12/07 T.Shibata
	
	$Id: plant_sky.c,v 1.1.1.3 2002/11/19 11:48:38 Yoshizawa1 Exp $

*/

//テストタイプ屋根半球円柱
//ベースの雲円柱×３

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef PSX2
#include <math.h>
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


#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

//11*12 == 132
//44 * 3
//(N_PRIMS*N_VERTS)==(N_STRIP_VERTS*N_STRIPS)

#define		N_PRIMS			(12)
#define		N_VERTS			(8)
#define		N_ALLVERTS		(N_VERTS*N_PRIMS)
#define		N_STRIP_VERTS	(8)
#define		N_STRIPS		(12)

#ifdef BP_PS2
#define	   	MEM_SCR_UV	((void*)(SCRPAD_ADDR))
#define	   	MEM_SCR_POS	((void*)(MEM_SCR_UV + sizeof(DG_PRIM2_UVRGB)*N_ALLVERTS))
#define	   	MEM_SCR_TMP	((void*)(MEM_SCR_POS + sizeof(FVECTOR)*N_ALLVERTS))
#else
#define	   	MEM_SCR_UV	((void*)(SCRPAD_ADDR))
#define	   	MEM_SCR_POS	((void*)((char *)MEM_SCR_UV + sizeof(DG_PRIM2_UVRGB)*N_ALLVERTS))
#define	   	MEM_SCR_TMP	((void*)((char *)MEM_SCR_POS + sizeof(FVECTOR)*N_ALLVERTS))
#endif

#define				SKY_SCALE	(110.0f)
//#define			SKY_SCALE	(1.0f)

#define		SKY_COLOR_R		(130)
#define		SKY_COLOR_G		(200)
#define		SKY_COLOR_B		(250)

#define		FOG_COLOR_R		(55)
#define		FOG_COLOR_G		(55)
#define		FOG_COLOR_B		(55)
#define		FOG_COLOR_A		(55)

#define 	CLOUD_COLOR		(255)
#define 	CLOUD_ALPHA		(128)

typedef struct {
	GV_ACT_EX		actor;
	DG_PRIM2		*base_prim;
	DG_PRIM2		*base_cloud[4];
	int				name;
	int				map;
	float			base_cloud_angle[4];
	CVECTOR			color;

	int				flag;
	int				fog_col;
	int				pre_fogcol;

	FVECTOR			center;
	FVECTOR			bound[12][2];
	FVECTOR			base_cloud_pos[4][30];
} Work;

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

static void MoveBaseCloud( Work *work,float add_angle, int num )
{
	DG_PRIM2	*prim = work->base_cloud[num];
	int			clock;
	FVECTOR		*pos;
	FMATRIX		world;
	FVECTOR		force = {0.0f,0.0f,0.0f,0.0f};
	
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	pos = prim->pos[clock];

	force.vx = cosf(work->base_cloud_angle[num]);
	force.vz = sinf(work->base_cloud_angle[num]);

	TS_MakeMatrix( &world, &force, &work->center );
#if 0
	DG_COPY_MAT(&world,&DG_UnitMatrix);
	DG_COPY_VEC((FVECTOR*)world.m[3],&work->center);
	
	world.m[2][3] = cosf(work->base_cloud_angle[num]);
	world.m[2][0] = sinf(work->base_cloud_angle[num]);
	world.m[0][3] = -sinf(work->base_cloud_angle[num]);
	world.m[0][0] = cosf(work->base_cloud_angle[num]);
#endif
	DG_SetPos(&world);
	DG_PutVector(work->base_cloud_pos[num],pos,30);
	
	work->base_cloud_angle[num]+=add_angle;
	if(work->base_cloud_angle[num]>2.0f*PI)work->base_cloud_angle[num]-=2.0f*PI;
	
}

static void Act(Work *work)
{
	DG_PRIM2	*base_prim = work->base_prim;
	int 		clock,i,flags;
//	FMATRIX		world;

	DG_SwitchBuffPrim2( base_prim );
	clock = base_prim->buffer_clock;

//	DG_COPY_MAT(&world,&DG_UnitMatrix);
//	DG_COPY_VEC((FVECTOR*)world.m[3],&work->center);
	
	MoveBaseCloud( work, PI/65536.0f*1.0f, 0 );
	MoveBaseCloud( work, PI/65536.0f*2.0f, 1 );
	MoveBaseCloud( work, PI/65536.0f*3.0f, 2 );
	MoveBaseCloud( work, PI/65536.0f*4.0f, 3 );
	
	if( IT_Thermal == GM_Item ){
		if(work->flag == 0){
			//フォグ値保存と書き換え
			work->flag = 1;
			work->pre_fogcol = *(int*)&DG_FogColor;
#ifdef BP_PS2
			DG_FogColor = (CVECTOR){ FOG_COLOR_R, FOG_COLOR_G, FOG_COLOR_B, FOG_COLOR_A };
#else
			{
				CVECTOR tmp={ FOG_COLOR_R, FOG_COLOR_G, FOG_COLOR_B, FOG_COLOR_A };
				DG_FogColor = tmp ;
			}
#endif
			DG_InvisiblePrim2(base_prim);
			//DG_InvisiblePrim2(work->base_cloud[0]);
			//DG_InvisiblePrim2(work->base_cloud[1]);
			//DG_InvisiblePrim2(work->base_cloud[2]);
			//DG_InvisiblePrim2(work->base_cloud[3]);
		}
		return;
	}else{
		if(work->flag){
			//フォグ値戻し
			work->flag = 0;
			DG_FogColor = *(CVECTOR*)&work->pre_fogcol;
			DG_VisiblePrim2(base_prim);
			//DG_VisiblePrim2(work->base_cloud[0]);
			//DG_VisiblePrim2(work->base_cloud[1]);
			//DG_VisiblePrim2(work->base_cloud[2]);
			//DG_VisiblePrim2(work->base_cloud[3]);
		}
	}
	
	for( i = 0; i < N_PRIMS; i++ ){
		//NewBoundingBoxView(&work->bound[i][0],&work->bound[i][1],128,32,32);
		flags = DG_BoundCheck(&DG_UnitMatrix,&work->bound[i][0],&work->bound[i][1]);
		//printf("invis %d[%x]\n",i,flags);
		base_prim->packet[clock][i].flag = base_prim->packet[clock][i].flag & ~(0x03<<12);
		base_prim->packet[clock][i].flag |= (flags&0x3)<<12;
#if 0
		if(flags&1)
			//NewBoundingBoxView_1( &DG_UnitMatrix, &work->bound[i][0],&work->bound[i][1], 0x0000ffff );
			NewBoundingBoxView(&work->bound[i][0],&work->bound[i][1],0,255,0);
#endif
	}
	
#if 0
	{
		FVECTOR bound0 = { 4800.0f*SKY_SCALE,3000.0f*SKY_SCALE, 4800.0f*SKY_SCALE,1.0f};
		FVECTOR bound1 = {-4800.0f*SKY_SCALE,4000.0f*SKY_SCALE,-4800.0f*SKY_SCALE,1.0f};

		bound0.vx += work->center.vx;
		bound0.vy += work->center.vy;
		bound0.vz += work->center.vz;
		bound1.vx += work->center.vx;
		bound1.vy += work->center.vy;
		bound1.vz += work->center.vz;
		//NewBoundingBoxView(&bound0,&bound1,0,255,0);
		NewBoundingBoxView_1( &DG_UnitMatrix, &bound0,&bound1, 0x0000ffff );
	}
#endif
}

static void Die(Work *work)
{
	if(work->base_prim)GM_FreePrim2(work->base_prim);
	if(work->base_cloud[0])GM_FreePrim2(work->base_cloud[0]);
	if(work->base_cloud[1])GM_FreePrim2(work->base_cloud[1]);
	if(work->base_cloud[2])GM_FreePrim2(work->base_cloud[2]);
	if(work->base_cloud[3])GM_FreePrim2(work->base_cloud[3]);
}

#if 0
static FVECTOR InitStripVertsPos[] = {
	{     0.000f, 10000.000f,    0.000f, 1.0f },
	{  3826.834f,  9238.795f,    0.000f, 1.0f },
	{  7071.067f,  7071.069f,    0.000f, 1.0f },
	{  9238.795f,  3826.835f,    0.000f, 1.0f },
	{  9904.849f,   478.355f,    0.000f, 1.0f },
	{ 10000.000f,     0.000f,    0.000f, 1.0f },
};
#else
static FVECTOR InitStripVertsPos[] = {
	{     0.000f, 10000.000f,    0.000f, 1.0f },
	{  7071.068f,  8828.727f,    0.000f, 1.0f },
	{ 10000.000f,  6000.000f,    0.000f, 1.0f },
	{ 10000.000f,  4000.000f,    0.000f, 1.0f },
	{ 10000.000f,  2000.000f,    0.000f, 1.0f },
	{ 10000.000f,     0.000f,    0.000f, 1.0f },
};
#endif

static float WhichMax3(float a, float b, float c)
{
	if(a > b){
		if(a > c) return a;
		else return c;
	}else{
		if(b > c) return b;
		else return c;
	}
}

static float WhichMin3(float a, float b, float c)
{
	if(a < b){
		if(a < c) return a;
		else return c;
	}else{
		if(b < c) return b;
		else return c;
	}
}
static DG_PRIM2 *InitBaseCloud( Work *work, float len, float off_y, float len_y,
								short rot_vy, int num, int tex_code,
								int color,  int alpha, int pri )
{
	DG_TEX				*tex;
	DG_PRIM2			*prim;
	FVECTOR				*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB		*uvrgb = MEM_SCR_UV;
	SVECTOR				rot = { 0, rot_vy, 0, 0};
	int					i,j;

	FVECTOR				InitPos[2] = {
		{ 0.0f, off_y*SKY_SCALE+len_y*SKY_SCALE, len*SKY_SCALE, 1.0f },
		{ 0.0f, off_y*SKY_SCALE, len*SKY_SCALE, 1.0f },
	};

//	GV_StrCode("sky_02_add_alp")
	tex = DG_GetTexture(tex_code);
	if(!tex) return NULL;
	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX,
						 1, 15*2 );
	if(!prim){ printf("ERR!! MAKE PRIM!! <plant_sky.c>\n"); return (NULL); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	prim->raise = pri * (int)SKY_SCALE;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	for(i = 0; i < 3; i++ ){
		for(j = 0; j < 5; j++ ){
			DG_SetPos2(&work->center,&rot);
			DG_PutVector(InitPos,pos,2);
			DG_RotVector(InitPos,&work->base_cloud_pos[num][(i*5+j)*2],2);
			
			if(j!=4) rot.vy += (short)(4096.0f/12.0f);
			if(j==4 && i == 2) rot.vy = rot_vy;
			uvrgb[0].u = FTOI12( 1.0f/4.0f * (float)j * tex->u_scale + tex->u_offset );
			uvrgb[0].v = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
			uvrgb[0].f = (j)?DRAW_KICK_CODE:VERT_KICK_CODE;
			uvrgb[0].q = 4096;
			uvrgb[0].r = color;//CLOUD_COLOR;
			uvrgb[0].g = color;//CLOUD_COLOR;
			uvrgb[0].b = color;//CLOUD_COLOR;
			uvrgb[0].a = alpha;//CLOUD_ALPHA;

			uvrgb[1].u = FTOI12( 1.0f/4.0f * (float)j * tex->u_scale + tex->u_offset );
			uvrgb[1].v = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
			uvrgb[1].f = (j)?DRAW_KICK_CODE:VERT_KICK_CODE;
			uvrgb[1].q = 4096;
			uvrgb[1].r = color;//CLOUD_COLOR;
			uvrgb[1].g = color;//CLOUD_COLOR;
			uvrgb[1].b = color;//CLOUD_COLOR;
			uvrgb[1].a = alpha;//CLOUD_ALPHA;
		
			uvrgb+=2;
			pos+=2;
		}
	}
	work->base_cloud_angle[num] = 0.0f;//PI*(float)rot_vy/2048.0f;
		
//	TS_Scr_Mem( work->base_cloud_pos[num], MEM_SCR_POS, sizeof(FVECTOR), 30 );
	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), 30 );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), 30 );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), 30 );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), 30 );
	
	return prim;
}

static int InitBasePrim( Work *work )
{
	DG_TEX				*tex;
	DG_PRIM2			*prim;
	FVECTOR				*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB		*uvrgb = MEM_SCR_UV;
	int					i;//,j;
	FMATRIX				world;
	FVECTOR				*fvtemp = MEM_SCR_TMP;
	SVECTOR				rot = { 0, 0, 0, 0};
	
	tex = DG_GetTexture(0);
	prim = work->base_prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
										   N_PRIMS, N_VERTS );

	if(!prim){ printf("ERR!! MAKE PRIM!! <plant_sky.c>\n"); return (-1); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	//天井のストリップ
	for( i = 0; i < N_STRIP_VERTS; i++ ){
		uvrgb->u = 0;
		uvrgb->v = 0;
		uvrgb->f = (i<2 || i==5 || i==6)?VERT_KICK_CODE:DRAW_KICK_CODE;
		uvrgb->q = 4096;

		uvrgb->r = work->color.r;//SKY_COLOR_R;
		uvrgb->g = work->color.g;//SKY_COLOR_G;
		uvrgb->b = work->color.b;//SKY_COLOR_B;

//		uvrgb->a = 130 - 26*((i+1)>>1);
		uvrgb->a = 128;
//printf("%d\n",uvrgb->a);
		uvrgb++;
	}
	uvrgb = MEM_SCR_UV;
	for( i = 0; i < 4; i++ ){
		//*uvrgb = *(DG_PRIM2_UVRGB*)MEM_SCR_UV;
		memcpy(uvrgb,MEM_SCR_UV,sizeof(DG_PRIM2_UVRGB)*N_STRIP_VERTS);

		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
#ifdef BP_PS2
		TS_ScaleMatrix( &world, &world, &(FVECTOR){SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f});
#else
		{
			FVECTOR tmp = {SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f};
			TS_ScaleMatrix( &world, &world, &tmp);
		}
#endif
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[0], &fvtemp[0], 3 );
		rot.vy += 4096/8;
		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
#ifdef BP_PS2
		TS_ScaleMatrix( &world, &world, &(FVECTOR){SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f});
#else
		{
			FVECTOR tmp = {SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f};
			TS_ScaleMatrix( &world, &world, &tmp );
		}
#endif
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[1], &fvtemp[3], 2 );
		rot.vy += 4096/8;
		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
#ifdef BP_PS2
		TS_ScaleMatrix( &world, &world, &(FVECTOR){SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f});
#else
		{
			FVECTOR tmp = {SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f};
			TS_ScaleMatrix( &world, &world, &tmp );
		}
#endif
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[1], &fvtemp[5], 2 );
		
		DG_COPY_VEC(&pos[0],&fvtemp[0]);
		DG_COPY_VEC(&pos[1],&fvtemp[5]);
		DG_COPY_VEC(&pos[2],&fvtemp[1]);
		DG_COPY_VEC(&pos[3],&fvtemp[4]);
		DG_COPY_VEC(&pos[4],&fvtemp[2]);
		DG_COPY_VEC(&pos[5],&fvtemp[5]);
		DG_COPY_VEC(&pos[6],&fvtemp[4]);
		DG_COPY_VEC(&pos[7],&fvtemp[6]);

		//bound max
		work->bound[i][0].vx = WhichMax3(fvtemp[2].vx,fvtemp[6].vx,fvtemp[0].vx);
		work->bound[i][0].vy = fvtemp[0].vy;
		work->bound[i][0].vz = WhichMax3(fvtemp[2].vz,fvtemp[6].vz,fvtemp[0].vz);
		work->bound[i][0].vw = 1.0f;

		work->bound[i][1].vx = WhichMin3(fvtemp[2].vx,fvtemp[6].vx,fvtemp[0].vx);
		work->bound[i][1].vy = fvtemp[4].vy;
		work->bound[i][1].vz = WhichMin3(fvtemp[2].vz,fvtemp[6].vz,fvtemp[0].vz);
		work->bound[i][1].vw = 1.0f;


		uvrgb += N_STRIP_VERTS;
		pos += N_STRIP_VERTS;
	}
	
	//壁ストリップ
	for( i = 0; i < N_STRIP_VERTS; i++ ){
		uvrgb->u = 0;
		uvrgb->v = 0;
		uvrgb->f = (i<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
		uvrgb->q = 4096;

		uvrgb->r = work->color.r;//SKY_COLOR_R;
		uvrgb->g = work->color.g;//SKY_COLOR_G;
		uvrgb->b = work->color.b;//SKY_COLOR_B;

		uvrgb->a = (short)(128.0f - 128.0f*(float)(i>>1)/3.0f);
		//uvrgb->a = 32;
		//printf("%d\n",uvrgb->a);
		uvrgb++;
	}
	
	uvrgb -= N_STRIP_VERTS;
	
	rot.vy = 0;
	
	for( i = 0; i < 8; i++ ){
		//*uvrgb = *(DG_PRIM2_UVRGB*)MEM_SCR_UV;
		memcpy(uvrgb,&((DG_PRIM2_UVRGB*)MEM_SCR_UV)[4*N_STRIP_VERTS],sizeof(DG_PRIM2_UVRGB)*N_STRIP_VERTS);

		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
#ifdef BP_PS2
		TS_ScaleMatrix( &world, &world, &(FVECTOR){SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f});
#else
		{
			FVECTOR tmp = {SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f};
			TS_ScaleMatrix( &world, &world, &tmp );
		}
#endif
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[2], &fvtemp[0], 4 );
		rot.vy += 4096/8;
		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
#ifdef BP_PS2
		TS_ScaleMatrix( &world, &world, &(FVECTOR){SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f});
#else
		{
			FVECTOR tmp = {SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f};
			TS_ScaleMatrix( &world, &world, &tmp );
		}
#endif
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[2], &fvtemp[4], 4 );

		DG_COPY_VEC(&pos[0],&fvtemp[0]);
		DG_COPY_VEC(&pos[1],&fvtemp[4]);
		DG_COPY_VEC(&pos[2],&fvtemp[1]);
		DG_COPY_VEC(&pos[3],&fvtemp[5]);
		DG_COPY_VEC(&pos[4],&fvtemp[2]);
		DG_COPY_VEC(&pos[5],&fvtemp[6]);
		DG_COPY_VEC(&pos[6],&fvtemp[3]);
		DG_COPY_VEC(&pos[7],&fvtemp[7]);
		
		//bound max
		work->bound[i+4][0].vx = (fvtemp[0].vx>fvtemp[4].vx)?fvtemp[0].vx:fvtemp[4].vx;
		work->bound[i+4][0].vy = fvtemp[0].vy;
		work->bound[i+4][0].vz = (fvtemp[0].vz>fvtemp[4].vz)?fvtemp[0].vz:fvtemp[4].vz;
		work->bound[i+4][0].vw = 1.0f;
		
		work->bound[i+4][1].vx = (fvtemp[0].vx>fvtemp[4].vx)?fvtemp[4].vx:fvtemp[0].vx;
		work->bound[i+4][1].vy = fvtemp[7].vy;
		work->bound[i+4][1].vz = (fvtemp[0].vz>fvtemp[4].vz)?fvtemp[4].vz:fvtemp[0].vz;
		work->bound[i+4][1].vw = 1.0f;
		
		
		uvrgb += N_STRIP_VERTS;
		pos += N_STRIP_VERTS;
	}
	
	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), N_ALLVERTS );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), N_ALLVERTS );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );

	return 0;
}

static void GetOptions( Work *work )
{
	//GCL_GetOptionValue( 'n', 0 );
	if(GCL_GetOption('c')){
		work->color.r = GCL_GetNextInt();
		work->color.g = GCL_GetNextInt();
		work->color.b = GCL_GetNextInt();
	}else{
		work->color.r = SKY_COLOR_R;
		work->color.g = SKY_COLOR_G;
		work->color.b = SKY_COLOR_B;
	}
	if(GCL_GetOption('p')){
		work->center.vx = GCL_GetNextInt();
		work->center.vy = GCL_GetNextInt();
		work->center.vz = GCL_GetNextInt();
		work->center.vw = 1.0f;
	}else{
		work->center.vx = 0.0f;
		work->center.vy = 0.0f;
		work->center.vz = 0.0f;
		work->center.vw = 1.0f;
	}
}

static int GetResources( Work *work )
{
	GetOptions( work );

	if(InitBasePrim( work )) return -1;

	work->base_cloud[0] = InitBaseCloud( work, 9600.0f, 500.0f, 6000.0f, 0*1024/4,
										 0, GV_StrCode("sky_1_alp"),
										 192, 96, 0 );
	work->base_cloud[1] = InitBaseCloud( work, 8800.0f, 500.0f, 6000.0f, 1*1024/4,
										 1, GV_StrCode("sky_2_alp"),
										 224, 96, 100 );
	work->base_cloud[2] = InitBaseCloud( work, 8000.0f, 300.0f, 6000.0f, 2*1024/4,
										 2, GV_StrCode("sky_3_alp"),
										 CLOUD_COLOR, CLOUD_ALPHA, 200 );
	work->base_cloud[3] = InitBaseCloud( work, 7200.0f, 300.0f, 6000.0f, 3*1024/4,
										 3, GV_StrCode("sky_4_alp"),
										 CLOUD_COLOR, CLOUD_ALPHA, 300 );
	return 0;
}

void *NewPlantSkyMain( int name, int map )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		
		work->name = name;
		work->map = map;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		printf("new plant_sky.c\n");
	}

	return (void *)work ;
}

