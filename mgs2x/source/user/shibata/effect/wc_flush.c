//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    wc_flush.c
	流れろ便器

	2001/07/24 T.Shibata

	$Id: wc_flush.c,v 1.1.1.3 2002/11/19 11:48:42 Yoshizawa1 Exp $
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

#define CLOCK_COUNT	(BP_BASE_TICK())


extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );

//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

#define		GET_COL_R(_rgba)	(((_rgba)>>24)&0xff)
#define		GET_COL_G(_rgba)	(((_rgba)>>16)&0xff)
#define		GET_COL_B(_rgba)	(((_rgba)>> 8)&0xff)
#define		GET_COL_A(_rgba)	(((_rgba)>> 0)&0xff)
#define		SET_COL(_r,_g,_b,_a)	(((_r)<<24)|((_g)<<16)|((_b)<<8)|(_a))
//#define 	TS_DEBUG

#define MEM_SCR_POS		((void*)(SCRPAD_ADDR))
#define MEM_SCR_UV		((void*)(SCRPAD_ADDR+sizeof(FVECTOR)*256))

#define N_MAX_PRIM		(4)

typedef struct {
	GV_ACT_EX		actor;
	FMATRIX			root;
	int				name;
	int				map;
	int				wait_time;
	int				step;
#if 1
	DG_PRIM2		*prim[N_MAX_PRIM];
	float			v_offset[N_MAX_PRIM];
	
	//float			u_offset[N_MAX_PRIM];
	int				mode[N_MAX_PRIM];
	DG_TEX			*tex[N_MAX_PRIM];
	float			add_v_offset[N_MAX_PRIM];
#else
	DG_PRIM2		*prim;
	DG_TEX			*tex;
	float			v_offset;
	float			u_offset;
#endif
	
} Work;

#define		V_OFF_MAX		(64.0f)
#define		V_OFF_MIN		(0.0f)
#define		V_FLUSH_SIZE	(128.0f)
#define		V_SCALE_SIZE	(2.0f)

#define		U_OFF_MAX		(32.0f)
#define		U_OFF_MIN		(0.0f)
#define		U_FLUSH_SIZE	(64.0f)
#define		U_SCALE_SIZE	(2.0f)

typedef struct {
	short	vx;
	short	vy;
	short	vz;
	short	u;
	short	v;
	short	flags;
	short	count;
	short	nx;
	short	ny;
	short	nz;
}VERTS_DATA;

#define N_MASK_VERTS	(sizeof(MaskVerts)/sizeof(MaskVerts[0]))

