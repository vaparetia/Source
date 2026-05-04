//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * outline - outline.c
 * アウトライン
 * 2002/03/11 S.Yamashita
 * $Id: outline.c,v 1.1.1.3 2002/11/19 11:51:51 Yoshizawa1 Exp $
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

//#include "libdg.h"
//#include "libgv.h"
#include "gameheader.h"

#define __CHARA_NAME__ "Outline"
#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
#include "../sy_util/sy_util.h"

#include "outline.h"

#include "BP_EndianSupport.h"

/*******************************************************************************
 * definitions and typedefs and structures
 */

#define MEM_ADDR1 ((void*)(SCRPAD_ADDR + 0x0000))
#define MEM_ADDR2 ((void*)(SCRPAD_ADDR + 0x2000))

/*******************************************************************************
 * macro
 */

#define RND(n) (((BP_PS2_rand() >> 16) * n) >> 15)

#define COL_A(flag) (outline->col[0][OLO_FLAG_COLORMASK & (flag)])
#define COL_B(flag) (outline->col[1][OLO_FLAG_COLORMASK & (flag)])

static void EndianSwap_OL_VERTEX( OL_VERTEX *pData )
{
   BP_LE_SwapSInt_Inp( &pData->flag );
   BP_LE_SwapFloat_Inp( &pData->fx );
   BP_LE_SwapFloat_Inp( &pData->fy );
   BP_LE_SwapFloat_Inp( &pData->fz );
}

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * アウトラインの生成
 */
