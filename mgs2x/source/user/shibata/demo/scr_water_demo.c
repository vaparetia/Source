//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scr_water.c
	画面壊れ
	
	2000/12/18 T.Shibata
	
	$Id: scr_water_demo.c,v 1.1.1.3 2002/11/19 11:48:34 Yoshizawa1 Exp $

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
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"
#include	"../util/ts_util.h"
#include	"../util/dma_set.h"

//#define TEST

#define CLOCK_COUNT	(BP_BASE_TICK())

#define MDL_PRIM	(SCE_GS_SET_PRIM(4,1,1,0,0,0,1,0,0))
#define MDL_TEST	(SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 ))

#ifdef TEST
#define MDL_ALPHA	(SCE_GS_SET_ALPHA(0,1,2,1,16))
#else
#define MDL_ALPHA	(SCE_GS_SET_ALPHA(2,2,2,0,128))
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


typedef struct {
	float		vx,vy;
	short		index0,index1;
	short		len,rot;

#ifndef PSX2
	float       x, y ;
	float       u, v ;
	short       r,g,b,a ;
#endif
} Point_Data;

typedef struct {
	DG_DMATAG		dmatag_tr;			// ＧＩＦ接続ＤＭＡタグ
	TR_BUFFER		tr_buffer;			//								退避(フレームをバックに)
	DG_DMATAG		dmatag_mdl;			// ＧＩＦ接続ＤＭＡタグ
	MDL_DRAW		mdl_draw;			// 								モデル描画の設定
	DG_DMATAG		dmatag_mverts;		// ＧＩＦ接続ＤＭＡタグ			モデル頂点
	DG_DMATAG		dmatag_test;		// ＧＩＦ接続ＤＭＡタグ
	PACKET_END		end_paket;			//								ピクセルテスト復元
	DG_DMATAG		dmatag_offset;		// オフセット環境復元用
	DG_DMATAG		dmatag_end;			// RETタグ
}WARP_PACK;

typedef	struct	{
	GV_ACT_EX			actor ;
	
	int					name ;
	int					map ;	
	int 				n_mverts;
	int					m_size;
	
	Point_Data			points[N_POINTS];
	WARP_PACK			*warp_pack[2];
	void				*mverts[2];
	DG_DMAPACK			*dmapack;
	FVECTOR				diff_color;
	float				warp;
	float				timer;
	float				time_scl;


#ifndef PSX2
	void				*packet_mem;
	void                *prim ;
	int xbox_packet_size;
#endif

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

#define TestWaterLevel GM_WaterLevel

#if 0
#define		MEM_SCR_POINTS	((void *)(SCRPAD_ADDR))
#define		MEM_SCR_MPACKE	((void *)(MEM_SCR_POINTS+sizeof(Point_Data)*N_POINTS))
#define		MEM_SCR_BOTTOM	((void *)(MEM_SCR_MPACKE+sizeof(DG_GIFTAG)+sizeof(MVERT_DATA)*N_VERTS))
#else
#define		MEM_SCR_MPACKE	((void *)(SCRPAD_ADDR))
#define		MEM_SCR_POINTS	((void *)(SCRPAD_ADDR))
#define		MEM_SCR_BOTTOM	((void *)(MEM_SCR_MPACKE+sizeof(DG_GIFTAG)+sizeof(MVERT_DATA)*N_VERTS))
#endif

static void InitDmaTags( Work *work, WARP_PACK *packet, int which )
{
	// --------  DMATag  ----------------------------
	// ＤＭＡタグ		退避
	packet->dmatag_tr.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(TR_BUFFER) ) ;
	packet->dmatag_tr.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_tr.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(TR_BUFFER), 0) ;
	// ＤＭＡタグ		モデル描画設定
	packet->dmatag_mdl.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(MDL_DRAW) ) ;
	packet->dmatag_mdl.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_mdl.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(MDL_DRAW), 0) ;
	// ＤＭＡタグ		ピクセルテスト復元
	packet->dmatag_test.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(PACKET_END) ) ;
	packet->dmatag_test.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_test.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(PACKET_END), 0) ;
	// オフセット環境復元用
	packet->dmatag_offset.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWOFFSET) ) ;
	packet->dmatag_offset.addr = &( DG_Chanl( 0 )->draw_offset[ which ] ) ;
	packet->dmatag_offset.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_offset.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWOFFSET), 0) ;
	// RETタグ
	packet->dmatag_end.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 ) ;
	packet->dmatag_end.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_end.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	
	// ＤＭＡタグ		モデル頂点
	packet->dmatag_mverts.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, work->m_size/sizeof(u_long128) ) ;
	packet->dmatag_mverts.addr = work->mverts[which];
	packet->dmatag_mverts.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_mverts.vifcode[1] = SCE_VIF1_SET_DIRECT( work->m_size/sizeof(u_long128), 0) ;
}