static VERTS_DATA MaskVerts[] = {
	{  117,  550,  -31, 2791,   13, 0x8fff,    0, 2885, -346, -2886, },
	{  117,  550,   44, 3215,   13, 0x8fff,    0, 3941, -162, -1102, },
	{   82,    0,    0, 2607, 3451, 0x0fff,    0, 3097, -2544, -843, },
	{  133,  125,   81, 3283, 2528, 0x0fff,    0, 3861, -1294, -433, },
	{   67,  100,  194, 3850, 2804, 0x0fff,    0, 2766, -1847, 2389, },

	{  -67,  100,  194,  345, 2826, 0x8fff,    0, -2762, -1851, 2390, },
	{ -133,  125,   82,  848, 2528, 0x8fff,    0, -3861, -1301, -418, },
	{  -82,    0,    0, 1566, 3451, 0x0fff,    0, -3097, -2546, -837, },
	{ -117,  550,   44,  874,   13, 0x0fff,    0, -3944, -163, -1090, },
	{ -117,  550,  -31, 1328,   13, 0x0fff,    0, -2885, -346, -2886, },

	{  117,  550,  -31, 2804,   13, 0x8fff,    0, 2885, -346, -2886, },
	{   82,    0,    0, 2607, 3451, 0x8fff,    0, 3097, -2544, -843, },
	{ -117,  550,  -31, 1327,   13, 0x0fff,    0, -2885, -346, -2886, },
	{  -82,    0,    0, 1566, 3451, 0x0fff,    0, -3097, -2546, -837, },

	{  -47,    0,  157, 1788, 3451, 0x8fff,    0, -1282, -3561, 1564, },
	{   47,    0,  157, 2385, 3451, 0x8fff,    0, 1282, -3561, 1564, },
	{  -67,  100,  194, 1638, 2835, 0x0fff,    0, -2762, -1851, 2390, },
	{   67,  100,  194, 2493, 2835, 0x0fff,    0, 2766, -1847, 2389, },

	{   -5,    0,   74, 2558, 4052, 0x8fff,    0,    0, -4096,    0, },
	{  -82,    0,    0, 2612, 3472, 0x8fff,    0, -3097, -2546, -837, },
	{    5,    0,   74, 1545, 4064, 0x0fff,    0,    0, -4096,    0, },
	{   82,    0,    0, 1575, 3472, 0x0fff,    0, 3097, -2544, -843, },

	{    5,    0,   74, 3640, 4055, 0x8fff,    0,    0, -4096,    0, },
	{   82,    0,    0, 3652, 3451, 0x8fff,    0, 3097, -2544, -843, },
	{    3,    0,   83, 2571, 4055, 0x0fff,    0,    0, -4096,    0, },
	{   47,    0,  157, 2598, 3451, 0x0fff,    0, 1282, -3561, 1564, },

	{   -3,    0,   83, 1550, 4055, 0x8fff,    0,    0, -4096,    0, },
	{  -47,    0,  157, 1554, 3451, 0x8fff,    0, -1282, -3561, 1564, },
	{   -5,    0,   74,  503, 4055, 0x0fff,    0,    0, -4096,    0, },
	{  -82,    0,    0,  521, 3451, 0x0fff,    0, -3097, -2546, -837, },

	{    3,    0,   83, 2559, 4055, 0x8fff,    0,    0, -4096,    0, },
	{   47,    0,  157, 2598, 3451, 0x8fff,    0, 1282, -3561, 1564, },
	{   -3,    0,   83, 1562, 4055, 0x0fff,    0,    0, -4096,    0, },
	{  -47,    0,  157, 1554, 3451, 0x0fff,    0, -1282, -3561, 1564, },

	{  -82,    0,    0, 1566, 3451, 0x8fff,    0, -3097, -2546, -837, },
	{  -47,    0,  157,  530, 3451, 0x8fff,    0, -1282, -3561, 1564, },
	{  -67,  100,  194,  345, 2826, 0x0fff,    0, -2762, -1851, 2390, },

	{   67,  100,  194, 3876, 2793, 0x8fff,    0, 2766, -1847, 2389, },
	{   47,    0,  157, 3622, 3456, 0x8fff,    0, 1282, -3561, 1564, },
	{   82,    0,    0, 2645, 3456, 0x0fff,    0, 3097, -2544, -843, },

	{   -3,    0,   83, 2031, 4055, 0x8fff,    0,    0, -4096,    0, },
	{    0,    0,   79, 2048, 4096, 0x8fff,    0,    0, -4096,    0, },
	{    3,    0,   83, 2069, 4055, 0x0fff,    0,    0, -4096,    0, },

	{   -5,    0,   74, 2017, 4055, 0x8fff,    0,    0, -4096,    0, },
	{    0,    0,   79, 2048, 4096, 0x8fff,    0,    0, -4096,    0, },
	{   -3,    0,   83, 2083, 4055, 0x0fff,    0,    0, -4096,    0, },

	{    3,    0,   83, 2017, 4055, 0x8fff,    0,    0, -4096,    0, },
	{    0,    0,   79, 2048, 4096, 0x8fff,    0,    0, -4096,    0, },
	{    5,    0,   74, 2082, 4055, 0x0fff,    0,    0, -4096,    0, },

	{    5,    0,   74, 2042, 2028, 0x8fff,    0,    0, -4096,    0, },
	{    0,    0,   79, 2074, 1933, 0x8fff,    0,    0, -4096,    0, },
	{   -5,    0,   74, 2107, 2028, 0x0fff,    0,    0, -4096,    0, },

#if 0
	{  117,  550,   44, 3421,    0, 0x8fff,    0, 3941, -162, -1102, },
	{   82,    0,    0, 3813, 4096, 0x0fff,    0, 3503, -1896, -953, },
	{  133,  125,   81, 3081, 3143, 0x0fff,    0, 3861, -1294, -433, },
	{   67,  100,  194, 2099, 3351, 0x0fff,    0, 2766, -1847, 2389, },

	{  -67,  100,  194, 2099, 3351, 0x8fff,    0, -2762, -1851, 2390, },
	{ -133,  125,   82, 3088, 3143, 0x8fff,    0, -3861, -1301, -418, },
	{  -82,    0,    0, 3813, 4096, 0x0fff,    0, -3503, -1898, -947, },
	{ -117,  550,   44, 3421,    0, 0x0fff,    0, -3944, -163, -1090, },
	{ -117,  550,  -31, 4096,    0, 0x0fff,    0, -2885, -346, -2886, },

	{  117,  550,  -31, 2057,   21, 0x8fff,    0, 2885, -346, -2886, },
	{   82,    0,    0, 1774, 4096, 0x8fff,    0, 3503, -1896, -953, },
	{ -117,  550,  -31,    0,   21, 0x0fff,    0, -2885, -346, -2886, },
	{  -82,    0,    0,  298, 4096, 0x0fff,    0, -3503, -1898, -947, },

	{  -47,    0,  157, 1493, 4096, 0x8fff,    0, -1758, -3014, 2144, },
	{   47,    0,  157,  580, 4096, 0x8fff,    0, 1758, -3014, 2144, },
	{  -67,  100,  194, 1699, 3328, 0x0fff,    0, -2762, -1851, 2390, },
	{   67,  100,  194,  374, 3328, 0x0fff,    0, 2766, -1847, 2389, },

	{   82,    0,    0,  273, 4096, 0x8fff,    0, 3503, -1896, -953, },
	{   47,    0,  157,  579, 2901, 0x8fff,    0, 1758, -3014, 2144, },
	{  -82,    0,    0, 1800, 4096, 0x0fff,    0, -3503, -1898, -947, },
	{  -47,    0,  157, 1494, 2901, 0x0fff,    0, -1758, -3014, 2144, },

	{   67,  100,  194, 2099, 3351, 0x8fff,    0, 2766, -1847, 2389, },
	{   47,    0,  157, 2439, 4096, 0x8fff,    0, 1758, -3014, 2144, },
	{   82,    0,    0, 3813, 4096, 0x0fff,    0, 3503, -1896, -953, },

	{  -82,    0,    0, 3813, 4096, 0x8fff,    0, -3503, -1898, -947, },
	{  -47,    0,  157, 2439, 4096, 0x8fff,    0, -1758, -3014, 2144, },
	{  -67,  100,  194, 2099, 3351, 0x0fff,    0, -2762, -1851, 2390, },
#endif
};


