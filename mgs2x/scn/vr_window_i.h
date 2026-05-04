/*
	vr_window_i.h
	    ＶＲウインドウのリソース(イタリア語版)

	2002/05/21 H.Yoshiike
	$Id: vr_window_i.h,v 1.21 2002/10/07 08:13:12 usr03202 Exp $


*/


#ifndef d:VR_WINDOW_I
#define	VR_WINDOW_I	1

resource	VRMSG_I	{
//------------------------
// 共通部分のリソース
//------------------------
時間: { TEMPO LIMITE:}
分: {min}
秒: {sec}
敵兵数: { NEMICI    :}
人: { 　 }
標的数: { BERSAGLI  :}
爆弾数: { BOMBE     :}
個: { 　 }

//------------------------
// ステージ固有のリソース
//------------------------
// VR MISSION
// SNEAKING MODE-SNEAKING 1-10
// vs01a
VRM_SNK_SNK_vs01a: << EOF
Arriva alla meta senza farti scoprire!
Appiattirsi contro il muro &`e un buon
metodo.
EOF

// vs02a
VRM_SNK_SNK_vs02a: << EOF
Arriva alla meta senza farti scoprire!
La pattuglia nemica ha un percorso fisso.
EOF

// vs03a
VRM_SNK_SNK_vs03a: << EOF
Arriva alla meta senza farti scoprire!
Anticipa il comportamento nemico e usa
l'INCURSIONE per arrivare in fondo.
EOF

// vs04a
VRM_SNK_SNK_vs04a: << EOF
Arriva alla meta senza farti scoprire!
Attira il nemico per aprirti un varco.
EOF

// vs05a
VRM_SNK_SNK_vs05a: << EOF
Arriva alla meta senza farti scoprire!
Usa la SOSPENSIONE per superare
la linea nemica.
EOF

// vs06a
VRM_SNK_SNK_vs06a: << EOF
Arriva alla meta!
Attenzione a pavimenti scricchiolanti
e bagnati su cui lasci le impronte.
La missione termina se ti scoprono 
o cadi in una buca.
EOF

// vs07a
VRM_SNK_SNK_vs07a: << EOF
Usa bene gli armadietti e arriva alla meta!
La missione termina se ti scoprono.
I controlli aumentano se il nemico si sveglia.
EOF

// vs08a
VRM_SNK_SNK_vs08a: << EOF
Arriva alla meta senza farti scoprire da
nemici o telecamere di sorveglianza.
La missione termina se ti scoprono.
Sotto la telecamera non ti vedranno.
EOF

// vs09a
VRM_SNK_SNK_vs09a: << EOF
Usa bene gli oggetti e arriva alla meta!
La missione termina se ti scoprono.
EOF

// vs10a
VRM_SNK_SNK_vs10a: << EOF
Usa ci&`o che hai appreso fin qui e
arriva alla meta senza farti scoprire!
EOF


// SNEAKING MODE-ELIMINATE ALL 1-10
// vs01a
VRM_SNK_ELM_vs01a: << EOF
Metti k.o. il nemico e arriva alla meta!
Oltre che ucciderlo, puoi anche
tramortirlo o dargli sedativi.
EOF

// vs02a
VRM_SNK_ELM_vs02a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
Per sparare con precisione, usa la 
telecamera con Visuale Soggettiva 
e mira bene.
EOF

// vs03a
VRM_SNK_ELM_vs03a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
I loro punti deboli sono la testa, il cuore
e l'inguine. Se miri a questi punti, li
metti k.o. con un solo colpo.
EOF

// vs04a
VRM_SNK_ELM_vs04a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
Usa un oggetto, batti sul muro o spara
vicino per attirare la loro attenzione.
EOF

// vs05a
VRM_SNK_ELM_vs05a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
Per metterli k.o. salta su dalla
SOSPENSIONE o saltagli addosso.
EOF

// vs06a
VRM_SNK_ELM_vs06a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
Attenzione a pavimenti scricchiolanti
e bagnati. La missione termina se
cadi in una buca.
EOF

// vs07a
VRM_SNK_ELM_vs07a: << EOF
Metti k.o. i nemici in posizione e
arriva alla meta. Se il tentativo fallisce,
si attiva la modalit&`a ALLERTA
e compare una squadra di attacco.
EOF

// vs08a
VRM_SNK_ELM_vs08a: << EOF
Metti k.o. i nemici in posizione e
arriva alla meta. Attenzione alla telecamera
di sorveglianza. La missione termina
se ti scoprono.
EOF

// vs09a
VRM_SNK_ELM_vs09a: << EOF
Metti k.o. i nemici in posizione e
arriva alla meta. Usa bene gli oggetti.
EOF

// vs10a
VRM_SNK_ELM_vs10a: << EOF
Usa ci&`o che hai appreso fin qui, metti k.o.
i nemici in posizione e arriva alla meta.
EOF


// WEAPON MODE-HANDGUN 1-5
// wp01a
VRM_WPN_HGN_wp01a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp02a
VRM_WPN_HGN_wp02a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp03a
VRM_WPN_HGN_wp03a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
I bersagli arancioni esplodono se vengono 
distrutti. Possono essere usati per 
distruggere bersagli vicini.
EOF

// wp04a
VRM_WPN_HGN_wp04a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
Usa bene il tasto Aggancio per spostarti
mentre prendi la mira. Se distruggi
gli ostacoli sottrattivi (bersagli rossi),
il tuo punteggio cala.
EOF

// wp05a
VRM_WPN_HGN_wp05a: << EOF
Usa ci&`o che hai appreso fin qui per 
distruggere tutti i bersagli e dirigiti 
alla meta!
EOF


// WEAPON MODE-ASSAULT RIFLE 1-5
// wp11a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp11a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp12a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp12a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp13a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp13a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
Se distruggi gli ostacoli sottrattivi
(bersagli rossi), il tuo punteggio cala.
EOF

// wp14a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp14a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp15a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp15a: << EOF
Usa ci&`o che hai appreso fin qui, distruggi
tutti i bersagli e dirigiti alla meta!
EOF


// WEAPON MODE-C4/CLAYMORE 1-5
// wp21a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp21a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
Usa armi diverse a seconda dei casi!
EOF

// wp22a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp22a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
I bersagli arancioni esplodono
se vengono distrutti. Attenzione a non
farti raggiungere dall'esplosione!
EOF

// wp23a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp23a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp24a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp24a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp25a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp25a: << EOF
Usa ci&`o che hai appreso fin qui 
per distruggere tutti i bersagli 
e dirigiti alla meta!
EOF


// WEAPON MODE-GRENADE 1-5
// wp31a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp31a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp32a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp32a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp33a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp33a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp34a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp34a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp35a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp35a: << EOF
Usa ci&`o che hai appreso fin qui 
per distruggere tutti i bersagli 
e dirigiti alla meta!
EOF


// WEAPON MODE-PSG1 1-5
// wp41a
VRM_WPN_PSG_wp41a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
Trova il posto migliore per sparare stando 
nascosto. Non puoi spostarti con un'arma 
equipaggiata. Fai attenzione!
EOF

// wp42a
VRM_WPN_PSG_wp42a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp43a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp43a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
I bersagli arancioni esplodono se vengono 
distrutti. Possono essere usati per 
distruggere bersagli vicini.
EOF

// wp44a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp44a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
Se distruggi gli ostacoli sottrattivi
(bersagli rossi), il tuo punteggio cala.
EOF

// wp45a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp45a: << EOF
Usa ci&`o che hai appreso fin qui, distruggi
tutti i bersagli e dirigiti alla meta!
EOF


// WEAPON MODE-STINGER 1-5
// wp51a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp51a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
Non puoi spostarti con un'arma 
equipaggiata. Fai attenzione!
Prova ad avvicinarti ai bersagli
se non riesci a colpirli.
EOF

// wp52a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp52a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp53a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp53a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp54a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp54a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp55a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp55a: << EOF
Usa ci&`o che hai appreso fin qui 
per distruggere tutti i bersagli 
e dirigiti alla meta!
EOF


// WEAPON MODE-NIKITA 1-5
// wp61a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp61a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp62a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp62a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp63a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp63a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp64a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp64a: << EOF
Distruggi il bersaglio e dirigiti alla meta!
Se distruggi gli ostacoli sottrattivi
(bersagli rossi), il tuo punteggio cala.
EOF

// wp65a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp65a: << EOF
Usa ci&`o che hai appreso fin qui 
per distruggere il bersaglio 
e dirigiti alla meta!
EOF


// WEAPON MODE-HF.BLADE/NO WEAPON 1-5
// wp71a
VRM_WPN_HFB_wp71a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp72a
VRM_WPN_HFB_wp72a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
EOF

// wp73a
VRM_WPN_HFB_wp73a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
Scaraventa lontano i bersagli vicini
con un calcio o una capriola!
EOF

// wp74a
VRM_WPN_HFB_wp74a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
Se distruggi gli ostacoli sottrattivi
(bersagli rossi), il tuo punteggio cala.
EOF

// wp75a
VRM_WPN_HFB_wp75a: << EOF
Usa ci&`o che hai appreso fin qui 
per distruggere tutti i bersagli 
e dirigiti alla meta!
EOF


// FIRST PERSON VIEW MODE- 1-5
// sp21a	/* 修正:2002/08/21 M.Uehara */
VRM_FPV_sp21a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
Usa armi diverse a seconda dei casi!
EOF


// sp22a	//	修正	2002/08.21	H.Yoshiike
VRM_FPV_sp22a: << EOF
Arriva vivo alla meta!
Usa bene gli oggetti.
EOF


// sp23a
VRM_FPV_sp23a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
EOF


// sp24a	//	修正	2002/08.21	H.Yoshiike
VRM_FPV_sp24a: << EOF
Distruggi tutti i bersagli e dirigiti alla meta!
Fai attenzione alle guardie nemiche.
EOF


// sp25a
VRM_FPV_sp25a: << EOF
Disinnesca tutte le bombe!
Il suono emesso dalle bombe pu&`o farti
capire dove sono state piazzate.
EOF


// VARIETY MODE- 1-7
// sp01a-1 アスレチックステージ機雷バージョン
VRM_VRT_sp01a_1: << EOF
Usa varie mosse e arriva alla meta!
Attenzione alle mine!
EOF

// sp01a-1 アスレチックステージ的バージョン
VRM_VRT_sp01a_2: << EOF
Distruggi tutti i bersagli e arriva alla meta!
EOF

// sp02a
VRM_VRT_sp02a: << EOF
Usa la mossa SOSPENSIONE-CADUTA-
PRESA e arriva alla meta! Premi il 
tasto Azione al momento giusto mentre 
cadi per andare in SOSPENSIONE!
EOF

// sp03a ザコサバイバル		//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp03a: << EOF
ZAKO SURVIVAL
Metti k.o. tutti i nemici!
EOF

// sp06a 狙撃ミッション
VRM_VRT_sp06a_1: << EOF
Metti k.o. tutti i nemici che si avvicinano!
Non farli avvicinare alla persona 
distesa al suolo!
EOF

VRM_VRT_sp06a_2: << EOF
Metti k.o. tutti i nemici che si avvicinano!
Non fare avvicinare i nemici al curry!
EOF
VRM_VRT_sp06a_3: << EOF
Metti k.o. tutti i nemici che si avvicinano!
Non fare avvicinare i nemici ai tagliolini!
EOF

VRM_VRT_sp06a_4: << EOF
Metti k.o. tutti i nemici che si avvicinano!
Non fare avvicinare i nemici alla 
scatola di cartone!
EOF

// sp07a ゴルルゴンステージ					//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp07a: << EOF
Arriva alla meta senza farti scoprire
dall'enorme creatura! Osservala 
attentamente appoggiandoti alla parete,
sbircia da dietro l'angolo e superala! 
EOF

// sp08a ダークステージ						//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp08a: << EOF
Metti k.o. tutti i nemici nel buio
e arriva alla meta!
Usa bene gli oggetti.
EOF


// STREAKING MODE- 1
// sp09a
VRM_STR_sp09a: << EOF
誰にも見つからずにゴールを目指せ！
EOF

// STREAKING MODE- 1
// st01a
VRM_STR_st01a: << EOF
Arriva alla meta in 5 livelli consecutivi
senza farti scoprire!
EOF

VRM_STR_st02a: << EOF
Arriva alla meta senza farti scoprire!
EOF

VRM_STR_st03a: << EOF
Arriva alla meta senza farti scoprire!
Ricordati: l'INCURSIONE &`e efficace.
EOF

VRM_STR_st04a: << EOF
Arriva alla meta senza farti scoprire!
La sorveglianza &`e fitta.
Per&`o c'&`e una via d'uscita.
EOF

VRM_STR_st05a: << EOF
Questo &`e il livello finale.
Non perdere la concentrazione.
Arriva alla meta!
EOF


// ALTNATIVE MISSION
// BOMB DISPOSAL 1-5
// 爆弾解体は各ステージ共通で	//	修正	2002/08.21	H.Yoshiike 
// ライデン＆スネーク
ALT_BMB_1: << EOF
Disinnesca tutte le bombe!
Il suono emesso dalle bombe pu&`o farti
capire dove sono state piazzate.
EOF

// プリスキン＆タキシードスネーク
ALT_BMB_2: << EOF
Disinnesca tutte le bombe!
Le bombe esplodono automaticamente
se ti trovi entro un raggio di 2 metri.
Fai attenzione!
EOF

// 忍者＆前作スネーク
ALT_BMB_3: << EOF
Disinnesca tutte le bombe!
Usa entrambi i sensori A e B e
tieni d'occhio tutti i posti!
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
Metti k.o. tutti i nemici e arriva alla meta!
Puoi tramortirli o dargli
sedativi invece di ucciderli.
EOF

// a01a
ALT_ELM_a01a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
EOF

// a19a
ALT_ELM_a19a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
EOF

// a20a
ALT_ELM_a20a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
EOF

// a24d
ALT_ELM_a24d: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
EOF

// a24a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a24a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
Lo schioppo nemico &`e potente.
Fai molta attenzione!
EOF

// a31a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a31a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
EOF

// a22a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a22a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
EOF

// a42a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a42a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
EOF

// a02a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a02a: << EOF
Metti k.o. tutti i nemici e arriva alla meta!
Ci&`o che vedi non &`e sempre tutto 
ci&`o che c'&`e.
EOF


// HOLD UP 1-10
// a15b
ALT_HLD_a15b: << EOF
Minaccia tutti i nemici e arriva alla meta!
La missione termina se ti scoprono. 
Puoi svegliare i nemici addormentati 
con lo spray refrigerante.
EOF

// a12a
ALT_HLD_a12a: << EOF
Minaccia tutti i nemici e arriva alla meta!
La missione termina se ti scoprono.
EOF

// a24d
ALT_HLD_a24d: << EOF
Minaccia tutti i nemici e arriva alla meta!
La missione termina se ti scoprono.
EOF

// a13b
ALT_HLD_a13b: << EOF
Minaccia tutti i nemici e arriva alla meta!
La missione termina se ti scoprono. 
Attenzione ai soldati che chiamano 
regolarmente!
EOF

// a14a
ALT_HLD_a14a: << EOF
Minaccia tutti i nemici e arriva alla meta!
La missione termina se ti scoprono.
EOF

// a22a
ALT_HLD_a22a: << EOF
Minaccia tutti i nemici e arriva alla meta!
La missione termina se ti scoprono.
EOF

// a01b
ALT_HLD_a01b: << EOF
Minaccia tutti i nemici e arriva alla meta!
La missione termina se ti scoprono.
EOF

// a42a
ALT_HLD_a42a: << EOF
Minaccia tutti i nemici e arriva alla meta!
La missione termina se ti scoprono.
EOF

// a20b
ALT_HLD_a20b: << EOF
Minaccia tutti i nemici e arriva alla meta!
La missione termina se ti scoprono.
EOF

// a31a
ALT_HLD_a31a: << EOF
Minaccia tutti i nemici e arriva alla meta!
La missione termina se ti scoprono.
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
Scatta una foto 
come mostrato!
EOF

// a01b
ALT_PHT_a01b_RAI: << EOF
Scatta una foto 
come mostrato!
EOF

// a01f
ALT_PHT_a01f_RAI: << EOF
Scatta una foto al
"!" visualizzato.
EOF

// a00c
ALT_PHT_a00c_RAI: << EOF
Scatta una foto alle stelle!
EOF

// a00a
ALT_PHT_a00a_RAI: << EOF
Corre voce che un soldato fantasma
si aggiri sul ponte.
Trova il fantasma e scattagli una foto!
EOF

// a03a
ALT_PHT_a03a_RAI: << EOF
Scatta una foto al
fantasma! I fantasmi
odiano i posti luminosi.
EOF


// PHOTOGRAPH SNAKE 1-5
// a01a
ALT_PHT_a01a_SNA_A: << EOF
Scatta una foto al
poster come mostrato!
EOF

ALT_PHT_a01a_SNA_B1: << EOF
Trova la foto della ragazza che va
a genio allo sviluppatore!
Yutaka Negishi (34 anni, coniugato)
Titoli a cui ha lavorato: Metal Gear Solid 2
(direttore artistico sfondi)
EOF

ALT_PHT_a01a_SNA_B2: << EOF
Trova la foto della ragazza che va
a genio allo sviluppatore!
Masahiro Hinami (36 anni, coniugato)
Titolo rappresentativo: Ring of Red
EOF

ALT_PHT_a01a_SNA_B3: << EOF
Trova la foto della ragazza che va
a genio allo sviluppatore!
Yoshikazu Matsuhana (padre di 2 bimbi)
Titoli a cui ha lavorato: Policenauts, 
Snatcher, serie Metal Gear Solid
EOF

ALT_PHT_a01a_SNA_B4: << EOF
Trova la foto della ragazza che va
a genio allo sviluppatore!
Shinta Nojiri (31 anni, pi&`u o meno single)
Titoli a cui ha lavorato: Metal Gear (GB),
Metal Gear Solid 2 (unit&`a Script)
EOF

ALT_PHT_a01a_SNA_B5: << EOF
Trova la foto della ragazza che va
a genio allo sviluppatore!
Sadaaki Kaneyoshi (29 anni, single)
Titoli a cui ha lavorato: serie GuitarFreaks,
Metal Gear Solid 2 (unit&`a Script)
EOF

ALT_PHT_a01a_SNA_B6: << EOF
Trova la foto della ragazza che va
a genio allo sviluppatore!
Hideo Kojima
EOF

// a24d
ALT_PHT_a24d_SNA: << EOF
Scatta una foto
come mostrato!
EOF

// a24a
ALT_PHT_a24a_SNA: << EOF
Scatta una foto al
poster con la stessa
posa della sagoma.
EOF

// a02a
ALT_PHT_a02a_SNA: << EOF
Scatta una foto con
la stessa posa della
sagoma.
EOF

// a41a
ALT_PHT_a41a_SNA: << EOF
"Assassinio in cella di prigione"
Raiden &`e stato ucciso.
Scatta una foto al killer!
EOF

// a24c
ALT_PHT_a24c_SNA: << EOF
"Omicidio in serie"
I soldati sono stati uccisi.
Scatta una foto al killer!
EOF


}


#else

print 'vr_window_e.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'


#endif
