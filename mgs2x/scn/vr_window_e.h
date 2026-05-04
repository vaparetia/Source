/*
	vr_window_e.h
	    ＶＲウインドウのリソース(英語版)

	2002/05/21 H.Yoshiike
	$Id: vr_window_e.h,v 1.39 2002/09/25 00:46:42 usr03202 Exp $


*/


#ifndef d:VR_WINDOW_E
#define	VR_WINDOW_E	1

resource	VRMSG_E	{
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

// wp03a	//済み
VRM_WPN_HGN_wp03a: << EOF
Destroy all targets and head to the goal!
The orange targets explode
when destroyed. They can be used to
destroy nearby targets.
EOF

// wp04a	//	済み
VRM_WPN_HGN_wp04a: << EOF
Destroy all targets and head to the goal!
Make good use of the Lock-on button to
move while aiming your gun. Destroying
the deduction obstacles (red targets)
will lower your score.
EOF

// wp05a	//済み
VRM_WPN_HGN_wp05a: << EOF
Use what you've learned so far to destroy
all targets and head to the goal!
EOF


// WEAPON MODE-ASSAULT RIFLE 1-5
// wp11a	//
VRM_WPN_ASR_wp11a: << EOF
Destroy all targets and head to the goal!
EOF

// wp12a	//
VRM_WPN_ASR_wp12a: << EOF
Destroy all targets and head to the goal!
EOF

// wp13a	//
VRM_WPN_ASR_wp13a: << EOF
Destroy all targets and head to the goal!
Destroying the deduction obstacles
(red targets) will lower your score.
EOF

// wp14a	//
VRM_WPN_ASR_wp14a: << EOF
Destroy all targets and head to the goal!
EOF

// wp15a	// 済み
VRM_WPN_ASR_wp15a: << EOF
Use what you've learned so far and destroy
all targets and head to the goal!
EOF


// WEAPON MODE-C4/CLAYMORE 1-5
// wp21a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp21a: << EOF
Destroy all targets and head to the goal!
Use different weapons accordingly!
EOF

// wp22a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp22a: << EOF
Destroy all targets and head to the goal!
The orange targets explode
when destroyed. Be careful not to be
caught in the explosion!
EOF

// wp23a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp23a: << EOF
Destroy all targets and head to the goal!
EOF

// wp24a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp24a: << EOF
Destroy all targets and head to the goal!
EOF

// wp25a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp25a: << EOF
Use what you've learned so far to destroy
all targets and head to the goal!
EOF


// WEAPON MODE-GRENADE 1-5
// wp31a	// 済み
VRM_WPN_GRN_wp31a: << EOF
Destroy all targets and head to the goal!
Grenades bounce off walls and targets.
Make good use of this feature.
EOF

// wp32a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp32a: << EOF
Destroy all targets and head to the goal!
EOF

// wp33a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp33a: << EOF
Destroy all targets and head to the goal!
EOF

// wp34a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp34a: << EOF
Destroy all targets and head to the goal!
EOF

// wp35a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp35a: << EOF
Use what you've learned so far to destroy
all targets and head to the goal!
EOF


// WEAPON MODE-PSG1 1-5
// wp41a
VRM_WPN_PSG_wp41a: << EOF
Destroy all targets and head to the goal!
Find the best sniping spot. You cannot
move with a weapon equipped. Be careful!
EOF

// wp42a
VRM_WPN_PSG_wp42a: << EOF
Destroy all targets and head to the goal!
EOF

// wp43a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp43a: << EOF
Destroy all targets and head to the goal!
The orange targets explode
when destroyed. They can be used to
destroy nearby targets.
EOF

// wp44a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp44a: << EOF
Destroy all targets and head to the goal!
Destroying the deduction obstacles
(red targets) will lower your score.
EOF

// wp45a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp45a: << EOF
Use what you've learned so far and destroy
all targets and head to the goal!
EOF

// WEAPON MODE-STINGER 1-5
// wp51a	//済み
VRM_WPN_STG_wp51a: << EOF
Destroy all targets and head to the goal!
You cannot move with a weapon equipped. 
Be careful! Try moving closer to the
targets if you have trouble hitting them.
EOF

// wp52a	//
VRM_WPN_STG_wp52a: << EOF
Destroy all targets and head to the goal!
EOF

// wp53a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp53a: << EOF
Destroy all targets and head to the goal!
EOF

// wp54a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp54a: << EOF
Destroy all targets and head to the goal!
EOF

// wp55a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp55a: << EOF
Use what you've learned so far to destroy
all targets and head to the goal!
EOF


// WEAPON MODE-NIKITA 1-5
// wp61a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp61a: << EOF
Destroy all targets and head to the goal!
EOF

// wp62a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp62a: << EOF
Destroy all targets and head to the goal!
EOF

// wp63a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp63a: << EOF
Destroy all targets and head to the goal!
EOF

// wp64a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp64a: << EOF
Destroy the target and head to the goal!
Destroying the deduction obstacles
(red targets) will lower your score.
EOF

// wp65a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp65a: << EOF
Use what you've learned so far to destroy
the target and head to the goal!
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

// wp73a	//	修正	2002/08/19	S.Mukaide
VRM_WPN_HFB_wp73a: << EOF
Destroy all targets and head to the goal!
Knock nearby targets into distant
ones with a kick or somersault!
EOF

// wp74a	//	修正	2002/08/19	S.Mukaide
VRM_WPN_HFB_wp74a: << EOF
Destroy all targets and head to the goal!
Destroying the deduction obstacles
(red targets) will lower your score.
EOF

// wp75a	//	修正	2002/08/19	S.Mukaide
VRM_WPN_HFB_wp75a: << EOF
Use what you've learned so far to destroy
all targets and head to the goal!
EOF


// FIRST PERSON VIEW MODE- 1-5
// sp21a	/* 修正:2002/08/21 M.Uehara */
VRM_FPV_sp21a: << EOF
Destroy all targets and head to the goal!
Use different weapons accordingly!
EOF


// sp22a	//	修正	2002/08.21	H.Yoshiike
VRM_FPV_sp22a: << EOF
Get to the goal alive!
Use the items wisely.
EOF


// sp23a
VRM_FPV_sp23a: << EOF
Take out all enemies and get to the goal!
EOF


// sp24a	//	修正	2002/08.21	H.Yoshiike
VRM_FPV_sp24a: << EOF
Destroy all targets and head to the goal!
Watch out for enemy guards.
EOF


// sp25a
VRM_FPV_sp25a: << EOF
Defuse all bombs!
The sound the bombs make may give
you clues about their locations.
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
ZAKO SURVIVAL
Take out all enemies!
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
Get to the goal without being spotted by
the gigantic creature! Watch it carefully 
by standing against the wall, peeking
around the corner and get through!
Make good use of the corner view camera!
EOF

// sp08a ダークステージ						//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp08a: << EOF
Take out all enemies in the dark
and get to the goal!
Use the items wisely.
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
Defuse all bombs!
The sound the bombs make may give
you clues about their locations.
EOF

// プリスキン＆タキシードスネーク
ALT_BMB_2: << EOF
Defuse all bombs!
The bombs will automatically explode
if you come within a 2-meter radius.
Be careful!
EOF

// 忍者＆前作スネーク
ALT_BMB_3: << EOF
Defuse all bombs!
Use both Sensors A & B and
keep an eye on all places!
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
You may knock them unconscious or
tranquilize them instead of killing them.
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

// a24a		/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a24a: << EOF
Take out all enemies and get to the goal!
The enemy shotgun is powerful.
Be very careful!
EOF

// a31a		/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a31a: << EOF
Take out all enemies and get to the goal!
EOF

// a22a		/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a22a: << EOF
Take out all enemies and get to the goal!
EOF

// a42a		/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a42a: << EOF
Take out all enemies and get to the goal!
EOF

// a02a		/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a02a: << EOF
Take out all enemies and get to the goal!
What you see isn't always all that's there.
EOF


// HOLD UP 1-10
// a15b
ALT_HLD_a15b: << EOF
Hold up all enemies and get to the goal!
The mission will be terminated if you
are discovered. You can wake up
sleeping enemies with the coolant spray.
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
The mission will be terminated if you
are discovered. Be careful of soldiers
making regular calls!
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
Take a photo of
the displayed "!".
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
Find the photograph of the girl of the
developer's type!
Yutaka Negishi (34 years old, married)
Titles worked on: Metal Gear Solid 2
(Background Art Director)
EOF

ALT_PHT_a01a_SNA_B2: << EOF
Find the photograph of the girl of the
developer's type!
Masahiro Hinami (36 years old, married)
Representative title: Ring of Red
EOF

ALT_PHT_a01a_SNA_B3: << EOF
Find the photograph of the girl of the
developer's type!
Yoshikazu Matsuhana (father of 2 kids)
Titles worked on: Snatcher, Policenauts,
Lethal Enforcers, Metal Gear Solid series
EOF

ALT_PHT_a01a_SNA_B4: << EOF
Find the photograph of the girl of the
developer's type!
Shinta Nojiri (31years old, sort of single)
Titles worked on: Metal Gear (GB),
Metal Gear Solid 2 (Script unit)
EOF

ALT_PHT_a01a_SNA_B5: << EOF
Find the photograph of the girl of the
developer's type!
Sadaaki Kaneyoshi (29 years old, single)
Titles worked on: GuitarFreaks series,
Metal Gear Solid 2 (Script unit)
EOF

ALT_PHT_a01a_SNA_B6: << EOF
Find the photograph of the girl of the
developer's type!
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
