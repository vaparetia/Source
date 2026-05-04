//
//	2D_tex.h
// £²£Ä¥Æ¥¯¥¹¥Á¥ã
//
//	written by H.Yoshiike 2000.Oct
//
//	$Id: 2D_tex.h,v 1.51 2002/10/02 23:32:05 usr01475 Exp $

// ¶¦ÄÌ¤â¤Î¤Ï¤³¤Á¤é¤ÇÄêµÁ

#if 0 /* ¥µ¥Ö¥¹¥¿¥ó¥¹¤Ç¤ÏÉÔÍ× */
#if defined(MGS2_LANG_ENGLISH) && defined(STAGE_TANKER)
#define		ctrl_tex    menu/ctrl_exp_eng/ctrl1_alp_ovl.bmp \
                        menu/ctrl_exp_eng/ctrl2_alp_ovl.bmp \
                        menu/ctrl_exp_eng/ctrl3_alp_ovl.bmp \
                        menu/ctrl_exp_eng/ctrl4_alp_ovl.bmp
#elif defined(STAGE_TANKER)
#define		ctrl_tex    menu/ctrl_exp/ctrl1_alp_ovl.bmp \
                        menu/ctrl_exp/ctrl2_alp_ovl.bmp \
                        menu/ctrl_exp/ctrl3_alp_ovl.bmp \
                        menu/ctrl_exp/ctrl4_alp_ovl.bmp
#else
#define		ctrl_tex
#endif
#else
#define		ctrl_tex
#endif

/*2001.05.24 sigeno ¥¿¥¤¥Þ¡¼ÍÑ¥Æ¥¯¥¹¥Á¥ã¤òcommon¤«¤éÊ¬Î¥*/
#define			radar_tex \
					debug/shigeno/radar/alt_alp_ovl.bmp \
					debug/shigeno/radar/alt_b_alp_ovl.bmp \
					debug/shigeno/radar/cau_alp_ovl.bmp \
					debug/shigeno/radar/eva_alp_ovl.bmp \
					debug/shigeno/radar/jam_alp_ovl.bmp \
					debug/shigeno/radar/nom_alp_ovl.bmp \
					debug/shigeno/radar/clr_alp_ovl.bmp \
					debug/shigeno/radar/nom_b_alp_ovl.bmp \
					debug/shigeno/radar/clr_b_alp_ovl.bmp \
					debug/shigeno/radar/rdr_noise1.bmp \
					debug/shigeno/radar/rdr_noise2.bmp \
					debug/shigeno/radar/rdr_noise3.bmp \
					debug/shigeno/radar/rdr_yaji_alp_ovl.bmp \
					effect/blood/chi01_msk.bmp // by koba4  Â©·Ñ¤®¥Ý¥¤¥ó¥È

#define			common_tex \
					radar_tex \
					ctrl_tex \
					debug/sonoyama/scope/zoom_alp_ovl.bmp \
					debug/sonoyama/scope/mode_alp_ovl.bmp \
					debug/sonoyama/scope/angle_alp_ovl.bmp \
					codec/parts/call_alp_ovl.bmp \
/*¥¿¥¤¥Þ¡¼ÍÑ*/
/*2001.06.29 ¥¿¥¤¥Þ¡¼ÊÑ¹¹¤Ë¤Ä¤­ÉÔÍ×¤Ë*/
#define		timer_tex \
			debug/shigeno/radar/timefont_alp_ovl.bmp \
			debug/shigeno/radar/timefont_mini_alp_ovl.bmp \
			debug/shigeno/radar/comma_alp_ovl.bmp \
			2D/timer/timebg_alp_ovl.bmp \
			2D/timer/fatman/bomleft_alp_ovl.bmp

// ¥¹¥Æ¡¼¥¸´Ö¥í¡¼¥É¤Ë½Ð¤ë¥¹¥Æ¡¼¥¸Ì¾¤Ï¤Ò¤È¤Ä¤ËÅý¹ç¤µ¤ì¤Þ¤·¤¿¡£
#ifndef MGS2_KOREA
#define WORLD_NAME(str)	menu/world_name/##str
#else
// ´Ú¹ñÈÇ
#define WORLD_NAME(str)	menu/world_name_kor/##str
#endif

