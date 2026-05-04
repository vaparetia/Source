//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scr_crack.c
	カメラ割れ モデルを使った奴
	(c_crack_mdl.c 整理版)
	2000/07/05 T.Shibata
	
	$Id: scr_crack.c,v 1.1.1.3 2002/11/19 11:48:40 Yoshizawa1 Exp $

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

#define CLOCK_COUNT	(BP_BASE_TICK())

#define MODEL_CODE 	(GV_StrCode("brk_scr07_full"))

//#define TRI_CODE 	(GV_StrCode("c_crack"))
//#define TEX_CODE 	(GV_StrCode("glass_brk1_alp"))



//#define TRI_CODE	(GV_StrCode("c_crack_sub"))
//#define TEX_CODE  	(GV_StrCode("brake_grass3_sub"))
#define TRI_CODE	(GV_StrCode("c_crack_add"))
#define TEX_CODE 	(GV_StrCode("brake_grass3bw_add"))
//#define TRI_CODE	(GV_StrCode("c_crackg_sub"))
//#define TEX_CODE 	(GV_StrCode("brake_grass3g_sub"))

//------------------------------------------------
//テクスチャーなし
#define NO_TEX
//------------------------------------------------

#define TRI_CODE_SUB 	TRI_CODE
#define TEX_CODE_SUB 	TEX_CODE

#define TRI_CODE_ADD 	TRI_CODE
#define TEX_CODE_ADD 	TEX_CODE

#define SPR_PRIM_SUB	(SCE_GS_SET_PRIM(6,0,1,0,1,0,0,0,0))
#define SPR_RGBA_SUB	(SCE_GS_SET_RGBAQ(0x80,0x80,0x80,0x80,0x3F800000))
#define SPR_ALPHA_SUB	(SCE_GS_SET_ALPHA(2,0,2,1,0x20))

#define SPR_PRIM_ADD	(SCE_GS_SET_PRIM(6,0,1,0,1,0,0,0,0))
#define SPR_RGBA_ADD	(SCE_GS_SET_RGBAQ(0x80,0x80,0x80,0x80,0x3F800000))
#define SPR_ALPHA_ADD	(SCE_GS_SET_ALPHA(0,2,2,1,0x40))

#define MDL_PRIM	(SCE_GS_SET_PRIM(4,1,1,0,0,0,1,0,0))
#define MDL_TEST	(SCE_GS_SET_TEST( 0, 7, 0, 0, 0, 0, 1, 1 ))
#define MDL_ALPHA	(SCE_GS_SET_ALPHA(2,2,2,0,0x80))

#define PIX_SHIFT_X	(2.0f)
#define PIX_SHIFT_Y	(1.0f)
#define INIT_MIN_S		(PIX_SHIFT_X/512.0f)
#define INIT_MIN_T		(PIX_SHIFT_Y/(float)(DRAW_HEIGHT/2))
#define INIT_MAX_S		(1.0f - INIT_MIN_S)
#define INIT_MAX_T		(1.0f - INIT_MIN_T)

#define ADD_PIX_SHIFT_X	(INIT_MIN_S/10.0f)
#define ADD_PIX_SHIFT_Y	(INIT_MIN_T/10.0f)

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
}CRACK_PACK;

typedef	struct	{
	GV_ACT_EX			actor ;
//	int				name ;
//	int				map ;
	
	int				camera_num ;
	int				mode;
	int 			n_mverts;
	int				m_size;
	DG_DEF			*def;
	CRACK_PACK		*crack_pack[2];
	MVERT_DATA		*mverts;
	SVERT_DATA		*sverts_sub[2];
	SVERT_DATA		*sverts_add[2];
	DG_DMAPACK		*dmapack;

	void				*packet_mem;

} Work ;

static Work *g_work = NULL;

