/*
	fall.h                     
	    落下死用プロック

	2001/4/11 S.Mukaide         
	$Id: fall.h,v 1.12 2001/07/03 11:10:17 usr03379 Exp $                      

	
*/

// ファイルが二重呼びされたときの対処
#ifndef d:FALL_H
#define FALL_H	1

/////////////////////////
// ヘッダーで記述するproc
/////////////////////////
// エルード落下死、ジャンプ落下死で共通に使用するプロック
	//	振動やメニューのオフ
	proc ゲームオーバー開始処理 {
		#if d:DEBUG_PRINT
			print 'gameover_task_start'
		#endif
/*
		//	ＳＥはプログラム呼び
		command ＳＥセットモード -s d:SD_V_PFALL01 \
		-p	$i:プレイヤー位置Ｘ,$i:プレイヤー位置Ｙ,$i:プレイヤー位置Ｚ \
		-m 1

//		command セットサウンドコード -c d:SD_V_PFALL01

		#if d:DEBUG_PRINT
			print 'HIMEI_HIMEI_HIMEI_HIMEI_HIMEI_HIMEI_HIMEI_HIMEI_HIMEI'
		#endif
*/
		command プレイヤーモーション振動ＯＦＦ
		if ( $w:体力 > 0 ) {
			// 体力が０になったときはそのときの処理にお任せ
			command メニュー設定 \
				-gage off \
				-radar off \
				-subwin off \
				-radio off \
				-pause off
		}
	}


// エルード落下死で共通に使用するプロック
	//	パッド操作制御などゲ−ムオーバー時に行われる処理を開始
	proc エルード落下死開始処理 {
		#if d:DEBUG_PRINT
			print 'elude_gameover_start'
		#endif

		// エルードの場合はcommandを自前で呼んで登録する
		command ゲームオーバー処理開始 \
			-exec {
				@ゲームオーバー開始処理
			}
	}

	//	エルード落下死終了処理
	proc  エルード落下死終了処理 {
		chara delay 水面落下ディレイ \
			-time 50 \
			-exec {
				command ゲームオーバー処理終了
			}
	}

// ジャンプ落下死で共通に使用するプロック
	// ジャンプ落下死の発動(ゲームオーバー終了処理を自前で用意する必要あり)
	proc ジャンプ落下死発動 $:フラグ $:高さ {
		#if d:DEBUG_PRINT
			print 'jump_gameover_start'
			print '$w:プレイヤー方向 ' $w:プレイヤー方向
		
		
		#endif
		

		command プレイヤー状態取得
/*
		if ( $status & d:PFLAG_RUN ) {
			mesg プレイヤー d:PLAYER rotate $w:プレイヤー方向			//	走りはその向きのまま
		} else if ( $status & d:PFLAG_ROLLING ) {
			mesg プレイヤー d:PLAYER rotate ( $w:プレイヤー方向 + 2048 )	//	側転中は逆向き
		}
*/
		if ( $status & d:PFLAG_ROLLING ) {
			mesg プレイヤー d:PLAYER rotate ( $w:プレイヤー方向 + 2048 )	//	側転中は逆向き
		} else {
			mesg プレイヤー d:PLAYER rotate $w:プレイヤー方向			//	それ以外はその向きのまま
		}

		mesg プレイヤー d:PLAYER fall \
			d:モーションリスト:non_elude_fall_high \
			d:モーションリスト:non_elude_fall_high \
			d:モーションリスト:non_elude_fall_high \
			$:高さ \	//	この高さまで落ちたらゲームオーバー終了処理が呼ばれる
			$:フラグ \
			ジャンプ落下死開始処理 \
			ゲームオーバー終了処理
	}

//# endif





/*	以下は参考用。各gclで記述する必要があるprocです

/////////////////////////
// 各gclで記述するproc
/////////////////////////
// エルード落下死、ジャンプ落下死で各ｇｃｌごとに指定するプロック
	//	ゲームオーバー終了処理 ※音声や振動、フェードアウトなどは落ちる場所によって変化する可能性があるので各gclで行う
	proc ゲームオーバー終了処理 {
//		command セットサウンドコード -c d:se_code:SD_V_POBORE01	//ＳＥおぼれ悲鳴
		chara パッド振動 落下振動 -vibfile drop_sea
	}


// エルード落下死の設定
	proc エルード落下死設定 {
		#if d:DEBUG_PRINT
			print 'gameover_set'
		#endif

		// エルード落下中に死亡トラップに入ったとき		トラップに入るとゲームオーバー処理が開始されます。
		trap de001 d:PLAYER \
			-mask 入る \
			-state d:TRP_STATE_ELUDE_FALL \
			-exec {
				#if d:DEBUG_PRINT
					print 'elude_gameover_trap_in'
				#endif

				chara カメラ設定 ゲームオーバーカメラ \
					-c 0 \
					-b d:CAM_MIN,-6000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 3960,2864,0 -f 4340 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1

				@エルード落下死開始処理

				// SEはを呼ぶときは各自で
//				if ( $w:体力 > 0 ) {
//					// 体力が０のときは死亡時の音声が流れるため別処理
//					chara delay 水面落下ディレイ \
//						-time 15 \
//						-exec {
//							command セットサウンドコード -c d:se_code:SD_V_POBORE01	//ＳＥおぼれ悲鳴
//						}
//				}

			}

		trap de001 d:PLAYER \		トラップから出るときにゲームオーバー処理が終了します。（よって、トラップは落下死させたい高度まで引く必要があります。）
			-mask 出る \
			-state d:TRP_STATE_ELUDE_FALL \
			-exec {
				#if d:DEBUG_PRINT
					print 'elude_gameover_trap_out'
				#endif

				@ゲームオーバー終了処理		// 振動、音、フェードなどを行う
				@エルード落下死終了処理		// command ゲームオーバー処理開始を終了する
			}

	}

	proc ジャンプ落下死開始処理 {
		#if d:DEBUG_PRINT
			print 'jump_gameover_start'
		#endif

		chara カメラ設定 ゲームオーバーカメラ \
			-c 0 \
			-b d:CAM_MIN,-5000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
			-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
//			-r 3960,1683,0 -f 4340 \
			-r 3800,2400,0 -f 4340 \
			-a 200 \
			-i 0 -1 0 0 \
			-s 1

		@ゲームオーバー開始処理
	}


	proc ジャンプ落下死設定 {
		#if d:DEBUG_PRINT
			print 'jump_gameover_set'
		#endif

		//	落下死発動
		trap de001 d:PLAYER \
			-mask 入る \
			-exec {
				command プレイヤー状態取得
				//	歩きはおっとっとエルードなので走りと側転のみトラップに引っかかる
				if ( ($status & d:PFLAG_RUN) || ($status & d:PFLAG_ROLLING) ) {
					@ジャンプ落下死発動 0x0004 -41000
				}
			}
	}

*/



































#else

print 'fall.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif

