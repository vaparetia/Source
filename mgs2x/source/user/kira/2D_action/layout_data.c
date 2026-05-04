//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
layout_data.c
レイアウト/アクションデータの読み込み

2000/01/16    Y.Kira
$Id: layout_data.c,v 1.2 2002/12/20 06:01:01 takaki Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
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

#include "gameheader.h"

#include "font.h"
#include "libfs.h"

#include "sprite_2d.h"  /* 2D モジュールを使用 */


#define _layout_data_c_
#include "layout_malloc.h"

#include "layout_config.h"
#include "layout_data.h"
#include "layout_conv.h"
#include "l_util.h"

#include "BP_EndianSupport.h"

#ifdef PSX2

#ifdef _DEBUG_
#define DBG(args...)  printf(args)
#else
#define DBG(...)
#endif /* _DEBUG_ */

#else

#include <stdarg.h>
#ifdef _DEBUG_
static inline void DBG( char *f, ... )
{
   va_list		argptr ;
   va_start( argptr, format );
   vsprintf( buff, format, argptr );
   va_end( argptr );
}
#else
static void inline DBG( char *f, ... )
{
   va_list		argptr ;
   va_start( argptr, f );
   //vsprintf( buff, f, argptr );
   va_end( argptr );
}
#endif /* _DEBUG_ */

#endif

#define CH 4

typedef unsigned short TType_UShort;
typedef short TType_SShort;
typedef int TType_SInt;
typedef unsigned int TType_UInt;
typedef char TType_char;
typedef float TType_Float;

static inline char BP_LE_Swapchar(char c) { return c; }

#define TYPE_LOOKUP(_type) TType_##_type
#define GetData(_ptr, _type) BP_LE_Swap##_type( *( TYPE_LOOKUP(_type) * )_ptr ), _ptr = (TYPE_LOOKUP(_type) *)_ptr + 1
#define GetDataUnswapped(_ptr, _type)  *(_type *)_ptr,  _ptr = (_type *)_ptr + 1


/* 属性値と 2D オブジェクト種別の対応表 */
static const SPR_ID attrib_list[] = {
   SP_EMPTY,
   SP_POINT,
   SP_LINE,
   SP_LINESTRIP,
   SP_BOX,
   SP_SPRITE,
   SP_ZOOM,
   SP_SPIN,
   SP_POLY,
   SP_TRIANGLE,
};

/* 指定可能な頂点数(LineStrip 以外は固定) */
static const int vertex_nums[] = { 1, 1, 2, -1, 2, 1, 2, 1, 4, 3 };

static int (*convfunc_list[])(l2dSprite *, int, int, float) = {
   lconvEmpty,       /* 空オブジェクト               */
   lconvPoint,       /* 点                           */
   lconvLine,        /* 線分                         */
   lconvLineStrip,   /* 折れ線                       */
   lconvBox,         /* 矩形(中抜き)                 */
   lconvSprite,      /* スプライト(位置、サイズ指定) */
   lconvZoom,        /* スプライト(対角位置指定)     */
   lconvSpin,        /* スプライト(回転)             */
   lconvPoly,        /* スプライト(任意4頂点)        */
   lconvTriangle,    /* スプライト(三角形)           */
};

/* オブジェクト管理領域初期化 */
l2dSprite * l2dSpriteInit(l2dSprite * spr)
{
   spr->id         = -1;     /* オブジェクト ID */
   spr->obj        = NULL;   /* オブジェクトに対応するスプライトのポインタ */
   spr->vertex_num = 0;      /* 操作可能な頂点数                           */
   spr->vertex     = NULL;   /* 頂点領域                                   */

   spr->tex_code   = -1;     /* テクスチャの StrCode                       */
   spr->tex_handle = -1;     /* 対応するテクスチャを 2D オブジェクトモジュールに
                             登録した際のテクスチャハンドル             */

   spr->u          = 0;      /* 初期 U 値                                  */
   spr->v          = 0;      /* 初期 V 値                                  */
   spr->w          = 0;      /* 初期テクセル幅                             */
   spr->h          = 0;      /* 初期テクセル高さ                           */

   spr->width      = 0.0F;   /* 初期表示幅(Sprite 型のみで使用)            */
   spr->height     = 0.0F;   /* 初期表示高(Sprite 型のみで使用)            */

   spr->now_status = -1;     /* 現時点での状態番号                         */
   spr->status_num = 0;      /* オブジェクトに与えられた状態の数           */
   spr->stat       = NULL;   /* オブジェクトに与えられた状態               */

   spr->conv_func  = NULL;   /* 状態間補間関数へのポインタ                 */

   spr->magni      = 1.0F;   /* 下位オブジェクト座標値の係数               */

   return spr;
}


