//
//	goverpic.h	
//
//	written by H.Yoshiike 2001.JUL
//
//	$Id: goverpic.h,v 1.12 2001/09/04 08:53:24 usr03692 Exp $

// 爆発ゲームオーバー時のPICT
#if defined( WN_W12B ) || defined( WN_W14A ) || defined( WN_W16A ) || defined( WN_W16B ) || defined( WN_W22A )
// 爆発ゲームオーバーのみ
pack_all '-s' 	gmov.tri 2D/game_over/plant_bomb2_alp_ovl.bmp

#elif defined( WN_W12A ) || defined ( WN_W12C )
// 爆発ゲームオーバーと溺死ゲームオーバーと落下ゲームオーバー
pack_all '-s' 	gmov.tri \
		2D/game_over/plant_bomb2_alp_ovl.bmp \
		2D/game_over/dekisi_alp_ovl.bmp \
		2D/game_over/fall_alp_ovl.bmp

#elif defined( WN_W20A ) || defined ( WN_W21B )
// 爆発ゲームオーバーと溺死ゲームオーバーとエマゲームオーバー
pack_all '-s' 	gmov.tri \
		2D/game_over/plant_bomb2_alp_ovl.bmp \
		2D/game_over/dekisi_alp_ovl.bmp \
		2D/game_over/ema_died_alp_ovl.bmp

#elif defined( WN_W24A )
// エマゲームオーバー
pack_all '-s' 	gmov.tri \
		2D/game_over/ema_died_alp_ovl.bmp

#elif defined( WN_W20B ) || defined ( WN_W20C )
// 爆発ゲームオーバー(ヘリポート)と溺死ゲームオーバー
pack_all '-s' 	gmov.tri \
		2D/game_over/plant_bomb_alp_ovl.bmp \
		2D/game_over/dekisi_alp_ovl.bmp

#elif defined( WN_W31C )
// 溺死ゲームオーバー
pack_all '-s' 	gmov.tri \
		2D/game_over/dekisi_alp_ovl.bmp

#else
// 爆発ゲームオーバーと溺死ゲームオーバー
pack_all '-s' 	gmov.tri \
		2D/game_over/plant_bomb2_alp_ovl.bmp \
		2D/game_over/dekisi_alp_ovl.bmp
#endif