int CreateOutline(	/* 1: 成功 */
					/* 0: 失敗 */
	OUTLINE *outline,	/* アウトライン */
	void    *mem_le,		/* メモリアドレス */
	DG_OBJS *dg_objs,	/* ＤＧオブジェクト (NULL: 動かない) */
	CVECTOR *col_a,		/* 色Ａ配列 (NULL: 全て白になる) */
	FVECTOR *offset,	/* オフセット (NULL: オフセットなし) */
	int     flag,     /* フラグ */
   int     was_swapped)		
{
	int     i, j, k;
	int     *p_le;
	FVECTOR trans  = {0.0f, 0.0f, 0.0f, 0.0f};

   // BP - Endian Swapping
   // Note that the only things that are endian swapped in the Outline 
   // are OL_VERTEX objects.  Everything else is kept little-endian
   // because it's only used at load time and it's not all mapped to a structure
   // but rather just streaming in through an int pointer

	SY_PRINTF3("CreateOutline\n");

	ASSERT(outline != NULL);
	ASSERT(mem_le  != NULL);

	/* アウトラインの初期化 */
	DestroyOutline(outline);
	outline->mem_le  = mem_le;
	outline->dg_objs = dg_objs;
	if(col_a != NULL)  memcpy(outline->col[0], col_a, sizeof(outline->col[0]));
	else               memset(outline->col[0], 0xff , sizeof(outline->col[0]));
	if(offset != NULL) outline->offset = *offset;

	/* メモリから読み込み */
	p_le = (int *)mem_le;

	/* フラグ */
	outline->flag = (BP_LE_SwapSInt(*p_le) & OL_FLAG_FILEMASK) | (flag & OL_FLAG_OPTIONMASK);
	p_le++;
	SY_PRINTF1("Outline Flag: %x\n", outline->flag);

	/* 移動値の修正 */
	if(outline->dg_objs != NULL)
	{
		if((outline->flag & OL_FLAG_ONEOBJ) && (outline->flag & OL_FLAG_ERASETRANS))
		{
			trans = outline->dg_objs->trans;
		}
	}

	/* オブジェクト数 */
	outline->objcount = BP_LE_SwapSInt( *p_le );
	p_le++;
	if(    (outline->dg_objs != NULL)
		&& ((outline->flag & OL_FLAG_ONEOBJ) == 0))
	{
		/* 関節モードのチェック */
		if(outline->dg_objs->n_models < outline->objcount)
		{
			SY_PRINTF2("Object count does not match!\n");
			DestroyOutline(outline);
			return 0;
		}
	}
	SY_PRINTF1("Outline Obj Count: %d\n", outline->objcount);

	/* メモリ確保 */
	if((outline->ol_obj = (OL_OBJECT *)GV_Malloc(sizeof(OL_OBJECT) * outline->objcount)) == NULL)
	{
		SY_PRINTF2("No memory!\n");
		DestroyOutline(outline);
		return 0;
	}

	/* 各オブジェクト */
	for(i = 0; i < outline->objcount; i++)
	{
		OL_OBJECT *ol_obj;

		ol_obj = &outline->ol_obj[i];

		/* 移動値の修正 */
		if(outline->dg_objs != NULL)
		{
			if(!(outline->flag & OL_FLAG_ONEOBJ) && (outline->flag & OL_FLAG_ERASETRANS))
			{
				DG_OBJ *dg_obj;

				dg_obj = &outline->dg_objs->objs[i];
				trans  = dg_obj->trans;
				while(dg_obj->parent > 0)
				{
					dg_obj    = &outline->dg_objs->objs[dg_obj->parent];
					trans.vx += dg_obj->trans.vx;
					trans.vy += dg_obj->trans.vy;
					trans.vz += dg_obj->trans.vz;
				}
			}
		}

		/* ＩＤ */
		ol_obj->id = BP_LE_SwapSInt( *p_le );
		p_le++;

		/* フラグ */
		ol_obj->flag = BP_LE_SwapSInt( *p_le );
		p_le++;

		/* プリミティブ数 */
		ol_obj->prims = BP_LE_SwapSInt( *p_le );
		p_le++;
		if(ol_obj->prims < 1)
		{
			SY_PRINTF2("No Primitive!\n");
			DestroyOutline(outline);
			return 0;
		}

		/* 頂点数 */
		ol_obj->verts = BP_LE_SwapSInt( *p_le );
		p_le++;
		if(ol_obj->verts < 2)
		{
			SY_PRINTF2("Not enough Vertex!\n");
			DestroyOutline(outline);
			return 0;
		}

		/* 頂点数２ */
		if(ol_obj->prims == 1)
		{
			ol_obj->verts2 = 0;
		}
		else
		{
			ol_obj->verts2 = BP_LE_SwapSInt( *p_le );
			p_le++;
			if(ol_obj->verts2 < 2)
			{
				SY_PRINTF2("Not enough Vertex2!\n");
				DestroyOutline(outline);
				return 0;
			}
		}

		/* ＤＧプリミティブ */
		if(outline->flag & OL_FLAG_ANTIALIASING)
			ol_obj->dgprims = GM_MakePrim2(DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_ALPHA|DG_PRIM2_ANTIALIASING, ol_obj->prims, ol_obj->verts);
		else
			ol_obj->dgprims = GM_MakePrim2(DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_ALPHA, ol_obj->prims, ol_obj->verts);
		if(ol_obj->dgprims == NULL)
		{
			SY_PRINTF2("MakePrim Failed!\n");
			DestroyOutline(outline);
			return 0;
		}
	DG_SetPrim2Alpha( ol_obj->dgprims, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

		/* ＤＧプリミティブの初期化 */
		{
			OL_VERTEX      *ol_vert;
			FVECTOR        *pos  , *pos0  , *pos1;
			DG_PRIM2_UVRGB *uvrgb, *uvrgb0, *uvrgb1;

			ol_vert      = (OL_VERTEX *)p_le;

			ol_obj->edge = 0;

			pos0   = ol_obj->dgprims->pos[0];
			uvrgb0 = ol_obj->dgprims->uvrgb[0];
			pos1   = ol_obj->dgprims->pos[1];
			uvrgb1 = ol_obj->dgprims->uvrgb[1];

			for(k = 0; k < ol_obj->prims; k++)
			{
            pos   = MEM_ADDR1;
				uvrgb = MEM_ADDR2;

				/* 位置の設定等 */
				for(j = 0; j < ol_obj->verts; j++)
				{
					if(    (ol_obj->prims == 1    )
						|| (k != ol_obj->prims - 1)
						|| (j < ol_obj->verts2    ))
					{
                  // BP - NOTE THAT THIS IS THE ONLY DATA THAT IS SWAPPED IN THE OUTLINE
                  if ( !was_swapped )
                  {
                     EndianSwap_OL_VERTEX( ol_vert );
                  }

						pos[j].vx  = ol_vert->fx - trans.vx + outline->offset.vx;
						pos[j].vy  = ol_vert->fy - trans.vy + outline->offset.vy;
						pos[j].vz  = ol_vert->fz - trans.vz + outline->offset.vz;
						pos[j].vw  = 1.0;
						uvrgb[j].q = 4096;
						if(ol_vert->flag & OLV_FLAG_DRAWEDGE) {uvrgb[j].f = 0x0fff; ol_obj->edge++;}
						else                                  {uvrgb[j].f = 0x8fff;                }
						ol_vert++;
					}
					else
					{
						/* 余り */
						pos[j].vx = pos[j].vy = pos[j].vz = 0.0;
						pos[j].vw = 1.0;
						uvrgb[j].q = 4096;
						uvrgb[j].f = 0x8fff;
					}
				}
				/* 色の設定 */
				if(outline->flag & OL_FLAG_ANTIALIASING)
					sy_setrgba(MEM_ADDR2, COL_A(ol_obj->flag).r, COL_A(ol_obj->flag).g, COL_A(ol_obj->flag).b, 128                   , ol_obj->verts);
				else
					sy_setrgba(MEM_ADDR2, COL_A(ol_obj->flag).r, COL_A(ol_obj->flag).g, COL_A(ol_obj->flag).b, COL_A(ol_obj->flag).cd, ol_obj->verts);

				/* メモリのコピー */
				sy_memcpy(pos0  , MEM_ADDR1, sizeof(FVECTOR)       , ol_obj->verts);
				sy_memcpy(uvrgb0, MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), ol_obj->verts);
				sy_memcpy(pos1  , MEM_ADDR1, sizeof(FVECTOR)       , ol_obj->verts);
				sy_memcpy(uvrgb1, MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), ol_obj->verts);
				pos0   += ol_obj->verts;
				uvrgb0 += ol_obj->verts;
				pos1   += ol_obj->verts;
				uvrgb1 += ol_obj->verts;
			}

			p_le           = (int *)ol_vert;
			outline->edge += ol_obj->edge;
		}

		/* マトリクス */
		if(outline->dg_objs != NULL)
		{
			if(outline->flag & OL_FLAG_ONEOBJ)
			{
				ol_obj->dgprims->root = &outline->dg_objs->world;
			}
			else
			{
				ol_obj->dgprims->root = &outline->dg_objs->objs[i].world;
			}
		}
	} /* for(i = 0; i < outline->objcount; i++) */

	return 1;
}

