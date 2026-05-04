/*
	sdemo.h
	    シナリオカメラデモ関連

	1999/09/03 Y.Matsuhana
	$Id: sdemo.h,v 1.10 2002/06/25 09:13:33 usr03682 Exp $


*/

// ファイルが二重呼びされたときの対処
#ifndef d:SDEMO_H
#define SDEMO_H	1

#define SDEMO_CINEMASCREEN_ON	0x000001	// シネマスクリーンを出す
#define SDEMO_CANCEL_ENABLE	0x000002	// キャンセル可能
#define SDEMO_RADAR_OFF		0x000004	// デモ中レーダーOFF
#define SDEMO_PAD_DISABLE	0x000008	// パッド操作禁止
// 注）マイナスを設定判定に使用しているためＭＳＢの使用を禁ず 

proc シナリオカメラデモフラグ変更 $:フラグ $:開始時停止時間 $:終了時停止時間 {
	eval( $w:シナリオデモカメラフラグ = $:フラグ ) ;
	eval( $w:開始時停止時間ワーク = $:開始時停止時間 ) ;
	eval( $w:終了時停止時間ワーク = $:終了時停止時間 ) ;
}
proc シナリオカメラデモ $:p_時間 $:p_PX1 $:p_PY1 $:p_PZ1 $:p_TX1 $:p_TY1 $:p_TZ1 $:p_AGL1 $:p_PX2 $:p_PY2 $:p_PZ2 $:p_TX2 $:p_TY2 $:p_TZ2 $:p_AGL2 {
	if ( $f:シナリオカメラデモ起動中 == 0  ) {		// ワークの処理の関係で多重呼び出しには対応していません。
		eval( $f:シナリオカメラデモ起動中 = 1 ) ;
		eval( $w:時間 = $:p_時間 ) ;
		eval( $i:PX1 = $:p_PX1 ) ;
		eval( $i:PY1 = $:p_PY1 ) ;
		eval( $i:PZ1 = $:p_PZ1 ) ;
		eval( $i:TX1 = $:p_TX1 ) ;
		eval( $i:TY1 = $:p_TY1 ) ;
		eval( $i:TZ1 = $:p_TZ1 ) ;
		eval( $i:PX2 = $:p_PX2 ) ;
		eval( $i:PY2 = $:p_PY2 ) ;
		eval( $i:PZ2 = $:p_PZ2 ) ;
		eval( $i:TX2 = $:p_TX2 ) ;
		eval( $i:TY2 = $:p_TY2 ) ;
		eval( $i:TZ2 = $:p_TZ2 ) ;
		eval( $i:AGL1 = $:p_AGL1 ) ;
		eval( $i:AGL2 = $:p_AGL2 ) ;
		if ( $w:シナリオデモカメラフラグ < 0 ) {
			eval( $w:シナリオデモカメラフラグ = (d:SDEMO_CINEMASCREEN_ON | d:SDEMO_CANCEL_ENABLE | d:SDEMO_RADAR_OFF | d:SDEMO_PAD_DISABLE ) )
		}
		// 特に変更がなければ開始、終了ともに１秒間の”ため”を作る。
		if ( $w:開始時停止時間ワーク <= 0 ) {
			eval( $w:開始時停止時間ワーク = 60 ) ;
		}
		if ( $w:終了時停止時間ワーク <= 0 ) {
			eval( $w:終了時停止時間ワーク = 60 ) ;
		}
		command 無線設定 -reset
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_CINEMASCREEN_ON ) {
			#if d:DEBUG_PRINT
				print 'sdemo cinemascreen set'
			#endif
			chara シネマスクリーン シナリオシネマスクリーン \
				-top d:CINEMA_BELT_UPPER \			// demo_vardef.h にて定義
				-bottom d:CINEMA_BELT_LOWER \		// demo_vardef.h にて定義
				-ftime d:C_FADE_TIME				// demo_vardef.h にて定義
			chara delay シネマ遅れ -time 1 -exec {mesg シネマスクリーン シナリオシネマスクリーン フェードアウト 1 }
		}
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_PAD_DISABLE ) {
			#if d:DEBUG_PRINT
				print 'sdemo pad off'
			#endif
			command パッド操作 -release
		}
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_CANCEL_ENABLE ) {
			#if d:DEBUG_PRINT
				print 'sdemo demo cancel enable'
			#endif
			chara デモキャンセルチェック シナリオカメラデモ専用キャンセル \
				-p シナリオカメラデモ専用キャンセル処理
		}
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_RADAR_OFF ) {
			#if d:DEBUG_PRINT
				print 'sdemo menu off'
			#endif
			command メニュー設定 \
				-menu off \
				-gage off \
				-radar off \
				-radio off \
				-pause off
		}
		
		eval( $i:SDEMOCNT = 0 ) ;
		#if d:DEBUG_PRINT
			print 'sdemo camera set 1'
		#endif
		eval( $f:シナリオカメラは起動されたか = 1 ) ;
		command サイト表示制御 -switch off ;
		chara カメラ シナリオデモ専用カメラ -c 0 -l 0 -p 1
		chara カメラ設定 シナリオデモ専用カメラ \
				-c 1 \
				-p $:p_PX1,$:p_PY1,$:p_PZ1 -t $:p_TX1,$:p_TY1,$:p_TZ1 \
				-a $:p_AGL1 \
				-i 0 0 0 0 \
				-s 1
		#if d:DEBUG_PRINT
			print 'sdemo camera set 2'
		#endif
		if( $w:開始時停止時間ワーク <= 0 ) { eval( $w:開始時停止時間ワーク = 60 ); }
		chara delay 開始ため -time $w:開始時停止時間ワーク -exec { 
			eval( $f:カメラパン連続実行開始中 = 1 );
			chara プロック連続実行 カメラパン \
				-exec { 
					if ( `%ゲームオーバーチェック` ) {
						// プレイヤーが死んでいたらデモ中止 
						mesg プロック連続実行 カメラパン kill
						eval( $f:カメラパン連続実行開始中 = 0 );
						@シナリオカメラデモ専用キャンセル処理 ;
					} else {
						eval( $i:PX  = (($i:PX2-$i:PX1)*$i:SDEMOCNT/$w:時間)+$i:PX1 )
						eval( $i:PY  = (($i:PY2-$i:PY1)*$i:SDEMOCNT/$w:時間)+$i:PY1 )
						eval( $i:PZ  = (($i:PZ2-$i:PZ1)*$i:SDEMOCNT/$w:時間)+$i:PZ1 )
						eval( $i:TX  = (($i:TX2-$i:TX1)*$i:SDEMOCNT/$w:時間)+$i:TX1 )
						eval( $i:TY  = (($i:TY2-$i:TY1)*$i:SDEMOCNT/$w:時間)+$i:TY1 )
						eval( $i:TZ  = (($i:TZ2-$i:TZ1)*$i:SDEMOCNT/$w:時間)+$i:TZ1 )
						eval( $w:AGL = (($i:AGL2-$i:AGL1)*$i:SDEMOCNT/$w:時間)+$i:AGL1 )
						if( $f:シナリオカメラは起動されたか == 1 ) {
							chara カメラ設定 シナリオデモ専用カメラ \
									-c 1 \
									-p $i:PX,$i:PY,$i:PZ -t $i:TX,$i:TY,$i:TZ \
									-a $w:AGL \
									-i 0 0 0 0 \
									-s 1
						}
						eval( $i:SDEMOCNT = $i:SDEMOCNT+1 );
					}
				 } \
				-time $w:時間 \
				-X { 
					eval( $f:カメラパン連続実行開始中 = 0 );
					if( $w:終了時停止時間ワーク <= 0 ) { eval( $w:終了時停止時間ワーク = 60 ); }
					chara delay 終了ため -time $w:終了時停止時間ワーク -exec { 
						@シナリオカメラデモ専用キャンセル処理 ;
					}
				}
		}
		#if d:DEBUG_PRINT
			print 'sdemo set end'
		#endif
	} else {
		#if d:DEBUG_PRINT
		print "Warning! Scenario Camera Demo ------ multi callback !"
		#endif
	}
}

