/*
 chanl.cpp
 チャンネル管理ルーチン

 1999/07/07 K.Takabe
 $Id: chanl.cpp,v 1.1.1.3 2002/11/19 11:42:03 Yoshizawa1 Exp $

*/

/*
  2001/03/22 F.Miyauchi(xbox)
  (chanl.c,v 1.61 2001/03/11 23:54:20 usr02774)
*/

/*

    void        DG_InitChanlSystem(shift)
    int        shift ;        ダブルバッファシフト幅

        モジュール初期化

    void        DG_ResetChanlSystem(flag)
    int        flag ;        フラグ

        モジュール再初期化（ステージロード時など）

    void        DG_DrawChanlSystem(which)
    int        which ;        ダブルバッファ選択

        描画開始（ DrawOTag() ）

    void        DG_ClearChanlSystem(which)
    int        which ;        ダブルバッファ選択

        フレーム開始処理（ ClearOTagR() など）

    void        DG_SortChanlSystem(which)
    int        which ;        ダブルバッファ選択

        フレーム終了処理（次のフレームのパケットを生成）

    void        DG_SetDrawEnv(chanl, env)
    int        chanl ;        チャンネル番号
    DRAWENV        *env ;        描画環境

        指定チャンネルの描画環境を変更する
        変更が有効になるのは、次のフレームから

    ------------------------------------------------

    int        DG_QueueObjs(objs)
    DG_OBJS        *objs ;        物体ハンドラ

        物体ハンドラをキューに登録

    void        DG_DequeueObjs(objs)
    DG_OBJS        *objs ;        物体ハンドラ

        物体ハンドラをキューから削除

    int        DG_QueuePrim(prim)
    DG_PRIM        *prim ;        プリミティブ

        プリミティブをキューに登録

    void        DG_DequeuePrim(prim)
    DG_PRIM        *prim ;        プリミティブ

        プリミティブをキューから削除
*/

#ifndef KP_XBOX
#include <windows.h>
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "private.h"

//-----------------------------------------------------------------------------

// 特権モードフラグ
int DG_PrivilegeMode = 0;

// チャンネルリスト
DG_CHANL        DG_Chanls[DG_MAX_CHANLS]; /* チャンネル＝カメラとして見ていいかも */
DG_OBJ_QUEUE    DG_ObjQueue;

/*
  共通オブジェクトキューバッファ
  （各オブジェクトキューに割り振られる。最終的には動的に割り振りを変更できるようにしたい）
*/
#define MAX_QUEUE_BUFFER    (16*1024)
int     DG_UseQueueBuffer = 0;                /* 共通キューバッファ使用量 */
void    *DG_QueueBuffer[MAX_QUEUE_BUFFER];    /* 共通キューバッファ */

/*
  プリミティブオフセット設定パケット
*/
//static DG_DRAWOFFSET DG_DrawOffset[2];        /* ハイレゾ用 */
//static DG_DRAWOFFSET DG_DrawOffsetLow[2];    /* ローレゾ用 */

//-----------------------------------------------------------------------------
// チャンネル処理ユニット
//-----------------------------------------------------------------------------

// ダミー
void DG_DummyChanl(DG_CHANL *chanl, int which)
{
	//printf("DG_DummyChanl(%d)\n", which);
}

static QueFunc NormalStageList[] = {/* 標準処理ステージ */
    DG_PluginStartChanl,       /* ●プラグイン初期化ステージ */
    DG_FrameChanl,             /* 描画環境初期化＆設定ステージ */
    DG_PluginChanl,            /* ●プラグイン処理ステージ（０） */
    DG_ScreenChanl,            /* マトリクス計算＆バウンディングチェック */
    //DG_Chain2Chanl,            /* マルチテクスチャオブジェクトのＤＭＡ接続ステージ */
    DG_ChainEvmChanl,          /* マルチウェイトエンベロープオブジェクト処理ステージ */
    DG_ChainChanl,             /* 不透明オブジェクトのＤＭＡ接続ステージ */
    //DG_Chain2ChanlLatter,      /* マルチテクスチャオブジェクトのＤＭＡ接続ステージ */
	DG_ChainEvmChanlLatter,    /* マルチウェイトエンベロープオブジェクト処理ステージ */
    DG_ShadowChainChanl,       /* (特殊)影投影処理ステージ */
    //DG_SpotChainChanl,         /* (特殊)スポットライト投影ステージ */
    DG_PluginChanl,            /* ●プラグイン処理ステージ（１） */
    DG_Prim2Chanl,             /* プリミティブソート処理ステージ */
    DG_SortChainChanl,         /* 半透明オブジェクト＆プリミティブのＤＭＡ接続ステージ */
	DG_ParticleChanl,          /* パーティクル処理ステージ */
	DG_ShadowVolChanl,         /* シャドウボリューム処理ステージ */
    DG_PluginChanl,            /* ●プラグイン処理ステージ（２） */
    DG_PluginChanl,            /* ●プラグイン処理ステージ（３） */
    DG_PluginEndChanl,         /* ●プラグイン終了ステージ */
    DG_DummyChanl
};

