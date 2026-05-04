//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    demo_sun.c
    デモの沈む太陽
    2001/07/07 Yuuta Kunibe	
    $Id: demo_sun.c,v 1.1.1.3 2002/11/19 11:44:37 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include    "libutl.h"
#include	"gameheader.h"

#include	"../../shibata/util/ts_util.h"

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
#define				SUN_SCALE	(110.0f)
//#define			SUN_SCALE	(1.0f)


extern void *NewOnlineCheck_Static( int map_id,
									FVECTOR *from, FVECTOR *to,
									int chk_flag, int seg_flag, int flr_flag,
									int *result,
									int turn, int phase, float max_len  );

typedef struct {
	GV_ACT_EX		actor;
	FVECTOR			center;
	DG_PRIM2		*sun;
	DG_PRIM2		*flare[N_FLARES];
//	DG_PRIM2		*halation;

	FMATRIX			world;
	FVECTOR			line_check_pos[4];	
	int				result[4];
	int				name;
	int				map;
    int				code[6];
	float			scale;
	float			alpha;
	float			w,h;

	FVECTOR			color;
	FVECTOR			add_color;
	int				color_count;

	int				ex_col;

	float			speed;
	float			add_speed;
	int				speed_count;

	int				status;
	
} Work;

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

/* -------------------------------------------------------------- */







/* レンズフレア非表示 */
static void Flare_Invisible(Work *work)
{
	int i;	
	for( i = 0; i < N_FLARES; i++ ){
		if(work->flare[i]) DG_InvisiblePrim2(work->flare[i]);
	}
}

/* レンズフレア表示 */
static void Flare_Visible(Work *work)
{
	int i;
	for( i = 0; i < N_FLARES; i++ ){
		if(work->flare[i]) DG_VisiblePrim2(work->flare[i]);
	}
}


/* レンズフレア表示非表示チェック */
static float CheckResult( Work *work )
{

	int i;
	int	temp = 4;

	for(i=0;i<4;i++){
		temp += work->result[i];
	}
	if(temp){
		//出る
		Flare_Visible(work);
	}else{
		//消す
		Flare_Invisible(work);
	}

	return ((float)temp/8.0f);

}


