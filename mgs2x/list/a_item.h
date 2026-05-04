//
//	a_item.h
//
//	written by S.Mukaide 2002.JUN
//	$Id:

// オルタナティブミッション用共通アイテムボックス

//	危険防止のため、銃器本体のデファインを立てたら弾薬のデファインも立てる
//	ＵＳＰ
#ifdef USP
	#define	USP_AM
#endif
//	ソコム
#ifdef SCM
	#define	SCM_AM
#endif
//	Ｍ９
#ifdef M9
	#define	M9_AM
#endif
//	Ｍ４
#ifdef M4
	#define	M4_AM
#endif
//	ＡＫ
#ifdef AK
	#define	AK_AM
#endif
//	ＰＳＧ１
#ifdef PSG1
	#define	PSG1_AM
#endif
//	ＰＳＧ１−Ｔ
#ifdef PSG1T
	#define	PSG1T_AM
#endif
//	ＲＧＢ６
#ifdef RGB
	#define	RGB_AM
#endif
//	スティンガー
#ifdef STG
	#define	STG_AM
#endif
//	ニキータ
#ifdef NKT
	#define	NKT_AM
#endif



//	何か一つでもデファインされていたらカスタムモード
#if defined (USP) || defined (SCM) || defined (M9) || defined (USP_SP) || defined (SCM_SP)
#elif defined (USP_AM) || defined (SCM_AM) || defined (M9_AM)
#elif defined (M4) || defined (AK) || defined (PSG1) || defined (PSG1T) || defined (AK_SP)
#elif 	defined (M4_AM) || defined (AK_AM) || defined (PSG1_AM) || defined (PSG1T_AM)
#elif 	defined (RGB) || defined (STG) || defined (NKT)
#elif 	defined (RGB_AM) || defined (STG_AM) || defined (NKT_AM)
#elif 	defined (GRE) || defined (CGR) || defined (SGR) || defined (CLM) || defined (C4)
#elif 	defined (MGZ)
#elif 	defined (MIC)
#elif 	defined (RTN)
#elif 	defined (BS) || defined (DZP) || defined (SHV) || defined (COLD)
#elif 	defined (TGL) || defined (NGL)
#elif 	defined (MND) || defined (BSN_B)
#elif 	defined (CBX) || defined (BAM)
#elif 	defined (D_CAM)
#elif 	defined (CIG)
#elif 	defined (PSN)

//	それ以外は全てのアイテムボックスが入る
#else

#define	USP			//	ＵＳＰ本体
#define	SCM			//	ソコム本体
#define	M9			//	Ｍ９本体
#define	USP_SP		//	ＵＳＰサプレッサ
#define	SCM_SP		//	ソコムサプレッサ
#define	USP_AM		//	ＵＳＰ弾薬
#define	SCM_AM		//	ソコム弾薬
#define	M9_AM		//	Ｍ９弾薬
#define	M4			//	Ｍ４本体
#define	AK			//	ＡＫ本体
#define	PSG1		//	ＰＳＧ１本体
#define	PSG1T		//	ＰＳＧ１−Ｔ本体
#define	AK_SP		//	ＡＫサプレッサ
#define	M4_AM		//	Ｍ４弾薬
#define	AK_AM		//	ＡＫ弾薬
#define	PSG1_AM		//	ＰＳＧ１弾薬
#define	PSG1T_AM	//	ＰＳＧ１−Ｔ弾薬
#define	RGB			//	ＲＧＢ６本体
#define	STG			//	スティンガー本体
#define	NKT			//	ニキータ本体
#define	RGB_AM		//	ＲＧＢ６弾薬
#define	STG_AM		//	スティンガー弾薬
#define	NKT_AM		//	ニキータ弾薬
#define	GRE			//	グレネード
#define	CGR			//	チャフ
#define	SGR			//	スタン
#define	CLM			//	クレイモア
#define	C4			//	Ｃ４
#define	MGZ			//	雑誌
#define	MIC			//	指向性マイク
#define	RTN			//	レーション
#define	BS			//	止血材
#define	DZP			//	ジアゼパム
#define	SHV			//	髭剃り
#define	COLD		//	風邪薬
#define	TGL			//	サーマルゴーグル
#define	NGL			//	暗視ゴーグル
#define	MND			//	地雷探知機
#define	BSN_B		//	爆弾センサーＢ
#define	CBX			//	ダンボール
#define	BAM			//	ボディアーマー
#define	D_CAM		//	デジタルカメラ
//#define	CIG			//	煙草（本編の使用なし）
//#define	PSN			//	振動センサー（本編の使用なし）

#endif

//	ダンボールは全てのステージに入れる必要がある
#ifndef CBX
	#define	CBX			//	ダンボール
#endif