static QueFunc MenuStageList[] = {
    DG_PluginStartChanl,       /* ●プラグイン初期化ステージ */
    DG_FrameChanl,             /* 描画環境初期化＆設定ステージ */
//    DG_PluginChanl,            /* ●プラグイン処理ステージ（０） */
//    DG_PluginChanl,            /* ●プラグイン処理ステージ（１） */
//    DG_PluginChanl,            /* ●プラグイン処理ステージ（２） */
//    DG_PluginChanl,            /* ●プラグイン処理ステージ（３） */
    DG_PluginEndChanl,         /* ●プラグイン終了ステージ */
    DG_DummyChanl
};

#ifndef    NO_PROTOTYPE
static    void    SetParam(DG_CHANL *, int);
#endif

//-----------------------------------------------------------------------------

// 描画環境を設定する
//static void SetDefDrawEnv(DG_DRAWENV *env, DG_DRAWOFFSET *offset, int page,
//						  int x, int y, int w, int h)
//{
	// GS依存部分のためカット
//}

// デフォルトの描画環境を設定
void DG_SetDrawEnv(DG_CHANL *cp, int x, int y, int w, int h)
{
    cp->width = w;
    cp->height = h;
    cp->offset_x = x;
    cp->offset_y = y;

    //SetDefDrawEnv(&cp->draw_env[0], &cp->draw_offset[0], 0, x, y, w, h);
    //SetDefDrawEnv(&cp->draw_env[1], &cp->draw_offset[1], 1, x, y, w, h);
}

// 描画領域の制限を行う（シネマカット時の負荷軽減用なので使用には注意！）
void DG_ChangeDrawLimit(DG_CHANL *cp, int x1, int y1, int x2, int y2)
{
	//GS依存
    //*(u_long64*)&cp->draw_env[0].datas.scissor1 =  SCE_GS_SET_SCISSOR(x1, x2, y1, y2);
    //*(u_long64*)&cp->draw_env[1].datas.scissor1 =  SCE_GS_SET_SCISSOR(x1, x2, y1, y2);
}

// 指定したチャンネルの描画環境復元ＤＭＡパケットを指定アドレスに書き込む
void *DG_PopDefaultDrawEnv(DG_CHANL *cp, void *addr)
{
	//PS2依存
    return addr;
}

//-----------------------------------------------------------------------------

// チャンネルにパラメータを設定
static void SetParam(DG_CHANL *cp, int obj_num)
{
    cp->flag = 0;
    cp->group_id = 1;
    if (obj_num < 0){
        cp->obj_queue = NULL;
    } else {
        /* オブジェクトキューの設定 */
        cp->obj_queue = &DG_ObjQueue;
    }
}

// オブジェクトキューバッファを確保する
static void DG_AllocObjectBuffer(DG_OBJ_BUFFER *obj_buff, int max, int id)
{
    obj_buff->max_queue = max;
    obj_buff->n_queue = 0;
    obj_buff->queue = &DG_QueueBuffer[DG_UseQueueBuffer];
    obj_buff->id = id;
    DG_UseQueueBuffer += max;
    //printf("%d %d\n", DG_UseQueueBuffer, MAX_QUEUE_BUFFER);
    ASSERT(DG_UseQueueBuffer <= MAX_QUEUE_BUFFER);
}

