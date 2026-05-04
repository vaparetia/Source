//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xobjs.c
	新物体ハンドラ処理ルーチン

	2002/03/12 K.Takabe
	$Id: xobjs.c,v 1.1.1.3 2002/11/19 11:42:36 Yoshizawa1 Exp $

*/
/*

	DG_OBJS		*DG_MakeObjs( def, flag, chanl )
	DG_DEF		*def ;		モデルデータ
	int		flag ;		処理フラグ
	int		chanl ;		描画チャンネル

		あるモデルデータに対応する、物体ハンドラを作成する


	DG_OBJS		*DG_MakeObjs2( DG_DEF *def, int flag, int chanl, DG_MDL **mdl_list, int n_list )
	DG_DEF		*def ;			基本モデルデータ
	int			flag ;			処理フラグ
	int			chanl ;			描画チャンネル
	DG_MDL		**mdl_list ;	各関節のモデルデータポインタの配列
	int			n_list ;		用意したモデルポインタ配列の要素数

		各関節を任意のモデルで初期化する

	void DG_ChangeModelObj( DG_OBJ *obj, DG_MDL *mdl )
	DG_OBJ		*obj ;	差し替えるオブジェクト
	DG_MDL		*mdl ;	差し替え用モデルデータ

		任意のオブジェクトのモデルを変更する

	void		DG_FreeObjs( objs )
	DG_OBJS		*objs ;		物体ハンドラ

		物体ハンドラと、そのパケットメモリを解放する
		（プレシェイドバッファは解放されないので注意）

		->  プレシェイドバッファをobjs側に確保するようにしたのに伴い
			プレシェイドバッファも解放されるように変更 ( 97/11/08 )

	void DG_ConnectObjs( DG_OBJS *parent, DG_OBJS *child )
	DG_OBJS		*parent ;	関連付けされる親オブジェクト
	DG_OBJS		*child ;	関連付けする子オブジェクト

		子オブジェクトを親オブジェクトに関連させる

	void DG_DisconnectObjs( DG_OBJS *parent, DG_OBJS *child )
	DG_OBJS		*parent ;	関連付けされる親オブジェクト
	DG_OBJS		*child ;	関連付けする子オブジェクト

		子オブジェクト関連をはずす


	void DG_SetFogParamObjs( DG_OBJS *objs, float near, float far )
	DG_OBJS		*objs ;		設定するオブジェクト
	float		near ;		フォグニア
	float		far ;		フォグファー

		オブジェクト固有フォグパラメータを設定する
		DG_FLAG_SHADE及びDG_FLAG_FOGPARAMを指定しているときのみ有効


	void DG_SetLowObjs( DG_OBJS *objs, DG_OBJS *low_objs )
	DG_OBJS		*objs ;		親オブジェクト
	DG_OBJS		*low_objs ;	ローポリゴンオブジェクト

		オブジェクトにローポリゴン版オブジェクトを関連付けする
		これで指定したローポリゴンオブジェクトはDG_QueueObj()を行う必要はなく、
		マトリクスの設定なども行う必要はない。但しDG_FreeObjs()は忘れずに
		行うこと


	void DG_WriteMdlPaketUV( int tri_code, DG_MDLPACK *pack )
	int			tri_code ;	使用するＴＲＩファイルのＩＤ
	DG_MDLPACK	*pack ;		補正するオブジェクトパケット

		テクスチャからＵＶ値を補正する


------------------------------------------------ 以下残骸
	void		DG_SetJointFrame( objs, rots )
	DG_OBJS		*objs ;		物体ハンドラ
	SVECTOR		*rots ;		回転関節データ

		物体ハンドラを、回転関節型に設定する

	void		DG_SetSlideFrame( objs, movs )
	DG_OBJS		*objs ;		物体ハンドラ
	SVECTOR		*movs ;		横すべり関節データ

		物体ハンドラを、横すべり回転関節型に設定する

*/

#ifdef KP_XBOX //BP


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"

#ifdef DEBUG_MODE
#ifdef DG_MakeObjs
#undef DG_MakeObjs
#endif
#ifdef DG_MakeObjs2
#undef DG_MakeObjs2
#endif
#endif

