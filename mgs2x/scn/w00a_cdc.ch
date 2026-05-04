/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_t.h"
//eval($w:t_story = d:ST_船内入った)

extern 	command ゲット敵兵状態 

@rt_オタコンＳＡＶＥ＿デフォルト無線セット
command 無線設定 \
	-base 14112 t:RTO_オタコンデフォルト 


//右舷１Ｆ 開かないドア
trap r_no_open_door_dr501 d:PLAYER \
	-mask ? \
	-exec {
			print 'no_open_door_dr501'
			command 無線設定 \
				-over $3 14112 t:RTO_甲板開かない水密扉dr501
			if( ($3 == 出る) && ($f:rft_開かない水密扉ＣＡＬＬ_dr501発動中) ){
				//CALL中ならキャンセルしちゃう
				print 'Call Cancelだよ'
				command 無線設定 -reset
				eval($f:rft_開かない水密扉ＣＡＬＬ_dr501発動中 = 0)
			}
	}

//しつこく開けようとする彼もしくは彼女にはＣＡＬＬをお見舞い！
trap dr501 d:PLAYER \
	-mask いる \
	-button d:ACTION_BUTTON \
	-state 0 \
	-exec {
			eval($b:rft_開かない水密扉開けた回数_dr501 = $b:rft_開かない水密扉開けた回数_dr501 +1)
			if( ($b:rft_開かない水密扉開けた回数_dr501 >= 3) && \
					(!$f:rft_RTO_甲板開かない水密扉ＣＡＬＬ_dr501聞いた) && \
					(!$f:rft_開かない水密扉ＣＡＬＬ_dr501発動中) && \
					(`@rt_重要ＣＡＬＬしてもいいよ`) ){

				eval($f:rft_開かない水密扉ＣＡＬＬ_dr501発動中 = 1)
				command 無線設定 \
					-call 14112 t:RTO_甲板開かない水密扉ＣＡＬＬ_dr501 d:CODEC_CALL
		}
	}



//左舷２Ｆ 開かないドア
trap r_no_open_door_dr502 d:PLAYER \
	-mask ? \
	-exec {
			print 'no_open_door_dr502'
			command 無線設定 \
				-over $3 14112 t:RTO_甲板開かない水密扉dr502
			if( ($3 == 出る) && ($f:rft_開かない水密扉ＣＡＬＬ_dr502発動中) ){
				//CALL中ならキャンセルしちゃう
				print 'Call Cancelだよ'
				command 無線設定 -reset
				eval($f:rft_開かない水密扉ＣＡＬＬ_dr502発動中 = 0)
			}
	}
trap dr502 d:PLAYER \
	-mask いる \
	-button d:ACTION_BUTTON \
	-state 0 \
	-exec {
			eval($b:rft_開かない水密扉開けた回数_dr502 = $b:rft_開かない水密扉開けた回数_dr502 +1)
			if( ($b:rft_開かない水密扉開けた回数_dr502 >= 3) && \
					(!$f:rft_RTO_甲板開かない水密扉ＣＡＬＬ_dr502聞いた) && \
					(`@rt_重要ＣＡＬＬしてもいいよ`) && \
					(!$f:rft_開かない水密扉ＣＡＬＬ_dr502発動中) ){

				eval($f:rft_開かない水密扉ＣＡＬＬ_dr502発動中 = 1)
				command 無線設定 \
					-call 14112 t:RTO_甲板開かない水密扉ＣＡＬＬ_dr502 d:CODEC_CALL
		}
	}



//右舷２Ｆ 開いたり開かなかったりするドア
//HARD以上は開かないドアに早変わり
//eval($w:ゲーム設定 = d:LEVEL_HARD)