/*******************************************************************************
 * アウトラインの破棄
 */
void DestroyOutline(
	OUTLINE *outline)	/* アウトライン */
{
	int i;

	SY_PRINTF3("DestroyOutline\n");

	ASSERT(outline != NULL);

	/* アウトラインオブジェクト配列 */
	if(outline->ol_obj != NULL)
	{
		for(i = 0; i < outline->objcount; i++)
		{
			if(outline->ol_obj[i].dgprims != NULL)
				GM_FreePrim2(outline->ol_obj[i].dgprims);
		}

		GV_Free(outline->ol_obj);
	}

	memset(outline, 0, sizeof(OUTLINE));
}

/*******************************************************************************
 * アウトラインの毎フレーム処理
 */
void ActOutline(
	OUTLINE *outline)	/* アウトライン */
{
	int i,j,  k;

	ASSERT(outline != NULL);

	if(outline->flag & OL_FLAG_COL_MODIFIED)
	{
		outline->flag &= ~OL_FLAG_COL_MODIFIED;

		for(i = 0; i < outline->objcount; i++)
		{
			OL_OBJECT *ol_obj;

			ol_obj = &outline->ol_obj[i];

			/* プリミティブのアニメーション */
			{
				DG_PRIM2_UVRGB *dst, *src;

				DG_SwitchBuffPrim2(ol_obj->dgprims);
				dst = ol_obj->dgprims->uvrgb[ol_obj->dgprims->buffer_clock];
				src = ol_obj->dgprims->uvrgb[1 - ol_obj->dgprims->buffer_clock];

				for(k = 0; k < ol_obj->prims; k++)
				{
					/* 色の設定 */
					sy_memcpy2(MEM_ADDR2, src, sizeof(DG_PRIM2_UVRGB), ol_obj->verts);
					if(outline->flag & OL_FLAG_ANTIALIASING)
					{
						sy_setrgba(MEM_ADDR2, COL_A(ol_obj->flag).r, COL_A(ol_obj->flag).g, COL_A(ol_obj->flag).b, 128                   , ol_obj->verts);
					}
					else
					{
						sy_setrgba(MEM_ADDR2, COL_A(ol_obj->flag).r, COL_A(ol_obj->flag).g, COL_A(ol_obj->flag).b, COL_A(ol_obj->flag).cd, ol_obj->verts);
					}
					sy_memcpy(dst, MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), ol_obj->verts);
					dst += ol_obj->verts;
					src += ol_obj->verts;
				}
			}
		}
	}
}

