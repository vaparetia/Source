/*
	conv4.c
	Kms to Kms2 version up(旧形式のモデルフォーマットを新形式に変換)

	2000/03/05 K.Takabe
	$Id: conv4.c,v 1.2 2002/03/28 04:38:40 usr03700 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "fmt_kms.h"
#include "prog_utl.h"

#if 1
#define DEBUG_MES(a)	/*(a)*/
#else
#define DEBUG_MES(a)	printf(a)
#endif

/* ---------------------------------------------------------------- */
/* ポインタの復元処理 */
static void KmsSetup( OLD_KMS_DEF *def )
{
	int		i, j, n_models ;
	OLD_KMS_MDL	*mdl ;
	OLD_KMS_OBJPACK	*pack ;

	/* 古いモデルではごみが入っている可能性があるため */
	n_models = def->n_x_models ;
	if ( ( n_models & ~4095 ) || ( n_models < def->n_models ) ){
		def->n_x_models = def->n_models ;
		n_models = def->n_models ;
	}

	mdl = def->models ;
	for ( i = def->n_x_models ; i > 0 ; i--, mdl++ ){
		mdl->packs = (OLD_KMS_OBJPACK*)( (int)mdl->packs + (int)def );
		pack = mdl->packs ;
		for ( j = mdl->n_packs ; j > 0 ; j--, pack++ ){
			pack->verts = (short*)( (int)pack->verts + (int)def );
			pack->norms = (short*)( (int)pack->norms + (int)def );
			pack->uvs = (short*)( (int)pack->uvs + (int)def );
			//pack->rgbs = (short*)( (int)pack->rgbs + (int)def );	/* 未使用なので */
#if 0
			{/* カリング用フラグ設定 */
				int	k ;
				for ( k = 2 ; k < pack->n_verts ; k++ ){
					if ( ( pack->norms[ k * 4 + 3 ] & 0x8000 ) == 0 ){
						FVECTOR	n, v1, v2, v3, l1, l2 ;
						SVtoFV0( &v1, (SVECTOR*)&pack->verts[ ( k - 2 ) * 4 ] );
						SVtoFV0( &v2, (SVECTOR*)&pack->verts[ ( k - 1 ) * 4 ] );
						SVtoFV0( &v3, (SVECTOR*)&pack->verts[ ( k - 0 ) * 4 ] );
						SVtoFV12( &n, (SVECTOR*)&pack->norms[ ( k - 0 ) * 4 ] );
						_sceVu0SubVector( &l1, &v2, &v1 );
						_sceVu0SubVector( &l2, &v3, &v2 );
						_sceVu0OuterProduct( &v1, &l1, &l2 );
						if ( _sceVu0InnerProduct( &n, &v1 ) < 0.0f ){
							pack->norms[ k * 4 + 3 ] &= 0x8000 ;
							pack->norms[ k * 4 + 3 ] |= 0x0000 ;
						} else {
							pack->norms[ k * 4 + 3 ] &= 0x8000 ;
							pack->norms[ k * 4 + 3 ] |= 0x0020 ;
						}
					}
				}
			}
#endif
		}
	}

}
/* ---------------------------------------------------------------- */
static void WriteKms2File( char *out_filename, OLD_KMS_DEF *old_def )
{
	int				size, offset, i ;
	OLD_KMS_MDL		*old_mdl ;
	OLD_KMS_OBJPACK	*old_pack ;
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
	def->data_format = MGS_MODEL_NORM ;
	def->n_models = old_def->n_models ;
	def->n_x_models = old_def->n_x_models ;
	def->texture = old_def->models[0].pad2 ;
	def->lx = old_def->lx ;
	def->ly = old_def->ly ;
	def->lz = old_def->lz ;
	def->ux = old_def->ux ;
	def->uy = old_def->uy ;
	def->uz = old_def->uz ;
	def->tx = old_def->models[0].tx ;
	def->ty = old_def->models[0].ty ;
	def->tz = old_def->models[0].tz ;

	/* オブジェクトユニットのコンバート */
	DEBUG_MES("header convert...\n");
	mdl = def->models ;
	old_mdl = old_def->models ;
	total_packs = 0 ;
	for ( i = 0 ; i < def->n_x_models ; i++ ){
		mdl->type = old_mdl->type ;
		mdl->n_packs = old_mdl->n_packs ;
		mdl->lx = old_mdl->lx ; mdl->ly = old_mdl->ly ; mdl->lz = old_mdl->lz ; 
		mdl->ux = old_mdl->ux ; mdl->uy = old_mdl->uy ; mdl->uz = old_mdl->uz ; 
		mdl->tx = old_mdl->tx ; mdl->ty = old_mdl->ty ; mdl->tz = old_mdl->tz ; 
		mdl->parent = old_mdl->parent ;
		mdl->packs = (void*)offset ;
		offset += sizeof(KMS_MDLPACK) * mdl->n_packs ;
		total_packs += mdl->n_packs ;
		mdl++ ;
		old_mdl++ ;
	}

	/* モデルパケットのコンバート */
	DEBUG_MES("verts convert...\n");
	pack = (KMS_MDLPACK*)&def->models[ def->n_x_models ] ;
	old_pack = (OLD_KMS_OBJPACK*)&old_def->models[ old_def->n_x_models ] ;
	//printf("total packs %d\n", total_packs );
	for ( i = 0 ; i < total_packs ; i++ ){
		//printf("%d %d %d\n", i, old_pack[i].n_verts, old_pack[i].tex_id );
		pack[i].flag = old_pack[i].flag | ( DG_PACKFLAG_TEX0 | DG_PACKFLAG_UV0 ) ;
		pack[i].n_verts = old_pack[i].n_verts ;
		pack[i].tex_id[0] = old_pack[i].tex_id ;
		pack[i].verts = (void*)offset ;
		size = AlignSize16( sizeof(short)*4, pack[i].n_verts );
		WriteData( datas, offset, old_pack[i].verts, size ); offset += size ;
	}
	DEBUG_MES("norm convert...\n");
	pack = (KMS_MDLPACK*)&def->models[ def->n_x_models ] ;
	old_pack = (OLD_KMS_OBJPACK*)&old_def->models[ old_def->n_x_models ] ;
	for ( i = 0 ; i < total_packs ; i++ ){
		pack[i].norms = (void*)offset ;
		size = AlignSize16( sizeof(short)*4, pack[i].n_verts );
		WriteData( datas, offset, old_pack[i].norms, size ); offset += size ;
	}
	DEBUG_MES("uv convert...\n");
	pack = (KMS_MDLPACK*)&def->models[ def->n_x_models ] ;
	old_pack = (OLD_KMS_OBJPACK*)&old_def->models[ old_def->n_x_models ] ;
	for ( i = 0 ; i < total_packs ; i++ ){
		pack[i].uvs[0] = (void*)offset ;
		size = AlignSize16( sizeof(short)*2, pack[i].n_verts );
		WriteData( datas, offset, old_pack[i].uvs, size ); offset += size ;
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
int Convert_KmsToKms2( char *input_filename )
{
	OLD_KMS_DEF	*old_def ;
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

	new_def = (KMS_DEF*)old_def ;
	format_id = DG_GetMdlFormat( new_def );
	if ( format_id == MGS_MODEL_NORM || format_id == MGS_MODEL_MULTITEX ){
		printf("this is new version kms file!!(%d)\n", format_id );
		free( old_def );
		return ( 0 );
	}

	DEBUG_MES("old kms setup....\n");
	KmsSetup( old_def );

	WriteKms2File( input_filename, old_def );

	free( old_def );

	return (0);
}
