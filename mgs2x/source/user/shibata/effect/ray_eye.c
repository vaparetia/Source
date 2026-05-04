//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ray_eye.c
	メタルレイの目
	2000/01/10 T.Shibata
	
	$Id: ray_eye.c,v 1.1.1.3 2002/11/19 11:48:39 Yoshizawa1 Exp $
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

#define		N_TAILS				(4)
#define		N_TAILS_JOINTS		(32)
#define		N_TAILS_PRIMS		(2)
#define		N_TAILS_VERTS		(64)

#define		N_EYES				(32)
#define		N_EYES_PRIMS		(1)
#define		N_EYES_VERTS		(32)

#define		DEF_COL_R	(120)
#define		DEF_COL_G	(240)
#define		DEF_COL_B	(255)
#define		DEF_COL_A	(32)

#define		GET_COL_R(_rgba)	(((_rgba)>>24)&0xff)
#define		GET_COL_G(_rgba)	(((_rgba)>>16)&0xff)
#define		GET_COL_B(_rgba)	(((_rgba)>>8) &0xff)
#define		GET_COL_A(_rgba)	(((_rgba)>>0) &0xff)


#define		SCREEN_Z	(51.0f)

extern void *NewOnlineCheck_Static( int map_id,
									FVECTOR *from, FVECTOR *to,
									int chk_flag, int seg_flag, int flr_flag,
									int *result,
									int turn, int phase, float max_len  );

extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );

//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )

#define	PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

#define	GET_SCREEN_TRAN_X(_cam) (SCREEN_Z / (ASPECT_X() ) / DG_Chanls[(_cam)].screen)
#define	GET_SCREEN_TRAN_Y(_cam) (SCREEN_Z / (ASPECT_Y() * DG_Chanls[(_cam)].width / \
											 DG_Chanls[(_cam)].height) / DG_Chanls[(_cam)].screen)

#ifdef BP_PS2
#define MEM_SCR_POS ((void*)(SCRPAD_ADDR))
#define MEM_SCR_UV ((void*)(MEM_SCR_POS+sizeof(FVECTOR)*N_TAILS_PRIMS*N_TAILS_VERTS))
#else
#define MEM_SCR_POS ((void*)(SCRPAD_ADDR))
#define MEM_SCR_UV ((void*)((char *)MEM_SCR_POS+sizeof(FVECTOR)*N_TAILS_PRIMS*N_TAILS_VERTS))
#endif

typedef struct {
	GV_ACT_EX	actor;
	DG_PRIM2	*tails;
	DG_PRIM2	*eyes;
	FMATRIX		*ray_head;
	int			name;
	int			map;
	int			ray_name;
	int			mode;
	short		alpha,timer;
	int			result[N_TAILS*2];
	float		point_alpha[N_TAILS*2];
	char		point_flags[N_TAILS*2];
	FVECTOR		check[N_TAILS*2];
	FVECTOR		norm[2];
	
	FVECTOR		eyes_pos[N_EYES];

	FVECTOR		side_flare_pos[4][2];
	u_char		side_flare_alpha[4];
} Work;


/*
前：520, 1041, 2467
後ろ：713, 1152, 1970
*/
static FVECTOR TailsShift[4][2] = {
	{	//左ポチ
		{  511.0f, 1039.0f, 2485.0f, 1.0f },
		{  571.0f, 1073.0f, 2335.0f, 1.0f },
	},
	{	//左ニョーン
		{  571.0f, 1073.0f, 2335.0f, 1.0f },
//		{  571.0f, 1073.0f, 2335.0f, 1.0f },
		{  713.0f, 1152.0f, 1970.0f, 1.0f },
	},
	{	//右ポチ
		{ -511.0f, 1039.0f, 2485.0f, 1.0f },
		{ -571.0f, 1073.0f, 2335.0f, 1.0f },
	},
	{	//右ニョーン
		{ -571.0f, 1073.0f, 2335.0f, 1.0f },
//		{ -571.0f, 1073.0f, 2335.0f, 1.0f },
		{ -713.0f, 1152.0f, 1970.0f, 1.0f },
	},
};
/*
static FVECTOR EyeCenterShift[2] = {
	{  648.0f, 1118.0f, 2138.0f, 1.0f },
	{ -648.0f, 1118.0f, 2138.0f, 1.0f },
};
*/
static FVECTOR EyePolyRot[2] = {
	{  45.0501f*PI/180.0f,  66.2154f*PI/180.0f,  152.381f*PI/180.0f, 0.0f },
	{  45.0501f*PI/180.0f, -66.2154f*PI/180.0f, -152.381f*PI/180.0f, 0.0f },
};

static FVECTOR EyesVertsShift[N_EYES] = {
	//左
	{  541.0f, 1056.0f, 2410.0f, 1.0f },
	{  541.0f, 1056.0f, 2410.0f, 1.0f },
	{  541.0f, 1056.0f, 2410.0f, 1.0f },
	
	{  600.0f, 1090.0000f, 2260.000f, 1.0f },
	{  608.0f, 1094.5833f, 2239.666f, 1.0f },
	{  616.0f, 1099.1666f, 2219.333f, 1.0f },
	{  624.0f, 1103.7500f, 2199.000f, 1.0f },
	{  632.0f, 1108.3333f, 2178.666f, 1.0f },
	{  640.0f, 1112.9166f, 2158.333f, 1.0f },
	{  648.0f, 1117.5000f, 2138.000f, 1.0f },
	{  656.0f, 1122.0833f, 2117.666f, 1.0f },
	{  664.0f, 1126.6666f, 2097.333f, 1.0f },
	{  672.0f, 1131.2500f, 2077.000f, 1.0f },
	{  680.0f, 1135.8333f, 2056.666f, 1.0f },
	{  686.0f, 1140.4166f, 2036.333f, 1.0f },
	{  696.0f, 1145.0000f, 2016.000f, 1.0f },
	
	//右
	{ -541.0f, 1056.0f, 2410.0f, 1.0f },
	{ -541.0f, 1056.0f, 2410.0f, 1.0f },
	{ -541.0f, 1056.0f, 2410.0f, 1.0f },
	
	{ -600.0f, 1090.0000f, 2260.000f, 1.0f },
	{ -608.0f, 1094.5833f, 2239.666f, 1.0f },
	{ -616.0f, 1099.1666f, 2219.333f, 1.0f },
	{ -624.0f, 1103.7500f, 2199.000f, 1.0f },
	{ -632.0f, 1108.3333f, 2178.666f, 1.0f },
	{ -640.0f, 1112.9166f, 2158.333f, 1.0f },
	{ -648.0f, 1117.5000f, 2138.000f, 1.0f },
	{ -656.0f, 1122.0833f, 2117.666f, 1.0f },
	{ -664.0f, 1126.6666f, 2097.333f, 1.0f },
	{ -672.0f, 1131.2500f, 2077.000f, 1.0f },
	{ -680.0f, 1135.8333f, 2056.666f, 1.0f },
	{ -686.0f, 1140.4166f, 2036.333f, 1.0f },
	{ -696.0f, 1145.0000f, 2016.000f, 1.0f },
	
};