/*----------------------------------------------------------------*/
static int SetMultiTexType( DG_MDLPACK *mdlpack )
{
	int		type ;
	DG_TEX	*tex[ 3 ];

	tex[0] = (DG_TEX*)mdlpack->tex_id[0] ;
	tex[1] = (DG_TEX*)mdlpack->tex_id[1] ;
	tex[2] = (DG_TEX*)mdlpack->tex_id[2] ;
	if ( tex[1] == NULL && tex[2] == NULL ) return ( DG_MULTITEX_NORMAL );	/* シングルテクスチャ */
	/* 環境マップかどうかをチェック */
	if ( tex[2] != NULL ){
		if ( ( tex[2]->flag & TEXINFO_FLAG_MAPMASK ) == TEXINFO_FLAG_EMAP ){
#if 0
			if ( tex[1] != NULL ){
				if ( ( tex[0]->tex_trans.alpha.data & 0xff ) == SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0 ) ){
					/* 重ね乗せ用２テクスチャ環境マップ（アルファベース＋加算環境マップ） */
					return ( DG_MULTITEX_EMAP3 );
				}
				/* ＭＧＳ２標準環境マップ */
				return ( DG_MULTITEX_EMAP );
			} else {
				if ( ( tex[0]->tex_trans.alpha.data & 0xff ) == SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0 ) ){
					/* 重ね乗せ用２テクスチャ環境マップ（減算ベース＋加算環境マップ） */
					return ( DG_MULTITEX_EMAP2 );
				}
				/* ＭＧＳ２標準環境マップ */
				return ( DG_MULTITEX_EMAP );
			}
#else
			if ( tex[1] != NULL ){
				/* ＭＧＳ２標準環境マップ */
				return ( DG_MULTITEX_EMAP );
			} else {
				if ( ( tex[0]->tex_trans.alpha.data & 0xff ) == SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0 ) ){
					/* 重ね乗せ用２テクスチャ環境マップ（減算ベース＋加算環境マップ） */
					return ( DG_MULTITEX_EMAP2 );
				}
				/* ＭＧＳ２標準環境マップ */
				return ( DG_MULTITEX_EMAP4 );
			}
#endif
		}
	}
	/* トゥーンシェードマップかどうかをチェック */
	if ( tex[2] != NULL ){
		if ( ( tex[2]->flag & TEXINFO_FLAG_MAPMASK ) == TEXINFO_FLAG_SMAP ){
			return ( DG_MULTITEX_SMAP );
		}
	}
	/* バンプマップかどうかをチェック */
	/* その他特殊系かどうかをチェック */
	return ( DG_MULTITEX_NORMAL3 );
}


/*----------------------------------------------------------------*/

