/*
	conv2.c
	convert program

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>


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

static EVX_VERTEX		VertexBuffer[ 20000 ];
static int				MaxVertexBuffer ;
static int				CheckVertex ;

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
/* ---------------------------------------------------------------- */
u_short		IndexChangeTable[ 32738 ];
int	AddVertex( EVX_VERTEX *verts )
{
	int		i ;
	EVX_VERTEX	*v ;
	v = VertexBuffer ;
	CheckVertex++ ;
	for ( i = 0 ; i < MaxVertexBuffer ; i++, v++ ){
		if ( v->vx != verts->vx ) continue ;
		if ( v->vy != verts->vy ) continue ;
		if ( v->vz != verts->vz ) continue ;
		if ( v->nx != verts->nx ) continue ;
		if ( v->ny != verts->ny ) continue ;
		if ( v->nz != verts->nz ) continue ;
		if ( v->u0 != verts->u0 ) continue ;
		if ( v->v0 != verts->v0 ) continue ;
		if ( v->q0 != verts->q0 ) continue ;
		if ( v->u1 != verts->u1 ) continue ;
		if ( v->v1 != verts->v1 ) continue ;
		if ( v->q1 != verts->q1 ) continue ;
		if ( v->u2 != verts->u2 ) continue ;
		if ( v->v2 != verts->v2 ) continue ;
		if ( v->q2 != verts->q2 ) continue ;
		if ( v->weight[0] != verts->weight[0] ) continue ;
		if ( v->weight[1] != verts->weight[1] ) continue ;
		if ( v->weight[2] != verts->weight[2] ) continue ;
		if ( v->weight[3] != verts->weight[3] ) continue ;
		if ( v->index[0] != verts->index[0] ) continue ;
		if ( v->index[1] != verts->index[1] ) continue ;
		if ( v->index[2] != verts->index[2] ) continue ;
		if ( v->index[3] != verts->index[3] ) continue ;
		return ( i );
	}
	*v = *verts ;
	MaxVertexBuffer++ ;
	return ( MaxVertexBuffer - 1 );
}
void ConvertCommonVertex( OLD_EVM_DEF *def )
{
	int			i, j, v_count ;
	EVM_PACK	*pack ;
	short		*verts, *norms, *uvs0, *uvs1, *uvs2 ;
	u_char		*weight ;
	u_short		*index ;
	EVX_VERTEX	*vertex, v ;

	MaxVertexBuffer = 0 ;
	CheckVertex = 0 ;
	vertex = VertexBuffer ;

	v_count = 0 ;
	pack = def->packet ;
	for ( i = def->n_packs ; i > 0 ; i--, pack++ ){
		verts = pack->verts ;
		norms = pack->norms ;
		uvs0 = pack->uvs[0] ;
		uvs1 = pack->uvs[1] ;
		uvs2 = pack->uvs[2] ;
		weight = pack->weight ;
		/* 各頂点を共有頂点バッファに登録する */
		for ( j = 0 ; j < pack->n_verts ; j++ ){
			memset( &v, 0, sizeof(v) );
			/* XBOX頂点ストリーム用に頂点を合成 */
			v.vx = verts[ 0 ] ;
			v.vy = verts[ 1 ] ;
			v.vz = verts[ 2 ] ;
			v.vw = 1 ;
			verts += 4 ;
			v.nx = norms[ 0 ] * 32767 / 4096 ;
			v.ny = norms[ 1 ] * 32767 / 4096 ;
			v.nz = norms[ 2 ] * 32767 / 4096 ;
			norms += 4 ;
			if ( uvs0 != NULL ){
				v.u0 = uvs0[ 0 ] * 32767 / 4096 ;
				v.v0 = uvs0[ 1 ] * 32767 / 4096 ;
				v.q0 = uvs0[ 2 ] * 32767 / 4096 ;
				uvs0 += 4 ;
			}
			if ( uvs1 != NULL ){
				v.u1 = uvs1[ 0 ] * 32767 / 4096 ;
				v.v1 = uvs1[ 1 ] * 32767 / 4096 ;
				v.q1 = uvs1[ 2 ] * 32767 / 4096 ;
				uvs1 += 4 ;
			}
			if ( uvs2 != NULL ){
				v.u2 = uvs2[ 0 ] * 32767 / 4096 ;
				v.v2 = uvs2[ 1 ] * 32767 / 4096 ;
				v.q2 = uvs2[ 2 ] * 32767 / 4096 ;
				uvs2 += 4 ;
			}
			//v.weight[ 0 ] = weight[ 0 ] * 32767 / 128 ;
			v.weight[ 1 ] = weight[ 1 ] * 32767 / 128 ;
			v.weight[ 2 ] = weight[ 2 ] * 32767 / 128 ;
			v.weight[ 3 ] = weight[ 3 ] * 32767 / 128 ;
			v.weight[ 0 ] = 32767 - v.weight[ 1 ] - v.weight[ 2 ] - v.weight[ 3 ] ;
			v.index[ 0 ] = weight[ 4 ] ;
			v.index[ 1 ] = weight[ 5 ] ;
			v.index[ 2 ] = weight[ 6 ] ;
			v.index[ 3 ] = weight[ 7 ] ;
			weight += 8 ;
			/* 頂点を共有頂点に登録しインデックスを記録 */
			IndexChangeTable[ v_count++ ] = AddVertex( &v );
		}
	}

	pack = def->packet ;
	for ( i = def->n_packs ; i > 0 ; i--, pack++ ){
		/* インデックスを共有頂点バッファ用に置き換える */
		index = pack->index ;
		for ( j = 0 ; j < pack->n_indices ; j++, index++ ){
			*index = IndexChangeTable[ *index ];
		}
	}
	
}

