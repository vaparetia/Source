//
//	orga.h	
//
//	1999/12/18  T.Morita
//       $Id: orga_demo.h,v 1.2 2000/10/31 02:10:26 usr04098 Exp $
//

// マルチウェイト
human	org_def/org_def.evm

// 共有モデル

// 上記以外のノーマルモデル
pack_all		org_def_nm.tri \
	human/org_def/org_gbhlst.dar \
	human/org_def/org_sk_cover.dar \
	human/org_def/org_radio.dar 

// キャラクタ 
human/org_def	org_def.evm \
				org_gbhlst.kms \
				org_sk_cover.kms \
				org_radio.kms 

cvd		human/org_def/org_sgl.cv2			vn

