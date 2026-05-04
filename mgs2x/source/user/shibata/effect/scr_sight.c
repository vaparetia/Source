//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scr_mdl_morph.c
	歪みモデルを使った奴(歪み変化)

	2001/05/28 T.Shibata
	
	$Id: scr_sight.c,v 1.1.1.3 2002/11/19 11:48:41 Yoshizawa1 Exp $

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
#define MBODY_CODE 	(GV_StrCode("psg_lenz_bd"))
#define MMOR0_CODE 	(GV_StrCode("psg_lenz_fr"))

#define MDL_PRIM	(SCE_GS_SET_PRIM(4,0,1,0,0,0,1,0,0))
#define MDL_TEST	(SCE_GS_SET_TEST( 0, 7, 0, 0, 0, 0, 1, 1 ))
#define MDL_ALPHA	(SCE_GS_SET_ALPHA(2,2,2,0,0x80))

#define WORPCOFF 	(20000.0f)

typedef struct {
	float from_z,dz;
	float x,y;
} MORPH_DATA;


typedef struct {
	DG_DMATAG		dmatag_tr;				// ＧＩＦ接続ＤＭＡタグ
	TR_BUFFER		tr_buffer;				//								退避(フレームをバックに)
	DG_DMATAG		dmatag_mdl;				// ＧＩＦ接続ＤＭＡタグ
	MDL_DRAW		mdl_draw;				// 								モデル描画の設定
	DG_DMATAG		dmatag_mverts_body;		// ＧＩＦ接続ＤＭＡタグ			モデル頂点(固定)
	DG_DMATAG		dmatag_mverts_morph;	// ＧＩＦ接続ＤＭＡタグ			モデル頂点(変化)
	DG_DMATAG		dmatag_test;			// ＧＩＦ接続ＤＭＡタグ
	PACKET_END		end_paket;				//								ピクセルテスト復元
	DG_DMATAG		dmatag_offset;			// オフセット環境復元用
	DG_DMATAG		dmatag_end;				// RETタグ
}CRACK_PACK;

typedef	struct	{
	GV_ACT_EX			actor ;
//	int				name ;
//	int				map ;
	
	int				camera_num ;
	int				mode;
	int 			n_mverts_body;
	int 			n_mverts_morph;

	int				m_size_body;
	int				m_size_morph;	
	DG_DEF			*def;
	DG_DEF			*def_k;
	
	DG_DEF			*def_l;
	CRACK_PACK		*crack_pack[2];
	void			*mverts_body;
	
	void			*mverts_morph[2];
	DG_DMAPACK		*dmapack;
	float			*angle;
	
	MORPH_DATA		*morph_data;
	

	float			before_angle;

#ifndef PSX2
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
	packet->dmatag_mverts_body.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, work->m_size_body/sizeof(u_long128)) ;
	packet->dmatag_mverts_body.addr = work->mverts_body;
	packet->dmatag_mverts_body.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_mverts_body.vifcode[1] = SCE_VIF1_SET_DIRECT( work->m_size_body/sizeof(u_long128), 0) ;

	// ＤＭＡタグ		モデル頂点(変化)
	packet->dmatag_mverts_morph.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, work->m_size_morph/sizeof(u_long128)) ;
	packet->dmatag_mverts_morph.addr = work->mverts_morph[which];
	packet->dmatag_mverts_morph.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_mverts_morph.vifcode[1] = SCE_VIF1_SET_DIRECT( work->m_size_morph/sizeof(u_long128), 0) ;

}

