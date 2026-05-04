//
//	mrl_a02a.h
//
//	2002/05/31	S.Kaneyoshi
//	$Id: mrl_a02a.h,v 1.1 2002/05/31 08:54:29 usr04249 Exp $
//

// マルチウェイト

//（シングルウェイト）マルチテクスチャ

pack_all org_mt.tri human/org_def/org_def_sh_mt.dar human/org_def/org_def.dar
human	org_def/org_def_sh_mt.kms
human	org_def/org_def.evm



debug	toyota/demo_usp.kms



// 共有モデル

// 上記以外のノーマルモデル
pack_all		org_def_nm.tri \
	human/org_def/org_sgl.dar \
	human/org_def/org_gbhlst.dar \
	human/org_def/org_sk_cover.dar \
	human/org_def/org_radio_forhand.dar
//	human/org_def/org_hair_back.dar \
//	human/org_def/org_hair_front.dar \
//	goods/demo_goods/demo_org_hut.dar

// キャラクタ
human/org_def	org_sgl.kms \
//				org_def_sh_mt.kms \
				org_gbhlst.kms \
				org_sk_cover.kms \
				org_radio_forhand.kms
//				org_hair_front.kms \
//				org_hair_back.kms \
//				org_hair_back.kms

cvd		human/org_def/org_sgl.cv2			vn
//cvd		human/org_def/org_def_sh_mt.cv2		vn
//cvd		human/org_def/org_hair_front.cv2	v
//cvd		human/org_def/org_hair_back.cv2		v

// 帽子
goods	demo_goods/demo_org_hut.kms

// モーション
	//mtn			mrl_sp06a.mls
	//mtn				orga.mls
	mtn		corps_org_w00c.mls

human		mrl_def/mrl_def_smile_sh_mt.kms