#if 0
/*******************************************************************************
 * シグナル処理
	OL_SIG_SetColorA = 0,	 CVECTOR[4] へのポインタ
	OL_SIG_SetColorB,		 CVECTOR[4] へのポインタ
	OL_SIG_SetColorAB,		 OL_COLOR へのポインタ
	OL_SIG_SetColor,		 OL_COLOR へのポインタ
	OL_SIG_SetRange,		 OL_RANGE へのポインタ
 */
int OutlineSignal(	/* 処理結果 */
	OUTLINE *outline)	/* アウトライン */
	int  signal,	/* シグナル */
	int  value)		/* 付加情報 */
{
	int  ret;
	WORK *work  = (WORK *)pwork;

	SY_PRINTF3("ReceiveSignal\n");
	SY_PRINTF2("Signal: %d\n", signal);

	ASSERT(outline != NULL);

	switch(signal)
	{
	case OL_SIG_SetColorA:
		OutlineSetColorA(&work->outline, (CVECTOR *)value);
		ret = OL_SIG_SetColorA;
		break;

	case OL_SIG_SetColorB:
		OutlineSetColorB(&work->outline, (CVECTOR *)value);
		ret = OL_SIG_SetColorB;
		break;

	case OL_SIG_SetColorAB:
		OutlineSetColorAB(&work->outline,
			((OL_COLOR *)value)->col_num, ((OL_COLOR *)value)->col);
		ret = OL_SIG_SetColorAB;
		break;

	case OL_SIG_SetColor:
		OutlineSetColor(&work->outline,
			((OL_COLOR *)value)->col_num, ((OL_COLOR *)value)->col);
		ret = OL_SIG_SetColor;
		break;

	case OL_SIG_SetRange:
		if(((OL_RANGE *)value)->mode < 3)
			OutlineSetXYZRange(&work->outline,
				((OL_RANGE *)value)->mode, ((OL_RANGE *)value)->range);
		else
			OutlineSetPRange(&work->outline,
				((OL_RANGE *)value)->mode, ((OL_RANGE *)value)->range);
		ret = OL_SIG_SetRange;
		break;

	default:
		return GV_DefaultSignalFunc(work, signal, value);
	}

	return ret;
}
#endif

/*******************************************************************************
 * アウトライン 表示の設定
 */