enum {
	MSG_RAYEYE_R_FADEIN = 0,
	MSG_RAYEYE_R_FADEOUT,
	MSG_RAYEYE_L_FADEIN,
	MSG_RAYEYE_L_FADEOUT,
	MSG_RAYEYE_R_VISBL,
	MSG_RAYEYE_R_INVISBL,
	MSG_RAYEYE_L_VISBL,
	MSG_RAYEYE_L_INVISBL,

	
	MSG_RAYEYE_CHECK_MODE,
	MSG_RAYEYE_NOCHECK_MODE,
	MSG_RAYEYE_ONOFFON,
	MSG_RAYEYE_ONOFFOFF,
};

#define POINT_FLAG_NOP		(0x00)
#define POINT_FLAG_FADEOUT	(0x01)
#define POINT_FLAG_FADEIN	(0x02)
#define POINT_FLAG_INVISBL	(0x80)

#define MODE_NOCHECK		(0x01)
#define MODE_ONFFF			(0x02)
#define MODE_PREONFFF		(0x04)

static CONTROL* SearchControl( int name, int map )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		if ( ( control->map & map ) == 0 ) continue ;
		return ( control ) ;
	}
	printf("ray_eye.c: search faild control !!\n");
	return ( NULL );
}

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
		  case MSG_RAYEYE_R_FADEIN:
			work->point_flags[4] = (work->point_flags[4] & 0xf0) | POINT_FLAG_FADEIN;
			work->point_flags[5] = (work->point_flags[5] & 0xf0) | POINT_FLAG_FADEIN;
			work->point_flags[6] = (work->point_flags[6] & 0xf0) | POINT_FLAG_FADEIN;
			work->point_flags[7] = (work->point_flags[7] & 0xf0) | POINT_FLAG_FADEIN;
			//printf("R FadeIn\n");
			break;
		  case MSG_RAYEYE_R_FADEOUT:
			work->point_flags[4] = (work->point_flags[4] & 0xf0) | POINT_FLAG_FADEOUT;
			work->point_flags[5] = (work->point_flags[5] & 0xf0) | POINT_FLAG_FADEOUT;
			work->point_flags[6] = (work->point_flags[6] & 0xf0) | POINT_FLAG_FADEOUT;
			work->point_flags[7] = (work->point_flags[7] & 0xf0) | POINT_FLAG_FADEOUT;
			//printf("R FadeOut\n");
			break;
		  case MSG_RAYEYE_L_FADEIN:
			work->point_flags[0] = (work->point_flags[0] & 0xf0) | POINT_FLAG_FADEIN;
			work->point_flags[1] = (work->point_flags[1] & 0xf0) | POINT_FLAG_FADEIN;
			work->point_flags[2] = (work->point_flags[2] & 0xf0) | POINT_FLAG_FADEIN;
			work->point_flags[3] = (work->point_flags[3] & 0xf0) | POINT_FLAG_FADEIN;
			//printf("L FadeIn\n");
			break;
		  case MSG_RAYEYE_L_FADEOUT:
			work->point_flags[0] = (work->point_flags[0] & 0xf0) | POINT_FLAG_FADEOUT;
			work->point_flags[1] = (work->point_flags[1] & 0xf0) | POINT_FLAG_FADEOUT;
			work->point_flags[2] = (work->point_flags[2] & 0xf0) | POINT_FLAG_FADEOUT;
			work->point_flags[3] = (work->point_flags[3] & 0xf0) | POINT_FLAG_FADEOUT;
			//printf("L FadeOut\n");
			break;
		  case MSG_RAYEYE_R_VISBL:
			work->point_flags[4] &= ~(POINT_FLAG_INVISBL);
			work->point_flags[5] &= ~(POINT_FLAG_INVISBL);
			work->point_flags[6] &= ~(POINT_FLAG_INVISBL);
			work->point_flags[7] &= ~(POINT_FLAG_INVISBL);
			break;
		  case MSG_RAYEYE_R_INVISBL:
			work->point_flags[4] |= POINT_FLAG_INVISBL;
			work->point_flags[5] |= POINT_FLAG_INVISBL;
			work->point_flags[6] |= POINT_FLAG_INVISBL;
			work->point_flags[7] |= POINT_FLAG_INVISBL;
			break;
		  case MSG_RAYEYE_L_VISBL:
			work->point_flags[0] &= ~(POINT_FLAG_INVISBL);
			work->point_flags[1] &= ~(POINT_FLAG_INVISBL);
			work->point_flags[2] &= ~(POINT_FLAG_INVISBL);
			work->point_flags[3] &= ~(POINT_FLAG_INVISBL);
			break;
		  case MSG_RAYEYE_L_INVISBL:
			work->point_flags[0] |= POINT_FLAG_INVISBL;
			work->point_flags[1] |= POINT_FLAG_INVISBL;
			work->point_flags[2] |= POINT_FLAG_INVISBL;
			work->point_flags[3] |= POINT_FLAG_INVISBL;
			
			break;
		  case MSG_RAYEYE_CHECK_MODE:
			work->mode &= ~(MODE_NOCHECK);
			break;
		  case MSG_RAYEYE_NOCHECK_MODE:
			work->mode |= MODE_NOCHECK;
			break;
		  case MSG_RAYEYE_ONOFFON:
			work->mode |= MODE_ONFFF;
			work->mode &= ~MODE_PREONFFF;
			break;
		  case MSG_RAYEYE_ONOFFOFF:
			work->mode |= MODE_PREONFFF;
			break;
		  default:
			printf("ray_eye.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	return (0);
}

static float PointAlphaCheck( float *ratio, char *flag )
{
	float ret;

	switch((*flag)&0x0f){
	  case POINT_FLAG_NOP:
		break;
	  case POINT_FLAG_FADEOUT:
		*ratio -= 0.05f;
		if(*ratio < 0.0f){
			*ratio = 0.0f;
			*flag &= 0xf0;
		}
		break;
	  case POINT_FLAG_FADEIN:
		*ratio += 0.05f;
		if(*ratio > 1.0f){
			*ratio = 1.0f;
			*flag &= 0xf0;
		}
		break;
	}
	ret = *ratio;
	if((*flag)&POINT_FLAG_INVISBL) { ret = 0.0f; }

	return ret;
}

