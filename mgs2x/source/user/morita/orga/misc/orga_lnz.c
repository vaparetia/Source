//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    orga_lnz.c
    投光器のレンズフレア

    2000/12/14 T.Shibata
    2001/04/14 T.Morita Revised
	
    $Id: orga_lnz.c,v 1.1.1.3 2002/11/19 11:46:26 Yoshizawa1 Exp $

*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libutl.h"
#include	"gameheader.h"

#define		SCREEN_Z	(51.0f)

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)
#define		GET_SCREEN_TRAN_X(_cam) (SCREEN_Z / (ASPECT_X() ) / DG_Chanls[(_cam)].screen)
#define		GET_SCREEN_TRAN_Y(_cam) (SCREEN_Z / (ASPECT_Y() * DG_Chanls[(_cam)].width / \
				 DG_Chanls[(_cam)].height) / DG_Chanls[(_cam)].screen)

#define		N_PRIMS_F			(1)
#define		N_VERTS_F			(4)
#define		N_ALLVERTS_F		(N_VERTS_F*N_PRIMS_F)

#define		N_FLARES			(16)
#define		N_FLARE_VERTS		(4)

#define	   	MEM_SCR_UV	((void*)(SCRPAD_ADDR))
#define	   	MEM_SCR_POS	((void*)(MEM_SCR_UV + sizeof(DG_PRIM2_UVRGB)*N_ALLVERTS_F))
#define	   	MEM_SCR_TMP	((void*)(MEM_SCR_POS + sizeof(FVECTOR)*N_ALLVERTS_F))

#define		ALPHA		(18.0f)


extern void *NewOnlineCheck_Static( int map_id,
				    FVECTOR *from, FVECTOR *to,
				    int chk_flag, int seg_flag, int flr_flag,
				    int *result,
				    int turn, int phase, float max_len  );
extern void TS_MakeMatrix( FMATRIX *out_mat, FVECTOR *from_vec ,FVECTOR *pos ) ;

typedef struct
{
    GV_ACT_EX  actor     ;
    FVECTOR    center    ;
    FVECTOR    init_check_pos[4] ;
    FVECTOR    line_check_pos[4] ;
    FMATRIX   *world            ;
    DG_PRIM2  *flare[N_FLARES]  ;
    int        result[4] ;
    int        max_alpha ;
    int        tic       ;
    float      size      ;
#ifndef PSX2
    int        tgl       ;
#endif
} Work ;

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
	{ 0.8f, TRAN_SCR_SIZE(0.8f), 4.0f, -0.0001f },		//太陽付近
//	{ 0.4f, TRAN_SCR_SIZE(0.4f), 0.0f, -0.0f },			//太陽付近
	{ 0.4f, TRAN_SCR_SIZE(0.4f), 0.0f,  0.0f },			//太陽付近
	{ 0.8f, TRAN_SCR_SIZE(2.8f), 8.0f,  0.0001f },		//太陽付近
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

static void Flare_Invisible( Work *work, int mode )
{
    int i ;

    for( i=0 ; i<N_FLARES ; i++ )
	if ( mode || FlareData[i].vw >= 1.0f )
	    if ( work->flare[i] )
		DG_InvisiblePrim2( work->flare[i] ) ;
}

static void Flare_Visible( Work *work )
{
    int i ;

    for( i=0 ; i<N_FLARES ; i++ )
	if ( work->flare[i] )
	    DG_VisiblePrim2( work->flare[i] ) ;
}

static float CheckResult( Work *work )
{
    int i;
    int	temp = 4;

    for( i=0 ; i<4 ; i++ )
	temp += work->result[i]>=0 ? work->result[i] : work->result[0] ;
    if ( temp )	/*出る*/
	Flare_Visible( work ) ;
    else        /*消す*/
	Flare_Invisible( work, 1 ) ;

    return (float)temp/8.0f ;
}

