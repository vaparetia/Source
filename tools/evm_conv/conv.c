/*
	conv.c
	convert program

*/

#include <stdio.h>
#include <stdlib.h>


typedef struct {
	float	vx, vy, vz, vw ;
} FVECTOR ;
typedef struct {
	short	vx, vy, vz, pad ;
} SVECTOR ;
typedef struct {
	unsigned char	r, g, b, a ;
} CVECTOR ;

#include "fmt_kms.h"
#include "prog_utl.h"

/* ---------------------------------------------------------------- */
#define SET_ADDR( a, b ) ( ( a == NULL ) ? NULL : ((void*)( (int)a + (int)b )) )
#define UNSET_ADDR( a, b, c ) ( ( a == NULL ) ? NULL : ((void*)( (int)a - (int)b + (int)c )) )
#define FLOAT_MAX	(9999999999999.0f)
#define DG_MAX( _a, _b ) ( ( (_a) > (_b) ) ? (_a) : (_b) )
#define DG_MIN( _a, _b ) ( ( (_a) < (_b) ) ? (_a) : (_b) )


static void *data_top_addr ;
static int	data_size ;

/* ---------------------------------------------------------------- */
/* マトリクスリストの最適化用ワーク */
static int mat_list[8] ;
static int mat_conv_table[8] ;
static int max_mat_list ;
/*  */
static void InitMatrixList( void )
{
	int		i ;
	for ( i = 0 ; i < 8 ; i++ ) mat_list[i] = 255 ;
	max_mat_list = 0 ;
}
static int CheckAddMatrixList( unsigned char *mat_id )
{
	int		i, j ;
	int		local_mat_list[8] ;
	int		local_max_mat_list ;

	for ( i = 0 ; i < 8 ; i++ ) local_mat_list[ i ] = mat_list[ i ] ;
	local_max_mat_list = max_mat_list ;

	for ( i = 0 ; i < 8 ; i++ ){
		if ( mat_id[ i ] == 0xff ) break ;
		for ( j = 0 ; j < local_max_mat_list ; j++ ){
			if ( local_mat_list[ j ] == mat_id[ i ] ) break ;
		}
		if ( j == local_max_mat_list ){
			if ( local_max_mat_list >= 8 ) return ( -1 );
			local_mat_list[ j ] = mat_id[ i ] ;
			local_max_mat_list++ ;
		}
	}
	return ( local_max_mat_list );
}
static int AddMatrixList( unsigned char *mat_id )
{
	int		i, j ;

	for ( i = 0 ; i < 8 ; i++ ){
		if ( mat_id[ i ] == 0xff ) break ;
		for ( j = 0 ; j < max_mat_list ; j++ ){
			if ( mat_list[ j ] == mat_id[ i ] ) break ;
		}
		if ( j == max_mat_list ){
			if ( max_mat_list >= 8 ) return ( -1 );
			mat_list[ j ] = mat_id[ i ] ;
			max_mat_list++ ;
		}
	}
	//printf("%d : %d %d %d %d %d %d %d %d\n", max_mat_list,
	//	   mat_list[0], mat_list[1], mat_list[2], mat_list[3],
	//	   mat_list[4], mat_list[5], mat_list[6], mat_list[7] );
	return ( max_mat_list );
}
static void SortMatrixList( void )
{
	int		i, j, tmp ;
	for ( i = 0 ; i < ( max_mat_list - 1 ) ; i++ ){
		for ( j = 0 ; j < ( max_mat_list - 1 - i ) ; j++ ){
			if ( mat_list[ j ] > mat_list[ j + 1 ] ){
				tmp = mat_list[ j ] ;
				mat_list[ j ] = mat_list[ j + 1 ] ;
				mat_list[ j + 1 ] = tmp ;
			}
		}
	}
}
static void MakeMatrixListConvertTable( unsigned char *mat_id )
{
	int		i, j ;
	//printf("%d %d %d %d %d %d %d %d\n",
	//	   mat_id[0], mat_id[1], mat_id[2], mat_id[3],
	//	   mat_id[4], mat_id[5], mat_id[6], mat_id[7] );
	for ( i = 0 ; i < 8 ; i++ ){
		if ( mat_id[ i ] == 0xff ){
			mat_conv_table[ i ] = -1 ;
			continue ;
		}
		for ( j = 0 ; j < max_mat_list ; j++ ){
			if ( mat_list[ j ] == mat_id[ i ] ){
				mat_conv_table[ i ] = j ;
				break ;
			}
		}
		if ( j == max_mat_list ) printf("--------- error ------------\n");
	}
#if 0
	printf("table: %3d %3d %3d %3d %3d %3d %3d %3d\n", 
		   mat_conv_table[0], mat_conv_table[1], mat_conv_table[2], mat_conv_table[3], 
		   mat_conv_table[4], mat_conv_table[5], mat_conv_table[6], mat_conv_table[7] );
#endif
}
static void ChangeMatrixID( unsigned char *mat_id, unsigned char *weight, int n_verts, int n_mat )
{
	int		i, j ;
	MakeMatrixListConvertTable( mat_id );
	for ( i = 0 ; i < n_verts ; i++ ){
		for ( j = 0 ; j < n_mat ; j++ ){
			weight[ j + 4 ] = mat_conv_table[ weight[ j + 4 ] / 4 ] * 4 ;
			if ( weight[ j ] == 0 ) weight[ j + 4 ] = 0 ;
		}
		weight += 8 ;
	}
	for ( i = 0 ; i < 8 ; i++ ){
		mat_id[ i ] = mat_list[ i ] ;
	}
}