static void TaileAct_NoCheck(Work *work)
{
	DG_PRIM2		*tails = work->tails;
	DG_PRIM2		*eyes = work->eyes;
	int				clock,i,j,alpha;
	float			alpha_ratio0,alpha_ratio1;
	FVECTOR			*pos = MEM_SCR_POS,*pre_pos;
	DG_PRIM2_UVRGB	*uvrgb = MEM_SCR_UV,*pre_uvrgb;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	float			screen_near_x,screen_near_y;
	
	screen_near_x = GET_SCREEN_TRAN_X(0);
	screen_near_y = GET_SCREEN_TRAN_Y(0);
	
	CheckMesgParam( work );
	
	//点滅のアルファ
	if(work->mode&MODE_ONFFF){
		switch(work->timer){
		  case 0:
			if(++work->alpha>=32){
				work->timer++;
				if(work->mode&MODE_PREONFFF){
					work->mode &= ~(MODE_PREONFFF);
					work->mode &= ~(MODE_ONFFF);
				}
			}
			break;
		  case 90:
			if(--work->alpha<=0) work->timer++;
			break;
		  case 180:
			work->timer = 0;
			break;
		  default:
			work->timer++;
			break;
		}
	}
	alpha = work->alpha;

	//printf("alpha = %d\n",alpha);
#if 0
	if(!SearchControl( work->ray_name, work->map )){		
		DG_InvisiblePrim2(work->tails);
		DG_InvisiblePrim2(work->eyes);
		GV_DestroyActor(work);
		return;
	}
#endif
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( tails) )
   {
      return;
   }
	DG_SwitchBuffPrim2( eyes );
	clock = tails->buffer_clock;
	pre_pos = tails->pos[1-clock];
	pre_uvrgb = tails->uvrgb[1-clock];
	uvrgbwh = eyes->uvrgb[clock];
	
	TS_Mem_Scr( &uvrgb[0], &pre_uvrgb[0], sizeof(DG_PRIM2_UVRGB), N_TAILS_VERTS*N_TAILS_PRIMS );
//	TS_Mem_Scr( &uvrgb[2+N_TAILS_VERTS], &pre_uvrgb[N_TAILS_VERTS], sizeof(DG_PRIM2_UVRGB), N_TAILS_VERTS-2 );
	
	TS_Mem_Scr( &pos[2],                &pre_pos[0],              sizeof(FVECTOR), N_TAILS_JOINTS-2 );
	TS_Mem_Scr( &pos[2+N_TAILS_JOINTS], &pre_pos[N_TAILS_JOINTS], sizeof(FVECTOR), N_TAILS_JOINTS-2 );
	TS_Mem_Scr( &pos[2+N_TAILS_JOINTS*2], &pre_pos[N_TAILS_JOINTS*2], sizeof(FVECTOR), N_TAILS_JOINTS-2 );
	TS_Mem_Scr( &pos[2+N_TAILS_JOINTS*3], &pre_pos[N_TAILS_JOINTS*3], sizeof(FVECTOR), N_TAILS_JOINTS-2 );

	//ポチ点滅
	uvrgbwh[0].a = alpha;
	uvrgbwh[1].a = alpha;
	uvrgbwh[2].a = alpha;
	uvrgbwh[16].a = alpha;
	uvrgbwh[17].a = alpha;
	uvrgbwh[18].a = alpha;
	
	//アルファ減算
	for( i = 0; i < N_TAILS; i++ ){
		for( j = N_TAILS_JOINTS/2-2; j >= 0; j-- ){
			if(uvrgb[N_TAILS_JOINTS*i+j*2].a > 64/N_TAILS_JOINTS/2 )
				uvrgb[N_TAILS_JOINTS*i+j*2+2].a = uvrgb[N_TAILS_JOINTS*i+j*2].a - DEF_COL_A/(N_TAILS_JOINTS/2);
			else uvrgb[N_TAILS_JOINTS*i+j*2+2].a = 0;
			
			if(uvrgb[N_TAILS_JOINTS*i+j*2+1].a> 64/N_TAILS_JOINTS/2 )
				uvrgb[N_TAILS_JOINTS*i+j*2+3].a = uvrgb[N_TAILS_JOINTS*i+j*2+1].a - DEF_COL_A/(N_TAILS_JOINTS/2);
			else uvrgb[N_TAILS_JOINTS*i+j*2+3].a = 0;

			//printf("%3d:%d\n",N_TAILS_JOINTS*i+j*2+3,uvrgb[N_TAILS_JOINTS*i+j*2+3].a);
			//printf("%3d:%d\n",N_TAILS_JOINTS*i+j*2+2,uvrgb[N_TAILS_JOINTS*i+j*2+2].a);
		}
	}

	DG_SetPos( work->ray_head );
//	DG_PutVector( EyesVertsShift, eyes->pos[clock], N_EYES );
	DG_PutVector( work->eyes_pos, eyes->pos[clock], N_EYES );
//	DG_RotVector( work->norm, norm, 2 );

	for( i = 0; i < N_TAILS; i++ ){

		DG_SetPos( work->ray_head );
		
		DG_PutVector( TailsShift[i], &pos[N_TAILS_JOINTS*i], 2 );
		
		DG_COPY_VEC( &work->check[i*2], &pos[N_TAILS_JOINTS*i+0] );
		DG_COPY_VEC( &work->check[i*2+1], &pos[N_TAILS_JOINTS*i+1] );
		
		DG_SetPos( &DG_Chanls[DG_CHANL_MAIN].eye_pers );
		DG_PutVector( &pos[0+N_TAILS_JOINTS*i], &pos[0+N_TAILS_JOINTS*i], 2 );
		
		pos[0+N_TAILS_JOINTS*i].vx = pos[0+N_TAILS_JOINTS*i].vx / pos[0+N_TAILS_JOINTS*i].vw;
		pos[0+N_TAILS_JOINTS*i].vy = pos[0+N_TAILS_JOINTS*i].vy / pos[0+N_TAILS_JOINTS*i].vw;
		pos[1+N_TAILS_JOINTS*i].vx = pos[1+N_TAILS_JOINTS*i].vx / pos[1+N_TAILS_JOINTS*i].vw;
		pos[1+N_TAILS_JOINTS*i].vy = pos[1+N_TAILS_JOINTS*i].vy / pos[1+N_TAILS_JOINTS*i].vw;

		pos[i*N_TAILS_JOINTS].vx *= screen_near_x;
		pos[i*N_TAILS_JOINTS].vy *= screen_near_y;
		pos[i*N_TAILS_JOINTS].vz = SCREEN_Z;

		pos[i*N_TAILS_JOINTS+1].vx *= screen_near_x;
		pos[i*N_TAILS_JOINTS+1].vy *= screen_near_y;
		pos[i*N_TAILS_JOINTS+1].vz = SCREEN_Z;

		DG_COPY_VEC( &work->side_flare_pos[i][0], &pos[N_TAILS_JOINTS*i+0] );
		DG_COPY_VEC( &work->side_flare_pos[i][1], &pos[N_TAILS_JOINTS*i+1] );

		//uvrgb[i*N_TAILS_JOINTS].a = 0;
		//uvrgb[i*N_TAILS_JOINTS+1].a = 0;

		alpha_ratio0 = PointAlphaCheck( &work->point_alpha[i*2], &work->point_flags[i*2]);
		alpha_ratio1 = PointAlphaCheck( &work->point_alpha[i*2+1], &work->point_flags[i*2+1]);

		if(!(i&1)){
			uvrgb[i*N_TAILS_JOINTS].a = (int)((float)DEF_COL_A*(float)alpha/32.0f*alpha_ratio0);
			uvrgb[i*N_TAILS_JOINTS+1].a = (int)((float)DEF_COL_A*(float)alpha/32.0f*alpha_ratio1);

			if(uvrgb[i*N_TAILS_JOINTS].a > uvrgb[i*N_TAILS_JOINTS+1].a)
				work->side_flare_alpha[i] = uvrgb[i*N_TAILS_JOINTS].a;
			else
				work->side_flare_alpha[i] = uvrgb[i*N_TAILS_JOINTS+1].a;
		}else{
			uvrgb[i*N_TAILS_JOINTS].a = (int)((float)DEF_COL_A*alpha_ratio0);
			uvrgb[i*N_TAILS_JOINTS+1].a = (int)((float)DEF_COL_A*alpha_ratio1);

			if(uvrgb[i*N_TAILS_JOINTS].a > uvrgb[i*N_TAILS_JOINTS+1].a)
				work->side_flare_alpha[i] = uvrgb[i*N_TAILS_JOINTS].a;
			else
				work->side_flare_alpha[i] = uvrgb[i*N_TAILS_JOINTS+1].a;
		}
	}

	TS_Scr_Mem( tails->pos[clock], MEM_SCR_POS, sizeof(FVECTOR), N_TAILS_PRIMS*N_TAILS_VERTS );
	TS_Scr_Mem( tails->uvrgb[clock], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), N_TAILS_PRIMS*N_TAILS_VERTS );

    //PRINT_PFVEC(0, (FVECTOR*)work->ray_head->m[0]);
	//PRINT_PFVEC(1, (FVECTOR*)work->ray_head->m[1]);
	//PRINT_PFVEC(2, (FVECTOR*)work->ray_head->m[2]);
	//PRINT_PFVEC(3, (FVECTOR*)work->ray_head->m[3]);
	//printf("\n");

}