// プラグイン用オブジェクトキューバッファを確保する
DG_OBJ_BUFFER *DG_MakeUserObjectBuffer(int max, int id)
{
    int        i;
    DG_OBJ_BUFFER    *obj_buff;

    for (i = 0; i < DG_MAX_USR_OBJQUEUE; i++){
        obj_buff = &DG_ObjQueue.user_buffer[i];
        if (obj_buff->id == 0){
            obj_buff->max_queue = max;
            obj_buff->n_queue = 0;
            obj_buff->queue = &DG_QueueBuffer[DG_UseQueueBuffer];
            obj_buff->id = id;
            DG_UseQueueBuffer += max;
            ASSERT(DG_UseQueueBuffer <= MAX_QUEUE_BUFFER);
            return (obj_buff);
        }
    }
    /* 空きオブジェクトキューバッファがない場合 */
    ASSERT(0);
    return (NULL);
}

// プラグイン用オブジェクトキューバッファを開放する
void DG_FreeUserObjectBuffer(DG_OBJ_BUFFER *obj_buff)
{
    obj_buff->id = 0;
}

//-----------------------------------------------------------------------------

void DG_InitChanlSystem(int flag)
{
    DG_CHANL *cp;
    //int i;

    /*
	  各オブジェクトキューの初期化
    */
    /* 新初期化ルーチン（各オブジェクトキューにバッファを割り当てる） */
    DG_UseQueueBuffer = 0;
    GV_ZeroMemory(&DG_ObjQueue, sizeof(DG_OBJ_QUEUE));
    DG_AllocObjectBuffer(&DG_ObjQueue.objs_buffer, 512+256, 0);
    DG_AllocObjectBuffer(&DG_ObjQueue.shdwwrite_buffer, 512, 0);
    DG_AllocObjectBuffer(&DG_ObjQueue.spot_buffer, 512, 0);
    DG_AllocObjectBuffer(&DG_ObjQueue.evmobj_buffer, 512, 0);
    DG_AllocObjectBuffer(&DG_ObjQueue.prim2_buffer, 512, 0);
    DG_AllocObjectBuffer(&DG_ObjQueue.particle_buffer, 512, 0);	

    /*
	  メインチャンネル（チャンネル０）
    */
    cp = DG_Chanls;
    SetParam(cp, 0);
    DG_SetDrawEnv(cp, 0, 0, DRAW_WIDTH, DRAW_HEIGHT);
    cp->bg_clear_flag = 1;
    cp->chanl_num = 0;
    cp->high_reso = 1;
    cp->flag = 1;
    cp->n_stage = sizeof(NormalStageList) / sizeof(QueFunc);
    cp->stage_list = NormalStageList;
    cp ++;
    /*
	  チャンネル１
    */
    SetParam(cp, 0);
    DG_SetDrawEnv(cp, DRAW_WIDTH/4, DRAW_HEIGHT/3, DRAW_WIDTH/3, DRAW_HEIGHT/4);
    cp->bg_clear_flag = 1;
    cp->chanl_num = 1;
    cp->high_reso = 1;
    cp->flag = 0;
    cp->n_stage = sizeof(NormalStageList) / sizeof(QueFunc);
    cp->stage_list = NormalStageList;
    cp ++;
    /*
	  チャンネル２
    */
    SetParam(cp, 0);
    DG_SetDrawEnv(cp, 0, 0, DRAW_WIDTH, DRAW_HEIGHT);
    cp->bg_clear_flag = 1;
    cp->chanl_num = 2;
    cp->high_reso = 1;
    cp->flag = 0;
    cp->n_stage = sizeof(NormalStageList) / sizeof(QueFunc);
    cp->stage_list = NormalStageList;
    cp ++;
    /*
	  チャンネル３
    */
    SetParam(cp, 0);
    DG_SetDrawEnv(cp, 0, 0, DRAW_WIDTH, DRAW_HEIGHT);
    cp->bg_clear_flag = 1;
    cp->chanl_num = 3;
    cp->high_reso = 1;
    cp->flag = 0;
    cp->n_stage = sizeof(NormalStageList) / sizeof(QueFunc);
    cp->stage_list = NormalStageList;
    cp ++;
    /*
	  メニュー用チャンネル（チャンネル４）
    */
    SetParam(cp, 4);
    DG_SetDrawEnv(cp, 0, 0, DRAW_WIDTH, DRAW_HEIGHT);
	// GS依存
    //cp->draw_env[0].datas.test1.ZTE = 1;
    //cp->draw_env[0].datas.test1.ZTST = 1;
    //cp->draw_env[1].datas.test1.ZTE = 1;
    //cp->draw_env[1].datas.test1.ZTST = 1;
    cp->bg_clear_flag = 0;
    cp->chanl_num = 4;
    cp->high_reso = 0;        /* メニューはデフォルトではローレゾに */
    cp->flag = 1;
    cp->n_stage = sizeof(MenuStageList) / sizeof(QueFunc);
    cp->stage_list = MenuStageList;
    cp ++;

    /*
	  プラグインシステム初期化
    */
	DG_InitPluginSystem();

	/*
	  ＭＧＳ２デフォルト起動プラグイン組み込み
    */
    //DG_AddPluginComdl();
	//extern void DG_AddPluginDmapack( void );
	//DG_AddPluginDmapack();

	// コールバック専用プラグイン(Xbox新規)
	DG_AddPluginCallbackObj();
	
	//..メニューのプラグインテスト
	extern void PluginTest(void);
	PluginTest();
	
	
	
    //DG_AddPluginOptcmf();
    /* 実験組み込み */
    //DG_AddPluginPatch();
}