proc シナリオカメラデモ専用キャンセル処理 {
	// もし発生と同時にキャンセルされた場合に存在しないカメラをkillしたりする事でエラーが出るため、
	// 全てのキャンセル処理は1フレーム後に処理する事とする.
	#if d:DEBUG_PRINT
		print 'sdemo end proc start'
	#endif
	chara delay 遅れ -time 1 -exec {
		#if d:DEBUG_PRINT
			print 'sdemo camera kill'
		#endif
		if ( $f:シナリオカメラは起動されたか == 1 ) {
			command サイト表示制御 -switch on ;
			chara カメラ設定 シナリオデモ専用カメラ -k 1	// もう不要なのでKILLします
			eval( $f:シナリオカメラは起動されたか = 0 ) ;
		}
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_CINEMASCREEN_ON ) {
			#if d:DEBUG_PRINT
				print 'sdemo cinema screen kill'
			#endif
			mesg シネマスクリーン シナリオシネマスクリーン kill
		}
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_PAD_DISABLE ) {
			#if d:DEBUG_PRINT
				print 'sdemo pad cancel'
			#endif
			command パッド操作 -cancel
		}
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_RADAR_OFF ) {
			#if d:DEBUG_PRINT
				print 'sdemo menu on'
			#endif
			command メニュー設定 \
				-menu on \
				-gage on \
				-radar on \
				-radio on \
				-pause on
		}
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_CANCEL_ENABLE ) {
			#if d:DEBUG_PRINT
				print 'sdemo demo cancel kill'
			#endif
			mesg デモキャンセルチェック シナリオカメラデモ専用キャンセル kill
		}
		#if d:DEBUG_PRINT
			print 'sdemo end etc'
		#endif
		mesg delay 開始ため kill
		mesg delay 終了ため kill

		if ( $f:カメラパン連続実行開始中 == 1 ) {
			// カメラパン中にキャンセルされてきた場合にこの処理が必要
			eval( $f:カメラパン連続実行開始中 = 0 );
			mesg プロック連続実行 カメラパン kill
		}

		eval( $w:シナリオデモカメラフラグ = -1 ) ;
		eval( $w:開始時停止時間ワーク = -1 ) ;
		eval( $w:終了時停止時間ワーク = -1 ) ;

		eval( $f:シナリオカメラデモ起動中 = 0 ) ;
		#if d:DEBUG_PRINT
			print 'sdemo end proc end'
		#endif
	}
}