DG_OBJS		*DG_MakeObjs( DG_DEF *def, int flag, int chanl )
{
	DG_OBJS		*objs ;
	DG_OBJ		*obj ;
	DG_MDL		*mdl ;
	DG_OBJ_PACKET	*pack ;
	int		buf_size ;
	int		i, j, n_models ;

	flag |= DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3 ;
	ASSERT( def != NULL );
	n_models = def->n_x_models ;
	/*
		物体ハンドラのバッファを確保する
	*/
	buf_size = sizeof( DG_OBJS ) + sizeof( DG_OBJ ) * n_models ;
	if ( ( objs = (DG_OBJS *)GV_Malloc( buf_size ) ) == NULL ){
		return NULL ;
	}
	/*
		基本データ初期化
	*/
	GV_ZeroMemory( objs, buf_size ) ;
	if ( DG_GetMdlFormat( def ) == MGS_MODEL_MULTITEX || DG_GetMdlFormat( def ) == MGS_MODEL_MULTITEX_A ){
		printf("multi texture kms init!\n");
		flag |= DG_FLAG_MULTITEX ;
	}
	objs->world = DG_UnitMatrix;
	objs->def = def ;
	objs->n_models = n_models ;
	objs->flag = flag ;
	objs->chanl = chanl ;
	objs->light = &DG_LightMatrix ;
	//objs->tri_id = DG_SearchTriFromKms( def );
	objs->tri_id = def->texture ;
	objs->bound_min.vx = def->lx ;
	objs->bound_min.vy = def->ly ;
	objs->bound_min.vz = def->lz ;
	objs->bound_min.vw = 1.0f ;
	objs->bound_max.vx = def->ux ;
	objs->bound_max.vy = def->uy ;
	objs->bound_max.vz = def->uz ;
	objs->bound_max.vw = 1.0f ;
	objs->trans.vx = def->tx ;
	objs->trans.vy = def->ty ;
	objs->trans.vz = def->tz ;
	objs->trans.vw = 1.0f ;

	/* 最初は全グループ表示にしておく（2000/01/11 M.Sonoyama） */
	objs->group_id = 0x7fffffff ;	

	/*
		物体ユニットデータ初期化
	*/
	mdl = def->models ;
	obj = objs->objs ;

	//printf("make objs ... %08x %d %d\n", def, def->texture, def->n_models );

	for ( i = n_models ; i > 0 ; -- i ) {
		DG_MDLPACK	*mdlpack ;
		/* 各オブジェクトユニットの設定 */
		obj->world = DG_UnitMatrix;
		obj->light = &DG_LightMatrix ;
		obj->model = mdl ;
		obj->flag = objs->flag ;
		obj->n_packs = mdl->n_packs ;
		obj->parent = mdl->parent ;
		obj->mdl_type = mdl->type ;
		obj->bound_min.vx = mdl->lx ;
		obj->bound_min.vy = mdl->ly ;
		obj->bound_min.vz = mdl->lz ;
		obj->bound_max.vx = mdl->ux ;
		obj->bound_max.vy = mdl->uy ;
		obj->bound_max.vz = mdl->uz ;
		obj->trans.vx = mdl->tx ;
		obj->trans.vy = mdl->ty ;
		obj->trans.vz = mdl->tz ;
		obj->trans.vw = 1.0f ;
		//obj->pre_packet = mdl->pre_packet ;
		obj->vbuff = mdl->vbuff ;				/* 頂点バッファ記録 */
		obj->index = mdl->packs->index ;		/* インデックスバッファ記録 */
		obj->stride = mdl->stride ;				/* 頂点バッファのストライド記録 */
		if ( obj->n_packs == 0 ){
			printf("warning: null packet\n");
			obj ++ ;
			mdl ++ ;
			continue ;
		}
		/* 頂点情報のポインタ設定 */
		obj->verts = (SVECTOR*)mdl->packs->verts ;
		obj->norms = (SVECTOR*)mdl->packs->norms ;
		obj->uvs[0] = mdl->packs->uvs[0] ;
		obj->uvs[1] = mdl->packs->uvs[1] ;
		obj->uvs[2] = mdl->packs->uvs[2] ;
		//printf("%08x %08x %08x %08x %08x \n", obj->verts, obj->norms, obj->uvs[0], obj->uvs[1], obj->uvs[2] );
		/* 各パケット情報の設定 */
		obj->packets = pack = GV_Malloc( sizeof(DG_OBJ_PACKET) * obj->n_packs );
		if ( pack == NULL ){
			printf("create objs error!!\n");
			DG_FreeObjs( objs );
			return ( NULL );
		}
		GV_ZeroMemory( pack, sizeof(DG_OBJ_PACKET) * obj->n_packs );
		mdlpack = mdl->packs ;
		for ( j = 0 ; j < mdl->n_packs ; j++ ){
#ifdef PSX2			
#ifdef DEBUG_MODE
			if ( mdlpack->n_verts > 64 ) printf("too many n_verts(%d)\n",mdlpack->n_verts);
#endif
#endif			
			if ( obj->uvs[0] == NULL && mdlpack->uvs[0] != NULL ) obj->uvs[0] = mdlpack->uvs[0] ;
			if ( obj->uvs[1] == NULL && mdlpack->uvs[1] != NULL ) obj->uvs[1] = mdlpack->uvs[1] ;
			if ( obj->uvs[2] == NULL && mdlpack->uvs[2] != NULL ) obj->uvs[2] = mdlpack->uvs[2] ;
			pack->flag = mdlpack->flag ;
			pack->n_indices = mdlpack->n_indices ;
			//pack->data_ptr = (void*)mdlpack->pad ;	/* これは今は必要ない？（実験用か？） */
			pack->n_verts = mdlpack->n_verts ;
			pack->norms_offset = pack->verts_offset =
			  ( mdlpack->n_verts + 1 ) / 2 ;
			pack->uvs_offset[2] = pack->uvs_offset[1] = pack->uvs_offset[0] =
			  ( mdlpack->n_verts + 3 ) / 4 ;
			if ( !( pack->flag & DG_PACKFLAG_UV0 ) ) pack->uvs_offset[0] = 0 ;
			if ( !( pack->flag & DG_PACKFLAG_UV1 ) ) pack->uvs_offset[1] = 0 ;
			if ( !( pack->flag & DG_PACKFLAG_UV2 ) ) pack->uvs_offset[2] = 0 ;
			//if ( flag & DG_FLAG_MULTITEX ) printf("%08x %d \n", pack->flag, pack->n_verts );
			if ( mdlpack->tex_id[0] != 0 ) pack->tex_ptr[0] = &( (DG_TEX*)mdlpack->tex_id[0] )->tex_trans ;
			if ( mdlpack->tex_id[1] != 0 ) pack->tex_ptr[1] = &( (DG_TEX*)mdlpack->tex_id[1] )->tex_trans ;
			if ( mdlpack->tex_id[2] != 0 ) pack->tex_ptr[2] = &( (DG_TEX*)mdlpack->tex_id[2] )->tex_trans ;
			pack->mtex_type = SetMultiTexType( mdlpack );
			pack++ ;
			mdlpack++ ;
		}
		obj ++ ;
		mdl ++ ;
	}
	return objs ;
}

