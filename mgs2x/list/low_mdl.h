//
//	low_mdl.h
//	ローポリ劇場用共通モデル
//	
//	2001/09/29	S.Mukaide
//	$Id: low_mdl.h,v 1.14 2002/09/25 05:39:43 usr03379 Exp $

// KMSライデン
#ifndef NO_KMS_RAI
pack_all	rai_tex_mt.tri	\
            human/rai_def/rai_def_sh_mt.dar			\
            human/rai_def/rai_def_mt.dar			\
            human/rai_def/rai_mag.dar				\
            human/rai_def/rai_hlst.dar				\
            human/rai_def/rai_grip.dar				\
            human/rai_def/rai_hair_bounding.dar		\
			human/rai_def/rai_hair_mh_mt.dar		\
			human/rai_def/rai_hair_shadow_mt.dar	\
			weapon/m92_rai/m92_rai.dar		\
			weapon/m92_rai/m92_rai_sub.dar	\
			weapon/m92_rai/m92_rai_amo.dar	\
			weapon/m92_sna/m92_bul2.dar		\
			weapon/usp/usp_emb.dar			\
			weapon/usp/usp_amo.dar			\
			weapon/fms/fms_emb.dar			\
			weapon/usp/usp_sub.dar			\
			human/gbs_def/gbs_knif.dar		\
			human/gbs_def/gbs_mag.dar

//			human/rai_def/rai_shadow.dar

human		rai_def/rai_def_sh_mt.kms
human		rai_def/rai_def_mt.kms

#else
pack_all 	rai_tex_nm.tri	\
            human/rai_def/rai_mag.dar				\
            human/rai_def/rai_hlst.dar				\
            human/rai_def/rai_grip.dar				\
            human/rai_def/rai_def_mt.dar			\
            human/rai_def/rai_hair_bounding.dar		\
			human/rai_def/rai_hair_mh_mt.dar 		\
			human/rai_def/rai_hair_shadow_mt.dar	\
			weapon/m92_rai/m92_rai.dar		\
			weapon/m92_rai/m92_rai_sub.dar	\
			weapon/m92_rai/m92_rai_amo.dar	\
			weapon/m92_sna/m92_bul2.dar		\
			weapon/usp/usp_emb.dar			\
			weapon/usp/usp_amo.dar			\
			weapon/fms/fms_emb.dar			\
			weapon/usp/usp_sub.dar			\
			human/gbs_def/gbs_knif.dar		\
			human/gbs_def/gbs_mag.dar

//			human/rai_def/rai_shadow.dar

human		rai_def/rai_def_mt.kms

#endif

human		rai_def/rai_mag.kms
human		rai_def/rai_hlst.kms
human		rai_def/rai_grip.kms

// 髪の毛
human		rai_def/rai_hair_mh_mt.evm
human		rai_def/rai_hair_shadow_mt.kms

// 髪の毛用バウンディングボックス
human		rai_def/rai_hair_bounding.kms

// 影
goods		shadow
human		rai_def/rai_shadow.kms
#ifdef MGS2_XBOX_1
pack_trnall	effect.tri effect/shdw/foot_shadow_alp.bmp  /*XBOX用の足影*/
#endif

// リネーム
rename		rai_def_mt.kms	rai_def.kms
rename		rai_def_mt.cv2	rai_def.cv2

// 武器
weapon		m92_rai/m92_rai.kms
weapon		m92_rai/m92_rai_sub.kms
weapon		m92_rai/m92_rai_amo.kms
weapon		m92_sna/m92_bul2.kms
weapon		usp/usp_emb.kms	
weapon		usp/usp_amo.kms	
weapon		fms/fms_emb.kms
weapon		usp/usp_sub.kms

human		gbs_def/gbs_knif.kms
human       gbs_def/gbs_mag.kms


//	KMSスネーク
#ifndef NO_KMS_SNA
pack_all	sna_def_mt.tri \
	human/sna_def/sna_def_sh.dar	\
	human/sna_def/sna_def.dar		\
	human/sna_def/sna_bdn1.dar		\
	human/sna_def/sna_bdn2.dar		\
	human/sna_def/sna_mag1.dar		\
	human/sna_def/sna_mag2.dar		\
	human/sna_def/sna_mag3.dar		\
	human/sna_def/sna_mag4.dar		\
	human/sna_def/sna_shadow.dar

