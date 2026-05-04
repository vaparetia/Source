/*
	conv3.c
	マルチウェイトエンベロープモデル変換(kms=>evm)

	2000/01/22 K.Takabe
	$Id: conv3.c,v 1.8 2000/08/01 05:39:17 usr02774 Exp $
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
	KMS_OBJPACK	*pack ;

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
		mdl->packs = (KMS_OBJPACK*)( (int)mdl->packs + (int)def );
		pack = mdl->packs ;
		for ( j = mdl->n_packs ; j > 0 ; j--, pack++ ){
			pack->verts = (short*)( (int)pack->verts + (int)def );
			pack->norms = (short*)( (int)pack->norms + (int)def );
			pack->uvs = (short*)( (int)pack->uvs + (int)def );
			//pack->rgbs = (short*)( (int)pack->rgbs + (int)def );	/* 未使用なので */
		}
	}

	return ( def ) ;
}
/* ================================================================ */
/*
	スケルトン情報の生成
*/
static MakeSkeleton( KMS_DEF *kms_def, EVM_DEF *evm_def )
{
	int		i ;
	KMS_MDL	*kms_mdl ;

	/* ヘッダーの複製 */
	evm_def->n_models = kms_def->n_models ;
	evm_def->n_x_models = kms_def->n_x_models ;
	evm_def->lx = kms_def->lx ;
	evm_def->ly = kms_def->ly ;
	evm_def->lz = kms_def->lz ;
	evm_def->ux = kms_def->ux ;
	evm_def->uy = kms_def->uy ;
	evm_def->uz = kms_def->uz ;
	evm_def->type = 0 ;
	evm_def->texture = kms_def->models[0].pad2 ;
	evm_def->n_packs = 0 ;
	/* スケルトン情報の構築 */
	for ( i = 0 ; i < kms_def->n_models ; i++ ){
		kms_mdl = &kms_def->models[ i ] ;
		evm_def->skeleton[ i ].rt_tx = evm_def->skeleton[ i ].tx = kms_mdl->tx ;
		evm_def->skeleton[ i ].rt_ty = evm_def->skeleton[ i ].ty = kms_mdl->ty ;
		evm_def->skeleton[ i ].rt_tz = evm_def->skeleton[ i ].tz = kms_mdl->tz ;
		while ( kms_mdl->parent != -1 ){
			kms_mdl = &kms_def->models[ kms_mdl->parent ] ;
			evm_def->skeleton[ i ].rt_tx += kms_mdl->tx ;
			evm_def->skeleton[ i ].rt_ty += kms_mdl->ty ;
			evm_def->skeleton[ i ].rt_tz += kms_mdl->tz ;
		}
#if 0
		printf("%d offset %f %f %f (%f %f %f)\n",
			   i,
			   evm_def->skeleton[ i ].rt_tx,
			   evm_def->skeleton[ i ].rt_ty,
			   evm_def->skeleton[ i ].rt_tz,
			   evm_def->skeleton[ i ].tx,
			   evm_def->skeleton[ i ].ty,
			   evm_def->skeleton[ i ].tz
			   );
#endif
	}
}