if($w:ゲーム設定 >= d:LEVEL_HARD){
	trap r_open_door_dr503 d:PLAYER \
		-mask ? \
		-exec {
			print 'r_open_door_dr503'
			command 無線設定 \
				-over $3 14112 t:RTO_甲板開かない水密扉dr503
			if( ($3 == 出る) && ($f:rft_開かない水密扉ＣＡＬＬ_dr503発動中) ){
				//CALL中ならキャンセルしちゃう
				print 'Call Cancelだよ'
				command 無線設定 -reset
				eval($f:rft_開かない水密扉ＣＡＬＬ_dr503発動中 = 0)
			}
		}
//しつこく開けようとする彼もしくは彼女にはＣＡＬＬをお見舞い！
	trap dr503 d:PLAYER \
		-mask いる \
		-button d:ACTION_BUTTON \
		-state 0 \
		-exec {
			eval($f:rft_開かない水密扉開けた_dr503 = 1)
			eval($b:rft_開かない水密扉開けた回数_dr503 = $b:rft_開かない水密扉開けた回数_dr503 +1)
			if( ($b:rft_開かない水密扉開けた回数_dr503 >= 3) && \
					(!$f:rft_RTO_甲板開かない水密扉ＣＡＬＬ_dr503聞いた) && \
					(`@rt_重要ＣＡＬＬしてもいいよ`) && \
					(!$f:rft_開かない水密扉ＣＡＬＬ_dr503発動中) ){

				eval($f:rft_開かない水密扉ＣＡＬＬ_dr503発動中 = 1)
				command 無線設定 \
					-call 14112 t:RTO_甲板開かない水密扉ＣＡＬＬ_dr503 d:CODEC_CALL
		}
	}

//HARD未満はほんのり開き気味な感じである。
}else {
	trap r_open_door_dr503 d:PLAYER \
		-mask 入る \
		-exec {
			print 'r_open_door_dr503 in 聞いた:'$f:rft_甲板水密扉聞いた
			print 't_story:'$w:t_story

			command 無線設定 \
				-over $3 14112 t:RTO_甲板水密扉 
			if( (!$f:rft_甲板水密扉聞いた) && \
					(`@rt_重要ＣＡＬＬしてもいいよ`) && \
					(!$f:rfp_w00a_水密ドアdr503ＣＡＬＬした) && \
					($w:t_story < d:ST_船内入った) ){
				eval($f:rfp_w00a_水密ドアdr503ＣＡＬＬした = 1)
				command 無線設定 \
					-call 14112 t:RTO_甲板水密扉 d:CODEC_CALL
			}
	}
	trap dr503 d:PLAYER \
		-mask いる \
		-button d:ACTION_BUTTON \
		-state 0 \
		-exec {
			if($f:rfp_w00a_水密ドアdr503ＣＡＬＬした){
				eval($f:rfp_w00a_水密ドアdr503ＣＡＬＬした = 0)
				command 無線設定 -reset
			}
		}

	//CALLキャンセルくん
	trap r_open_door_dr503 d:PLAYER \
		-mask 出る \
		-exec {
			print 'r_open_door_dr503 out'
			command 無線設定 \
				-over $3 14112 t:RTO_甲板水密扉 
			//コールキャンセル出来次第いれるよるぐ
			if($w:t_story < d:ST_船内入った){
				eval($f:rfp_w00a_水密ドアdr503ＣＡＬＬした = 0)
				command 無線設定 -reset
			}
	}
}




//左舷１Ｆ 開いちゃうドア
trap dr500 d:PLAYER \
	-mask 入る \
	-exec {
			print 'open_door in'
			command 無線設定 \
				-over $3 14112 t:RTO_甲板水密扉 
			if( (!$f:rft_甲板水密扉聞いた) && \
					(`@rt_重要ＣＡＬＬしてもいいよ`) && \
					(!$f:rfp_w00a_水密扉dr500ＣＡＬＬした) && \
					($w:t_story < d:ST_船内入った) ){
				eval($f:rfp_w00a_水密扉dr500ＣＡＬＬした = 1)
				command 無線設定 \
					-call 14112 t:RTO_甲板水密扉 d:CODEC_CALL
			}
	}

//CALLキャンセルくん
trap dr500 d:PLAYER \
	-mask いる \
	-button d:ACTION_BUTTON \
	-state 0 \
	-exec {
		if($f:rfp_w00a_水密扉dr500ＣＡＬＬした){
			eval($f:rfp_w00a_水密扉dr500ＣＡＬＬした = 0)
			command 無線設定 -reset
		}
	}

