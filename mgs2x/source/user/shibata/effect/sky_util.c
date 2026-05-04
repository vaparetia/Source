//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    plant_sun.c
	ゲーム中の太陽
	2001/05/15 T.Shibata
	
	$Id: sky_util.c,v 1.1.1.3 2002/11/19 11:48:42 Yoshizawa1 Exp $

*/
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include	"../util/ts_util.h"
#include	"gameheader.h"
//#include	"../../kano/preview/prekey.h"
//#include	"../../kano/preview/preview_def.h"
//#include	"../../kano/preview/prefile.h"

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

//11*12 == 132
//44 * 3
//(N_PRIMS*N_VERTS)==(N_STRIP_VERTS*N_STRIPS)

#define		N_PRIMS			(12)
#define		N_VERTS			(8+1)
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

#define				SKY_SCALE	(32.0f)
//#define			SKY_SCALE	(1.0f)

#define		SKY_COLOR_R		(130)
#define		SKY_COLOR_G		(200)
#define		SKY_COLOR_B		(250)

#define 	CLOUD_COLOR		(255)
#define 	CLOUD_ALPHA		(128)

#define		N_MAX_MODLS			(4)


#define		SKY_FLAGS_NO_MOVE	(0x0001)
#define		SKY_FLAGS_NO_BASE	(0x0002)
#define		SKY_FLAGS_NO_CLOUD	(0x0004)

#define		SKY_FLAGS_CAMERA	(0x0008)

typedef struct _sky_util_work{
	GV_ACT_EX		actor;
	int				name;
	int				map;
	DG_PRIM2		*base_cloud[4];
	float			base_cloud_angle[4];
	DG_PRIM2		*base_prim;
	CVECTOR			color;
	int				flags;
	int				cloud_num;
	FMATRIX			root;
	FMATRIX			obj_root[N_MAX_MODLS];
	FMATRIX			lights[N_MAX_MODLS][2];

	FVECTOR			center;
	FVECTOR			bound[12][2];
	FVECTOR			base_cloud_pos[4][30];

	float			cloud_speed[N_MAX_MODLS+4];
	float			rot_y[N_MAX_MODLS];
	float			base_rot_y[N_MAX_MODLS];
	float			sky_bottom;
	float			scale;
	int				mdl_num;
	DG_OBJS			*objs[N_MAX_MODLS];
} Work;


static DG_PRIM2 *InitBaseCloud( Work *work, float len, float off_y, float len_y,
								short rot_vy, int num, int tex_code,
								int color,  int alpha, int pri );

static void MoveBaseCloud( Work *work,float add_angle, int num )
{
	DG_PRIM2	*prim = work->base_cloud[num];
	int			clock;
	FVECTOR		*pos;
	FMATRIX		world;
	FVECTOR		force = {0.0f,0.0f,0.0f,0.0f};
	FVECTOR		*eye_vec = (FVECTOR*)DG_Chanls[0].eye.m[2];
	if( !prim ) return;

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	pos = prim->pos[clock];

	_sceVu0ScaleVector( pos, eye_vec, 1600000.0f );
	pos++;
	force.vx = cosf(work->base_cloud_angle[num]);
	force.vz = sinf(work->base_cloud_angle[num]);

	TS_MakeMatrix( &world, &force, &DG_ZeroVector );//(FVECTOR*)work->root.m[3] );

	DG_SetPos( &world );
	DG_PutVector( work->base_cloud_pos[num], pos, 30 );
	
	work->base_cloud_angle[num] += add_angle;
	if(work->base_cloud_angle[num] > PI )work->base_cloud_angle[num]-=2.0f*PI;
}

