//
//	marine_t12a1D_w04c.h
//
//	written by H.Yoshiike 1999.Oct
//
//	$Id: marine.h,v 1.12 2001/08/05 12:01:19 usr01749 Exp $


// （シングルウェイト）マルチテクスチャ
pack_all	hold_marine_mt.tri \
	human/us_parts/obj_glass.dar \
	human/us_parts/obj_helmet_def_s.dar \
	human/us_parts/obj_helmet_low_s.dar \
	human/us_parts/obj_helmet_mid_s.dar \
	human/us_parts/us_def_5_s.dar \
	human/us_parts/us_low_5_s.dar \
	human/us_parts/us_mid_5_s.dar

human/us_parts	obj_glass.kms \
				obj_helmet_def_s.kms \
				obj_helmet_low_s.kms \
				obj_helmet_mid_s.kms \
				us_def_5_s.kms \
				us_def_pants.kms \
				us_low_5_s.kms \
				us_mid_5_s.kms \
				obj_headphone_def.kms

arkms		w04a_us_def_2.kls
weapon		m4


goods	field_camera


// リネーム
rename		w04a_us_def_2.kms	w04a_us_def.kms

mtn			ushold.mls \
	'-DDEL_SHIELD=DUMMY \
	-DDEL_SHOTGUN=DUMMY \
	-DDEL_HIGH=DUMMY \
	-DDEL_TNG=DUMMY \
	-DDEL_SHOUBEN=DUMMY \
	-DDEL_TOILET=DUMMY \
	-DDEL_LOCKER=DUMMY \
	-DDEL_NEAR_ATTACK=DUMMY \
	-DDEL_CLE=DUMMY \
	-DDEL_GRD=DUMMY \
	-DDEL_RADIO=DUMMY \
	-DDEL_EROTICA=DUMMY \
	-DDEL_PARTS=DUMMY \
	-DDEL_USHOLD=DUMMY \
	-DDEL_WALKMAN=DUMMY '