/* 関節毎にモデルを指定 */
DG_OBJS		*DG_MakeObjs2( DG_DEF *def, int flag, int chanl, DG_MDL **mdl_list, int n_list )
{
	DG_OBJS		*objs ;
	DG_OBJ		*obj ;
	DG_MDL		*mdl ;
	DG_OBJ_PACKET	*pack ;
	int		buf_size ;
	int		i, j, n_models ;

	flag |= DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3 ;
	ASSERT( def != NULL );
	n_models = def->n_x_models ;
	/*
		物体ハンドラのバッファを確保する
	*/
	buf_size = sizeof( DG_OBJS ) + sizeof( DG_OBJ ) * n_models ;
	if ( ( objs = (DG_OBJS *)GV_Malloc( buf_size ) ) == NULL ){
		return NULL ;
	}
	/*
		基本データ初期化
	*/
	if ( def->data_format == MGS_MODEL_MULTITEX || def->data_format == MGS_MODEL_MULTITEX_A ){
		flag |= DG_FLAG_MULTITEX ;
	}
	GV_ZeroMemory( objs, buf_size ) ;
	objs->world = DG_UnitMatrix;
	objs->def = def ;
	objs->n_models = n_models ;
	objs->flag = flag ;
	objs->chanl = chanl ;
	objs->light = &DG_LightMatrix ;
	//objs->tri_id = DG_SearchTriFromKms( def );
	objs->tri_id = def->texture ;
	objs->bound_min.vx = def->lx ;
	objs->bound_min.vy = def->ly ;
	objs->bound_min.vz = def->lz ;
	objs->bound_min.vw = 1.0f ;
	objs->bound_max.vx = def->ux ;
	objs->bound_max.vy = def->uy ;
	objs->bound_max.vz = def->uz ;
	objs->bound_max.vw = 1.0f ;
	objs->trans.vx = def->tx ;
	objs->trans.vy = def->ty ;
	objs->trans.vz = def->tz ;
	objs->trans.vw = 1.0f ;

	/* 最初は全グループ表示にしておく（2000/01/20 M.Sonoyama） */
	objs->group_id = 0x7fffffff ;	

	/*
		物体ユニットデータ初期化
	*/
	mdl = def->models ;
	obj = objs->objs ;

	for ( i = n_models ; i > 0 ; -- i ) {
		DG_MDLPACK	*mdlpack ;
		/* 各オブジェクトユニットの設定 */
		obj->world = DG_UnitMatrix;
		obj->light = &DG_LightMatrix ;
		obj->flag = objs->flag ;
		if ( n_list > 0 && *mdl_list != NULL ){
			obj->model = *mdl_list ;
		} else {
			obj->model = mdl ;
		}
		obj->n_packs = obj->model->n_packs ;
		obj->parent = obj->model->parent ;
		obj->mdl_type = obj->model->type ;
		obj->bound_min.vx = obj->model->lx ;
		obj->bound_min.vy = obj->model->ly ;
		obj->bound_min.vz = obj->model->lz ;
		obj->bound_max.vx = obj->model->ux ;
		obj->bound_max.vy = obj->model->uy ;
		obj->bound_max.vz = obj->model->uz ;
		obj->trans.vx = obj->model->tx ;
		obj->trans.vy = obj->model->ty ;
		obj->trans.vz = obj->model->tz ;
		obj->trans.vw = 1.0f ;
		obj->vbuff = obj->model->vbuff ;				/* 頂点バッファ記録 */
		obj->index = obj->model->packs->index ;			/* インデックスバッファ記録 */
		obj->stride = obj->model->stride ;				/* 頂点バッファストライド記録 */
		//obj->pre_packet = obj->model->pre_packet ;
		/* 頂点情報のポインタ設定 */
		obj->verts = (SVECTOR*)obj->model->packs->verts ;
		obj->norms = (SVECTOR*)obj->model->packs->norms ;
		obj->uvs[0] = obj->model->packs->uvs[0] ;
		obj->uvs[1] = obj->model->packs->uvs[1] ;
		obj->uvs[2] = obj->model->packs->uvs[2] ;
		/* 各パケット情報の設定 */
		obj->packets = pack = GV_Malloc( sizeof(DG_OBJ_PACKET) * obj->n_packs );
		if ( pack == NULL ){
			printf("create objs error!!\n");
			DG_FreeObjs( objs );
			return ( NULL );
		}
		GV_ZeroMemory( pack, sizeof(DG_OBJ_PACKET) * obj->n_packs );
		mdlpack = obj->model->packs ;
		for ( j = 0 ; j < obj->n_packs ; j++ ){
			if ( obj->uvs[0] == NULL && mdlpack->uvs[0] != NULL ) obj->uvs[0] = mdlpack->uvs[0] ;
			if ( obj->uvs[1] == NULL && mdlpack->uvs[1] != NULL ) obj->uvs[1] = mdlpack->uvs[1] ;
			if ( obj->uvs[2] == NULL && mdlpack->uvs[2] != NULL ) obj->uvs[2] = mdlpack->uvs[2] ;
			pack->flag = mdlpack->flag ;
			pack->n_indices = mdlpack->n_indices ;
			//pack->data_ptr = (void*)mdlpack->pad ;	/* これはもう必要ないかも */
			pack->n_verts = mdlpack->n_verts ;
			pack->norms_offset = pack->verts_offset =
			  ( mdlpack->n_verts + 1 ) / 2 ;
			pack->uvs_offset[2] = pack->uvs_offset[1] = pack->uvs_offset[0] =
			  ( mdlpack->n_verts + 3 ) / 4 ;
			if ( !( pack->flag & DG_PACKFLAG_UV0 ) ) pack->uvs_offset[0] = 0 ;
			if ( !( pack->flag & DG_PACKFLAG_UV1 ) ) pack->uvs_offset[1] = 0 ;
			if ( !( pack->flag & DG_PACKFLAG_UV2 ) ) pack->uvs_offset[2] = 0 ;
			if ( mdlpack->tex_id[0] != 0 ) pack->tex_ptr[0] = &( (DG_TEX*)mdlpack->tex_id[0] )->tex_trans ;
			if ( mdlpack->tex_id[1] != 0 ) pack->tex_ptr[1] = &( (DG_TEX*)mdlpack->tex_id[1] )->tex_trans ;
			if ( mdlpack->tex_id[2] != 0 ) pack->tex_ptr[2] = &( (DG_TEX*)mdlpack->tex_id[2] )->tex_trans ;
			pack->mtex_type = SetMultiTexType( mdlpack );
			pack++ ;
			mdlpack++ ;
		}
		obj++ ;
		mdl++ ;
		mdl_list++ ;
		n_list-- ;
	}
	return objs ;
}

