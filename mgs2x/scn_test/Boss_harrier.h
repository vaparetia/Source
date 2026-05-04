/********************************************************************************/
/*	Boss_harrier.h								*/
/*	ボス ハリアー関連のプロック群 GCL呼び					*/
/*	2001/07/09 H.Satoyoshi							*/
/*	$Id:	*/
/********************************************************************************/

/*+++++++++++++++++++++++++++++++++++<proc>+++++++++++++++++++++++++++++++++++++*/
/*	名前:   エフェクト設定							*/
/*	説明:	エフェクト類を設定する						*/
/*		現状フォグのみ							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
proc フォグ設定 {

// 向手君作成の"室内に入ると暗くなる"設定のため「chara フォグ」を
// 明るさ可変機能に置き換えます。(2001.7.26 Y.Matsuhana)

/*
	chara フォグ 霧\
	-c 220, 237, 198 \
	-n -15000 \
	-f 300000 
*/

	@明るさ可変機能 220, 237, 198 \
				-20000 250000 \
				ev092 -1000

	chara ローカル風 local_wind \
	 -n 1 \
	 -d 0 0 0 0 0 0 \
	    0 \
	    0 \
	    0 



// データの並び
// ２点で立方体を表現（６つ）、
// その立方体内の風方向（Ｙ軸のみ１つ）
// 風方向ランダム幅（4096(=360度)）（全方向に加味される、１つ）
// 風速MAX（１つ）
// 以上９つの数字で一塊。

}



