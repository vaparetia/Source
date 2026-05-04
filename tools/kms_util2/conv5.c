/*
	conv5.c
	Kms2 to Kms2a version up(旧形式のモデルフォーマットを新形式に変換)

	2000/05/31 K.Takabe
	$Id: conv5.c,v 1.4 2002/04/10 02:41:47 usr04098 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "fmt_kms.h"
#include "prog_utl.h"

#if 1
#define	DEBUG_MES(a) {printf(a);}
#else
#define	DEBUG_MES(a) {}
#endif


/* ＤＭＡ関連定義 */
#define DMATAG_PCE_0		(2<<26)
#define DMATAG_PCE_1		(3<<26)
#define DMATAG_ID_REFE		(0<<28)
#define DMATAG_ID_CNT		(1<<28)
#define DMATAG_ID_NEXT		(2<<28)
#define DMATAG_ID_REF		(3<<28)
#define DMATAG_ID_REFS		(4<<28)
#define DMATAG_ID_CALL		(5<<28)
#define DMATAG_ID_RET		(6<<28)
#define DMATAG_ID_END		(7<<28)
#define DMATAG_IRQ			(1<<31)
#define DMATAG_SET_QWC( flag, qwc ) ( (flag) | (qwc) )
#define DMATAG_SET_ADDR( addr ) ( (int)(addr) )
#define DMA_SIZE_OF( n )	( ( sizeof( n ) + 3 ) / 4 )

/* ＶＩＦｃｏｄｅ関連定義 */
#define VIF_VERT_PACK		(0x0d)	/* X(1.15.0),Y(1.15.0),Z(1.15.0),W(1.3.12) */
#define VIF_NORM_PACK		(0x0d)	/* X(1.3.12),Y(1.3.12),Z(1.3.12),W(0,16,0) */
#define VIF_UV_PACK			(0x15)	/* U(1.3.12),V(1.3.12) */
#define VIF_RGBA_PACK		(0x0e)	/* R(0.8.0),G(0.8.0),B(0.8.0),A(0.8.0) */
#define VIF_DATA128			(0x0c)	/*  */

/*-vif1--------------------------------------------------*/


#define SCE_VIF1_SET_CODE(immediate, num, cmd, irq) ((u_int)(immediate) | ((u_int)(num) << 16) | ((u_int)(cmd) << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_NOP(irq) ((u_int)(irq) << 31)
#define SCE_VIF1_SET_STCYCL(wl, cl, irq) ((u_int)(cl) | ((u_int)(wl) << 8) | ((u_int)(0) << 16) | ((u_int)0x01 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_OFFSET(offset, irq) ((u_int)(offset) | ((u_int)0x02 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_BASE(base, irq) ((u_int)(base) | ((u_int)0x03 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_ITOP(itop, irq) ((u_int)(itop) | ((u_int)0x04 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_STMOD(stmod, irq) ((u_int)(stmod) | ((u_int)0x05 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_MSKPATH3(msk, irq) ((u_int)(msk) | ((u_int)0x06 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_MARK(mark, irq) ((u_int)(mark) | ((u_int)0x07 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_FLUSHE(irq) (((u_int)0x10 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_FLUSH(irq) (((u_int)0x11 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_FLUSHA(irq) (((u_int)0x13 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_MSCAL(vuaddr, irq) ((u_int)(vuaddr) | ((u_int)0x14 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_MSCNT(irq) (((u_int)0x17 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_MSCALF(vuaddr, irq) ((u_int)(vuaddr) | ((u_int)0x15 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_STMASK(irq) (((u_int)0x20 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_STROW(irq) (((u_int)0x30 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_STCOL(irq) (((u_int)0x31 << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_MPG(vuaddr, num, irq) ((u_int)(vuaddr) | ((u_int)(num) << 16) | ((u_int)(0x4a) << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_DIRECT(count, irq) ((u_int)(count) | ((u_int)(0x50) << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_DIRECTHL(count, irq) ((u_int)(count) | ((u_int)(0x51) << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_UNPACK(vuaddr, num, cmd, irq) ((u_int)(vuaddr) | ((u_int)(num) << 16) | ((u_int)(0x60 | (cmd)) << 24) | ((u_int)(irq) << 31))
#define SCE_VIF1_SET_UNPACKR(vuaddr, num, cmd, irq) ((u_int)((vuaddr) | 0x8000) | ((u_int)(num) << 16) | ((u_int)(0x60 | (cmd)) << 24) | ((u_int)(irq) << 31))


