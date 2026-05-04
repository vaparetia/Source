//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * layout manager - layoutman.c
 * レイアウトマネージャ
 * 2002/04/10 S.Yamashita
 * $Id: layoutman.c,v 1.1.1.3 2002/11/19 11:51:39 Yoshizawa1 Exp $
 */

/*******************************************************************************
 * include
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libdg.cnf"
#include "libmt.h"
#include "gameheader.h"
#include "def_dma.h"
#include "libutl.h"

#include "lom.h"

/*******************************************************************************
 * local
 */
static void SetKeyRepeat(LAYOUTMAN *layoutman, int key_type, int status);
static int  CheckInput  (LAYOUTMAN *layoutman, LOM_PAD *lom_pad);
static int  Execute     (LAYOUTMAN *layoutman, LOM_PAD *lom_pad);

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * レイアウトマネージャの生成
 */
int CreateLayoutman(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* .o2d データのファイル名 */
	int       chanl,		/* 描画チャンネル */
	int       base_pri,		/* 基底プライオリティ */
	int       add_flag)		/* 追加フラグ */
{
	SY_PRINTF3("CreateLayoutman\n");

	return CreateLayoutman3(layoutman, strcode, chanl, base_pri, add_flag, 0, 0, NULL, NULL, NULL, 0, 0, 0);
}

int CreateLayoutman2(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN       *layoutman,		/* レイアウトマネージャ */
	int             strcode,		/* .o2d データのファイル名 */
	int             chanl,			/* 描画チャンネル */
	int             base_pri,		/* 基底プライオリティ */
	int             add_flag,		/* 追加フラグ */

	int             tri_name,		/* アイコンＴＲＩ名 */
	int             proc_id,		/* プロックＩＤ */
	LOM_CALLBACK    callback,		/* コールバック関数 */
	LOM_SIG_HANDLER sig_handler,	/* シグナルハンドラ */
	void            *pWork)			/* 呼び出し元キャラのワーク */
{
	SY_PRINTF3("CreateLayoutman2\n");

	return CreateLayoutman3(layoutman, strcode, chanl, base_pri, add_flag, tri_name, proc_id, callback, sig_handler, pWork, 0, 0, 0);
}

int CreateLayoutman3(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN       *layoutman,		/* レイアウトマネージャ */
	int             strcode,		/* .o2d データのファイル名 */
	int             chanl,			/* 描画チャンネル */
	int             base_pri,		/* 基底プライオリティ */
	int             add_flag,		/* 追加フラグ */

	int             tri_name,		/* アイコンＴＲＩ名 */
	int             proc_id,		/* プロックＩＤ */
	LOM_CALLBACK    callback,		/* コールバック関数 */
	LOM_SIG_HANDLER sig_handler,	/* シグナルハンドラ */
	void            *pWork,			/* 呼び出し元キャラのワーク */

	short           key_rep_1,		/* キーリピート１ */
	short           key_rep_2,		/* キーリピート２ */
	int             flag)			/* フラグ */
{
	SY_PRINTF3("CreateLayoutman3\n");

	return CreateLayoutman4(layoutman, strcode, chanl, base_pri, add_flag, GV_PAUSE_PAUSE, tri_name, proc_id, callback, sig_handler, pWork, key_rep_1, key_rep_2, flag);
}

int CreateLayoutman4(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN       *layoutman,		/* レイアウトマネージャ */
	int             strcode,		/* .o2d データのファイル名 */
	int             chanl,			/* 描画チャンネル */
	int             base_pri,		/* 基底プライオリティ */
	int             add_flag,		/* 追加フラグ */
	int             pause_level,	/* 処理をしないポーズレベル */

	int             tri_name,		/* アイコンＴＲＩ名 */
	int             proc_id,		/* プロックＩＤ */
	LOM_CALLBACK    callback,		/* コールバック関数 */
	LOM_SIG_HANDLER sig_handler,	/* シグナルハンドラ */
	void            *pWork,			/* 呼び出し元キャラのワーク */

	short           key_rep_1,		/* キーリピート１ */
	short           key_rep_2,		/* キーリピート２ */
	int             flag)			/* フラグ */
{
	SY_PRINTF3("CreateLayoutman4\n");

	ASSERT(layoutman != NULL);

	if(    (strcode < 1)
		|| ((chanl < 0) || (5 < chanl))
		|| ((base_pri < 0) || (7 < base_pri))
		|| (tri_name < 0)
		|| (proc_id < 0)
		|| ((key_rep_1 < 0) || (LOM_KEY_REP_MAX < key_rep_1            ))
		|| ((key_rep_2 < 0) || (LOM_KEY_REP_MAX < key_rep_1 + key_rep_2)))
	{
		SY_PRINTF2("Invalid Parameter For CreateLayoutman.\n");
		return 0;
	}

	if(layoutman->flag & LOM_ENABLED)
	{
		SY_PRINTF2("Layoutman Already Enabled.\n");
		return 0;
	}

	memset(layoutman, 0x00, sizeof(LAYOUTMAN));

	/* ２Ｄドライバーの初期化 */
	SPR_Init2D_ObjectDriver();

	/* レイアウトのロード */
	if((layoutman->layout = L2D_LoadLayout2(strcode, chanl, base_pri, add_flag, pause_level)) < 0)
	{
		SY_PRINTF2("Load Layout[%d] Failed. (Error Code: %d)\n", strcode, layoutman->layout);
		return 0;
	}
	SY_PRINTF1("Load Layout[%d] Succeeded. (Handle: %d)\n", strcode, layoutman->layout);

	/* ＴＲＩのロード */
	if(tri_name == 0)
	{
		layoutman->tri = -1;
		SY_PRINTF1("No TRI Set.\n");
	}
	else
	{
		if((layoutman->tri = SPR_LoadTexture(tri_name)) == -1)
		{
			SY_PRINTF2("Load TRI[%d] Failed.\n", tri_name);
			return 0;
		}
		SY_PRINTF1("Load TRI Succeeded.\n");
	}

	/* レイアウトマンの設定 */
	layoutman->chanl      = chanl;
	layoutman->proc_id    = proc_id;
	layoutman->callback   = callback;
	layoutman->chara_work = pWork;
	layoutman->key_rep[0] = key_rep_1;
	layoutman->key_rep[1] = key_rep_1 + key_rep_2;
	layoutman->flag       = flag;

	/* シグナルハンドラ */
	if(sig_handler != NULL)
		L2D_SetSignalHandle(layoutman->layout, pWork, sig_handler);

	/* 初期状態にする */
	layoutman->flag |= LOM_ENABLED;
	SY_PRINTF1("== LOM ENABLED ==.\n");

	return 1;
}

/*******************************************************************************
 * レイアウトマネージャの破棄
 */