proc シナリオカメラデモ（シネマ） $:p_時間 $:p_PX1 $:p_PY1 $:p_PZ1 $:p_TX1 $:p_TY1 $:p_TZ1 $:p_AGL1 $:p_PX2 $:p_PY2 $:p_PZ2 $:p_TX2 $:p_TY2 $:p_TZ2 $:p_AGL2 {
	if ( $f:シナリオカメラデモ起動中 == 0  ) {		// ワークの処理の関係で多重呼び出しには対応していません。
		eval( $f:シナリオカメラデモ起動中 = 1 ) ;
		eval( $w:時間 = $:p_時間 ) ;
		eval( $i:PX1 = $:p_PX1 ) ;
		eval( $i:PY1 = $:p_PY1 ) ;
		eval( $i:PZ1 = $:p_PZ1 ) ;
		eval( $i:TX1 = $:p_TX1 ) ;
		eval( $i:TY1 = $:p_TY1 ) ;
		eval( $i:TZ1 = $:p_TZ1 ) ;
		eval( $i:PX2 = $:p_PX2 ) ;
		eval( $i:PY2 = $:p_PY2 ) ;
		eval( $i:PZ2 = $:p_PZ2 ) ;
		eval( $i:TX2 = $:p_TX2 ) ;
		eval( $i:TY2 = $:p_TY2 ) ;
		eval( $i:TZ2 = $:p_TZ2 ) ;
		eval( $i:AGL1 = $:p_AGL1 ) ;
		eval( $i:AGL2 = $:p_AGL2 ) ;
		if ( $w:シナリオデモカメラフラグ < 0 ) {
			eval( $w:シナリオデモカメラフラグ = (d:SDEMO_CINEMASCREEN_ON | d:SDEMO_CANCEL_ENABLE | d:SDEMO_RADAR_OFF | d:SDEMO_PAD_DISABLE ) )
		}
		// 特に変更がなければ開始、終了ともに１秒間の”ため”を作る。
		if ( $w:開始時停止時間ワーク <= 0 ) {
			eval( $w:開始時停止時間ワーク = 60 ) ;
		}
		if ( $w:終了時停止時間ワーク <= 0 ) {
			eval( $w:終了時停止時間ワーク = 60 ) ;
		}
		command 無線設定 -reset
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_CINEMASCREEN_ON ) {
			#if d:DEBUG_PRINT
				print 'sdemo cinemascreen set'
			#endif
			chara delay シネマ遅れ -time 1 -exec {mesg シネマスクリーン シネマ フェードアウト 0 }
		}
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_PAD_DISABLE ) {
			#if d:DEBUG_PRINT
				print 'sdemo pad off'
			#endif
			command パッド操作 -release
		}
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_CANCEL_ENABLE ) {
			#if d:DEBUG_PRINT
				print 'sdemo demo cancel enable'
			#endif
			chara デモキャンセルチェック シナリオカメラデモ専用キャンセル \
				-p シナリオカメラデモ専用キャンセル処理（シネマ） ;
		}
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_RADAR_OFF ) {
			#if d:DEBUG_PRINT
				print 'sdemo menu off'
			#endif
			command メニュー設定 \
				-menu off \
				-gage off \
				-radar off \
				-radio off \
				-pause off
		}
		
		eval( $i:SDEMOCNT = 0 ) ;
		#if d:DEBUG_PRINT
			print 'sdemo camera set 1'
		#endif
		eval( $f:シナリオカメラは起動されたか = 1 ) ;
		command サイト表示制御 -switch off ;
		chara カメラ シナリオデモ専用カメラ -c 0 -l 0 -p 1
		chara カメラ設定 シナリオデモ専用カメラ \
				-c 1 \
				-p $:p_PX1,$:p_PY1,$:p_PZ1 -t $:p_TX1,$:p_TY1,$:p_TZ1 \
				-a $:p_AGL1 \
				-i 0 0 0 0 \
				-s 1
		#if d:DEBUG_PRINT
			print 'sdemo camera set 2'
		#endif
		if( $w:開始時停止時間ワーク <= 0 ) { eval( $w:開始時停止時間ワーク = 60 ); }
		chara delay 開始ため -time $w:開始時停止時間ワーク -exec { 
			eval( $f:カメラパン連続実行開始中 = 1 );
			chara プロック連続実行 カメラパン \
				-exec { 
					if ( `%ゲームオーバーチェック` ) {
						// プレイヤーが死んでいたらデモ中止 
						mesg プロック連続実行 カメラパン kill
						eval( $f:カメラパン連続実行開始中 = 0 );
						@シナリオカメラデモ専用キャンセル処理（シネマ） ;
					} else {
						eval( $i:PX  = (($i:PX2-$i:PX1)*$i:SDEMOCNT/$w:時間)+$i:PX1 )
						eval( $i:PY  = (($i:PY2-$i:PY1)*$i:SDEMOCNT/$w:時間)+$i:PY1 )
						eval( $i:PZ  = (($i:PZ2-$i:PZ1)*$i:SDEMOCNT/$w:時間)+$i:PZ1 )
						eval( $i:TX  = (($i:TX2-$i:TX1)*$i:SDEMOCNT/$w:時間)+$i:TX1 )
						eval( $i:TY  = (($i:TY2-$i:TY1)*$i:SDEMOCNT/$w:時間)+$i:TY1 )
						eval( $i:TZ  = (($i:TZ2-$i:TZ1)*$i:SDEMOCNT/$w:時間)+$i:TZ1 )
						eval( $w:AGL = (($i:AGL2-$i:AGL1)*$i:SDEMOCNT/$w:時間)+$i:AGL1 )
						if ( $f:シナリオカメラは起動されたか == 1 ) {
							chara カメラ設定 シナリオデモ専用カメラ \
									-c 1 \
									-p $i:PX,$i:PY,$i:PZ -t $i:TX,$i:TY,$i:TZ \
									-a $w:AGL \
									-i 0 0 0 0 \
									-s 1
						}
						eval( $i:SDEMOCNT = $i:SDEMOCNT+1 );
					}
				 } \
				-time $w:時間 \
				-end_exec { 
					eval( $f:カメラパン連続実行開始中 = 0 );
					if( $w:終了時停止時間ワーク <= 0 ) { eval( $w:終了時停止時間ワーク = 60 ); }
					chara delay 終了ため -time $w:終了時停止時間ワーク -exec { 
						@シナリオカメラデモ専用キャンセル処理（シネマ） ;
					}
				}
		}
		#if d:DEBUG_PRINT
			print 'sdemo set end'
		#endif
	} else {
		#if d:DEBUG_PRINT
		print "Warning! Scenario Camera Demo ------ multi callback !"
		#endif
	}
}