void OutlineShow(
	OUTLINE *outline,	/* アウトライン */
	int     show)		/* 0: 非表示  1: 表示 */
{
	int i;

	if(show == 0)
	{
		for(i = 0; i < outline->objcount; i++)
			DG_InvisiblePrim2(outline->ol_obj[i].dgprims);
	}
	else
	{
		for(i = 0; i < outline->objcount; i++)
			DG_VisiblePrim2(outline->ol_obj[i].dgprims);
	}
}

/*******************************************************************************
 * アウトライン 色配列Ａの設定
 */
void OutlineSetColorA(
	OUTLINE *outline,	/* アウトライン */
	CVECTOR *col_a)		/* 色配列（４要素） */
{
	SY_PRINTF3("OutlineSetColorA\n");

	ASSERT(outline != NULL)
	ASSERT(col_a   != NULL)

	memcpy(outline->col[0], col_a, sizeof(outline->col[0]));

	outline->flag |= OL_FLAG_COL_MODIFIED;
}

/*******************************************************************************
 * アウトライン 色配列Ｂの設定
 */
void OutlineSetColorB(
	OUTLINE *outline,	/* アウトライン */
	CVECTOR *col_b)		/* 色配列（４要素） */
{
	SY_PRINTF3("OutlineSetColorB\n");

	ASSERT(outline != NULL)
	ASSERT(col_b   != NULL)

	memcpy(outline->col[1], col_b, sizeof(outline->col[1]));

	outline->flag |= OL_FLAG_COL_MODIFIED;
}

/*******************************************************************************
 * アウトライン 色ＡＢの設定
 */
void OutlineSetColorAB(
	OUTLINE *outline,	/* アウトライン */
	int     col_num,	/* 色番号 */
	CVECTOR *col_ab)	/* 色ＡＢ（２要素） */
{
	SY_PRINTF3("OutlineSetColorAB\n");

	ASSERT(outline != NULL)
	ASSERT(col_ab  != NULL)

	if((col_num < 0) || (3 < col_num))
		return;

	memcpy(&outline->col[0][col_num], &col_ab[0], sizeof(outline->col[0][col_num]));
	memcpy(&outline->col[1][col_num], &col_ab[1], sizeof(outline->col[1][col_num]));

	outline->flag |= OL_FLAG_COL_MODIFIED;
}

/*******************************************************************************
 * アウトライン 色の設定
 */
void OutlineSetColor(
	OUTLINE *outline,	/* アウトライン */
	int     col_num,	/* 色番号  0:０のＡ  1:１のＡ ...  4:０のＢ  5:１のＢ ... */
	CVECTOR *col)		/* 色 */
{
	SY_PRINTF3("OutlineSetColor\n");

	ASSERT(outline != NULL)
	ASSERT(col     != NULL)

	if((col_num < 0) || (7 < col_num))
		return;

	if(col_num < 4)
	{
		outline->col[0][col_num] = *col;
	}
	else
	{
		outline->col[1][col_num % 4] = *col;
	}

	outline->flag |= OL_FLAG_COL_MODIFIED;
}

/*******************************************************************************
 * アウトライン Ｘ・Ｙ・Ｚ座標範囲の設定
 */
