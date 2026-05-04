//
//	w46a_game.h
//
//	written by H.Yoshiike 2000.Aug
//
//	$Id: w46a_game.h,v 1.29 2002/07/16 04:18:09 usr02774 Exp $


//	ステージ名表記用
#define		WN_W46A
#define	E_SKY_NORMAL			// 空

// 各ステージ共通要素
#include		"p_common.h"
#include		"common.h"
#include		"c_effect.h"
//include		wp_plant.h

// キャラクタ 

// 強制モーション
mtn		rai_w46a.mls

// 量産メタル実験モーション
//mtn		w46a_metal.mls
mtn		pdray.mls

//	空
pack_trnall effect.tri \
	effect/wave/wave12_alp_ovl.bmp

// RAY's effect
pack_trnall effect.tri \
		effect/wind/wind01.bmp \
		effect/blood/blood_1e_msk.bmp \
		effect/pdray_effect/pdray_eye_hl_alp.bmp \
		effect/pdray_effect/pdray_eye_noise_alp.bmp \
		effect/smoke/smoke_lp3_alp.bmp \
		effect/smoke/smoke_lp4_alp.bmp \
		effect/splash/splash06_alp.bmp \
		effect/drop/drop01_msk.bmp \
		effect/weapon_effect/muzzlefrash_01_alp.bmp

// ワールド
// それ以外のモデル 
world		w46a
lt2			w46a.lt2
hazard		w46a.hzx


// オブジェ関係
// 小さいサイズのテクスチャはまとめておく
debug 		kimura/ryousan_metal
meca		pdray/pdray_def_mt.kms
cvd			meca/pdray/pdray_def_mt.cv2
pack_all	pdray_other.tri \
				meca/pdray/pdray_standby_lod.dar \
				meca/pdray/pdray_out_p_lod.dar \
				meca/pdray/pdray_kneemsl.dar \
				meca/pdray/pdray_kneemsl_r.dar \
				meca/pdray/pdray_kneemsl_l.dar \
				meca/pdray/pdray_backmsl.dar \
				meca/pdray/pdray_backmsl_open.dar \
				meca/pdray/pdray_backmsl_close.dar \
				meca/pdray/pdray_landmine.dar \
				meca/pdray/pdray_number_dummy.dar \
				meca/pdray/pdray_number_a01_r.dar \
				meca/pdray/pdray_number_a01_l.dar \
				meca/pdray/pdray_number_a02_r.dar \
				meca/pdray/pdray_number_a02_l.dar \
				meca/pdray/pdray_number_a03_r.dar \
				meca/pdray/pdray_number_a03_l.dar \
				meca/pdray/pdray_number_a04_r.dar \
				meca/pdray/pdray_number_a04_l.dar \
				meca/pdray/pdray_number_a05_r.dar \
				meca/pdray/pdray_number_a05_l.dar \
				meca/pdray/pdray_number_b01_r.dar \
				meca/pdray/pdray_number_b01_l.dar 
meca		pdray/pdray_standby_lod.kms
meca		pdray/pdray_out_p_lod.kms
meca		pdray/pdray_kneemsl.kms
meca		pdray/pdray_kneemsl_r.kms
meca		pdray/pdray_kneemsl_l.kms
meca		pdray/pdray_backmsl.kms
meca		pdray/pdray_backmsl_open.kms
meca		pdray/pdray_backmsl_close.kms
meca		pdray/pdray_landmine.kms
meca		pdray/pdray_number_dummy.kms
meca		pdray/pdray_number_a01_r.kms
meca		pdray/pdray_number_a01_l.kms
meca		pdray/pdray_number_a02_r.kms
meca		pdray/pdray_number_a02_l.kms
meca		pdray/pdray_number_a03_r.kms
meca		pdray/pdray_number_a03_l.kms
meca		pdray/pdray_number_a04_r.kms
meca		pdray/pdray_number_a04_l.kms
meca		pdray/pdray_number_a05_r.kms
meca		pdray/pdray_number_a05_l.kms
meca		pdray/pdray_number_b01_r.kms
meca		pdray/pdray_number_b01_l.kms 

// アイテム
itembox

//	カタナ
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
mtn		rai_blade.mls

//	全体マップ
l2d         2D/mapbug/mapbug.l2d // map bug

//	ボスラッシュ用
gcx			boss

// リネーム

// デバッグ用
// 小さいサイズのテクスチャはまとめておく
