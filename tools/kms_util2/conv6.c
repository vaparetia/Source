/*
	conv6.c
	KMSのストリップ最適化

	2001/02/16 K.Takabe
	$Id: conv6.c,v 1.2 2002/03/28 04:38:40 usr03700 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "fmt_kms.h"
#include "prog_utl.h"
#include "poly_utl.h"

/* ================================================================ */
/*
	プログラムで使用する構造体宣言
*/


#define MAX_PACKET	(1024*16)
/* プログラム使用グローバルワーク */
typedef struct _program_work {
	POLY_PACKET		poly_stack ;				/* パケット化していないポリゴンの集合 */
	int				n_packet_list ;				/* パケット化の終了した数 */
	POLY_PACKET		packet_list[ MAX_PACKET ] ;	/* パケット化の終了したパケット */
	int				n_tmp_packets ;			/* パケット化の終了した数 */
	POLY_PACKET		tmp_packets[ MAX_PACKET ] ;	/* パケット化の終了したパケット */
} PROGRAM_WORK ;

PROGRAM_WORK	*work ;


/* ================================================================ */
/*
	プログラムワークの初期化
 */

/* グローバルワークの初期化 */
static void InitProgramWork( void )
{
	int		i ;
	POLYGON	*poly ;
	work = Malloc0( sizeof(PROGRAM_WORK) );
	InitPolygonManagement();

	InitPolygonPacket( &work->poly_stack );
	work->n_packet_list = 0 ;
	for ( i = 0 ; i < MAX_PACKET ; i++ ){
		InitPolygonPacket( &work->packet_list[i] );
	}
	/* テンポラリパケットの初期化 */
	work->n_tmp_packets = 0 ;
	for ( i = 0 ; i < MAX_PACKET ; i++ ){
		InitPolygonPacket( &work->tmp_packets[i] );
	}
	

}
/* グローバルワークの再初期化 */
static void ResetProgramWork( void )
{
	int		i ;
	POLYGON	*poly ;

	FreePolygonPacket( &work->poly_stack );
	work->n_packet_list = 0 ;
	for ( i = 0 ; i < MAX_PACKET ; i++ ){
		FreePolygonPacket( &work->packet_list[i] );
	}
	/* テンポラリパケットの初期化 */
	work->n_tmp_packets = 0 ;
	for ( i = 0 ; i < MAX_PACKET ; i++ ){
		FreePolygonPacket( &work->tmp_packets[i] );
	}
	

}
/* グローバルワークの解放 */
static void EndProgramWork( void )
{
	EndPolygonManagement();
	Free( work );
}

/* ================================================================ */
/*
	Open Kms file
 */
static KMS_DEF *OpenKms( char *filename )
{
	FILE	*fp ;
	int		size, i, j ;
	KMS_DEF	*def ;
	KMS_MDL	*mdl ;
	KMS_MDLPACK	*pack ;

	if ( ( fp = fopen( filename, "rb" ) ) == NULL ){
		printf("kms file open error!(%s)\n", filename );
		return ( NULL );
	}

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	def = Malloc0( size );
	fread( def, 1, size, fp );
	fclose( fp );

	if ( def->n_x_models == 0 ) def->n_x_models = def->n_models ;

	/* kms setup */
	mdl = def->models ;
	for ( i = def->n_x_models ; i > 0 ; i--, mdl++ ){
		mdl->packs = (KMS_MDLPACK*)( (int)mdl->packs + (int)def );
		pack = mdl->packs ;
		for ( j = mdl->n_packs ; j > 0 ; j--, pack++ ){
			if ( pack->verts != NULL ) pack->verts = (short*)( (int)pack->verts + (int)def );
			if ( pack->norms != NULL ) pack->norms = (short*)( (int)pack->norms + (int)def );
			if ( pack->uvs[0] != NULL ) pack->uvs[0] = (short*)( (int)pack->uvs[0] + (int)def );
			if ( pack->uvs[1] != NULL ) pack->uvs[1] = (short*)( (int)pack->uvs[1] + (int)def );
			if ( pack->uvs[2] != NULL ) pack->uvs[2] = (short*)( (int)pack->uvs[2] + (int)def );
			//if ( pack->rgbs != NULL ) pack->rgbs = (short*)( (int)pack->rgbs + (int)def );	/* 未使用 */
		}
	}

	return ( def ) ;
}