static void TaileAct_Check(Work *work)
{
	DG_PRIM2		*tails = work->tails;
	DG_PRIM2		*eyes = work->eyes;
	int				clock,i,j,alpha;
	FVECTOR			*pos = MEM_SCR_POS,*pre_pos,eye_inner,norm[2];
	DG_PRIM2_UVRGB	*uvrgb = MEM_SCR_UV,*pre_uvrgb;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	float			screen_near_x,screen_near_y,inner;

	screen_near_x = GET_SCREEN_TRAN_X(0);
	screen_near_y = GET_SCREEN_TRAN_Y(0);


	switch(work->timer){
	  case 0:
		if(++work->alpha>=32) work->timer++;
		break;
	  case 60:
		if(--work->alpha<=0) work->timer++;
		break;
	  case 120:
		work->timer = 0;
		break;
	  default:
		work->timer++;
		break;
	}
	alpha = work->alpha;

	//printf("alpha = %d\n",alpha);
#if 0
	if(!SearchControl( work->ray_name, work->map )){		
		DG_InvisiblePrim2(work->tails);
		DG_InvisiblePrim2(work->eyes);
		GV_DestroyActor(work);
		return;
	}
#endif
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( tails ) )
   {
      return;
   }
	DG_SwitchBuffPrim2( eyes );
	clock = tails->buffer_clock;
	pre_pos = tails->pos[1-clock];
	pre_uvrgb = tails->uvrgb[1-clock];
	uvrgbwh = eyes->uvrgb[clock];
	
	TS_Mem_Scr( &uvrgb[0], &pre_uvrgb[0], sizeof(DG_PRIM2_UVRGB), N_TAILS_VERTS*N_TAILS_PRIMS );
//	TS_Mem_Scr( &uvrgb[2+N_TAILS_VERTS], &pre_uvrgb[N_TAILS_VERTS], sizeof(DG_PRIM2_UVRGB), N_TAILS_VERTS-2 );
	
	TS_Mem_Scr( &pos[2],                &pre_pos[0],              sizeof(FVECTOR), N_TAILS_JOINTS-2 );
	TS_Mem_Scr( &pos[2+N_TAILS_JOINTS], &pre_pos[N_TAILS_JOINTS], sizeof(FVECTOR), N_TAILS_JOINTS-2 );
	TS_Mem_Scr( &pos[2+N_TAILS_JOINTS*2], &pre_pos[N_TAILS_JOINTS*2], sizeof(FVECTOR), N_TAILS_JOINTS-2 );
	TS_Mem_Scr( &pos[2+N_TAILS_JOINTS*3], &pre_pos[N_TAILS_JOINTS*3], sizeof(FVECTOR), N_TAILS_JOINTS-2 );

	//ポチ点滅
	uvrgbwh[0].a = alpha;
	uvrgbwh[1].a = alpha;
	uvrgbwh[2].a = alpha;
	uvrgbwh[16].a = alpha;
	uvrgbwh[17].a = alpha;
	uvrgbwh[18].a = alpha;
	
	//アルファ減算
	for( i = 0; i < N_TAILS; i++ ){
		for( j = N_TAILS_JOINTS/2-2; j >= 0; j-- ){
			if(uvrgb[N_TAILS_JOINTS*i+j*2].a > 64/N_TAILS_JOINTS/2 )
				uvrgb[N_TAILS_JOINTS*i+j*2+2].a = uvrgb[N_TAILS_JOINTS*i+j*2].a - DEF_COL_A/(N_TAILS_JOINTS/2);
			else uvrgb[N_TAILS_JOINTS*i+j*2+2].a = 0;
			
			if(uvrgb[N_TAILS_JOINTS*i+j*2+1].a>64/N_TAILS_JOINTS/2)
				uvrgb[N_TAILS_JOINTS*i+j*2+3].a = uvrgb[N_TAILS_JOINTS*i+j*2+1].a-DEF_COL_A/(N_TAILS_JOINTS/2);
			else uvrgb[N_TAILS_JOINTS*i+j*2+3].a = 0;

			//printf("%3d:%d\n",N_TAILS_JOINTS*i+j*2+3,uvrgb[N_TAILS_JOINTS*i+j*2+3].a);
			//printf("%3d:%d\n",N_TAILS_JOINTS*i+j*2+2,uvrgb[N_TAILS_JOINTS*i+j*2+2].a);
		}
	}

#if 1
	DG_SetPos( work->ray_head );