void OutlineSetXYZRange(
	OUTLINE *outline,	/* アウトライン */
	int     mode,		/* 0:Ｘ方向  1:Ｙ方向  2:Ｚ方向 */
	float   *range)		/* 座標範囲 */
{
	int i, j, k;
	int *p_le;

	SY_PRINTF3("OutlineSetXYZRange\n");

	ASSERT(outline != NULL)
	ASSERT(range   != NULL)

	if((mode < 0) || (2 < mode))
		return;

	/* メモリから読み込み */
	p_le = (int *)outline->mem_le;
	/* フラグ */
	p_le++;
	/* オブジェクト数 */
	p_le++;

	/* 各オブジェクト */
	for(i = 0; i < outline->objcount; i++)
	{
		OL_OBJECT *ol_obj;

		ol_obj = &outline->ol_obj[i];

		/* ＩＤ */
		p_le++;
		/* フラグ */
		p_le++;
		/* プリミティブ数 */
		p_le++;
		/* 頂点数 */
		p_le++;
		/* 頂点数２ */
		if(ol_obj->prims != 1)
			p_le++;

		/* ＤＧプリミティブ */
		{
			OL_VERTEX      *ol_vert;
			DG_PRIM2_UVRGB *dst, *src, *uvrgb;

			ol_vert = (OL_VERTEX *)p_le; // BP - THIS HAS ALREADY BEEN ENDIAN SWAPPED!!!11

			DG_SwitchBuffPrim2(ol_obj->dgprims);
			dst = ol_obj->dgprims->uvrgb[ol_obj->dgprims->buffer_clock];
			src = ol_obj->dgprims->uvrgb[1 - ol_obj->dgprims->buffer_clock];

			for(k = 0; k < ol_obj->prims; k++)
			{
				uvrgb = MEM_ADDR2;

				/* 色の設定 */
				sy_memcpy2(MEM_ADDR2, src, sizeof(DG_PRIM2_UVRGB), ol_obj->verts);
				for(j = 0; j < ol_obj->verts; j++)
				{
					if(    (ol_obj->prims == 1    )
						|| (k != ol_obj->prims - 1)
						|| (j < ol_obj->verts2    ))
					{
						if(range[0] < range[1])
						{
							if(    ((mode == 0) && ((ol_vert->fx < range[0]) || (range[1] < ol_vert->fx)))
								|| ((mode == 1) && ((ol_vert->fy < range[0]) || (range[1] < ol_vert->fy)))
								|| ((mode == 2) && ((ol_vert->fz < range[0]) || (range[1] < ol_vert->fz))))
							{
								// 自分は範囲外
								uvrgb[j].f = 0x8fff;
							}
							else
							{
								// 自分は範囲内
								uvrgb[j].f = 0x8fff;
								if(j != 0)
								{
									if(  !(((mode == 0) && (((ol_vert - 1)->fx < range[0]) || (range[1] < (ol_vert - 1)->fx)))
										|| ((mode == 1) && (((ol_vert - 1)->fy < range[0]) || (range[1] < (ol_vert - 1)->fy)))
										|| ((mode == 2) && (((ol_vert - 1)->fz < range[0]) || (range[1] < (ol_vert - 1)->fz)))))
									{
										if(ol_vert->flag & OLV_FLAG_DRAWEDGE) uvrgb[j].f = 0x0fff;
										else                                  uvrgb[j].f = 0x8fff;
									}
								}
							}
						}
						else
						{
							if(    ((mode == 0) && ((range[1] < ol_vert->fx) && (ol_vert->fx < range[0])))
								|| ((mode == 1) && ((range[1] < ol_vert->fy) && (ol_vert->fy < range[0])))
								|| ((mode == 2) && ((range[1] < ol_vert->fz) && (ol_vert->fz < range[0]))))
							{
								// 自分は範囲外
								uvrgb[j].f = 0x8fff;
							}
							else
							{
								// 自分は範囲内
								uvrgb[j].f = 0x8fff;
								if(j != 0)
								{
									if(  !(((mode == 0) && ((range[1] < (ol_vert - 1)->fx) && ((ol_vert - 1)->fx < range[0])))
										|| ((mode == 1) && ((range[1] < (ol_vert - 1)->fy) && ((ol_vert - 1)->fy < range[0])))
										|| ((mode == 2) && ((range[1] < (ol_vert - 1)->fz) && ((ol_vert - 1)->fz < range[0])))))
									{
										if(ol_vert->flag & OLV_FLAG_DRAWEDGE) uvrgb[j].f = 0x0fff;
										else                                  uvrgb[j].f = 0x8fff;
									}
								}
							}
						}
						ol_vert++;
					}
				}
				sy_memcpy(dst, MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), ol_obj->verts);

				dst += ol_obj->verts;
				src += ol_obj->verts;
			}

			p_le = (int *)ol_vert;
		}
	}	/* for(i = 0; i < outline->objcount; i++) */
}