#if 0
static void *InitMVerts( Work *work, void *packet )
{
	DG_DEF *def = work->def;
	DG_MDL *mdl = def->models;
	DG_OBJPACK *objpack = mdl->packs;
	SVECTOR *sverts;
	SVECTOR *snorms;
	int	i,j,cnt=0;
	float   x, y ;
	float   u, v, du,dv ;
	int 	n_verts;
	short 	*index;
	DG_VERTEX_KMSS *vbuff;

	vbuff = mdl->vbuff;

	for( i = 0; i < mdl->n_packs; i++ ){
		int dm = 0;


		index   = objpack->index;
		n_verts = objpack->n_indices;
		cnt = 0;

		packet  = DG_SetDmapackTriangleStrip( packet, 500 );
		for( j = 0; j < n_verts; j++ ){
			sverts = &vbuff[*index].vx;
			snorms = &vbuff[*index].nx;

			if( j == 500 ){
				int tm;
				tm = 998 - 500;
				printf("tm2 =%d\n",tm);
				packet  = DG_SetDmapackTriangleStrip( packet, tm );
			} else if( j == 998 ){
				int tm;
				tm = n_verts - 998;
				printf("tm3 =%d\n",tm);
				packet  = DG_SetDmapackTriangleStrip( packet, tm );
			}				
			  
			x = (((20480 + sverts->vx))/10 - 2048)/512.0f*DRAW_WIDTH + DRAW_WIDTH/2 ;
			y = sverts->vy;
#ifdef PAL     // BP JG - unused as it's #if 0'd out
			y = y * 512.0f / 448.0f ;
#endif
			//同じ比率にしないと格好悪いので　Xと同じ倍率で引き伸ばす->( /512.0f*DRAW_WIDTH )
			y = (((20480 - y * 2     ))/10 - 2048)/512.0f*DRAW_WIDTH + DRAW_HEIGHT/2 ;


			//du = (128.0f * 2.0f) * (float)snorms->vx/4096.0f;// * ((float)verts->vz/500.0f+1.0f);
			//dv = (128.0f * 2.0f) * (float)snorms->vy/4096.0f;// * ((float)verts->vz/500.0f+1.0f);
			du = (128.0f * 2.0f) * (float)snorms->vx/32767.0f;// * ((float)verts->vz/500.0f+1.0f);
			dv = (128.0f * 2.0f) * (float)snorms->vy/32767.0f;// * ((float)verts->vz/500.0f+1.0f);
			u = DG_FRAME_U((256.0f + (float)sverts->vx/10.0f)*SHIBATA_FSHIFT+du/16.0f);
			v = DG_FRAME_V(((float)(DRAW_HEIGHT/2) + (float)(-sverts->vy<<1)/10.0f)*SHIBATA_FSHIFT+dv/16.0f);
			packet = DG_SetDmapackVertex( packet, x,y, u,v, DG_MakeDmaPackColorFromInt(0x80808080) ) ;
			
			index ++;
			cnt ++;
		}
		objpack++;
	}
	return packet ;
}
#else

static void *InitMVerts( Work *work, void *packet )
{
   DG_DEF *def = work->def;
   DG_MDL 		*mdl = def->models;
   DG_OBJPACK 	*objpack;
   SVECTOR 	*sverts;
   SVECTOR 	*snorms;
   int			i,j,k,x,y;
   float       u, v, du, dv ;
   DG_DMAPACK_COLOR rgbq = DG_MakeDmaPackColorFromInt(0x80808080);

   DG_DMAPACK_VERTEX* pLastVert = 0;

   objpack = mdl->packs;
   
   for( i = 0; i < mdl->n_packs; i++ )
   {
      DG_DMAPACK_TRIANGLESTRIP* pTriStripPacket = (DG_DMAPACK_TRIANGLESTRIP*)packet;
      int lastVertKickFlag = 1;

      packet  = DG_SetDmapackTriangleStrip( packet, 0 );

      sverts = (SVECTOR*)objpack->verts;
      snorms = (SVECTOR*)objpack->norms;
      for( j = 0; j < objpack->n_verts; j++ ){
         int const currentKickFlag = (snorms->pad & 0x8000);

         x = sverts->vx / 10 + DRAW_WIDTH / 2;
         y = -sverts->vy / 5 + DRAW_HEIGHT / 2;

         du = (128.0f * 2.0f) * (float)snorms->vx / 4096.0f;
         dv = (128.0f * 2.0f) * (float)snorms->vy / 4096.0f;
         
         u = DG_FRAME_U(x + du / 16.0f);
         v = DG_FRAME_V(y + dv / 16.0f);

         if(u>1.0f) u = 1.0f;
         if(v>1.0f) v = 1.0f;
         if(u<0.0f) u = 0.0f;
         if(v<0.0f) v = 0.0f;

         if( currentKickFlag )
         {
            if( !lastVertKickFlag )
            {
               // duplicate last vertex
               DG_DMAPACK_VERTEX* skipVert = packet;
               memcpy(skipVert, pLastVert, sizeof(DG_DMAPACK_VERTEX));
               packet = (void*)(skipVert + 1);

               // create duplicate of next vertex
               packet = DG_SetDmapackVertex( packet, x, y, u, v, rgbq ) ;

               // advance count by two
               pTriStripPacket->v_count += 2;
            }
         }

         lastVertKickFlag = currentKickFlag;

         pLastVert = packet;
         packet = DG_SetDmapackVertex( packet, x,y, u,v, rgbq ) ;
         ++pTriStripPacket->v_count;

         ++sverts;
         ++snorms;
      }
      objpack++;
   }
   
   return packet ;
}

