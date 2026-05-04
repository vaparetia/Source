/*******************************************************************************
 * layout manager - layoutman.h
 * レイアウトマネージャ
 * 2002/04/10 S.Yamashita
 * $Id: layoutman.h,v 1.1.1.3 2002/11/19 11:51:39 Yoshizawa1 Exp $
 */

#ifndef __INC_LAYOUTMAN__
#define __INC_LAYOUTMAN__

/*******************************************************************************
 * description
 */

/*******************************************************************************
 * include
 */

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"

/*******************************************************************************
 * definitions and typedefs and structures
 */

/* レイアウトマネージャフラグ */
#define LOM_ENABLED        (0x00000001)		/* 有効状態 */
#define LOM_DATA_LOADED    (0x00000002)		/* データロード済み状態 */
#define LOM_DEFAULT_ACTION (0x00000004)		/* デフォルトアクション状態 */
#define LOM_KEY_REP_1      (0x00010000)		/* 十字ボタン系をキーリピート */
#define LOM_KEY_REP_2      (0x00020000)		/* ＡＢボタン系をキーリピート */
#define LOM_KEY_REP_3      (0x00040000)		/* ＬＲボタン系をキーリピート */
#define LOM_KEY_REP_4      (0x00080000)		/* スタートボタン系をキーリピート */
#define LOM_INPUT_DIRECT   (0x00100000)		/* 入力の直接モード */

#define LOM_KEY_SIG     (0x8000)		/* キー信号 */
#define LOM_KEY_ON      (0x4000)		/* キー入力状態 */
#define LOM_KEY_REP_MAX (0x3fff)		/* キーリピート最大値(16383) */

/* ＬＯＭデータフラグ */
#define LOM_DATA_END     (1 << (sizeof(int) * 8 - 1))	/* データの終わりを示す */
#define LOM_ACT_LOOP     (-1)							/* ループ再生 */
#define LOM_ICO_NOTEX    (-1)							/* テクスチャ指定無し */
#define LOM_ICO_LOOP     (-1)							/* ループ表示 */
#define LOM_PAD_ALLACT   (-1)							/* 全てのアクション */
#define LOM_PAD_ALLOBJ   (-1)							/* 全てのオブジェクト */
#define LOM_PAD_ANYINPUT (0x80000000)					/* どんな入力でも */
#define LOM_MODE_NOLIST  (-1)							/* リスト無し */

/* ＡＣＴフラグ */
#define LOM_ACT_TYPEMASK (0x0000ffff)			/* タイプフラグマスク */
#define LOM_ACT_COUNT    (0)					/* 指定回数を再生 */
#define LOM_ACT_FRAME    (1)					/* 指定フレーム数を再生 */
#define LOM_ACT_PAUSE    (2)					/* 指定時間一時停止 */
#define LOM_ACT_CONTI    (3)					/* 一時停止状態の解除 */
#define LOM_ACT_SPEED    (4)					/* 指定速度に設定 */
#define LOM_ACT_BITMASK  (~LOM_ACT_TYPEMASK)	/* ビットフラグマスク */
#define LOM_ACT_HIDE     (0x10000000)			/* 非表示 */
#define LOM_ACT_SHOW     (0x20000000)			/* 表示 */

/* ＩＣＯフラグ */
#define LOM_ICO_ALPHA  (0x00000001)	/* アルファ値有効 */
//#define LOM_ICO_FADE   (0x00000002)	/* フェードで移行 */
#define LOM_ICO_REL    (0x000000f0)	/* 座標が相対 */
#define LOM_ICO_SIZE   (0x00000300)	/* 右と下座標がサイズ */
#define LOM_ICO_L_REL  (0x00000010)	/* 左座標が相対 */
#define LOM_ICO_T_REL  (0x00000020)	/* 上座標が相対 */
#define LOM_ICO_R_REL  (0x00000040)	/* 右座標が相対 */
#define LOM_ICO_B_REL  (0x00000080)	/* 下座標が相対 */
#define LOM_ICO_R_SIZE (0x00000100)	/* 右座標が幅サイズ */
#define LOM_ICO_B_SIZE (0x00000200)	/* 下座標が高さサイズ */

/* ＯＢＪフラグ */
#define LOM_OBJ_NOOBJ     (0x00000001)	/* L2D オブジェクト無し */
#define LOM_OBJ_ANIME_ORD (0x00000002)	/* 順番にアイコンアニメーション */
#define LOM_OBJ_ANIME_REV (0x00000004)	/* 逆順にアイコンアニメーション */
#define LOM_OBJ_INTERP_1  (0x00000010)	/* 指定フレーム数補間移行 */
#define LOM_OBJ_INTERP_2  (0x00000020)	/* 指定分母補間移行 */
//#define LOM_OBJ_INTERP_3  (0x00000040)	/* 指定移動量補間移行 */
//#define LOM_OBJ_BLINK_1   (0x00000100)	/* 　４フレーム毎に点滅 */
//#define LOM_OBJ_BLINK_2   (0x00000200)	/* 　８フレーム毎に点滅 */
//#define LOM_OBJ_BLINK_3   (0x00000400)	/* １６フレーム毎に点滅 */
//#define LOM_OBJ_BLINK_4   (0x00000800)	/* ３２フレーム毎に点滅 */

