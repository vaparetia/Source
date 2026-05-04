/*
	cap_resource.h
	    字幕のリソース

	2002/02/28 H.Yoshiike
	$Id: cap_resource.h,v 1.6 2002/08/12 04:04:24 usr04761 Exp $


*/

#ifndef d:CAP_RESOURCE
#define	CAP_RESOURCE	1

#ifdef d:KOREA
//----------------------------------------------------------------
// 韓国版の字幕リソース
//----------------------------------------------------------------
	#ifdef d:STAGE_W00B
		proc ＳＥ字幕リソース {
		    resource 字幕韓 {
			隠れてるだけ:'需郊伽霜聖 馬切壱?'
			たいした事無いわね:'益幻 砧獣走!'
			逃げてるつもり:'需奄幻 拝暗劃?'
			バカにしてるの:'鎧亜 郊沙 匝 焼蟹?'
			食らえ:'限焼虞!'
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA01 -r [字幕韓:隠れてるだけ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA02 -r [字幕韓:たいした事無いわね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA22 -r [字幕韓:逃げてるつもり]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA23 -r [字幕韓:バカにしてるの]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_ORGA04  -r [字幕韓:食らえ]
		}
	#endif

	#ifdef d:STAGE_W11C
	// フォーチュン戦の字幕リソース
		proc ＳＥ字幕リソース {
		    resource 字幕韓 {
				あたら:'錯蟹穐陥. '
				お前に:'薦企稽 背坐虞.'
				早く私を殺して:'嬢辞 劾 宋食 左獣走.'
				何してるの？:'舌貝馬蟹! 劾 宋食坐! '
				出てらっ:'戚軒稽 蟹人!'
				だめね:'煽訓!'
				はずれ:'錯蟹穐陥.'
				殺してみなさ:'鎧 壱搭聖 魁舌鎧操!'
				貴方に私の悲:'獲 鎧 壱搭聖 侯虞!'
				当たってないわ:'貝 恥拭 限走 省澗陥. '
				当たらないわ:'暁 錯蟹穐陥. '
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN02 -r [字幕韓:出てらっ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN03 -r [字幕韓:だめね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN04 -r [字幕韓:はずれ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN05 -r [字幕韓:あたら]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN06 -r [字幕韓:お前に]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN11 -r [字幕韓:早く私を殺して]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN12 -r [字幕韓:何してるの？]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN13 -r [字幕韓:殺してみなさ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN14 -r [字幕韓:貴方に私の悲]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN15 -r [字幕韓:当たってないわ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN16 -r [字幕韓:当たらないわ]
		}
	#endif

	#ifdef d:STAGE_W20C
		// 韓国語
		proc ファットマンＳＥ字幕リソース {
			resource ＳＥ字幕リソース韓 {
				まぶしい:'鎧 勧!'
				みえない:'勧戚 照左食!'
				邪魔するな:'襖閃獄形!'
				何をする:'更 馬澗 憎戚走?'
				どかーん:'歌!'
				どこだ:'嬢巨 赤蟹!?'
				出てこい:'蟹人虞!'
				無駄だ:'廃号 限聖 桟 梅浦!'
				きかんな:'暁 詞側 什弾嬢!'
				落ちろ:'嬢渠..鎧形亜蟹?'
				ばかめ:'宜企亜軒!'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN01 -r [ＳＥ字幕リソース韓:まぶしい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN02 -r [ＳＥ字幕リソース韓:みえない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB01 -r [ＳＥ字幕リソース韓:邪魔するな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB02 -r [ＳＥ字幕リソース韓:何をする]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDOKN0 -r [ＳＥ字幕リソース韓:どかーん]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE01 -r [ＳＥ字幕リソース韓:どこだ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE02 -r [ＳＥ字幕リソース韓:出てこい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD01 -r [ＳＥ字幕リソース韓:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD02 -r [ＳＥ字幕リソース韓:きかんな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT01 -r [ＳＥ字幕リソース韓:落ちろ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT02 -r [ＳＥ字幕リソース韓:ばかめ]
		}
	#endif

	#ifdef d:STAGE_W25A
		//韓
		proc ハリアＳＥ字幕リソース {
			resource ＳＥ字幕リソース韓 {
				食らえ！:'股嬢隅!'
				何処だ？:'嬢巨 需醸蟹!'
				そこか！:'暗奄 赤浦!'
				蜂の巣:'亜欠稽 幻級嬢爽走!'
				くたばれ:'宋嬢虞!!!'
				よけられ:'嬢巨 杷背 左獣走!'
				あの世に:'鈷走惟 宋食爽畏陥!'
				焼け死ね:'宋嬢!!'
				ぬぅ:'薦掩!'
				糞っ:'賎舌!'
				どうした:'戚闇 更醤?'
				効かんな:'社遂 蒸陥!'
				なにぃ！:'更醤...!'
				馬鹿な！:'照掬!'
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
	#endif

	#ifdef d:STAGE_W31C
	// ヴァンプ戦の字幕リソース
		proc ヴァンプＳＥ字幕リソース {
			resource ＳＥ字幕リソース韓 {
				無駄だ！:'獣娃涯搾 馬走 原虞!'
				読めるぞ:'鎧 勧聖 杷馬形壱?'
				遅い:'格巷 汗形!'
				無駄だ:'嬢顕蒸走!'
				目が！:'鎧 勧!'
				見えん:'勧戚...照 左食!'
				動けない:'崇送析 呪亜 蒸嬢!?'
				体が！？:'鎧 倖!?'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB02 -r [ＳＥ字幕リソース韓:無駄だ！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB03 -r [ＳＥ字幕リソース韓:読めるぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR01 -r [ＳＥ字幕リソース韓:遅い]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR02 -r [ＳＥ字幕リソース韓:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN01 -r [ＳＥ字幕リソース韓:目が！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN02 -r [ＳＥ字幕リソース韓:見えん]

			#ifndef d:BOSS_MODE
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース韓:動けない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース韓:体が！？]
			#else
			if ( $s:ボスラッシュプレイヤー != スネーク ){
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース韓:動けない]
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース韓:体が！？]
			}
			#endif
		}
	#endif

	#ifdef d:STAGE_W61A
	// ソリダス戦の字幕リソース
		resource	ソリダス字幕リソース {
			// 韓
			さすがだな:	'Good work, Jack!'
			ここからだ: 'But this is where it gets interesting!'
		}

		// 韓
		proc ソリダスＳＥ字幕リソース {
			resource ＳＥ字幕リソース韓 {
				どうした:'舛源 廃宿馬浦!'
				そんなものか:'益 舛亀 鉱拭 公馬蟹?'
				そこまでか:'益幻 匂奄馬獣走, 先?'
				何してる:'更 馬切澗 暗劃!'
				ふざけるな:'舌貝精 益幻 団虞!'
				行くぞ:'疏焼!'
				くらえ:'戚暗蟹 股嬢!'
				しねい:'宋嬢!'
				こっちだ:'食奄 赤陥!'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO02 -r [ＳＥ字幕リソース韓:どうした]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO03 -r [ＳＥ字幕リソース韓:そんなものか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO04 -r [ＳＥ字幕リソース韓:そこまでか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU01 -r [ＳＥ字幕リソース韓:何してる]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU02 -r [ＳＥ字幕リソース韓:ふざけるな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA11 -r [ＳＥ字幕リソース韓:行くぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA12 -r [ＳＥ字幕リソース韓:くらえ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA13 -r [ＳＥ字幕リソース韓:しねい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLSPE01 -r [ＳＥ字幕リソース韓:こっちだ]
		}
	#endif

	#ifdef d:STAGE_W32A
	//	ＮＰＣスネークの字幕リソース
		//	韓
		proc ＮＰＣスネーク音声ＳＥリソース設定 {
			resource ＮＰＣスネーク音声ＳＥ韓国 {
				どこを狙っている:'業郊稽 繕層背!'
			};
			command ＳＥ字幕登録 -s d:se_code:SD_V_SNADKN01 -r [ＮＰＣスネーク音声ＳＥ韓国:どこを狙っている]
		}
	#endif








#else
//----------------------------------------------------------------
// 欧州版の字幕リソース(※言語選択がある場合は米国版もここになるかも)
//----------------------------------------------------------------
	#ifdef d:STAGE_W00B
		proc ＳＥ字幕リソース:01 {
		    resource 字幕英 {
			隠れてるだけ:'Are you just going to play hide and seek?'
			たいした事無いわね:'Not much there!'
			逃げてるつもり:'Is that your idea of hiding?'
			バカにしてるの:'What do you think I am, stupid?'
			食らえ:'Take This!'
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA01 -r [字幕英:隠れてるだけ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA02 -r [字幕英:たいした事無いわね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA22 -r [字幕英:逃げてるつもり]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA23 -r [字幕英:バカにしてるの]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_ORGA04  -r [字幕英:食らえ]
		}

		proc ＳＥ字幕リソース:02 {
		    resource 字幕仏 {
			隠れてるだけ:'Tu vas jouer &`a cache-cache encore longtemps ?'
			たいした事無いわね:'Je ne crois pas !'
			逃げてるつもり:'C\'est comme &~qa que tu te caches, toi ?'
			バカにしてるの:'Tu me prends pour une d&\'ebile ou quoi ?'
			食らえ:'Prends &~qa!'
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA01 -r [字幕仏:隠れてるだけ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA02 -r [字幕仏:たいした事無いわね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA22 -r [字幕仏:逃げてるつもり]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA23 -r [字幕仏:バカにしてるの]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_ORGA04  -r [字幕仏:食らえ]
		}

		proc ＳＥ字幕リソース:03 {
		    resource 字幕独 {
			隠れてるだけ:'Hast Du jetzt vor, Verstecken zu spielen?'
			たいした事無いわね:'Das bringt nichts!'
			逃げてるつもり:'Ist das Deine Art, Dich zu verstecken?'
			バカにしてるの:'Du glaubst wohl, ich bin dumm?'
			食らえ:'Nimm\' das!'
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA01 -r [字幕独:隠れてるだけ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA02 -r [字幕独:たいした事無いわね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA22 -r [字幕独:逃げてるつもり]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA23 -r [字幕独:バカにしてるの]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_ORGA04  -r [字幕独:食らえ]
		}

		proc ＳＥ字幕リソース:04 {
		    resource 字幕伊 {
			隠れてるだけ:'Vuoi continuare a giocare a nascondino per molto?'
			たいした事無いわね:'Puah, che mezza calzetta!'
			逃げてるつもり:'&`E questa la tua idea di un nascondiglio?'
			バカにしてるの:'Pensi che sia cos&`i stupida?'
			食らえ:'Prendi questo!'
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA01 -r [字幕伊:隠れてるだけ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA02 -r [字幕伊:たいした事無いわね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA22 -r [字幕伊:逃げてるつもり]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA23 -r [字幕伊:バカにしてるの]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_ORGA04  -r [字幕伊:食らえ]
		}

		proc ＳＥ字幕リソース:05 {
		    resource 字幕西 {
			隠れてるだけ:'&~?Vas a seguir jugando al escondite o qu&\'e?'
			たいした事無いわね:'&~!No por mucho m&\'as!'
			逃げてるつもり:'&~?Y crees que est&\'as bien escondido?'
			バカにしてるの:'&~?Qu&\'e crees que soy boba?'
			食らえ:'&~!Coge esto!'
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA01 -r [字幕西:隠れてるだけ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA02 -r [字幕西:たいした事無いわね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA22 -r [字幕西:逃げてるつもり]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA23 -r [字幕西:バカにしてるの]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_ORGA04  -r [字幕西:食らえ]
		}


		proc ＳＥ字幕リソース:07 {
		    resource 字幕日 {
			隠れてるだけ:'隠れてるだけ！？'
			たいした事無いわね:'たいしたことないわね！？'
			逃げてるつもり:'逃げてるつもり！？'
			バカにしてるの:'馬鹿にしてるの！？'
			食らえ:'くらえ！'
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA01 -r [字幕日:隠れてるだけ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA02 -r [字幕日:たいした事無いわね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA22 -r [字幕日:逃げてるつもり]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_SORGA23 -r [字幕日:バカにしてるの]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_ORGA04  -r [字幕日:食らえ]
		}
	#endif

	#ifdef d:STAGE_W11C
	// フォーチュン戦の字幕リソース
		proc ＳＥ字幕リソース:01 {
		    resource 字幕英 {
				あたら:'That went nowhere.'
				お前に:'What good are you ...'
				早く私を殺して:'Kill me now.'
				何してるの？:'What are you doing! Kill me!'
				出てらっ:'Come on out!'
				だめね:'Sorry!'
				はずれ:'You missed.'
				殺してみなさ:'Put me out of my misery now!'
				貴方に私の悲:'You can\'t understand my pain!'
				当たってないわ:'I\'m not hit.'
				当たらないわ:'You didn\'t hit me.'
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN02 -r [字幕英:出てらっ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN03 -r [字幕英:だめね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN04 -r [字幕英:はずれ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN05 -r [字幕英:あたら]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN06 -r [字幕英:お前に]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN11 -r [字幕英:早く私を殺して]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN12 -r [字幕英:何してるの？]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN13 -r [字幕英:殺してみなさ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN14 -r [字幕英:貴方に私の悲]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN15 -r [字幕英:当たってないわ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN16 -r [字幕英:当たらないわ]
		}

		proc ＳＥ字幕リソース:02 {
		    resource 字幕仏 {
				あたら:'Ce tir &\'etait minable.'
				お前に:'Tu sais tirer ...'
				早く私を殺して:'Tue-moi, maintenant !'
				何してるの？:'Qu\'est-ce que tu attends ! Tue-moi !'
			 	出てらっ:'Sors de ton trou !'
				だめね:'D&\'esol&\'ee !'
				はずれ:'Tu as rat&\'e ton coup.'
				殺してみなさ:'Mets un terme &`a tous mes malheurs !'
				貴方に私の悲:'Tu ne sais pas &`a quel point je souffre !'
				当たってないわ:'Je n\'ai pas &\'et&\'e touch&\'ee.'
				当たらないわ:'Tu ne m\'as pas touch&\'ee.'
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN02 -r [字幕仏:出てらっ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN03 -r [字幕仏:だめね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN04 -r [字幕仏:はずれ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN05 -r [字幕仏:あたら]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN06 -r [字幕仏:お前に]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN11 -r [字幕仏:早く私を殺して]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN12 -r [字幕仏:何してるの？]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN13 -r [字幕仏:殺してみなさ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN14 -r [字幕仏:貴方に私の悲]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN15 -r [字幕仏:当たってないわ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN16 -r [字幕仏:当たらないわ]
		}

		proc ＳＥ字幕リソース:03 {
		    resource 字幕独 {
				あたら:'Das ging ja total daneben.'
				お前に:'Wozu taugst Du &~uberhaupt...'
				早く私を殺して:'T&~ote mich jetzt!'
				何してるの？:'Was machst Du da!? T&~ote mich!'
				出てらっ:'Komm raus!'
				だめね:'Sorry!'
				はずれ:'Verfehlt.'
				殺してみなさ:'Erl&~ose mich jetzt!'
				貴方に私の悲:'Du kannst meinen Schmerz nicht verstehen!'
				当たってないわ:'Immer noch nicht getroffen.'
				当たらないわ:'Du hast mich nicht getroffen.'
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN02 -r [字幕独:出てらっ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN03 -r [字幕独:だめね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN04 -r [字幕独:はずれ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN05 -r [字幕独:あたら]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN06 -r [字幕独:お前に]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN11 -r [字幕独:早く私を殺して]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN12 -r [字幕独:何してるの？]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN13 -r [字幕独:殺してみなさ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN14 -r [字幕独:貴方に私の悲]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN15 -r [字幕独:当たってないわ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN16 -r [字幕独:当たらないわ]
		}

		proc ＳＥ字幕リソース:04 {
		    resource 字幕伊 {
				あたら:'Ti &`e andata buca.'
				お前に:'Sei una mezza calzetta...'
				早く私を殺して:'Uccidimi subito!'
				何してるの？:'Cosa fai!? Uccidimi!'
				出てらっ:'Vieni fuori!'
				だめね:'Spiacente!'
				はずれ:'Mi hai mancata.'
				殺してみなさ:'Poni fine alla mia miserabile esistenza!'
				貴方に私の悲:'Non puoi capire quanto soffro!'
				当たってないわ:'Non mi hai colpita.'
				当たらないわ:'Mi hai mancata.'
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN02 -r [字幕伊:出てらっ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN03 -r [字幕伊:だめね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN04 -r [字幕伊:はずれ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN05 -r [字幕伊:あたら]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN06 -r [字幕伊:お前に]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN11 -r [字幕伊:早く私を殺して]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN12 -r [字幕伊:何してるの？]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN13 -r [字幕伊:殺してみなさ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN14 -r [字幕伊:貴方に私の悲]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN15 -r [字幕伊:当たってないわ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN16 -r [字幕伊:当たらないわ]
		}

		proc ＳＥ字幕リソース:05 {
		    resource 字幕西 {
				あたら:'Se ha ido muy lejos.'
				お前に:'Qu&\'e malo eres...'
				早く私を殺して:'&~!M&\'atame ahora!'
				何してるの？:'Pero &~?qu&\'e haces? &~!M&\'atame!'
				出てらっ:'&~!Sal si te atreves!'
				だめね:'&~!Lo siento!'
				はずれ:'No me has dado.'
				殺してみなさ:'&~!Pon fin a este sinvivir!'
				貴方に私の悲:'&~!Nunca podr&\'as entender mi sufrimiento!'
				当たってないわ:'No estoy herida.'
				当たらないわ:'No me has dado.'
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN02 -r [字幕西:出てらっ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN03 -r [字幕西:だめね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN04 -r [字幕西:はずれ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN05 -r [字幕西:あたら]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN06 -r [字幕西:お前に]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN11 -r [字幕西:早く私を殺して]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN12 -r [字幕西:何してるの？]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN13 -r [字幕西:殺してみなさ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN14 -r [字幕西:貴方に私の悲]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN15 -r [字幕西:当たってないわ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN16 -r [字幕西:当たらないわ]
		}


		proc ＳＥ字幕リソース:07 {
		    resource 字幕日 {
				あたら:'当たらない'
				お前に:'お前には無理か……'
				早く私を殺して:'早く、私を殺して！'
				何してるの？:'何してるの！私を殺しなさい！'
				出てらっ:'出てらっしゃい！'
				だめね:'駄目ね'
				はずれ:'外れ'
				殺してみなさ:'殺してみなさい！早く！'
				貴方に私の悲:'あなたに私も悲しみがわかる？'
				当たってないわ:'当たってないわ'
				当たらないわ:'当たらないわ'
		    }
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN02 -r [字幕日:出てらっ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN03 -r [字幕日:だめね]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN04 -r [字幕日:はずれ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN05 -r [字幕日:あたら]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN06 -r [字幕日:お前に]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN11 -r [字幕日:早く私を殺して]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN12 -r [字幕日:何してるの？]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN13 -r [字幕日:殺してみなさ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN14 -r [字幕日:貴方に私の悲]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN15 -r [字幕日:当たってないわ]
		    command ＳＥ字幕登録 -s d:se_code:SD_V_FORTUN16 -r [字幕日:当たらないわ]
		}
	#endif

	#ifdef d:STAGE_W20C
		// 英語
		proc ファットマンＳＥ字幕リソース:01 {
			resource ＳＥ字幕リソース英 {
				まぶしい:'My eyes!'
				みえない:'I can\'t see!'
				邪魔するな:'Get out of my way!'
				何をする:'What do you think you\'re doing!'
				どかーん:'boom!'
				どこだ:'Where are you !?'
				出てこい:'Come out!'
				無駄だ:'I almost felt that one!'
				きかんな:'Another \'flesh wound\'!'
				落ちろ:'Going -- down?'
				ばかめ:'Idiot!'

			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN01 -r [ＳＥ字幕リソース英:まぶしい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN02 -r [ＳＥ字幕リソース英:みえない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB01 -r [ＳＥ字幕リソース英:邪魔するな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB02 -r [ＳＥ字幕リソース英:何をする]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDOKN0 -r [ＳＥ字幕リソース英:どかーん]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE01 -r [ＳＥ字幕リソース英:どこだ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE02 -r [ＳＥ字幕リソース英:出てこい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD01 -r [ＳＥ字幕リソース英:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD02 -r [ＳＥ字幕リソース英:きかんな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT01 -r [ＳＥ字幕リソース英:落ちろ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT02 -r [ＳＥ字幕リソース英:ばかめ]
		}

		// フランス語
		proc ファットマンＳＥ字幕リソース:02 {
			resource ＳＥ字幕リソース仏 {
				まぶしい:'Mes yeux !'
				みえない:'Je ne vois plus rien !'
				邪魔するな:'Ote-toi de mon chemin !'
				何をする:'Tu te prends pour qui ?'
				どかーん:'BOUM !'
				どこだ:'O&`u es-tu !?'
				出てこい:'Montre-toi !'
				無駄だ:'J\'ai failli sentir quelque chose, dis donc !'
				きかんな:'Encore une blessure superficielle !'
				落ちろ:'Tu me quittes d&\'ej&`a !'
				ばかめ:'Idiot !'

			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN01 -r [ＳＥ字幕リソース仏:まぶしい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN02 -r [ＳＥ字幕リソース仏:みえない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB01 -r [ＳＥ字幕リソース仏:邪魔するな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB02 -r [ＳＥ字幕リソース仏:何をする]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDOKN0 -r [ＳＥ字幕リソース仏:どかーん]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE01 -r [ＳＥ字幕リソース仏:どこだ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE02 -r [ＳＥ字幕リソース仏:出てこい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD01 -r [ＳＥ字幕リソース仏:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD02 -r [ＳＥ字幕リソース仏:きかんな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT01 -r [ＳＥ字幕リソース仏:落ちろ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT02 -r [ＳＥ字幕リソース仏:ばかめ]
		}

		// ドイツ語
		proc ファットマンＳＥ字幕リソース:03 {
			resource ＳＥ字幕リソース独 {
				まぶしい:'Meine Augen!'
				みえない:'Ich kann nicht mehr sehen!'
				邪魔するな:'Aus dem Weg mit Dir!'
				何をする:'Was glaubst Du eigentlich, was Du da tust!?'
				どかーん:'BUMM!'
				どこだ:'Wo steckst Du denn!?'
				出てこい:'Komm raus!'
				無駄だ:'Fast h&~atte ich was gesp&~urt!'
				きかんな:'Noch eine \'Fleischwunde\'!'
				落ちろ:'Na, willst Du runter?'
				ばかめ:'Idiot!'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN01 -r [ＳＥ字幕リソース独:まぶしい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN02 -r [ＳＥ字幕リソース独:みえない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB01 -r [ＳＥ字幕リソース独:邪魔するな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB02 -r [ＳＥ字幕リソース独:何をする]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDOKN0 -r [ＳＥ字幕リソース独:どかーん]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE01 -r [ＳＥ字幕リソース独:どこだ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE02 -r [ＳＥ字幕リソース独:出てこい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD01 -r [ＳＥ字幕リソース独:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD02 -r [ＳＥ字幕リソース独:きかんな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT01 -r [ＳＥ字幕リソース独:落ちろ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT02 -r [ＳＥ字幕リソース独:ばかめ]
		}

		// イタリア語
		proc ファットマンＳＥ字幕リソース:04 {
			resource ＳＥ字幕リソース伊 {
				まぶしい:'I miei occhi!'
				みえない:'Non vedo niente!'
				邪魔するな:'Fuori dai piedi!'
				何をする:'Cosa pensi di fare!'
				どかーん:'BUUM!'
				どこだ:'Dove sei finito!?'
				出てこい:'Vieni fuori, ho detto!'
				無駄だ:'Quasi quasi mi rifacevi i lineamenti!'
				きかんな:'Un altro \'graffio superficiale\'!'
				落ちろ:'Tu scendi, signore?'
				ばかめ:'Idiota!'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN01 -r [ＳＥ字幕リソース伊:まぶしい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN02 -r [ＳＥ字幕リソース伊:みえない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB01 -r [ＳＥ字幕リソース伊:邪魔するな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB02 -r [ＳＥ字幕リソース伊:何をする]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDOKN0 -r [ＳＥ字幕リソース伊:どかーん]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE01 -r [ＳＥ字幕リソース伊:どこだ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE02 -r [ＳＥ字幕リソース伊:出てこい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD01 -r [ＳＥ字幕リソース伊:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD02 -r [ＳＥ字幕リソース伊:きかんな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT01 -r [ＳＥ字幕リソース伊:落ちろ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT02 -r [ＳＥ字幕リソース伊:ばかめ]
		}


		// スペイン語
		proc ファットマンＳＥ字幕リソース:05 {
			resource ＳＥ字幕リソース西 {
				まぶしい:'&~!Mis ojos!'
				みえない:'&~!No veo!'
				邪魔するな:'&~!Al&\'ejate de mi camino!'
				何をする:'&~!Qu&\'e crees que est&\'as haciendo!'
				どかーん:'&~!BOOOOMMM!'
				どこだ:'&~?&~!D&\'onde est&\'as!?'
				出てこい:'&~!Sal de ah&\'i!'
				無駄だ:'&~!&\'Esa casi la he sentido!'
				きかんな:'&~!Otra "herida superficial"!'
				落ちろ:'&~?Te ... caes?'
				ばかめ:'&~!Idiota!'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN01 -r [ＳＥ字幕リソース西:まぶしい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN02 -r [ＳＥ字幕リソース西:みえない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB01 -r [ＳＥ字幕リソース西:邪魔するな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB02 -r [ＳＥ字幕リソース西:何をする]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDOKN0 -r [ＳＥ字幕リソース西:どかーん]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE01 -r [ＳＥ字幕リソース西:どこだ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE02 -r [ＳＥ字幕リソース西:出てこい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD01 -r [ＳＥ字幕リソース西:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD02 -r [ＳＥ字幕リソース西:きかんな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT01 -r [ＳＥ字幕リソース西:落ちろ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT02 -r [ＳＥ字幕リソース西:ばかめ]
		}


		// 日本語
		proc ファットマンＳＥ字幕リソース:07 {
			resource ＳＥ字幕リソース日 {
				まぶしい:'まぶしい！'
				みえない:'見えない！'
				邪魔するな:'邪魔するな！'
				何をする:'何をする！'
				どかーん:'どかーん！'
				どこだ:'どこだ！？'
				出てこい:'出てこい！'
				無駄だ:'無駄だ！'
				きかんな:'きかんな!'
				落ちろ:'落ちろ'
				ばかめ:'ばかめ！'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN01 -r [ＳＥ字幕リソース日:まぶしい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATSTN02 -r [ＳＥ字幕リソース日:みえない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB01 -r [ＳＥ字幕リソース日:邪魔するな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDMB02 -r [ＳＥ字幕リソース日:何をする]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATDOKN0 -r [ＳＥ字幕リソース日:どかーん]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE01 -r [ＳＥ字幕リソース日:どこだ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATWHE02 -r [ＳＥ字幕リソース日:出てこい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD01 -r [ＳＥ字幕リソース日:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATNOD02 -r [ＳＥ字幕リソース日:きかんな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT01 -r [ＳＥ字幕リソース日:落ちろ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_FATCUT02 -r [ＳＥ字幕リソース日:ばかめ]
		}
	#endif

	#ifdef d:STAGE_W25A
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

		//日
		proc ハリアＳＥ字幕リソース:07 {
			resource ＳＥ字幕リソース日 {
				食らえ！:'食らえっ'
				何処だ？:'何処だ！？'
				そこか！:'そこか！？'
				蜂の巣:'蜂の巣にしてやる！'
				くたばれ:'くたばれぇぇ！！'
				よけられ:'よけられるかぁっ！？'
				あの世に:'あの世に行け！！'
				焼け死ね:'焼け死ねぇ！！'
				ぬぅ:'ぬぅっ'
				糞っ:'糞っ'
				どうした:'どうしたっ！'
				効かんな:'効かんなぁっ！'
				なにぃ！:'なにぃぃっ！！'
				馬鹿な！:'馬鹿な！！'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT101 -r [ＳＥ字幕リソース日:食らえ！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT102 -r [ＳＥ字幕リソース日:何処だ？]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT103 -r [ＳＥ字幕リソース日:そこか！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT301 -r [ＳＥ字幕リソース日:蜂の巣]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT401 -r [ＳＥ字幕リソース日:くたばれ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT501 -r [ＳＥ字幕リソース日:よけられ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT601 -r [ＳＥ字幕リソース日:あの世に]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLAT701 -r [ＳＥ字幕リソース日:焼け死ね]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM101 -r [ＳＥ字幕リソース日:ぬぅ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM102 -r [ＳＥ字幕リソース日:糞っ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM201 -r [ＳＥ字幕リソース日:どうした]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM202 -r [ＳＥ字幕リソース日:効かんな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM401 -r [ＳＥ字幕リソース日:なにぃ！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLDM402 -r [ＳＥ字幕リソース日:馬鹿な！]
		}
	#endif

	#ifdef d:STAGE_W31C
	// ヴァンプ戦の字幕リソース
		proc ヴァンプＳＥ字幕リソース:01 {
			resource ＳＥ字幕リソース英 {
				無駄だ！:'You\'re wasting your time!'
				読めるぞ:'I can read you!'
				遅い:'Too slow!'
				無駄だ:'Not a chance!'
				目が！:'My eyes!'
				見えん:'Can\'t… see…!'
				動けない:'I can\'t move!?'
				体が！？:'My body!?'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB02 -r [ＳＥ字幕リソース英:無駄だ！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB03 -r [ＳＥ字幕リソース英:読めるぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR01 -r [ＳＥ字幕リソース英:遅い]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR02 -r [ＳＥ字幕リソース英:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN01 -r [ＳＥ字幕リソース英:目が！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN02 -r [ＳＥ字幕リソース英:見えん]

			#ifndef d:BOSS_MODE
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース英:動けない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース英:体が！？]
			#else
			if ( $s:ボスラッシュプレイヤー != スネーク ){
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース英:動けない]
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース英:体が！？]
			}
			#endif
		}

		proc ヴァンプＳＥ字幕リソース:02 {
			resource ＳＥ字幕リソース仏 {
				無駄だ！:'Arr&^ete de te ridiculiser !'
				読めるぞ:'Tout ce que tu fais...'
				遅い:'Tu es trop lent !'
				無駄だ:'Tu perds ton temps !'
				目が！:'Mes yeux !'
				見えん:'Je ne vois plus rien !'
				動けない:'Je ne peux plus bouger ??'
				体が！？:'Mon corps !!'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB02 -r [ＳＥ字幕リソース仏:無駄だ！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB03 -r [ＳＥ字幕リソース仏:読めるぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR01 -r [ＳＥ字幕リソース仏:遅い]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR02 -r [ＳＥ字幕リソース仏:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN01 -r [ＳＥ字幕リソース仏:目が！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN02 -r [ＳＥ字幕リソース仏:見えん]
			#ifndef d:BOSS_MODE
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース仏:動けない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース仏:体が！？]
			#else
			if ( $s:ボスラッシュプレイヤー != スネーク ){
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース仏:動けない]
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース仏:体が！？]
			}
			#endif
		}

		proc ヴァンプＳＥ字幕リソース:03 {
			resource ＳＥ字幕リソース独 {
				無駄だ！:'Reine Zeitverschwendung!'
				読めるぞ:'Ich lese Dich --!'
				遅い:'Zu langsam!'
				無駄だ:'Keine Chance!'
				目が！:'Meine Augen!'
				見えん:'Ich kann nichts, sehen!'
				動けない:'Ich kann mich nicht bewegen!?'
				体が！？:'Mein K&~orper!?'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB02 -r [ＳＥ字幕リソース独:無駄だ！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB03 -r [ＳＥ字幕リソース独:読めるぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR01 -r [ＳＥ字幕リソース独:遅い]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR02 -r [ＳＥ字幕リソース独:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN01 -r [ＳＥ字幕リソース独:目が！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN02 -r [ＳＥ字幕リソース独:見えん]
			#ifndef d:BOSS_MODE
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース独:動けない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース独:体が！？]
			#else
			if ( $s:ボスラッシュプレイヤー != スネーク ){
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース独:動けない]
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース独:体が！？]
			}
			#endif
		}

		proc ヴァンプＳＥ字幕リソース:04 {
			resource ＳＥ字幕リソース伊 {
				無駄だ！:'Perdi il tuo tempo!'
				読めるぞ:'Ti leggo nel pensiero!'
				遅い:'Troppo lento!'
				無駄だ:'Ma neanche per sogno!'
				目が！:'I miei occhi!'
				見えん:'Non ... ci vedo...!'
				動けない:'Non posso muovermi!?'
				体が！？:'Il mio corpo!?'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB02 -r [ＳＥ字幕リソース伊:無駄だ！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB03 -r [ＳＥ字幕リソース伊:読めるぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR01 -r [ＳＥ字幕リソース伊:遅い]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR02 -r [ＳＥ字幕リソース伊:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN01 -r [ＳＥ字幕リソース伊:目が！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN02 -r [ＳＥ字幕リソース伊:見えん]
			#ifndef d:BOSS_MODE
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース伊:動けない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース伊:体が！？]
			#else
			if ( $s:ボスラッシュプレイヤー != スネーク ){
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース伊:動けない]
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース伊:体が！？]
			}
			#endif
		}

		proc ヴァンプＳＥ字幕リソース:05 {
			resource ＳＥ字幕リソース西 {
				無駄だ！:'&~!Est&\'as perdiendo el tiempo!'
				読めるぞ:'&~!Puedo leerte como...!'
				遅い:'&~!Demasiado lento!'
				無駄だ:'&~!No tienes ninguna oportunidad!'
				目が！:'&~!Mis ojos!'
				見えん:'&~!No puedo ver...!'
				動けない:'&~?&~!No me puedo mover!?'
				体が！？:'&~?&~!Mi cuerpo!?'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB02 -r [ＳＥ字幕リソース西:無駄だ！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB03 -r [ＳＥ字幕リソース西:読めるぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR01 -r [ＳＥ字幕リソース西:遅い]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR02 -r [ＳＥ字幕リソース西:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN01 -r [ＳＥ字幕リソース西:目が！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN02 -r [ＳＥ字幕リソース西:見えん]
			#ifndef d:BOSS_MODE
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース西:動けない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース西:体が！？]
			#else
			if ( $s:ボスラッシュプレイヤー != スネーク ){
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース西:動けない]
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース西:体が！？]
			}
			#endif
		}

		proc ヴァンプＳＥ字幕リソース:07 {
			resource ＳＥ字幕リソース日 {
				無駄だ！:'無駄だ！'
				読めるぞ:'読めるぞ'
				遅い:'遅い'
				無駄だ:'無駄だ！'
				目が！:'目が！'
				見えん:'くっ！見えん！'
				動けない:'動けない！'
				体が！？:'体が！？'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB02 -r [ＳＥ字幕リソース日:無駄だ！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESB03 -r [ＳＥ字幕リソース日:読めるぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR01 -r [ＳＥ字幕リソース日:遅い]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPESR02 -r [ＳＥ字幕リソース日:無駄だ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN01 -r [ＳＥ字幕リソース日:目が！]
			command ＳＥ字幕登録 -s d:se_code:SD_V_VMPSTN02 -r [ＳＥ字幕リソース日:見えん]

			#ifndef d:BOSS_MODE
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース日:動けない]
			command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース日:体が！？]
			#else
			if ( $s:ボスラッシュプレイヤー != スネーク ){
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB01 -r [ＳＥ字幕リソース日:動けない]
				command ＳＥ字幕登録 -s d:se_code:SD_V_RAISIB02 -r [ＳＥ字幕リソース日:体が！？]
			}
			#endif
		}
	#endif

	#ifdef d:STAGE_W61A
	// ソリダス戦の字幕リソース
		resource	ソリダス字幕リソース {
			//英
			さすがだな:	'Good work, Jack!'
			ここからだ: 'But this is where it gets interesting!'
			//独
			さすがだなＧ: 'Gute Arbeit, Jack!'
			ここからだＧ: 'Aber jetzt wird es erst interessant!'
			//仏
			さすがだなＦ: 'C\'est bien jou&\'e, Jack !'
			ここからだＦ: 'Passons &`a la vitesse sup&\'erieure !'
			//伊
			さすがだなＩ: 'E bravo il mio Jack!'
			ここからだＩ: 'Ora inizia a farsi interessante, la cosa!'
			//西
			さすがだなＳ: '&~!Buen trabajo Jack!'
			ここからだＳ: '&~!Pero ahora las cosas se van a poner interesantes!'
			//日
			さすがだなＪ:	'さすがだな ジャック！'
			ここからだＪ: 'だが ここからだ！！！'
		}

		//英
		proc ソリダスＳＥ字幕リソース:01 {
			resource ＳＥ字幕リソース英 {
				どうした:'What\'s wrong with you?'
				そんなものか:'Is that the best you can do?'
				そこまでか:'Ready to give up, Jack?'
				何してる:'What\'re you doing!'
				ふざけるな:'Quit playing around!'
				行くぞ:'Let\'s go!'
				くらえ:'Take this!'
				しねい:'Die!'
				こっちだ:'Over here!'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO02 -r [ＳＥ字幕リソース英:どうした]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO03 -r [ＳＥ字幕リソース英:そんなものか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO04 -r [ＳＥ字幕リソース英:そこまでか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU01 -r [ＳＥ字幕リソース英:何してる]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU02 -r [ＳＥ字幕リソース英:ふざけるな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA11 -r [ＳＥ字幕リソース英:行くぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA12 -r [ＳＥ字幕リソース英:くらえ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA13 -r [ＳＥ字幕リソース英:しねい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLSPE01 -r [ＳＥ字幕リソース英:こっちだ]
		}
		//仏
		proc ソリダスＳＥ字幕リソース:02 {
			resource ＳＥ字幕リソース仏 {

				どうした:'C\'est quoi, ton probl&`eme ?'
				そんなものか:'C\'est tout ce que tu as dans le ventre ?'
				そこまでか:'Tu jettes l\'&\'eponge, Jack ?'
				何してる:'Qu\'est-ce que tu fabriques ?'
				ふざけるな:'Arr&^ete de faire le mariole !'
				行くぞ:'C\'est parti !'
				くらえ:'Prends &~qa !'
				しねい:'Cr&`eve !'
				こっちだ:'Par ici !'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO02 -r [ＳＥ字幕リソース仏:どうした]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO03 -r [ＳＥ字幕リソース仏:そんなものか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO04 -r [ＳＥ字幕リソース仏:そこまでか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU01 -r [ＳＥ字幕リソース仏:何してる]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU02 -r [ＳＥ字幕リソース仏:ふざけるな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA11 -r [ＳＥ字幕リソース仏:行くぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA12 -r [ＳＥ字幕リソース仏:くらえ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA13 -r [ＳＥ字幕リソース仏:しねい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLSPE01 -r [ＳＥ字幕リソース仏:こっちだ]
		}
		//独
		proc ソリダスＳＥ字幕リソース:03 {
			resource ＳＥ字幕リソース独 {
				どうした:'Was ist los mit Dir?'
				そんなものか:'Ist das alles, was Du kannst?'
				そこまでか:'Willst Du aufgeben, Jack?'
				何してる:'Was machst Du denn da!'
				ふざけるな:'H&~or mit dem Unsinn auf!'
				行くぞ:'Los! Fangen wir an!'
				くらえ:'Hier!'
				しねい:'Stirb!'
				こっちだ:'Hierher!'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO02 -r [ＳＥ字幕リソース独:どうした]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO03 -r [ＳＥ字幕リソース独:そんなものか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO04 -r [ＳＥ字幕リソース独:そこまでか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU01 -r [ＳＥ字幕リソース独:何してる]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU02 -r [ＳＥ字幕リソース独:ふざけるな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA11 -r [ＳＥ字幕リソース独:行くぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA12 -r [ＳＥ字幕リソース独:くらえ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA13 -r [ＳＥ字幕リソース独:しねい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLSPE01 -r [ＳＥ字幕リソース独:こっちだ]
		}
		//伊
		proc ソリダスＳＥ字幕リソース:04 {
			resource ＳＥ字幕リソース伊 {
				どうした:'Ma che ti prende?'
				そんなものか:'&`E questo il meglio che sai fare?'
				そこまでか:'Gi&`a pronto a mollare, Jack?'
				何してる:'Cosa stai facendo!'
				ふざけるな:'Smettila di fare il bambino!'
				行くぞ:'Forza!'
				くらえ:'Prendi questo!'
				しねい:'Muori!'
				こっちだ:'Qui!'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO02 -r [ＳＥ字幕リソース伊:どうした]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO03 -r [ＳＥ字幕リソース伊:そんなものか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO04 -r [ＳＥ字幕リソース伊:そこまでか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU01 -r [ＳＥ字幕リソース伊:何してる]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU02 -r [ＳＥ字幕リソース伊:ふざけるな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA11 -r [ＳＥ字幕リソース伊:行くぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA12 -r [ＳＥ字幕リソース伊:くらえ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA13 -r [ＳＥ字幕リソース伊:しねい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLSPE01 -r [ＳＥ字幕リソース伊:こっちだ]
		}
		//西
		proc ソリダスＳＥ字幕リソース:05 {
			resource ＳＥ字幕リソース西 {
				どうした:'&~?Qu&\'e te pasa?'
				そんなものか:'&~?Eso es todo lo que sabes hacer?'
				そこまでか:'&~?Te rindes ya, Jack?'
				何してる:'&~!Qu&\'e est&\'as haciendo!'
				ふざけるな:'&~!Deja de jugar!'
				行くぞ:'&~!Venga!'
				くらえ:'&~!Toma &\'esta!'
				しねい:'&~!Muere!'
				こっちだ:'&~!Por aqu&\'i!'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO02 -r [ＳＥ字幕リソース西:どうした]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO03 -r [ＳＥ字幕リソース西:そんなものか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO04 -r [ＳＥ字幕リソース西:そこまでか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU01 -r [ＳＥ字幕リソース西:何してる]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU02 -r [ＳＥ字幕リソース西:ふざけるな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA11 -r [ＳＥ字幕リソース西:行くぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA12 -r [ＳＥ字幕リソース西:くらえ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA13 -r [ＳＥ字幕リソース西:しねい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLSPE01 -r [ＳＥ字幕リソース西:こっちだ]
		}
		//日
		proc ソリダスＳＥ字幕リソース:07 {
			resource ＳＥ字幕リソース日 {
				どうした:'どうした！'
				そんなものか:'そんなものか！'
				そこまでか:'そこまでか、ジャック！'
				何してる:'何をしてる'
				ふざけるな:'ふざけるなぁ！'
				行くぞ:'いくぞっ！！'
				くらえ:'くらえぇっ！！'
				しねい:'死ねぃっ！'
				こっちだ:'こっちだ！！'
			}
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO02 -r [ＳＥ字幕リソース日:どうした]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO03 -r [ＳＥ字幕リソース日:そんなものか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLCHO04 -r [ＳＥ字幕リソース日:そこまでか]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU01 -r [ＳＥ字幕リソース日:何してる]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLELU02 -r [ＳＥ字幕リソース日:ふざけるな]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA11 -r [ＳＥ字幕リソース日:行くぞ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA12 -r [ＳＥ字幕リソース日:くらえ]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLKIA13 -r [ＳＥ字幕リソース日:しねい]
			command ＳＥ字幕登録 -s d:se_code:SD_V_SOLSPE01 -r [ＳＥ字幕リソース日:こっちだ]
		}

	#endif

	#ifdef d:STAGE_W32A
	//	ＮＰＣスネークの字幕リソース
		//	英
		proc ＮＰＣスネーク音声ＳＥリソース設定:01 {
			resource ＮＰＣスネーク音声ＳＥ英語 {
				どこを狙っている:'Watch where you\'re aiming!'
			};
			command ＳＥ字幕登録 -s d:se_code:SD_V_SNADKN01 -r [ＮＰＣスネーク音声ＳＥ英語:どこを狙っている]
		}
		//	仏
		proc ＮＰＣスネーク音声ＳＥリソース設定:02 {
			resource ＮＰＣスネーク音声ＳＥフランス語 {
				どこを狙っている:'Regarde o&\`u tu vises!'
			};
			command ＳＥ字幕登録 -s d:se_code:SD_V_SNADKN01 -r [ＮＰＣスネーク音声ＳＥフランス語:どこを狙っている]
		}
		//	独
		proc ＮＰＣスネーク音声ＳＥリソース設定:03 {
			resource ＮＰＣスネーク音声ＳＥドイツ語 {
				どこを狙っている:'Pa&~s auf, wo Du hinschie&~st!'
			};
			command ＳＥ字幕登録 -s d:se_code:SD_V_SNADKN01 -r [ＮＰＣスネーク音声ＳＥドイツ語:どこを狙っている]
		}
		//	伊
		proc ＮＰＣスネーク音声ＳＥリソース設定:04 {
			resource ＮＰＣスネーク音声ＳＥイタリア語 {
				どこを狙っている:'Guarda a dove miri!'
			};
			command ＳＥ字幕登録 -s d:se_code:SD_V_SNADKN01 -r [ＮＰＣスネーク音声ＳＥイタリア語:どこを狙っている]
		}
		//	西
		proc ＮＰＣスネーク音声ＳＥリソース設定:05 {
			resource ＮＰＣスネーク音声ＳＥスペイン語 {
				どこを狙っている:'&~!Mira hacia donde apuntas!'
			};
			command ＳＥ字幕登録 -s d:se_code:SD_V_SNADKN01 -r [ＮＰＣスネーク音声ＳＥスペイン語:どこを狙っている]
		}

		// 日
		proc ＮＰＣスネーク音声ＳＥリソース設定:07 {
			resource ＮＰＣスネーク音声ＳＥ日本語 {
				どこを狙っている:'どこを狙ってるんだ'
			};
			command ＳＥ字幕登録 -s d:se_code:SD_V_SNADKN01 -r [ＮＰＣスネーク音声ＳＥ日本語:どこを狙っている]
		}

	#endif


#endif


#else

print 'cap_resource.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'


#endif