#define VER_MAKE(major, minor, patch) (((major) << 16) + (minor << 8) + (patch))
#define VER_DECODE(ver) (((int)(ver).c_ver[1] << 16) + ((int)(ver).c_ver[2] << 8) + ((int)(ver).c_ver[3]))

/*
* オブジェクト定義セクションの読み込み
*/
l2dData * l2dObjectDefine(l2dData * dat, void * ptr, int chanl)
{
   int obj_nums;
   int i, j;
   int parent, mode, vertex;
   l2dSprite * spr_array;
   SPR_OBJ * parent_obj;
   int ver0_1_0, ver0_1_1;
   SPR_ID sprId;

   /* デバッグモード時にデータバージョン表示 */
   DBG("Ver: %x.%x.%x\n",
      dat->ver.c_ver[1], dat->ver.c_ver[2], dat->ver.c_ver[3]);

   /* 定義個数を取得 */
   obj_nums = GetData( ptr, SInt );
   DBG("Num: %d\n", obj_nums);
   if(NULL == (spr_array = MALLOC(sizeof(l2dSprite) * obj_nums))) return NULL;
   ver0_1_0 = VER_CHECK(&dat->ver, 0, 1, 0); /* Version 0.1.0 以降のフラグ */
   ver0_1_1 = VER_CHECK(&dat->ver, 0, 1, 1); /* Version 0.1.1 以降のフラグ */

   DBG("Ver. 0.1.0 or later [%s]\n", (ver0_1_0) ? "YES" : "NO");
   DBG("Ver. 0.1.1 or later [%s]\n", (ver0_1_1) ? "YES" : "NO");

   for(i = 0; i < obj_nums; i++)
   {
      /* 一旦初期化 */
      l2dSpriteInit(&spr_array[i]);

      spr_array[i].code = -1;

      /* バージョンが 0.1.0 以降であれば、
      ここにオブジェクト識別用 StrCode が入る */
      if(ver0_1_0)
         spr_array[i].code = GetData(ptr, SInt);  /* オブジェクトの StrCode */

      spr_array[i].id = GetData(ptr, SShort); /* ID     */
      parent = GetData(ptr, SShort);          /* 親 ID  */
      mode   = GetData(ptr, char);           /* 属性   */
      vertex = GetData(ptr, char);           /* 頂点数 */
      ptr = (char *)ptr + 2;  /* 未定義領域 */
      if(ver0_1_0)
         ptr = (char *)ptr + 4;

      /* 基準プライオリティ値 */
      spr_array[i].pri_base = dat->base_pri;

      /* プライオリティ補正値(初期値は 0) */
      spr_array[i].pri_adjust = 0;

      /* 親オブジェクトの実体ポインタを得る */
      parent_obj = (parent < 0) ? NULL : spr_array[parent].obj;

      /* 対応する 2D オブジェクトを作成する */
      sprId = attrib_list[mode];
      // adjust if we want pixel perfect (use floats throughout the pipeline)
      if (VER_DECODE(dat->ver) >= VER_MAKE(0,1,3))
      {
         switch (sprId)
         {
         case SP_SPRITE:
            sprId = SP_SPRITE_F;
            break;
         case SP_SPIN:
            sprId = SP_SPIN_F;
            break;
         case SP_ZOOM:
            sprId = SP_ZOOM_F;
            break;
         case SP_BOX:
            sprId = SP_BOX_F;
            break;
         case SP_POLY:
            sprId = SP_POLY_F;
            break;
         }
      }
      spr_array[i].obj = SPR_Create_2D_Object(sprId, chanl, parent_obj);

      // ASSERT(spr_array[i].obj != NULL);
      /* メモリが足りん場合は,それまで確保したものを全て破棄し、
      ハンドルを用意出来なかったものとする。 */
      if(NULL == spr_array[i].obj)
      {
         for(j = i - 1; j >= 0; j--)
         {
            SPR_Destroy_2D_Object(spr_array[j].obj);  /* 2Dobject を破棄 */
            FREE(spr_array[j].vertex);                /* 頂点ワークを破棄 */
         }
         FREE(spr_array);
         return NULL;
      }

      DBG("<1>\n");

      /* 2D オブジェクトのプライオリティ値を設定する。 */
      SPR_SetPriority(spr_array[i].obj, dat->base_pri);

      SPR_HIDE(spr_array[i].obj);  /* 作成直後は非表示にしておく */
      DBG("<2>\n");

      /* オブジェクトの状態間補間関数を設定する */
      spr_array[i].conv_func = convfunc_list[mode];

      /* LineStrip オブジェクトの場合、頂点数を指定する */
      if(attrib_list[mode] == SP_LINESTRIP)
         SPR_SetLineStripVertexNumber(spr_array[i].obj, vertex);
      else
         vertex = vertex_nums[mode];

      DBG("<3>\n");

      /* 内部的な指定可能頂点数を記録 */
      spr_array[i].vertex_num = vertex;

      /* 指定可能頂点数分の頂点ワークを確保 */
      spr_array[i].vertex = MALLOC(sizeof(l2dVertex) * vertex);

      // ASSERT(NULL != spr_array[i].vertex);
      if(NULL == spr_array[i].vertex)
      {
         SPR_Destroy_2D_Object(spr_array[i].obj);
         for(j = i - 1; j >= 0; j--)
         {
            SPR_Destroy_2D_Object(spr_array[j].obj);  /* 2Dobject を破棄 */
            FREE(spr_array[j].vertex);                /* 頂点ワークを破棄 */
         }
         FREE(spr_array);
         return NULL;
      }

      DBG("<4>\n");

      /* 全ての頂点の座標および RGBA 値を指定されていないことにする */
      for(j = 0; j < vertex; j++)
      {
         spr_array[i].vertex[j].vert = 0;
         spr_array[i].vertex[j].rgba = 0;
      }

      /* 現時点では、このオブジェクトに与えられた状態は無いことにする */
      spr_array[i].status_num = 0;
      spr_array[i].stat       = NULL;
   }

   dat->obj_nums = obj_nums;
   dat->obj      = spr_array;

   return dat;  /* 作成されたオブジェクト管理領域を返す */
}