// ¥¿¥ó¥«¡¼ÊÔ
// ¹ÃÈÄ
#ifdef WN_W00A
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/senbi_kanpan_right_alp_ovl.bmp )\
	WORLD_NAME(tanker_name/senbi_kanpan_left_alp_ovl.bmp )\
	WORLD_NAME(tanker_name/senbi_kanpan_alp_ovl.bmp )\
	WORLD_NAME(tanker_name/1f_live_left_alp_ovl.bmp )\
	WORLD_NAME(tanker_name/2f_live_right_alp_ovl.bmp )\
	WORLD_NAME(tanker_name/1f_live_right_alp_ovl.bmp ) /*¥¢¥Ê¥¶¡¼ÍÑÄÉ²Ã¥Æ¥¯¥¹¥Á¥ã*/   \
	WORLD_NAME(tanker_name/2f_live_left_alp_ovl.bmp )

#elif defined( WN_W01A )
// µï½»¶è£±³¬Æî
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/senbi_kanpan_left_alp_ovl.bmp )\
	WORLD_NAME(tanker_name/senbi_kanpan_right_alp_ovl.bmp )\
	WORLD_NAME(tanker_name/1f_live_right_alp_ovl.bmp )\
	WORLD_NAME(tanker_name/1f_live_left_alp_ovl.bmp )\
	WORLD_NAME(tanker_name/1f_live_alp_ovl.bmp )\
	WORLD_NAME(tanker_name/refre_right_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/refre_left_alp_ovl.bmp)

#elif defined( WN_W01F )
// µï½»¶è£±³¬ËÌ¥ê¥Õ¥ì¥Ã¥·¥å¥ë¡¼¥à
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/1f_live_right_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/1f_live_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/refre_right_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/refre_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/refre_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/2f_live_right_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kikan_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kikan_ent_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kikan_ent_right_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kikan_right_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kikan_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/2f_live_left_alp_ovl.bmp) 

#elif defined( WN_W01B )
// µï½»¶è£²³¬
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/refre_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/senbi_kanpan_right_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/2f_live_right_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/2f_live_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/2f_live_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/3f_live_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/senbi_kanpan_left_alp_ovl.bmp )

#elif defined( WN_W01C )
// µï½»¶è£³³¬
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/2f_live_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/3f_live_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/3f_live_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/4f_live_alp_ovl.bmp)

#elif defined( WN_W01D )
// µï½»¶è£´³¬
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/3f_live_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/4f_live_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/5f_souda_alp_ovl.bmp)

#elif defined( WN_W01E )
// µï½»¶è£µ³¬
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/4f_live_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/5f_souda_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kanpan_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kanpan_right_alp_ovl.bmp)

#elif defined( WN_W00B )
// ¹ÃÈÄ¥ª¥ë¥¬Àï
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/5f_souda_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kanpan_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kanpan_left_alp_ovl.bmp)

#elif defined( WN_W00C )
// ¹Ò³¤¹ÃÈÄ±¦¸¿¥¦¥¤¥ó¥°
// ¹Ò³¤¹ÃÈÄº¸¸¿¥¦¥¤¥ó¥°
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/5f_souda_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kanpan_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kanpan_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kanpan_right_alp_ovl.bmp)

#elif defined( WN_W02A )
// µ¡´Ø¼¼
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/kikan_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kanpan_02_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kikan_ent_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kikan_ent_right_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kikan_right_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kikan_left_alp_ovl.bmp)

#elif defined( WN_W03A )
// ÂèÆó¹ÃÈÄº¸¸¿
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/kikan_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kanpan_02_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kanpan_02_right_alp_ovl.bmp)

#elif defined( WN_W03B )
// ÂèÆó¹ÃÈÄ±¦¸¿
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/kanpan_02_left_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/kanpan_02_right_alp_ovl.bmp)

#elif defined( WN_W04A )
// Âè°ìÁ¥ÁÒ
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/sensou_01_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/sensou_02_alp_ovl.bmp)

#elif defined( WN_W04B )
// ÂèÆóÁ¥ÁÒ
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/sensou_01_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/sensou_02_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/sensou_03_alp_ovl.bmp)

#elif defined( WN_W04C )
// Âè»°Á¥ÁÒ
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(tanker_name/sensou_02_alp_ovl.bmp) \
	WORLD_NAME(tanker_name/sensou_03_alp_ovl.bmp)

#elif defined( WN_W11A ) || defined( WN_W11B ) || defined( WN_W11C )
// °Ê²¼¥×¥é¥ó¥ÈÊÔ-----------------------------------------------------------
// £ÁµÓÄìÉô³¤Äì¥É¥Ã¥¯
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/a_kaitei_alp_ovl.bmp) \
	WORLD_NAME(plant_name/a_okujou_alp_ovl.bmp)

