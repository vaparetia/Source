/*
	shadowvl.c
	シャドーボリューム生成ルーチン

	1999/07/07 K.Takabe
	$Id: shadowvl.c,v 1.1.1.3 2002/11/19 11:42:24 Yoshizawa1 Exp $

*/
/*

*/

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"

typedef struct _scrpad{
	FMATRIX		light_eye ;
	FMATRIX		tmp_mat ;
	FVECTOR		org_light_pos ;
	FVECTOR		light_pos ;
	FVECTOR		tmp_vec ;
	FVECTOR		verts[128] ;
	FVECTOR		norms[128] ;
} ScrpadWork ;

typedef struct {
	DG_OBJENV	objenv ;
	int			max_num ;
	void		*now ;
	int			pad1,pad2 ;
	void		*ot3[64] ;
} ScrpadWork2 ;


/*----------------------------------------------------------------*/
static void WorkSetVector( FVECTOR *verts, FVECTOR *norms, SVECTOR *iverts, SVECTOR *inorms, int n )
{
	asm ("
		move	$10,%0
		move	$11,%1
		move	$12,%2
		move	$13,%3
		move	$14,%4
0:
		ld		$15,0($12)
		ld		$16,0($13)
		ld		$17,8($12)
		ld		$18,8($13)
		pextlh	$15,$15,$0
		pextlh	$16,$16,$0
		pextlh	$17,$17,$0
		pextlh	$18,$18,$0
		psraw	$15,$15,16
		psraw	$16,$16,16
		psraw	$17,$17,16
		psraw	$18,$18,16
		qmtc2	$15,vf16
		qmtc2	$16,vf17
		qmtc2	$17,vf18
		qmtc2	$18,vf19
		vitof0.xyzw		vf16,vf16
		vitof12.xyzw	vf17,vf17
		vitof0.xyzw		vf18,vf18
		vitof12.xyzw	vf19,vf19
		sqc2			vf16,0($10)
		sqc2			vf17,0($11)
		sqc2			vf18,16($10)
		sqc2			vf19,16($11)
		addi			$14,$14,-2
		addi			$10,$10,32
		addi			$11,$11,32
		addi			$12,$12,16
		addi			$13,$13,16
		bgtz			$14,0b
	"::"r"(verts),"r"(norms),"r"(iverts),"r"(inorms),"r"(n):
	"$10","$11","$12","$13","$14","$15","$16","$17","$18");
}



/*----------------------------------------------------------------*/
static void MakeShadowVolumeObjPack( DG_OBJPACK *obj_pack, DG_SUBPACK *sub_packet )
{
	ScrpadWork	*work = (ScrpadWork*)SCRPAD_ADDR ;
	FVECTOR		*fverts, *fnorms ;
	SVECTOR		*sverts, *snorms ;
	float		f, scale ;
	int			i ;

	/* ワーク確保処理 */
	if ( sub_packet->verts == NULL ){
		sub_packet->verts = GV_Malloc( sizeof(SVECTOR) * obj_pack->n_verts );
		sub_packet->packet.verts_tag.addr = sub_packet->verts ;
	}

	/* 頂点、法線を浮動小数点に変換し、ワークへ保存 */
	WorkSetVector( work->verts, work->norms, obj_pack->verts, obj_pack->norms, obj_pack->n_verts );

	fverts = work->verts ;
	fnorms = work->norms ;
	sverts = sub_packet->verts ;
	for ( i = obj_pack->n_verts ; i > 0 ; i-- ){
		work->tmp_vec.vx = fverts->vx - work->light_pos.vx ;
		work->tmp_vec.vy = fverts->vy - work->light_pos.vy ;
		work->tmp_vec.vz = fverts->vz - work->light_pos.vz ;
		f = work->tmp_vec.vx * work->tmp_vec.vx + work->tmp_vec.vy * work->tmp_vec.vy + work->tmp_vec.vz * work->tmp_vec.vz ;
		scale = f = DG_RSQRT( f ) ;
		f *= work->tmp_vec.vx * fnorms->vx + work->tmp_vec.vy * fnorms->vy + work->tmp_vec.vz * fnorms->vz ;
		if ( f < 0.0F ){
			/* 法線が光源とは逆を向いているのでモデルの裏側と判断し、頂点を引き伸ばす */
			scale = scale * 5000.0f ;
			fverts->vx += work->tmp_vec.vx * scale ;
			fverts->vy += work->tmp_vec.vy * scale ;
			fverts->vz += work->tmp_vec.vz * scale ;
			sverts->vx = (int)fverts->vx ;
			sverts->vy = (int)fverts->vy ;
			sverts->vz = (int)fverts->vz ;
			sverts->pad = 4096 ;
		} else {
			sverts->vx = (int)fverts->vx ;
			sverts->vy = (int)fverts->vy ;
			sverts->vz = (int)fverts->vz ;
			sverts->pad = 4096 ;
		}
		fverts++ ;
		fnorms++ ;
		sverts++ ;
	}

}

