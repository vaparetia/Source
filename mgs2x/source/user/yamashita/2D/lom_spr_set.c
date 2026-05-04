//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * layout manager - lom_spr_set.c
 * レイアウトマネージャ
 * 2002/04/16 S.Yamashita
 * $Id: lom_spr_set.c,v 1.1.1.3 2002/11/19 11:51:41 Yoshizawa1 Exp $
 */

/*******************************************************************************
 * include
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if 0 //BP_PS2 def PSX2
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
 * スプライトの位置の設定
 */
int LOM_Spr_SetPosition(	/* 1: 成功 */
							/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_POS   *pos,			/* 位置 */
	int       flag)			/* 1: 親のスケールとオフセットを見る */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_Spr_SetPosition\n");

	ASSERT(layoutman != NULL);
	ASSERT(pos       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(27)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((spr = (SPR_SPRITE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
   if(spr->head.head.id != SP_SPRITE && spr->head.head.id != SP_SPRITE_F)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_SPRITE.\n", strcode);
		return 0;
	}

	spr->pos = *pos;

	/* 親のスケールとオフセットを見る */
	if(flag == 1)
	{
		SPR_OBJ *obj;

		obj = spr->head.head.parent;
		while(obj != NULL)
		{
			switch(obj->head.id)
			{
			case SP_EMPTY:
				if(((SPR_EMPTY *)obj)->head.scale != 0.0f)
				{
					spr->pos.x /= ((SPR_EMPTY *)obj)->head.scale;
					spr->pos.y /= ((SPR_EMPTY *)obj)->head.scale;
				}
				spr->pos.x -= ((SPR_EMPTY *)obj)->pos.x;
				spr->pos.y -= ((SPR_EMPTY *)obj)->pos.y;
				obj         = ((SPR_EMPTY *)obj)->head.parent;
				break;

			case SP_LINE:
				if(((SPR_LINE *)obj)->head.scale != 0.0f)
				{
					spr->pos.x /= ((SPR_LINE *)obj)->head.scale;
					spr->pos.y /= ((SPR_LINE *)obj)->head.scale;
				}
				spr->pos.x -= ((SPR_LINE *)obj)->pos[0].x;
				spr->pos.y -= ((SPR_LINE *)obj)->pos[0].y;
				obj         = ((SPR_LINE *)obj)->head.parent;
				break;

			case SP_SPRITE:
         case SP_SPRITE_F:
				if(((SPR_SPRITE *)obj)->head.head.scale != 0.0f)
				{
					spr->pos.x /= ((SPR_SPRITE *)obj)->head.head.scale;
					spr->pos.y /= ((SPR_SPRITE *)obj)->head.head.scale;
				}
				spr->pos.x -= ((SPR_SPRITE *)obj)->pos.x;
				spr->pos.y -= ((SPR_SPRITE *)obj)->pos.y;
				obj         = ((SPR_SPRITE *)obj)->head.head.parent;
				break;

			default:
				SY_PRINTF2("Invalid Sprite Type[%d].\n", obj->head.id);
				return 0;
			}
		}
	}

	return 1;
}

/*******************************************************************************
 * スプライトの色の設定
 */
int LOM_Spr_SetColor(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_COLOR *col)			/* 色 */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_Spr_SetColor\n");

	ASSERT(layoutman != NULL);
	ASSERT(col       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(28)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((spr = (SPR_SPRITE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
   if(spr->head.head.id != SP_SPRITE && spr->head.head.id != SP_SPRITE_F)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_SPRITE.\n", strcode);
		return 0;
	}

	spr->col = *col;

	return 1;
}

/*******************************************************************************
 * スプライトの幅の設定
 */
int LOM_Spr_SetWidth(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	float     width,		/* 幅 */
	int       flag)			/* 1: 親のスケールを見る */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_Spr_SetWidth\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(29)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((spr = (SPR_SPRITE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
   if(spr->head.head.id != SP_SPRITE && spr->head.head.id != SP_SPRITE_F)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_SPRITE.\n", strcode);
		return 0;
	}

	spr->dw = width;

	/* 親のスケールを見る */
	if(flag == 1)
	{
		SPR_OBJ *obj;

		obj = spr->head.head.parent;
		while(obj != NULL)
		{
			if(obj->head.scale != 0.0f)
				spr->dw /= obj->head.scale;
			obj = obj->head.parent;
		}
	}

	return 1;
}

/*******************************************************************************
 * スプライトの高さの設定
 */
int LOM_Spr_SetHeight(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	float     height,		/* 幅 */
	int       flag)			/* 1: 親のスケールを見る */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_Spr_SetHeight\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(30)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((spr = (SPR_SPRITE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
   if(spr->head.head.id != SP_SPRITE && spr->head.head.id != SP_SPRITE_F)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_SPRITE.\n", strcode);
		return 0;
	}

	spr->dh = height;

	/* 親のスケールを見る */
	if(flag == 1)
	{
		SPR_OBJ *obj;

		obj = spr->head.head.parent;
		while(obj != NULL)
		{
			if(obj->head.scale != 0.0f)
				spr->dh /= obj->head.scale;
			obj = obj->head.parent;
		}
	}

	return 1;
}

/*******************************************************************************
 * スプライトのフラグの設定
 */
int LOM_Spr_SetFlags(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	int       flags)		/* フラグ */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_Spr_SetFlags\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(31)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((spr = (SPR_SPRITE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
   if(spr->head.head.id != SP_SPRITE && spr->head.head.id != SP_SPRITE_F)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_SPRITE.\n", strcode);
		return 0;
	}

	spr->head.head.flags = flags;

	return 1;
}

/*******************************************************************************
 * ラインの位置０の設定
 */
int LOM_Lin_SetPosition0(	/* 1: 成功 */
							/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_POS   *pos,			/* 位置 */
	int       flag)			/* 1: 親のスケールとオフセットを見る */
{
	SPR_LINE *lin;

	SY_PRINTF3("LOM_Lin_SetPosition0\n");

	ASSERT(layoutman != NULL);
	ASSERT(pos       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(32)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((lin = (SPR_LINE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
	if(lin->head.id != SP_LINE)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_LINE.\n", strcode);
		return 0;
	}

	lin->pos[0] = *pos;

	/* 親のスケールとオフセットを見る */
	if(flag == 1)
	{
		SPR_OBJ *obj;

		obj = lin->head.parent;
		while(obj != NULL)
		{
			switch(obj->head.id)
			{
			case SP_EMPTY:
				if(((SPR_EMPTY *)obj)->head.scale != 0.0f)
				{
					lin->pos[0].x /= ((SPR_EMPTY *)obj)->head.scale;
					lin->pos[0].y /= ((SPR_EMPTY *)obj)->head.scale;
				}
				lin->pos[0].x -= ((SPR_EMPTY *)obj)->pos.x;
				lin->pos[0].y -= ((SPR_EMPTY *)obj)->pos.y;
				obj            = ((SPR_EMPTY *)obj)->head.parent;
				break;

			case SP_LINE:
				if(((SPR_LINE *)obj)->head.scale != 0.0f)
				{
					lin->pos[0].x /= ((SPR_LINE *)obj)->head.scale;
					lin->pos[0].y /= ((SPR_LINE *)obj)->head.scale;
				}
				lin->pos[0].x -= ((SPR_LINE *)obj)->pos[0].x;
				lin->pos[0].y -= ((SPR_LINE *)obj)->pos[0].y;
				obj            = ((SPR_LINE *)obj)->head.parent;
				break;

			case SP_SPRITE:
         case SP_SPRITE_F:
				if(((SPR_SPRITE *)obj)->head.head.scale != 0.0f)
				{
					lin->pos[0].x /= ((SPR_SPRITE *)obj)->head.head.scale;
					lin->pos[0].y /= ((SPR_SPRITE *)obj)->head.head.scale;
				}
				lin->pos[0].x -= ((SPR_SPRITE *)obj)->pos.x;
				lin->pos[0].y -= ((SPR_SPRITE *)obj)->pos.y;
				obj            = ((SPR_SPRITE *)obj)->head.head.parent;
				break;

			default:
				SY_PRINTF2("Invalid Sprite Type[%d].\n", obj->head.id);
				return 0;
			}
		}
	}

	return 1;
}

/*******************************************************************************
 * ラインの位置１の設定
 */
int LOM_Lin_SetPosition1(	/* 1: 成功 */
							/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_POS   *pos,			/* 位置 */
	int       flag)			/* 1: 親のスケールとオフセットを見る */
{
	SPR_LINE *lin;

	SY_PRINTF3("LOM_Lin_SetPosition1\n");

	ASSERT(layoutman != NULL);
	ASSERT(pos       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(33)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((lin = (SPR_LINE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
	if(lin->head.id != SP_LINE)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_LINE.\n", strcode);
		return 0;
	}

	lin->pos[1] = *pos;

	/* 親のスケールとオフセットを見る */
	if(flag == 1)
	{
		SPR_OBJ *obj;

		obj = lin->head.parent;
		while(obj != NULL)
		{
			switch(obj->head.id)
			{
			case SP_EMPTY:
				if(((SPR_EMPTY *)obj)->head.scale != 0.0f)
				{
					lin->pos[1].x /= ((SPR_EMPTY *)obj)->head.scale;
					lin->pos[1].y /= ((SPR_EMPTY *)obj)->head.scale;
				}
				lin->pos[1].x -= ((SPR_EMPTY *)obj)->pos.x;
				lin->pos[1].y -= ((SPR_EMPTY *)obj)->pos.y;
				obj            = ((SPR_EMPTY *)obj)->head.parent;
				break;

			case SP_LINE:
				if(((SPR_LINE *)obj)->head.scale != 0.0f)
				{
					lin->pos[1].x /= ((SPR_LINE *)obj)->head.scale;
					lin->pos[1].y /= ((SPR_LINE *)obj)->head.scale;
				}
				lin->pos[1].x -= ((SPR_LINE *)obj)->pos[0].x;
				lin->pos[1].y -= ((SPR_LINE *)obj)->pos[0].y;
				obj            = ((SPR_LINE *)obj)->head.parent;
				break;

			case SP_SPRITE:
         case SP_SPRITE_F:
				if(((SPR_SPRITE *)obj)->head.head.scale != 0.0f)
				{
					lin->pos[1].x /= ((SPR_SPRITE *)obj)->head.head.scale;
					lin->pos[1].y /= ((SPR_SPRITE *)obj)->head.head.scale;
				}
				lin->pos[1].x -= ((SPR_SPRITE *)obj)->pos.x;
				lin->pos[1].y -= ((SPR_SPRITE *)obj)->pos.y;
				obj            = ((SPR_SPRITE *)obj)->head.head.parent;
				break;

			default:
				SY_PRINTF2("Invalid Sprite Type[%d].\n", obj->head.id);
				return 0;
			}
		}
	}

	return 1;
}

/*******************************************************************************
 * ラインの色０の設定
 */
int LOM_Lin_SetColor0(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_COLOR *col)			/* 色 */
{
	SPR_LINE *lin;

	SY_PRINTF3("LOM_Lin_SetColor0\n");

	ASSERT(layoutman != NULL);
	ASSERT(col       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(34)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((lin = (SPR_LINE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
	if(lin->head.id != SP_LINE)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_LINE.\n", strcode);
		return 0;
	}

	lin->col[0] = *col;

	return 1;
}

/*******************************************************************************
 * ラインの色１の設定
 */
int LOM_Lin_SetColor1(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_COLOR *col)			/* 色 */
{
	SPR_LINE *lin;

	SY_PRINTF3("LOM_Lin_SetColor1\n");

	ASSERT(layoutman != NULL);
	ASSERT(col       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(35)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((lin = (SPR_LINE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
	if(lin->head.id != SP_LINE)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_LINE.\n", strcode);
		return 0;
	}

	lin->col[1] = *col;

	return 1;
}

/*******************************************************************************
 * ラインのフラグの設定
 */
int LOM_Lin_SetFlags(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	int       flags)		/* フラグ */
{
	SPR_LINE *lin;

	SY_PRINTF3("LOM_Lin_SetFlags\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(36)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((lin = (SPR_LINE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
	if(lin->head.id != SP_LINE)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_LINE.\n", strcode);
		return 0;
	}

	lin->head.flags = flags;

	return 1;
}

/*******************************************************************************
 * 空プリミティブの位置の設定
 */
int LOM_Emp_SetPosition(	/* 1: 成功 */
							/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_POS   *pos,			/* 位置 */
	int       flag)			/* 1: 親のスケールとオフセットを見る */
{
	SPR_EMPTY *emp;

	SY_PRINTF3("LOM_Emp_SetPosition\n");

	ASSERT(layoutman != NULL);
	ASSERT(pos       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(37)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((emp = (SPR_EMPTY *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
	if(emp->head.id != SP_EMPTY)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_EMPTY.\n", strcode);
		return 0;
	}

	emp->pos = *pos;

	/* 親のスケールとオフセットを見る */
	if(flag == 1)
	{
		SPR_OBJ *obj;

		obj = emp->head.parent;
		while(obj != NULL)
		{
			switch(obj->head.id)
			{
			case SP_EMPTY:
				if(((SPR_EMPTY *)obj)->head.scale != 0.0f)
				{
					emp->pos.x /= ((SPR_EMPTY *)obj)->head.scale;
					emp->pos.y /= ((SPR_EMPTY *)obj)->head.scale;
				}
				emp->pos.x -= ((SPR_EMPTY *)obj)->pos.x;
				emp->pos.y -= ((SPR_EMPTY *)obj)->pos.y;
				obj         = ((SPR_EMPTY *)obj)->head.parent;
				break;

			case SP_LINE:
				if(((SPR_LINE *)obj)->head.scale != 0.0f)
				{
					emp->pos.x /= ((SPR_LINE *)obj)->head.scale;
					emp->pos.y /= ((SPR_LINE *)obj)->head.scale;
				}
				emp->pos.x -= ((SPR_LINE *)obj)->pos[0].x;
				emp->pos.y -= ((SPR_LINE *)obj)->pos[0].y;
				obj         = ((SPR_LINE *)obj)->head.parent;
				break;

			case SP_SPRITE:
         case SP_SPRITE_F:
				if(((SPR_SPRITE *)obj)->head.head.scale != 0.0f)
				{
					emp->pos.x /= ((SPR_SPRITE *)obj)->head.head.scale;
					emp->pos.y /= ((SPR_SPRITE *)obj)->head.head.scale;
				}
				emp->pos.x -= ((SPR_SPRITE *)obj)->pos.x;
				emp->pos.y -= ((SPR_SPRITE *)obj)->pos.y;
				obj         = ((SPR_SPRITE *)obj)->head.head.parent;
				break;

			default:
				SY_PRINTF2("Invalid Sprite Type[%d].\n", obj->head.id);
				return 0;
			}
		}
	}

	return 1;
}

/*******************************************************************************
 * 空プリミティブのフラグの設定
 */
int LOM_Emp_SetFlags(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	int       flags)		/* フラグ */
{
	SPR_EMPTY *emp;

	SY_PRINTF3("LOM_Emp_SetFlags\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(38)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((emp = (SPR_EMPTY *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
	if(emp->head.id != SP_EMPTY)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_EMPTY.\n", strcode);
		return 0;
	}

	emp->head.flags = flags;

	return 1;
}

/*******************************************************************************
 * スプライトテクスチャのＵ値の設定
 */
int LOM_SprTex_SetU(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	float     u)			/* Ｕ値 */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_SprTex_SetU\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(39)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((spr = (SPR_SPRITE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
   if(spr->head.head.id != SP_SPRITE && spr->head.head.id != SP_SPRITE_F)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_SPRITE.\n", strcode);
		return 0;
	}

	spr->head.tex.u = SPR_FIXED(u);

	return 1;
}

/*******************************************************************************
 * スプライトテクスチャのＶ値の設定
 */
int LOM_SprTex_SetV(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	float     v)			/* Ｖ値 */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_SprTex_SetV\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(40)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((spr = (SPR_SPRITE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
   if(spr->head.head.id != SP_SPRITE && spr->head.head.id != SP_SPRITE_F)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_SPRITE.\n", strcode);
		return 0;
	}

	spr->head.tex.v = SPR_FIXED(v);

	return 1;
}

/*******************************************************************************
 * スプライトテクスチャの幅の設定
 */
int LOM_SprTex_SetWidth(	/* 1: 成功 */
							/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	float     width)		/* 幅 */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_SprTex_SetWidth\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(41)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((spr = (SPR_SPRITE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
   if(spr->head.head.id != SP_SPRITE && spr->head.head.id != SP_SPRITE_F)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_SPRITE.\n", strcode);
		return 0;
	}

	spr->head.tex.w = SPR_FIXED(width);

	return 1;
}

/*******************************************************************************
 * スプライトテクスチャの高さの設定
 */
int LOM_SprTex_SetHeight(	/* 1: 成功 */
							/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	float     height)		/* 高さ */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_SprTex_SetHeight\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(42)\n");
		return 0;
	}

	/* オブジェクトの取得 */
	if((spr = (SPR_SPRITE *)L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return 0;
	}
	/* スプライト種別識別ＩＤのチェック */
   if(spr->head.head.id != SP_SPRITE && spr->head.head.id != SP_SPRITE_F)
	{
		SY_PRINTF2("Object[%d] Type Is Not SP_SPRITE.\n", strcode);
		return 0;
	}

	spr->head.tex.h = SPR_FIXED(height);

	return 1;
}
