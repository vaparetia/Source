//
//	sna_txd.h
//
//	written by H.Yoshiike 2002.Mar
//  $Id$
//

// スネーク

// マルチウェイト
pack_all	sna_def_mw.tri \
			human/snh_txd_hands/snh_txd_mh_mt.dar \
			human/sna_txd/sna_txd_suso_mh_mt.dar

// （シングルウェイト）マルチテクスチャ
pack_all	sna_txd_mt.tri \
			human/sna_txd/sna_txd_mt.dar \
			human/sna_txd/sna_txd_sh_mt.dar
human		sna_txd/sna_txd_mt.kms
human		sna_txd/sna_txd_sh_mt.kms
human		sna_txd/sna_txd_suso_mh_mt.evm
human		sna_txd/sna_txd_bounding.kms
//ＫＭＳ版すそ ポリゴン分解エフェクト用
human		sna_txd/sna_txd_suso_sh_mt.kms


// 共有モデル（co_モデル）
//pack_all	sna_txd_cm.tri \

// 上記以外のノーマルモデル

pack_all	sna_txd_nm.tri \
    human/snh_snake_hands/snh_def_dummy.dar \
    debug/shibata/brk_screen/brk_scr07_full.dar

// 主観時の腕
human		snh_txd_hands/snh_txd_mh_mt.evm
human		snh_snake_hands/snh_def_dummy.kms

// 影
goods		shadow
human		sna_txd/sna_txd_shadow.kms

#ifdef MGS2_XBOX_1
pack_trnall	effect.tri effect/shdw/foot_shadow_alp.bmp  /*XBOX用の足影*/
#endif

//主観カメラわれ
debug		shibata/brk_screen/brk_scr07_full.kms

// 頂点データ
// ローポリシングルウエイトスネーク用
cvd		human/sna_txd/sna_txd_mt.cv2
// ハイポリシングルウエイトスネーク用
cvd		human/sna_txd/sna_txd_sh_mt.cv2


// モーション

// スネーク常駐モーション
mtn		snake.mls
// 主観時の腕のモーション
mtn		sna_arm_mh.mls

// リネーム
rename		sna_txd_mt.kms sna_def.kms
rename		sna_txd_mt.cv2 sna_def.cv2
rename		sna_txd_sh_mt.kms sna_def_sh.kms
rename		sna_txd_sh_mt.cv2 sna_def_sh.cv2
rename		snh_txd_mh_mt.evm snh_def_mh_mt.evm
rename		sna_txd_shadow.kms sna_shadow.kms