/*
* 状態定義セクションの読み込み
*/
l2dData * l2dObjectStatus(l2dData * dat, void * ptr, float safeZoneOffsetY)
{
   l2dStatus * stat_array;
   int         stat_nums;
   int         i, j;
   int         id, stat_cnt;
   int         tri_strcode;
   int         tri_handle = -1;
   int         vertex_num;
   int         ver0_1_2;
   float       ws_scale_x, ws_scale_y;
   int ver0_1_3 = VER_MAKE(0, 1, 3);
   int ver = VER_DECODE(dat->ver);

   ver0_1_2 = VER_CHECK(&dat->ver, 0, 1, 2); /* Version 0.1.2 以降のフラグ */
   DBG("Ver. 0.1.2 or later [%s]\n", (ver0_1_2) ? "YES" : "NO");


   // scale to 512x400 again
   ws_scale_x = 512.0f / dat->xsize;
   ws_scale_y = 400.0f / dat->ysize;

   /* 状態を定義されているオブジェクトの数を取得 */
   stat_nums = GetData(ptr, SInt);

   /* 各オブジェクトの状態を取得 */
   for(i = 0; i < stat_nums; i++)
   {
      id       = GetData(ptr, SShort); /* 対象 ObjID */
      stat_cnt = GetData(ptr, SShort); /* 保持状態数 */

      /* ID, 状態数ともに負の値であるならば、使用 tri の指定とする */
      /* エラーが起こっている場合は、追加しても無駄なので、追加しない */
      if((id < 0) && (stat_cnt < 0))
      {
         /* それ以後のテクスチャ指定で使用される tri の StrCode を切替える */
         tri_strcode = GetData(ptr, SInt);
         tri_handle  = SPR_LoadTexture(tri_strcode & 0xffffff);

#ifdef DEBUG_MODE
         if(tri_handle < 0)
            printf("<L2D> Warning: TRI is full.\n");
#endif /* DEBUG_MODE */

         /* テクスチャがロード出来ないと正常に表示されないのでエラーとする*/
         if(tri_handle < 0) return NULL;

         /*
         * 新規登録テクスチャリストに、取得したテクスチャハンドルを追加する
         */
         if((tri_handle >= 0) && (dat->texture_cnt < MAX_TEXTURES))
         {
            dat->tex_handles[ dat->texture_cnt ] = tri_handle;
            dat->texture_cnt++;  /* テクスチャ数の追加 */
         }
         continue;
      }

      vertex_num = dat->obj[id].vertex_num;
      stat_array = MALLOC(sizeof(l2dStatus) * stat_cnt);
      // ASSERT(stat_array != NULL);
      if(NULL == stat_array) 
      {
         /* そのオブジェクトの状態定義を一切無しにする */
         dat->obj[id].status_num = 0;
         dat->obj[id].stat       = NULL;
         return NULL;
      }

      /* 状態定義の取得 */
      for(j = 0; j < stat_cnt; j++)
      {
         unsigned int cmd;
         l2dStatus * stat;
         int vn;

         stat = stat_array + j;

         /* 状態が保持する頂点設定情報領域の確保 */
         stat->vertex = MALLOC(sizeof(l2dVertex) * vertex_num);
         // ASSERT(stat->vertex != NULL);
         if(NULL == stat->vertex)
         {
            int k;

            /* それまで確保した頂点設定情報領域を全て開放 */
            for(k = 0; k < j; k++) FREE(stat_array[k].vertex);

            /* 状態保持領域を無かったことにする */
            FREE(stat_array);

            /* 登録する値を,「状態無し」にする。 */
            dat->obj[id].status_num = 0;
            dat->obj[id].stat       = NULL;
            return NULL;
         }

         for(vn = 0; vn < vertex_num; vn++)
         {
            stat->vertex[vn].vert = 0;
            stat->vertex[vn].rgba = 0;
         }

         /* 状態名 StrCode を取得 */
         stat->code = GetData(ptr, SInt);

         stat->tex_code = -1;
         stat->tex_handle = -1;
         stat->alpha_flg = 0;
         stat->alpha = 0;
         stat->pos_u = -1.0F;
         stat->pos_v = -1.0F;
         stat->size_u = -1.0F;
         stat->size_v = -1.0F;
         stat->magni = 1.0F;
         stat->pri_adj = 0;

         /* 状態定義シーケンスを読み込み、順次状態を設定する */
         while(cmd = GetData(ptr, SInt), cmd & STAT_CMD_MASK)
            switch(cmd & STAT_CMD_MASK)
         {
            case STAT_CMD_DISP:   /* 表示状態指定         */
               stat->disp = cmd & 1;
               stat->h_rev = (cmd >> 1) & 1;  /* 水平反転 */
               stat->v_rev = (cmd >> 2) & 1;  /* 垂直反転 */
               break;
            case STAT_CMD_TEX:    /* 使用テクスチャ指定   */
               stat->tex_code = cmd & 0xffffff;
               stat->tex_handle = tri_handle;
               /* まだ UV 値が指定されていなければ、デフォルトとして
               (0.0F, 0.0F)-(1.0F, 1.0F) の UV 値を指定する */
               if(stat->pos_u < 0.0F)
                  stat->pos_u = stat->pos_v = 0.0F;
               if(stat->size_u < 0.0F)
                  stat->size_u = stat->size_v = 1.0F;
               break;
            case STAT_CMD_VERTEX: /* 頂点座標値指定       */
               {
                  int begin, num, k;
                  unsigned short x, y;
                  float fx, fy;

                  begin = cmd & 0xff;
                  num   = (cmd >> 8) & 0xff;
                  for(k = 0; k < num; k++)
                  {
                     if (ver < ver0_1_3)
                     {
                        x = GetData(ptr, UShort);
                        y = GetData(ptr, UShort);

                        /* 取得した座標値は 16bit 浮動小数点値であるため、
                        float 型への変換を行ったものを使用する。 */

                        fx = lutil_short_to_float(x);
                        fy = lutil_short_to_float(y);
                     }
                     else
                     {
                        fx = GetData(ptr, Float);
                        fy = GetData(ptr, Float);
                     }


                     /* 座標配列に格納し、
                     その座標値が指定されているというフラグを設定 */
                     stat->vertex[begin + k].x = fx * ws_scale_x;
                     stat->vertex[begin + k].y = fy * ws_scale_y;
                     stat->vertex[begin + k].y = stat->vertex[begin + k].y + (safeZoneOffsetY);          //BP JG - add an offset to certain screens.. (mainly front end) to avoid safezone issues.

                     stat->vertex[begin + k].vert = 1;
                  }
               }
               break;
            case STAT_CMD_RGBA:   /* 頂点 RGBA値指定      */
               {
                  int begin, num, k;
                  unsigned int tmp;

                  begin = cmd & 0xff;
                  num   = (cmd >> 8) & 0xff;
                  for(k = 0; k < num; k++)
                  {
                     tmp = GetData(ptr, UInt);

                     /* 座標配列に格納し、
                     その座標値が指定されているというフラグを設定 */
                     stat->vertex[begin + k].r =  tmp        & 0xff;
                     stat->vertex[begin + k].g = (tmp >> 8)  & 0xff;
                     stat->vertex[begin + k].b = (tmp >> 16) & 0xff;
                     stat->vertex[begin + k].a = (tmp >> 24) & 0xff;
                     stat->vertex[begin + k].rgba = 1;
                  }
               }
               break;
            case STAT_CMD_SIZE:   /* 表示サイズ指定       */
               {
                  float fw, fh;
                  if (ver < ver0_1_3)
                  {
                     unsigned short width, height;

                     width  = GetData(ptr, UShort);
                     height = GetData(ptr, UShort);

                     /* 取得した表示サイズは 16bit 浮動小数点値であるため、
                     float 型に変換する */
                     fw = lutil_short_to_float(width);
                     fh = lutil_short_to_float(height);
                  }
                  else
                  {
                     fw = GetData(ptr, Float);
                     fh = GetData(ptr, Float);
                  }

                  stat->size_w = fw * ws_scale_x;
                  stat->size_h = fh * ws_scale_y;
               }
               break;
            case STAT_CMD_UV:     /* UV値指定             */
               {
                  unsigned short u, v;
                  float fu, fv;

                  u = GetData(ptr, UShort);
                  v = GetData(ptr, UShort);

                  /* 取得した表示サイズは 16bit 浮動小数点値であるため、
                  float 型に変換する */
                  fu = lutil_short_to_float(u);
                  fv = lutil_short_to_float(v);
                  DBG("\tUV:(%8.3f, %8.3f\n", fu, fv);
                  stat->pos_u = fu;
                  stat->pos_v = fv;
               }
               break; 
            case STAT_CMD_UVSIZE: /* テクスチャサイズ指定 */
               {
                  unsigned short width, height;
                  float fw, fh;

                  width  = GetData(ptr, UShort);
                  height = GetData(ptr, UShort);

                  /* 取得した表示サイズは 16bit 浮動小数点値であるため、
                  float 型に変換する */
                  fw = lutil_short_to_float(width);
                  fh = lutil_short_to_float(height);

                  DBG("\tUVsize:(%8.3f, %8.3f\n", fw, fh);
                  stat->size_u = fw;
                  stat->size_v = fh;
               }
               break;
            case STAT_CMD_ALPHA:
               stat->alpha_flg = 1;
               stat->alpha = (cmd & 0xffff) | ((cmd & 0xff0000) << 16);
               break;
            case STAT_CMD_PRI:
               stat->pri_adj = cmd & 7;  /* プライオリティ補正値 */
               break;
            case STAT_CMD_CENTER:
               {
                  float fcx, fcy;
                  if (ver < ver0_1_3)
                  {
                     unsigned short scx, scy;
                     scx = GetData(ptr, UShort);
                     scy = GetData(ptr, UShort);

                     fcx = lutil_short_to_float(scx) * ws_scale_x;
                     fcy = lutil_short_to_float(scy) * ws_scale_y;
                  }
                  else
                  {
                     fcx = GetData(ptr, Float);
                     fcy = GetData(ptr, Float);
                  }
                  stat->center_x = fcx;
                  stat->center_y = fcy;
               }
               break;
            case STAT_CMD_ANGLE:
               {
                  unsigned short sang;
                  sang = cmd & 0xffff;
                  stat->angle = lutil_short_to_float(sang);
               }
               break;
            case STAT_CMD_MAGNI:
               {
                  unsigned short magni;
                  magni = cmd & 0xffff;
                  stat->magni = lutil_short_to_float(magni);
               }
               break;
            case STAT_CMD_PARTUV:   /* 分数指定 UV 値 */
               {
                  unsigned int Vv;

                  Vv = GetData(ptr, UInt);

                  stat->pos_u =
                     (float)((cmd >> 16) & 0x0fff) / (float)(cmd & 0x0fff);
                  stat->pos_v =
                     (float)((Vv >> 16) & 0x0fff) / (float)(Vv & 0x0fff);
               }
               break;
            case STAT_CMD_PTXSIZ:   /* 分数指定テクスチャサイズ */
               {
                  unsigned int Vv;

                  Vv = GetData(ptr, UInt);

                  stat->size_u =
                     (float)((cmd >> 16) & 0x0fff) / (float)(cmd & 0x0fff);
                  stat->size_v =
                     (float)((Vv >> 16) & 0x0fff) / (float)(Vv & 0x0fff);
               }
               break;
         }
      }



      dat->obj[id].status_num = stat_cnt;
      dat->obj[id].stat       = stat_array;
   }
   return dat;
}

