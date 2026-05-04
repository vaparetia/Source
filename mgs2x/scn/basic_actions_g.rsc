/*
	basic_actions_g.rsc
	    おもしろムービー用ドイツ語説明テキスト

	2000/11/10 H.Yoshiike
	$Id: basic_actions_g.rsc,v 1.12 2002/10/21 05:06:43 usr03202 Exp $


*/

/* ページ数が2ページにわたるもの
	初級１:
	初級３:
	初級７:

	中級２:
	中級４:
	中級８:

	上級２:
	上級５:
*/

resource 題名_g {
	初級１:
		'Angriff in der Beobachtungsperspektive'
//		'主観攻撃'
	初級２:
		'Wegschleudern'
//		'投げ'
	初級３:
		'Schulterblick'
//		'ビハインドカメラ'
	初級４:	
		'Auf Objekte klettern'
//		'台登り'
	初級５:
		'Nach vorne rollen'
//		'ローリング'
	初級６:
		'Gegner ziehen'
//		'敵兵引きずり'
	初級７:
		'Hangeln'
//		'エルード'
	初級８:
		'Spind &~offnen/schlie&~sen'
//		'ロッカー開閉'
	中級１:
		'Karton'
//		'ダンボール'
	中級２:
		'W&~urgegriff'
//		'首締め'
	中級３:
		'Um die Ecke sp&~ahen'
//		'覗き込み'
	中級４:
		'Schritt zur Seite'
//		'サイドステップ'
	中級５:
		'An die Wand klopfen'
//		'壁たたき'
	中級６:
		'Angriff durch Rollen'
//		'ローリング攻撃'
	中級７:
		'Buch'
//		'雑誌'
	中級８:
		'Schnellwechsel'	
//		'クイックチェンジ'	
	上級１:
		'Hervorspringen und Schie&~sen'
//		'飛び出し撃ち'
	上級２:
		'&~Uberfallen'
//		'ホールドアップ'
	上級３:
		'Schutzschild'
//		'盾'
	上級４:
		'Zielen und gleichzeitig bewegen'
//		'構え移動'
	上級５:
		'Objekt heraussch&~utteln'	
//		'アイテム奪取'	
	上級６:
		'Funkger&~at zerst&~oren'
//		'無線機破壊'
	上級７:
		'Feuerl&~oscher'
//		'消火器'
	上級８:
		'Hangeln und herunterspringen'
//		'エルード飛び降り'
	};


