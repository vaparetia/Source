//=============================================================================
// snake_voice.h
// スネークの音声コード
// 2001/07/30 K.Kano
// $Id: snake_voice.h,v 1.1.1.3 2002/11/19 11:43:23 Yoshizawa1 Exp $
//


#ifndef _snake_voice_h_
#define _snake_voice_h_


enum {
	// ★ライデンと出会ったスネークは貧血のためしばらく休んで居眠りをする。
	VOICE_INDEX_ZZZ=0,
	// スネーク　「ＺＺｚｚｚｚ……（いびき）」

	// ★ライデンは眠ったスネークを殴ることも出来る。スネークは怒る。
	VOICE_INDEX_ANGRY0,
	//スネーク　「何をする！」
	VOICE_INDEX_ANGRY1,
	// スネーク　「ふざけたことをするな！」
	VOICE_INDEX_ANGRY2,
	// スネーク　「何のつもりだ！」
	VOICE_INDEX_ANGRY3,
	// スネーク　「構うな！」
	VOICE_INDEX_ANGRY4,
	// スネーク　「眠らせろ！」

	// ★ライデンが眠ったスネークに銃を向けると目を覚ます。
	VOICE_INDEX_BRING_GUN0,
	// スネーク　「（不敵に）若いの、何のつもりだ？」
	VOICE_INDEX_BRING_GUN1,
	// スネーク　「（不敵に）やめておけ」
	VOICE_INDEX_BRING_GUN2,
	// スネーク　「（不敵に）お前に撃てるのか？」

	// ★ライデンがスネークを本当に撃つとスネークも撃ち返す。
	VOICE_INDEX_SHOOT_GUN0,
	// スネーク　「馬鹿者」
	VOICE_INDEX_SHOOT_GUN1,
	// スネーク　「ふざけるな」
	VOICE_INDEX_SHOOT_GUN2,
	// スネーク　「何を考えている」

	// ★狙撃イベント中
	//  オイルフェンスを渡るエマをスナイパーライフルで援護する時の台詞。
	//  ライデンから撃たれたら撃ち返して一喝する
	VOICE_INDEX_SHOOT_PSG0,
	// スネーク　「何を考えてるんだ」
	VOICE_INDEX_SHOOT_PSG1,
	// スネーク　「どこを狙ってる」
	VOICE_INDEX_SHOOT_PSG2,
	// スネーク　「ふざけるな」
	VOICE_INDEX_SHOOT_PSG3,
	// スネーク　「いい加減にしろ」

	// ★イベント中、マイクを向けると独り言を言っているのが聞こえる。
	VOICE_INDEX_TALK_ALONE0,
	// スネーク　「（エマを見ながら）うむ……悪くない……」
	VOICE_INDEX_TALK_ALONE1,
	// スネーク　「食事に誘うにはオタコンの許可がいるかな……」
	VOICE_INDEX_TALK_ALONE2,
	// スネーク　「大丈夫か……！」
	VOICE_INDEX_TALK_ALONE3,
	// スネーク　「あいつ（ライデン）何してる……！」
	VOICE_INDEX_TALK_ALONE4,
	// スネーク　「危ない……！」
	VOICE_INDEX_TALK_ALONE5,
	// スネーク　「ふぅ……」
	VOICE_INDEX_TALK_ALONE6,
	// スネーク　「いいぞ……」
	VOICE_INDEX_TALK_ALONE7,
	// スネーク　「もう少しだ……」
	VOICE_INDEX_TALK_ALONE8,
	// スネーク　「（撃ちそこなったらしい）ちぃ……」
	VOICE_INDEX_TALK_ALONE9,
	// スネーク　「（うまく当てたらしい）よし……」

	// ★スネークと一緒
	//   アーセナルギア内部でライデン（プレイヤー）と共に戦うスネークの台詞。
	// ライデンが遅れ気味
	VOICE_INDEX_RAIDEN_LATE0,
	// スネーク　　「何をしてる？」
	VOICE_INDEX_RAIDEN_LATE1,
	// スネーク　　「早く来い！」
	VOICE_INDEX_RAIDEN_LATE2,
	// スネーク　　「おいていくぞ！」

	// スネークが突撃していく時
	VOICE_INDEX_SNAKE_GO0,
	// スネーク　　「行くぞ！」
	VOICE_INDEX_SNAKE_GO1,
	// スネーク　　「ついてこい！」
	VOICE_INDEX_SNAKE_GO2,
	// スネーク　　「遅れるな！」

	// ライデンが敵に囲まれている時
	VOICE_INDEX_ENEMY_ARROUND0,
	// スネーク　　「援護する！」
	VOICE_INDEX_ENEMY_ARROUND1,
	// スネーク　　「先に行け！」
	VOICE_INDEX_ENEMY_ARROUND2,
	// スネーク　　「今行くぞ！」

	// スネークが飛び出し撃ちをしている間
	VOICE_INDEX_GO_RAIDEN0,
	// スネーク　　「今だ、行け！」
	VOICE_INDEX_GO_RAIDEN1,
	// スネーク　　「突っ込め！」

	// ライデンのＬＩＦＥが少ない
	VOICE_INDEX_WORRY0,
	// スネーク　　「大丈夫か」
	VOICE_INDEX_WORRY1,
	// スネーク　　「無理するな！」

	// アイテムをなげてよこす時
	VOICE_INDEX_THROW_ITEM0,
	// スネーク　　「これを使え！」
	VOICE_INDEX_THROW_ITEM1,
	// スネーク　　「ライデン、受け取れ！」
	VOICE_INDEX_THROW_ITEM2,
	// スネーク　　「ライデン、アイテムだ！」
};


#endif