/*
* アクション定義セクションを読み込む
*/
l2dData * l2dObjectAction(l2dData * dat, void * ptr)
{
   l2dAction * act_array;
   l2dAction * act;
   int         act_nums;
   int         i, j;


   act_nums = GetData(ptr, SInt);
   act_array = MALLOC(sizeof(l2dAction) * act_nums);
   // ASSERT(NULL != act_array);
   if(NULL == act_array)
   {
      dat->act_nums = 0;
      dat->act = NULL;
      return NULL;
   }

   for(i = 0; i < act_nums; i++)
   {
      l2dTrack * track;
      int id, step_nums;

      act = act_array + i;

      /* アクションデータ識別用 StrCode を得る */
      act->code       = GetData(ptr, SInt);
      act->tracks_num = GetData(ptr, SInt);

      /* トラックの数だけトラック構造体を確保する。
      トラック構造体は、再生中にワークエリアとして使用される。*/
      act->tracks = MALLOC(sizeof(l2dTrack) * act->tracks_num);
      //ASSERT(NULL != act->tracks);
      if(NULL == act->tracks)
      {
         int k;

         /* これまで確保した全てのアクションを開放する */
         for(k = 0; k < i; k++)
         {
            act = act_array + k;
            FREE(act->tracks);
         }
         FREE(act_array);
         dat->act_nums = 0;
         dat->act = NULL;
         return NULL;
      }

      /* 各トラック構造体を初期化する */
      for(j = 0; j < act->tracks_num; j++)
      {
         track = act->tracks + j;

         id        = GetData(ptr, SShort);
         step_nums = GetData(ptr, SShort);

         track->target = dat->obj + id;

         /* 現在のポインタ位置をそのままシーケンス位置とする */

         track->steps_le = ptr;

         track->pc = 0;
         track->cmd = 0;
         track->form_old = track->form_new = -1;
         track->time_limit = track->time_cnt = 0;

         /* トラック情報が終了するまで読み飛ばす */
         {
            unsigned int cmd, stat;
            do {
               cmd = GetData(ptr, UInt);
               switch(cmd & ACT_CMD_MASK)
               {
               case ACT_CMD_MORF:
               case ACT_CMD_SIGN:
                  stat = GetData(ptr, UInt);
                  break;
               default:
                  break;
               }
            } while((cmd & ACT_CMD_MASK) != ACT_CMD_END);
         }
      }
   }
   dat->act      = act_array;
   dat->act_nums = act_nums;
   return dat;
}

