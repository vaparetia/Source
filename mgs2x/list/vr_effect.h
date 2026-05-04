//
//	vr_effect.h	
//	ＶＲステージで使用する追加エフェクト
//	written by H.Yoshiike 2002.Mar
//
//	$Id$

//ゴールポスト
// 10247211 
pack_trn	vreffect_trn.tri \
			effect_vr/lockon/lockon_tag1_add_alp.bmp \
			effect_vr/lockon/lockon_octa_add_alp.bmp \
			effect_vr/noise \
			effect_vr/nametag/nametag_octa_def_add_alp.bmp \
			effect_vr/nametag/nametag_cube_def_add_alp.bmp \
			effect_vr/nametag/nametag_square_def_add_alp.bmp \
			effect_vr/nametag/nametag_straw_def_add_alp.bmp \
			effect_vr/nametag/nametag_hexa_def_add_alp.bmp \
			effect_vr/nametag/nametag_triangle_def_add_alp.bmp \
			effect_vr/nametag/nametag_wall_def_add_alp.bmp \
			effect_vr/nametag/nametag_gbs1_def_add_alp.bmp


meca		vr2_goalpost/vr2_goalpost.kms



//	ＶＲ空
pack_trnall	effect.tri \
			effect/plant_sky/sky_m1_alp.bmp \
			effect/plant_sky/sky_m2_alp.bmp \
			effect/plant_sky/sky_m4_alp.bmp \
			effect/plant_sky/sky_sun_m_alp.bmp

effect_vr	tenkyu/vs_sky_wall.kms
effect_vr	tenkyu/vr2_tenkyu.kms


//	床と壁のマーカー
pack_trn	vr_marker.tri \
			effect_vr/hzd_marker/floor_marker1_add_alp.bmp \
			effect_vr/hzd_marker/floor_marker2_add_alp.bmp \
			effect_vr/hzd_marker/wall_marker1_add_alp.bmp \
			effect_vr/hzd_marker/wall_marker2_add_alp.bmp 



// ＶＲ用追加エフェクト
// 弾痕
pack_trnall	effect.tri \
			effect_vr/dankon/vr2_dankon1_add_alp.bmp \
			effect_vr/dankon/vr2_dankon2_add_alp.bmp

// 跳弾
effect_vr	fragment/vr2_frg1_cm.kms
effect_vr	fragment/vr2_frg2_cm.kms
effect_vr	fragment/vr2_frg3_cm.kms
effect_vr	fragment/vr2_frg4_cm.kms
effect_vr	fragment/vr2_frg5_cm.kms

// 足跡
pack_trnall	effect.tri \
			effect_vr/footstamp/vr2_footstamp1_add_alp.bmp \
			effect_vr/wetpanel/vr2_wetpanel_alp.bmp

// 血
pack_trnall	effect.tri \
			effect_vr/blood/vr2_ketchap00_alp.bmp \
			effect_vr/blood/vr2_chi01_alp.bmp \
			effect_vr/blood/vr2_chi02_alp.bmp \
			effect_vr/blood/vr2_chi03_alp.bmp \
			effect_vr/blood/vr2_chi04_alp.bmp \
			effect_vr/blood/vr2_chi05_alp.bmp

// ＶＲ出現エフェクト レンズフレアを利用 通常はc_effect.hで呼ばれているはず
pack_trnall	flare.tri \
			effect/flare/flare_b1_msk.bmp /* 2001.05.31 向手追加/レンズフレア */ \
			effect/flare/flare_b2_msk.bmp \
			effect/flare/flare_g1_msk.bmp \
			effect/flare/flare_g2_msk.bmp \
			effect/flare/flare_r1_msk.bmp \
			effect/flare/flare_r2_msk.bmp

// タイマー用L2D
#ifdef MGS2_VRTRIAL
l2d		2D/vr_score/vr_system.l2d
#else
l2d		2D/vr_score/vr_system_five.l2d
#endif

// ＶＲウィンドウ用L2D
#ifdef MGS2_VRTRIAL
l2d		2D/vr_window_trial/vr_window_trial.l2d 
#else
l2d		2D/vr_window/vr_window.l2d 
#endif

// ＶＲクリア用L2D
#ifdef MGS2_VRTRIAL
l2d		2D/vr_clear_trial/vr_clear_trial.l2d
#else
l2d		2D/vr_clear/vr_clear.l2d
#endif
l2d		2D/vr_dcm/vr_dcm.l2d //フォーカス

// ポーズ時のL2D
l2d		2D/vr_pause/vr_pause.l2d


// 各ステージ共通lt2ファイル
lt2			vr_morning.lt2
lt2			vr_evening.lt2
lt2			vr_night.lt2

// デバッグ用
// 小さいサイズのテクスチャはまとめておく


// リネーム