/*******************************************************************************
 * アウトライン ％範囲の設定
 */
void OutlineSetPRange(
	OUTLINE *outline,	/* アウトライン */
	int     mode,		/* 3:部分（全体）  4:部分タイプ２（間接ごと） */
	float   *range)		/* ％範囲（0.0 ～ 1.0）（２要素） */
{
	int i, j, k;
	int *p_le;
	int irange[2];
	int edge = 0;

	SY_PRINTF3("OutlineSetPRange\n");

	ASSERT(outline != NULL)
	ASSERT(range   != NULL)

	if((mode < 3) || (4 < mode))
		return;

	/* 範囲の設定 */
	if(mode == 3)
	{
		irange[0] = outline->edge * range[0];
		irange[1] = outline->edge * range[1];
		edge      = 0;
	}

	/* メモリから読み込み */
	p_le = (int *)outline->mem_le;
	/* フラグ */
	p_le++;
	/* オブジェクト数 */
	p_le++;

	/* 各オブジェクト */
	for(i = 0; i < outline->objcount; i++)
	{
		OL_OBJECT *ol_obj;

		ol_obj = &outline->ol_obj[i];

		/* 範囲の設定 */
		if(mode == 4)
		{
			irange[0] = ol_obj->edge * range[0];
			irange[1] = ol_obj->edge * range[1];
			edge      = 0;
		}

		/* ＩＤ */
		p_le++;
		/* フラグ */
		p_le++;
		/* プリミティブ数 */
		p_le++;
		/* 頂点数 */
		p_le++;
		/* 頂点数２ */
		if(ol_obj->prims != 1)
			p_le++;

		/* ＤＧプリミティブ */
		{
			OL_VERTEX      *ol_vert;
			DG_PRIM2_UVRGB *dst, *src, *uvrgb;

			ol_vert = (OL_VERTEX *)p_le; // BP - This has already been endian swapped!!!11

			DG_SwitchBuffPrim2(ol_obj->dgprims);
			dst = ol_obj->dgprims->uvrgb[ol_obj->dgprims->buffer_clock];
			src = ol_obj->dgprims->uvrgb[1 - ol_obj->dgprims->buffer_clock];

			for(k = 0; k < ol_obj->prims; k++)
			{
				uvrgb = MEM_ADDR2;

				/* 色の設定 */
				sy_memcpy2(MEM_ADDR2, src, sizeof(DG_PRIM2_UVRGB), ol_obj->verts);
				for(j = 0; j < ol_obj->verts; j++)
				{
					if(    (ol_obj->prims == 1    )
						|| (k != ol_obj->prims - 1)
						|| (j < ol_obj->verts2    ))
					{
						if(irange[0] < irange[1])
						{
							if((edge < irange[0]) || (irange[1] < edge))
							{
								// 自分は範囲外
								uvrgb[j].f = 0x8fff;
								if(ol_vert->flag & OLV_FLAG_DRAWEDGE) {edge++;}
							}
							else
							{
								// 自分は範囲内
								if(ol_vert->flag & OLV_FLAG_DRAWEDGE) {uvrgb[j].f = 0x0fff; edge++;}
								else                                  {uvrgb[j].f = 0x8fff;        }
							}
						}
						else
						{
							if((irange[0] < edge) || (edge < irange[1]))
							{
								// 自分は範囲外
								uvrgb[j].f = 0x8fff;
								if(ol_vert->flag & OLV_FLAG_DRAWEDGE) {edge++;}
							}
							else
							{
								// 自分は範囲内
								if(ol_vert->flag & OLV_FLAG_DRAWEDGE) {uvrgb[j].f = 0x0fff; edge++;}
								else                                  {uvrgb[j].f = 0x8fff;        }
							}
						}
						ol_vert++;
					}
				}
				sy_memcpy(dst, MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), ol_obj->verts);

				dst += ol_obj->verts;
				src += ol_obj->verts;
			}

			p_le = (int *)ol_vert;
		}
	}	/* for(i = 0; i < outline->objcount; i++) */
}
#if 0
/*******************************************************************************
 * アウトライン 色スライドモードの設定
 */
