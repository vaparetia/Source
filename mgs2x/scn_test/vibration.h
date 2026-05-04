/*
	vibration.h
	振動データプロック
	2000/09/08 S.Kaneyoshi
	$Id: vibration.h,v 1.1 2002/02/01 06:08:32 usr01475 Exp $
*/

enum 水密ドアモーションリスト {
		non_hatch_loop = 0,
		non_hatch_loop_rev,
		non_hatch_open_r_pull,
		non_hatch_open_r_push,
		non_hatch_open_l_pull,
		non_hatch_open_l_push,
		non_hatch_not_open
};

enum ロッカーモーションリスト {
		non_locker_s_open_out = 0,		// 外から開ける
		non_locker_s_close_out,			// 外から閉める
		non_locker_s_open_in,			// 内から開ける
		non_locker_s_close_in,			// 内から閉める
		non_locker_s_close_out_turn,	// 振り返る
		non_locker_s_open_out_dummy,
		non_locker_s_not_open,			//
		non_walk_back_surprise,
		non_behind_idle,
		non_behind_move_r,
		non_behind_move_l
}

proc プレイヤー振動設定 $:モーションファイル名 {
#ifdef	d:VIB_WTDOOR
	// 水密ドア
		chara プレイヤー振動 右水密ドアオープンＰＵＬＬ \
				-marfile sna_hatch \
				-order d:水密ドアモーションリスト:non_hatch_open_r_pull \
				-vib_file sna_non_hatch_open_r_pull

		chara プレイヤー振動 右水密ドアオープンＰＵＳＨ \
				-marfile sna_hatch \
				-order d:水密ドアモーションリスト:non_hatch_open_r_push \
				-vib_file sna_non_hatch_open_r_pull	// 後に正式データを作成する

		chara プレイヤー振動 左水密ドアオープンＰＵＬＬ \
				-marfile sna_hatch \
				-order d:水密ドアモーションリスト:non_hatch_open_l_pull \
				-vib_file sna_non_hatch_open_r_pull	// 後に正式データを作成する

		chara プレイヤー振動 左水密ドアオープンＰＵＳＨ \
				-marfile sna_hatch \
				-order d:水密ドアモーションリスト:non_hatch_open_l_push \
				-vib_file sna_non_hatch_open_r_pull	// 後に正式データを作成する

		chara プレイヤー振動 水密ドア開かない \
				-marfile sna_hatch \
				-order d:水密ドアモーションリスト:non_hatch_not_open \
				-vib_file sna_non_hatch_not_open
#endif

#ifdef d:VIB_LOCKER
	// ロッカー
#if 0
		chara プレイヤー振動 ロッカー振動＿外から開ける \
				-marfile sna_locker \
				-order d:ロッカーモーションリスト:non_locker_s_open_out \
				-vib_file

		chara プレイヤー振動 ロッカー振動＿外から閉める\
				-marfile sna_locker \
				-order d:ロッカーモーションリスト:non_locker_s_close_out \
				-vib_file

		chara プレイヤー振動 ロッカー振動＿内から開ける \
				-marfile sna_locker \
				-order d:ロッカーモーションリスト:non_locker_s_open_in \
				-vib_file

		chara プレイヤー振動 ロッカー振動＿内から閉める \
				-marfile sna_locker \
				-order d:ロッカーモーションリスト:non_locker_s_close_in \
				-vib_file
#endif
		chara プレイヤー振動 ロッカー振動＿開かない \
				-marfile sna_locker \
				-order d:ロッカーモーションリスト:non_locker_s_not_open \
				-vib_file sna_non_locker_s_not_open
#endif

#ifdef d:VIB_ELUDE
	// エルード
/*
		chara プレイヤー振動 エルードノーマル \
				-marfile $:モーションファイル名 \
				-order d:モーションリスト:none_drop_start \
				-vib_file sna_non_drop_start

		chara プレイヤー振動 エルードビハインド \
				-marfile $:モーションファイル名 \
				-order d:モーションリスト:sna_non_drop_start_behind_t \
				-vib_file sna_non_drop_start_behind_t

		chara プレイヤー振動 エルード終了壁無し \
				-marfile $:モーションファイル名 \
				-order d:モーションリスト:none_drop_end_fence \
				-vib_file sna_non_drop_end_fence

		chara プレイヤー振動 エルード終了壁有り \
				-marfile $:モーションファイル名 \
				-order d:モーションリスト:none_drop_end_wall \
				-vib_file sna_non_drop_end_wall
*/
#endif

#ifdef d:VIB_AUTO_JUMP
	// オートジャンプ
		chara プレイヤー振動 ジャンプアップ \
				-marfile $:モーションファイル名 \
				-order d:モーションリスト:non_jump_up \
				-vib_file sna_non_jump_up
		chara プレイヤー振動 ジャンプダウン \
				-marfile $:モーションファイル名 \
				-order d:モーションリスト:non_jump_down_start \
				-vib_file sna_non_jump_down_start
		chara プレイヤー振動 ジャンプダウンエンド \
				-marfile $:モーションファイル名 \
				-order d:モーションリスト:non_jump_down_end \
				-vib_file sna_non_jump_down_end
#endif
}