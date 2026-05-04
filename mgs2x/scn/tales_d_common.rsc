/*
	tales_e_common.rsc
	    スネークテイルズ紙芝居用各言語共通親リソースヘッダ

	2002/06/04 S.Mukaide
	$Id: tales_d_common.rsc,v 1.3 2002/08/10 07:59:17 usr03005 Exp $


*/

// フォントチェック用define
/*
-fontcheckは省略可能です。５つのパラメータは順に 
width, height, chara_skip, line_skip, flag　をあらわします。
flagが1の場合は、禁則を行わない。(日本語以外)

-fontcheck width height chara_skip line_skip flag
*/
#define	TITLE_FONT_AREA		-fontcheck 520 120 0 12 0 


//---------------------------------------
//	テイルズＤ(デッドマンが暴れる話)
//---------------------------------------

resource テイルズ＿Ｄ d:TITLE_FONT_AREA {


D1_スタート: << EOF
#C(101)#C1
EOF


D2_シールズ全滅: << EOF
#C(101)#C1
EOF


D3_Ｄ脚到着: << EOF
#C(101)#C1
EOF


D4_Ｅ脚梯子部屋到着: << EOF
#C(101)#C1
EOF


D5_シェル２中央棟到着: << EOF
#C(101)#C1
EOF


//配電盤破壊して大統領部屋入った。
//分岐：
//時間掛かった：D6_0→D6_1A→D6_2→D6_3A→D6_4
//時間かからなかった：D6_0→D6_1B→D6_2→D6_3B→D6_4
D6_海軍大佐語り: << EOF
#C(101)#C1
#C2
#C3
#C4
#C5
EOF


D7_エレベータ搭乗: << EOF
#C(101)#C1
EOF


//分岐
//海軍大佐時間掛かった＆ヴァンプ殺した:
//D8_1XA→D8_2→D8_3AX→D8_4AA→D8_5XA→D8_6
//海軍大佐時間掛かった＆ヴァンプ気絶:
//D8_1XB→D8_2→D8_3AX→D8_4AB→D8_5AB→D8_6
//海軍大佐時間掛らない＆ヴァンプ殺した:
//D8_1XA→D8_2→D8_3BX→D8_4BA→D8_5XA→D8_6
//海軍大佐時間掛らない＆ヴァンプ気絶:
//D8_1XB→D8_2→D8_3BX→D8_4BB→D8_5BB→D8_6
D8_エンディング: << EOF
#C(101)#C1
#C2
#C3
#C4
#W#C5
｜
#C6
EOF



}
