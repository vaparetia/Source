/*
	special_e.h
	    special.gcl内でincludeされる。
	    おもしろムービー用英語説明テキスト

	2000/11/28 T.Fukushima
	$Id: special_e.rsc,v 1.1 2000/11/28 08:20:22 usr03005 Exp $


*/

resource 題名 {
	初級１:
		{Walk/Run}
//		'歩く/走る'
	初級２:
		{Punch/Kick}
//		'パンチ/キック'
	初級３:
		{Strangle}
//		'首絞め'
	初級４:
		{Throw}
//		'投げ'
	初級５:
		{Corner View mode(1)}
//		'ビハインドカメラ（１）'
	初級６:
		{Corner View mode(2)}
//		'ビハインドカメラ（２）'
	初級７:
		{Peek}
//		'のぞきこみ'
	初級８:
		{First Person View Attack}
//		'主観攻撃'
	初級９:
		{Punch in First Person View}
//		'主観パンチ'
	初級１０:
		{Strafe in First Person View}
//		'主観左右ステップ'
	初級１１:
		{Watertight Door}
//		'水密扉開け'
	初級１２:
		{Climb Object}
//		'台登り'
	初級１３:
		{Roll Forward}
//		'とびこみ前転'
	初級１４:
		{Knock on Wall}
//		'壁たたき'
	初級１５:
		{Intrusion mode}
//		'イントルードカメラ'
	中級１:
		{Drag Body}
//		'敵兵引きずり'
	中級２:
		{Shoot Radio}
//		'無線機破壊'
	中級３:
		{Obstruct Radio}
//		'チャフによる無線機妨害'
	中級４:
		{Enemy Passing by}
//		'敵兵やり過ごし'
	中級５:
		{Hanging mode}
//		'エルードぶらさがり'
	中級６:
		{Locker}
//		'ロッカー開け閉め'
	中級７:
		{Avoid Clearing(1)}
//		'クリアリング回避（１）'
	中級８:
		{Avoid Clearing(2)}
//		'クリアリング回避（２）'
	上級１:
		{Jumpout Shot}
//		'とびだし撃ち'
	上級２:
		{Jump Down from Hanging}
//		'エルード飛び降り'
	上級３:
		{Steam Pipe}
//		'パイプ蒸気攻撃'
	上級４:
		{Take Hostage}
//		'敵兵人質'
	上級５:
		{Attack by Rolling}
//		'飛び込み前転アタック'
	上級６:
		{Hold Up}
//		'ホールドアップ'
	上級７:
		{Threaten for Item}
//		'アイテム奪取（１）'
	上級８:
		{Shake Body}
//		'アイテム奪取（２）'
	上級９:
		{Fire Extinguisher}
//		'消火器撹乱'
	ダミー:
		{DUMMY}
//		'ダミー'
	
};

resource 説明 {
	//1.ma0001 移動(歩く/走る）
	初級１説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Use directional button or left analog stick.
			How long you press the button determines
			whether you walk or run.
		}
//		'ゲーム中で最も基本となる動作。左スティック、'
//		'方向キーのどちらでも操作可能。共に入力する'
//		'強さによって、歩く、走るを使い分けることが'
//		'できる。'

	//2.ma0002 パンチ/キック
	初級２説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Press ○ button to punch.  Press consecu-
			tively for a punch-punch-kick combo to
			knock down enemy.  Use when surrounded
			by enemy or when you have no ammo.
		}
//		'＃｛○、パンチ｝＃ボタンを押すとパンチを放つ。連打すればパンチ・'
//		'パンチ・キックの＃｛連続技、コンボ｝＃になり、敵を吹き飛ばすこと'
//		'が出来る。敵に囲まれた場合や武器の弾が残って'
//		'いない場合に使うと有効。'

	//3.ma003 首絞め
	初級３説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Walk up from behind w/o a weapon & press
			□ button (w/o left analog stick) to strangle
			enemy. Hit □ button rapidly to take enemy
			out.  To drag enemy, hold down □ button. 
		}
//		'武器を装備しないで敵の背後に近づき、左スティック'
//		'を入れずに＃｛□、武器｝＃ボタンを押すと、敵の首を絞めることが'
//		'出来る。＃｛□、武器｝＃ボタン連打で敵を倒すことが出来、押しっ'
//		'ぱなしにすれば敵を引きずって移動することも出来る。'

	//4.ma004 投げ
	初級４説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Use left analog stick and □ button near
			enemy (w/o weapon) to throw enemy.
			You can make enemy go unconscious at times.
		}