#elif defined( WN_W12A ) || defined( WN_W12C )
// £ÁµÓ²°¾å
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/a_kaitei_alp_ovl.bmp) \
	WORLD_NAME(plant_name/a_okujou_alp_ovl.bmp) \
	WORLD_NAME(plant_name/a_pump_alp_ovl.bmp)

#elif defined( WN_W12B )
// £ÁµÓ¥Ý¥ó¥×¼¼
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/a_okujou_alp_ovl.bmp) \
	WORLD_NAME(plant_name/a_pump_alp_ovl.bmp) \
	WORLD_NAME(plant_name/ab_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/fa_renraku_alp_ovl.bmp)

#elif defined( WN_W13A ) || defined( WN_W13B )
// £Á£ÂÏ¢Íí¶¶
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/a_pump_alp_ovl.bmp) \
	WORLD_NAME(plant_name/ab_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/b_henden_alp_ovl.bmp)

#elif defined( WN_W14A )
// £ÂµÓÊÑÅÅ¼¼
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/ab_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/b_henden_alp_ovl.bmp) \
	WORLD_NAME(plant_name/bc_renraku_alp_ovl.bmp)

#elif defined( WN_W15A ) || defined( WN_W15B )
// £Â£ÃÏ¢Íí¶¶
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/b_henden_alp_ovl.bmp) \
	WORLD_NAME(plant_name/bc_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/c_shokudou_alp_ovl.bmp)

#elif defined( WN_W16A ) || defined( WN_W16B )
// £ÃµÓ¿©Æ²
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/bc_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/c_shokudou_alp_ovl.bmp) \
	WORLD_NAME(plant_name/cd_renraku_alp_ovl.bmp)

#elif defined( WN_W17A )
// £Ã£ÄÏ¢Íí¶¶
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/c_shokudou_alp_ovl.bmp) \
	WORLD_NAME(plant_name/cd_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/d_chinden_alp_ovl.bmp)

#elif defined( WN_W18A )
// £ÄµÓÂè°ìÄÀÅÂÃÓ
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/cd_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/d_chinden_alp_ovl.bmp) \
	WORLD_NAME(plant_name/de_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/dg_renraku_alp_ovl.bmp)

#elif defined( WN_W19A )
// £Ä£ÅÏ¢Íí¶¶
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/d_chinden_alp_ovl.bmp) \
	WORLD_NAME(plant_name/de_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/e_shuhai_1f_alp_ovl.bmp)

#elif defined( WN_W20A )
// £ÅµÓ½¸ÇÛ¾ì
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/de_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/e_shuhai_alp_ovl.bmp) \
	WORLD_NAME(plant_name/e_shuhai_1f_alp_ovl.bmp) \
	WORLD_NAME(plant_name/e_shuhai_2f_alp_ovl.bmp) \
	WORLD_NAME(plant_name/e_shuhai_b1_alp_ovl.bmp) \
	WORLD_NAME(plant_name/e_heliport_alp_ovl.bmp) \
	WORLD_NAME(plant_name/ef_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/a_pump_alp_ovl.bmp) \
	WORLD_NAME(plant_name/b_henden_alp_ovl.bmp) \
	WORLD_NAME(plant_name/c_shokudou_alp_ovl.bmp) \
	WORLD_NAME(plant_name/f_souko_b1_alp_ovl.bmp)

#elif defined( WN_W20B ) || defined( WN_W20C ) || defined( WN_W20D )
// £ÅµÓ¥Ø¥ê¥Ý¡¼¥È
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/e_shuhai_2f_alp_ovl.bmp) \
	WORLD_NAME(plant_name/e_heliport_alp_ovl.bmp)

#elif defined( WN_W21A ) || defined( WN_W21B )
// £Å£ÆÏ¢Íí¶¶
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/e_shuhai_1f_alp_ovl.bmp) \
	WORLD_NAME(plant_name/ef_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/f_souko_1f_alp_ovl.bmp) \
	WORLD_NAME(plant_name/1_chuou_1f_alp_ovl.bmp)

#elif defined( WN_W22A )
// £ÆµÓÁÒ¸Ë
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/ef_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/f_souko_alp_ovl.bmp) \
	WORLD_NAME(plant_name/f_souko_1f_alp_ovl.bmp) \
	WORLD_NAME(plant_name/f_souko_b1_alp_ovl.bmp) \
	WORLD_NAME(plant_name/fa_renraku_alp_ovl.bmp)

#elif defined( WN_W23A ) || defined( WN_W23B )
// £Æ£ÁÏ¢Íí¶¶
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/f_souko_1f_alp_ovl.bmp) \
	WORLD_NAME(plant_name/f_souko_b1_alp_ovl.bmp) \
	WORLD_NAME(plant_name/fa_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/a_pump_alp_ovl.bmp)

