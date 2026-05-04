/*	basic_actions_e.rsc
	    おもしろムービー用英語説明テキスト
	2000/11/10 H.Yoshiike
	$Id: basic_actions_e.rsc,v 1.17 2002/10/21 05:06:00 usr03202 Exp $
*/

/* ページ数が2ページにわたるもの
	初級１:
	初級６:
	中級１:
	上級５:
*/

resource 題名_e {
	初級１:
		'First Person View Attack'
//		'主観攻撃'
	初級２:
		'Throw'
//		'投げ'
	初級３:
		'Corner View'
//		'ビハインドカメラ'
	初級４:	
		'Climb up Objects'
//		'台登り'
	初級５:
		'Roll Forward'
//		'ローリング'
	初級６:
		'Dragging Enemies'
//		'敵兵引きずり'
	初級７:
		'Hanging'
//		'エルード'
	初級８:
		'Open / Close Locker '
//		'ロッカー開閉'
	中級１:
		'Cardboard Box'
//		'ダンボール'
	中級２:
		'Choke Hold'
//		'首締め'
	中級３:
		'Peek around Corner'
//		'覗き込み'
	中級４:
		'Step Sideways'
//		'サイドステップ'
	中級５:
		'Knock on Wall'
//		'壁たたき'
	中級６:
		'Rolling Attack'
//		'ローリング攻撃'
	中級７:
		'Book'
//		'雑誌'
	中級８:
		'Quick Change'	
//		'クイックチェンジ'	

	上級１:
		'Jump-out Shot'
//		'飛び出し撃ち'
	上級２:
		'Hold up'
//		'ホールドアップ'
	上級３:
		'Shield'
//		'盾'
	上級４:
		'Aim and Move'
//		'構え移動'
	上級５:
		'Shake out Item'	
//		'アイテム奪取'	
	上級６:
		'Destroy Radio'
//		'無線機破壊'
	上級７:
		'Fire Extinguisher'
//		'消火器'
	上級８:
		'Hang and Jump Down'
//		'エルード飛び降り'
	};