/*--- メッセージ受信関数 ---*/
static int CheckMesgParam( Work *work )
{

	GV_MSG	*msg;
	int		mes_num;
	int		num;
	int		ret = 0;


	mes_num = GV_ReceiveMessage( work->name, &msg );
	msg += mes_num - 1;

	while( --mes_num >= 0 ){

		num = msg->message[0];
		switch( num ){
		case 0:		/* 表示 */
			ret = 0;
			Flare_Visible(work);
			DG_VisiblePrim2(work->sun);
			work->status = 0;
			break;
		case 1:		/* 非表示 */
			ret = 1;
			Flare_Invisible(work);
			DG_InvisiblePrim2(work->sun);
			work->status = 1;
			break;
		case 2:		/* スピード変更 */
			ret = 0;
			work->speed_count = DIRECT_TICK( msg->message[2] );
			if ( work->speed_count > 0 ) {
				work->add_speed = ( (float)msg->message[1] - work->speed ) / (float)work->speed_count;
			}
			else {
				work->speed_count = 0;
			}
			break;
		case 3:		/* 色変更 */
			ret = 0;
			work->color_count = DIRECT_TICK( msg->message[5] );
			if ( work->color_count > 0 ) {
				work->add_color.vx = ( (float)msg->message[1] - work->color.vx ) / (float)work->color_count;
				work->add_color.vy = ( (float)msg->message[2] - work->color.vy ) / (float)work->color_count;
				work->add_color.vz = ( (float)msg->message[3] - work->color.vz ) / (float)work->color_count;
				work->add_color.vw = ( (float)msg->message[4] - work->color.vw ) / (float)work->color_count;
			}
			else {
				work->color_count = 0;
			}
			break;
		default:
			printf("sky_util.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;

	}

	return ret;

}



static void Flare_Act(Work *work)
{

	FVECTOR				*pos,center,m_pos;
	DG_PRIM2_UVRGB		*uvrgb;
	int 				i,j,clock;
	float				screen_near_x, screen_near_y, scr_pos_x, scr_pos_y;
	float				alpha = work->alpha,temp,temp_x,temp_y;
	float				ratio;


	if ( CheckMesgParam( work ) ) {
		//GV_WaitMessage( work, work->name );
		//return;
	}


	ratio = 0.0f;
	if ( work->status == 0 ) {
		ratio = CheckResult( work );
		if( ratio <= 0.0f ) return;
	}
	

	DG_SetPos( &DG_Chanls[0].eye_pers );
	DG_PutVector( &work->center, &m_pos, 1 );



	//カメラに入ってる？
	//座標で計算
	
	scr_pos_x = m_pos.vx / m_pos.vw;
	scr_pos_y = m_pos.vy / m_pos.vw;

	temp_x = fpu_Abs(scr_pos_x);
	temp_y = fpu_Abs(scr_pos_y);

	if( IT_Thermal == GM_Item ){
		Flare_Invisible(work);
		return ;
	}
	
	if ( m_pos.vz >= m_pos.vw || temp_x > 1.4f || temp_y > 1.4f ) {
		Flare_Invisible(work);
		//printf("out of camera\n");
		return ;
	}

	screen_near_x = GET_SCREEN_TRAN_X(0);
	screen_near_y = GET_SCREEN_TRAN_Y(0);
    //-1.0f<=>1.0f
	temp = temp_x - 1.1f;
	if(temp > 0.0f){
		alpha = work->alpha - temp*work->alpha/0.3f;
	}
	temp = temp_y - 1.1f;
	if(temp > 0.0f){
		alpha = work->alpha - temp*work->alpha/0.3f;
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

		for ( j=0 ; j<4 ; j++ ) {
			uvrgb[j].r = (int)work->color.vx;
			uvrgb[j].g = (int)work->color.vy;
			uvrgb[j].b = (int)work->color.vz;
			uvrgb[j].a = (int)alpha*ratio;
		}
		
		/*uvrgb[0].a = (int)alpha*ratio;
		uvrgb[1].a = (int)alpha*ratio;
		uvrgb[2].a = (int)alpha*ratio;
		uvrgb[3].a = (int)alpha*ratio;*/

	}
	
		
}



static void Act(Work *work)
{
	FVECTOR		init_pos[4] = {
		{  240.0f * work->scale, -240.0f * work->scale, 0.0f, 1.0f },
		{ -240.0f * work->scale, -240.0f * work->scale, 0.0f, 1.0f },
		{  240.0f * work->scale,  240.0f * work->scale, 0.0f, 1.0f },
		{ -240.0f * work->scale,  240.0f * work->scale, 0.0f, 1.0f },
	};
	int					clock;
	FVECTOR				*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;



	/* for message check */
	/*GV_MSG		msg;
    if ( GV_PadData[1].press & PAD_X ) {	// 表示
	    work->code[0] = 0;	//id

	    msg.address = work->name;
	    msg.message = work->code;
	    msg.message_len = 1;
	    GV_SendMessage( &msg );		
	}
    if ( GV_PadData[1].press & PAD_Y ) {	// 非表示
	    work->code[0] = 1;	//id

	    msg.address = work->name;
	    msg.message = work->code;
	    msg.message_len = 1;
	    GV_SendMessage( &msg );		
	}	
    if ( GV_PadData[1].press & PAD_L2 ) {	
	    work->code[0] = 2;	//id
	    work->code[1] = 300;//speed
		work->code[2] = 60;	//count

	    msg.address = work->name;
	    msg.message = work->code;
	    msg.message_len = 3;
	    GV_SendMessage( &msg );
    }
    if ( GV_PadData[1].press & PAD_R2 ) {
		work->code[0] = 3;	//id
	    work->code[1] = 0;	//r
	    work->code[2] = 0;	//g
		work->code[3] = 128;//b
		work->code[4] = 128;//a
		work->code[5] = 60;	//count

	    msg.address = work->name;
	    msg.message = work->code;
	    msg.message_len = 6;
	    GV_SendMessage( &msg );
    }*/



	/* スピード更新 */
	if ( work->speed_count > 0 ) {
		work->speed += work->add_speed;
		work->speed_count--;
		//printf("speed %f\n", work->speed);
	}

	/* 色更新 */
	if ( work->color_count > 0 ) {
		_sceVu0AddVector( &work->color, &work->color, &work->add_color );
		work->color_count--;
		//printf("color r %f, g %f, b %f, a %f\n", work->color.vx, work->color.vy, work->color.vz, work->color.vw );
	}



	/* 本体更新 */
	DG_SwitchBuffPrim2( work->sun );

	clock = work->sun->buffer_clock;
	pos     = work->sun->pos[clock];
	uvrgbwh = work->sun->uvrgb[clock];

	/* 位置変更 */
	work->center.vy -= work->speed;
	DG_COPY_VEC( work->sun->pos[clock], &work->center );

	/* 色変更 */
	uvrgbwh->r = (short)work->color.vx;
	uvrgbwh->g = (short)work->color.vy;
	uvrgbwh->b = (short)work->color.vz;
	uvrgbwh->a = (short)work->color.vw;


	
	TS_MakeMatrix(&work->world,(FVECTOR*)DG_Chanls[0].eye.m[2],&work->center);
	DG_SetPos(&work->world);
	DG_PutVector(init_pos,work->line_check_pos,4);

	Flare_Act(work);
	//printf("result = [%d][%d][%d][%d]\n",work->result[0],work->result[1],work->result[2],work->result[3]);


	work->ex_col = (((int)(work->color.vx)&255)<<24)|
	         (((int)(work->color.vy)&255)<<16)|
	         (((int)(work->color.vz)&255)<< 8)|
	         (((int)(work->color.vw)&255)<< 0);

}



static void Die(Work *work)
{
	int i;
	if(work->sun)GM_FreePrim2(work->sun);
	for( i = 0; i < N_FLARES; i++)
		if(work->flare[i])GM_FreePrim2(work->flare[i]);
	
//	if(work->halation)GM_FreePrim2(work->halation);
}



// 太陽本体初期化
static int InitSunPrim( Work *work )
{

	DG_PRIM2			*prim;
	DG_TEX				*tex;
	DG_PRIM2_UVRGBWH	*uvrgbwh0,*uvrgbwh1;
	FVECTOR				*pos0,*pos1;


	
	prim = work->sun = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 1 );
	if ( !prim ) {
		printf("ERR!! MAKE PRIM!! <plant_sun.c>\n");
		return (-1);
	}
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	prim->raise = 1000 * (int)work->scale;

	//tex = DG_GetTexture( 1126305 );
	tex = DG_GetTexture( 14630308 ); 	// 夕陽テクスチャ sky_sun_e_alp.bmp 2001.08.31変更
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos0     = prim->pos[0];
	pos1     = prim->pos[1];
	uvrgbwh0 = prim->uvrgb[0];
	uvrgbwh1 = prim->uvrgb[1];

	DG_COPY_VEC( pos0, &work->center );
	DG_COPY_VEC( pos1, &work->center );

	uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	uvrgbwh0->f0 = uvrgbwh1->f0 = DRAW_KICK_CODE;
	uvrgbwh0->f1 = uvrgbwh1->f1 = DRAW_KICK_CODE;
	uvrgbwh0->q0 = uvrgbwh1->q0 = 4096;
	uvrgbwh0->q1 = uvrgbwh1->q1 = 4096;
	uvrgbwh0->r  = uvrgbwh1->r  = work->color.vx;
	uvrgbwh0->g  = uvrgbwh1->g  = work->color.vy;
	uvrgbwh0->b  = uvrgbwh1->b  = work->color.vz;
	uvrgbwh0->a  = uvrgbwh1->a  = work->color.vw;
	uvrgbwh0->w  = uvrgbwh1->w  = (int)(work->w);
	uvrgbwh0->h  = uvrgbwh1->h  = (int)(work->h);

	return (0);

}




//レンズフレア初期化
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

		if ( !tex ) {
			printf("ERR!! NO TEX!! <plant_sun.c>\n");
			return(-1);
		}

		prim = work->flare[i] = GM_MakePrim2( FLARE_FLAG,N_PRIMS_F,N_VERTS_F );
		if ( !prim ) {
			printf("ERR!! MAKE PRIM!! <plant_sun.c>\n");
			return (-1);
		}

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

		for ( j = 0; j < N_FLARE_VERTS; j++ ) {

			DG_COPY_VEC( pos0, &DG_ZeroVector );
			DG_COPY_VEC( pos1, &DG_ZeroVector );
			
			uvrgb0->u = uvrgb1->u = u[j&1];
			uvrgb0->v = uvrgb1->v = v[j>>1];
			uvrgb0->f = uvrgb1->f = (j<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
			uvrgb0->q = uvrgb1->q = 4096;
			uvrgb0->r = uvrgb1->r = (int)work->color.vx;
			uvrgb0->g = uvrgb1->g = (int)work->color.vy;
			uvrgb0->b = uvrgb1->b = (int)work->color.vz;
			uvrgb0->a = uvrgb1->a = (int)ALPHA;

			uvrgb0++;
			uvrgb1++;
			pos0++;
			pos1++;

		}

	}

	return 0;

}




