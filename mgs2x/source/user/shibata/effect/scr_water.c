//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scr_water.c
	画面壊れ
	
	2000/12/18 T.Shibata
	
	$Id: scr_water.c,v 1.4 2002/11/23 12:36:05 Yoshizawa1 Exp $

*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "libutl.h"

#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"
#include	"../util/ts_util.h"
#include	"../util/dma_set.h"

#include "BP_Renderer.h"

#define TEST

#define CLOCK_COUNT	(BP_BASE_TICK())


#define MDL_TEST	(SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 ))

#ifdef TEST
#define MDL_ALPHA	(SCE_GS_SET_ALPHA(0,1,2,1,72))
#define MDL_PRIM	(SCE_GS_SET_PRIM(4,1,1,0,1,0,1,0,0))
#else
#define MDL_ALPHA	(SCE_GS_SET_ALPHA(2,2,2,0,128))
#define MDL_PRIM	(SCE_GS_SET_PRIM(4,1,1,0,0,0,1,0,0))
#endif

#define	N_TATE		(11)
#define	N_YOKO		(11)

#define N_POINT_X	(N_YOKO+1)
#define N_POINT_Y	(N_TATE+1)
#define N_POINTS	(N_POINT_X*N_POINT_Y)
#define N_STRIP 	(N_POINT_X*2)
#define N_VERTS 	(N_STRIP*N_TATE)

#define INIT_X	(-(float)(DRAW_WIDTH/2))
#define INIT_Y	(-(float)(DRAW_HEIGHT/2))
#define DIFF_X	((float)DRAW_WIDTH/(float)N_YOKO)
#define DIFF_Y	((float)DRAW_HEIGHT/(float)N_TATE)
#define END_X	((float)(DRAW_WIDTH/2))
#define END_Y	((float)(DRAW_HEIGHT/2))

#if 0
#define SINPUKU_X	(4.0f)
#define SINPUKU_Y	(4.0f)
#define HATYOU_X	((2.0f * M_PI)/128.0f)
#define HATYOU_Y	((2.0f * M_PI)/128.0f)
#define SYUUKI_X	(60.0f)
#define SYUUKI_Y	(60.0f)
#define ADD_TX		((2.0f * M_PI)/SYUUKI_X)
#define ADD_TY		((2.0f * M_PI)/SYUUKI_Y)
#else

#define SINPUKU_X	(128)
#define SINPUKU_Y	(128)

#define PRE_LOOP_X	((SINPUKU_X<<1)+1)
#define PRE_LOOP_Y	((SINPUKU_Y<<1)+1)
#define LOOP_X		((PRE_LOOP_X<<1)+1)
#define LOOP_Y		((PRE_LOOP_Y<<1)+1)

#endif

#define		DIFF_COLOR_R	(128.0f-128.0f)
#define		DIFF_COLOR_G	(128.0f-128.0f)
#define		DIFF_COLOR_B	(160.0f-128.0f)

#define			NO_INDEX_FLAG	(0x8000)
#define			DRAW_INDEX_FLAG	(0x4000)
#define			GetIndex(_index)	((_index)&0x3fff)

#if 1
typedef struct {
	float		vx,vy;
	short		index0,index1;
	short		diff_u,diff_v;
	float		diff_ratio;

	float       x, y ;
	float       u, v ;
	short       r,g,b,a ;

} Point_Data;

#else
typedef FVECTOR Point_Data;
#endif

typedef	struct	{
	GV_ACT_EX			actor ;
	
	int					name ;
	int					map ;	
	int 				n_mverts;
	int					m_size;
	
	Point_Data			points[N_POINTS];
	void				*mverts[2];
	DG_DMAPACK			*dmapack;
	float				pre_cameralevel;
	short				flags,step;
	FVECTOR				dvec;
	FVECTOR				diff_color;
	float				warp;
	int					timer;

	void				*packet_mem;
	void                *prim ;
	int xbox_packet_size;

} Work ;

/*
ステップ
　　０：水の外
　　　　　スリープ中
　　１：水に入った
　　　　　激しい波から収束へ
　　２：水の中
　　　　　歪み起動中
　　３：水から出た
　　　　　歪み消しとスリープ処理
　　　　　１へ戻る
*/
extern float GM_WaterLevel;