proc シナリオカメラデモ専用キャンセル処理（シネマ） {
	// もし発生と同時にキャンセルされた場合に存在しないカメラをkillしたりする事でエラーが出るため、
	// 全てのキャンセル処理は1フレーム後に処理する事とする.
	#if d:DEBUG_PRINT
		print 'sdemo end proc start'
	#endif
	if ( $w:シナリオデモカメラフラグ & d:SDEMO_CANCEL_ENABLE ) {
		#if d:DEBUG_PRINT
			print 'sdemo demo cancel kill'
		#endif
		mesg デモキャンセルチェック シナリオカメラデモ専用キャンセル kill
	}
	chara delay 遅れ -time 1 -exec {
		#if d:DEBUG_PRINT
			print 'sdemo camera kill'
		#endif
		if ( $f:シナリオカメラは起動されたか == 1 ) {
			command サイト表示制御 -switch on ;
			chara カメラ設定 シナリオデモ専用カメラ -k 1	// もう不要なのでKILLします
			eval( $f:シナリオカメラは起動されたか = 0 ) ;
		}
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_CINEMASCREEN_ON ) {
			#if d:DEBUG_PRINT
				print 'sdemo cinema screen off'
			#endif
			mesg シネマスクリーン シネマ フェードイン 0 ;
		}
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_PAD_DISABLE ) {
			#if d:DEBUG_PRINT
				print 'sdemo pad cancel'
			#endif
			command パッド操作 -cancel
		}
		if ( $w:シナリオデモカメラフラグ & d:SDEMO_RADAR_OFF ) {
			#if d:DEBUG_PRINT
				print 'sdemo menu on'
			#endif
			command メニュー設定 \
				-menu on \
				-gage on \
				-radar on \
				-radio on \
				-pause on
		}
		#if d:DEBUG_PRINT
			print 'sdemo end etc'
		#endif
		mesg delay 開始ため kill
		mesg delay 終了ため kill

		if ( $f:カメラパン連続実行開始中 == 1 ) {
			// カメラパン中にキャンセルされてきた場合にこの処理が必要
			eval( $f:カメラパン連続実行開始中 = 0 );
			mesg プロック連続実行 カメラパン kill
		}

		eval( $w:シナリオデモカメラフラグ = -1 ) ;
		eval( $w:開始時停止時間ワーク = -1 ) ;
		eval( $w:終了時停止時間ワーク = -1 ) ;

		eval( $f:シナリオカメラデモ起動中 = 0 ) ;
		#if d:DEBUG_PRINT
			print 'sdemo end proc end'
		#endif
	}
}




#else

print 'sdemo.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif


