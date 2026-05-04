/*
  brk_hazard.c
  共有箱型ハザード 定義ファイル

  2000/03/30 T. Morita
  2000/10/16 1.8 T.Morita 
  $Id: brk_hazard.h,v 1.1.1.3 2002/11/19 11:45:31 Yoshizawa1 Exp $
*/

#ifndef __BRK_HZD_H__
#define __BRK_HZD_H__

/* ステージハザード用ID */
enum stage_id_t
{
    /* タンカー編 */
    BRK_NO_STAGE = 0       ,
    BRK_HZD_W01A1_STAIRWAY ,
    BRK_HZD_W01A1_BAR      ,
    BRK_HZD_W01A1_TV_SPACE1,
    BRK_HZD_W01A1_TV_SPACE2,
    BRK_HZD_W01D_STORERAGE ,
    BRK_HZD_W01D_KITCHEN   ,
    BRK_HZD_W01D_DINING_L  ,
    BRK_HZD_W01D_DINING_R  ,
    BRK_HZD_W01D_DINING3   ,
    BRK_HZD_W01A1_BAR2     ,
    BRK_HZD_W01A1_BAR3     ,/*11*/

    /* プラント編 */
    BRK_HZD_W11B_FORTUNE   ,/*12*/
    BRK_HZD_W16A_KITCHEN   ,
    BRK_HZD_W16A_SINK      ,
    BRK_HZD_W16A_DASHBOARD ,
    BRK_HZD_W16A_LRG_GLASS ,
    BRK_HZD_W16A_TOILET_M  ,/*17*/
    BRK_HZD_W16A_TOILET_W  ,
    BRK_HZD_W16A_TABLE1    ,
    BRK_HZD_W16A_TABLE2    ,
    BRK_HZD_W16A_TABLE3    ,/*21*/
    BRK_HZD_W16A_LOCKER    ,
    BRK_HZD_W16A_CABINET   ,/*23*/

    BRK_HZD_W24B_REFRESH   ,/*24*/
    BRK_HZD_W24B_BOTTLEKEEP,

    BRK_HZD_W24C_PARTY     ,/*26*/
    BRK_HZD_W24C_SPEECH    ,

    BRK_HZD_W24D_COMP_ROOM1,
    BRK_HZD_W24D_COMP_ROOM2,
} ;


/* ステージハザード用フラグ */
enum
{
    BRK_HZD_NULL    = 0x00000000,/* NULLハザード */
    BRK_HZD_OUTSIDE = 0x00000001,/* 箱の中に必ず入らないようにするための当たり */   
    BRK_HZD_INSIDE  = 0x00000002,/* 箱の中に常に入っているようにするための当たり */  
    BRK_HZD_X_BOX   = 0x00000004,/* 箱にX軸正方向が空いている当たり */
    BRK_HZD_Y_BOX   = 0x00000008,/* 箱にY軸正方向が空いている当たり */
    BRK_HZD_Z_BOX   = 0x00000010,/* 箱にZ軸正方向が空いている当たり */
    BRK_HZD_FLGMSK  = 0x00000fff,/* フラグのマスク */

    BRK_HZD_ROTATE  = 0x00008000,/* 箱に回転がかかっている (負荷＋) */

    BRK_HZD_FLOOR   = 0x00000001,/* 床に当たった */   
    BRK_HZD_WALL    = 0x00000002,/* 壁に当たった */   
    BRK_HZD_EN_TILT = 0x00000010,/* 重心が外れている */   
    BRK_HZD_THROUGH = 0x00000020,/* メリ込んでいる */   
    BRK_HZD_HITONE  = 0x00008000,/* 一つ以上にあったって要る */   

    BRK_HZD_FIXONLY = 0x00010000,/* FIXのみを検索 */
    BRK_HZD_USRMASK = 0xffff0000,/* USRからのオプション指示 */
} ;

#undef BRK_N_HZD
#define BRK_N_HZD 64   /*登録ハザードキューの最大個数*/


typedef struct hzd_t
{
    int      flag   ;
    FMATRIX *root   ;/* 親 */
    FMATRIX  world  ;/* root を親にする world.m[3]が中心位置 */
    FMATRIX  work   ;/* root親がある時に,計算される */
    FMATRIX  invrs  ;/* workの逆行列 */
    FVECTOR  size   ;/* ハザードのサイズ   vwには長さが入る */
} HZD_BOX ;

/* proto type */
extern int      BRK_CheckHazard( HZD_BOX *hzd, FVECTOR *pos, FVECTOR *pos_v,
				 FVECTOR *bounce, FVECTOR *sphere ) ;
extern int      BRK_CheckTargetHazard( HZD_BOX *hzd, FMATRIX *new, FVECTOR *pos_v,
				       FVECTOR *bounce, TARGET *t ) ;
extern HZD_BOX *BRK_InitHazard( enum stage_id_t id ) ;
extern HZD_BOX *BRK_MakeHazard( int flag, FMATRIX *root, FMATRIX *world,
				FVECTOR *size, FVECTOR *center, TARGET *t ) ;
extern int      BRK_FreeHazard( HZD_BOX *h ) ;

extern void     BRK_MoveHazardToScr( HZD_BOX *fix ) ;
extern void     BRK_ExitHazardToScr( void ) ;
extern int      BRK_CheckHazardScr( FVECTOR *pos, FVECTOR *pos_v, FVECTOR *bounce, FVECTOR *sphere ) ;

/* 内部関数 */
extern int BRK_HZD_SetOutSize( HZD_BOX *h, FVECTOR *npos, FVECTOR *lpos, FVECTOR *sphere,
			       FVECTOR *lsize, FVECTOR *rsize ) ;
extern int BRK_HZD_SetInSize( HZD_BOX *h, FVECTOR *npos, FVECTOR *lpos, FVECTOR *sphere,
			      FVECTOR *lsize, FVECTOR *rsize ) ;

extern int BRK_HZD_OutsideBoxCheck( HZD_BOX *h, FMATRIX *world,
				    FVECTOR *npos, FVECTOR *lpos, FVECTOR *ldir,
				    FVECTOR *lsize, FVECTOR *rsize, int m ) ;
extern int BRK_HZD_InsideBoxCheck( HZD_BOX *h, FMATRIX *world,
				   FVECTOR *npos, FVECTOR *lpos, FVECTOR *ldir,
				   FVECTOR *lsize, FVECTOR *rsize, int m ) ;
extern int HZX_OpenBoxCheck( HZD_BOX *t, FMATRIX *world,
			     FVECTOR *npos, FVECTOR *lpos, FVECTOR *ldir, FVECTOR *sphere,
			     int axis ) ;
extern int HZX_PlaneCheck( HZD_BOX *h, FMATRIX *world,
			   FVECTOR *npos, FVECTOR *lpos, FVECTOR *ldir, FVECTOR *sphere ) ;



extern HZD_BOX *BRK_HZDList[] ;
extern int BRK_HZD_NList ;

extern HZD_BOX *BRK_HZD_Hazard   ;
extern int      BRK_HZD_Flag     ;
extern float    BRK_HZD_HitDist  ;
extern FVECTOR  BRK_HZD_HitPos   ;
extern float    BRK_HZD_Velocity ;
extern FVECTOR  BRK_HZD_Normal   ;

extern FVECTOR  BRK_HZD_NoBounce ;

extern void *NewHzdBoxView( HZD_BOX *targ, u_char r, u_char g, u_char b ) ;
extern void *NewHzdBoxView2( HZD_BOX *targ, u_char r, u_char g, u_char b ) ;


#endif /* __BRK_HZD_H__ */