#if 0
#define		MEM_SCR_POINTS	((void *)(SCRPAD_ADDR))
#define		MEM_SCR_MPACKE	((void *)(MEM_SCR_POINTS+sizeof(Point_Data)*N_POINTS))
#define		MEM_SCR_BOTTOM	((void *)(MEM_SCR_MPACKE+sizeof(DG_GIFTAG)+sizeof(MVERT_DATA)*N_VERTS))
#else
#define		MEM_SCR_MPACKE	((void *)(SCRPAD_ADDR))
#define		MEM_SCR_POINTS	((void *)(SCRPAD_ADDR))
#define		MEM_SCR_BOTTOM	((void *)(MEM_SCR_MPACKE+sizeof(DG_GIFTAG)+sizeof(MVERT_DATA)*N_VERTS))
#endif

static void InitMVerts( Work *work )
{
	DG_GIFTAG		*giftag = MEM_SCR_MPACKE;//(DG_GIFTAG*)work->mverts[0];
	MVERT_DATA		*mverts = (MVERT_DATA*)(giftag+1);
	Point_Data		*point = work->points;//MEM_SCR_POINTS;
	int	i,j;
	int ix,iy,iu,iv;
	//float	fdu,fdv;
	float	ftemp0,ftemp1,ftemp2,ftemp3;
	
	giftag->tag  = SCE_GIF_SET_TAG(SIZEOF_QWORD(MVERT_DATA)*work->n_mverts, 1, 0, 0, 0, 1);
	giftag->regs = GS_REGS_AD;

	mverts = (MVERT_DATA*)(giftag+1);

	for( i = 0; i < N_POINT_Y; i++ ){
		for( j = 0; j < N_POINT_X; j++ ){
			point->vx = INIT_X + DIFF_X * (float)j;
			point->vy = INIT_Y + DIFF_Y * (float)i;
			if(j==N_POINT_X-1) point->vx = END_X;
			if(i==N_POINT_Y-1) point->vy = END_Y;

			point->diff_ratio = 0.0f;//(float)i/(float)(N_POINT_Y-1);

			point->index0 = -1;//NO_INDEX_FLAG;
			point->index1 = -1;//NO_INDEX_FLAG;


			//ずらしのランダマイズ
			ftemp0 = sinf(2.0f*PI*(float)(i*N_POINT_X+j)/(float)N_POINTS);
			ftemp1 = sinf(3.5f*PI*(float)(i*N_POINT_X+j)/(float)N_POINTS);
			ftemp2 = sinf(5.0f*PI*(float)(i*N_POINT_X+j)/(float)N_POINTS);
			ftemp3 = sinf(6.6f*PI*(float)(i*N_POINT_X+j)/(float)N_POINTS);

			ftemp0 *= ftemp0;
			ftemp2 *= ftemp2;

			
			point->diff_u = (float)(LOOP_X/4)*ftemp0*3.0f+(float)(LOOP_X/4)*ftemp1;
			point->diff_v = (float)(LOOP_X/4)*ftemp2*3.5f+(float)(LOOP_X/4)*ftemp3*0.5f;

			point->x = DRAW_WIDTH /2 + point->vx ;
			point->y = DRAW_HEIGHT/2 + point->vy ;
			point->u = DG_FRAME_U( (float)(DRAW_WIDTH /2) + point->vx ) ;
			point->v = DG_FRAME_V( (float)(DRAW_HEIGHT/2) + point->vy ) ;

			point->r = 128 ;
			point->g = 128 ;
			point->b = 128 ;

			point++;
		}
	}

	TS_Scr_Mem( work->mverts[0], MEM_SCR_MPACKE, sizeof(char), work->m_size );
	TS_Scr_Mem( work->mverts[1], MEM_SCR_MPACKE, sizeof(char), work->m_size );

}