void DG_ResetChanlSystem(int flag)
{
    DG_Chanls[0].flag = 1; // メインチャンネル
    DG_Chanls[1].flag = 0;
    DG_Chanls[2].flag = 0;
    DG_Chanls[3].flag = 0;
    DG_Chanls[4].flag = 1; // メニューチャンネル
    DG_Chanls[0].bg_clear_flag = 1;
    DG_Chanls[1].bg_clear_flag = 1;
    DG_Chanls[2].bg_clear_flag = 1;
    DG_Chanls[3].bg_clear_flag = 1;
    DG_Chanls[4].bg_clear_flag = 0;
    DG_PrivilegeMode = 0;
}

// チェック用/仮
static void PrintMatrix(char *str, FMATRIX *mat)
{
	printf(str);
	printf("%.3f %.3f %.3f %.3f\n"
		   "%.3f %.3f %.3f %.3f\n"
		   "%.3f %.3f %.3f %.3f\n"
		   "%.3f %.3f %.3f %.3f\n",
		   mat->m[0][0],
		   mat->m[0][1],
		   mat->m[0][2],
		   mat->m[0][3],
		   mat->m[1][0],
		   mat->m[1][1],
		   mat->m[1][2],
		   mat->m[1][3],
		   mat->m[2][0],
		   mat->m[2][1],
		   mat->m[2][2],
		   mat->m[2][3],
		   mat->m[3][0],
		   mat->m[3][1],
		   mat->m[3][2],
		   mat->m[3][3]);
}

// chain.cpp に移動
void DG_DrawChanlSystem(int which)
{
}

void DG_ClearChanlSystem(int which)
{
    /* 描画ＤＭＡバッファ初期化 */
    //DG_DmaClear(which);
}

void DG_UnDrawChanlSystem(int which)
{
    //DG_RetryStoreChanl();
    /* 描画ＤＭＡ動作のスキップ */
    //DG_DmaSkip(which);
}

static long64 stop_chanl_system_flag = 0;

void DG_SortChanlSystem(int which)
{
    DG_CHANL *cp;
    int      i, j;

	// チャネルシステム停止
    if (stop_chanl_system_flag) {
        return;
    }

	// 画面を消す前にイメージストアパケットを接続する
	// TODO : DG_StoreChanl / XBOXでは無理
    //DG_StoreChanl(NULL, 1 - which);        /* イメージストア処理 */

	// チャンネル処理ユニットを、各チャンネルに実行する
    cp = DG_Chanls;

	GV_PROFILE_CHANL_START();

	//static float chanl_time[128];
	//int disp_flag = 0;
    for (j = DG_MAX_CHANLS; j > 0; j --, cp ++) {
        if (cp->flag == 0) continue;

        /* テクスチャの多重ロードチェック用初期化 */
        for (i = 0; i < DG_MaxTextures; i ++) {
            DG_TextureList[i].flag = 0;
        }
        /* チャンネルの各描画フェースを処理 */
        for (i = 0; i < cp->n_stage; i ++) {
			//float time0, time1;
			//time0 = DG_GetMsecTimer();
            (*cp->stage_list[i])(cp, which);
			//time1 = DG_GetMsecTimer();			
            GV_PROFILE_CHANL();

			//if (j == DG_MAX_CHANLS) {
			//chanl_time[i] = time1 - time0;
			//if (chanl_time[i] > 15.0f) disp_flag = 1;
			//}
        }
    }

	//if (disp_flag) {
	//for (i = 0; i < DG_Chanls[0].n_stage; i++) {
	//printf("chanl_time[%d] = %f\n", i, chanl_time[i]);
	//}
	//}
}