/* ================================================================ */
/*
	ポリゴン情報の生成（ＫＭＳからのコンバート処理）
*/
/* KMSファイルからポリゴン情報を復元する */
static void CreatePolygonData( KMS_DEF *kms_def, int target_obj_num )
{
	static FVECTOR		zero_vector = {0.0f, 0.0f, 0.0f, 0.0f};
	static WEIGHT		zero_weight = { 0, 0.0f };
	static MATERIAL		null_material = { 0 };
	KMS_MDL		*mdl ;
	KMS_MDLPACK	*pack ;
	int			i, j, k, use_weight, use_material = 1, joint, parent, pack_flag, n_mat ;
	short		*verts, *norms, *uvs[3] ;
	VERTEX		vertex[3], *v ;
	MATERIAL	mat[3] ;

	mdl = kms_def->models ;
	for ( i = 0 ; i < kms_def->n_x_models ; i++, mdl++ ){
		if ( i != target_obj_num ) continue ;
		pack = mdl->packs ;
		if ( mdl->type & DG_TYPE_EXTEND ){/* kms独自の付随モデルフラグ */
			joint = mdl->parent ;
			parent = kms_def->models[ joint ].parent ;
		} else {
			joint = i ;
			parent = mdl->parent ;
		}
		for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
			verts = pack->verts ;
			norms = pack->norms ;
			uvs[0] = pack->uvs[0] ;
			uvs[1] = pack->uvs[1] ;
			uvs[2] = pack->uvs[2] ;
			n_mat = 0 ;
			mat[0].tex_id = pack->tex_id[0] ; if ( mat[0].tex_id != 0 && mat[0].tex_id != 1 ) n_mat++ ;
			mat[1].tex_id = pack->tex_id[1] ; if ( mat[1].tex_id != 0 && mat[1].tex_id != 1 ) n_mat++ ;
			mat[2].tex_id = pack->tex_id[2] ; if ( mat[2].tex_id != 0 && mat[2].tex_id != 1 ) n_mat++ ;
			pack_flag = pack->flag ;
			for ( k = 0 ; k < pack->n_verts ; k++ ){
				/* 頂点キック処理 */
				vertex[0] = vertex[1] ;
				vertex[1] = vertex[2] ;
				v = &vertex[2] ;
				InitVertex( v );
				v->v.vx = (float)verts[0] ;
				v->v.vy = (float)verts[1] ;
				v->v.vz = (float)verts[2] ;
				v->n.vx = (float)norms[0] / 4096.0f ;
				v->n.vy = (float)norms[1] / 4096.0f ;
				v->n.vz = (float)norms[2] / 4096.0f ;
				if ( uvs[0] != NULL ){
					v->uv[0].vx = (float)uvs[0][0] / 4096.0f ;
					v->uv[0].vy = (float)uvs[0][1] / 4096.0f ;
				} else {
					v->uv[0].vx = (float)0.0f ;
					v->uv[0].vy = (float)0.0f ;
				}
				if ( uvs[1] != NULL ){
					v->uv[1].vx = (float)uvs[1][0] / 4096.0f ;
					v->uv[1].vy = (float)uvs[1][1] / 4096.0f ;
				} else {
					v->uv[1].vx = (float)0.0f ;
					v->uv[1].vy = (float)0.0f ;
				}
				if ( uvs[2] != NULL ){
					v->uv[2].vx = (float)uvs[2][0] / 4096.0f ;
					v->uv[2].vy = (float)uvs[2][1] / 4096.0f ;
				} else {
					v->uv[2].vx = (float)0.0f ;
					v->uv[2].vy = (float)0.0f ;
				}
				if ( pack_flag & DG_PACKFLAG_ENVELOPE ){
					/* シングルウェイトエンベロープ付き */
					v->weight.weight[0].skel_num = parent ;
					v->weight.weight[0].weight = 1.0f - (float)verts[3] / 4096.0f ;
					v->weight.weight[1].skel_num = joint ;
					v->weight.weight[1].weight = (float)verts[3] / 4096.0f ;
					v->weight.max_use = 2 ;
				} else {
					/* エンベロープなし */
					v->weight.weight[0].skel_num = joint ;
					v->weight.weight[0].weight = 1.0f ;
					v->weight.max_use = 1 ;
				}

				/* 描画キック判断 */
				if ( !( norms[3] & 0x8000 ) ){
					/* ポリゴン生成処理 */
					POLYGON	*poly ;
					poly = AllocPolygon() ;
					poly->flag = 0 ;
					poly->n_material = n_mat ;
					poly->vertex[0] = vertex[0] ;
					poly->vertex[1] = vertex[1] ;
					poly->vertex[2] = vertex[2] ;
					poly->material[0] = mat[0] ;
					poly->material[1] = mat[1] ;
					poly->material[2] = mat[2] ;
					AddPolygon( &work->poly_stack, poly );
				}
				verts += 4 ;
				norms += 4 ;
				if ( uvs[0] != NULL ) uvs[0] += 2 ;
				if ( uvs[1] != NULL ) uvs[1] += 2 ;
				if ( uvs[2] != NULL ) uvs[2] += 2 ;
			}
		}
	}
}