l2dData * l2dAdditionalFlag(l2dData * dat, int add_flag)
{
   int i;

   if(NULL == dat) return NULL;
   for(i = 0; i < dat->obj_nums; i++)
      dat->obj[i].obj->head.flags |= add_flag;
   dat->additional_flag = add_flag;
   return dat;
}


l2dData * l2dSetupData(void * entry_ptr, int chanl, int base_pri, int add_flag, float safeZoneOffsetY)
{
   l2dData * dat;
   void * section_def;   /* オブジェクト定義セクション開始ポインタ */
   void * section_stat;  /* 状態定義セクション開始ポインタ         */
   void * section_act;   /* アクション定義セクション開始ポインタ   */
   void * ptr;
   unsigned int def, stat, act;

   if(NULL == (dat = MALLOC(sizeof(l2dData)))) return NULL;
   dat->obj_nums = 0;
   dat->obj      = NULL;
   dat->act_nums = 0;
   dat->act      = NULL;
   dat->texture_cnt = 0;
   ptr = entry_ptr;

   // BP - GetDataUnswapped works here because l2dVersion is a char array
   dat->ver  = GetDataUnswapped(ptr, l2dVersion);  /* バージョン情報                 */
   dat->base_pri = base_pri;          /* 基準プライオリティ値               */

   if (VER_DECODE(dat->ver) >= VER_MAKE(0,1,3))
   {
      dat->xsize = (float)GetData(ptr, UInt);
      dat->ysize = (float)GetData(ptr, UInt);
   }

   def = GetData(ptr, UInt); /* オブジェクト定義セクションアドレス */

   dat->widescreen = def & OLA_VERSION_HD;
   def = def & ~OLA_VERSION_HD;

   if (VER_DECODE(dat->ver) < VER_MAKE(0, 1, 3))
   {
      if (dat->widescreen > 0)
      {
         dat->xsize = 1280.0f;
         dat->ysize = 720.0f;
      }
      else
      {
         dat->xsize = 512.0f;
         dat->ysize = 400.0f;
      }
   }

   stat = GetData(ptr, UInt); /* 状態定義セクションアドレス         */
   act  = GetData(ptr, UInt); /* アクション定義セクションアドレス   */


   /* 各セクションの開始アドレスを得る */
   section_def  = (void *)((unsigned char *)entry_ptr + def);
   section_stat = (void *)((unsigned char *)entry_ptr + stat);
   section_act  = (void *)((unsigned char *)entry_ptr + act);

   /* オブジェクト定義セクションの読み込み */
   if(NULL == l2dObjectDefine(dat, section_def, chanl))
   {
      l2dFreeData(dat);
      return NULL;
   }


   /* 状態定義セクションの読み込み */
   if(NULL == l2dObjectStatus(dat, section_stat, safeZoneOffsetY))
   {
      l2dFreeData(dat);
      return NULL;
   }

   /* アクション定義セクションの読み込み */
   if(NULL == l2dObjectAction(dat, section_act))
   {
      l2dFreeData(dat);
      return NULL;
   }

   /* 追加フラグを表示オブジェクト全てに追加 */
   l2dAdditionalFlag(dat, add_flag);

   return dat;
}