/* ＰＡＤフラグ */
#define LOM_PAD_PRESS   (0x00000001)	/* 押された時 */
#define LOM_PAD_RELEASE (0x00000002)	/* 離された時 */
#define LOM_PAD_ON      (0x00000010)	/* 押されている時 */
#define LOM_PAD_OFF     (0x00000020)	/* 離されている時 */
#define LOM_PAD_ON_SIG  (0x00000040)	/* ON 信号毎に */
#define LOM_PAD_OFF_SIG (0x00000080)	/* OFF 信号毎に */

/* 実行タイプ */
enum
{
	LOM_EXE_CALLBACK = 0,	/* コールバック実行 */
	LOM_EXE_PROC,			/* プロック実行 */

	LOM_EXE_MODE_NEXT,		/* 次の　　　　　　　　ＬＯＭモードへ移行 */
	LOM_EXE_MODE_PREV,		/* 前の　　　　　　　　ＬＯＭモードへ移行 */
	LOM_EXE_MODE_NAME,		/* 指定モード名の　　　ＬＯＭモードへ移行 */
	LOM_EXE_MODE_INDEX,		/* 指定インデックスの　ＬＯＭモードへ移行 */
	LOM_EXE_MODE_NEXT2,		/* 次の　　　　　　　　ＬＯＭモードへ移行 */
	LOM_EXE_MODE_PREV2,		/* 前の　　　　　　　　ＬＯＭモードへ移行 */
	LOM_EXE_MODE_NAME2,		/* 指定モード名の　　　ＬＯＭモードへ移行 */
	LOM_EXE_MODE_INDEX2,	/* 指定インデックスの　ＬＯＭモードへ移行 */

	LOM_EXE_ACT_NEXT,		/* 次の　　　　　　　　ＬＯＭアクションへ移行 */
	LOM_EXE_ACT_PREV,		/* 前の　　　　　　　　ＬＯＭアクションへ移行 */
	LOM_EXE_ACT_NAME,		/* 指定アクション名の　ＬＯＭアクションへ移行 */
	LOM_EXE_ACT_INDEX,		/* 指定インデックスの　ＬＯＭアクションへ移行 */

	LOM_EXE_ICO_NEXT,		/* 次の　　　　　　　　ＬＯＭアイコンへ移行 */
	LOM_EXE_ICO_PREV,		/* 前の　　　　　　　　ＬＯＭアイコンへ移行 */
	LOM_EXE_ICO_NAME,		/* 指定アイコン名の　　ＬＯＭアイコンへ移行 */
	LOM_EXE_ICO_INDEX,		/* 指定インデックスの　ＬＯＭアイコンへ移行 */

	LOM_EXE_OBJ_NEXT,		/* 次の　　　　　　　　　ＬＯＭオブジェクトへ移行 */
	LOM_EXE_OBJ_PREV,		/* 前の　　　　　　　　　ＬＯＭオブジェクトへ移行 */
	LOM_EXE_OBJ_NAME,		/* 指定オブジェクト名の　ＬＯＭオブジェクトへ移行 */
	LOM_EXE_OBJ_INDEX,		/* 指定インデックスの　　ＬＯＭオブジェクトへ移行 */
};

/* キータイプ */
enum
{
	LOM_KEY_U = 0,	/* 上 */
	LOM_KEY_D,		/* 下 */
	LOM_KEY_L,		/* 左 */
	LOM_KEY_R,		/* 右 */
	LOM_KEY_A,		/* Ａ */
	LOM_KEY_B,		/* Ｂ */
	LOM_KEY_X,		/* Ｘ */
	LOM_KEY_Y,		/* Ｙ */
	LOM_KEY_L1,		/* Ｌ１ */
	LOM_KEY_R1,		/* Ｒ１ */
	LOM_KEY_L2,		/* Ｌ２ */
	LOM_KEY_R2,		/* Ｒ２ */
	LOM_KEY_ST,		/* スタート */
	LOM_KEY_SE,		/* セレクト */
	LOM_KEY_AL,		/* アナログＬ */
	LOM_KEY_AR,		/* アナログＲ */
	LOM_KEY_MAX,
};

/* 方向タイプ */
enum
{
	LOM_LEFT = 0,
	LOM_TOP,
	LOM_RIGHT,
	LOM_BOTTOM,
};

