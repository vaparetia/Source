/*
	basic_actions_i.rsc
	    おもしろムービー用イタリア語説明テキスト

	2000/11/10 H.Yoshiike
	$Id: basic_actions_i.rsc,v 1.12 2002/10/21 05:06:49 usr03202 Exp $


*/
/*
ページが２ページにわたるもの
	初級１:
	初級３:
	初級６:
	初級８:

	中級１:
	中級２:
	中級４:
	中級８:

	上級５:
*/

resource 題名_i {

	初級１:
		'Attacco in Visuale Soggettiva'
//		'主観攻撃'
	初級２:
		'Scaglia'
//		'投げ'
	初級３:
		'Visuale Angolare'
//		'ビハインドカメラ'
	初級４:	
		'Arrampicati sugli oggetti'
//		'台登り'
	初級５:
		'Rotola in avanti'
//		'ローリング'
	初級６:
		'Trascina il nemico'
//		'敵兵引きずり'
	初級７:
		'Sospensione'
//		'エルード'
	初級８:
		'Apri/chiudi l\'armadietto'
//		'ロッカー開閉'
	中級１:
		'Scatola di cartone'
//		'ダンボール'
	中級２:
		'Strangola'
//		'首締め'
	中級３:
		'Sbircia dietro l\'angolo'
//		'覗き込み'
	中級４:
		'Muoviti lateralmente'
//		'サイドステップ'
	中級５:
		'Batti contro il muro'
//		'壁たたき'
	中級６:
		'Attacco in rotolata'
//		'ローリング攻撃'
	中級７:
		'Libro'
//		'雑誌'
	中級８:
		'Cambio rapido'	
//		'クイックチェンジ'	
	上級１:
		'Balza fuori'
//		'飛び出し撃ち'
	上級２:
		'Punta l\'arma'
//		'ホールドアップ'
	上級３:
		'Fatti scudo'
//		'盾'
	上級４:
		'Mira e muoviti'
//		'構え移動'
	上級５:
		'Scuoti l\'oggetto'	
//		'アイテム奪取'	
	上級６:
		'Distruggi la radio'
//		'無線機破壊'
	上級７:
		'Estintore'
//		'消火器'
	上級８:
		'Sospenditi e salta gi&`u'
//		'エルード飛び降り'
	};


