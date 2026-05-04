//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * layout manager - lom_load.c
 * レイアウトマネージャ
 * 2002/04/16 S.Yamashita
 * $Id: lom_load.c,v 1.1.1.3 2002/11/19 11:51:40 Yoshizawa1 Exp $
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
 * public
 */
/*******************************************************************************
 * ＬＯＭアクションリストのロード
 */
int LOM_LoadActlist(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       *data,		/* ＬＯＭデータ */
	int       *pos)			/* データ位置 */
{
	int pos2;
	int datacount;
	int index;
	int index2;

	SY_PRINTF3("LOM_LoadActlist\n");

	ASSERT(layoutman != NULL);
	ASSERT(data      != NULL);
	ASSERT(pos       != NULL);

	/* ＬＯＭアクション数の取得 */
	pos2 = (*pos) + 1;
	datacount = 0;
	while(data[pos2] != LOM_DATA_END)
	{
		datacount++;
		pos2 += LOM_ACT_DATA_COUNT;
	}
	if(datacount == 0)
	{
		SY_PRINTF2("Invalid LOM Data: No LOM Act Data.\n");
		return 0;
	}

	/* 新規ＬＯＭアクションリスト */
	if((index = LOM_NewActlist(layoutman, data[*pos], datacount)) == -1)
	{
		SY_PRINTF2("Create LOM Actlist Failed.\n");
		return 0;
	}
	(*pos)++;
	SY_PRINTF1("New LOM Actlist\n");

	/* ＬＯＭアクションの設定 */
	for(index2 = 0; index2 < datacount; index2++)
	{
		LOM_ACT *lom_act = LOM_GetActI2(layoutman, index, index2);

		lom_act->act_name = data[*pos]; (*pos)++;
		lom_act->count    = data[*pos]; (*pos)++;
		lom_act->flag     = data[*pos]; (*pos)++;
	}
	(*pos)++;

	return 1;
}

/*******************************************************************************
 * ＬＯＭアイコンリストのロード
 */
int LOM_LoadIcolist(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       *data,		/* ＬＯＭデータ */
	int       *pos)			/* データ位置 */
{
	int pos2;
	int datacount;
	int index;
	int index2;

	SY_PRINTF3("LOM_LoadIcolist\n");

	ASSERT(layoutman != NULL);
	ASSERT(data      != NULL);
	ASSERT(pos       != NULL);

	/* ＬＯＭアイコン数の取得 */
	pos2 = (*pos) + 1;
	datacount = 0;
	while(data[pos2] != LOM_DATA_END)
	{
		datacount++;
		pos2 += LOM_ICO_DATA_COUNT;
	}
	if(datacount == 0)
	{
		SY_PRINTF2("Invalid LOM Data: No LOM Ico Data.\n");
		return 0;
	}

	/* 新規ＬＯＭアイコンリスト */
	if((index = LOM_NewIcolist(layoutman, data[*pos], datacount)) == -1)
	{
		SY_PRINTF2("Create LOM Icolist Failed.\n");
		return 0;
	}
	(*pos)++;
	SY_PRINTF1("New LOM Icolist\n");

	/* ＬＯＭアイコンの設定 */
	for(index2 = 0; index2 < datacount; index2++)
	{
		LOM_ICO *lom_ico = LOM_GetIcoI2(layoutman, index, index2);

		lom_ico->ico_name        = data[*pos]; (*pos)++;
		lom_ico->tex_name        = data[*pos]; (*pos)++;
		lom_ico->pos[LOM_LEFT]   = data[*pos]; (*pos)++;
		lom_ico->pos[LOM_TOP]    = data[*pos]; (*pos)++;
		lom_ico->pos[LOM_RIGHT]  = data[*pos]; (*pos)++;
		lom_ico->pos[LOM_BOTTOM] = data[*pos]; (*pos)++;
		lom_ico->col.r           = data[*pos]; (*pos)++;
		lom_ico->col.g           = data[*pos]; (*pos)++;
		lom_ico->col.b           = data[*pos]; (*pos)++;
		lom_ico->col.a           = data[*pos]; (*pos)++;
		lom_ico->count           = data[*pos]; (*pos)++;
		lom_ico->flag            = data[*pos]; (*pos)++;

		/* スプライト */
		{
			SPR_POS spr_pos = {0.0f, 0.0f};

         // TODO - check if we need to change this to a SPRITE_F
			if((lom_ico->sprite = SPR_Create_2D_Object(SP_SPRITE, layoutman->chanl, NULL)) == NULL)
			{
				SY_PRINTF2("Create Sprite Failed.\n");
				return 0;
			}
			SPR_SetPriority(lom_ico->sprite, SPR_PRI_MOST_NEAR);
			if(lom_ico->tex_name != LOM_ICO_NOTEX)
			{
				if(layoutman->tri == -1)
				{
					SY_PRINTF2("TRI Not Set.\n");
					return 0;
				}
				if(SPR_ObjSetTexture(lom_ico->sprite, lom_ico->tex_name, layoutman->tri) == -1)
				{
					SY_PRINTF2("Set Texture Failed.\n");
					return 0;
				}
			}
			SPR_SetPosSprite  (lom_ico->sprite,  &spr_pos);
			SPR_SetSizeSprite (lom_ico->sprite, 0.0f, 0.0f);
			SPR_SetColorSprite(lom_ico->sprite, lom_ico->col.r, lom_ico->col.g, lom_ico->col.b, lom_ico->col.a);
			lom_ico->sprite->sprite.head.head.flags |= SPR_FLAG_HIDDEN;
			if(lom_ico->flag & LOM_ICO_ALPHA)
			{
				lom_ico->sprite->sprite.head.head.flags |= SPR_FLAG_ALPHA;
				lom_ico->sprite->sprite.head.head.alpha  = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0x00);
			}
		}
	}
	(*pos)++;

	return 1;
}

