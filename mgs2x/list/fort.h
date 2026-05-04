//
//	fort.h	
//
//	2000/01/31  T.Morita
//       $Id: fort.h,v 1.8 2001/04/04 02:39:33 usr04098 Exp $
//

pack_trnall  transparancy.tri \
	human/for_coat/for_lnr_sling.bmp

// テクスチャ重複を回避するために、フォーチュンを一つに
pack_all for_def.tri \
	human/for_def/for_def_sh_mt.dar \
	human/for_coat/for_parts_lsode_mh.dar \
	human/for_coat/for_parts_rsode_mh.dar \
	human/for_coat/for_parts_coat_mh.dar \
	human/for_coat/for_hair_coat_mh.dar

pack_all for_def_weap.tri \
	human/for_def/for_mag.dar \
	weapon/linearrifle

human for_def/for_def_sh_mt.kms
human for_def/for_mag.kms
human/for_coat  for_parts_lsode_mh.evm \
		for_parts_rsode_mh.evm \
		for_parts_coat_mh.evm \
		for_hair_coat_mh.evm \
		for_coat_mh_mt.evm \
		for_coat_bounding.kms 

mtn     fort.mls
weapon	linearrifle
cvd	weapon/linearrifle/lnr.cv2 vn