/* ================================================================ */
/*
	Main Routine
*/
int Convert_EvmVersionUp2( char *input_filename )
{
	char	output_filename[ 1024 ] ;
	EVX_DEF	*evm_def ;
	OLD_EVM_DEF	*old_evm_def ;
	FILE		*fp ;
	int			i, offset, size, s ;

	InitMatrixList();

	if ( ( old_evm_def = LoadOldEvm( input_filename ) ) == NULL ){
		return ( -1 );
	}

	if ( old_evm_def->type & DG_EVMTYPE_FORMAT_V4 ) return ( 0 );

	/* ウェイトをマトリクスインデックス方式へ変換 */
	OldEvmDataOptimize( old_evm_def );

	/* 共有頂点化 */
	ConvertCommonVertex( old_evm_def );
	printf("total vertex = %d (%d)\n", MaxVertexBuffer, CheckVertex );

	/* ヘッダー情報のコピー */
	evm_def = Malloc0( sizeof(EVX_DEF) + sizeof(EVX_SKEL) * old_evm_def->n_x_models );
	evm_def->data_format = 0 ;
	evm_def->n_models = old_evm_def->n_models ;
	evm_def->n_x_models = old_evm_def->n_x_models ;
	evm_def->lx = old_evm_def->lx ;
	evm_def->ly = old_evm_def->ly ;
	evm_def->lz = old_evm_def->lz ;
	evm_def->ux = old_evm_def->ux ;
	evm_def->uy = old_evm_def->uy ;
	evm_def->uz = old_evm_def->uz ;
	evm_def->type = old_evm_def->type ;
	evm_def->type |= DG_EVMTYPE_FORMAT_V4 ;
	evm_def->texture = old_evm_def->texture ;
	evm_def->n_packs = old_evm_def->n_packs ;
	evm_def->stride = sizeof(EVX_VERTEX) ;
	evm_def->n_verts = MaxVertexBuffer ;
	for ( i = 0 ; i < evm_def->n_x_models ; i++ ){
		evm_def->skeleton[ i ].flag = old_evm_def->skeleton[ i ].flag ;
		evm_def->skeleton[ i ].parent = old_evm_def->skeleton[ i ].parent ;
		evm_def->skeleton[ i ].tx = old_evm_def->skeleton[ i ].tx ;
		evm_def->skeleton[ i ].ty = old_evm_def->skeleton[ i ].ty ;
		evm_def->skeleton[ i ].tz = old_evm_def->skeleton[ i ].tz ;
		evm_def->skeleton[ i ].rt_tx = old_evm_def->skeleton[ i ].rt_tx ;
		evm_def->skeleton[ i ].rt_ty = old_evm_def->skeleton[ i ].rt_ty ;
		evm_def->skeleton[ i ].rt_tz = old_evm_def->skeleton[ i ].rt_tz ;
		evm_def->skeleton[ i ].pad0 = 0 ;
		evm_def->skeleton[ i ].pad1 = 0 ;
	}
	offset = sizeof(EVX_DEF) + sizeof(EVX_SKEL) * old_evm_def->n_x_models ;
	offset += sizeof(EVX_PACK) * evm_def->n_packs ;
	offset += sizeof(EVX_VERTEX) * MaxVertexBuffer ;
	evm_def->packet = (void*)( sizeof(EVX_DEF) + sizeof(EVX_SKEL) * old_evm_def->n_x_models );
	evm_def->vbuff = (void*)( sizeof(EVX_DEF) +
							  sizeof(EVX_SKEL) * old_evm_def->n_x_models +
							  sizeof(EVX_PACK) * evm_def->n_packs );

	/* スケルトンのバウンディング計算 */
	MakeSkeletonBound( old_evm_def, evm_def->skeleton );

	size = 0 ;

	if ( ( fp = fopen( input_filename, "wb" ) ) == NULL ) return ( -1 );

	/* ヘッダ書き込み */
	s = sizeof(EVX_DEF) ;
	fwrite( evm_def, s, 1, fp );
	size += s ;

	/* スケルトン情報書き込み */
	s = sizeof(EVX_SKEL) * evm_def->n_x_models ;
	fwrite( evm_def->skeleton, s, 1, fp );
	size += s ;

	/* パケット書き込み */
	for ( i = 0 ; i < evm_def->n_packs ; i++ ){
		EVM_PACK	*pack ;
		EVX_PACK	write_pack ;

		pack = &old_evm_def->packet[ i ] ;

		write_pack.flag = pack->flag ;
		write_pack.tex_id[0] = pack->tex_id[0] ;
		write_pack.tex_id[1] = pack->tex_id[1] ;
		write_pack.tex_id[2] = pack->tex_id[2] ;
		write_pack.n_verts = 0 ;
		write_pack.n_indices = pack->n_indices ;
		write_pack.n_mats = pack->n_mats ;
		write_pack.mat_id[0] = pack->mat_id[0] ;
		write_pack.mat_id[1] = pack->mat_id[1] ;
		write_pack.mat_id[2] = pack->mat_id[2] ;
		write_pack.mat_id[3] = pack->mat_id[3] ;
		write_pack.mat_id[4] = pack->mat_id[4] ;
		write_pack.mat_id[5] = pack->mat_id[5] ;
		write_pack.mat_id[6] = pack->mat_id[6] ;
		write_pack.mat_id[7] = pack->mat_id[7] ;
		write_pack.verts = NULL ;
		write_pack.norms = NULL ;
		write_pack.uvs[0] = NULL ;
		write_pack.uvs[1] = NULL ;
		write_pack.uvs[2] = NULL ;
		write_pack.weight = NULL ;
		write_pack.rgbs = NULL ;
		write_pack.index = offset ;
		offset += ( sizeof(short) * pack->n_indices + 15 ) & ~15 ;
		fwrite( &write_pack, sizeof(EVX_PACK), 1, fp );
	}

	/* 頂点データ書き込み */
	s = ( sizeof(EVX_VERTEX) * MaxVertexBuffer + 15 ) & ~15 ;
	fwrite( VertexBuffer, s, 1, fp );

	/* インデックスデータ書き込み */
	for ( i = 0 ; i < evm_def->n_packs ; i++ ){
		EVM_PACK	*pack ;
		pack = &old_evm_def->packet[ i ] ;
		s = ( sizeof(short) * pack->n_indices + 15 ) & ~15 ;
		fwrite( pack->index, s, 1, fp );
	}

	fclose( fp );

	free( evm_def );
	free( old_evm_def );

	return ( 0 );
}


