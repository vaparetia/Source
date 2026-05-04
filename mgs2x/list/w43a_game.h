// w43a_game.h
// ＡＧ・丸通路
// 2001/04/27 S.Yamashita
// $Id: w43a_game.h,v 1.35 2002/10/03 15:17:35 usr04761 Exp $

//	ステージ名表記用
#define		WN_W43A

// 各ステージ共通要素
#include	"common.h"
#include		"c_effect.h"
//include		wp_plant.h

// キャラクタ
	//	ＮＰＣスネーク
	//	髭剃り対応
pack_all sna_nm.tri \
			human/sna_def/sna_def.dar \
			human/sna_shaved/sna_shaved_mt.dar
human		sna_def/sna_def.kms

//human		sna_def/sna_def_mh.evm	デモで呼んでいる
human		sna_def/sna_bdn1.kms
human		sna_def/sna_bdn2.kms
far			sna_npc.far

weapon		usp/usp.kms
weapon		usp/usp_amo.kms
weapon		m4/m4a_nm.kms

// 強制モーション
mtn			rai_w43a.mls

// NPCスネークのモーション
mtn		'-DW32A_MOTION=DUMMY -DW17A_MOTION=DUMMY -DW14A_MOTION=DUMMY' npc_snake.mls
mtn		npc_snake_cap.mls
mtn		npc_snake_drag.mls
vib		rai/sna_npc_m4a_iya4rai_resist.vib

// ワールド
// マルチテクスチャモデル
// それ以外のモデル
pack_all	w43a_dr \
				world/door/w43a_dr_left.dar \
				world/door/w43a_dr_right.dar

pack_all	aaa.tri \
			world/w43a/w43a_ribbon

world		w43a/w43a
world		w43a/w43a_dammy
world		w43a/w43a_ribbon

lt2			w43a.lt2
hazard		w43a.hzx

world		door/w43a_dr_left.kms
world		door/w43a_dr_right.kms

//	空中文字
/*
pack_trnall	f_window.tri \
			world/w43a/w43a_ribbon/w43a_efct_rbn_01_alp_add_ovl.bmp \
			world/w43a/w43a_ribbon/w43a_efct_rbn_02_alp_add_ovl.bmp \
			world/w43a/w43a_ribbon/w43a_efct_rbn_03_alp_add_ovl.bmp \
			world/w43a/w43a_ribbon/w43a_efct_rbn_04_alp_add_ovl.bmp \
			world/w43a/w43a_ribbon/w43a_efct_rbn_05_alp_add_ovl.bmp \
			world/w43a/w43a_ribbon/w43a_efct_rbn_06_alp_add_ovl.bmp \
			world/w43a/w43a_ribbon/w43a_efct_rbn_07_alp_add_ovl.bmp \
			world/w43a/w43a_ribbon/w43a_efct_rbn_08_alp_add_ovl.bmp
*/
// オブジェクト
// 小さいサイズのテクスチャはまとめておく
// カタナ
pack_trnall	katana.tri \
			debug/shibata/staffroll/alpha03_alp_ovl.bmp

pack_all	hfb_mt.tri \
				weapon/hfb/hfb_mt.dar \
				weapon/hfb/hfb_mineuchi_mt.dar \
				weapon/hfb/hfb_sub_mt.dar \
				weapon/hfb/hfb_mineuchi_sub_mt.dar

weapon		hfb/hfb_mt.kms
weapon		hfb/hfb_mineuchi_mt.kms
weapon		hfb/hfb_sub_mt.kms
weapon		hfb/hfb_mineuchi_sub_mt.kms

mtn			rai_blade.mls

// アイテム
itembox

// ドッグタグ
pack_trnall	kirari.tri \
		effect/mark/kirari_alp_ovl.bmp
goods	dogtag/dogtag_sna.kms

//	全体マップ
l2d         2D/mapbug/mapbug.l2d // map bug

//	刀説明デモ用テクスチャ
#ifndef MGS2_XBOX
/* PS2 */
	//	PAL版
