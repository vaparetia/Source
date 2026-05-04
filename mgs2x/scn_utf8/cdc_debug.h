//
//	cdc_debug.h
//	無線モーションチェック用おもしろデバッグファイル select.gclから呼び出されちゃう。
//
//	2000/10/24	T.Fukushima
//	$Id: cdc_debug.h,v 1.30 2002/06/10 10:43:53 usr03379 Exp $

if ($b:res_mode == d:RES_SNAKE) {
	chara 2Dレイアウトドライバ 2Dレイアウトドライバちゃん
	chara 無線システム Codec -player sna_def
}


proc rt_vc090101 {
	command 無線設定 -call 14112 t:RTO_vc090101 d:CODEC_DIRECT 終わり
}

proc rt_vc090201 {
	command 無線設定 -call 14112 t:RTO_vc090201 d:CODEC_DIRECT 終わり
}

proc rt_vc090211 {
	command 無線設定 -call 14112 t:RTO_vc090211 d:CODEC_DIRECT 終わり
}

proc rt_vc090301 {
	command 無線設定 -call 14112 t:RTO_vc090301 d:CODEC_DIRECT 終わり
}

proc rt_vc090311 {
	command 無線設定 -call 14112 t:RTO_vc090311 d:CODEC_DIRECT 終わり
}

proc rt_vc090401 {
	command 無線設定 -call 14112 t:RTO_vc090401 d:CODEC_DIRECT 終わり
}

proc rt_vc090501 {
	command 無線設定 -call 14112 t:RTO_vc090501 d:CODEC_DIRECT 終わり
}

proc rt_vc090601 {
	command 無線設定 -call 14112 t:RTO_vc090601 d:CODEC_DIRECT 終わり
}

proc rt_vc090701 {
	command 無線設定 -call 14112 t:RTO_vc090701 d:CODEC_DIRECT 終わり
}

proc rt_vc090801 {
	command 無線設定 -call 14112 t:RTO_vc090801 d:CODEC_DIRECT 終わり
}

proc rt_vc090901 {
	command 無線設定 -call 14112 t:RTO_vc090901 d:CODEC_DIRECT 終わり
}

proc rt_vc091001 {
	command 無線設定 -call 14112 t:RTO_vc091001 d:CODEC_DIRECT 終わり
}

proc rt_vc091011 {
	command 無線設定 -call 14112 t:RTO_vc091011 d:CODEC_DIRECT 終わり
}

proc rt_vc091021 {
	command 無線設定 -call 14112 t:RTO_vc091021 d:CODEC_DIRECT 終わり
}

proc rt_vc091101 {
	command 無線設定 -call 14112 t:RTO_vc091101 d:CODEC_DIRECT 終わり
}

proc rt_vc091201 {
	command 無線設定 -call 14112 t:RTO_vc091201 d:CODEC_DIRECT 終わり
}

proc rt_vc091211 {
	command 無線設定 -call 14112 t:RTO_vc091211 d:CODEC_DIRECT 終わり
}

proc rt_vc091301 {
	command 無線設定 -call 14112 t:RTO_vc091301 d:CODEC_DIRECT 終わり
}

proc rt_vc091401 {
	command 無線設定 -call 14112 t:RTO_vc091401 d:CODEC_DIRECT 終わり
}

proc rt_vc091501 {
	command 無線設定 -call 14112 t:RTO_vc091501 d:CODEC_DIRECT 終わり
}

proc rt_vc091601 {
	command 無線設定 -call 14112 t:RTO_vc091601 d:CODEC_DIRECT 終わり
}

proc rt_vc091611 {
	command 無線設定 -call 14112 t:RTO_vc091611 d:CODEC_DIRECT 終わり
}

proc rt_vc091621 {
	command 無線設定 -call 14112 t:RTO_vc091621 d:CODEC_DIRECT 終わり
}

proc rt_vc091631 {
	command 無線設定 -call 14112 t:RTO_vc091631 d:CODEC_DIRECT 終わり
}

proc rt_vc092001 {
	command 無線設定 -call 14112 t:RTO_vc092001 d:CODEC_DIRECT 終わり
}

proc codec_set_t_vc090101_vc090601 {
	chara select vc090101_vc090601 -s {
		'vc090101' rt_vc090101
		'vc090201' rt_vc090201
		'vc090211' rt_vc090211
		'vc090301' rt_vc090301
		'vc090311' rt_vc090311
		'vc090401' rt_vc090401
		'vc090501' rt_vc090501
		'vc090601' rt_vc090601
		'vc090701' rt_vc090701
		'back' back_to_root_t_otacon_add
		'exit' exit_codec_debug_mode_t_otacon_add
	}
}

proc codec_set_t_vc090801_vc091301 {
	chara select vc090801_vc091301 -s {
		'vc090801' rt_vc090801
		'vc090901' rt_vc090901
		'vc091001' rt_vc091001
		'vc091011' rt_vc091011
		'vc091021' rt_vc091021
		'vc091101' rt_vc091101
		'vc091201' rt_vc091201
		'vc091211' rt_vc091211
		'vc091301' rt_vc091301
		'back' back_to_root_t_otacon_add
		'exit' exit_codec_debug_mode_t_otacon_add
	}
}

proc codec_set_t_vc091401_vc092001 {
	chara select vc091401_vc092001 -s {
		'vc091401' rt_vc091401
		'vc091501' rt_vc091501
		'vc091601' rt_vc091601
		'vc091611' rt_vc091611
		'vc091621' rt_vc091621
		'vc091631' rt_vc091631
		'vc092001' rt_vc092001
		'back' back_to_root_t_otacon_add
		'exit' exit_codec_debug_mode_t_otacon_add
	}
}

proc codec_set_root_t_otacon_add {
	chara select root -s {
		'vc090101 vc090601' enter_vc090101_vc090601
		'vc090801 vc091301' enter_vc090801_vc091301
		'vc091401 vc092001' enter_vc091401_vc092001
		'back' exit_codec_debug_mode_t_otacon_add
	}
}

proc enter_vc090101_vc090601 {
	eval($f:rft_enter_vc090101_vc090601 = 1)
	restart
}

proc enter_vc090801_vc091301 {
	eval($f:rft_enter_vc090801_vc091301 = 1)
	restart
}

proc enter_vc091401_vc092001 {
	eval($f:rft_enter_vc091401_vc092001 = 1)
	restart
}

proc codec_debug_list_t_otacon_add {
	if($f:rft_enter_vc090101_vc090601){
		@codec_set_t_vc090101_vc090601

	}else if($f:rft_enter_vc090801_vc091301){
		@codec_set_t_vc090801_vc091301

	}else if($f:rft_enter_vc091401_vc092001){
		@codec_set_t_vc091401_vc092001

	}else {
		@clear_flag_t_otacon_add
		@codec_set_root_t_otacon_add
	}
}

proc clear_flag_t_otacon_add {
	eval($f:rft_enter_vc090101_vc090601 = 0)
	eval($f:rft_enter_vc090801_vc091301 = 0)
	eval($f:rft_enter_vc091401_vc092001 = 0)
}

proc exit_codec_debug_mode_t_otacon_add {
	@clear_flag_t_otacon_add
//	eval($f:rft_enter_codec_debug_mode_t_otacon_add = 0)
	restart
}

proc back_to_root_t_otacon_add {
	@clear_flag_t_otacon_add
	restart
}


proc 終わり {
	restart
}

proc enter_codec_debug_mode_t {
	eval($f:rft_enter_codec_debug_mode_t = 1)
	restart
}

proc clear_flag {
	eval($f:rft_enter_demo = 0)
	eval($f:rft_enter_demo_at = 0) // 体験版後
	eval($f:rft_enter_trial_codec = 0)
	eval($f:rft_enter_after_trial_codec = 0) // 体験版後
	eval($f:rft_enter_save = 0) // 体験版後
	eval($f:rft_enter_otacon_add = 0) // タンカー編オタコン追加
	eval($f:rft_enter_vc000101_vc000701 = 0)
	eval($f:rft_vc000801X_vc001401 = 0)
	eval($f:rft_enter_vc001501_vc002201 = 0)
	eval($f:rft_enter_vc002301_vc002801 = 0)
	eval($f:rft_enter_vc003201_vc004101 = 0)
	eval($f:rft_enter_vc004201_vc007601 = 0)
	eval($f:rft_enter_vc007602_vc008201 = 0)
	eval($f:rft_enter_vc008301_vc008505 = 0)
	eval($f:rft_enter_vc008601_vc013305 = 0)
	eval($f:rft_enter_vc013306_vc013602 = 0)
	eval($f:rft_enter_vc013701_vc014401 = 0)
	eval($f:rft_enter_vc014402_vc014902 = 0)
	eval($f:rft_enter_vc015001_vc015502 = 0)
	eval($f:rft_enter_vc015601_vc015901 = 0)
	eval($f:rft_enter_vc016001_vc016801 = 0)
	eval($f:rft_enter_vc016802_vc017103X = 0)
	eval($f:rft_enter_vc017103XX_vc017601 = 0)
	eval($f:rft_enter_vc017701_vc018701 = 0)
	eval($f:rft_enter_vc018801_vc019201 = 0)
	eval($f:rft_enter_modified = 0)
	eval($f:rft_enter_add1 = 0)

// 以下は体験版後分
	eval($f:rft_enter_vc002901 = 0)
	eval($f:rft_enter_vc003001_vc003901 = 0)
	eval($f:rft_enter_vc004001_vc004901 = 0)
	eval($f:rft_enter_vc005201_vc005904 = 0)
	eval($f:rft_enter_vc006001_vc006901 = 0)
	eval($f:rft_enter_vc007001_vc007402 = 0)
	eval($f:rft_enter_vc008901 = 0)
	eval($f:rft_enter_vc009001_vc009901 = 0) // added on 01/03/21
	eval($f:rft_enter_vc010001_vc010903 = 0) // added on 01/03/21
	eval($f:rft_enter_vc011001_vc011902 = 0) // added on 01/03/21
	eval($f:rft_enter_vc012001_vc012901 = 0) // added on 01/03/21
	eval($f:rft_enter_vc013001_vc013202 = 0) // added on 01/03/21
	eval($f:rft_enter_vc020101_vc020901 = 0) // added on 01/04/10
	eval($f:rft_enter_vc021001_vc021901 = 0) // added on 01/04/10
	eval($f:rft_enter_vc022001_vc022901 = 0) // added on 01/04/10
	eval($f:rft_enter_vc023001_vc023901 = 0) // added on 01/04/10
	eval($f:rft_enter_vc024001_vc024101 = 0) // added on 01/04/10
}


proc exit_codec_debug_mode {
	@clear_flag
	eval($f:rft_enter_codec_debug_mode_t = 0)
	restart
}

proc back_to_root{
	@clear_flag
	restart
}