static void MakeShadowVolumeObj( DG_OBJ *obj, int which )
{
	ScrpadWork	*work = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJPACK	*obj_pack ;
	DG_SUBPACK	*sub_packet ;
	int		i ;

	/* 相対光源位置を求める */
	sceVu0TransposeMatrix( &work->tmp_mat, &obj->world );
	sceVu0ApplyMatrix( &work->light_pos, &work->tmp_mat, &work->org_light_pos );

	obj_pack = obj->packs ;
	sub_packet = obj->sub_packet[ which ] ;
	for ( i = obj->n_packs ; i > 0 ; i-- ){
		MakeShadowVolumeObjPack( obj_pack, sub_packet );
		obj_pack++ ;
		sub_packet++ ;
	}
}

void DG_MakeShadowVolumeObjs( DG_OBJS *objs, FVECTOR *lit_pos, int which )
{
	ScrpadWork	*work ;
	DG_OBJ		*obj ;
	int			i ;

	work = (ScrpadWork*)SCRPAD_ADDR ;
	work->org_light_pos = *lit_pos ;
	obj = objs->objs ;
	for ( i = objs->n_models ; i > 0 ; i-- ){
		MakeShadowVolumeObj( obj, which );
		obj++ ;
	}
}


/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*  */
static u_long	start_shadow_gif_packet[] ALIGN16 = {
	SCE_GIF_SET_TAG(7, 1, 0, 0, 0, 1),
	GS_REGS_AD,
	SCE_GS_SET_FRAME(BUFFER_PAGE(2)/2048, BUFFER_WIDTH/64, SCE_GS_PSMCT16S, 0 ),
	SCE_GS_FRAME_1,
	SCE_GS_SET_TEST( 0,0,0,0,0,0,0,0 ),
	SCE_GS_TEST_1,
	SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0,0,0,0,0,0,0,0 ),
	SCE_GS_PRIM,
	SCE_GS_SET_RGBAQ( 0, 0, 0, 0, 0x00000000 ),
	SCE_GS_RGBAQ,
	SCE_GS_SET_XYZF( (2048-DRAW_WIDTH/2)*16, (2048-DRAW_HEIGHT/2)*16, 0, 255 ),
	SCE_GS_XYZF2,
	SCE_GS_SET_XYZF( (2048+DRAW_WIDTH/2)*16, (2048+DRAW_HEIGHT/2)*16, 0, 255 ),
	SCE_GS_XYZF2,
	SCE_GS_SET_TEST( 1,7,0,0,0,0,1,2 ),
	SCE_GS_TEST_1
};
static u_long	end_shadow_gif_packet[2][12*2+2] ALIGN16 = {
	{
		SCE_GIF_SET_TAG(12, 1, 0, 0, 0, 1),
		GS_REGS_AD,
		SCE_GS_SET_FRAME(BUFFER_PAGE(0)/2048, BUFFER_WIDTH/64, FRAME_BUFFER_COLOR_MODE(), 0 ),
		SCE_GS_FRAME_1,
		SCE_GS_SET_TEST( 1,7,0,0,0,0,0,0 ),
		SCE_GS_TEST_1,
		SCE_GS_SET_CLAMP(1,0,0,0,0,0),
		SCE_GS_CLAMP_1,
		SCE_GS_SET_TEX0_1(BUFFER_PAGE(2)/64, BUFFER_WIDTH/64, SCE_GS_PSMCT16S, 9, 8, 0, 0, 0, 0, 0, 0, 0 ),
		SCE_GS_TEX0_1,
		//SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ),
		SCE_GS_SET_ALPHA( 2, 0, 2, 1, 128 ),
		//SCE_GS_SET_ALPHA( 0, 1, 2, 1, 64 ),
		SCE_GS_ALPHA_1,
		SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0,1,0,1,0,1,0,0 ),
		SCE_GS_PRIM,
		//SCE_GS_SET_RGBAQ( 0, 0, 0, 64, 0x00000000 ),
		SCE_GS_SET_RGBAQ( 128, 128, 128, 64, 0x00000000 ),
		SCE_GS_RGBAQ,
		SCE_GS_SET_UV( 0, 0 ),
		SCE_GS_UV,
		SCE_GS_SET_XYZF( (2048-DRAW_WIDTH/2)*16, (2048-DRAW_HEIGHT/2)*16, 0, 255 ),
		SCE_GS_XYZF2,
		SCE_GS_SET_UV( DRAW_WIDTH*16, DRAW_HEIGHT*16 ),
		SCE_GS_UV,
		SCE_GS_SET_XYZF( (2048+DRAW_WIDTH/2)*16, (2048+DRAW_HEIGHT/2)*16, 0, 255 ),
		SCE_GS_XYZF2,
		SCE_GS_SET_TEST( 1,7,0,0,0,0,1,2 ),
		SCE_GS_TEST_1
	},{
		SCE_GIF_SET_TAG(12, 1, 0, 0, 0, 1),
		GS_REGS_AD,
		SCE_GS_SET_FRAME(BUFFER_PAGE(1)/2048, BUFFER_WIDTH/64, FRAME_BUFFER_COLOR_MODE(), 0 ),
		SCE_GS_FRAME_1,
		SCE_GS_SET_TEST( 1,7,0,0,0,0,0,0 ),
		SCE_GS_TEST_1,
		SCE_GS_SET_CLAMP(1,0,0,0,0,0),
		SCE_GS_CLAMP_1,
		SCE_GS_SET_TEX0(BUFFER_PAGE(2)/64, BUFFER_WIDTH/64, SCE_GS_PSMCT16S, 9, 9, 0, 0, 0, 0, 0, 0, 0 ),
		SCE_GS_TEX0_1,
		//SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ),
		SCE_GS_SET_ALPHA( 2, 0, 2, 1, 128 ),
		//SCE_GS_SET_ALPHA( 0, 2, 2, 1, 128 ),
		//SCE_GS_SET_ALPHA( 0, 1, 2, 1, 64 ),
		SCE_GS_ALPHA_1,
		SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0,1,0,1,0,1,0,0 ),
		SCE_GS_PRIM,
		//SCE_GS_SET_RGBAQ( 0, 0, 0, 64, 0x00000000 ),
		SCE_GS_SET_RGBAQ( 128, 128, 128, 64, 0x00000000 ),
		SCE_GS_RGBAQ,
		SCE_GS_SET_UV( 0, 0 ),
		SCE_GS_UV,
		SCE_GS_SET_XYZF( (2048-DRAW_WIDTH/2)*16, (2048-DRAW_HEIGHT/2)*16, 0, 255 ),
		SCE_GS_XYZF2,
		SCE_GS_SET_UV( DRAW_WIDTH*16, DRAW_HEIGHT*16 ),
		SCE_GS_UV,
		SCE_GS_SET_XYZF( (2048+DRAW_WIDTH/2)*16, (2048+DRAW_HEIGHT/2)*16, 0, 255 ),
		SCE_GS_XYZF2,
		SCE_GS_SET_TEST( 1,7,0,0,0,0,1,2 ),
		SCE_GS_TEST_1
	}
};