//-----------------------------------------------------------------------------

// オブジェクト登録
static int QueueObjs(DG_OBJ_BUFFER *obj_buff, void *objs)
{
    int n;

    if (objs == NULL) return -1;

	//printf("######## QueueObjs\n");
	//printf("%08x,%08x %d %d %08x\n", obj_buff, obj_buff->queue,
	//obj_buff->n_queue, obj_buff->max_queue, objs);

	// 登録順番をくずさないようにキューに追加する
    n = obj_buff->n_queue;
    if (n >= obj_buff->max_queue) {
        //printf("QueueObjs : queue buffer over!! (%d)\n", n);
        return - 1;
    }
    obj_buff->queue[n ++] = objs;
    obj_buff->n_queue = n;
    return 0;
}

// オブジェクト削除
static void DequeueObjs(DG_OBJ_BUFFER *obj_buff, void *objs)
{
    void **oque;
    int  i, n;

    // キュー登録位置を検索
    oque = obj_buff->queue;
    n = obj_buff->n_queue;
    for (i = n; i > 0; -- i) {
        if (*oque == objs) goto found;
        oque ++;
    }
    WARNING("dequeued invalid objs %x\n", objs);
    return;

	found : {
		//printf("######## DequeueObjs\n");
		//printf("%08x,%08x %d %d %08x\n", obj_buff, obj_buff->queue,
		//obj_buff->n_queue, obj_buff->max_queue, objs);

		// 登録順番をくずさないようにキューから削除する
		for (-- i; i > 0; -- i) {
			oque[0] = oque[1];
			oque ++;
		}
		obj_buff->n_queue = -- n;
	}
}

static void* SearchQueueObjs(DG_OBJ_BUFFER *obj_buff, void *objs)
{
    void        **oque;
    int            n, i;
    /*
	  キュー登録位置を検索
    */
    oque = obj_buff->queue;
    n = obj_buff->n_queue;
    for (i = n; i > 0; -- i) {
        if (*oque == objs) break;
        oque ++;
    }
    if (i == 0) return (NULL);
    return (oque);
}

/*
        通常オブジェクト
    */
int DG_QueueObjs(DG_OBJS *objs)
{
    int        ret;
    //printf("objs:");
    ret = QueueObjs(&DG_Chanls[objs->chanl].obj_queue->objs_buffer, objs);
    if (DG_Chanls[objs->chanl].obj_queue->objs_buffer.n_queue > 512) {
		GV_ERROR(GV_ERROR_MAX_OBJ_QUEUE);
	}
    if (ret != 0) {
		GV_ERROR(GV_ERROR_MAX_OBJ_QUEUE);
	}
    return (ret);
}

void DG_DequeueObjs(DG_OBJS *objs)
{
    DequeueObjs(&DG_Chanls[objs->chanl].obj_queue->objs_buffer, objs);
}

DG_OBJS* DG_SearchQueueObjs(DG_OBJS *objs)
{
    return (DG_OBJS *)SearchQueueObjs(&DG_Chanls[0].obj_queue->objs_buffer, objs);
}

/*
  影投影オブジェクト
*/
int DG_QueueShdwwriteObjs(DG_OBJS *objs)
{
    int        ret;
    //printf("objs2:");
    ret = QueueObjs(&DG_Chanls[objs->chanl].obj_queue->shdwwrite_buffer, objs);
    if (ret != 0) {
		GV_ERROR(GV_ERROR_MAX_MISC_QUEUE);
	}
    return (ret);
}

void DG_DequeueShdwwriteObjs(DG_OBJS *objs)
{
    DequeueObjs(&DG_Chanls[objs->chanl].obj_queue->shdwwrite_buffer, objs);
}