/* 入力フラグ(参考) */
/*
#define PAD_UDLR		(0xf000)
#define PAD_ABXY		(0x00f0)
#define PAD_LR			(0x000f)

#define PAD_U			(0x1000)
#define PAD_D			(0x4000)
#define PAD_L			(0x8000)
#define PAD_R			(0x2000)
#define PAD_A			(0x0020)
#define PAD_B			(0x0040)
#define PAD_X			(0x0010)
#define PAD_Y			(0x0080)

#define PAD_L1			(0x0004)
#define PAD_R1			(0x0008)
#define PAD_L2			(0x0001)
#define PAD_R2			(0x0002)

#define PAD_STA			(0x0800)
#define PAD_SEL			(0x0100)

#define PAD_AL			(0x0200)
#define PAD_AR			(0x0400)
*/

/* 識別ＩＤ */
#define LOM_ACTLIST_DATA ( 4292666)
#define LOM_ICOLIST_DATA (10585145)
#define LOM_OBJLIST_DATA (  100149)
#define LOM_PADLIST_DATA ( 4294576)
#define LOM_MODE_DATA    ( 4686097)

#define LOM_PAD_OK       ( 0x10000000 )
#define LOM_PAD_CANCEL   ( 0x20000000 )

#define LOM_ACTLIST_00 (6180300)
#define LOM_ACTLIST_01 (6180301)
#define LOM_ACTLIST_02 (6180302)
#define LOM_ACTLIST_03 (6180303)
#define LOM_ACTLIST_04 (6180304)
#define LOM_ACTLIST_05 (6180305)
#define LOM_ACTLIST_06 (6180306)
#define LOM_ACTLIST_07 (6180307)
#define LOM_ACTLIST_08 (6180308)
#define LOM_ACTLIST_09 (6180309)
#define LOM_ACTLIST_10 (6180332)
#define LOM_ACTLIST_11 (6180333)
#define LOM_ACTLIST_12 (6180334)
#define LOM_ACTLIST_13 (6180335)
#define LOM_ACTLIST_14 (6180336)
#define LOM_ACTLIST_15 (6180337)
#define LOM_ACTLIST_16 (6180338)
#define LOM_ACTLIST_17 (6180339)
#define LOM_ACTLIST_18 (6180340)
#define LOM_ACTLIST_19 (6180341)
#define LOM_ACTLIST_20 (6180364)
#define LOM_ACTLIST_21 (6180365)
#define LOM_ACTLIST_22 (6180366)
#define LOM_ACTLIST_23 (6180367)
#define LOM_ACTLIST_24 (6180368)
#define LOM_ACTLIST_25 (6180369)
#define LOM_ACTLIST_26 (6180370)
#define LOM_ACTLIST_27 (6180371)
#define LOM_ACTLIST_28 (6180372)
#define LOM_ACTLIST_29 (6180373)
#define LOM_ACTLIST_30 (6180396)
#define LOM_ACTLIST_31 (6180397)
#define LOM_ACTLIST_32 (6180398)
#define LOM_ACTLIST_33 (6180399)
#define LOM_ACTLIST_34 (6180400)
#define LOM_ACTLIST_35 (6180401)
#define LOM_ACTLIST_36 (6180402)
#define LOM_ACTLIST_37 (6180403)
#define LOM_ACTLIST_38 (6180404)
#define LOM_ACTLIST_39 (6180405)
#define LOM_ACTLIST_40 (6180428)
#define LOM_ACTLIST_41 (6180429)
#define LOM_ACTLIST_42 (6180430)
#define LOM_ACTLIST_43 (6180431)
#define LOM_ACTLIST_44 (6180432)
#define LOM_ACTLIST_45 (6180433)
#define LOM_ACTLIST_46 (6180434)
#define LOM_ACTLIST_47 (6180435)
#define LOM_ACTLIST_48 (6180436)
#define LOM_ACTLIST_49 (6180437)
#define LOM_ACTLIST_50 (6180460)
#define LOM_ACTLIST_51 (6180461)
#define LOM_ACTLIST_52 (6180462)
#define LOM_ACTLIST_53 (6180463)
#define LOM_ACTLIST_54 (6180464)
#define LOM_ACTLIST_55 (6180465)
#define LOM_ACTLIST_56 (6180466)
#define LOM_ACTLIST_57 (6180467)
#define LOM_ACTLIST_58 (6180468)
#define LOM_ACTLIST_59 (6180469)
#define LOM_ACTLIST_60 (6180492)
#define LOM_ACTLIST_61 (6180493)
#define LOM_ACTLIST_62 (6180494)
#define LOM_ACTLIST_63 (6180495)
#define LOM_ACTLIST_64 (6180496)
#define LOM_ACTLIST_65 (6180497)
#define LOM_ACTLIST_66 (6180498)
#define LOM_ACTLIST_67 (6180499)
#define LOM_ACTLIST_68 (6180500)
#define LOM_ACTLIST_69 (6180501)
#define LOM_ACTLIST_70 (6180524)
#define LOM_ACTLIST_71 (6180525)
#define LOM_ACTLIST_72 (6180526)
#define LOM_ACTLIST_73 (6180527)
#define LOM_ACTLIST_74 (6180528)
#define LOM_ACTLIST_75 (6180529)
#define LOM_ACTLIST_76 (6180530)
#define LOM_ACTLIST_77 (6180531)
#define LOM_ACTLIST_78 (6180532)
#define LOM_ACTLIST_79 (6180533)
#define LOM_ACTLIST_80 (6180556)
#define LOM_ACTLIST_81 (6180557)
#define LOM_ACTLIST_82 (6180558)
#define LOM_ACTLIST_83 (6180559)
#define LOM_ACTLIST_84 (6180560)
#define LOM_ACTLIST_85 (6180561)
#define LOM_ACTLIST_86 (6180562)
#define LOM_ACTLIST_87 (6180563)
#define LOM_ACTLIST_88 (6180564)
#define LOM_ACTLIST_89 (6180565)