/* 部分的にモデルを変更する */
void DG_ChangeModelObj( DG_OBJ *obj, DG_MDL *mdl )
{
	int		i ;
	DG_OBJ_PACKET	*pack ;
	DG_MDLPACK		*mdlpack ;
	/* プリシェード型オブジェクトには未対応なので注意 */
	ASSERT( !( obj->flag & DG_FLAG_PAINT ) );

	/* メモリ確保 */
	pack = GV_Malloc( sizeof(DG_OBJ_PACKET) * mdl->n_packs );
	if ( pack == NULL ){
		printf("obj change error\n");
		return ;
	}

	/* メモリ開放 */
	GV_Free( obj->packets );
	/* DG_OBJの再設定 */
	obj->model = mdl ;
	obj->n_packs = mdl->n_packs ;
	obj->packets = pack ;
	obj->verts = (SVECTOR*)mdl->packs->verts ;
	obj->norms = (SVECTOR*)mdl->packs->norms ;
	obj->uvs[0] = mdl->packs->uvs[0] ;
	obj->uvs[1] = mdl->packs->uvs[1] ;
	obj->uvs[2] = mdl->packs->uvs[2] ;
	obj->parent = mdl->parent ;
	obj->mdl_type = mdl->type ;
	obj->bound_min.vx = mdl->lx ;
	obj->bound_min.vy = mdl->ly ;
	obj->bound_min.vz = mdl->lz ;
	obj->bound_max.vx = mdl->ux ;
	obj->bound_max.vy = mdl->uy ;
	obj->bound_max.vz = mdl->uz ;
	obj->trans.vx = mdl->tx ;
	obj->trans.vy = mdl->ty ;
	obj->trans.vz = mdl->tz ;
	obj->trans.vw = 1.0f ;
	obj->vbuff = mdl->vbuff ;				/* 頂点バッファ記録 */
	obj->index = mdl->packs->index ;		/* インデックスバッファ記録 */
	obj->stride = mdl->stride ;				/* 頂点バッファストライド記録 */
	//obj->pre_packet = mdl->pre_packet ;
	mdlpack = mdl->packs ;
	for ( i = 0 ; i < obj->n_packs ; i++ ){
		if ( obj->uvs[0] == NULL && mdlpack->uvs[0] != NULL ) obj->uvs[0] = mdlpack->uvs[0] ;
		if ( obj->uvs[1] == NULL && mdlpack->uvs[1] != NULL ) obj->uvs[1] = mdlpack->uvs[1] ;
		if ( obj->uvs[2] == NULL && mdlpack->uvs[2] != NULL ) obj->uvs[2] = mdlpack->uvs[2] ;
		pack->flag = mdlpack->flag ;
		pack->n_indices = mdlpack->n_indices ;
		//pack->data_ptr = (void*)mdlpack->pad ;	/* これはもう必要ないかも */
		pack->n_verts = mdl->packs[i].n_verts ;
		pack->norms_offset = pack->verts_offset = ( mdlpack->n_verts + 1 ) / 2 ;
		pack->uvs_offset[2] = pack->uvs_offset[1] = pack->uvs_offset[0] = ( mdlpack->n_verts + 3 ) / 4 ;
		if ( !( pack->flag & DG_PACKFLAG_UV0 ) ) pack->uvs_offset[0] = 0 ;
		if ( !( pack->flag & DG_PACKFLAG_UV1 ) ) pack->uvs_offset[1] = 0 ;
		if ( !( pack->flag & DG_PACKFLAG_UV2 ) ) pack->uvs_offset[2] = 0 ;
		if ( mdlpack->tex_id[0] != 0 )	pack->tex_ptr[0] = &( (DG_TEX*)mdlpack->tex_id[0] )->tex_trans ;
		else							pack->tex_ptr[0] = NULL ;
		if ( mdlpack->tex_id[1] != 0 )	pack->tex_ptr[1] = &( (DG_TEX*)mdlpack->tex_id[1] )->tex_trans ;
		else							pack->tex_ptr[1] = NULL ;
		if ( mdlpack->tex_id[2] != 0 )	pack->tex_ptr[2] = &( (DG_TEX*)mdlpack->tex_id[2] )->tex_trans ;
		else							pack->tex_ptr[2] = NULL ;
		pack->mtex_type = SetMultiTexType( mdlpack );
		pack++ ;
		mdlpack++ ;
	}
}