/* ================================================================ */
/*
	パケットを生成する
*/
/* ポリゴンの使用するスケルトン数を取得する */
static int GetPolygonSkeletonWeight( POLYGON *poly )
{
	POLY_PACKET	packet ;
	POLYGON		tmp_poly ;
	InitPolygonPacket( &packet );
	CopyPolygon( &tmp_poly, poly );
	AddPolygon( &packet, &tmp_poly );
	return ( GetUseSkeletonWeight( &packet ) );
}
/* テクスチャによる分類を行なう */
static int CutPacket_of_TopTexture( POLY_PACKET *new_packet, POLY_PACKET *org_packet )
{
	POLYGON		*poly, *top_poly, *tmp_poly ;
	int			i ;

	InitPolygonPacket( new_packet );
	top_poly = poly = GetFirstPolygon( org_packet );
	while ( poly != NULL ){
		if ( top_poly->n_material == poly->n_material ){
			for ( i = 0 ; i < top_poly->n_material ; i++ ){
				if ( top_poly->material[i].tex_id != poly->material[i].tex_id ){
					poly = GetNextPolygon( poly );
					break ;
				}
			}
			if ( i == top_poly->n_material ){
				//if ( GetPolygonSkeletonWeight( top_poly ) != GetPolygonSkeletonWeight( poly ) ){
				//	poly = GetNextPolygon( poly );
				//	continue ;
				//}
				/* ステージ変換スピードチェック実験用（必要ない） */
				//if ( top_poly->vertex[0].weight.weight[0].skel_num != poly->vertex[0].weight.weight[0].skel_num ){
				//	poly = GetNextPolygon( poly );
				//	continue ;
				//}
				/* マテリアルが同一であった場合 */
				tmp_poly = poly ;
				poly = GetNextPolygon( poly );			/* 次のポリゴンを取得しておく */
				DeletePolygon( org_packet, tmp_poly );	/* 元のパケットから削除 */
				AddPolygon( new_packet, tmp_poly );		/* 新しいパケットへ追加 */
			}
		} else {
			poly = GetNextPolygon( poly );
		}
	}
	return GetTotalPolygons( new_packet );
}
/* 一番長いストリップを切り出す */
static int	CutMostLongStripPacket( POLY_PACKET *new_packet, POLY_PACKET *org_packet, int stop_n_strip )
{
	int		total_packet ;
	POLY_PACKET		tmp_packet ;		/* ストリップポリゴン検索用テンポラリ */
	POLYGON			*start_poly ;		/* ストリップ開始用ポリゴン */
	POLYGON			*poly ;
	POLYGON			*tmp_poly ;			/* オリジナルデータ一時記憶用 */
	int				max_strip ;			/* 検出した最大ストリップ長 */
	int				select_type ;		/* 最大ストリップ長を検出したときの開始パターン */
	POLYGON			*select_poly ;		/* 最大ストリップ長を検出したときのポリゴン情報 */
	POLY_PACKET		strip_packet ;		/* 構築中のストリップポリゴンパケット */
	int				i ;

	max_strip = 0 ;
	select_type = 0 ;
	select_poly = NULL ;

	//Strip_InitPolygonPacketLink( org_packet );

	/* 最大長のストリップを検索する */
	InitPolygonPacket( new_packet );
	start_poly = GetFirstPolygon( org_packet );
	while ( start_poly != NULL ){
		for ( i = 0 ; i < 6 ; i++ ){
			int		n ;
			Strip_InitPolygonPacketFlag( org_packet );
			n = Strip_GetStripPolygons( org_packet, start_poly, i );
			if ( n > max_strip ){
				/* 最大数を更新した場合には記録しておく */
				max_strip = n ;
				select_type = i ;
				select_poly = start_poly ;
				if ( max_strip == stop_n_strip ) break ;
			}
		}
		if ( max_strip == stop_n_strip ) break ;
		start_poly = GetNextPolygon( start_poly );
	}
	//printf("(...%d )", max_strip);

	/* 正式にストリップを構築する */
	Strip_CutStripPolygons( new_packet, org_packet, select_poly, select_type );
	Strip_OptimizeStripPacket( new_packet, select_type );

	return ( max_strip );
}
/* スケルトン数が８以内になるように分割する */
static void SeparateStripPacket( POLY_PACKET *new_packet, POLY_PACKET *packet )
{
	POLY_PACKET	tmp_packet ;
	POLYGON		*poly, *tmp_poly ;

	InitPolygonPacket( new_packet );
	poly = GetFirstPolygon( packet );
	do {
		tmp_poly = AllocPolygon() ;
		CopyPolygon( tmp_poly, poly );
		/* チェック用のダミーパケットを作成 */
		DuplicatePolygonPacket( &tmp_packet, new_packet );
		AddPolygon( &tmp_packet, tmp_poly );
		if ( GetUseSkeletonWeight( &tmp_packet ) > 8 ){
			/* 使用スケルトン数が８を超えてしまった場合には繋げない */
			FreePolygonPacket( &tmp_packet );
			poly = GetNextPolygon( poly );
			/* ストリップを崩してしまうと不具合がでるので続けずに終了 */
			break ;
		} else {
			FreePolygonPacket( &tmp_packet );
			tmp_poly = GetNextPolygon( poly );
			DeletePolygon( packet, poly );
			AddPolygon( new_packet, poly );
			poly = tmp_poly ;
		}
	} while ( poly != NULL );
}