/*
* 領域の開放
*/
void l2dFreeData(l2dData * dat)
{
   int i, j;
   l2dAction * act;
   l2dSprite * spr;

#ifdef KP_WINDOWS
   DG_ExeBufferSync(FALSE) ;	// 安全策
#endif

   /* アクション領域開放 */
   DBG("act_nums = %d\n", dat->act_nums);
   for(i = 0; i < dat->act_nums; i++)
   {
      act = dat->act + i;
      FREE(act->tracks);
   }
   if(NULL != dat->act) FREE(dat->act);

   /*
   * このモジュールで登録したテクスチャを開放する。
   */
   DBG("texture_cnt = %d\n", dat->texture_cnt);
   for(i = 0; i < dat->texture_cnt; i++)
      SPR_KillTexture(dat->tex_handles[i]);

   /*
   * オブジェクト領域開放
   * オブジェクトは親を開放すると、その子も自動的に開放されてしまう。
   * 開放済の領域を再度開放しないために、後から登録したものを優先的に開放する
   */
   for(i = dat->obj_nums - 1; i >= 0; i--)
   {
      spr = dat->obj + i;
      SPR_Destroy_2D_Object(spr->obj);  /* 2D オブジェクト実体の破棄 */
      FREE(spr->vertex);                /* 座標エリアの破棄 */
      for(j = 0; j < spr->status_num; j++) FREE(spr->stat[j].vertex);
      if(NULL != spr->stat)FREE(spr->stat);
   }
   if(NULL != dat->obj) FREE(dat->obj);

   /* l2dData そのものの開放 */
   FREE(dat);
}

/*
* 該当する StrCode を持つ l2dSprite 構造体のポインタを得る
*/
void * l2dGetParts(l2dData * dat, int code)
{
   int i;

   for(i = 0; i < dat->obj_nums; i++) {
      if(dat->obj[i].code == code) return &(dat->obj[i]);
   }
#ifdef DEBUG_MODE
   if (code != -1 )
      printf( "layout_data.c : parts not found for <%d>.\n", code ) ;
#endif
   return NULL;
}

/*
* 該当する StrCode を持つ 2D オブジェクトのポインタを取得する
*/
SPR_OBJ * l2dGetObject(l2dData * dat, int code)
{
   l2dSprite * spr;

   if(NULL == (spr = l2dGetParts(dat, code))) {
      return NULL;
   }
   return spr->obj;
}
