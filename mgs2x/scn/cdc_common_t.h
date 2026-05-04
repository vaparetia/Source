//
//	cdc_common_t.h
//	タンカー編！無線共通関数用
//
//	2001/05/16	T.Fukushima
//	$Id: cdc_common_t.h,v 1.12 2001/09/05 09:32:45 usr02011 Exp $
//chara 無線システム Codec -player sna_def


extern command プレイヤー状態取得
extern command varsave

proc rt_重要ＣＡＬＬしてもいいよ {
	if(`%ゲームオーバーチェック` != 1){
		return 1
	}else {
		return 0
	}
}

proc rt_通常ＣＡＬＬしてもいいよ {
	if( (`%StreamIsPlay` == 0) && \
		(`%無線状態` == 0) && \
		(`%ゲームオーバーチェック` != 1) ){
		return 1
	}else {
		return 0
	}
}

proc rt_懸垂ＣＡＬＬ終了 {
	eval($f:rft_懸垂握力アップ後聞いた = 1)
	command varsave $f:rft_懸垂握力アップ後聞いた
}

proc rt_懸垂ＣＡＬＬ {
	if( ($w:スネーク懸垂回数 >= 100) && (!$f:rft_懸垂握力アップ後聞いた) && \
		(!$f:rft_懸垂ＣＡＬＬした) && (`@rt_重要ＣＡＬＬしてもいいよ`) ){
		eval($f:rft_懸垂ＣＡＬＬした = 1)
		command 無線設定 \
			-call 14112 t:RTO_懸垂ＣＡＬＬ d:CODEC_CALL rt_懸垂ＣＡＬＬ終了
	}
}


//流血してます！聞いてません！の時はCALLしてみるのもいいかな？
if($s:エリア != $s:rft_前のステージ){
	eval($f:rft_ロード後流血ＣＡＬＬ上等 = 1)
}else {
	eval($f:rft_ロード後流血ＣＡＬＬ上等 = 0)
}
eval($s:rft_前のステージ = $s:エリア)
command varsave $s:rft_前のステージ


proc rt_流血ＣＡＬＬ起動連続実行 {
	if( ($f:ロードチェックＯＮフラグ) && (!$f:rft_流血ＣＡＬＬした) && \
		(!$f:rft_流血止血条件説明聞いた) && \
		($w:t_story < d:ST_T04a1D艦橋カモフ発見ポリゴンデモ開始) ){
			eval($f:rft_流血ＣＡＬＬした = 1)
			command 無線設定 \
				-call 14112 t:RTO_流血状態ＣＡＬＬ d:CODEC_CALL
	}
}

command プレイヤー状態取得 -d
if( ($status & d:PFLAG_BLOOD) && (!$f:rft_流血止血条件説明聞いた) && \
	($f:rft_ロード後流血ＣＡＬＬ上等) && ($w:t_story < d:ST_機関室赤外線センサークリア) ){
	chara プロック連続実行 流血ＣＡＬＬ起動さま \
		-proc rt_流血ＣＡＬＬ起動連続実行 \
		-time 300
}



proc rt_オタコンＳＡＶＥ＿通常無線セット {
	command 無線設定 \
	-base 14096 t:RTS_ＳＡＶＥデフォルト
}

proc rt_オタコンＳＡＶＥ＿デフォルト無線セット {
	@rt_オタコンＳＡＶＥ＿通常無線セット
}

proc rt_短縮ダイアル＿タンカーオタコン {
	command 無線メモリー設定  -area 0 -name 0 -freq 14112
}
proc rt_短縮ダイアル＿タンカーＳＡＶＥ {
	command 無線メモリー設定  -area 1 -name 1 -freq 14096
}

command 無線メモリー設定 -reset
@rt_短縮ダイアル＿タンカーオタコン
@rt_短縮ダイアル＿タンカーＳＡＶＥ