#define LOM_ICOLIST_00 (6170061)
#define LOM_ICOLIST_01 (6170062)
#define LOM_ICOLIST_02 (6170063)
#define LOM_ICOLIST_03 (6170064)
#define LOM_ICOLIST_04 (6170065)
#define LOM_ICOLIST_05 (6170066)
#define LOM_ICOLIST_06 (6170067)
#define LOM_ICOLIST_07 (6170068)
#define LOM_ICOLIST_08 (6170069)
#define LOM_ICOLIST_09 (6170070)
#define LOM_ICOLIST_10 (6170093)
#define LOM_ICOLIST_11 (6170094)
#define LOM_ICOLIST_12 (6170095)
#define LOM_ICOLIST_13 (6170096)
#define LOM_ICOLIST_14 (6170097)
#define LOM_ICOLIST_15 (6170098)
#define LOM_ICOLIST_16 (6170099)
#define LOM_ICOLIST_17 (6170100)
#define LOM_ICOLIST_18 (6170101)
#define LOM_ICOLIST_19 (6170102)

#define LOM_ICO_00 (2785936)
#define LOM_ICO_01 (2785937)
#define LOM_ICO_02 (2785938)
#define LOM_ICO_03 (2785939)
#define LOM_ICO_04 (2785940)
#define LOM_ICO_05 (2785941)
#define LOM_ICO_06 (2785942)
#define LOM_ICO_07 (2785943)
#define LOM_ICO_08 (2785944)
#define LOM_ICO_09 (2785945)
#define LOM_ICO_10 (2785968)
#define LOM_ICO_11 (2785969)
#define LOM_ICO_12 (2785970)
#define LOM_ICO_13 (2785971)
#define LOM_ICO_14 (2785972)
#define LOM_ICO_15 (2785973)
#define LOM_ICO_16 (2785974)
#define LOM_ICO_17 (2785975)
#define LOM_ICO_18 (2785976)
#define LOM_ICO_19 (2785977)

#define LOM_OBJLIST_00 (1899982)
#define LOM_OBJLIST_01 (1899983)
#define LOM_OBJLIST_02 (1899984)
#define LOM_OBJLIST_03 (1899985)
#define LOM_OBJLIST_04 (1899986)
#define LOM_OBJLIST_05 (1899987)
#define LOM_OBJLIST_06 (1899988)
#define LOM_OBJLIST_07 (1899989)
#define LOM_OBJLIST_08 (1899990)
#define LOM_OBJLIST_09 (1899991)
#define LOM_OBJLIST_10 (1900014)
#define LOM_OBJLIST_11 (1900015)
#define LOM_OBJLIST_12 (1900016)
#define LOM_OBJLIST_13 (1900017)
#define LOM_OBJLIST_14 (1900018)
#define LOM_OBJLIST_15 (1900019)
#define LOM_OBJLIST_16 (1900020)
#define LOM_OBJLIST_17 (1900021)
#define LOM_OBJLIST_18 (1900022)
#define LOM_OBJLIST_19 (1900023)

#define LOM_OBJ_00 (1573532)
#define LOM_OBJ_01 (1573533)
#define LOM_OBJ_02 (1573534)
#define LOM_OBJ_03 (1573535)
#define LOM_OBJ_04 (1573536)
#define LOM_OBJ_05 (1573537)
#define LOM_OBJ_06 (1573538)
#define LOM_OBJ_07 (1573539)
#define LOM_OBJ_08 (1573540)
#define LOM_OBJ_09 (1573541)
#define LOM_OBJ_10 (1573564)
#define LOM_OBJ_11 (1573565)
#define LOM_OBJ_12 (1573566)
#define LOM_OBJ_13 (1573567)
#define LOM_OBJ_14 (1573568)
#define LOM_OBJ_15 (1573569)
#define LOM_OBJ_16 (1573570)
#define LOM_OBJ_17 (1573571)
#define LOM_OBJ_18 (1573572)
#define LOM_OBJ_19 (1573573)