//		'敵の近くで、武器を装備せずに左スティックを'
//		'入れながら＃｛□、武器｝＃ボタンを押すと、敵を投げることが'
//		'出来る。敵を気絶させることも可能。'

	//5.ma005 ビハインドモード
	初級５説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Input left analog stick toward wall near
			corner to enter corner view mode.  You
			can stay hidden and look beyond corner.
		}
//		'曲がり角近くで壁に向かって左スティックを'
//		'押し込むと、ビハインドカメラになる。'
//		'物陰に隠れながら曲がり角の先をうかがうことが'
//		'可能。'

	//6.ma0006 ビハインドモード（２）
	初級６説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Move camera with right analog stick during
			corner view mode.  View is more limited than
			when peeking around corner, but there is no
			risk of being spotted.
		}
//		'ビハインドカメラ時は、右スティックでカメラを'
//		'動かすことが出来る。のぞきこみより見える範囲は'
//		'少ないが、体を乗り出さないぶん安全に曲がり角の'
//		'先を偵察することが出来る。'

	//7.ma0007 のぞきこみ
	初級７説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Press L2/R2 button during corner view mode
			to peek around corner.  You could be seen
			by enemy when leaning sideways.  Be careful.
		}
//		'ビハインドカメラの時にＬ２ボタンまたはＲ２ボタンを'
//		'押すとのぞきこみが出来る。曲がり角の先をさらに'
//		'よくうかがうことが可能だが、体を乗り出している'
//		'ところを敵に見つからないように注意が必要。'

	//8.ma0008 主観撃ち
	初級８説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Enter First Person View mode w/ R1 button
			(w/ weapon) and press □ button to hold
			weapon for First Person View Attack. 
			You can aim at and shoot specific spots.
		}
//		'武器を装備した状態でＲ１ボタンで主観にして、'
//		'＃｛□、武器｝＃ボタンで武器を構えれば主観攻撃が出来る。'
//		'主観攻撃を使えば、＃｛俯瞰、フカン｝＃では撃てないものを撃つ'
//		'ことも可能。敵の急所を狙い撃ちすることも出来る。'

	//9.ma0009 主観パンチ
	初級９説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			You can punch and kick with ○ button during
			First Person View Mode.
		}
//		'Ｒ１ボタンで主観にしている時でも＃｛○、パンチ｝＃ボタンで'
//		'パンチ・キックを繰り出すことが出来る。'

	//10.ma0010 主観左右ステップ
	初級１０説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Press L2/R2 button during First Person View
			mode to step sideways. Press L2 & R2 buttons 
			together to stretch up.  Effective during gun
			fights while hiding behind objects.
		}
//		'主観の状態でＬ２ボタン、Ｒ２ボタンを押すと主観の'
//		'ままそれぞれ左右にステップする。Ｌ２ボタンとＲ２'
//		'ボタンを同時に押すと背伸びすることも出来る。銃撃'
//		'戦時等、物陰に隠れながら攻撃する時に使うと有効。'

	//11.ma0011水密ドア開け
	初級１１説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Press △ button toward watertight door 
			handle & hold it down to open door.  Press
			△ button rapidly to turn handle quickly.
		}
//		'水密扉に向かって＃｛△、アクション｝＃ボタンを押してハンドルをつかみ、'
//		'そのまま＃｛△、アクション｝＃ボタンを押しっぱなしにすれば、ハンドル'
//		'を回して扉を開けることが出来る。ハンドルを回して'
//		'いる時に＃｛△、アクション｝＃ボタンを連打すると早く開けることも可能。'

	//12.ma0012台登り
	初級１２説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Press  △ button to climb up objects
			with heights close to your waist.
		}
//		'腰くらいの高さのものは、＃｛△、アクション｝＃ボタンを押せば'
//		'その上によじ登ることが出来る。'

	//13.ma0013 飛び込み前転
	初級１３説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Press × button when running to roll forward.
			Effective to jump from one object to another.
			Hold down × button while rolling to start
			crawling at the end of the roll.
		}
