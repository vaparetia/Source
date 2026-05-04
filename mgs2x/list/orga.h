//
//	orga.h	
//
//	1999/12/18  T.Morita
//       $Id: orga.h,v 1.23 2001/06/28 10:34:07 usr01098 Exp $
//

// テクスチャ重複を回避するために、オルガを一つに
pack_all	org_def_mw_mt.tri \
//	human/org_def/org_def.dar \
	human/org_def/org_def_sh_mt.dar

// マルチウェイト
//pack_all        m_org_def_mw.tri \
//	human/org_def/org_def.dar
//human	org_def/org_def.evm 

//（シングルウェイト）マルチテクスチャ
//human	org_def/org_def_sh_mt.kms

// 共有モデル

// 上記以外のノーマルモデル
pack_all		org_def_nm.tri \
	human/org_def/org_sgl.dar \
	human/org_def/org_gbhlst.dar \
	human/org_def/org_sk_cover.dar \
	human/org_def/org_radio.dar 

// キャラクタ 
human/org_def	org_def.evm \
				org_sgl.kms \
				org_def_sh_mt.kms \
				org_gbhlst.kms \
				org_sk_cover.kms \
				org_radio.kms 

cvd		human/org_def/org_sgl.cv2			vn
cvd		human/org_def/org_def_sh_mt.cv2		vn

// モーション
mtn		orga.mls
