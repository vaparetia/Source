//
//	iro_pli.h
//
//	written by H.Yoshiike 2002.Mar
//  $Id$
//

// スネーク

// マルチウェイト
pack_all	iro_def_mw.tri \
    human/irh_iroquois_hands/irh_def_mh_mt.dar

// （シングルウェイト）マルチテクスチャ
pack_all	iro_def_mt.tri \
			human/iro_def/iro_def_mt.dar \
			human/iro_def/iro_def_sh_mt.dar \
			human/iro_def/iro_mag.dar
human		iro_def/iro_def_mt.kms
human		iro_def/iro_def_sh_mt.kms
human		iro_def/iro_mag.kms

// 共有モデル（co_モデル）
//pack_all	iro_def_cm.tri \

// 上記以外のノーマルモデル

pack_all	iro_def_nm.tri \
    human/snh_snake_hands/snh_def_dummy.dar \
    human/sna_def/sna_shadow.dar \
    debug/shibata/brk_screen/brk_scr07_full.dar

// 主観時の腕
human		irh_iroquois_hands/irh_def_mh_mt.evm
human		snh_snake_hands/snh_def_dummy.kms

// 影
goods		shadow
human		iro_def/iro_shadow.kms
#ifdef MGS2_XBOX_1
pack_trnall	effect.tri effect/shdw/foot_shadow_alp.bmp  /*XBOX用の足影*/
#endif

//主観カメラわれ
debug		shibata/brk_screen/brk_scr07_full.kms

// 頂点データ
// ローポリシングルウエイトスネーク用
cvd		human/iro_def/iro_def_mt.cv2
// ハイポリシングルウエイトスネーク用
cvd		human/iro_def/iro_def_sh_mt.cv2


// モーション

// スネーク常駐モーション
mtn		snake.mls
// 主観時の腕のモーション
mtn		sna_arm_mh.mls

// リネーム
rename		iro_def_mt.kms sna_def.kms
rename		iro_def_mt.cv2 sna_def.cv2
rename		iro_def_sh_mt.kms sna_def_sh.kms
rename		iro_def_sh_mt.cv2 sna_def_sh.cv2
rename		irh_def_mh_mt.evm snh_def_mh_mt.evm
rename		iro_shadow.kms sna_shadow.kms