static int GetResources( Work *work, int name, FVECTOR *pos, FVECTOR *direction, float speed, float scale,
						 float w, float h, int col, int alpha )
{


	FVECTOR	vectmp;


	work->name = name;
	work->map = GM_CurrentStageMap;
	
	work->scale = scale;

	if ( _sceVu0InnerProduct( direction, direction ) == 0.0f ) {
		work->center.vx = 0.0f;
		work->center.vy = 0.0f;
		work->center.vz = 0.0f;
		work->center.vw = 0.0f;
	}	
	else {
		_sceVu0Normalize( &vectmp, direction );
		_sceVu0ScaleVector( &work->center, &vectmp, -9000.0f * work->scale );
		work->center.vw = 0.0f;
	}		
	
	_sceVu0AddVector( &work->center, &work->center, pos );

	work->speed = speed;
	work->add_speed = 0.0f;
	work->speed_count = 0;
	
	//DG_COPY_VEC( &work->color, color );
	work->color.vx = (float)( ( col >> 24 )&0xff );
	work->color.vy = (float)( ( col >> 16 )&0xff );
	work->color.vz = (float)( ( col >> 8 )&0xff );
	work->color.vw = (float)( ( col )&0xff );	
	work->alpha = (float)alpha;
	work->w     = (float)w * scale;
	work->h     = (float)h * scale;

	work->status = 0;


	if ( InitSunPrim( work ) ) {
		return -1;
	}
	if ( InitFlarePrim( work ) ) {
		return -1;
	}
	


	GV_SetActorChild( work, NewOnlineCheck_Static( GM_CurrentMap,
												   &work->line_check_pos[0], (FVECTOR*)DG_Chanls[0].eye.m[3],
												   HZX_CHK_ALL, 0, 0,
												   &work->result[0],
												   4, 4, 100.0f ) );
	GV_SetActorChild( work, NewOnlineCheck_Static( GM_CurrentMap,
												   &work->line_check_pos[1], (FVECTOR*)DG_Chanls[0].eye.m[3],
												   HZX_CHK_ALL, 0, 0,
												   &work->result[1],
												   4, 4, 100.0f ) );
	GV_SetActorChild( work, NewOnlineCheck_Static( GM_CurrentMap,
												   &work->line_check_pos[2], (FVECTOR*)DG_Chanls[0].eye.m[3],
												   HZX_CHK_ALL, 0, 0,
												   &work->result[2],
												   4, 4, 100.0f ) );
	GV_SetActorChild( work, NewOnlineCheck_Static( GM_CurrentMap,
												   &work->line_check_pos[3], (FVECTOR*)DG_Chanls[0].eye.m[3],
												   HZX_CHK_ALL, 0, 0,
												   &work->result[3],
												   4, 4, 100.0f ) );

	{
		extern void *NewKirariWaterSun2( FMATRIX *world, int *col );
		void	*p0;
		p0 = NewKirariWaterSun2( &work->world, &work->ex_col );
		if( p0 ) {
			GV_SetActorChild( work, p0 );
		}
	}

	return 0;
}



void *NewDemoSun( int name, FMATRIX *mat, float speed, float scale,
				  int w, int h, int col, int alpha )
{

	Work	*work;

	work = (Work*)GV_NewEffect( GV_ACTOR_EFFECT, sizeof(Work) );

	if ( work != NULL ) {
		GV_SetActor( &(work->actor), Act, Die );
		GV_ActorEX( &work->actor );
		
		if ( GetResources( work, name, (FVECTOR *)mat->m[3], (FVECTOR *)mat->m[2], speed, scale, w, h, col, alpha ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}

	}

	return (void *)work;

}