static void Flare_Act(Work *work, FVECTOR *center )
{
    FVECTOR        *pos, m_pos ;
    DG_PRIM2_UVRGB *uvrgb    ;
    int 	    i, clock ;
    float	    screen_near_x, screen_near_y, scr_pos_x, scr_pos_y ;
    float	    alpha = (float)work->max_alpha ;
    float	    ratio, temp, temp_x, temp_y ;
    float           dir ;

    /* 赤外線スコープの時 フレアを消す */
    if ( IT_Thermal == GM_Item )
    {
	Flare_Invisible( work, 1 ) ;
	return ;
    }

    ratio = CheckResult( work ) ;
    if ( ratio <= 0.0f )
    {
	Flare_Invisible( work, 1 ) ;
	return ;
    }

    DG_SetPos( &DG_Chanls[0].eye_pers ) ;
    DG_PutVector( center, &m_pos, 1 ) ;

    /* カメラに入ってる？ */
    /* 座標で計算         */
    scr_pos_x = m_pos.vx / m_pos.vw ;
    scr_pos_y = m_pos.vy / m_pos.vw ;
    temp_x = fpu_Abs( scr_pos_x ) ;
    temp_y = fpu_Abs( scr_pos_y ) ;

    /* 画面外なので 消して処理もしない */
    if ( m_pos.vz >= m_pos.vw || temp_x > 1.4f || temp_y > 1.4f )
    {
	Flare_Invisible( work, 1 ) ;
	return ;
    }

    /* 同じ方を向いているか */
    dir = _sceVu0InnerProduct( (FVECTOR *)work->world->m[2],
			       (FVECTOR *)DG_Chanls[0].eye.m[2] ) ;

    /* 俯瞰の時は 半分消す */
    if ( !GM_CheckPlayerStatus( PLAYER_WATCH ) )
	Flare_Invisible( work, 0 ) ;
    else
	ratio *= dir*dir ;
    if ( dir > 0.0f )
    {
	Flare_Invisible( work, 1 ) ;
	return ;
    }

    screen_near_x = GET_SCREEN_TRAN_X(0);
    screen_near_y = GET_SCREEN_TRAN_Y(0);
    /* -1.0f<=>1.0f*/
    temp = temp_x - 1.1f;
    if ( temp > 0.0f )
	alpha = (float)work->max_alpha * (1.0f - temp/0.3f) ;
    temp = temp_y - 1.1f;
    if ( temp > 0.0f )
	alpha = (float)work->max_alpha * (1.0f - temp/0.3f) ;
    for( i = 0; i < N_FLARES; i++ )
    {
	DG_SwitchBuffPrim2( work->flare[i] );
	clock = work->flare[i]->buffer_clock;
		
	pos = work->flare[i]->pos[clock];
	uvrgb = work->flare[i]->uvrgb[clock];
	
	center->vx = scr_pos_x + (FlareData[i].vw * -scr_pos_x);
	center->vy = scr_pos_y + (FlareData[i].vw * -scr_pos_y);

	pos[0].vx = screen_near_x * (center->vx - FlareData[i].vx);
	pos[0].vy = screen_near_y * (center->vy - FlareData[i].vy);
	pos[0].vz = SCREEN_Z;
	pos[1].vx = screen_near_x * (center->vx + FlareData[i].vx);
	pos[1].vy = screen_near_y * (center->vy - FlareData[i].vy);
	pos[1].vz = SCREEN_Z;
	pos[2].vx = screen_near_x * (center->vx - FlareData[i].vx);
	pos[2].vy = screen_near_y * (center->vy + FlareData[i].vy);
	pos[2].vz = SCREEN_Z;
	pos[3].vx = screen_near_x * (center->vx + FlareData[i].vx);
	pos[3].vy = screen_near_y * (center->vy + FlareData[i].vy);
	pos[3].vz = SCREEN_Z;

	uvrgb[0].a = (int)(alpha*ratio + FlareData[i].vz) ;
	uvrgb[1].a = (int)(alpha*ratio + FlareData[i].vz) ;
	uvrgb[2].a = (int)(alpha*ratio + FlareData[i].vz) ;
	uvrgb[3].a = (int)(alpha*ratio + FlareData[i].vz) ;
    }
}

static void Act(Work *work)
{
    FMATRIX world  ;
    FVECTOR center ;

    _sceVu0ApplyMatrix( &center, work->world, &work->center ) ;

    TS_MakeMatrix( &world, (FVECTOR*)DG_Chanls[0].eye.m[2], &center ) ;
    DG_SetPos( &world ) ;
    DG_PutVector( work->init_check_pos, work->line_check_pos, 4 ) ;

    if ( GM_CheckPlayerStatus( PLAYER_WATCH ) )
	if ( work->tic++ >= 6*5/TIME_BASE )
	{
	    work->tic = 0 ; 

	    //投光器まぶしい(6fps連)
#ifdef PSX2		
	    GM_SeSetMode( SD_A_DAZZLING, &center, GM_SEMODE_BOMB ) ;
#else
		work->tgl ^= 1 ;
		if ( work->tgl ) {
			GM_SeSetMode( SD_A_DAZZLING, &center, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_A_DAZZLIN2, &center, GM_SEMODE_BOMB ) ;
		}
#endif
	}
    Flare_Act( work, &center ) ;
}