void DestroyLayoutman(
	LAYOUTMAN *layoutman)	/* レイアウトマネージャ */
{
	int i, j;

	SY_PRINTF3("DestroyLayoutman\n");

	ASSERT(layoutman != NULL);

	/* ＬＯＭモード配列 */
	if(layoutman->lom_mode)
		GV_Free(layoutman->lom_mode);

	/* ＬＯＭパッドリスト配列 */
	if(layoutman->lom_padlist)
	{
		for(i = 0; i < layoutman->n_lom_padlist; i++)
		{
			/* ＬＯＭパッド配列 */
			if(layoutman->lom_padlist[i].lom_pad)
				GV_Free(layoutman->lom_padlist[i].lom_pad);
		}

		GV_Free(layoutman->lom_padlist);
	}

	/* ＬＯＭオブジェクトリスト配列 */
	if(layoutman->lom_objlist)
	{
		for(i = 0; i < layoutman->n_lom_objlist; i++)
		{
			/* ＬＯＭオブジェクト配列 */
			if(layoutman->lom_objlist[i].lom_obj)
				GV_Free(layoutman->lom_objlist[i].lom_obj);
		}

		GV_Free(layoutman->lom_objlist);
	}

	/* ＬＯＭアイコンリスト配列 */
	if(layoutman->lom_icolist)
	{
		for(i = 0; i < layoutman->n_lom_icolist; i++)
		{
			/* ＬＯＭアイコン配列 */
			if(layoutman->lom_icolist[i].lom_ico)
			{
				for(j = 0; j < layoutman->lom_icolist[i].n_lom_ico; j++)
				{
					if(layoutman->lom_icolist[i].lom_ico[j].sprite)
						if(SPR_Destroy_2D_Object(layoutman->lom_icolist[i].lom_ico[j].sprite) == -1)
							SY_PRINTF2("Destroy Sprite Failed.\n");
				}
				GV_Free(layoutman->lom_icolist[i].lom_ico);
			}
		}

		GV_Free(layoutman->lom_icolist);
	}

	/* ＬＯＭアクションリスト配列 */
	if(layoutman->lom_actlist)
	{
		for(i = 0; i < layoutman->n_lom_actlist; i++)
		{
			/* ＬＯＭアクション配列 */
			if(layoutman->lom_actlist[i].lom_act)
				GV_Free(layoutman->lom_actlist[i].lom_act);
		}

		GV_Free(layoutman->lom_actlist);
	}

	/* ＴＲＩの解放 */
	if(layoutman->tri != -1)
		if(SPR_KillTexture(layoutman->tri) == -1)
			SY_PRINTF2("Kill TRI Failed.\n");

	/* レイアウトの解放 */
	if(layoutman->flag & LOM_ENABLED)
	{
		i = L2D_ReleaseLayout(layoutman->layout);
		SY_PRINTF2("Release Layout.(%d)\n", i);
	}

	memset(layoutman, 0x00, sizeof(LAYOUTMAN));
}

/*******************************************************************************
 * ＬＯＭデータのロード
 */
int LoadLOMData(	/* 1: 成功 */
					/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       *data)		/* ＬＯＭデータ */
{
	int pos  = 0;
	int type = 0;	/* 1:LOM_ACTLIST_DATA  2:LOM_ICOLIST_DATA  2:LOM_OBJLIST_DATA  3:LOM_PADLIST_DATA  4:LOM_MODE_DATA */

	SY_PRINTF3("LoadLOMData\n");

	ASSERT(layoutman != NULL);
	ASSERT(data      != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(1)\n");
		return 0;
	}
	if(layoutman->flag & LOM_DATA_LOADED)
	{
		SY_PRINTF2("LOM Data Already Loaded.\n");
		return 0;
	}

	/* データの終端は LOM_DATA_END */
	while(data[pos] != LOM_DATA_END)
	{
		if(type == 0)
		{
			switch(data[pos])
			{
			case LOM_ACTLIST_DATA:
				/* LOM_ACTLIST_DATA へ切り替え */
				type = 1;
				SY_PRINTF1("LOM_ACTLIST_DATA start.\n");
				break;

			case LOM_ICOLIST_DATA:
				/* LOM_ICOLIST_DATA へ切り替え */
				type = 2;
				SY_PRINTF1("LOM_ICOLIST_DATA start.\n");
				break;

			case LOM_OBJLIST_DATA:
				/* LOM_OBJLIST_DATA へ切り替え */
				type = 3;
				SY_PRINTF1("LOM_OBJLIST_DATA start.\n");
				break;

			case LOM_PADLIST_DATA:
				/* LOM_PADLIST_DATA へ切り替え */
				type = 4;
				SY_PRINTF1("LOM_PADLIST_DATA start.\n");
				break;

			case LOM_MODE_DATA:
				/* LOM_MODE_DATA へ切り替え */
				type = 5;
				SY_PRINTF1("LOM_MODE_DATA start.\n");
				break;

			default:
				SY_PRINTF2("Invalid LOM Data.\n");
				ASSERT(0);
			}
			pos++;
		}
		else
		{
			switch(type)
			{
			case 1:
				/* ＬＯＭアクションリストのロード */
				if(LOM_LoadActlist(layoutman, data, &pos) == 0)
				{
					SY_PRINTF2("Load LOM Actlist Failed.\n");
					ASSERT(0);
				}

				if(data[pos] == LOM_DATA_END)
				{
					/* LOM_ACTLIST_DATA の終わり */
					type = 0;
					pos++;
					SY_PRINTF1("LOM_ACTLIST_DATA end.\n");
				}
				break;

			case 2:
				/* ＬＯＭアイコンリストのロード */
				if(LOM_LoadIcolist(layoutman, data, &pos) == 0)
				{
					SY_PRINTF2("Load LOM Icolist Failed.\n");
					ASSERT(0);
				}

				if(data[pos] == LOM_DATA_END)
				{
					/* LOM_ICOLIST_DATA の終わり */
					type = 0;
					pos++;
					SY_PRINTF1("LOM_ICOLIST_DATA end.\n");
				}
				break;

			case 3:
				/* ＬＯＭオブジェクトリストのロード */
				if(LOM_LoadObjlist(layoutman, data, &pos) == 0)
				{
					SY_PRINTF2("Load LOM Objlist Failed.\n");
					ASSERT(0);
				}

				if(data[pos] == LOM_DATA_END)
				{
					/* LOM_OBJLIST_DATA の終わり */
					type = 0;
					pos++;
					SY_PRINTF1("LOM_OBJLIST_DATA end.\n");
				}
				break;

			case 4:
				/* ＬＯＭパッドリストのロード */
				if(LOM_LoadPadlist(layoutman, data, &pos) == 0)
				{
					SY_PRINTF2("Load LOM Padlist Failed.\n");
					ASSERT(0);
				}

				if(data[pos] == LOM_DATA_END)
				{
					/* LOM_PADLIST_DATA の終わり */
					type = 0;
					pos++;
					SY_PRINTF1("LOM_PADLIST_DATA end.\n");
				}
				break;

			case 5:
				/* ＬＯＭモードのロード */
				if(LOM_LoadMode(layoutman, data, &pos) == 0)
				{
					SY_PRINTF2("Load LOM Mode Failed.\n");
					ASSERT(0);
				}

				/* LOM_MODE_DATA の終わり */
				type = 0;
				SY_PRINTF1("LOM_MODE_DATA end.\n");
				break;

			default:
				SY_PRINTF2("Invalid LOM Data.\n");
				ASSERT(0);
			}
		}
	} /* while(data[pos] != LOM_DATA_END) */

	if(layoutman->n_lom_actlist == 0)
	{
		SY_PRINTF2("No LOM Actlist Data.\n");
		ASSERT(0);
	}
	if(layoutman->n_lom_mode == 0)
	{
		SY_PRINTF2("No LOM Mode Data.\n");
		ASSERT(0);
	}
	layoutman->flag |= LOM_DATA_LOADED;
	SY_PRINTF1("Load LOM Data Succeeded.\n");

	/* ロード内容をダンプ */
	{
		int i;

		SY_PRINTF1("LAYOUTMAN\n");
		SY_PRINTF1("\tlayout       : %d\n", layoutman->layout       );
		SY_PRINTF1("\ttri          : %d\n", layoutman->tri          );
		SY_PRINTF1("\tchanl        : %d\n", layoutman->chanl        );
		SY_PRINTF1("\tkey_rep[0]   : %d\n", layoutman->key_rep[0]   );
		SY_PRINTF1("\tkey_rep[1]   : %d\n", layoutman->key_rep[1]   );
		SY_PRINTF1("\tflag         : %d\n", layoutman->flag         );
		SY_PRINTF1("\tn_lom_actlist: %d\n", layoutman->n_lom_actlist);
		SY_PRINTF1("\tn_lom_icolist: %d\n", layoutman->n_lom_icolist);
		SY_PRINTF1("\tn_lom_objlist: %d\n", layoutman->n_lom_objlist);
		SY_PRINTF1("\tn_lom_padlist: %d\n", layoutman->n_lom_padlist);
		SY_PRINTF1("\tn_lom_mode   : %d\n", layoutman->n_lom_mode   );

		for(i = 0; i < layoutman->n_lom_actlist; i++)
		{
			SY_PRINTF1("LOM_ACTLIST %d\n", i);
			SY_PRINTF1("\tactlist_name: %d\n", layoutman->lom_actlist[i].actlist_name);
			SY_PRINTF1("\tn_lom_act   : %d\n", layoutman->lom_actlist[i].n_lom_act   );
		}

		for(i = 0; i < layoutman->n_lom_icolist; i++)
		{
			SY_PRINTF1("LOM_ICOLIST %d\n", i);
			SY_PRINTF1("\ticolist_name: %d\n", layoutman->lom_icolist[i].icolist_name);
			SY_PRINTF1("\tn_lom_ico   : %d\n", layoutman->lom_icolist[i].n_lom_ico   );
		}

		for(i = 0; i < layoutman->n_lom_objlist; i++)
		{
			SY_PRINTF1("LOM_OBJLIST %d\n", i);
			SY_PRINTF1("\tobjlist_name: %d\n", layoutman->lom_objlist[i].objlist_name);
			SY_PRINTF1("\tn_lom_obj   : %d\n", layoutman->lom_objlist[i].n_lom_obj   );
		}

		for(i = 0; i < layoutman->n_lom_padlist; i++)
		{
			SY_PRINTF1("LOM_PADLIST %d\n", i);
			SY_PRINTF1("\tpadlist_name: %d\n", layoutman->lom_padlist[i].padlist_name);
			SY_PRINTF1("\tn_lom_pad   : %d\n", layoutman->lom_padlist[i].n_lom_pad   );
		}

		for(i = 0; i < layoutman->n_lom_mode; i++)
		{
			SY_PRINTF1("LOM_MODE %d\n", i);
			SY_PRINTF1("\tmode_name        : %d\n", layoutman->lom_mode[i].mode_name        );
			SY_PRINTF1("\tlom_actlist_index: %d\n", layoutman->lom_mode[i].lom_actlist_index);
			SY_PRINTF1("\tlom_objlist_index: %d\n", layoutman->lom_mode[i].lom_objlist_index);
			SY_PRINTF1("\tlom_padlist_index: %d\n", layoutman->lom_mode[i].lom_padlist_index);
		}
	}

	/* モードを初期化 */
	LOM_SetCurModeI(layoutman, 0, -1, 0, 0);

	/* デフォルトアクションを起動 */
	if(L2D_EvokeActionByNumber(layoutman->layout, 0) != L2D_STAT_ACK)
	{
		SY_PRINTF2("Evoke Default Action Failed.\n");
		return 0;
	}
	layoutman->flag |= LOM_DEFAULT_ACTION;
	SY_PRINTF1("Evoke Default Action.\n");

	return 1;
}