static inline float TS_Bound( float _f, float _max, float _min)
{
	float _t ;

#ifdef BP_PSX2_ASM
	asm ("
    min.s %0,%1,%2
    max.s %0,%0,%3
    ":"=f"(_t):"f"(_f),"f"(_max),"f"(_min));
#else
	_t = ( _f < _max ) ? _f : _max ;
	_t = ( _t > _min ) ? _t : _min ;
#endif
	return _t ;
}

static int CheckWaterLevel(Work *work)
{
	FVECTOR	*eye = (FVECTOR*)DG_Chanls[0].eye.m[3];
	
	int		ret = 0;

	//水に入るか？
	if( eye->vy < GM_WaterLevel ){
		ret = 1;
	}
	//水から出たか？
	if( eye->vy > GM_WaterLevel ){
		//FVECTOR fvtemp;
		DG_SetPos( &DG_Chanls[0].eye_inv );
		DG_RotVector( (FVECTOR*)DG_Chanls[0].eye.m[1], &work->dvec, 1 );
		//DG_RotVector( &(FVECTOR){0.0f,256.0f,0.0f,0.0f}, &fvtemp, 1 );

		//work->dvec.vx = DG_Chanls[0].eye.m[1][0];
		//work->dvec.vy = DG_Chanls[0].eye.m[1][1];
		//work->dvec.vz = DG_Chanls[0].eye.m[1][0] * -256.0f;
		//work->dvec.vw = DG_Chanls[0].eye.m[1][1] * -256.0f;
		
		work->dvec.vz = work->dvec.vx * -256.0f;
		work->dvec.vw = work->dvec.vy * -256.0f;

		//printf("dvec x:%f y:%f |pos x:%f y:%f\n",work->dvec.vx,work->dvec.vy,work->dvec.vz,work->dvec.vw);
		ret = 3;
	}

	work->pre_cameralevel = eye->vy;
	
	return ret;
}

//割合変化
//モード　０：激しくなる
//モード　１：収束
//モード　３：流れ落ちる
//dvec		：流れ落ちる方向
static int DiffRatio_Act( Work *work, int mode )
{
	Point_Data 	*point = work->points;
	int 		i,check=0;
	FVECTOR		fvtemp;
	
	switch(mode){
	  case 0:
		for( i = 0; i < N_POINTS; i++ ){
			point->diff_ratio += 0.05f * rnd() + 0.075f;

			point->diff_u += CLOCK_COUNT;//irnd()%CLOCK_COUNT+1;
			point->diff_v += CLOCK_COUNT;//irnd()%CLOCK_COUNT+1;
			if(point->diff_u > LOOP_X) point->diff_u -= LOOP_X<<1;
			if(point->diff_v > LOOP_Y) point->diff_v -= LOOP_Y<<1;

			if(point->diff_ratio > 1.0f){
				point->diff_ratio = 1.0f;
				check++;
			}
			point++;
		}
		break;
	  case 1:
		for( i = 0; i < N_POINTS; i++ ){
			point->diff_ratio -= 0.05f * rnd();
			if(point->diff_ratio < 1.0f){
				point->diff_ratio = 1.0f;
				check++;
			}
			point++;
		}
		break;
	  case 2:
		fvtemp.vx = work->dvec.vx * (42.667f*(float)work->timer);
		fvtemp.vy = work->dvec.vy * (42.667f*(float)work->timer);
		fvtemp.vz = work->dvec.vz;
		fvtemp.vw = work->dvec.vw;
		//_sceVu0AddVector(&fvtemp, &fvtemp,
		//				 (&(FVECTOR){ (float)-DRAW_WIDTH/2, (float)-DRAW_HEIGHT/2, 0.0f, 0.0f }));
		work->timer++;

		for( i = 0; i < N_POINTS; i++ ){
			float dx,dy;
			
			if(fvtemp.vx < 0.0001f && -0.0001f < fvtemp.vx ) dx = 0.0f;
			else dx = (point->vx - fvtemp.vz) / fvtemp.vx;
			
			if(fvtemp.vy < 0.0001f && -0.0001f < fvtemp.vy ) dy = 0.0f;
			else dy = (point->vy - fvtemp.vw) / fvtemp.vy;
			if( dx + dy <= 1.0f ){
				point->diff_ratio -= 0.05f;// * rnd();
			
				if(point->diff_ratio < 0.0f){
					point->diff_ratio = 0.0f;
					//printf("%d\n",i);
					check++;
				}
			}
			point++;
		}
		//printf("\n");
		break;
	  default:
		break;
	}

	if(check==N_POINTS) return 1;
	return 0;
	
}

static void ScrWarp( Work *work )
{
	DG_GIFTAG		*giftag = (DG_GIFTAG*)work->mverts[DG_Clock];//MEM_SCR_MPACKE;//(DG_GIFTAG*)work->mverts[0];
	MVERT_DATA	*mverts = (MVERT_DATA*)(giftag+1);
	Point_Data		*point = work->points;
	int	i;
	float fdu,fdv,fu,fv;
	int iu,iv,uv;
	CVECTOR			color;
	
	for( i = 0; i < N_POINTS; i++ )
   {
		point->diff_u += CLOCK_COUNT*2;//irnd()%CLOCK_COUNT+1;
		point->diff_v += CLOCK_COUNT*2;//irnd()%CLOCK_COUNT+1;
		if(point->diff_u > LOOP_X) point->diff_u -= LOOP_X<<1;
		if(point->diff_v > LOOP_Y) point->diff_v -= LOOP_Y<<1;
			
		fdu = (float)(abs(point->diff_u) - SINPUKU_X) * point->diff_ratio / work->warp;
		fdv = (float)(abs(point->diff_v) - SINPUKU_Y) * point->diff_ratio / work->warp;
		
		fu = ((float)(DRAW_WIDTH/2) + point->vx + fdu );
		fv = ((float)(DRAW_HEIGHT/2) + point->vy + fdv );
		
		fu = TS_Bound( fu, (float)DRAW_WIDTH-0.5f, 0.5f );
		fv = TS_Bound( fv, (float)DRAW_HEIGHT-0.5f, 0.5f );
		point->u = DG_FRAME_U( fu ) ;
		point->v = DG_FRAME_V( fv ) ;

		point->r = 128 + (char)(work->diff_color.vx * point->diff_ratio);
		point->g = 128 + (char)(work->diff_color.vy * point->diff_ratio);
		point->b = 128 + (char)(work->diff_color.vz * point->diff_ratio);
		point++;
	}
}

static void PauseAct(Work *work)
{
	DG_GIFTAG		*giftag = (DG_GIFTAG*)work->mverts[DG_Clock];
	MVERT_DATA		*mverts = (MVERT_DATA*)(giftag+1);
	Point_Data		*point = work->points;
	int	i;
	float fdu,fdv,fu,fv;
	int iu,iv,uv;
	CVECTOR			color;
	
	for( i = 0; i < N_POINTS; i++ ){
		fdu = (float)(abs(point->diff_u) - SINPUKU_X) * point->diff_ratio / work->warp;
		fdv = (float)(abs(point->diff_v) - SINPUKU_Y) * point->diff_ratio / work->warp;
		
		fu = ((float)(DRAW_WIDTH/2) + point->vx + fdu );
		fv = ((float)(DRAW_HEIGHT/2) + point->vy + fdv );
		
		fu = TS_Bound( fu, (float)DRAW_WIDTH-0.5f, 0.5f );
		fv = TS_Bound( fv, (float)DRAW_HEIGHT-0.5f, 0.5f );
		point->u = DG_FRAME_U( fu ) ;
		point->v = DG_FRAME_V( fv ) ;
		point->r = 128 + (char)(work->diff_color.vx * point->diff_ratio);
		point->g = 128 + (char)(work->diff_color.vy * point->diff_ratio);
		point->b = 128 + (char)(work->diff_color.vz * point->diff_ratio);

		point++;
	}

}

static void MakeDmaMesh( Work *work )
{
	Point_Data *point = work->points;
	Point_Data *p    ;
	int       	x, y ;
	
	for( y=N_TATE ; --y>=0 ; ){
		work->prim = DG_SetDmapackTriangleStrip( work->prim, N_POINT_X*2 );
		for( x=N_POINT_X ; --x>=0 ; ){
			p  = point     ;
			work->prim = DG_SetDmapackVertex( work->prim,  p->x,p->y,  p->u,p->v,
											 DG_MakeDMAPackColor(p->r,p->g,p->b,0x80) ) ;
			p += N_POINT_X ;
			work->prim = DG_SetDmapackVertex( work->prim,  p->x,p->y,  p->u,p->v,
											 DG_MakeDMAPackColor(p->r,p->g,p->b,0x80) ) ;
			point++ ;
		}
	}
}

static void OutWaterWait(Work *work)
{
	if(!work->timer++)
		work->dmapack->flag |= DG_DMAPACK_INVISIBLE0;
	if(CheckWaterLevel(work)==1){
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0;
		work->step++;
		work->timer = 0;
		//printf("to Water \n");
	}
}

static void InWaterActive(Work *work)
{
	if( GM_CheckGameStatus( STATE_DISP_GAMEOVER ) ||
	   ( GM_CheckMenuStatus( MENU_WEAPON_OPEN | MENU_ITEM_OPEN | MENU_RADIO_ON ) )){
		if( work->flags & 0x0001 )
			work->dmapack->flag |= DG_DMAPACK_INVISIBLE0;
		work->flags |= 0x0001;
		PauseAct(work);
		return;
	}else if( (GV_PauseLevel & GV_PAUSE_PAUSE) ){
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0;
		PauseAct(work);
		return;
	}else{
		work->flags &= ~0x0001;
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0;
	}

	if(CheckWaterLevel(work)==3){
		work->timer = 0;
		work->step = 3;
		//printf("from Water \n");
		
	}else if(DiffRatio_Act( work, work->timer )){
		work->timer++;
		work->timer = 0;
		work->step++;
		//printf("in Water \n");
	}
	ScrWarp( work );
/*
	if(work->timer >= 2){
		work->timer = 0;
		work->step++;
		//printf("in Water \n");
	}
*/
}

static void InWaterWait(Work *work)
{
	if( GM_CheckGameStatus( STATE_DISP_GAMEOVER ) ||
	   ( GM_CheckMenuStatus( MENU_WEAPON_OPEN | MENU_ITEM_OPEN | MENU_RADIO_ON ) )){
		if( work->flags & 0x0001 )
			work->dmapack->flag |= DG_DMAPACK_INVISIBLE0;
		work->flags |= 0x0001;
		PauseAct(work);
		return;
	}else if( (GV_PauseLevel & GV_PAUSE_PAUSE) ){
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0;
		PauseAct(work);
		return;
	}else{
		work->flags &= ~0x0001;
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0;
	}

	if(CheckWaterLevel(work)==3){
		work->step++;
		work->timer = 0;
		//printf("from Water \n");
	}
	ScrWarp( work );
}

static void OutWaterActive(Work *work)
{
	if( GM_CheckGameStatus( STATE_DISP_GAMEOVER ) ||
	   ( GM_CheckMenuStatus( MENU_WEAPON_OPEN | MENU_ITEM_OPEN | MENU_RADIO_ON ) )){
		if( work->flags & 0x0001 )
			work->dmapack->flag |= DG_DMAPACK_INVISIBLE0;
		work->flags |= 0x0001;
		PauseAct(work);
		return;
	}else if( (GV_PauseLevel & GV_PAUSE_PAUSE) ){
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0;
		PauseAct(work);
		return;
	}else{
		work->flags &= ~0x0001;
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0;
	}

	if(CheckWaterLevel(work)==1){
		work->step = 1;
		work->timer = 0;
		//printf("to Water \n");
	}else if(DiffRatio_Act( work, 2 )){
		work->timer = -1;
		work->step = 0;
		//printf("out Water \n");
	}
	ScrWarp( work );
}

static void Act( Work *work )
{
	work->prim = work->dmapack->autopacket ;
	work->prim = InitTrBuffer( work->prim, 0 ) ;
	work->prim = InitMdlDraw( work->prim, MDL_TEST, MDL_ALPHA, MDL_PRIM ) ;

	switch(work->step){
	  case 0:
		OutWaterWait(work);
		break;
	  case 1:
		InWaterActive(work);
		break;
	  case 2:
		InWaterWait(work);
		break;
	  case 3:
		OutWaterActive(work);
		break;
	  default:
		break;
	}

#if !BP_VITA
#if BP_RENDER_SINGLE_RENDER_TARGET
   // One render target?  Never render this water screen effect.
   work->dmapack->flag |= DG_DMAPACK_INVISIBLE0;
#endif
#endif

	MakeDmaMesh( work ) ;
	work->prim = DG_SetDmapackEnd( work->prim ) ;
	//printf( "scr_water.c need Size%d\n",  (int)work->prim-(int)work->packet_mem  ) ;
	if( work->xbox_packet_size < ((int)work->prim-(int)work->packet_mem) ){
		printf("xbox_packet_size not enough!!\n");
		ASSERT( 0 );
	}	

}

static void Die( Work *work )
{
	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}

	if(work->packet_mem) GV_DelayedFree( work->packet_mem );
	if(work->mverts[0]) GV_DelayedFree(work->mverts[0]);

}