#define LOM_PADLIST_00 (3919310)
#define LOM_PADLIST_01 (3919311)
#define LOM_PADLIST_02 (3919312)
#define LOM_PADLIST_03 (3919313)
#define LOM_PADLIST_04 (3919314)
#define LOM_PADLIST_05 (3919315)
#define LOM_PADLIST_06 (3919316)
#define LOM_PADLIST_07 (3919317)
#define LOM_PADLIST_08 (3919318)
#define LOM_PADLIST_09 (3919319)
#define LOM_PADLIST_10 (3919342)
#define LOM_PADLIST_11 (3919343)
#define LOM_PADLIST_12 (3919344)
#define LOM_PADLIST_13 (3919345)
#define LOM_PADLIST_14 (3919346)
#define LOM_PADLIST_15 (3919347)
#define LOM_PADLIST_16 (3919348)
#define LOM_PADLIST_17 (3919349)
#define LOM_PADLIST_18 (3919350)
#define LOM_PADLIST_19 (3919351)

#define LOM_MODE_00 (9703244)
#define LOM_MODE_01 (9703245)
#define LOM_MODE_02 (9703246)
#define LOM_MODE_03 (9703247)
#define LOM_MODE_04 (9703248)
#define LOM_MODE_05 (9703249)
#define LOM_MODE_06 (9703250)
#define LOM_MODE_07 (9703251)
#define LOM_MODE_08 (9703252)
#define LOM_MODE_09 (9703253)
#define LOM_MODE_10 (9703276)
#define LOM_MODE_11 (9703277)
#define LOM_MODE_12 (9703278)
#define LOM_MODE_13 (9703279)
#define LOM_MODE_14 (9703280)
#define LOM_MODE_15 (9703281)
#define LOM_MODE_16 (9703282)
#define LOM_MODE_17 (9703283)
#define LOM_MODE_18 (9703284)
#define LOM_MODE_19 (9703285)
#define LOM_MODE_20 (9703308)
#define LOM_MODE_21 (9703309)
#define LOM_MODE_22 (9703310)
#define LOM_MODE_23 (9703311)
#define LOM_MODE_24 (9703312)
#define LOM_MODE_25 (9703313)
#define LOM_MODE_26 (9703314)
#define LOM_MODE_27 (9703315)
#define LOM_MODE_28 (9703316)
#define LOM_MODE_29 (9703317)
#define LOM_MODE_30 (9703340)
#define LOM_MODE_31 (9703341)
#define LOM_MODE_32 (9703342)
#define LOM_MODE_33 (9703343)
#define LOM_MODE_34 (9703344)
#define LOM_MODE_35 (9703345)
#define LOM_MODE_36 (9703346)
#define LOM_MODE_37 (9703347)
#define LOM_MODE_38 (9703348)
#define LOM_MODE_39 (9703349)
#define LOM_MODE_40 (9703372)
#define LOM_MODE_41 (9703373)
#define LOM_MODE_42 (9703374)
#define LOM_MODE_43 (9703375)
#define LOM_MODE_44 (9703376)
#define LOM_MODE_45 (9703377)
#define LOM_MODE_46 (9703378)
#define LOM_MODE_47 (9703379)
#define LOM_MODE_48 (9703380)
#define LOM_MODE_49 (9703381)
#define LOM_MODE_50 (9703404)
#define LOM_MODE_51 (9703405)
#define LOM_MODE_52 (9703406)
#define LOM_MODE_53 (9703407)
#define LOM_MODE_54 (9703408)
#define LOM_MODE_55 (9703409)
#define LOM_MODE_56 (9703410)
#define LOM_MODE_57 (9703411)
#define LOM_MODE_58 (9703412)
#define LOM_MODE_59 (9703413)
#define LOM_MODE_60 (9703436)
#define LOM_MODE_61 (9703437)
#define LOM_MODE_62 (9703438)
#define LOM_MODE_63 (9703439)
#define LOM_MODE_64 (9703440)
#define LOM_MODE_65 (9703441)
#define LOM_MODE_66 (9703442)
#define LOM_MODE_67 (9703443)
#define LOM_MODE_68 (9703444)
#define LOM_MODE_69 (9703445)
#define LOM_MODE_70 (9703468)
#define LOM_MODE_71 (9703469)
#define LOM_MODE_72 (9703470)
#define LOM_MODE_73 (9703471)
#define LOM_MODE_74 (9703472)
#define LOM_MODE_75 (9703473)
#define LOM_MODE_76 (9703474)
#define LOM_MODE_77 (9703475)
#define LOM_MODE_78 (9703476)
#define LOM_MODE_79 (9703477)
#define LOM_MODE_80 (9703500)
#define LOM_MODE_81 (9703501)
#define LOM_MODE_82 (9703502)
#define LOM_MODE_83 (9703503)
#define LOM_MODE_84 (9703504)
#define LOM_MODE_85 (9703505)
#define LOM_MODE_86 (9703506)
#define LOM_MODE_87 (9703507)
#define LOM_MODE_88 (9703508)
#define LOM_MODE_89 (9703509)
#define LOM_MODE_90 (9703532)
#define LOM_MODE_91 (9703533)
#define LOM_MODE_92 (9703534)
#define LOM_MODE_93 (9703535)
#define LOM_MODE_94 (9703536)
#define LOM_MODE_95 (9703537)
#define LOM_MODE_96 (9703538)
#define LOM_MODE_97 (9703539)
#define LOM_MODE_98 (9703540)
#define LOM_MODE_99 (9703541)
#define LOM_MODE_100 (8515010)
#define LOM_MODE_101 (8515011)
#define LOM_MODE_102 (8515012)
#define LOM_MODE_103 (8515013)
#define LOM_MODE_104 (8515014)
#define LOM_MODE_105 (8515015)
#define LOM_MODE_106 (8515016)
#define LOM_MODE_107 (8515017)
#define LOM_MODE_108 (8515018)
#define LOM_MODE_109 (8515019)
#define LOM_MODE_110 (8515042)
#define LOM_MODE_111 (8515043)
#define LOM_MODE_112 (8515044)
#define LOM_MODE_113 (8515045)
#define LOM_MODE_114 (8515046)
#define LOM_MODE_115 (8515047)
#define LOM_MODE_116 (8515048)
#define LOM_MODE_117 (8515049)
#define LOM_MODE_118 (8515050)
#define LOM_MODE_119 (8515051)
#define LOM_MODE_120 (8515074)
#define LOM_MODE_121 (8515075)
#define LOM_MODE_122 (8515076)
#define LOM_MODE_123 (8515077)
#define LOM_MODE_124 (8515078)
#define LOM_MODE_125 (8515079)
#define LOM_MODE_126 (8515080)
#define LOM_MODE_127 (8515081)
#define LOM_MODE_128 (8515082)
#define LOM_MODE_129 (8515083)