#define MAX_ALPHA		(64.0f)
//#define MAX_ALPHA		(550.0f)
//texスクロール縦付
static void ActScrPrimVoff( DG_PRIM2 *prim, int n_verts, int alpha, VERTS_DATA *vdata,
							float v_off, float u_off, DG_TEX *tex, int mode )
{
	int 			clock,i;
	DG_PRIM2_UVRGB	*uvrgb;
	VERTS_DATA 		*data = vdata;
	float			v_off_set = v_off/V_FLUSH_SIZE;
//	float			u_off_set = u_off/U_FLUSH_SIZE;
	
	//_sceVu0MulMatrix( &l, &light[1], &light[0]);

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	uvrgb = prim->uvrgb[clock];

	for( i = 0; i < n_verts; i++ ){
		float	v = (float)data->v/(4096.0f*V_SCALE_SIZE) + v_off_set;
//		float	u = (float)data->u/(4096.0f*U_SCALE_SIZE) + u_off_set;
#if 1
		float	temp;

		if(mode){
			temp = 1.0f - (float)data->vy/550.0f;
		}else{
			temp = (float)data->vy/550.0f;
		}
		temp = (float)alpha * (temp*3.0f + 1.0f);
		if( temp < 0.0f ) temp = 0.0f;
		else if( temp > 64.0f ) temp = 64.0f;

		uvrgb->a = (int)temp;
#else
		uvrgb->a = alpha;
#endif
//		if(mode)
//			uvrgb->u = FTOI12( u * tex->u_scale + tex->u_offset );
		uvrgb->v = FTOI12( v * tex->v_scale + tex->v_offset );
		
		data++;
		uvrgb++;
	}
}

