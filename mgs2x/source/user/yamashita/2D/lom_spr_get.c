//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * layout manager - lom_spr_get.c
 * レイアウトマネージャ
 * 2002/04/16 S.Yamashita
 * $Id: lom_spr_get.c,v 1.1.1.3 2002/11/19 11:51:40 Yoshizawa1 Exp $
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
 * macros
 */

#if 0 //BP_PS2 def PSX2
	/* --- PS2 --- */
	#define SPR_UNFIXED(n) ((float)(n) / 16.0F)
#else
	/* --- XBOX --- */
	#define SPR_UNFIXED(n) ((float)(n))
#endif

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * スプライト種別識別ＩＤの取得
 */
int LOM_GetSprID(	/* スプライト種別識別ＩＤ */
					/* -1: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode)		/* オブジェクト名 */
{
	SPR_OBJ *spr;

	SY_PRINTF3("LOM_GetSprID\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(10)\n");
		return -1;
	}

	/* オブジェクトの取得 */
	if((spr = L2D_GetObject(layoutman->layout, strcode)) == NULL)
	{
		SY_PRINTF2("Get Object[%d] Failed.\n", strcode);
		return -1;
	}

	return ((SPR_HEADER *)spr)->id;
}

/*******************************************************************************
 * スプライトの位置の取得
 */