#elif defined( WN_W24A )
// ¥·¥§¥ë£±Ãæ±ûÅï£±£Æ
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/ef_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/1_chuou_1f_alp_ovl.bmp) \
	WORLD_NAME(plant_name/1_chuou_b1_alp_ovl.bmp) \
	WORLD_NAME(plant_name/1_chuou_b2_densan_alp_ovl.bmp)

#elif defined( WN_W24B )
// ¥·¥§¥ë£±Ãæ±ûÅï£Â£±
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/1_chuou_1f_alp_ovl.bmp) \
	WORLD_NAME(plant_name/1_chuou_b1_alp_ovl.bmp) \
	WORLD_NAME(plant_name/1_chuou_b1_shukai_alp_ovl.bmp) \
	WORLD_NAME(plant_name/1_chuou_b2_densan_alp_ovl.bmp)

#elif defined( WN_W24C )
// ¥·¥§¥ë£±Ãæ±ûÅï£Â£±½¸²ñ¾ì
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/1_chuou_b1_alp_ovl.bmp) \
	WORLD_NAME(plant_name/1_chuou_b1_shukai_alp_ovl.bmp)

#elif defined( WN_W24D )
// ¥·¥§¥ë£±Ãæ±ûÅï£Â£²ÅÅ»»¼¼
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/1_chuou_1f_alp_ovl.bmp) \
	WORLD_NAME(plant_name/1_chuou_b1_alp_ovl.bmp) \
	WORLD_NAME(plant_name/1_chuou_b2_densan_alp_ovl.bmp)

#elif defined( WN_W25A )
// ¥·¥§¥ë£±¥·¥§¥ë£²Ï¢Íí¶¶
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/d_chinden_alp_ovl.bmp) \
	WORLD_NAME(plant_name/dg_renraku_alp_ovl.bmp)

#elif defined( WN_W25B )
// ¥·¥§¥ë£±¥·¥§¥ë£²Ï¢Íí¶¶¡Á£Ì£ÇÏ¢Íí¶¶
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/dg_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/lg_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/l_gaishu_alp_ovl.bmp)

#elif defined( WN_W25C ) || defined( WN_W25D )
// £ÌµÓ³°¼þ¡Á£Ë£ÌÏ¢Íí¶¶
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/lg_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/l_gaishu_alp_ovl.bmp) \
	WORLD_NAME(plant_name/kl_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/l_osui_alp_ovl.bmp) \
	WORLD_NAME(plant_name/2_chuou_1f_kuki_alp_ovl.bmp)

#elif defined( WN_W28A )
// £ÌµÓ±ø¿å½èÍý¾ì
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/kl_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/l_osui_alp_ovl.bmp) \
	WORLD_NAME(plant_name/l_oilfence_alp_ovl.bmp)

#elif defined( WN_W31A ) || defined( WN_W31D )
// ¥·¥§¥ë£²Ãæ±ûÅï£±£Æ¶õµ¤À¶¾ô¼¼
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/kl_renraku_alp_ovl.bmp) \
	WORLD_NAME(plant_name/2_chuou_1f_kuki_alp_ovl.bmp) \
	WORLD_NAME(plant_name/2_chuou_b1_roka_01_alp_ovl.bmp)

#elif defined( WN_W31B )
// ¥·¥§¥ë£²Ãæ±ûÅï£Â£±Âè°ìßÉ²á¼¼
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/2_chuou_1f_kuki_alp_ovl.bmp) \
	WORLD_NAME(plant_name/2_chuou_b1_roka_01_alp_ovl.bmp) \
	WORLD_NAME(plant_name/2_chuou_b1_roka_02_alp_ovl.bmp)

#elif defined( WN_W31C )
// ¥·¥§¥ë£²Ãæ±ûÅï£Â£±ÂèÆóßÉ²á¼¼
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/2_chuou_b1_roka_01_alp_ovl.bmp) \
	WORLD_NAME(plant_name/2_chuou_b1_roka_02_alp_ovl.bmp)

#elif defined( WN_W32A )
// £ÌµÓ²¼Éô¥ª¥¤¥ë¥Õ¥§¥ó¥¹
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/l_oilfence_alp_ovl.bmp) \
	WORLD_NAME(plant_name/e_shuhai_b1_alp_ovl.bmp)