/*----------------------------------------------------------------*/
	/*
		・
	*/
static int ShadowVolumeStart( void *tag_addr )
{
	unsigned int	*tag ;
	int				size ;

	tag = (unsigned int*)tag_addr ;

	tag[0] = DMATAG_SET_QWC( DMATAG_ID_REF, 8 );
	tag[1] = DMATAG_SET_ADDR( start_shadow_gif_packet );
	tag[2] = SCE_VIF1_SET_NOP( 0 ) ;
	tag[3] = SCE_VIF1_SET_DIRECT(8,0) ;
	tag += 4 ;
	return ( 1 );
}
static int ShadowVolumeEnd( void *tag_addr )
{
	unsigned int	*tag ;
	int				size ;

	tag = (unsigned int*)tag_addr ;

	tag[0] = DMATAG_SET_QWC( DMATAG_ID_REF, 13 );
	tag[1] = DMATAG_SET_ADDR( end_shadow_gif_packet[DG_Clock] );
	tag[2] = SCE_VIF1_SET_FLUSH(0) ;
	tag[3] = SCE_VIF1_SET_DIRECT(13,0) ;
	tag += 4 ;

	return ( 1 );
}

static int	*next_buff ;

/*----------------------------------------------------------------*/

	/*
		シャドーボリューム描画オブジェクトをＤＭＡに接続する
	*/