trap dr500 d:PLAYER \
	-mask 出る \
	-exec {
			print 'open_door out'
			command 無線設定 \
				-over $3 14112 t:RTO_甲板水密扉 
			 //コールキャンセル出来次第いれるよるぐ
			if($w:t_story < d:ST_船内入った){
				command 無線設定 -reset
				eval($f:rfp_w00a_水密扉dr500ＣＡＬＬした = 0)
			}
			
	}


//二階に敵兵ちゃんがいるいる？
//でたでた
trap r_2F_enemy 敵兵 \
	-mask 出る \
	-exec {
		eval($f:rft_w00a二階敵兵ちゃんいるよヘイヘイ = 0)
		print 'r_2F_enemy out:'$f:rft_w00a二階敵兵ちゃんいるよヘイヘイ
	}

//いるいる
trap r_2F_enemy 敵兵 \
	-mask いる \
	-exec {
//		print 'r_2F_enemy in:'$f:rft_w00a二階敵兵ちゃんいるよヘイヘイ
		eval($f:rft_w00a二階敵兵ちゃんいるよヘイヘイ = 1)
	}

trap r_2FenemyCaution d:PLAYER \
	-mask ? \
	-exec {
			print 'r_2FenemyCaution'
			command 無線設定 \
				-over $3 14112 t:RTO_甲板二階の敵は主観攻撃
	}



//・救命艇近く 
trap r_boat d:PLAYER \
	-mask ? \
	-exec {
			print 'r_boat'
			command 無線設定 \
				-over $3 14112 t:RTO_救命艇近く
	}


//・死体捨て場 
trap r_deadly_drive d:PLAYER \
	-mask * \
	-exec {
//			print 'r_deadly_drive'
			command 無線設定 \
				-over $3 14112 t:RTO_死体捨てられる場所近く
	}

//段差箱近く
trap r_box d:PLAYER \
	-mask ? \
	-exec {
			print 'r_box'
			command 無線設定 \
				-over $3 14112 t:RTO_段差近く
	}
//段差箱。今ならお得なエルード説明付き
trap r_box_n d:PLAYER \
	-mask ? \
	-exec {
//			print 'r_box_n'
			command 無線設定 \
				-over $3 14112 t:RTO_段差近く北
	}


/*------------------------------------------------
エルーダー近くとその間とは？
------------------------------------------------*/
/*
	}else if($:出るいるその他 == 出る){
		if($f:rft_懸垂ＣＡＬＬした){
			command 無線設定 -reset
		}
	}
*/



proc rt_エルード近く $:出るいるその他 {
		command 無線設定 \
			-over $1 14112 t:RTO_エルード近く
		@rt_懸垂ＣＡＬＬ
}

proc rt_エルード近く２ $:出るいるその他 {
	command 無線設定 \
		-over $1 14112 t:RTO_エルード近く２
		@rt_懸垂ＣＡＬＬ
}
proc rt_奈落エルード近く $:出るいるその他 {
	command 無線設定 \
		-over $1 14112 t:RTO_奈落エルード近く
	@rt_懸垂ＣＡＬＬ
}
proc rt_２Ｆ敵兵注意エルード近く $:出るいるその他 {
	command 無線設定 \
		-over $1 14112 t:RTO_２Ｆ敵兵注意エルード近く
		@rt_懸垂ＣＡＬＬ
}


//２Ｆ敵兵注意エリア 近くエルード
trap r_caution_erude d:PLAYER \
	-mask * \
	-exec {
//		print 'r_caution_erude おてもやん'
			@rt_２Ｆ敵兵注意エルード近く $3
	}

//奈落エルーどん
trap r_near_erude_death d:PLAYER \
	-mask * \
	-exec {
//		print 'r_near_erude_death ずばばしちょ'
		@rt_奈落エルード近く $3
	}

