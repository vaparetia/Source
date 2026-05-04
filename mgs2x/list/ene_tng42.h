//
//	ene_tng42.h	
//
// $Id: ene_tng42.h,v 1.3 2002/05/13 09:07:55 usr03379 Exp $
// ダンボール発見モーションを追加した天狗兵　ｗ42用

pack_trnall katana_eft.tri \
	debug/shibata/staffroll/alpha03_alp_ovl.bmp
human	tng_def/tng_high.kms
human	tng_def/tng_saya.kms
human	tng_def/tng_magazine.kms
human	tng_def/tng_strap.kms
cvd		human/tng_def/tng_high.cv2 vn



weapon/p90
weapon	tbl/tbl_katana.kms
item	sougan 



mtn	gbs.mls '-DDEL_SHIELD=DUMMY -DDEL_SHOTGUN=DUMMY \
	-DDEL_WALKMAN=DUMMY \
	-DDEL_EROTICA=DUMMY \
	-DDEL_PARTS=DUMMY \
	-DDEL_TEST=DUMMY'
//	-DDEL_TNG=DUMMY \
//	-DDEL_RADIO=DUMMY \
//	-DDEL_BOX=DUMMY \
//	-DDEL_GRD=DUMMY \ 
//	-DDEL_NEAR_ATTACK=DUMMY \
//	-DDEL_HANG=DUMMY \
//	-DDEL_CARRY=DUMMY \