/* ================================================================ */
/*
	書き出しデータ用に整理し、まとめる
*/
static void MakeKMSDataPacket( void )
{
	int		i, max ;
	int		use_skeleton ;
	POLY_PACKET	check_packet, new_packet, *trg_packet ;

	InitPolygonPacket( &check_packet );
	max = work->n_tmp_packets ;
	/* 最初は使用スケルトン数が１の物からまとめていく */
	use_skeleton = 1 ;
	while ( max  > 0 ){

		/* 新たなパケットを生成 */
		InitPolygonPacket( &new_packet );
		for ( i = 0 ; i < work->n_tmp_packets ; i++ ){
			trg_packet = &work->tmp_packets[i] ;
			/* 既にポリゴンのないパケットならスキップ */
			if ( GetTotalPolygons( trg_packet ) == 0 ) continue ;
			/* 現在まとめようとしている数以上にスケルトンを使用している場合もスキップ */
			if ( GetUseSkeletonWeight( trg_packet ) != use_skeleton ) continue ;

			/* 試しにパケットを接続してみる */
			DuplicatePolygonPacket( &check_packet, &new_packet );
			AddPolygonPacket( &check_packet, trg_packet );

			/* 使用スケルトン数が増えてしまった場合には結合をあきらめる */
			if ( GetUseSkeletonWeight( &check_packet ) > use_skeleton ){
				FreePolygonPacket( &check_packet );
				continue ;
			}
			/* もう必要ないので開放 */
			FreePolygonPacket( &check_packet );
			/* このパケットをまとめてしまう */
			max-- ;
			MovePolygonPacket( &new_packet, trg_packet );
		}

		/* 新しいパケットが生成できない場合には使用スケルトン数を増やしてチャレンジ */
		if ( GetTotalPolygons( &new_packet ) == 0 ){
			use_skeleton++ ;
			continue ;
		} else {
			POLYGON	*poly, *prev ;
			int		points ;

			OptimizePolygonPacket( &new_packet );

			/* ３２頂点以内になるように調整して登録 */
			poly = GetFirstPolygon( &new_packet );
			DeletePolygon( &new_packet, poly );
			AddPolygon( &work->packet_list[ work->n_packet_list ], poly );
			points = 3 ;
			prev = poly ;
			while ( ( poly = GetFirstPolygon( &new_packet ) ) != NULL ){
				/* 最大頂点チェック */
				if ( points > (29+32) ){
					/* パケットを一度切り離す */
					//printf("packet %d points\n", points );
					OptimizePolygonPacket( &work->packet_list[ work->n_packet_list ] );
					work->n_packet_list++ ;
					points = 3 ;
				} else {
					/* 頂点共有チェック */
					if ( IsEqualVertex( &prev->vertex[1], &poly->vertex[0] ) &&
						IsEqualVertex( &prev->vertex[2], &poly->vertex[1] ) ){
						/* ２点共有ストリップ */
						points += 1 ;
					} else {
						/* 独立３角形 */
						points += 3 ;
					}
				}
				DeletePolygon( &new_packet, poly );
				AddPolygon( &work->packet_list[ work->n_packet_list ], poly );
				prev = poly ;
			}
			//printf("packet %d points\n", points );
			OptimizePolygonPacket( &work->packet_list[ work->n_packet_list ] );
			work->n_packet_list++ ;
#if 0
			/* 情報表示 */
			printf(" packed ... max = %d use_skeleton = %d \n",
				   GetTotalPolygons( &new_packet ),
				   GetUseSkeletonWeight( &new_packet ) );
			FreePolygonPacket( &new_packet );
#endif
		}

	}
}