/*+++++++++++++++++++++++++++++++++++<proc>+++++++++++++++++++++++++++++++++++++*/
/*	名前:   ハリアー本体設置						*/
/*	説明:	ハリアー/カサッカの本体を設置する				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
proc ボスハリアー関連設定 {

    command プラグイン強制吹っ飛ばし

    command スティンガー性能	\
	-t 8			\
	-m 30000		\
	-p 252			\
	-d 0			\
	-life 240		\
	-s 0, -150, 0
	
    command 水位設定 -lv -40000
    command レーダーズーム設定 700

    chara ボスハリアー harrier	\
	-pos 1418,11901,-211965 \
	-life 1000 1000		\
	-follow 1		\
	-itemproc w25aハリアーアイテム出現	\
	-stageproc w25aステージ破壊		\
	-dieproc ハリアーアウト			\
	-eventproc w25aハリアー演出		\
	-onsei	t:vc104501 t:vc104502 t:vc104503 t:vc104510 t:vc104511 t:vc104512\
	        t:vc104513 t:vc104514 t:vc104515 t:vc104516 t:vc104524 t:vc104525\
	        t:vc104526 t:vc104530 t:vc104531 t:vc104532 t:vc104509\
	-color	128 130 132 128\			// ノズルブラー
	        212  86  64 156\			// ボンボリ色
	         54  42  32 128\			// Tempライトカラー
                 56  50  48 128   64  50  40  128\	// ノズル攻撃色
                 56  36  28 128   10   2   2  128\	// ロケットバーニア
                 72  56  48  96   16   2   0   64	// アムラームバーニア
	
	#ifndef d:BOSS_MODE
		chara ハリアー戦カサッカ kasacka		\
			-pos	14020 28741 -135617		\
			-onsei	t:vc104504 t:vc104505 t:vc104506 t:vc104507 t:vc104508 t:vc104518\
			        t:vc104519 t:vc104520 t:vc104591 t:vc104592\
			-eventproc カサッカイベント関数 \
			-dieproc カサッカ墜落関数
	#else
		if ( $s:ボスラッシュプレイヤー == スネーク ) {
			chara ボスラッシュカサッカ kasacka \
				-pos	14020 28741 -135617		\
				-onsei	t:vc104504 t:vc104505 t:vc104506 t:vc104507 t:vc104508 t:vc104518\
				        t:vc104519 t:vc104520 t:vc104591 t:vc104592\
				-eventproc カサッカイベント関数 \
				-dieproc カサッカ墜落関数
		} else {
			chara ハリアー戦カサッカ kasacka		\
				-pos	14020 28741 -135617		\
				-onsei	t:vc104504 t:vc104505 t:vc104506 t:vc104507 t:vc104508 t:vc104518\
				        t:vc104519 t:vc104520 t:vc104591 t:vc104592\
				-eventproc カサッカイベント関数 \
				-dieproc カサッカ墜落関数
		}
	#endif
 chara ハリアーＢＧＭマネ bgmmaneger
}
//added by Fukushima 01/08/19
proc カサッカ墜落関数 {
	eval($f:pgo_カサッカ死亡 = 1)
}


/*+++++++++++++++++++++++++++++++++++<proc>+++++++++++++++++++++++++++++++++++++*/
/*	名前:   拡張海面設定							*/
/*	説明:	拡張海を設定する						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
proc 拡張海面設置 {

#if 1
    chara 拡張海表示 sea_serface_ex		\
	-t	entyou_oil_sea			\
	-pos	0,-40000,-150000		\
	-color  145 185 120			\
	-lines  5
#elseif
chara 拡張海表示皿型 sara_umi		\
	-tex  d_oil_sea_alp_ovl_mod0222	\
	-pos  0,-40000,-150000		\
	-color 128 128 128		\
	-radius  5000 10000		\
	-hight   3000
#endif

    mesg プラント海面 plant_sea 上下運動計算処理 0
}

/*+++++++++++++++++++++++++++++++++++<proc>+++++++++++++++++++++++++++++++++++++*/
/*	名前:   字幕リソース登録						*/
/*	説明:	各言語対応字幕データを設定する					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

command 字幕言語設定
@ハリアＳＥ字幕リソース


//英
proc ハリアＳＥ字幕リソース:01 {
    resource ＳＥ字幕リソース英 {
		食らえ！:'Take this!'
		何処だ？:'Where are you!'
		そこか！:'There you are!'
		蜂の巣:'I\'ll shred you to pieces!'
	        くたばれ:'Go to hell!!!'
	        よけられ:'Try dodging this!'
	        あの世に:'I\'ll let you go out in style!'
		焼け死ね:'Burn, baby!!'
		ぬぅ:'Dammit!'
		糞っ:'Damn!'
		どうした:'What\'s this?'
	        効かんな:'That didn\'t hurt at all!'
		なにぃ！:'What the -- !!'
		馬鹿な！:'No!'
		}

		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT101 -r [ＳＥ字幕リソース英:食らえ！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT102 -r [ＳＥ字幕リソース英:何処だ？]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT103 -r [ＳＥ字幕リソース英:そこか！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT301 -r [ＳＥ字幕リソース英:蜂の巣]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT401 -r [ＳＥ字幕リソース英:くたばれ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT501 -r [ＳＥ字幕リソース英:よけられ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT601 -r [ＳＥ字幕リソース英:あの世に]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT701 -r [ＳＥ字幕リソース英:焼け死ね]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM101 -r [ＳＥ字幕リソース英:ぬぅ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM102 -r [ＳＥ字幕リソース英:糞っ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM201 -r [ＳＥ字幕リソース英:どうした]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM202 -r [ＳＥ字幕リソース英:効かんな]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM401 -r [ＳＥ字幕リソース英:なにぃ！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM402 -r [ＳＥ字幕リソース英:馬鹿な！]
	}
	//仏
	proc ハリアＳＥ字幕リソース:02 {
		resource ＳＥ字幕リソース仏 {
		食らえ！:'Prends &~qa !'
		何処だ？:'O&`u es-tu ?!'
		そこか！:'Tiens !'
		蜂の巣:'Je vais te transformer en chair &`a p&^at&\'e !'
		くたばれ:'Va en Enfer !!!'
		よけられ:'Voyons voir ce que tu fais de &~qa !'
		あの世に:'Je vais te faire partir en beaut&\'e,|tu vas voir !'
		焼け死ね:'Crame, mon pote !'
		ぬぅ:'Merde !'
		糞っ:'Mince !'
		どうした:'Tu rigoles ou quoi ?'
		効かんな:'Je n\'ai rien senti du tout !'
		なにぃ！:'Qu\'est-ce que… !'
		馬鹿な！:'Non !'
		}
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT101 -r [ＳＥ字幕リソース仏:食らえ！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT102 -r [ＳＥ字幕リソース仏:何処だ？]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT103 -r [ＳＥ字幕リソース仏:そこか！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT301 -r [ＳＥ字幕リソース仏:蜂の巣]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT401 -r [ＳＥ字幕リソース仏:くたばれ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT501 -r [ＳＥ字幕リソース仏:よけられ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT601 -r [ＳＥ字幕リソース仏:あの世に]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT701 -r [ＳＥ字幕リソース仏:焼け死ね]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM101 -r [ＳＥ字幕リソース仏:ぬぅ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM102 -r [ＳＥ字幕リソース仏:糞っ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM201 -r [ＳＥ字幕リソース仏:どうした]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM202 -r [ＳＥ字幕リソース仏:効かんな]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM401 -r [ＳＥ字幕リソース仏:なにぃ！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM402 -r [ＳＥ字幕リソース仏:馬鹿な！]
	}
	//独
	proc ハリアＳＥ字幕リソース:03 {
		resource ＳＥ字幕リソース独 {
		食らえ！:'Hier, nimm das!'
		何処だ？:'Wo bist Du!?'
		そこか！:'Ach, da!'
		蜂の巣:'Ich mache Hackfleisch aus Dir!'
		くたばれ:'Zur H&~olle mit Dir!!!'
		よけられ:'Na, kannst Du da noch ausweichen!?'
		あの世に:'Ich verschaffe Dir einen w&~urdigen Abgang!'
		焼け死ね:'Brenn, Baby!!'
		ぬぅ:'Verdammt!'
		糞っ:'Verdammt!'
		どうした:'Was ist das?'
		効かんな:'Das hat ja &~uberhaupt nicht wehgetan!'
		なにぃ！:'Was zum - !'
		馬鹿な！:'Nein!'
		}
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT101 -r [ＳＥ字幕リソース独:食らえ！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT102 -r [ＳＥ字幕リソース独:何処だ？]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT103 -r [ＳＥ字幕リソース独:そこか！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT301 -r [ＳＥ字幕リソース独:蜂の巣]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT401 -r [ＳＥ字幕リソース独:くたばれ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT501 -r [ＳＥ字幕リソース独:よけられ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT601 -r [ＳＥ字幕リソース独:あの世に]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT701 -r [ＳＥ字幕リソース独:焼け死ね]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM101 -r [ＳＥ字幕リソース独:ぬぅ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM102 -r [ＳＥ字幕リソース独:糞っ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM201 -r [ＳＥ字幕リソース独:どうした]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM202 -r [ＳＥ字幕リソース独:効かんな]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM401 -r [ＳＥ字幕リソース独:なにぃ！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM402 -r [ＳＥ字幕リソース独:馬鹿な！]
	}
	//伊
	proc ハリアＳＥ字幕リソース:04 {
		resource ＳＥ字幕リソース伊 {
		食らえ！:'Prendi questo!'
		何処だ？:'Dove sei finito!?'
		そこか！:'Ah, ecco dove!'
		蜂の巣:'Ti faccio a pezzi!'
		くたばれ:'Va\' all\'inferno!!!'
		よけられ:'Cerca di schivare questo!'
		あの世に:'Ti lascer&`o morire con stile!'
		焼け死ね:'Brucia, baby!!'
		ぬぅ:'Dannazione!'
		糞っ:'Maledizione!'
		どうした:'Cos\'&`e questo?'
		効かんな:'Non ho sentito niente!'
		なにぃ！:'Cosa diavolo -!!'
		馬鹿な！:'No!'
		}
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT101 -r [ＳＥ字幕リソース伊:食らえ！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT102 -r [ＳＥ字幕リソース伊:何処だ？]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT103 -r [ＳＥ字幕リソース伊:そこか！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT301 -r [ＳＥ字幕リソース伊:蜂の巣]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT401 -r [ＳＥ字幕リソース伊:くたばれ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT501 -r [ＳＥ字幕リソース伊:よけられ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT601 -r [ＳＥ字幕リソース伊:あの世に]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT701 -r [ＳＥ字幕リソース伊:焼け死ね]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM101 -r [ＳＥ字幕リソース伊:ぬぅ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM102 -r [ＳＥ字幕リソース伊:糞っ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM201 -r [ＳＥ字幕リソース伊:どうした]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM202 -r [ＳＥ字幕リソース伊:効かんな]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM401 -r [ＳＥ字幕リソース伊:なにぃ！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM402 -r [ＳＥ字幕リソース伊:馬鹿な！]
	}
	//西
	proc ハリアＳＥ字幕リソース:05 {
		resource ＳＥ字幕リソース西 {
		食らえ！:'&~!Toma &\'esta!'
		何処だ？:'&~!D&\'onde est&\'as!'
		そこか！:'&~!Ah&\'i est&\'as!'
		蜂の巣:'&~!Te har&\'e pedazos!'
		くたばれ:'&~!&~!&~!Vete al infierno!!!'
		よけられ:'&~!Intenta esquivar esto!'
		あの世に:'&~!Dejar&\'e que mueras con clase!'
		焼け死ね:'&~!&~!&~!Qu&\'emate chaval!!!'
		ぬぅ:'&~!Maldici&\'on!'
		糞っ:'&~!Mierda!'
		どうした:'&~?Qu&\'e es esto?'
		効かんな:'&~!Ni me he enterado!'
		なにぃ！:'&~!Qu&\'e demonios...!'
		馬鹿な！:'&~!No!'
		}
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT101 -r [ＳＥ字幕リソース西:食らえ！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT102 -r [ＳＥ字幕リソース西:何処だ？]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT103 -r [ＳＥ字幕リソース西:そこか！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT301 -r [ＳＥ字幕リソース西:蜂の巣]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT401 -r [ＳＥ字幕リソース西:くたばれ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT501 -r [ＳＥ字幕リソース西:よけられ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT601 -r [ＳＥ字幕リソース西:あの世に]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT701 -r [ＳＥ字幕リソース西:焼け死ね]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM101 -r [ＳＥ字幕リソース西:ぬぅ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM102 -r [ＳＥ字幕リソース西:糞っ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM201 -r [ＳＥ字幕リソース西:どうした]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM202 -r [ＳＥ字幕リソース西:効かんな]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM401 -r [ＳＥ字幕リソース西:なにぃ！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM402 -r [ＳＥ字幕リソース西:馬鹿な！]
	}
	//韓
	proc ハリアＳＥ字幕リソース:06 {
		resource ＳＥ字幕リソース韓 {
		食らえ！:'Take this!'
		何処だ？:'Where are you!'
		そこか！:'There you are!'
		蜂の巣:'I\'ll shred you to pieces!'
	        くたばれ:'Go to hell!!!'
	        よけられ:'Try dodging this!'
	        あの世に:'I\'ll let you go out in style!'
		焼け死ね:'Burn, baby!!'
		ぬぅ:'Dammit!'
		糞っ:'Damn!'
		どうした:'What\'s this?'
	        効かんな:'That didn\'t hurt at all!'
		なにぃ！:'What the -- !!'
		馬鹿な！:'No!'
		}
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT101 -r [ＳＥ字幕リソース韓:食らえ！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT102 -r [ＳＥ字幕リソース韓:何処だ？]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT103 -r [ＳＥ字幕リソース韓:そこか！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT301 -r [ＳＥ字幕リソース韓:蜂の巣]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT401 -r [ＳＥ字幕リソース韓:くたばれ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT501 -r [ＳＥ字幕リソース韓:よけられ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT601 -r [ＳＥ字幕リソース韓:あの世に]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT701 -r [ＳＥ字幕リソース韓:焼け死ね]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM101 -r [ＳＥ字幕リソース韓:ぬぅ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM102 -r [ＳＥ字幕リソース韓:糞っ]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM201 -r [ＳＥ字幕リソース韓:どうした]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM202 -r [ＳＥ字幕リソース韓:効かんな]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM401 -r [ＳＥ字幕リソース韓:なにぃ！]
		command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM402 -r [ＳＥ字幕リソース韓:馬鹿な！]
		    }






