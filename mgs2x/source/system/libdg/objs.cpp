/*
    objs.c
    新物体ハンドラ処理ルーチン

    1999/07/07 K.Takabe
    $Id: objs.cpp,v 1.1.1.3 2002/11/19 11:42:15 Yoshizawa1 Exp $

*/

// TODO : VertexBufferの作成タイミングについて
//   現在、DG_MakeObjs の際にVertexBufferを作成しているが、
//   「頂点アニメしないもの」に関しては VertexBufferはモデル毎に一つで良い。

/*

    DG_OBJS        *DG_MakeObjs(def, flag, chanl)
    DG_DEF        *def ;        モデルデータ
    int        flag ;        処理フラグ
    int        chanl ;        描画チャンネル

        あるモデルデータに対応する、物体ハンドラを作成する


    DG_OBJS        *DG_MakeObjs2(DG_DEF *def, int flag, int chanl, DG_MDL **mdl_list, int n_list)
    DG_DEF        *def ;            基本モデルデータ
    int            flag ;            処理フラグ
    int            chanl ;            描画チャンネル
    DG_MDL        **mdl_list ;    各関節のモデルデータポインタの配列
    int            n_list ;        用意したモデルポインタ配列の要素数

        各関節を任意のモデルで初期化する

    void DG_ChangeModelObj(DG_OBJ *obj, DG_MDL *mdl)
    DG_OBJ        *obj ;    差し替えるオブジェクト
    DG_MDL        *mdl ;    差し替え用モデルデータ

        任意のオブジェクトのモデルを変更する

    void        DG_FreeObjs(objs)
    DG_OBJS        *objs ;        物体ハンドラ

        物体ハンドラと、そのパケットメモリを解放する
        （プレシェイドバッファは解放されないので注意）

        ->  プレシェイドバッファをobjs側に確保するようにしたのに伴い
            プレシェイドバッファも解放されるように変更 (97/11/08)

    void DG_ConnectObjs(DG_OBJS *parent, DG_OBJS *child)
    DG_OBJS        *parent ;    関連付けされる親オブジェクト
    DG_OBJS        *child ;    関連付けする子オブジェクト

        子オブジェクトを親オブジェクトに関連させる

    void DG_DisconnectObjs(DG_OBJS *parent, DG_OBJS *child)
    DG_OBJS        *parent ;    関連付けされる親オブジェクト
    DG_OBJS        *child ;    関連付けする子オブジェクト

        子オブジェクト関連をはずす

    void DG_WriteMdlPaketUV(int tri_code, DG_MDLPACK *pack)
    int            tri_code ;    使用するＴＲＩファイルのＩＤ
    DG_MDLPACK    *pack ;        補正するオブジェクトパケット

        テクスチャからＵＶ値を補正する


------------------------------------------------ 以下残骸
    void        DG_SetJointFrame(objs, rots)
    DG_OBJS        *objs ;        物体ハンドラ
    SVECTOR        *rots ;        回転関節データ

        物体ハンドラを、回転関節型に設定する

    void        DG_SetSlideFrame(objs, movs)
    DG_OBJS        *objs ;        物体ハンドラ
    SVECTOR        *movs ;        横すべり関節データ

        物体ハンドラを、横すべり回転関節型に設定する

*/

#ifndef KP_XBOX
#include <windows.h>
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "private.h" 

#ifdef DEBUG_MODE
#ifdef DG_MakeObjs
#undef DG_MakeObjs
#endif
#ifdef DG_MakeObjs2
#undef DG_MakeObjs2
#endif
#endif