void		DG_FreeObjs( DG_OBJS *objs )
{
	DG_OBJ		*obj ;
	int		i ;

	if ( objs == NULL ) return ;

	DG_FreePreshade( objs );
	obj = objs->objs ;
	if ( !( objs->flag & DG_FLAG_DELAYED ) ){
		/* 通常開放処理 */
		for ( i = objs->n_models ; i > 0 ; -- i ) {
			if ( obj->packets != NULL ){
				GV_Free( obj->packets );
			}
			obj ++ ;
		}
		GV_Free( objs ) ;
	} else {
		/* １フレーム遅らせ開放 */
		for ( i = objs->n_models ; i > 0 ; -- i ) {
			if ( obj->packets != NULL ){
				GV_DelayedFree( obj->packets );
			}
			obj ++ ;
		}
		GV_DelayedFree( objs ) ;
	}
}

	/*
		子オブジェクトを親オブジェクトに関連させる
	*/
void DG_ConnectObjs( DG_OBJS *parent, DG_OBJS *child )
{
	//printf("objs.c: connect objs start (%08x,%08x) time=%d\n", parent, child, GV_Time );
	while ( parent->next != NULL ){
		//printf("objs.c: next = %08x\n", parent->next );
		parent = parent->next ;
	}
	//printf("objs.c: connect objs end(%08x)\n", child->next );
	parent->next = child ;
}

	/*
		子オブジェクト関連をはずす
	*/
