/*
	vr_window_f.h
	    ＶＲウインドウのリソース(フランス語版)

	2002/05/21 H.Yoshiike
	$Id: vr_window_f.h,v 1.21 2002/10/07 08:13:01 usr03202 Exp $


*/


#ifndef d:VR_WINDOW_F
#define	VR_WINDOW_F	1

resource	VRMSG_F	{
//------------------------
// 共通部分のリソース
//------------------------
時間: { DELAI     :}
分: {min}
秒: {s}
敵兵数: { ENNEMIS   :}
人: { 　 }
標的数: { CIBLES    :}
爆弾数: { BOMBES    :}
個: { 　 }

//------------------------
// ステージ固有のリソース
//------------------------
// VR MISSION
// SNEAKING MODE-SNEAKING 1-10
// vs01a
VRM_SNK_SNK_vs01a: << EOF
Atteignez l'objectif sans vous faire rep&'erer !
Plaquez-vous contre le mur,
ce sera plus facile.
EOF

// vs02a
VRM_SNK_SNK_vs02a: << EOF
Atteignez l'objectif sans vous faire rep&'erer !
Les patrouilles suivent toujours le m&|eme 
trajet.
EOF

// vs03a
VRM_SNK_SNK_vs03a: << EOF
Atteignez l'objectif sans vous faire rep&'erer !
Anticipez les mouvements de l'ennemi et
servez-vous du mode INTRUSION pour 
y parvenir.
EOF

// vs04a
VRM_SNK_SNK_vs04a: << EOF
Atteignez l'objectif sans vous faire rep&'erer !
Trompez l'ennemi pour pouvoir passer.
EOF

// vs05a
VRM_SNK_SNK_vs05a: << EOF
Atteignez l'objectif sans vous faire rep&'erer !
Servez-vous du mode SUSPENSION pour
franchir la ligne ennemie.
EOF

// vs06a
VRM_SNK_SNK_vs06a: << EOF
Atteignez l'objectif ! Faites attention sur 
le plancher qui craque et ne laissez pas 
d'empreinte sur le sol mouill&'e.
Si on vous d&'ecouvre ou si vous tombez 
dans un trou, la mission sera termin&'ee.
EOF

// vs07a
VRM_SNK_SNK_vs07a: << EOF
Faites bon usage des casiers et atteignez 
l'objectif ! Si on vous d&'ecouvre, 
la mission sera termin&'ee. S'il se r&'eveille, 
l'ennemi renforcera la s&'ecurit&'e.
EOF

// vs08a
VRM_SNK_SNK_vs08a: << EOF
Atteignez l'objectif sans vous faire rep&'erer 
par l'ennemi ou la cam&'era de surveillance.
Si on vous d&'ecouvre, la mission 
sera termin&'ee. Juste au-dessous 
de la cam&'era, on ne vous verra pas.
EOF

// vs09a
VRM_SNK_SNK_vs09a: << EOF
Faites bon usage des objets et atteignez 
l'objectif ! Si on vous d&'ecouvre, 
la mission sera termin&'ee.
EOF

// vs10a
VRM_SNK_SNK_vs10a: << EOF
Faites bon usage de tout ce que vous 
avez appris et atteignez l'objectif sans 
vous faire d&'ecouvrir !
EOF


// SNEAKING MODE-ELIMINATE ALL 1-10
// vs01a
VRM_SNK_ELM_vs01a: << EOF
Eliminez l'ennemi et atteignez l'objectif !
Vous pouvez l'assommer, le droguer et 
bien s&|ur le tuer.
EOF

// vs02a
VRM_SNK_ELM_vs02a: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif ! Pour tirer avec pr&'ecision, 
passez en Observation &`a la premi&`ere 
personne et visez bien.
EOF

// vs03a
VRM_SNK_ELM_vs03a: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif ! Ils ont plusieurs points faibles :
la t&|ete, le c&@eur et l'aine. 
Il suffit de les toucher &`a un de ces 
endroits pour les tuer.
EOF

// vs04a
VRM_SNK_ELM_vs04a: << EOF
Eliminez tous les ennemis et atteignez l'objectif !
Servez-vous d'un objet, donnez des coups dans le mur 
ou tirez pr&`es de l'ennemi pour attirer son attention.
EOF

// vs05a
VRM_SNK_ELM_vs05a: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif ! Par exemple, vous pouvez 
leur sauter dessus ou retomber sur eux 
en mode SUSPENSION.
EOF

// vs06a
VRM_SNK_ELM_vs06a: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif ! Faites attention sur le plancher
qui craque et ne laissez pas d'empreinte 
sur le sol mouill&'e. Si vous tombez dans 
un trou, la mission sera termin&'ee.
EOF

// vs07a
VRM_SNK_ELM_vs07a: << EOF
Eliminez les ennemis post&'es et atteignez 
l'objectif. Si votre tentative &'echoue, 
le mode ALERTE se d&'eclenchera 
et les renforts arriveront.
EOF

// vs08a
VRM_SNK_ELM_vs08a: << EOF
Eliminez les ennemis post&'es et atteignez 
l'objectif. Faites attention &`a la cam&'era 
de surveillance. Si on vous d&'ecouvre, 
la mission sera termin&'ee.
EOF

// vs09a
VRM_SNK_ELM_vs09a: << EOF
Eliminez les ennemis post&'es et atteignez 
l'objectif. Faites bon usage des objets.
EOF

// vs10a
VRM_SNK_ELM_vs10a: << EOF
Servez-vous de tout ce que vous avez 
appris jusqu'ici pour &'eliminer les ennemis 
post&'es et atteindre l'objectif.
EOF


// WEAPON MODE-HANDGUN 1-5
// wp01a
VRM_WPN_HGN_wp01a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp02a
VRM_WPN_HGN_wp02a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp03a
VRM_WPN_HGN_wp03a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif ! Les cibles oranges explosent
quand on les d&'etruit. Servez-vous en
pour d&'etruire les cibles proches.
EOF

// wp04a
VRM_WPN_HGN_wp04a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif ! Si vous d&'etruisez les 
obstacles de d&'eduction (cibles rouges),
votre score baissera.
EOF

// wp05a
VRM_WPN_HGN_wp05a: << EOF
Servez-vous de tout ce que vous avez 
appris pour d&'etruire toutes les cibles 
et atteindre l'objectif !
EOF


// WEAPON MODE-ASSAULT RIFLE 1-5
// wp11a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp11a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp12a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp12a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp13a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp13a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif ! Si vous d&'etruisez les 
obstacles de d&'eduction (cibles rouges), 
votre score baissera.
EOF

// wp14a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp14a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp15a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp15a: << EOF
Servez-vous de tout ce que vous avez 
appris pour d&'etruire toutes les cibles 
et atteindre l'objectif !
EOF


// WEAPON MODE-C4/CLAYMORE 1-5
// wp21a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp21a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif ! Faites bon usage 
des diff&'erentes armes !
EOF

// wp22a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp22a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif ! Les cibles oranges explosent
quand on les d&'etruit. Ne vous laissez 
pas prendre dans l'explosion !
EOF

// wp23a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp23a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp24a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp24a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp25a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp25a: << EOF
Servez-vous de tout ce que vous avez 
appris pour d&'etruire toutes les cibles 
et atteindre l'objectif !
EOF


// WEAPON MODE-GRENADE 1-5
// wp31a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp31a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp32a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp32a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp33a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp33a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp34a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp34a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp35a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp35a: << EOF
Servez-vous de tout ce que vous avez 
appris pour d&'etruire toutes les cibles 
et atteindre l'objectif !
EOF


// WEAPON MODE-PSG1 1-5
// wp41a
VRM_WPN_PSG_wp41a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif ! Trouvez le meilleur endroit 
pour tirer. Vous ne pouvez pas vous 
d&'eplacer sans arme. Attention !
EOF

// wp42a
VRM_WPN_PSG_wp42a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp43a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp43a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif ! Les cibles oranges explosent
quand on les d&'etruit. Servez-vous en
pour d&'etruire les cibles proches.
EOF

// wp44a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp44a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif ! Si vous d&'etruisez les 
obstacles de d&'eduction (cibles rouges), 
votre score baissera.
EOF

// wp45a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp45a: << EOF
Servez-vous de tout ce que vous avez 
appris pour d&'etruire toutes les cibles 
et atteindre l'objectif !
EOF


// WEAPON MODE-STINGER 1-5
// wp51a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp51a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif ! Vous ne pouvez pas vous 
d&'eplacer sans arme. Attention !
Essayez de vous rapprocher des cibles 
si vous avez du mal &`a les atteindre.
EOF

// wp52a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp52a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp53a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp53a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp54a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp54a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp55a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp55a: << EOF
Servez-vous de tout ce que vous avez 
appris pour d&'etruire toutes les cibles 
et atteindre l'objectif !
EOF


// WEAPON MODE-NIKITA 1-5
// wp61a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp61a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp62a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp62a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp63a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp63a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp64a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp64a: << EOF
D&'etruisez la cible et atteignez l'objectif !
Si vous d&'etruisez les obstacles 
de d&'eduction (cibles rouges), 
votre score baissera.
EOF

// wp65a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp65a: << EOF
Servez-vous de tout ce que vous avez 
appris pour d&'etruire toutes les cibles 
et atteindre l'objectif !
EOF


// WEAPON MODE-HF.BLADE/NO WEAPON 1-5
// wp71a
VRM_WPN_HFB_wp71a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp72a
VRM_WPN_HFB_wp72a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// wp73a
VRM_WPN_HFB_wp73a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif ! Propulsez les cibles proches 
au loin d'un coup ou d'un saut !
EOF

// wp74a
VRM_WPN_HFB_wp74a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif ! Si vous d&'etruisez les 
obstacles de d&'eduction (cibles rouges), 
votre score baissera.
EOF

// wp75a
VRM_WPN_HFB_wp75a: << EOF
Servez-vous de tout ce que vous avez 
appris pour d&'etruire toutes les cibles 
et atteindre l'objectif !
EOF


// FIRST PERSON VIEW MODE- 1-5
// sp21a	/* 修正:2002/08/21 M.Uehara */
VRM_FPV_sp21a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif ! Faites bon usage 
des diff&'erentes armes !
EOF


// sp22a	//	修正	2002/08.21	H.Yoshiike
VRM_FPV_sp22a: << EOF
Atteignez l'objectif vivant !
Faites bon usage des objets.
EOF


// sp23a
VRM_FPV_sp23a: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif !
EOF


// sp24a	//	修正	2002/08.21	H.Yoshiike
VRM_FPV_sp24a: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif ! Faites attention aux 
gardes ennemis.
EOF


// sp25a
VRM_FPV_sp25a: << EOF
D&'esamorcez toutes les bombes ! Le bruit
que les bombes font peut vous donner 
une indication de leur emplacement.
EOF


// VARIETY MODE- 1-7
// sp01a-1 アスレチックステージ機雷バージョン
VRM_VRT_sp01a_1: << EOF
Utilisez diff&'erents mouvements 
et atteignez l'objectif !
Faites attention aux mines !
EOF

// sp01a-1 アスレチックステージ的バージョン
VRM_VRT_sp01a_2: << EOF
D&'etruisez toutes les cibles et atteignez 
l'objectif !
EOF

// sp02a
VRM_VRT_sp02a: << EOF
Servez-vous du mouvement SUSPENSION-
LAISSER TOMBER-ATTRAPER et 
atteignez l'objectif. Appuyez sur la touche
Action au bon moment quand vous tombez
et vous pourrez vous SUSPENDRE encore.
EOF

// sp03a ザコサバイバル		//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp03a: << EOF
ZAKO SURVIVAL
Eliminez tous les ennemis !
EOF

// sp06a 狙撃ミッション
VRM_VRT_sp06a_1: << EOF
Eliminez tous les ennemis qui 
approchent ! Ne les laissez pas 
s'approcher de la personne 
&'etendue au sol !
EOF

VRM_VRT_sp06a_2: << EOF
Eliminez tous les ennemis qui 
approchent ! Ne laissez pas 
l'ennemi s'approcher du curry !
EOF

VRM_VRT_sp06a_3: << EOF
Eliminez tous les ennemis qui 
approchent ! Ne laissez pas 
l'ennemi s'approcher des p&|ates !
EOF

VRM_VRT_sp06a_4: << EOF
Eliminez tous les ennemis qui 
approchent ! Ne laissez pas 
l'ennemi s'approcher du carton !
EOF

// sp07a ゴルルゴンステージ					//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp07a: << EOF
Atteignez l'objectif sans vous faire 
rep&'erer par la gigantesque cr&'eature ! 
Observez-la soigneusement en vous 
tenant contre le mur, en jetant un coup 
d'&@eil apr&`es le coin et parvenez &`a vos fins !


Atteignez l'objectif sans vous faire 
rep&'erer par la cr&'eature gigantesque !
Gardez l'&@eil sur elle et avancez !
EOF

// sp08a ダークステージ						//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp08a: << EOF
Eliminez tous les ennemis dans le noir
et atteignez l'objectif !
Faites bon usage des objets.
EOF


// STREAKING MODE- 1
// sp09a
VRM_STR_sp09a: << EOF
誰にも見つからずにゴールを目指せ！
EOF

// STREAKING MODE- 1
// st01a
VRM_STR_st01a: << EOF
Atteignez l'objectif en 5 &'etapes 
cons&'ecutives sans vous faire rep&'erer !
EOF

VRM_STR_st02a: << EOF
Atteignez l'objectif sans vous 
faire rep&'erer !
EOF

VRM_STR_st03a: << EOF
Atteignez l'objectif sans vous faire 
rep&'erer ! Pensez au mode INTRUSION. 
C'est efficace !
EOF

VRM_STR_st04a: << EOF
Atteignez l'objectif sans vous faire 
rep&'erer ! La s&'ecurit&'e est serr&'ee.
Mais il y a une issue.
EOF

VRM_STR_st05a: << EOF
Voici la derni&`ere &'etape.
Ne vous d&'econcentrez pas.
Atteignez l'objectif !
EOF


// ALTNATIVE MISSION
// BOMB DISPOSAL 1-5
// 爆弾解体は各ステージ共通で	//	修正	2002/08.21	H.Yoshiike 
// ライデン＆スネーク
ALT_BMB_1: << EOF
D&'esamorcez toutes les bombes ! Le bruit
que les bombes font peut vous donner 
une indication de leur emplacement.
EOF

// プリスキン＆タキシードスネーク
ALT_BMB_2: << EOF
D&'esamorcez toutes les bombes ! Les 
bombes exploseront automatiquement si
vous entrez dans un rayon de 2 m&`etres.
Faites attention !
EOF

// 忍者＆前作スネーク
ALT_BMB_3: << EOF
D&'esamorcez toutes les bombes !
Servez-vous des d&'etecteurs A et B et
regardez bien partout !
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
Eliminez tous les ennemis et atteignez 
l'objectif ! Vous pouvez les assommer 
ou leur administrer du tranquillisant 
au lieu de les tuer.
EOF

// a01a
ALT_ELM_a01a: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif !
EOF

// a19a
ALT_ELM_a19a: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif !
EOF

// a20a
ALT_ELM_a20a: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif !
EOF

// a24d
ALT_ELM_a24d: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif !
EOF

// a24a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a24a: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif ! Le fusil de l'ennemi est 
puissant. Faites tr&`es attention !
EOF

// a31a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a31a: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif !
EOF

// a22a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a22a: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif !
EOF

// a42a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a42a: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif !
EOF

// a02a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a02a: << EOF
Eliminez tous les ennemis et atteignez 
l'objectif ! Il y a parfois plus que 
ce que vous voyez.
EOF


// HOLD UP 1-10
// a15b
ALT_HLD_a15b: << EOF
Braquez tous les ennemis et atteignez 
l'objectif ! Si on vous d&'ecouvre, 
la mission sera termin&'ee. 
Vous pouvez r&'eveiller les ennemis 
endormis avec le r&'efrig&'erant.
EOF

// a12a
ALT_HLD_a12a: << EOF
Braquez tous les ennemis et atteignez 
l'objectif ! Si on vous d&'ecouvre, 
la mission sera termin&'ee.
EOF

// a24d
ALT_HLD_a24d: << EOF
Braquez tous les ennemis et atteignez 
l'objectif ! Si on vous d&'ecouvre, 
la mission sera termin&'ee.
EOF

// a13b
ALT_HLD_a13b: << EOF
Braquez tous les ennemis et atteignez 
l'objectif ! Si on vous d&'ecouvre, 
la mission sera termin&'ee. 
Faites attention aux soldats qui passent 
des appels r&'eguliers !
EOF

// a14a
ALT_HLD_a14a: << EOF
Braquez tous les ennemis et atteignez 
l'objectif ! Si on vous d&'ecouvre, 
la mission sera termin&'ee.
EOF

// a22a
ALT_HLD_a22a: << EOF
Braquez tous les ennemis et atteignez 
l'objectif ! Si on vous d&'ecouvre, 
la mission sera termin&'ee.
EOF

// a01b
ALT_HLD_a01b: << EOF
Braquez tous les ennemis et atteignez 
l'objectif ! Si on vous d&'ecouvre, 
la mission sera termin&'ee.
EOF

// a42a
ALT_HLD_a42a: << EOF
Braquez tous les ennemis et atteignez 
l'objectif ! Si on vous d&'ecouvre, 
la mission sera termin&'ee.
EOF

// a20b
ALT_HLD_a20b: << EOF
Braquez tous les ennemis et atteignez 
l'objectif ! Si on vous d&'ecouvre, 
la mission sera termin&'ee.
EOF

// a31a
ALT_HLD_a31a: << EOF
Braquez tous les ennemis et atteignez 
l'objectif ! Si on vous d&'ecouvre, 
la mission sera termin&'ee.
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
Prenez une photo 
comme indiqu&'e !
EOF

// a01b
ALT_PHT_a01b_RAI: << EOF
Prenez une photo 
comme indiqu&'e !
EOF

// a01f
ALT_PHT_a01f_RAI: << EOF
Prenez une photo du
"!" affich&'e.
EOF

// a00c
ALT_PHT_a00c_RAI: << EOF
Prenez une photo des &'etoiles !
EOF

// a00a
ALT_PHT_a00a_RAI: << EOF
Il para&|it que le fant&|ome d'un soldat 
erre sur le pont.
Trouvez-le et prenez-le en photo !
EOF

// a03a
ALT_PHT_a03a_RAI: << EOF
Prenez une photo du 
fant&|ome ! 
Les fant&|omes 
d&'etestent la lumi&`ere.
EOF


// PHOTOGRAPH SNAKE 1-5
// a01a
ALT_PHT_a01a_SNA_A: << EOF
Prenez une photo 
de l'affiche comme 
indiqu&'e !
EOF

ALT_PHT_a01a_SNA_B1: << EOF
Trouvez la photographie de la fille 
du type du d&'eveloppeur !
Yutaka Negishi (34 ans, mari&'e)
A travaill&'e sur les titres suivants : 
Metal Gear Solid 2 (directeur artistique)
EOF

ALT_PHT_a01a_SNA_B2: << EOF
Trouvez la photographie de la fille 
du type du d&'eveloppeur !
Masahiro Hinami (36 ans, mari&'e)
Titre repr&'esentatif : Ring of Red
EOF

ALT_PHT_a01a_SNA_B3: << EOF
Trouvez la photographie de la fille 
du type du d&'eveloppeur !
Yoshikazu Matsuhana (34 ans, mari&'e)
A travaill&'e sur les titres suivants : 
Policenauts, Metal Gear Solid series
EOF

ALT_PHT_a01a_SNA_B4: << EOF
Trouvez la photographie de la fille 
du type du d&'eveloppeur !
Shinta Nojiri (31 ans, c&'elibataire)
A travaill&'e sur les titres suivants : 
Metal Gear Solid 2 (unit&'e Sc&'enario)
EOF

ALT_PHT_a01a_SNA_B5: << EOF
Trouvez la photographie de la fille 
du type du d&'eveloppeur !
Sadaaki Kaneyoshi (29 ans, c&'elibataire)
A travaill&'e sur les titres suivants : 
Metal Gear Solid 2 (unit&'e Sc&'enario)
EOF

ALT_PHT_a01a_SNA_B6: << EOF
Trouvez la photographie de la fille 
du type du d&'eveloppeur !
Hideo Kojima
EOF

// a24d
ALT_PHT_a24d_SNA: << EOF
Prenez une photo 
comme indiqu&'e !
EOF

// a24a
ALT_PHT_a24a_SNA: << EOF
Prenez une photo de 
l'affiche avec la 
m&|eme pose que 
la silhouette.
EOF

// a02a
ALT_PHT_a02a_SNA: << EOF
Prenez une photo avec 
la m&|eme pose que la 
silhouette.
EOF

// a41a
ALT_PHT_a41a_SNA: << EOF
&~< Meurtre dans la cellule de la prison &~>
Raiden a &'et&'e tu&'e.
Prenez une photo du tueur !
EOF

// a24c
ALT_PHT_a24c_SNA: << EOF
&~< Meurtres en s&'erie &~>
Des soldats ont &'et&'e tu&'es.
Prenez une photo du tueur !
EOF


}


#else

print 'vr_window_e.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'


#endif
