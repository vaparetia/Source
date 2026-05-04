//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scr_mdl_morph.c
	歪みモデルを使った奴(歪み変化)

	2001/05/28 T.Shibata
	
	$Id: scr_goggles_demo.c,v 1.1.1.3 2002/11/19 11:48:34 Yoshizawa1 Exp $

*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if 0 //BP_PS2 def PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include "libutl.h"
#endif

#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"camera.h"
#include	"def_dma.h"
#include	"../util/ts_util.h"
#include	"../util/dma_set.h"


#define CLOCK_COUNT	(BP_BASE_TICK())

#define XY_OFF			(2048.0f)
#define XY_SCL			(2048.0f)
#define DWIDTH_OFF		((float)(DRAW_WIDTH>>1))
#define HEIGHT_OFF		((float)(DRAW_HEIGHT>>1))
#define DWIDTH_SCL		((float)(DRAW_WIDTH>>1))
#define HEIGHT_SCL		((float)(DRAW_HEIGHT>>1))

#define BUNKATSU		(2)

//#define MODEL_CODE 	(GV_StrCode("brk_scr07_full"))
#define MBODY_CODE 	(GV_StrCode("raiden_gogle_lenz"))

#define MDL_PRIM	(SCE_GS_SET_PRIM(4,1,1,0,1,0,1,0,0))
#define MDL_TEST	(SCE_GS_SET_TEST( 0, 7, 0, 0, 0, 0, 1, 1 ))
#define MDL_ALPHA	(SCE_GS_SET_ALPHA(0,1,0,1,0x80))

#define WORPCOFF 	(20000.0f)

typedef struct {
	float from_z,dz;
	float x,y;
} MORPH_DATA;


typedef struct {
	DG_DMATAG		dmatag_tr;				// ＧＩＦ接続ＤＭＡタグ
	TR_BUFFER		tr_buffer;				//								退避(フレームをバックに)
	DG_DMATAG		dmatag_frc;				// ＧＩＦ接続ＤＭＡタグ
	FRAME_CLEAR		fr_clear;				//								画面クリアー
	DG_DMATAG		dmatag_mdl;				// ＧＩＦ接続ＤＭＡタグ
	MDL_DRAW		mdl_draw;				// 								モデル描画の設定
	DG_DMATAG		dmatag_mverts;			// ＧＩＦ接続ＤＭＡタグ			モデル頂点(固定)
	DG_DMATAG		dmatag_test;			// ＧＩＦ接続ＤＭＡタグ
	PACKET_END		end_paket;				//								ピクセルテスト復元
	DG_DMATAG		dmatag_offset;			// オフセット環境復元用
	DG_DMATAG		dmatag_end;				// RETタグ
} CRACK_PACK;

typedef	struct	{
	GV_ACT_EX			actor ;
//	int					name;
//	int					map;
	
	int				camera_num ;
	int				mode;
	int 			n_mverts;

	int				m_size;
	
	CRACK_PACK		*crack_pack[2];
	void			*mverts;
	
	DG_DMAPACK		*dmapack;

#if 1 //BP_XBOX ndef PSX2
	void				*packet_mem;
#endif
	
} Work ;

#define		MEM_SCR_MORPH		((void *)(SCRPAD_ADDR))


static void InitDmaTags( Work *work, CRACK_PACK *packet, int which )
{
	// --------  DMATag  ----------------------------
	// ＤＭＡタグ		退避
	packet->dmatag_tr.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(TR_BUFFER) ) ;
	packet->dmatag_tr.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_tr.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(TR_BUFFER), 0) ;
	// 画面クリア
	packet->dmatag_frc.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(FRAME_CLEAR) ) ;
	packet->dmatag_frc.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_frc.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(FRAME_CLEAR), 0) ;
	
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
	
	// ＤＭＡタグ		モデル頂点(固定)
	packet->dmatag_mverts.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, work->m_size/sizeof(u_long128)) ;
	packet->dmatag_mverts.addr = work->mverts;
	packet->dmatag_mverts.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_mverts.vifcode[1] = SCE_VIF1_SET_DIRECT( work->m_size/sizeof(u_long128), 0) ;
}