//		'走りながら＃｛×、ホフク｝＃ボタンを押すと、とびこみ前転が出来る。'
//		'物陰から物陰に素早く移動する場合等で有効。'
//		'とびこみ前転中に＃｛×、ホフク｝＃ボタンを押しっぱなしにすると、'
//		'とびこみ前転終了後、そのままホフクになる。'

	//14.ma0014 壁叩き
	初級１４説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Stand up against wall and press ○ button to
			knock on wall.  Make noise to draw attention
			of enemy that never seems to leave his spot.
		}
//		'壁に張りついた状態で＃｛○、パンチ｝＃ボタンを押すと、壁を叩いて'
//		'音を立てることが出来る。持ち場から動こうとしない'
//		'敵がいる場合は、音を立てておびき寄せてみるのも'
//		'効果的。'

	//15.ma0015 イントルード
	初級１５説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Crawl under narrow spaces to enter Intrusion
			mode. Use left analog stick to move forward
			or backward & face left or right. You can 
			also enter First Person View mode and attack.
		}
//		'机の下等の狭い所にホフクでもぐりこむと'
//		'イントルードカメラになる。イントルードカメラの'
//		'時は左スティックの上で前進、下で後退、左右で'
//		'それぞれの方向を向く操作になる。主観攻撃も可能。'

	//16.mb0001 敵兵引きずり
	中級１説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Press □ button near unconscious / dead 
			enemy (w/o weapon) to hold body.  Hold
			down □ button to drag enemy.  Effective to
			hide body and avoid being found.
		}
//		'倒れた敵の近くで武器を装備せずに＃｛□、武器｝＃ボタンを押すと'
//		'敵の体を抱える。そのまま＃｛□、武器｝＃ボタンを押しっぱなしに'
//		'すれば、敵を引きずって移動させることが出来る。'
//		'倒した敵の体を発見されないよう隠す時に有効。'

	//17.mb0002 無線機破壊
	中級２説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Enemy will use radio (on right waist) to call
			for backup when finding Snake.  If you shoot
			radio before radio transmission is made, you
			can prevent backup from coming.
		}
//		'敵はスネークを見つけると、腰の右後ろにつけた'
//		'無線機で増援部隊を呼ぶ。しかし敵が無線連絡を'
//		'行う前に無線機を破壊すれば、応援が呼ばれるのを'
//		'阻止することが出来る。'

	//18. mb003 チャフによる妨害
	中級３説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Use chaff grenade to interfere with electro-
			nic devices (surveillance camera, radio, 
			etc.).  Since radio cannot be used, enemy
			cannot call for backup.
		}
//		'チャフ・グレネードを使えば監視カメラ等の電子機器'
//		'を無効に出来るが、同時に敵の無線を妨害することも'
//		'出来る。無線が使用不能になるので、敵は増援部隊を'
//		'呼ぶことが出来なくなる。'

	//19. mb0004 敵兵通り過ぎ
	中級４説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			When chased in narrow corridors, hide
			yourself in indented spots to make enemy
			pass by.
		}
//		'狭い廊下で敵に追撃されても、くぼみにうまく身を'
//		'隠せば敵をやりすごすことが出来る。'

	//20. mb0005 エルードぶらさがり登り*
	中級５説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Press △ button near handrail to jump over
			handrail and hang to avoid enemy.  You can 
			move sideways and also enter First Person
			View mode.
		}
//		'乗り越えられる手すりの近くで＃｛△、アクション｝＃ボタンを押すと、'
//		'手すりを乗り越えて向こう側にぶら下がることが'
//		'出来る。うまく使えば敵をやり過ごすことも可能。'
//		'左右に移動したり主観にすることも出来る。'

	//21. mb0006 ロッカー開け閉め
	中級６説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Stand in front of locker & press △ button
			to open locker.  Press △ button again in
			front of open locker to close it.
		}
//		'ロッカーは扉の前に立って＃｛△、アクション｝＃ボタンを押すと扉を'
//		'開けることが出来る。開いたロッカーの前で'
//		'もう一度＃｛△、アクション｝＃ボタンを押せば扉を閉める。'

	//22. mb007 ロッカークリアリング
	中級７説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Hide inside locker when being chased. 
			If enemy sees you entering locker, you
			will be attacked.  Be careful.
		}