//	モデルを定義
//	ウェポン
	//	ＵＳＰ本体
	#ifdef USP
		#define	USP_SET			weapon/handgun_ibox.kms weapon/handgun_ibox_sh.kms weapon/usp_label.kms
	#else
		#define	USP_SET
	#endif
	//	ソコム本体
	#ifdef SCM
		#define	SCM_SET			weapon/handgun_ibox.kms weapon/handgun_ibox_sh.kms weapon/scm_label.kms
	#else
		#define	SCM_SET
	#endif
	//	Ｍ９本体
	#ifdef M9
		#define	M9_SET			weapon/handgun_ibox.kms weapon/handgun_ibox_sh.kms weapon/m92_label.kms
	#else
		#define	M9_SET
	#endif
	//	ＵＳＰサプレッサ
	#ifdef USP_SP
		#define	USP_SP_SET		weapon/handgun_ibox.kms weapon/handgun_ibox_sh.kms weapon/usp_sp_label.kms
	#else
		#define	USP_SP_SET
	#endif
	//	ソコムサプレッサ
	#ifdef SCM_SP
		#define	SCM_SP_SET		weapon/handgun_ibox.kms weapon/handgun_ibox_sh.kms weapon/scm_sp_label.kms
	#else
		#define	SCM_SP_SET
	#endif

	//	ＵＳＰ弾薬
	#ifdef USP_AM
		#define	USP_AM_SET		amo/handgun_amo_ibox.kms amo/handgun_amo_ibox_sh.kms amo/usp_amo_label.kms
	#else
		#define	USP_AM_SET
	#endif
	//	ソコム弾薬
	#ifdef SCM_AM
		#define	SCM_AM_SET		amo/handgun_amo_ibox.kms amo/handgun_amo_ibox_sh.kms amo/scm_amo_label.kms
	#else
		#define	SCM_AM_SET
	#endif
	//	Ｍ９弾薬
	#ifdef M9_AM
		#define	M9_AM_SET		amo/handgun_amo_ibox.kms amo/handgun_amo_ibox_sh.kms amo/m92_amo_label.kms
	#else
		#define	M9_AM_SET
	#endif

	//	Ｍ４本体
	#ifdef M4
		#define	M4_SET			weapon/rifle_ibox.kms weapon/rifle_ibox_sh.kms weapon/m4_label.kms
	#else
		#define	M4_SET
	#endif
	//	ＡＫ本体
	#ifdef AK
		#define	AK_SET			weapon/rifle_ibox.kms weapon/rifle_ibox_sh.kms weapon/ak_label.kms
	#else
		#define	AK_SET
	#endif
	//	ＰＳＧ１本体
	#ifdef PSG1
		#define	PSG1_SET		weapon/rifle_ibox.kms weapon/rifle_ibox_sh.kms weapon/psg_label.kms
	#else
		#define	PSG1_SET
	#endif
	//	ＰＳＧ１−Ｔ本体
	#ifdef PSG1T
		#define	PSG1T_SET		weapon/rifle_ibox.kms weapon/rifle_ibox_sh.kms weapon/psg_t_label.kms
	#else
		#define	PSG1T_SET
	#endif
	//	ＡＫサプレッサ
	#ifdef AK_SP
		#define	AK_SP_SET		weapon/rifle_ibox.kms weapon/rifle_ibox_sh.kms weapon/ak_sp_label.kms
	#else
		#define	AK_SP_SET
	#endif

	//	Ｍ４弾薬
	#ifdef M4_AM
		#define	M4_AM_SET		amo/rifle_amo_ibox.kms amo/rifle_amo_ibox_sh.kms amo/m4_amo_label.kms
	#else
		#define	M4_AM_SET
	#endif
	//	ＡＫ弾薬
	#ifdef AK_AM
		#define	AK_AM_SET		amo/rifle_amo_ibox.kms amo/rifle_amo_ibox_sh.kms amo/aks_amo_label.kms
	#else
		#define	AK_AM_SET
	#endif
	//	ＰＳＧ１弾薬
	#ifdef PSG1_AM
		#define	PSG1_AM_SET		amo/rifle_amo_ibox.kms amo/rifle_amo_ibox_sh.kms amo/psg_amo_label.kms
	#else
		#define	PSG1_AM_SET
	#endif
	//	ＰＳＧ１−Ｔ弾薬
	#ifdef PSG1T_AM
		#define	PSG1T_AM_SET	amo/rifle_amo_ibox.kms amo/rifle_amo_ibox_sh.kms amo/psgT_amo_label.kms
	#else
		#define	PSG1T_AM_SET
	#endif

	//	ＲＧＢ６本体
	#ifdef RGB
		#define RGB_SET			weapon/launcher_ibox.kms weapon/launcher_ibox_sh.kms weapon/rgb_label.kms
	#else
		#define RGB_SET
	#endif
	//	スティンガー本体
	#ifdef STG
		#define STG_SET			weapon/launcher_ibox.kms weapon/launcher_ibox_sh.kms weapon/stg_label.kms
	#else
		#define STG_SET
	#endif
	//	ニキータ本体
	#ifdef NKT
		#define NKT_SET			weapon/launcher_ibox.kms weapon/launcher_ibox_sh.kms weapon/nkt_label.kms
	#else
		#define NKT_SET
	#endif

	//	ＲＧＢ６弾薬
	#ifdef RGB_AM
		#define	RGB_AM_SET		amo/launcher_amo_ibox.kms amo/launcher_amo_ibox_sh.kms amo/rgb_amo_label.kms
	#else
		#define	RGB_AM_SET
	#endif
	//	スティンガー弾薬
	#ifdef STG_AM
		#define	STG_AM_SET		amo/launcher_amo_ibox.kms amo/launcher_amo_ibox_sh.kms amo/stg_amo_label.kms
	#else
		#define	STG_AM_SET
	#endif
	//	ニキータ弾薬
	#ifdef NKT_AM
		#define	NKT_AM_SET		amo/launcher_amo_ibox.kms amo/launcher_amo_ibox_sh.kms amo/nkt_amo_label.kms
	#else
		#define	NKT_AM_SET
	#endif

	//	グレネード
	#ifdef GRE
		#define	GRE_SET			weapon/grenade_ibox.kms weapon/grenade_ibox_sh.kms weapon/gre_label.kms
	#else
		#define	GRE_SET
	#endif
	//	チャフ
	#ifdef CGR
		#define	CGR_SET			weapon/grenade_ibox.kms weapon/grenade_ibox_sh.kms weapon/cgr_label.kms
	#else
		#define	CGR_SET
	#endif
	//	スタン
	#ifdef SGR
		#define	SGR_SET			weapon/grenade_ibox.kms weapon/grenade_ibox_sh.kms weapon/sgr_label.kms
	#else
		#define	SGR_SET
	#endif
	//	クレイモア
	#ifdef CLM
		#define	CLM_SET			weapon/grenade_ibox.kms weapon/grenade_ibox_sh.kms weapon/clm_label.kms
	#else
		#define	CLM_SET
	#endif
	//	Ｃ４
	#ifdef C4
		#define	C4_SET			weapon/grenade_ibox.kms weapon/grenade_ibox_sh.kms weapon/cfr_label.kms
	#else
		#define	C4_SET
	#endif

	//	雑誌
	#ifdef MGZ
		#define	MGZ_SET			weapon/magazine_ibox.kms weapon/magazine_sh.kms weapon/magazine_label.kms
	#else
		#define	MGZ_SET
	#endif

	//	指向性マイク
	#ifdef MIC
		#define	MIC_SET			weapon/box_ibox.kms	weapon/box_ibox_sh.kms weapon/dmp_label.kms
	#else
		#define	MIC_SET
	#endif