static void InitMVerts( Work *work )
{
	DG_GIFTAG		*giftag = MEM_SCR_MPACKE;//(DG_GIFTAG*)work->mverts[0];
	MVERT_DATA		*mverts = (MVERT_DATA*)(giftag+1);
	Point_Data		*point = work->points;//MEM_SCR_POINTS;
	int	i,j;
	int ix,iy,iu,iv;
	//float	fdu,fdv;
	//float	ftemp0,ftemp1,ftemp2,ftemp3;
	
	giftag->tag  = SCE_GIF_SET_TAG(SIZEOF_QWORD(MVERT_DATA)*work->n_mverts, 1, 0, 0, 0, 1);
	giftag->regs = GS_REGS_AD;

	mverts = (MVERT_DATA*)(giftag+1);

	for( i = 0; i < N_POINT_Y; i++ ){
		for( j = 0; j < N_POINT_X; j++ ){
			point->vx = INIT_X + DIFF_X * (float)j;
			point->vy = INIT_Y + DIFF_Y * (float)i;
			if(j==N_POINT_X-1) point->vx = END_X;
			if(i==N_POINT_Y-1) point->vy = END_Y;

			point->len = (short)(work->warp * 32.0f);
			
			point->index0 = -1;//NO_INDEX_FLAG;
			point->index1 = -1;//NO_INDEX_FLAG;

#if 0
			printf("%d	x:%f y:%f\n",i*N_POINT_X+j,
				   point->vx,point->vy);
//			printf("%d	x:%f y:%f u:%d v:%d\n",i*N_POINT_X+j,
//				   point->vx,point->vy,point->diff_u,point->diff_v);
#endif

#ifdef PSX2
			ix = DG_FTOI((2048.0f + point->vx)*SHIBATA_FSHIFT);
			iy = DG_FTOI((2048.0f + point->vy)*SHIBATA_FSHIFT);
			iu = DG_FTOI(((float)(DRAW_WIDTH/2) + point->vx)*SHIBATA_FSHIFT);
			iv = DG_FTOI(((float)(DRAW_HEIGHT/2) + point->vy)*SHIBATA_FSHIFT);

			if(iu>MAX_U) iu = MAX_U;
			else if(iu<MIN_UV) iu = MIN_UV;
			if(iv>MAX_V) iv = MAX_V;
			else if(iv<MIN_UV) iv = MIN_UV;
			
			if( i!=N_TATE ){
				mverts[ i*N_STRIP + j*2 ].rgbq.reg = SCE_GS_RGBAQ;
				mverts[ i*N_STRIP + j*2 ].uv.reg = SCE_GS_UV;
				mverts[ i*N_STRIP + j*2 ].xyz.reg = (j)?SCE_GS_XYZ2:SCE_GS_XYZ3;
				mverts[ i*N_STRIP + j*2 ].rgbq.data =
					SCE_GS_SET_RGBAQ(128,128,128,0x80,0x3F800000);
				mverts[ i*N_STRIP + j*2 ].xyz.data = SCE_GS_SET_XYZ(ix,iy,0);
				mverts[ i*N_STRIP + j*2 ].uv.data = SCE_GS_SET_UV(iu,iv);
				//printf("write index0 %d\n",i*N_STRIP + j*2);
				
				point->index0 = i*N_STRIP + j*2;
				//point->index0 = (i*N_STRIP + j*2) | NO_INDEX_FLAG;
				//if(j) point->index0 |= DRAW_INDEX_FLAG;
			}

			if( i ){
				mverts[ (i-1)*N_STRIP + j*2+1 ].rgbq.reg = SCE_GS_RGBAQ;
				mverts[ (i-1)*N_STRIP + j*2+1 ].uv.reg = SCE_GS_UV;
				mverts[ (i-1)*N_STRIP + j*2+1 ].xyz.reg = (j)?SCE_GS_XYZ2:SCE_GS_XYZ3;
				mverts[ (i-1)*N_STRIP + j*2+1 ].rgbq.data =
					SCE_GS_SET_RGBAQ(128,128,128,0x80,0x3F800000);
				mverts[ (i-1)*N_STRIP + j*2+1 ].xyz.data = SCE_GS_SET_XYZ(ix,iy,0);
				mverts[ (i-1)*N_STRIP + j*2+1 ].uv.data = SCE_GS_SET_UV(iu,iv);
				//printf("write index1 %d\n",(i-1)*N_STRIP + j*2+1);

				point->index1 = (i-1)*N_STRIP + j*2+1;
				//point->index1 = ((i-1)*N_STRIP + j*2+1) | NO_INDEX_FLAG;
				//if(j) point->index1 |= DRAW_INDEX_FLAG;
			}
#else
			point->x = DRAW_WIDTH /2 + point->vx ;
			point->y = DRAW_HEIGHT/2 + point->vy ;
			point->u = DG_FRAME_U( (float)(DRAW_WIDTH /2) + point->vx ) ;
			point->v = DG_FRAME_V( (float)(DRAW_HEIGHT/2) + point->vy ) ;

			point->r = 128 ;
			point->g = 128 ;
			point->b = 128 ;
#endif
			point++;
		}
	}

	//memcpy( work->mverts[0], MEM_SCR_MPACKE, work->m_size);
	//memcpy( work->mverts[1], MEM_SCR_MPACKE, work->m_size);

	TS_Scr_Mem( work->mverts[0], MEM_SCR_MPACKE, sizeof(char), work->m_size );
	TS_Scr_Mem( work->mverts[1], MEM_SCR_MPACKE, sizeof(char), work->m_size );
	
//	TS_Scr_Mem( work->points, MEM_SCR_POINTS, sizeof(FVECTOR), N_POINTS );

	
//	printf("verts = %d\n",cnt);

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
	_t = (_max<_f ? _max :
		  _min>_f ? _min : _f ) ;
#endif
	
	return _t ;
}