/* ================================================================ */
/*
	ＥＶＭファイル書き出し（かなりいい加減）
 */
static void WriteVerts( KMS_MDLPACK *packs, int num, VERTEX *v, int flag )
{
	int		i, w ;
	((SVECTOR*)packs->verts)[num].vx = v->v.vx * 16 ;
	((SVECTOR*)packs->verts)[num].vy = v->v.vy * 16 ;
	((SVECTOR*)packs->verts)[num].vz = v->v.vz * 16 ;
	if ( v->weight.max_use == 1 ){
		((SVECTOR*)packs->verts)[num].vw = v->weight.weight[0].weight * 4096 ;
	} else {
		((SVECTOR*)packs->verts)[num].vw = v->weight.weight[1].weight * 4096 ;
	}
	((SVECTOR*)packs->norms)[num].vx = v->n.vx * 4096 ;
	((SVECTOR*)packs->norms)[num].vy = v->n.vy * 4096 ;
	((SVECTOR*)packs->norms)[num].vz = v->n.vz * 4096 ;
	((SVECTOR*)packs->norms)[num].vw = flag ;
	((STVECTOR*)packs->uvs[0])[num].vx = v->uv[0].vx * 4096 ;
	((STVECTOR*)packs->uvs[0])[num].vy = v->uv[0].vy * 4096 ;
	((STVECTOR*)packs->uvs[1])[num].vx = v->uv[1].vx * 4096 ;
	((STVECTOR*)packs->uvs[1])[num].vy = v->uv[1].vy * 4096 ;
	((STVECTOR*)packs->uvs[2])[num].vx = v->uv[2].vx * 4096 ;
	((STVECTOR*)packs->uvs[2])[num].vy = v->uv[2].vy * 4096 ;
}
static int WriteKmsFile( char *filename, KMS_DEF *def, int n_obj )
{
	FILE		*fp ;
	KMS_MDLPACK	*packs ;
	void		*verts_buffer, *write_buffer ;
	int			i, j, verts_offset, write_offset, total_verts = 0 ;

	packs = Malloc0( sizeof(KMS_MDLPACK) * work->n_packet_list );
	verts_buffer = Malloc0( ( 8 * 7 * 32) *  work->n_packet_list );
	verts_offset = 0 ;

	def->models[n_obj].n_packs = work->n_packet_list ;
	def->models[n_obj].packs = packs ;

	/* 書き出しデータの生成処理 */
	/* パケットデータの生成 */
	for ( i = 0 ; i < work->n_packet_list ; i++ ){
		POLYGON	*poly, *prev ;

		poly = GetFirstPolygon( &work->packet_list[ i ] );

		packs[ i ].flag = 0 ;
		if ( poly->material[0].tex_id != 0 ) packs[ i ].flag |= DG_PACKFLAG_TEX0|DG_PACKFLAG_UV0 ;
		if ( poly->material[1].tex_id != 0 ) packs[ i ].flag |= DG_PACKFLAG_TEX1|DG_PACKFLAG_UV1 ;
		if ( poly->material[2].tex_id != 0 ) packs[ i ].flag |= DG_PACKFLAG_TEX2|DG_PACKFLAG_UV2 ;
		packs[ i ].n_verts = 0 ;
		packs[ i ].tex_id[0] = poly->material[0].tex_id ;
		packs[ i ].tex_id[1] = poly->material[1].tex_id ;
		packs[ i ].tex_id[2] = poly->material[2].tex_id ;
		packs[ i ].verts = (void*)( verts_buffer + verts_offset ) ; verts_offset += sizeof(SVECTOR) * 32 ;
		packs[ i ].norms = (void*)( verts_buffer + verts_offset ) ; verts_offset += sizeof(SVECTOR) * 32 ;
		packs[ i ].uvs[0] = (void*)( verts_buffer + verts_offset ) ; verts_offset += sizeof(STVECTOR) * 32 ;
		packs[ i ].uvs[1] = (void*)( verts_buffer + verts_offset ) ; verts_offset += sizeof(STVECTOR) * 32 ;
		packs[ i ].uvs[2] = (void*)( verts_buffer + verts_offset ) ; verts_offset += sizeof(STVECTOR) * 32 ;

		WriteVerts( &packs[ i ], 0, &poly->vertex[ 0 ], 0x8fff );
		WriteVerts( &packs[ i ], 1, &poly->vertex[ 1 ], 0x8fff );
		WriteVerts( &packs[ i ], 2, &poly->vertex[ 2 ], 0x0fff );
		packs[ i ].n_verts += 3 ;
		prev = poly ;
		while ( ( poly = GetNextPolygon( prev ) ) != NULL ){
			if ( IsEqualVertex( &prev->vertex[1], &poly->vertex[0] ) &&
				IsEqualVertex( &prev->vertex[2], &poly->vertex[1] ) ){
				/* ２点共有ストリップ */
				j = 2 ;
			} else {
				/* 独立３角形 */
				j = 0 ;
			}
			for ( ; j < 3 ; j++ ){
				WriteVerts( &packs[ i ], packs[i].n_verts, &poly->vertex[ j ], ( j != 2 ) ? 0x8fff : 0x0fff );
				packs[i].n_verts++ ;
			}
			prev = poly ;
		}
		total_verts += packs[i].n_verts ;
		printf("%d = %d verts (%d)  \n", i, packs[i].n_verts, total_verts );

		FreePolygonPacket( &work->packet_list[ i ] );
	}
	printf("%d:total_verts %d  \n", n_obj, total_verts );

#if 0
	write_buffer = Malloc0( 1024*1024*4 );
	write_offset = 0 ;
	{
		EVM_DEF		*write_def ;
		EVM_PACK	*write_pack ;
		int			size ;

		write_def = write_buffer + write_offset ;	/* 書き出し用EVM_DEFアドレス取得 */
		size = sizeof(EVM_DEF) + sizeof(EVM_SKEL) * def->n_models ;
		WriteData( write_buffer, write_offset, def, size ); write_offset += size ;

		write_pack = write_buffer + write_offset ;	/* 書き出し用EVM_PACKアドレス取得 */
		write_def->packet = (void*)write_offset ;	/* EVM_PACKへのオフセット記録 */
		size = sizeof(EVM_PACK) * def->n_packs ;
		WriteData( write_buffer, write_offset, packs, size ); write_offset += size ;

		/* 頂点データの書き出し */
		for ( i = 0 ; i < def->n_packs ; i++ ){
			write_pack[i].verts = (void*)write_offset ;	/* 頂点へのオフセット記録 */
			size = AlignSize16( sizeof(SVECTOR), write_pack[i].n_verts );
			WriteData( write_buffer, write_offset, packs[i].verts, size ); write_offset += size ;
		}
		/* 法線データの書き出し */
		for ( i = 0 ; i < def->n_packs ; i++ ){
			write_pack[i].norms = (void*)write_offset ;	/* 法線へのオフセット記録 */
			size = AlignSize16( sizeof(SVECTOR), write_pack[i].n_verts );
			WriteData( write_buffer, write_offset, packs[i].norms, size ); write_offset += size ;
		}
		/* ＵＶ０の書き出し */
		for ( i = 0 ; i < def->n_packs ; i++ ){
			write_pack[i].uvs0 = (void*)write_offset ;	/* ＵＶ０へのオフセット記録 */
			size = AlignSize16( sizeof(SVECTOR), write_pack[i].n_verts );
			WriteData( write_buffer, write_offset, packs[i].uvs0, size ); write_offset += size ;
		}
		/* ＵＶ１の書き出し */
		for ( i = 0 ; i < def->n_packs ; i++ ){
			write_pack[i].uvs1 = (void*)write_offset ;	/* ＵＶ２へのオフセット記録 */
			size = AlignSize16( sizeof(SVECTOR), write_pack[i].n_verts );
			WriteData( write_buffer, write_offset, packs[i].uvs1, size ); write_offset += size ;
		}
		/* ＵＶ２の書き出し */
		for ( i = 0 ; i < def->n_packs ; i++ ){
			write_pack[i].uvs2 = (void*)write_offset ;	/* ＵＶ２へのオフセット記録 */
			size = AlignSize16( sizeof(SVECTOR), write_pack[i].n_verts );
			WriteData( write_buffer, write_offset, packs[i].uvs2, size ); write_offset += size ;
		}
		/* ウェイト値の書き出し */
		for ( i = 0 ; i < def->n_packs ; i++ ){
			write_pack[i].weight = (void*)write_offset ;/* ウェイト値へのオフセット記録 */
			size = AlignSize16( sizeof(unsigned char)*8, write_pack[i].n_verts );
			WriteData( write_buffer, write_offset, packs[i].weight, size ); write_offset += size ;
		}
	}


	if ( ( fp = fopen( filename, "wb" ) ) == NULL ){
		fprintf( stderr, "file write error!!(%s)\n", filename );
		return ( -1 );
	}

	fwrite( write_buffer, 1, write_offset, fp );
	fclose( fp );

	Free( write_buffer );
	Free( packs );
	Free( verts_buffer );

#endif

}