/* データ数 */
#define LOM_ACT_DATA_COUNT  (3)		/* ＬＯＭアクションのデータ数 */
#define LOM_ICO_DATA_COUNT  (12)	/* ＬＯＭアイコンのデータ数 */
#define LOM_OBJ_DATA_COUNT  (4)		/* ＬＯＭオブジェクトのデータ数 */
#define LOM_PAD_DATA_COUNT  (6)		/* ＬＯＭパッドのデータ数 */
#define LOM_MODE_DATA_COUNT (4)		/* ＬＯＭモードのデータ数 */



/* コールバック関数 */
typedef int (*LOM_CALLBACK)(void *pWork, int param);

/* シグナルハンドラ */
typedef void (*LOM_SIG_HANDLER)(void *pWork, int sign, int value);



/* ＬＯＭアクション */
typedef struct tagLOM_ACT
{
	int act_name;	/* アクション名 */
	int count;		/* カウント */
	int flag;		/* フラグ */
}
LOM_ACT;

/* ＬＯＭアクションリスト */
typedef struct tagLOM_ACTLIST
{
	int     actlist_name;		/* アクションリスト名 */
	LOM_ACT *lom_act;			/* ＬＯＭアクション配列 */
	int     n_lom_act;			/* ＬＯＭアクション数 */
	int     c_lom_act_index;	/* カレントＬＯＭアクションインデックス */
}
LOM_ACTLIST;



/* ＬＯＭアイコン */
typedef struct tagLOM_ICO
{
	int       ico_name;		/* アイコン名 */
	int       tex_name;		/* テクスチャ名 */
	float     pos[4];		/* 位置(左、上、右、下) */
	SPR_COLOR col;			/* 色 */
	int       count;		/* カウント */
	int       flag;			/* フラグ */
	SPR_OBJ   *sprite;		/* スプライト */
}
LOM_ICO;

/* ＬＯＭアイコンリスト */
typedef struct tagLOM_ICOLIST
{
	int     icolist_name;		/* アイコンリスト名 */
	LOM_ICO *lom_ico;			/* ＬＯＭアイコン配列 */
	int     n_lom_ico;			/* ＬＯＭアイコン数 */
	int     c_lom_ico_index;	/* カレントＬＯＭアイコンインデックス */
}
LOM_ICOLIST;



/* ＬＯＭオブジェクト */
typedef struct tagLOM_OBJ
{
	int obj_name;		/* オブジェクト名 */
	int icolist_name;	/* アイコンリスト名 */
	int count;			/* カウント */
	int flag;			/* フラグ */
}
LOM_OBJ;

/* ＬＯＭオブジェクトリスト */
typedef struct tagLOM_OBJLIST
{
	int     objlist_name;		/* オブジェクトリスト名 */
	LOM_OBJ *lom_obj;			/* ＬＯＭオブジェクト配列 */
	int     n_lom_obj;			/* ＬＯＭオブジェクト数 */
	int     c_lom_obj_index;	/* カレントＬＯＭオブジェクトインデックス */
}
LOM_OBJLIST;



/* ＬＯＭパッド */
typedef struct tagLOM_PAD
{
	int act_name;		/* アクション名 */
	int obj_name;		/* オブジェクト名 */
	int input;			/* 入力 */
	int execute;		/* 実行番号 */
	int param;			/* 実行引数 */
	int flag;			/* フラグ */
}
LOM_PAD;