/* ---------------------------------------------------------------- */
static void SVtoFV0( FVECTOR *v, SVECTOR *sv ){
	v->vx = sv->vx ;
	v->vy = sv->vy ;
	v->vz = sv->vz ;
	v->vw = 1.0f ;
}
static void SVtoFV12( FVECTOR *v, SVECTOR *sv ){
	v->vx = (float)sv->vx / 4096.0f ;
	v->vy = (float)sv->vy / 4096.0f  ;
	v->vz = (float)sv->vz / 4096.0f  ;
	v->vw = 1.0f ;
}
/* ---------------------------------------------------------------- */

static void OldEvmDataSetup( OLD_EVM_DEF *def )
{
	int			i ;
	EVM_PACK	*pack ;

	def->packet = SET_ADDR( def->packet, def );
	pack = def->packet ;
	for ( i = def->n_packs ; i > 0 ; i--, pack++ ){
		pack->verts = SET_ADDR( pack->verts, def );
		pack->norms = SET_ADDR( pack->norms, def );
		pack->uvs[0] = SET_ADDR( pack->uvs[0], def );
		pack->uvs[1] = SET_ADDR( pack->uvs[1], def );
		pack->uvs[2] = SET_ADDR( pack->uvs[2], def );
		pack->weight = SET_ADDR( pack->weight, def );
		pack->index  = SET_ADDR( pack->index,  def );
	}
}
static void OldEvmDataOptimize( OLD_EVM_DEF *def )
{
	int			i ;
	EVM_PACK	*pack ;

	{/* マトリクスリストの最適化 */
		int		j, start_pack_num, end_pack_num ;
		InitMatrixList();
		pack = def->packet ;
		start_pack_num = 0 ;
		for ( i = 0 ; i < def->n_packs ; i++ ){
#if 0
			printf("a pack %03d:%3d %3d %3d %3d %3d %3d %3d %3d\n",
				   i,
				   pack[i].mat_id[0], pack[i].mat_id[1], pack[i].mat_id[2], pack[i].mat_id[3],
				   pack[i].mat_id[4], pack[i].mat_id[5], pack[i].mat_id[6], pack[i].mat_id[7] );
#endif
			if ( CheckAddMatrixList( pack[i].mat_id ) >= 0 ){
				AddMatrixList( pack[i].mat_id );
				end_pack_num = i ;
			} else {
				SortMatrixList();
				for ( j = start_pack_num ; j <= end_pack_num ; j++ ){
#if 0
					printf("a pack %03d:%3d %3d %3d %3d %3d %3d %3d %3d\n", j,
						   pack[j].mat_id[0], pack[j].mat_id[1], pack[j].mat_id[2], pack[j].mat_id[3],
						   pack[j].mat_id[4], pack[j].mat_id[5], pack[j].mat_id[6], pack[j].mat_id[7] );
#endif
					ChangeMatrixID( pack[j].mat_id,
									pack[j].weight,
									pack[j].n_verts,
									pack[j].n_mats );
#if 0
					printf("b pack %03d:%3d %3d %3d %3d %3d %3d %3d %3d\n", j,
						   pack[j].mat_id[0], pack[j].mat_id[1], pack[j].mat_id[2], pack[j].mat_id[3],
						   pack[j].mat_id[4], pack[j].mat_id[5], pack[j].mat_id[6], pack[j].mat_id[7] );
#endif
				}
				InitMatrixList();
				start_pack_num = i ;
				AddMatrixList( pack[i].mat_id );
				end_pack_num = i ;
			}
		}
#if 0
		for ( i = 0 ; i < def->n_packs ; i++, pack++ ){
			printf("pack %03d:%3d %3d %3d %3d %3d %3d %3d %3d\n",
				   i,
				   pack->mat_id[0], pack->mat_id[1], pack->mat_id[2], pack->mat_id[3],
				   pack->mat_id[4], pack->mat_id[5], pack->mat_id[6], pack->mat_id[7]
				   );
		}
#endif
	}
}