/*******************************************************************************
 * ＬＯＭオブジェクトリストのロード
 */
int LOM_LoadObjlist(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       *data,		/* ＬＯＭデータ */
	int       *pos)			/* データ位置 */
{
	int pos2;
	int datacount;
	int index;
	int index2;

	SY_PRINTF3("LOM_LoadObjlist\n");

	ASSERT(layoutman != NULL);
	ASSERT(data      != NULL);
	ASSERT(pos       != NULL);

	/* ＬＯＭオブジェクト数の取得 */
	pos2 = (*pos) + 1;
	datacount = 0;
	while(data[pos2] != LOM_DATA_END)
	{
		datacount++;
		pos2 += LOM_OBJ_DATA_COUNT;
	}
	if(datacount == 0)
	{
		SY_PRINTF2("Invalid LOM Data: No LOM Obj Data.\n");
		return 0;
	}

	/* 新規ＬＯＭオブジェクトリスト */
	if((index = LOM_NewObjlist(layoutman, data[*pos], datacount)) == -1)
	{
		SY_PRINTF2("Create LOM Objlist Failed.\n");
		return 0;
	}
	(*pos)++;
	SY_PRINTF1("New LOM Objlist\n");

	/* ＬＯＭオブジェクトの設定 */
	for(index2 = 0; index2 < datacount; index2++)
	{
		LOM_OBJ *lom_obj = LOM_GetObjI2(layoutman, index, index2);

		lom_obj->obj_name     = data[*pos]; (*pos)++;
		lom_obj->icolist_name = data[*pos]; (*pos)++;
		lom_obj->count        = data[*pos]; (*pos)++;
		lom_obj->flag         = data[*pos]; (*pos)++;
	}
	(*pos)++;

	return 1;
}

/*******************************************************************************
 * ＬＯＭパッドリストのロード
 */