#if 0 //BP_PS2 def PSX2
static void InitMVerts( void *verts, DG_DEF *def, int n_verts, short shift_x, short shift_y )
{
	DG_MDL 		*mdl = def->models;
	DG_OBJPACK 	*objpack;
	SVECTOR 	*sverts;
	SVECTOR 	*snorms;
	DG_GIFTAG	*giftag = (DG_GIFTAG*)verts;
	MVERT_DATA	*mverts = (MVERT_DATA*)(giftag+1);
	int			i,j,k,x,y,u,v,cnt=0;
	short		svx,svy,snx,sny;
//	float		temp;
	
	giftag->tag  = SCE_GIF_SET_TAG(SIZEOF_QWORD(MVERT_DATA)*n_verts*BUNKATSU, 1, 0, 0, 0, 1);
	giftag->regs = GS_REGS_AD;

	for( k = 0; k < BUNKATSU; k++ ){
		objpack = mdl->packs;
		for( i = 0; i < mdl->n_packs; i++ ){
			sverts = (SVECTOR*)objpack->verts;
			snorms = (SVECTOR*)objpack->norms;
			for( j = 0; j < objpack->n_verts; j++ ){
				mverts->rgbq.reg = SCE_GS_RGBAQ;
				mverts->uv.reg = SCE_GS_UV;
				mverts->xyz.reg = (snorms->pad&0x8000)?SCE_GS_XYZ3:SCE_GS_XYZ2;

				svx = (short)((float)((k&1)?-sverts->vx:sverts->vx) * 1.5f ) + shift_x;
				svy = (short)((float)(-((k&2)?-sverts->vy:sverts->vy) * 1.5f)) + shift_y;
				snx = (short)((float)((k&1)?-snorms->vx:snorms->vx) * 1.5f);
				sny = (short)((float)(-((k&2)?-snorms->vy:snorms->vy)) * 1.5f);
				
				x = ((2048 + svx)<<4);
				y = ((2048 + svy)<<4);

//				u = DG_FTOI( ( (float)(DRAW_WIDTH>>1)  + ( (float)snx/4096,0f ) * 16.0f + (float)svx ) * SHIBATA_FSHIFT );
				u = DG_FTOI( ( (float)(DRAW_WIDTH>>1)  + ( (float)snx/4096.0f ) * 16.0f + (float)svx ) * SHIBATA_FSHIFT );
				v = DG_FTOI( ( (float)(DRAW_HEIGHT>>1) + ( (float)sny/4096.0f ) * 16.0f + (float)svy ) * SHIBATA_FSHIFT );

				if(u>MAX_U) u = MAX_U;
				if(v>MAX_V) v = MAX_V;
				if(u<MIN_UV) u = MIN_UV;
				if(v<MIN_UV) v = MIN_UV;
				if( sverts->vz > 0 )
					mverts->rgbq.data = SCE_GS_SET_RGBAQ(0xff,0x80,0x00,0x80,0);
				else
					mverts->rgbq.data = SCE_GS_SET_RGBAQ(0x40,0x40,0x40,0x80,0);
				mverts->uv.data = SCE_GS_SET_UV(u,v);
				mverts->xyz.data = SCE_GS_SET_XYZ(x,y,0);
			
				mverts++;
				sverts++;
				snorms++;

				cnt++;
			}
			objpack++;
		}
	}
	//printf("verts = %d\n",cnt);

}
#else //XBOX