static int GetResources( Work *work, int mode, int camera_num )
{
	DG_DMAPACK		*dmapack;
	u_char			*verts;

//	work->mode = mode;
//	work->camera_num = camera_num;
	
	work->pre_cameralevel = DG_Chanls[0].eye.m[3][1];
	
	work->n_mverts = N_VERTS;	//モデルの頂点数
	work->m_size = sizeof(DG_GIFTAG)+sizeof(MVERT_DATA)*work->n_mverts;
	work->timer = 0;
	
	if(GCL_GetOption('c')){
		work->diff_color.vx = (float)GCL_GetNextInt()-128.0f;
		work->diff_color.vy = (float)GCL_GetNextInt()-128.0f;
		work->diff_color.vz = (float)GCL_GetNextInt()-128.0f;
	}else{
		work->diff_color.vx = DIFF_COLOR_R;
		work->diff_color.vy = DIFF_COLOR_G;
		work->diff_color.vz = DIFF_COLOR_B;
	}
	if(GCL_GetOption('w')){
		work->warp = 128.0f/(float)GCL_GetNextInt();
	}else{
		work->warp = 32.0f;
	}
	// 頂点データメモり確保
	verts = GV_Malloc( work->m_size*2 );
	if(verts == NULL){ printf("ERR!! MALLOC MVERTS!!\n"); return -1; }

	work->mverts[0] = (MVERT_DATA*)&verts[0];
	work->mverts[1] = (MVERT_DATA*)&verts[work->m_size];
	
	// ＤＭＡパケット型オブジェクト作成
	work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL|DG_DMAPACK_INVISIBLE, DG_DMAPACK_PHASE_AFTER, 144 );
	if(dmapack==NULL){ printf("ERR!! MAKE DMAPACK!!\n"); return -1; }
	DG_QueueDmapack( dmapack );

	
	if(GM_WaterLevel > work->pre_cameralevel){
		work->step = 1;
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0;
	}else{
		work->step = 0;
		work->dmapack->flag |= DG_DMAPACK_INVISIBLE0;
	}

	work->xbox_packet_size = 4350+8;
	if ( !(work->packet_mem = GV_Malloc( work->xbox_packet_size )) ) {
		printf("ERR!! MALLOC scr_water.c!!\n");
		return -1 ;
	}
	dmapack->autopacket = work->packet_mem ;
	DG_SetDmapackEnd( dmapack->autopacket ) ;

	//printf( "scr_water.c need Size%d\n",  (int)prim-(int)work->packet_mem  ) ;

	InitMVerts( work );

	return (0);
}
static Work *g_work = NULL;

void *NewScrWater( int name, int map )
{
	Work		*work ;
	//printf("SCR_BOTTOM = %p\n", MEM_SCR_BOTTOM);
	//if(g_work) return NULL;

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
//	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, 0, 0 ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		g_work = work;
	}
	
	//printf("size/128 = %f\n",(float)work->m_size/(float)sizeof(u_long128));
	//printf("Points Num = %d\n", N_POINTS);
	//printf("Verts Num = %d\n", N_VERTS);

	return (void *)work ;
}