int LOM_Spr_GetPosition(	/* 1: 成功 */
							/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_POS   *pos,			/* 位置 */
	int       flag)			/* 1: 親のスケールとオフセットを見る */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_Spr_GetPosition\n");

	ASSERT(layoutman != NULL);
	ASSERT(pos       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(11)\n");
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

	*pos = spr->pos;

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
				pos->x *= ((SPR_EMPTY *)obj)->head.scale;
				pos->y *= ((SPR_EMPTY *)obj)->head.scale;
				pos->x += ((SPR_EMPTY *)obj)->pos.x;
				pos->y += ((SPR_EMPTY *)obj)->pos.y;
				obj     = ((SPR_EMPTY *)obj)->head.parent;
				break;

			case SP_LINE:
				pos->x *= ((SPR_LINE *)obj)->head.scale;
				pos->y *= ((SPR_LINE *)obj)->head.scale;
				pos->x += ((SPR_LINE *)obj)->pos[0].x;
				pos->y += ((SPR_LINE *)obj)->pos[0].y;
				obj     = ((SPR_LINE *)obj)->head.parent;
				break;

			case SP_SPRITE:
         case SP_SPRITE_F:
				pos->x *= ((SPR_SPRITE *)obj)->head.head.scale;
				pos->y *= ((SPR_SPRITE *)obj)->head.head.scale;
				pos->x += ((SPR_SPRITE *)obj)->pos.x;
				pos->y += ((SPR_SPRITE *)obj)->pos.y;
				obj     = ((SPR_SPRITE *)obj)->head.head.parent;
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
 * スプライトの色の取得
 */
int LOM_Spr_GetColor(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_COLOR *col)			/* 色 */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_Spr_GetColor\n");

	ASSERT(layoutman != NULL);
	ASSERT(col       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(12)\n");
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

	*col = spr->col;

	return 1;
}

/*******************************************************************************
 * スプライトの幅の取得
 */
int LOM_Spr_GetWidth(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	float     *width,		/* 幅 */
	int       flag)			/* 1: 親のスケールを見る */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_Spr_GetWidth\n");

	ASSERT(layoutman != NULL);
	ASSERT(width     != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(13)\n");
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

	*width = spr->dw;

	/* 親のスケールを見る */
	if(flag == 1)
	{
		SPR_OBJ *obj;

		obj = spr->head.head.parent;
		while(obj != NULL)
		{
			*width *= obj->head.scale;
			obj     = obj->head.parent;
		}
	}

	return 1;
}

/*******************************************************************************
 * スプライトの高さの取得
 */
int LOM_Spr_GetHeight(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	float     *height,		/* 高さ */
	int       flag)			/* 1: 親のスケールを見る */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_Spr_GetHeight\n");

	ASSERT(layoutman != NULL);
	ASSERT(height    != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(14)\n");
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

	*height = spr->dh;

	/* 親のスケールを見る */
	if(flag == 1)
	{
		SPR_OBJ *obj;

		obj = spr->head.head.parent;
		while(obj != NULL)
		{
			*height *= obj->head.scale;
			obj      = obj->head.parent;
		}
	}

	return 1;
}

/*******************************************************************************
 * スプライトのフラグの取得
 */
int LOM_Spr_GetFlags(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	int       *flags)		/* フラグ */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_Spr_GetFlags\n");

	ASSERT(layoutman != NULL);
	ASSERT(flags     != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(15)\n");
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

	*flags = spr->head.head.flags;

	return 1;
}

/*******************************************************************************
 * ラインの位置０の取得
 */
int LOM_Lin_GetPosition0(	/* 1: 成功 */
							/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_POS   *pos,			/* 位置 */
	int       flag)			/* 1: 親のスケールとオフセットを見る */
{
	SPR_LINE *lin;

	SY_PRINTF3("LOM_Lin_GetPosition0\n");

	ASSERT(layoutman != NULL);
	ASSERT(pos       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(16)\n");
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

	*pos = lin->pos[0];

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
				pos->x *= ((SPR_EMPTY *)obj)->head.scale;
				pos->y *= ((SPR_EMPTY *)obj)->head.scale;
				pos->x += ((SPR_EMPTY *)obj)->pos.x;
				pos->y += ((SPR_EMPTY *)obj)->pos.y;
				obj     = ((SPR_EMPTY *)obj)->head.parent;
				break;

			case SP_LINE:
				pos->x *= ((SPR_LINE *)obj)->head.scale;
				pos->y *= ((SPR_LINE *)obj)->head.scale;
				pos->x += ((SPR_LINE *)obj)->pos[0].x;
				pos->y += ((SPR_LINE *)obj)->pos[0].y;
				obj     = ((SPR_LINE *)obj)->head.parent;
				break;

			case SP_SPRITE:
         case SP_SPRITE_F:
				pos->x *= ((SPR_SPRITE *)obj)->head.head.scale;
				pos->y *= ((SPR_SPRITE *)obj)->head.head.scale;
				pos->x += ((SPR_SPRITE *)obj)->pos.x;
				pos->y += ((SPR_SPRITE *)obj)->pos.y;
				obj     = ((SPR_SPRITE *)obj)->head.head.parent;
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
 * ラインの位置１の取得
 */
int LOM_Lin_GetPosition1(	/* 1: 成功 */
							/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_POS   *pos,			/* 位置 */
	int       flag)			/* 1: 親のスケールとオフセットを見る */
{
	SPR_LINE *lin;

	SY_PRINTF3("LOM_Lin_GetPosition1\n");

	ASSERT(layoutman != NULL);
	ASSERT(pos       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(17)\n");
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

	*pos = lin->pos[1];

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
				pos->x *= ((SPR_EMPTY *)obj)->head.scale;
				pos->y *= ((SPR_EMPTY *)obj)->head.scale;
				pos->x += ((SPR_EMPTY *)obj)->pos.x;
				pos->y += ((SPR_EMPTY *)obj)->pos.y;
				obj     = ((SPR_EMPTY *)obj)->head.parent;
				break;

			case SP_LINE:
				pos->x *= ((SPR_LINE *)obj)->head.scale;
				pos->y *= ((SPR_LINE *)obj)->head.scale;
				pos->x += ((SPR_LINE *)obj)->pos[0].x;
				pos->y += ((SPR_LINE *)obj)->pos[0].y;
				obj     = ((SPR_LINE *)obj)->head.parent;
				break;

			case SP_SPRITE:
         case SP_SPRITE_F:
				pos->x *= ((SPR_SPRITE *)obj)->head.head.scale;
				pos->y *= ((SPR_SPRITE *)obj)->head.head.scale;
				pos->x += ((SPR_SPRITE *)obj)->pos.x;
				pos->y += ((SPR_SPRITE *)obj)->pos.y;
				obj     = ((SPR_SPRITE *)obj)->head.head.parent;
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
 * ラインの色０の取得
 */
int LOM_Lin_GetColor0(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_COLOR *col)			/* 色 */
{
	SPR_LINE *lin;

	SY_PRINTF3("LOM_Lin_GetColor0\n");

	ASSERT(layoutman != NULL);
	ASSERT(col       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(18)\n");
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

	*col = lin->col[0];

	return 1;
}

/*******************************************************************************
 * ラインの色１の取得
 */
int LOM_Lin_GetColor1(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_COLOR *col)			/* 色 */
{
	SPR_LINE *lin;

	SY_PRINTF3("LOM_Lin_GetColor1\n");

	ASSERT(layoutman != NULL);
	ASSERT(col       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(19)\n");
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

	*col = lin->col[1];

	return 1;
}

/*******************************************************************************
 * ラインのフラグの取得
 */
int LOM_Lin_GetFlags(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	int       *flags)		/* フラグ */
{
	SPR_LINE *lin;

	SY_PRINTF3("LOM_Lin_GetFlags\n");

	ASSERT(layoutman != NULL);
	ASSERT(flags     != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(20)\n");
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

	*flags = lin->head.flags;;

	return 1;
}

/*******************************************************************************
 * 空プリミティブの位置の取得
 */
int LOM_Emp_GetPosition(	/* 1: 成功 */
							/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	SPR_POS   *pos,			/* 位置 */
	int       flag)			/* 1: 親のスケールとオフセットを見る */
{
	SPR_EMPTY *emp;

	SY_PRINTF3("LOM_Emp_GetPosition\n");

	ASSERT(layoutman != NULL);
	ASSERT(pos       != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(21)\n");
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

	*pos = emp->pos;

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
				pos->x *= ((SPR_EMPTY *)obj)->head.scale;
				pos->y *= ((SPR_EMPTY *)obj)->head.scale;
				pos->x += ((SPR_EMPTY *)obj)->pos.x;
				pos->y += ((SPR_EMPTY *)obj)->pos.y;
				obj     = ((SPR_EMPTY *)obj)->head.parent;
				break;

			case SP_LINE:
				pos->x *= ((SPR_LINE *)obj)->head.scale;
				pos->y *= ((SPR_LINE *)obj)->head.scale;
				pos->x += ((SPR_LINE *)obj)->pos[0].x;
				pos->y += ((SPR_LINE *)obj)->pos[0].y;
				obj     = ((SPR_LINE *)obj)->head.parent;
				break;

			case SP_SPRITE_F:
				pos->x *= ((SPR_SPRITE *)obj)->head.head.scale;
				pos->y *= ((SPR_SPRITE *)obj)->head.head.scale;
				pos->x += ((SPR_SPRITE *)obj)->pos.x;
				pos->y += ((SPR_SPRITE *)obj)->pos.y;
				obj     = ((SPR_SPRITE *)obj)->head.head.parent;
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
 * 空プリミティブのフラグの取得
 */
int LOM_Emp_GetFlags(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	int       *flags)		/* フラグ */
{
	SPR_EMPTY *emp;

	SY_PRINTF3("LOM_Emp_GetFlags\n");

	ASSERT(layoutman != NULL);
	ASSERT(flags     != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(22)\n");
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

	*flags = emp->head.flags;

	return 1;
}

/*******************************************************************************
 * スプライトテクスチャのＵ値の取得
 */
int LOM_SprTex_GetU(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	float     *u)			/* Ｕ値 */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_SprTex_GetU\n");

	ASSERT(layoutman != NULL);
	ASSERT(u         != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(23)\n");
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

	*u = SPR_UNFIXED(spr->head.tex.u);

	return 1;
}

/*******************************************************************************
 * スプライトテクスチャのＶ値の取得
 */
int LOM_SprTex_GetV(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	float     *v)			/* Ｖ値 */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_SprTex_GetV\n");

	ASSERT(layoutman != NULL);
	ASSERT(v         != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(24)\n");
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

	*v = SPR_UNFIXED(spr->head.tex.v);

	return 1;
}

/*******************************************************************************
 * スプライトテクスチャの幅の取得
 */
int LOM_SprTex_GetWidth(	/* 1: 成功 */
							/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	float     *width)		/* 幅 */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_SprTex_GetWidth\n");

	ASSERT(layoutman != NULL);
	ASSERT(width     != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(25)\n");
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

	*width = SPR_UNFIXED(spr->head.tex.w);

	return 1;
}

/*******************************************************************************
 * スプライトテクスチャの高さの取得
 */
int LOM_SprTex_GetHeight(	/* 1: 成功 */
							/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       strcode,		/* オブジェクト名 */
	float     *height)		/* 高さ */
{
	SPR_SPRITE *spr;

	SY_PRINTF3("LOM_SprTex_GetHeight\n");

	ASSERT(layoutman != NULL);
	ASSERT(height    != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(26)\n");
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

	*height = SPR_UNFIXED(spr->head.tex.h);

	return 1;
}
