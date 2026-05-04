#ifndef _node_bg_h_
#define _node_bg_h_

/*
 * ノード背景の表示チャネル
 */
#define NODE_BG_CHANL  3

/*
 * ノード表示中の Fog 色
 */
#define NODE_FOG_R  22
#define NODE_FOG_G  26
#define NODE_FOG_B  24

/*
 * 背景の表示基底プライオリティ
 */
#define NODE_BG_PRI 1


/*
 * 各パーツの名称
 */
#define NODE_BG_L2D_log			0x0001be47		/* log */
#define NODE_BG_L2D_scroll		0x006acad8		/* scroll */
#define NODE_BG_L2D_ny_map		0x00c14173		/* ny_map */
#define NODE_BG_L2D_chm1		0x00332dd1		/* chm1 */
#define NODE_BG_L2D_chm2		0x00332dd2		/* chm2 */
#define NODE_BG_L2D_chm3		0x00332dd3		/* chm3 */
#define NODE_BG_L2D_ny_map_bg	0x00bb6947		/* ny_map_bg */
#define NODE_BG_L2D_fog1		0x0034c911		/* fog1 */
#define NODE_BG_L2D_fog2		0x0034c912		/* fog2 */
#define NODE_BG_L2D_fog3		0x0034c913		/* fog3 */
#define NODE_BG_L2D_grd			0x0001aaa4		/* grd */
#define NODE_BG_L2D_plant_map	0x00e4b9b3		/* plant_map */
#define NODE_BG_L2D_ray_wall	0x0030bb24		/* ray_wall */
#define NODE_BG_L2D_loop_wall	0x00a2a692		/* roop_wall */
#define NODE_BG_L2D_genzi		0x00a447af		/* genzi */


/*
 * 各パーツの基底プライオリティ値
 */
#define NODE_BG_PRI_BACK	  0   /* 0:背景               */
#define NODE_BG_PRI_NYMAP_BG  1   /* 1:NY マップ BG       */
#define NODE_BG_PRI_NYMAP     2   /* 2:NY マップ          */
#define NODE_BG_PRI_FOG		  3   /* 3:フォグ             */
#define NODE_BG_PRI_SCROLL    1   /* 3:スクロールグリッド */
#define NODE_BG_PRI_LOG       3   /* 3:ログ表示           */
#define NODE_BG_PRI_PLMAP	  3   /* 3:プラントマップ     */
#define NODE_BG_PRI_RAY		  3   /* 3:MG Ray             */
#define NODE_BG_PRI_CHM       4   /* 4:ベンゼン君         */
#define NODE_BG_PRI_WALL	  5   /* 5:ループ壁紙         */
#define NODE_BG_PRI_GRD		  2	  /* 6:シネマスコープ     */
#define NODE_BG_PRI_FADE	  7   /* 7:フェード用         */
#define NODE_BG_PRI_GENZI	  3   /* 3:源氏紋             */


/*
 * 抽象的な移動方向を指示するシグナル
 */
#define NODE_BG_SIGNAL_RIGHT      0x0001    /* 右   */
#define NODE_BG_SIGNAL_LEFT       0x0002    /* 左   */
#define NODE_BG_SIGNAL_UPPER      0x0004    /* 上   */
#define NODE_BG_SIGNAL_LOWER      0x0008    /* 下   */
#define NODE_BG_SIGNAL_RIGHTUP    0x0005    /* 右上 */
#define NODE_BG_SIGNAL_LEFTDOWN   0x000a    /* 左下 */
#define NODE_BG_SIGNAL_STOP		  0x0080    /* 移動停止 */

#define NODE_BG_SIGNAL_OPENGRD	  0x0090	/* シネスコ開始 */
#define NODE_BG_SIGNAL_CLOSEGRD	  0x0091	/* シネスコ終了 */

#define NODE_BG_SIGNAL_OPENPLMAP  0x00a0	/* プラントマップの表示 */
#define NODE_BG_SIGNAL_CLOSEPLMAP 0x00a1	/* プラントマップの消去 */

#define NODE_BG_SIGNAL_OPENRAY    0x00b0	/* MGレイの表示 */
#define NODE_BG_SIGNAL_CLOSERAY   0x00b1	/* MGレイの消去 */

#define NODE_BG_SIGNAL_THUNDER	  0x00c0	/* 雷鳴 */
#define NODE_BG_SIGNAL_CHGFACE	  0x00c1	/* タイトルの顔変え */

/*
 * 源氏紋の表示
 */
#define NODE_BG_SIGNAL_GZ_SHOW_MAIN    0x00d0   /* showGenziMain */
#define NODE_BG_SIGNAL_GZ_SHOW_OPTION  0x00d1   /* showGenziOpt  */
#define NODE_BG_SIGNAL_GZ_SHOW_NEWGAME 0x00d2   /* showGenziNew  */
#define NODE_BG_SIGNAL_GZ_SHOW_DIF     0x00d3   /* showGenziDif  */
#define NODE_BG_SIGNAL_GZ_SHOW_QUEST   0x00d4   /* showGenziQst  */
#define NODE_BG_SIGNAL_GZ_HIDE_MAIN    0x00d5   /* hideGenziMain */
#define NODE_BG_SIGNAL_GZ_HIDE_OPTION  0x00d6   /* hideGenziOpt  */
#define NODE_BG_SIGNAL_GZ_HIDE_NEWGAME 0x00d7   /* hideGenziNew  */
#define NODE_BG_SIGNAL_GZ_HIDE_DIF     0x00d8   /* hideGenziDif  */
#define NODE_BG_SIGNAL_GZ_HIDE_QUEST   0x00d9   /* hideGenziQst  */




/* ノード背景の終了を命じるシグナル */
#define NODE_BG_SIGNAL_KILL       0x0010    /* 強制終了 */

/* ノード背景が、自分が死んだことを親に通知するシグナル */
#define NODE_BG_SIGNAL_DEAD       0x0011


/* どの l2d にも存在する "DefaultAction" の strcode */
#define NODE_BG_DefaultAction   0x0008a3fb

/* どの l2d にも存在する "ROOT" の strcode */
#define NODE_BG_ROOT            0x002a4634


void * NewNodeBG_P(void);


#endif /* _node_bg_h_ */
