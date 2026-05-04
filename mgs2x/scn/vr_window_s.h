/*
	vr_window_s.h
	    ＶＲウインドウのリソース(スペイン語版)

	2002/05/21 H.Yoshiike
	$Id: vr_window_s.h,v 1.22 2002/10/07 08:13:18 usr03202 Exp $


*/


#ifndef d:VR_WINDOW_S
#define	VR_WINDOW_S	1

resource	VRMSG_S	{
//------------------------
// 共通部分のリソース
//------------------------
時間: { L&'IMITE DE TIEMPO:}
分: {Min}
秒: {Seg}
敵兵数: { ENEMIGOS  :}
人: { 　 }
標的数: { BLANCOS   :}
爆弾数: { BOMBAS    :}
個: { 　 }

//------------------------
// ステージ固有のリソース
//------------------------
// VR MISSION
// SNEAKING MODE-SNEAKING 1-10
// vs01a
VRM_SNK_SNK_vs01a: << EOF
&~!Llega hasta tu objetivo sin ser 
descubierto! Ser&'a m&'as f&'acil si te pegas
contra la pared.
EOF

// vs02a
VRM_SNK_SNK_vs02a: << EOF
&~!Llega hasta tu objetivo sin ser
descubierto! El enemigo siempre patrulla
la misma zona.
EOF

// vs03a
VRM_SNK_SNK_vs03a: << EOF
&~!Llega hasta tu objetivo sin ser
descubierto! Adivinar el comportamiento
del enemigo y el uso de INTRUSI&'ON
son la clave del &'exito.
EOF

// vs04a
VRM_SNK_SNK_vs04a: << EOF
&~!Llega hasta tu objetivo sin ser
descubierto! Atrae al enemigo
para conseguir una ruta.
EOF

// vs05a
VRM_SNK_SNK_vs05a: << EOF
&~!Llega hasta tu objetivo sin ser
descubierto! Utiliza modo COLGADO 
y pasa por la l&'inea del enemigo.
EOF

// vs06a
VRM_SNK_SNK_vs06a: << EOF
&~!Llega al objetivo!
Cuidado con los suelos que hacen ruido y
los que est&'an mojados, porque dejar&'as
huellas. Si te descubren o te caes en un
agujero, la misi&'on habr&'a terminado.
EOF

// vs07a
VRM_SNK_SNK_vs07a: << EOF
&~!Utiliza bien las taquillas y llega al 
objetivo! La misi&'on se terminar&'a si te 
descubren. La seguridad ser&'a mayor
cuando el enemigo se despierte.
EOF

// vs08a
VRM_SNK_SNK_vs08a: << EOF
Llega hasta el objetivo sin ser descubierto
por el enemigo o la c&'amara de seguridad.
La misi&'on se terminar&'a si te descubren. 
Justo debajo de la c&'amara, 
hay un punto ciego.
EOF

// vs09a
VRM_SNK_SNK_vs09a: << EOF
&~!Utiliza bien los objetos para llegar
al objetivo! La misi&'on se terminar&'a 
si te descubren.
EOF

// vs10a
VRM_SNK_SNK_vs10a: << EOF
&~!Utiliza lo que has aprendido hasta 
ahora para llegar hasta el objetivo 
sin ser descubierto!
EOF


// SNEAKING MODE-ELIMINATE ALL 1-10
// vs01a
VRM_SNK_ELM_vs01a: << EOF
&~!Acaba con el enemigo y llega al objetivo!
Adem&'as de matarle, podr&'as dejarle 
inconsciente o inyectarle un tranquilizante.
EOF

// vs02a
VRM_SNK_ELM_vs02a: << EOF
&~!Acaba con todos los enemigos y llega 
al objetivo! Si quieres disparar con 
mayor precisi&'on, pasa a la c&'amara 
Visi&'on Primera Persona y apunta bien.
EOF

// vs03a
VRM_SNK_ELM_vs03a: << EOF
&~!Acaba con todos los enemigos y llega 
al objetivo! Sus puntos d&'ebiles son la 
cabeza, el coraz&'on y las ingles. Si les 
das en sus puntos d&'ebiles, acabar&'as 
con ellos de un solo disparo.
EOF

// vs04a
VRM_SNK_ELM_vs04a: << EOF
&~!Acaba con todos los enemigos y llega 
al objetivo! Utiliza un objeto, da un golpe 
a la pared o dispara para llamar la 
atenci&'on del enemigo.
EOF

// vs05a
VRM_SNK_ELM_vs05a: << EOF
&~!Acaba con todos los enemigos y llega 
al objetivo! Acaba con ellos saltando 
hacia arriba desde COLGADO o 
saltando encima de ellos.
EOF

// vs06a
VRM_SNK_ELM_vs06a: << EOF
&~!Acaba con todos los enemigos y llega 
al objetivo! Cuidado con los suelos que 
hacen ruido y los que est&'an mojados. 
La misi&'on se terminar&'a si te caes 
en un agujero.
EOF

// vs07a
VRM_SNK_ELM_vs07a: << EOF
Acaba con todos los enemigos que est&'en
en posici&'on y llega al objetivo. Si no lo
consigues, pasar&'as al modo ALERTA,
y aparecer&'a un equipo de ataque.
EOF

// vs08a
VRM_SNK_ELM_vs08a: << EOF
Acaba con todos los enemigos que est&'en
en posici&'on y llega al objetivo. Cuidado
con las c&'amaras de seguridad. La misi&'on 
se terminar&'a si te descubren.
EOF

// vs09a
VRM_SNK_ELM_vs09a: << EOF
Acaba con todos los enemigos que est&'en
en posici&'on y llega al objetivo. Utiliza bien
todos los objetos.
EOF

// vs10a
VRM_SNK_ELM_vs10a: << EOF
Utiliza lo que has aprendido hasta ahora 
para acabar con todos los enemigos en 
posici&'on y llegar hasta el objetivo.
EOF


// WEAPON MODE-HANDGUN 1-5
// wp01a
VRM_WPN_HGN_wp01a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp02a
VRM_WPN_HGN_wp02a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp03a
VRM_WPN_HGN_wp03a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo! Los blancos naranjas 
explotan cuando son destruidos. Util&'izalos 
para destruir blancos que haya cerca.
EOF

// wp04a
VRM_WPN_HGN_wp04a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo! Haz buen uso del bot&'on de 
bloquear para disparar mientras te mueves.
Si destruyes los obst&'aculos de resta 
(blancos rojos), tu puntuaci&'on disminuir&'a.
EOF

// wp05a
VRM_WPN_HGN_wp05a: << EOF
&~!Utiliza lo que has aprendido hasta 
ahora para destruir todos los blancos y 
llegar hasta el objetivo!
EOF


// WEAPON MODE-ASSAULT RIFLE 1-5
// wp11a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp11a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp12a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp12a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp13a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp13a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo! Si destruyes los obst&'aculos 
de resta (blancos rojos), tu puntuaci&'on 
disminuir&'a.
EOF

// wp14a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp14a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp15a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_ASR_wp15a: << EOF
&~!Utiliza lo que has aprendido hasta 
ahora para destruir todos los blancos y 
llegar hasta el objetivo!
EOF


// WEAPON MODE-C4/CLAYMORE 1-5
// wp21a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp21a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo! &~!Utiliza las armas que sean 
necesarias!
EOF

// wp22a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp22a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo! Los blancos naranjas 
explotan cuando son destruidos. Cuidado 
que no te pille la explosi&'on.
EOF

// wp23a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp23a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp24a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp24a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp25a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_C4C_wp25a: << EOF
&~!Utiliza lo que has aprendido hasta 
ahora para destruir todos los blancos y 
llegar hasta el objetivo!
EOF


// WEAPON MODE-GRENADE 1-5
// wp31a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp31a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp32a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp32a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp33a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp33a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp34a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp34a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp35a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_GRN_wp35a: << EOF
&~!Utiliza lo que has aprendido hasta 
ahora para destruir todos los blancos y 
llegar hasta el objetivo!
EOF


// WEAPON MODE-PSG1 1-5
// wp41a
VRM_WPN_PSG_wp41a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo! Busca un buen lugar para 
disparar. Cuando tengas un arma, 
no te podr&'as mover. &~!Ten cuidado!
EOF

// wp42a
VRM_WPN_PSG_wp42a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp43a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp43a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo! Los blancos naranjas 
explotan cuando son destruidos. Util&'izalos
para destruir blancos que haya cerca.
EOF

// wp44a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp44a: << EOF
&~!Destruye todos los blancos y ve hacia el objetivo!
Si destruyes los obst&'aculos de resta 
(blancos rojos), tu puntuaci&'on disminuir&'a.
EOF

// wp45a	// 修正 S.Kaneyoshi 2002.08.21
VRM_WPN_PSG_wp45a: << EOF
&~!Utiliza lo que has aprendido hasta 
ahora para destruir todos los blancos y 
llegar hasta el objetivo!
EOF


// WEAPON MODE-STINGER 1-5
// wp51a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp51a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo! Cuando tengas un arma, 
no te podr&'as mover. &~!Ten cuidado!
Ac&'ercate m&'as al blanco si no 
consigues darle.
EOF

// wp52a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp52a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp53a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp53a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp54a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp54a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp55a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_STG_wp55a: << EOF
&~!Utiliza lo que has aprendido hasta 
ahora para destruir todos los blancos y 
llegar hasta el objetivo!
EOF


// WEAPON MODE-NIKITA 1-5
// wp61a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp61a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp62a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp62a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp63a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp63a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp64a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp64a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo! Si destruyes los obst&'aculos 
de resta (blancos rojos), tu puntuaci&'on 
disminuir&'a.
EOF

// wp65a	/* 修正:2002/08/21 M.Uehara */
VRM_WPN_NKT_wp65a: << EOF
&~!Utiliza lo que has aprendido hasta 
ahora para destruir el blanco y llegar 
hasta el objetivo!
EOF


// WEAPON MODE-HF.BLADE/NO WEAPON 1-5
// wp71a
VRM_WPN_HFB_wp71a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp72a
VRM_WPN_HFB_wp72a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// wp73a
VRM_WPN_HFB_wp73a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo! &~!Aleja los blancos que 
est&'an cerca d&'andoles una patada o 
un salto mortal!
EOF

// wp74a
VRM_WPN_HFB_wp74a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo! Si destruyes los obst&'aculos 
de resta (blancos rojos), tu puntuaci&'on 
disminuir&'a.
EOF

// wp75a
VRM_WPN_HFB_wp75a: << EOF
&~!Utiliza lo que has aprendido hasta 
ahora para destruir todos los blancos y 
llegar hasta el objetivo!
EOF


// FIRST PERSON VIEW MODE- 1-5
// sp21a	/* 修正:2002/08/21 M.Uehara */
VRM_FPV_sp21a: << EOF
&~!Destruye todos los blancos y ve hacia el
objetivo! &~!Utiliza el arma que necesites!
EOF


// sp22a	//	修正	2002/08.21	H.Yoshiike
VRM_FPV_sp22a: << EOF
&~!Llega vivo hasta el objetivo!
Utiliza bien los objetos.
EOF


// sp23a
VRM_FPV_sp23a: << EOF
&~!Acaba con todos los enemigos y ve 
hacia el objetivo!
EOF


// sp24a	//	修正	2002/08.21	H.Yoshiike
VRM_FPV_sp24a: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo! Cuidado con los guardias 
enemigos.
EOF


// sp25a
VRM_FPV_sp25a: << EOF
&~!Desactiva todas las bombas!
El sonido de las bombas te puede 
ayudar a encontrar a los enemigos.
EOF


// VARIETY MODE- 1-7
// sp01a-1 アスレチックステージ機雷バージョン
VRM_VRT_sp01a_1: << EOF
&~!Utiliza diferentes movimientos y llega 
al objetivo! &~!Cuidado con las minas!
EOF

// sp01a-1 アスレチックステージ的バージョン
VRM_VRT_sp01a_2: << EOF
&~!Destruye todos los blancos y ve hacia 
el objetivo!
EOF

// sp02a
VRM_VRT_sp02a: << EOF
&~!Utiliza el movimiento COLGADO-CAER-
AGARRAR y llega al objetivo!
&~!Pulsa el bot&'on Acci&'on justo cuando 
te est&'es cayendo y te podr&'as volver 
a COLGAR de nuevo!
EOF

// sp03a ザコサバイバル		//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp03a: << EOF
ZAKO SURVIVAL
&~!Acaba con todos los enemigos!
EOF

// sp06a 狙撃ミッション
VRM_VRT_sp06a_1: << EOF
&~!Acaba con todos los enemigos que se 
acerquen! &~!No dejes que se acerquen a 
la persona que est&'a tirada en el suelo!
EOF

VRM_VRT_sp06a_2: << EOF
&~!Acaba con todos los enemigos que se 
acerquen! &~!No dejes que se acerquen 
al curry!
EOF

VRM_VRT_sp06a_3: << EOF
&~!Acaba con todos los enemigos que se 
acerquen! &~!No dejes que se acerquen 
a los tallarines!
EOF

VRM_VRT_sp06a_4: << EOF
&~!Acaba con todos los enemigos que se 
acerquen! &~!No dejes que se acerquen 
a las cajas de cart&'on!
EOF

// sp07a ゴルルゴンステージ					//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp07a: << EOF
&~!Llega hasta el objetivo sin ser descubierto
por la criatura gigante! Obs&'ervala con 
atenci&'on desde la pared, echa un vistazo 
desde la esquina y &~!a por ella!
&~!Utiliza bien la c&'amara en visi&'on esquina!
EOF

// sp08a ダークステージ						//	修正	2002/08.21	H.Yoshiike
VRM_VRT_sp08a: << EOF
&~!Acaba con todos los enemigos que hay 
en la oscuridad y llega hasta el objetivo!
Utiliza bien los objetos.
EOF


// STREAKING MODE- 1
// sp09a
VRM_STR_sp09a: << EOF
誰にも見つからずにゴールを目指せ！
EOF

// STREAKING MODE- 1
// st01a
VRM_STR_st01a: << EOF
&~!Llega hasta el objetivo en 5 niveles 
consecutivos sin ser descubierto!
EOF

VRM_STR_st02a: << EOF
&~!Llega hasta el objetivo sin ser 
descubierto!
EOF

VRM_STR_st03a: << EOF
&~!Llega hasta el objetivo sin ser 
descubierto! Recuerda que el modo 
INTRUSI&'ON es muy &'util.
EOF

VRM_STR_st04a: << EOF
&~!Llega hasta el objetivo sin ser 
descubierto! Hay mucha seguridad.
Sin embargo, hay una forma de llegar.
EOF

VRM_STR_st05a: << EOF
&'Este es el nivel final.
No pierdas concentraci&'on.
&~!Llega hasta el objetivo!
EOF


// ALTNATIVE MISSION
// BOMB DISPOSAL 1-5
// 爆弾解体は各ステージ共通で	//	修正	2002/08.21	H.Yoshiike 
// ライデン＆スネーク
ALT_BMB_1: << EOF
&~!Desactiva las bombas!
El sonido de las bombas te puede 
ayudar a encontrar a los enemigos.
EOF

// プリスキン＆タキシードスネーク
ALT_BMB_2: << EOF
&~!Desactiva las bombas!
Las bombas explotar&'an autom&'aticamente
si te acercas a 2 metros de ella.
&~!Ten cuidado!
EOF

// 忍者＆前作スネーク
ALT_BMB_3: << EOF
&~!Desactiva las bombas!
Utiliza los sensores A y B &~!y mira
bien por todos los lados!
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
&~!Acaba con todos los enemigos y llega 
hasta el objetivo! En vez de matarles, 
podr&'as dejarles inconscientes o
inyectarles un tranquilizante.
EOF

// a01a
ALT_ELM_a01a: << EOF
&~!Acaba con todos los enemigos y llega 
hasta el objetivo!
EOF

// a19a
ALT_ELM_a19a: << EOF
&~!Acaba con todos los enemigos y llega 
hasta el objetivo!
EOF

// a20a
ALT_ELM_a20a: << EOF
&~!Acaba con todos los enemigos y llega 
hasta el objetivo!
EOF

// a24d
ALT_ELM_a24d: << EOF
&~!Acaba con todos los enemigos y llega 
hasta el objetivo!
EOF

// a24a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a24a: << EOF
&~!Acaba con todos los enemigos y llega 
hasta el objetivo!
El disparo del enemigo es muy potente.
&~!Ten mucho cuidado!
EOF

// a31a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a31a: << EOF
&~!Acaba con todos los enemigos y llega 
hasta el objetivo!
EOF

// a22a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a22a: << EOF
&~!Acaba con todos los enemigos y llega 
hasta el objetivo!
EOF

// a42a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a42a: << EOF
&~!Acaba con todos los enemigos y llega 
hasta el objetivo!
EOF

// a02a	/* 修正:2002/08/21 M.Uehara */
ALT_ELM_a02a: << EOF
&~!Acaba con todos los enemigos y llega 
hasta el objetivo!
Lo que ves, no siempre es lo que hay.
EOF


// HOLD UP 1-10
// a15b
ALT_HLD_a15b: << EOF
&~!Ret&'en a todos los enemigos y llega hasta 
el objetivo! La misi&'on se terminar&'a si te 
descubren. Puedes despertar a enemigos 
con el espray refrigerante.
EOF

// a12a
ALT_HLD_a12a: << EOF
&~!Ret&'en a todos los enemigos y llega 
hasta el objetivo! La misi&'on se 
terminar&'a si te descubren.
EOF

// a24d
ALT_HLD_a24d: << EOF
&~!Ret&'en a todos los enemigos y llega 
hasta el objetivo! La misi&'on se 
terminar&'a si te descubren.
EOF

// a13b
ALT_HLD_a13b: << EOF
&~!Ret&'en a todos los enemigos y llega 
hasta el objetivo! La misi&'on se 
terminar&'a si te descubren. &~! Cuidado 
con los soldados que est&'en realizando 
llamadas rutinarias!
EOF

// a14a
ALT_HLD_a14a: << EOF
&~!Ret&'en a todos los enemigos y llega 
hasta el objetivo! La misi&'on se 
terminar&'a si te descubren.
EOF

// a22a
ALT_HLD_a22a: << EOF
&~!Ret&'en a todos los enemigos y llega 
hasta el objetivo! La misi&'on se 
terminar&'a si te descubren.
EOF

// a01b
ALT_HLD_a01b: << EOF
&~!Ret&'en a todos los enemigos y llega 
hasta el objetivo! La misi&'on se 
terminar&'a si te descubren.
EOF

// a42a
ALT_HLD_a42a: << EOF
&~!Ret&'en a todos los enemigos y llega 
hasta el objetivo! La misi&'on se 
terminar&'a si te descubren.
EOF

// a20b
ALT_HLD_a20b: << EOF
&~!Ret&'en a todos los enemigos y llega 
hasta el objetivo! La misi&'on se 
terminar&'a si te descubren.
EOF

// a31a
ALT_HLD_a31a: << EOF
&~!Ret&'en a todos los enemigos y llega 
hasta el objetivo! La misi&'on se 
terminar&'a si te descubren.
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
&~!Saca una foto tal y 
como se indica!
EOF

// a01b
ALT_PHT_a01b_RAI: << EOF
&~!Saca una foto tal y 
como se indica!
EOF

// a01f
ALT_PHT_a01f_RAI: << EOF
Saca una foto del 
s&'imbolo "!".
EOF

// a00c
ALT_PHT_a00c_RAI: << EOF
&~!Saca una foto de las estrellas!
EOF

// a00a
ALT_PHT_a00a_RAI: << EOF
Hay un rumor sobre un soldado 
fantasma que vaga por la cubierta. 
&~!B&'uscale y s&'acale una foto!
EOF

// a03a
ALT_PHT_a03a_RAI: << EOF
&~!S&'acale una foto al 
fantasma! 
Los fantasmas odian 
los lugares muy 
iluminados.
EOF


// PHOTOGRAPH SNAKE 1-5
// a01a
ALT_PHT_a01a_SNA_A: << EOF
&~!S&'acale una foto al 
p&'oster tal y como 
se indica!
EOF

ALT_PHT_a01a_SNA_B1: << EOF
Busca la foto del tipo de chica preferida de.
Yutaka Negishi (34 a&~nos, casado)
Juegos en los que ha trabajado: 
Metal Gear Solid 2
(Director Art&'istico del Escenario de Fondo)
EOF

ALT_PHT_a01a_SNA_B2: << EOF
Busca la foto del tipo de chica preferida de.
Masahiro Hinami (36 a&~nos, casado)
Juego representativo: Ring of Red
EOF

ALT_PHT_a01a_SNA_B3: << EOF
Busca la foto del tipo de chica preferida de.
Yoshikazu Matsuhana (padre de 2 ni&~nos)
Juegos en los que ha trabajado: Snatcher, 
Policenauts, Lethal Enforcers, 
las series de Metal Gear Solid
EOF

ALT_PHT_a01a_SNA_B4: << EOF
Busca la foto del tipo de chica preferida de.
Shinta Nojiri (31 a&~nos, "&~?&~!soltero!?")
Juegos en los que ha trabajado: 
Metal Gear (GB), 
Metal Gear Solid 2 (unidad del gui&'on)
EOF

ALT_PHT_a01a_SNA_B5: << EOF
Busca la foto del tipo de chica preferida de.
Sadaaki Kaneyoshi (29 a&~nos, soltero)
Juegos en los que ha trabajado: 
las series de GuitearFreaks, 
Metal Gear Solid 2 (unidad de gui&'on)
EOF

ALT_PHT_a01a_SNA_B6: << EOF
Busca la foto del tipo de chica preferida de.
Hideo Kojima
EOF

// a24d
ALT_PHT_a24d_SNA: << EOF
&~!Saca una foto tal y 
como se indica!
EOF

// a24a
ALT_PHT_a24a_SNA: << EOF
Saca una foto al 
p&'oster con la misma 
pose que la silueta.
EOF

// a02a
ALT_PHT_a02a_SNA: << EOF
Saca una foto con la 
misma pose que la 
silueta.
EOF

// a41a
ALT_PHT_a41a_SNA: << EOF
"Asesinato en la Celda de una Prisi&'on"
Raiden ha sido asesinado.
&~!Saca una foto a su asesino!
EOF

// a24c
ALT_PHT_a24c_SNA: << EOF
"Asesino en Serie"
Los soldados han sido asesinados.
&~!Saca una foto a su asesino!
EOF


}


#else

print 'vr_window_e.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'


#endif
