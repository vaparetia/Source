//
//	w11ab_common.h
//
//	written by H.Yoshiike 2000.Aug
//
//	$Id: w11ab_common.h,v 1.18 2002/09/11 11:57:20 usr03682 Exp $


// 特殊ゲームオーバーPICT
#include	"goverpic.h"

// 強制モーション
mtn		rai_w11a.mls			// ステージ固有
mtn		rai_wt_hatch.mls		// 水密ドア開閉モーション
mtn		hatch_door_sna.mls 		// 水密ドア扉開閉モーション
mtn		hatch_handle_sna.mls	// 水密ドアハンドル回転モーション
mtn		sna_locker.mls			// ロッカー開閉モーション
mtn		locker.mls				// ロッカー扉開閉モーション
mtn		rai_swim.mls			// 泳ぎモーション

// 追加エフェクト
// 水面照り返し
// テクスチャアニメデータ
anmtex	w11.rat \
	effect/w11_hakei

// 気泡
//debug	kimura/temp/scr_kihou03.kms
//cvd		debug/kimura/temp/scr_kihou03.cv2 vn
effect	scr_effect/scr_kihou01.kms
cvd		effect/scr_effect/scr_kihou01.cv2 vn

// ワールド
// マルチテクスチャモデル
pack_all	w11a_mt.tri world/w11a/w11a0/w11a0_speobj2_mt.dar
// それ以外のモデル
option makexti6 -l 1024 -f PAL8
pack_all	w11a.tri world/w11a/w11a0 world/w11a/w11a1
option makexti6
pack_all	water.tri world/w11a/w11a0_water/w11a0_ref_water_alp_ovl.bmp

world		w11a/w11a0
world		w11a/w11a1

// オブジェ関係
// 小さいサイズのテクスチャはまとめておく
pack_all	object.tri \
	world/door/w11a_sdr_body_l.dar \
	world/door/w11a_sdr_handle.dar \
	world/door/dr_free.dar \
	goods/locker/neo_locker_door.dar \
	goods/c4_kaitai_b

world		door/dr_free.kms
world		door/w11a_sdr_body_l.kms
world		door/w11a_sdr_handle.kms
goods		locker/neo_locker_door.kms
goods		c4_kaitai_b

// 水膜
effect	scr_effect/scr_waterline01.kms
cvd		effect/scr_effect/scr_waterline01.cv2 vn

// 主観カメラ水滴
debug		shibata/awa_model/awa_model2.kms
cvd			debug/shibata/awa_model/awa_model2.cv2

// 振動データ
vib		etc/dock_ele_01.vib
vib		etc/dock_ele_02.vib
vib		etc/dock_ele_03.vib

// 仮置き
// 水飛沫用cv2
cvd		human/sna_skl/sna_skl3.cvd vn




// リネーム