enum {
	MESG_WCFLUSH_VIS = 0,
	MESG_WCFLUSH_INV,
	
	MESG_WCFLUSH_CALL,
};

enum {
	WCFLUSH_STEP_CALLWAIT = 0,
	WCFLUSH_STEP_PREWAIT,
	WCFLUSH_STEP_INIT,
	WCFLUSH_STEP_FLUSH_START,		// 流れ始め
	WCFLUSH_STEP_FLUSH,				// 流れ
	WCFLUSH_STEP_FLUSH_END,			// 流れ終わり
	WCFLUSH_STEP_WAIT,				// 水ためる時間
};


//
#define TIME_FLUSH_PREWAIT	(16)
#define TIME_FLUSH_START	(32)
#define TIME_FLUSH			(64)
#define TIME_FLUSH_END		(64)

#define ALL_FUSH_TIME		(TIME_FLUSH_START+TIME_FLUSH+TIME_FLUSH_END)

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
		  case MESG_WCFLUSH_VIS:
			break;
		  case MESG_WCFLUSH_INV:
			break;
		  case MESG_WCFLUSH_CALL:
			if( !work->step ) work->step++;
			work->wait_time = TIME_FLUSH_PREWAIT;
			printf("wc_flush call\n");
			break;
		  default:
			printf("wc_flush.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	return 0;
}

static void FlushActPrim( Work *work, int alpha, int mode )
{
	int i;

	for( i = 0; i < N_MAX_PRIM; i++ ){
		if( !work->prim[i] ) continue;
		ActScrPrimVoff( work->prim[i], N_MASK_VERTS,
						alpha,
						MaskVerts,
						work->v_offset[i], 0, work->tex[i], mode );
		work->v_offset[i] -= work->add_v_offset[i];
		if( work->v_offset[i] > V_OFF_MAX ) work->v_offset[i] -= V_OFF_MAX;
		else if( work->v_offset[i] < V_OFF_MIN ) work->v_offset[i] += V_OFF_MAX;

#if 0
		if( work->mode[i] == 1 ){
			work->u_offset[i] += U_OFF_MAX/(float)ALL_FUSH_TIME;
			if( work->u_offset[i] > U_OFF_MAX ) work->u_offset[i] = U_OFF_MIN;
		}else if( work->mode[i] == 1 ){
			work->u_offset[i] -= U_OFF_MAX/(float)ALL_FUSH_TIME;
			if( work->u_offset[i] < U_OFF_MIN ) work->u_offset[i] = U_OFF_MAX;
		}
#endif
	}

}

static void InvFlushPrim( Work *work )
{
	int i;
	for( i = 0; i < N_MAX_PRIM; i++ )
		if( work->prim[i] ) DG_InvisiblePrim2(work->prim[i]);
}

static void VisFlushPrim( Work *work )
{
	int i;
	for( i = 0; i < N_MAX_PRIM; i++ )
		if( work->prim[i] ) DG_VisiblePrim2(work->prim[i]);
}

