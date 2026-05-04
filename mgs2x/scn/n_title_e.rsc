/*
	ファイル名		n_title_e.rsc
	作成日			2001/06/06
	作者			S.Kaneyoshi
	説明			タイトル画面説明テキスト英語版
	$Id: n_title_e.rsc,v 1.5 2002/03/26 06:23:44 usr04761 Exp $
*/

#include "dogtag_view.h" // dogtag data <-- koba4

#include "loadgame_e.rsc"


#include "stagename_e.h"
#include "mcwarning_e.rsc"

#include "option_e.rsc"

#include "pre_story1_e.rsc"
#include "pre_story2_e.rsc"
#include "pre_story3_e.rsc"

// by koba4
resource アンケート {
	１番:{
I've cleared the last game multiple times,
so bring on the action!
	 },
	２番:{
I managed to clear the last game,
but action isn't my strong point.
	},
	３番:{
I didn't clear the last game myself,
but I watched everything!
	},
	４番:{
I didn't clear the last game
and action isn't my strong point!
	},
	５番:{
I didn't clear the last game,
but bring on the action!
	},
};
/*
	１番:
		'前作を何度もクリアしたのでアクションは得意！',
	２番:
		'前作をなんとかクリアしたがアクションは苦手！',
	３番:
		'前作は自力ではクリアしてないが最後まで観た！',
	４番:
		'前作はクリアしてないが、アクションは苦手！',
	５番:
		'前作はクリアしてないが、アクションは得意！',
*/
resource きくちゃん声 {
	始まり:
	$t:{
		t:vc126101 ,
	}
	終り:
	$t:{
		t:vc126102 ,
	}
};

command ローカルリソース設定 \
	-string アメリカアンケート [アンケート:*] \
            ロードゲームリソース [ロードゲームリソース:*] \
			ステージ名 [ステージ名:*] \
			難易度名 [難易度名:*] \
			メモリーカード警告メッセージ [メモリーカード警告メッセージ:*] \
			オプションの説明 [オプションの説明:*] \
			シャドーモセス日本1[コラム:*] \
			シャドーモセス日本2[真実の真実:*] \
			シャドーモセス日本3[真実:*] \
			シャドーモセスアメリカ1[コラム:*] \
			シャドーモセスアメリカ2[真実の真実:*] \
			シャドーモセスアメリカ3[真実:*] \
			シャドーモセスフランス1[コラム:*] \
			シャドーモセスフランス2[真実の真実:*] \
			シャドーモセスフランス3[真実:*] \
			シャドーモセスイタリア1[コラム:*] \
			シャドーモセスイタリア2[真実の真実:*] \
			シャドーモセスイタリア3[真実:*] \
			シャドーモセスドイツ1[コラム:*] \
			シャドーモセスドイツ2[真実の真実:*] \
			シャドーモセスドイツ3[真実:*] \
			シャドーモセススペイン1[コラム:*] \
			シャドーモセススペイン2[真実の真実:*] \
			シャドーモセススペイン3[真実:*] \
			レーダーの説明[レーダーの説明:*] \
			クイックチェンジの説明[クイックチェンジの説明:*] \
	-binary きくちゃん声 [きくちゃん声:*] \
			ドッグタグデータ[dogtag_view:*]