//
//	ene_plant_w32a.h	
//
//	written by H.Yoshiike 2000.Nov
//

// 狙撃イベント用敵兵
//tex
pack_all w32_enemy_nm.tri \
human/gps_def/gps_sling.dar \
human/gbs_def/gbs_kanagu.dar \
human/gbs_def/gbs_hlst.dar \
human/gps_def/gps_knife.dar \
weapon/abakan/abk.dar \
weapon/abakan/abk_amo.dar \
item/radio \
item/sougan

// キャラクタ 
human	gps_def/gps_sling.kms
human	gps_def/gps_def_mt.kms
human	gbs_def/gbs_kanagu.kms
human	gbs_def/gbs_hlst.kms 				/* ホルスター */ 
human	gps_def/gps_knife.kms

cvd human/gps_def/gps_def_mt.cv2 vn

weapon	abakan/abk.kms
weapon	abakan/abk_amo.kms

item		radio 
item		sougan


// モーション
//mtn	gbs.mls '-DDEL_SHIELD=DUMMY -DDEL_SHOTGUN=DUMMY '
mtn	gbs.mls '-DDEL_SHIELD=DUMMY -DDEL_SHOTGUN=DUMMY \
	-DDEL_WALKMAN=DUMMY -DDEL_TNG=DUMMY \
	-DDEL_RADIO=DUMMY \
	-DDEL_EROTICA=DUMMY \
	-DDEL_BOX=DUMMY \
	-DDEL_GRD=DUMMY \
	-DDEL_NEAR_ATTACK=DUMMY \
	-DDEL_HANG=DUMMY \
//	-DDEL_PARTS=DUMMY \
	-DDEL_CARRY=DUMMY \
	-DDEL_TEST=DUMMY'

