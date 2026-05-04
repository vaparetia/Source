#ifndef _map_3d_h_
#define _map_3d_h_

#define MAP_CHANL  2   /* 全体マップを表示する描画チャネル */

#define MAP_ATTRIB  (DG_FLAG_ONEPIECE )//| DG_FLAG_SHADE)
#define MAP_DG_FLAG ( DG_FLAG_INVISIBLE | DG_FLAG_INVISIBLE3 | MAP_ATTRIB )


#define PAUSE_KEY   PAD_STA   /* ポーズ状態に入るキー */

#define MAP_OBJ_X     300000.0F
#define MAP_OBJ_Y     300000.0F
#define MAP_OBJ_Z     -300000.0F

#define MAP_LOOKUP_X      ( -700.0F + MAP_OBJ_X )
#define MAP_LOOKUP_Y      ( -200.0F + MAP_OBJ_Y )
#define MAP_LOOKUP_Z      ( 0.0F + MAP_OBJ_Z )

#define MAP_LOOKUP_X_TANKER      (9600.0F)
#define MAP_LOOKUP_Y_TANKER      (5000.0F)
#define MAP_LOOKUP_Z_TANKER      (0.0F)

#define MAP_CAMERA_X      ( -700.0F + MAP_OBJ_X )
#define MAP_CAMERA_Y      ( 14800.0F + MAP_OBJ_Y )

#ifdef PAL
#define MAP_CAMERA_Z      ( 21000.0F + MAP_OBJ_Z )
#else
#define MAP_CAMERA_Z      ( 20000.0F + MAP_OBJ_Z )
#endif

#define MAP_CAMERA_X_TANKER      (9600.0F)
#define MAP_CAMERA_Y_TANKER      (5000.0F)
#define MAP_CAMERA_Z_TANKER      (20000.0F)

#define MAP_CAMERA_ZOOM_TANKER   (1.2F)
#define MAP_CAMERA_ZOOM          (2.5F)

#define MAP_FADE_TIME     ( DIRECT_TICK( 75 ) )
#define MAP_FADE_MAX      104
#define MAP_ROT_TIME      1500

#define MAP_LIGHT_X       -1.0F
#define MAP_LIGHT_Y       -5.0F
#define MAP_LIGHT_Z       -1.0F

#define MAP_LIGHT_X_TANKER     0.0F
#define MAP_LIGHT_Y_TANKER    -5.0F
#define MAP_LIGHT_Z_TANKER     1.0F

/*
 * 主光源色
 */
/* 主光源色(共通) */
#define MAP_COLOR_R  (90)
#define MAP_COLOR_G  (120)
#define MAP_COLOR_B  (24)

#define MAP_COLOR_R_TANKER  (90)
#define MAP_COLOR_G_TANKER  (120)
#define MAP_COLOR_B_TANKER  (24)

/* ノーマルな状態のアンビエント */
#define MAP_AMB_ACT_R  (80)
#define MAP_AMB_ACT_G  (90)
#define MAP_AMB_ACT_B  (88)

/* 破壊されたエリアのアンビエント */
#define MAP_AMB_BRK_R  (0)
#define MAP_AMB_BRK_G  (0)
#define MAP_AMB_BRK_B  (32)

/* プレイヤのいるエリアのアンビエント */
#define MAP_AMB_PLY_R  (178)
#define MAP_AMB_PLY_G  (8)
#define MAP_AMB_PLY_B  (58)

// 選択中のもの
#define MAP_AMB_SEL_R  (10)
#define MAP_AMB_SEL_G  (120)
#define MAP_AMB_SEL_B  (235)

#define MAP_LIGHT_ANIM_TIME ( DIRECT_TICK( 300 ) )


#ifndef _map_3d_c_
#define EXT   extern
#else
#define EXT
#endif /* _map_3d_c_ */




#undef EXT


#endif /* _map_3d_h_ */