static void InvAllSkyPrimObj( Work *work )
{
	int i;

	if(work->base_prim)DG_InvisiblePrim2(work->base_prim);
	if(work->base_cloud[0])DG_InvisiblePrim2(work->base_cloud[0]);
	if(work->base_cloud[1])DG_InvisiblePrim2(work->base_cloud[1]);
	if(work->base_cloud[2])DG_InvisiblePrim2(work->base_cloud[2]);
	if(work->base_cloud[3])DG_InvisiblePrim2(work->base_cloud[3]);
	for( i = 0; i < N_MAX_MODLS; i++ ){
		if(work->objs[i]){
			DG_InvisibleObjs( work->objs[i] );
		}
	}
	//printf("はい消えたー\n");
}

static void VisAllSkyPrimObj( Work *work )
{
	int i;

	if(work->base_prim)DG_VisiblePrim2(work->base_prim);
	if(work->base_cloud[0])DG_VisiblePrim2(work->base_cloud[0]);
	if(work->base_cloud[1])DG_VisiblePrim2(work->base_cloud[1]);
	if(work->base_cloud[2])DG_VisiblePrim2(work->base_cloud[2]);
	if(work->base_cloud[3])DG_VisiblePrim2(work->base_cloud[3]);
	for( i = 0; i < N_MAX_MODLS; i++ ){
		if(work->objs[i]){
			DG_VisibleObjs( work->objs[i] );
		}
	}
	//printf("みえたー\n");
}

static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;
	int	ret = 0;
	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case 0:
			ret = 0;
			VisAllSkyPrimObj( work );
			break;
		  case 1:
			InvAllSkyPrimObj( work );
			ret = 1;
			break;
		  default:
			printf("sky_util.c Msg Err!![%d]\n",num);
		}
		msg--;
	}

	return ret;
}


static void Act(Work *work)
{
	DG_PRIM2	*base_prim = work->base_prim;
	int			i;
//	printf("my name is %d\n", work->name );
	if( CheckMesgParam( work ) ){
		GV_WaitMessage( work, work->name );
		return;
	}
	
	
	if( !(work->flags & (SKY_FLAGS_NO_MOVE|SKY_FLAGS_NO_CLOUD)) ){
		MoveBaseCloud( work, PI/65536.0f*1.0f * work->cloud_speed[0]/100.0f, 0 );
		MoveBaseCloud( work, PI/65536.0f*2.0f * work->cloud_speed[1]/100.0f, 1 );
		MoveBaseCloud( work, PI/65536.0f*3.0f * work->cloud_speed[2]/100.0f, 2 );
		MoveBaseCloud( work, PI/65536.0f*4.0f * work->cloud_speed[3]/100.0f, 3 );
	}

	for( i = 0; i < N_MAX_MODLS; i++ ){
		if( !(work->flags & SKY_FLAGS_NO_MOVE) ){
			float rot_y;

			work->rot_y[i] += PI/65536.0f * work->cloud_speed[i+4]/100.0f;
			while( work->rot_y[i] > PI ) work->rot_y[i] -= 2.0f*PI;
			while( work->rot_y[i] < -PI ) work->rot_y[i] += 2.0f*PI;
			rot_y = work->rot_y[i] + work->base_rot_y[i];

			while( rot_y > PI ) rot_y -= 2.0f*PI;
			while( rot_y < -PI ) rot_y += 2.0f*PI;

			_sceVu0RotMatrixY( &work->obj_root[i], &work->root, rot_y );
			_sceVu0ScaleVector( work->obj_root[i].m[0], work->obj_root[i].m[0], work->scale );
			_sceVu0ScaleVector( work->obj_root[i].m[1], work->obj_root[i].m[1], work->scale );
			_sceVu0ScaleVector( work->obj_root[i].m[2], work->obj_root[i].m[2], work->scale );
			DG_COPY_VEC( work->obj_root[i].m[3], work->root.m[3] );
		}
		if( work->flags & SKY_FLAGS_CAMERA ){

			DG_COPY_VEC( (FVECTOR*)work->obj_root[i].m[3], (FVECTOR*)DG_Chanls[0].eye.m[3] );
			work->obj_root[i].m[3][1] = work->center.vy;
			//printf("sky:");PRINT_PFVEC(0,(FVECTOR*)work->obj_root[i].m[3]);
			//printf("sky: far %f [%p]\n", DG_ClipFar,&DG_ClipFar);
			//DG_ClipFar = 32.0f*80.0f*5000.0f;
		}
		{
			//extern float DG_ClipFar;
			//printf("sky: far %f [%p]\n", DG_ClipFar,&DG_ClipFar);
		}
	}

	if( !(work->flags & SKY_FLAGS_NO_BASE) && base_prim ){
		int 		clock,i;
		FVECTOR		*eye_vec = (FVECTOR*)DG_Chanls[0].eye.m[2];
		FVECTOR		*pos;
		DG_SwitchBuffPrim2( base_prim );
		clock = base_prim->buffer_clock;
		pos = base_prim->pos[clock];
		

		for( i = 0; i < N_PRIMS; i++ ){
			int flags = DG_BoundCheck(&work->root,&work->bound[i][0],&work->bound[i][1]);
			//printf("invis %d[%x]\n",i,flags);
			base_prim->packet[clock][i].flag = base_prim->packet[clock][i].flag & ~(0x03<<12);
			base_prim->packet[clock][i].flag |= (flags&0x3)<<12;
			_sceVu0ScaleVector( pos, eye_vec, 16000000.0f );
			//NewBoundingBoxView(&work->bound[i][0],&work->bound[i][1],128,32,32);
			pos += N_VERTS;
		}
	}
}