#ifdef PSX2
static void InitMVerts( void *verts, DG_DEF *def, int n_verts )
{
	DG_MDL 		*mdl = def->models;
	DG_OBJPACK 	*objpack;
	SVECTOR 	*sverts;
	SVECTOR 	*snorms;
	DG_GIFTAG	*giftag = (DG_GIFTAG*)verts;
	MVERT_DATA	*mverts = (MVERT_DATA*)(giftag+1);
	int			i,j,k,x,y,u,v,cnt=0;
	short		svx,svy;//,snx,sny;
	float		temp;
	
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

				svx = (k&1)?-sverts->vx:sverts->vx;
				svy = (k&2)?-sverts->vy:sverts->vy;
			//	snx = (k&1)?-snorms->vx:snorms->vx;
			//	sny = snorms->vy;//(k&2)?-snorms->vy:snorms->vy;
				
				x = ((2048 + svx)<<4);
				y = ((2048 + svy)<<4);

				temp = powf(2.0f, ((float)sverts->vz)/WORPCOFF);

				u = DG_FTOI(((float)(DRAW_WIDTH>>1) + (float)svx * temp )*SHIBATA_FSHIFT);
				v = DG_FTOI(((float)(DRAW_HEIGHT>>1) + (float)svy * temp )*SHIBATA_FSHIFT);
				if(u>MAX_U) u = MAX_U;
				if(v>MAX_V) v = MAX_V;
				if(u<MIN_UV) u = MIN_UV;
				if(v<MIN_UV) v = MIN_UV;

				mverts->rgbq.data = SCE_GS_SET_RGBAQ(0x80,0x80,0x80,0x80,0);
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
#else
static void *InitMVerts( void *packet, DG_DEF *def, int n_verts )
{
	DG_MDL 		*mdl = def->models;
	DG_OBJPACK 	*objpack;
	SVECTOR 	*sverts;
	SVECTOR 	*snorms;
	int			i,j,k,x,y,cnt=0;
	float       u, v ;
	float     	svx,svy;//,snx,sny;
	float		temp;

	int 	n_indices;
	short 	*index;
	DG_VERTEX_KMSS *vbuff;

	vbuff = mdl->vbuff;
	
	for( k = 0; k < BUNKATSU; k++ ){
		objpack = mdl->packs;
		for( i = 0; i < mdl->n_packs; i++ ){
			int dm = 0;

			index   = objpack->index;
			n_indices = objpack->n_indices;
			printf("m_verts [%d]\n",n_indices);
			packet  = DG_SetDmapackTriangleStrip( packet, n_indices );

			for( j = 0; j < n_indices; j++ ){
				sverts = &vbuff[*index].vx;
				snorms = &vbuff[*index].nx;

				svx = k&1 ? -sverts->vx : sverts->vx ;
				svy = k&2 ? -sverts->vy : sverts->vy ;

				x = (DRAW_WIDTH /2) + svx ;
				y = (DRAW_HEIGHT/2) + svy ;

				temp = powf(2.0f, ((float)sverts->vz)/WORPCOFF) ;

				u = DG_FRAME_U( (float)(DRAW_WIDTH /2) + svx * temp ) ;
				v = DG_FRAME_V( (float)(DRAW_HEIGHT/2) + svy * temp ) ;

				packet = DG_SetDmapackVertex( packet, x,y, u,v, SCE_GS_SET_RGBAQ(0x80,0x80,0x80,0x80,0) ) ;
			
				index ++;
				cnt++;
			}
			objpack++;
		}
	}

	return packet ;
}
#endif



#ifdef PSX2
static void InitMorphData( MORPH_DATA *w_morph_data, DG_DEF *from )
{
	DG_MDL 		*from_mdl = from->models;
	DG_OBJPACK 	*from_objpack = from_mdl->packs;
	SVECTOR 	*fverts;
	MORPH_DATA 	*morph_data = MEM_SCR_MORPH;
	int 		i,j,cnt = 0;
	
	for( i = 0; i < from_mdl->n_packs; i++ ){
		fverts = (SVECTOR*)from_objpack->verts;
		for( j = 0; j < from_objpack->n_verts; j++ ){
#if 0
			printf("[%d][%d]fx %d: fx %d: fz %d:\n", i, j, fverts->vx, fverts->vy, fverts->vz );
#endif
			morph_data->from_z = -119.0f;//(float)fverts->vz;
			morph_data->dz = (float)fverts->vz - morph_data->from_z;//(float)(tverts->vz - fverts->vz)/(62.0f);
			morph_data->x = (float)fverts->vx;
			morph_data->y = (float)fverts->vy;
			
			morph_data++;
			fverts++;
			cnt++;
		}
		from_objpack++;
	}
	//printf(" aaaa %d\n",cnt);
	TS_Scr_Mem( w_morph_data, MEM_SCR_MORPH, sizeof(MORPH_DATA), cnt );
}
#else //XBOX
static void InitMorphData( MORPH_DATA *w_morph_data, DG_DEF *from )
{
	DG_MDL 		*from_mdl = from->models;
	DG_OBJPACK 	*from_objpack = from_mdl->packs;
	SVECTOR 	*fverts;
	MORPH_DATA 	*morph_data = MEM_SCR_MORPH;
	int 		i,j,cnt = 0;
	int 		n_indices;
	DG_VERTEX_KMSS *vbuff;
	short 		*index;
	
	vbuff = from_mdl->vbuff;
	
	for( i = 0; i < from_mdl->n_packs; i++ ){
		//fverts = (SVECTOR*)from_objpack->verts;
		n_indices = from_objpack->n_indices;
		index = from_objpack->index;
		for( j = 0; j < n_indices; j++ ){			
			fverts = &vbuff[*index].vx;

#if 0
			printf("[%d][%d]fx %d: fx %d: fz %d:\n", i, j, fverts->vx, fverts->vy, fverts->vz );
#endif
			morph_data->from_z = -119.0f;//(float)fverts->vz;
			morph_data->dz = (float)fverts->vz - morph_data->from_z;//(float)(tverts->vz - fverts->vz)/(62.0f);
			morph_data->x = (float)fverts->vx;
			morph_data->y = (float)fverts->vy;
			
			morph_data++;
			cnt++;
		}
		from_objpack++;
	}
	//printf(" aaaa %d\n",cnt);
	TS_Scr_Mem( w_morph_data, MEM_SCR_MORPH, sizeof(MORPH_DATA), cnt );
}
#endif


#ifdef PSX2
static void SetMVertsMorph( Work *work ,u_char *data, float angle )
{
	MORPH_DATA	*morph_data;
	MVERT_DATA	*mverts = (MVERT_DATA*)(data + sizeof(DG_GIFTAG));
	float		z,temp;
	int 		i,j,u,v;
	float		x,y,k = 52.0f - (angle - 8.0f);

//	printf("n_verts %d\n",work->n_mverts_morph);
	TS_Mem_Scr( MEM_SCR_MORPH, work->morph_data, sizeof(MORPH_DATA), work->n_mverts_morph );

	for( i = 0; i < BUNKATSU; i++ ){
		morph_data = MEM_SCR_MORPH;
		for( j = 0; j < work->n_mverts_morph; j ++ ){
			x = (i&1)?-morph_data->x:morph_data->x;
			y = (i&2)?-morph_data->y:morph_data->y;
//			x /= 4.0f;
//			y *= 2.0f;
			z = morph_data->from_z + morph_data->dz * k;
			temp = powf(2.0f, (z)/WORPCOFF);

			u = DG_FTOI(((float)(DRAW_WIDTH>>1) + x * (temp))*SHIBATA_FSHIFT);
			v = DG_FTOI(((float)(DRAW_HEIGHT>>1) + y * (temp))*SHIBATA_FSHIFT);

			if(u>MAX_U) u = MAX_U;
			else if(u<MIN_UV) u = MIN_UV;
			if(v>MAX_V) v = MAX_V;
			else if(v<MIN_UV) v = MIN_UV;

//			if( j > work->n_mverts_morph-3 ){
//				printf("%d] u %d: v %d\n",j,u,v);
//			}
			mverts->uv.data = SCE_GS_SET_UV(u,v);
			//if( j >= work->n_mverts_morph-1-2 )
			//mverts->rgbq.data = SCE_GS_SET_RGBAQ(0x00,0x00,0x00,0x80,0);

			mverts++;
			morph_data++;
		}
	}
}
#endif


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
/* ここはPS2のみ　変化がほとんど無いので処理を省きます。すみませんmm(..)mm　T.Morita */
#ifdef PSX2
	if(work->before_angle != *work->angle){
		work->before_angle = *work->angle;

		SetMVertsMorph( work, work->mverts_morph[DG_Clock], *work->angle );
		work->mode = 1;
	}else if(work->mode == 1){
		SetMVertsMorph( work, work->mverts_morph[DG_Clock], work->before_angle );
		work->mode = 0;
	}
#endif
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
#ifdef PSX2
	if(work->crack_pack[0]) GV_DelayedFree(work->crack_pack[0]);
#else
	if(work->packet_mem) GV_DelayedFree( work->packet_mem );
#endif
	if(work->mverts_body) GV_DelayedFree(work->mverts_body);
	if(work->mverts_morph[0]) GV_DelayedFree(work->mverts_morph[0]);
	if(work->morph_data) GV_DelayedFree(work->morph_data);
	g_work = NULL;
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int mode, int camera_num )
{
	DG_DMAPACK		*dmapack;
	u_char			*verts;
	DG_DEF			*def;
	GM_CameraSet	*cam;
#ifdef PSX2	
	CRACK_PACK		*packet;
	int				i;
#else
	void            *prim ;
#endif


	work->mode = mode;
	work->camera_num = camera_num;

	cam = GM_GetNextCamera( 0 );//GM_GetCurrentCamera( 0 );
	//GM_FindCameraSet( GV_StrCode( "ズームカメラ" ), 0);
	if(cam == NULL){
		printf("ERR!! NO SCOPE CAMERA!!\n");
		return -1;
	}
	work->angle = &cam->angle;
	work->before_angle = cam->angle;
	
	def = work->def = (DG_DEF*)GV_GetCache( GV_CacheID( MBODY_CODE, 'k' ));
	if(def==NULL){
		printf("ERR!! NO MODEL!!\n");
		return -1;
	}
	def = work->def_k = (DG_DEF*)GV_GetCache( GV_CacheID( MMOR0_CODE, 'k' ));
	if(def==NULL){
		printf("ERR!! NO MODEL!!\n");
		return -1;
	}
	
	work->n_mverts_body = GetMdlVerts(work->def);		//モデルの頂点数
	work->n_mverts_morph = GetMdlVerts(work->def_k);	//モデルの頂点数
		
	work->m_size_body  = sizeof(DG_GIFTAG)+sizeof(MVERT_DATA)*work->n_mverts_body*BUNKATSU;
	work->m_size_morph = sizeof(DG_GIFTAG)+sizeof(MVERT_DATA)*work->n_mverts_morph*BUNKATSU;

	// 頂点データメモり確保
	verts = work->mverts_body = GV_Malloc( work->m_size_body );
	if(verts == NULL){
		printf("ERR!! MALLOC MVERTS scr_sight.c!!\n");
		return -1;
	}
	
	// 頂点データメモり確保
	verts = GV_Malloc( work->m_size_morph * 2 );
	if(verts == NULL){
		printf("ERR!! MALLOC MVERTS scr_sight.c!!\n");
		return -1;
	}
	work->mverts_morph[0] = (MVERT_DATA*)(verts);
	work->mverts_morph[1] = (MVERT_DATA*)(verts+work->m_size_morph);

	work->morph_data = GV_Malloc( sizeof(MORPH_DATA) * work->n_mverts_morph );
	if(work->morph_data == NULL){
		printf("ERR!! MALLOC MORPH DATA scr_sight.c!!\n");
		return -1;
	}

	// ＤＭＡパケット型オブジェクト作成
	work->dmapack= dmapack = DG_MakeDmapack( DG_DMAPACK_NORMAL|DG_DMAPACK_INVISIBLE1|DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3,
											 DG_DMAPACK_PHASE_AFTER );
	if(dmapack==NULL){
		printf("ERR!! MAKE DMAPACK scr_sight.c!!\n");
		return -1;
	}
	DG_QueueDmapack( dmapack );

	/* モー不データを初期化する */
	InitMorphData( work->morph_data, work->def_k );

#ifdef PSX2
	// パケットメモリ割り当て
	packet = GV_Malloc( sizeof(CRACK_PACK) * 2 );
	if(packet == NULL){
		printf("ERR!! MALLOC scr_sight.c!!\n");
		return -1;
	}
	work->crack_pack[0] = dmapack->packet[0] = &packet[0] ;
	work->crack_pack[1] = dmapack->packet[1] = &packet[1] ;
	
	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaTags( work, &packet[i], i );
		InitTrBuffer( &packet[i].tr_buffer, i );
		InitMdlDraw( &packet[i].mdl_draw, MDL_TEST, MDL_ALPHA, MDL_PRIM );
		InitMVerts( work->mverts_morph[i], work->def_k, work->n_mverts_morph );
		SetMVertsMorph( work, work->mverts_morph[i], *work->angle );
		InitEndPacket( &packet[i].end_paket );
	}
	
	InitMVerts( work->mverts_body, work->def, work->n_mverts_body );
#else
	{
		int xbox_packet_size = 10900;/* 下の＊＊＊のprintf文のサイズ分が必要 */
		if ( !(work->packet_mem = GV_Malloc( xbox_packet_size )) ) {
			printf("ERR!! MALLOC scr_sight.c!!\n");
			return -1 ;
		}
		dmapack->autopacket = work->packet_mem ;
		prim = dmapack->autopacket ;
		prim = InitTrBuffer( prim, 0 ) ;
		prim = InitMdlDraw( prim, MDL_TEST, MDL_ALPHA, MDL_PRIM ) ;
		prim = InitMVerts( prim, work->def, work->n_mverts_body );
		DG_SetDmapackEnd( prim ) ;
   
		printf( "scr_sight.c need Size%d\n",  (int)prim-(int)work->packet_mem  ) ;/* ＊＊＊ */
		if( xbox_packet_size < ((int)prim-(int)work->packet_mem) ){
			printf("xbox_packet_size not enough!!\n");
			ASSERT( 0 );
		}
	}
#endif

	
	return (0);
}

/*
mode: 1(非表示) 0（起動,表示） -1（終了）
二重起動はしません
*/
void *NewScrSightMorph( int mode, int camera_num )
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
