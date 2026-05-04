/*
	xobjs.c
	新物体ハンドラ処理ルーチン

	2002/03/12 K.Takabe
	$Id: wobjs.c,v 1.25 2003/01/09 13:14:20 takaki Exp $

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

//#define	OBJ_VERTEXBUFFER_POOL		(D3DPOOL_DEFAULT)	// 使用POOL
#define	OBJ_VERTEXBUFFER_POOL		(D3DPOOL_MANAGED)	// 使用POOL
#define	OBJ_VERTEXBUFFER_LOCKFLAG	(D3DLOCK_NOSYSLOCK)	// LOCK FLAG
#define	OBJ_INDEXBUFFER_LOCKFLAG	(D3DLOCK_NOSYSLOCK)	// LOCK FLAG

static void ObjSetVertexBuffer( DG_OBJ *obj, DG_MDL *mdl )
{
	DWORD	stride ;
	DWORD	n_verts ;
	DWORD	n_indices ;
	BOOL	mtex ;
	DG_MDL_VERTEX_BUFFER_TBL vbuff_tbl ;

	stride    = obj->stride ;
	n_verts   = obj->n_verts ;
	n_indices = obj->n_indices ;

	obj->d3d_vbuff  = NULL ;
	obj->d3d_cvbuff = NULL ;
	obj->d3d_ibuff  = NULL ;

	mtex = obj->flag & DG_FLAG_MULTITEX ;	// Multi Texture判定

	/*-- Direct3D Vertex Buffer作成 --------------------------------------------*/

	if( !mtex && (obj->flag & DG_FLAG_PAINT) )
	{
		if( DG_CheckObjUseVertexShader() )
		{
			DWORD	n_cverts ;

			/*-- Vertex Shader有り ---------------------------------------------*/

			if( DG_CheckIndexPrimitiveUseable() ){ n_cverts = n_verts ; }
			else{ n_cverts =  n_indices ; }

			DG_CreateVertexBuffer( sizeof(CVECTOR) * n_cverts,
								DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
								0, OBJ_VERTEXBUFFER_POOL,
								&obj->d3d_cvbuff) ;
			obj->cvbuff_size = sizeof(CVECTOR) * n_cverts ;
			/*------------------------------------------------------------------*/
		}
		else
		{
			DWORD	size ;
			BYTE	*dst ;

			/*-- Shader無しの場合 ----------------------------------------------*/

			if( DG_CheckIndexPrimitiveUseable() )
			{
				DG_CreateVertexBuffer(sizeof(DG_VERTEX_KMSS_FVF) * n_verts,
								DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
								0, OBJ_VERTEXBUFFER_POOL,
								&obj->d3d_cvbuff) ;
				obj->cvbuff_size = sizeof(DG_VERTEX_KMSS_FVF) * n_verts ;

				/*-- 内容複写 --------------------------------------------------*/

				if( obj->d3d_cvbuff )
				{
					size = sizeof(DG_VERTEX_KMSS_FVF) * n_verts ;
					IDirect3DVertexBuffer8_Lock(obj->d3d_cvbuff, 0, size, &dst,
												OBJ_VERTEXBUFFER_LOCKFLAG) ;
					DG_ConvertKmss2KmssFVF((void *)dst, (void *)obj->vbuff, NULL, n_verts) ;
					IDirect3DVertexBuffer8_Unlock(obj->d3d_cvbuff) ;
				}
				/*--------------------------------------------------------------*/
			}
			else
			{
				DG_CreateVertexBuffer(sizeof(DG_VERTEX_KMSS_FVF) * n_indices,
								DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
								0, OBJ_VERTEXBUFFER_POOL,
								&obj->d3d_cvbuff) ;
				obj->cvbuff_size = sizeof(DG_VERTEX_KMSS_FVF) * n_indices ;

				/*-- 内容複写 --------------------------------------------------*/

				if( obj->d3d_cvbuff )
				{
					size = sizeof(DG_VERTEX_KMSS_FVF) * n_indices ;

					if( obj->wflag & DG_OBJ_WFLAG_ORDERED_INDEX )
					{
						/*-- 整列された頂点 ------------------------------------*/

						IDirect3DVertexBuffer8_Lock(obj->d3d_cvbuff, 0, size, &dst,
													OBJ_VERTEXBUFFER_LOCKFLAG) ;
						DG_ConvertKmss2KmssFVF((void *)dst, (void *)obj->vbuff,
											NULL, n_indices) ;
						IDirect3DVertexBuffer8_Unlock(obj->d3d_cvbuff) ;
						/*------------------------------------------------------*/
					}
					else
					{
						BYTE	*tmp ;

						/*-- 頂点置換 ------------------------------------------*/

						tmp = (void *)DG_GetWorkBuffer() ;

						DG_ArrangeVertexByIndex((void *)tmp, (void *)obj->vbuff, stride,
										obj->index, n_indices) ;

						IDirect3DVertexBuffer8_Lock(obj->d3d_cvbuff, 0, size, &dst,
												OBJ_VERTEXBUFFER_LOCKFLAG) ;
						DG_ConvertKmss2KmssFVF((void *)dst, (void *)tmp, NULL, n_indices) ;
						IDirect3DVertexBuffer8_Unlock(obj->d3d_cvbuff) ;
						/*------------------------------------------------------*/
					}
				}
				/*--------------------------------------------------------------*/
			}
			/*------------------------------------------------------------------*/
		}
	}
	/*--------------------------------------------------------------------------*/

	/*-- Vertex Buffer取得 -----------------------------------------------------*/

	DG_GetDGMdlVertexBuffer(mdl, &vbuff_tbl) ;
			
	obj->d3d_vbuff  = vbuff_tbl.vbuff ;
	obj->d3d_ibuff  = vbuff_tbl.ibuff ;

	if( vbuff_tbl.flag & DG_MDL_VERTEX_BUFFER_TBL_ORDERED_INDEX )
	{
		obj->wflag |= DG_OBJ_WFLAG_ORDERED_INDEX ;
	}
	else
	{
		obj->wflag &= ~DG_OBJ_WFLAG_ORDERED_INDEX ;
	}
	/*--------------------------------------------------------------------------*/
}

/*----------------------------------------------------------------*/
static void ObjReleaseVertexBuffer( DG_OBJ *obj )
{
	DG_ExeBufferSync(FALSE) ;	// 安全策

	/*-- 専用データの解放 ------------------------------------------------------*/

	if( obj->wflag & DG_OBJ_WFLAG_USE_PRIVATE_VBUFF )
	{
		DG_ReleaseDGObjPrivateVertexBuffer(obj) ;
	}
	/*--------------------------------------------------------------------------*/

	if( obj->d3d_cvbuff ){ DG_ReleaseD3DVertexBuffer(obj->d3d_cvbuff) ; }

	obj->d3d_vbuff  = NULL ;
	obj->d3d_cvbuff = NULL ;
	obj->d3d_ibuff  = NULL ;

	obj->cvbuff_size = 0 ;
}