resource 説明_i {
	初級１説明:
#ifdef d:XBOX
		{
			Mira in Visuale Soggettiva e usa il tasto 
			direzionale per puntare e sparare a punti
			specifici - i punti deboli del nemico e posti a
			varie altezze a cui di norma non puoi sparare.
		}
	初級１説明_1:
		{
			In Visuale Soggettiva puoi anche
			sferrare pugni.
		}
#endif
#ifdef d:PSX2
		{
			Prendi la mira nella Visuale Soggettiva e usa
			il tasto direzionale per mirare e sparare a 
			punti specifici - i punti deboli del nemico e 
			altri punti a varie altezze che normalmente 
		}
	初級１説明_1:
		{
			non riusciresti a colpire. In Visuale Soggettiva 
			puoi anche sferrare pugni. 
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
			Senza armi equipaggiate, usa la levetta
			sinistra e il tasto Armi accanto a un
			nemico per scaraventarlo. A volte ci&`o fa
			perdere i sensi al nemico.
		}
#endif
#ifdef d:PSX2
		{
			Usa la levetta analogica sinistra e il tasto Armi
			vicino al nemico (senza arma) per scagliare 
			il nemico. Talvolta potrai far perdere i sensi
			al nemico. 
		}
#endif
/*
			Use left analog stick and □ button near
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
			Muovi la levetta sinistra verso una parete 
			accanto a un angolo per entrare in Visuale 
			Angolare. Puoi stare nascosto e sbirciare 
			da dietro l'angolo. In Visuale Angolare 
		}
	初級３説明_1:
		{
			muovi la fotocamera con la levetta destra.
		}
#endif
#ifdef d:PSX2
		{
			Immetti la levetta analogica sinistra verso il
			muro vicino all'angolo per passare alla visuale
			angolare. Puoi rimanere nascosto e guardare
			dietro l'angolo. 
		}
	初級３説明_1:
		{
			Puoi muovere la telecamera usando la levetta
			analogica destra nella visuale angolare. 
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
			Premi il tasto Azione per arrampicarti sugli
			oggetti fino all'altezza della vita. Non
			puoi arrampicarti su oggetti troppo alti.
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
			Premi il tasto Strisciare mentre corri per rotolare
			in avanti. Efficace per saltare da un oggetto
			all'altro. Tieni premuto il tasto Strisciare mentre
			rotoli per iniziare a strisciare alla fine.
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
			Premi il tasto Armi vicino a un nemico svenuto/
			morto (senza arma) per sollevare il corpo.
			Tieni premuto il tasto Armi per trascinare
			il nemico. 
		}
	初級６説明_1:
		{
			Efficace per nascondere il corpo
			e non farsi scoprire.
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
			Premi il tasto Azione vicino a un corrimano per
			saltare e sospenderti per sfuggire al nemico.
			Puoi spostarti lateralmente e anche passare
			alla Visuale Soggettiva.
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
			Mettiti davanti all'armadietto e premi il tasto
			Azione per aprirlo. Ripremi il tasto Azione
			davanti all'armadietto aperto per chiuderlo.
			Puoi aprire l'armadietto anche se ci stai
		}
	初級８説明_1:
		{
			appoggiato contro.
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
			Se ti nascondi sotto una scatola di cartone e
			rimani fermo, il nemico ti ignora. Se ti muovi
			e lui ti vede, ti trover&`a. Se blocchi la strada al
			nemico mentre sei dentro la scatola, ti farai
		}
	中級１説明_1:
		{
			scoprire.
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
			Senza armi equipaggiate, avvicinati a un
			nemico dalle spalle e premi il tasto Armi, 
			senza usare la levetta sinistra. Premi
			rapidamente il tasto Armi per mettere k.o. il 
		}
	中級２説明_1:
		{
			nemico. Per trascinare il nemico, tieni 
			premuto il tasto Armi.
		}
#endif
#ifdef d:PSX2
		{
			Muoviti alle spalle del nemico senza un'arma
			e premi il tasto Armi (senza levetta analogica
			sinistra) per strangolare il nemico. Premi in
			rapida successione il tasto Armi per eliminare 
		}
	中級２説明_1:
		{
			il nemico. Per trascinare il nemico,
			tieni premuto il tasto Armi 
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
			Premi il tasto Passo destro/sinistro in
			Visuale Angolare per sbirciare da un angolo.
			Se ti inclini obliquamente, un nemico ti
			potrebbe vedere. Fai attenzione.
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
			Premi il tasto Passo destro/sinistro in Visuale
			Soggettiva per muoverti obliquamente.
			Premi i tasti Passo destro e sinistro insieme
			per allungarti. Efficace mentre ti ripari dietro
		}
	中級４説明_1:
		{
			oggetti durante le sparatorie.
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
			Appoggiati al muro e premi il tasto Pugno
			per battere contro il muro. Fai rumore per
			attirare un nemico che non sembra mai
			voler lasciare la sua postazione.
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
			Se rotoli contro un nemico, lo potrai far
			cadere a terra. Efficace se sei circondato
			da nemici.
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
			Se lasci un libro lungo il percorso dei nemici,
			attirerai la loro attenzione. Non efficace nelle
			modalit&`a Fuga e Allerta.
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
			Puoi deselezionare le armi equipaggiate e 
			gli oggetti premendo rapidamente il tasto del 
			menu Oggetti o Armi. Premilo di nuovo per 
			riequipaggiare. Torna utile con 
		}
	中級８説明_1:
		{
			equipaggiamenti in Visuale Soggettiva
			come il Mirino.
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
			Nella Visuale Angolare, equipaggia l'arma e
			premi il tasto Armi per balzare fuori da dietro
			l'angolo e puntare l'arma. Puoi attaccare
			velocemente da dietro un oggetto.
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
			Arriva fino alle spalle del nemico senza farti
			scoprire e punta la pistola premendo il tasto Armi
			per far alzare le mani al nemico. Se miri ai suoi
			punti deboli, il nemico potrebbe darti un oggetto.
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
			Quando stai strangolando un nemico e lo tieni
			in ostaggio, gli altri nemici esiteranno a sparare.
			A quel punto puoi quindi fuggire.
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
			Punta le armi premendo il tasto Armi.
			Per muoverti mentre punti l'arma, premi
			entrambi i tasti Armi e Aggancio.
			Mentre ti muovi puoi anche sparare.
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
			Premi il tasto Armi vicino a un nemico svenuto /
			morto per sollevare il corpo. Rilascia il tasto Armi
			per posare a terra il corpo. Ripeti per scuotere
			il corpo e  talvolta il nemico lascia cadere
		}
	上級５説明_1:
		{
			una scatola con un oggetto.
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
			Il nemico usa la radio (sul fianco destro)
			per chiedere rinforzi quando trovarli. Se
			spari alla radio prima che possa farlo,
			puoi impedire l'arrivo dei rinforzi.
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
			Spara agli estintori e fuoriusciranno
			violentemente sostanze chimiche. Usalo
			per disorientare i nemici.
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
			Premi il tasto Strisciare in modalit&`a Sospensione
			per saltare gi&`u. Se atterri in testa al nemico, puoi
			tramortirlo.
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