//-----------------------------------------------------------------------------
// オブジェクトを生成する
//-----------------------------------------------------------------------------
DG_OBJS *DG_MakeObjs(DG_DEF *def, int flag, int chanl)
{
    DG_OBJS       *objs;
    DG_OBJ        *obj;
    DG_MDL        *mdl;
    DG_OBJ_PACKET    *pack;
    int        buf_size;
    int        i, j, n_models;
	DG_VBUF_INFO  *info;
	DG_INDEX_INFO *index;
	
    flag |= DG_FLAG_INVISIBLE2 | DG_FLAG_INVISIBLE3;
    ASSERT(def != NULL);
    n_models = def->n_x_models;

    /* 物体ハンドラのバッファを確保する */
    buf_size = sizeof(DG_OBJS) + sizeof(DG_OBJ) * n_models;
	//printf("n_models = %d, buf_size = %d\n", n_models, buf_size);
    if ((objs = (DG_OBJS *)GV_Malloc(buf_size)) == NULL){
        return NULL;
    }
    /*
        基本データ初期化
    */
    GV_ZeroMemory(objs, buf_size);
    if (def->data_format == MGS_MODEL_MULTITEX
		|| def->data_format == MGS_MODEL_MULTITEX_A) {
        flag |= DG_FLAG_MULTITEX;
    }
    objs->world = DG_UnitMatrix;
    objs->def = def;
    objs->n_models = n_models;
    objs->flag = flag;
    objs->chanl = chanl;
    objs->light = &DG_LightMatrix;
	objs->exec_func = NULL;

	// TODO:TRI_ID 取得
    //objs->tri_id = DG_SearchTriFromKms(def);
    objs->bound_min.vx = def->lx;
	objs->bound_min.vy = def->ly;
	objs->bound_min.vz = def->lz;
    objs->bound_min.vw = 1.0f;
    objs->bound_max.vx = def->ux;
	objs->bound_max.vy = def->uy;
	objs->bound_max.vz = def->uz;
    objs->bound_max.vw = 1.0f;
    objs->trans.vx = def->tx;
	objs->trans.vy = def->ty;
	objs->trans.vz = def->tz;
    objs->trans.vw = 1.0f;

    // 最初は全グループ表示にしておく
    objs->group_id = 0x7fffffff;    

    // 物体ユニットデータ初期化
    mdl = def->models;
    obj = objs->objs;

	// Xbox : ツール対応まで。まどろっこしい…。
	objs->tri_id = DG_GetTexture(mdl->packs->tex_id[0])->tri_id;
//	objs->tri_id = def->texture ;
	
    //printf("make objs ... %08x %d %d\n", def, def->texture, def->n_models);

	info = (DG_VBUF_INFO *)def->vbuf_info;  // 頂点バッファ/インデックスバッファ情報
	objs->vbuf_info = info;
	index = (DG_INDEX_INFO *)info->index;   // インデックス情報の先頭
    for (i = n_models; i > 0; -- i) {
        DG_MDLPACK    *mdlpack;
        /* 各オブジェクトユニットの設定 */
        obj->world = DG_UnitMatrix;
        obj->light = &DG_LightMatrix;
        obj->model = mdl;
        obj->flag = objs->flag;
        obj->n_packs = mdl->n_packs;
        obj->parent = mdl->parent;
        obj->mdl_type = mdl->type;
        obj->bound_min.vx = mdl->lx;
        obj->bound_min.vy = mdl->ly;
        obj->bound_min.vz = mdl->lz;
		obj->bound_min.vw = 1.0f ;
        obj->bound_max.vx = mdl->ux;
        obj->bound_max.vy = mdl->uy;
        obj->bound_max.vz = mdl->uz;
		obj->bound_max.vw = 1.0f ;
        obj->trans.vx = mdl->tx;
        obj->trans.vy = mdl->ty;
        obj->trans.vz = mdl->tz;
        obj->trans.vw = 1.0f;
		
        //obj->pre_packet = mdl->pre_packet;
        if (obj->n_packs == 0) {
            printf("warning: null packet\n");
            obj ++;
            mdl ++;
            continue;
        }
        // 頂点情報のポインタ設定
        obj->verts = (SVECTOR *)mdl->packs->verts;
        obj->norms = (SVECTOR *)mdl->packs->norms;
        obj->uvs[0] = (TVECTOR_S *)mdl->packs->uvs[0];
        obj->uvs[1] = (TVECTOR_S *)mdl->packs->uvs[1];
        obj->uvs[2] = (TVECTOR_S *)mdl->packs->uvs[2];
        //printf("%08x %08x %08x %08x %08x \n",
		//obj->verts, obj->norms, obj->uvs[0], obj->uvs[1], obj->uvs[2]);

		/* 各パケット情報の設定 */
		pack = (DG_OBJ_PACKET *)GV_Malloc(sizeof(DG_OBJ_PACKET) * obj->n_packs);
        obj->packets = pack;
        GV_ZeroMemory(pack, sizeof(DG_OBJ_PACKET) * obj->n_packs);
        if (pack == NULL){
            printf("create objs error!!\n");
            DG_FreeObjs(objs);
            return (NULL);
        }
        mdlpack = mdl->packs;
		
        for (j = 0; j < (int)mdl->n_packs; j ++) {
#if 0
            if (mdlpack->n_verts > 64) {
				printf("too many n_verts(%d)\n",mdlpack->n_verts);
			}
#endif
			// VertexBuffer生成/データセット
			//pack->vbuf_info = (DG_VBUF_INFO *)def->vbuf_info;
			// mdlpack へのポインタを代入
			pack->mdlpack = mdlpack;
			
			// TODO: マルチテクスチャ
			// テクスチャIDのセット
			if (mdlpack->tex_id[0] == 0) {
				printf("NULL TEXTURE...\n");
				obj->packets[j].ptex = NULL;
			} else {
				obj->packets[j].ptex = ((DG_TEX *)mdlpack->tex_id[0])->tex_trans.ptex;
			}

            if (obj->uvs[0] == NULL && mdlpack->uvs[0] != NULL) obj->uvs[0] = (TVECTOR_S *)mdlpack->uvs[0];
            if (obj->uvs[1] == NULL && mdlpack->uvs[1] != NULL) obj->uvs[1] = (TVECTOR_S *)mdlpack->uvs[1];
            if (obj->uvs[2] == NULL && mdlpack->uvs[2] != NULL) obj->uvs[2] = (TVECTOR_S *)mdlpack->uvs[2];
            pack->flag = mdlpack->flag;
            //pack->data_ptr = (void *)mdlpack->pad;
            pack->n_verts = mdlpack->n_verts;
            pack->norms_offset = pack->verts_offset =
					(mdlpack->n_verts + 1) / 2;
            pack->uvs_offset[2] = pack->uvs_offset[1] = pack->uvs_offset[0] =
					(mdlpack->n_verts + 3) / 4;
            if (!(pack->flag & DG_PACKFLAG_UV0)) pack->uvs_offset[0] = 0;
            if (!(pack->flag & DG_PACKFLAG_UV1)) pack->uvs_offset[1] = 0;
            if (!(pack->flag & DG_PACKFLAG_UV2)) pack->uvs_offset[2] = 0;
            //if (flag & DG_FLAG_MULTITEX) printf("%08x %d \n", pack->flag, pack->n_verts);
			// テクスチャパラメータ指定
            if (mdlpack->tex_id[0] != 0) {
				pack->tex_ptr[0] = &((DG_TEX *)mdlpack->tex_id[0])->tex_trans;
			}
            if (mdlpack->tex_id[1] != 0) {
				pack->tex_ptr[1] = &((DG_TEX *)mdlpack->tex_id[1])->tex_trans;
			}
            if (mdlpack->tex_id[2] != 0) {
				pack->tex_ptr[2] = &((DG_TEX *)mdlpack->tex_id[2])->tex_trans;
			}
			pack->index = index++; // インデックスバッファ情報

			//printf("MakeObj\n");
			//printf("index->min = %d\n", index->min);
			//printf("index->num = %d\n", index->num);
			//printf("index->start = %d\n", index->start);
			//printf("index->prims = %d\n", index->prims);
			
            pack ++;
            mdlpack ++;
        }
        obj ++;
        mdl ++;
    }
    return objs;
}

