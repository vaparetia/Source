//
//	mrl.h	
//
//	2002/06/11  S.Kaneyoshi
//$Id: mrl.h,v 1.1 2002/06/11 13:10:26 usr04249 Exp $
//


// キャラクタ 
pack_all	mrl.tri \
	human/mrl_def/mrl_def_sh_mt.dar \
	human/mrl_def/mrl_def_smile_sh_mt.dar \
	human/mrl_def/mrl_dummy.dar \
	human/mrl_def/mrl_waist_mag.dar \
	human/mrl_def/mrl_waist_pack.dar


human/mrl_def	mrl_def_hair_mh.evm \
				mrl_def_mh_mt.evm \
				mrl_fc_def_mh_mt.evm \
				mrl_def_sh_mt.kms \
				mrl_def_smile_sh_mt.kms \
				mrl_dummy.kms \
				mrl_waist_mag.kms \
				mrl_waist_pack.kms


// orga.h から

	pack_all		org_def_nm.tri \
		human/org_def/org_sgl.dar \
		human/org_def/org_gbhlst.dar \
		human/org_def/org_sk_cover.dar \
		human/org_def/org_radio.dar 

	human/org_def	org_sgl.kms \
					org_gbhlst.kms \
					org_sk_cover.kms \
					org_radio.kms 

	cvd		human/org_def/org_sgl.cv2			vn
	cvd		human/org_def/org_def_sh_mt.cv2		vn

	mtn		orga.mls