#ifdef MGS2_AREA_EU
	pack_all	padtexture.tri	\
				2D/act_telop/e \
				2D/act_telop/pal/act_telop08_fr_alp_ovl.bmp \
				2D/act_telop/pal/act_telop08_de_alp_ovl.bmp \
				2D/act_telop/pal/act_telop08_it_alp_ovl.bmp \
				2D/act_telop/pal/act_telop08_es_alp_ovl.bmp \
				2D/act_telop/pal/act_telop09_fr_alp_ovl.bmp \
				2D/act_telop/pal/act_telop09_de_alp_ovl.bmp \
				2D/act_telop/pal/act_telop09_it_alp_ovl.bmp \
				2D/act_telop/pal/act_telop09_es_alp_ovl.bmp \
				2D/act_telop/pal/act_telop10_fr_alp_ovl.bmp \
				2D/act_telop/pal/act_telop10_de_alp_ovl.bmp \
				2D/act_telop/pal/act_telop10_it_alp_ovl.bmp \
				2D/act_telop/pal/act_telop10_es_alp_ovl.bmp \
				2D/act_telop/pal/act_telop11_fr_alp_ovl.bmp \
				2D/act_telop/pal/act_telop11_de_alp_ovl.bmp \
				2D/act_telop/pal/act_telop11_it_alp_ovl.bmp \
				2D/act_telop/pal/act_telop11_es_alp_ovl.bmp

	// 韓国版
#elif defined( MGS2_KOREA )
	pack_all	padtexture.tri \
					2D/act_telop/kor/act_telop8_alp_ovl.bmp \
					2D/act_telop/kor/act_telop9_alp_ovl.bmp \
					2D/act_telop/kor/act_telop10_alp_ovl.bmp \
					2D/act_telop/kor/act_telop11_alp_ovl.bmp

	// アメリカ版
#elif defined( MGS2_AREA_US )
	pack_all	padtexture.tri 2D/act_telop/e

#else
	//	日本版
	pack_all	padtexture.tri	\
				2D/act_telop/e/act_telop8_alp_ovl.bmp \
				2D/act_telop/e/act_telop9_alp_ovl.bmp \
				2D/act_telop/e/act_telop10_alp_ovl.bmp \
				2D/act_telop/e/act_telop11_alp_ovl.bmp \
				2D/act_telop/j/act_telop08_jp_alp_ovl.bmp \
				2D/act_telop/j/act_telop09_jp_alp_ovl.bmp \
				2D/act_telop/j/act_telop10_jp_alp_ovl.bmp \
				2D/act_telop/j/act_telop11_jp_alp_ovl.bmp 


#endif
#else
/* XBOX */
#ifdef MGS2_AREA_US
	// アメリカ版
	pack_all	padtexture.tri 2D/act_telop/x_eng
#endif
#ifdef MGS2_AREA_EU
	// ヨーロッパ版
	pack_all	padtexture.tri	\
				2D/act_telop/x_eng/act_telop8_alp_ovl.bmp \
				2D/act_telop/x_eng/act_telop9_alp_ovl.bmp \
				2D/act_telop/x_eng/act_telop10_alp_ovl.bmp \
				2D/act_telop/x_eng/act_telop11_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop08_fr_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop08_de_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop08_it_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop08_es_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop09_fr_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop09_de_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop09_it_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop09_es_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop10_fr_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop10_de_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop10_it_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop10_es_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop11_fr_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop11_de_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop11_it_alp_ovl.bmp \
				2D/act_telop/x_pal/act_telop11_es_alp_ovl.bmp
#endif
#endif

// デバッグ用



// リネーム


//スネーク用Ｍ４
weapon/m4	m4a_nm.kms
weapon/usp	usp.kms


// 裸ライデン
// ライデン

pack_all	rai_n_tex_mt.tri \
            human/rai_naked/rai_naked_mt.dar \
			human/rai_naked/rai_naked_sh_mt.dar

pack_all 	rai_tex_n_nm.tri \
            human/rah_raiden_hand/rah_def_dummy.dar 
	
pack_all	rai_tex_n_mw.tri \
//			human/rah_raiden_hand/rah_def_mh_mt.dar
			human/rah_raiden_hand/rah_naked_mh_mt.dar

//	裸モデル
human		rai_naked/rai_naked_mt.kms
//	ハイポリ
human		rai_naked/rai_naked_sh_mt.kms

// ドッグタグ（ほんもの）
human		rai_naked/rai_dogtag_naked_mh.evm
//	ドッグタグバウンディングボックス
human		rai_naked/rai_dogtag_bounding.kms

// 主観腕
//human	    rah_raiden_hand/rah_def_mh_mt.evm
human		rah_raiden_hand/rah_naked_mh_mt.evm
human		rah_raiden_hand/rah_def_dummy.kms

cvd			human/rai_naked/rai_naked_mt.cv2
cvd			human/rai_naked/rai_naked_sh_mt.cv2


// モーション
mtn			rai_naked.mls
mtn			rai_arm_mh.mls

//	パッドデモ
rpd			w43a_paddemo.rpd

// リネーム
rename		rai_naked_mt.kms	rai_naked.kms
rename		rai_naked_mt.cv2	rai_naked.cv2

rename		rai_naked_sh_mt.kms		rai_naked_sh.kms
rename		rai_naked_sh_mt.cv2		rai_naked_sh.cv2