static void Die(Work *work)
{
	int i;
	if(work->base_prim)GM_FreePrim2(work->base_prim);
	if(work->base_cloud[0])GM_FreePrim2(work->base_cloud[0]);
	if(work->base_cloud[1])GM_FreePrim2(work->base_cloud[1]);
	if(work->base_cloud[2])GM_FreePrim2(work->base_cloud[2]);
	if(work->base_cloud[3])GM_FreePrim2(work->base_cloud[3]);
	for( i = 0; i < N_MAX_MODLS; i++ ){
		if(work->objs[i]){
			DG_DequeueObjs( work->objs[i] );
			DG_FreeObjs( work->objs[i] );
		}
	}
}


static FVECTOR InitStripVertsPos[] = {
	{     0.000f, 10000.000f,    0.000f, 1.0f },
	{  7071.068f,  8828.727f,    0.000f, 1.0f },
	{ 10000.000f,  6000.000f,    0.000f, 1.0f },

	{ 10000.000f,     0.000f,    0.000f, 1.0f },	
};

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
		{ 0.0f, off_y*work->scale+len_y*work->scale, len*work->scale, 1.0f },
		{ 0.0f, off_y*work->scale, len*work->scale, 1.0f },
	};

//	GV_StrCode("sky_02_add_alp")
	tex = DG_GetTexture(tex_code);
	if(!tex) return NULL;
	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_NOMSAA,
						 1, 15*2 + 1 );
	if(!prim){ printf("ERR!! MAKE PRIM!! <plant_sky.c>\n"); return (NULL); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	prim->raise = -pri * 10 * (int)SKY_SCALE;
	//prim->raise = pri * (int)work->scale;
	prim->root = &work->root;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	DG_COPY_VEC( pos, &DG_ZeroVector );
	uvrgb[0].u = 0;
	uvrgb[0].v = 0;
	uvrgb[0].f = VERT_KICK_CODE;
	uvrgb[0].q = 0;
	uvrgb[0].r = 0;
	uvrgb[0].g = 0;
	uvrgb[0].b = 0;
	uvrgb[0].a = 0;
	
	pos++;
	uvrgb++;
	for(i = 0; i < 3; i++ ){
		for(j = 0; j < 5; j++ ){
			if(j==4 && i == 2) rot.vy = rot_vy;
			DG_SetPos2(&work->center,&rot);
			DG_PutVector(InitPos,pos,2);
			DG_RotVector(InitPos,&work->base_cloud_pos[num][(i*5+j)*2],2);
			
			if(j!=4) rot.vy += (short)(4096.0f/12.0f);
			
			uvrgb[0].u = FTOI12( 1.0f/4.0f * (float)j * tex->u_scale + tex->u_offset );
			uvrgb[0].v = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
			uvrgb[0].f = (j)?DRAW_KICK_CODE:VERT_KICK_CODE;
			uvrgb[0].q = 4096;
			uvrgb[0].r = color&0xff;//CLOUD_COLOR;
			uvrgb[0].g = (color>>8)&0xff;//CLOUD_COLOR;
			uvrgb[0].b = (color>>16)&0xff;//CLOUD_COLOR;
			uvrgb[0].a = alpha;//CLOUD_ALPHA;

			uvrgb[1].u = FTOI12( 1.0f/4.0f * (float)j * tex->u_scale + tex->u_offset );
			uvrgb[1].v = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
			uvrgb[1].f = (j)?DRAW_KICK_CODE:VERT_KICK_CODE;
			uvrgb[1].q = 4096;
			uvrgb[1].r = (color   )&0xff;//CLOUD_COLOR;
			uvrgb[1].g = (color>>8)&0xff;//CLOUD_COLOR;
			uvrgb[1].b = (color>>16)&0xff;//CLOUD_COLOR;
			uvrgb[1].a = alpha;//CLOUD_ALPHA;
		
			uvrgb+=2;
			pos+=2;
		}
	}
	
	work->base_cloud_angle[num] = 0.0f;//PI*(float)rot_vy/2048.0f;
		
//	TS_Scr_Mem( work->base_cloud_pos[num], MEM_SCR_POS, sizeof(FVECTOR), 31 );
	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), 31 );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), 31 );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), 31 );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), 31 );
	
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
	FVECTOR				scale = { work->scale, work->scale, work->scale, 0.0f };
	SVECTOR				rot = { 0, 0, 0, 0};
	
	tex = DG_GetTexture(0);
	prim = work->base_prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_NOMSAA,
										   N_PRIMS, N_VERTS );

	if(!prim){ printf("ERR!! MAKE PRIM!! <plant_sky.c>\n"); return (-1); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	prim->root = &work->root;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	//prim->raise = -10000;
	prim->raise = -1000000;
	//天井のストリップ


	uvrgb[0].u = 0;
	uvrgb[0].v = 0;
	uvrgb[0].f = VERT_KICK_CODE;
	uvrgb[0].q = 0;
	uvrgb[0].r = 0;
	uvrgb[0].g = 0;
	uvrgb[0].b = 0;
	uvrgb[0].a = 0;
	
	uvrgb++;
	
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
		memcpy(uvrgb,MEM_SCR_UV,sizeof(DG_PRIM2_UVRGB)*(N_STRIP_VERTS+1));
		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos++;

#if 0
		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix( &world, &world, &scale );
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[0], &fvtemp[0], 3 );
		rot.vy += 4096/8;
		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix( &world, &world, &scale);
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[1], &fvtemp[3], 2 );
		rot.vy += 4096/8;
		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix( &world, &world, &scale);
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[1], &fvtemp[5], 2 );
#else
		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix( &world, &world, &scale );
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[0], &fvtemp[0], 3 );
		rot.vy += 4096/8;
		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix( &world, &world, &scale);
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[1], &fvtemp[3], 2 );
		rot.vy += 4096/8;
		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix( &world, &world, &scale);
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[1], &fvtemp[5], 2 );
#endif
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


		uvrgb += N_STRIP_VERTS+1;
		pos += N_STRIP_VERTS;
	}

	uvrgb[0].u = 0;
	uvrgb[0].v = 0;
	uvrgb[0].f = VERT_KICK_CODE;
	uvrgb[0].q = 0;
	uvrgb[0].r = 0;
	uvrgb[0].g = 0;
	uvrgb[0].b = 0;
	uvrgb[0].a = 0;
	
	uvrgb++;

	//壁ストリップ
	for( i = 0; i < N_STRIP_VERTS; i++ ){
		uvrgb->u = 0;
		uvrgb->v = 0;
		uvrgb->f = (i<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
		uvrgb->q = 4096;

		uvrgb->r = work->color.r;//SKY_COLOR_R;
		uvrgb->g = work->color.g;//SKY_COLOR_G;
		uvrgb->b = work->color.b;//SKY_COLOR_B;

		uvrgb->a = (i>=2)?128:0;//(short)(128.0f - 128.0f*(float)(i>>1)/3.0f);
		//uvrgb->a = 32;
		//printf("%d\n",uvrgb->a);
		uvrgb++;
	}
	
	uvrgb -= N_STRIP_VERTS+1;
	
	rot.vy = 0;

	for( i = 0; i < 8; i++ ){
		//*uvrgb = *(DG_PRIM2_UVRGB*)MEM_SCR_UV;
		memcpy(uvrgb,&((DG_PRIM2_UVRGB*)MEM_SCR_UV)[4*(N_STRIP_VERTS+1)],sizeof(DG_PRIM2_UVRGB)*(N_STRIP_VERTS+1));
		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos++;

#if 0
		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix( &world, &world, &scale);
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[2], &fvtemp[0], 2 );
		rot.vy += 4096/8;
		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix( &world, &world, &scale);
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[2], &fvtemp[2], 2 );
#else
		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix( &world, &world, &scale);
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[2], &fvtemp[0], 2 );
		rot.vy += 4096/8;
		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix( &world, &world, &scale);
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[2], &fvtemp[2], 2 );
#endif
		DG_COPY_VEC(&pos[0],&fvtemp[1]);	//bottom
		DG_COPY_VEC(&pos[1],&fvtemp[3]);	//bottom

		DG_COPY_VEC(&pos[2],&fvtemp[0]);
		DG_COPY_VEC(&pos[3],&fvtemp[2]);
		DG_COPY_VEC(&pos[4],&fvtemp[0]);
		DG_COPY_VEC(&pos[5],&fvtemp[2]);
		DG_COPY_VEC(&pos[6],&fvtemp[0]);
		DG_COPY_VEC(&pos[7],&fvtemp[2]);

		//bound max
		work->bound[i+4][0].vx = (fvtemp[0].vx>fvtemp[2].vx)?fvtemp[0].vx:fvtemp[2].vx;
		work->bound[i+4][0].vy = fvtemp[1].vy;
		work->bound[i+4][0].vz = (fvtemp[0].vz>fvtemp[2].vz)?fvtemp[0].vz:fvtemp[2].vz;
		work->bound[i+4][0].vw = 1.0f;
		
		work->bound[i+4][1].vx = (fvtemp[0].vx>fvtemp[2].vx)?fvtemp[2].vx:fvtemp[0].vx;
		work->bound[i+4][1].vy = fvtemp[0].vy;
		work->bound[i+4][1].vz = (fvtemp[0].vz>fvtemp[2].vz)?fvtemp[2].vz:fvtemp[0].vz;
		work->bound[i+4][1].vw = 1.0f;
		
		pos[0].vy = pos[2].vy - work->sky_bottom*work->scale;
		pos[1].vy = pos[3].vy - work->sky_bottom*work->scale;
		work->bound[i+4][0].vy = (pos[0].vy>pos[2].vy)?pos[0].vy:pos[2].vy;
		work->bound[i+4][1].vy = (pos[0].vy>pos[2].vy)?pos[2].vy:pos[0].vy;

		uvrgb += N_STRIP_VERTS+1;
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
	// -c	color RGBA
	// -p	pos
	// -b	size
	// -l	scale（今はなし）
	// -f	flags	(cloud_inv sky_inv cloud_move)
	// -t	テクスチャー数	max 4
	//		(名前) * -t
	// -m	モデル数			max 4
	//		(名前 + アンビエント) * -m

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

	work->sky_bottom = (float)GCL_GetOptionValue( 'b', 6000 );
	work->scale = (float)GCL_GetOptionValue( 'l', 110 );
	work->flags = GCL_GetOptionValue( 'f', 0 );
}