/* 関節毎にモデルを指定 */
DG_OBJS *DG_MakeObjs2(DG_DEF *def, int flag, int chanl, DG_MDL **mdl_list, int n_list)
{
    DG_OBJS       *objs;
    DG_OBJ        *obj;
    DG_MDL        *mdl;
    DG_OBJ_PACKET *pack;
    int        buf_size;
    int        i, j, n_models;
	DG_VBUF_INFO  *info;
	DG_INDEX_INFO *index;

    flag |= DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3;
    ASSERT(def != NULL);
    n_models = def->n_x_models;
    /*
        物体ハンドラのバッファを確保する
    */
    buf_size = sizeof(DG_OBJS) + sizeof(DG_OBJ) * n_models;
    if ((objs = (DG_OBJS *)GV_Malloc(buf_size)) == NULL){
        return NULL;
    }
    /*
        基本データ初期化
    */
    if (def->data_format == MGS_MODEL_MULTITEX || def->data_format == MGS_MODEL_MULTITEX_A){
        flag |= DG_FLAG_MULTITEX;
    }
    GV_ZeroMemory(objs, buf_size);
    objs->world = DG_UnitMatrix;
    objs->def = def;
    objs->n_models = n_models;
    objs->flag = flag;
    objs->chanl = chanl;
    objs->light = &DG_LightMatrix;
	objs->exec_func = NULL;	
	// TODO: DG_SearchTriFromKms
    //objs->tri_id = DG_SearchTriFromKms(def);
    objs->bound_min.vx = def->lx;
    objs->bound_min.vy = def->ly;
    objs->bound_min.vz = def->lz;
    objs->bound_min.vw = 1.0f;
    objs->bound_max.vx = def->ux;
    objs->bound_max.vy = def->uy;
    objs->bound_max.vz = def->uz;
    objs->bound_max.vw = 1.0f;
    objs->trans.vx = def->tx;
    objs->trans.vy = def->ty;
    objs->trans.vz = def->tz;
    objs->trans.vw = 1.0f;

    /* 最初は全グループ表示にしておく（2000/01/20 M.Sonoyama） */
    objs->group_id = 0x7fffffff;    

    /*
        物体ユニットデータ初期化
    */
    mdl = def->models;
    obj = objs->objs;

	// Xbox : ツール対応まで。まどろっこしい…。
	objs->tri_id = DG_GetTexture(mdl->packs->tex_id[0])->tri_id;
	//objs->tri_id = def->texture ;
	
	info = (DG_VBUF_INFO *)def->vbuf_info;  // 頂点バッファ/インデックスバッファ情報
	objs->vbuf_info = info;
	index = (DG_INDEX_INFO *)info->index;   // インデックス情報の先頭
    for (i = n_models; i > 0; -- i) {
        DG_MDLPACK    *mdlpack;
        /* 各オブジェクトユニットの設定 */
        obj->world = DG_UnitMatrix;
        obj->light = &DG_LightMatrix;
        obj->flag = objs->flag;
        if (n_list > 0 && *mdl_list != NULL){
            obj->model = *mdl_list;
        } else {
            obj->model = mdl;
        }
        obj->n_packs = obj->model->n_packs;
        obj->parent = obj->model->parent;
        obj->mdl_type = obj->model->type;
        obj->bound_min.vx = obj->model->lx;
        obj->bound_min.vy = obj->model->ly;
        obj->bound_min.vz = obj->model->lz;
        obj->bound_min.vw = 1.0f ;
        obj->bound_max.vx = obj->model->ux;
        obj->bound_max.vy = obj->model->uy;
        obj->bound_max.vz = obj->model->uz;
        obj->bound_max.vw = 1.0f ;
        obj->trans.vx = obj->model->tx;
        obj->trans.vy = obj->model->ty;
        obj->trans.vz = obj->model->tz;
        obj->trans.vw = 1.0f;
        /* 頂点情報のポインタ設定 */
        obj->verts = (SVECTOR *)obj->model->packs->verts;
        obj->norms = (SVECTOR *)obj->model->packs->norms;
        obj->uvs[0] = (TVECTOR_S *)obj->model->packs->uvs[0];
        obj->uvs[1] = (TVECTOR_S *)obj->model->packs->uvs[1];
        obj->uvs[2] = (TVECTOR_S *)obj->model->packs->uvs[2];
        /* 各パケット情報の設定 */
		pack = (DG_OBJ_PACKET *)GV_Malloc(sizeof(DG_OBJ_PACKET) * obj->n_packs);
        obj->packets = pack;
        GV_ZeroMemory(pack, sizeof(DG_OBJ_PACKET) * obj->n_packs);
        if (pack == NULL){
            printf("create objs error!!\n");
            DG_FreeObjs(objs);
            return (NULL);
        }
        mdlpack = obj->model->packs;
        for (j = 0; j < obj->n_packs; j++){
            if (obj->uvs[0] == NULL && mdlpack->uvs[0] != NULL) obj->uvs[0] = (TVECTOR_S *)mdlpack->uvs[0];
            if (obj->uvs[1] == NULL && mdlpack->uvs[1] != NULL) obj->uvs[1] = (TVECTOR_S *)mdlpack->uvs[1];
            if (obj->uvs[2] == NULL && mdlpack->uvs[2] != NULL) obj->uvs[2] = (TVECTOR_S *)mdlpack->uvs[2];
            pack->flag = mdlpack->flag;
            //pack->data_ptr = (void *)mdlpack->pad;
            pack->n_verts = mdlpack->n_verts;
            pack->norms_offset = pack->verts_offset =
              (mdlpack->n_verts + 1) / 2;
            pack->uvs_offset[2] = pack->uvs_offset[1] = pack->uvs_offset[0] =
              (mdlpack->n_verts + 3) / 4;
            if (!(pack->flag & DG_PACKFLAG_UV0)) pack->uvs_offset[0] = 0;
            if (!(pack->flag & DG_PACKFLAG_UV1)) pack->uvs_offset[1] = 0;
            if (!(pack->flag & DG_PACKFLAG_UV2)) pack->uvs_offset[2] = 0;
			// TODO : TEXTRANS
            if (mdlpack->tex_id[0] != 0) {
				pack->tex_ptr[0] = &((DG_TEX*)mdlpack->tex_id[0])->tex_trans;
			}
            if (mdlpack->tex_id[1] != 0) {
				pack->tex_ptr[1] = &((DG_TEX*)mdlpack->tex_id[1])->tex_trans;
			}
            if (mdlpack->tex_id[2] != 0) {
				pack->tex_ptr[2] = &((DG_TEX*)mdlpack->tex_id[2])->tex_trans;
			}
			pack->index = index++ ; // インデックスバッファ情報
            pack++;
            mdlpack++;
        }
        obj++;
        mdl++;
        mdl_list++;
        n_list--;
    }
    return objs;
}