static	void	ChainShadowVolumeObjs( DG_OBJS *objs )
{
	ScrpadWork2	*scrpad = (ScrpadWork2*)SCRPAD_ADDR ;
	DG_OBJ		*obj ;
	DG_OBJPACK	*pack ;
	DG_SUBPACK	*sub_packet ;
	int			i, j, size ;
	int			first_flag, prog_no, bound ;

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		/* クリップアウトチェック */
		if ( obj->bound_mode & 2 ) continue ;
		if ( obj->model->type & DG_TYPE_TRANS ) continue ;
		/* ライトマトリクスを計算 */

		/* オブジェクト描画パラメータ構造体にデータをセット */
		scrpad->objenv.screen = obj->screen ;
		scrpad->objenv.connection = obj->inv_mat ;
		first_flag = 1 ;
		pack = obj->packs ;
		sub_packet = obj->sub_packet[ DG_Clock ] ;
		bound = obj->bound_mode ;
		size = ShadowVolumeStart( next_buff );
		next_buff = DG_SendScrpadDmaBuffer( size );
		for ( j = obj->n_packs ; j > 0 ; j-- ){
			/* 転送頂点データアドレスのの復元処理 */
			if ( sub_packet->flag & 7 ){
				if ( sub_packet->flag & DG_VANIME_VERTS ) sub_packet->packet.verts_tag.addr = sub_packet->verts ;
				if ( sub_packet->flag & DG_VANIME_NORMS ) sub_packet->packet.norms_tag.addr = sub_packet->norms ;
				if ( sub_packet->flag & DG_VANIME_UVS ) sub_packet->packet.uvs_tag.addr = sub_packet->uvs ;
			}
			/* 実行するＶＵ１プログラムを決定 */
			if ( bound & 1 )		prog_no = 2 ;
			else					prog_no = 0 ;
			/* エンベロープチェック */
			if ( pack->flag & DG_PACKFLAG_ENVELOPE ) prog_no += 4 ;
			/* プリシェードチェック */
			if ( objs->flag & DG_FLAG_PAINT ) prog_no += 8 ;

			/* オブジェクトパケットのデータをＤＭＡに登録する */
			size = DG_WriteObjPacks( next_buff, &sub_packet->packet, &(scrpad->objenv), prog_no, first_flag );
			next_buff = DG_SendScrpadDmaBuffer( size );
			first_flag = 0 ;
			pack ++ ;
			sub_packet++ ;
		}
		size = ShadowVolumeEnd( next_buff );
		next_buff = DG_SendScrpadDmaBuffer( size );
	}

}

/*----------------------------------------------------------------*/

	/*
		キューされた各オブジェクトをＤＭＡパケットに接続する
	*/
void		DG_ChainShadowVolumeChanl( DG_CHANL *cp, int which )
{
	DG_OBJS		**oque, *objs ;
	DG_OBJ_QUEUE	*que ;
	int		i, size ;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;

	/* ＤＭＡバッファオープン(VIF1) */
	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
	next_buff = DG_InitScrpadDmaBuffer( 512 - (sizeof(ScrpadWork2)+31)/32 );
	//next_buff = DG_InitScrpadDmaBuffer( 256 );

	/* 初期化パケットの設定 */
	size = DG_WriteObjsPacketInit3( next_buff, cp );
	next_buff = DG_SendScrpadDmaBuffer( size );

	oque = que->queue ;
	for ( i = que->n_queue ; i > 0 ; -- i, oque++ ) {
		objs = *oque ;
		if ( objs->flag & DG_FLAG_INVISIBLE ) continue ;
		if ( !( objs->flag & DG_FLAG_SHADOWVOL ) ) continue ;
		ChainShadowVolumeObjs( *oque );
	}

	/* 終了パケットの設定 */
	size = DG_WriteObjsPacketEnd( next_buff );
	next_buff = DG_SendScrpadDmaBuffer( size );

	/* ＤＭＡ転送終了待ちのダミー */
	DG_InitScrpadDmaBuffer(0);

	/* ＤＭＡバッファクローズ */
	DG_CloseDmaTask();

}
