/*
	basic_actions_f.rsc
	    おもしろムービー用フランス語説明テキスト

	2000/11/10 H.Yoshiike
	$Id: basic_actions_f.rsc,v 1.14 2002/10/21 05:06:39 usr03202 Exp $


*/

/* ページ数が2ページにわたるもの
	初級１:
	初級３:
	初級５:
	初級６:
	初級８:

	中級１:
	中級２:
	中級４:
	中級８:

	上級２:
	上級４:
	上級５:
	上級６:
*/

resource 題名_f {
	初級１:
		'Attaque en Observation &`a la premi&`ere personne'

	初級１_1:
		{
			Attaque en Observation
			&`a la premi&`ere personne
		}

//		'主観攻撃'
	初級２:
		'Lancer'
//		'投げ'
	初級３:
		'Vue en coin'
//		'ビハインドカメラ'
	初級４:	
		'Escalader des objets'
//		'台登り'
	初級５:
		'Faire une roulade avant'
//		'ローリング'
	初級６:
		'Tra&^iner des ennemis'
//		'敵兵引きずり'
	初級７:
		'Suspension'
//		'エルード'
	初級８:
		'Ouvrir / Refermer un casier'
//		'ロッカー開閉'
	中級１:
		'Bo&^ite en carton'
//		'ダンボール'
	中級２:
		'Etranglement'
//		'首締め'
	中級３:
		'Jeter un coup d\'&@eil derri&`ere le coin'
	中級３_1:
		{
			Jeter un coup d\'&@eil
			derri&`ere le coin
		}
//		'覗き込み'
	中級４:
		'Se d&\'eplacer lat&\'eralement'
//		'サイドステップ'
	中級５:
		'Frapper contre le mur'
//		'壁たたき'
	中級６:
		'Roulade d\'attaque'
//		'ローリング攻撃'
	中級７:
		'Livre'
//		'雑誌'
	中級８:
		'Changement rapide'	
//		'クイックチェンジ'	
	上級１:
		'Tir en plein saut'
//		'飛び出し撃ち'
	上級２:
		'Pointer une arme'
//		'ホールドアップ'
	上級３:
		'Bouclier'
//		'盾'
	上級４:
		'Viser en bougeant'
//		'構え移動'
	上級５:
		'Faire tomber un objet'	
//		'アイテム奪取'	
	上級６:
		'D&\'etruire la radio'
//		'無線機破壊'
	上級７:
		'Extincteur'
//		'消火器'
	上級８:
		'Suspension et retomb&\'ee'
//		'エルード飛び降り'
	};


resource 説明_f {
#ifdef d:XBOX
	初級１説明:
		{
			Visez en Observation &`a la premi&`ere personne 
			et servez-vous du bouton multidirectionnel
			pour viser et tirer &`a des endroits sp&'ecifiques :
			les points faibles de l'ennemi et les endroits
		}
	初級１説明_1:
		{
			&`a des hauteurs variables normalement
			inaccessibles. Vous pouvez aussi donner
			un coup de point en Observation &`a la
			premi&`ere personne.
		}
#endif
#ifdef d:PSX2
	初級１説明:
		{
			Visez en Observation &`a la premi&`ere personne et
			actionnez la touche directionnelle pour tirer sur
			des points pr&'ecis. Vous toucherez les points
			faibles de l'adversaire &`a des hauteurs
		}
	初級１説明_1:
		{
			normalement difficiles &`a atteindre. L'Observation
			&`a la premi&`ere personne sert aussi &`a donner
			des coups de poing. 
		}
#endif
/*			Aim in First Person View and use the
			directional button to aim at and shoot specific
			spots -- enemy weakpoints and spots at 
			various heights you normally cannot shoot.
			You can also punch in First Person View.
*/
//		'銃等を構えて主観視点にすると、方向キーにより'
//		'上下左右、自在に狙いを定める事が可能。'
//		'敵の急所や高低差があって通常では狙えない場所も'
//		'攻撃できる。また主観視点でパンチ等もくりだせる。'

	初級２説明:
#ifdef d:XBOX
		{
			Lorsque vous &^etes sans arme, servez-vous
			du stick analogique gauche et de la touche
			Arme pr&`es d'un ennemi pour le propulser. 
			Cela a parfois pour effet de l'assommer.
		}
#endif
#ifdef d:PSX2
		{
			Actionnez le joystick analogique gauche et la
			touche Arme, pr&`es d'un ennemi (sans arme),
			pour lancer ce dernier contre le sol. Vous
			pourrez parfois lui faire perdre connaissance.
		}
#endif
/*			Use left analog stick and □ button near
			enemy (w/o weapon) to throw enemy.
			You can make enemy go unconscious at times.
*/
// 体験版と同じ
//		'敵の近くで、武器を装備せずに左スティックを'
//		'入れながら＃｛□、武器｝＃ボタンを押すと、敵を投げることが'
//		'できる。敵を気絶させることも可能。'

	初級３説明:
	//ビハインドモード
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
#ifdef d:XBOX
		{
			Orientez le stick analogique gauche vers le
			mur pr&`es du coin pour passer en Vue en coin.
			Vous pouvez rester cach&'e et regarder apr&`es
			le coin. Vous pouvez d&'eplacer la cam&'era &`a 
		}
	初級３説明_1:
		{
			l'aide du stick analogique droit en Vue en coin.
		}
#endif
#ifdef d:PSX2
		{
			Faites bouger le joystick analogique gauche en
			direction du mur, pr&`es d'un coin, pour entrer en
			Vue en coin et rester cach&'e(e) tout en inspectant
			les abords du coin. Pour d&'eplacer la cam&'era
		}
	初級３説明_1:
		{
			en Vue en coin, utilisez le joystick analogique 
			droit. 
		}
#endif
/*			Input left analog stick toward wall near
			corner to enter corner view.  You can stay
			hidden and look beyond corner. You can move
			camera with right analog stick in corner view.
*/
//		'曲がり角近くで壁に向かって左スティックを'
//		'倒し続けると、壁に張りついてビハインドカメラに'
//		'なる。物陰に隠れながら先を伺うことができる。'
//		'この間、右スティックでカメラを操作可能。'

	初級４説明:
		{
			Actionnez la touche Action pour escalader
			les objets &`a hauteur de votre taille.
			Vous ne pourrez pas escalader
			les objets trop hauts.
		}
/*			Press  △ button to climb up objects
			with heights close to your waist.
			You cannot climb up heights too high.
*/
//		'腰くらいの高さのものは、＃｛△、アクション｝＃ボタンを押せば'
//		'その上によじ登ることができる。'
//		'高すぎる場合は登る事ができない。'

	初級５説明:
		{
			Actionnez la touche Ramper, pendant la course,
			pour faire une roulade avant, utile pour
			sauter d'un objet &`a l'autre. Maintenez la
			touche Ramper enfonc&'ee,
		}
	初級５説明_1:
		{
			pendant la roulade, pour ramper &`a la fin
			de cette derni&`ere.
		}
/*			Press × button when running to roll forward.
			Effective to jump from one object to another.
			Hold down × button while rolling to start
			crawling at the end of the roll.
*/
// 体験版と同じ
//		'走りながら＃｛×、ホフク｝＃ボタンを押すと、ローリングができる。'
//		'物陰から物陰に素早く移動する場合や一定距離以下の'
//		'穴を飛び越えるのに有効。＃｛×、ホフク｝＃ボタンを押しっぱなしに'
//		'すると、終了後そのままホフクになる。'

	初級６説明:
		{
			Actionnez la touche Arme, pr&`es d'un ennemi
			inconscient ou mort (sans arme), pour vous
			saisir du corps. Maintenez la touche Arme
			enfonc&'ee pour le tra&^iner,
		}
	初級６説明_1:
		{
			man&@euvre utile pour cacher
			le corps et l'emp&^echer d'&^etre trouv&'e.
		}
/*			Press □ button near unconscious / dead 
			enemy (w/o weapon) to hold body.  Hold
			down □ button to drag enemy.  Effective to
			hide body and avoid being found.
*/
// 体験版と同じ
//		'倒れた敵の近くで武器を装備せずに＃｛□、武器｝＃ボタンを押すと'
//		'敵の体を抱える。そのまま＃｛□、武器｝＃ボタンを押しっぱなしに'
//		'すれば、敵を引きずって移動させることができる。'
//		'倒した敵の体を発見されないよう隠す時に有効。'

	初級７説明:
		{
			Actionnez la touche Action, pr&`es d'un garde-fou,
			pour sauter de l'autre c&^ot&'e, vous y suspendre et
			&'eviter l'ennemi. Vous pouvez bouger sur le c&^ot&'e
			et entrer en Observation &`a la premi&`ere personne.
		}
/*			Press △ button near handrail to jump over
			handrail and hang to avoid enemy.  You can 
			move sideways and also enter First Person
			View.
*/
// 体験版と同じ
//		'乗り越えられる手すりの近くで＃｛△、アクション｝＃ボタンを押すと、'
//		'手すりを乗り越えて向こう側にぶら下がることが'
//		'できる。うまく使えば敵をやり過ごすことも可能。'
//		'左右に移動したり主観にすることもできる。'

	初級８説明:
		{
			Placez-vous devant un casier et actionnez
			la touche Action pour l'ouvrir. Actionnez de
			nouveau la touche Action, devant le casier
			ouvert, pour le refermer.
		}
	初級８説明_1:
		{
			Le casier peut &^etre ouvert m&^eme
			si vous vous appuyez contre lui.
		}
/*			Stand in front of locker & press △ button
			to open locker.  Press △ button again in
			front of open locker to close it. You can open
			locker even when standing against it.
*/
//		'ロッカーは扉の前に立って＃｛△、アクション｝＃ボタンを押すと扉を'
//		'開けることができる。開いたロッカーの前で'
//		'もう一度＃｛△、アクション｝＃ボタンを押せば扉を閉める。'
//		'ロッカーに張付いた状態からでも可能。'



	中級１説明:
		{
			Si vous vous cachez sous une bo&^ite en carton,
			l'ennemi vous ignorera. Si vous bougez en &'etant
			dans son p&'erim&`etre de vision, vous serez
			rep&'er&'e(e). Si votre bo&^ite bloque
		}
	中級１説明_1:
		{
			le chemin de l'ennemi, ce dernier
			vous d&'ebusquera.
		}
/*			If you hide under a cardboard box and stay
			still, the enemy will ignore you. If you move
			when in his sight, you will be found. Blocking 
			enemy paths when in the box will make 
			the enemy discover you.
*/
//		'通路の端などでダンボールをかぶって動かなければ、'
//		'敵の視界に入っても怪しまれない。ただし視界内で'
//		'少しでも動くと見つかる。また隠れ場所が敵の移動の'
//		'邪魔になる場合にも怪しまれるので注意が必要。'

	中級２説明:
#ifdef d:XBOX
		{
			Lorsque vous &^etes sans arme, approchez-vous
			de l'ennemi par derri&`ere et appuyez sur la
			touche Arme, sans vous servir du stick
			analogique gauche. Appuyez sur la touche 
		}
	中級２説明_1:
		{
			Arme rapidement pour &'eliminer l'ennemi. 
			Pour tra&^iner l'ennemi, maintenez la touche
			Arme enfonc&'ee.
		}
#endif
#ifdef d:PSX2
		{
			Approchez-vous par derri&`ere, sans arme,
			et actionnez la touche Arme (sans toucher
			le joystick analogique gauche) pour &'etrangler
			l'ennemi.
		}
	中級２説明_1:
		{
			Frappez rapidement la touche Arme pour
			l'assommer. Pour le tra&^iner, maintenez
			la touche Arme enfonc&'ee.

		}
#endif
/*			Walk up from behind w/o a weapon & press
			□ button (w/o left analog stick) to strangle
			enemy. Hit □ button rapidly to take enemy
			out.  To drag enemy, hold down □ button. 
*/
// 体験版と同じ
//		'武器を装備しないで敵の背後に近づき、左スティック'
//		'を入れずに＃｛□、武器｝＃ボタンを押すと、敵の首を絞めることが'
//		'出来る。＃｛□、武器｝＃ボタン連打で敵を倒すことができ、押しっ'
//		'ぱなしにすれば敵を引きずって移動することもできる。'

	中級３説明:
		{
			Appuyez sur la touche Pas &`a gauche/droite
			en Vue en coin pour regarder apr&`es un coin.
			Un ennemi peut vous voir si vous penchez
			la t&^ete. Faites attention.
		}
/*			Press L2/R2 button during corner view
			to peek around corner.  You could be seen
			by enemy when leaning sideways.  Be careful.
*/
// 体験版と同じ
//		'ビハインドカメラの時にＬ２ボタンまたはＲ２ボタンを'
//		'押すと覗き込みができる。曲がり角の先をさらに'
//		'よく伺うことが可能だが、体を乗り出している'
//		'ところを敵に見つからないように注意が必要。'

	中級４説明:
		{
			Appuyez sur la touche Pas &`a gauche/droite
			en Observation &`a la premi&`ere personne pour
			faire un pas sur le c&^ot&'e. Appuyez sur les
			touches Pas &`a gauche et Pas &`a droite en
		}
	中級４説明_1:
		{
			m&^eme temps pour vous hisser sur la pointe
			des pieds. C'est efficace quand vous vous
			cachez derri&`ere des objets pendant les
			&'echanges de tirs.
		}
/*			Press L2/R2 button during First Person View
			to step sideways. Press L2 & R2 buttons 
			together to stretch up.  Effective during gun
			fights while hiding behind objects.
*/
// 体験版と同じ
//		'主観の状態でＬ２ボタン、Ｒ２ボタンを押すと主観の'
//		'ままそれぞれ左右にステップする。Ｌ２ボタンとＲ２'
//		'ボタンを同時に押すと背伸びすることもできる。銃撃'
//		'戦時等、物陰に隠れながら攻撃する時に使うと有効。'

	中級５説明:
		{
			Placez-vous contre un mur et actionnez la
			touche Coup pour frapper contre le mur. Faites
			du bruit pour attirer l'attention d'un ennemi
			qui ne quitte jamais son poste.
		}
/*			Stand up against wall and press ○ button to
			knock on wall.  Make noise to draw attention
			of enemy that never seems to leave his spot.
*/
// 体験版と同じ
//		'壁に張りついた状態で＃｛○、パンチ｝＃ボタンを押すと、壁を叩いて'
//		'音を立てることができる。持ち場から動こうとしない'
//		'敵がいる場合は、音を立てておびき寄せてみるのも'
//		'効果的。'

	中級６説明:
		{
			En faisant une roulade sur l'ennemi, vous le
			jetterez &`a terre, man&@euvre utile lorsque vous
			&^etes encercl&'e(e) par l'ennemi.
		}
/*			If you roll into enemy, you can knock
			down enemy.  Effective when surrounded
			by enemies.
*/
//		'ローリングで敵に向かって飛び込むと、'
//		'敵を吹き飛ばすことができる。敵に囲まれた時等に'
//		'使うと有効。'

	中級７説明:
		{
			En pla&~qant un livre sur le passage de l'ennemi,
			vous attirerez son attention. Ce n'est pas utile
			dans les modes Evasion et Alerte.
		}
/*			Placing a book where enemies walk by will
			catch their attention.  Not effective in Evasion
			and Alert modes.
*/
//		'雑誌を敵兵の通りそうな場所に置いておく事で、'
//		'注意をそらせる事ができる。'
//		'回避モードや危険モードでは効果がない。'
	中級８説明:
		{
			Pour d&'es&'electionner les armes et les objets
			dont vous &^etes &'equip&'e, appuyez rapidement
			sur la touche de menu Objet ou Arme.
			Rappuyez dessus pour vous r&'e&'equiper.
		}
	中級８説明_1:
		{
			Cela est pratique pour les &'equipements
			utilis&'es en Observation &`a la premi&`ere 
			personne, comme la lunette.
		}
/*
			You can deselect equipped weapons and items
			by pressing the L2 or R2 button quickly. Press
			it again to re-equip. Comes in handy with 
			First Person View equipment like the Scope.
*/
//		'クイックチェンジ'
//		'装備した武器や装備は、Ｌ２／Ｒ２ボタンを素早く'
//		'押すことで解除できる。もう一度、同じ操作で'
//		'もとの装備に戻る。装備時には移動できない'
//		'スコープなど主観系装備を着脱するのに便利。'

	上級１説明:
		{
			En Vue en coin, armez-vous et actionnez
			la touche Arme pour sortir d'un coin en sautant
			et pointer votre arme. Vous pouvez attaquer
			rapidement &`a partir de derri&`ere les objets.
		}
/*			During Corner View, equip weapon &
			press □ button to jump out from corner to
			point gun.  You can attack quickly from 
			behind object.
*/
//		'ビハインドカメラ時、ハンドガン等を装備して'
//		'＃｛□、武器｝＃ボタンを押すと物陰から飛び出して銃を構える。'
//		'物陰から素早く攻撃することが可能。'

	上級２説明:
		{
			Faufilez-vous jusqu'&`a l'ennemi sans vous
			faire voir et pointez votre arme vers lui, en
			actionnant la touche Arme, pour le tenir en
			&'echec. Si vous visez les points
		}
	上級２説明_1:
		{
			faibles de l'ennemi, ce dernier vous
			donnera peut-&^etre un objet.
		}
/*			Sneak up to enemy w/o being found & hold up
			handgun by pressing □ button to make enemy
			hold up. Aim at enemy weakpoints and the
			enemy may give you and item.
*/
//		'見つからないよう敵の近くへ忍び寄り、ハンドガン等'
//		'を装備して＃｛□、武器｝＃ボタンで構えると、敵に武器を突き付'
//		'けてホールドアップさせられる。その状態で前から'
//		'急所を狙うとアイテム等を出す場合がある。'

	上級３説明:
		{
			Quand vous serez en train d'&'etrangler un
			ennemi ou de le garder en otage, les autres
			ennemis h&'esiteront &`a tirer. Profitez de ce
			moment pour vous enfuir.
		}
/*			When enemy is being strangled and held
			hostage, other enemies will hesitate to 
			shoot. You can escape during this moment.
*/
// 体験版と同じ
//		'首絞めで敵を羽交い絞めにしていると、敵は仲間に'
//		'弾が当たることを怖れて、攻撃をためらう。'
//		'その間に逃げることも可能。'

	上級４説明:
		{
			Pour viser, appuyez sur la touche Arme.
			Pour vous d&'eplacer tout en visant, appuyez 
			en m&^eme temps sur la touche Arme et sur 
			la touche de verrouillage. Vous pouvez 
		
		}
	上級４説明_1:
		{
			aussi tirer tout en vous d&'epla&~qant.
		}
/*			You can aim guns by pressing the □button.
			While doing so, press the L1 button and you
			can move while aiming the gun. You can also
			shoot while moving.
		'ハンドガンや自動小銃は＃｛□、武器｝＃ボタンを押しこむと'
		'構えるが、その状態でさらにＬ１ボタンを押し込めば'
		'武器を構えたまま移動可能となる。移動しながらの射撃も可能。'
*/

	上級５説明:
		{
			Actionnez la touche Arme, pr&`es d'un ennemi
			inconscient ou mort, pour saisir le corps.
			Rel&^achez la touche Arme pour le l&^acher.
			Recommencez pour secouer le corps et vous
		}
	上級５説明_1:
		{
			verrez peut-&^etre tomber une bo&^ite d'objets.
		}
/*			Press □ button near unconscious / dead 
			enemy to hold body.  Release □ button to let 
			go of body.  Repeat this to shake body & 
			enemy will sometimes drop item box.
*/
// 体験版と同じ
//		'倒れている敵の近くで＃｛□、武器｝＃ボタンを押すと敵の体を'
//		'抱える。＃｛□、武器｝＃ボタンを離すと下ろすが、これを'
//		'繰り返して敵の体を揺するとアイテムボックスを'
//		'出すことがある。'

	上級６説明:
		{
			Quand il vous aura trouv&'e, l'ennemi utilisera
			sa radio (sur sa hanche droite) pour demander
			des renforts. Si vous tirez sur la radio avant que
			l'appel radio puisse se faire, vous emp&^echerez
		}
	上級６説明_1:
		{
			l'arriv&'ee des renforts.
		}
/*			Enemy will use radio (on right waist) to call
			for backup when finding Snake.  If you shoot
			radio before radio transmission is made, you
			can prevent backup from coming.
*/
// 体験版と同じ
//		'敵は侵入者を見つけると、腰の右後ろにつけた'
//		'無線機で増援部隊を呼ぶ。しかし敵が無線連絡を'
//		'行う前に無線機を破壊すれば、応援が呼ばれるのを'
//		'阻止することができる。'

	上級７説明:
		{
			En tirant sur les extincteurs, vous ferez gicler
			les produits chimiques qui s'y trouvent. Faites
			cela pour d&'esorienter les ennemis.
		}
/*			Shoot the fire extinguishers and chemicals
			will shoot out.  Use this to disorient 
			enemies chasing Snake.
*/
// 体験版と同じ
//		'施設内に設置されている消火器を銃で撃つと'
//		'消火剤が噴き出す。これを利用すれば、追撃'
//		'してくる敵を撹乱することもできる。'

	上級８説明:
		{
			Pour retomber sur le sol, actionnez la touche
			Ramper en mode de Suspension. Si vous
			atterrissez sur l'ennemi, vous le jetterez au
			sol.
		}
/*			Press × button during Hanging mode to jump
			down.  If you jump down and land on top of 
			enemy, you can knock out enemy.
*/
// 体験版と同じ
//		'エルード中に＃｛×、ホフク｝＃ボタンを押すとその場所から下に'
//		'飛び降りる。この時、敵の真上に落ちるようにすると、'
//		'一撃で敵を気絶させることが出来る。'

};

// 新規
