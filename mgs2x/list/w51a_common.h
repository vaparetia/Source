//
//	w51a_common.h	
//	アーセナルギア上
//
//	written by H.Yoshiike 2001.May
//
//	$Id: w51a_common.h,v 1.8 2001/10/02 05:51:22 usr03682 Exp $


// c_effect用define
#define	EFFECT_CUSTUM			// c_effectのデフォルトdefineを使用しない
#define	E_KOGETUKI				// 壁への焦げ付き
#define	E_FOOTSTAMP				// 足跡
#define	E_ENEMY					// 敵兵系
#define	E_ENEMY_LIGHT			// 敵兵ライト、スティンガー、ニキータ、剣の火花、しょんべん兵
#define	E_ENEMY_N_GL_LIGHT		// 敵兵暗視ゴーグル、マンハッタンライト
#define	E_MANHATTAN_LIGHT		// マンハッタンライト
#define	E_C4BOMB				// Ｃ４爆弾、赤外線センサー
#define	E_WATER_MINE			// クロスフェード（甲板のライトで使用）＆ 水中機雷
#define	E_CROSS_FADE_LIGHT		// クロスフェード（甲板のライトで使用）
#define	E_CHARA_DENTOU			// chara 懐中電灯、フォーチュン戦
#define	E_WATER					// 水関係
#define	E_CHARA_HAMON			// chara 波紋
#define	E_HIMATU				// 水飛沫
#define	E_CHARA_MADOAME			// chara 窓雨
#define	E_STEAM					// 粉 & 水蒸気
#define	E_POWDER				// 小麦粉
#define	E_LINEAR_GUN			// リニアガン
#define	E_SUN					// 太陽
#define	E_OTHER1				// 水面、水中、リニアガン、雨、フェード、バルカン人形、ダンボール主観、濁流、ソリダス剣（予定）、カメラ前の水滴
#define	E_OTHER2				// プラズマ、剣残像、カミナリ、衝撃波、落下物水飛沫
#define	E_OTHER3				// ドッグタグ、ハリアエフェクト、ソリダスエフェクト
#define	BONBORI					// ぼんぼり


// 各ステージ共通要素
#include	"p_common.h"
#include	"common.h"
#include	"c_effect.h"

// キャラクタ
// スネーク
pack_all sna_nm.tri \
	human/sna_def/sna_def.dar \
	human/sna_shaved/sna_shaved_mt.dar \
	human/sna_def/sna_def.dar \
	human/sna_def/sna_bdn2.dar \
	human/sna_def/sna_bdn1.dar \
	human/sna_def/sna_mag1.dar \
	human/sna_def/sna_mag2.dar \
	human/sna_def/sna_mag3.dar \
	human/sna_def/sna_mag4.dar

pack_all sna_mw.tri \
	human/sna_def/sna_def_mh.dar \
	human/sna_def/sna_def_addhand_mh_mt.dar \
	human/sna_shaved/sna_shaved_mh_mt.dar

human		sna_def/sna_def_mh.evm
human		sna_def/sna_def.kms
human		sna_def/sna_bdn2.kms
human		sna_def/sna_bdn1.kms
human		sna_def/sna_mag1.kms
human		sna_def/sna_mag2.kms
human		sna_def/sna_mag3.kms
human		sna_def/sna_mag4.kms

// ソリダス
pack_all	sol_mh.tri	\
			human/sol_def/sol_def_mh_mt.dar \
			human/sol_def/sol_gantai_mh_mt.dar \
			human/sol_def/sol_snakearm_2_mh_mt.dar

pack_all	sol.tri	\
			human/sol_def/sol_gantai_obj.dar \
			goods/demo_katana_sol/demo_mst_uni_mount.dar \
			goods/demo_katana_sol/demo_mst_sht.dar \
			goods/demo_katana_sol/demo_kwt_uni_mount.dar \
			goods/demo_katana_sol/demo_kwt_sht.dar \
			human/sol_def/sol_katana_w_sdw.dar