static DG_OBJS *InitItemObjs( int code, FMATRIX *root, FMATRIX *light )
{
	DG_DEF		*def;
	DG_OBJS		*objs;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( code, 'k' ) ) ;
	if(!def){ return NULL; }
	objs = DG_MakeObjs( def, DG_FLAG_NOFOG, 0 );
	if(!objs) return NULL;
	if(DG_QueueObjs( objs )<0) return NULL;
	if(root) objs->root = root;
	DG_InvisibleObjs( objs );
	
	if(light) DG_SetLightMatrix( objs, light );
	return objs;
}

static int GetResources( Work *work )
{
	int		i;

	GetOptions( work );

	if( !(work->flags & SKY_FLAGS_NO_BASE) )
		if(InitBasePrim( work )) return -1;

	DG_COPY_MAT( &work->root, &DG_UnitMatrix );
	DG_COPY_VEC( (FVECTOR*)work->root.m[3], &work->center );

	work->cloud_num = GCL_GetOptionValue( 't', 0 );
	for( i = 0; i < work->cloud_num; i++ ){
		int tex_code = GCL_GetNextInt();
		int	rgb;
		rgb  = GCL_GetNextInt()    ;
		rgb |= GCL_GetNextInt()<<8 ;
		rgb |= GCL_GetNextInt()<<16;
		work->cloud_speed[i] = (float)GCL_GetNextInt();
		
		if( tex_code == 48 ) continue;
		work->base_cloud[i] = InitBaseCloud( work, 9600.0f - 800.0f * (float)i,
											 ((i<2)?500.0f:300.0f),
											 6000.0f,
											 i*1024/4,
											 i, tex_code,
											 rgb,
											 ((i<2)?96:CLOUD_ALPHA),
											 i*100 );


	}


	work->mdl_num = GCL_GetOptionValue( 'm', 0 );
	for( i = 0; i < work->mdl_num; i++ ){
		int code;
		_sceVu0ScaleVector( work->obj_root[i].m[0], work->root.m[0], work->scale );
		_sceVu0ScaleVector( work->obj_root[i].m[1], work->root.m[1], work->scale );
		_sceVu0ScaleVector( work->obj_root[i].m[2], work->root.m[2], work->scale );
		DG_COPY_VEC( work->obj_root[i].m[3], &work->center );
		code = GCL_GetNextInt();
		work->objs[i] = InitItemObjs( code, &work->obj_root[i], work->lights[i] );
		work->objs[i]->group_id = work->map;
		if( !work->objs[i] )printf("mdl code [%d] is nothing\n", code );
		//memset( work->lights[i], 0, sizeof(FMATRIX)*2 );
		work->lights[i][1].m[3][0] = (float)GCL_GetNextInt();
		work->lights[i][1].m[3][1] = (float)GCL_GetNextInt();
		work->lights[i][1].m[3][2] = (float)GCL_GetNextInt();
		work->cloud_speed[i+4] = (float)GCL_GetNextInt();
		work->base_rot_y[i] = (float)GCL_GetNextInt()/10000.0f;
	}

	return 0;
}

void *NewSkyUtil( int name, int map )
{
	Work *work = NULL;
	work = (Work*)GV_NewActor(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		work->name = name;
		work->map = map;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		//printf("へいへいほー[%d]\n",work->name);
	}

	return (void *)work ;
}