/*----------------------------------------------------------------*/
static int SetMultiTexType( DG_MDLPACK *mdlpack )
{
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
	int		i, j, k, n_models ;
#ifdef _WINDOWS
	int		n_verts ;
	int		n_indices ;
#endif

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
	mdl = (DG_MDL *)def->models ;
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
		obj->n_verts   = 0 ;					/* 頂点数 */
		obj->n_indices = 0 ;					/* 頂点インデックス数 */
		obj->wflag     = 0 ;
		if ( obj->n_packs == 0 ){
			printf("warning: null packet\n");
			obj ++ ;
			mdl ++ ;
			continue ;
		}
		/* 頂点情報のポインタ設定 */
		obj->verts = (SVECTOR*)mdl->packs->verts ;
		obj->norms = (SVECTOR*)mdl->packs->norms ;
		obj->uvs[0] = (TVECTOR_S *)mdl->packs->uvs[0] ;
		obj->uvs[1] = (TVECTOR_S *)mdl->packs->uvs[1] ;
		obj->uvs[2] = (TVECTOR_S *)mdl->packs->uvs[2] ;
		//printf("%08x %08x %08x %08x %08x \n", obj->verts, obj->norms, obj->uvs[0], obj->uvs[1], obj->uvs[2] );
		/* 各パケット情報の設定 */
		obj->packets = pack = GV_Malloc( sizeof(DG_OBJ_PACKET) * obj->n_packs );
		if ( pack == NULL ){
			printf("create objs error!!\n");
			DG_FreeObjs( objs );
			return ( NULL );
		}
#ifdef _WINDOWS
		n_indices = 0 ;
		n_verts   = 0 ;
#endif
		GV_ZeroMemory( pack, sizeof(DG_OBJ_PACKET) * obj->n_packs );
		mdlpack = mdl->packs ;
		for ( j = (int)mdl->n_packs; j > 0 ; j-- ){
#ifdef PSX2			
#ifdef DEBUG_MODE
			if ( mdlpack->n_verts > 64 ) printf("too many n_verts(%d)\n",mdlpack->n_verts);
#endif
#endif			
			if ( obj->uvs[0] == NULL && mdlpack->uvs[0] != NULL ) obj->uvs[0] = (TVECTOR_S *)mdlpack->uvs[0] ;
			if ( obj->uvs[1] == NULL && mdlpack->uvs[1] != NULL ) obj->uvs[1] = (TVECTOR_S *)mdlpack->uvs[1] ;
			if ( obj->uvs[2] == NULL && mdlpack->uvs[2] != NULL ) obj->uvs[2] = (TVECTOR_S *)mdlpack->uvs[2] ;
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

#ifdef _WINDOWS
			/*-- 最大/最小参照頂点計算 -----------------------------------------*/

			if( mdlpack->n_indices )
			{

				WORD	idx_vtx_min ;
				WORD	idx_vtx_max ;
				WORD	*idx_ptr ;
				WORD	idx_val ;

				idx_ptr = (WORD *)obj->index + n_indices ;
				idx_val = *idx_ptr ;
				idx_vtx_min = idx_val ;
				idx_vtx_max = idx_val ;

				idx_ptr++ ;
				for(k=mdlpack->n_indices-1; k>0; k--, idx_ptr++)
				{
					idx_val = *idx_ptr ;
					if(      idx_vtx_max < idx_val ){ idx_vtx_max = idx_val ; }
					else if( idx_vtx_min > idx_val ){ idx_vtx_min = idx_val ; }
				}
				idx_vtx_max++ ;	// 補正

				pack->idx_vtx_min = idx_vtx_min ;
				pack->idx_vtx_num = idx_vtx_max - idx_vtx_min ;
				
				if( n_verts < idx_vtx_max ){ n_verts = idx_vtx_max  ; }
			}
			/*------------------------------------------------------------------*/

			n_indices += mdlpack->n_indices ;
#endif
			pack++ ;
			mdlpack++ ;
		}
#ifdef _WINDOWS
		obj->n_verts   = n_verts ;		/* 頂点数 */
		obj->n_indices = n_indices ;	/* 頂点インデックス数 */

		/*-- Direct3D Vertex Buffer作成 ----------------------------------------*/

		ObjSetVertexBuffer(obj, obj->model) ;
		/*----------------------------------------------------------------------*/

#endif

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
	int		i, j, k, n_models ;
#ifdef _WINDOWS
	int		n_verts ;
	int		n_indices ;
	DWORD	multi_tex_max ;
#endif

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
	mdl = (DG_MDL *)def->models ;
	obj = objs->objs ;

#ifdef _WINDOWS
	multi_tex_max = DG_GetMultiTexMax() ;
#endif

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
		obj->n_verts   = 0 ;					/* 頂点数 */
		obj->n_indices = 0 ;					/* 頂点インデックス数 */
		obj->wflag     = 0 ;
		//obj->pre_packet = obj->model->pre_packet ;
		/* 頂点情報のポインタ設定 */
		obj->verts = (SVECTOR*)obj->model->packs->verts ;
		obj->norms = (SVECTOR*)obj->model->packs->norms ;
		obj->uvs[0] = (TVECTOR_S *)obj->model->packs->uvs[0] ;
		obj->uvs[1] = (TVECTOR_S *)obj->model->packs->uvs[1] ;
		obj->uvs[2] = (TVECTOR_S *)obj->model->packs->uvs[2] ;
		/* 各パケット情報の設定 */
		obj->packets = pack = GV_Malloc( sizeof(DG_OBJ_PACKET) * obj->n_packs );
		if ( pack == NULL ){
			printf("create objs error!!\n");
			DG_FreeObjs( objs );
			return ( NULL );
		}
#ifdef _WINDOWS
		n_indices = 0 ;
		n_verts   = 0 ;
#endif
		GV_ZeroMemory( pack, sizeof(DG_OBJ_PACKET) * obj->n_packs );
		mdlpack = obj->model->packs ;
		for ( j = 0 ; j < obj->n_packs ; j++ ){
			if ( obj->uvs[0] == NULL && mdlpack->uvs[0] != NULL ) obj->uvs[0] = (TVECTOR_S *)mdlpack->uvs[0] ;
			if ( obj->uvs[1] == NULL && mdlpack->uvs[1] != NULL ) obj->uvs[1] = (TVECTOR_S *)mdlpack->uvs[1] ;
			if ( obj->uvs[2] == NULL && mdlpack->uvs[2] != NULL ) obj->uvs[2] = (TVECTOR_S *)mdlpack->uvs[2] ;
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
#ifndef _WINDOWS
			if ( mdlpack->tex_id[0] != 0 ) pack->tex_ptr[0] = &( (DG_TEX*)mdlpack->tex_id[0] )->tex_trans ;
			if ( mdlpack->tex_id[1] != 0 ) pack->tex_ptr[1] = &( (DG_TEX*)mdlpack->tex_id[1] )->tex_trans ;
			if ( mdlpack->tex_id[2] != 0 ) pack->tex_ptr[2] = &( (DG_TEX*)mdlpack->tex_id[2] )->tex_trans ;
#else
			if( mdlpack->tex_id[0] )
			{
				pack->tex_ptr[0] = &( (DG_TEX*)mdlpack->tex_id[0] )->tex_trans ;
			}
			if( mdlpack->tex_id[1] && (multi_tex_max >= 2) )
			{
				pack->tex_ptr[1] = &( (DG_TEX*)mdlpack->tex_id[1] )->tex_trans ;
			}
			if( mdlpack->tex_id[2] && (multi_tex_max >= 3) )
			{
				pack->tex_ptr[2] = &( (DG_TEX*)mdlpack->tex_id[2] )->tex_trans ;
			}
#endif
			pack->mtex_type = SetMultiTexType( mdlpack );
#ifdef _WINDOWS
			/*-- 最大/最小参照頂点計算 -----------------------------------------*/

			if( mdlpack->n_indices )
			{

				WORD	idx_vtx_min ;
				WORD	idx_vtx_max ;
				WORD	*idx_ptr ;
				WORD	idx_val ;

				idx_ptr = (WORD *)obj->index + n_indices ;
				idx_val = *idx_ptr ;
				idx_vtx_min = idx_val ;
				idx_vtx_max = idx_val ;

				idx_ptr++ ;
				for(k=mdlpack->n_indices-1; k>0; k--, idx_ptr++)
				{
					idx_val = *idx_ptr ;
					if(      idx_vtx_max < idx_val ){ idx_vtx_max = idx_val ; }
					else if( idx_vtx_min > idx_val ){ idx_vtx_min = idx_val ; }
				}
				idx_vtx_max++ ;	// 補正

				pack->idx_vtx_min = idx_vtx_min ;
				pack->idx_vtx_num = idx_vtx_max - idx_vtx_min ;
				
				if( n_verts < idx_vtx_max ){ n_verts = idx_vtx_max  ; }
			}
			/*------------------------------------------------------------------*/

			n_indices += mdlpack->n_indices ;
#endif
			pack++ ;
			mdlpack++ ;
		}
#ifdef _WINDOWS
		obj->n_verts   = n_verts ;		/* 頂点数 */
		obj->n_indices = n_indices ;	/* 頂点インデックス数 */

		/*-- Direct3D Vertex Buffer作成 ----------------------------------------*/

		ObjSetVertexBuffer(obj, obj->model) ;
		/*----------------------------------------------------------------------*/
#endif

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
	int		i, j ;
	DG_OBJ_PACKET	*pack ;
	DG_MDLPACK		*mdlpack ;
#ifdef _WINDOWS
	int		n_verts ;
	int		n_indices ;
	DWORD	multi_tex_max ;
#endif
	/* プリシェード型オブジェクトには未対応なので注意 */
	ASSERT( !( obj->flag & DG_FLAG_PAINT ) );

	/* メモリ確保 */
	pack = GV_Malloc( sizeof(DG_OBJ_PACKET) * mdl->n_packs );
	if ( pack == NULL ){
		printf("obj change error\n");
		return ;
	}

	/* メモリ開放 */
	DG_FreeObj( obj );
	/* DG_OBJの再設定 */
	obj->model = mdl ;
	obj->n_packs = mdl->n_packs ;
	obj->packets = pack ;
	obj->verts = (SVECTOR*)mdl->packs->verts ;
	obj->norms = (SVECTOR*)mdl->packs->norms ;
	obj->uvs[0] = (TVECTOR_S *)mdl->packs->uvs[0] ;
	obj->uvs[1] = (TVECTOR_S *)mdl->packs->uvs[1] ;
	obj->uvs[2] = (TVECTOR_S *)mdl->packs->uvs[2] ;
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
	obj->n_verts   = 0 ;					/* 頂点数 */
	obj->n_indices = 0 ;					/* 頂点インデックス数 */
	obj->wflag     = 0 ;
	//obj->pre_packet = mdl->pre_packet ;
#ifdef _WINDOWS
	n_indices = 0 ;
	n_verts   = 0 ;
	multi_tex_max = DG_GetMultiTexMax() ;
#endif
	mdlpack = mdl->packs ;
	for ( i = 0 ; i < obj->n_packs ; i++ ){
		if ( obj->uvs[0] == NULL && mdlpack->uvs[0] != NULL ) obj->uvs[0] = (TVECTOR_S *)mdlpack->uvs[0] ;
		if ( obj->uvs[1] == NULL && mdlpack->uvs[1] != NULL ) obj->uvs[1] = (TVECTOR_S *)mdlpack->uvs[1] ;
		if ( obj->uvs[2] == NULL && mdlpack->uvs[2] != NULL ) obj->uvs[2] = (TVECTOR_S *)mdlpack->uvs[2] ;
		pack->flag = mdlpack->flag ;
		pack->n_indices = mdlpack->n_indices ;
		//pack->data_ptr = (void*)mdlpack->pad ;	/* これはもう必要ないかも */
		pack->n_verts = mdl->packs[i].n_verts ;
		pack->norms_offset = pack->verts_offset = ( mdlpack->n_verts + 1 ) / 2 ;
		pack->uvs_offset[2] = pack->uvs_offset[1] = pack->uvs_offset[0] = ( mdlpack->n_verts + 3 ) / 4 ;
		if ( !( pack->flag & DG_PACKFLAG_UV0 ) ) pack->uvs_offset[0] = 0 ;
		if ( !( pack->flag & DG_PACKFLAG_UV1 ) ) pack->uvs_offset[1] = 0 ;
		if ( !( pack->flag & DG_PACKFLAG_UV2 ) ) pack->uvs_offset[2] = 0 ;
#ifndef _WINDOWS
		if ( mdlpack->tex_id[0] != 0 )	pack->tex_ptr[0] = &( (DG_TEX*)mdlpack->tex_id[0] )->tex_trans ;
		else							pack->tex_ptr[0] = NULL ;
		if ( mdlpack->tex_id[1] != 0 )	pack->tex_ptr[1] = &( (DG_TEX*)mdlpack->tex_id[1] )->tex_trans ;
		else							pack->tex_ptr[1] = NULL ;
		if ( mdlpack->tex_id[2] != 0 )	pack->tex_ptr[2] = &( (DG_TEX*)mdlpack->tex_id[2] )->tex_trans ;
		else							pack->tex_ptr[2] = NULL ;
#else
		pack->tex_ptr[0] = NULL ;
		pack->tex_ptr[1] = NULL ;
		pack->tex_ptr[2] = NULL ;
		if( mdlpack->tex_id[0] )
		{
			pack->tex_ptr[0] = &( (DG_TEX*)mdlpack->tex_id[0] )->tex_trans ;
		}
		if( mdlpack->tex_id[1] && (multi_tex_max >= 2) )
		{
			pack->tex_ptr[1] = &( (DG_TEX*)mdlpack->tex_id[1] )->tex_trans ;
		}
		if( mdlpack->tex_id[2] && (multi_tex_max >= 3) )
		{
			pack->tex_ptr[2] = &( (DG_TEX*)mdlpack->tex_id[2] )->tex_trans ;
		}
#endif
		pack->mtex_type = SetMultiTexType( mdlpack );
#ifdef _WINDOWS
		/*-- 最大/最小参照頂点計算 -----------------------------------------*/

		if( mdlpack->n_indices )
		{
			WORD	idx_vtx_min ;
			WORD	idx_vtx_max ;
			WORD	*idx_ptr ;
			WORD	idx_val ;

			idx_ptr = (WORD *)obj->index + n_indices ;
			idx_val = *idx_ptr ;
			idx_vtx_min = idx_val ;
			idx_vtx_max = idx_val ;

			idx_ptr++ ;
			for(j=mdlpack->n_indices-1; j>0; j--, idx_ptr++)
			{
				idx_val = *idx_ptr ;
				if(      idx_vtx_max < idx_val ){ idx_vtx_max = idx_val ; }
				else if( idx_vtx_min > idx_val ){ idx_vtx_min = idx_val ; }
			}
			idx_vtx_max++ ;	// 補正

			pack->idx_vtx_min = idx_vtx_min ;
			pack->idx_vtx_num = idx_vtx_max - idx_vtx_min ;
			
			if( n_verts < idx_vtx_max ){ n_verts = idx_vtx_max  ; }
		}
		/*------------------------------------------------------------------*/

		n_indices += mdlpack->n_indices ;
#endif
		pack++ ;
		mdlpack++ ;
	}
#ifdef _WINDOWS
	obj->n_verts   = n_verts ;		/* 頂点数 */
	obj->n_indices = n_indices ;	/* 頂点インデックス数 */

	/*-- Direct3D Vertex Buffer作成 ----------------------------------------*/

	ObjSetVertexBuffer(obj, mdl) ;
	/*----------------------------------------------------------------------*/
#endif

}

void	DG_FreeObj(DG_OBJ *obj)
{
	if ( obj->packets != NULL ){
		GV_Free( obj->packets );
	}

	ObjReleaseVertexBuffer(obj) ;
}