static void ScrWarp( Work *work )
{
	DG_GIFTAG		*giftag = (DG_GIFTAG*)work->mverts[DG_Clock];
	MVERT_DATA		*mverts = (MVERT_DATA*)(giftag+1);
	Point_Data		*point = work->points;
	int	i;
	float fdu,fdv,fu,fv;
	int iu,iv,uv;
	CVECTOR			color;
	
	for( i = 0; i < N_POINTS; i++ ){

//TS_SINs(_s) TS_SINs( (work->timer&0x0fff) - (point->vx/256.0f) )
//pos->vy = 1024.0f + 256.0f * sinf( work->step*PI/32 - pos->vx/800.0f ) + 256.0f * sinf( work->step*PI/16 - pos->vz/800.0f );
		fdu = //4.0f * TS_COSs( ((short)(work->timer * 16.0f)&0x0fff) - (short)(8.0f * point->vx) ) +
			  //3.0f * TS_COSs( ((short)(work->timer * 24.0f)&0x0fff) - (short)(2.0f * point->vx) ) +
			  //2.0f * TS_COSs( ((short)(work->timer * 32.0f)&0x0fff) - (short)(4.0f * point->vx) ) +
			  8.0f * TS_COSs( ((short)(work->timer * 14.0f )&0x0fff) - (short)(point->vx * 8.0f) );
		
		fdv = //3.0f * TS_SINs( ((short)(work->timer * 40.0f)&0x0fff) - (short)(2.0f * point->vy) ) +
			  //7.0f * TS_SINs( ((short)(work->timer * 32.0f)&0x0fff) - (short)(4.0f * point->vy) ) +
			  //2.0f * TS_SINs( ((short)(work->timer * 64.0f)&0x0fff) - (short)(point->vy) ) +
			  8.0f * TS_SINs( ((short)(work->timer * 14.0f )&0x0fff) - (short)(point->vy * 8.0f) );
		fdu = work->warp * fdu / 8.0f;//17.0f;
		fdv = work->warp * fdv / 8.0f;//34.0f; 

		//if( i == 0 ) printf( "SHIBA [%d] du %f: dv %f\n", work->timer, fdu, fdv );
		fu = ((float)(DRAW_WIDTH/2) + point->vx + fdu );
		fv = ((float)(DRAW_HEIGHT/2) + point->vy + fdv );
		
		fu = TS_Bound( fu, (float)DRAW_WIDTH-0.5f, 0.5f );
		fv = TS_Bound( fv, (float)DRAW_HEIGHT-0.5f, 0.5f );
#ifdef PSX2
		iu = DG_FTOI(fu * SHIBATA_FSHIFT);
		iv = DG_FTOI(fv * SHIBATA_FSHIFT);
#if 1
		color.r = 128 + (char)(work->diff_color.vx);
		color.g = 128 + (char)(work->diff_color.vy);
		color.b = 128 + (char)(work->diff_color.vz);
#endif
		uv = SCE_GS_SET_UV(iu,iv);

		if( point->index0 >= 0 ){
			mverts[point->index0].uv.data = uv;
#if 1
			mverts[point->index0].rgbq.data =
				SCE_GS_SET_RGBAQ(color.r,color.g,color.b,0x80,0x3F800000);
#endif
		}
		if( point->index1 >= 0){
			mverts[point->index1].uv.data = uv;
#if 1
			mverts[point->index1].rgbq.data =
				SCE_GS_SET_RGBAQ(color.r,color.g,color.b,0x80,0x3F800000);
#endif
		}

#else
		point->u = DG_FRAME_U( fu * SHIBATA_FSHIFT ) ;
		point->v = DG_FRAME_V( fv * SHIBATA_FSHIFT ) ;

		point->r = 128 + (char)(work->diff_color.vx);
		point->g = 128 + (char)(work->diff_color.vy);
		point->b = 128 + (char)(work->diff_color.vz);
#endif
		point++;
	}

//	TS_Scr_Mem( work->mverts[DG_Clock], MEM_SCR_MPACKE, sizeof(u_long128), work->m_size/sizeof(u_long128) );

}