void OutlineSetSlideMode(
	OUTLINE *outline,		/* アウトライン */
	int     slide_time)		/* 色配列（４要素） */
{
	SY_PRINTF3("OutlineSetSlideMode\n");

	ASSERT(outline != NULL)

	if(slide_time < 1)
		return;

	outline->flag &= ~OL_FLAG_MODEMASK;
	outline->flag |= OL_FLAG_SLIDE;

	outline->slide_time    = slide_time;
	outline->slide_counter = 0;
}

/*******************************************************************************
 * アウトライン グラデーションモードの設定
 */
void OutlineSetGradMode(
	OUTLINE *outline,	/* アウトライン */
	int     mode,		/* 0:Ｘ方向  1:Ｙ方向  2:Ｚ方向  3:部分（全体）  4:部分タイプ２（間接ごと） */
	float   *range,		/* 座標範囲／％範囲配列（２要素） */
	char    col_num)	/* 色番号 */
{
	SY_PRINTF3("OutlineSetGradMode\n");

	ASSERT(outline != NULL)
	ASSERT(range   != NULL)

	if(    ((mode    < 0) || (4 < mode   ))
		|| ((col_num < 0) || (3 < col_num)))
		return;

	outline->flag &= ~OL_FLAG_MODEMASK;
	switch(mode)
	{
	case 0:
		outline->flag      |= OL_FLAG_XGRAD;
		outline->x_range[0] = range[0];
		outline->x_range[1] = range[1];
		outline->x_col_num  = col_num;
		break;

	case 1:
		outline->flag      |= OL_FLAG_YGRAD;
		outline->y_range[0] = range[0];
		outline->y_range[1] = range[1];
		outline->y_col_num  = col_num;
		break;

	case 2:
		outline->flag      |= OL_FLAG_ZGRAD;
		outline->z_range[0] = range[0];
		outline->z_range[1] = range[1];
		outline->z_col_num  = col_num;
		break;

	case 3:
		outline->flag      |= OL_FLAG_PGRAD;
		outline->p_range[0] = range[0];
		outline->p_range[1] = range[1];
		outline->p_col_num  = col_num;
		break;

	case 4:
		outline->flag      |= OL_FLAG_PGRAD2;
		outline->p_range[0] = range[0];
		outline->p_range[1] = range[1];
		outline->p_col_num  = col_num;
		break;

	default:
		break;
	}
}

/*******************************************************************************
 * アウトライン グラデーションモードの座標範囲／％範囲の設定
 */
void OutlineSetGradRange(
	OUTLINE *outline,	/* アウトライン */
	int     mode,		/* 0:Ｘ方向  1:Ｙ方向  2:Ｚ方向  3:部分（全体）  4:部分タイプ２（間接ごと） */
	float   *range)		/* 座標範囲／％範囲配列（２要素） */
{
	SY_PRINTF3("OutlineSetGradRange\n");

	ASSERT(outline != NULL)
	ASSERT(range   != NULL)

	switch(mode)
	{
	case 0:
		outline->x_range[0] = range[0];
		outline->x_range[1] = range[1];
		break;

	case 1:
		outline->y_range[0] = range[0];
		outline->y_range[1] = range[1];
		break;

	case 2:
		outline->z_range[0] = range[0];
		outline->z_range[1] = range[1];
		break;

	case 3:
		outline->p_range[0] = range[0];
		outline->p_range[1] = range[1];
		break;

	case 4:
		outline->p_range[0] = range[0];
		outline->p_range[1] = range[1];
		break;

	default:
		break;
	}
}
#endif