resource 説明_g {
	初級１説明:
#ifdef d:XBOX
		{
			In der Beobachtungsperspektive (BP) zielen, 
			mit dem Steuerkreuz besondere Punkte 
			anvisieren und beschie&~sen ? 
			Feindschwachpunkte und Punkte in 
		}
	初級１説明_1:
		{
			verschiedenen H&~ohen, die normalerweise
			nicht zu treffen sind. Sie k&~onnen in der BP
			auch schlagen.
		}
#endif
#ifdef d:PSX2
		{
			Boxen u. zielen Sie in der Beobachtungs-
			perspektive. Mit der Richtungstaste k&~onnen
			Sie zielen u. schie&~sen: auf Schwachstellen
			des Feindes u. normalerweise unerreichbare
		}
	初級１説明_1:
		{
			Stellen in unterschiedlichen H&~ohen.
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
			Wenn keine Waffe aufgenommen wurde, mit
			dem Ministick links und der Waffentaste einen
			nahen Feind werfen. Manchmal wird der Feind
			dadurch bewusstlos geschlagen.
		}
#endif
#ifdef d:PSX2
		{
			Mit den Linker Analog-Stick und Waffentaste
			(ohne Waffe) k&~onnen Sie den Feind aus der
			N&~ahe  wegschleudern oder bewu&~stlos machen.
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
			Schulterblick (SB) aufrufen - Ministick links
			zur Wand nahe der Ecke bewegen. Sie 
			k&~onnen verborgen bleiben und um die Ecke 
			schauen. Im SB kann die Kamera mit dem
		}
	初級３説明_1:
		{
			Ministick rechts bewegt werden.
		}
#endif
#ifdef d:PSX2
		{
			Dr&~ucken Sie den Linker Analog-Stick zur Wand
			nahe der Ecke, um in den Schulterblick zu
			wechseln u. ungesehen um die Ecke zu schauen.
			Der Rechter Analog-Stick bewegt hier die 
		}
	初級３説明_1:
		{
			Kamera. 
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
			Mit der Aktionstaste klettern Sie auf Objekte 
			bis zu Taillenh&~ohe. Viel h&~oher k&~onnen
			Sie nicht klettern.
		}
/*			Press △ button to climb up objects
			with heights close to your waist.
			You cannot climb up heights too high.
*/
//		'腰くらいの高さのものは、＃｛△、アクション｝＃ボタンを押せば'
//		'その上によじ登ることができる。'
//		'高すぎる場合は登る事ができない。'

	初級５説明:
		{
			Durch Dr&~ucken der Kriechtaste beim Rennen
			rollen Sie nach vorne (gut zum Springen 
			zwischen Objekten). Halten Sie die Kriechtaste
			beim Rollen gedr&~uckt, um danach zu kriechen.
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
			Dr&~ucken Sie die Waffentaste nahe eines bewu&~st-
			losen/toten Gegners (ohne Waffe), um den 
			K&~orper zu halten. Waffentaste gedr&~uckt halten, 
			um Gegner zu ziehen. Gut zum Verstecken.
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
			Durch Dr&~ucken der Aktionstaste nahe des
			Gel&~anders springen Sie dar&~uber und bleiben
			h&~angend f&~ur den Gegner unsichtbar.
			Sie k&~onnen sich seitlich bewegen u.
		}
	初級７説明_1:
		{
			die Beobachtungspersp. aufrufen.
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
			Vor dem Spind stehend &~offnen Sie mit der 
			Aktionstaste den Spind. Dr&~ucken Sie nochmal die
			Aktionstaste, um den Spind wieder zu schlie&~sen.
			Er l&~a&~st sich auch bei Dagegenlehnen &~offnen.
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
			Wenn Sie sich still unter einem Karton verstek-
			ken, ignoriert Sie der Gegner. Wenn Sie sich 
			unter dem Karton in seiner Sichtweite bewegen 
			oder ihm den Weg verstellen, entdeckt er Sie.
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
			Wenn keine Waffe aufgenommen wurde, von
			hinten an den Feind herangehen und die
			Waffentaste dr&~ucken, ohne den Ministick links
			zu benutzen. Die Waffentaste schnell dr&~ucken,
		}
	中級２説明_1:
		{
			um den Feind auszuschalten. Die Waffentaste
			dr&~ucken und halten, um den Feind zu ziehen.
		}
#endif
#ifdef d:PSX2
		{
			N&~ahern Sie sich ohne Waffe von hinten und
			w&~urgen Sie den Gegner mit der Waffentaste
			(ohne den Linker Analog-Stick). Waffentaste
			mehrmals schnell dr&~ucken, 
		}
	中級２説明_1:
		{
			um ihn unsch&~adlich zu machen.
			Mit gedr&~uckter Waffentaste ziehen Sie ihn.
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
			Im Schulterblick die linke/rechte Schritttaste
			dr&~ucken, um um die Ecke zu sp&~ahen.
			Wenn Sie sich zur Seite lehnen, k&~onnte Sie
			ein Feind sehen. Vorsicht!
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
			In der Beobachtungsperspektive die linke/rechte
			Schritttaste dr&~ucken, um zur Seite zu treten.
			Zum Hochstrecken die linke u. rechte
			Schritttaste gleichzeitig dr&~ucken.
		}
	中級４説明_1:
		{
			Ist bei Schie&~sereien beim Verbergen
			hinter Objekten wirkungsvoll.
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
			Stellen Sie sich gegen die Wand u. dr&~ucken Sie
			die Schlagtaste, um an die Wand zu klopfen. So 
			erregen Sie die Aufmerksamkeit des 
			Gegners, der immer hier zu sein scheint.
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
			Wenn Sie in den Gegner rollen, k&~onnen Sie 
			ihn umwerfen. Effektiv, wenn Sie von Gegnern 
			umzingelt sind.
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
			Legen Sie den Gegnern ein Buch in den Weg, 
			um ihre Aufmerksamkeit zu erwecken. Wirkt 
			nicht im Ausweich- und Alarmmodus.
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
			Aufgenommene Waffen oder Objekte abw&~ahlen -
			Objekt- oder Waffenmen&~utaste schnell dr&~ucken.
			Zum Wiederaufnehmen Taste erneut dr&~ucken.
			Bei Ausr&~ustungen der Beobachtungsper-
		}
	中級８説明_1:
		{
			spektive g&~unstig, wie etwa Zielfernrohr.
		}
/*			You can deselect equipped weapons and items
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
			Im Schulterblick Waffe nehmen u. Waffentaste
			dr&~ucken, um hinter der Ecke mit schu&~sberei-
			ter Waffe hervorzuspringen. Hinter einem 
			Objekt haben Sie eine gute Angriffsposition. 
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
			Schleichen Sie sich unbemerkt an einen Feind 
			an und dr&~ucken Sie die Waffentaste, um ihn mit
			der Pistole zu &~uberfallen. Zielen Sie auf seine 
			Schwachstellen. Vielleicht gibt er Ihnen
		}
	上級２説明_1:
		{
			ein Objekt.
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
			Wenn ein Gegner im W&~urgegriff als Geisel
			gehalten wird, schie&~sen andere Gegner
			nicht so leicht. Sie k&~onnen entkommen!
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
			Mit der Waffen zielen ? Waffentaste dr&~ucken.
			Bewegen und dabei mit der Waffe zielen - 
			Waffentaste und Aufschalttaste dr&~ucken.
			Sie k&~onnen auch aus der Bewegung schie&~sen.
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
			Dr&~ucken Sie die Waffentaste nahe eines bewu&~st-
			losen/toten Gegners, um ihn zu halten. Lassen 
			Sie die Waffentaste los und damit auch den
			K&~orper. 
		}
	上級５説明_1:
		{
			Aktion wiederholen, dann l&~a&~st der Gegner 
			u.U. eine Objektkiste fallen.
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
			Der Gegner ruft per Funk (an der rechten H&~ufte)
			Verst&~arkung, wenn er Sie gefunden hat. 
			Schie&~sen Sie vorher auf das Funkger&~at, so da&~s
			keine Verst&~arkung anr&~ucken kann. 
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
			Schie&~sen Sie auf die Feuerl&~oscher, damit 
			Chemikalien herausspritzen, um Gegner zu
			verwirren.
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
			Dr&~ucken Sie im Hangelmodus die Kriechtaste,
			um nach unten zu springen. Wenn Sie dabei auf
			einem Feind landen, ist er k.o.
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
