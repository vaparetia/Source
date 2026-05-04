//
//	sleep_orga.h (w00c専用)
//
//	1999/12/18  T.Morita
//       $Id: orga_slp.h,v 1.6 2001/05/24 02:49:08 usr04249 Exp $
//

// マルチウェイト
//pack_all		org_slp_mw.tri \
//	human/org_def/org_def_sleepface.dar
human	org_def/org_def_sleepface.evm

//human	org_def/org_def_sleepface_sh_mt.kms

//（シングルウェイト）マルチテクスチャ
// 共有モデル

// 上記以外のノーマルモデル
pack_all		org_def_nm.tri \
	human/org_def/org_sgl.dar \
	human/org_def/org_gbhlst.dar \
	human/org_def/org_sk_cover.dar \
	human/org_def/org_radio.dar

// キャラクタ
//human	org_def/org_def_sleepface.evm \
human/org_def	org_sgl.kms \
				org_gbhlst.kms \
				org_sk_cover.kms \
				org_radio.kms

cvd		human/org_def/org_sgl.cv2         vn

// モーション
//mtn		slp_orga
