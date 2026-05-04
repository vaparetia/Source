//
//	alt_effect.h	
//	オルタナティブミッションで使用する追加エフェクト
//	written by H.Yoshiike 2002.Mar
//
//	$Id$

// ゴールポスト
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
			effect_vr/nametag/nametag_wall_def_add_alp.bmp

// ＶＲ出現エフェクト レンズフレアを利用 通常はc_effect.hで呼ばれているはず
pack_trnall	flare.tri \
			effect/flare/flare_b1_msk.bmp /* 2001.05.31 向手追加/レンズフレア */ \
			effect/flare/flare_b2_msk.bmp \
			effect/flare/flare_g1_msk.bmp \
			effect/flare/flare_g2_msk.bmp \
			effect/flare/flare_r1_msk.bmp \
			effect/flare/flare_r2_msk.bmp


meca		vr2_goalpost/vr2_goalpost.kms
cvd			meca/vr2_goalpost/vr2_goalpost.cv2 v 

// タイマー用L2D
#ifdef MGS2_VRTRIAL
l2d		2D/vr_score/vr_system.l2d
#else
l2d		2D/vr_score/vr_system_five.l2d
#endif

// ＶＲウィンドウ用L2D
l2d		2D/vr_window/vr_window.l2d 

// ＶＲクリア用L2D
l2d		2D/vr_clear/vr_clear_alt.l2d
l2d		2D/vr_dcm/vr_dcm.l2d //フォーカス

// ポーズ時のL2D
l2d		2D/vr_pause_map/vr_pause_map.l2d

// デバッグ用
// 小さいサイズのテクスチャはまとめておく


// リネーム
