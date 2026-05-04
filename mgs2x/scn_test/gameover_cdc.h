/*
	gameover_cdc.h
	    ゲームオーバー時の特殊無線処理を行うファイルかもしれない。
	    gameover.hでincludeされる、多分。      

	2000/11/05 T.Fukushima         
	$Id: gameover_cdc.h,v 1.1 2002/02/01 06:08:31 usr01475 Exp $                      

	
*/

proc rt_orga_gameover_flag_set{

	#define r_オルガ最大体力 128
	command ゲットオルガライフ $w:オルガライフ
	eval($w:rft_オルガＬＩＦＥぱぁせんと = ($w:オルガライフ * 100) / d:r_オルガ最大体力 )
	eval($w:rft_スネークＬＩＦＥぱぁせんと = ($w:体力* 100) / $w:体力最大)
//	print '$w:rft_オルガＬＩＦＥぱぁせんと:'$w:rft_オルガＬＩＦＥぱぁせんと
//	print '$w:rft_スネークＬＩＦＥぱぁせんと:'$w:rft_スネークＬＩＦＥぱぁせんと

	if($w:rft_ゲームオーバー時オルガＬＩＦＥ平均 != 0){
		eval($w:rft_ゲームオーバー時オルガＬＩＦＥ平均 = \
			($w:rft_ゲームオーバー時オルガＬＩＦＥ平均 + $w:rft_オルガＬＩＦＥぱぁせんと) / 2 )
	}else {
		eval($w:rft_ゲームオーバー時オルガＬＩＦＥ平均 = $w:rft_オルガＬＩＦＥぱぁせんと )
	}
//	print '$w:rft_ゲームオーバー時オルガＬＩＦＥ平均:'$w:rft_ゲームオーバー時オルガＬＩＦＥ平均

	//ゲームオーバー回数インクリメント
	eval($w:rft_オルガ戦ゲームオーバー回数 = $w:rft_オルガ戦ゲームオーバー回数 +1)

	//ヒントＣＡＬＬ出すかどうか判定
	if( (2 <= $w:rft_オルガ戦ゲームオーバー回数) && ($w:rft_オルガ戦ゲームオーバー回数 <= 5) &&\
		($w:rft_ゲームオーバー時オルガＬＩＦＥ平均 >= 70) ){
		eval($f:rtf_オルガ戦ヒントＣＡＬＬする１ = 1)
	}else {
		eval($f:rtf_オルガ戦ヒントＣＡＬＬする１ = 0)
	}

	//変数ＳＡＶＥ
	command varsave $w:rft_ゲームオーバー時オルガＬＩＦＥ平均 
	command varsave $w:rft_オルガ戦ゲームオーバー回数 
	command varsave $f:rtf_オルガ戦ヒントＣＡＬＬする１
	command varsave $f:rtf_オルガ戦ヒントＣＡＬＬ１聞いた
}


proc rt_無線ゲームオーバー処理 {
#ifdef STAGE_W00b
		@rt_orga_gameover_flag_set
#endif
}

