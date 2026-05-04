/*
	callback.cpp
	libdgコールバックプラグイン(Xbox新設)

	2001/07/31 F.Miyauchi
	$Id: callback.cpp,v 1.1.1.3 2002/11/19 11:42:02 Yoshizawa1 Exp $
*/

//
// 描画のタイミングで、ユーザが指定したコールバックを呼ぶだけのプラグイン。
// NORMAL PHASE (不透明モデル描画後フェーズ)
// LAST PHASE (最終描画フェーズ)
// の2箇所をサポートします。
//
// OBJ/PRIM2等々とは関係なく、自由に描画したい時に使用してください。
//

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef KP_XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include "libgv.h"
#include "libdg.h"

//-----------------------------------------------------------------------------

// プラグイン固有設定
#define PLUGIN_ID   (GV_StrCode("callback"))
#define PLUGIN_FLAG (DG_PLUGIN_FLAG_OBJBUFFER|DG_PLUGIN_FLAG_ENABLE|DG_PLUGIN_FLAG_ENABLEMENU)

#define PLUGIN_PHASE (DG_PLUGIN_PHASE_FIRST|DG_PLUGIN_PHASE_NORMAL|DG_PLUGIN_PHASE_AFTER|DG_PLUGIN_PHASE_LAST)
#define PLUGIN_PRIO         (DG_PLUGIN_PRIO_NORMAL)
#define PLUGIN_USE_QUEUE (64)

// プラグイン固有変数
static int PluginStartFlag = 0;
static DG_PLUGIN CallbackPlugin;
static void PluginActor(DG_CHANL *, int, DG_OBJ_BUFFER *, int);

//-----------------------------------------------------------------------------

// Normal Phase と Last Phase の2つのコールバックタイミングを作成する
void DG_AddPluginCallbackObj(void)
{
	if (PluginStartFlag != 0) return;

	DG_MakePlugin(&CallbackPlugin, PLUGIN_ID, PLUGIN_FLAG, PLUGIN_PHASE,
				  PLUGIN_PRIO, PluginActor, PLUGIN_USE_QUEUE);
	DG_AddPlugin(&CallbackPlugin);
	PluginStartFlag = 1;
}

void DG_DeletePluginCallbackObj(void)
{
	DG_DeletePlugin(&CallbackPlugin);
	DG_FreePlugin(&CallbackPlugin);
	PluginStartFlag = 0;
}

//-----------------------------------------------------------------------------

int DG_QueueCallbackObj(DG_CALLBACK_OBJ *obj)
{
	if (PluginStartFlag == 0) return 0;
	if (obj == NULL) return 0;

	/* プライオリティを考慮しつつキューに追加 */
	int n;
	DG_CALLBACK_OBJ **objs;
	DG_OBJ_BUFFER *obj_buff;
	obj_buff = CallbackPlugin.obj_buffer;
	
	if (obj_buff->n_queue >= obj_buff->max_queue) {
		printf("que buffer over !!!\n");
		return -1;
	}
	objs = (DG_CALLBACK_OBJ **) &obj_buff->queue[obj_buff->n_queue - 1];
	for (n = obj_buff->n_queue - 1; n >= 0; objs--, n--) {
		if ((*objs)->pri > obj->pri) {
			objs[1] = objs[0];
		} else {
			break;
		}
	}
	objs++;
	*objs = obj;
	obj_buff->n_queue++;
	return 1;
	//return DG_QueueUserObject(CallbackPlugin.obj_buffer, obj);
}

void DG_DequeueCallbackObj(DG_CALLBACK_OBJ *obj)
{
	if (PluginStartFlag == 0) return;
	DG_DequeueUserObject(CallbackPlugin.obj_buffer, obj);	
}

void DG_FreeCallbackObj(DG_CALLBACK_OBJ *obj)
{
	GV_Free(obj);
}

//-----------------------------------------------------------------------------

DG_CALLBACK_OBJ *DG_MakeCallbackObj(DG_CallbackObjFunc func, int flag,
									int phase, int pri, void *data)
{
	DG_CALLBACK_OBJ *obj;

	obj = (DG_CALLBACK_OBJ *) GV_Malloc(sizeof(DG_CALLBACK_OBJ));
	obj->flag = flag;
	obj->phase = phase;
	obj->pri = pri;
	obj->func = func;
	obj->data = data;
	return obj;
}

void DG_SetCallbackObj(DG_CALLBACK_OBJ *obj, DG_CallbackObjFunc func, int flag,
					   int phase, int pri, void *data)
{
	obj->flag = flag;
	obj->phase = phase;
	obj->pri = pri;
	obj->func = func;
	obj->data = data;
}

//-----------------------------------------------------------------------------

// プラグイン実行アクター

static void PluginActor(DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status)
{
	int i;
	int invisible_flag, chanl_flag;
	DG_CALLBACK_OBJ **que;
	
	invisible_flag = DG_CALLBACK_INVISIBLE0 << cp->chanl_num;
	/* メニュー用チャンネルで描くかどうか */
	chanl_flag = (cp->chanl_num != 4) ? DG_CALLBACK_NORMAL : DG_CALLBACK_MENU;
	
	if (obj_buff->n_queue == 0) return;

	DG_BeginScene();
	que = (DG_CALLBACK_OBJ **)obj_buff->queue;
	for (i = obj_buff->n_queue; i > 0; i--, que++) {
		DG_CALLBACK_OBJ *obj;
		obj = *que;
		if (!(obj->flag & chanl_flag)) continue;
		if (obj->flag & invisible_flag) continue;
		if (!(obj->phase & status)) continue;
		if (obj->func) { 
			obj->func(cp, which, obj, obj->data);
		}
	}
	DG_EndScene();
}

//-----------------------------------------------------------------------------