#if 0 //BP_XBOX
static void *InitMVerts( void *packet, void *verts, DG_DEF *def, int n_verts )
{
	DG_MDL 		*mdl = def->models;
	DG_OBJPACK 	*objpack;
	SVECTOR 	*sverts;
	SVECTOR 	*snorms;
	int			i,j,k,x,y;
	float       u, v ;
	float		svx,svy,snx,sny;
	int         rgbq ;

	int 	m_verts;
	short 	*index;
	DG_VERTEX_KMSS *vbuff;

	vbuff = mdl->vbuff;

	for( k = 0; k < BUNKATSU; k++ ){
		objpack = mdl->packs;
		for( i = 0; i < mdl->n_packs; i++ ){
			//int dm = 0;

			index   = objpack->index;
			m_verts = objpack->n_indices;
			printf("m_verts [%d]\n",m_verts);
			//packet  = DG_SetDmapackTriangleStrip( packet, m_verts );
			packet  = DG_SetDmapackTriangleStrip( packet, 496 );
			for( j = 0; j < m_verts; j++ ){
				sverts = &vbuff[*index].vx;
				snorms = &vbuff[*index].nx;
				
				if( j == 496 ){/* 最大頂点数を超えたため途中でストリップを分ける */
					int tm;
					tm = m_verts - 496;
					packet  = DG_SetDmapackTriangleStrip( packet, tm );
				}

				svx =  (k&1 ? -sverts->vx : sverts->vx) * 1.5f ;
				svy = -(k&2 ? -sverts->vy : sverts->vy) * 1.5f ;
				snx =  (k&1 ? -snorms->vx : snorms->vx) * 1.5f ;
				sny = -(k&2 ? -snorms->vy : snorms->vy) * 1.5f ;
			    
				x = (DRAW_WIDTH /2) + svx * DRAW_WIDTH  / 512 ;
				y = (DRAW_HEIGHT/2) + svy * DRAW_HEIGHT / 448 ;

				//u = DG_FRAME_U( ( snx/4096.0f ) * 16.0f + x ) ;
				//v = DG_FRAME_V( ( sny/4096.0f ) * 16.0f + y ) ;
				u = DG_FRAME_U( ( snx/32767.0f ) * 16.0f + x ) ;
				v = DG_FRAME_V( ( sny/32767.0f ) * 16.0f + y ) ;

				if( sverts->vz > 0 )
				  rgbq = SCE_GS_SET_RGBAQ(0xff,0x80,0x00,0x80,0);
				else
				  rgbq = SCE_GS_SET_RGBAQ(0x40,0x40,0x40,0x80,0);

				packet = DG_SetDmapackVertex( packet, x,y, u,v, rgbq ) ;
			
				index ++;
			}
			objpack++;
		}
	}

	return packet ;

}
#else
static void *InitMVerts( void *packet, void *verts, DG_DEF *def, int n_verts, short shift_x, short shift_y  )
{
   DG_MDL 		*mdl = def->models;
   DG_OBJPACK 	*objpack;
   SVECTOR 	*sverts;
   SVECTOR 	*snorms;
   int			i,j,k,x,y;
   float       u, v ;
   float		svx,svy,snx,sny;
   DG_DMAPACK_COLOR rgbq ;

   DG_DMAPACK_VERTEX* pLastVert = 0;

   for( k = 0; k < BUNKATSU; k++ ){
      objpack = mdl->packs;
      for( i = 0; i < mdl->n_packs; i++ ){
         DG_DMAPACK_TRIANGLESTRIP* pTriStripPacket = (DG_DMAPACK_TRIANGLESTRIP*)packet;
         int lastVertKickFlag = 1;

         packet  = DG_SetDmapackTriangleStrip( packet, 0 );

         sverts = (SVECTOR*)objpack->verts;
         snorms = (SVECTOR*)objpack->norms;
         for( j = 0; j < objpack->n_verts; j++ ){
            int const currentKickFlag = (snorms->pad & 0x8000);

            svx =  (k&1 ? -sverts->vx : sverts->vx) * 1.5f + shift_x ;
            svy = -(k&2 ? -sverts->vy : sverts->vy) * 1.5f + shift_y ;
            snx =  (k&1 ? -snorms->vx : snorms->vx) * 1.5f ;
            sny = -(k&2 ? -snorms->vy : snorms->vy) * 1.5f ;

            x = (DRAW_WIDTH /2) + svx * DRAW_WIDTH  / 512 ;
            y = (DRAW_HEIGHT/2) + svy * DRAW_HEIGHT / 448 ;

            u = DG_FRAME_U( ( snx/4096.0f ) * 16.0f + x ) ;
            v = DG_FRAME_V( ( sny/4096.0f ) * 16.0f + y ) ;

            if( sverts->vz > 0 )
               rgbq = DG_MakeDMAPackColor(0xff,0x80,0x00,0x80);
            else
               rgbq = DG_MakeDMAPackColor(0x40,0x40,0x40,0x80);

            if( currentKickFlag )
            {
               if( !lastVertKickFlag )
               {
                  // duplicate last vertex
                  DG_DMAPACK_VERTEX* skipVert = packet;
                  memcpy(skipVert, pLastVert, sizeof(DG_DMAPACK_VERTEX));
                  packet = (void*)(skipVert + 1);

                  // create duplicate of next vertex
                  packet = DG_SetDmapackVertex( packet, x,y, u,v, rgbq ) ;

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
   }

   return packet ;
}
#endif

#endif

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	if ( GM_CheckSightStatus( SGT_Invisible ) ) {
		/* このときは強制的に非表示 */
		
		work->dmapack->flag |= DG_DMAPACK_INVISIBLE0;
	}else{
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0;
	}

}
static Work *g_work = NULL;

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
	
#if 0 //BP_PS2 def PSX2
	if(work->crack_pack[0]) GV_DelayedFree(work->crack_pack[0]);
#else
	if(work->packet_mem) GV_DelayedFree( work->packet_mem );
#endif
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
#if 0 //BP_PS2 def PSX2	
	CRACK_PACK		*packet;
#else
	void            *prim ;
#endif

	work->mode = mode;
	work->camera_num = camera_num;
	
	def = (DG_DEF*)GV_GetCache( GV_CacheID( MBODY_CODE, 'k' ));
	if(def==NULL){
		printf("ERR!! NO MODEL!!\n");
		return -1;
	}

	work->n_mverts = GetMdlVerts(def);		//モデルの頂点数
	
	//printf("SCR_BOTTOM = %p\n",MEM_SCR_MORPH + sizeof(MORPH_DATA)*work->n_mverts_morph);
	
	
	work->m_size = sizeof(DG_GIFTAG)+sizeof(MVERT_DATA)*work->n_mverts*BUNKATSU;

	// 頂点データメモり確保
	verts = work->mverts = GV_Malloc( work->m_size );
	if(verts == NULL){
		printf("ERR!! MALLOC MVERTS!!\n");
		return -1;
	}

	// ＤＭＡパケット型オブジェクト作成
	work->dmapack= dmapack = DG_MakeDmapack( DG_DMAPACK_NORMAL|DG_DMAPACK_INVISIBLE1|DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3,
											 DG_DMAPACK_PHASE_AFTER );
	if(dmapack==NULL){
		printf("ERR!! MAKE DMAPACK!!\n");
		return -1;
	}

	DG_QueueDmapack( dmapack );

#if 0 //BP_PS2 def PSX2	

	// パケットメモリ割り当て
	packet = GV_Malloc( sizeof(CRACK_PACK) * 2 );
	if(packet == NULL){
		printf("ERR!! MALLOC CRACK_PACK!!\n");
		return -1;
	}
	work->crack_pack[0] = dmapack->packet[0] = &packet[0] ;
	work->crack_pack[1] = dmapack->packet[1] = &packet[1] ;
	

	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaTags( work, &packet[i], i );
		InitTrBuffer( &packet[i].tr_buffer, i );
		InitMdlDraw( &packet[i].mdl_draw, MDL_TEST, MDL_ALPHA, MDL_PRIM );
		InitEndPacket( &packet[i].end_paket );
		InitFrameClear( &packet[i].fr_clear,
						MDL_TEST, SCE_GS_SET_RGBAQ(0x00,0x00,0x00,0x40,0), MDL_ALPHA, 0 );		//フレームクリアー
	}
	InitMVerts( work->mverts, def, work->n_mverts, 0, -35);
#else /*PSX2*/
	{
		int xbox_packet_size = 31000;/* 下の＊＊＊のprintf文のサイズ分が必要 */
		if ( !(work->packet_mem = GV_Malloc( xbox_packet_size )) ) {
			return -1 ;
		}
		dmapack->autopacket = work->packet_mem ;
		prim = dmapack->autopacket ;
		prim = InitTrBuffer( prim, 0 );
		prim = InitFrameClear( prim,MDL_TEST, SCE_GS_SET_RGBAQ(0x00,0x00,0x00,0x40,0), MDL_ALPHA, 0 );	//フレームクリアー
		prim = InitMdlDraw( prim, MDL_TEST, 0/*MDL_ALPHA*/, MDL_PRIM );
		prim = InitMVerts( prim, work->mverts, def, work->n_mverts, 0, -35 );
		DG_SetDmapackEnd( prim );

		printf( "scr_goggle_demo.c : Size%d\n",  (int)prim-(int)work->packet_mem  ) ;/* ＊＊＊ */
		if( xbox_packet_size < ((int)prim-(int)work->packet_mem) ){
			printf("xbox_packet_size not enough!!\n");
			ASSERT( 0 );
		}
	}
#endif

	
	return (0);
}

/*
mode: 0（起動） -1（終了）
二重起動はしません
*/
void *NewScrGoggles_demo( int mode, int camera_num )
{
	Work		*work ;

	if(mode < 0){
		if(g_work) GV_DestroyActor( g_work ) ;
		g_work = NULL;
		return NULL;
	}else if( mode > 0 ){
		if(g_work)
			g_work->dmapack->flag |= DG_DMAPACK_INVISIBLE0;
		return NULL;
	}

	if(g_work){
		g_work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0;
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