#ifndef PSX2
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
											 SCE_GS_SET_RGBAQ(p->r,p->g,p->b,0x80,0) ) ;
			p += N_POINT_X ;
			work->prim = DG_SetDmapackVertex( work->prim,  p->x,p->y,  p->u,p->v,
											 SCE_GS_SET_RGBAQ(p->r,p->g,p->b,0x80,0) ) ;
			point++ ;
		}
	}
}
#endif

static void Act( Work *work )
{
#ifndef PSX2
	work->prim = work->dmapack->autopacket ;
	work->prim = InitTrBuffer( work->prim, 0 ) ;
	work->prim = InitMdlDraw( work->prim, MDL_TEST, MDL_ALPHA, MDL_PRIM ) ;
#endif

	//if(!(GV_PauseLevel & GV_PAUSE_PAUSE))
	work->timer += work->time_scl * (float)CLOCK_COUNT/6.0f;
	ScrWarp( work );


#ifndef PSX2
	MakeDmaMesh( work ) ;
	DG_SetDmapackEnd( work->prim ) ;
	if( work->xbox_packet_size < ((int)work->prim-(int)work->packet_mem) ){
		printf("xbox_packet_size not enough!!\n");
		ASSERT( 0 );
	}
#endif
}

static void Die( Work *work )
{
	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
	
#ifdef PSX2	
	if(work->warp_pack[0]) GV_DelayedFree(work->warp_pack[0]);
#else
	if(work->packet_mem) GV_DelayedFree( work->packet_mem );
#endif
	if(work->mverts[0]) GV_DelayedFree(work->mverts[0]);
}