human		sna_def/sna_def_sh.kms
human		sna_def/sna_def.kms
human		sna_def/sna_bdn1.kms
human		sna_def/sna_bdn2.kms
human		sna_def/sna_mag1.kms
human		sna_def/sna_mag2.kms
human		sna_def/sna_mag3.kms
human		sna_def/sna_mag4.kms
human		sna_def/sna_shadow.kms

#else

	#ifdef SH_SNA
	pack_all	sna_tex_nm.tri	\
				human/sna_def/sna_bdn1.dar	\
				human/sna_def/sna_bdn2.dar	\
				human/sna_def/sna_mag1.dar	\
				human/sna_def/sna_mag2.dar	\
				human/sna_def/sna_mag3.dar	\
				human/sna_def/sna_mag4.dar	\
				human/sna_def/sna_def_sh.dar \
				human/sna_def/sna_shadow.dar

	human		sna_def/sna_def_sh.kms
	human		sna_def/sna_bdn1.kms
	human		sna_def/sna_bdn2.kms
	human		sna_def/sna_mag1.kms
	human		sna_def/sna_mag2.kms
	human		sna_def/sna_mag3.kms
	human		sna_def/sna_mag4.kms
	human		sna_def/sna_shadow.kms

	#elif defined( NO_SNA )
	//	何もなし
	#else
	pack_all	sna_tex_nm.tri	\
				human/sna_def/sna_bdn1.dar	\
				human/sna_def/sna_bdn2.dar	\
				human/sna_def/sna_mag1.dar	\
				human/sna_def/sna_mag2.dar	\
				human/sna_def/sna_mag3.dar	\
				human/sna_def/sna_mag4.dar	\
				human/sna_def/sna_def.dar	\
				human/sna_def/sna_shadow.dar

	human		sna_def/sna_def.kms
	human		sna_def/sna_bdn1.kms
	human		sna_def/sna_bdn2.kms
	human		sna_def/sna_mag1.kms
	human		sna_def/sna_mag2.kms
	human		sna_def/sna_mag3.kms
	human		sna_def/sna_mag4.kms
	human		sna_def/sna_shadow.kms

	#endif

#endif


//	nullモデル
goods		null/null.kms



//	追加キャラ

//	ＭＧＳ１スネーク
#define	SNA1_SET	sna_mgs1/sna_mgs1.kms
#define	SNA1E_SET	sna_mgs1/sna_mgs1_mh.evm

//	ＭＧＳ１オセロット
#define	REV1_SET	rev_mgs1/rev_mgs1.kms
#define	REV1E_SET	rev_mgs1/rev_mgs1_mh.evm

//	おばちゃん
#define	LADY_SET	cit_def/cit_femalee_def.kms
#define	SKIRT_SET	cit_def/cit_female_skirt2_mh.evm

//	忍者
#define	NINJA_SET	org_tng/org_tng_pal_mh_mt.evm


//	ビジネスマン
#ifndef NO_BIS
	#define	BIS_SET		cit_def/cit_maley_ctg_sh.kms
#else
	#define	BIS_SET
#endif

//	ＯＬ
#ifndef NO_OL
	#define	OL_SET		cit_def/cit_femaled_def.kms
#else
	#define	OL_SET
#endif

//	ハイテク兵
#ifndef NO_EMEMY
	#define	ENEMY_SET	htc_def/htc_def_mt.kms
#else
	#define	ENEMY_SET
#endif

//	人間をパックする
human	\
		SNA1E_SET	\
		REV1E_SET	\
		LADY_SET	\
		SKIRT_SET	\
		NINJA_SET	\
		SNA1_SET	\
		REV1_SET	\
		BIS_SET		\
		OL_SET		\
		ENEMY_SET


//リネーム
rename		m92_rai.kms			m92.kms
rename		m92_rai_sub.kms		m92_sub.kms
rename		m92_rai_amo.kms		m92_amo.kms


