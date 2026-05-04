/*
	vr_window_k.h
	    ＶＲウインドウのリソース(韓国語版)

	2002/05/21 H.Yoshiike
	$Id: vr_window_k.h,v 1.20 2002/08/23 13:07:45 usr03682 Exp $


*/


#ifndef d:VR_WINDOW_K
#define	VR_WINDOW_K	1

resource	VRMSG_K	{
//------------------------
// 共通部分のリソース
//------------------------
時間: { TIME LIMIT:}
分: {min}
秒: {sec}
敵兵数: { ENEMIES   :}
人: { 　 }
標的数: { TARGETS   :}
爆弾数: { BOMBS     :}
個: { 　 }

//------------------------
// ステージ固有のリソース
//------------------------
// VR MISSION
// SNEAKING MODE-SNEAKING 1-10
// vs01a
VRM_SNK_SNK_vs01a: << EOF
Get to the goal without being discovered!
Flattening yourself against the wall
will help.
EOF

// vs02a
VRM_SNK_SNK_vs02a: << EOF
Get to the goal without being discovered!
The enemy patrol route is fixed.
EOF

// vs03a
VRM_SNK_SNK_vs03a: << EOF
Get to the goal without being discovered!
Predicting enemy behavior and making
use of INTRUSION is key to completion.
EOF

// vs04a
VRM_SNK_SNK_vs04a: << EOF
Get to the goal without being discovered!
Lure the enemy to get yourself a route.
EOF

// vs05a
VRM_SNK_SNK_vs05a: << EOF
Get to the goal without being discovered!
Use HANGING and get through
the enemy line.
EOF

// vs06a
VRM_SNK_SNK_vs06a: << EOF
Get to the goal!
Beware of the noise-making floor and
wet floor that makes you leave footprints.
The mission will be terminated if you are
discovered or fall into a hole.
EOF

// vs07a
VRM_SNK_SNK_vs07a: << EOF
Use the lockers wisely and get to the goal!
The mission will be terminated
if you are discovered.
Security will tighten if the enemy wakes up.
EOF

// vs08a
VRM_SNK_SNK_vs08a: << EOF
Get to the goal without being spotted by
the enemy or surveillance camera.
The mission will be terminated
if you are discovered.
Right below the camera is a blind spot.
EOF

// vs09a
VRM_SNK_SNK_vs09a: << EOF
Use the items wisely and get to the goal!
The mission will be terminated
if you are discovered.
EOF

// vs10a
VRM_SNK_SNK_vs10a: << EOF
Use what you learned so far and
get to the goal without being discovered!
EOF


// SNEAKING MODE-ELIMINATE ALL 1-10
// vs01a
VRM_SNK_ELM_vs01a: << EOF
Take out the enemy and get to the goal!
Knocking him unconscious or tranquilizing
him in addition to killing him will count.
EOF

// vs02a
VRM_SNK_ELM_vs02a: << EOF
Take out all enemies and get to the goal!
If you want to shoot accurately, switch to
the First Person View camera and aim well.
EOF

// vs03a
VRM_SNK_ELM_vs03a: << EOF
Take out all enemies and get to the goal!
Their head, heart, and groin are their
weakpoints. Hitting them in these spots
will take then out with one shot.
EOF

// vs04a
VRM_SNK_ELM_vs04a: << EOF
Take out all enemies and get to the goal!
Use an item, knock on the wall, or shoot
nearby to draw their attention.
EOF

// vs05a
VRM_SNK_ELM_vs05a: << EOF
Take out all enemies and get to the goal!
Taking them out by jumping back up
from HANGING or jumping down
on them is effective.
EOF

// vs06a
VRM_SNK_ELM_vs06a: << EOF
Take out all enemies and get to the goal!
Beware of the noise-making floor and
wet floor.The mission will be terminated
if you fall in a hole.
EOF

// vs07a
VRM_SNK_ELM_vs07a: << EOF
Take out the enemies in position and
get to the goal. A failure in your attempt
will result in ALERT mode,
causing an attack team to appear.
EOF

// vs08a
VRM_SNK_ELM_vs08a: << EOF
Take out the enemies in position and
get to the goal. Beware of the surveillance
camera. The mission will be terminated
if you are discovered.
EOF

// vs09a
VRM_SNK_ELM_vs09a: << EOF
Take out the enemies in position and get
to the goal. Make good use of the items.
EOF

// vs10a
VRM_SNK_ELM_vs10a: << EOF
Use what you learned so far and take out
the enemies in position and get to the goal.
EOF


// WEAPON MODE-HANDGUN 1-5
// wp01a
VRM_WPN_HGN_wp01a: << EOF
Destroy all targets and head to the goal!
EOF

// wp02a
VRM_WPN_HGN_wp02a: << EOF
Destroy all targets and head to the goal!
EOF

// wp03a
VRM_WPN_HGN_wp03a: << EOF
Destroy all targets and head to the goal!
EOF

// wp04a
VRM_WPN_HGN_wp04a: << EOF
Destroy all targets and head to the goal!
EOF

// wp05a
VRM_WPN_HGN_wp05a: << EOF
Destroy all targets and head to the goal!
EOF


// WEAPON MODE-ASSAULT RIFLE 1-5
// wp11a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp11a: << EOF
全ての標的を破壊し、ゴールせよ！
余分な弾を撃たないように注意しろ！
EOF

// wp12a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp12a: << EOF
全ての標的を破壊し、ゴールせよ！
一列に並んだ時は、コンボを狙うチャンスだ！
EOF

// wp13a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp13a: << EOF
全ての標的を破壊し、ゴールせよ！
移動する標的のタイミングを計って狙え！
EOF

// wp14a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp14a: << EOF
全ての標的を破壊し、ゴールせよ！
走りながら撃つと、タイムを縮めることも可能だ！
EOF

// wp15a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp15a: << EOF
今までの訓練で学んだ事を活用して、
全ての標的を破壊し、ゴールせよ！
EOF


// WEAPON MODE-C4/CLAYMORE 1-5
// wp21a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp21a: << EOF
全ての標的を破壊し、ゴールせよ！
Ｃ４とクレイモアがステージ上に落ちている。
状況にふさわしい武器を考え、拾って使用しろ！
EOF

// wp22a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp22a: << EOF
全ての標的を破壊し、ゴールせよ！
オレンジ色の標的は爆発する。
爆風に巻き込まれないように注意しろ！
EOF

// wp23a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp23a: << EOF
全ての標的を破壊し、ゴールせよ！
Ｃ４は壁に貼り付けることも可能だ。
EOF

// wp24a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp24a: << EOF
全ての標的を破壊し、ゴールせよ！
移動する標的のルートを見極めて、狙いやすい
設置位置を見つけるのがポイントだ。
EOF

// wp25a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp25a: << EOF
今までの訓練で学んだ事を活用して、
全ての標的を破壊し、ゴールせよ！
青い壁は破壊できる。
移動距離を短縮することも可能だ。
EOF


// WEAPON MODE-GRENADE 1-5
// wp31a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp31a: << EOF
全ての標的を破壊し、ゴールせよ！
弾道が曲がるＲＧＢ６は、離れた位置から
標的の少し上を狙うと当てやすいぞ！
EOF

// wp32a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp32a: << EOF
全ての標的を破壊し、ゴールせよ！
グレネードは、壁や標的に当たると跳ね返る。
それを利用して近くの床面に落とし、爆風に
巻き込むといい。
EOF

// wp33a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp33a: << EOF
全ての標的を破壊し、ゴールせよ！
移動する標的のタイミングを計って狙え！
EOF

// wp34a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp34a: << EOF
全ての標的を破壊し、ゴールせよ！
障害物の陰に隠れる標的は、標的の見えやすい
位置に移動して狙うといい。
EOF

// wp35a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp35a: << EOF
今までの訓練で学んだ事を活用して、
全ての標的を破壊し、ゴールせよ！
EOF


// WEAPON MODE-PSG1 1-5
// wp41a
VRM_WPN_PSG_wp41a: << EOF
Destroy all targets and head to the goal!
EOF

// wp42a
VRM_WPN_PSG_wp42a: << EOF
Destroy all targets and head to the goal!
EOF

// wp43a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp43a: << EOF
全ての標的を破壊し、ゴールせよ！
オレンジの標的は爆発するぞ。
うまく活用して周囲の標的を誘爆せよ！
EOF

// wp44a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp44a: << EOF
赤い標的は破壊すると減点される。
赤い標的以外の全ての的を破壊して
ゴールせよ！
EOF

// wp45a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp45a: << EOF
今までの訓練で学んだ事を活用して、
標的を破壊し、ゴールせよ！
EOF


// WEAPON MODE-STINGER 1-5
// wp51a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp51a: << EOF
全ての標的を破壊し、ゴールせよ！
スティンガーの滞空時間には限界がある。
発射位置を考えて狙うのがポイントだ。
EOF

// wp52a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp52a: << EOF
全ての標的を破壊し、ゴールせよ！
障害物の陰に隠れる標的は、
発射位置を移動すると狙いやすいぞ！
EOF

// wp53a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp53a: << EOF
全ての標的を破壊し、ゴールせよ！
移動する標的のタイミングを計って狙え！
EOF

// wp54a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp54a: << EOF
全ての標的を破壊し、ゴールせよ！
移動する標的のルートを見極めて、狙いやすい
発射位置を見つけるのがポイントだ。
EOF

// wp55a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp55a: << EOF
今までの訓練で学んだ事を活用して、
全ての標的を破壊し、ゴールせよ！
EOF


// WEAPON MODE-NIKITA 1-5
// wp61a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp61a: << EOF
全ての標的を破壊し、ゴールせよ！
EOF

// wp62a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp62a: << EOF
全ての標的を破壊し、ゴールせよ！
標的が停止する瞬間がチャンスだ！
EOF

// wp63a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp63a: << EOF
全ての標的を破壊し、ゴールせよ！
ステージの床面や天井面に沿うようにして、
ニキータの高度を変化させて狙え！
EOF

// wp64a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp64a: << EOF
青色の標的を破壊し、ゴールせよ！
途中の傷害物に当たらないように気をつけろ！
EOF

// wp65a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp65a: << EOF
今までの訓練で学んだ事を活用して、
青色の標的を破壊し、ゴールせよ！
EOF


// WEAPON MODE-HF.BLADE/NO WEAPON 1-5
// wp71a
VRM_WPN_HFB_wp71a: << EOF
Destroy all targets and head to the goal!
EOF

// wp72a
VRM_WPN_HFB_wp72a: << EOF
Destroy all targets and head to the goal!
EOF

// wp73a
VRM_WPN_HFB_wp73a: << EOF
Destroy all targets and head to the goal!
EOF

// wp74a
VRM_WPN_HFB_wp74a: << EOF
Destroy all targets and head to the goal!
EOF

// wp75a
VRM_WPN_HFB_wp75a: << EOF
Destroy all targets and head to the goal!
EOF


// FIRST PERSON VIEW MODE- 1-5
// sp21a	/* 修正:2002/08/21 M.Uehara */
VRM_FPV_sp21a: << EOF
全ての標的を破壊し、ゴールせよ！
標的の種類にあわせて武器を変えると、
狙いやすいぞ！
EOF


// sp22a	//	修正	2002/08.21	H.Yoshiike
VRM_FPV_sp22a: << EOF
生きて、ゴールせよ！
落ちているアイテムを有効に活用せよ。
EOF


// sp23a
VRM_FPV_sp23a: << EOF
Take out all enemies and get to the goal!
EOF


// sp24a	//	修正	2002/08.21	H.Yoshiike
VRM_FPV_sp24a: << EOF
全ての標的を破壊し、ゴールせよ！
暗闇で警備している敵兵に注意せよ。
EOF


// sp25a
VRM_FPV_sp25a: << EOF
Defuse all bombs!
EOF


// VARIETY MODE- 1-7
// sp01a-1 アスレチックステージ機雷バージョン
VRM_VRT_sp01a_1: << EOF
Use various moves and get to the goal!
Beware of mines!
EOF

// sp01a-1 アスレチックステージ的バージョン
VRM_VRT_sp01a_2: << EOF
Destroy all targets and get to the goal!
EOF

// sp02a
VRM_VRT_sp02a: << EOF
Use the HANGING-DROP-CATCH move
and get to the goal!
Press the Action button at the right timing
while falling and you can HANG again!
EOF

// sp03a ザコサバイバル		//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp03a: << EOF
ザコサバイバル
襲いくる全ての敵兵を倒せ！
EOF

// sp06a 狙撃ミッション
VRM_VRT_sp06a_1: << EOF
Take out all approaching enemies!
Do not let them come close to the person
lying on the floor!
EOF

VRM_VRT_sp06a_2: << EOF
Take out all approaching enemies!
Don't let the enemies get close
to the curry!
EOF
VRM_VRT_sp06a_3: << EOF
Take out all approaching enemies!
Don't let the enemies get close
to the noodles!
EOF

VRM_VRT_sp06a_4: << EOF
Take out all approaching enemies!
Don't let the enemies get close
to the cardboard box!
EOF

// sp07a ゴルルゴンステージ					//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp07a: << EOF
謎の巨大生物に見つからずに、ゴールせよ！
相手の動きをよく観察し、
うまくスキを見つけて、
切り抜けろ！
EOF

// sp08a ダークステージ						//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp08a: << EOF
暗闇に潜む敵兵を全て倒して、ゴールせよ！
アイテムをうまく活用せよ。
EOF


// STREAKING MODE- 1
// sp09a
VRM_STR_sp09a: << EOF
誰にも見つからずにゴールを目指せ！
EOF

// STREAKING MODE- 1
// st01a
VRM_STR_st01a: << EOF
Get to the goal through 5 consecutive
stages without being discovered!
EOF

VRM_STR_st02a: << EOF
Get to the goal without being discovered!
EOF

VRM_STR_st03a: << EOF
Get to the goal without being discovered!
Keep in mind INTRUSION is effective.
EOF

VRM_STR_st04a: << EOF
Get to the goal without being discovered!
Security is tight.
However, there is a way through.
EOF

VRM_STR_st05a: << EOF
This is the final stage.
Do not lose your concentration.
Get to the goal!
EOF


// ALTNATIVE MISSION
// BOMB DISPOSAL 1-5
// 爆弾解体は各ステージ共通で	//	修正	2002/08.21	H.Yoshiike 
// ライデン＆スネーク
ALT_BMB_1: << EOF
センサーＡを使用して、全ての爆弾を
解体せよ！
爆弾の信号音も聞き逃すな！
EOF

// プリスキン＆タキシードスネーク
ALT_BMB_2: << EOF
センサーＢを使用して、全ての爆弾を
解体せよ！
爆弾は侵入者が半径２ｍ以内に
近づくと自動的に爆発する。
EOF

// 忍者＆前作スネーク
ALT_BMB_3: << EOF
センサーＡとＢの両方を使用して、
全ての爆弾を解体せよ！
あらゆる場所に気を配れ！
EOF

// a31a
ALT_BMB_a31a: << EOF
Defuse all bombs!
EOF

// a02a
ALT_BMB_a02a: << EOF
Defuse all bombs!
EOF

// a01a a01f
ALT_BMB_a01f: << EOF
Defuse all bombs!
EOF

// a41a
ALT_BMB_a41a: << EOF
Defuse all bombs!
EOF



// ELIMINATE 1-10
// a23b
ALT_ELM_a23b: << EOF
Take out all enemies and get to the goal!
EOF

// a01a
ALT_ELM_a01a: << EOF
Take out all enemies and get to the goal!
EOF

// a19a
ALT_ELM_a19a: << EOF
Take out all enemies and get to the goal!
EOF

// a20a
ALT_ELM_a20a: << EOF
Take out all enemies and get to the goal!
EOF

// a24d
ALT_ELM_a24d: << EOF
Take out all enemies and get to the goal!
EOF

// a24a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a24a: << EOF
全ての敵兵を倒してゴールせよ！
敵兵の持つショットガンは強力だ。十分に
注意しろ！
EOF

// a31a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a31a: << EOF
全ての敵兵を倒してゴールせよ！
敵兵の持つ盾は頑丈だ。弾を使い過ぎないように
注意しろ！
EOF

// a22a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a22a: << EOF
全ての敵兵を倒してゴールせよ！
限られた弾薬に注意しろ！
柵にぶら下がって敵兵の頭上に落ちると、
一撃で気絶させることも可能だ。
EOF

// a42a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a42a: << EOF
全ての敵兵を倒してゴールせよ！
２階から監視している敵兵に特に注意しろ！
壁を叩いたりして、障害物の陰に誘き寄せる
のも有効な手段だ。
EOF

// a02a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a02a: << EOF
目に見えるものが全てとは限らない。
五感を駆使して、全ての敵兵を倒してゴールせよ！
EOF


// HOLD UP 1-10
// a15b
ALT_HLD_a15b: << EOF
Hold up all enemies and get to the goal!
The mission will be terminated
if you are discovered.
EOF

// a12a
ALT_HLD_a12a: << EOF
Hold up all enemies and get to the goal!
The mission will be terminated
if you are discovered.
EOF

// a24d
ALT_HLD_a24d: << EOF
Hold up all enemies and get to the goal!
The mission will be terminated
if you are discovered.
EOF

// a13b
ALT_HLD_a13b: << EOF
Hold up all enemies and get to the goal!
The mission will be terminated
if you are discovered.
EOF

// a14a
ALT_HLD_a14a: << EOF
Hold up all enemies and get to the goal!
The mission will be terminated
if you are discovered.
EOF

// a22a
ALT_HLD_a22a: << EOF
Hold up all enemies and get to the goal!
The mission will be terminated
if you are discovered.
EOF

// a01b
ALT_HLD_a01b: << EOF
Hold up all enemies and get to the goal!
The mission will be terminated
if you are discovered.
EOF

// a42a
ALT_HLD_a42a: << EOF
Hold up all enemies and get to the goal!
The mission will be terminated
if you are discovered.
EOF

// a20b
ALT_HLD_a20b: << EOF
Hold up all enemies and get to the goal!
The mission will be terminated
if you are discovered.
EOF

// a31a
ALT_HLD_a31a: << EOF
Hold up all enemies and get to the goal!
The mission will be terminated
if you are discovered.
EOF


// ESCAPE RAIDEN 1-5
// a01a
ALT_ESC_a01a_RAI: << EOF
潜入モードになるまで逃げきり、ゴールを目指せ！
EOF

// a20a
ALT_ESC_a20a_RAI: << EOF
潜入モードになるまで逃げきり、ゴールを目指せ！
EOF

// a22a_1
ALT_ESC_a22a_RAI_1: << EOF
潜入モードになるまで逃げきり、ゴールを目指せ！
EOF

// a24a
ALT_ESC_a24a_RAI: << EOF
潜入モードになるまで逃げきり、ゴールを目指せ！
EOF

// a22a_2
ALT_ESC_a22a_RAI_2: << EOF
潜入モードになるまで逃げきり、ゴールを目指せ！
EOF


// ESCAPE SNAKE 1-5
// a01d
ALT_ESC_a01d_SNA: << EOF
潜入モードになるまで逃げきり、ゴールを目指せ！
EOF

// a14a
ALT_ESC_a14a_SNA: << EOF
潜入モードになるまで逃げきり、ゴールを目指せ！
EOF

// a16a
ALT_ESC_a16a_SNA: << EOF
潜入モードになるまで逃げきり、ゴールを目指せ！
EOF

// a22a_1
ALT_ESC_a22a_SNA_1: << EOF
潜入モードになるまで逃げきり、ゴールを目指せ！
EOF

// a22a_2
ALT_ESC_a22a_SNA_2: << EOF
潜入モードになるまで逃げきり、ゴールを目指せ！
EOF


// PHOTOGRAPH RAIDEN 1-5
// a01a
ALT_PHT_a01a_RAI: << EOF
Take a photograph
as shown!
EOF

// a01b
ALT_PHT_a01b_RAI: << EOF
Take a photograph
as shown!
EOF

// a01f
ALT_PHT_a01f_RAI: << EOF
Take a photograph
as shown!
EOF

// a00c
ALT_PHT_a00c_RAI: << EOF
Take a photograph of the stars!
EOF

// a00a
ALT_PHT_a00a_RAI: << EOF
There is a rumor that a soldier ghost
wanders the deck.
Find the ghost and take its photograph!
EOF

// a03a
ALT_PHT_a03a_RAI: << EOF
Take a photograph of
the ghost! Ghosts hate
anywhere that is bright.
EOF


// PHOTOGRAPH SNAKE 1-5
// a01a
ALT_PHT_a01a_SNA_A: << EOF
Take a photograph of
the poster as shown!
EOF

ALT_PHT_a01a_SNA_B1: << EOF
Take a photograph of
the poster as shown!
EOF

ALT_PHT_a01a_SNA_B2: << EOF
Take a photograph of
the poster as shown!
EOF

ALT_PHT_a01a_SNA_B3: << EOF
Take a photograph of
the poster as shown!
EOF

ALT_PHT_a01a_SNA_B4: << EOF
Take a photograph of
the poster as shown!
EOF

ALT_PHT_a01a_SNA_B5: << EOF
Take a photograph of
the poster as shown!
EOF

ALT_PHT_a01a_SNA_B6: << EOF
Hideo Kojima
EOF

// a24d
ALT_PHT_a24d_SNA: << EOF
Take a photograph
as shown!
EOF

// a24a
ALT_PHT_a24a_SNA: << EOF
Take a photograph of
a poster with the same
pose as the silhouette.
EOF

// a02a
ALT_PHT_a02a_SNA: << EOF
Take a photograph with
the same pose as the
silhouette.
EOF

// a41a
ALT_PHT_a41a_SNA: << EOF
"Murder in the Prison Cell"
Raiden has been killed.
Take a photograph of the killer!
EOF

// a24c
ALT_PHT_a24c_SNA: << EOF
"Serial Murder"
Soldiers have been killed.
Take a photograph of the killer!
EOF


}


#else

print 'vr_window_e.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'


#endif