/*---------------------------------------------------------
このへんに呼び出しプロックを書いちゃうぞ
---------------------------------------------------------*/
proc rt_vc050101 {
	command 無線設定 \
	-call 14112 t:RTO_vc050101 d:CODEC_DIRECT 終わり
}

proc rt_vc050204 {
	command 無線設定 \
	-call 14112 t:RTO_vc050204 d:CODEC_DIRECT 終わり
}

proc rt_vc050301 {
	command 無線設定 \
	-call 14112 t:RTO_vc050301 d:CODEC_DIRECT 終わり
}


proc rt_vc050401 {
	command 無線設定 \
	-call 14112 t:RTO_vc050401 d:CODEC_DIRECT 終わり
}

// 以下は体験版後分
// 新規作成分
proc rt_vc050302 {
	command 無線設定 \
	-call 14112 t:RTO_vc050302 d:CODEC_DIRECT 終わり
}

// 新規作成分
proc rt_vc080101 {
	command 無線設定 \
	-call 14112 t:RTO_vc080101 d:CODEC_DIRECT 終わり
}

// 新規作成分
proc rt_vc080201 {
	command 無線設定 \
	-call 14112 t:RTO_vc080201 d:CODEC_DIRECT 終わり
}

// 新規作成分
proc rt_vc080301 {
	command 無線設定 \
	-call 14112 t:RTO_vc080301 d:CODEC_DIRECT 終わり
}

// 新規作成分
proc rt_vc080401 {
	command 無線設定 \
	-call 14112 t:RTO_vc080401 d:CODEC_DIRECT 終わり
}

// 新規作成分
proc rt_vc080402 {
	command 無線設定 \
	-call 14112 t:RTO_vc080402 d:CODEC_DIRECT 終わり
}


/*--------------------------------------------------
ふつーの無線機でんにゃ
-------------------------------------------------*/
proc rt_vc000101 {
	print 'rt_vc000101 むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000101 d:CODEC_DIRECT 終わり
}

proc rt_vc000201 {
	print 'rt_vc000201 むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000201 d:CODEC_DIRECT 終わり
}

proc rt_vc000301 {
	print 'rt_vc000301 むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000301 d:CODEC_DIRECT 終わり
}

proc rt_vc000302 {
	print 'rt_vc000302 むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000302 d:CODEC_DIRECT 終わり
}
proc rt_vc000303 {
	print 'rt_vc000303 むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000303 d:CODEC_DIRECT 終わり
}


proc rt_vc000401X {
	print 'rt_vc000401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000401X d:CODEC_DIRECT 終わり
}
proc rt_vc000401XX {
	print 'rt_vc000401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000401XX d:CODEC_DIRECT 終わり
}

proc rt_vc000501 {
	print 'rt_vc000501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000501 d:CODEC_DIRECT 終わり
}
proc rt_vc000601 {
	print 'rt_vc000601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000601 d:CODEC_DIRECT 終わり
}
proc rt_vc000701 {
	print 'rt_vc000701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000701 d:CODEC_DIRECT 終わり
}
proc rt_vc000801X {
	print 'rt_vc000801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000801 d:CODEC_DIRECT 終わり
}
proc rt_vc000802X {
	print 'rt_vc000802むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000802 d:CODEC_DIRECT 終わり
}
proc rt_vc000802XX {
	print 'rt_vc000802むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000803 d:CODEC_DIRECT 終わり
}
proc rt_vc000803 {
	print 'rt_vc000803むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000804 d:CODEC_DIRECT 終わり
}
proc rt_vc000901 {
	print 'rt_vc000901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc000901 d:CODEC_DIRECT 終わり
}

proc rt_vc001001 {
	print 'rt_vc001001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc001001 d:CODEC_DIRECT 終わり
}
proc rt_vc001101 {
	print 'rt_vc001101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc001101 d:CODEC_DIRECT 終わり
}
proc rt_vc001201 {
	print 'rt_vc001201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc001201 d:CODEC_DIRECT 終わり
}

proc rt_vc001301 {
	print 'rt_vc001301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc001301 d:CODEC_DIRECT 終わり
}
proc rt_vc001401 {
	print 'rt_vc001401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc001401 d:CODEC_DIRECT 終わり
}
proc rt_vc001501 {
	print 'rt_vc001501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc001501 d:CODEC_DIRECT 終わり
}

//体験版用
proc rt_vc001601 {
	print 'rt_vc001601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc001601 d:CODEC_DIRECT 終わり
}
proc rt_vc001701 {
	print 'rt_vc001701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc001701 d:CODEC_DIRECT 終わり
}
proc rt_vc001702 {
	print 'rt_vc001702むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc001702 d:CODEC_DIRECT 終わり
}
proc rt_vc001801 {
	print 'rt_vc001801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc001801 d:CODEC_DIRECT 終わり
}

proc rt_vc001901 {
	print 'rt_vc001901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc001901 d:CODEC_DIRECT 終わり
}
proc rt_vc002001 {
	print 'rt_vc002001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002001 d:CODEC_DIRECT 終わり
}
proc rt_vc002002 {
	print 'rt_vc002002むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002002 d:CODEC_DIRECT 終わり
}
proc rt_vc002101 {
	print 'rt_vc002101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002101 d:CODEC_DIRECT 終わり
}
proc rt_vc002201 {
	print 'rt_vc002201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002201 d:CODEC_DIRECT 終わり
}

//製品版のみみん。ほーちみん。
proc rt_vc002301 {
	print 'rt_vc002301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002301 d:CODEC_DIRECT 終わり
}


proc rt_vc002401 {
	print 'rt_vc002401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002401 d:CODEC_DIRECT 終わり
}
proc rt_vc002402 {
	print 'rt_vc002402むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002402 d:CODEC_DIRECT 終わり
}
proc rt_vc002403 {
	print 'rt_vc002403むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002403 d:CODEC_DIRECT 終わり
}

proc rt_vc002501 {
	print 'rt_vc002501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002501 d:CODEC_DIRECT 終わり
}
proc rt_vc002502 {
	print 'rt_vc002502むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002502 d:CODEC_DIRECT 終わり
}
proc rt_vc002503 {
	print 'rt_vc002503むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002503 d:CODEC_DIRECT 終わり
}
proc rt_vc002601 {
	print 'rt_vc002601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002601 d:CODEC_DIRECT 終わり
}
proc rt_vc002701 {
	print 'rt_vc002701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002701 d:CODEC_DIRECT 終わり
}
proc rt_vc002801 {
	print 'rt_vc002801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002801 d:CODEC_DIRECT 終わり
}

/*
block codec 14？{
	talk 112 d:顔その1 {
	@rto_舵 
//％ない！
}
*/

// 新規作成分
proc rt_vc002901 {
	print 'rt_vc002901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002901 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc003001 {
	print 'rt_vc003001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003001 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc003101 {
	print 'rt_vc003101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003101 d:CODEC_DIRECT 終わり
}
proc rt_vc003201 {
	print 'rt_vc003201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003201 d:CODEC_DIRECT 終わり
}

proc rt_vc003202 {
	print 'rt_vc003202むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003202 d:CODEC_DIRECT 終わり
}
proc rt_vc003203 {
	print 'rt_vc003203むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003203 d:CODEC_DIRECT 終わり
}

proc rt_vc003204 {
	print 'rt_vc003204むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003204 d:CODEC_DIRECT 終わり
}
proc rt_vc003205 {
	print 'rt_vc003205むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003205 d:CODEC_DIRECT 終わり
}
proc rt_vc003206 {
	print 'rt_vc003206むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003206 d:CODEC_DIRECT 終わり
}
proc rt_vc003301 {
	print 'rt_vc003301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003301 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc003401 {
	print 'rt_vc003401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003401 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc003507 {
	print 'rt_vc003501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003507 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc003601 {
	print 'rt_vc003601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003601 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc003701 {
	print 'rt_vc003701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003701 d:CODEC_DIRECT 終わり
}
proc rt_vc003801X {
	print 'rt_vc003801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003801X d:CODEC_DIRECT 終わり
}
proc rt_vc003801XX {
	print 'rt_vc003801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003801XX d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc003901 {
	print 'rt_vc003901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc003901 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc004001 {
	print 'rt_vc004001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004001 d:CODEC_DIRECT 終わり
}
proc rt_vc004101 {
	print 'rt_vc004101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004101 d:CODEC_DIRECT 終わり
}
proc rt_vc004201 {
	print 'rt_vc004201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004201 d:CODEC_DIRECT 終わり
}
proc rt_vc004202 {
	print 'rt_vc004202むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004202 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc004203 {
	print 'rt_vc004203むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004203 d:CODEC_DIRECT 終わり
}
proc rt_vc004301 {
	print 'rt_vc004301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004301 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc004401 {
	print 'rt_vc004401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004401 d:CODEC_DIRECT 終わり
}
proc rt_vc004501 {
	print 'rt_vc004501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004501 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc004601 {
	print 'rt_vc004601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004601 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc004701 {
	print 'rt_vc004701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004701 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc004801 {
	print 'rt_vc004801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004801 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc004802 {
	print 'rt_vc004802むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004802 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc004901 {
	print 'rt_vc004901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004901 d:CODEC_DIRECT 終わり
}
proc rt_vc005001 {
	print 'rt_vc005001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc005001 d:CODEC_DIRECT 終わり
}

/*
proc rto_懸垂握力アップ後 {
/	print 'rto_懸垂握力アップむがが'
vox { ％ない？
	talk オタコン 標準 -m {
		[TO_懸垂握力アップ後:オタコン_000]
	}
//}
}
*/
proc rt_vc005101 {
	print 'rt_vc005101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc005101 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc005201 {
	print 'rt_vc005201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc005201 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc005301 {
	print 'rt_vc005301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc005301 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc005401 {
	print 'rt_vc005401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc005401 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc005501 {
	print 'rt_vc005501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc005501 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc005601 {
	print 'rt_vc005601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc005601 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc005701 {
	print 'rt_vc005701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc005701 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc005801 {
	print 'rt_vc005801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc005801 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc005901 {
	print 'rt_vc005901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc005901 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc005902 {
	print 'rt_vc005902むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc005902 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc005903 {
	print 'rt_vc005903むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc005903 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc005904 {
	print 'rt_vc005904むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc005904 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc006001 {
	print 'rt_vc006001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc006001 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc006002 {
	print 'rt_vc006002むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc006002 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc006101 {
	print 'rt_vc006101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc006101 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc006201 {
	print 'rt_vc006201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc006201 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc006301 {
	print 'rt_vc006301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc006301 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc006401 {
	print 'rt_vc006401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc006401 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc006501 {
	print 'rt_vc006501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc006501 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc006601 {
	print 'rt_vc006601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc006601 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc006701 {
	print 'rt_vc006701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc006701 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc006801 {
	print 'rt_vc006801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc006801 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc006901 {
	print 'rt_vc006901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc006901 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc007001 {
	print 'rt_vc007001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007001 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc007101 {
	print 'rt_vc007101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007101 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc007102 {
	print 'rt_vc007102むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007102 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc007103 {
	print 'rt_vc007103むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007103 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc007104 {
	print 'rt_vc007104むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007104 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc007105 {
	print 'rt_vc007105むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007105 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc007201 {
	print 'rt_vc007201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007201 d:CODEC_DIRECT 終わり
}