/* 部分的にモデルを変更する */
void DG_ChangeModelObj(DG_OBJ *obj, DG_MDL *mdl)
{
    int        i;
    DG_OBJ_PACKET    *pack;
    DG_MDLPACK        *mdlpack;
    /* プリシェード型オブジェクトには未対応なので注意 */
    ASSERT(!(obj->flag & DG_FLAG_PAINT));

    /* メモリ確保 */
    pack = (DG_OBJ_PACKET *)GV_Malloc(sizeof(DG_OBJ_PACKET) * mdl->n_packs);
    if (pack == NULL){
        printf("obj change error\n");
        return;
    }

    /* メモリ開放 */
    GV_Free(obj->packets);
    /* DG_OBJの再設定 */
    obj->model = mdl;
    obj->n_packs = mdl->n_packs;
    obj->packets = pack;
    obj->verts = (SVECTOR *)mdl->packs->verts;
    obj->norms = (SVECTOR *)mdl->packs->norms;
    obj->uvs[0] = (TVECTOR_S *)mdl->packs->uvs[0];
    obj->uvs[1] = (TVECTOR_S *)mdl->packs->uvs[1];
    obj->uvs[2] = (TVECTOR_S *)mdl->packs->uvs[2];
    obj->parent = mdl->parent;
    obj->mdl_type = mdl->type;
    obj->bound_min.vx = mdl->lx;
    obj->bound_min.vy = mdl->ly;
    obj->bound_min.vz = mdl->lz;
	obj->bound_min.vw = 1.0f ;
    obj->bound_max.vx = mdl->ux;
    obj->bound_max.vy = mdl->uy;
    obj->bound_max.vz = mdl->uz;
	obj->bound_max.vw = 1.0f ;
    obj->trans.vx = mdl->tx;
    obj->trans.vy = mdl->ty;
    obj->trans.vz = mdl->tz;
    obj->trans.vw = 1.0f;
    //obj->pre_packet = mdl->pre_packet;
    mdlpack = mdl->packs;
    for (i = 0; i < obj->n_packs; i++){
        if (obj->uvs[0] == NULL && mdlpack->uvs[0] != NULL) obj->uvs[0] = (TVECTOR_S *)mdlpack->uvs[0];
        if (obj->uvs[1] == NULL && mdlpack->uvs[1] != NULL) obj->uvs[1] = (TVECTOR_S *)mdlpack->uvs[1];
        if (obj->uvs[2] == NULL && mdlpack->uvs[2] != NULL) obj->uvs[2] = (TVECTOR_S *)mdlpack->uvs[2];
        pack->flag = mdlpack->flag;
        //pack->data_ptr = (void *)mdlpack->pad;
        pack->n_verts = mdl->packs[i].n_verts;
        pack->norms_offset = pack->verts_offset = (mdlpack->n_verts + 1) / 2;
        pack->uvs_offset[2] = pack->uvs_offset[1] = pack->uvs_offset[0] = (mdlpack->n_verts + 3) / 4;
        if (!(pack->flag & DG_PACKFLAG_UV0)) pack->uvs_offset[0] = 0;
        if (!(pack->flag & DG_PACKFLAG_UV1)) pack->uvs_offset[1] = 0;
        if (!(pack->flag & DG_PACKFLAG_UV2)) pack->uvs_offset[2] = 0;
		// TODO: DG_TEX
        if (mdlpack->tex_id[0] != 0)    {
			pack->tex_ptr[0] = &((DG_TEX*)mdlpack->tex_id[0])->tex_trans;
		} else {
			pack->tex_ptr[0] = NULL;
		}
		
        if (mdlpack->tex_id[1] != 0) {
			pack->tex_ptr[1] = &((DG_TEX*)mdlpack->tex_id[1])->tex_trans;
		} else {
			pack->tex_ptr[1] = NULL;
		}
        if (mdlpack->tex_id[2] != 0) {
			pack->tex_ptr[2] = &((DG_TEX*)mdlpack->tex_id[2])->tex_trans;
		} else {
			pack->tex_ptr[2] = NULL;
		}
        pack ++;
        mdlpack ++;
    }
}