static OLD_EVM_DEF *LoadOldEvm( char *filename )
{
	FILE	*fp ;
	OLD_EVM_DEF	*old_evm_def ;
	int		size ;

	if ( ( fp = fopen( filename, "rb" ) ) == NULL ){
		printf("file open error!!(%s)\n", filename );
		return ( NULL );
	}
	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	old_evm_def = Malloc0( size );

	fread( old_evm_def, size, 1, fp );

	fclose( fp );

	OldEvmDataSetup( old_evm_def );

	data_top_addr = &old_evm_def->packet[ old_evm_def->n_packs ] ;
	data_size = size - sizeof(OLD_EVM_DEF)
		- sizeof(OLD_EVM_SKEL) * old_evm_def->n_x_models
		- sizeof(EVM_PACK) * old_evm_def->n_packs ;

	return ( old_evm_def );
}

/* スケルトンのバウンディングを生成 */
static void MakeSkeletonBound( OLD_EVM_DEF *def, EVM_SKEL *skel )
{
	int			i, j, index ;
	EVM_PACK	*pack ;
	FVECTOR		*bound_list, *max, *min ;
	SVECTOR		*vec ;
	char	*weight ;

	bound_list = Malloc0( sizeof(FVECTOR) * 2 * def->n_x_models );
	/* バウンディングの初期化 */
	for ( i = 0 ; i < def->n_x_models ; i++ ){
		bound_list[ i * 2 + 0 ].vx = FLOAT_MAX ;
		bound_list[ i * 2 + 0 ].vy = FLOAT_MAX ;
		bound_list[ i * 2 + 0 ].vz = FLOAT_MAX ;
		bound_list[ i * 2 + 0 ].vw = 0.0f ;			/* 記録チェックフラグとして使用 */
		bound_list[ i * 2 + 1 ].vx = -FLOAT_MAX ;
		bound_list[ i * 2 + 1 ].vy = -FLOAT_MAX ;
		bound_list[ i * 2 + 1 ].vz = -FLOAT_MAX ;
		bound_list[ i * 2 + 1 ].vw = 1.0f ;
	}

	pack = def->packet ;
	for ( i = def->n_packs ; i > 0 ; i--, pack++ ){
		weight = (unsigned char*)pack->weight ;
		vec = (SVECTOR*)pack->verts ;
		if ( def->type & DG_EVMTYPE_LARGE ){/* バウンディング再計算 */
			int	k ;
			for ( k = 0 ; k < pack->n_verts ; k++ ){
				FVECTOR	v ;
				SVtoFV0( &v, vec );
				for ( j = 0 ; j < pack->n_mats ; j++ ){
					if ( weight[j] == 0 ) continue ;
					index = pack->mat_id[ weight[j+4]/4 ];
					if ( index == 0xff ) continue ;
					/* ウェイトの掛かっているスケルトンに対してバウンディングを更新 */
					min = &bound_list[ index * 2 + 0 ];
					max = &bound_list[ index * 2 + 1 ];
					max->vx = DG_MAX( max->vx, v.vx );
					max->vy = DG_MAX( max->vy, v.vy );
					max->vz = DG_MAX( max->vz, v.vz );
					min->vx = DG_MIN( min->vx, v.vx );
					min->vy = DG_MIN( min->vy, v.vy );
					min->vz = DG_MIN( min->vz, v.vz );
					min->vw = 1.0f ;	/* 記録フラグを立てる */
				}
				vec++ ;
				weight += 8 ;
			}
		} else {
			int	k ;
			for ( k = 0 ; k < pack->n_verts ; k++ ){
				FVECTOR	v ;
				SVtoFV0( &v, vec );
				v.vx /= 16.0f ;
				v.vy /= 16.0f ;
				v.vz /= 16.0f ;
				for ( j = 0 ; j < pack->n_mats ; j++ ){
					if ( weight[j] == 0 ) continue ;
					index = pack->mat_id[ weight[j+4]/4 ];
					if ( index == 0xff ) continue ;
					/* ウェイトの掛かっているスケルトンに対してバウンディングを更新 */
					min = &bound_list[ index * 2 + 0 ];
					max = &bound_list[ index * 2 + 1 ];
					max->vx = DG_MAX( max->vx, v.vx );
					max->vy = DG_MAX( max->vy, v.vy );
					max->vz = DG_MAX( max->vz, v.vz );
					min->vx = DG_MIN( min->vx, v.vx );
					min->vy = DG_MIN( min->vy, v.vy );
					min->vz = DG_MIN( min->vz, v.vz );
					min->vw = 1.0f ;	/* 記録フラグを立てる */
				}
				vec++ ;
				weight += 8 ;
			}
		}
		
	}

	for ( i = 0 ; i < def->n_x_models ; i++ ){
		/* ウェイトが掛かっていないダミースケルトンにはスケルトン中心座標をバウンディングに設定 */
		if ( bound_list[ i * 2 + 0 ].vw == 0.0f ){
			bound_list[ i * 2 + 0 ].vx = def->skeleton[ i ].rt_tx ;
			bound_list[ i * 2 + 0 ].vy = def->skeleton[ i ].rt_ty ;
			bound_list[ i * 2 + 0 ].vz = def->skeleton[ i ].rt_tz ;
			bound_list[ i * 2 + 1 ].vx = def->skeleton[ i ].rt_tx ;
			bound_list[ i * 2 + 1 ].vy = def->skeleton[ i ].rt_ty ;
			bound_list[ i * 2 + 1 ].vz = def->skeleton[ i ].rt_tz ;
			bound_list[ i * 2 + 0 ].vw = 1.0f ;
		}
		skel[ i ].lx = bound_list[ i * 2 + 0 ].vx ;
		skel[ i ].ly = bound_list[ i * 2 + 0 ].vy ;
		skel[ i ].lz = bound_list[ i * 2 + 0 ].vz ;
		skel[ i ].ux = bound_list[ i * 2 + 1 ].vx ;
		skel[ i ].uy = bound_list[ i * 2 + 1 ].vy ;
		skel[ i ].uz = bound_list[ i * 2 + 1 ].vz ;
		//printf("%d:%f %f %f %f %f %f\n", i,
		//	   bound_list[i*2+0].vx, bound_list[i*2+0].vy, bound_list[i*2+0].vz,
		//	   bound_list[i*2+1].vx, bound_list[i*2+1].vy, bound_list[i*2+1].vz );
	}
	free( bound_list );

}