/* ================================================================ */
/*
	Main Routine
*/
int Convert_OptimizeKms2( char *input_filename, char *output_filename1 )
{
	char	output_filename[ 1024 ] ;
	KMS_DEF	*kms_def ;
	KMS_DEF	*new_kms_def ;
	POLY_PACKET	tmp_packet ;
	int		i, n, total_vertex = 0, size, j ;
	int		n_strip, max_strip ;

	if ( ( kms_def = OpenKms( input_filename ) ) == NULL ){
		return ( -1 );
	}
	if ( DG_GetMdlFormat( kms_def ) != MGS_MODEL_NORM && DG_GetMdlFormat( kms_def ) != MGS_MODEL_MULTITEX ){
		free( kms_def );
		return ( -1 );
	}
	MakeFileName( output_filename, input_filename,  input_filename, "kms2" );

	InitProgramWork();

	size = sizeof(KMS_DEF) + sizeof(KMS_MDL) * kms_def->n_x_models ;
	new_kms_def = Malloc0( size );
	*new_kms_def = *kms_def ;
	for ( i = 0 ; i < kms_def->n_x_models ; i++ ){
		new_kms_def->models[ i ] = kms_def->models[ i ];
	}

	for ( j = 0 ; j < kms_def->n_x_models ; j++ ){
		printf("=== start obj %d ===\n", j );

		/* ポリゴン情報の生成 */
		CreatePolygonData( kms_def, j );

		/* 共有頂点ＩＤ初期化 */
		IndexedVertexInitialize();

		DisplayPolygonManagementStatus();
		printf("use_skeletons: %d\n", GetUseSkeletonWeight( &work->poly_stack ) );

		/* マテリアル単位で切り分けてポリゴンを処理していく */
		while ( n = CutPacket_of_TopTexture( &tmp_packet, &work->poly_stack ) ){
			/* 共有頂点ＩＤを割り当てる */
			IndexedVertex( &tmp_packet );

#if 1
			/* 情報表示 */
			printf(" tex_id = %8d,%8d,%8d  polygon = %4d use_skeleton = %2d \n",
				   GetFirstPolygon( &tmp_packet )->material[0].tex_id ,
				   GetFirstPolygon( &tmp_packet )->material[1].tex_id ,
				   GetFirstPolygon( &tmp_packet )->material[2].tex_id ,
				   n,
				   GetUseSkeletonWeight( &tmp_packet ) );
#endif

			/* ストリップ化処理を行なう */
			work->n_tmp_packets = 0 ;

			/* リンク情報を初期化（以後このパケットは直接触ってはいけない） */
			Strip_InitPolygonPacketLink( &tmp_packet );		

			max_strip = GetTotalPolygons( &tmp_packet ) ;
			while ( GetTotalPolygons( &tmp_packet ) > 0 ){
				POLY_PACKET	local_packet ;

				/* 最長のストリップを生成する */
				n_strip = CutMostLongStripPacket( &local_packet, &tmp_packet, max_strip );
				if ( n_strip < max_strip ) max_strip = n_strip ;
				if ( GetTotalPolygons( &tmp_packet ) < 0 ){
					printf("!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				}
				//printf("strip %d  \n", n_strip );
#if 0
				/* 情報表示 */
				printf(" max = %d use_skeleton = %d (last %d)%c",
					   GetTotalPolygons( &local_packet ),
					   GetUseSkeletonWeight( &local_packet ),
					   GetTotalPolygons( &tmp_packet ),
					   0xd );
				fflush( stdout );
#endif
#if 1
				/* スケルトン数が多すぎる場合には切断を行なう */
				while ( GetTotalPolygons( &local_packet ) != 0 ){
					POLY_PACKET		tmptmp_packet ;
					SeparateStripPacket( &tmptmp_packet, &local_packet );
					total_vertex += GetTotalPolygons( &tmptmp_packet ) + 2 ;
					/* 一時的に別領域に移動して保管する */
					MovePolygonPacket( &work->tmp_packets[work->n_tmp_packets++], &tmptmp_packet );
				}
#else
				total_vertex += GetTotalPolygons( &local_packet ) + 2 ;

				/* 一時的に別領域に移動して保管する */
				MovePolygonPacket( &work->tmp_packets[work->n_tmp_packets++], &local_packet );
#endif
				/* ポリゴンの開放（実際には既に空っぽになっているはず） */
				FreePolygonPacket( &local_packet );
			}

			/* スケルトンの重みなどをチェックして３２頂点以内になるように整理する */
			MakeKMSDataPacket();

			/* ポリゴンの開放（実際には既に空っぽになっているはず） */
			FreePolygonPacket( &tmp_packet );

		}

		WriteKmsFile( output_filename, new_kms_def, j );

		DisplayPolygonManagementStatus();
		ResetProgramWork();
	}

	printf("total_vertex: %d                          \n", total_vertex );

	/* ファイルへの書き出し */
	//WriteEVMFile( output_filename, evm_def );

	Free( new_kms_def );
	free( kms_def );
	EndProgramWork();
}



