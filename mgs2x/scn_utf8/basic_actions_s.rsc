/*
	basic_actions_s.rsc
	    おもしろムービー用スペイン語説明テキスト

	2000/11/10 H.Yoshiike
	$Id: basic_actions_s.rsc,v 1.15 2002/10/21 05:06:55 usr03202 Exp $


*/
/* ページ数が２ページにわたるもの
	初級１:
	初級３:
	初級５:
	初級６:

	中級１:
	中級２:
	中級４:
	中級８:

	上級５:
*/

resource 題名_s {
	初級１:
		'Ataque visi&\'on Primera Persona'
//		'主観攻撃'
	初級２:
		'Lanzar'
//		'投げ'
	初級３:
		'Visi&\'on Esquina'
//		'ビハインドカメラ'
	初級４:	
		'Trepar por objetos'
//		'台登り'
	初級５:
		'Rodar hacia delante'
//		'ローリング'
	初級６:
		'Arrastrar a enemigos'
//		'敵兵引きずり'
	初級７:
		'Colgando'
//		'エルード'
	初級８:
		'Abrir / Cerrar una taquilla'
//		'ロッカー開閉'
	中級１:
		'Caja de cart&\'on'
//		'ダンボール'
	中級２:
		'Estrangular'
//		'首締め'
	中級３:
		'Echar un vistazo desde visi&\'on Esquina'
//		'覗き込み'
	中級４:
		'Avanzar de lado'
//		'サイドステップ'
	中級５:
		'Dar golpes en la pared'
//		'壁たたき'
	中級６:
		'Ataque rodando'
//		'ローリング攻撃'
	中級７:
		'Libro'
//		'雑誌'
	中級８:
		'Cambio r&\'apido'	
//		'クイックチェンジ'	
	上級１:
		'Saltar y disparar'
//		'飛び出し撃ち'
	上級２:
		'A punta de pistola'
//		'ホールドアップ'
	上級３:
		'Escudo Protector'
//		'盾'
	上級４:
		'Apuntar y moverse'
//		'構え移動'
	上級５:
		'Hacer caer un objeto'	
//		'アイテム奪取'	
	上級６:
		'Destruir la radio'
//		'無線機破壊'
	上級７:
		'Extintor'
//		'消火器'
	上級８:
		'Colgando y saltar hacia abajo'
//		'エルード飛び降り'
	};