/*---------------------------------------------
デフォルトん会話ん系ん会話ん呼び出しんプロックん
----------------------------------------------*/
proc rt_vc007301 {
	print 'rt_vc007301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007301 d:CODEC_DIRECT 終わり
}

//製品版のみ
proc rt_vc007501 {
	print 'rt_vc007501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007501 d:CODEC_DIRECT 終わり
}
proc rt_vc007502 {
	print 'rt_vc007502むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007502 d:CODEC_DIRECT 終わり
}

proc rt_vc007601 {
	print 'rt_vc007601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007601 d:CODEC_DIRECT 終わり
}
proc rt_vc007602 {
	print 'rt_vc007602むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007602 d:CODEC_DIRECT 終わり
}
proc rt_vc007701 {
	print 'rt_vc007701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007701 d:CODEC_DIRECT 終わり
}

proc rt_vc007901X {
	print 'rt_vc007901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007901X d:CODEC_DIRECT 終わり
}
proc rt_vc007901XX {
	print 'rt_vc007901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007901XX d:CODEC_DIRECT 終わり
}
proc rt_vc007801 {
	print 'rt_vc007801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007801 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc007401 {
	print 'rt_vc007401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007401 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc007402 {
	print 'rt_vc007402むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc007402 d:CODEC_DIRECT 終わり
}
proc rt_vc008001 {
	print 'rt_vc008001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008001 d:CODEC_DIRECT 終わり
}
proc rt_vc008101 {
	print 'rt_vc008101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008101 d:CODEC_DIRECT 終わり
}

	//＠オルガ、スネークＬＩＦＥ判断
proc rt_vc008102 {
	print 'rt_vc008102むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008102 d:CODEC_DIRECT 終わり
}
proc rt_vc008103 {
	print 'rt_vc008103むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008103 d:CODEC_DIRECT 終わり
}

proc rt_vc008201 {
	print 'rt_vc008201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008201 d:CODEC_DIRECT 終わり
}
proc rt_vc008301 {
	print 'rt_vc008301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008301 d:CODEC_DIRECT 終わり
}
proc rt_vc008401 {
	print 'rt_vc008401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008401 d:CODEC_DIRECT 終わり
}
proc rt_vc008402 {
	print 'rt_vc008402むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008402 d:CODEC_DIRECT 終わり
}
proc rt_vc008403 {
	print 'rt_vc008403むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008403 d:CODEC_DIRECT 終わり
}
proc rt_vc008404 {
	print 'rt_vc008404むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008404 d:CODEC_DIRECT 終わり
}

proc rt_vc008501 {
	print 'rt_vc008501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008501 d:CODEC_DIRECT 終わり
}
proc rt_vc008502 {
	print 'rt_vc008502むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008502 d:CODEC_DIRECT 終わり
}
proc rt_vc008503 {
	print 'rt_vc008503むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008503 d:CODEC_DIRECT 終わり
}
proc rt_vc008504 {
	print 'rt_vc008504むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008504 d:CODEC_DIRECT 終わり
}
proc rt_vc008505 {
	print 'rt_vc008505むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008505 d:CODEC_DIRECT 終わり
}

proc rt_vc008601 {
	print 'rt_vc008601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008601 d:CODEC_DIRECT 終わり
}
proc rt_vc008701 {
	print 'rt_vc008701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008701 d:CODEC_DIRECT 終わり
}
proc rt_vc008801 {
	print 'rt_vc008801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008801 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc008901 {
	print 'rt_vc008901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc008901 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc009001 {
	print 'rt_vc009001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009001 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc009101 {
	print 'rt_vc009101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009101 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc009201 {
	print 'rt_vc009201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009201 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc009301 {
	print 'rt_vc009301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009301 d:CODEC_DIRECT 終わり
}

/*
デフォルト説明系会話呼び出しプロックス
クラックキメてぇ。
*/