//	DG_PutVector( EyesVertsShift, eyes->pos[clock], N_EYES );
	DG_PutVector( work->eyes_pos, eyes->pos[clock], N_EYES );
	DG_RotVector( work->norm, norm, 2 );

	DG_COPY_VEC(&eye_inner,(FVECTOR*)DG_Chanls[0].eye.m[2]);
	
	for( i = 0; i < N_TAILS; i++ ){
		DG_SetPos( work->ray_head );
		
		DG_PutVector( TailsShift[i], &pos[N_TAILS_JOINTS*i], 2 );
		
		DG_COPY_VEC( &work->check[i*2], &pos[N_TAILS_JOINTS*i+0] );
		DG_COPY_VEC( &work->check[i*2+1], &pos[N_TAILS_JOINTS*i+1] );
		
		DG_SetPos( &DG_Chanls[DG_CHANL_MAIN].eye_pers );
		DG_PutVector( &pos[0+N_TAILS_JOINTS*i], &pos[0+N_TAILS_JOINTS*i], 2 );
		
		pos[0+N_TAILS_JOINTS*i].vx = pos[0+N_TAILS_JOINTS*i].vx / pos[0+N_TAILS_JOINTS*i].vw;
		pos[0+N_TAILS_JOINTS*i].vy = pos[0+N_TAILS_JOINTS*i].vy / pos[0+N_TAILS_JOINTS*i].vw;
		pos[1+N_TAILS_JOINTS*i].vx = pos[1+N_TAILS_JOINTS*i].vx / pos[1+N_TAILS_JOINTS*i].vw;
		pos[1+N_TAILS_JOINTS*i].vy = pos[1+N_TAILS_JOINTS*i].vy / pos[1+N_TAILS_JOINTS*i].vw;
		
		//uvrgb[i*N_TAILS_JOINTS].a = 0;
		//uvrgb[i*N_TAILS_JOINTS+1].a = 0;

		inner = _sceVu0InnerProduct( &eye_inner, &norm[i/2] );
		if(inner < 0.1f &&
		   fabs(pos[i*N_TAILS_JOINTS].vx) < 1.0f &&
		   fabs(pos[i*N_TAILS_JOINTS].vy) < 1.0f &&
		   pos[i*N_TAILS_JOINTS].vz < pos[i*N_TAILS_JOINTS].vw &&
		   work->result[i*2] > 0){
			
			work->point_alpha[i*2] += 0.05f;
			if(work->point_alpha[i*2] > 1.0f) work->point_alpha[i*2] = 1.0f;
		}else{
			work->point_alpha[i*2] -= 0.05f;
			if(work->point_alpha[i*2] < 0.0f) work->point_alpha[i*2] = 0.0f;
		}
		
		if(inner < 0.1f &&
		   fabs(pos[i*N_TAILS_JOINTS+1].vx) < 1.0f &&
		   fabs(pos[i*N_TAILS_JOINTS+1].vy) < 1.0f &&
		   pos[i*N_TAILS_JOINTS+1].vz < pos[i*N_TAILS_JOINTS+1].vw &&
		   work->result[i*2+1] > 0){
			
			work->point_alpha[i*2+1] += 0.05f;
			if(work->point_alpha[i*2+1]>1.0f) work->point_alpha[i*2+1] = 1.0f;
		}else{
			work->point_alpha[i*2+1] -= 0.05f;
			if(work->point_alpha[i*2+1]<0.0f) work->point_alpha[i*2+1] = 0.0f;
		}
		//printf("%d alpha ratio %f\n",i*2+1,work->point_alpha[i*2+1]);
		
      // BP_WARNING - The following should be (!( i&1 )) to actually behave correctly.  Not fixing.
      if(!i&1){
			//ポチ
			uvrgb[i*N_TAILS_JOINTS].a = (int)((float)DEF_COL_A*(float)alpha/32.0f*work->point_alpha[i*2]);
			uvrgb[i*N_TAILS_JOINTS+1].a = (int)((float)DEF_COL_A*(float)alpha/32.0f*work->point_alpha[i*2+1]);
			if(uvrgb[i*N_TAILS_JOINTS].a > uvrgb[i*N_TAILS_JOINTS+1].a)
				work->side_flare_alpha[i] = uvrgb[i*N_TAILS_JOINTS].a;
			else
				work->side_flare_alpha[i] = uvrgb[i*N_TAILS_JOINTS+1].a;
		} else {
			//ニョーン
			uvrgb[i*N_TAILS_JOINTS].a = (int)((float)DEF_COL_A*work->point_alpha[i*2]);
			uvrgb[i*N_TAILS_JOINTS+1].a = (int)((float)DEF_COL_A*work->point_alpha[i*2+1]);
			if(uvrgb[i*N_TAILS_JOINTS].a > uvrgb[i*N_TAILS_JOINTS+1].a)
				work->side_flare_alpha[i] = uvrgb[i*N_TAILS_JOINTS].a;
			else
				work->side_flare_alpha[i] = uvrgb[i*N_TAILS_JOINTS+1].a;
		}

		pos[i*N_TAILS_JOINTS].vx *= screen_near_x;
		pos[i*N_TAILS_JOINTS].vy *= screen_near_y;
		pos[i*N_TAILS_JOINTS].vz = SCREEN_Z;

		pos[i*N_TAILS_JOINTS+1].vx *= screen_near_x;
		pos[i*N_TAILS_JOINTS+1].vy *= screen_near_y;
		pos[i*N_TAILS_JOINTS+1].vz = SCREEN_Z;
		
		DG_COPY_VEC( &work->side_flare_pos[i][0], &pos[N_TAILS_JOINTS*i+0] );
		DG_COPY_VEC( &work->side_flare_pos[i][1], &pos[N_TAILS_JOINTS*i+1] );
	}
#else
	DG_SetPos( work->ray_head );
	DG_PutVector( TailsShift[0], &pos[0], 2 );
	DG_PutVector( TailsShift[1], &pos[N_TAILS_JOINTS], 2 );
	DG_PutVector( TailsShift[2], &pos[N_TAILS_JOINTS*2], 2 );
	DG_PutVector( TailsShift[3], &pos[N_TAILS_JOINTS*3], 2 );
	