static void Act( Work *work )
{
	//SD_A_SMLBEN01
//extern int	OK_PutSplush( FVECTOR *center, FVECTOR *force );
//	int		i;

#if 1
	CheckMesgParam( work );
	
	switch(work->step){
	  case WCFLUSH_STEP_CALLWAIT:
		GV_WaitMessage( work, work->name );
		break;
	  case WCFLUSH_STEP_PREWAIT:
		if( --work->wait_time < 0 ){
			work->step++;
			FlushActPrim( work, 0, 0 );
		}
		break;
	  case WCFLUSH_STEP_INIT:
		VisFlushPrim( work );
		FlushActPrim( work, 0, 0 );
		GM_SeSetMode( SD_A_SMLBEN01, (FVECTOR*)work->root.m[3], GM_SEMODE_NORMAL );
		work->step++;
		work->wait_time = TIME_FLUSH_START;
		break;
	  case WCFLUSH_STEP_FLUSH_START:
		FlushActPrim( work, (int)(MAX_ALPHA - MAX_ALPHA*(float)work->wait_time/(float)TIME_FLUSH_START), 0 );
		if( --work->wait_time < 0 ){
			work->wait_time = TIME_FLUSH;
			work->step++;
		}
		break;
	  case WCFLUSH_STEP_FLUSH:
		FlushActPrim( work, MAX_ALPHA, 0 );

		if( --work->wait_time < 0 ){
			work->wait_time = TIME_FLUSH_END;
			work->step++;
		}

		break;
	  case WCFLUSH_STEP_FLUSH_END:		
		FlushActPrim( work, (int)(MAX_ALPHA*(float)work->wait_time/(float)TIME_FLUSH_END), 1 );
		if( --work->wait_time < 0 ){
			work->wait_time = 60;
			InvFlushPrim( work );
			work->step++;
		}		
		break;
	  case WCFLUSH_STEP_WAIT:
		if( --work->wait_time < 0 ){
			work->wait_time = 0;
			work->step = 0;
		}
		
		break;
	  default:
		break;
	}
#else
	/*
	ActScrPrimVoff( work->prim[0], N_MASK_VERTS,
					96 - (int)DG_FABS(96.0f*(work->u_offset[0] - U_OFF_MAX/2.0f)/(U_OFF_MAX/2.0f)),
					MaskVerts,
					work->v_offset[0], work->u_offset[0], work->tex[0] );

	
	work->v_offset[0] -= 2.0f;
	if( work->v_offset[0] > V_OFF_MAX ) work->v_offset[0] -= V_OFF_MAX;
	else if( work->v_offset[0] < V_OFF_MIN ) work->v_offset[0] += V_OFF_MAX;
	work->u_offset[0] += U_OFF_MAX/256.0f;
	if( work->u_offset[0] > U_OFF_MAX ) work->u_offset[0] = U_OFF_MIN;
	//else if( work->u_offset < U_OFF_MIN ) work->u_offset += U_OFF_MAX;
	*/

/*
	for( i = 0; i < N_MAX_PRIM; i++ ){
		if( !work->prim[i] ) continue;
		ActScrPrimVoff( work->prim[i], N_MASK_VERTS,
						64,//96 - (int)DG_FABS(96.0f*(work->u_offset[i] - U_OFF_MAX/2.0f)/(U_OFF_MAX/2.0f)),
						MaskVerts,
						work->v_offset[i], work->u_offset[i], work->tex[i], work->mode[i] );
		work->v_offset[i] -= work->add_v_offset[i];
		if( work->v_offset[i] > V_OFF_MAX ) work->v_offset[i] -= V_OFF_MAX;
		else if( work->v_offset[i] < V_OFF_MIN ) work->v_offset[i] += V_OFF_MAX;

		
		if( work->mode[i] == 1 ){
			work->u_offset[i] += U_OFF_MAX/(float)ALL_FUSH_TIME;
			if( work->u_offset[i] > U_OFF_MAX ) work->u_offset[i] = U_OFF_MIN;
		}else if( work->mode[i] == 1 ){
			work->u_offset[i] -= U_OFF_MAX/(float)ALL_FUSH_TIME;
			if( work->u_offset[i] < U_OFF_MIN ) work->u_offset[i] = U_OFF_MAX;
		}
		
	}
	*/
#endif

}

static void Die(Work *work)
{
	int i;
	for( i = 0; i < N_MAX_PRIM; i++ ){
		if( work->prim[i] ) GM_FreePrim2( work->prim[i] );
	}
}


