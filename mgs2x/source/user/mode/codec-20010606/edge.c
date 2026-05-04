/*
 * 
 * edge.c
 * 顔表示窓の縁を段階的にアルファブレンディングして
 * ぼかすためのパケットを生成する
 *
 * 2000/11/29   Y.Kira
 *
 * $Id: edge.c,v 1.1.1.3 2002/11/19 11:45:07 Yoshizawa1 Exp $
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include "gameheader.h"
#include "codec.h"

#include "font.h"
#include "cjimaku.h"
#include "libfs.h"
#include "libdg.h"
#include "libmt.h"
#include "libutl.h"
#include "dmapack.h"
#include "def_dma.h"

#define _edge_c_
#include "edge.h"

int  edgeInitFaceEdgePacket(FaceEdge * packet, int chanl, u_long tex0,
			    int x_base, int y_base,
			    int u_base, int v_base,
			    int x0, int y0, int x1, int y1, int z)
{
  int i, alpha;
  int X0, Y0, X1, Y1;
  u_long test, zbuf;

  test = *(u_long *)&DG_Chanls[chanl].draw_env[0].datas.test1;
  zbuf = *(u_long *)&DG_Chanls[chanl].draw_env[0].datas.zbuf1;

  /* GIFtag */
  packet->giftag.tag = SCE_GIF_SET_TAG(Qsize(FaceEdge), 1, 0, 0, 0, 1);
  packet->giftag.regs = GS_REGS_AD;

  /* パケット本体の初期化 */
  /* 最初の TEST_1 レジスタは、常にデプステストが成功するように設定 */
  packet->data.test0.reg = SCE_GS_TEST_1;
  packet->data.test0.data = SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1);

  packet->data.zbuf0.reg = SCE_GS_ZBUF_1;
  packet->data.zbuf0.reg = zbuf & ~I64(0x100000000);


  packet->data.test1.reg = SCE_GS_TEST_1;
  packet->data.test1.data = test;

  packet->data.zbuf1.reg = SCE_GS_ZBUF_1;
  packet->data.zbuf1.reg = zbuf;


  packet->data.tex0.reg = SCE_GS_TEX0_1;
  packet->data.tex0.data = tex0;

  packet->data.spr_prim.reg = SCE_GS_PRIM;
  packet->data.spr_prim.data =
    SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,
		    0, /* IIP: Flat Shading           */
		    1, /* TME: Texture Mapping  [ON]  */
		    0, /* FGE: Fogging          [OFF] */
		    1, /* ABE: Alpha Blending   [ON]  */
		    1, /* AA1: 1pass anti alias [OFF] */
		    1, /* FST: UV mapping             */
		    0, /* CTXT: Context 1             */
		    0);/* FIX: Flagment Fix     [OFF] */

  packet->data.rgbq.reg = SCE_GS_RGBAQ;
  packet->data.rgbq.data =
    SCE_GS_SET_RGBAQ(EDGE_R, EDGE_G, EDGE_B, EDGE_BASE_ALPHA, 0);

  packet->data.xyz0.reg = SCE_GS_XYZ2;
  packet->data.xyz0.data = SCE_GS_SET_XYZ((x_base + x0) * 16,
					  (y_base + y0) * 16, z);
  packet->data.xyz1.reg = SCE_GS_XYZ2;
  packet->data.xyz1.data = SCE_GS_SET_XYZ((x_base + x1) * 16,
					  (y_base + y1) * 16, z);
  
  packet->data.uv0.reg = SCE_GS_UV;
  packet->data.uv0.data = SCE_GS_SET_UV((u_base + x0) * 16,(v_base + y0) * 16);

  packet->data.uv1.reg = SCE_GS_UV;
  packet->data.uv1.data = SCE_GS_SET_UV((u_base + x1) * 16,(v_base + y1) * 16);


  /* 縁のぼかし部分 */
  packet->data.box_prim.reg = SCE_GS_PRIM;
  packet->data.box_prim.data =
    SCE_GS_SET_PRIM(SCE_GS_PRIM_LINESTRIP,
		    0, /* IIP: Flat Shading           */
		    1, /* TME: Texture Mapping  [ON]  */
		    0, /* FGE: Fogging          [OFF] */
		    1, /* ABE: Alpha Blending   [ON]  */
		    0, /* AA1: 1pass anti alias [OFF] */
		    1, /* FST: UV mapping             */
		    0, /* CTXT: Context 1             */
		    0);/* FIX: Flagment Fix     [OFF] */

  for(i = 0; i < EDGE_DEPTH; i++)
    {
      X0 = x0 + i, Y0 = y0 + i;
      X1 = x1 - i, Y1 = y1 - i;
      alpha = EDGE_BASE_ALPHA * i / EDGE_DEPTH;

      /* alpha を段階的に可変させる。*/
      packet->data.box[i].rgbq.reg = SCE_GS_RGBAQ;
      packet->data.box[i].rgbq.data =
	SCE_GS_SET_RGBAQ(EDGE_R, EDGE_G, EDGE_B, alpha, 0);

      packet->data.box[i].texa.reg = SCE_GS_TEXA;
      packet->data.box[i].texa.reg = alpha;
      

      /* 頂点座標の設定 */
      packet->data.box[i].xyz0.reg = SCE_GS_XYZ3;
      packet->data.box[i].xyz1.reg = SCE_GS_XYZ2;
      packet->data.box[i].xyz2.reg = SCE_GS_XYZ2;
      packet->data.box[i].xyz3.reg = SCE_GS_XYZ2;
      packet->data.box[i].xyz4.reg = SCE_GS_XYZ2;

      packet->data.box[i].xyz0.data = SCE_GS_SET_XYZ((x_base + X0) * 16,
						     (y_base + Y0) * 16, z);
      packet->data.box[i].xyz1.data = SCE_GS_SET_XYZ((x_base + X1) * 16,
						     (y_base + Y0) * 16, z);
      packet->data.box[i].xyz2.data = SCE_GS_SET_XYZ((x_base + X1) * 16,
						     (y_base + Y1) * 16, z);
      packet->data.box[i].xyz3.data = SCE_GS_SET_XYZ((x_base + X0) * 16,
						     (y_base + Y1) * 16, z);
      packet->data.box[i].xyz4.data = SCE_GS_SET_XYZ((x_base + X0) * 16,
						     (y_base + Y0) * 16, z);

      /* UV 値の設定 */
      packet->data.box[i].uv0.reg = SCE_GS_UV;
      packet->data.box[i].uv1.reg = SCE_GS_UV;
      packet->data.box[i].uv2.reg = SCE_GS_UV;
      packet->data.box[i].uv3.reg = SCE_GS_UV;
      packet->data.box[i].uv4.reg = SCE_GS_UV;
      
      packet->data.box[i].uv0.data = SCE_GS_SET_UV((u_base + X0) * 16,
						   (v_base + Y0) * 16);
      packet->data.box[i].uv1.data = SCE_GS_SET_UV((u_base + X1) * 16,
						   (v_base + Y0) * 16);
      packet->data.box[i].uv2.data = SCE_GS_SET_UV((u_base + X1) * 16,
						   (v_base + Y1) * 16);
      packet->data.box[i].uv3.data = SCE_GS_SET_UV((u_base + X0) * 16,
						   (v_base + Y1) * 16);
      packet->data.box[i].uv4.data = SCE_GS_SET_UV((u_base + X0) * 16,
						   (v_base + Y0) * 16);
    }
  
  return 0;
}