void		DG_FreeObjs( DG_OBJS *objs )
{
	DG_OBJ		*obj ;
	int		i ;

	if ( objs == NULL ) return ;

	DG_ExeBufferSync(FALSE) ;	// 安全策

	DG_FreePreshade( objs );
	obj = objs->objs ;
	if ( !( objs->flag & DG_FLAG_DELAYED ) ){
		/* 通常開放処理 */
		for ( i = objs->n_models ; i > 0 ; -- i ) {
			if ( obj->packets != NULL ){
				GV_Free( obj->packets );
			}

			if( obj->d3d_cvbuff ){ DG_ReleaseD3DVertexBuffer(obj->d3d_cvbuff) ; }
			obj->d3d_vbuff  = NULL ;
			obj->d3d_cvbuff = NULL ;
			obj->d3d_ibuff  = NULL ;

			obj ++ ;
		}
		GV_Free( objs ) ;
	} else {
		/* １フレーム遅らせ開放 */
		for ( i = objs->n_models ; i > 0 ; -- i ) {
			if ( obj->packets != NULL ){
				GV_DelayedFree( obj->packets );
			}

			if( obj->d3d_cvbuff ){ DG_ReleaseD3DVertexBuffer(obj->d3d_cvbuff) ; }
			obj->d3d_vbuff  = NULL ;
			obj->d3d_cvbuff = NULL ;
			obj->d3d_ibuff  = NULL ;

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

/*----------------------------------------------------------------*/

typedef	struct	DGMdlVtxBuffElem_
{
	DWORD						id ;		// 識別子(Address)
	DG_MDL_VERTEX_BUFFER_TBL	tbl ;
} DGMdlVtxBuffElem ;

#define	DG_MDL_VTXBUFF_ELEM_MAX	(0x1000)		// 登録最大数(安全の為)

typedef	struct	DGMdlVtxBuffMan_
{
	int					top ;
	int					bottom ;

	int					elem_num ;
	DGMdlVtxBuffElem	elem[DG_MDL_VTXBUFF_ELEM_MAX] ;
} DGMdlVtxBuffMan ;

static	DGMdlVtxBuffMan	_dgmdl_vtxbuff_man ;

#define	getDGMdlID(mdl_)	(DWORD)(mdl_)

/*------------------------------------------------------------------------------*/
/*		DG_InitDGMdlVertexBufferManager											*/
/*			DG_MDL用Vertex Buffer管理初期化										*/
/*------------------------------------------------------------------------------*/
void	DG_InitDGMdlVertexBufferManager(void)
{
	//printf("DG_InitDGMdlVertexBufferManager()\n") ;

	ZeroMemory(&_dgmdl_vtxbuff_man, sizeof(DGMdlVtxBuffMan)) ;
}

/*------------------------------------------------------------------------------*/
/*		DG_ReleaseDGMdlVertexBufferManager										*/
/*			DG_MDL用Vertex Buffer管理終了処理									*/
/*------------------------------------------------------------------------------*/
void	DG_ReleaseDGMdlVertexBufferManager(void)
{
	int					i ;
	DGMdlVtxBuffElem	*elem ;

	//printf("DG_ReleaseDGMdlVertexBufferManager()\n") ;

	/*-- 登録されている物解放 --------------------------------------------------*/

	elem = _dgmdl_vtxbuff_man.elem ;
	for(i=DG_MDL_VTXBUFF_ELEM_MAX; i>0; i--, elem++)
	{
		if( elem->id )
		{
			if( elem->tbl.vbuff ){ DG_ReleaseD3DVertexBuffer(elem->tbl.vbuff) ; }
			if( elem->tbl.ibuff ){ DG_ReleaseD3DIndexBuffer(elem->tbl.ibuff) ; }
		}
	}
	/*--------------------------------------------------------------------------*/

	ZeroMemory(&_dgmdl_vtxbuff_man, sizeof(DGMdlVtxBuffMan)) ;
}

/*------------------------------------------------------------------------------*/
/*		DG_MakeDGMdlVertexBuffer												*/
/*			DG_MDL用Vertex Bufferを作成する										*/
/*------------------------------------------------------------------------------*/
static	DGMdlVtxBuffElem *_DG_MakeDGMdlVertexBuffer(DG_MDL *mdl) ;

static	void	DG_MakeDGMdlVertexBuffer_Normal(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl) ;
static	void	DG_MakeDGMdlVertexBuffer_NoIndex(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl) ;
static	void	DG_MakeDGMdlVertexBuffer_NoVertexShader(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl) ;
static	void	DG_MakeDGMdlVertexBuffer_NoVertexShaderNoIndex(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl) ;

static	void	DG_MakeDGMdlVertexBuffer_MTNoVertexShader(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl) ;
static	void	DG_MakeDGMdlVertexBuffer_MTNoVertexShaderNoIndex(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl) ;

void	DG_MakeDGMdlVertexBuffer(DG_MDL *mdl)
{
	_DG_MakeDGMdlVertexBuffer(mdl) ;
}

static	DGMdlVtxBuffElem *_DG_MakeDGMdlVertexBuffer(DG_MDL *mdl)
{
	int					i, k ;
	DGMdlVtxBuffElem	*elem ;
	DWORD				n_verts ;
	DWORD				n_indices ;

	//printf("DG_MakeDGMdlVertexBuffer(0x%08X)...Num %d\n", (DWORD)mdl,
	//				_dgmdl_vtxbuff_man.elem_num) ;

	/*-- 既に登録されていたら解放 ----------------------------------------------*/

	DG_ReleaseDGMdlVertexBuffer(mdl) ;
	/*--------------------------------------------------------------------------*/

	/*-- 登録不可能判定 --------------------------------------------------------*/

	if( _dgmdl_vtxbuff_man.elem_num >= DG_MDL_VTXBUFF_ELEM_MAX )
	{
#ifdef DEBUG_MODE
		printf("[ Fatal Warning ] DG_MakeDGMdlVertexBuffer Faild!!\n") ;
		HANGUP() ;
#endif
		return(NULL) ;
	}
	/*--------------------------------------------------------------------------*/

	/*-- 空いているバッファを検索する ------------------------------------------*/

	elem = _dgmdl_vtxbuff_man.elem ;
	for(i=DG_MDL_VTXBUFF_ELEM_MAX; i>0; i--, elem++)
	{
		if( !elem->id ){ break ; }	// 検索Hit!
	}
	ASSERT( i ) ;

	i = DG_MDL_VTXBUFF_ELEM_MAX - i ;	// 要素番号に変換
	/*--------------------------------------------------------------------------*/

	/*-- 管理情報更新 ----------------------------------------------------------*/

	_dgmdl_vtxbuff_man.elem_num++ ;
	if(      _dgmdl_vtxbuff_man.top    > i   ){ _dgmdl_vtxbuff_man.top    = i ; }
	else if( _dgmdl_vtxbuff_man.bottom < i+1 ){ _dgmdl_vtxbuff_man.bottom = i+1 ; }
	/*--------------------------------------------------------------------------*/

	/*-- 要素設定 --------------------------------------------------------------*/

	elem->id = getDGMdlID(mdl) ;		// アドレスをIDとして使用

	{
		DG_MDLPACK	*mdlpack ;
		WORD		idx_vtx_max ;
		WORD		*idx_ptr ;
		WORD		*idx_root ;
		WORD		idx_val ;
		void		(*create_func)(DG_MDL*, DG_MDL_VERTEX_BUFFER_TBL*) ;

		n_verts   = 0 ;
		n_indices = 0 ;
		mdlpack = mdl->packs ;
		idx_root = (WORD *)mdl->packs->index ;

		for(i=mdl->n_packs; i>0; i--, mdlpack++)
		{
			/*-- 最大/最小参照頂点計算 -----------------------------------------*/

			if( mdlpack->n_indices )
			{

				idx_ptr = idx_root + n_indices ;
				idx_val = *idx_ptr ;
				idx_vtx_max = idx_val ;

				idx_ptr++ ;
				for(k=mdlpack->n_indices-1; k>0; k--, idx_ptr++)
				{
					idx_val = *idx_ptr ;
					if(idx_vtx_max < idx_val ){ idx_vtx_max = idx_val ; }
				}
				idx_vtx_max++ ;	// 補正

				if( n_verts < idx_vtx_max ){ n_verts = idx_vtx_max  ; }
			}
			/*------------------------------------------------------------------*/

			n_indices += mdlpack->n_indices ;
		}

		elem->tbl.vbuff = NULL ;
		elem->tbl.ibuff = NULL ;
#ifdef DEBUG_MODE
		elem->tbl.vbuff_size = 0 ;
		elem->tbl.ibuff_size = 0 ;
#endif

		/*-- Vertex Buffer作成関数の選定 ---------------------------------------*/

		if( DG_CheckObjUseVertexShader() )
		{
			if( DG_CheckIndexPrimitiveUseable() )
			{
				create_func = DG_MakeDGMdlVertexBuffer_Normal ;
			}
			else
			{
				create_func = DG_MakeDGMdlVertexBuffer_NoIndex ;
			}
		}
		else
		{
			if( mdl->stride == sizeof(DG_VERTEX_KMSS) )
			{
				/*-- KMSS形式 ----------------------------------------------*/

				if( DG_CheckIndexPrimitiveUseable() )
				{
					create_func = DG_MakeDGMdlVertexBuffer_NoVertexShader ;
				}
				else
				{
					create_func = DG_MakeDGMdlVertexBuffer_NoVertexShaderNoIndex ;
				}
				/*--------------------------------------------------------------*/
			}
			else
			{
				/*-- KMSM形式 --------------------------------------------------*/

				if( DG_CheckIndexPrimitiveUseable() )
				{
					create_func = DG_MakeDGMdlVertexBuffer_MTNoVertexShader ;
				}
				else
				{
					create_func = DG_MakeDGMdlVertexBuffer_MTNoVertexShaderNoIndex ;
				}
				/*--------------------------------------------------------------*/
			}
		}
		/*----------------------------------------------------------------------*/

		/*-- Vertex Buffer作成 -------------------------------------------------*/

		elem->tbl.n_verts   = n_verts ;
		elem->tbl.n_indices = n_indices ;
		if( (n_verts > 0) && (n_indices > 0) )
		{
			create_func(mdl, &elem->tbl) ;	// Vertex Buffer作成
		}
		/*----------------------------------------------------------------------*/
	}
	/*--------------------------------------------------------------------------*/

	/*-- Index状態検査 ---------------------------------------------------------*/

	{
		WORD	*index ;
		BOOL	chk ;

		chk = TRUE ;
		index = (WORD *)mdl->packs->index ;
		for(i=0; i<n_indices; i++, index++)
		{
			if( i != *index )
			{
				chk = FALSE ;
				break ;
			}
		}

		if( chk ){ elem->tbl.flag |= DG_MDL_VERTEX_BUFFER_TBL_ORDERED_INDEX ; }
		else{ elem->tbl.flag &= ~DG_MDL_VERTEX_BUFFER_TBL_ORDERED_INDEX ; }
	}
	/*--------------------------------------------------------------------------*/
	//printf("	-->(0x%08X, 0x%08X)\n", (DWORD)elem->tbl.vbuff, (DWORD)elem->tbl.ibuff) ;

	return( elem ) ;
}

	/*------------------------------------------------------------------------------*/
	/*		DG_MakeDGMdlVertexBuffer_Normal		通常版								*/
	/*------------------------------------------------------------------------------*/
static	void	DG_MakeDGMdlVertexBuffer_Normal(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl)
{
	DWORD	stride ;
	DWORD	n_verts ;
	DWORD	n_indices ;
	DWORD	size ;
	BYTE	*dst ;

	stride    = mdl->stride ;
	n_verts   = tbl->n_verts ;
	n_indices = tbl->n_indices ;

	/*-- Vertex Buffer 作成 --------------------------------------------------------*/

	DG_CreateVertexBuffer(stride * n_verts,
					DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY, 0, OBJ_VERTEXBUFFER_POOL,
					&tbl->vbuff) ;
	DG_CreateIndexBuffer(n_indices * sizeof(WORD),
					DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, OBJ_VERTEXBUFFER_POOL,
					&tbl->ibuff) ;
#ifdef DEBUG_MODE
	tbl->vbuff_size = stride * n_verts ;
	tbl->ibuff_size = n_indices * sizeof(WORD) ;
#endif
	/*------------------------------------------------------------------------------*/

	/*-- 内容複写 ------------------------------------------------------------------*/

	if( tbl->vbuff )
	{
		size = stride * n_verts ;
		IDirect3DVertexBuffer8_Lock(tbl->vbuff, 0, size, &dst,
								OBJ_VERTEXBUFFER_LOCKFLAG) ;
		memcpy(dst, mdl->vbuff, size) ;
		IDirect3DVertexBuffer8_Unlock(tbl->vbuff) ;
	}
	if( tbl->ibuff )
	{
		size = sizeof(WORD) * n_indices ;
		IDirect3DIndexBuffer8_Lock(tbl->ibuff, 0, size, &dst,
											OBJ_INDEXBUFFER_LOCKFLAG) ;
		memcpy(dst, mdl->packs->index, size) ;
		IDirect3DIndexBuffer8_Unlock(tbl->ibuff) ;
	}
	/*------------------------------------------------------------------------------*/
}

	/*------------------------------------------------------------------------------*/
	/*		DG_MakeDGMdlVertexBuffer_NoIndex	Index非対応版						*/
	/*------------------------------------------------------------------------------*/
static	void	DG_MakeDGMdlVertexBuffer_NoIndex(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl)
{
	DWORD	stride ;
	DWORD	n_indices ;
	DWORD	size ;
	BYTE	*dst ;

	stride    = mdl->stride ;
	n_indices = tbl->n_indices ;

	/*-- Vertex Buffer 作成 --------------------------------------------------------*/

	DG_CreateVertexBuffer(stride * n_indices,
						DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY, 0, OBJ_VERTEXBUFFER_POOL,
						&tbl->vbuff) ;
#ifdef DEBUG_MODE
	tbl->vbuff_size = stride * n_indices ;
#endif
	/*------------------------------------------------------------------------------*/

	/*-- 内容複写 ------------------------------------------------------------------*/

	if( tbl->vbuff )
	{
		size = stride * n_indices ;
		IDirect3DVertexBuffer8_Lock(tbl->vbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
		DG_ArrangeVertexByIndex(dst, mdl->vbuff, stride,
							mdl->packs->index, n_indices) ;
		IDirect3DVertexBuffer8_Unlock(tbl->vbuff) ;
	}
	/*------------------------------------------------------------------------------*/
}

	/*------------------------------------------------------------------------------*/
	/*		DG_MakeDGMdlVertexBuffer_NoVertexShader	VertexShader非対応版			*/
	/*------------------------------------------------------------------------------*/
static	void	DG_MakeDGMdlVertexBuffer_NoVertexShader(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl)
{
	DWORD	n_verts ;
	DWORD	n_indices ;
	DWORD	size ;
	BYTE	*dst ;

	n_verts   = tbl->n_verts ;
	n_indices = tbl->n_indices ;

	/*-- Vertex Buffer 作成 --------------------------------------------------------*/

	DG_CreateVertexBuffer(sizeof(DG_VERTEX_KMSS_FVF) * n_verts,
					DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY, 0, OBJ_VERTEXBUFFER_POOL,
					&tbl->vbuff) ;
	DG_CreateIndexBuffer(n_indices * sizeof(WORD),
					DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
					OBJ_VERTEXBUFFER_POOL,
					&tbl->ibuff) ;
#ifdef DEBUG_MODE
	tbl->vbuff_size = sizeof(DG_VERTEX_KMSS_FVF) * n_verts ;
	tbl->ibuff_size = n_indices * sizeof(WORD) ;
#endif
	/*------------------------------------------------------------------------------*/

	/*-- 内容複写 ------------------------------------------------------------------*/

	if( tbl->vbuff )
	{
		size = sizeof(DG_VERTEX_KMSS_FVF) * n_verts ;
		IDirect3DVertexBuffer8_Lock(tbl->vbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
		DG_ConvertKmss2KmssFVF((void *)dst, (void *)mdl->vbuff, NULL, n_verts) ;
		IDirect3DVertexBuffer8_Unlock(tbl->vbuff) ;
	}
	if( tbl->ibuff )
	{
		size = sizeof(WORD) * n_indices ;
		IDirect3DIndexBuffer8_Lock(tbl->ibuff, 0, size, &dst, OBJ_INDEXBUFFER_LOCKFLAG) ;
		memcpy(dst, mdl->packs->index, size) ;
		IDirect3DIndexBuffer8_Unlock(tbl->ibuff) ;
	}
	/*------------------------------------------------------------------------------*/
}

	/*------------------------------------------------------------------------------*/
	/*		DG_MakeDGMdlVertexBuffer_NoVertexShaderNoIndex	VertexShader/Index非対応版*/
	/*------------------------------------------------------------------------------*/
static	void	DG_MakeDGMdlVertexBuffer_NoVertexShaderNoIndex(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl)
{
	DWORD	n_indices ;
	DWORD	size ;
	BYTE	*dst ;

	n_indices = tbl->n_indices ;

	/*-- Vertex Buffer 作成 --------------------------------------------------------*/

	DG_CreateVertexBuffer(sizeof(DG_VERTEX_KMSS_FVF) * n_indices,
						DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY, 0, OBJ_VERTEXBUFFER_POOL,
						&tbl->vbuff) ;
#ifdef DEBUG_MODE
	tbl->vbuff_size = sizeof(DG_VERTEX_KMSS_FVF) * n_indices ;
#endif
	/*------------------------------------------------------------------------------*/

	/*-- 内容複写 ------------------------------------------------------------------*/

	if( tbl->vbuff )
	{
		BYTE	*tmp ;

		tmp =(void *)DG_GetWorkBuffer() ;

		DG_ConvertKmss2KmssFVF((void *)tmp, (void *)mdl->vbuff, NULL, tbl->n_verts) ;

		size = sizeof(DG_VERTEX_KMSS_FVF) * n_indices ;
		IDirect3DVertexBuffer8_Lock(tbl->vbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
		DG_ArrangeVertexByIndex(dst, tmp, sizeof(DG_VERTEX_KMSS_FVF),
							mdl->packs->index, n_indices) ;
		IDirect3DVertexBuffer8_Unlock(tbl->vbuff) ;
	}
	/*------------------------------------------------------------------------------*/
}

	/*------------------------------------------------------------------------------*/
	/*		DG_MakeDGMdlVertexBuffer_MTNoVertexShader	VertexShader非対応版(KMSM)	*/
	/*------------------------------------------------------------------------------*/
static	void	DG_MakeDGMdlVertexBuffer_MTNoVertexShader(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl)
{
	DWORD	n_verts ;
	DWORD	n_indices ;
	DWORD	size ;
	BYTE	*dst ;

	n_verts   = tbl->n_verts ;
	n_indices = tbl->n_indices ;

	/*-- Vertex Buffer 作成 --------------------------------------------------------*/

	DG_CreateVertexBuffer(sizeof(DG_VERTEX_KMSM_FVF) * n_verts,
					DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY, 0, OBJ_VERTEXBUFFER_POOL,
					&tbl->vbuff) ;
	DG_CreateIndexBuffer(n_indices * sizeof(WORD),
					DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, OBJ_VERTEXBUFFER_POOL,
					&tbl->ibuff) ;
#ifdef DEBUG_MODE
	tbl->vbuff_size = sizeof(DG_VERTEX_KMSM_FVF) * n_verts ;
	tbl->ibuff_size = n_indices * sizeof(WORD) ;
#endif
	/*------------------------------------------------------------------------------*/

	/*-- 内容複写 ------------------------------------------------------------------*/

	if( tbl->vbuff )
	{
		size = sizeof(DG_VERTEX_KMSM_FVF) * n_verts ;
		IDirect3DVertexBuffer8_Lock(tbl->vbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
		DG_ConvertKmsm2KmsmFVF((void *)dst, (void *)mdl->vbuff, NULL, n_verts) ;
		IDirect3DVertexBuffer8_Unlock(tbl->vbuff) ;
	}
	if( tbl->ibuff )
	{
		size = sizeof(WORD) * n_indices ;
		IDirect3DIndexBuffer8_Lock(tbl->ibuff, 0, size, &dst, OBJ_INDEXBUFFER_LOCKFLAG) ;
		memcpy(dst, mdl->packs->index, size) ;
		IDirect3DIndexBuffer8_Unlock(tbl->ibuff) ;
	}
	/*------------------------------------------------------------------------------*/
}

	/*------------------------------------------------------------------------------*/
	/*		DG_MakeDGMdlVertexBuffer_MTNoVertexShaderNoIndex						*/
	/*				VertexShader/Index非対応版(KMSM)								*/
	/*------------------------------------------------------------------------------*/
static	void	DG_MakeDGMdlVertexBuffer_MTNoVertexShaderNoIndex(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl)
{
	DWORD	n_indices ;
	DWORD	size ;
	BYTE	*dst ;

	n_indices = tbl->n_indices ;

	/*-- Vertex Buffer 作成 --------------------------------------------------------*/

	DG_CreateVertexBuffer(sizeof(DG_VERTEX_KMSM_FVF) * n_indices,
						DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY, 0, OBJ_VERTEXBUFFER_POOL,
						&tbl->vbuff) ;
#ifdef DEBUG_MODE
	tbl->vbuff_size = sizeof(DG_VERTEX_KMSM_FVF) * n_indices ;
#endif
	/*------------------------------------------------------------------------------*/

	/*-- 内容複写 ------------------------------------------------------------------*/

	if( tbl->vbuff )
	{
		BYTE	*tmp ;

		tmp =(void *)DG_GetWorkBuffer() ;

		DG_ConvertKmsm2KmsmFVF((void *)tmp, (void *)mdl->vbuff, NULL, tbl->n_verts) ;

		size = sizeof(DG_VERTEX_KMSM_FVF) * n_indices ;
		IDirect3DVertexBuffer8_Lock(tbl->vbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
		DG_ArrangeVertexByIndex(dst, tmp, sizeof(DG_VERTEX_KMSM_FVF),
							mdl->packs->index, n_indices) ;
		IDirect3DVertexBuffer8_Unlock(tbl->vbuff) ;
	}
	/*------------------------------------------------------------------------------*/
}

/*------------------------------------------------------------------------------*/
/*		DG_ReleaseDGMdlVertexBuffer												*/
/*			DG_MDL用Vertex Bufferを解放する										*/
/*------------------------------------------------------------------------------*/
void	DG_ReleaseDGMdlVertexBuffer(DG_MDL *mdl)
{
	int					i ;
	DGMdlVtxBuffElem	*elem ;
	int					top, bottom ;

	//printf("DG_ReleaseDGMdlVertexBuffer(0x%08X)\n", (DWORD)mdl) ;

	if( !_dgmdl_vtxbuff_man.elem_num ){ return ; }

	/*-- 空いているバッファを検索する ------------------------------------------*/

	top    = _dgmdl_vtxbuff_man.top ;
	bottom = _dgmdl_vtxbuff_man.bottom ;
	elem = &_dgmdl_vtxbuff_man.elem[top] ;
	for(i=top; i<bottom; i++, elem++)
	{
		if( elem->id == getDGMdlID(mdl) ){ break ; }	// 検索Hit!
	}
	if( i == bottom ){ return ; }
	/*--------------------------------------------------------------------------*/

	/*-- バッファ解放 ----------------------------------------------------------*/

	if( elem->tbl.vbuff  ){ DG_ReleaseD3DVertexBuffer(elem->tbl.vbuff) ; }
	if( elem->tbl.ibuff  ){ DG_ReleaseD3DIndexBuffer(elem->tbl.ibuff) ; }
	elem->id = 0 ;
	/*--------------------------------------------------------------------------*/


	/*-- 管理情報更新 ----------------------------------------------------------*/

	_dgmdl_vtxbuff_man.elem_num-- ;
	if( _dgmdl_vtxbuff_man.elem_num == 0 )
	{
		_dgmdl_vtxbuff_man.top    = 0 ;
		_dgmdl_vtxbuff_man.bottom = 0 ;
	}
	else
	{
		/*-- TOP更新 -----------------------------------------------------------*/
		elem = &_dgmdl_vtxbuff_man.elem[top] ;
		for(i=top; i<bottom; i++, elem++){ if( elem->id ){ break ; } }
		top = i ;
		_dgmdl_vtxbuff_man.top = i ;
		/*----------------------------------------------------------------------*/

		/*-- BOTTOM更新 --------------------------------------------------------*/

		elem = &_dgmdl_vtxbuff_man.elem[bottom-1] ;
		for(i=bottom-1; i>top; i--, elem--){ if( elem->id ){ break ; } }
		bottom = i+1 ;
		_dgmdl_vtxbuff_man.bottom = i+1 ;
		/*----------------------------------------------------------------------*/
	}
	/*--------------------------------------------------------------------------*/
}

/*------------------------------------------------------------------------------*/
/*		DG_ReleaseDGMdlVertexBufferAll											*/
/*			DG_MDL用Vertex Bufferを全て解放する									*/
/*------------------------------------------------------------------------------*/
void	DG_ReleaseDGMdlVertexBufferAll(void)
{
	int					i ;
	DGMdlVtxBuffElem	*elem ;

	//printf("DG_ReleaseDGMdlVertexBufferAll()\n") ;

	DG_ExeBufferSync(FALSE) ;	// 安全策

	/*-- 登録されている物解放 --------------------------------------------------*/

	elem = _dgmdl_vtxbuff_man.elem ;
	for(i=DG_MDL_VTXBUFF_ELEM_MAX; i>0; i--, elem++)
	{
		if( elem->id )
		{
			if( elem->tbl.vbuff ){ DG_ReleaseD3DVertexBuffer(elem->tbl.vbuff) ; }
			if( elem->tbl.ibuff ){ DG_ReleaseD3DIndexBuffer(elem->tbl.ibuff) ; }
		}
	}
	/*--------------------------------------------------------------------------*/

	ZeroMemory(&_dgmdl_vtxbuff_man, sizeof(DGMdlVtxBuffMan)) ;
}

/*------------------------------------------------------------------------------*/
/*		DG_GetDGMdlVertexBuffer													*/
/*			DG_MDL用Vertex Bufferを取得する										*/
/*------------------------------------------------------------------------------*/
static DG_MDL_VERTEX_BUFFER_TBL	_dgmdl_vtxbuff_null_tbl = { NULL, NULL } ;

void	DG_GetDGMdlVertexBuffer(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl)
{
	int					i ;
	DGMdlVtxBuffElem	*elem ;
	int					top, bottom ;

	//printf("DG_GetDGMdlVertexBuffer(0x%08X)\n", (DWORD)mdl) ;

#if FALSE
	if( !_dgmdl_vtxbuff_man.elem_num )
	{
		ASSERT(0) ;
		*tbl = _dgmdl_vtxbuff_null_tbl ;	// Dummy
		return ;
	}
#endif

	/*-- バッファ検索する ------------------------------------------------------*/

	top    = _dgmdl_vtxbuff_man.top ;
	bottom = _dgmdl_vtxbuff_man.bottom ;
	elem   = &_dgmdl_vtxbuff_man.elem[top] ;
	for(i=top; i<bottom; i++, elem++)
	{
		if( elem->id == getDGMdlID(mdl) ){ break ; }	// 検索Hit!
	}

	if( i == bottom )
	{
		elem = _DG_MakeDGMdlVertexBuffer(mdl) ;		// 無ければ作る
		if( !elem )
		{
			*tbl = _dgmdl_vtxbuff_null_tbl ;	// Dummy
			return ;	// 新規に作れなかった...
		}
	}
	/*--------------------------------------------------------------------------*/

	/*-- 正常終了 --------------------------------------------------------------*/

	ASSERT( elem->tbl.n_verts && elem->tbl.n_indices ) ;
	*tbl = elem->tbl ;
	/*--------------------------------------------------------------------------*/

}

/*------------------------------------------------------------------------------*/
/*		DG_GetDGMdlVertexBuffer2												*/
/*			DG_MDL用Vertex Bufferを取得する(新規作成はしない)					*/
/*------------------------------------------------------------------------------*/

BOOL	DG_GetDGMdlVertexBuffer2(DG_MDL *mdl, DG_MDL_VERTEX_BUFFER_TBL *tbl)
{
	int					i ;
	DGMdlVtxBuffElem	*elem ;
	int					top, bottom ;

	/*-- バッファ検索する ------------------------------------------------------*/

	top    = _dgmdl_vtxbuff_man.top ;
	bottom = _dgmdl_vtxbuff_man.bottom ;
	elem   = &_dgmdl_vtxbuff_man.elem[top] ;
	for(i=top; i<bottom; i++, elem++)
	{
		if( elem->id == getDGMdlID(mdl) ){ break ; }	// 検索Hit!
	}

	if( i == bottom ){ return(FALSE) ; }
	/*--------------------------------------------------------------------------*/

	/*-- 正常終了 --------------------------------------------------------------*/

	ASSERT( elem->tbl.n_verts && elem->tbl.n_indices ) ;
	*tbl = elem->tbl ;
	return(TRUE) ;
	/*--------------------------------------------------------------------------*/

}

/*------------------------------------------------------------------------------*/
/*		DG_CheckDGMdlVertexBuffer												*/
/*			指定されたVertexBufferが存在するか検査								*/
/*------------------------------------------------------------------------------*/
BOOL	DG_CheckDGMdlVertexBuffer(LPDIRECT3DVERTEXBUFFER8 pVertexBuffer)
{
	int					i ;
	DGMdlVtxBuffElem	*elem ;
	int					top, bottom ;
	BOOL				chk ;

	/*-- バッファ検索する ------------------------------------------------------*/

	chk = FALSE ;

	top    = _dgmdl_vtxbuff_man.top ;
	bottom = _dgmdl_vtxbuff_man.bottom ;
	elem   = &_dgmdl_vtxbuff_man.elem[top] ;
	for(i=top; i<bottom; i++, elem++)
	{
		if( !elem->id ){ continue ; }
		if( elem->tbl.vbuff == pVertexBuffer )
		{
			chk = TRUE ;
			break ;	// 検索Hit!
		}
	}
	/*--------------------------------------------------------------------------*/

	return(chk) ;
}

/*----------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
/*		DG_AssignDGObjVertexBuffer												*/
/*			Vertex Buffer変更通達												*/
/*------------------------------------------------------------------------------*/
void DG_AssignDGObjVertexBuffer(DG_OBJ *obj)
{
	obj->vbuff_dirty = TRUE ;	// フラグON
}

/*------------------------------------------------------------------------------*/
/*		DG_FlushDGObjVertexBuffer												*/
/*			Vertex Buffer変更の適用												*/
/*------------------------------------------------------------------------------*/
static inline	void DG_FlushDGObjVertexBuffer_Idx(DG_OBJ *obj) ;
static inline	void DG_FlushDGObjVertexBuffer_NIdx(DG_OBJ *obj) ;

static inline	void DG_FlushDGObjVertexBuffer_Idx(DG_OBJ *obj)
{
	DWORD	size ;
	BYTE	*dst ;
	DWORD	stride ;
	DWORD	n_verts ;

	/*-- Index対応版 -----------------------------------------------------------*/

	if( obj->d3d_vbuff )
	{
		stride  = obj->stride ;
		n_verts = obj->n_verts ;
		size    = stride * n_verts ;

		IDirect3DVertexBuffer8_Lock(obj->d3d_vbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
		memcpy(dst, obj->vbuff, size) ;
		IDirect3DVertexBuffer8_Unlock(obj->d3d_vbuff) ;
	}
	/*--------------------------------------------------------------------------*/
}


static inline	void DG_FlushDGObjVertexBuffer_NIdx(DG_OBJ *obj)
{
	DWORD	size ;
	BYTE	*dst ;
	DWORD	stride ;
	DWORD	n_indices ;

	/*-- Index非対応版 ---------------------------------------------------------*/

	if( obj->d3d_vbuff )
	{
		stride    = obj->stride ;
		n_indices = obj->n_indices ;
		size      = stride * n_indices ;

		IDirect3DVertexBuffer8_Lock(obj->d3d_vbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
		if( obj->wflag & DG_OBJ_WFLAG_ORDERED_INDEX )
		{
			memcpy(dst, obj->vbuff, size) ;
		}
		else
		{
			DG_ArrangeVertexByIndex(dst, obj->vbuff, stride,
								obj->index, n_indices) ;
		}
		IDirect3DVertexBuffer8_Unlock(obj->d3d_vbuff) ;
	}
	/*--------------------------------------------------------------------------*/
}

void DG_FlushDGObjVertexBuffer(DG_OBJ *obj)
{
	if( DG_CheckIndexPrimitiveUseable() )
	{
		DG_FlushDGObjVertexBuffer_Idx(obj) ;
	}
	else
	{
		DG_FlushDGObjVertexBuffer_NIdx(obj) ;
	}

	obj->vbuff_dirty = FALSE ;	// フラグOFF
}

/*------------------------------------------------------------------------------*/
/*		DG_FlushDGObjVertexBufferNVS											*/
/*			Vertex Buffer変更の適用(Vertex Shader非対応版)						*/
/*------------------------------------------------------------------------------*/
void DG_FlushDGObjVertexBufferNVS(DG_OBJ *obj)
{
	BYTE	*dst ;
	BYTE	*tmp ;
	DWORD	size ;
	LPDIRECT3DVERTEXBUFFER8	vbuff ;
	DWORD	n_verts ;
	DWORD	n_indices ;

	if( obj->flag & DG_FLAG_MULTITEX ) 
	{
		if( obj->rgbs ){ vbuff = obj->d3d_cvbuff ; }
		else{ vbuff = obj->d3d_vbuff ; }
	}
	else
	{
		if( obj->flag & DG_FLAG_PAINT ){ vbuff = obj->d3d_cvbuff ; }
		else{ vbuff = obj->d3d_vbuff ; }
	}

	if( vbuff )
	{
		if( DG_CheckIndexPrimitiveUseable() )
		{
			if( obj->stride == sizeof(DG_VERTEX_KMSS) )
			{
				n_verts = obj->n_verts ;
				size = sizeof(DG_VERTEX_KMSS_FVF) * n_verts ;
				IDirect3DVertexBuffer8_Lock(vbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
				DG_OvertwriteKmssVtx2KmssFVF((void *)dst, (void *)obj->vbuff, n_verts) ;
				IDirect3DVertexBuffer8_Unlock(vbuff) ;
			}
			else
			{
				n_verts = obj->n_verts ;
				size = sizeof(DG_VERTEX_KMSM_FVF) * n_verts ;
				IDirect3DVertexBuffer8_Lock(vbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
				DG_OvertwriteKmsmVtx2KmsmFVF((void *)dst, (void *)obj->vbuff, n_verts) ;
				IDirect3DVertexBuffer8_Unlock(vbuff) ;
			}
		}
		else
		{
			if( obj->stride == sizeof(DG_VERTEX_KMSS) )
			{
				n_indices = obj->n_indices ;
				size      = sizeof(DG_VERTEX_KMSS_FVF) * n_indices ;
				if( obj->wflag & DG_OBJ_WFLAG_ORDERED_INDEX )
				{
					tmp = obj->vbuff ;
				}
				else
				{
					tmp = DG_GetWorkBuffer() ;
					DG_ArrangeVertexByIndex(tmp, obj->vbuff, sizeof(DG_VERTEX_KMSS),
											obj->index, n_indices) ;
				}
				IDirect3DVertexBuffer8_Lock(vbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
				DG_OvertwriteKmssVtx2KmssFVF((void *)dst, (void *)tmp, n_indices) ;
				IDirect3DVertexBuffer8_Unlock(vbuff) ;
			}
			else
			{
				n_indices = obj->n_indices ;
				size      = sizeof(DG_VERTEX_KMSM_FVF) * n_indices ;
				if( obj->wflag & DG_OBJ_WFLAG_ORDERED_INDEX )
				{
					tmp = obj->vbuff ;
				}
				else
				{
					tmp = DG_GetWorkBuffer() ;
					DG_ArrangeVertexByIndex(tmp, obj->vbuff, sizeof(DG_VERTEX_KMSM),
											obj->index, n_indices) ;
				}

				IDirect3DVertexBuffer8_Lock(vbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
				DG_OvertwriteKmsmVtx2KmsmFVF((void *)dst, (void *)tmp, n_indices) ;
				IDirect3DVertexBuffer8_Unlock(vbuff) ;
			}	
		}
	}

	obj->vbuff_dirty = FALSE ;	// フラグOFF
}

/*------------------------------------------------------------------------------*/
/*		DG_AssignDGObjCVertexBuffer												*/
/*			Color Vertex Buffer変更の適用										*/
/*------------------------------------------------------------------------------*/
void DG_AssignDGObjCVertexBuffer(DG_OBJ *obj)
{
	obj->cvbuff_dirty = TRUE ;	// フラグON

	/*-- Color Bufferがない場合は作成します ------------------------------------*/

	if( !obj->d3d_cvbuff )
	{
		DWORD	cstride ;
		DWORD	n_verts ;
		DWORD	n_indices ;
		DWORD	size ;

		if( DG_CheckObjUseVertexShader() )
		{
			if( obj->flag & DG_FLAG_MULTITEX ){ cstride = sizeof(SVECTOR) ; }
			else{ cstride = sizeof(CVECTOR) ; }
		}
		else
		{
			if( obj->stride == sizeof(DG_VERTEX_KMSS) )
			{
				cstride = sizeof(DG_VERTEX_KMSS_FVF) ;
			}
			else
			{
				cstride = sizeof(DG_VERTEX_KMSM_FVF) ;
			}
		}

		if( DG_CheckIndexPrimitiveUseable() )
		{
			n_verts = obj->n_verts ;
			size = n_verts * cstride ;
		}
		else
		{
			n_indices = obj->n_indices ;
			size = n_indices * cstride ;
		}

		if( size )
		{
			DG_CreateVertexBuffer(size,
								DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
								0, OBJ_VERTEXBUFFER_POOL,
								&obj->d3d_cvbuff) ;
			obj->cvbuff_size = size ;

			if( !DG_CheckObjUseVertexShader() ){ DG_AssignDGObjVertexBuffer(obj) ; }
		}

	}
	/*--------------------------------------------------------------------------*/
}

/*------------------------------------------------------------------------------*/
/*		DG_FlushDGObjCVertexBuffer												*/
/*			Color Vertex Buffer変更の適用										*/
/*------------------------------------------------------------------------------*/
void DG_FlushDGObjCVertexBuffer(DG_OBJ *obj)
{
	DWORD	cstride ;
	BYTE 	*dst ;
	DWORD	size ;
	DWORD	n_indices ;
	void	*cbuff ;
	HRESULT	hr ;

	ASSERT( obj->d3d_cvbuff ) ;

	if( obj->flag & DG_FLAG_MULTITEX )
	{
		cstride = sizeof(SVECTOR) ;
		cbuff   = obj->rgbs ;
	}
	else
	{
		cstride = sizeof(CVECTOR) ;
		cbuff   = obj->norms ;
	}

	if( DG_CheckObjUseVertexShader() )
	{
		/*-- 通常版 ------------------------------------------------------------*/

		if( DG_CheckIndexPrimitiveUseable() )
		{
			size = cstride * obj->n_verts ;
#ifdef DEBUG_MODE
			if( size > obj->cvbuff_size ){ printf("[WARN!!]\n") ; size = obj->cvbuff_size ; }
#endif
			hr = IDirect3DVertexBuffer8_Lock(obj->d3d_cvbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
			ASSERT( !FAILED(hr) ) ;
			if( !FAILED(hr) )
			{
				memcpy((void *)dst, cbuff,  size) ;
				IDirect3DVertexBuffer8_Unlock(obj->d3d_cvbuff) ;
			}
		}
		else
		{
			n_indices = obj->n_indices ;
			size      = cstride * n_indices ;

			hr = IDirect3DVertexBuffer8_Lock(obj->d3d_cvbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
			ASSERT( !FAILED(hr) ) ;
			if( !FAILED(hr) )
			{
				if( obj->wflag & DG_OBJ_WFLAG_ORDERED_INDEX )
				{
					memcpy(dst, cbuff, size) ;
				}
				else
				{
					DG_ArrangeVertexByIndex(dst, cbuff, cstride,
									obj->index, n_indices) ;
				}
				IDirect3DVertexBuffer8_Unlock(obj->d3d_cvbuff) ;
			}
		}
		/*----------------------------------------------------------------------*/
	}
	else
	{
		/*-- VertexShader非対応版 ----------------------------------------------*/

		if( obj->stride == sizeof(DG_VERTEX_KMSS) )
		{
			void	(*overtwrite_func)(DG_VERTEX_KMSS_FVF *, void *, DWORD) ;

			/*-- KMSS形式 ------------------------------------------------------*/

#if FALSE
			//ASSERT( !(obj->flag & DG_FLAG_MULTITEX) ) ;
			if( DG_CheckIndexPrimitiveUseable() )
			{
				size = sizeof(DG_VERTEX_KMSS_FVF) * obj->n_verts ;

				hr = IDirect3DVertexBuffer8_Lock(obj->d3d_cvbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
				ASSERT( !FAILED(hr) ) ;
				if( !FAILED(hr) )
				{
					DG_OvertwriteKmssCol2KmssFVF((void *)dst, cbuff, obj->n_verts) ;
					IDirect3DVertexBuffer8_Unlock(obj->d3d_cvbuff) ;
				}
			}
			else
			{
				void	*tmp ;

				n_indices = obj->n_indices ;
				size      = sizeof(DG_VERTEX_KMSS_FVF) * n_indices ;

				if( obj->wflag & DG_OBJ_WFLAG_ORDERED_INDEX )
				{
					tmp = cbuff ;
				}
				else
				{
					tmp = DG_GetWorkBuffer() ;
					DG_ArrangeVertexByIndex(tmp, cbuff, sizeof(CVECTOR),
											obj->index, n_indices) ;
				}

				hr = IDirect3DVertexBuffer8_Lock(obj->d3d_cvbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
				ASSERT( !FAILED(hr) ) ;
				if( !FAILED(hr) )
				{
					DG_OvertwriteKmssCol2KmssFVF((void *)dst, tmp, obj->n_indices) ;
					IDirect3DVertexBuffer8_Unlock(obj->d3d_cvbuff) ;
				}
			}
#else
			if( !(obj->flag & DG_FLAG_MULTITEX) )
			{
				overtwrite_func = (void *)DG_OvertwriteKmssCol2KmssFVF ;
			}
			else
			{
				overtwrite_func = (void *)DG_OvertwriteKmsmCol2KmssFVF ;
			}

			if( DG_CheckIndexPrimitiveUseable() )
			{
				size = sizeof(DG_VERTEX_KMSS_FVF) * obj->n_verts ;

				hr = IDirect3DVertexBuffer8_Lock(obj->d3d_cvbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
				ASSERT( !FAILED(hr) ) ;
				if( !FAILED(hr) )
				{
					overtwrite_func((void *)dst, cbuff, obj->n_verts) ;
					IDirect3DVertexBuffer8_Unlock(obj->d3d_cvbuff) ;
				}
			}
			else
			{
				void	*tmp ;

				n_indices = obj->n_indices ;
				size      = sizeof(DG_VERTEX_KMSS_FVF) * n_indices ;

				if( obj->wflag & DG_OBJ_WFLAG_ORDERED_INDEX )
				{
					tmp = cbuff ;
				}
				else
				{
					tmp = DG_GetWorkBuffer() ;
					DG_ArrangeVertexByIndex(tmp, cbuff, cstride,
											obj->index, n_indices) ;
				}

				hr = IDirect3DVertexBuffer8_Lock(obj->d3d_cvbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
				ASSERT( !FAILED(hr) ) ;
				if( !FAILED(hr) )
				{
					overtwrite_func((void *)dst, tmp, obj->n_indices) ;
					IDirect3DVertexBuffer8_Unlock(obj->d3d_cvbuff) ;
				}
			}
#endif
			/*------------------------------------------------------------------*/
		}
		else
		{
			/*-- KMSM形式 ------------------------------------------------------*/

			ASSERT( obj->flag & DG_FLAG_MULTITEX ) ;

			if( DG_CheckIndexPrimitiveUseable() )
			{
				size = sizeof(DG_VERTEX_KMSM_FVF) * obj->n_verts ;

				hr = IDirect3DVertexBuffer8_Lock(obj->d3d_cvbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
				ASSERT( !FAILED(hr) ) ;
				if( !FAILED(hr) )
				{
					DG_OvertwriteKmsmCol2KmsmFVF((void *)dst, cbuff, obj->n_verts) ;
					IDirect3DVertexBuffer8_Unlock(obj->d3d_cvbuff) ;
				}
			}
			else
			{
				void	*tmp ;

				n_indices = obj->n_indices ;
				size      = sizeof(DG_VERTEX_KMSM_FVF) * n_indices ;

				if( obj->wflag & DG_OBJ_WFLAG_ORDERED_INDEX )
				{
					tmp = cbuff ;
				}
				else
				{
					tmp = DG_GetWorkBuffer() ;
					DG_ArrangeVertexByIndex(tmp, cbuff, cstride,
										obj->index, n_indices) ;
				}

				hr = IDirect3DVertexBuffer8_Lock(obj->d3d_cvbuff, 0, size, &dst, OBJ_VERTEXBUFFER_LOCKFLAG) ;
				ASSERT( !FAILED(hr) ) ;
				if( !FAILED(hr) )
				{
					DG_OvertwriteKmsmCol2KmsmFVF((void *)dst, tmp, n_indices) ;
					IDirect3DVertexBuffer8_Unlock(obj->d3d_cvbuff) ;
				}
			}
		}
		/*----------------------------------------------------------------------*/
	}

	obj->cvbuff_dirty = FALSE ;	// フラグOFF
}

/*------------------------------------------------------------------------------*/
/*		DG_AssignDGObjIndexBuffer												*/
/*			Vertex Buffer変更通達												*/
/*------------------------------------------------------------------------------*/
void DG_AssignDGObjIndexBuffer(DG_OBJ *obj)
{
	obj->ibuff_dirty = TRUE ;	// フラグON
}

/*------------------------------------------------------------------------------*/
/*		DG_AssignDGObjIndexBuffer												*/
/*			Vertex Buffer変更の適用												*/
/*------------------------------------------------------------------------------*/
void DG_FlushDGObjIndexBuffer(DG_OBJ *obj)
{
	DWORD	size ;
	BYTE	*dst ;
	DWORD	n_indices ;

	n_indices = obj->n_indices ;

	if( DG_CheckIndexPrimitiveUseable() )
	{
		if( obj->d3d_ibuff )
		{
			size = sizeof(WORD) * n_indices ;
			IDirect3DIndexBuffer8_Lock(obj->d3d_ibuff, 0, size, &dst, OBJ_INDEXBUFFER_LOCKFLAG) ;
			memcpy(dst, obj->index, size) ;
			IDirect3DIndexBuffer8_Unlock(obj->d3d_ibuff) ;
		}
	}
	else
	{
		DG_FlushDGObjVertexBuffer_NIdx(obj) ;
		if( obj->d3d_cvbuff ){ DG_FlushDGObjCVertexBuffer(obj) ; }
		obj->vbuff_dirty  = FALSE ;	// 頂点変更を行ったのでフラグOFF
		obj->cvbuff_dirty = FALSE ;
	}

	obj->ibuff_dirty = FALSE ;	// フラグOFF
}

/*------------------------------------------------------------------------------*/
/*		DG_AssignDGObjIndexBufferNVS											*/
/*			Vertex Buffer変更の適用(Vertex Shader非対応版)						*/
/*------------------------------------------------------------------------------*/
void DG_FlushDGObjIndexBufferNVS(DG_OBJ *obj)
{
	DWORD	size ;
	BYTE	*dst ;
	DWORD	n_indices ;

	n_indices = obj->n_indices ;

	if( DG_CheckIndexPrimitiveUseable() )
	{
		if( obj->d3d_ibuff )
		{
			size = sizeof(WORD) * n_indices ;
			IDirect3DIndexBuffer8_Lock(obj->d3d_ibuff, 0, size, &dst, OBJ_INDEXBUFFER_LOCKFLAG) ;
			memcpy(dst, obj->index, size) ;
			IDirect3DIndexBuffer8_Unlock(obj->d3d_ibuff) ;
		}
	}
	else
	{
		DG_FlushDGObjVertexBufferNVS(obj) ;
		if( obj->d3d_cvbuff ){ DG_FlushDGObjCVertexBuffer(obj) ; }
		obj->vbuff_dirty  = FALSE ;	// 頂点変更を行ったのでフラグOFF
		obj->cvbuff_dirty = FALSE ;
	}

	obj->ibuff_dirty = FALSE ;	// フラグOFF
}

/*------------------------------------------------------------------------------*/
/*		DG_ConvertKmss2KmssFVF													*/
/*			VertexFormatの変更													*/
/*------------------------------------------------------------------------------*/
void DG_ConvertKmss2KmssFVF(DG_VERTEX_KMSS_FVF *dst,
								DG_VERTEX_KMSS *src, DWORD *srccol, DWORD num)
{
	int		i ;

	if( srccol )
	{
		for(i=(int)num; i>0; i--, dst++, src++, srccol++)
		{
			dst->vx = (float)src->vx ;
			dst->vy = (float)src->vy ;
			dst->vz = (float)src->vz ;
			dst->wt = (float)src->wt * (1.0f / 32767.0f) ;

			dst->nx = (float)src->nx * (1.0f / 32767.0f) ;
			dst->ny = (float)src->ny * (1.0f / 32767.0f) ;
			dst->nz = (float)src->nz * (1.0f / 32767.0f) ;

			dst->u0 = (float)src->u0 ;
			dst->v0 = (float)src->v0 ;

			dst->diffuse = *srccol ;		/* diffuse */
		}
	}
	else
	{
		for(i=(int)num; i>0; i--, dst++, src++)
		{
			dst->vx = (float)src->vx ;
			dst->vy = (float)src->vy ;
			dst->vz = (float)src->vz ;
			dst->wt = (float)src->wt * (1.0f / 32767.0f) ;

			dst->nx = (float)src->nx * (1.0f / 32767.0f) ;
			dst->ny = (float)src->ny * (1.0f / 32767.0f) ;
			dst->nz = (float)src->nz * (1.0f / 32767.0f) ;

			dst->u0 = (float)src->u0 ;
			dst->v0 = (float)src->v0 ;

			dst->diffuse = 0x80ffffff ;		/* diffuse */
		}
	}
}

/*------------------------------------------------------------------------------*/
/*		DG_OvertwriteKmssVtx2KmssFVF											*/
/*			VertexFormat頂点要素の上書き										*/
/*------------------------------------------------------------------------------*/
void DG_OvertwriteKmssVtx2KmssFVF(DG_VERTEX_KMSS_FVF *dst,
								DG_VERTEX_KMSS *src, DWORD num)
{
	int		i ;

	for(i=(int)num; i>0; i--, dst++, src++)
	{
		dst->vx = (float)src->vx ;
		dst->vy = (float)src->vy ;
		dst->vz = (float)src->vz ;
		dst->wt = (float)src->wt * (1.0f / 32767.0f) ;

		dst->nx = (float)src->nx * (1.0f / 32767.0f) ;
		dst->ny = (float)src->ny * (1.0f / 32767.0f) ;
		dst->nz = (float)src->nz * (1.0f / 32767.0f) ;

		dst->u0 = (float)src->u0 ;
		dst->v0 = (float)src->v0 ;
	}
}

/*------------------------------------------------------------------------------*/
/*		DG_OvertwriteKmssCol2KmssFVF											*/
/*			VertexFormat色要素の上書き											*/
/*------------------------------------------------------------------------------*/
void DG_OvertwriteKmssCol2KmssFVF(DG_VERTEX_KMSS_FVF *dst,
								DWORD *srccol, DWORD num)
{
	int		i ;

	for(i=(int)num; i>0; i--, dst++, srccol++)
	{
		dst->diffuse = *srccol ;		/* diffuse */
	}
}

/*------------------------------------------------------------------------------*/
/*		DG_SetAllKmssCol2KmssFVF												*/
/*			VertexFormat全色要素の指定色上書き									*/
/*------------------------------------------------------------------------------*/
void DG_SetAllKmssCol2KmssFVF(DG_VERTEX_KMSS_FVF *dst, DWORD col, DWORD num)
{
	int		i ;

	for(i=(int)num; i>0; i--, dst++)
	{
		dst->diffuse = col ;		/* diffuse */
	}
}

/*------------------------------------------------------------------------------*/
/*		DG_ConvertKmss2KmssFVF													*/
/*			VertexFormatの変更													*/
/*------------------------------------------------------------------------------*/
void DG_ConvertKmsm2KmsmFVF(DG_VERTEX_KMSM_FVF *dst,
							DG_VERTEX_KMSM *src, SVECTOR *srccol, DWORD num)
{
	int		i ;
	DWORD	col ;
	SVECTOR	colvec ;

	if( srccol )
	{
		for(i=(int)num; i>0; i--, dst++, src++, srccol++)
		{
			dst->vx = (float)src->vx ;
			dst->vy = (float)src->vy ;
			dst->vz = (float)src->vz ;
			dst->wt = (float)src->wt * (1.0f / 32767.0f) ;

			dst->nx = (float)src->nx * (1.0f / 32767.0f) ;
			dst->ny = (float)src->ny * (1.0f / 32767.0f) ;
			dst->nz = (float)src->nz * (1.0f / 32767.0f) ;

			dst->u0 = (float)src->u0 ;
			dst->v0 = (float)src->v0 ;
			dst->u1 = (float)src->u1 ;
			dst->v1 = (float)src->v1 ;
			dst->u2 = (float)src->u2 ;
			dst->v2 = (float)src->v2 ;

			colvec = *srccol ;
			col = (0xff << 24)
				| (((colvec.vx >> 12) & 0xff) << 16)
				| (((colvec.vy >> 12) & 0xff) << 8)
				| ((colvec.vz >> 12) & 0xff) ;
			dst->diffuse = col ;		/* diffuse */
		}
	}
	else
	{
		for(i=(int)num; i>0; i--, dst++, src++)
		{
			dst->vx = (float)src->vx ;
			dst->vy = (float)src->vy ;
			dst->vz = (float)src->vz ;
			dst->wt = (float)src->wt * (1.0f / 32767.0f) ;

			dst->nx = (float)src->nx * (1.0f / 32767.0f) ;
			dst->ny = (float)src->ny * (1.0f / 32767.0f) ;
			dst->nz = (float)src->nz * (1.0f / 32767.0f) ;

			dst->u0 = (float)src->u0 ;
			dst->v0 = (float)src->v0 ;
			dst->u1 = (float)src->u1 ;
			dst->v1 = (float)src->v1 ;
			dst->u2 = (float)src->u2 ;
			dst->v2 = (float)src->v2 ;

			dst->diffuse = 0x80ffffff ;		/* diffuse */
		}
	}
}

/*------------------------------------------------------------------------------*/
/*		DG_OvertwriteKmsmVtx2KmsmFVF											*/
/*			VertexFormat頂点要素の上書き										*/
/*------------------------------------------------------------------------------*/
void DG_OvertwriteKmsmVtx2KmsmFVF(DG_VERTEX_KMSM_FVF *dst,
								DG_VERTEX_KMSM *src, DWORD num)
{
	int		i ;

	for(i=(int)num; i>0; i--, dst++, src++)
	{
		dst->vx = (float)src->vx ;
		dst->vy = (float)src->vy ;
		dst->vz = (float)src->vz ;
		dst->wt = (float)src->wt * (1.0f / 32767.0f) ;

		dst->nx = (float)src->nx * (1.0f / 32767.0f) ;
		dst->ny = (float)src->ny * (1.0f / 32767.0f) ;
		dst->nz = (float)src->nz * (1.0f / 32767.0f) ;

		dst->u0 = (float)src->u0 ;
		dst->v0 = (float)src->v0 ;
		dst->u1 = (float)src->u1 ;
		dst->v1 = (float)src->v1 ;
		dst->u2 = (float)src->u2 ;
		dst->v2 = (float)src->v2 ;
	}
}

/*------------------------------------------------------------------------------*/
/*		DG_OvertwriteKmsmCol2KmsmFVF											*/
/*			VertexFormat色要素の上書き											*/
/*------------------------------------------------------------------------------*/
void DG_OvertwriteKmsmCol2KmsmFVF(DG_VERTEX_KMSM_FVF *dst,
								SVECTOR *srccol, DWORD num)
{
	int		i ;
	DWORD	col ;
	SVECTOR	colvec ;
	DWORD	val ;

	for(i=(int)num; i>0; i--, dst++, srccol++)
	{
		colvec = *srccol ;
		col = (0xff << 24) ;

		val = (colvec.vx >> 4) ;
		if( val ){ val-- ; }
		col |= val << 16 ;

		val = (colvec.vy >> 4) ;
		if( val ){ val-- ; }
		col |= val << 8;

		val = (colvec.vz >> 4) ;
		if( val ){ val-- ; }
		col |= val ;

		dst->diffuse = col ;		/* diffuse */
	}
}

/*------------------------------------------------------------------------------*/
/*		DG_OvertwriteKmsmCol2KmssFVF											*/
/*			VertexFormat色要素の上書き											*/
/*------------------------------------------------------------------------------*/
void DG_OvertwriteKmsmCol2KmssFVF(DG_VERTEX_KMSS_FVF *dst,
								SVECTOR *srccol, DWORD num)
{
	int		i ;
	DWORD	col ;
	SVECTOR	colvec ;
	DWORD	val ;

	for(i=(int)num; i>0; i--, dst++, srccol++)
	{
		colvec = *srccol ;
		col = (0xff << 24) ;

		val = (colvec.vx >> 4) ;
		if( val ){ val-- ; }
		col |= val << 16 ;

		val = (colvec.vy >> 4) ;
		if( val ){ val-- ; }
		col |= val << 8;

		val = (colvec.vz >> 4) ;
		if( val ){ val-- ; }
		col |= val ;

		dst->diffuse = col ;		/* diffuse */
	}
}


/*------------------------------------------------------------------------------*/
/*		DG_SetAllKmsmCol2KmsmFVF												*/
/*			VertexFormat全色要素の指定色上書き									*/
/*------------------------------------------------------------------------------*/
void DG_SetAllKmsmCol2KmsmFVF(DG_VERTEX_KMSM_FVF *dst, DWORD col, DWORD num)
{
	int		i ;

	for(i=(int)num; i>0; i--, dst++)
	{
		dst->diffuse = col ;		/* diffuse */
	}
}

/*------------------------------------------------------------------------------*/
/*		DG_CreateDGObjPrivateVertexBuffer										*/
/*			専用VertexBuffer作成												*/
/*------------------------------------------------------------------------------*/
static BOOL _DG_CreateDGObjPrivateVertexBuffer(DG_OBJ *obj, int n_indices, int n_verts) ;
static BOOL _DG_CreateDGObjPrivateVertexBuffer_NIdx(DG_OBJ *obj, int n_indices, int n_verts) ;
static BOOL _DG_CreateDGObjPrivateVertexBuffer_NVS(DG_OBJ *obj, int n_indices, int n_verts) ;
static BOOL _DG_CreateDGObjPrivateVertexBuffer_NVSNIdx(DG_OBJ *obj, int n_indices, int n_verts) ;

BOOL DG_CreateDGObjPrivateVertexBuffer(DG_OBJ *obj, int n_indices, int n_verts)
{
	BOOL	ret ;

	if( !n_indices || !n_verts )
	{
		ASSERT(0) ;
		return(FALSE) ;
	}

	if( (obj->wflag & DG_OBJ_WFLAG_USE_PRIVATE_VBUFF) )
	{
		DG_ReleaseDGObjPrivateVertexBuffer(obj);
	}

	if( DG_CheckObjUseVertexShader() )
	{
		/*-- VertexShader使用版 ------------------------------------------------*/

		if( DG_CheckIndexPrimitiveUseable() )
		{
			ret = _DG_CreateDGObjPrivateVertexBuffer(obj, n_indices, n_verts) ;
		}
		else
		{
			ret = _DG_CreateDGObjPrivateVertexBuffer_NIdx(obj, n_indices, n_verts) ;
		}
		/*----------------------------------------------------------------------*/
	}
	else
	{
		/*-- VertexBuffer非対応版 ----------------------------------------------*/

		if( DG_CheckIndexPrimitiveUseable() )
		{
			ret = _DG_CreateDGObjPrivateVertexBuffer_NVS(obj, n_indices, n_verts) ;
		}
		else
		{
			ret = _DG_CreateDGObjPrivateVertexBuffer_NVSNIdx(obj, n_indices, n_verts) ;
		}
		/*----------------------------------------------------------------------*/
	}

	if( ret ){ obj->wflag |= DG_OBJ_WFLAG_USE_PRIVATE_VBUFF ; }

	return(ret) ;
}

static BOOL _DG_CreateDGObjPrivateVertexBuffer(DG_OBJ *obj, int n_indices, int n_verts)
{
	DWORD	size ;

	obj->n_verts   = n_verts ;
	obj->n_indices = n_indices ;

	size = n_verts * obj->stride ;

	/*-- Buffer新規作成 --------------------------------------------------------*/

	DG_CreateVertexBuffer(size,
				DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
				0, OBJ_VERTEXBUFFER_POOL,
				&obj->d3d_vbuff) ;

	DG_CreateIndexBuffer(n_indices * sizeof(WORD),
				DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
				D3DFMT_INDEX16, OBJ_VERTEXBUFFER_POOL,
				&obj->d3d_ibuff) ;
	/*--------------------------------------------------------------------------*/

	/*-- 色バッファ再作成 ------------------------------------------------------*/

	if( obj->d3d_cvbuff )
	{
		DWORD	csize ;
		DWORD	cstride ;

		if( obj->flag & DG_FLAG_MULTITEX ){ cstride = sizeof(SVECTOR) ; }
		else{ cstride = sizeof(CVECTOR) ; }

		csize = cstride * n_verts ;
		if( csize > obj->cvbuff_size )	// 現在のバッファで十分なら、再確保はしない
		{
			/*-- 現在のバッファの破棄 ------------------------------------------*/

			DG_ExeBufferSync(FALSE) ;
			DG_ReleaseD3DVertexBuffer(obj->d3d_cvbuff) ;
			obj->d3d_cvbuff = NULL ;
			/*------------------------------------------------------------------*/

			/*-- 新規にバッファ作成 --------------------------------------------*/

			DG_CreateVertexBuffer(csize,
						DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
						0, OBJ_VERTEXBUFFER_POOL,
						&obj->d3d_cvbuff) ;
			obj->cvbuff_size = csize ;
			/*------------------------------------------------------------------*/
		}
	}
	/*--------------------------------------------------------------------------*/

	return(TRUE) ;
}

static BOOL _DG_CreateDGObjPrivateVertexBuffer_NIdx(DG_OBJ *obj, int n_indices, int n_verts)
{
	DWORD	size ;

	obj->n_verts   = n_verts ;
	obj->n_indices = n_indices ;

	size = n_indices * obj->stride ;

	/*-- Buffer新規作成 --------------------------------------------------------*/

	DG_CreateVertexBuffer(size,
				DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
				0, OBJ_VERTEXBUFFER_POOL,
				&obj->d3d_vbuff) ;
	/*--------------------------------------------------------------------------*/

	/*-- 色バッファ再作成 ------------------------------------------------------*/

	if( obj->d3d_cvbuff )
	{
		DWORD	csize ;
		DWORD	cstride ;

		if( obj->flag & DG_FLAG_MULTITEX ){ cstride = sizeof(SVECTOR) ; }
		else{ cstride = sizeof(CVECTOR) ; }

		csize = cstride * n_indices ;
		if( csize > obj->cvbuff_size )	// 現在のバッファで十分なら、再確保はしない
		{
			/*-- 現在のバッファの破棄 ------------------------------------------*/

			DG_ExeBufferSync(FALSE) ;
			DG_ReleaseD3DVertexBuffer(obj->d3d_cvbuff) ;
			obj->d3d_cvbuff = NULL ;
			/*------------------------------------------------------------------*/

			/*-- 新規にバッファ作成 --------------------------------------------*/

			DG_CreateVertexBuffer(csize,
						DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
						0, OBJ_VERTEXBUFFER_POOL,
						&obj->d3d_cvbuff) ;
			obj->cvbuff_size = csize ;
			/*------------------------------------------------------------------*/
		}
	}
	/*--------------------------------------------------------------------------*/

	return(TRUE) ;
}

static BOOL _DG_CreateDGObjPrivateVertexBuffer_NVS(DG_OBJ *obj, int n_indices, int n_verts)
{
	DWORD					size ;
	DWORD					stride ;
	LPDIRECT3DVERTEXBUFFER8	*vbuff ;

	/*-- CVbuffは一旦解放します ------------------------------------------------*/

	if( obj->d3d_cvbuff )
	{
		DG_ExeBufferSync(FALSE) ;
		DG_ReleaseD3DVertexBuffer(obj->d3d_cvbuff) ;
		obj->d3d_cvbuff = NULL ;
	}
	/*--------------------------------------------------------------------------*/

	/*-- Stride & 対象VertexBuffer検査 -----------------------------------------*/

	if( obj->stride == sizeof(DG_VERTEX_KMSS) ){ stride = sizeof(DG_VERTEX_KMSS_FVF) ; }
	else{ stride = sizeof(DG_VERTEX_KMSM_FVF) ; }

	if( obj->flag & DG_FLAG_MULTITEX )
	{
		if( obj->rgbs ){ vbuff = &obj->d3d_cvbuff ; }
		else{ vbuff = &obj->d3d_vbuff ; }
	}
	else
	{
		if( obj->flag & DG_FLAG_PAINT ){ vbuff = &obj->d3d_cvbuff ; }
		else{ vbuff = &obj->d3d_vbuff ; }
	}
	/*--------------------------------------------------------------------------*/

	obj->n_verts   = n_verts ;
	obj->n_indices = n_indices ;

	size = n_verts * stride ;

	/*-- Buffer新規作成 --------------------------------------------------------*/

	DG_CreateVertexBuffer(size,
				DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
				0, OBJ_VERTEXBUFFER_POOL,
				vbuff) ;

	DG_CreateIndexBuffer(n_indices * sizeof(WORD),
				DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
				D3DFMT_INDEX16, OBJ_VERTEXBUFFER_POOL,
				&obj->d3d_ibuff) ;
	/*--------------------------------------------------------------------------*/

	/*-- 頂点色初期設定 --------------------------------------------------------*/

	if( *vbuff )
	{
		BYTE	*dst ;

		DG_ExeBufferSync(FALSE) ;
		IDirect3DIndexBuffer8_Lock(*vbuff, 0, size, &dst, OBJ_INDEXBUFFER_LOCKFLAG) ;
		if( obj->stride == sizeof(DG_VERTEX_KMSS) )
		{
			DG_SetAllKmssCol2KmssFVF((DG_VERTEX_KMSS_FVF *)dst, 0x80ffffff, obj->n_verts) ;
		}
		else
		{
			DG_SetAllKmsmCol2KmsmFVF((DG_VERTEX_KMSM_FVF *)dst, 0x80ffffff, obj->n_verts) ;
		}
		IDirect3DIndexBuffer8_Unlock(*vbuff) ;
	}
	/*--------------------------------------------------------------------------*/

	return(TRUE) ;
}

static BOOL _DG_CreateDGObjPrivateVertexBuffer_NVSNIdx(DG_OBJ *obj,
												int n_indices, int n_verts)
{
	DWORD					size ;
	DWORD					stride ;
	LPDIRECT3DVERTEXBUFFER8	*vbuff ;

	/*-- CVbuffは一旦解放します ------------------------------------------------*/

	if( obj->d3d_cvbuff )
	{
		DG_ExeBufferSync(FALSE) ;
		DG_ReleaseD3DVertexBuffer(obj->d3d_cvbuff) ;
		obj->d3d_cvbuff = NULL ;
	}
	/*--------------------------------------------------------------------------*/

	/*-- Stride & 対象VertexBuffer検査 -----------------------------------------*/

	if( obj->stride == sizeof(DG_VERTEX_KMSS) ){ stride = sizeof(DG_VERTEX_KMSS_FVF) ; }
	else{ stride = sizeof(DG_VERTEX_KMSM_FVF) ; }

	if( obj->flag & DG_FLAG_MULTITEX )
	{
		if( obj->rgbs ){ vbuff = &obj->d3d_cvbuff ; }
		else{ vbuff = &obj->d3d_vbuff ; }
	}
	else
	{
		if( obj->flag & DG_FLAG_PAINT ){ vbuff = &obj->d3d_cvbuff ; }
		else{ vbuff = &obj->d3d_vbuff ; }
	}
	/*--------------------------------------------------------------------------*/

	obj->n_verts   = n_verts ;
	obj->n_indices = n_indices ;

	size = n_indices * stride ;

	/*-- Buffer新規作成 --------------------------------------------------------*/

	DG_CreateVertexBuffer(size,
				DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
				0, OBJ_VERTEXBUFFER_POOL,
				vbuff) ;
	/*--------------------------------------------------------------------------*/

	/*-- 頂点色初期設定 --------------------------------------------------------*/

	if( *vbuff )
	{
		BYTE	*dst ;

		DG_ExeBufferSync(FALSE) ;	// 安全策
		IDirect3DIndexBuffer8_Lock(*vbuff, 0, size, &dst, OBJ_INDEXBUFFER_LOCKFLAG) ;
		if( obj->stride == sizeof(DG_VERTEX_KMSS) )
		{
			DG_SetAllKmssCol2KmssFVF((DG_VERTEX_KMSS_FVF *)dst, 0x80ffffff, obj->n_indices) ;
		}
		else
		{
			DG_SetAllKmsmCol2KmsmFVF((DG_VERTEX_KMSM_FVF *)dst, 0x80ffffff, obj->n_indices) ;
		}
		IDirect3DIndexBuffer8_Unlock(*vbuff) ;
	}
	/*--------------------------------------------------------------------------*/
	return(TRUE) ;
}

/*------------------------------------------------------------------------------*/
/*		DG_ReleaseDGObjPrivateVertexBuffer										*/
/*			専用VertexBuffer解放												*/
/*------------------------------------------------------------------------------*/
static BOOL	_DG_ReleaseDGObjPrivateVertexBuffer(DG_OBJ *obj) ;
static BOOL	_DG_ReleaseDGObjPrivateVertexBuffer_NIdx(DG_OBJ *obj) ;
static BOOL	_DG_ReleaseDGObjPrivateVertexBuffer_NVS(DG_OBJ *obj) ;
static BOOL	_DG_ReleaseDGObjPrivateVertexBuffer_NVSNIdx(DG_OBJ *obj) ;

BOOL DG_ReleaseDGObjPrivateVertexBuffer(DG_OBJ *obj)
{
	BOOL	ret ;

	if( !(obj->wflag & DG_OBJ_WFLAG_USE_PRIVATE_VBUFF) ){ return(FALSE) ; }

	if( DG_CheckObjUseVertexShader() )
	{
		/*-- VertexShader使用版 ------------------------------------------------*/

		if( DG_CheckIndexPrimitiveUseable() )
		{
			ret = _DG_ReleaseDGObjPrivateVertexBuffer(obj) ;
		}
		else
		{
			ret = _DG_ReleaseDGObjPrivateVertexBuffer_NIdx(obj) ;
		}
		/*----------------------------------------------------------------------*/
	}
	else
	{
		/*-- VertexBuffer非対応版 ----------------------------------------------*/

		if( DG_CheckIndexPrimitiveUseable() )
		{
			ret = _DG_ReleaseDGObjPrivateVertexBuffer_NVS(obj) ;
		}
		else
		{
			ret = _DG_ReleaseDGObjPrivateVertexBuffer_NVSNIdx(obj) ;
		}
		/*----------------------------------------------------------------------*/
	}

	obj->wflag &= ~DG_OBJ_WFLAG_USE_PRIVATE_VBUFF ;

	return( ret ) ;
}

static void	_DG_ReleaseDGObjPrivateVertexBuffer_ResetVertexBuffer(DG_OBJ *obj)
{
	DG_MDL_VERTEX_BUFFER_TBL	vbuff_tbl ;
	DG_MDL						*mdl ;

	/*-- 解放 ------------------------------------------------------------------*/

	if( obj->d3d_vbuff || obj->d3d_ibuff )
	{
		DG_ExeBufferSync(FALSE) ;

		if( obj->d3d_vbuff ){ DG_ReleaseD3DVertexBuffer(obj->d3d_vbuff) ; }
		if( obj->d3d_ibuff ){ DG_ReleaseD3DIndexBuffer(obj->d3d_ibuff) ;  }
		obj->d3d_vbuff = NULL ;
		obj->d3d_ibuff = NULL ;
	}
	/*--------------------------------------------------------------------------*/

	/*-- Packet情報復帰処理 ----------------------------------------------------*/

	{
		int							i, k ;
		DG_OBJ_PACKET				*pack ;
		DG_MDLPACK					*mdlpack ;
		DWORD						n_indices ;
		DWORD						n_verts ;

		mdl = obj->model ;

		n_indices = 0 ;
		n_verts = 0 ;

		pack    = obj->packets ;
		mdlpack = mdl->packs ;
		for(i=(int)mdl->n_packs; i>0 ; i--, pack++, mdlpack++)
		{
			/*-- 最大/最小参照頂点計算 -------------------------------------------*/

			if( mdlpack->n_indices )
			{
				WORD	idx_vtx_min ;
				WORD	idx_vtx_max ;
				WORD	*idx_ptr ;
				WORD	idx_val ;

				idx_ptr = (WORD *)obj->index + n_indices ;
				idx_val = *idx_ptr ;
				idx_vtx_min = idx_val ;
				idx_vtx_max = idx_val ;

				idx_ptr++ ;
				for(k=mdlpack->n_indices-1; k>0; k--, idx_ptr++)
				{
					idx_val = *idx_ptr ;
					if(      idx_vtx_max < idx_val ){ idx_vtx_max = idx_val ; }
					else if( idx_vtx_min > idx_val ){ idx_vtx_min = idx_val ; }
				}
				idx_vtx_max++ ;	// 補正

				pack->idx_vtx_min = idx_vtx_min ;
				pack->idx_vtx_num = idx_vtx_max - idx_vtx_min ;
				
				if( n_verts < idx_vtx_max ){ n_verts = idx_vtx_max  ; }
			}
			/*----------------------------------------------------------------------*/
			n_indices += mdlpack->n_indices ;
		}
	}
	/*--------------------------------------------------------------------------*/

	/*-- Vertex Buffer 再設定 --------------------------------------------------*/

	if( DG_GetDGMdlVertexBuffer2(mdl, &vbuff_tbl) )
	{
		obj->verts = (SVECTOR*)mdl->packs->verts ;
		obj->index = mdl->packs->index ;

		obj->n_verts   = vbuff_tbl.n_verts ;
		obj->n_indices = vbuff_tbl.n_indices ;

		obj->d3d_vbuff  = vbuff_tbl.vbuff ;
		obj->d3d_ibuff  = vbuff_tbl.ibuff ;

		if( vbuff_tbl.flag & DG_MDL_VERTEX_BUFFER_TBL_ORDERED_INDEX )
		{
			obj->wflag |= DG_OBJ_WFLAG_ORDERED_INDEX ;
		}
		else
		{
			obj->wflag &= ~DG_OBJ_WFLAG_ORDERED_INDEX ;
		}
	}

	/*-- 更新フラグOff ---------------------------------------------------------*/

	obj->ibuff_dirty  = FALSE ;
	obj->vbuff_dirty  = FALSE ;
	/*--------------------------------------------------------------------------*/
}

static BOOL	_DG_ReleaseDGObjPrivateVertexBuffer(DG_OBJ *obj)
{
	/*-- VertexBuffer再設定処理 ------------------------------------------------*/

	_DG_ReleaseDGObjPrivateVertexBuffer_ResetVertexBuffer(obj) ;
	/*--------------------------------------------------------------------------*/

	/*-- 色頂点バッファ更新 ----------------------------------------------------*/

	if( obj->d3d_cvbuff )
	{
		DWORD	csize ;
		DWORD	cstride ;

		if( obj->flag & DG_FLAG_MULTITEX ){ cstride = sizeof(SVECTOR) ; }
		else{ cstride = sizeof(CVECTOR) ; }

		csize = cstride * obj->n_verts ;
		if( csize > obj->cvbuff_size )	// 現在のバッファで十分なら、再確保はしない
		{
			/*-- 現在のバッファの破棄 ------------------------------------------*/

			DG_ReleaseD3DVertexBuffer(obj->d3d_cvbuff) ;
			obj->d3d_cvbuff = NULL ;
			/*------------------------------------------------------------------*/

			/*-- 新規にバッファ作成 --------------------------------------------*/

			DG_CreateVertexBuffer(csize,
						DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
						0, OBJ_VERTEXBUFFER_POOL,
						&obj->d3d_cvbuff) ;
			obj->cvbuff_size = csize ;
			/*------------------------------------------------------------------*/

			obj->cvbuff_dirty = TRUE ;
		}
	}
	/*--------------------------------------------------------------------------*/

	return(TRUE) ;
}

static BOOL	_DG_ReleaseDGObjPrivateVertexBuffer_NIdx(DG_OBJ *obj)
{
	/*-- VertexBuffer再設定処理 ------------------------------------------------*/

	_DG_ReleaseDGObjPrivateVertexBuffer_ResetVertexBuffer(obj) ;
	/*--------------------------------------------------------------------------*/

	/*-- 色頂点バッファ更新 ----------------------------------------------------*/

	if( obj->d3d_cvbuff )
	{
		DWORD	csize ;
		DWORD	cstride ;

		if( obj->flag & DG_FLAG_MULTITEX ){ cstride = sizeof(SVECTOR) ; }
		else{ cstride = sizeof(CVECTOR) ; }

		csize = cstride * obj->n_indices ;
		if( csize > obj->cvbuff_size )	// 現在のバッファで十分なら、再確保はしない
		{
			/*-- 現在のバッファの破棄 ------------------------------------------*/

			DG_ReleaseD3DVertexBuffer(obj->d3d_cvbuff) ;
			obj->d3d_cvbuff = NULL ;
			/*------------------------------------------------------------------*/

			/*-- 新規にバッファ作成 --------------------------------------------*/

			DG_CreateVertexBuffer(csize,
						DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
						0, OBJ_VERTEXBUFFER_POOL,
						&obj->d3d_cvbuff) ;
			obj->cvbuff_size = csize ;
			/*------------------------------------------------------------------*/

			obj->cvbuff_dirty = TRUE ;
		}
	}
	/*--------------------------------------------------------------------------*/

	/*-- Packet情報復帰処理 ----------------------------------------------------*/

	{
		int				i, k ;
		DG_MDL			*mdl ;
		DG_OBJ_PACKET	*pack ;
		DG_MDLPACK		*mdlpack ;
		DWORD			n_indices ;
		DWORD			n_verts ;

		mdl = obj->model ;

		n_indices = 0 ;
		n_verts = 0 ;

		pack    = obj->packets ;
		mdlpack = mdl->packs ;
		for(i=(int)mdl->n_packs; i>0 ; i--, pack++, mdlpack++)
		{
			/*-- 最大/最小参照頂点計算 -------------------------------------------*/

			if( mdlpack->n_indices )
			{
				WORD	idx_vtx_min ;
				WORD	idx_vtx_max ;
				WORD	*idx_ptr ;
				WORD	idx_val ;

				idx_ptr = (WORD *)obj->index + n_indices ;
				idx_val = *idx_ptr ;
				idx_vtx_min = idx_val ;
				idx_vtx_max = idx_val ;

				idx_ptr++ ;
				for(k=mdlpack->n_indices-1; k>0; k--, idx_ptr++)
				{
					idx_val = *idx_ptr ;
					if(      idx_vtx_max < idx_val ){ idx_vtx_max = idx_val ; }
					else if( idx_vtx_min > idx_val ){ idx_vtx_min = idx_val ; }
				}
				idx_vtx_max++ ;	// 補正

				pack->idx_vtx_min = idx_vtx_min ;
				pack->idx_vtx_num = idx_vtx_max - idx_vtx_min ;
				
				if( n_verts < idx_vtx_max ){ n_verts = idx_vtx_max  ; }
			}
			/*----------------------------------------------------------------------*/
			n_indices += mdlpack->n_indices ;
		}
	}
	/*--------------------------------------------------------------------------*/

	return(TRUE) ;
}

static BOOL	_DG_ReleaseDGObjPrivateVertexBuffer_NVS(DG_OBJ *obj)
{
	DWORD						size ;
	DWORD						stride ;
	DG_MDL_VERTEX_BUFFER_TBL	vbuff_tbl ;
	BOOL						vbuff_tbl_chk ;
	BOOL						use_cvbuff ;
	DG_MDL						*mdl ;


	/*-- Stride & 対象VertexBuffer検査 -----------------------------------------*/

	if( obj->stride == sizeof(DG_VERTEX_KMSS) ){ stride = sizeof(DG_VERTEX_KMSS_FVF) ; }
	else{ stride = sizeof(DG_VERTEX_KMSM_FVF) ; }

	use_cvbuff = FALSE ;
	if( obj->flag & DG_FLAG_MULTITEX )
	{
		if( obj->rgbs ){ use_cvbuff = TRUE ; }
	}
	else
	{
		if( obj->flag & DG_FLAG_PAINT ){ use_cvbuff = TRUE ; }
	}
	/*--------------------------------------------------------------------------*/

	/*-- 解放 ------------------------------------------------------------------*/

	DG_ExeBufferSync(FALSE) ;
	if( obj->d3d_ibuff )
	{
		DG_ReleaseD3DIndexBuffer(obj->d3d_ibuff) ;
	}
	if( !use_cvbuff )
	{
		if( obj->d3d_vbuff ){ DG_ReleaseD3DVertexBuffer(obj->d3d_vbuff) ; }
	}
	/*--------------------------------------------------------------------------*/

	/*-- デフォルトのVertexBuffer取得 ------------------------------------------*/

	mdl = obj->model ;
	if( (vbuff_tbl_chk = DG_GetDGMdlVertexBuffer2(mdl, &vbuff_tbl)) )
	{
		obj->verts = (SVECTOR*)mdl->packs->verts ;
		obj->index = mdl->packs->index ;

		obj->n_verts    = vbuff_tbl.n_verts ;
		obj->n_indices  = vbuff_tbl.n_indices ;

		obj->d3d_vbuff  = vbuff_tbl.vbuff ;
		obj->d3d_ibuff  = vbuff_tbl.ibuff ;

		if( vbuff_tbl.flag & DG_MDL_VERTEX_BUFFER_TBL_ORDERED_INDEX )
		{
			obj->wflag |= DG_OBJ_WFLAG_ORDERED_INDEX ;
		}
		else
		{
			obj->wflag &= ~DG_OBJ_WFLAG_ORDERED_INDEX ;
		}
	}
	/*--------------------------------------------------------------------------*/

	/*-- 更新フラグOff ---------------------------------------------------------*/

	obj->ibuff_dirty  = FALSE ;
	obj->vbuff_dirty  = FALSE ;
	/*--------------------------------------------------------------------------*/

	/*-- Buffer新規作成 --------------------------------------------------------*/

	if( vbuff_tbl_chk && use_cvbuff )
	{
		size = stride * vbuff_tbl.n_verts ;

		DG_CreateVertexBuffer(size,
					DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
					0, OBJ_VERTEXBUFFER_POOL,
					&obj->d3d_cvbuff) ;

		obj->cvbuff_size = size ;

		obj->vbuff_dirty  = TRUE ;	// 転送は外部に任せる
		obj->cvbuff_dirty = TRUE ;
	}
	/*--------------------------------------------------------------------------*/

	return(TRUE) ;
}

static BOOL	_DG_ReleaseDGObjPrivateVertexBuffer_NVSNIdx(DG_OBJ *obj)
{
	DWORD						size ;
	DWORD						stride ;
	DG_MDL_VERTEX_BUFFER_TBL	vbuff_tbl ;
	BOOL						vbuff_tbl_chk ;
	BOOL						use_cvbuff ;
	DG_MDL						*mdl ;

	/*-- Stride & 対象VertexBuffer検査 -----------------------------------------*/

	if( obj->stride == sizeof(DG_VERTEX_KMSS) ){ stride = sizeof(DG_VERTEX_KMSS_FVF) ; }
	else{ stride = sizeof(DG_VERTEX_KMSM_FVF) ; }

	use_cvbuff = FALSE ;
	if( obj->flag & DG_FLAG_MULTITEX )
	{
		if( obj->rgbs ){ use_cvbuff = TRUE ; }
	}
	else
	{
		if( obj->flag & DG_FLAG_PAINT ){ use_cvbuff = TRUE ; }
	}
	/*--------------------------------------------------------------------------*/

	/*-- 解放 ------------------------------------------------------------------*/

	DG_ExeBufferSync(FALSE) ;
	if( obj->d3d_ibuff )
	{
		DG_ReleaseD3DIndexBuffer(obj->d3d_ibuff) ;
	}
	if( !use_cvbuff )
	{
		if( obj->d3d_vbuff ){ DG_ReleaseD3DVertexBuffer(obj->d3d_vbuff) ; }
	}
	/*--------------------------------------------------------------------------*/

	/*-- デフォルトのVertexBuffer取得 ------------------------------------------*/

	mdl = obj->model ;
	if( (vbuff_tbl_chk = DG_GetDGMdlVertexBuffer2(mdl, &vbuff_tbl)) )
	{
		obj->verts = (SVECTOR*)mdl->packs->verts ;
		obj->index = mdl->packs->index ;

		obj->n_verts    = vbuff_tbl.n_verts ;
		obj->n_indices  = vbuff_tbl.n_indices ;

		obj->d3d_vbuff  = vbuff_tbl.vbuff ;
		obj->d3d_ibuff  = vbuff_tbl.ibuff ;

		if( vbuff_tbl.flag & DG_MDL_VERTEX_BUFFER_TBL_ORDERED_INDEX )
		{
			obj->wflag |= DG_OBJ_WFLAG_ORDERED_INDEX ;
		}
		else
		{
			obj->wflag &= ~DG_OBJ_WFLAG_ORDERED_INDEX ;
		}
	}
	/*--------------------------------------------------------------------------*/

	/*-- 更新フラグOff ---------------------------------------------------------*/

	obj->ibuff_dirty  = FALSE ;
	obj->vbuff_dirty  = FALSE ;
	/*--------------------------------------------------------------------------*/

	/*-- Buffer新規作成 --------------------------------------------------------*/

	if( vbuff_tbl_chk && use_cvbuff )
	{
		size = stride * vbuff_tbl.n_indices ;

		DG_CreateVertexBuffer(size,
					DG_WinApp.obj_vbuff_usage | D3DUSAGE_WRITEONLY,
					0, OBJ_VERTEXBUFFER_POOL,
					&obj->d3d_cvbuff) ;

		obj->cvbuff_size = size ;

		obj->vbuff_dirty  = TRUE ;	// 転送は外部に任せる
		obj->cvbuff_dirty = TRUE ;
	}
	/*--------------------------------------------------------------------------*/

	return(TRUE) ;
}