//ＵＳＰ入手後。ＣＡＬＬ？
proc rt_vc009401 {
	print 'rt_vc009401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009401 d:CODEC_DIRECT 終わり
}
// 新規作成分
proc rt_vc009402 {
	print 'rt_vc009402むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009402 d:CODEC_DIRECT 終わり
}
proc rt_vc009403 {
	print 'rt_vc009403むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009403 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc009501 {
	print 'rt_vc009501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009501 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc009502 {
	print 'rt_vc009502むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009502 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc009601 {
	print 'rt_vc009601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009601 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc009602 {
	print 'rt_vc009602むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009602 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc009603 {
	print 'rt_vc009603むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009603 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc009604 {
	print 'rt_vc009604むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009604 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc009605 {
	print 'rt_vc009605むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009605 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc009606 {
	print 'rt_vc009606むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009606 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc009701 {
	print 'rt_vc009701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009701 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc009801 {
	print 'rt_vc009801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009801 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc009901 {
	print 'rt_vc009901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc009901 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010001 {
	print 'rt_vc010001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010001 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010101 {
	print 'rt_vc010101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010101 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010102 {
	print 'rt_vc010102むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010102 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010103 {
	print 'rt_vc010103むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010103 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010201 {
	print 'rt_vc010201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010201 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010301 {
	print 'rt_vc010301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010301 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010401 {
	print 'rt_vc010401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010401 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010402 {
	print 'rt_vc010402むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010402 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010501 {
	print 'rt_vc010501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010501 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010601 {
	print 'rt_vc010601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010601 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010701 {
	print 'rt_vc010701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010701 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010702 {
	print 'rt_vc010702むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010702 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010703 {
	print 'rt_vc010703むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010703 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010704 {
	print 'rt_vc010704むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010704 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010705 {
	print 'rt_vc010705むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010705 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010801 {
	print 'rt_vc010801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010801 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010802 {
	print 'rt_vc010802むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010802 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010803 {
	print 'rt_vc010803むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010803 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010804 {
	print 'rt_vc010804むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010804 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010805 {
	print 'rt_vc010805むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010805 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010806 {
	print 'rt_vc010806むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010806 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010807 {
	print 'rt_vc010807むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010807 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010808 {
	print 'rt_vc010808がが'
	command 無線設定 \
		-call 14112 t:RTO_vc010808 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010809 {
	print 'rt_vc010809むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010809 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010810 {
	print 'rt_vc010810むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010810 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010811 {
	print 'rt_vc010811むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010811 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010812 {
	print 'rt_vc010812むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010812 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010813 {
	print 'rt_vc010813むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010813 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010814 {
	print 'rt_vc010814むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010814 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010901 {
	print 'rt_vc010901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010901 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010902 {
	print 'rt_vc010902むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010902 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc010903 {
	print 'rt_vc010903むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc010903 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011001 {
	print 'rt_vc011001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011001 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011101 {
	print 'rt_vc011101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011101 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011201 {
	print 'rt_vc011201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011201 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011202 {
	print 'rt_vc011202むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011202 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011203 {
	print 'rt_vc011203むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011203 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011204 {
	print 'rt_vc011204むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011204 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011205 {
	print 'rt_vc011205むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011205 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011206 {
	print 'rt_vc011206むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011206 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011207 {
	print 'rt_vc011207むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011207 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011208 {
	print 'rt_vc011208むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011208 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011209 {
	print 'rt_vc011209むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011209 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011210 {
	print 'rt_vc011210むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011210 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011211 {
	print 'rt_vc011211むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011211 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011212 {
	print 'rt_vc011212むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011212 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011301 {
	print 'rt_vc011301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011301 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011302 {
	print 'rt_vc011302むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011302 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011303 {
	print 'rt_vc011303むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011303 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011304 {
	print 'rt_vc011304むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011304 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011401 {
	print 'rt_vc011401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011401 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011402 {
	print 'rt_vc011402むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011402 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011403 {
	print 'rt_vc011403むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011403 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011404 {
	print 'rt_vc011404むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011404 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011405 {
	print 'rt_vc011405むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011405 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011501 {
	print 'rt_vc011501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011501 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011502 {
	print 'rt_vc011502むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011502 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011503 {
	print 'rt_vc011503むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011503 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011601 {
	print 'rt_vc011601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011601 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011602 {
	print 'rt_vc011602むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011602 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011603 {
	print 'rt_vc011603むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011603 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011701 {
	print 'rt_vc011701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011701 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011702 {
	print 'rt_vc011702むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011702 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011801 {
	print 'rt_vc011801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011801 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011802 {
	print 'rt_vc011802むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011802 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011901 {
	print 'rt_vc011901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011901 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc011902 {
	print 'rt_vc011902むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc011902 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012001 {
	print 'rt_vc012001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012001 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012002 {
	print 'rt_vc012002むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012002 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012101 {
	print 'rt_vc012101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012101 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012102 {
	print 'rt_vc012102むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012102 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012103 {
	print 'rt_vc012103むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012103 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012104 {
	print 'rt_vc012104むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012104 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012201 {
	print 'rt_vc012201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012201 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012202 {
	print 'rt_vc012202むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012202 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012203 {
	print 'rt_vc012203むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012203 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012301 {
	print 'rt_vc012301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012301 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012302 {
	print 'rt_vc012302むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012302 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012401 {
	print 'rt_vc012401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012401 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012501 {
	print 'rt_vc012501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012501 d:CODEC_DIRECT 終わり
}
proc rt_vc012502 {
	print 'rt_vc012502むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012502 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012601 {
	print 'rt_vc012601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012601 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012701 {
	print 'rt_vc012701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012701 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012801 {
	print 'rt_vc012801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012801 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc012901 {
	print 'rt_vc012901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc012901 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc013001 {
	print 'rt_vc013001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013001 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc013101 {
	print 'rt_vc013101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013101 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc013102 {
	print 'rt_vc013102むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013102 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc013103 {
	print 'rt_vc013103むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013103 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc013104 {
	print 'rt_vc013104むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013104 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc013105 {
	print 'rt_vc013105むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013105 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc013106 {
	print 'rt_vc013106むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013106 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc013201 {
	print 'rt_vc013201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013201 d:CODEC_DIRECT 終わり
}
// added on 01/03/21
proc rt_vc013202 {
	print 'rt_vc013202むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013202 d:CODEC_DIRECT 終わり
}
proc rt_vc013301 {
	print 'rt_vc013301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013301 d:CODEC_DIRECT 終わり
}
proc rt_vc013302 {
	print 'rt_vc013302むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013302 d:CODEC_DIRECT 終わり
}
proc rt_vc013303 {
	print 'rt_vc013303むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013303 d:CODEC_DIRECT 終わり
}
proc rt_vc013304 {
	print 'rt_vc013304むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013304 d:CODEC_DIRECT 終わり
}
proc rt_vc013305 {
	print 'rt_vc013305むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013305 d:CODEC_DIRECT 終わり
}
proc rt_vc013306 {
	print 'rt_vc013306むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013306 d:CODEC_DIRECT 終わり
}
proc rt_vc013307 {
	print 'rt_vc013307むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013307 d:CODEC_DIRECT 終わり
}
proc rt_vc013308 {
	print 'rt_vc013308むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013308 d:CODEC_DIRECT 終わり
}
proc rt_vc013401 {
	print 'rt_vc013401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013401 d:CODEC_DIRECT 終わり
}
proc rt_vc013501 {
	print 'rt_vc013501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013501 d:CODEC_DIRECT 終わり
}
proc rt_vc013502 {
	print 'rt_vc013502むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013502 d:CODEC_DIRECT 終わり
}
proc rt_vc013503 {
	print 'rt_vc013503むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013503 d:CODEC_DIRECT 終わり
}
proc rt_vc013504 {
	print 'rt_vc013504むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013504 d:CODEC_DIRECT 終わり
}

proc rt_vc013601 {
	print 'rt_vc013601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013601 d:CODEC_DIRECT 終わり
}
proc rt_vc013602 {
	print 'rt_vc013602むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013602 d:CODEC_DIRECT 終わり
}
proc rt_vc013701 {
	print 'rt_vc013701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013701 d:CODEC_DIRECT 終わり
}
proc rt_vc013801 {
	print 'rt_vc013801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013801 d:CODEC_DIRECT 終わり
}
proc rt_vc013901 {
	print 'rt_vc013901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013901 d:CODEC_DIRECT 終わり
}
proc rt_vc013902 {
	print 'rt_vc013902むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013902 d:CODEC_DIRECT 終わり
}
proc rt_vc013903 {
	print 'rt_vc013903むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc013903 d:CODEC_DIRECT 終わり
}
proc rt_vc014001 {
	print 'rt_vc014001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014001 d:CODEC_DIRECT 終わり
}
proc rt_vc014101 {
	print 'rt_vc014101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014101 d:CODEC_DIRECT 終わり
}
proc rt_vc014201 {
	print 'rt_vc014201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014201 d:CODEC_DIRECT 終わり
}
proc rt_vc014301 {
	print 'rt_vc014301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014301 d:CODEC_DIRECT 終わり
}
proc rt_vc014401 {
	print 'rt_vc014401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014401 d:CODEC_DIRECT 終わり
}
proc rt_vc014402 {
	print 'rt_vc014402むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014402 d:CODEC_DIRECT 終わり
}
proc rt_vc014403 {
	print 'rt_vc014403むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014403 d:CODEC_DIRECT 終わり
}
proc rt_vc014501 {
	print 'rt_vc014501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014501 d:CODEC_DIRECT 終わり
}
proc rt_vc014502 {
	print 'rt_vc014502むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014502 d:CODEC_DIRECT 終わり
}
proc rt_vc014601 {
	print 'rt_vc014601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014601 d:CODEC_DIRECT 終わり
}
proc rt_vc014701 {
	print 'rt_vc014701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014701 d:CODEC_DIRECT 終わり
}
proc rt_vc014702 {
	print 'rt_vc014702むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014702 d:CODEC_DIRECT 終わり
}
proc rt_vc014801 {
	print 'rt_vc014801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014801 d:CODEC_DIRECT 終わり
}
proc rt_vc014901 {
	print 'rt_vc014901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014901 d:CODEC_DIRECT 終わり
}
proc rt_vc014902 {
	print 'rt_vc014902むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc014902 d:CODEC_DIRECT 終わり
}
proc rt_vc015001 {
	print 'rt_vc015001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015001 d:CODEC_DIRECT 終わり
}
proc rt_vc015101 {
	print 'rt_vc015101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015101 d:CODEC_DIRECT 終わり
}
proc rt_vc015102 {
	print 'rt_vc015102むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015102 d:CODEC_DIRECT 終わり
}

proc rt_vc015201 {
	print 'rt_vc015201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015201 d:CODEC_DIRECT 終わり
}
proc rt_vc015301 {
	print 'rt_vc015301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015301 d:CODEC_DIRECT 終わり
}
proc rt_vc015401 {
	print 'rt_vc015401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015401 d:CODEC_DIRECT 終わり
}
proc rt_vc015402 {
	print 'rt_vc015402むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015402 d:CODEC_DIRECT 終わり
}
proc rt_vc015501 {
	print 'rt_vc015501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015501 d:CODEC_DIRECT 終わり
}
proc rt_vc015502 {
	print 'rt_vc015502むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015502 d:CODEC_DIRECT 終わり
}
proc rt_vc015601 {
	print 'rt_vc015601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015601 d:CODEC_DIRECT 終わり
}
proc rt_vc015602 {
	print 'rt_vc015602むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015602 d:CODEC_DIRECT 終わり
}
proc rt_vc015603 {
	print 'rt_vc015603むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015603 d:CODEC_DIRECT 終わり
}
proc rt_vc015604 {
	print 'rt_vc015604むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015604 d:CODEC_DIRECT 終わり
}
proc rt_vc015605 {
	print 'rt_vc015605むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015605 d:CODEC_DIRECT 終わり
}
proc rt_vc015606 {
	print 'rt_vc015606むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015606 d:CODEC_DIRECT 終わり
}
proc rt_vc015607 {
	print 'rt_vc015607むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015607 d:CODEC_DIRECT 終わり
}
proc rt_vc015701 {
	print 'rt_vc015701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015701 d:CODEC_DIRECT 終わり
}
proc rt_vc015801 {
	print 'rt_vc015801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015801 d:CODEC_DIRECT 終わり
}
proc rt_vc015901 {
	print 'rt_vc015901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc015901 d:CODEC_DIRECT 終わり
}
proc rt_vc016001 {
	print 'rt_vc016001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016001 d:CODEC_DIRECT 終わり
}
proc rt_vc016201 {
	print 'rt_vc016201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016201 d:CODEC_DIRECT 終わり
}
proc rt_vc016202 {
	print 'rt_vc016202むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016202 d:CODEC_DIRECT 終わり
}
proc rt_vc016301 {
	print 'rt_vc016301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016301 d:CODEC_DIRECT 終わり
}
proc rt_vc016302 {
	print 'rt_vc016302むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016302 d:CODEC_DIRECT 終わり
}
proc rt_vc016401 {
	print 'rt_vc016401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016401 d:CODEC_DIRECT 終わり
}
proc rt_vc016501 {
	print 'rt_vc016501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016501 d:CODEC_DIRECT 終わり
}
proc rt_vc016601 {
	print 'rt_vc016601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016601 d:CODEC_DIRECT 終わり
}
proc rt_vc016701 {
	print 'rt_vc016701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016701 d:CODEC_DIRECT 終わり
}
proc rt_vc016702 {
	print 'rt_vc016702むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016702 d:CODEC_DIRECT 終わり
}
proc rt_vc016703 {
	print 'rt_vc016703むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016703 d:CODEC_DIRECT 終わり
}
proc rt_vc016801 {
	print 'rt_vc016801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016801 d:CODEC_DIRECT 終わり
}
proc rt_vc016802 {
	print 'rt_vc016802むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016802 d:CODEC_DIRECT 終わり
}
proc rt_vc016803 {
	print 'rt_vc016803むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016803 d:CODEC_DIRECT 終わり
}
proc rt_vc016901 {
	print 'rt_vc016901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016901 d:CODEC_DIRECT 終わり
}
proc rt_vc016902 {
	print 'rt_vc016902むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016902 d:CODEC_DIRECT 終わり
}
proc rt_vc017001X {
	print 'rt_vc017001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017001X d:CODEC_DIRECT 終わり
}
proc rt_vc017001XX {
	print 'rt_vc017001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017001XX d:CODEC_DIRECT 終わり
}
proc rt_vc017001XXX {
	print 'rt_vc017001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017001XXX d:CODEC_DIRECT 終わり
}
proc rt_vc017101 {
	print 'rt_vc017101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017101 d:CODEC_DIRECT 終わり
}
proc rt_vc017102 {
	print 'rt_vc017102むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017102 d:CODEC_DIRECT 終わり
}
proc rt_vc017103X {
	print 'rt_vc017103むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017103X d:CODEC_DIRECT 終わり
}
proc rt_vc017103XX {
	print 'rt_vc017103むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017103XX d:CODEC_DIRECT 終わり
}
proc rt_vc017201 {
	print 'rt_vc017201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017201 d:CODEC_DIRECT 終わり
}
proc rt_vc017202 {
	print 'rt_vc017202むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017202 d:CODEC_DIRECT 終わり
}
proc rt_vc017203 {
	print 'rt_vc017203むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017203 d:CODEC_DIRECT 終わり
}
proc rt_vc017301 {
	print 'rt_vc017301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017301 d:CODEC_DIRECT 終わり
}
proc rt_vc017401 {
	print 'rt_vc017401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017401 d:CODEC_DIRECT 終わり
}
proc rt_vc017501 {
	print 'rt_vc017501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017501 d:CODEC_DIRECT 終わり
}
proc rt_vc017502X {
	print 'rt_vc017502むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017502X d:CODEC_DIRECT 終わり
}
proc rt_vc017502XX {
	print 'rt_vc017502むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017502XX d:CODEC_DIRECT 終わり
}
proc rt_vc017601 {
	print 'rt_vc017601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017601 d:CODEC_DIRECT 終わり
}
proc rt_vc017701 {
	print 'rt_vc017701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017701 d:CODEC_DIRECT 終わり
}
proc rt_vc017801 {
	print 'rt_vc017801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017801 d:CODEC_DIRECT 終わり
}
proc rt_vc017901 {
	print 'rt_vc017901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc017901 d:CODEC_DIRECT 終わり
}
proc rt_vc018001 {
	print 'rt_vc018001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc018001 d:CODEC_DIRECT 終わり
}
proc rt_vc018101 {
	print 'rt_vc018101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc018101 d:CODEC_DIRECT 終わり
}
proc rt_vc018102 {
	print 'rt_vc018102むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc018102 d:CODEC_DIRECT 終わり
}
proc rt_vc018201 {
	print 'rt_vc018201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc018201 d:CODEC_DIRECT 終わり
}
proc rt_vc018301 {
	print 'rt_vc018301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc018301 d:CODEC_DIRECT 終わり
}
proc rt_vc018401 {
	print 'rt_vc018401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc018401 d:CODEC_DIRECT 終わり
}
proc rt_vc018501 {
	print 'rt_vc018501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc018501 d:CODEC_DIRECT 終わり
}
proc rt_vc018601 {
	print 'rt_vc018601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc018601 d:CODEC_DIRECT 終わり
}

/*
追加されました。
*/
proc rt_vc018701 {
	print 'rt_vc018701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc018701 d:CODEC_DIRECT 終わり
}
proc rt_vc018801 {
	print 'rt_vc018801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc018801 d:CODEC_DIRECT 終わり
}
proc rt_vc018901 {
	print 'rt_vc018901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc018901 d:CODEC_DIRECT 終わり
}
proc rt_vc019001 {
	print 'rt_vc019001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc019001 d:CODEC_DIRECT 終わり
}
proc rt_vc019101 {
	print 'rt_vc019101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc019101 d:CODEC_DIRECT 終わり
}
proc rt_vc019201 {
	print 'rt_vc019201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc019201 d:CODEC_DIRECT 終わり
}

//ついかっす
proc rt_vc016101 {
	print 'rt_vc016101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc016101 d:CODEC_DIRECT 終わり
}

proc rt_vc002404 {
	print 'vc002404むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc002404 d:CODEC_DIRECT 終わり
}

proc rt_vc004803 {
	print 'vc004803むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc004803 d:CODEC_DIRECT 終わり
}

/*------------------------------------------*/
/* タンカー編セーブ無線分　平野追加　010410 */
/*------------------------------------------*/
proc rt_vc020101 {
	print 'vc020101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc020101 d:CODEC_DIRECT 終わり
}

proc rt_vc020201 {
	print 'vc020201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc020201 d:CODEC_DIRECT 終わり
}

proc rt_vc020301 {
	print 'vc020301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc020301 d:CODEC_DIRECT 終わり
}

proc rt_vc020401 {
	print 'vc020401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc020401 d:CODEC_DIRECT 終わり
}

proc rt_vc020501 {
	print 'vc020501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc020501 d:CODEC_DIRECT 終わり
}

proc rt_vc020601 {
	print 'vc020601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc020601 d:CODEC_DIRECT 終わり
}

proc rt_vc020701 {
	print 'vc020701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc020701 d:CODEC_DIRECT 終わり
}

proc rt_vc020801 {
	print 'vc020801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc020801 d:CODEC_DIRECT 終わり
}

proc rt_vc020901 {
	print 'vc020901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc020901 d:CODEC_DIRECT 終わり
}

proc rt_vc021001 {
	print 'vc021001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc021001 d:CODEC_DIRECT 終わり
}

proc rt_vc021101 {
	print 'vc021101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc021101 d:CODEC_DIRECT 終わり
}

proc rt_vc021201 {
	print 'vc021201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc021201 d:CODEC_DIRECT 終わり
}

proc rt_vc021301 {
	print 'vc021301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc021301 d:CODEC_DIRECT 終わり
}

proc rt_vc021401 {
	print 'vc021401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc021401 d:CODEC_DIRECT 終わり
}

proc rt_vc021501 {
	print 'vc021501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc021501 d:CODEC_DIRECT 終わり
}

proc rt_vc021601 {
	print 'vc021601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc021601 d:CODEC_DIRECT 終わり
}

proc rt_vc021701 {
	print 'vc021701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc021701 d:CODEC_DIRECT 終わり
}

proc rt_vc021801 {
	print 'vc021801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc021801 d:CODEC_DIRECT 終わり
}

proc rt_vc021901 {
	print 'vc021901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc021901 d:CODEC_DIRECT 終わり
}

proc rt_vc022001 {
	print 'vc022001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc022001 d:CODEC_DIRECT 終わり
}

proc rt_vc022101 {
	print 'vc022101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc022101 d:CODEC_DIRECT 終わり
}

proc rt_vc022201 {
	print 'vc022201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc022201 d:CODEC_DIRECT 終わり
}

proc rt_vc022301 {
	print 'vc022301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc022301 d:CODEC_DIRECT 終わり
}

proc rt_vc022401 {
	print 'vc022401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc022401 d:CODEC_DIRECT 終わり
}

proc rt_vc022501 {
	print 'vc022501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc022501 d:CODEC_DIRECT 終わり
}

proc rt_vc022601 {
	print 'vc022601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc022601 d:CODEC_DIRECT 終わり
}

proc rt_vc022701 {
	print 'vc022701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc022701 d:CODEC_DIRECT 終わり
}

proc rt_vc022801 {
	print 'vc022801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc022801 d:CODEC_DIRECT 終わり
}

proc rt_vc022901 {
	print 'vc022901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc022901 d:CODEC_DIRECT 終わり
}

proc rt_vc023001 {
	print 'vc023001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc023001 d:CODEC_DIRECT 終わり
}

proc rt_vc023101 {
	print 'vc023101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc023101 d:CODEC_DIRECT 終わり
}

proc rt_vc023201 {
	print 'vc023201むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc023201 d:CODEC_DIRECT 終わり
}

proc rt_vc023301 {
	print 'vc023301むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc023301 d:CODEC_DIRECT 終わり
}

proc rt_vc023401 {
	print 'vc023401むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc023401 d:CODEC_DIRECT 終わり
}

proc rt_vc023501 {
	print 'vc023501むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc023501 d:CODEC_DIRECT 終わり
}

proc rt_vc023601 {
	print 'vc023601むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc023601 d:CODEC_DIRECT 終わり
}

proc rt_vc023701 {
	print 'vc023701むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc023701 d:CODEC_DIRECT 終わり
}

proc rt_vc023801 {
	print 'vc023801むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc023801 d:CODEC_DIRECT 終わり
}

proc rt_vc023901 {
	print 'vc023901むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc023901 d:CODEC_DIRECT 終わり
}

proc rt_vc024001 {
	print 'vc024001むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc024001 d:CODEC_DIRECT 終わり
}

proc rt_vc024101 {
	print 'vc024101むがが'
	command 無線設定 \
		-call 14112 t:RTO_vc024101 d:CODEC_DIRECT 終わり
}




/*---------------------------------------------------
セレクトキャラセットだにゅん。
----------------------------------------------------*/
proc enter_demo {
	eval($f:rft_enter_demo = 1)
	restart
}

proc enter_demo_at {
	eval($f:rft_enter_demo_at = 1)
	restart
}

// added by S.Hirano 01/02/16
proc enter_trial_codec {
	eval($f:rft_enter_trial_codec = 1)
	restart
}

// added by S.Hirano 01/02/16
proc enter_after_trial_codec {
	eval($f:rft_enter_after_trial_codec = 1)
	restart
}

// added by S.Hirano 01/04/10
proc enter_save {
	eval($f:rft_enter_save = 1)
	restart
}

proc enter_otacon_add {
	eval($f:rft_enter_otacon_add = 1)
	restart
}

proc enter_vc000101_vc000701 {
print '$f:rft_enter_vc000101_vc000701 = 1です'
	eval($f:rft_enter_vc000101_vc000701 = 1)
	restart
}

proc enter_vc000801X_vc001401 {
	eval($f:rft_vc000801X_vc001401 = 1)
	restart
}

proc enter_vc001501_vc002201 {
	eval($f:rft_enter_vc001501_vc002201 = 1)
	restart
}

proc enter_vc002301_vc002801 {
	eval($f:rft_enter_vc002301_vc002801 = 1)
	restart
}

proc enter_vc003201_vc004101 {
	eval($f:rft_enter_vc003201_vc004101 = 1)
	restart
}

proc enter_vc004201_vc007601 {
	eval($f:rft_enter_vc004201_vc007601 = 1)
	restart
}

proc enter_vc007602_vc008201 {
	eval($f:rft_enter_vc007602_vc008201 = 1)
	restart
}

proc enter_vc008301_vc008505 {
	eval($f:rft_enter_vc008301_vc008505 = 1)
	restart
}

proc enter_vc008601_vc013305 {
	eval($f:rft_enter_vc008601_vc013305 = 1)
	restart
}

proc enter_vc013306_vc013602 {
	eval($f:rft_enter_vc013306_vc013602 = 1)
	restart
}

proc enter_vc013701_vc014401 {
	eval($f:rft_enter_vc013701_vc014401 = 1)
	restart
}

proc enter_vc014402_vc014902 {
	eval($f:rft_enter_vc014402_vc014902 = 1)
	restart
}

proc enter_vc015001_vc015502 {
	eval($f:rft_enter_vc015001_vc015502 = 1)
	restart
}

proc enter_vc015601_vc015901 {
	eval($f:rft_enter_vc015601_vc015901 = 1)
	restart
}

proc enter_vc016001_vc016801 {
	eval($f:rft_enter_vc016001_vc016801 = 1)
	restart
}

proc enter_vc016802_vc017103X {
	eval($f:rft_enter_vc016802_vc017103X = 1)
	restart
}

proc enter_vc017103XX_vc017601 {
	eval($f:rft_enter_vc017103XX_vc017601 = 1)
	restart
}

proc enter_vc017701_vc018701 {
	eval($f:rft_enter_vc017701_vc018701 = 1)
	restart
}

proc enter_vc018801_vc019201 {
	eval($f:rft_enter_vc018801_vc019201 = 1)
	restart
}

proc enter_modified {
	eval($f:rft_enter_modified = 1)
	restart
}

proc enter_add1 {
	eval($f:rft_enter_add1 = 1)
	restart
}

// 以下は体験版後分
proc enter_vc002901 {
	eval($f:rft_enter_vc002901 = 1)
	restart
}

proc enter_vc003001_vc003901 {
	eval($f:rft_enter_vc003001_vc003901 = 1)
	restart
}

proc enter_vc004001_vc004901 {
	eval($f:rft_enter_vc004001_vc004901 = 1)
	restart
}

proc enter_vc005201_vc005904 {
	eval($f:rft_enter_vc005201_vc005904 = 1)
	restart
}

proc enter_vc006001_vc006901 {
	eval($f:rft_enter_vc006001_vc006901 = 1)
	restart
}

proc enter_vc007001_vc007402 {
	eval($f:rft_enter_vc007001_vc007402 = 1)
	restart
}

proc enter_vc008901 {
	eval($f:rft_enter_vc008901 = 1)
	restart
}
/*
proc enter_vc009001_vc009402 {
	eval($f:rft_enter_vc009001_vc009402 = 1)
	restart
}
*/

// added on 01/03/21
proc enter_vc009001_vc009901 {
	eval($f:rft_enter_vc009001_vc009901 = 1)
	restart
}

// added on 01/03/21
proc enter_vc010001_vc010903 {
	eval($f:rft_enter_vc010001_vc010903 = 1)
	restart
}

// added on 01/03/21
proc enter_vc011001_vc011902 {
	eval($f:rft_enter_vc011001_vc011902 = 1)
	restart
}

// added on 01/03/21
proc enter_vc012001_vc012901 {
	eval($f:rft_enter_vc012001_vc012901 = 1)
	restart
}

// added on 01/03/21
proc enter_vc013001_vc013202 {
	eval($f:rft_enter_vc013001_vc013202 = 1)
	restart
}

// added on 01/04/10
proc enter_vc020101_vc020901 {
	eval($f:rft_enter_vc020101_vc020901 = 1)
	restart
}

// added on 01/04/10
proc enter_vc021001_vc021901 {
	eval($f:rft_enter_vc021001_vc021901 = 1)
	restart
}

// added on 01/04/10
proc enter_vc022001_vc022901 {
	eval($f:rft_enter_vc022001_vc022901 = 1)
	restart
}

// added on 01/04/10
proc enter_vc023001_vc023901 {
	eval($f:rft_enter_vc023001_vc023901 = 1)
	restart
}

// added on 01/04/10
proc enter_vc024001_vc024101 {
	eval($f:rft_enter_vc024001_vc024101 = 1)
	restart
}


#if 1
// 体験版後分
proc codec_set_root {
	chara select root -s {
		'demo'			enter_demo
		'demo after trial'	enter_demo_at
		'trial'			enter_trial_codec
		'after trial'	enter_after_trial_codec
		'save'	enter_save
		'otacon add' enter_otacon_add
		'exit' exit_codec_debug_mode
	}
}
#else
proc codec_set_root {
	chara select root -s {
		'demo' enter_demo
		'vc000101-vc000701'	enter_vc000101_vc000701
		'vc000801-vc001401'	enter_vc000801X_vc001401
		'vc001501-vc002201'	enter_vc001501_vc002201
		'vc002301-vc002801'	enter_vc002301_vc002801
		'vc003201-vc004101'	enter_vc003201_vc004101
		'vc004201-vc007601'	enter_vc004201_vc007601
		'vc007602-vc008201'	enter_vc007602_vc008201
		'vc008301-vc008505'	enter_vc008301_vc008505
		'vc008601-vc013305'	enter_vc008601_vc013305
		'vc013306-vc013602'	enter_vc013306_vc013602
		'vc013701-vc014401'	enter_vc013701_vc014401
		'vc014402-vc014902'	enter_vc014402_vc014902
		'vc015001-vc015502'	enter_vc015001_vc015502
		'vc015601-vc015901'	enter_vc015601_vc015901
		'vc016001-vc016801'	enter_vc016001_vc016801
		'vc016802-vc017103'	enter_vc016802_vc017103X
		'vc017104-vc017601'	enter_vc017103XX_vc017601
		'vc017701-vc018701'	enter_vc017701_vc018701
		'vc018801-vc019201'	enter_vc018801_vc019201
		'modified'	enter_modified
		'add1'	enter_add1
		'exit' exit_codec_debug_mode
	}
}
#endif

proc codec_set_demo {
	chara select demo -s {
		'00b1R_opening vc050101' rt_vc050101
		'00b1R_opening vc050204' rt_vc050204
//		'03b1R_after_Kamov_trial vc050301' rt_vc050301
		'04a0R_birdge vc050401' rt_vc050401
		'10b1R_hold_2 vc080401' rt_vc080401
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc rt_06b1R_after_olga {
	command 無線設定 \
	-call 14112 t:RTO_06b1Rオルガ戦勝利後無線機デモ１ d:CODEC_DIRECT 終わり
}

proc rt_10b1R_hold_2 {
	command 無線設定 \
	-call 14112 t:RTO_10b1R船倉無線機デモ２ d:CODEC_DIRECT 終わり
}

proc codec_set_demo_at {
	chara select demo_at -s {
		'03b1R_after_Kamov_product vc050302' rt_vc050302
		'06b1R_after_olga' rt_06b1R_after_olga
		'09b1R_hold_1 vc080301' rt_vc080301
		'10b1R_hold_2' rt_10b1R_hold_2
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

// added by S.Hirano 02/01/16
proc codec_set_trial {
	chara select trial -s {
		'vc000101-vc000701'	enter_vc000101_vc000701
		'vc000801-vc001401'	enter_vc000801X_vc001401
		'vc001501-vc002201'	enter_vc001501_vc002201
		'vc002301-vc002801'	enter_vc002301_vc002801
		'vc003201-vc004101'	enter_vc003201_vc004101
		'vc004201-vc007601'	enter_vc004201_vc007601
		'vc007602-vc008201'	enter_vc007602_vc008201
		'vc008301-vc008505'	enter_vc008301_vc008505
		'vc008601-vc013305'	enter_vc008601_vc013305
		'vc013306-vc013602'	enter_vc013306_vc013602
		'vc013701-vc014401'	enter_vc013701_vc014401
		'vc014402-vc014902'	enter_vc014402_vc014902
		'vc015001-vc015502'	enter_vc015001_vc015502
		'vc015601-vc015901'	enter_vc015601_vc015901
		'vc016001-vc016801'	enter_vc016001_vc016801
		'vc016802-vc017103'	enter_vc016802_vc017103X
		'vc017104-vc017601'	enter_vc017103XX_vc017601
		'vc017701-vc018701'	enter_vc017701_vc018701
		'vc018801-vc019201'	enter_vc018801_vc019201
		'modified'	enter_modified
		'add1'	enter_add1
		'exit' exit_codec_debug_mode
	}
}

// added by S.Hirano 02/01/16
proc codec_set_after_trial {
	chara select after_trial -s {
		'vc002901'	enter_vc002901
		'vc003001_vc003901'	enter_vc003001_vc003901
		'vc004001_vc004901'	enter_vc004001_vc004901
		'vc005201_vc005904'	enter_vc005201_vc005904
		'vc006001_vc006901'	enter_vc006001_vc006901
		'vc007001_vc007402'	enter_vc007001_vc007402
		'vc008902'	enter_vc008901
		'vc009001_vc009901'	enter_vc009001_vc009901 // added on 01/03/21
		'vc010001_vc010903'	enter_vc010001_vc010903 // added on 01/03/21
		'vc011001_vc011902'	enter_vc011001_vc011902 // added on 01/03/21
		'vc012001_vc012901'	enter_vc012001_vc012901 // added on 01/03/21
		'vc013001_vc013202'	enter_vc013001_vc013202 // added on 01/03/21
		'exit' exit_codec_debug_mode
	}
}

// added by S.Hirano 01/04/09
proc codec_set_save {
	chara select save -s {
		'vc020101_vc020901'	enter_vc020101_vc020901
		'vc021001_vc021901'	enter_vc021001_vc021901
		'vc022001_vc022901'	enter_vc022001_vc022901
		'vc023001_vc023901'	enter_vc023001_vc023901
		'vc024001_vc024101'	enter_vc024001_vc024101
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc000101_vc000701 {
	chara select vc000101_vc000701 -s {
		'vc000101' rt_vc000101
		'vc000201' rt_vc000201
		'vc000301' rt_vc000301
		'vc000302' rt_vc000302
		'vc000303' rt_vc000303
		'vc000401' rt_vc000401X
		'vc000402' rt_vc000401XX
		'vc000501' rt_vc000501
		'vc000601' rt_vc000601
		'vc000701' rt_vc000701
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}



proc codec_set_vc000801X_vc001401 {
	chara select vc000801X_vc001401 -s {
		'vc000801' rt_vc000801X
		'vc000802' rt_vc000802X
		'vc000803' rt_vc000802XX
		'vc000804' rt_vc000803
		'vc000901' rt_vc000901
		'vc001001' rt_vc001001
		'vc001101' rt_vc001101
		'vc001201' rt_vc001201
		'vc001301' rt_vc001301
		'vc001401' rt_vc001401
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc001501_vc002201 {
	chara select vc001501_vc002201 -s {
		'vc001501' rt_vc001501
		'vc001601' rt_vc001601
		'vc001701' rt_vc001701
		'vc001702' rt_vc001702
		'vc001801' rt_vc001801
		'vc001901' rt_vc001901
		'vc002001' rt_vc002001
		'vc002002' rt_vc002002
		'vc002101' rt_vc002101
		'vc002201' rt_vc002201
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc002301_vc002801 {
	chara select vc002301_vc002801 -s {
		'vc002301' rt_vc002301
		'vc002401' rt_vc002401
		'vc002402X' rt_vc002402
		'vc002403' rt_vc002403
		'vc002501' rt_vc002501
		'vc002502' rt_vc002502
		'vc002503' rt_vc002503
		'vc002601' rt_vc002601
		'vc002701' rt_vc002701
		'vc002801' rt_vc002801
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc003201_vc004101 {
	chara select vc003201_vc004101 -s {
		'vc003201' rt_vc003201
		'vc003202' rt_vc003202
		'vc003203' rt_vc003203
		'vc003204' rt_vc003204
		'vc003205' rt_vc003205
		'vc003206' rt_vc003206
		'vc003301' rt_vc003301
		'vc003801' rt_vc003801X
		'vc003802' rt_vc003801XX
		'vc004101' rt_vc004101
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc004201_vc007601 {
	chara select vc004201_vc007601 -s {
		'vc004201' rt_vc004201
		'vc004202' rt_vc004202
		'vc004301' rt_vc004301
		'vc004501' rt_vc004501
		'vc005001' rt_vc005001
		'vc005101' rt_vc005101
		'vc007301' rt_vc007301
		'vc007501' rt_vc007501
		'vc007502' rt_vc007502
		'vc007601' rt_vc007601
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc007602_vc008201 {
	chara select vc007602_vc008201 -s {
		'vc007602' rt_vc007602
		'vc007701' rt_vc007701
		'vc007901' rt_vc007901X
		'vc007902' rt_vc007901XX
		'vc007801' rt_vc007801
		'vc008001' rt_vc008001
		'vc008101' rt_vc008101
		'vc008102' rt_vc008102
		'vc008103' rt_vc008103
		'vc008201' rt_vc008201
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc008301_vc008505 {
	chara select vc008301_vc008505 -s {
		'vc008301' rt_vc008301
		'vc008401' rt_vc008401
		'vc008402' rt_vc008402
		'vc008403' rt_vc008403
		'vc008404' rt_vc008404
		'vc008501' rt_vc008501
		'vc008502' rt_vc008502
		'vc008503' rt_vc008503
		'vc008504' rt_vc008504
		'vc008505' rt_vc008505
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc008601_vc013305 {
	chara select vc008601_vc013305 -s {
		'vc008601' rt_vc008601
		'vc008701' rt_vc008701
		'vc008801' rt_vc008801
		'vc009401' rt_vc009401
		'vc009403' rt_vc009403
		'vc013301' rt_vc013301
		'vc013302' rt_vc013302
		'vc013303' rt_vc013303
		'vc013304' rt_vc013304
		'vc013305' rt_vc013305
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc013306_vc013602 {
	chara select vc013306_vc013602 -s {
		'vc013306' rt_vc013306
		'vc013307' rt_vc013307
		'vc013308' rt_vc013308
		'vc013401' rt_vc013401
		'vc013501' rt_vc013501
		'vc013502' rt_vc013502
		'vc013503' rt_vc013503
		'vc013504' rt_vc013504
		'vc013601' rt_vc013601
		'vc013602' rt_vc013602
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc013701_vc014401 {
	chara select vc013701_vc014401 -s {
		'vc013701' rt_vc013701
		'vc013801' rt_vc013801
		'vc013901' rt_vc013901
		'vc013902' rt_vc013902
		'vc013903' rt_vc013903
		'vc014001' rt_vc014001
		'vc014101' rt_vc014101
		'vc014201' rt_vc014201
		'vc014301' rt_vc014301
		'vc014401' rt_vc014401
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}
proc codec_set_vc014402_vc014902 {
	chara select vc014402_vc014902 -s {
		'vc014402' rt_vc014402
		'vc014403' rt_vc014403
		'vc014501' rt_vc014501
		'vc014502' rt_vc014502
		'vc014601' rt_vc014601
		'vc014701' rt_vc014701
		'vc014702' rt_vc014702
		'vc014801' rt_vc014801
		'vc014901' rt_vc014901
		'vc014902' rt_vc014902
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}
proc codec_set_vc015001_vc015502 {
	chara select vc015001_vc015502 -s {
		'vc015001' rt_vc015001
		'vc015101' rt_vc015101
		'vc015102' rt_vc015102
		'vc015201' rt_vc015201
		'vc015301' rt_vc015301
		'vc015401' rt_vc015401
		'vc015402' rt_vc015402
		'vc015501' rt_vc015501
		'vc015502' rt_vc015502
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}
proc codec_set_vc015601_vc015901 {
	chara select vc015601_vc015901 -s {
		'vc015601' rt_vc015601
		'vc015602' rt_vc015602
		'vc015603' rt_vc015603
		'vc015604' rt_vc015604
		'vc015605' rt_vc015605
		'vc015606' rt_vc015606
		'vc015607' rt_vc015607
		'vc015701' rt_vc015701
		'vc015801' rt_vc015801
		'vc015901' rt_vc015901
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}
proc codec_set_vc016001_vc016801 {
	chara select vc016001_vc016801 -s {
		'vc016001' rt_vc016001
		'vc016201' rt_vc016201
		'vc016202' rt_vc016202
		'vc016301' rt_vc016301
		'vc016302' rt_vc016302
		'vc016401' rt_vc016401
		'vc016501' rt_vc016501
		'vc016601' rt_vc016601
		'vc016701' rt_vc016701
		'vc016702' rt_vc016702
		'vc016703' rt_vc016703
		'vc016801' rt_vc016801
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}
proc codec_set_vc016802_vc017103X {
	chara select vc016802_vc017103X -s {
		'vc016802' rt_vc016802
		'vc016803' rt_vc016803
		'vc016901' rt_vc016901
		'vc016902' rt_vc016902
		'vc017001' rt_vc017001X
		'vc017002' rt_vc017001XX
		'vc017003' rt_vc017001XXX
		'vc017101' rt_vc017101
		'vc017102' rt_vc017102
		'vc017103' rt_vc017103X
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}
proc codec_set_vc017103XX_vc017601 {
	chara select vc017104_vc017601 -s {
		'vc017104' rt_vc017103XX
		'vc017201' rt_vc017201
		'vc017202' rt_vc017202
		'vc017203' rt_vc017203
		'vc017301' rt_vc017301
		'vc017401' rt_vc017401
		'vc017501' rt_vc017501
		'vc017502' rt_vc017502X
		'vc017503' rt_vc017502XX
		'vc017601' rt_vc017601
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}
proc codec_set_vc017701_vc018701 {
	chara select vc017701_vc018701 -s {
		'vc017701' rt_vc017701
		'vc017801' rt_vc017801
		'vc017901' rt_vc017901
		'vc018001' rt_vc018001
		'vc018101' rt_vc018101
		'vc018102' rt_vc018102
		'vc018201' rt_vc018201
		'vc018301' rt_vc018301
		'vc018401' rt_vc018401
		'vc018501' rt_vc018501
		'vc018601' rt_vc018601
		'vc018701' rt_vc018701
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}
proc codec_set_vc018801_vc019201 {
	chara select vc018801_vc019201 -s {
		'vc018801' rt_vc018801
		'vc018901' rt_vc018901
		'vc019001' rt_vc019001
		'vc019101' rt_vc019101
		'vc019201' rt_vc019201
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}
proc codec_set_modified {
	chara select modified -s {
		'vc000401' rt_vc000401X
		'vc000402' rt_vc000401XX

		'vc000801' rt_vc000801X
		'vc000802' rt_vc000802X
		'vc000803' rt_vc000802XX
		'vc000804' rt_vc000803

		'vc003801' rt_vc003801X
		'vc003802' rt_vc003801XX

		'vc007601' rt_vc007601
		'vc007602' rt_vc007602

		'vc007901' rt_vc007901X
		'vc007902' rt_vc007901XX

		'vc017001' rt_vc017001X
		'vc017002' rt_vc017001XX
		'vc017003' rt_vc017001XXX

		'vc017103' rt_vc017103X
		'vc017104' rt_vc017103XX

		'vc017502' rt_vc017502X
		'vc017503' rt_vc017502XX
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_add1 {
	chara select add1 -s {
		'vc016101' rt_vc016101
		'vc002404' rt_vc002404
		'vc004803' rt_vc004803
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

// 以下は体験版後分
proc codec_set_vc002901 {
	chara select vc002901 -s {
		'vc002901' rt_vc002901
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc003001_vc003901 {
	chara select vc003001_vc003901 -s {
		'vc003001' rt_vc003001
		'vc003101' rt_vc003101
		'vc003401' rt_vc003401
		'vc003507' rt_vc003507
		'vc003601' rt_vc003601
		'vc003701' rt_vc003701
		'vc003901' rt_vc003901
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc004001_vc004901 {
	chara select vc004001_vc004901 -s {
		'vc004001' rt_vc004001
		'vc004203' rt_vc004203
		'vc004401' rt_vc004401
		'vc004601' rt_vc004601
		'vc004701' rt_vc004701
		'vc004801' rt_vc004801
		'vc004802' rt_vc004802
//		'vc004901' rt_vc004901
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc005201_vc005904 {
	chara select vc005201_vc005904 -s {
		'vc005201' rt_vc005201
		'vc005301' rt_vc005301
		'vc005401' rt_vc005401
		'vc005501' rt_vc005501
		'vc005601' rt_vc005601
		'vc005701' rt_vc005701
		'vc005801' rt_vc005801
		'vc005901' rt_vc005901
		'vc005902' rt_vc005902
		'vc005903' rt_vc005903
		'vc005904' rt_vc005904
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc006001_vc006901 {
	chara select vc006001_vc006901 -s {
		'vc006001' rt_vc006001
		'vc006002' rt_vc006002
		'vc006101' rt_vc006101
		'vc006201' rt_vc006201
		'vc006301' rt_vc006301
		'vc006401' rt_vc006401
		'vc006501' rt_vc006501
		'vc006601' rt_vc006601
		'vc006701' rt_vc006701
		'vc006801' rt_vc006801
		'vc006901' rt_vc006901
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc007001_vc007402 {
	chara select vc007001_vc007402 -s {
		'vc007001' rt_vc007001
		'vc007101' rt_vc007101
//		'vc007102' rt_vc007102
		'vc007103' rt_vc007103
		'vc007104' rt_vc007104
		'vc007105' rt_vc007105
		'vc007201' rt_vc007201
		'vc007401' rt_vc007401
		'vc007402' rt_vc007402
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

proc codec_set_vc008901 {
	chara select rt_vc008901 -s {
		'vc008902' rt_vc008901
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

/* added on 01/03/22 */
proc codec_set_vc009001_vc009901 {
	chara select rt_vc009001_vc009901 -s {
		'vc009001' rt_vc009001
		'vc009101' rt_vc009101
		'vc009201' rt_vc009201
		'vc009301' rt_vc009301
		'vc009402' rt_vc009402

		'vc009501' rt_vc009501
		'vc009502' rt_vc009502
		'vc009601' rt_vc009601
		'vc009602' rt_vc009602
		'vc009603' rt_vc009603
		'vc009604' rt_vc009604
		'vc009605' rt_vc009605
		'vc009606' rt_vc009606
		'vc009701' rt_vc009701
		'vc009801' rt_vc009801
		'vc009901' rt_vc009901
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}
/* added on 01/03/21 */
proc codec_set_vc010001_vc010903 {
	chara select vc010001_vc010903 -s {
		'vc010001' rt_vc010001
		'vc010101' rt_vc010101
		'vc010102' rt_vc010102
		'vc010103' rt_vc010103
		'vc010201' rt_vc010201
		'vc010301' rt_vc010301
		'vc010401' rt_vc010401
		'vc010402' rt_vc010402
//		'vc010501' rt_vc010501
		'vc010601' rt_vc010601
		'vc010701' rt_vc010701
		'vc010702' rt_vc010702
		'vc010703' rt_vc010703
		'vc010704' rt_vc010704
		'vc010705' rt_vc010705
		'vc010801' rt_vc010801
		'vc010802' rt_vc010802
		'vc010803' rt_vc010803
		'vc010804' rt_vc010804
		'vc010805' rt_vc010805
		'vc010806' rt_vc010806
		'vc010807' rt_vc010807
		'vc010808' rt_vc010808
		'vc010809' rt_vc010809
		'vc010810' rt_vc010810
		'vc010811' rt_vc010811
		'vc010812' rt_vc010812
		'vc010813' rt_vc010813
		'vc010814' rt_vc010814
		'vc010901' rt_vc010901
		'vc010902' rt_vc010902
		'vc010903' rt_vc010903
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}
/* added on 01/03/21 */
proc codec_set_vc011001_vc011902 {
	chara select vc011001_vc011902 -s {
		'vc011001' rt_vc011001
		'vc011101' rt_vc011101
		'vc011201' rt_vc011201
		'vc011202' rt_vc011202
		'vc011203' rt_vc011203
		'vc011204' rt_vc011204
		'vc011205' rt_vc011205
		'vc011206' rt_vc011206
		'vc011207' rt_vc011207
		'vc011208' rt_vc011208
		'vc011209' rt_vc011209
		'vc011210' rt_vc011210
		'vc011211' rt_vc011211
		'vc011212' rt_vc011212
		'vc011301' rt_vc011301
		'vc011302' rt_vc011302
		'vc011303' rt_vc011303
		'vc011304' rt_vc011304
		'vc011401' rt_vc011401
		'vc011402' rt_vc011402
		'vc011403' rt_vc011403
		'vc011404' rt_vc011404
		'vc011405' rt_vc011405
		'vc011501' rt_vc011501
		'vc011502' rt_vc011502
		'vc011503' rt_vc011503
		'vc011601' rt_vc011601
		'vc011602' rt_vc011602
		'vc011603' rt_vc011603
		'vc011701' rt_vc011701
		'vc011702' rt_vc011702
		'vc011801' rt_vc011801
		'vc011802' rt_vc011802
		'vc011901' rt_vc011901
		'vc011902' rt_vc011902
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}
/* added on 01/03/21 */
proc codec_set_vc012001_vc012901 {
	chara select vc012001_vc012901 -s {
		'vc012001' rt_vc012001
		'vc012002' rt_vc012002
		'vc012101' rt_vc012101
		'vc012102' rt_vc012102
		'vc012103' rt_vc012103
		'vc012104' rt_vc012104
		'vc012201' rt_vc012201
		'vc012202' rt_vc012202
		'vc012203' rt_vc012203
		'vc012301' rt_vc012301
		'vc012302' rt_vc012302
		'vc012401' rt_vc012401
		'vc012501' rt_vc012501
		'vc012502' rt_vc012502
		'vc012601' rt_vc012601
		'vc012701' rt_vc012701
		'vc012801' rt_vc012801
		'vc012901' rt_vc012901
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}
/* added on 01/03/21 */
proc codec_set_vc013001_vc013202 {
	chara select vc013001_vc013202 -s {
		'vc013001' rt_vc013001
		'vc013101' rt_vc013101
		'vc013102' rt_vc013102
		'vc013103' rt_vc013103
		'vc013104' rt_vc013104
		'vc013105' rt_vc013105
		'vc013106' rt_vc013106
		'vc013201' rt_vc013201
		'vc013202' rt_vc013202
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

/* added on 01/04/10 */
proc codec_set_vc020101_vc020901 {
	chara select vc020101_vc020901 -s {
		'vc020101' rt_vc020101
		'vc020201' rt_vc020201
		'vc020301' rt_vc020301
		'vc020401' rt_vc020401
		'vc020501' rt_vc020501
		'vc020601' rt_vc020601
		'vc020701' rt_vc020701
		'vc020801' rt_vc020801
		'vc020901' rt_vc020901
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

/* added on 01/04/10 */
proc codec_set_vc021001_vc021901 {
	chara select vc021001_vc021901 -s {
		'vc021001' rt_vc021001
		'vc021101' rt_vc021101
		'vc021201' rt_vc021201
		'vc021301' rt_vc021301
		'vc021401' rt_vc021401
		'vc021501' rt_vc021501
		'vc021601' rt_vc021601
		'vc021701' rt_vc021701
		'vc021801' rt_vc021801
		'vc021901' rt_vc021901
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

/* added on 01/04/10 */
proc codec_set_vc022001_vc022901 {
	chara select vc022001_vc022901 -s {
		'vc022001' rt_vc022001
		'vc022101' rt_vc022101
		'vc022201' rt_vc022201
		'vc022301' rt_vc022301
		'vc022401' rt_vc022401
		'vc022501' rt_vc022501
		'vc022601' rt_vc022601
		'vc022701' rt_vc022701
		'vc022801' rt_vc022801
		'vc022901' rt_vc022901
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

/* added on 01/04/10 */
proc codec_set_vc023001_vc023901 {
	chara select vc023001_vc023901 -s {
		'vc023001' rt_vc023001
		'vc023101' rt_vc023101
		'vc023201' rt_vc023201
		'vc023301' rt_vc023301
		'vc023401' rt_vc023401
		'vc023501' rt_vc023501
		'vc023601' rt_vc023601
		'vc023701' rt_vc023701
		'vc023801' rt_vc023801
		'vc023901' rt_vc023901
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}

/* added on 01/04/10 */
proc codec_set_vc024001_vc024101 {
	chara select vc024001_vc024101 -s {
		'vc024001' rt_vc024001
		'vc024101' rt_vc024101
		'back' back_to_root
		'exit' exit_codec_debug_mode
	}
}


#if 1
// 体験版後分含む
proc codec_debug_list_set_t {
	if($f:rft_enter_demo){
		@codec_set_demo

	}else if($f:rft_enter_demo_at){
		@codec_set_demo_at

	}else if($f:rft_enter_trial_codec){

		if($f:rft_enter_vc000101_vc000701){
			@codec_set_vc000101_vc000701

		}else if($f:rft_vc000801X_vc001401){
			@codec_set_vc000801X_vc001401

		}else if($f:rft_enter_vc001501_vc002201){
			@codec_set_vc001501_vc002201

		}else if($f:rft_enter_vc002301_vc002801){
			@codec_set_vc002301_vc002801

		}else if($f:rft_enter_vc003201_vc004101){
			@codec_set_vc003201_vc004101

		}else if($f:rft_enter_vc004201_vc007601){
			@codec_set_vc004201_vc007601

		}else if($f:rft_enter_vc007602_vc008201){
			@codec_set_vc007602_vc008201

		}else if($f:rft_enter_vc008301_vc008505){
			@codec_set_vc008301_vc008505

		}else if($f:rft_enter_vc008601_vc013305){
			@codec_set_vc008601_vc013305

		}else if($f:rft_enter_vc013306_vc013602){
			@codec_set_vc013306_vc013602

		}else if($f:rft_enter_vc013701_vc014401){
			@codec_set_vc013701_vc014401

		}else if($f:rft_enter_vc014402_vc014902){
			@codec_set_vc014402_vc014902

		}else if($f:rft_enter_vc015001_vc015502){
			@codec_set_vc015001_vc015502

		}else if($f:rft_enter_vc015601_vc015901){
			@codec_set_vc015601_vc015901

		}else if($f:rft_enter_vc016001_vc016801){
			@codec_set_vc016001_vc016801

		}else if($f:rft_enter_vc016802_vc017103X){
			@codec_set_vc016802_vc017103X

		}else if($f:rft_enter_vc017103XX_vc017601){
			@codec_set_vc017103XX_vc017601

		}else if($f:rft_enter_vc017701_vc018701){
			@codec_set_vc017701_vc018701

		}else if($f:rft_enter_vc018801_vc019201){
			@codec_set_vc018801_vc019201

		}else if($f:rft_enter_modified){
			@codec_set_modified

		}else if($f:rft_enter_add1){
			@codec_set_add1

		}else {
			@codec_set_trial

		}

	}else if($f:rft_enter_after_trial_codec){


// フラグは後でclear_flagの中で０に。


		if($f:rft_enter_vc002901){
			@codec_set_vc002901

		}else if($f:rft_enter_vc003001_vc003901){
			@codec_set_vc003001_vc003901

		}else if($f:rft_enter_vc004001_vc004901){
			@codec_set_vc004001_vc004901

		}else if($f:rft_enter_vc005201_vc005904){
			@codec_set_vc005201_vc005904

		}else if($f:rft_enter_vc006001_vc006901){
			@codec_set_vc006001_vc006901

		}else if($f:rft_enter_vc007001_vc007402){
			@codec_set_vc007001_vc007402

		}else if($f:rft_enter_vc008901){
			@codec_set_vc008901

		// added on 01/03/21
		}else if($f:rft_enter_vc009001_vc009901){
			@codec_set_vc009001_vc009901

		// added on 01/03/21
		}else if($f:rft_enter_vc010001_vc010903){
			@codec_set_vc010001_vc010903

		// added on 01/03/21
		}else if($f:rft_enter_vc011001_vc011902){
			@codec_set_vc011001_vc011902

		// added on 01/03/21
		}else if($f:rft_enter_vc012001_vc012901){
			@codec_set_vc012001_vc012901

		// added on 01/03/21
		}else if($f:rft_enter_vc013001_vc013202){
			@codec_set_vc013001_vc013202

		}else{
			@codec_set_after_trial
		}

	}else if($f:rft_enter_save){

// セーブ会話分
// フラグは後でclear_flagの中で０に。

		if($f:rft_enter_vc020101_vc020901){
			@codec_set_vc020101_vc020901

		}else if($f:rft_enter_vc021001_vc021901){
			@codec_set_vc021001_vc021901

		}else if($f:rft_enter_vc022001_vc022901){
			@codec_set_vc022001_vc022901

		}else if($f:rft_enter_vc023001_vc023901){
			@codec_set_vc023001_vc023901

		}else if($f:rft_enter_vc024001_vc024101){
			@codec_set_vc024001_vc024101

		} else {
			@codec_set_save
		}

	}else if($f:rft_enter_otacon_add) {
		@codec_debug_list_t_otacon_add

	}else {
		@clear_flag
		@codec_set_root
	}
}
#else
proc codec_debug_list_set_t {
	if($f:rft_enter_demo){
		@codec_set_demo

	}else if($f:rft_enter_vc000101_vc000701){
		@codec_set_vc000101_vc000701

	}else if($f:rft_vc000801X_vc001401){
		@codec_set_vc000801X_vc001401

	}else if($f:rft_enter_vc001501_vc002201){
		@codec_set_vc001501_vc002201

	}else if($f:rft_enter_vc002301_vc002801){
		@codec_set_vc002301_vc002801

	}else if($f:rft_enter_vc003201_vc004101){
		@codec_set_vc003201_vc004101

	}else if($f:rft_enter_vc004201_vc007601){
		@codec_set_vc004201_vc007601

	}else if($f:rft_enter_vc007602_vc008201){
		@codec_set_vc007602_vc008201

	}else if($f:rft_enter_vc008301_vc008505){
		@codec_set_vc008301_vc008505

	}else if($f:rft_enter_vc008601_vc013305){
		@codec_set_vc008601_vc013305

	}else if($f:rft_enter_vc013306_vc013602){
		@codec_set_vc013306_vc013602

	}else if($f:rft_enter_vc013701_vc014401){
		@codec_set_vc013701_vc014401

	}else if($f:rft_enter_vc014402_vc014902){
		@codec_set_vc014402_vc014902

	}else if($f:rft_enter_vc015001_vc015502){
		@codec_set_vc015001_vc015502

	}else if($f:rft_enter_vc015601_vc015901){
		@codec_set_vc015601_vc015901

	}else if($f:rft_enter_vc016001_vc016801){
		@codec_set_vc016001_vc016801

	}else if($f:rft_enter_vc016802_vc017103X){
		@codec_set_vc016802_vc017103X

	}else if($f:rft_enter_vc017103XX_vc017601){
		@codec_set_vc017103XX_vc017601

	}else if($f:rft_enter_vc017701_vc018701){
		@codec_set_vc017701_vc018701

	}else if($f:rft_enter_vc018801_vc019201){
		@codec_set_vc018801_vc019201

	}else if($f:rft_enter_modified){
		@codec_set_modified

	}else if($f:rft_enter_add1){
		@codec_set_add1

	}else {
		@clear_flag
		@codec_set_root
	}
}
#endif