static DG_PRIM2 *InitFlushPrim( int n_prims, int n_verts, int tex_code, VERTS_DATA *vdata, int color, int mode,
							   int flag, FMATRIX *root )
{
	DG_PRIM2		*prim;	
	DG_TEX 			*tex;
	int				i;
	VERTS_DATA 		*data = vdata;
	FVECTOR			*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB	*uvrgb = MEM_SCR_UV;

	tex = DG_GetTexture(tex_code);
	if(!tex) { return(NULL); }
	prim = GM_MakePrim2( mode, n_prims, n_verts );
	if(!prim){ printf("ERR!! MAKE PRIM!!wc_flush.c\n"); return (NULL); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	DG_ConfigPrim2Tex( prim, tex );

	prim->root = root;
	for( i = 0; i < n_verts*n_prims; i++ ){
		pos->vx = (float)data->vx;//*100.0f;
		pos->vy = (float)data->vy;//*100.0f;
		pos->vz = (float)data->vz;//*100.0f;
		pos->vw = 1.0f;

		uvrgb->r = GET_COL_R(color);
		uvrgb->g = GET_COL_G(color);
		uvrgb->b = GET_COL_B(color);
		uvrgb->a = GET_COL_A(color);
		//if(flag)
			uvrgb->f = data->flags;
		//else
		//	uvrgb->f = data->flags|0x0fff;

		uvrgb->q = 4096;
		uvrgb->u = FTOI12((float)data->u/4096.0f * tex->u_scale + tex->u_offset);
		uvrgb->v = FTOI12((float)data->v/4096.0f * tex->v_scale + tex->v_offset);

		//PRINT_PFVEC(i,pos);
		uvrgb++; pos++; data++;
	}

	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );

	//printf("max %d: min %d\n",max,min);
	return prim;
}

#define TEX_CODE0	(GV_StrCode("waterflow01_alp"))
#define TEX_CODE1	(GV_StrCode("waterflow02_alp"))
#define TEX_CODE2	(GV_StrCode("waterflow03_alp"))

static int InitFlush( Work *work )
{
	int			i;
	int			tex_code[N_MAX_PRIM] = {
		TEX_CODE1,
		TEX_CODE1,
//		TEX_CODE2,
//		TEX_CODE2,
		TEX_CODE0,
		TEX_CODE0,
	};

	int			mode[N_MAX_PRIM] = {
		0,0,/*1,2,*/0,0,
	};

	int			alpha[N_MAX_PRIM] = {
//		1,0,/*1,0,*/0,0,
		1,0,/*1,0,*/1,0,
	};

	int			color[N_MAX_PRIM] = {
		SET_COL(16,16,16,128),
		SET_COL(16,16,16,128),
//		SET_COL(64,64,64,128),
//		SET_COL(32,32,32,128),
		SET_COL(16,16,16,128),
		SET_COL(16,16,16,128),
	};
	for( i = 0 ; i < N_MAX_PRIM; i++ ){
		DG_PRIM2	*prim;
		//ノイズ		
		work->tex[i] = DG_GetTexture(tex_code[i]);
		prim = work->prim[i] = InitFlushPrim( 1, N_MASK_VERTS,
										   tex_code[i],
										   MaskVerts,
										   color[i],
										   DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX, 0,
										   &work->root );
		if(!prim) return -1;
		if( alpha[i] ) DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
		else DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


		work->add_v_offset[i] = 0.5f + 0.5f*rnd();
		prim->raise = 2000*(i+1);
		work->mode[i] = mode[i];
		work->v_offset[i] = V_OFF_MAX*rnd();
	}
	
	//work->u_offset[1] = U_OFF_MAX;
	return 0;
}

static int GetResources( Work *work )
{
	//FVECTOR test_center = { -62750.0f, 500.0f, -87144.0f, 1.0f};
	FVECTOR		pos;

	
	if( GCL_GetOption( 'p' ) == NULL ){ printf("not input pos<water_blood.c>\n"); return -1; }
	pos.vx = (float)GCL_GetNextInt();
	pos.vy = (float)GCL_GetNextInt();
	pos.vz = (float)GCL_GetNextInt();
	pos.vw = 1.0f;

	DG_COPY_MAT( &work->root, &DG_UnitMatrix );
	DG_COPY_VEC( (FVECTOR*)work->root.m[3], &pos );
	if( InitFlush( work ) ) return -1;
	
	InvFlushPrim( work );

	//NewDbugSprite( ((FVECTOR*)work->root.m[3]), 100.0f );
	//DG_InvisiblePrim2(work->prim);

	return 0;
}

void *NewWcFlush_Scn( int name, int map )
{
	Work *work = NULL;
	//return 1;
	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor( &(work->actor), Act, Die );
		GV_ActorEX( &work->actor );		
		work->name = name;
		work->map = map;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL;
		}
	}

	return (void *)work ;
}