void DG_DisconnectObjs( DG_OBJS *parent, DG_OBJS *child )
{
	/* 親が先に死んでいると不具合が起こる可能性があるため予めチェックしておく */
	if ( child == NULL ) return ;
	if ( parent == NULL ) return ;
	if ( DG_SearchQueueObjs( parent ) == NULL ) return ;

	while ( parent->next != child ){
		parent = parent->next ;
		if ( parent == NULL ){
			printf("Disconnect error!!\n");
			return ;
		}
	}
	parent->next = parent->next->next ;
}

	/*
		オブジェクト固有フォグパラメータを設定する
	*/
void DG_SetFogParamObjs( DG_OBJS *objs, float var_near, float var_far )
{
#if 0
	/* PS2 */
	objs->fog_param[0] = 255.0f / ( var_near - var_far );
	objs->fog_param[1] = 255.0f - objs->fog_param[0] * var_near ;
#else
	/* XBOX */
	objs->fog_param[0] = - 1.0f / ( var_near - var_far );
	objs->fog_param[1] = - objs->fog_param[0] * var_near ;
#endif
}

	/*
		オブジェクトにローポリゴン版オブジェクトを関連付けする
		これで指定したローポリゴンオブジェクトはDG_QueueObj()を行う必要はなく、
		マトリクスの設定なども行う必要はない。但しDG_FreeObjs()は忘れずに
		行うこと
	*/
void DG_SetLowObjs( DG_OBJS *objs, DG_OBJS *low_objs )
{
	if ( objs == NULL || low_objs == NULL ) return ;
	//if ( objs->n_models != low_objs->n_models ) return ;
	if ( objs->def->n_models != low_objs->def->n_models ) return ;
	objs->low = low_objs ;
	/* 念のためバウンディングを調整する必要はあるのか？ */
}