#elif defined( WN_W41A )
// ¥¢¡¼¥»¥Ê¥ë¥®¥¢°ß
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/ag_stmach_alp_ovl.bmp) \
	WORLD_NAME(plant_name/ag_jejunum_alp_ovl.bmp) \
	WORLD_NAME(plant_name/aoyama_alp_ovl.bmp) \
	WORLD_NAME(plant_name/azabu_alp_ovl.bmp) \
	WORLD_NAME(plant_name/funabashi_alp_ovl.bmp) \
	WORLD_NAME(plant_name/newyork_alp_ovl.bmp)

#elif defined( WN_W42A )
// ¥¢¡¼¥»¥Ê¥ë¥®¥¢¶õÄ²
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/ag_stmach_alp_ovl.bmp) \
	WORLD_NAME(plant_name/ag_jejunum_alp_ovl.bmp) \
	WORLD_NAME(plant_name/aoyama_alp_ovl.bmp) \
	WORLD_NAME(plant_name/azabu_alp_ovl.bmp) \
	WORLD_NAME(plant_name/funabashi_alp_ovl.bmp) \
	WORLD_NAME(plant_name/newyork_alp_ovl.bmp) \
	WORLD_NAME(plant_name/ag_ascending_alp_ovl.bmp)

#elif defined( WN_W43A )
// ¥¢¡¼¥»¥Ê¥ë¥®¥¢¾å¹Ô·ëÄ²
	#if defined( WN_ANOTHER )
	//	¥¢¥Ê¥¶¡¼¥ß¥Ã¥·¥ç¥óÍÑ
	pack_all	'-s' 2D_tex.tri \
		common_tex \
		WORLD_NAME(plant_name/ag_jejunum_alp_ovl.bmp) \
		WORLD_NAME(plant_name/ag_ascending_alp_ovl.bmp) \
		WORLD_NAME(plant_name/ag_ileum_alp_ovl.bmp)
	#else
	//	ÄÌ¾ïÍÑ
	pack_all	'-s' 2D_tex.tri \
		common_tex \
		WORLD_NAME(plant_name/ag_ascending_alp_ovl.bmp) \
		WORLD_NAME(plant_name/ag_ileum_alp_ovl.bmp)
	#endif
#elif defined( WN_W44A )
// ¥¢¡¼¥»¥Ê¥ë¥®¥¢²óÄ²
	#if defined( WN_ANOTHER )
	//	¥¢¥Ê¥¶¡¼¥ß¥Ã¥·¥ç¥óÍÑ
	pack_all	'-s' 2D_tex.tri \
		common_tex \
		WORLD_NAME(plant_name/ag_ascending_alp_ovl.bmp) \
		WORLD_NAME(plant_name/ag_ileum_alp_ovl.bmp) \
		WORLD_NAME(plant_name/ag_sigmoid_alp_ovl.bmp)
	#else
	//	ÄÌ¾ïÍÑ
	pack_all	'-s' 2D_tex.tri \
		common_tex \
		WORLD_NAME(plant_name/ag_ileum_alp_ovl.bmp) \
		WORLD_NAME(plant_name/ag_sigmoid_alp_ovl.bmp)
	#endif

#elif defined( WN_W45A )
	// ¥¢¡¼¥»¥Ê¥ë¥®¥¢£Ó¾õ·ëÄ²
	#if defined( WN_ANOTHER )
	//	¥¢¥Ê¥¶¡¼¥ß¥Ã¥·¥ç¥óÍÑ
	pack_all	'-s' 2D_tex.tri \
		common_tex \
		WORLD_NAME(plant_name/ag_ileum_alp_ovl.bmp) \
		WORLD_NAME(plant_name/ag_sigmoid_alp_ovl.bmp) \
		WORLD_NAME(plant_name/ag_rectum_alp_ovl.bmp)
	#else
	//	ÄÌ¾ïÍÑ
	pack_all	'-s' 2D_tex.tri \
		common_tex \
		WORLD_NAME(plant_name/ag_sigmoid_alp_ovl.bmp) \
		WORLD_NAME(plant_name/ag_rectum_alp_ovl.bmp)
	#endif

#elif defined( WN_W46A)
// ¥¢¡¼¥»¥Ê¥ë¥®¥¢Ä¾Ä²
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/ag_rectum_alp_ovl.bmp)

#elif defined( WN_W61A)
// ¥¢¡¼¥»¥Ê¥ë¥®¥¢Ä¾Ä²
pack_all	'-s' 2D_tex.tri \
	common_tex \
	WORLD_NAME(plant_name/federal_alp_ovl.bmp)

#else
// ¤½¤ÎÂ¾¤Î¥¹¥Æ¡¼¥¸
pack_all	'-s' 2D_tex.tri \
	common_tex

#endif