static int GetResources( Work *work, float warp, int col )
{
	DG_DMAPACK		*dmapack;
	WARP_PACK		*packet;
	u_char			*verts;
	int				i;

	
	work->n_mverts = N_VERTS;	//モデルの頂点数
	work->m_size = sizeof(DG_GIFTAG)+sizeof(MVERT_DATA)*work->n_mverts;
	work->timer = 0.0f;
	
	work->diff_color.vx = ((col>>24)&0xff)-128.0f;
	work->diff_color.vy = ((col>>16)&0xff)-128.0f;
	work->diff_color.vz = ((col>>8)&0xff)-128.0f;

	work->warp = warp;

	// 頂点データメモり確保
	verts = GV_Malloc( work->m_size*2 );
	if(verts == NULL){ printf("ERR!! MALLOC MVERTS!!\n"); return -1; }

	work->mverts[0] = (MVERT_DATA*)&verts[0];
	work->mverts[1] = (MVERT_DATA*)&verts[work->m_size];
	
	// ＤＭＡパケット型オブジェクト作成
	work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL, DG_DMAPACK_PHASE_AFTER, 144 );
	if(dmapack==NULL){ printf("ERR!! MAKE DMAPACK!!\n"); return -1; }
	DG_QueueDmapack( dmapack );

#ifdef PSX2
	// パケットメモリ割り当て
	packet = GV_Malloc( sizeof(WARP_PACK) * 2 );
	if(packet == NULL){ printf("ERR!! MALLOC WARP_PACK!!\n"); return -1; }
	
	work->warp_pack[0] = dmapack->packet[0] = &packet[0] ;
	work->warp_pack[1] = dmapack->packet[1] = &packet[1] ;
	
	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaTags( work, &packet[i], i );
		InitTrBuffer( &packet[i].tr_buffer, i );
		InitMdlDraw( &packet[i].mdl_draw, MDL_TEST, MDL_ALPHA, MDL_PRIM );
		InitEndPacket( &packet[i].end_paket );
		
		packet[i].mdl_draw.data.rgbq.data = SCE_GS_SET_RGBAQ(0x20,0x80,0x90,0x80,0x3F800000);
#ifdef TEST
		//ブラーモード
		packet[i].mdl_draw.data.tex0.data = SCE_GS_SET_TEX0( BUFFER_PAGE(1-i)/64,
															 BUFFER_WIDTH/64,
															 FRAME_BUFFER_COLOR_MODE(),
															 10,10,0,0,0,0,0,0,0);
#endif
	}
	InitMVerts( work );
#else
	{
		work->xbox_packet_size = 4450 + 8 ;
		if ( !(work->packet_mem = GV_Malloc( work->xbox_packet_size )) ) {
			printf("ERR!! MALLOC scr_water_demo.c!!\n");
			return -1 ;
		}
		dmapack->autopacket = work->packet_mem ;
		DG_SetDmapackEnd( dmapack->autopacket ) ;
		
		//printf( "scr_water_demo.c need Size%d\n",  (int)prim-(int)work->packet_mem  ) ;
	}
#endif

	
	return (0);
}

void *NewScrWater_Demo( float warp, int col, float time_scl )
{
	Work		*work ;

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->time_scl = time_scl;
		if ( GetResources( work, warp, col ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}
	
	return (void *)work ;
}