resource 説明_s {
	初級１説明:
#ifdef d:XBOX
		{
			En Visi&'on Primera Persona, usa el mando de
			direcci&'on para apuntar y disparar a blancos
			espec&'ificos - puntos d&'ebiles de los enemigos
			y lugares innacesibles normalmente. 
		}
	初級１説明_1:
		{
			Desde Visi&'on Primera Persona, tambi&'en
			puedes dar pu&~netazos.
		}
#endif
#ifdef d:PSX2
		{
			Apunta en la visi&'on Primera Persona y utiliza
			el bot&'on de direcci&'on para apuntar y disparar
			a lugares espec&'ificos. No puedes disparar a los
			puntos d&'ebiles del enemigo ni a lugares que
		}
	初級１説明_1:
		{
			tengan  diferentes alturas. En la visi&'on Primera
			Persona tambi&'en puedes dar pu&~netazos.
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
			Sin ning&'un arma equipada, usa el stick 
			anal&'ogico izquierdo y el bot&'on Arma cuando 
			est&'es cerca de un enemigo para derribarle. En
			ocasiones, el enemigo perder&'a el conocimiento.
		}
#endif
#ifdef d:PSX2
		{
			Utiliza el joystick anal&'ogico izquierdo y
			el bot&'on Arma cuando est&'es cerca del enemigo
			(desarmado) para tirarle. En ocasiones, podr&'as
			lograr que el enemigo pierda el conocimiento.
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
			Mueve el stick anal&'ogico izquierdo hacia la
			pared cerca de una esquina para pasar a
			Visi&'on Esquina. Te puedes esconder y echar
			un vistazo desde all&'i. En Visi&'on Esquina 
		}
	初級３説明_1:
		{
			puedes mover la c&'amara con el stick 
			anal&'ogico derecho.
		}
#endif
#ifdef d:PSX2
		{
			Para pasar a la visi&'on Esquina mueve el joystick
			anal&'ogico izquierdo hacia la pared que haya
			cerca de la esquina. Podr&'as permanecer
			escondido y echar un vistazo desde
		}
	初級３説明_1:
		{
			la esquina. En la visi&'on Esquina podr&'as mover
			la c&'amara con el joystick anal&'ogico derecho.
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
			Pulsa el bot&'on Acci&'on para trepar por objetos
			que te lleguen a la cintura. No podr&'as 
			trepar objetos que sean demasiado altos.
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
			Pulsa el bot&'on Gatear cuando est&'es corriendo
			para rodar hacia delante. Muy &'util cuando quieras
			saltar de un objeto a otro. Mant&'en pulsado el
			bot&'on Gatear cuando est&'es rodando para
		}
	初級５説明_1:
		{
			comenzar a gatear cuando dejes de rodar.
		}
/*			Press X button when running to roll forward.
			Effective to jump from one object to another.
			Hold down X button while rolling to start
			crawling at the end of the roll.
*/
// 体験版と同じ
//		'走りながら＃｛X、ホフク｝＃ボタンを押すと、ローリングができる。'
//		'物陰から物陰に素早く移動する場合や一定距離以下の'
//		'穴を飛び越えるのに有効。＃｛X、ホフク｝＃ボタンを押しっぱなしに'
//		'すると、終了後そのままホフクになる。'

	初級６説明:
		{
			Pulsa el bot&'on Arma cuando est&'es cerca de
			enemigos inconscientes o muertos (desarmados)
			para sujetar su cuerpo. Mant&'en pulsado
			el bot&'on Arma para arrastrar su cuerpo.
		}
	初級６説明_1:
		{
			&'Util para esconder y evitar que encuentren
			el cuerpo de un enemigo.
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
			Pulsa el bot&'on Acci&'on cuando est&'es cerca de una
			barandilla para colgarte y evitar que te vea el
			enemigo. Podr&'as moverte de lado y pasar a la
			visi&'on Primera Persona.
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
			Col&'ocate enfrente de una taquilla y pulsa
			el bot&'on Acci&'on para abrir la taquilla. P&'ulsalo
			de nuevo para cerrarla. Puedes abrir la taquilla
			incluso cuando est&'es apoyado en ella.
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
			Si te escondes debajo de una caja de cart&'on
			y no te mueves, el enemigo no te descubrir&'a.
			Si te mueves cuando el enemigo est&'e cerca,
			te descubrir&'a. Si bloqueas el camino
		}
	中級１説明_1:
		{
			del enemigo cuando est&'es dentro de la caja,
			ser&'as descubierto.
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
			Sin ning&'un arma, ac&'ercate al enemigo por la 
			espalda y pulsa el bot&'on Arma sin usar el stick
			anal&'ogico izquierdo. Pulsa el bot&'on Arma 
			r&'apidamente para acabar con el enemigo. 
		}
	中級２説明_1:
		{
			Para arrastrar al enemigo, mant&'en pulsado 
			el bot&'on Arma. 
		}
#endif
#ifdef d:PSX2
		{
			Ac&'ercate por detr&'as desarmado y pulsa el bot&'on
			Arma (sin el joystick anal&'ogico izquierdo) para
			estrangular al enemigo. Pulsa el bot&'on Arma
			r&'apidamente para acabar con el enemigo.
		}
	中級２説明_1:
		{
			Para arrastrar al enemigo, mant&'en pulsado 
			el bot&'on Arma. 
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
			Pulsa el bot&'on de Paso Izquierdo/Derecho 
			desde Visi&'on Esquina para echar un vistazo. 
			El enemigo te puede descubrir cuando te 
			eches hacia delante. Ten cuidado.
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
			Pulsa el bot&'on de Paso Izquierdo/Derecho
			desde Visi&'on Primera Persona para dar un
			paso de lado. Pulsa el bot&'on de Paso 
			Izquierdo/Derecho al mismo tiempo para
		}
	中級４説明_1:
		{
			estirarte. Muy &'util para esconderte detr&'as
			de objetos durante tiroteos.
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
			Col&'ocate contra una pared y pulsa el bot&'on 
			Pu&~netazo para derribar la pared. Haz ruido
			para llamar la atenci&'on del enemigo que nunca
			abandone su posici&'on.
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
			Si vas rodando hacia un enemigo le podr&'as
			tirar. &'Util cuando el enemigo te est&'e
			rodeando.
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
			Si colocas un libro por donde pase el enemigo
			llamar&'as su atenci&'on. No funciona en el modo
			Evasi&'on y Alerta.
		}
/*			Placing a book where enemies walk by will
			catch their attention.  Not effective in Evasion
			and Alert modes.
		'雑誌を敵兵の通りそうな場所に置いておく事で、'
		'注意をそらせる事ができる。'
		'回避モードや危険モードでは効果がない。'
*/
	中級８説明:
		{
			Puedes dejar el arma u objeto que tengas
			pulsando el bot&'on del men&'u de Objetos o de 
			Armas r&'apidamente . P&'ulsalo de nuevo para 
			volver a cogerlo. Muy &'util con equipamiento 
		}
	中級８説明_1:
		{
			como la Mirilla en Visi&'on Primera Persona.
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
			Durante la visi&'on Esquina, coge un arma
			y pulsa el bot&'on Arma para saltar desde
			la esquina y apuntar con el arma. Puedes
			atacar r&'apidamente desde detr&'as de un objeto.
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
			Inf&'iltrate hacia el enemigo que no haya sido
			descubierto y pulsa el bot&'on Arma para apuntarle
			con la pistola. Apunta a los puntos d&'ebiles
			del enemigo e igual te da un objeto.
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
			Cuando est&'es estrangulando a un reh&'en,
			los otros enemigos dudar&'an al disparar.
			Aprovecha este momento para escapar.
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
			Apunta con la pistola pulsando el bot&'on Arma.
			Para moverte mientras apuntas, pulsa al mismo
			tiempo el bot&'on Arma y el bot&'on Bloquear.
			Tambi&'en puedes disparar mientras te mueves.
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
			Pulsa el bot&'on Arma cuando est&'es cerca
			de enemigos inconscientes o muertos para
			sujetar su cuerpo. Suelta el bot&'on Arma para
			soltar su cuerpo. Repite lo anterior para
		}
	上級５説明_1:
		{
			agitar su cuerpo y quiz&'as se le caiga
			alg&'un objeto.
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
			El enemigo usar&'a la radio (en su cadera derecha)
			para pedir refuerzos cuando vean a usted. Si
			disparas a la radio antes de que lo haga,
			evitar&'as que pida refuerzos.
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
			Dispara a los extintores y productos qu&'imicos
			para causar una explosi&'on. Hazlo para 
			desorientar a los enemigos.
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
			Pulsa el bot&'on Gatear cuando est&'es en el modo
			Colgando para saltar hacia abajo. Si saltas y
			caes encima de un enemigo, le podr&'as dejar
			inconsciente.
		}
/*			Press X button during Hanging mode to jump
			down.  If you jump down and land on top of 
			enemy, you can knock out enemy.
*/
// 体験版と同じ
//		'エルード中に＃｛X、ホフク｝＃ボタンを押すとその場所から下に'
//		'飛び降りる。この時、敵の真上に落ちるようにすると、'
//		'一撃で敵を気絶させることが出来る。'

};


// 新規
