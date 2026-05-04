/******************************************************************************
 * vr_pause - vr_map_3d.h
 * ＶＲ全体マップ表示３Ｄ  *NewVRMap3D
 * 2002/07/08 S.Yamashita
 * $Id: vr_map_3d.h,v 1.1.1.3 2002/11/19 11:51:44 Yoshizawa1 Exp $
 */

#ifndef __INC_VR_MAP_3D__
#define __INC_VR_MAP_3D__

/******************************************************************************
 * defines
 */

#define VRMAP_STRCODE (12790628)	/* vr_map_3d */

/* シグナルタイプ */
enum
{
	VRMAP_CHILD_END = VRMAP_STRCODE,	/* 子供が終わった */
};

/* 爆弾設置数 */
#define BOMB_MAX (32)

/* 動作モード */
enum
{
	SK_TANKER  = 0,
	SK_PLANT,
	SK_ARSENAL,
	SK_OTHER,
};

/* 各テーブルビットフラグ */
enum
{
	F_TANKER_BODY = 0x00000001,	/* map_tanker_body      */
	F_TANKER_00A  = 0x00000002,	/* map_tanker_00a       船尾甲板 */
	F_TANKER_01A  = 0x00000008,	/* map_tanker_01a       船橋一階居住区 */
	F_TANKER_01F  = 0x00000004,	/* map_tanker_01f       船橋一階リフレッシュルーム */
	F_TANKER_01B  = 0x00000010,	/* map_tanker_01b       船橋二階居住区 */
	F_TANKER_01C  = 0x00000020,	/* map_tanker_01c       船橋三階居住区 */
	F_TANKER_01D  = 0x00000040,	/* map_tanker_01d       船橋四階居住区 */
	F_TANKER_01E  = 0x00000080,	/* map_tanker_01e       船橋五階操舵室 */
	F_TANKER_00B  = 0x00000100,	/* map_tanker_00b       航海甲板ウィング */
	F_TANKER_02A  = 0x00000200,	/* map_tanker_02a       機関室 */
	F_TANKER_03B  = 0x00000400,	/* map_tanker_03b       第二甲板右舷 */
	F_TANKER_03A  = 0x00000800,	/* map_tanker_03a       第二甲板左舷 */
	F_TANKER_03_O = 0x00001000,	/* map_tanker_03_other  */
	F_TANKER_04A  = 0x00002000,	/* map_tanker_04a       第一船倉 */
	F_TANKER_04B  = 0x00004000,	/* map_tanker_04b       第二船倉 */
	F_TANKER_04C  = 0x00008000,	/* map_tanker_04c       第三船倉 */
};

enum
{
	F_PLANT_A     = 0x00000001,	/* a_w12                Ａ脚ポンプ施設 */
	F_PLANT_AB    = 0x00000002,	/* ab_w13               ＡＢ連絡橋 */
	F_PLANT_B     = 0x00000004,	/* b_w14                Ｂ脚変電室 */
	F_PLANT_BC    = 0x00000008,	/* bc_w15               ＢＣ連絡橋 */
	F_PLANT_C     = 0x00000010,	/* c_w16                Ｃ脚食堂 */
	F_PLANT_CD    = 0x00000020,	/* cd_w17               ＣＤ連絡橋 */
	F_PLANT_D     = 0x00000040,	/* d_w18                Ｄ脚第一沈殿池 */
	F_PLANT_DE    = 0x00000080,	/* de_w19               ＤＥ連絡橋 */
	F_PLANT_E     = 0x00000100,	/* e_w20                Ｅ脚貨物集配施設 */
	F_PLANT_EF    = 0x00000200,	/* ef_w21               ＥＦ連絡橋 */
	F_PLANT_F     = 0x00000400,	/* f_w22                Ｆ脚倉庫 */
	F_PLANT_FA    = 0x00000800,	/* af_w23               ＦＡ連絡橋 */
	F_PLANT_S1    = 0x00001000,	/* s1_center_w24        シェル１中央棟 */
	F_PLANT_DG    = 0x00002000,	/* dg_w25               シェル１シェル２連絡橋 */
	F_PLANT_G     = 0x00004000,	/* g_w25                Ｇ脚油処理施設 */
	F_PLANT_GH    = 0x00008000,	/* gh                   ＧＨ連絡橋 */
	F_PLANT_H     = 0x00010000,	/* h                    Ｈ脚倉庫 */
	F_PLANT_HI    = 0x00020000,	/* hi                   ＨＩ連絡橋 */
	F_PLANT_I     = 0x00040000,	/* i                    Ｉ脚機材組立施設 */
	F_PLANT_IJ    = 0x00080000,	/* ij                   ＩＪ連絡橋 */
	F_PLANT_J     = 0x00100000,	/* j                    Ｊ脚発電施設 */
	F_PLANT_JK    = 0x00200000,	/* jk                   ＪＫ連絡橋 */
	F_PLANT_K     = 0x00400000,	/* k                    Ｋ脚生物化学実験施設 */
	F_PLANT_KL    = 0x00800000,	/* lk_w25               ＫＬ連絡橋 */
	F_PLANT_L     = 0x01000000,	/* l_w25                Ｌ脚汚水処理場 */
	F_PLANT_LG    = 0x02000000,	/* gl_w25               ＬＧ連絡橋 */
	F_PLANT_S2    = 0x04000000,	/* s2_center_w31        シェル２中央棟 */
	F_PLANT_L2    = 0x08000000,	/* l_w32                Ｌ脚下部オイルフェンス */
	F_PLANT_OTHER = 0x10000000,	/* other                Ｈ脚下部オイルフェンス */
};

