/*
	tales_c_common.rsc
	    スネークテイルズ紙芝居用各言語共通親リソースヘッダ

	2002/06/04 S.Mukaide
	$Id: tales_c_common.rsc,v 1.3 2002/08/10 06:25:53 usr03005 Exp $


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
//	テイルズＣ(めるりが暴れる話)
//---------------------------------------

resource テイルズ＿Ｃ d:TITLE_FONT_AREA {

	C1_スタート: << EOF
#C(101)#C1
EOF

C2_船橋到着: << EOF
#C(101)#C1
EOF


C3_長廊下戦前: << EOF
#C(101)#C1
EOF


C4_長廊下戦後: << EOF
#C(101)#C1
EOF

C5_機関室脱出: << EOF
#C(101)#C1
EOF

C6_メリル再会: << EOF
#C(101)#C1
EOF


//メリル戦後親リソース
//分岐：
//メリル殺した場合：C7_1A→C7_2→C7_3A
//メリル気絶させた場合：C7_1B→C7_2→C7_3B
	C7_メリル戦後: << EOF
#C(101)#C1
#C2
#C3
EOF



}