void DG_FreeObjs(DG_OBJS *objs)
{
    int i;
    DG_OBJ *obj;

    if (objs == NULL) return;

	// TODO : DG_FreePreshade
    //DG_FreePreshade(objs);

    obj = objs->objs;
    for (i = objs->n_models; i > 0; -- i) {
        if (obj->packets != NULL){
#if 0
			// 頂点バッファ解放
			for (j = obj->n_packs; j > 0; -- j) {
				obj->packets[j - 1].verts_buf->Release();
			}
#endif
            GV_Free(obj->packets);
        }
        obj ++;
    }
    GV_Free(objs);

#if 0 // PS2版
    obj = objs->objs;
    for (i = objs->n_models; i > 0; -- i) {
        if (obj->packets != NULL){
            GV_Free(obj->packets);
        }
        obj ++;
    }
    GV_Free(objs);
#endif
}

    /*
        子オブジェクトを親オブジェクトに関連させる
    */
void DG_ConnectObjs(DG_OBJS *parent, DG_OBJS *child)
{
    //printf("objs.c: connect objs start (%08x,%08x) time=%d\n", parent, child, GV_Time);
    while (parent->next != NULL){
        //printf("objs.c: next = %08x\n", parent->next);
        parent = parent->next;
    }
    //printf("objs.c: connect objs end(%08x)\n", child->next);
    parent->next = child;
}

    /*
        子オブジェクト関連をはずす
    */