//	DG_PutVector( EyesVertsShift, eyes->pos[clock], N_EYES );
	DG_PutVector( work->eyes_pos, eyes->pos[clock], N_EYES );
	DG_RotVector( work->norm, norm, 2 );

	DG_COPY_VEC( &work->check[0], &pos[0] );
	DG_COPY_VEC( &work->check[1], &pos[1] );
	DG_COPY_VEC( &work->check[2], &pos[N_TAILS_JOINTS+0] );
	DG_COPY_VEC( &work->check[3], &pos[N_TAILS_JOINTS+1] );
	DG_COPY_VEC( &work->check[4], &pos[N_TAILS_JOINTS*2+0] );
	DG_COPY_VEC( &work->check[5], &pos[N_TAILS_JOINTS*2+1] );
	DG_COPY_VEC( &work->check[6], &pos[N_TAILS_JOINTS*3+0] );
	DG_COPY_VEC( &work->check[7], &pos[N_TAILS_JOINTS*3+1] );

	DG_SetPos( &DG_Chanls[DG_CHANL_MAIN].eye_pers );
	DG_PutVector( &pos[0], &pos[0], 2 );
	DG_PutVector( &pos[0+N_TAILS_JOINTS], &pos[0+N_TAILS_JOINTS], 2 );
	DG_PutVector( &pos[0+N_TAILS_JOINTS*2], &pos[0+N_TAILS_JOINTS*2], 2 );
	DG_PutVector( &pos[0+N_TAILS_JOINTS*3], &pos[0+N_TAILS_JOINTS*3], 2 );
	
	pos[0].vx = pos[0].vx / pos[0].vw;
	pos[0].vy = pos[0].vy / pos[0].vw;
	pos[1].vx = pos[1].vx / pos[1].vw;
	pos[1].vy = pos[1].vy / pos[1].vw;

	pos[0+N_TAILS_JOINTS].vx = pos[0+N_TAILS_JOINTS].vx / pos[0+N_TAILS_JOINTS].vw;
	pos[0+N_TAILS_JOINTS].vy = pos[0+N_TAILS_JOINTS].vy / pos[0+N_TAILS_JOINTS].vw;
	pos[1+N_TAILS_JOINTS].vx = pos[1+N_TAILS_JOINTS].vx / pos[1+N_TAILS_JOINTS].vw;
	pos[1+N_TAILS_JOINTS].vy = pos[1+N_TAILS_JOINTS].vy / pos[1+N_TAILS_JOINTS].vw;
	
	pos[0+N_TAILS_JOINTS*2].vx = pos[0+N_TAILS_JOINTS*2].vx / pos[0+N_TAILS_JOINTS*2].vw;
	pos[0+N_TAILS_JOINTS*2].vy = pos[0+N_TAILS_JOINTS*2].vy / pos[0+N_TAILS_JOINTS*2].vw;
	pos[1+N_TAILS_JOINTS*2].vx = pos[1+N_TAILS_JOINTS*2].vx / pos[1+N_TAILS_JOINTS*2].vw;
	pos[1+N_TAILS_JOINTS*2].vy = pos[1+N_TAILS_JOINTS*2].vy / pos[1+N_TAILS_JOINTS*2].vw;
	
	pos[0+N_TAILS_JOINTS*3].vx = pos[0+N_TAILS_JOINTS*3].vx / pos[0+N_TAILS_JOINTS*3].vw;
	pos[0+N_TAILS_JOINTS*3].vy = pos[0+N_TAILS_JOINTS*3].vy / pos[0+N_TAILS_JOINTS*3].vw;
	pos[1+N_TAILS_JOINTS*3].vx = pos[1+N_TAILS_JOINTS*3].vx / pos[1+N_TAILS_JOINTS*3].vw;
	pos[1+N_TAILS_JOINTS*3].vy = pos[1+N_TAILS_JOINTS*3].vy / pos[1+N_TAILS_JOINTS*3].vw;

	//チェックなどなど
	for( i = 0; i < N_TAILS; i++ ){
		uvrgb[i*N_TAILS_JOINTS].a = 0;
		uvrgb[i*N_TAILS_JOINTS+1].a = 0;


		inner = _sceVu0InnerProduct(&eye_inner,&norm[i]);
		if(inner < 0.1f){
			if(fabs(pos[i*N_TAILS_JOINTS].vx) < 1.0f &&
			   fabs(pos[i*N_TAILS_JOINTS].vy) < 1.0f &&
			   pos[i*N_TAILS_JOINTS].vz < pos[i*N_TAILS_JOINTS].vw ){

				if(work->result[i*2] > 0){
					uvrgb[i*N_TAILS_JOINTS].a = DEF_COL_A;
				}
			}
			if(fabs(pos[i*N_TAILS_JOINTS+1].vx) < 1.0f &&
			   fabs(pos[i*N_TAILS_JOINTS+1].vy) < 1.0f &&
			   pos[i*N_TAILS_JOINTS+1].vz < pos[i*N_TAILS_JOINTS+1].vw ){

				
				if(work->result[i*2+1] > 0){
					uvrgb[i*N_TAILS_JOINTS+1].a = DEF_COL_A;
				}
			}
		}
		pos[i*N_TAILS_JOINTS].vx *= screen_near_x;
		pos[i*N_TAILS_JOINTS].vy *= screen_near_y;
		pos[i*N_TAILS_JOINTS].vz = SCREEN_Z;

		pos[i*N_TAILS_JOINTS+1].vx *= screen_near_x;
		pos[i*N_TAILS_JOINTS+1].vy *= screen_near_y;
		pos[i*N_TAILS_JOINTS+1].vz = SCREEN_Z;
	}
#endif
	TS_Scr_Mem( tails->pos[clock], MEM_SCR_POS, sizeof(FVECTOR), N_TAILS_PRIMS*N_TAILS_VERTS );
	TS_Scr_Mem( tails->uvrgb[clock], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), N_TAILS_PRIMS*N_TAILS_VERTS );

    //PRINT_PFVEC(0, (FVECTOR*)work->ray_head->m[0]);
	//PRINT_PFVEC(1, (FVECTOR*)work->ray_head->m[1]);
	//PRINT_PFVEC(2, (FVECTOR*)work->ray_head->m[2]);
	//PRINT_PFVEC(3, (FVECTOR*)work->ray_head->m[3]);
	//printf("\n");
#if 0
	//デバッグ法線表示
	{
		extern void *NewTsDebugLine( FVECTOR *verts, int n_verts, int color );
		FVECTOR		temp[2];

		DG_SetPos(work->ray_head);
		DG_PutVector(&EyeCenterShift[0],&temp[0],1);
		DG_RotVector(&work->norm[0],&temp[1],1);
		_sceVu0ScaleVector(&temp[1],&temp[1],5000.0f);
		_sceVu0AddVector(&temp[1],&temp[1],&temp[0]);
		NewTsDebugLine( temp, 2, 0x80101080 );
		
		AN_Test_Eye2( &temp[0], 2 );
		AN_Test_Eye2( &temp[1], 2 );
		//PRINT_PFVEC(0,&temp[0]);
		//PRINT_PFVEC(1,&temp[1]);
		
		DG_PutVector(&EyeCenterShift[1],&temp[0],1);
		DG_RotVector(&work->norm[1],&temp[1],1);
		_sceVu0ScaleVector(&temp[1],&temp[1],5000.0f);
		_sceVu0AddVector(&temp[1],&temp[1],&temp[0]);
		NewTsDebugLine( temp, 2, 0x10801080 );
		AN_Test_Eye2( &temp[0], 2 );
		AN_Test_Eye2( &temp[1], 2 );
		//PRINT_PFVEC(2,&temp[0]);
		//PRINT_PFVEC(3,&temp[1]);
	}
#endif

}

static void Act(Work *work)
{
	if(work->mode&MODE_NOCHECK)
		TaileAct_NoCheck(work);
	else
		TaileAct_Check(work);
}

static void Die(Work *work)
{
	if(work->tails) GM_FreePrim2(work->tails);
	if(work->eyes) GM_FreePrim2(work->eyes);
}

static void InitEyesData( DG_PRIM2 *prim, DG_TEX *tex, FMATRIX *ray_head, int mode )
{
	FVECTOR				*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	int					i;

	pos = (void*)(SCRPAD_ADDR);
	uvrgbwh = (void*)(SCRPAD_ADDR+sizeof(FVECTOR)*N_EYES);
	
	DG_SetPos( ray_head );
	DG_PutVector( EyesVertsShift, pos, N_EYES );

	for( i = 0; i < N_EYES; i++ ){
		uvrgbwh->r = DEF_COL_R;
		uvrgbwh->g = DEF_COL_G;
		uvrgbwh->b = DEF_COL_B;
		uvrgbwh->a = 24;//DEF_COL_A;
		
		uvrgbwh->f0 = 0;
		uvrgbwh->f1 = 0;
		uvrgbwh->q0 = 4096;
		uvrgbwh->q1 = 4096;
		
		uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		
		uvrgbwh->w = 360;
		uvrgbwh->h = 360;
		uvrgbwh++;
	}
	TS_Scr_Mem( prim->pos[0], (void*)(SCRPAD_ADDR), sizeof(FVECTOR), N_EYES );
	TS_Scr_Mem( prim->pos[1], (void*)(SCRPAD_ADDR), sizeof(FVECTOR), N_EYES );

	TS_Scr_Mem( prim->uvrgb[0], (void*)(SCRPAD_ADDR+sizeof(FVECTOR)*N_EYES),
				sizeof(DG_PRIM2_UVRGBWH), N_EYES );
	TS_Scr_Mem( prim->uvrgb[1], (void*)(SCRPAD_ADDR+sizeof(FVECTOR)*N_EYES),
				sizeof(DG_PRIM2_UVRGBWH), N_EYES );
}