//		'敵に追われていても、うまくロッカーの中に隠れれば'
//		'やり過ごすことが出来る。ただしロッカーに入る'
//		'ところを見られたりすると、攻撃されてしまうので'
//		'注意。'

	//23. mb0008 倉庫クリアリング
	中級８説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			When you place yourself in cofined space,
			enemy will start clearing area to find Snake.
			Hide yourself properly to avoid being found.
		}
//		'入り組んだ場所に逃げ込んだ場合、敵は'
//		'クリアリングを行い、スネークを探し出そうとする。'
//		'この時は、うまく身を隠さなければ見つけられて'
//		'しまうので注意。'

	//24. mc0001 飛び出し撃ち
	上級１説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			During Corner View mode, equip handgun &
			press □ button to jump out from corner to
			point gun.  You can attack quickly from 
			behind object.
		}
//		'ビハインドカメラ時、ハンドガンを装備して'
//		'＃｛□、武器｝＃ボタンを押すと物陰から飛び出して銃を構える。'
//		'物陰から素早く攻撃することが可能。'


	//25. mc0002 エルード飛び降り気絶
	上級２説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Press × button during Hanging mode to jump
			down.  If you jump down and land on top of 
			enemy, you can knock out enemy.
		}
//		'エルード中に＃｛×、キャンセル｝＃ボタンを押すとその場所から下に'
//		'飛び降りる。この時、敵の真上に落ちるようにすると、'
//		'一撃で敵を気絶させることが出来る。'

	//26. mc0003 パイプ蒸気攻撃
	上級３説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Some pipes running along walls and ceiling
			shoot out steam when shot.  Since steam is of
			high temperature, it will damage enemy if 
			you make it shoot in direction of enemy.
		}
//		'壁や天井を這っているパイプにはハンドガンで撃つと'
//		'蒸気が吹き出すものがある。蒸気は高熱なので、'
//		'上手く敵に当てればダメージを与えることが出来る。'

	//27.mc0004 敵兵人質
	上級４説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			When enemy is being strangled and held
			hostage, other enemies will hesitate to 
			shoot. You can escape during this moment.
		}
//		'首絞めで敵を羽交い締めにしていると、敵は仲間を'
//		'撃つことを怖れて、攻撃をためらう。その間に逃げる'
//		'ことも可能。'

	//28. mc0005 飛び込み前転アタック*
	上級５説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			If you roll into enemy, you can knock
			down enemy.  Effective when surrounded
			by enemies.
		}
//		'とびこみ前転で敵に向かってとびこむと、'
//		'敵を吹き飛ばすことが出来る。敵に囲まれた時などに'
//		'使うと有効。'

	//29. mc0006 ホールドアップ
	上級６説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Sneak up to enemy w/o being found & hold up
			handgun by pressing □ button to make enemy
			hold up.  While holding up, enemy cannot do
			anything.
		}
//		'見つからないように敵の近くへ忍び寄り、ハンドガン'
//		'を装備して＃｛□、武器｝＃ボタンで構えると、敵に銃を突き付けて'
//		'ホールドアップさせることが出来る。手を上げている'
//		'間、敵は無力になる。'

	//30. mc0007 ホールドアップアイテム搾取
	上級７説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			While enemy is holding up, move to front & 
			aim at head or groin in First Person View
			mode.  Enemy in fear will give you item box.
		}
//		'ホールドアップさせている敵の正面にまわり、'
//		'主観で頭部や急所に銃を突き付けると、'
//		'敵は命乞いをしながらアイテムボックスを出す。'

	//31. mc0008 倒れている敵兵からアイテム搾取
	上級８説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Press □ button near unconscious / dead 
			enemy to hold body.  Release □ button to let 
			go of body.  Repeat this to shake body & 
			enemy will sometimes drop item box.
		}
//		'倒れている敵の近くで＃｛□、武器｝＃ボタンを押すと敵の体を'
//		'抱える。＃｛□、武器｝＃ボタンを離すと下ろすが、これを'
//		'繰り返して敵の体を揺するとアイテムボックスを'
//		'出すことがある。'

	//32. mc0009 消火器
	上級９説明:
//	'ああああああああああああああああああああああああ' //もぢすうものさし。
		{
			Shoot the fire extinguishers and chemicals
			will shoot out.  Use this to disorient 
			enemies chasing Snake.
		}
//		'船内に設置されている消火器を銃で撃つと消火剤が'
//		'噴き出す。これを利用すれば、追撃してくる敵を'
//		'撹乱することも出来る。'
};