void DG_DisconnectObjs(DG_OBJS *parent, DG_OBJS *child)
{
    /* 親が先に死んでいると不具合が起こる可能性があるため予めチェックしておく */
    if (DG_SearchQueueObjs(parent) == NULL) return;

    while (parent->next != child){
        if (parent == NULL){
            printf("Disconnect error!!\n");
            return;
        }
        parent = parent->next;
    }
    parent->next = parent->next->next;
}



/*----------------------------------------------------------------*/

void DG_WriteMdlPaketUV(int tri_code, DG_MDLPACK *pack)
{
    float  u_scale, v_scale, u_offset, v_offset;
    short  *uv;
    DG_TEX *tex;
    int    i, j;
    static int uv_check_table[3] = {
		DG_PACKFLAG_UV0,
		DG_PACKFLAG_UV1 | DG_PACKFLAG_BMAP,
		DG_PACKFLAG_UV2 | DG_PACKFLAG_BMAP
	};

    //printf("%d %d %d\n", pack->tex_id[0], pack->tex_id[1], pack->tex_id[2]);
	//printf(":tex_id = %08x %08x %08x\n",
	//pack->tex_id[0], pack->tex_id[1], pack->tex_id[2]);
    for (i = 0; i < 3; i ++) {
        if (pack->tex_id[i] == 0 || pack->tex_id[i] == 1) {
            if (i == 0) {
                if (pack->tex_id[0] != 0){
                    printf("warning: no texture packet %08x\n", pack->flag);
                }
				// TODO: DG_GetTexture2
                pack->tex_id[i] = (int)DG_GetTexture2(tri_code, pack->tex_id[i]);
				//pack->tex_id[i] = (int)DG_GetTexture(pack->tex_id[i]);
            }
            continue;
        }
        /* ＵＶ値のスケール補正（既にテクスチャが読み込まれている必要あり） */
		// TODO:DG_GetTexture2
        tex = DG_GetTexture2(tri_code, pack->tex_id[i]);
		//tex = DG_GetTexture(pack->tex_id[i]);
        if (tex != NULL){
            u_scale = tex->u_scale;
            v_scale = tex->v_scale;
            u_offset = tex->u_offset * 4096.0f;
            v_offset = tex->v_offset * 4096.0f;
        } else {
            u_scale = v_scale = 1.0f;
            u_offset = v_offset = 0.0f;
        }
        uv = (short *)pack->uvs[i];
        if (uv != NULL){
            for (j = pack->n_verts; j > 0; j --) {
                uv[0] = (short)DG_FTOI((float)uv[0] * u_scale + u_offset);
                uv[1] = (short)DG_FTOI((float)uv[1] * v_scale + v_offset);
                uv += 2;
            }
        }
        pack->tex_id[i] = (int)tex;
    }
}

/*----------------------------------------------------------------*/

// デバッグ用・名前付きDG_OBJS
#ifdef DEBUG_MODE
DG_OBJS *DG_MakeObjsD(DG_DEF *def, int flag, int chanl, char *fname)
{
    DG_OBJS *objs;
    objs = DG_MakeObjs(def, flag, chanl);
    objs->fname = fname;
    return objs;
}

DG_OBJS *DG_MakeObjs2D(DG_DEF *def, int flag, int chanl,
					   DG_MDL **mdl_list, int n_list, char *fname)
{
    DG_OBJS *objs;
    objs = DG_MakeObjs2(def, flag, chanl, mdl_list, n_list);
    objs->fname = fname;
	return objs;
}
#endif

/*----------------------------------------------------------------*/