/*******************************************************************************
 * レイアウトマネージャの毎フレーム処理
 */
int ActLayoutman(	/* 実行結果 */
					/* 1: 成功 */
					/* 0: 失敗 */
	LAYOUTMAN *layoutman)	/* レイアウトマネージャ */
{
	LOM_ACTLIST *lom_actlist;
	LOM_ACT     *lom_act;
	LOM_ICOLIST *lom_icolist;
	LOM_ICO     *lom_ico;
	LOM_OBJLIST *lom_objlist;
	LOM_OBJ     *lom_obj;
	LOM_PADLIST *lom_padlist;
	LOM_PAD     *lom_pad;
	SPR_POS     spr_pos[2];
	float       pos[4];
	int         index;
	int         ret;
	int         flags;

	ASSERT(layoutman != NULL);

	if(    ((layoutman->flag & LOM_ENABLED    ) == 0)
		|| ((layoutman->flag & LOM_DATA_LOADED) == 0)
		|| (layoutman->flag & LOM_DEFAULT_ACTION))
		return 0;	/* 無効状態 */


	/* カレント */
	lom_actlist = LOM_GetActlistI(layoutman, layoutman->c_lom_actlist_index);
	lom_act     = LOM_GetActI(lom_actlist, lom_actlist->c_lom_act_index);
	lom_objlist = LOM_GetObjlistI(layoutman, layoutman->c_lom_objlist_index);
	if(lom_objlist != NULL)
		lom_obj = LOM_GetObjI(lom_objlist, lom_objlist->c_lom_obj_index);
	else
		lom_obj = NULL;


	/* キーリピート */
	if(layoutman->flag & LOM_KEY_REP_1)
	{
		if(layoutman->flag & LOM_INPUT_DIRECT)
		{
			SetKeyRepeat(layoutman, LOM_KEY_U, GV_PadDataDirect[0].status & PAD_U);
			SetKeyRepeat(layoutman, LOM_KEY_D, GV_PadDataDirect[0].status & PAD_D);
			SetKeyRepeat(layoutman, LOM_KEY_L, GV_PadDataDirect[0].status & PAD_L);
			SetKeyRepeat(layoutman, LOM_KEY_R, GV_PadDataDirect[0].status & PAD_R);
		}
		else
		{
			SetKeyRepeat(layoutman, LOM_KEY_U, GV_PadData[0].status & PAD_U);
			SetKeyRepeat(layoutman, LOM_KEY_D, GV_PadData[0].status & PAD_D);
			SetKeyRepeat(layoutman, LOM_KEY_L, GV_PadData[0].status & PAD_L);
			SetKeyRepeat(layoutman, LOM_KEY_R, GV_PadData[0].status & PAD_R);
		}
	}
	if(layoutman->flag & LOM_KEY_REP_2)
	{
		if(layoutman->flag & LOM_INPUT_DIRECT)
		{
			SetKeyRepeat(layoutman, LOM_KEY_A, GV_PadDataDirect[0].status & PAD_A);
			SetKeyRepeat(layoutman, LOM_KEY_B, GV_PadDataDirect[0].status & PAD_B);
			SetKeyRepeat(layoutman, LOM_KEY_X, GV_PadDataDirect[0].status & PAD_X);
			SetKeyRepeat(layoutman, LOM_KEY_Y, GV_PadDataDirect[0].status & PAD_Y);
		}
		else
		{
			SetKeyRepeat(layoutman, LOM_KEY_A, GV_PadData[0].status & PAD_A);
			SetKeyRepeat(layoutman, LOM_KEY_B, GV_PadData[0].status & PAD_B);
			SetKeyRepeat(layoutman, LOM_KEY_X, GV_PadData[0].status & PAD_X);
			SetKeyRepeat(layoutman, LOM_KEY_Y, GV_PadData[0].status & PAD_Y);
		}
	}
	if(layoutman->flag & LOM_KEY_REP_3)
	{
		if(layoutman->flag & LOM_INPUT_DIRECT)
		{
			SetKeyRepeat(layoutman, LOM_KEY_L1, GV_PadDataDirect[0].status & PAD_L1);
			SetKeyRepeat(layoutman, LOM_KEY_R1, GV_PadDataDirect[0].status & PAD_R1);
			SetKeyRepeat(layoutman, LOM_KEY_L2, GV_PadDataDirect[0].status & PAD_L2);
			SetKeyRepeat(layoutman, LOM_KEY_R2, GV_PadDataDirect[0].status & PAD_R2);
		}
		else
		{
			SetKeyRepeat(layoutman, LOM_KEY_L1, GV_PadData[0].status & PAD_L1);
			SetKeyRepeat(layoutman, LOM_KEY_R1, GV_PadData[0].status & PAD_R1);
			SetKeyRepeat(layoutman, LOM_KEY_L2, GV_PadData[0].status & PAD_L2);
			SetKeyRepeat(layoutman, LOM_KEY_R2, GV_PadData[0].status & PAD_R2);
		}
	}
	if(layoutman->flag & LOM_KEY_REP_4)
	{
		if(layoutman->flag & LOM_INPUT_DIRECT)
		{
			SetKeyRepeat(layoutman, LOM_KEY_ST, GV_PadDataDirect[0].status & PAD_STA);
			SetKeyRepeat(layoutman, LOM_KEY_SE, GV_PadDataDirect[0].status & PAD_SEL);
			SetKeyRepeat(layoutman, LOM_KEY_AL, GV_PadDataDirect[0].status & PAD_AL);
			SetKeyRepeat(layoutman, LOM_KEY_AR, GV_PadDataDirect[0].status & PAD_AR);
		}
		else
		{
			SetKeyRepeat(layoutman, LOM_KEY_ST, GV_PadData[0].status & PAD_STA);
			SetKeyRepeat(layoutman, LOM_KEY_SE, GV_PadData[0].status & PAD_SEL);
			SetKeyRepeat(layoutman, LOM_KEY_AL, GV_PadData[0].status & PAD_AL);
			SetKeyRepeat(layoutman, LOM_KEY_AR, GV_PadData[0].status & PAD_AR);
		}
	}


	/* パッドチェック */
	lom_padlist = LOM_GetPadlistI(layoutman, layoutman->c_lom_padlist_index);
	if(lom_padlist != NULL)
	{
		for(index = 0; index < lom_padlist->n_lom_pad; index++)
		{
			lom_pad = &lom_padlist->lom_pad[index];

			/* アクションチェック */
			if(    (lom_pad->act_name == lom_act->act_name)
				|| (lom_pad->act_name == LOM_PAD_ALLACT))
			{
				/* オブジェクトチェック */
				if(    ((lom_obj != NULL) && (lom_pad->obj_name == lom_obj->obj_name))
					|| (lom_pad->obj_name == LOM_PAD_ALLOBJ))
				{
					/* 入力チェック */
					if(    (lom_pad->input == LOM_PAD_ANYINPUT)
						|| (CheckInput(layoutman, lom_pad)))
					{
						if((ret = Execute(layoutman, lom_pad)) != 1)
							return ret;

						/* カレント */
						lom_actlist = LOM_GetActlistI(layoutman, layoutman->c_lom_actlist_index);
						lom_act     = LOM_GetActI(lom_actlist, lom_actlist->c_lom_act_index);
						lom_objlist = LOM_GetObjlistI(layoutman, layoutman->c_lom_objlist_index);
						if(lom_objlist != NULL)
							lom_obj = LOM_GetObjI(lom_objlist, lom_objlist->c_lom_obj_index);
						else
							lom_obj = NULL;
						break;
					}
				}
			}
		}
	}


	/* アクション */
	switch(lom_act->flag & LOM_ACT_TYPEMASK)
	{
	case LOM_ACT_COUNT:		/* 指定回数を再生 */
		/* 再生中 */
		if(L2D_ActionStatus(layoutman->layout) == L2D_STAT_BUSY)
			break;

		if(lom_act->count == LOM_ACT_LOOP)
		{
			/* ループ */
			if(L2D_EvokeAction(layoutman->layout, lom_act->act_name) != L2D_STAT_ACK)
				SY_PRINTF1("Evoke Action[%d] Failed.\n", lom_act->act_name);
			else
			{
				SY_PRINTF1("Evoke Action: %d\n", lom_act->act_name);
				layoutman->count = 0;
			}
			break;
		}

		if(layoutman->act_count < lom_act->count - 1)
		{
			/* 現在のアクションをもう一度 */
			layoutman->act_count++;
			if(L2D_EvokeAction(layoutman->layout, lom_act->act_name) != L2D_STAT_ACK)
				SY_PRINTF1("Evoke Action[%d] Failed.\n", lom_act->act_name);
			else
			{
				SY_PRINTF1("Evoke Action: %d\n", lom_act->act_name);
				layoutman->count = 0;
			}
			break;
		}

		/* 次のＬＯＭアクション */
		LOM_SetCurActI(layoutman, lom_actlist->c_lom_act_index + 1);
		lom_act = LOM_GetActI(lom_actlist, lom_actlist->c_lom_act_index);
		layoutman->act_count++;
		break;

	case LOM_ACT_FRAME:		/* 指定フレーム数を再生 */
		if(lom_act->count == LOM_ACT_LOOP)
		{
			/* ループ */
			if(L2D_ActionStatus(layoutman->layout) != L2D_STAT_BUSY)
			{
				/* 現在のアクションをもう一度 */
				if(L2D_EvokeAction(layoutman->layout, lom_act->act_name) != L2D_STAT_ACK)
					SY_PRINTF1("Evoke Action[%d] Failed.\n", lom_act->act_name);
				else
				{
					SY_PRINTF1("Evoke Action: %d\n", lom_act->act_name);
					layoutman->count = 0;
				}
			}
			break;
		}

		if(layoutman->act_count < lom_act->count)
		{
			/* 再生中 */
			layoutman->act_count++;
			break;
		}

		/* 次のＬＯＭアクション */
		LOM_SetCurActI(layoutman, lom_actlist->c_lom_act_index + 1);
		lom_act = LOM_GetActI(lom_actlist, lom_actlist->c_lom_act_index);
		layoutman->act_count++;
		break;

	case LOM_ACT_PAUSE:		/* 指定時間一時停止 */
		if(lom_act->count == LOM_ACT_LOOP)
		{
			/* ループ */
			break;
		}

		if(layoutman->act_count < lom_act->count)
		{
			/* 一時停止中 */
			layoutman->act_count++;
			break;
		}

		/* 次のＬＯＭアクション */
		LOM_SetCurActI(layoutman, lom_actlist->c_lom_act_index + 1);
		lom_act = LOM_GetActI(lom_actlist, lom_actlist->c_lom_act_index);
		layoutman->act_count++;
		break;

	case LOM_ACT_CONTI:		/* 一時停止状態の解除 */
		if(lom_act->count == LOM_ACT_LOOP)
		{
			/* ループ */
			break;
		}

		if(layoutman->act_count < lom_act->count)
		{
			/* 一時停止中 */
			layoutman->act_count++;
			break;
		}

		/* 次のＬＯＭアクション */
		LOM_SetCurActI(layoutman, lom_actlist->c_lom_act_index + 1);
		lom_act = LOM_GetActI(lom_actlist, lom_actlist->c_lom_act_index);
		layoutman->act_count++;
		break;

	default:
		SY_PRINTF1("Invalid Act Type[%d].", lom_act->flag & LOM_ACT_TYPEMASK);
		break;
	}


	/* カーソル */
	if(lom_objlist != NULL)
	{
		/* カレント */
		lom_icolist = LOM_GetIcolistI(layoutman, layoutman->c_lom_icolist_index);
		lom_ico     = LOM_GetIcoI(lom_icolist, lom_icolist->c_lom_ico_index);

		/* アイコン */
		if(lom_ico->count != LOM_ICO_LOOP)
		{
			if(layoutman->ico_count < lom_ico->count)
			{
				layoutman->ico_count++;
			}
			else
			{
				/* 次のアイコン */
				if(lom_obj->flag & LOM_OBJ_ANIME_ORD)
				{
					if(lom_icolist->c_lom_ico_index + 1 < lom_icolist->n_lom_ico)
						LOM_SetCurIcoI(layoutman, lom_icolist->c_lom_ico_index + 1);
					else
						LOM_SetCurIcoI(layoutman, 0);
				}
				else if(lom_obj->flag & LOM_OBJ_ANIME_REV)
				{
					if(lom_icolist->c_lom_ico_index > 0)
						LOM_SetCurIcoI(layoutman, lom_icolist->c_lom_ico_index - 1);
					else
						LOM_SetCurIcoI(layoutman, lom_icolist->n_lom_ico - 1);
					LOM_SetCurIcoI(layoutman, lom_icolist->c_lom_ico_index - 1);
				}
				lom_ico = LOM_GetIcoI(lom_icolist, lom_icolist->c_lom_ico_index);
			}
		}

		/* オブジェクトの矩形 */
		if(lom_obj->flag & LOM_OBJ_NOOBJ)
		{
			spr_pos[0].x = spr_pos[0].y = spr_pos[1].x = spr_pos[1].y = 0.0f;
		}
		else
		{
			int type;

			type = LOM_GetSprID(layoutman, lom_obj->obj_name);
			switch(type)
			{
			case SP_SPRITE:
         case SP_SPRITE_F:
				LOM_Spr_GetPosition(layoutman, lom_obj->obj_name, &spr_pos[0], 1);
				LOM_Spr_GetWidth(layoutman, lom_obj->obj_name, &spr_pos[1].x, 1);
				LOM_Spr_GetHeight(layoutman, lom_obj->obj_name, &spr_pos[1].y, 1);
				spr_pos[1].x += spr_pos[0].x;
				spr_pos[1].y += spr_pos[0].y;
				break;

			case SP_LINE:
				LOM_Lin_GetPosition0(layoutman, lom_obj->obj_name, &spr_pos[0], 1);
				LOM_Lin_GetPosition1(layoutman, lom_obj->obj_name, &spr_pos[1], 1);
				break;

			case SP_EMPTY:
				LOM_Emp_GetPosition(layoutman, lom_obj->obj_name, &spr_pos[0], 1);
				spr_pos[1] = spr_pos[0];
				break;

			default:
				SY_PRINTF2("Invalid Sprite Type[%d].\n", type);
				return 0;	/* エラー */
			}
		}

		/* 位置補正 */
		if(lom_ico->flag & LOM_ICO_L_REL) pos[LOM_LEFT  ] = spr_pos[0].x + lom_ico->pos[LOM_LEFT];
		else                              pos[LOM_LEFT  ] = lom_ico->pos[LOM_LEFT];
		if(lom_ico->flag & LOM_ICO_T_REL) pos[LOM_TOP   ] = spr_pos[0].y + lom_ico->pos[LOM_TOP];
		else                              pos[LOM_TOP   ] = lom_ico->pos[LOM_TOP];

		if     (lom_ico->flag & LOM_ICO_R_REL ) pos[LOM_RIGHT ] = spr_pos[1].x + lom_ico->pos[LOM_RIGHT];
		else if(lom_ico->flag & LOM_ICO_R_SIZE) pos[LOM_RIGHT ] = pos[LOM_LEFT] + lom_ico->pos[LOM_RIGHT];
		else                                    pos[LOM_RIGHT ] = lom_ico->pos[LOM_RIGHT];
		if     (lom_ico->flag & LOM_ICO_B_REL)  pos[LOM_BOTTOM] = spr_pos[1].y + lom_ico->pos[LOM_BOTTOM];
		else if(lom_ico->flag & LOM_ICO_B_SIZE) pos[LOM_BOTTOM] = pos[LOM_TOP] + lom_ico->pos[LOM_BOTTOM];
		else                                    pos[LOM_BOTTOM] = lom_ico->pos[LOM_BOTTOM];

		/* 補間 */
		if(lom_obj->flag & LOM_OBJ_INTERP_1)	/* 指定フレーム数補間移行 */
		{
			if(layoutman->obj_count < lom_obj->count)
			{
				layoutman->obj_count++;

				pos[LOM_LEFT  ] = layoutman->ico_pos[0][LOM_LEFT  ] + ((pos[LOM_LEFT  ] - layoutman->ico_pos[0][LOM_LEFT  ]) * layoutman->obj_count / lom_obj->count);
				pos[LOM_TOP   ] = layoutman->ico_pos[0][LOM_TOP   ] + ((pos[LOM_TOP   ] - layoutman->ico_pos[0][LOM_TOP   ]) * layoutman->obj_count / lom_obj->count);
				pos[LOM_RIGHT ] = layoutman->ico_pos[0][LOM_RIGHT ] + ((pos[LOM_RIGHT ] - layoutman->ico_pos[0][LOM_RIGHT ]) * layoutman->obj_count / lom_obj->count);
				pos[LOM_BOTTOM] = layoutman->ico_pos[0][LOM_BOTTOM] + ((pos[LOM_BOTTOM] - layoutman->ico_pos[0][LOM_BOTTOM]) * layoutman->obj_count / lom_obj->count);
			}
			else
			{
				layoutman->ico_pos[0][LOM_LEFT  ] = pos[LOM_LEFT  ];
				layoutman->ico_pos[0][LOM_TOP   ] = pos[LOM_TOP   ];
				layoutman->ico_pos[0][LOM_RIGHT ] = pos[LOM_RIGHT ];
				layoutman->ico_pos[0][LOM_BOTTOM] = pos[LOM_BOTTOM];
			}
		}
		else if(lom_obj->flag & LOM_OBJ_INTERP_2)	/* 指定分母補間移行 */
		{
			pos[LOM_LEFT  ] = layoutman->ico_pos[1][LOM_LEFT  ] + ((pos[LOM_LEFT  ] - layoutman->ico_pos[1][LOM_LEFT  ]) / lom_obj->count);
			pos[LOM_TOP   ] = layoutman->ico_pos[1][LOM_TOP   ] + ((pos[LOM_TOP   ] - layoutman->ico_pos[1][LOM_TOP   ]) / lom_obj->count);
			pos[LOM_RIGHT ] = layoutman->ico_pos[1][LOM_RIGHT ] + ((pos[LOM_RIGHT ] - layoutman->ico_pos[1][LOM_RIGHT ]) / lom_obj->count);
			pos[LOM_BOTTOM] = layoutman->ico_pos[1][LOM_BOTTOM] + ((pos[LOM_BOTTOM] - layoutman->ico_pos[1][LOM_BOTTOM]) / lom_obj->count);

			layoutman->ico_pos[0][LOM_LEFT  ] = pos[LOM_LEFT  ];
			layoutman->ico_pos[0][LOM_TOP   ] = pos[LOM_TOP   ];
			layoutman->ico_pos[0][LOM_RIGHT ] = pos[LOM_RIGHT ];
			layoutman->ico_pos[0][LOM_BOTTOM] = pos[LOM_BOTTOM];
		}
//		else if(lom_obj->flag & LOM_OBJ_INTERP_3)	/* 指定移動量補間移行 */
//		{
//			/* 仮 */
//			layoutman->ico_pos[0][LOM_LEFT  ] = pos[LOM_LEFT  ];
//			layoutman->ico_pos[0][LOM_TOP   ] = pos[LOM_TOP   ];
//			layoutman->ico_pos[0][LOM_RIGHT ] = pos[LOM_RIGHT ];
//			layoutman->ico_pos[0][LOM_BOTTOM] = pos[LOM_BOTTOM];
//		}
		else
		{
			layoutman->ico_pos[0][LOM_LEFT  ] = pos[LOM_LEFT  ];
			layoutman->ico_pos[0][LOM_TOP   ] = pos[LOM_TOP   ];
			layoutman->ico_pos[0][LOM_RIGHT ] = pos[LOM_RIGHT ];
			layoutman->ico_pos[0][LOM_BOTTOM] = pos[LOM_BOTTOM];
		}
		layoutman->ico_pos[1][LOM_LEFT  ] = pos[LOM_LEFT  ];
		layoutman->ico_pos[1][LOM_TOP   ] = pos[LOM_TOP   ];
		layoutman->ico_pos[1][LOM_RIGHT ] = pos[LOM_RIGHT ];
		layoutman->ico_pos[1][LOM_BOTTOM] = pos[LOM_BOTTOM];


      //BP JG looking for a cursor that's 14x8. (Missions screen, VR Pause.)
      if ( (pos[LOM_RIGHT]-pos[LOM_LEFT])==14 && (pos[LOM_BOTTOM]-pos[LOM_TOP])==8 )
      {
         // resize it for widescreen.
         pos[LOM_RIGHT] -= 4;
         pos[LOM_LEFT] += 4;
         pos[LOM_RIGHT] += 4;

      }


		/* アイコンの位置を設定 */
		spr_pos[0].x = pos[LOM_LEFT];
		spr_pos[0].y = pos[LOM_TOP];
      
      SPR_SetPosSprite (lom_ico->sprite,  &spr_pos[0]);
		SPR_SetSizeSprite(lom_ico->sprite, pos[LOM_RIGHT] - pos[LOM_LEFT], pos[LOM_BOTTOM] - pos[LOM_TOP]);

//		SY_PRINTF1("ico pos: %f, %f, %f, %f\n", spr_pos[0].x, spr_pos[0].y, pos[LOM_RIGHT] - pos[LOM_LEFT], pos[LOM_BOTTOM] - pos[LOM_TOP]);
	}


	/* 表示の設定 */
	if(layoutman->count == 0)
	{
//		lom_actlist = LOM_GetActlistI(layoutman, layoutman->c_lom_actlist_index);
//		lom_act     = LOM_GetActI(lom_actlist, lom_actlist->c_lom_act_index);

		LOM_Emp_GetFlags(layoutman, 2770484 /* ROOT */, &flags);
		if     (lom_act->flag & LOM_ACT_HIDE) { flags |=  SPR_FLAG_HIDDEN; SY_PRINTF1("ROOT Flag Set (HIDE).\n"); }
		else if(lom_act->flag & LOM_ACT_SHOW) { flags &= ~SPR_FLAG_HIDDEN; SY_PRINTF1("ROOT Flag Set (SHOW).\n"); }
		LOM_Emp_SetFlags(layoutman, 2770484 /* ROOT */, flags);
	}


	layoutman->count++;
	return 1;
}