static void InitTailsData( DG_PRIM2 *prim, DG_TEX *tex, FMATRIX *ray_head, int mode )
{
	FVECTOR				*pos,init_tails_pos[2];
	DG_PRIM2_UVRGB		*uvrgb;
	int					i,j;
	float				screen_near_x,screen_near_y;
	
	screen_near_x = GET_SCREEN_TRAN_X(0);
	screen_near_y = GET_SCREEN_TRAN_Y(0);

	pos = (void*)(SCRPAD_ADDR);
	uvrgb = (void*)(SCRPAD_ADDR+sizeof(FVECTOR)*N_TAILS_PRIMS*N_TAILS_VERTS);

	for( i = 0; i < N_TAILS; i++ ){
		DG_SetPos( ray_head );
		DG_PutVector( TailsShift[i], init_tails_pos, 2 );

		DG_SetPos( &DG_Chanls[DG_CHANL_MAIN].eye_pers );
		DG_PutVector( init_tails_pos, init_tails_pos, 2 );

		for( j = 0; j < N_TAILS_JOINTS; j++ ){
			pos->vz = SCREEN_Z;
			pos->vx = screen_near_x * init_tails_pos[j&1].vx / init_tails_pos[j&1].vw;
			pos->vy = screen_near_y * init_tails_pos[j&1].vy / init_tails_pos[j&1].vw;
			//DG_COPY_VEC( pos, &init_tails_pos[j&1] );
			
			uvrgb->r = (int)((float)DEF_COL_R*0.75f);
			uvrgb->g = (int)((float)DEF_COL_G*0.75f);
			uvrgb->b = (int)((float)DEF_COL_B*0.75f);
			uvrgb->a = 0;//128/N_TAILS_JOINTS * (N_TAILS_JOINTS-j);
			uvrgb->f = (j<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
			uvrgb->q = 4096;
			//uvrgb->u = 0;
			//uvrgb->v = 0;
			uvrgb->u = FTOI12( ((j&1)?1.0f:0.0f) * tex->u_scale + tex->u_offset );
			uvrgb->v = FTOI12( 1.0f*(float)(j/2)/(float)(N_TAILS_VERTS/2-1) * tex->v_scale + tex->v_offset );

			//printf("u %f:v %f\n",1.0f*(float)(j/2)/(float)(N_TAILS_VERTS/2-1),((j&1)?1.0f:0.0f));
			pos++;
			uvrgb++;
		}
	}

	TS_Scr_Mem( prim->pos[0], (void*)(SCRPAD_ADDR), sizeof(FVECTOR), N_TAILS_PRIMS*N_TAILS_VERTS );
	TS_Scr_Mem( prim->pos[1], (void*)(SCRPAD_ADDR), sizeof(FVECTOR), N_TAILS_PRIMS*N_TAILS_VERTS );

	TS_Scr_Mem( prim->uvrgb[0], (void*)(SCRPAD_ADDR+sizeof(FVECTOR)*N_TAILS_PRIMS*N_TAILS_VERTS),
				sizeof(DG_PRIM2_UVRGB), N_TAILS_PRIMS*N_TAILS_VERTS );
	TS_Scr_Mem( prim->uvrgb[1], (void*)(SCRPAD_ADDR+sizeof(FVECTOR)*N_TAILS_PRIMS*N_TAILS_VERTS),
				sizeof(DG_PRIM2_UVRGB), N_TAILS_PRIMS*N_TAILS_VERTS );
	
}


static int GetResources_Scn( Work *work )
{
	DG_PRIM2	*prim;
	DG_TEX		*tex;
	CONTROL		*control;
	int			ray_name,mode,i;
	FMATRIX		world;
	
	work->ray_name = ray_name = GCL_GetOptionValue( 'r', 0 );
	mode = GCL_GetOptionValue( 'm', 0 );
	work->mode = (mode)?MODE_NOCHECK:0;

	_sceVu0RotMatrix(&world,&DG_UnitMatrix,&EyePolyRot[0]);
	DG_COPY_VEC(&work->norm[0],(FVECTOR*)world.m[2]);
	_sceVu0RotMatrix(&world,&DG_UnitMatrix,&EyePolyRot[1]);
	DG_COPY_VEC(&work->norm[1],(FVECTOR*)world.m[2]);

	for( i = 0; i < 2; i++ ){
		FVECTOR		add;
		_sceVu0ScaleVector( &add, &work->norm[i], 16.0f);

		_sceVu0AddVector( &work->eyes_pos[ 0+i*16], &add, &EyesVertsShift[ 0+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 1+i*16], &add, &EyesVertsShift[ 1+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 2+i*16], &add, &EyesVertsShift[ 2+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 3+i*16], &add, &EyesVertsShift[ 3+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 4+i*16], &add, &EyesVertsShift[ 4+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 5+i*16], &add, &EyesVertsShift[ 5+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 6+i*16], &add, &EyesVertsShift[ 6+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 7+i*16], &add, &EyesVertsShift[ 7+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 8+i*16], &add, &EyesVertsShift[ 8+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 9+i*16], &add, &EyesVertsShift[ 9+i*16]);
		_sceVu0AddVector( &work->eyes_pos[10+i*16], &add, &EyesVertsShift[10+i*16]);
		_sceVu0AddVector( &work->eyes_pos[11+i*16], &add, &EyesVertsShift[11+i*16]);
		_sceVu0AddVector( &work->eyes_pos[12+i*16], &add, &EyesVertsShift[12+i*16]);
		_sceVu0AddVector( &work->eyes_pos[13+i*16], &add, &EyesVertsShift[13+i*16]);
		_sceVu0AddVector( &work->eyes_pos[14+i*16], &add, &EyesVertsShift[14+i*16]);
		_sceVu0AddVector( &work->eyes_pos[15+i*16], &add, &EyesVertsShift[15+i*16]);
	}
	
	control = SearchControl( ray_name, work->map );
	if(!control){ printf("ERR!! WRONG NAME!! <ray_eye.c>\n"); return -1; };
	//work->ray_head = &(control->object->objs->objs[54].world);		//E3用のモデルの場合
	work->ray_head = &(control->object->objs->objs[14].world);	//現在のモデルの場合


	//テイルブラーの初期化
	//tex = DG_GetTexture(3182109);
	tex = DG_GetTexture(9135611);
	prim = work->tails = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_ON_CAMERA,
									   N_TAILS_PRIMS, N_TAILS_VERTS );
	if(!prim){ printf("ERR!! MAKE PRIM!! <ray_eye.c>\n"); return (-1); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	InitTailsData( prim, tex, work->ray_head, mode );



	//目のプリムの初期化
	//tex = DG_GetTexture(3182109);
	tex = DG_GetTexture(2055109);
	prim = work->eyes = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
									  N_EYES_PRIMS, N_EYES_VERTS );
	if(!prim){ printf("ERR!! MAKE PRIM!! <ray_eye.c>\n"); return (-1); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	InitEyesData( prim, tex, work->ray_head, mode );

	for( i = 0; i < N_TAILS*2; i++ ){
		GV_SetActorChild(work,NewOnlineCheck_Static( GM_CurrentMap,
													 &work->check[i], (FVECTOR*)DG_Chanls[0].eye.m[3],
													 HZX_CHK_ALL, 0, 0,
													 &work->result[i],
													 4, 4, 100.0f  ));
		work->point_alpha[i] = 0.0f;
	}
	
#if 1
	{
		extern void *NewSideFlareWidth(
			FVECTOR *pos_scr0,	// 死ぬまで参照し続ける
			FVECTOR *pos_scr1,	// 死ぬまで参照し続ける
			u_char	*alpha		// 死ぬまで参照し続ける
		);
		for( i=0; i<4; i++ ){
			NewSideFlareWidth( &work->side_flare_pos[i][0], &work->side_flare_pos[i][1], &work->side_flare_alpha[i] );
		}
	}
#endif
	work->alpha = 0;
	work->timer = 0;
	//DG_InvisiblePrim2(work->tails);
	return (0);
}

void *NewRayEye_Scn( int name, int map )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		work->name = name;
		work->map = map;
		if(GetResources_Scn(work) < 0){
			GV_DestroyActor(work) ;
			return (void*)-1;//NULL ;
		}
		printf("ray eye\n");
	}

	return (void *)work ;
}
/*
メモ
　　画面内チェック	4点
　　　　↓
　　法線チェック	1本
　　　　↓
　　ハザードチェック　4本
　　　　↓
　　キャラチェック	2本か1本
*/