/* ================================================================ */
/*
	Main Routine
*/
int Convert_EvmVersionUp( char *input_filename )
{
	char	output_filename[ 1024 ] ;
	EVM_DEF	*evm_def ;
	OLD_EVM_DEF	*old_evm_def ;
	FILE		*fp ;
	int			i, offset, size, s ;

	InitMatrixList();

	if ( ( old_evm_def = LoadOldEvm( input_filename ) ) == NULL ){
		return ( -1 );
	}

	if ( old_evm_def->type & DG_EVMTYPE_FORMAT_V3 ) return ( 0 );

	OldEvmDataOptimize( old_evm_def );

	evm_def = Malloc0( sizeof(EVM_DEF) + sizeof(EVM_SKEL) * old_evm_def->n_x_models );
	evm_def->n_models = old_evm_def->n_models ;
	evm_def->n_x_models = old_evm_def->n_x_models ;
	evm_def->lx = old_evm_def->lx ;
	evm_def->ly = old_evm_def->ly ;
	evm_def->lz = old_evm_def->lz ;
	evm_def->ux = old_evm_def->ux ;
	evm_def->uy = old_evm_def->uy ;
	evm_def->uz = old_evm_def->uz ;
	evm_def->type = old_evm_def->type ;
	evm_def->type |= DG_EVMTYPE_FORMAT_V3 ;
	evm_def->texture = old_evm_def->texture ;
	evm_def->n_packs = old_evm_def->n_packs ;
	for ( i = 0 ; i < evm_def->n_x_models ; i++ ){
		evm_def->skeleton[ i ].flag = old_evm_def->skeleton[ i ].flag ;
		evm_def->skeleton[ i ].parent = old_evm_def->skeleton[ i ].parent ;
		evm_def->skeleton[ i ].tx = old_evm_def->skeleton[ i ].tx ;
		evm_def->skeleton[ i ].ty = old_evm_def->skeleton[ i ].ty ;
		evm_def->skeleton[ i ].tz = old_evm_def->skeleton[ i ].tz ;
		evm_def->skeleton[ i ].rt_tx = old_evm_def->skeleton[ i ].rt_tx ;
		evm_def->skeleton[ i ].rt_ty = old_evm_def->skeleton[ i ].rt_ty ;
		evm_def->skeleton[ i ].rt_tz = old_evm_def->skeleton[ i ].rt_tz ;
	}
	offset = sizeof(EVM_DEF) + sizeof(EVM_SKEL) * old_evm_def->n_x_models ;
	offset += sizeof(EVM_PACK) * evm_def->n_packs ;
	evm_def->packet = (void*)( sizeof(EVM_DEF) + sizeof(EVM_SKEL) * old_evm_def->n_x_models );

	MakeSkeletonBound( old_evm_def, evm_def->skeleton );

	size = 0 ;

	if ( ( fp = fopen( input_filename, "wb" ) ) == NULL ) return ( -1 );

	s = sizeof(EVM_DEF) ;
	fwrite( evm_def, s, 1, fp );
	size += s ;

	s = sizeof(EVM_SKEL) * evm_def->n_x_models ;
	fwrite( evm_def->skeleton, s, 1, fp );
	size += s ;

	for ( i = 0 ; i < evm_def->n_packs ; i++ ){
		EVM_PACK	*pack ;
		pack = &old_evm_def->packet[ i ] ;
		pack->verts = UNSET_ADDR( pack->verts, data_top_addr, offset );
		pack->norms = UNSET_ADDR( pack->norms, data_top_addr, offset );
		pack->uvs[0] = UNSET_ADDR( pack->uvs[0], data_top_addr, offset );
		pack->uvs[1] = UNSET_ADDR( pack->uvs[1], data_top_addr, offset );
		pack->uvs[2] = UNSET_ADDR( pack->uvs[2], data_top_addr, offset );
		pack->weight = UNSET_ADDR( pack->weight, data_top_addr, offset );
		pack->index = UNSET_ADDR( pack->index, data_top_addr, offset );
		fwrite( pack, sizeof(EVM_PACK), 1, fp );
	}

	fwrite( data_top_addr, data_size, 1, fp );

	fclose( fp );

	free( evm_def );
	free( old_evm_def );

	return ( 0 );
}


