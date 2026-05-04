//	ene_plant.h
//	written by S.Kaneyoshi 2001/04/27
//	$Id: ene_plant.h,v 1.37 2002/06/06 06:49:12 usr04249 Exp $


// ENE_T_GOL		タンカー編兵装ゴルルコ兵
// ENE_P_GOL		プラント編兵装ゴルルコ兵
// ENE_HIGH_TECH	ハイテクアタッカー
// ENE_ATTACKER		アタッカー
// ENE_SHIELD		盾兵
// ENE_SHOT			ショットガン兵
// ENE_GOGGLE		暗視ゴーグル長廊下兵
// ENE_WALK_MAN		ウォークマン

#ifdef ENE_SHIELD
	#define	ENE_SHIELD_DAR_1	item/shield
	#define	ENE_SHIELD_DAR_2	weapon/mkr/mkr.dar
#else
	#define	ENE_SHIELD_DAR_1	human/gbs_hand		// ダミー
	#define	ENE_SHIELD_DAR_2	human/gbs_hand		// ダミー
#endif


#ifdef ENE_SHOT
	#define	ENE_SHOT_DAR	weapon/spas12
#else
	#define	ENE_SHOT_DAR	human/gbs_hand		// ダミー
#endif


#ifdef ENE_GOGGLE
	#define	ENE_GOGGLE_DAR	item/gbs_nv_goggle
#else
	#define	ENE_GOGGLE_DAR	human/gbs_hand		// ダミー
#endif

#ifdef ENE_HIGH_TECH
	#define ENE_HIGH_TECH_DAR_1	weapon/m4_grn_htc/m4_grn_htc.dar
	#define ENE_HIGH_TECH_DAR_2	weapon/m4_grn_htc/m4a_grn_amo_htc.dar
	#define ENE_HIGH_TECH_DAR_3 human/htc_parts/htc_blood_face.dar
	#define ENE_HIGH_TECH_DAR_4	goods/demo_m4_grn_usm/demo_m4_grn_emb.dar
	#define ENE_HIGH_TECH_DAR_5	goods/demo_m4_grn_sel/demo_m4_grn_bul.dar
	#define ENE_HIGH_TECH_DAR_6 human/sel_def/sel_sling1.dar
	#define ENE_HIGH_TECH_DAR_7 human/sel_def/sel_sling2.dar
	#define ENE_HIGH_TECH_DAR_8 human/sel_def/sel_sling3.bmp
#else
	#define ENE_HIGH_TECH_DAR_1	human/gbs_hand	//ダミー
	#define ENE_HIGH_TECH_DAR_2 human/gbs_hand
	#define ENE_HIGH_TECH_DAR_3	human/gbs_hand
	#define ENE_HIGH_TECH_DAR_4	human/gbs_hand
	#define ENE_HIGH_TECH_DAR_5 human/gbs_hand
	#define ENE_HIGH_TECH_DAR_6 human/gbs_hand
	#define ENE_HIGH_TECH_DAR_7 human/gbs_hand
	#define ENE_HIGH_TECH_DAR_8 effect/mark/kirari_alp_ovl.bmp
#endif

#ifdef ENE_P_GOL
	#define ENE_EYE_DAR_0	human/gps_def/gps_eye0.dar
	#define ENE_EYE_DAR_1	human/gps_def/gps_eye1.dar
	#define ENE_EYE_DAR_2	human/gps_def/gps_eye2.dar
	#define ENE_EYE_DAR_3	human/gps_def/gps_eye3.dar
	#define ENE_EYE_DAR_4	human/gps_def/gps_eye4.dar
#else
	#define ENE_EYE_DAR_0	human/gbs_def/gbs_eye0.dar
	#define ENE_EYE_DAR_1	human/gbs_def/gbs_eye1.dar
	#define ENE_EYE_DAR_2	human/gbs_def/gbs_eye2.dar
	#define ENE_EYE_DAR_3	human/gbs_def/gbs_eye3.dar
	#define ENE_EYE_DAR_4	human/gbs_def/gbs_eye4.dar
#endif




#ifndef ENE_MAR
	#define ENE_MAR '-DDEL_SHOTGUN=DUMMY -DDEL_TNG=DUMMY -DDEL_SHIELD=DUMMY -DDEL_WALKMAN=DUMMY'
#endif

// コモデル
#ifdef ENE_SHIELD
	pack_all	gbs_def_cm.tri \
		item/shield/shl_frg1.dar // アクリル盾：破片
#endif



