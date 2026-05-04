//=============================================================================
// snake_sejimaku.h
// スネークの音声ＳＥの字幕リソース
// 2001/10/31 K.Kano
// $Id: snake_sejimaku.h,v 1.1 2002/02/01 06:08:32 usr01475 Exp $
//


#if 0

// 日
resource ＮＰＣスネーク音声ＳＥ {
どこを狙っている:
    'どこを狙ってるんだ'
};

#endif

// 英
resource ＮＰＣスネーク音声ＳＥ英語 {
どこを狙っている:
    'Watch where you\'re aiming!'
};

// 仏
resource ＮＰＣスネーク音声ＳＥフランス語 {
どこを狙っている:
    'Regarde o&\`u tu vises!'
};

// 独
resource ＮＰＣスネーク音声ＳＥドイツ語 {
どこを狙っている:
    'Pa&~s auf, wo Du hinschie&~st!'
};

// 伊
resource ＮＰＣスネーク音声ＳＥイタリア語 {
どこを狙っている:
    'Guarda a dove miri!'
};

// 西
resource ＮＰＣスネーク音声ＳＥスペイン語 {
どこを狙っている:
    '&~!Mira hacia donde apuntas!'
};


proc ＮＰＣスネーク音声ＳＥリソース設定:01 {
	command ＳＥ字幕登録 \
		-s d:se_code:SD_V_SNADKN01 \
		-r [ＮＰＣスネーク音声ＳＥ英語:どこを狙っている]
}

proc ＮＰＣスネーク音声ＳＥリソース設定:02 {
	command ＳＥ字幕登録 \
		-s d:se_code:SD_V_SNADKN01 \
		-r [ＮＰＣスネーク音声ＳＥフランス語:どこを狙っている]
}

proc ＮＰＣスネーク音声ＳＥリソース設定:03 {
	command ＳＥ字幕登録 \
		-s d:se_code:SD_V_SNADKN01 \
		-r [ＮＰＣスネーク音声ＳＥドイツ語:どこを狙っている]
}

proc ＮＰＣスネーク音声ＳＥリソース設定:04 {
	command ＳＥ字幕登録 \
		-s d:se_code:SD_V_SNADKN01 \
		-r [ＮＰＣスネーク音声ＳＥイタリア語:どこを狙っている]
}

proc ＮＰＣスネーク音声ＳＥリソース設定:05 {
	command ＳＥ字幕登録 \
		-s d:se_code:SD_V_SNADKN01 \
		-r [ＮＰＣスネーク音声ＳＥスペイン語:どこを狙っている]
}