/* ---------------------------------------------------------------- */
/* ポインタの復元処理 */
static void KmsDataSetup( KMS_DEF *def )
{
	KMS_MDL		*mdl ;
	KMS_MDLPACK	*pack ;
	int			i, j, n_models, tri_code ;

	mdl = def->models ;
	for ( i = def->n_x_models ; i > 0 ; i--, mdl++ ){
		mdl->packs = (KMS_MDLPACK*)( (int)mdl->packs + (int)def );
		pack = mdl->packs ;
		for ( j = mdl->n_packs ; j > 0 ; j--, pack++ ){
			pack->verts = (short*)( (int)pack->verts + (int)def );
			pack->norms = (short*)( (int)pack->norms + (int)def );
			if ( pack->uvs[0] != 0 && pack->flag & (DG_PACKFLAG_UV0))
				pack->uvs[0] = (short*)( (int)pack->uvs[0] + (int)def );
			if ( pack->uvs[1] != 0 && pack->flag & (DG_PACKFLAG_UV1|DG_PACKFLAG_BMAP))
				pack->uvs[1] = (short*)( (int)pack->uvs[1] + (int)def );
			if ( pack->uvs[2] != 0 && pack->flag & (DG_PACKFLAG_UV1|DG_PACKFLAG_BMAP))
				pack->uvs[2] = (short*)( (int)pack->uvs[2] + (int)def );
			if ( pack->rgbs != 0 ) pack->rgbs = (short*)( (int)pack->rgbs + (int)def );	/* 未使用なので */
			//printf("%08x\n", pack->pad );
		}
	}
}
/* ---------------------------------------------------------------- */
static void WriteKms2aFile( char *out_filename, KMS_DEF *old_def )
{
	int				size, offset, i ;
	KMS_MDL			*old_mdl ;
	KMS_MDLPACK		*old_pack ;
	KMS_DEF			*def ;
	KMS_MDL			*mdl ;
	KMS_MDLPACK		*pack ;
	void			*mem, *datas ;
	int				total_packs ;
	FILE			*fp ;

	datas = mem = Malloc0( 1024*1024*4 );

	offset = 0 ;
	size = sizeof(KMS_DEF) + sizeof(KMS_MDL) * old_def->n_x_models ;
	def = mem ; offset += size ;

	/* ヘッダーのコンバート */
	*def = *old_def ;
	//def->data_format = 0 ;
	if ( DG_GetMdlFormat( def ) == MGS_MODEL_NORM ) def->data_format = MGS_MODEL_NORM_A | DG_GetMdlFlag( def );
	if ( DG_GetMdlFormat( def ) == MGS_MODEL_MULTITEX ) def->data_format = MGS_MODEL_MULTITEX_A | DG_GetMdlFlag( def );

	/* オブジェクトユニットのコンバート */
	DEBUG_MES("header convert...\n");
	mdl = def->models ;
	old_mdl = old_def->models ;
	total_packs = 0 ;
	for ( i = 0 ; i < def->n_x_models ; i++ ){
		*mdl = *old_mdl ;
		mdl->packs = (void*)offset ;
		offset += sizeof(KMS_MDLPACK) * mdl->n_packs ;
		total_packs += mdl->n_packs ;
		mdl++ ;
		old_mdl++ ;
	}

	/* モデルパケットのコンバート */
	DEBUG_MES("verts convert...\n");
	pack = (KMS_MDLPACK*)&def->models[ def->n_x_models ] ;
	old_pack = (KMS_MDLPACK*)&old_def->models[ old_def->n_x_models ] ;
	//printf("total packs %d\n", total_packs );
	for ( i = 0 ; i < total_packs ; i++ ){
		//printf("%d %d %d\n", i, old_pack[i].n_verts, old_pack[i].tex_id );
		pack[i] = old_pack[i] ;
		pack[i].verts = (void*)offset ;
		size = AlignSize16( sizeof(short)*4, pack[i].n_verts );
		WriteData( datas, offset, old_pack[i].verts, size ); offset += size ;
	}
	DEBUG_MES("norm convert...\n");
	pack = (KMS_MDLPACK*)&def->models[ def->n_x_models ] ;
	old_pack = (KMS_MDLPACK*)&old_def->models[ old_def->n_x_models ] ;
	for ( i = 0 ; i < total_packs ; i++ ){
		pack[i].norms = (void*)offset ;
		size = AlignSize16( sizeof(short)*4, pack[i].n_verts );
		WriteData( datas, offset, old_pack[i].norms, size ); offset += size ;
	}
	DEBUG_MES("uv convert 0...\n");
	pack = (KMS_MDLPACK*)&def->models[ def->n_x_models ] ;
	old_pack = (KMS_MDLPACK*)&old_def->models[ old_def->n_x_models ] ;
	for ( i = 0 ; i < total_packs ; i++ ){
		if ( pack[i].flag & DG_PACKFLAG_UV0 && old_pack[i].uvs[0] != NULL ){
			pack[i].uvs[0] = (void*)offset ;
			size = AlignSize16( sizeof(short)*2, pack[i].n_verts );
			WriteData( datas, offset, old_pack[i].uvs[0], size ); offset += size ;
		} else {
			pack[i].uvs[0] = NULL ;
		}
	}
	DEBUG_MES("uv convert 1...\n");
	pack = (KMS_MDLPACK*)&def->models[ def->n_x_models ] ;
	old_pack = (KMS_MDLPACK*)&old_def->models[ old_def->n_x_models ] ;
	for ( i = 0 ; i < total_packs ; i++ ){
		if ( pack[i].flag & (DG_PACKFLAG_UV1|DG_PACKFLAG_BMAP) ){
			pack[i].uvs[1] = (void*)offset ;
			size = AlignSize16( sizeof(short)*2, pack[i].n_verts );
			WriteData( datas, offset, old_pack[i].uvs[1], size ); offset += size ;
		} else {
			pack[i].uvs[1] = NULL ;
		}
	}
	DEBUG_MES("uv convert 2...\n");
	pack = (KMS_MDLPACK*)&def->models[ def->n_x_models ] ;
	old_pack = (KMS_MDLPACK*)&old_def->models[ old_def->n_x_models ] ;
	for ( i = 0 ; i < total_packs ; i++ ){
		if ( pack[i].flag & (DG_PACKFLAG_UV2|DG_PACKFLAG_BMAP) ){
			pack[i].uvs[2] = (void*)offset ;
			size = AlignSize16( sizeof(short)*2, pack[i].n_verts );
			WriteData( datas, offset, old_pack[i].uvs[2], size ); offset += size ;
		} else {
			pack[i].uvs[2] = NULL ;
		}
	}
	DEBUG_MES("create new format vertex datas...\n");
	pack = (KMS_MDLPACK*)&def->models[ def->n_x_models ] ;
	old_pack = (KMS_MDLPACK*)&old_def->models[ old_def->n_x_models ] ;
	for ( i = 0 ; i < total_packs ; i++ ){
		u_int		*verts_tag, *norms_tag, *uvs0_tag, *uvs1_tag, *uvs2_tag ;
		int			top_offset ;

		offset = ( offset + 127 ) & 0xffffff80 ;
		top_offset = offset ;
		pack[i].pad = (int)offset ;
		/* 頂点データ書き込み */
		//DEBUG_MES("write verts...\n");
		verts_tag = (u_int*)( (int)datas + offset ) ;
		offset += 16 ;
		size = AlignSize16( sizeof(short)*4, pack[i].n_verts );
		WriteData( datas, offset, old_pack[i].verts, size ); offset += size ;
		/* 法線データ書き込み */
		//DEBUG_MES("write norms...\n");
		norms_tag = (u_int*)( (int)datas + offset ) ;
		offset += 16 ;
		size = AlignSize16( sizeof(short)*4, pack[i].n_verts );
		WriteData( datas, offset, old_pack[i].norms, size ); offset += size ;
		/* UV０データ書き込み */
		if ( pack[i].flag & (DG_PACKFLAG_UV0) ){
			//DEBUG_MES("write uvs0...\n");
			uvs0_tag = (u_int*)( (int)datas + offset ) ;
			offset += 16 ;
			size = AlignSize16( sizeof(short)*2, pack[i].n_verts );
			WriteData( datas, offset, old_pack[i].uvs[0], size ); offset += size ;
		} else {
			uvs0_tag = NULL ;
		}
		/* UV１データ書き込み */
		if ( pack[i].flag & (DG_PACKFLAG_UV1|DG_PACKFLAG_BMAP) ){
			//DEBUG_MES("write uvs1...\n");
			uvs1_tag = (u_int*)( (int)datas + offset ) ;
			offset += 16 ;
			size = AlignSize16( sizeof(short)*2, pack[i].n_verts );
			WriteData( datas, offset, old_pack[i].uvs[1], size ); offset += size ;
		} else {
			uvs1_tag = NULL ;
		}
		/* UV２データ書き込み */
		if ( pack[i].flag & (DG_PACKFLAG_UV2|DG_PACKFLAG_BMAP) ){
			//DEBUG_MES("write uvs2...\n");
			uvs2_tag = (u_int*)( (int)datas + offset ) ;
			offset += 16 ;
			size = AlignSize16( sizeof(short)*2, pack[i].n_verts );
			WriteData( datas, offset, old_pack[i].uvs[2], size ); offset += size ;
		} else {
			uvs2_tag = NULL ;
		}
		/* タグデータの書き込み */
		//DEBUG_MES("write dmatag...\n");
		//printf("top %d offset %d size %d\n", top_offset, offset, ( offset - top_offset ) / 16 - 1 );
		if ( DG_GetMdlFormat( def ) == MGS_MODEL_NORM_A ){
			verts_tag[0] = DMATAG_SET_QWC( DMATAG_ID_RET, ( offset - top_offset ) / 16 - 1 ) ;
			verts_tag[1] = 0 ;
			verts_tag[2] = SCE_VIF1_SET_STCYCL( 1, 3, 0 ) ;
			verts_tag[3] = SCE_VIF1_SET_UNPACKR( 0x100, (pack[i].n_verts+1)&0xfe, VIF_VERT_PACK, 0 ) ;
			norms_tag[0] = 0 ;
			norms_tag[1] = 0 ;
			norms_tag[2] = SCE_VIF1_SET_STCYCL( 1, 3, 0 ) ;
			norms_tag[3] = SCE_VIF1_SET_UNPACKR( 0x101, (pack[i].n_verts+1)&0xfe, VIF_NORM_PACK, 0 ) ;
			if ( uvs0_tag != NULL ){
				uvs0_tag[0] = 0 ;
				uvs0_tag[1] = 0 ;
				uvs0_tag[2] = SCE_VIF1_SET_STCYCL( 1, 3, 0 ) ;
				uvs0_tag[3] = SCE_VIF1_SET_UNPACKR( 0x102, (pack[i].n_verts+3)&0xfc, VIF_UV_PACK, 0 ) ;
			}
		} else {
			verts_tag[0] = DMATAG_SET_QWC( DMATAG_ID_RET, ( offset - top_offset ) / 16 - 1 ) ;
			verts_tag[1] = 0 ;
			verts_tag[2] = SCE_VIF1_SET_STCYCL( 1, 6, 0 ) ;
			verts_tag[3] = SCE_VIF1_SET_UNPACKR( 0x100, (pack[i].n_verts+1)&0xfe, VIF_VERT_PACK, 0 ) ;
			norms_tag[0] = 0 ;
			norms_tag[1] = 0 ;
			norms_tag[2] = SCE_VIF1_SET_STCYCL( 1, 6, 0 ) ;
			norms_tag[3] = SCE_VIF1_SET_UNPACKR( 0x101, (pack[i].n_verts+1)&0xfe, VIF_NORM_PACK, 0 ) ;
			if ( uvs0_tag != NULL ){
				uvs0_tag[0] = 0 ;
				uvs0_tag[1] = 0 ;
				uvs0_tag[2] = SCE_VIF1_SET_STCYCL( 1, 6, 0 ) ;
				uvs0_tag[3] = SCE_VIF1_SET_UNPACKR( 0x102, (pack[i].n_verts+3)&0xfc, VIF_UV_PACK, 0 ) ;
			}
			if ( uvs2_tag != NULL ){
				uvs1_tag[0] = 0 ;
				uvs1_tag[1] = 0 ;
				uvs1_tag[2] = SCE_VIF1_SET_STCYCL( 1, 6, 0 ) ;
				uvs1_tag[3] = SCE_VIF1_SET_UNPACKR( 0x103, (pack[i].n_verts+3)&0xfc, VIF_UV_PACK, 0 ) ;
			}
			if ( uvs2_tag != NULL ){
				uvs2_tag[0] = 0 ;
				uvs2_tag[1] = 0 ;
				uvs2_tag[2] = SCE_VIF1_SET_STCYCL( 1, 6, 0 ) ;
				uvs2_tag[3] = SCE_VIF1_SET_UNPACKR( 0x104, (pack[i].n_verts+3)&0xfc, VIF_UV_PACK, 0 ) ;
			}
		}
		//printf(" %08x %08x %08x %08x %08x \n", verts_tag, verts_tag[0], verts_tag[1], verts_tag[2], verts_tag[3] );

	}

	if ( ( fp = fopen( out_filename, "wb" ) ) == NULL ){
		printf("file write error!!(%s)\n", out_filename );
		Free( mem );
		return ;
	}
	fwrite( mem, 1, offset, fp );
	fclose( fp );

	Free( mem );
}

/* ---------------------------------------------------------------- */
int Convert_Kms2ToKms2a( char *input_filename )
{
	KMS_DEF		*old_def ;
	KMS_DEF		*new_def ;
	FILE	*fp ;
	int		size, format_id ;

	if ( ( fp = fopen( input_filename, "r+b" ) ) == NULL ){
		fprintf(stderr,"file open error!!(%s)\n", input_filename );
		return (-1);
	}

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	old_def = malloc( size );

	fread( old_def, 1, size, fp );
	fclose( fp );

	format_id = DG_GetMdlFormat( old_def );
	if ( !( format_id == MGS_MODEL_NORM || format_id == MGS_MODEL_MULTITEX ||
			format_id == MGS_MODEL_NORM_A || format_id == MGS_MODEL_MULTITEX_A ) ){
		printf("this is not kms2 file!!(%d)\n", format_id );
		free( old_def );
		return ( 0 );
	}

	DEBUG_MES("kms2 setup....\n");
	KmsDataSetup( old_def );

	WriteKms2aFile( input_filename, old_def );

	free( old_def );

	return (0);
}