//	アイテム
	//	レーション
	#ifdef RTN
		#define RTN_SET			item/ration_ibox.kms item/ration_ibox_sh.kms item/rtn_label.kms
	#else
		#define RTN_SET
	#endif

	//	止血材
	#ifdef BS
		#define	BS_SET			item/medicine_ibox.kms item/medicine_ibox_sh.kms item/sbs_label.kms
	#else
		#define	BS_SET
	#endif
	//	ジアゼパム
	#ifdef DZP
		#ifdef MGS2_LANG_ENGLISH	//	英語は表記違う
			#define	DZP_SET		item/medicine_ibox.kms item/medicine_ibox_sh.kms item/dzp2_label.kms
		#else
			#define	DZP_SET		item/medicine_ibox.kms item/medicine_ibox_sh.kms item/dzp_label.kms
		#endif
	#else
		#define	DZP_SET
	#endif
	//	髭剃り
	#ifdef SHV
		#define	SHV_SET			item/medicine_ibox.kms item/medicine_ibox_sh.kms item/shv_label.kms
	#else
		#define	SHV_SET
	#endif
	//	風邪薬
	#ifdef COLD
		#define	COLD_SET		item/medicine_ibox.kms item/medicine_ibox_sh.kms item/cold_medicine_label.kms
	#else
		#define	COLD_SET
	#endif

	//	サーマルゴーグル
	#ifdef TGL
		#define	TGL_SET			item/goggle_ibox.kms item/goggle_ibox_sh.kms item/tgl_label.kms
	#else
		#define	TGL_SET
	#endif
	//	暗視ゴーグル
	#ifdef NGL
		#define	NGL_SET			item/goggle_ibox.kms item/goggle_ibox_sh.kms item/ngl_label.kms
	#else
		#define	NGL_SET
	#endif

	//	地雷探知機
	#ifdef MND
		#define	MND_SET			item/detector_ibox.kms item/detector_ibox_sh.kms item/mnd_label.kms
	#else
		#define	MND_SET
	#endif
	//	爆弾センサーＢ
	#ifdef BSN_B
		#define	BSN_B_SET		item/detector_ibox.kms item/detector_ibox_sh.kms item/bsn_b_label.kms
	#else
		#define	BSN_B_SET
	#endif

	//	ダンボール
	#ifdef CBX
		#define	CBX_SET			item/box2_ibox.kms item/box2_ibox_sh.kms item/cbx_label.kms
	#else
		#define	CBX_SET
	#endif
	//	ボディーアーマー
	#ifdef BAM
		#define	BAM_SET			item/box2_ibox.kms item/box2_ibox_sh.kms item/bam_label.kms
	#else
		#define	BAM_SET
	#endif

	//	デジタルカメラ
	#ifdef D_CAM
		#define	D_CAM_SET		item/digital_camera_ibox.kms item/digital_camera_sh.kms item/digital_camera_label.kms
	#else
		#define	D_CAM_SET
	#endif

	//	タバコ（必要ないはずだが念のため）
	#ifdef CIG
		#define	CIG_SET			item/cigarette_ibox.kms item/cigarette_sh.kms item/cigarette_label.kms
	#else
		#define	CIG_SET
	#endif

	//	振動センサー（必要ないはずだが念のため）
	#ifdef PSN
		#define	PSN_SET			item/a_p_sensor_ibox.kms item/medicine_ibox_sh.kms item/a_p_sensor_ibox_label.kms	//	影用モデルはないので仮
	#else
		#define	PSN_SET
	#endif