/* ================================================================ */
/*
	ポリゴン情報の生成（ＫＭＳからのコンバート処理）
*/
/* KMSファイルからポリゴン情報を復元する */
void CreatePolygonData( KMS_DEF *kms_def, EVM_DEF *evm_def )
{
	static FVECTOR		zero_vector = {0.0f, 0.0f, 0.0f, 0.0f};
	static WEIGHT		zero_weight = { 0, 0.0f };
	static MATERIAL		null_material = { 0 };
	KMS_MDL		*mdl ;
	KMS_OBJPACK	*pack ;
	int			i, j, k, use_weight, use_material = 1, joint, parent ;
	short		*verts, *norms, *uvs ;
	VERTEX		vertex[3], *v ;
	MATERIAL	mat ;

	mdl = kms_def->models ;
	for ( i = 0 ; i < kms_def->n_x_models ; i++, mdl++ ){
		pack = mdl->packs ;
		if ( mdl->type & 0x4000 ){/* kms独自の付随モデルフラグ */
			joint = mdl->parent ;
			parent = kms_def->models[ joint ].parent ;
		} else {
			joint = i ;
			parent = mdl->parent ;
		}
		for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
			verts = pack->verts ;
			norms = pack->norms ;
			uvs = pack->uvs ;
			mat.tex_id = pack->tex_id ;
			for ( k = 0 ; k < pack->n_verts ; k++, verts += 4, norms += 4, uvs += 2 ){
				/* 頂点キック処理 */
				vertex[0] = vertex[1] ;
				vertex[1] = vertex[2] ;
				v = &vertex[2] ;
				InitVertex( v );
				v->v.vx = (float)verts[0] + evm_def->skeleton[joint].rt_tx ;
				v->v.vy = (float)verts[1] + evm_def->skeleton[joint].rt_ty ;
				v->v.vz = (float)verts[2] + evm_def->skeleton[joint].rt_tz ;
				v->n.vx = (float)norms[0] / 4096.0f ;
				v->n.vy = (float)norms[1] / 4096.0f ;
				v->n.vz = (float)norms[2] / 4096.0f ;
				v->uv[0].vx = (float)uvs[0] / 4096.0f ;
				v->uv[0].vy = (float)uvs[1] / 4096.0f ;
				if ( pack->flag & 0x0001 ){
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
					/* ステージモデル対策（本当は必要ない） */
					//if ( mdl->parent == -1 ) v->weight.weight[0].skel_num = 0 ;
				}

				/* 描画キック判断 */
				if ( !( norms[3] & 0x8000 ) ){
					/* ポリゴン生成処理 */
					POLYGON	*poly ;
					poly = AllocPolygon() ;
					poly->flag = 0 ;
					poly->n_material = 1 ;
					poly->vertex[0] = vertex[0] ;
					poly->vertex[1] = vertex[1] ;
					poly->vertex[2] = vertex[2] ;
					poly->material[0] = mat ;
					poly->material[1] = poly->material[2] = null_material ;
					AddPolygon( &work->poly_stack, poly );
				}
			}
		}
	}
}
/* 他のモデルデータをマルチテクスチャとして合成する */
void AddPolygonData( KMS_DEF *kms_def, EVM_DEF *evm_def )
{
	static FVECTOR		zero_vector = {0.0f, 0.0f, 0.0f, 0.0f};
	static WEIGHT		zero_weight = { 0, 0.0f };
	static MATERIAL		null_material = { 0 };
	KMS_MDL		*mdl ;
	KMS_OBJPACK	*pack ;
	int			i, j, k, use_weight, use_material = 1, joint, parent ;
	short		*verts, *norms, *uvs ;
	VERTEX		vertex[3], *v ;
	MATERIAL	mat ;

	mdl = kms_def->models ;
	for ( i = 0 ; i < kms_def->n_x_models ; i++, mdl++ ){
		pack = mdl->packs ;
		if ( mdl->type & 0x4000 ){/* kms独自の付随モデルフラグ */
			joint = mdl->parent ;
			parent = kms_def->models[ joint ].parent ;
		} else {
			joint = i ;
			parent = mdl->parent ;
		}
		for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
			verts = pack->verts ;
			norms = pack->norms ;
			uvs = pack->uvs ;
			mat.tex_id = pack->tex_id ;
			for ( k = 0 ; k < pack->n_verts ; k++, verts += 4, norms += 4, uvs += 2 ){
				/* 頂点キック処理 */
				vertex[0] = vertex[1] ;
				vertex[1] = vertex[2] ;
				v = &vertex[2] ;
				InitVertex( v );
				v->v.vx = (float)verts[0] + evm_def->skeleton[joint].rt_tx ;
				v->v.vy = (float)verts[1] + evm_def->skeleton[joint].rt_ty ;
				v->v.vz = (float)verts[2] + evm_def->skeleton[joint].rt_tz ;
				v->n.vx = (float)norms[0] / 4096.0f ;
				v->n.vy = (float)norms[1] / 4096.0f ;
				v->n.vz = (float)norms[2] / 4096.0f ;
				v->uv[0].vx = (float)uvs[0] / 4096.0f ;
				v->uv[0].vy = (float)uvs[1] / 4096.0f ;
				if ( pack->flag & 0x0001 ){
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
					/* ステージモデル対策（本当は必要ない） */
					//if ( mdl->parent == -1 ) v->weight.weight[0].skel_num = 0 ;
				}

				/* 描画キック判断 */
				if ( !( norms[3] & 0x8000 ) ){
					/* ポリゴン生成処理 */
					POLYGON	*poly ;
					poly = AllocPolygon() ;
					poly->flag = 0 ;
					poly->n_material = 1 ;
					poly->vertex[0] = vertex[0] ;
					poly->vertex[1] = vertex[1] ;
					poly->vertex[2] = vertex[2] ;
					poly->material[0] = mat ;
					poly->material[1] = poly->material[2] = null_material ;
					{/* マルチテクスチャとして加えられるかチェック */
						POLYGON	*check_poly ;
						check_poly = GetFirstPolygon( &work->poly_stack );
						while ( check_poly != NULL ){
//							if ( IsJoinMultiTexturePolygon( check_poly, poly ) ){
							if ( IsJoinMultiTexturePolygon2( check_poly, poly ) ){
//								JoinMultiTexturePolygon( check_poly, poly );
								JoinMultiTexturePolygon2( check_poly, poly );
								FreePolygon( poly );
								poly = NULL ;
								break ;
							}
							check_poly = GetNextPolygon( check_poly );
						}
					}
					if ( poly != NULL ){
						printf("multi texture add error!\n");
						//AddPolygon( &work->poly_stack, poly );
					}
				}
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
int CutPacket_of_TopTexture( POLY_PACKET *new_packet, POLY_PACKET *org_packet )
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
int	CutMostLongStripPacket( POLY_PACKET *new_packet, POLY_PACKET *org_packet, int stop_n_strip )
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
void SeparateStripPacket( POLY_PACKET *new_packet, POLY_PACKET *packet )
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
static void MakeEVMDataPacket( void )
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
				if ( points > 29 ){
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
static void WriteVerts( EVM_PACK *packs, int num, VERTEX *v, int flag )
{
	int		i, w ;
	((SVECTOR*)packs->verts)[num].vx = v->v.vx ;
	((SVECTOR*)packs->verts)[num].vy = v->v.vy ;
	((SVECTOR*)packs->verts)[num].vz = v->v.vz ;
	((SVECTOR*)packs->verts)[num].vw = flag ;
	((SVECTOR*)packs->norms)[num].vx = v->n.vx * 4096 ;
	((SVECTOR*)packs->norms)[num].vy = v->n.vy * 4096 ;
	((SVECTOR*)packs->norms)[num].vz = v->n.vz * 4096 ;
	((SVECTOR*)packs->norms)[num].vw = 0 ;
	((SVECTOR*)packs->uvs0)[num].vx = v->uv[0].vx * 4096 ;
	((SVECTOR*)packs->uvs0)[num].vy = v->uv[0].vy * 4096 ;
	((SVECTOR*)packs->uvs0)[num].vz = v->uv[0].vz * 4096 ;
	((SVECTOR*)packs->uvs0)[num].vw = 0 ;
	((SVECTOR*)packs->uvs1)[num].vx = v->uv[1].vx * 4096 ;
	((SVECTOR*)packs->uvs1)[num].vy = v->uv[1].vy * 4096 ;
	((SVECTOR*)packs->uvs1)[num].vz = v->uv[1].vz * 4096 ;
	((SVECTOR*)packs->uvs1)[num].vw = 0 ;
	((SVECTOR*)packs->uvs2)[num].vx = v->uv[2].vx * 4096 ;
	((SVECTOR*)packs->uvs2)[num].vy = v->uv[2].vy * 4096 ;
	((SVECTOR*)packs->uvs2)[num].vz = v->uv[2].vz * 4096 ;
	((SVECTOR*)packs->uvs2)[num].vw = 0 ;
	w = 0 ;
#define WEIGHT_SCALE	(128)
	for ( i = 0 ; i < 8 ; i++ ){
		((unsigned char*)packs->weight)[num*8+i] = v->weight.weight[i].weight * WEIGHT_SCALE ;
		w += ((unsigned char*)packs->weight)[num*8+i] ;
	}
	/* 合計が１相当にならない場合には補正を入れる */
	if ( w < WEIGHT_SCALE ){
		for ( i = 0 ; i < 8 ; i++ ){
			if ( ((unsigned char*)packs->weight)[num*8+i] != 0 ){
				((unsigned char*)packs->weight)[num*8+i] += WEIGHT_SCALE - w ;
				break ;
			}
		}
	}
}
static int WriteEVMFile( char *filename, EVM_DEF *def )
{
	FILE		*fp ;
	EVM_PACK	*packs ;
	void		*verts_buffer, *write_buffer ;
	int			i, j, verts_offset, write_offset ;

	packs = Malloc0( sizeof(EVM_PACK) * work->n_packet_list );
	verts_buffer = Malloc0( ( 8 * 7 * 32) *  work->n_packet_list );
	verts_offset = 0 ;

	def->n_packs = work->n_packet_list ;

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
		packs[ i ].n_mats = GetUseSkeletonWeight( &work->packet_list[ i ] );
		packs[ i ].tex_id0 = poly->material[0].tex_id ;
		packs[ i ].tex_id1 = poly->material[1].tex_id ;
		packs[ i ].tex_id2 = poly->material[2].tex_id ;
		packs[ i ].m0 = poly->vertex[0].weight.weight[0].skel_num ;
		packs[ i ].m1 = poly->vertex[0].weight.weight[1].skel_num ;
		packs[ i ].m2 = poly->vertex[0].weight.weight[2].skel_num ;
		packs[ i ].m3 = poly->vertex[0].weight.weight[3].skel_num ;
		packs[ i ].m4 = poly->vertex[0].weight.weight[4].skel_num ;
		packs[ i ].m5 = poly->vertex[0].weight.weight[5].skel_num ;
		packs[ i ].m6 = poly->vertex[0].weight.weight[6].skel_num ;
		packs[ i ].m7 = poly->vertex[0].weight.weight[7].skel_num ;
		packs[ i ].verts = (void*)( verts_buffer + verts_offset ) ; verts_offset += sizeof(SVECTOR) * 32 ;
		packs[ i ].norms = (void*)( verts_buffer + verts_offset ) ; verts_offset += sizeof(SVECTOR) * 32 ;
		packs[ i ].uvs0 = (void*)( verts_buffer + verts_offset ) ; verts_offset += sizeof(SVECTOR) * 32 ;
		packs[ i ].uvs1 = (void*)( verts_buffer + verts_offset ) ; verts_offset += sizeof(SVECTOR) * 32 ;
		packs[ i ].uvs2 = (void*)( verts_buffer + verts_offset ) ; verts_offset += sizeof(SVECTOR) * 32 ;
		packs[ i ].weight = (void*)( verts_buffer + verts_offset ) ; verts_offset += sizeof(unsigned char) * 8 * 32 ;

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
		//printf("%d = %d verts %d mat \n", i, packs[i].n_verts, packs[i].n_mats );

		FreePolygonPacket( &work->packet_list[ i ] );
	}

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



}

/* ================================================================ */
/*
	Main Routine
*/
int Convert_Kms_To_MultiWeightEnvelopeModel( char *input_filename, char *input_filename2, char *input_filename3 )
{
	char	output_filename[ 1024 ] ;
	KMS_DEF	*kms_def ;
	EVM_DEF	*evm_def ;
	POLY_PACKET	tmp_packet ;
	int		i, n, total_vertex = 0 ;
	int		n_strip, max_strip ;

	if ( ( kms_def = OpenKms( input_filename ) ) == NULL ){
		return ( -1 );
	}
	MakeFileName( output_filename, input_filename,  input_filename, "evm" );

	InitProgramWork();

	evm_def = Malloc0( sizeof(EVM_DEF) + sizeof(EVM_SKEL) * kms_def->n_models );

	/* スケルトン情報の作成 */
	MakeSkeleton( kms_def, evm_def );

	/* ポリゴン情報の生成 */
	CreatePolygonData( kms_def, evm_def );

	/* マルチテクスチャ情報の作成 */
	{
		KMS_DEF	*tmp_kms_def ;
		EVM_DEF	*tmp_evm_def ;
		/* 別のモデルを統合する */
		if ( input_filename2 != NULL ){
			if ( ( tmp_kms_def = OpenKms( input_filename2 ) ) != NULL ){
				tmp_evm_def = Malloc0( sizeof(EVM_DEF) + sizeof(EVM_SKEL) * tmp_kms_def->n_models );
				MakeSkeleton( tmp_kms_def, tmp_evm_def );
				AddPolygonData( tmp_kms_def, tmp_evm_def );
				Free( tmp_evm_def );
				free( tmp_kms_def );
			}
		}
		if ( input_filename3 != NULL ){
			if ( ( tmp_kms_def = OpenKms( input_filename3 ) ) != NULL ){
				tmp_evm_def = Malloc0( sizeof(EVM_DEF) + sizeof(EVM_SKEL) * tmp_kms_def->n_models );
				MakeSkeleton( tmp_kms_def, tmp_evm_def );
				AddPolygonData( tmp_kms_def, tmp_evm_def );
				Free( tmp_evm_def );
				free( tmp_kms_def );
			}
		}
	}

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
		while ( GetTotalPolygons( &tmp_packet ) != 0 ){
			POLY_PACKET	local_packet ;

			/* 最長のストリップを生成する */
			n_strip = CutMostLongStripPacket( &local_packet, &tmp_packet, max_strip );
			if ( n_strip < max_strip ) max_strip = n_strip ;
#if 1
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
		MakeEVMDataPacket();

		/* ポリゴンの開放（実際には既に空っぽになっているはず） */
		FreePolygonPacket( &tmp_packet );
	}

	printf("total_vertex: %d                          \n", total_vertex );

	/* ファイルへの書き出し */
	WriteEVMFile( output_filename, evm_def );

	DisplayPolygonManagementStatus();

	Free( evm_def );
	free( kms_def );
	EndProgramWork();
}