enum
{
	F_ARSENAL_41  = 0x00000001,	/* map_41_mt           アーセナルギア胃 */
	F_ARSENAL_42  = 0x00000002,	/* map_42_mt           アーセナルギア空腸 */
	F_ARSENAL_43  = 0x00000004,	/* map_43_mt           アーセナルギア上行結腸 */
	F_ARSENAL_44  = 0x00000008,	/* map_44_mt           アーセナルギア回腸 */
	F_ARSENAL_45  = 0x00000010,	/* map_45_mt           アーセナルギアＳ状結腸 */
};

/* 各モデル */
#define TANKER_BODY  (12414107)	/* map_tanker_body      */
#define TANKER_00A   ( 1383294)	/* map_tanker_00a       船尾甲板 */
#define TANKER_01A   ( 1383326)	/* map_tanker_01a       船橋一階居住区 */
#define TANKER_01F   ( 1383331)	/* map_tanker_01f       船橋一階リフレッシュルーム */
#define TANKER_01B   ( 1383327)	/* map_tanker_01b       船橋二階居住区 */
#define TANKER_01C   ( 1383328)	/* map_tanker_01c       船橋三階居住区 */
#define TANKER_01D   ( 1383329)	/* map_tanker_01d       船橋四階居住区 */
#define TANKER_01E   ( 1383330)	/* map_tanker_01e       船橋五階操舵室 */
#define TANKER_00B   ( 1383295)	/* map_tanker_00b       航海甲板ウィング */
#define TANKER_02A   ( 1383358)	/* map_tanker_02a       機関室 */
#define TANKER_03B   ( 1383391)	/* map_tanker_03b       第二甲板右舷 */
#define TANKER_03A   ( 1383390)	/* map_tanker_03a       第二甲板左舷 */
#define TANKER_03_O  ( 5629137)	/* map_tanker_03_other  */
#define TANKER_04A   ( 1383422)	/* map_tanker_04a       第一船倉 */
#define TANKER_04B   ( 1383423)	/* map_tanker_04b       第二船倉 */
#define TANKER_04C   ( 1383424)	/* map_tanker_04c       第三船倉 */

#define PLANT_A      ( 4285016)	/* a_w12                Ａ脚ポンプ施設 */
#define PLANT_AB     ( 5333787)	/* ab_w13               ＡＢ連絡橋 */
#define PLANT_B      ( 5333594)	/* b_w14                Ｂ脚変電室 */
#define PLANT_BC     ( 6382367)	/* bc_w15               ＢＣ連絡橋 */
#define PLANT_C      ( 6382172)	/* c_w16                Ｃ脚食堂 */
#define PLANT_CD     ( 7430947)	/* cd_w17               ＣＤ連絡橋 */
#define PLANT_D      ( 7430750)	/* d_w18                Ｄ脚第一沈殿池 */
#define PLANT_DE     ( 8479527)	/* de_w19               ＤＥ連絡橋 */
#define PLANT_E      ( 8479350)	/* e_w20                Ｅ脚貨物集配施設 */
#define PLANT_EF     ( 9528129)	/* ef_w21               ＥＦ連絡橋 */
#define PLANT_F      ( 9527928)	/* f_w22                Ｆ脚倉庫 */
#define PLANT_FA     ( 9528123)	/* af_w23               ＦＡ連絡橋 */
#define PLANT_S1     ( 2177614)	/* s1_center_w24        シェル１中央棟 */
#define PLANT_DG     (10576707)	/* dg_w25               シェル１シェル２連絡橋 */
#define PLANT_G      (10576507)	/* g_w25                Ｇ脚油処理施設 */
#define PLANT_GH     (    3400)	/* gh                   ＧＨ連絡橋 */
#define PLANT_H      (     104)	/* h                    Ｈ脚倉庫 */
#define PLANT_HI     (    3433)	/* hi                   ＨＩ連絡橋 */
#define PLANT_I      (     105)	/* i                    Ｉ脚機材組立施設 */
#define PLANT_IJ     (    3466)	/* ij                   ＩＪ連絡橋 */
#define PLANT_J      (     106)	/* j                    Ｊ脚発電施設 */
#define PLANT_JK     (    3499)	/* jk                   ＪＫ連絡橋 */
#define PLANT_K      (     107)	/* k                    Ｋ脚生物化学実験施設 */
#define PLANT_KL     (14771027)	/* lk_w25               ＫＬ連絡橋 */
#define PLANT_L      (15819387)	/* l_w25                Ｌ脚汚水処理場 */
#define PLANT_LG     (15819593)	/* gl_w25               ＬＧ連絡橋 */
#define PLANT_S2     ( 2177771)	/* s2_center_w31        シェル２中央棟 */
#define PLANT_L2     (15819416)	/* l_w32                Ｌ脚下部オイルフェンス */
#define PLANT_OTHER  ( 2862361)	/* other                Ｈ脚下部オイルフェンス */

#define ARSENAL_41   (13250261)	/* map_41_mt           アーセナルギア胃 */
#define ARSENAL_42   (13283029)	/* map_42_mt           アーセナルギア空腸 */
#define ARSENAL_43   (13315797)	/* map_43_mt           アーセナルギア上行結腸 */
#define ARSENAL_44   (13348565)	/* map_44_mt           アーセナルギア回腸 */
#define ARSENAL_45   (13381333)	/* map_45_mt           アーセナルギアＳ状結腸 */
#define ARSENAL_46   ( 1130579)	/* dummy_02            アーセナルギア直腸 */

/* 位置情報 */
typedef struct
{
	FVECTOR all_point[BOMB_MAX];	/* Ｂマーク位置 */
	int     position;				/* 現在位置 */
} Pos;

/*******************************************************************************
 */

#endif	/* __INC_VR_MAP_3D__ */