//エルーどん
trap r_near_erude d:PLAYER \
	-mask * \
	-exec {
//		print 'r_near_erude ずばばしちょ'
		@rt_エルード近く $3
	}


// ３F南廊下
trap by000 d:PLAYER \
	-mask * \
//				-button d:ACTION_BUTTON \		//ビヨンドボタン
	-exec {
//		print 'by000 おてもやん'
			@rt_エルード近く $3
	}


// ３F南廊下左
		// ------------------
trap by000l d:PLAYER \
	-mask * \
	-exec {
//		print 'by000 おてもやん'
			@rt_エルード近く $3
	}

// ３F南廊下右
		// ------------------
trap by000r d:PLAYER \
	-mask * \
	-exec {
//		print 'by000r もるごば'
		@rt_エルード近く $3
	}

// ３F東廊下１
trap by002r d:PLAYER \
	-mask * \
	-exec {
//		print 'by002r もるごば'
		@rt_エルード近く $3
	}

// ３F東廊下２
trap by008r d:PLAYER \
	-mask * \
	-exec {
//		print 'by008r もるごば'
		@rt_エルード近く $3
	}

// ３F東廊下３
// ------------------
trap by001r d:PLAYER \
	-mask * \
	-exec {
//		print 'by001r もるごば'
		@rt_エルード近く $3
	}

// ３F東廊下４
// ------------------
trap by009r d:PLAYER \
	-mask * \
	-exec {
//		print 'by009r もるごば'
		@rt_エルード近く $3
	}

// ３F東廊下５
// ------------------
trap by010r d:PLAYER \
	-mask * \
	-exec {
//		print 'by010r もるごば'
		@rt_エルード近く $3
	}


// ３Ｆ東廊下６
	// ------------------
trap by011r d:PLAYER \
	-mask * \
	-exec {
//		print 'by011r もるごば'
		@rt_エルード近く $3
	}

//３Ｆ東廊下７（ななめ）
trap by012r d:PLAYER \
	-mask * \
	-exec {
//		print 'by012r もるごば'
		@rt_エルード近く $3
	}


// ３F東廊下８（ボート上）
		// ------------------
trap by013r d:PLAYER \
	-mask * \
	-exec {
//		print 'by013r もるごば'
		@rt_エルード近く $3
	}

// ３F東廊下９（ボート上）
		// ------------------
trap by014r d:PLAYER \
	-mask * \
	-exec {
//		print 'by014r もるごば'
		@rt_エルード近く $3
	}


// ３Ｆ東廊下１０
		// ------------------
trap by015r d:PLAYER \
	-mask * \
	-exec {
//		print 'by015r もるごば'
		@rt_エルード近く２ $3
	}

// ３Ｆ西廊下１
trap by002l d:PLAYER \
	-mask * \
	-exec {
//		print 'by002l もるごば'
		@rt_エルード近く $3
	}

// ３Ｆ西廊下２
trap by008l d:PLAYER \
	-mask * \
	-exec {
//		print 'by008l もるごば'
		@rt_エルード近く $3
	}

// ３Ｆ西廊下３
trap by001l d:PLAYER \
	-mask * \
	-exec {
//		print 'by001l もるごば'
		@rt_エルード近く $3
	}

// ３Ｆ西廊下４
		// ------------------
trap by009l d:PLAYER \
	-mask * \
	-exec {
//		print 'by009l もるごば'
		@rt_エルード近く $3
	}

// ３F西廊下５
		// ------------------
trap by010l d:PLAYER \
	-mask * \
	-exec {
//		print 'by010l もるごば'
		@rt_エルード近く $3
	}

// ３Ｆ西廊下６
		// ------------------
trap by011l d:PLAYER \
	-mask * \
	-exec {
//		print 'by011l もるごば'
		@rt_エルード近く $3
	}

//３Ｆ西廊下７（ななめ）
trap by012l d:PLAYER \
	-mask * \
	-exec {
//		print 'by012l もるごば'
		@rt_エルード近く $3
	}

// ３F西廊下８（ボート上）
		// ------------------
