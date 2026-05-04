//
//	a45a_game.h（アナザーミッション用）
//
//	written by H.Yoshiike 2000.Aug
//
//	$Id: a45a_game.h,v 1.5 2002/06/04 10:04:31 usr03379 Exp $

//	ステージ名表記用
#define		WN_W45A
// アナザーミッション明示
#define		WN_ANOTHER

// 各ステージ共通要素
#include		"p_common.h"
#include		"common.h"
#include		"c_effect.h"
//include		wp_plant.h

//	オルタナティブ追加エフェクト
#include	"alt_effect.h"

// キャラクタ 
	//	天狗兵
include		ene_tng.h
	//	NPCスネーク
	//	髭剃り対応
pack_all sna_nm.tri \
			human/sna_def/sna_def.dar \
			human/sna_shaved/sna_shaved_mt.dar
human		sna_def/sna_def.kms

	//	髭剃り対応
pack_all sna_mw.tri \
			human/sna_def/sna_def_mh.dar \
			human/sna_shaved/sna_shaved_mh_mt.dar
human		sna_def/sna_def_mh.evm

human		sna_def/sna_bdn1.kms
human		sna_def/sna_bdn2.kms
far			sna_npc.far

//human		for_def

// 強制モーション
mtn		rai_a45a.mls
mtn		sna_a45a.mls
//mtn		sna_w45a.mls			// 仮置き用スネーク

// NPCスネークのモーション
mtn		'-DW32A_MOTION=DUMMY -DW17A_MOTION=DUMMY -DW14A_MOTION=DUMMY' npc_snake.mls
mtn		npc_snake_cap.mls
mtn		npc_snake_drag.mls

vib		rai/sna_npc_m4a_iya4rai_resist.vib

// ワールド
// それ以外のモデル 
pack_all	w45a_dr.tri \
			world/door/w45a_dr_left_oku.dar \
			world/door/w45a_dr_right_oku.dar \
			world/door/w45a_dr_left_temae.dar \
			world/door/w45a_dr_right_temae.dar


world		w45a/w45a
world		another/a45a	//	アナザー用ロードモデル

lt2			w45a.lt2
hazard		a45a.hzx

// オブジェ関係
// 小さいサイズのテクスチャはまとめておく
//world		door/w01f_dr.kms
world		door/w45a_dr_left_oku.kms
world		door/w45a_dr_right_oku.kms
world		door/w45a_dr_left_temae.kms
world		door/w45a_dr_right_temae.kms

//	空中文字
pack_trnall	f_window.tri world/w45a/other/w45a_efct_rbn_01_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_rbn_02_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_rbn_smt01_2_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_rbn_smt01_3_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_rbn_smt01_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_01_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_02_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_03_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_04_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_05_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_06_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_01_scr_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_02_scr_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_03_scr_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_04_scr_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_05_scr_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_caution_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_01_noiz_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_02_noiz_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_03_noiz_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_04_noiz_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_05_noiz_alp_add_ovl.bmp \
			world/w45a/other/w45a_efct_disp_06_noiz_alp_add_ovl.bmp


//カタナ
pack_all	hfb_mt.tri \
				weapon/hfb/hfb_mt.dar \
				weapon/hfb/hfb_mineuchi_mt.dar \
				weapon/hfb/hfb_sub_mt.dar \
				weapon/hfb/hfb_mineuchi_sub_mt.dar

weapon		hfb/hfb_mt.kms
weapon		hfb/hfb_mineuchi_mt.kms
weapon		hfb/hfb_sub_mt.kms
weapon		hfb/hfb_mineuchi_sub_mt.kms
mtn		rai_blade.mls

//スネーク用Ｍ４
weapon/m4	m4a_nm.kms
weapon/usp	usp.kms


// デバッグ用
// 小さいサイズのテクスチャはまとめておく

// アイテム
itembox

// ドッグタグ
pack_trnall	kirari.tri \
		effect/mark/kirari_alp_ovl.bmp
goods	dogtag/dogtag_sna.kms

//	全体マップ
l2d         2D/mapbug/mapbug.l2d // map bug

//偽ゲームオーバー画面
pack_all	fake_gameover.tri \
			2D/game_over/fake_failed_alp_ovl.bmp \
			2D/game_over/fake_exit_alp_ovl.bmp \
			2D/game_over/fake_cont_alp_ovl.bmp


// リネーム