/*******************************************************************************
 * カレントアイコンのプライオリティ設定
 */
int LOM_SetIcoPri(	/* 1: 成功 */
					/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	SPR_PRI   pri)			/* プライオリティ  0:FAR  7:NEAR */
{
	LOM_ICOLIST *lom_icolist;
	LOM_ICO     *lom_ico;

	SY_PRINTF3("LOM_SetIcoPri\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.\n");
		return 0;
	}

	lom_icolist = LOM_GetIcolistI(layoutman, layoutman->c_lom_icolist_index);
	lom_ico     = LOM_GetIcoI(lom_icolist, lom_icolist->c_lom_ico_index);

	SPR_SetPriority(lom_ico->sprite, pri);

	return 1;
}

/*******************************************************************************
 * カーソル位置の設定
 */
int LOM_SetIcoPos(	/* 1: 成功 */
					/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	int       left,			/* 左 */
	int       top,			/* 上 */
	int       right,		/* 右 */
	int       bottom,		/* 下 */
	int       size)			/* サイズフラグ */
{
//	SPR_SPRITE *spr;
//	SPR_OBJ    *obj;
	SPR_POS    pos, pos2;

	SY_PRINTF3("LOM_SetIcoPos\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.\n");
		return 0;
	}

#if 0
	/* オブジェクトの取得 */
	if((spr = (SPR_SPRITE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
	if(spr->head.head.id != SP_SPRITE)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_SPRITE.\n", strcode);
		return 0;
	}

	pos    = spr->pos;
	pos2.x = spr->dw;
	pos2.y = spr->dh;

	/* 親のスケールとオフセットを見る */
	obj = spr->head.head.parent;
	while(obj != NULL)
	{
		switch(obj->head.id)
		{
		case SP_EMPTY:
			pos2.x *= ((SPR_EMPTY *)obj)->head.scale;
			pos2.y *= ((SPR_EMPTY *)obj)->head.scale;
			pos.x  *= ((SPR_EMPTY *)obj)->head.scale;
			pos.y  *= ((SPR_EMPTY *)obj)->head.scale;
			pos.x  += ((SPR_EMPTY *)obj)->pos.x;
			pos.y  += ((SPR_EMPTY *)obj)->pos.y;
			obj     = ((SPR_EMPTY *)obj)->head.parent;
			break;

		case SP_LINE:
			pos2.x *= ((SPR_LINE *)obj)->head.scale;
			pos2.y *= ((SPR_LINE *)obj)->head.scale;
			pos.x  *= ((SPR_LINE *)obj)->head.scale;
			pos.y  *= ((SPR_LINE *)obj)->head.scale;
			pos.x  += ((SPR_LINE *)obj)->pos[0].x;
			pos.y  += ((SPR_LINE *)obj)->pos[0].y;
			obj     = ((SPR_LINE *)obj)->head.parent;
			break;

		case SP_SPRITE:
      case SP_SPRITE_F:
			pos2.x *= ((SPR_SPRITE *)obj)->head.head.scale;
			pos2.y *= ((SPR_SPRITE *)obj)->head.head.scale;
			pos.x  *= ((SPR_SPRITE *)obj)->head.head.scale;
			pos.y  *= ((SPR_SPRITE *)obj)->head.head.scale;
			pos.x  += ((SPR_SPRITE *)obj)->pos.x;
			pos.y  += ((SPR_SPRITE *)obj)->pos.y;
			obj     = ((SPR_SPRITE *)obj)->head.head.parent;
			break;

		default:
			SY_PRINTF2("Invalid Sprite Type[%d].\n", obj->head.id);
			return 0;
		}
	}
#endif

	LOM_Spr_GetPosition(layoutman, strcode, &pos, 1);
	LOM_Spr_GetWidth(layoutman, strcode, &pos2.x, 1);
	LOM_Spr_GetHeight(layoutman, strcode, &pos2.y, 1);

	/* カーソル位置の設定 */
	pos2.x += pos.x;
	pos2.y += pos.y;
	layoutman->ico_pos[0][LOM_LEFT  ] = layoutman->ico_pos[1][LOM_LEFT  ] = pos.x + left;
	layoutman->ico_pos[0][LOM_TOP   ] = layoutman->ico_pos[1][LOM_TOP   ] = pos.y + top;
	if(size == 0)
	{
		layoutman->ico_pos[0][LOM_RIGHT ] = layoutman->ico_pos[1][LOM_RIGHT ] = pos2.x + right;
		layoutman->ico_pos[0][LOM_BOTTOM] = layoutman->ico_pos[1][LOM_BOTTOM] = pos2.y + bottom;
	}
	else
	{
		layoutman->ico_pos[0][LOM_RIGHT ] = layoutman->ico_pos[1][LOM_RIGHT ] = pos.x + left + right;
		layoutman->ico_pos[0][LOM_BOTTOM] = layoutman->ico_pos[1][LOM_BOTTOM] = pos.y + top  + bottom;
	}

	{
		LOM_ICOLIST *lom_icolist;
		LOM_ICO     *lom_ico;

		if((lom_icolist = LOM_GetIcolistI(layoutman, layoutman->c_lom_icolist_index)) != NULL)
		{
			if((lom_ico = LOM_GetIcoI(lom_icolist, lom_icolist->c_lom_ico_index)) != NULL)
			{
				pos.x = layoutman->ico_pos[0][LOM_LEFT];
				pos.y = layoutman->ico_pos[0][LOM_TOP];
				SPR_SetPosSprite (lom_ico->sprite, &pos);
				SPR_SetSizeSprite(lom_ico->sprite, layoutman->ico_pos[0][LOM_RIGHT] - layoutman->ico_pos[0][LOM_LEFT], layoutman->ico_pos[0][LOM_BOTTOM] - layoutman->ico_pos[0][LOM_TOP]);
			}
		}
	}

	return 1;
}

/*******************************************************************************
 * static
 */
/*******************************************************************************
 * キーリピートの設定
 */
static void SetKeyRepeat(
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       key_type,		/* キータイプ */
	int       status)		/* 0: 離されている  1: 押されている */
{
	if(status)
	{
		if(layoutman->key_status[key_type] & LOM_KEY_ON)
		{
			/*
			 * 連続で押されている
			 */

			/* カウント */
			layoutman->key_status[key_type] &= LOM_KEY_REP_MAX;
			layoutman->key_status[key_type]++;

			/* キーリピート１ */
			if(layoutman->key_status[key_type] == layoutman->key_rep[0])
			{
				layoutman->key_status[key_type] |= LOM_KEY_SIG;
			}
			/* キーリピート２ */
			else if(layoutman->key_status[key_type] == layoutman->key_rep[1])
			{
				layoutman->key_status[key_type]  = layoutman->key_rep[0];
				layoutman->key_status[key_type] |= LOM_KEY_SIG;
			}

			layoutman->key_status[key_type] |= LOM_KEY_ON;
		}
		else
		{
			/*
			 * 押された瞬間
			 */
			layoutman->key_status[key_type]  = LOM_KEY_ON;
			layoutman->key_status[key_type] |= LOM_KEY_SIG;
		}
	}
	else
	{
		if(layoutman->key_status[key_type] & LOM_KEY_ON)
		{
			/*
			 * 離された瞬間
			 */
			layoutman->key_status[key_type]  = 0;
			layoutman->key_status[key_type] |= LOM_KEY_SIG;
		}
		else
		{
			/*
			 * 連続で離されている
			 */

			/* カウント */
			layoutman->key_status[key_type] &= LOM_KEY_REP_MAX;
			layoutman->key_status[key_type]++;

			/* キーリピート１ */
			if(layoutman->key_status[key_type] == layoutman->key_rep[0])
			{
				layoutman->key_status[key_type] |= LOM_KEY_SIG;
			}
			/* キーリピート２ */
			else if(layoutman->key_status[key_type] == layoutman->key_rep[1])
			{
				layoutman->key_status[key_type]  = layoutman->key_rep[0];
				layoutman->key_status[key_type] |= LOM_KEY_SIG;
			}
		}
	}
}

/*******************************************************************************
 * 入力判定
 */
static int CheckInput(	/* 判定結果 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	LOM_PAD   *lom_pad)		/* ＬＯＭパッド */
{
	if(layoutman->flag & LOM_INPUT_DIRECT)
	{
		if((lom_pad->flag & LOM_PAD_PRESS) && (GV_PadDataDirect[0].press & lom_pad->input))
		{
			return 1;
		}
		if((lom_pad->flag & LOM_PAD_RELEASE) && (GV_PadDataDirect[0].release & lom_pad->input))
		{
			return 1;
		}
		if((lom_pad->flag & LOM_PAD_ON) && (GV_PadDataDirect[0].status & lom_pad->input))
		{
			return 1;
		}
		if((lom_pad->flag & LOM_PAD_OFF) && !(GV_PadDataDirect[0].status & lom_pad->input))
		{
			return 1;
		}
	}
	else
	{
		if((lom_pad->flag & LOM_PAD_PRESS) && (GV_PadData[0].press & lom_pad->input))
		{
			return 1;
		}
		if((lom_pad->flag & LOM_PAD_RELEASE) && (GV_PadData[0].release & lom_pad->input))
		{
			return 1;
		}
		if((lom_pad->flag & LOM_PAD_ON) && (GV_PadData[0].status & lom_pad->input))
		{
			return 1;
		}
		if((lom_pad->flag & LOM_PAD_OFF) && !(GV_PadData[0].status & lom_pad->input))
		{
			return 1;
		}
	}
	if(lom_pad->flag & LOM_PAD_ON_SIG)
	{
		if(layoutman->flag & LOM_KEY_REP_1)
		{
			if((lom_pad->input & PAD_U  ) && (layoutman->key_status[LOM_KEY_U] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_U ] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_D  ) && (layoutman->key_status[LOM_KEY_D] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_D ] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_L  ) && (layoutman->key_status[LOM_KEY_L] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_L ] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_R  ) && (layoutman->key_status[LOM_KEY_R] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_R ] & LOM_KEY_ON)) return 1;
		}
		if(layoutman->flag & LOM_KEY_REP_2)
		{
			if((lom_pad->input & PAD_A  ) && (layoutman->key_status[LOM_KEY_A] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_A ] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_B  ) && (layoutman->key_status[LOM_KEY_B] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_B ] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_X  ) && (layoutman->key_status[LOM_KEY_X] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_X ] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_Y  ) && (layoutman->key_status[LOM_KEY_Y] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_Y ] & LOM_KEY_ON)) return 1;
		}
		if(layoutman->flag & LOM_KEY_REP_3)
		{
			if((lom_pad->input & PAD_L1 ) && (layoutman->key_status[LOM_KEY_L1] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_L1] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_R1 ) && (layoutman->key_status[LOM_KEY_R1] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_R1] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_L2 ) && (layoutman->key_status[LOM_KEY_L2] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_L2] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_R2 ) && (layoutman->key_status[LOM_KEY_R2] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_R2] & LOM_KEY_ON)) return 1;
		}
		if(layoutman->flag & LOM_KEY_REP_4)
		{
			if((lom_pad->input & PAD_STA) && (layoutman->key_status[LOM_KEY_ST] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_ST] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_SEL) && (layoutman->key_status[LOM_KEY_SE] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_SE] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_AL ) && (layoutman->key_status[LOM_KEY_AL] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_AL] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_AR ) && (layoutman->key_status[LOM_KEY_AR] & LOM_KEY_SIG) && (layoutman->key_status[LOM_KEY_AR] & LOM_KEY_ON)) return 1;
		}
	}
	if(lom_pad->flag & LOM_PAD_OFF_SIG)
	{
		if(layoutman->flag & LOM_KEY_REP_1)
		{
			if((lom_pad->input & PAD_U  ) && (layoutman->key_status[LOM_KEY_U] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_U ] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_D  ) && (layoutman->key_status[LOM_KEY_U] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_D ] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_L  ) && (layoutman->key_status[LOM_KEY_U] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_L ] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_R  ) && (layoutman->key_status[LOM_KEY_U] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_R ] & LOM_KEY_ON)) return 1;
		}
		if(layoutman->flag & LOM_KEY_REP_2)
		{
			if((lom_pad->input & PAD_A  ) && (layoutman->key_status[LOM_KEY_A] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_A ] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_B  ) && (layoutman->key_status[LOM_KEY_B] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_B ] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_X  ) && (layoutman->key_status[LOM_KEY_X] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_X ] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_Y  ) && (layoutman->key_status[LOM_KEY_Y] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_Y ] & LOM_KEY_ON)) return 1;
		}
		if(layoutman->flag & LOM_KEY_REP_3)
		{
			if((lom_pad->input & PAD_L1 ) && (layoutman->key_status[LOM_KEY_L1] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_L1] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_R1 ) && (layoutman->key_status[LOM_KEY_R1] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_R1] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_L2 ) && (layoutman->key_status[LOM_KEY_L2] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_L2] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_R2 ) && (layoutman->key_status[LOM_KEY_R2] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_R2] & LOM_KEY_ON)) return 1;
		}
		if(layoutman->flag & LOM_KEY_REP_4)
		{
			if((lom_pad->input & PAD_STA) && (layoutman->key_status[LOM_KEY_ST] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_ST] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_SEL) && (layoutman->key_status[LOM_KEY_SE] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_SE] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_AL ) && (layoutman->key_status[LOM_KEY_AL] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_AL] & LOM_KEY_ON)) return 1;
			if((lom_pad->input & PAD_AR ) && (layoutman->key_status[LOM_KEY_AR] & LOM_KEY_SIG) && !(layoutman->key_status[LOM_KEY_AR] & LOM_KEY_ON)) return 1;
		}
	}

	return 0;
}