#endif



/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
	if(work->packet_mem) GV_DelayedFree( work->packet_mem );
	if(work->mverts) GV_DelayedFree(work->mverts);

	g_work = NULL;
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int mode, int camera_num )
{
	DG_DMAPACK		*dmapack;
	u_char			*verts;
	DG_DEF			*def;
	int				i;
	void            *prim ;

	work->mode = mode;
	work->camera_num = camera_num;
	
	def = work->def = (DG_DEF*)GV_GetCache( GV_CacheID( MODEL_CODE, 'k' ));
	if(def==NULL){
		printf("ERR!! NO MODEL!!\n");
		return -1;
	}
	work->n_mverts = GetMdlVerts(def);	//モデルの頂点数
	work->m_size = sizeof(DG_GIFTAG)+sizeof(MVERT_DATA)*work->n_mverts;
	// 頂点データメモり確保
	verts = GV_Malloc( work->m_size );
	if(verts == NULL){
		printf("ERR!! MALLOC MVERTS!!\n");
		return -1;
	}
	work->mverts = (MVERT_DATA*)(verts);
//	work->mverts[1] = (MVERT_DATA*)(verts+work->m_size);

	// ＤＭＡパケット型オブジェクト作成
	work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL,DG_DMAPACK_PHASE_AFTER,128 );
	if(dmapack==NULL){
		printf("ERR!! MAKE DMAPACK!!\n");
		return -1;
	}
	DG_QueueDmapack( dmapack );

	{
		int xbox_packet_size = 22000;/* 下の＊＊＊のprintf文のサイズ分が必要 */
		if ( !(work->packet_mem = GV_Malloc( xbox_packet_size )) ) { 
			return -1 ;
		}
		dmapack->autopacket = work->packet_mem ;
		prim = dmapack->autopacket ;
		prim = InitTrBuffer( prim, 0 ) ;
		prim = InitMdlDraw( prim, MDL_TEST, MDL_ALPHA, MDL_PRIM ) ;
		prim = InitMVerts( work, prim ) ;
		DG_SetDmapackEnd( prim ) ;
		
		printf( "scr_crack.c need Size%d\n",  (int)prim-(int)work->packet_mem  ) ; /* ＊＊＊ */
		if( xbox_packet_size < ((int)prim-(int)work->packet_mem) ){
			printf("xbox_packet_size not enough!!\n");
			ASSERT( 0 );
		}
	}

	return (0);
}
//
// int mode: -1)在ったら殺す
// int camera_num: 未対応
void *NewScrCrack( int mode, int camera_num )
{
	Work		*work ;

	if(mode < 0){
		if(g_work) GV_DestroyActor( g_work ) ;
		g_work = NULL;
		return NULL;
	}

	if(g_work) return NULL;
		
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, mode, camera_num ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		g_work = work;
	}

	return (void *)work ;
}