trap by013l d:PLAYER \
	-mask * \
	-exec {
//		print 'by013l もるごば'
		@rt_エルード近く $3
	}

// ３F西廊下９（ボート上）
		// ------------------
trap by014l d:PLAYER \
	-mask * \
	-exec {
//		print 'by014l もるごば'
		@rt_エルード近く $3
	}

// ３Ｆ西廊下１０		※こっちはモーションが壁にめり込むので東とはエルード可能範囲が異なります。
		// ------------------
trap by015l d:PLAYER \
	-mask * \
	-exec {
//		print 'by015l もるごば'
		@rt_エルード近く２ $3
	}

//甲板中央最後尾
		// ------------------
trap by007 d:PLAYER \
	-mask * \
	-exec {
//		print 'by007  もるごば'
		@rt_エルード近く $3
	}


//左舷１Ｆ３（一番奥）
trap ca021l d:PLAYER \
	-mask * \
	-exec {
//		print 'ca021l もるごば'
		@rt_奈落エルード近く $3
	}

//右舷１Ｆ３（一番奥）
trap ca021r d:PLAYER \
	-mask * \
	-exec {
//		print 'ca021r もるごば'
		@rt_奈落エルード近く $3
	}

//左舷１Ｆ死体遺棄場所
/*
trap ca020l d:PLAYER \
	-mask * \
	-exec {
//		print 'ca020l もるごば'
		@rt_エルード近く $3
	}
*/

//右舷１Ｆ死体遺棄場所
/*
trap ca020r d:PLAYER \
	-mask * \
	-exec {
//		print 'ca020r もるごば'
		@rt_エルード近く $3
	}
*/

//左舷１Ｆ２
/*
trap by003l d:PLAYER \
	-mask * \
	-exec {
//		print 'by003l もるごば'
		@rt_エルード近く $3
	}
*/

/*
//右舷１Ｆ２
trap by003r d:PLAYER \
	-mask * \
	-exec {
//		print 'by003r もるごば'
		@rt_エルード近く $3
	}
*/

//左舷１Ｆ１
/*
trap by004l d:PLAYER \
	-mask * \
	-exec {
//		print 'by004l もるごば'
		@rt_エルード近く $3
	}
*/



/*
//右舷１Ｆ１
trap by004r d:PLAYER \
	-mask * \
	-exec {
//		print 'by004r もるごば'
		@rt_エルード近く $3
	}
*/

//左舷２Ｆななめ
trap by005l d:PLAYER \
	-mask * \
	-exec {
//		print 'by005l もるごば'
		@rt_エルード近く $3
	}

//左舷２Ｆ
		// ------------------
trap by006l d:PLAYER \
	-mask * \
	-exec {
//		print 'by006l もるごば'
		@rt_エルード近く $3
	}

//右舷２Ｆななめ
trap by005r d:PLAYER \
	-mask * \
	-exec {
//		print 'by005r もるごば'
		@rt_エルード近く $3
	}

//左舷２Ｆ
		// ------------------
trap by006r d:PLAYER \
	-mask * \
	-exec {
//		print 'by006r もるごば'
		@rt_エルード近く $3
	}


//###################################################################################
//							すぐおりエルード設定
//###################################################################################
//左舷クレーン北側
/*
		// ------------------
trap by020l d:PLAYER \
	-mask * \
	-exec {
//		print 'by020l もるごば'
		@rt_エルード近く $3
	}
*/

//右舷クレーン北側
/*
		// ------------------
trap by020r d:PLAYER \
	-mask * \
	-exec {
//		print 'by020r もるごば'
		@rt_エルード近く $3
	}
*/

//左舷クレーン西側
		// ------------------
trap by021l d:PLAYER \
	-mask * \
	-exec {
//		print 'by021l もるごば'
		@rt_エルード近く $3
	}

//右舷クレーン東側
		// ------------------
trap by021r d:PLAYER \
	-mask * \
	-exec {
//		print 'by021r もるごば'
		@rt_エルード近く $3
	}

//左舷クレーン南側
		// ------------------
trap by022l d:PLAYER \
	-mask * \
	-exec {
//		print 'by022l もるごば'
		@rt_エルード近く $3
	}

