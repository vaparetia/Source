/*
	vr_window_g.h
	    ＶＲウインドウのリソース(ドイツ語版)

	2002/05/21 H.Yoshiike
	$Id: vr_window_g.h,v 1.20 2002/10/07 08:13:06 usr03202 Exp $


*/


#ifndef d:VR_WINDOW_G
#define	VR_WINDOW_G	1

resource	VRMSG_G	{
//------------------------
// 共通部分のリソース
//------------------------
時間: { ZEITLIMIT :}
分: {Min}
秒: {Sek}
敵兵数: { FEINDE    :}
人: { 　 }
標的数: { ZIELE     :}
爆弾数: { BOMBEN    :}
個: { 　 }

//------------------------
// ステージ固有のリソース
//------------------------
// VR MISSION
// SNEAKING MODE-SNEAKING 1-10
// vs01a
VRM_SNK_SNK_vs01a: << EOF
Erreichen Sie das Ziel, ohne entdeckt
zu werden! Dr&~ucken Sie sich dabei
flach gegen die Wand.
EOF

// vs02a
VRM_SNK_SNK_vs02a: << EOF
Erreichen Sie das Ziel, ohne entdeckt
zu werden! Die Wache geht immer
dieselbe Runde.
EOF

// vs03a
VRM_SNK_SNK_vs03a: << EOF
Erreichen Sie das Ziel, ohne entdeckt zu
werden! Das Feindverhalten vorhersehen 
und EINDRINGEN sind der Schl&~ussel 
zum Erfolg.
EOF

// vs04a
VRM_SNK_SNK_vs04a: << EOF
Erreichen Sie das Ziel, ohne entdeckt zu
werden! K&~odern Sie den Feind, 
um einen Weg frei zu bekommen.
EOF

// vs05a
VRM_SNK_SNK_vs05a: << EOF
Erreichen Sie das Ziel, ohne entdeckt
zu werden! &~Uberwinden Sie durch
HANGELN die Linie des Feinds.
EOF

// vs06a
VRM_SNK_SNK_vs06a: << EOF
Erreichen Sie das Ziel! Vorsicht vor 
Fu&~sbodenger&~auschen bzw.
Spuren auf dem nassen Fu&~sboden.
Die Mission wird abgebrochen, wenn Sie
entdeckt werden oder in ein Loch fallen.
EOF

// vs07a
VRM_SNK_SNK_vs07a: << EOF
Nutzen Sie die Schr&~anke und erreichen 
Sie das Ziel! Die Mission wird 
abgebrochen, wenn Sie entdeckt werden. 
Die Sicherheit wird versch&~arft, 
wenn der Feind aufwacht.
EOF

// vs08a
VRM_SNK_SNK_vs08a: << EOF
Erreichen Sie das Ziel, ohne vom Feind
oder der Kamera entdeckt zu werden.
Die Mission wird abgebrochen, wenn
Sie entdeckt werden. Direkt unter der
Kamera ist ein blinder Fleck.
EOF

// vs09a
VRM_SNK_SNK_vs09a: << EOF
Nutzen Sie die Objekte klug und erreichen
Sie das Ziel! Die Mission wird
abgebrochen, wenn Sie entdeckt werden.
EOF

// vs10a
VRM_SNK_SNK_vs10a: << EOF
Nutzen Sie Ihre Erfahrungen und 
erreichen Sie das Ziel, ohne entdeckt 
zu werden!
EOF


// SNEAKING MODE-ELIMINATE ALL 1-10
// vs01a
VRM_SNK_ELM_vs01a: << EOF
Schalten Sie alle Feinde aus und erreichen
Sie das Ziel! Bewusstlosschlagen oder
Bet&~auben z&~ahlt zus&~atzlich zu T&~oten.
EOF

// vs02a
VRM_SNK_ELM_vs02a: << EOF
Schalten Sie alle Feinde aus und 
erreichen Sie das Ziel! Wechseln Sie 
zur Beobachtungsperspektivekamera 
und zielen Sie genau.
EOF

// vs03a
VRM_SNK_ELM_vs03a: << EOF
Schalten Sie alle Feinde aus und 
erreichen Sie das Ziel! Kopf, Herz und 
Leistengegend sind ihre Schwachpunkte. 
Ein Treffer an diesen Punkten reicht, 
um sie auszuschalten.
EOF

// vs04a
VRM_SNK_ELM_vs04a: << EOF
Schalten Sie alle Feinde aus und 
erreichen Sie das Ziel! Nutzen Sie ein 
Objekt, klopfen Sie an die Wand oder 
schie&~sen Sie, um ihre Aufmerksamkeit 
zu gewinnen.
EOF

// vs05a
VRM_SNK_ELM_vs05a: << EOF
Schalten Sie alle Feinde aus und 
erreichen Sie das Ziel! Dazu k&~onnen 
Sie aus dem HANGELN wieder hoch 
oder auf sie nach unten springen.
EOF

// vs06a
VRM_SNK_ELM_vs06a: << EOF
Schalten Sie alle Feinde aus und 
erreichen Sie das Ziel! Vorsicht, 
Fu&~sbodenger&~ausche bzw. 
nasser Fu&~sboden! Die Mission wird
abgebrochen, wenn Sie in ein Loch fallen.
EOF

// vs07a
VRM_SNK_ELM_vs07a: << EOF
Schalten Sie den Feind in seiner Position
aus und erreichen Sie das Ziel. Jeder
Fehler f&~uhrt zum Alarmmodus und
ein Angriffsteam erscheint.
EOF

// vs08a
VRM_SNK_ELM_vs08a: << EOF
Schalten Sie den Feind in seiner Position
aus und erreichen Sie das Ziel. Vorsicht,
Kamera! Die Mission wird abgebrochen,
wenn Sie entdeckt werden.
EOF

// vs09a
VRM_SNK_ELM_vs09a: << EOF
Schalten Sie den Feind in seiner Position
aus und erreichen Sie das Ziel. Setzen
Sie die Objekte klug ein.
EOF

// vs10a
VRM_SNK_ELM_vs10a: << EOF
Nutzen Sie Ihre Erfahrungen, um den 
Feind auszuschalten und das Ziel 
zu erreichen.
EOF


// WEAPON MODE-HANDGUN 1-5
// wp01a
VRM_WPN_HGN_wp01a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp02a
VRM_WPN_HGN_wp02a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp03a
VRM_WPN_HGN_wp03a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
Die orangenen Ziele explodieren bei
Zerst&~orung. Damit k&~onnen Ziele ganz
in der N&~ahe zerst&~ort werden.
EOF

// wp04a
VRM_WPN_HGN_wp04a: << EOF
Benutzen Sie die Aufschalttaste, um
w&~ahrend der Bewegung zu zielen. Die
Zerst&~orung von Abzugshindernissen
(roten Zielen) bewirkt Punktabzug.
EOF

// wp05a
VRM_WPN_HGN_wp05a: << EOF
Nutzen Sie Ihre Erfahrungen, um alle
Ziele zu zerst&~oren und dann weiter
zum Missionsziel!
EOF


// WEAPON MODE-ASSAULT RIFLE 1-5
// wp11a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp11a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp12a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp12a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp13a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp13a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
Zerst&~orung von Abzugshindernissen
(roten Zielen) bewirkt Punktabzug.
EOF

// wp14a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp14a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp15a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp15a: << EOF
Nutzen Sie Ihre Erfahrungen, zerst&~oren
Sie alle Ziele und dann weiter
zum Missionsziel!
EOF


// WEAPON MODE-C4/CLAYMORE 1-5
// wp21a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp21a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
Verschiedene Waffen entsprechend
einsetzen!
EOF

// wp22a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp22a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
Die orangenen Ziele explodieren
bei der Zerst&~orung. Geraten Sie
nicht in den Wirkungsradius!
EOF

// wp23a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp23a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp24a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp24a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp25a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp25a: << EOF
Nutzen Sie Ihre Erfahrungen, um alle
Ziele zu zerst&~oren und dann weiter
zum Missionsziel!
EOF


// WEAPON MODE-GRENADE 1-5
// wp31a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp31a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp32a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp32a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp33a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp33a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp34a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp34a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp35a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp35a: << EOF
Nutzen Sie Ihre Erfahrungen, um alle
Ziele zu zerst&~oren und dann weiter
zum Missionsziel!
EOF


// WEAPON MODE-PSG1 1-5
// wp41a
VRM_WPN_PSG_wp41a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel! Finden Sie den besten
Hinterhalt. Sie k&~onnen sich nicht mit
aufgenommener Waffe bewegen. 
Vorsicht!
EOF

// wp42a
VRM_WPN_PSG_wp42a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp43a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp43a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
Die orangenen Ziele explodieren
bei Zerst&~orung. Damit k&~onnen Ziele
ganz in der N&~ahe zerst&~ort werden.
EOF

// wp44a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp44a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
Zerst&~orung von Abzugshindernissen
(roten Zielen) bewirkt Punktabzug.
EOF

// wp45a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp45a: << EOF
Nutzen Sie Ihre Erfahrungen, zerst&~oren
Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// WEAPON MODE-STINGER 1-5
// wp51a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp51a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel! Sie k&~onnen sich nicht 
mit aufgenommener Waffe bewegen. 
Vorsicht! Gehen Sie n&~aher ran, 
wenn die Ziele schwer zu treffen sind.
EOF

// wp52a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp52a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp53a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp53a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp54a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp54a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp55a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp55a: << EOF
Nutzen Sie Ihre Erfahrungen, um alle
Ziele zu zerst&~oren und dann weiter
zum Missionsziel!
EOF


// WEAPON MODE-NIKITA 1-5
// wp61a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp61a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp62a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp62a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp63a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp63a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp64a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp64a: << EOF
Ziel zerst&~oren und dann weiter
zum Missionsziel!
Zerst&~orung von Abzugshindernissen
(roten Zielen) bewirkt Punktabzug.
EOF

// wp65a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp65a: << EOF
Nutzen Sie Ihre Erfahrungen, um das
Ziel zu zerst&~oren und dann weiter
zum Missionsziel!
EOF


// WEAPON MODE-HF.BLADE/NO WEAPON 1-5
// wp71a
VRM_WPN_HFB_wp71a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp72a
VRM_WPN_HFB_wp72a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
EOF

// wp73a
VRM_WPN_HFB_wp73a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
Verwandeln Sie mit einem Sto&~s oder
Purzelbaum nahe in ferne Ziele!
EOF

// wp74a
VRM_WPN_HFB_wp74a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
Zerst&~orung von Abzugshindernissen
(roten Zielen) bewirkt Punktabzug.
EOF

// wp75a
VRM_WPN_HFB_wp75a: << EOF
Nutzen Sie Ihre Erfahrungen, um alle
Ziele zu zerst&~oren und dann weiter
zum Missionsziel!
EOF


// FIRST PERSON VIEW MODE- 1-5
// sp21a	/* 修正:2002/08/21 M.Uehara */
VRM_FPV_sp21a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
Verschiedene Waffen entsprechend
einsetzen!
EOF


// sp22a	//	修正	2002/08.21	H.Yoshiike
VRM_FPV_sp22a: << EOF
Erreichen Sie lebend das Ziel!
Nutzen Sie die Objekte klug.
EOF


// sp23a
VRM_FPV_sp23a: << EOF
Schalten Sie alle Feinde aus und 
erreichen Sie das Ziel!
EOF


// sp24a	//	修正	2002/08.21	H.Yoshiike
VRM_FPV_sp24a: << EOF
Zerst&~oren Sie alle Ziele und dann weiter
zum Missionsziel!
Achten Sie auf Feindwachen.
EOF


// sp25a
VRM_FPV_sp25a: << EOF
Entsch&~arfen Sie alle Bomben!
Das Ger&~ausch von Bomben weist
darauf hin, wo sie eventuell sind.
EOF


// VARIETY MODE- 1-7
// sp01a-1 アスレチックステージ機雷バージョン
VRM_VRT_sp01a_1: << EOF
Erreichen Sie mit verschiedenen
Bewegungen das Ziel! Vorsicht, Minen!
EOF

// sp01a-1 アスレチックステージ的バージョン
VRM_VRT_sp01a_2: << EOF
Zerst&~oren Sie alle Ziele und erreichen
Sie das Ziel!
EOF

// sp02a
VRM_VRT_sp02a: << EOF
Erreichen Sie durch HANGELN
das Ziel!
Dr&~ucken Sie im Fallen rechtzeitig die
Aktionstaste, um wieder zu H&~ANGEN!
EOF

// sp03a ザコサバイバル		//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp03a: << EOF
ZAKO SURVIVAL
Schalten Sie alle Feinde aus!
EOF

// sp06a 狙撃ミッション
VRM_VRT_sp06a_1: << EOF
Schalten Sie alle Feinde aus, 
die sich n&~ahern!
Lassen Sie sie nicht an die Person 
auf dem Fu&~sboden heran!
EOF

VRM_VRT_sp06a_2: << EOF
Schalten Sie alle Feinde aus, 
die sich n&~ahern!
Lassen Sie die Feinde nicht in die 
N&~ahe des Currys!
EOF

VRM_VRT_sp06a_3: << EOF
Schalten Sie alle Feinde aus, 
die sich n&~ahern!
Lassen Sie die Feinde nicht in die 
N&~ahe der Nudeln!
EOF

VRM_VRT_sp06a_4: << EOF
Schalten Sie alle Feinde aus, 
die sich n&~ahern!
Lassen Sie die Feinde nicht in die 
N&~ahe des Kartons!
EOF

// sp07a ゴルルゴンステージ					//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp07a: << EOF
Erreichen Sie das Ziel, ohne vom
Riesengesch&~opf entdeckt zu werden!
An der Wand stehen, um die Ecke sp&~ahen,
genau beobachten und durchkommen!
Schulterblickkamera gut nutzen!
EOF

// sp08a ダークステージ						//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp08a: << EOF
Schalten Sie alle Feinde im Dunkeln
aus und erreichen Sie das Ziel!
Nutzen Sie die Objekte klug.
EOF


// STREAKING MODE- 1
// sp09a
VRM_STR_sp09a: << EOF
誰にも見つからずにゴールを目指せ！
EOF

// STREAKING MODE- 1
// st01a
VRM_STR_st01a: << EOF
Erreichen Sie das Ziel in 5 aufeinander-
folgenden Stufen, ohne 
entdeckt zu werden!
EOF

VRM_STR_st02a: << EOF
Erreichen Sie das Ziel, 
ohne entdeckt zu werden!
EOF

VRM_STR_st03a: << EOF
Erreichen Sie das Ziel, 
ohne entdeckt zu werden!
EINDRINGEN ist eine wirksame Methode.
EOF

VRM_STR_st04a: << EOF
Erreichen Sie das Ziel, 
ohne entdeckt zu werden!
Die Sicherheitsma&~snahmen sind streng.
Aber es gibt doch einen Weg.
EOF

VRM_STR_st05a: << EOF
Dies ist die letzte Stufe.
Jetzt nicht die Konzentration verlieren.
Erreichen Sie das Ziel!
EOF


// ALTNATIVE MISSION
// BOMB DISPOSAL 1-5
// 爆弾解体は各ステージ共通で	//	修正	2002/08.21	H.Yoshiike 
// ライデン＆スネーク
ALT_BMB_1: << EOF
Entsch&~arfen Sie alle Bomben!
Das Ger&~ausch von Bomben weist
darauf hin, wo sie eventuell sind.
EOF

// プリスキン＆タキシードスネーク
ALT_BMB_2: << EOF
Entsch&~arfen Sie alle Bomben!
Die Bomben explodieren automatisch
innnerhalb eines 2-Meter-Radius.
Vorsicht!
EOF

// 忍者＆前作スネーク
ALT_BMB_3: << EOF
Entsch&~arfen Sie alle Bomben!
Nutzen Sie Sensor A und B und
beobachten Sie alle Orte!
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
Schalten Sie alle Feinde aus und
erreichen Sie das Ziel!
Sie k&~onnen Sie bewusstlos schlagen
oder bet&~auben, statt sie zu t&~oten.
EOF

// a01a
ALT_ELM_a01a: << EOF
Schalten Sie alle Feinde aus und
erreichen Sie das Ziel!
EOF

// a19a
ALT_ELM_a19a: << EOF
Schalten Sie alle Feinde aus und
erreichen Sie das Ziel!
EOF

// a20a
ALT_ELM_a20a: << EOF
Schalten Sie alle Feinde aus und
erreichen Sie das Ziel!
EOF

// a24d
ALT_ELM_a24d: << EOF
Schalten Sie alle Feinde aus und
erreichen Sie das Ziel!
EOF

// a24a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a24a: << EOF
Schalten Sie alle Feinde aus und
erreichen Sie das Ziel!
Die Feindflinte ist wirkungsvoll.
Sie m&~ussen gut aufpassen!
EOF

// a31a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a31a: << EOF
Schalten Sie alle Feinde aus und
erreichen Sie das Ziel!
EOF

// a22a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a22a: << EOF
Schalten Sie alle Feinde aus und
erreichen Sie das Ziel!
EOF

// a42a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a42a: << EOF
Schalten Sie alle Feinde aus und
erreichen Sie das Ziel!
EOF

// a02a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a02a: << EOF
Schalten Sie alle Feinde aus und
erreichen Sie das Ziel!
Manchmal sehen Sie nicht alles!
EOF


// HOLD UP 1-10
// a15b
ALT_HLD_a15b: << EOF
Halten Sie alle Feinde auf und erreichen
Sie das Ziel! Die Mission wird 
abgebrochen, wenn Sie entdeckt werden. 
Wecken Sie schlafende Feinde mit 
dem K&~uhlspray.
EOF

// a12a
ALT_HLD_a12a: << EOF
Halten Sie alle Feinde auf und erreichen
Sie das Ziel! Die Mission wird 
abgebrochen, wenn Sie entdeckt werden.
EOF

// a24d
ALT_HLD_a24d: << EOF
Halten Sie alle Feinde auf und erreichen
Sie das Ziel! Die Mission wird 
abgebrochen, wenn Sie entdeckt werden.
EOF

// a13b
ALT_HLD_a13b: << EOF
Halten Sie alle Feinde auf und erreichen
Sie das Ziel! Die Mission wird 
abgebrochen, wenn Sie entdeckt werden. 
Vorsicht vor Soldaten, die regelm&~a&~sig 
Funkgespr&~ache f&~uhren!
EOF

// a14a
ALT_HLD_a14a: << EOF
Halten Sie alle Feinde auf und erreichen
Sie das Ziel! Die Mission wird 
abgebrochen, wenn Sie entdeckt werden.
EOF

// a22a
ALT_HLD_a22a: << EOF
Halten Sie alle Feinde auf und erreichen
Sie das Ziel! Die Mission wird 
abgebrochen, wenn Sie entdeckt werden.
EOF

// a01b
ALT_HLD_a01b: << EOF
Halten Sie alle Feinde auf und erreichen
Sie das Ziel! Die Mission wird 
abgebrochen, wenn Sie entdeckt werden.
EOF

// a42a
ALT_HLD_a42a: << EOF
Halten Sie alle Feinde auf und erreichen
Sie das Ziel! Die Mission wird 
abgebrochen, wenn Sie entdeckt werden.
EOF

// a20b
ALT_HLD_a20b: << EOF
Halten Sie alle Feinde auf und erreichen
Sie das Ziel! Die Mission wird 
abgebrochen, wenn Sie entdeckt werden.
EOF

// a31a
ALT_HLD_a31a: << EOF
Halten Sie alle Feinde auf und erreichen
Sie das Ziel! Die Mission wird 
abgebrochen, wenn Sie entdeckt werden.
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
Machen Sie ein Foto, 
wie hier gezeigt!
EOF

// a01b
ALT_PHT_a01b_RAI: << EOF
Machen Sie ein Foto, 
wie hier gezeigt!
EOF

// a01f
ALT_PHT_a01f_RAI: << EOF
Machen Sie ein Foto
des gezeigten "!".
EOF

// a00c
ALT_PHT_a00c_RAI: << EOF
Machen Sie ein Foto der Sterne!
EOF

// a00a
ALT_PHT_a00a_RAI: << EOF
Einem Ger&~ucht zufolge soll ein 
Soldatengespenst auf dem Deck 
umherirren. 
Finden Sie das Gespenst und 
fotografieren Sie es!
EOF

// a03a
ALT_PHT_a03a_RAI: << EOF
Fotografieren Sie das
Gespenst! Gespenster
hassen alles Helle.
EOF


// PHOTOGRAPH SNAKE 1-5
// a01a
ALT_PHT_a01a_SNA_A: << EOF
Machen Sie ein Foto 
des Posters, 
wie hier gezeigt!
EOF

ALT_PHT_a01a_SNA_B1: << EOF
Finden Sie das Foto vom Frauentyp
des Entwicklers!
Yutaka Negishi (34 Jahre, verheiratet)
Mitarbeit bei: Metal Gear Solid 2
(k&~unstl. Leiter Hintergrund)
EOF

ALT_PHT_a01a_SNA_B2: << EOF
Finden Sie das Foto vom Frauentyp
des Entwicklers!
Masahiro Hinami (36 Jahre, verheiratet)
Repr&~asentativer Titel: Ring of Red
EOF

ALT_PHT_a01a_SNA_B3: << EOF
Finden Sie das Foto vom Frauentyp
des Entwicklers!
Yoshikazu Matsuhana (Vater von 2 Kindern)
Mitarbeit bei: Snatcher, Policenauts,
Lethal Enforcers, Metal Gear Solid Serie
EOF

ALT_PHT_a01a_SNA_B4: << EOF
Finden Sie das Foto vom Frauentyp
des Entwicklers!
Shinta Nojiri (31 Jahre, so gut wie ledig)
Mitarbeit bei: Metal Gear (GB),
Metal Gear Solid 2 (Skripteinheit)
EOF

ALT_PHT_a01a_SNA_B5: << EOF
Finden Sie das Foto vom Frauentyp
des Entwicklers!
Sadaaki Kaneyoshi (29 Jahre, ledig)
Mitarbeit bei: GuitarFreaks Serie,
Metal Gear Solid 2 (Skripteinheit)
EOF

ALT_PHT_a01a_SNA_B6: << EOF
Finden Sie das Foto vom Frauentyp
des Entwicklers!
Hideo Kojima
EOF

// a24d
ALT_PHT_a24d_SNA: << EOF
Machen Sie ein Foto,
wie hier gezeigt!
EOF

// a24a
ALT_PHT_a24a_SNA: << EOF
Machen Sie ein Foto 
von einem Poster mit
der gleichen Pose wie
die Silhouette.
EOF

// a02a
ALT_PHT_a02a_SNA: << EOF
Machen Sie ein Foto 
mit der gleichen Pose
wie die Silhouette.
EOF

// a41a
ALT_PHT_a41a_SNA: << EOF
"Mord in der Gef&~angniszelle"
Raiden wurde get&~otet.
Machen Sie ein Foto des M&~orders!
EOF

// a24c
ALT_PHT_a24c_SNA: << EOF
"Serienm&~order"
Soldaten wurden get&~otet.
Machen Sie ein Foto des M&~orders!
EOF


}


#else

print 'vr_window_e.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'


#endif