int LOM_LoadPadlist(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       *data,		/* ＬＯＭデータ */
	int       *pos)			/* データ位置 */
{
	int pos2;
	int datacount;
	int index;
	int index2;

	SY_PRINTF3("LOM_LoadPadlist\n");

	ASSERT(layoutman != NULL);
	ASSERT(data      != NULL);
	ASSERT(pos       != NULL);

	/* ＬＯＭパッド数の取得 */
	pos2 = (*pos) + 1;
	datacount = 0;
	while(data[pos2] != LOM_DATA_END)
	{
		datacount++;
		pos2 += LOM_PAD_DATA_COUNT;
	}
	if(datacount == 0)
	{
		SY_PRINTF2("Invalid LOM Data: No LOM Pad Data.\n");
		return 0;
	}

	/* 新規ＬＯＭパッドリスト */
	if((index = LOM_NewPadlist(layoutman, data[*pos], datacount)) == -1)
	{
		SY_PRINTF2("Create LOM Padlist Failed.\n");
		return 0;
	}
	(*pos)++;
	SY_PRINTF1("New LOM Padlist\n");

	/* ＬＯＭパッドの設定 */
	for(index2 = 0; index2 < datacount; index2++)
	{
		LOM_PAD *lom_pad = LOM_GetPadI2(layoutman, index, index2);

		lom_pad->act_name = data[*pos]; (*pos)++;
		lom_pad->obj_name = data[*pos]; (*pos)++;
      
      //BP - Special case processing for OK/Cancel buttons
      {
         unsigned int input = data[*pos];

         //BP Sanity check to make sure our special case bits we're using to identify if OK/CANCEL button should be part of input value are not used anywhere else.
         {
            unsigned int filteredInput = input & ~(LOM_PAD_ANYINPUT | LOM_PAD_OK | LOM_PAD_CANCEL);
            
            if( filteredInput > 0xffff )
            {
               printf("BP: Invalid input assignment value used for layout data (%8.8x)\n", input);
               BP_BREAK;
            }
         }

         //BP Detect if OK button is part of input bitfield.
         if( input & LOM_PAD_OK )
         {
            //BP Replace with proper PAD value.
            input &= ~LOM_PAD_OK;
            input |= BP_Pad_GetOkAssignment();
         }

         //BP Detect if CANCEL button is part of input bitfield.
         if( input & LOM_PAD_CANCEL )
         {
            //BP Replace with proper PAD value.
            input &= ~LOM_PAD_CANCEL;
            input |= BP_Pad_GetCancelAssignment();
         }

		   lom_pad->input    = input; (*pos)++;
      }

		lom_pad->execute  = data[*pos]; (*pos)++;
		lom_pad->param    = data[*pos]; (*pos)++;
		lom_pad->flag     = data[*pos]; (*pos)++;
	}
	(*pos)++;

	return 1;
}

/*******************************************************************************
 * ＬＯＭモードのロード
 */
int LOM_LoadMode(	/* 1: 成功 */
					/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       *data,		/* ＬＯＭデータ */
	int       *pos)			/* データ位置 */
{
	int pos2;
	int datacount;
	int index;

	SY_PRINTF3("LOM_LoadMode\n");

	ASSERT(layoutman != NULL);
	ASSERT(data      != NULL);
	ASSERT(pos       != NULL);

	/* ＬＯＭモード数の取得 */
	pos2 = *pos;
	datacount = 0;
	while(data[pos2] != LOM_DATA_END)
	{
		datacount++;
		pos2 += LOM_MODE_DATA_COUNT;
	}
	if(datacount == 0)
	{
		SY_PRINTF2("Invalid LOM Data: No LOM Mode Data.\n");
		return 0;
	}

	/* 新規ＬＯＭモード */
	for(index = 0; index < datacount; index++)
	{
		if(LOM_NewMode2(layoutman, data[(*pos)],
			data[(*pos) + 1], data[(*pos) + 2], data[(*pos) + 3]) == -1)
		{
			SY_PRINTF2("Create LOM Mode Failed.\n");
			return 0;
		}
		(*pos) += LOM_MODE_DATA_COUNT;
		SY_PRINTF1("New LOM Mode\n");
	}
	(*pos)++;

	return 1;
}