//右舷クレーン南側
		// ------------------
trap by022r d:PLAYER \
	-mask * \
	-exec {
//		print 'by022r もるごば'
		@rt_エルード近く $3
	}

//左舷クレーン階段横南方向
		// ------------------
trap by023l d:PLAYER \
	-mask * \
	-exec {
//		print 'by023l もるごば'
		@rt_エルード近く $3
	}

/*
//左舷クレーン階段横西方向
		// ------------------
trap by023l d:PLAYER \
	-mask * \
	-exec {
//		print 'by023l もるごば'
		@rt_エルード近く $3
	}
*/

//右舷クレーン階段横南方向
		// ------------------
trap by023r d:PLAYER \
	-mask * \
	-exec {
//		print 'by023r もるごば'
		@rt_エルード近く $3
	}


//右舷クレーン階段横東方向
		// ------------------
/*
trap by023r d:PLAYER \
	-mask * \
	-exec {
//		print 'by023r もるごば'
		@rt_エルード近く $3
	}
*/

//強制デモ呼び出しプロック d00a_cdc.chに移行しました。むにに。
/*
proc rt_強制ＣＡＬＬ_T_00b1Rオープニング無線機デモ {
	command 無線設定 \
	-call 14112 t:RTO_00b1Rオープニング無線機デモ d:CODEC_DIRECT rt_強制ＣＡＬＬ_T_00b1Rオープニング無線機デモ終了
}

proc rt_強制ＣＡＬＬ_T_00b1Rオープニング無線機デモ終了 {
print 'まにまに'
}
@rt_強制ＣＡＬＬ_T_00b1Rオープニング無線機デモ
*/

/*
proc rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ体験版 {
	command 無線設定 \
	-call 14112 t:RTO_03b1Rカモフ登場後無線機デモ体験版 d:CODEC_DIRECT
}
@rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ体験版
*/

/*
proc rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ製品版 {
	command 無線設定 \
	-call 14112 t:RTO_03b1Rカモフ登場後無線機デモ製品版 d:CODEC_DIRECT rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ製品版終了 \
	-f d:CODEC_I_AC_O_FD \
	-d
}
proc rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ製品版終了 {
print 'ぬべらひょん'
}
@rt_強制ＣＡＬＬ_T_03b1Rカモフ登場後無線機デモ製品版
*/



/*
proc rt_強制ＣＡＬＬ_T_06b1Rオルガ戦勝利後無線機デモ１ {
	command 無線設定 \
	-call 14112 t:RTO_06b1Rオルガ戦勝利後無線機デモ１ d:CODEC_DIRECT
}
@rt_強制ＣＡＬＬ_T_06b1Rオルガ戦勝利後無線機デモ１
*/

/*
proc rt_強制ＣＡＬＬ_T_09b1R船倉無線機デモ１ {
	command 無線設定 \
	-call 14112 t:RTO_09b1R船倉無線機デモ１ d:CODEC_DIRECT
}
@rt_強制ＣＡＬＬ_T_09b1R船倉無線機デモ１
*/

/*
proc rt_強制ＣＡＬＬ_T_10b1R船倉無線機デモ２ {
	command 無線設定 \
	-call 14112 t:RTO_10b1R船倉無線機デモ２ d:CODEC_DIRECT
}
@rt_強制ＣＡＬＬ_T_10b1R船倉無線機デモ２
*/

/*
proc rt_強制ＣＡＬＬ_T_10b1R船倉無線機デモ２ {
	command 無線設定 \
	-call 14112 t:RPD_P001_02_R01_オープニング潜入２無線デモ１ d:CODEC_DIRECT
}
@rt_強制ＣＡＬＬ_T_10b1R船倉無線機デモ２
*/



/*
proc でばぐたろう {
	command 無線設定 \
	-call 14112 t:RTO_ダミー d:CODEC_DIRECT
}
@でばぐたろう
*/

/*
chara スクリーンショット すくりんしょと \
	-w 1 \
	-k d:PAD_A
*/