static void Die(Work *work)
{
    int i;

    for( i = 0; i < N_FLARES; i++)
	if( work->flare[i] )
	    GM_FreePrim2( work->flare[i] ) ;
}

//レンズフレア
#define	VERT_KICK_CODE	(0x8fff)
#define	DRAW_KICK_CODE	(0x0fff)
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define FLARE_FLAG (DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_ON_CAMERA)
static int InitFlarePrim( Work *work )
{
    DG_PRIM2		*prim;
    DG_TEX		*tex;
    DG_PRIM2_UVRGB	*uvrgb0,*uvrgb1;
    FVECTOR		*pos0,*pos1;
    int			i,j,u[2],v[2];

    for( i = 0; i < N_FLARES; i++ )
    {
	tex = DG_GetTexture(FlareTexCode[i]);
	//tex = DG_GetTexture(6715088);
	if(!tex)
	{
	    printf("ERR!! NO TEX!! <orga_lenz.c>\n");
	    return -1 ;
	}
	prim = work->flare[i] = GM_MakePrim2( FLARE_FLAG,N_PRIMS_F,N_VERTS_F );
	if ( !prim )
	{
	    printf("ERR!! MAKE PRIM!! <plant_sun.c>\n"); return (-1);
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
	    uvrgb0->a = uvrgb1->a = 0;//(int)ALPHA;

	    uvrgb0++; uvrgb1++;
	    pos0++; pos1++;

	}

    }
    return 0;
}

static int GetResources( Work *work )
{
    if ( InitFlarePrim( work ) )
	return -1 ;
	
    GV_SetActorChild(work,NewOnlineCheck_Static( GM_CurrentMap,
						 &work->line_check_pos[0], (FVECTOR*)DG_Chanls[0].eye.m[3],
						 HZX_CHK_F_SEGMENT, 
						 HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE, 
						 HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE,
						 &work->result[0],
						 4, 4, 100.0f  ));
    work->init_check_pos[0].vx = -work->size;
    work->init_check_pos[0].vy = work->size;
    work->init_check_pos[0].vz = 0.0f;
    work->init_check_pos[0].vw = 1.0f;

    if ( work->size == 0.0f )
    {
	work->result[1] = work->result[2] = work->result[3] = -1 ;
	return 0 ;
    }

    GV_SetActorChild(work,NewOnlineCheck_Static( GM_CurrentMap,
						 &work->line_check_pos[1], (FVECTOR*)DG_Chanls[0].eye.m[3],
						 HZX_CHK_F_SEGMENT, 
						 HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE, 
						 HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE,
						 &work->result[1],
						 4, 4, 100.0f  ));
    work->init_check_pos[1].vx = work->size;
    work->init_check_pos[1].vy = work->size;
    work->init_check_pos[1].vz = 0.0f;
    work->init_check_pos[1].vw = 1.0f;

    GV_SetActorChild(work,NewOnlineCheck_Static( GM_CurrentMap,
						 &work->line_check_pos[2], (FVECTOR*)DG_Chanls[0].eye.m[3],
						 HZX_CHK_F_SEGMENT, 
						 HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE, 
						 HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE,
						 &work->result[2],
						 4, 4, 100.0f  ));
    work->init_check_pos[2].vx = -work->size;
    work->init_check_pos[2].vy = -work->size;
    work->init_check_pos[2].vz = 0.0f;
    work->init_check_pos[2].vw = 1.0f;

    GV_SetActorChild(work,NewOnlineCheck_Static( GM_CurrentMap,
						 &work->line_check_pos[3], (FVECTOR*)DG_Chanls[0].eye.m[3],
						 HZX_CHK_F_SEGMENT, 
						 HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE, 
						 HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE,
						 &work->result[3],
						 4, 4, 100.0f  ));
    work->init_check_pos[3].vx = work->size;
    work->init_check_pos[3].vy = -work->size;
    work->init_check_pos[3].vz = 0.0f;
    work->init_check_pos[3].vw = 1.0f;

#ifndef PSX2
	work->tgl = 0 ;
#endif
    return 0;
}

void *NewOrgaLensFlr_Game( FMATRIX *world, FVECTOR *center, float size, int max_alpha )
{
    Work *work = NULL;

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof(Work) ) ;
    if(work){
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;

	work->world = world ;
	DG_COPY_VEC( &work->center, center ) ;
	work->max_alpha = max_alpha ;
	work->size = size ;
		
	if(GetResources( work ) < 0){
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }

    return (void *)work ;
}
