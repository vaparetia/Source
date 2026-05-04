//
//	cypher.h	
//
//	2001/06/07  H.Satoyoshi
//       $Id: cypher.h,v 1.8 2001/08/03 09:05:15 usr03664 Exp $
//
//	#define	NORM_CYP	通常サイファ
//	#define	GUN_CYP		ガンサイファ
//	#define	SNIPE_CYP	狙撃サイファ

#ifdef GUN_CYP
pack_all	gcyp_cm.tri meca/gun_cypher/gcyp_frg1_cm.dar \
                            meca/gun_cypher/gcyp_frg2_cm.dar \
                            meca/gun_cypher/gcyp_frg3_cm.dar \
                            item/shield/shl_frg1.dar

meca		gun_cypher/gcyp.kms
meca		gun_cypher/gcyp_frg1_cm.kms
meca		gun_cypher/gcyp_frg2_cm.kms
meca		gun_cypher/gcyp_frg3_cm.kms
item		shield/shl_frg1.kms
#endif

#ifdef SNIPE_CYP
pack_all	gcyp_cm.tri meca/gun_cypher/gcyp_frg1_cm.dar \
                            meca/gun_cypher/gcyp_frg2_cm.dar \
                            meca/gun_cypher/gcyp_frg3_cm.dar \
                            item/shield/shl_frg1.dar

meca		gun_cypher/gcyp.kms
meca		gun_cypher/gcyp_frg1_cm.kms
meca		gun_cypher/gcyp_frg2_cm.kms
meca		gun_cypher/gcyp_frg3_cm.kms
item		shield/shl_frg1.kms
#endif

#ifdef NORM_CYP
pack_all	cyp_cm.tri meca/cypher/cyp_frg1_cm.dar \
                           meca/cypher/cyp_frg2_cm.dar \
                           meca/cypher/cyp_frg3_cm.dar \
                           item/shield/shl_frg1.dar
meca		cypher/cyp.kms
meca		cypher/cyp_frg1_cm.kms
meca		cypher/cyp_frg2_cm.kms
meca		cypher/cyp_frg3_cm.kms
item		shield/shl_frg1.kms
#endif


// 爆発エフェクト
pack_all    cypbomb  effect/bakuha/bombgas6_alp.bmp effect/bakuha/bombgas1_alp.bmp \
                     effect/ray_effect/ray_eye_bonbori_alp.bmp


