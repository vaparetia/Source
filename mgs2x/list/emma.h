//
//	emma.h
//
//	2000/01/31  T.Morita
//       $Id: emma.h,v 1.21 2002/05/09 11:48:53 usr04098 Exp $
//

//エマ・エメリッヒ
//マルチとシングルは映り込みがあるため,同時に映るためTRIを分ける
human	ema_def/ema_def_mh_mt.evm
cvd 	human/ema_def/ema_raindammy.cv2 vn

//髪の毛,めがね,ＩＤカード等の付属品
human/ema_def  ema_bounding.kms ema_glasses_mt.kms
human/ema_def  ema_def_sh_mt.kms ema_hair_for_sngle.kms
human/ema_def  ema_hair_mh.evm ema_hair_swim_mh.evm ema_idrope_mh_mt.evm

//エマ用モーション(狙撃イベントと手繋ぎイベントをわける)
#ifndef W32A
//手繋ぎモーション
mtn '-DDEL_WITHRAIDEN=DUMMY' emma.mls
mtn   emacap.mls
mtn   emadrag.mls
//mtn   emadam.mls

//エマ手繋ぎ用 ライデンモーション
#ifndef WN_ANOTHER
mtn   rai_ee.mls
#else
mtn   sna_ee.mls	//	スネークテイルズ用
#endif

//エマ手繋ぎ用 表情モーションファイル
far   ema.far

//エマ専用振動ファイル
vib rai/ema_single_iya_resist.vib

#else
//狙撃モーション
mtn '-DDEL_SNIPER=DUMMY'  emma.mls
mtn   emacap.mls
mtn   emadrag.mls
//mtn   emadam.mls

#endif

#if defined( WN_W31B ) || defined( WN_W31C )
//水中エマ用 おんぶ強制モーション
mtn   ee_force.mls

//エマ水中用
mtn   ee_swim.mls
#endif