// ノーマルテクスチャ
pack_all	gbs_gba_def_nm.tri \
	human/gbs_hand \
	ENE_EYE_DAR_0 \
	ENE_EYE_DAR_1 \
	ENE_EYE_DAR_2 \
	ENE_EYE_DAR_3 \
	ENE_EYE_DAR_4 \
	goods/dogtag/dogtag_gps.dar				/* ドックタグ */ \
	human/gbs_def/gbs_bp.dar 				/* バックパック */ \
	human/gbs_def/gbs_hlst.dar 				/* ホルスター */ \
	human/gbs_def/gbs_sling.dar 			/* 警備兵のスリング */ \
	human/gbs_def/gbs_kanagu.dar 			/* スリングを留める金具 */\
	human/gba_def/gba_sling.dar 			/* アタッカーのスリング */ \
	human/gba_def/gba_knif.dar 				/* アタッカーのナイフ */ \
	human/gps_def/gps_knife.dar \
	ENE_SHIELD_DAR_1						/* アクリル制盾ディレクトリ */ \
	ENE_SHIELD_DAR_2						/* 盾兵マカロフ */ \
	ENE_GOGGLE_DAR	 						/* 暗視ゴーグルディレクトリ */ \
	item/radio 								/* 無線機ディレクトリ */ \
	item/sougan 							/* 双眼鏡ディレクトリ */ \
	ENE_SHOT_DAR							/* ショットガン */ \
	effect/gradation/gradation_alp_ovl.bmp 	/* 滲み血用テクスチャ */ \
	weapon/aks/aks.dar						/* all enemy */ \
	weapon/aks/aks_sp.dar					/* all enemy */ \
	ENE_HIGH_TECH_DAR_1						/* hi-tec */ \
	ENE_HIGH_TECH_DAR_2						/* hi-tec */ \
	ENE_HIGH_TECH_DAR_3						/* hi-tec */ \
	ENE_HIGH_TECH_DAR_4						/* hi-tec */ \
	ENE_HIGH_TECH_DAR_5						/* hi-tec */ \
	ENE_HIGH_TECH_DAR_6						/* hi-tec */ \
	ENE_HIGH_TECH_DAR_7						/* hi-tec */

	pack_trnall	kirari.tri \
		effect/mark/kirari_alp_ovl.bmp \
		ENE_HIGH_TECH_DAR_8					/* hi-tec */


	// マルチテクスチャ
	// gbsとgbaのテクスチャ重複はスネークほどではないが、gbsの方でマルチテクスチャとノーマル
	// の両方があるので、その部分の重複を回避するために一つに。(00.07.12)
	#ifdef ENE_T_GOL
		// タンカー警備兵
		pack_all	gbss.tri \
			human/gbs_def/gbs_def.dar \
			human/gba_def/gba_def_mt.dar

		// 警備兵のモデル
		human	gbs_def/gbs_def.kms
		cvd		human/gbs_def/gbs_def.cv2 vn
	#else
		#ifdef ENE_P_GOL
			// プラント警備兵

			pack_all gbss_mt.tri \
				human/gps_def/gps_def_mt.dar \
				human/gpa_def/gpa_def_mt.dar

			// 警備兵のモデル
			human	gps_def/gps_def_mt.kms
			cvd		human/gps_def/gps_def_mt.cv2 vn

		#else
			#ifdef ENE_VR_GOL
				// VR警備兵
				pack_all vrgbss_mt.tri \
				human/vr_gbs_def/vr2_gbs1_sh_mt.dar

				// VR警備兵のモデル
				human	vr_gbs_def/vr2_gbs1_sh_mt.kms
				cvd		human/vr_gbs_def/vr2_gbs1_sh_mt.cv2 vn
			
				#ifndef ENE_VR_NO_ATTACKER
					// VR攻撃兵
					pack_all vrgbss2_mt.tri \
						human/vr_gbs_def/vr2_gbs2_sh_mt.dar

					// VR攻撃兵のモデル
					human	vr_gbs_def/vr2_gbs2_sh_mt.kms
					cvd		human/vr_gbs_def/vr2_gbs2_sh_mt.cv2 vn
				#endif
			#else
				// デフォルト警備兵
				pack_all	gbss.tri \
					human/gbs_def/gbs_def.dar \
					human/gba_def/gba_def_mt.dar

				// 警備兵のモデル
				human	gbs_def/gbs_def.kms
				cvd		human/gbs_def/gbs_def.cv2 vn
			#endif
		#endif
	#endif

	// アタッカーのモデル
	#ifdef ENE_ATTACKER
		#ifdef ENE_T_GOL
			human		gba_def/gba_def_mt.kms
			cvd			human/gba_def/gba_def_mt.cv2 vn
		#else
			#ifdef ENE_P_GOL
				human		gpa_def/gpa_def_mt.kms
				cvd			human/gpa_def/gpa_def_mt.cv2 vn
			#else
				human		gba_def/gba_def_mt.kms
				cvd			human/gba_def/gba_def_mt.cv2 vn
			#endif
		#endif
	#endif

	#ifdef ENE_HIGH_TECH
		// ハイテクアタッカーのモデル
		human		htc_def/htc_def_mt.kms
		cvd			human/htc_def/htc_def_mt.cv2 vn
	#endif




	// 装備品のモデル
	#ifdef ENE_P_GOL
		human	gps_def/gps_eye0.kms
		human	gps_def/gps_eye1.kms
		human	gps_def/gps_eye2.kms
		human	gps_def/gps_eye3.kms
		human	gps_def/gps_eye4.kms
	#else
		human	gbs_def/gbs_eye0.kms
		human	gbs_def/gbs_eye1.kms
		human	gbs_def/gbs_eye2.kms
		human	gbs_def/gbs_eye3.kms
		human	gbs_def/gbs_eye4.kms
	#endif
	human	gbs_def/gbs_bp.kms 				/* バックパック */
	human	gbs_def/gbs_hlst.kms 				/* ホルスター */
	goods	dogtag/dogtag_gps.kms
		human	gbs_def/gbs_sling.kms 			/* 警備兵のスリング */
		human	gps_def/gps_sling.kms			/* プラント警備ゴルスリング */
		human	gpa_def/gpa_sling.kms			/* プラント攻撃ゴルゴルスリング */
	human	gbs_def/gbs_kanagu.kms 			/* スリングを留める金具 */
	human	gbs_def/gbs_def.kms

	human		gba_def/gba_sling.kms
    human		gba_def/gba_knif.kms
    human		gps_def/gps_knife.kms
	item		radio
	item		sougan
	weapon		aks/aks.kms
	weapon		aks/aks_sp.kms
	weapon		abakan/abk.kms
	weapon		abakan/abk_amo.kms



	#ifdef ENE_SHIELD
		item		shield			// 盾兵
		weapon		mkr/mkr.kms
	#endif
	#ifdef ENE_GOGGLE
		item		gbs_nv_goggle	// 暗視ゴーグル（長廊下兵）
	#endif
	#ifdef ENE_SHOT
		weapon		spas12			// ショットガン
	#endif
	#ifdef ENE_HIGH_TECH			// ハイテク兵
		weapon		m4_grn_htc/m4_grn_htc.kms
		weapon		m4_grn_htc/m4a_grn_amo_htc.kms
		goods		demo_m4_grn_usm/demo_m4_grn_emb.kms
		goods		demo_m4_grn_sel/demo_m4_grn_bul.kms
		human		htc_parts/htc_blood_face.kms
		human		sel_def/sel_sling1.kms
		human		sel_def/sel_sling2.kms
	#endif


	// モーション
	mtn 		gbs.mls ENE_MAR

	//フィンガーサイン用

	#ifndef ENE_NO_FINGER
		#ifdef ENE_P_GOL
			human	gps_def/gps_hand_def.kms
			cvd		human/gps_def/gps_hand_def.cv2 vn
			cvd		human/gps_def/gps_hand1.cv2 vn
			cvd		human/gps_def/gps_hand2.cv2 vn
			cvd		human/gps_def/gps_hand3.cv2 vn
			cvd		human/gps_def/gps_hand4.cv2 vn
			cvd		human/gps_def/gps_hand5.cv2 vn
		#else
			human	gbs_hand/gbs_hand_def.kms
			cvd		human/gbs_hand vn
		#endif
		// ハイテクアタッカー フィンガー
		#ifdef	ENE_HIGH_TECH
			human	htc_def/htc_hand_def.kms
			cvd		human/htc_def/htc_hand_def.cv2 vn
			cvd		human/htc_def/htc_hand_1.cv2 vn
			cvd		human/htc_def/htc_hand_2.cv2 vn
			cvd		human/htc_def/htc_hand_3.cv2 vn
			cvd		human/htc_def/htc_hand_4.cv2 vn
			cvd		human/htc_def/htc_hand_5.cv2 vn
		#endif
	#endif

	#ifdef ENE_SHADOW
		human	gbs_def/gbs_shadow.kms
	#endif