/* ＬＯＭパッドリスト */
typedef struct tagLOM_PADLIST
{
	int     padlist_name;	/* パッドリスト名 */
	LOM_PAD *lom_pad;		/* ＬＯＭパッド配列 */
	int     n_lom_pad;		/* ＬＯＭパッド数 */
}
LOM_PADLIST;



/* ＬＯＭモード */
typedef struct tagLOM_MODE
{
	int mode_name;			/* モード名 */
	int lom_actlist_index;	/* ＬＯＭアクションリストインデックス */
	int lom_objlist_index;	/* ＬＯＭオブジェクトリストインデックス */
	int lom_padlist_index;	/* ＬＯＭパッドリストインデックス */
}
LOM_MODE;



/* レイアウトマネージャ */
typedef struct tagLAYOUTMAN
{
	int          layout;					/* ２Ｄレイアウトハンドル */
	int          tri;						/* アイコンＴＲＩハンドル */
	int          chanl;						/* チャンネル */
	int          proc_id;					/* プロックＩＤ */
	LOM_CALLBACK callback;					/* コールバック関数 */
	void         *chara_work;				/* 呼び出し元キャラのワーク */
	short        key_rep[2];				/* キーリピート設定 */
	int          flag;						/* フラグ */

	int          count;						/* カウント */
	int          act_count;					/* アクションカウンタ */
	int          ico_count;					/* アイコンカウンタ */
	int          obj_count;					/* オブジェクトカウンタ */
	float        ico_pos[2][4];				/* アイコン位置(左、上、右、下) */
	short        key_status[LOM_KEY_MAX];	/* キー状態 */

	LOM_ACTLIST *lom_actlist;				/* ＬＯＭアクションリスト配列 */
	int         n_lom_actlist;				/* ＬＯＭアクションリスト数 */
	int         c_lom_actlist_index;		/* カレントＬＯＭアクションリストインデックス */
	LOM_ICOLIST *lom_icolist;				/* ＬＯＭアイコンリスト配列 */
	int         n_lom_icolist;				/* ＬＯＭアイコンリスト数 */
	int         c_lom_icolist_index;		/* カレントＬＯＭアイコンリストインデックス */
	LOM_OBJLIST *lom_objlist;				/* ＬＯＭオブジェクトリスト配列 */
	int         n_lom_objlist;				/* ＬＯＭオブジェクトリスト数 */
	int         c_lom_objlist_index;		/* カレントＬＯＭオブジェクトリストインデックス */
	LOM_PADLIST *lom_padlist;				/* ＬＯＭパッドリスト配列 */
	int         n_lom_padlist;				/* ＬＯＭパッドリスト数 */
	int         c_lom_padlist_index;		/* カレントＬＯＭパッドリストインデックス */

	LOM_MODE    *lom_mode;					/* ＬＯＭモード配列 */
	int         n_lom_mode;					/* ＬＯＭモード数 */
	int         c_lom_mode_index;			/* カレントＬＯＭモードインデックス */
	int         pad;
}
LAYOUTMAN;

/*******************************************************************************
 * functions
 */

/* layoutman.c */
int  CreateLayoutman (LAYOUTMAN *layoutman, int strcode, int chanl, int base_pri, int add_flag);
int  CreateLayoutman2(LAYOUTMAN *layoutman, int strcode, int chanl, int base_pri, int add_flag, int tri_name, int proc_id, LOM_CALLBACK callback, LOM_SIG_HANDLER sig_handler, void *pWork);
int  CreateLayoutman3(LAYOUTMAN *layoutman, int strcode, int chanl, int base_pri, int add_flag, int tri_name, int proc_id, LOM_CALLBACK callback, LOM_SIG_HANDLER sig_handler, void *pWork, short key_rep_1, short key_rep_2, int flag);
int  CreateLayoutman4(LAYOUTMAN *layoutman, int strcode, int chanl, int base_pri, int add_flag, int pause_level, int tri_name, int proc_id, LOM_CALLBACK callback, LOM_SIG_HANDLER sig_handler, void *pWork, short key_rep_1, short key_rep_2, int flag);
void DestroyLayoutman(LAYOUTMAN *layoutman);
int  LoadLOMData     (LAYOUTMAN *layoutman, int *data);
int  ActLayoutman    (LAYOUTMAN *layoutman);
int  LOM_SetIcoPos   (LAYOUTMAN *layoutman, int strcode, int left, int top, int right, int bottom, int size);
int  LOM_SetIcoPri   (LAYOUTMAN *layoutman, SPR_PRI pri);

/* lom_get_cur.c */
int LOM_GetCurActList (LAYOUTMAN *layoutman);
int LOM_GetCurActListI(LAYOUTMAN *layoutman);
int LOM_GetCurAct     (LAYOUTMAN *layoutman);
int LOM_GetCurActI    (LAYOUTMAN *layoutman);
int LOM_GetCurObjList (LAYOUTMAN *layoutman);
int LOM_GetCurObjListI(LAYOUTMAN *layoutman);
int LOM_GetCurObj     (LAYOUTMAN *layoutman);
int LOM_GetCurObjI    (LAYOUTMAN *layoutman);
int LOM_GetCurMode    (LAYOUTMAN *layoutman);
int LOM_GetCurModeI   (LAYOUTMAN *layoutman);