pack_trnall	effect.tri \
			effect/bonbori/w_bonbori.bmp

human	sol_def/sol_def_mh_mt.evm
human	sol_def/sol_gantai_mh_mt.evm
human	sol_def/sol_snakearm_2_mh_mt.evm
human	sol_def/sol_katana_w_sdw.kms
goods	demo_katana_sol/demo_mst_uni_mount.kms
goods	demo_katana_sol/demo_mst_sht.kms
goods	demo_katana_sol/demo_kwt_uni_mount.kms
goods	demo_katana_sol/demo_kwt_sht.kms

// フォーチュン
pack_all	fort_mh.tri	\
			human/for_def/for_def_mh_mt.dar \
			human/for_def/for_def_addhand_mh_mt.dar \
			human/for_def/for_hair_def_mh.dar

human		for_def/for_def_mh_mt.evm
human		for_def/for_def_addhand_mh_mt.evm
human		for_def/for_hair_def_mh.evm
human		for_def/for_def_sh_mt.kms
human		for_def/for_def_bounding.kms
human		for_def/for_kizu_mh.evm
weapon		linearrifle/lnr.kms 
goods		demo_linearrifle/demo_lnr.kms
human		for_def/for_mag.kms
//フォーチュンのスリングTEX
pack_trnall trn.tri human/for_coat/for_lnr_sling.bmp

// オセロット
pack_all	rev_mh.tri	\
			human/rev_plant/rev_plant_mh_mt.dar \
			human/rev_plant/rev_plant_sh_mt.dar \
			human/rev_def/rev_def_shadow.dar \
			human/rev_coat/rev_hair_fix.dar

human		rev_plant/rev_plant_mh_mt.evm
human		rev_plant/rev_plant_sh_mt.kms
human		rev_def/rev_def_shadow.kms
human		rev_coat/rev_hair_fix.kms
human		rev_coat/rev_coat_bounding.kms
//オセロットのＳＡＡ
weapon	saa/saa_lefthand.kms

// ＲＡＹ
pack_all	rey.tri	\
			meca/ray/ray_def_mt.dar \
			meca/ray/ray_canopy_mt.dar

meca		ray/ray_def_mt.kms
meca		ray/ray_canopy_mt.kms

//手錠（スネークとライデン）
goods	demo_tejyou/demo_tjo_rai_link.kms
goods	demo_tejyou/demo_tjo_sna_link.kms

//電磁バリア装置（オセロット装備）
goods	demo_denji_varia/demo_denji_varia.kms


// ワールド
// マルチテクスチャモデル
// それ以外のモデル 
pack_trnall	effect.tri \
	effect/wave/wave06.bmp

pack_all	sky_wall_w51a.tri \
	world/common_sky/w51a_sky_wall1.dar \
	world/common_sky/w51a_sky_wall2.dar \
	world/common_sky/w51a_sky_wall3.dar \
	world/common_sky/w51a_sky_wall4.dar

pack_trnall trn.tri \
		effect/plant_sky/sky_n1_alp.bmp \
		effect/plant_sky/sky_n2_alp.bmp \
		effect/plant_sky/sky_n4_alp.bmp \
		effect/plant_sky/sky_sun_n_alp.bmp 

world	common_sky/w51a_sky_wall1.kms
world	common_sky/w51a_sky_wall2.kms
world	common_sky/w51a_sky_wall3.kms
world	common_sky/w51a_sky_wall4.kms

world		w51a
lt2			w51a.lt2
hazard		w51a.hzx


// 高波用
pack_all oil.tri \
		effect/wave/wave06.bmp \
		effect/wave/wave09_alp_ovl.bmp

//エフェクトテクスチャ
pack_trnall trn.tri effect/blood/chi01_alp.bmp
pack_trnall trn.tri effect/bakuha/bombgas6_alp.bmp
pack_trnall trn.tri effect/blood/chi01_msk.bmp
pack_trnall trn.tri effect/color/col256_bld100.bmp
pack_trnall trn.tri effect/drop/drop01_msk.bmp