/*
  スポットライトオブジェクト
*/
int DG_QueueSpotObjs(DG_SPOT *objs)
{
    int        ret;
    //printf("spot:");
    /* 将来的に修正する */
    ret = QueueObjs(&DG_Chanls[0].obj_queue->spot_buffer, objs);
    if (ret != 0) GV_ERROR(GV_ERROR_MAX_MISC_QUEUE);
    return (ret);
}

void DG_DequeueSpotObjs(DG_SPOT *objs)
{
    /* 将来的に修正する */
    DequeueObjs(&DG_Chanls[0].obj_queue->spot_buffer, objs);
}

/*
  マルチウェイトエンベロープモデルオブジェクト
*/
int DG_QueueEvmObj(DG_EVMOBJ *evmobj)
{
    int        ret;
    //printf("evmobj:");
    ret = QueueObjs(&DG_Chanls[evmobj->chanl].obj_queue->evmobj_buffer, evmobj);
    if (ret != 0) GV_ERROR(GV_ERROR_MAX_MISC_QUEUE);
    return (ret);
}

void DG_DequeueEvmObj(DG_EVMOBJ *evmobj)
{
	DequeueObjs(&DG_Chanls[evmobj->chanl].obj_queue->evmobj_buffer, evmobj);
}

int DG_QueuePrim2(DG_PRIM2 *objs)
{
    int        ret;
    //printf("prim2:");
    ret = QueueObjs(&DG_Chanls[objs->chanl].obj_queue->prim2_buffer, objs);
    if (ret != 0) GV_ERROR(GV_ERROR_MAX_PRIM_QUEUE);
    return (ret);
}

void DG_DequeuePrim2(DG_PRIM2 *objs)
{
    DequeueObjs(&DG_Chanls[objs->chanl].obj_queue->prim2_buffer, objs);
}

int DG_QueueParticle(DG_PARTICLE *objs)
{
    int        ret;
    ret = QueueObjs(&DG_Chanls[objs->chanl].obj_queue->particle_buffer, objs);
    if (ret != 0) GV_ERROR(GV_ERROR_MAX_PRIM_QUEUE);
    return (ret);
}

void DG_DequeueParticle(DG_PARTICLE *objs)
{
    DequeueObjs(&DG_Chanls[objs->chanl].obj_queue->particle_buffer, objs);
}


//-----------------------------------------------------------------------------

/*
  プラグイン用オブジェクト
*/
int DG_QueueUserObject(DG_OBJ_BUFFER *obj_buff, void *objs)
{
    //printf("user object:");
    return QueueObjs(obj_buff, objs);
}

void DG_DequeueUserObject(DG_OBJ_BUFFER *obj_buff, void *objs)
{
    DequeueObjs(obj_buff, objs);
}

//-----------------------------------------------------------------------------

/* 特権オブジェクトモードの変更 */
int DG_SetPrivilegeMode(int mode)
{
    int    ret;
    ret = DG_PrivilegeMode;
    DG_PrivilegeMode = mode;
    return (ret);    /* とりあえず変更前の値を返しておく */
}

//-----------------------------------------------------------------------------

void DG_StopMainChanlSystem(void)
{
    /*
	 * メイン画面用のチャンネル処理を停止する。
	 * 各オブジェクト用のパケットをすべて解放。パケットエリアを使用可能にする。
	 * 描画が完全に終了したことを確認してからパケットエリアを使用すること。
	 */
    DG_CHANL     *cp;
    DG_OBJ_QUEUE *que;
    DG_OBJS      **oque, *objs;
    int i,j;

    stop_chanl_system_flag = 1;

    cp = DG_Chanls;        /* メインチャンネル */

    for (j = DG_MAX_CHANLS; j > 0; j--) {
        que = cp->obj_queue;
        if (que != NULL){
            oque = (DG_OBJS **)que->objs_buffer.queue;
            for (i = que->objs_buffer.n_queue; i > 0; i--) {
                objs = *(oque ++);
            }
        }
        cp++;
    }
}

void DG_RestartMainChanlSystem(void)
{
    /*
	  メイン画面用のチャンネル処理を再開する。
    */
	// 未実装
    //extern long DG_UnDrawFrameCount;
	
    stop_chanl_system_flag = 0;
	//DG_UnDrawFrameCount = 1;
}

//-----------------------------------------------------------------------------