/* lom_set_cur.c */
int LOM_SetCurActI (LAYOUTMAN *layoutman, int lom_act_index);
int LOM_SetCurAct  (LAYOUTMAN *layoutman, int act_name);
int LOM_SetCurIcoI (LAYOUTMAN *layoutman, int lom_ico_index);
int LOM_SetCurIco  (LAYOUTMAN *layoutman, int ico_name);
int LOM_SetCurObjI (LAYOUTMAN *layoutman, int lom_obj_index, int default_ico);
int LOM_SetCurObj  (LAYOUTMAN *layoutman, int obj_name, int default_ico);
int LOM_SetCurModeI(LAYOUTMAN *layoutman, int lom_mode_index, int default_act, int default_ico, int default_obj);
int LOM_SetCurMode (LAYOUTMAN *layoutman, int lom_mode_name, int default_act, int default_ico, int default_obj);

/* lom_sprite_get.c */
int LOM_GetSprID        (LAYOUTMAN *layoutman, int strcode);
int LOM_Spr_GetPosition (LAYOUTMAN *layoutman, int strcode, SPR_POS *pos, int flag);
int LOM_Spr_GetColor    (LAYOUTMAN *layoutman, int strcode, SPR_COLOR *col);
int LOM_Spr_GetWidth    (LAYOUTMAN *layoutman, int strcode, float *width, int flag);
int LOM_Spr_GetHeight   (LAYOUTMAN *layoutman, int strcode, float *height, int flag);
int LOM_Spr_GetFlags    (LAYOUTMAN *layoutman, int strcode, int *flags);
int LOM_Lin_GetPosition0(LAYOUTMAN *layoutman, int strcode, SPR_POS *pos, int flag);
int LOM_Lin_GetPosition1(LAYOUTMAN *layoutman, int strcode, SPR_POS *pos, int flag);
int LOM_Lin_GetColor0   (LAYOUTMAN *layoutman, int strcode, SPR_COLOR *col);
int LOM_Lin_GetColor1   (LAYOUTMAN *layoutman, int strcode, SPR_COLOR *col);
int LOM_Lin_GetFlags    (LAYOUTMAN *layoutman, int strcode, int *flags);
int LOM_Emp_GetPosition (LAYOUTMAN *layoutman, int strcode, SPR_POS *pos, int flag);
int LOM_Emp_GetFlags    (LAYOUTMAN *layoutman, int strcode, int *flags);
int LOM_SprTex_GetU     (LAYOUTMAN *layoutman, int strcode, float *u);
int LOM_SprTex_GetV     (LAYOUTMAN *layoutman, int strcode, float *v);
int LOM_SprTex_GetWidth (LAYOUTMAN *layoutman, int strcode, float *width);
int LOM_SprTex_GetHeight(LAYOUTMAN *layoutman, int strcode, float *height);

/* lom_sprite_set.c */
int LOM_Spr_SetPosition (LAYOUTMAN *layoutman, int strcode, SPR_POS *pos, int flag);
int LOM_Spr_SetColor    (LAYOUTMAN *layoutman, int strcode, SPR_COLOR *col);
int LOM_Spr_SetWidth    (LAYOUTMAN *layoutman, int strcode, float width, int flag);
int LOM_Spr_SetHeight   (LAYOUTMAN *layoutman, int strcode, float height, int flag);
int LOM_Spr_SetFlags    (LAYOUTMAN *layoutman, int strcode, int flags);
int LOM_Lin_SetPosition0(LAYOUTMAN *layoutman, int strcode, SPR_POS *pos, int flag);
int LOM_Lin_SetPosition1(LAYOUTMAN *layoutman, int strcode, SPR_POS *pos, int flag);
int LOM_Lin_SetColor0   (LAYOUTMAN *layoutman, int strcode, SPR_COLOR *col);
int LOM_Lin_SetColor1   (LAYOUTMAN *layoutman, int strcode, SPR_COLOR *col);
int LOM_Lin_SetFlags    (LAYOUTMAN *layoutman, int strcode, int flags);
int LOM_Emp_SetPosition (LAYOUTMAN *layoutman, int strcode, SPR_POS *pos, int flag);
int LOM_Emp_SetFlags    (LAYOUTMAN *layoutman, int strcode, int flags);
int LOM_SprTex_SetU     (LAYOUTMAN *layoutman, int strcode, float u);
int LOM_SprTex_SetV     (LAYOUTMAN *layoutman, int strcode, float v);
int LOM_SprTex_SetWidth (LAYOUTMAN *layoutman, int strcode, float width);
int LOM_SprTex_SetHeight(LAYOUTMAN *layoutman, int strcode, float height);

/*******************************************************************************
 */

#endif	/* __INC_LAYOUTMAN__ */