/*----------------------------------------------------------------*/

void DG_WriteMdlPaketUV( int tri_code, DG_MDLPACK *pack )
{
	float	u_scale, v_scale, u_offset, v_offset ;
	short	*uv ;
	DG_TEX	*tex ;
	int			i, j ;
	//static int	uv_check_table[3] = {DG_PACKFLAG_UV0,DG_PACKFLAG_UV1|DG_PACKFLAG_BMAP,DG_PACKFLAG_UV2|DG_PACKFLAG_BMAP};

	//printf("%d %d %d\n", pack->tex_id[0], pack->tex_id[1], pack->tex_id[2] );
	for ( i = 0 ; i < 3 ; i++ ){
		if ( pack->tex_id[i] == 0 || pack->tex_id[i] == 1 ){
			if ( i == 0 ){
				if ( pack->tex_id[0] != 0 ){
					printf("warning: no texture packet %08x\n", pack->flag);
				}
				pack->tex_id[i] = (int)DG_GetTexture2( tri_code, pack->tex_id[i] ) ;
			}
			continue ;
		}
		/* ＵＶ値のスケール補正（既にテクスチャが読み込まれている必要あり） */
		tex = DG_GetTexture2( tri_code, pack->tex_id[i] );
		if ( tex != NULL ){
			u_scale = tex->u_scale ;
			v_scale = tex->v_scale ;
			u_offset = tex->u_offset * 4096.0F ;
			v_offset = tex->v_offset * 4096.0F ;
		} else {
			u_scale = 1.0F ; v_scale = 1.0F ;
			u_offset = 0.0F ; v_offset = 0.0F ;
		}
		uv = pack->uvs[i] ;
		if ( uv != NULL ){
			for ( j = pack->n_verts ; j > 0 ; j-- ){
#ifndef UVADJUST
				uv[0] = (short)DG_FTOI( (float)uv[0] * u_scale + u_offset );
				uv[1] = (short)DG_FTOI( (float)uv[1] * v_scale + v_offset );
#endif
				uv += 2 ;
			}
		}
#if 0
		/* パック化したＤＭＡデータ中のＵＶ補正処理 */
		if ( ( pack->pad != 0 ) && ( pack->flag & uv_check_table[i] ) ){
			uv = (void*)( pack->pad + ( ( ( pack->n_verts + 1 ) / 2 ) * 16 + 16 ) * 2 );/* verts&norms */
			uv = (void*)( (int)uv + 16 + ( ( ( pack->n_verts + 3 ) / 4 ) * 16 + 16 ) * i ) ;
			//printf("pack:%08x pad: %08x addr:%08x\n", pack, pack->pad, uv );
			for ( j = pack->n_verts ; j > 0 ; j-- ){
				uv[0] = (short)DG_FTOI( (float)uv[0] * u_scale + u_offset );
				uv[1] = (short)DG_FTOI( (float)uv[1] * v_scale + v_offset );
				uv += 2 ;
			}
		} else {
			pack->pad = 0 ;
		}
#endif

		pack->tex_id[i] = (int)tex ;
	}
}

/*----------------------------------------------------------------*/

#ifdef DEBUG_MODE
DG_OBJS		*DG_MakeObjsD( DG_DEF *def, int flag, int chanl, char *fname )
{
	DG_OBJS	*objs ;
	objs = DG_MakeObjs( def, flag, chanl );
	if ( objs == NULL ) return NULL ;
	objs->fname = fname ;
	return ( objs );
}
DG_OBJS		*DG_MakeObjs2D( DG_DEF *def, int flag, int chanl, DG_MDL **mdl_list, int n_list, char *fname )
{
	DG_OBJS		*objs ;
	objs = DG_MakeObjs2( def, flag, chanl, mdl_list, n_list );
	if ( objs == NULL ) return NULL ;
	objs->fname = fname ;
	return ( objs );
}

#endif
/*----------------------------------------------------------------*/


#endif