resource 説明_e {
	初級１説明:
#ifdef d:XBOX
		{
			Aim in First Person View and use the 
			directional pad to aim at and shoot specific
			spots --enemy weak points and spots at various
			heights that you normally cannot shoot.
		}
	初級１説明_1:
		{
			You can also punch in First Person View.
		}
#endif
#ifdef d:PSX2
		{
			Aim in First Person View and use the 
			directional button to aim at and shoot specific
			spots --enemy weak points and spots at various
			heights that you normally cannot shoot.
		}
	初級１説明_1:
		{
			You can also punch in First Person View.
		}
#endif
//		'銃等を構えて主観視点にすると、方向キーにより'
//		'上下左右、自在に狙いを定める事が可能。'
//		'敵の急所や高低差があって通常では狙えない場所も'
//		'攻撃できる。また主観視点でパンチ等もくりだせる。'	

	初級２説明:
#ifdef d:XBOX
		{
			With no weapon equipped, use the left
			thumbstick and the Weapon button near
			an enemy to throw him. Sometimes this
			will knock the enemy unconscious.
		}
#endif
#ifdef d:PSX2
		{
			With no weapon equipped, use the left
			analog stick and the Weapon button 
			near an enemy to throw him. Sometimes
			this will knock the enemy unconscious.
		}
#endif

// 体験版と同じ
//		'敵の近くで、武器を装備せずに左スティックを'
//		'入れながら＃｛□、武器｝＃ボタンを押すと、敵を投げることが'
//		'できる。敵を気絶させることも可能。'

	初級３説明:	
//ビハインドモード//	'ああああああああああああああああああああああああ' 
//もぢすうものさし。
#ifdef d:XBOX
		{
			Input left thumbstick toward wall near corner
			to enter corner view.  You can stay hidden
			and look beyond corner. You can move camera
			with right thumbstick in corner view.
		}
#endif
#ifdef d:PSX2
		{
			Input left analog stick toward wall near
			corner to enter corner view.  You can stay
			hidden and look beyond corner. You can move
			camera with right analog stick in corner view.
		}
#endif
//		'曲がり角近くで壁に向かって左スティックを'
//		'倒し続けると、壁に張りついてビハインドカメラに'
//		'なる。物陰に隠れながら先を伺うことができる。'
//		'この間、右スティックでカメラを操作可能。'

	初級４説明:
		{
			Press Action button to climb up objects
			with heights close to your waist.
			You cannot climb up heights too high.
		}
//		'腰くらいの高さのものは、＃｛△、アクション｝＃ボタンを押せば'
//		'その上によじ登ることができる。'
//		'高すぎる場合は登る事ができない。'

	初級５説明:
		{
			Press Crawl button when running to roll forward.
			Effective to jump from one object to another.
			Hold down Crawl button while rolling to start
			crawling at the end of the roll.
		}
// 体験版と同じ
//		'走りながら＃｛×、ホフク｝＃ボタンを押すと、ローリングができる。'
//		'物陰から物陰に素早く移動する場合や一定距離以下の'
//		'穴を飛び越えるのに有効。＃｛×、ホフク｝＃ボタンを押しっぱなしに'
//		'すると、終了後そのままホフクになる。'	

	初級６説明:
		{
			With no weapon equipped, press the Weapon
			button near an unconscios/dead enemy to
			grab his body. Hold down the Weapon button
			to drag him.
			
		}
	初級６説明_1:
		{
			You can use this to hide bodies and
			avoid begin found.
		}
// 体験版と同じ
//		'倒れた敵の近くで武器を装備せずに＃｛□、武器｝＃ボタンを押すと'//		'敵の体を抱える。そのまま＃｛□、武器｝＃ボタンを押しっぱなしに'//		'すれば、敵を引きずって移動させることができる。'
//		'倒した敵の体を発見されないよう隠す時に有効。'	

	初級７説明:
		{
			Press Action button near handrail to jump
			over handrail and hang to avoid enemy. 
			You can move sideways and also enter
			First Person View.
		}
// 体験版と同じ
//		'乗り越えられる手すりの近くで＃｛△、アクション｝＃ボタンを押すと、'
//		'手すりを乗り越えて向こう側にぶら下がることが'
//		'できる。うまく使えば敵をやり過ごすことも可能。'
//		'左右に移動したり主観にすることもできる。'

	初級８説明:
		{
			Stand in front of locker & press Action button
			to open locker.  Press Action button again in
			front of open locker to close it. You can open
			locker even when standing against it.
		}
//		'ロッカーは扉の前に立って＃｛△、アクション｝＃ボタンを押すと扉を'
//		'開けることができる。開いたロッカーの前で'
//		'もう一度＃｛△、アクション｝＃ボタンを押せば扉を閉める。'
//		'ロッカーに張付いた状態からでも可能。'


	中級１説明:
		{
			If you hide under a cardboard box and stay
			still, the enemy will ignore you. If you move
			when in his sight, you will be found. Blocking 
			enemy's path while in the box will cause 
		}
	中級１説明_1:
		{
			the enemy to discover you.
		}
//		'通路の端などでダンボールをかぶって動かなければ、'
//		'敵の視界に入っても怪しまれない。ただし視界内で'
//		'少しでも動くと見つかる。また隠れ場所が敵の移動の'
//		'邪魔になる場合にも怪しまれるので注意が必要。'

	中級２説明:
#ifdef d:XBOX
		{
			With no weapon equipped, walk up to an enemy
			from behind & press the Weapon button, without
			using the left thumbstick. Hit the Weapon button
			rapidly to take the enemy out.
		}
	中級２説明_1:
		{
			To drag the enemy, hold down
			the Weapon button. 
		}
#endif
#ifdef d:PSX2
		{
			With no weapon equipped, walk up to an enemy
			from behind & press the Weapon button, without
			using the left analog stick. Hit the Weapon
			button rapidly to take the enemy out.
		}
	中級２説明_1:
		{
			To drag the enemy, hold down
			the Weapon button. 
		}
#endif
// 体験版と同じ
//		'武器を装備しないで敵の背後に近づき、左スティック'
//		'を入れずに＃｛□、武器｝＃ボタンを押すと、敵の首を絞めることが'
//		'出来る。＃｛□、武器｝＃ボタン連打で敵を倒すことができ、押しっ'
//		'ぱなしにすれば敵を引きずって移動することもできる。'

	中級３説明:
		{
			Press the left/right Step button during
			Corner View to peek around a corner.
			You could be seen by an enemy when
			leaning sideways. Be careful.
		}
// 体験版と同じ
//		'ビハインドカメラの時にＬ２ボタンまたはＲ２ボタンを'
//		'押すと覗き込みができる。曲がり角の先をさらに'
//		'よく伺うことが可能だが、体を乗り出している'
//		'ところを敵に見つからないように注意が必要。'

	中級４説明:
		{
			Press the Left/Right Step button during
			First Person View to step sideways. Press
			the Left & Right Step buttons together to
			stretch up. Effective while hiding behind
		}
	中級４説明_1:
		{
			 objects during gun fights.
		}
// 体験版と同じ
//		'主観の状態でＬ２ボタン、Ｒ２ボタンを押すと主観の'
//		'ままそれぞれ左右にステップする。Ｌ２ボタンとＲ２'
//		'ボタンを同時に押すと背伸びすることもできる。銃撃'
//		'戦時等、物陰に隠れながら攻撃する時に使うと有効。'

	中級５説明:
		{
			Stand up against a wall and press the Punch
			button to knock on the wall. This will draw
			the attention of an enemy who never seems
			to leave his spot.
		}
// 体験版と同じ
//		'壁に張りついた状態で＃｛○、パンチ｝＃ボタンを押すと、壁を叩いて'
//		'音を立てることができる。持ち場から動こうとしない'
//		'敵がいる場合は、音を立てておびき寄せてみるのも'
//		'効果的。'

	中級６説明:
		{
			If you roll into an enemy, you can knock him
			down. Effective when surrounded by enemies.
		}
//		'ローリングで敵に向かって飛び込むと、'
//		'敵を吹き飛ばすことができる。敵に囲まれた時等に'
//		'使うと有効。'

	中級７説明:
		{
			Placing a book in or near an enemy's path
			will catch his attention.  Not effective in
			Evasion and Alert modes.
		}
//		'雑誌を敵兵の通りそうな場所に置いておく事で、'
//		'注意をそらせる事ができる。'
//		'回避モードや危険モードでは効果がない。'

	中級８説明:
		{
			You can deselect equipped weapons and
			items by pressing the Item or Weapon menu
			button quickly.  Press it again to re-equip.
			Comes in handy with First Person View 
		}
	中級８説明_1:
		{
			equipment like the Scope.
		}
//		'クイックチェンジ'
//		'装備した武器や装備は、Ｌ２／Ｒ２ボタンを素早く'
//		'押すことで解除できる。もう一度、同じ操作で'
//		'もとの装備に戻る。装備時には移動できない'
//		'スコープなど主観系装備を着脱するのに便利。'

	上級１説明:
		{
			While in Corner View, equip weapon & press
			the Weapon button to jump out and aim.
			You can attack quickly around a corner.
		}
//		'ビハインドカメラ時、ハンドガン等を装備して'
//		'＃｛□、武器｝＃ボタンを押すと物陰から飛び出して銃を構える。'
//		'物陰から素早く攻撃することが可能。'

	上級２説明:
		{
			To hold up an enemy, sneak up and press
			the Weapon button to raise your gun.
			Aim at enemy weak points and the enemy
			may give you an item.
		}
//		'見つからないよう敵の近くへ忍び寄り、ハンドガン等'
//		'を装備して＃｛□、武器｝＃ボタンで構えると、敵に武器を突き付'
//		'けてホールドアップさせられる。その状態で前から'
//		'急所を狙うとアイテム等を出す場合がある。'

	上級３説明:
		{
			Use one enemy as a shield by strangling him
			and taking him hostage. Other enemies will
			hesitate to shoot. You can escape during
			this moment.
		}
// 体験版と同じ
//		'首絞めで敵を羽交い絞めにしていると、敵は仲間に'
//		'弾が当たることを怖れて、攻撃をためらう。'
//		'その間に逃げることも可能。'

	上級４説明:
		{
			Aim guns by pressing the Weapon button. 
			To move while aiming the gun, press both
			the Weapon button and the Lock-on button.
			You can also shoot while moving.
		}
//		'ハンドガンや自動小銃は＃｛□、武器｝＃ボタンを押しこむと'
//		'構えるが、その状態でさらにＬ１ボタンを押し込めば'
//		'武器を構えたまま移動可能となる。移動しながらの射撃も可能。'

	上級５説明:
		{
			Lift an unconscious or dead enemy by pressing
			the Weapon button. Release the Weapon
			button to drop the body. Continue pressing
			and releasing the Weapon button to shake
		}
	上級５説明_1:
		{
			the body. The enemy may drop an item.
		}
// 体験版と同じ
//		'倒れている敵の近くで＃｛□、武器｝＃ボタンを押すと敵の体を'
//		'抱える。＃｛□、武器｝＃ボタンを離すと下ろすが、これを'
//		'繰り返して敵の体を揺するとアイテムボックスを'
//		'出すことがある。'

	上級６説明:
		{
			When discovered by an enemy, prevent a call
			for backup by shooting the radio on the right
			side of his waist before he can call for help.
		}
// 体験版と同じ
//		'敵は侵入者を見つけると、腰の右後ろにつけた'
//		'無線機で増援部隊を呼ぶ。しかし敵が無線連絡を'
//		'行う前に無線機を破壊すれば、応援が呼ばれるのを'
//		'阻止することができる。'

	上級７説明:
		{
			Shoot the fire extinguishers and chemicals will
			spray out.  Use this to disorient enemies.
		}
// 体験版と同じ
//		'施設内に設置されている消火器を銃で撃つと'
//		'消火剤が噴き出す。これを利用すれば、追撃'
//		'してくる敵を撹乱することもできる。'

	上級８説明:
		{
			While in Hanging mode, press the Crawl button
			to jump down. To knock out an enemy, jump
			down and land on top of enemy.
		}
// 体験版と同じ
//		'エルード中に＃｛×、ホフク｝＃ボタンを押すとその場所から下に'
//		'飛び降りる。この時、敵の真上に落ちるようにすると、'
//		'一撃で敵を気絶させることが出来る。'

};

// 新規