//	モデルを呼ぶ
itembox		USP_SET			/*	ＵＳＰ本体	*/ \
			SCM_SET			/*	ソコム本体	*/ \
			M9_SET			/*	Ｍ９本体	*/ \
			USP_SP_SET		/*	ＵＳＰサプレッサ	*/ \
			SCM_SP_SET		/*	ソコムサプレッサ	*/ \
			USP_AM_SET		/*	ＵＳＰ弾薬	*/ \
			SCM_AM_SET		/*	ソコム弾薬	*/ \
			M9_AM_SET			/*	Ｍ９弾薬	*/ \
			M4_SET			/*	Ｍ４本体	*/ \
			AK_SET			/*	ＡＫ本体	*/ \
			PSG1_SET			/*	ＰＳＧ１本体	*/ \
			PSG1T_SET		/*	ＰＳＧ１−Ｔ本体	*/ \
			AK_SP_SET			/*	ＡＫサプレッサ	*/ \
			M4_AM_SET			/*	Ｍ４弾薬	*/ \
			AK_AM_SET			/*	ＡＫ弾薬	*/ \
			PSG1_AM_SET		/*	ＰＳＧ１弾薬	*/ \
			PSG1T_AM_SET		/*	ＰＳＧ１−Ｔ弾薬	*/ \
			RGB_SET			/*	ＲＧＢ６本体	*/ \
			STG_SET			/*	スティンガー本体	*/ \
			NKT_SET			/*	ニキータ本体	*/ \
			RGB_AM_SET		/*	ＲＧＢ６弾薬	*/ \
			STG_AM_SET		/*	スティンガー弾薬	*/ \
			NKT_AM_SET		/*	ニキータ弾薬	*/ \
			GRE_SET			/*	グレネード	*/ \
			CGR_SET			/*	チャフ	*/ \
			SGR_SET			/*	スタン	*/ \
			CLM_SET			/*	クレイモア	*/ \
			C4_SET			/*	Ｃ４	*/ \
			MGZ_SET			/*	雑誌	*/ \
			MIC_SET			/*	指向性マイク	*/ \
			RTN_SET			/*	レーション	*/ \
			BS_SET			/*	止血材	*/ \
			DZP_SET			/*	ジアゼパム	*/ \
			SHV_SET			/*	髭剃り	*/ \
			COLD_SET			/*	風邪薬	*/ \
			TGL_SET			/*	サーマルゴーグル	*/ \
			NGL_SET			/*	暗視ゴーグル	*/ \
			MND_SET			/*	地雷探知機	*/ \
			BSN_B_SET			/*	爆弾センサーＢ	*/ \
			CBX_SET			/*	ダンボール	*/ \
			BAM_SET			/*	ボディアーマー	*/ \
			D_CAM_SET			/*	デジタルカメラ	*/ \
			CIG_SET			/*	煙草（本編の使用なし）	*/ \
			PSN_SET				/*	振動センサー（本編の使用なし）*/