static int GetResources( Work *work )
{
	DG_PRIM2	*prim;
	DG_TEX		*tex;
	//CONTROL		*control;
	int			ray_name,mode,i;
	FMATRIX		world;

	ray_name = work->ray_name;
	work->mode = mode = MODE_NOCHECK|MSG_RAYEYE_ONOFFON;

	_sceVu0RotMatrix(&world,&DG_UnitMatrix,&EyePolyRot[0]);
	DG_COPY_VEC(&work->norm[0],(FVECTOR*)world.m[2]);
	_sceVu0RotMatrix(&world,&DG_UnitMatrix,&EyePolyRot[1]);
	DG_COPY_VEC(&work->norm[1],(FVECTOR*)world.m[2]);

	for( i = 0; i < 2; i++ ){
		FVECTOR		add;
		_sceVu0ScaleVector( &add, &work->norm[i], 16.0f);

		_sceVu0AddVector( &work->eyes_pos[ 0+i*16], &add, &EyesVertsShift[ 0+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 1+i*16], &add, &EyesVertsShift[ 1+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 2+i*16], &add, &EyesVertsShift[ 2+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 3+i*16], &add, &EyesVertsShift[ 3+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 4+i*16], &add, &EyesVertsShift[ 4+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 5+i*16], &add, &EyesVertsShift[ 5+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 6+i*16], &add, &EyesVertsShift[ 6+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 7+i*16], &add, &EyesVertsShift[ 7+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 8+i*16], &add, &EyesVertsShift[ 8+i*16]);
		_sceVu0AddVector( &work->eyes_pos[ 9+i*16], &add, &EyesVertsShift[ 9+i*16]);
		_sceVu0AddVector( &work->eyes_pos[10+i*16], &add, &EyesVertsShift[10+i*16]);
		_sceVu0AddVector( &work->eyes_pos[11+i*16], &add, &EyesVertsShift[11+i*16]);
		_sceVu0AddVector( &work->eyes_pos[12+i*16], &add, &EyesVertsShift[12+i*16]);
		_sceVu0AddVector( &work->eyes_pos[13+i*16], &add, &EyesVertsShift[13+i*16]);
		_sceVu0AddVector( &work->eyes_pos[14+i*16], &add, &EyesVertsShift[14+i*16]);
		_sceVu0AddVector( &work->eyes_pos[15+i*16], &add, &EyesVertsShift[15+i*16]);
	}
	
	//control = SearchControl( ray_name, work->map );
	//if(!control){ printf("ERR!! WRONG NAME!! <ray_eye.c>\n"); return -1; };
	//work->ray_head = &(control->object->objs->objs[54].world);		//E3用のモデルの場合
	//work->ray_head = &(control->object->objs->objs[14].world);	//現在のモデルの場合


	//テイルブラーの初期化
	//tex = DG_GetTexture(3182109);
	tex = DG_GetTexture(9135611);
	prim = work->tails = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_ON_CAMERA,
									   N_TAILS_PRIMS, N_TAILS_VERTS );
	if(!prim){ printf("ERR!! MAKE PRIM!! <ray_eye.c>\n"); return (-1); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	InitTailsData( prim, tex, work->ray_head, mode );



	//目のプリムの初期化
	//tex = DG_GetTexture(3182109);
	tex = DG_GetTexture(2055109);
	prim = work->eyes = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
									  N_EYES_PRIMS, N_EYES_VERTS );
	if(!prim){ printf("ERR!! MAKE PRIM!! <ray_eye.c>\n"); return (-1); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	InitEyesData( prim, tex, work->ray_head, mode );

	for( i = 0; i < N_TAILS*2; i++ ){
//		GV_SetActorChild(work,NewOnlineCheck_Static( GM_CurrentMap,
//													 &work->check[i], (FVECTOR*)DG_Chanls[0].eye.m[3],
//													 HZX_CHK_ALL, 0, 0,
//													 &work->result[i],
//													 4, 4, 100.0f  ));
		work->point_alpha[i] = 0.0f;
	}
	
#if 0
	{
		extern void *NewSideFlareWidth(
			FVECTOR *pos_scr0,	// 死ぬまで参照し続ける
			FVECTOR *pos_scr1,	// 死ぬまで参照し続ける
			u_char	*alpha		// 死ぬまで参照し続ける
		);
		for( i=0; i<4; i++ ){
			NewSideFlareWidth( &work->side_flare_pos[i][0], &work->side_flare_pos[i][1], &work->side_flare_alpha[i] );
		}
	}
#endif
	work->alpha = 0;
	work->timer = 0;
	//DG_InvisiblePrim2(work->tails);
	return (0);
}

void *NewRayEye( int name, OBJECT *ray )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		work->name = name;
//		work->ray_name = ray_name;
		work->ray_head = &ray->objs->objs[14].world;
		if(GetResources(work) < 0){
			GV_DestroyActor(work) ;
			return (void*)-1;//NULL ;
		}
		printf("ray eye\n");
	}

	return (void *)work ;
}
