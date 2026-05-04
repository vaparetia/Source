//
//	sna_mgs.h
//
//	written by H.Yoshiike 2002.Mar
//  $Id$
//

// スネーク

// マルチウェイト
pack_all	sna_def_mw.tri \
			human/snh_snake_hands/snh_oss_mh_mt.dar

// （シングルウェイト）マルチテクスチャ
pack_all	sna_mgs_mt.tri \
			human/sna_oss/sna_oss_mt.dar \
			human/sna_oss/sna_oss_sh_mt.dar
human		sna_oss/sna_oss_mt.kms
human		sna_oss/sna_oss_sh_mt.kms

// 共有モデル（co_モデル）
//pack_all	sna_oss_cm.tri \

// 上記以外のノーマルモデル
pack_all	sna_oss_nm.tri \
	human/sna_def/sna_bdn1.dar human/sna_def/sna_bdn2.dar \
	human/sna_def/sna_mugen_bdn1.dar human/sna_def/sna_mugen_bdn2.dar \
    human/snh_snake_hands/snh_def_dummy.dar \
    human/sna_def/sna_shadow.dar \
    debug/shibata/brk_screen/brk_scr07_full.dar

// バンダナ
human		sna_def/sna_bdn1.kms
human		sna_def/sna_bdn2.kms
human		sna_def/sna_mugen_bdn1.kms
human		sna_def/sna_mugen_bdn2.kms

// 主観時の腕
human		snh_snake_hands/snh_oss_mh_mt.evm
human		snh_snake_hands/snh_def_dummy.kms

// 影
goods		shadow
human		sna_oss/sna_oss_shadow.kms
#ifdef MGS2_XBOX_1
pack_trnall	effect.tri effect/shdw/foot_shadow_alp.bmp  /*XBOX用の足影*/
#endif

//主観カメラわれ
debug		shibata/brk_screen/brk_scr07_full.kms

// 頂点データ
// ローポリシングルウエイトスネーク用
cvd		human/sna_oss/sna_oss_mt.cv2
// ハイポリシングルウエイトスネーク用
cvd		human/sna_oss/sna_oss_sh_mt.cv2


// モーション

// スネーク常駐モーション
mtn		snake.mls
// 主観時の腕のモーション
mtn		sna_arm_mh.mls

// リネーム
rename		sna_oss_mt.kms sna_def.kms
rename		sna_oss_mt.cv2 sna_def.cv2
rename		sna_oss_sh_mt.kms sna_def_sh.kms
rename		sna_oss_sh_mt.cv2 sna_def_sh.cv2
rename		snh_oss_mh_mt.evm snh_def_mh_mt.evm
rename		sna_oss_shadow.kms sna_shadow.kms