/*******************************************************************************
 * 実行
 */
static int Execute(		/* 1: 成功:処理を続行 */
						/* 2: 成功:処理を中断 */
						/* 0: 失敗 */
						/* 各実行結果 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	LOM_PAD   *lom_pad)		/* ＬＯＭパッド */
{
	LOM_ACTLIST *lom_actlist;
	LOM_ICOLIST *lom_icolist;
	LOM_OBJLIST *lom_objlist;

	/* 実行 */
	SY_PRINTF1("Execute: %d\n", lom_pad->execute);
	switch(lom_pad->execute)
	{
	case LOM_EXE_CALLBACK:
		/* コールバック実行 */
		if(layoutman->callback == NULL)
			return 1;
		return (*(layoutman->callback))(layoutman->chara_work, lom_pad->param);

	case LOM_EXE_PROC:
		/* プロック実行 */
		if(layoutman->proc_id != 0)
		{
			GCL_ARGS args;
			int      param[2];

			args.argv = param;
			args.argc = 2;
			param[0]  = (int)layoutman->chara_work;
			param[1]  = lom_pad->param;

			GM_ExecProc(layoutman->proc_id, &args);
		}
		return 1;



	case LOM_EXE_MODE_NEXT:
		/* 次のＬＯＭモードへ移行 */
		LOM_SetCurModeI(layoutman, layoutman->c_lom_mode_index + 1, 1, 1, 1);
		return 1;

	case LOM_EXE_MODE_PREV:
		/* 前のＬＯＭモードへ移行 */
		LOM_SetCurModeI(layoutman, layoutman->c_lom_mode_index - 1, 1, 1, 1);
		return 1;

	case LOM_EXE_MODE_NAME:
		/* 指定モード名のＬＯＭモードへ移行 */
		LOM_SetCurMode(layoutman, lom_pad->param, 1, 1, 1);
		return 1;

	case LOM_EXE_MODE_INDEX:
		/* 指定インデックスのＬＯＭモードへ移行 */
		LOM_SetCurModeI(layoutman, lom_pad->param, 1, 1, 1);
		return 1;

	case LOM_EXE_MODE_NEXT2:
		/* 次のＬＯＭモードへ移行 */
		LOM_SetCurModeI(layoutman, layoutman->c_lom_mode_index + 1, 1, 0, 0);
		return 1;

	case LOM_EXE_MODE_PREV2:
		/* 前のＬＯＭモードへ移行 */
		LOM_SetCurModeI(layoutman, layoutman->c_lom_mode_index - 1, 1, 0, 0);
		return 1;

	case LOM_EXE_MODE_NAME2:
		/* 指定モード名のＬＯＭモードへ移行 */
		LOM_SetCurMode(layoutman, lom_pad->param, 1, 0, 0);
		return 1;

	case LOM_EXE_MODE_INDEX2:
		/* 指定インデックスのＬＯＭモードへ移行 */
		LOM_SetCurModeI(layoutman, lom_pad->param, 1, 0, 0);
		return 1;



	case LOM_EXE_ACT_NEXT:
		/* 次のＬＯＭアクションへ移行 */
		if((lom_actlist = LOM_GetActlistI(layoutman, layoutman->c_lom_actlist_index)) == NULL)
		{
			SY_PRINTF2("No Current Actlist.");
			return 0;
		}
		LOM_SetCurActI(layoutman, lom_actlist->c_lom_act_index + 1);
		return 1;

	case LOM_EXE_ACT_PREV:
		/* 前のＬＯＭアクションへ移行 */
		if((lom_actlist = LOM_GetActlistI(layoutman, layoutman->c_lom_actlist_index)) == NULL)
		{
			SY_PRINTF2("No Current Actlist.");
			return 0;
		}
		LOM_SetCurActI(layoutman, lom_actlist->c_lom_act_index - 1);
		return 1;

	case LOM_EXE_ACT_NAME:
		/* 指定アクション名のＬＯＭアクションへ移行 */
		LOM_SetCurAct(layoutman, lom_pad->param);
		return 1;

	case LOM_EXE_ACT_INDEX:
		/* 指定インデックスのＬＯＭアクションへ移行 */
		LOM_SetCurActI(layoutman, lom_pad->param);
		return 1;



	case LOM_EXE_ICO_NEXT:
		/* 次のＬＯＭアイコンへ移行 */
		if((lom_icolist = LOM_GetIcolistI(layoutman, layoutman->c_lom_icolist_index)) == NULL)
		{
			SY_PRINTF2("No Current Icolist.");
			return 0;
		}
		LOM_SetCurIcoI(layoutman, lom_icolist->c_lom_ico_index + 1);
		return 1;

	case LOM_EXE_ICO_PREV:
		/* 前のＬＯＭアイコンへ移行 */
		if((lom_icolist = LOM_GetIcolistI(layoutman, layoutman->c_lom_icolist_index)) == NULL)
		{
			SY_PRINTF2("No Current Icolist.");
			return 0;
		}
		LOM_SetCurIcoI(layoutman, lom_icolist->c_lom_ico_index - 1);
		return 1;

	case LOM_EXE_ICO_NAME:
		/* 指定アイコン名のＬＯＭアイコンへ移行 */
		LOM_SetCurIco(layoutman, lom_pad->param);
		return 1;

	case LOM_EXE_ICO_INDEX:
		/* 指定インデックスのＬＯＭアイコンへ移行 */
		LOM_SetCurIcoI(layoutman, lom_pad->param);
		return 1;



	case LOM_EXE_OBJ_NEXT:
		/* 次のＬＯＭオブジェクトへ移行 */
		if((lom_objlist = LOM_GetObjlistI(layoutman, layoutman->c_lom_objlist_index)) == NULL)
		{
			SY_PRINTF2("No Current Objlist.");
			return 0;
		}
		LOM_SetCurObjI(layoutman, lom_objlist->c_lom_obj_index + 1, 1);
		return 1;

	case LOM_EXE_OBJ_PREV:
		/* 前のＬＯＭオブジェクトへ移行 */
		if((lom_objlist = LOM_GetObjlistI(layoutman, layoutman->c_lom_objlist_index)) == NULL)
		{
			SY_PRINTF2("No Current Objlist.");
			return 0;
		}
		LOM_SetCurObjI(layoutman, lom_objlist->c_lom_obj_index - 1, 1);
		return 1;

	case LOM_EXE_OBJ_NAME:
		/* 指定オブジェクト名のＬＯＭオブジェクトへ移行 */
		LOM_SetCurObj(layoutman, lom_pad->param, 1);
		return 1;

	case LOM_EXE_OBJ_INDEX:
		/* 指定インデックスのＬＯＭオブジェクトへ移行 */
		LOM_SetCurObjI(layoutman, lom_pad->param, 1);
		return 1;



	default:
		SY_PRINTF2("Invalid Execute Number[%d].", lom_pad->execute);
		return 0;	/* 未定義の実行番号 */
	}
}
