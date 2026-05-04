/*
	tales_res_a_j.h
	    スネークテイルズ紙芝居用リソースヘッダ

	2002/06/04 S.Mukaide
	$Id: tales_a_e.rsc,v 1.14 2002/09/05 07:03:56 usr03005 Exp $


*/

// フォントチェック用define
/*
-fontcheckは省略可能です。５つのパラメータは順に 
width, height, chara_skip, line_skip, flag　をあらわします。
flagが1の場合は、禁則を行わない。(日本語以外)

-fontcheck width height chara_skip line_skip flag
*/
//#define	TITLE_FONT_AREA		-fontcheck 465 120 0 12 0 


//---------------------------------------
//	テイルズＡ(デブが暴れる話)
//---------------------------------------

resource テイルズ＿Ａ__e d:TITLE_FONT_AREA {

//最初
//★Ａ１ {
/*
ENGLISH!!
海中からの『ビッグ・シェル』潜入に無事成功したスネークは、Ａ脚底部の昇降機に乗り込んだ。キャンベルとのブリーフィングを反芻する。
今から６時間前、『ビッグ・シェル』が重武装した過激派グループに占拠された。テロリストは視察に訪れていた大統領と施設職員を人質にとり、政府を脅迫している。要求は２０項目に及ぶ戯れ言と現金３０億ドル。キャンベルは告げた。

「君に依頼する任務は二つ。海上除染施設『ビッグ・シェル』に単身潜入し大統領と人質を救出すること、テロリスト達を武装解除することだ。そのためにはあらゆる手段を用いて構わない。協力者と共に事態の収拾に当たってくれ」
「協力者？」
「シークレット・サービスのリチャード・エイムズが、テロリストの手を逃れているらしい」
聞き覚えのある名にスネークは驚いた。
「エイムズ？ナスターシャの旦那か？」
キャンベルが訂正する。
「元、旦那だよ。我々に出動を要請してきたのも彼だ。だが連絡手段が破壊されたらしく、今は連絡が取れない。彼と直接接触して情報を入手してくれ。最後の連絡はＦ脚からだった。今もＦ脚のどこかに隠れているはずだ。まずはＦ脚に向かってくれ」
｜
昇降機が上昇を始めた。スネークは水中装備を外しながら、キャンベルからの指令をもう一度検討した。エイムズは敵から身を隠しているはずだ。接近してもスネークが敵に追われているようだったら姿を現しはしないだろう。Ｆ脚に辿り着いたら敵に発見されないように気を付けながらエイムズを探さねばならない。Ｆ脚はこのＡ脚の北東だ。
*/

	A1_スタート: << EOF
He was in. Snake's underwater 
infiltration of Big Shell was complete. 
As he entered the elevator in the lower 
part of Strut A, he paused to reflect on 
his briefing with Campbell.
Six hours earlier, Big Shell had been 
taken over by a group of heavily armed 
extremists. The terrorists had kidnapped 
the President, who was visiting Big 
Shell on an inspection tour, and taken 
him hostage along with several 
employees of the facility. Now they were 
using the hostages to blackmail the 
government. The terrorists' demands 
were a joke: a list of 20-odd trivialities, 
plus $30 billion in cash.
"Your mission objectives are as follows," 
Campbell had said. "First, infiltrate the 
ocean cleanup facility known as 'Big 
Shell' and rescue the President and the 
other hostages. Second, neutralize the 
terrorists. You are authorized to use 
any means necessary to complete this 
mission. Your contact will be able to fill 
you in with more details later."
"Contact?"
"One of the President's Secret Service 
agents, a man by the name of Richard 
Ames, seems to have eluded the 
terrorists."
The name triggered something in 
Snake's brain. He'd heard it before.
"Ames? Isn't he Nastasha's husband?"
"Ex-husband," Campbell corrected him. 
"He's also the one who requested our 
help in the first place. But whatever 
means he was using to communicate 
with us has apparently been destroyed, 
and we can't get a hold of him any 
more. You've got to get in touch with 
Ames directly and find out anything you 
can about the situation inside. His last 
transmission came from Strut F, and he 
should still be hiding somewhere in that 
area. That's where you should head 
first."
｜
The elevator began its ascent. As Snake 
took off his diving equipment, he went 
over Campbell's last directive one more 
time in his head. Ames was supposedly 
hiding from these terrorists. Even if 
Snake managed to get close to him, 
Ames wouldn't come out of hiding if 
Snake were being followed. Once he 
arrived at Strut F and started looking 
for Ames, he'd have to be extra careful 
to avoid being detected by the enemy. 
Strut F was located northeast of Strut A.#C0
EOF
//}


//★昇降機に乗ると任務放棄と見なしてゲームオーバー {
/*
ENGLISH!!
昇降機が下降していく。戦場が遠ざかっていくのを感じながらスネークは煙草をくわた。火を付ける。無線機が鳴った。
「こちらスネーク」
「スネーク！一体何を考えている！？」
キャンベルの怒鳴り声が響いた。
「任務放棄か！？そんな男だとは思わなかったぞ！」
「大佐......」
「黙れ！もう君には頼まん！！」
通信は途絶え、キャンベルが応えることはもうなかった。下降する昇降機の中で、スネークは独りゆっくりと紫煙をくゆらせた。#C0
*/

	A_昇降機乗ってゲームオーバー: << EOF
The elevator began to descend. Snake 
took out a cigarette and lit it, feeling the 
battlefield slowly recede into the 
distance behind him. Just then, the 
Codec sounded.
"This is Snake."
"Snake! What the hell do you think 
you're doing?"
Campbell's furious voice echoed in 
Snake's head.
"Colonel -- "
"Shut up! I never should have trusted 
you!"
With that, the transmission cut off 
abruptly. That was the last Snake heard 
from Campbell. He stood alone in the 
elevator, watching the purple smoke 
curl around him in silence.#C0
EOF
//}


//Ｆ脚でエイムズと遭遇
//★Ａ２ {
/*
ENGLISH!!
スネークは男に呼びかけた。
「あんたがエイムズだな」
エイムズがうなずいた。
「君がソリッド・スネークだな？」
スネークが眉をひそめる。エイムズはその疑問を先回りした。
「来るのは君だと思っていた。最高の男を寄越すよう要請したからな。例のタンカーでの働きは見事だった」
スネークは顔をしかめた。
「テロリストを皆殺しにしたことが、か？」
苦い記憶が甦る。累々と横たわる少年兵達の死体。船内に充満する血の匂い。
「人質は全員救出したじゃないか」
同じような言葉は何度も聞いた。だが忌まわしい記憶を追い払う役には立たない。スネークは本題に入った。
「大統領はどこだ？」
エイムズは目をそらした。
「......不明だ」
「他の人質達は？」
「わからん」
スネークはため息をついた。
「何も情報はないというのか？」
「テロリストのリーダーを見た」
「誰だ？」
「ファットマン」
スネークは記憶を辿った。すぐにその名に行きつく。
ファットマン。ＳＥＡＬＳ崩れの爆弾魔。自称爆弾王。思想も信条もなく、金次第で右にも左にもつき、どんな神の手助けもするプロの犯罪者。しかしファットマンは３年前、教会を爆破した事件で逮捕されたと聞いていた。エイムズが吐き捨てるように答える。
「無罪になったんだよ」
「３０人以上を爆殺しながらか？」
エイムズが壁を蹴った。
「陪審が無罪評決を出したんだ。合理的疑いを超えていないとかなんとかな。奴には多くのスポンサーがいる。優秀な弁護士も雇える。奴は自由を買った。人殺しで得た金でな！」
エイムズは殺された人間の尊厳が無視されたことを嘆いた。正当な裁きが与えられるべきだったと怒った。そうしていればこのテロも起こらなかったと主張した。スネークはエイムズが落ち着くのを待って聞いた。
「それで、奴はどこに？」
「......ヘリポートで見た。今もそこから指揮をとっているはずだ」
「Ｅ脚の屋上だな」
立ち去ろうとするスネークをエイムズは呼び止めた。
「奴を無傷で拘束しようなどとは思わない方が良い。どんな手を使ってくるかわからんぞ。大統領の身も危ない」
エイムズはＵＳＰを取り出した。スネークをまっすぐ見据えて言う。
「必ず、殺すんだ」
エイムズはスネークにＵＳＰを手渡した。さらにいくつかのアイテムを取り出す。
「これも持って行ってくれ。ここへ逃げ込む途中でかき集めた。ファットマンを殺す時の役に立つはずだ」
冷却スプレー、爆弾の匂いを検知するセンサーＡ、レベル１のセキュリティカード。スネークはそれらを受け取り装備に加えた。
「わかった。ここから先は俺の仕事だ」
「頼む。私はしばらくここに隠れていることにする」
「それがいい」
スネークは部屋を出た。背後で扉がロックされる音が聞こえた。言葉通り、エイムズはここに閉じこもるつもりなのだろう。無線機が鳴った。キャンベルからの呼び出しだった。
「スネーク、人質の居所がわかったぞ」
「どうやって？」
「電子メールだ」
「電子メール？」
「携帯電話から政府筋へメッセージを送ってきている人物がいる。ジェニファーというシークレット・サービスだ」
「人質はどこに？」
「シェル１中央棟のＢ１らしい」
「テロリストのリーダーの居場所もわかった。人質とどちらを優先する？」
「それは君の判断に任せよう。頼んだぞ」#C0
*/
	A2_エイムズ遭遇: << EOF
Snake addressed the man.
"You must be Ames."
Ames nodded. "And you must be Solid 
Snake."
Learning to read Snake's shrugs, Ames 
took this as a "yes".
"I knew they'd send you. After all, 
I asked for the best man they had. That 
was quite an impressive job you pulled 
on that tanker a few years ago."
"You mean slaughtering those terrorists 
to the last man?" He felt the painful 
memories resurfacing in his mind -- 
scores of fresh, young soldiers lying 
dead in heaps on the deck, the acrid 
smell of blood permeating every corner 
of the ship....
"But you managed to get all the 
hostages out safely, didn't you?" 
asked Ames.
Snake had heard these words countless 
times. They never helped ease the 
pain. They never would. Snake did not 
feel like explaining that.
"Where is the President?" he said, 
getting to the point.
Ames looked away.
"I...don't know."
"What about the other hostages?"
"...I'm not sure."
"Are you telling me you don't know 
anything?"
"I've seen the leader of the terrorists."
"Who? Who is it?"
"Fatman."
Snake searched his memory. It didn't 
take long for him to trace the name.
Fatman. A disgraced former Navy SEAL 
turned mad bomber. The self-styled 
"Emperor of Explosives." A man with no 
philosophy or creed. A hardened criminal 
who'd sell himself to anyone for the 
right price. But hadn't he been arrested 
three years earlier after that 
church-bombing incident?
"He was acquitted," Ames spat in 
disgust, as if anticipating Snake's 
question.
"Even though more than thirty people 
died in the blast?"
"The jury found him not guilty. They said 
there was still a 'reasonable doubt' as 
to whether he had done it or not. 
Fatman's got a lot of rich and powerful 
friends who can afford to hire the best 
defense attorneys in the business. He 
bought his freedom -- with the same 
blood money he made from killing those 
innocent people!"
Ames' mouth twitched. Maybe he was 
going to grin.
"If he was judged correctly, this would 
not have happened.... In other words, 
the jury's mistake is about to kill us, 
and their President."
Ames turned around toward Snake.
"Snake, rescue the President. I know 
Fatman was at the heliport. He's 
probably still giving orders there now."
"The heliport...On the roof of Strut E, 
right?"
As Snake nodded and got up to leave, 
Ames stopped him.
"Don't waste your time trying to take 
him alive. He's got too many tricks up 
his sleeve. And the President's life is 
still in danger."
Ames took out his USP and looked 
straight into Snake's eyes.
"Promise me you'll kill him."
He handed the USP to Snake. Then he 
produced a few more objects from his 
jacket.
"Here, you'd better take these along 
with you, too. I picked them up on my 
way here. They should come in handy 
when you're taking care of Fatman."
Coolant spray, the bomb-detecting 
Sensor A, and a Level 1 security card. 
Snake took the items and added them to 
his equipment.
"All right. Leave the rest to me."
"I'm counting on you. I think I'll just 
stay here for a while."
"Sounds like a good plan to me."
Snake left the room. He heard a "click" 
as the door locked behind him. True to 
his word, Ames was planning to hole up 
in that room. 
The Codec sounded. It was Campbell.
"Snake, we've found out where the 
hostages are being held."
"How?"
"By e-mail. Someone in there is using 
their cell phone to send messages to 
the government -- a Secret Service 
agent named Jennifer."
"Where are the hostages?"
"She says they're in the Shell 1 Core, 
on level B1."
"I found out where the terrorist leader 
is, too. What's my priority here: 
the leader or the hostages?"
"I'll leave that up to you, Snake. 
Good luck."#C0
EOF
//}

/*
//ジェニファー無視してファットマンと対決
//分岐：AF1_0→AF1→AF2
//ジェニファー無視してファットマンと対決 親リソース {
	AF1_ジェニファー無視してファットマンと対決: << EOF
#C1#C2#C3
EOF
//}
*/

//ファットマン登場
//★ＡＦ０ {
/*
ENGLISH!!
コンテナの後ろから巨大な影が現れた。ファットマンだ。エイムズの情報は正しかった。ファットマンがスネークに向かって叫ぶ。
「大統領はどこだ！？」
*/
AF1_0: << EOF
A huge shadow appeared from behind 
the crate -- Fatman. Ames had been 
right. Fatman turned to face Snake and 
shouted:
"WHERE IS THE PRESIDENT?!"#C0
EOF
//}

//ジェニファーと会わずにファットマンのところへ言った場合
//★ＡＦ１ {
/*

ENGLISH!!
スネークは内心安堵した。奴等もまだ大統領を見つけてはいないらしい。
ファットマンが巨体を揺すりながら言う。
「貴様が例の男だろう？大統領を渡せ！」
どういうことか。既に大統領を逃がしていた人間がいるということなのか？だが当惑を悟られぬよう、スネークは虚勢を張った。
「断る！」
*/
AF1_1: << EOF
Snake felt a wave of relief wash over 
him. So they hadn't found the President 
yet, either.
"You're that guy who's been sneaking 
around here, aren't you?" Fatman's 
enormous body trembled as he spoke.
"Hand over the President!"
What was he talking about? Had 
someone already helped the President 
escape? Snake tried to mask his 
bewilderment with a bluff.
"Not a chance!"#C0
EOF
//}

//★ＡＦ２ {
/*

ENGLISH!!
ファットマンが右手に握った何かを突き出す。
「爆弾の無線式起爆装置だ。大統領を渡さなければ人質の命はない」
本気だ、とスネークは思ったが答えようがなかった。その沈黙を見て、ファットマンは凄惨な笑みを浮かべた。
「よくわかった」
右手を高く掲げる。スネークは叫んだ。
「よせ！」
ファットマンはスイッチを押した。
爆音が轟く。『ビッグ・シェル』全体が揺れた。振り返ったスネークは中央棟から黒煙が上がるのを見た。ファットマンの嘲笑が響く。
「これで皆死んだ。全て貴様の責任だ」
「ファットマン......！」
「よし、今から貴様に少しだけ時間をやる。後悔する間もなく殺されたくはないだろうからな。１、２......」
指を突きたてながら数を数える。５まで数えてファットマンは叫んだ。
「充分後悔したか？では死ね！」
*/
AF1_2: << EOF
Fatman thrust out his right hand to 
reveal a small object with a switch. 
"This is a remote triggering device for a 
bomb. Hand over the President or the 
hostages die."
He's not bluffing, thought Snake as he 
searched frantically for a response. 
Noticing Snake's silence, a gruesome 
smile spread across Fatman's grotesque 
features.
"Well, then..."
He raised his right hand into the air.
"Stop!" yelled Snake.
Fatman pushed the switch.
An explosion rang out in the distance, 
causing the entire Big Shell to shake 
violently. Whirling around, Snake saw a 
thick plume of black smoke rising from 
the Shell 1 Core. Fatman's cackles 
echoed in his head.
"They're all dead now. And it's all your 
fault."
"Fatman..."
"OK, now I'm gonna give you a few 
seconds to reflect on what just 
happened. I don't want to kill you 
without giving you a chance to mourn 
for those poor, innocent people, now. 
1... 2..."
Fatman lifted his fingers one by one 
as he counted.
"... 3... 4... 5! Are you finished? 
'Cause it's time to die!"#C0
EOF
//}

/*
//ジェニファー会わずにファットマン倒した
//分岐
//ファットマン殺した場合：AF2_1A→AF2_1_2→AF2_2→AF2_3→AF2_4→AF2_5A→AF2_6
//ファットマン気絶させた場合：AF2_1B1→AF2_1B2→AF2_2→AF2_3→AF2_4→AF2_5B→AF2_6
//ジェニファー会わずにファットマン倒した親リソース
AF2_ジェニファー会わずにファットマン倒した: << EOF
#C1#C2#C3#C4#C5#C6#C7
EOF
//}
*/

//ファットマン殺した場合
//★ＡＦ２－１Ａ {
/*
ENGLISH!!ファットマンは倒れた。スネークはその亡骸の側に立ち尽くした。中央棟ではたくさんの人質達が同じような姿で横たわっているはずだ。
*/
AF2_1A: << EOF
Fatman fell with a sickening thud. 
Snake stood silently over his bloated 
corpse. No doubt dozens of hostages 
were lying in a similar fashion over in 
the Shell 1 Core.#C0
EOF
//}

//ジェニファーと会っている場合
//★ＡＪＢ２ {
/*
ENGLISH!!
早く助けに来てね、と見上げるジェニファーの姿が脳裏に浮かぶ。
*/
AJB2: << EOF
Snake recalled Jennifer's last words to 
him before he left: "Hurry back, okay?"#C0
EOF
//}

//★ＡＦ２－２ {
/*
ENGLISH!!
......他に方法はなかったのだろうか。
*/
AF2_1_2: << EOF
"Why did things turn out this way?" 
Snake wondered. "Everything went 
wrong.... No, I'm the one who went 
wrong. There must have been some 
other way."#C0
EOF
//}

//ファットマン気絶させた場合
//★ＡＦ２－１Ｂ１ {
/*
ENGLISH!!
ファットマンは倒れた。かすかなうめき声が上がる。死んではいない。スネークはその巨体の傍らに立ち尽くした。中央棟ではたくさんの人質達が同じように横たわっているのだろう。ただし血を流して。息絶えた姿で。
*/
AF2_1B1: << EOF
A soft moan escaped Fatman's lips as 
he fell to the ground with a sickening 
thud. He'd live. Snake stood silently 
next to the bloated body. No doubt 
dozens of hostages were lying in a 
similar fashion over in the Shell 1 Core.
But those bodies would be lying bloody 
and broken, never to rise again.#C0
EOF
//}


//★ＡＦ２－１Ｂ２ {
/*

ENGLISH!!
｜
背後から声がした。
「なぜ殺さない？」
エイムズだった。手には銃が握られている。
「君がやらないなら......！」
エイムズが倒れたファットマンを狙った。銃声。血しぶきが上がる。ファットマンの体が震える。さらに撃つ。巨体が跳ね上がる。憑かれたように撃ち続ける。銃声が轟く。撃ち尽くした弾倉を入れ替え、また構える。
スネークはエイムズの手を抑えた。
「もう死んでる」
エイムズはようやくそれに気づいたようだった。スネークは言った。
「仲間の仇、か？」
エイムズは答えない。
「俺も撃った方がいいんじゃないか？」
「そうかもしれんな」
だがエイムズは銃を下ろした。
*/
AF2_1B2: << EOF
A voice called out from behind Snake's 
back.
"Why didn't you kill him?"
It was Ames. He was clutching a gun in 
his hand.
"If you won't do it...!"
Ames took aim at Fatman's unconscious
body and pulled the trigger. The first 
shot rang out. Fatman's body lurched, 
spraying blood across the floor. Another 
shot -- the body seemed to leap into the 
air. Ames continued to unload shot 
after shot into the corpse, as if 
possessed by some unseen force. 
The sound of gunfire resounded across 
the rooftop. He emptied one clip, 
reloaded, and prepared to fire again. 
Snake grabbed his hand.
"He's dead."
Ames finally came to his senses.
"Was that payback for someone you 
lost?" Snake asked.
Ames was silent.
"Then you'd better shoot me, too."
"Yeah, maybe you're right."
But Ames let the gun drop to his side.#C0
EOF
//}

//★ＡＦ２－２ {
/*
ENGLISH!!
｜
無線機が鳴った。キャンベルからの連絡だった。大統領は沿岸警備隊が救出したという。#C0
*/
	AF2_2: << EOF
The Codec sounded. It was Campbell. 
He reported that the Coast Guard had 
the President safely in their custody.#C0
EOF
//}

//ジェニファーと会っていない場合のみ
//★ＡＦ２－３ {
/*
ENGLISH!!
既に小型艇で『ビッグ・シェル』を脱出していたらしい。#C0
*/
	AF2_3: << EOF
He'd already managed to escape from 
Big Shell in a patrol boat.#C0
EOF
//}

//★ＡＦ２－４
/*
ENGLISH!!
そして、つい先ほど『ビッグ・シェル』にＳＥＡＬＳが突入した。残ったテロリストの制圧と同時に、爆破された中央棟からの人質救出も行われるという。だがその試みは無駄に終わるだろうとスネークは知っていた。
キャンベルが言う。
「エイムズの姿が見当たらないらしい。彼の行方を知らないか？」
*/
	AF2_4: << EOF
Moreover, the SEALs had just entered 
the facility. They were going to clean up 
the rest of the terrorists and rescue any 
hostages who were still alive in the 
wreckage of the Shell 1 Core. Snake 
knew, though, that their search would 
be fruitless.
"There's no sign of Ames anywhere," 
said Campbell. "Do you have any idea 
where he might have gone?"#C0
EOF
//}

//★ＡＦ２－５Ａ {
/*
ENGLISH!!
「いや......」
*/
	AF2_5A: << EOF
"Negative..."#C0
EOF
//}

//★ＡＦ２－５Ｂ {
/*

ENGLISH!!
振り返るとエイムズの姿は消えていた。スネークは答えた。
「いや......」
*/
	AF2_5B: << EOF
Snake turned around. Ames was gone.
"Negative..."#C0
EOF
//}

//★ＡＦ２－６ {
/*
ENGLISH!!
「そうか......」
キャンベルは他にも慰めらしきことを言っていたようだが、スネークには何も聞こえなかった。
*/
	AF2_6: << EOF
"I see..."
Campbell's reassuring voice offered 
encouragement, but Snake wasn't 
listening. His ears were already deaf to 
the world.#C0
EOF
//}

//人数合わせ用空リソース
	A_空: << EOF
#C0
EOF
//}


//人質部屋にてジェニファーを探せ
//分岐：
//人質部屋に入ったのが初めてでかつマイクを持っている場合、AM_0→AM_1B→AM_2。以後出入りしても紙芝居発生ナシ。
//人質部屋に入ったのが初めてでかつマイクを持っていない場合、AM_0→AM_1A→A_空。
//その後マイクを取って人質部屋に入った一回目にAM_1C→AM_2→AM_空。

/*
//人質部屋説明親リソース {
	AM_人質部屋イベント説明: << EOF
#C1#C2#C3
EOF
//}
*/

//★ＡＭ－０ {
/*
ENGLISH!!
スネークは集会場に入った。中には２０人以上の人質がいる。テロリストが巡回し、時折定時連絡を入れていた。異状が起きたとなれば、すぐさま増援が殺到してくるだろう。そうなれば逃げ場はない。敵に悟られずジェニファーへ接触するしかないようだ。問題はどうやってジェニファーを見分けるかということになる。スネークはキャンベルを無線で呼び出した。
「大佐、ジェニファーに身体的特徴を聞いてみてくれないか」
「それは無理だ」
「どうして？」
「ジェニファーは拘束されたまま後ろ手でメールを打っているんだそうだ。こちらからメールを送っても見ることは出来ない」
舌打ちするスネークにキャンベルは少し考えてから言った。
「そうだな......メールを打つ音がするのがジェニファーじゃないか？音を聞いてみろ」
「音か......」
*/
	AM_0: << EOF
Snake entered the assembly hall. 
There were the hostages -- more than 
twenty of them by the looks of it. 
Terrorist guards patrolled the area, 
checking in every so often by radio. 
If they got even a whiff of anything 
suspicious, the whole place would be 
flooded with reinforcements in no time. 
Once that happened, there'd be nowhere
to run. Snake would somehow have to 
get in touch with Jennifer without being 
detected. There was, however, another 
problem -- how to tell which hostage was
Jennifer? Snake called Campbell on the 
Codec.
"Colonel, can you tell me if Jennifer has 
any distinguishing features?"
"Sorry, I'm afraid I can't help you there."
"Why not?"
"Jennifer is apparently sending us 
e-mail with her hands tied behind her 
back. You wouldn't be able to tell 
which one is her even if we sent her 
mail from here."
Snake clicked his tongue in frustration. 
Campbell thought for a minute.
"I've got it. If you hear the sound of 
someone punching the keys on a 
cell phone, that person must be 
Jennifer. Listen for that sound."
"The sound of keys on a cell phone..."#C0
EOF
//}

//マイクない場合
//★ＡＭ－１Ａ {
/*
ENGLISH!!
「中央棟のどこかにマイクがあるはずだ。まずは集音マイクを探すんだ」#C0
*/
AM_1A: << EOF
"There should be a directional 
microphone somewhere in the Shell 1 
Core. Look for it before you go in."#C0
EOF
//}

//マイクある場合
//★ＡＭ－１Ｂ {
/*
ENGLISH!!
「スネーク、集音マイクを使うんだ」
*/
AM_1B: << EOF
"Snake, use the directional microphone."#C0
EOF
//}

//マイクを取って再び人質部屋に戻ってきた場合
//★ＡＭ－１Ｃ {
/*
ENGLISH!!
集会場に入ったスネークの元にキャンベルからの無線連絡が入った。
「スネーク、集音マイクを手に入れたな。マイクを使ってジェニファーを見つけるんだ」
*/
AM_1C: << EOF
As Snake entered the assembly hall, 
Campbell's voice sounded on the Codec.
"Good, you found the microphone. 
Now use it to find Jennifer."#C0
EOF
//}

//★ＡＭ－２ {
/*

ENGLISH!!
キャンベルは続けた。
「メールを打つ音がするのがジェニファーだ。音を聞いてジェニファーだと思ったら、マイクをその人質に向けたままアクションボタンを押して呼びかけろ。その人質が本当にジェニファーだったら、何らかの反応を示すはず」
「もし間違ったら？」
「物音を聞きつけて敵がやってくるかもしれんな。うかつに呼びかけない方がいいだろう。ジェニファーにアクションボタンで呼びかけるのは、その相手がジェニファーだと確信出来てからにしろ」#C0
*/
AM_2: << EOF
Campbell continued, "Jennifer is using 
her cell phone to send us e-mail. If you 
hear the sound of someone punching 
keys on a cell phone, turn towards the 
nearest hostage and press the Action 
Button to talk to that person.If it really 
is Jennifer, she should give you some 
sort of response."
"What if I'm wrong?"
"The enemy might hear the noise and 
come to investigate. Don't talk to just 
anyone. Don't press the Action Button 
until you're absolutely sure that it's 
Jennifer you're talking to."#C0
EOF
//}


//ジェニファー接触時
//★ＡＪ１ {
/*
ENGLISH!!
ジェニファーが何か言おうとする。スネークは素早く手を伸ばし、その口を塞いだ。
「騒がないでくれ。敵に気づかれる」
周囲を警戒しながら目隠しを外してやる。あらわになった茶色の瞳が嬉しそうに輝いた。
「悪いが、先に言っておく。今、俺の最優先任務は行方不明の大統領を見つけることだ。あんた達を助けるのは、彼の身柄を確保してからになる」
ジェニファーが眉をひそめる。
「まずは知っていることを教えてほしい。わかったらうなずいてくれ」
ジェニファーがうなずく。スネークは彼女の口から手を離した。
「どんな状況だ？」
「そうね、縛られて転がされてもう駄目かってところに、やっと助けが来たと思ったら、そいつはとんだ冷血野郎で、今はすっごくムカついてるってところかしら」
スネークは返答に窮した。
「そうではなくて......」
「冗談よ。大統領でしょ。脱出したわ」
ジェニファーはあっさりと言ってみせた。
「何？」
「とっくに脚底部から小型艇でここを離れてるはずよ。襲われた時すぐにシークレットサービスの仲間の一人が連れ出したの」
「......優秀な同僚だな」
「でしょ？」
ジェニファー自慢げに微笑んだ。
「まあ、私達はお見送りする前に捕まっちゃったんだけどね......」
「他に情報は？」
「爆弾が仕掛けられてる」
さらりと答えたジェニファーにスネークが聞き返す。
「爆弾だって？」
「ええ。私達が逃げ出したりしたら爆破するって」
「どこに仕掛けたと？」
「この中央棟のどこかみたいな口振りだった」
スネークはうなずいた。
「わかった。何とかする」
見張りのテロリストが仲間と連絡をとっているのが聞こえてきた。ここの警備を強化するつもりらしい。長居は出来そうにない。
「いまのところ、奴等に人質を害する気はなさそうだ。もうしばらく捕まったフリをしていてくれないか」
ジェニファーが口をとがらせる。
「後で助けに来る」
「......いいわ。レベル２のセキュリティカードが上着に入ってるから持っていって」
スネークは上着をまさぐった。ジェニファーが声を上げる。
「ちょっと、どこ触ってるの！？......そう、もうちょっと下......そこ、そこ」
レベル２のカードキーを手にしたスネークへジェニファーは言った。
「早く助けに来てね」
「わかった」
ジェニファーにもう一度目隠しをして、スネークは集会場を出た。#C0
*/
	AJ1_ジェニファー救出: << EOF
Jennifer looked as if she were about to 
say something. Snake swiftly reached 
out and covered her mouth with his 
hand.
"Keep your voice down. They'll hear us."
Snake removed her blindfold, keeping 
an eye out for approaching guards. 
Upon seeing Snake's face, Jennifer's 
clear brown eyes glowed with joy. 
But now was not the time to grant her 
wish. Snake said to her,
"I don't mean to disappoint you, but my 
first priority is to rescue the President. 
You'll just have to be patient until I get 
him to a safe place."
Jennifer's shoulders sagged.
"First, though, I need you to tell me 
everything you know. If you understand 
me, nod your head."
Jennifer nodded. Snake lifted his hand 
from her mouth.
"What's going on here?"
"Well, when they tied me up and 
dumped me here on the floor, I thought 
I'd never get out alive. Then this guy 
came to rescue me. But he turned out 
to be a cold-blooded jerk, and now I'm 
really mad."
Snake was at a loss for words. "I... 
that's not..."
"Just kidding. You're looking for the 
President, right? He's already gone," 
said Jennifer, almost casually. She 
continued while enjoying Snake's 
puzzled look,
"He escaped through the bottom of one 
of the struts. He should be on a boat 
back to Manhattan right about now. One 
of my fellow agents managed to sneak 
him out as we were being attacked."
"Are you sure?"
"Yes. Pretty good, aren't I?" Jennifer 
beamed with pride. "Too bad we got 
caught before we could see him off....
So, that's that. Maybe it's about time 
for you to save me."
If the President had escaped safely, 
saving the hostages was the new 
mission. Snake nodded.
"...OK. I've gotta take care of that guard 
first."
As Snake rose, Jennifer stopped him.
"That's not a good idea."
"Why not?"
"They supposedly set a bomb in here. 
They said they'd set it off if we tried to 
escape."
Snake made a small grunt. This wasn't 
going to be that easy.
"Do you know where is it planted?"
"As far as I've been able to tell, it 
should be somewhere here in the Shell 
1 Core."
"All right. I'll get to that first."
Just then, he overheard one of the 
terrorist guards talking to his comrades 
on the radio. It sounded like they were 
going to tighten the security in the 
hostage room. He'd have to get out fast.
"The terrorists don't seem to have any 
intention of harming the hostages. For 
now, at least, sit tight and pretend to be 
captured for a little while longer."
"Sure. I've got a Level 2 security card 
hidden in my jacket. Go ahead and take 
it."
As Snake felt through the jacket for the 
card, Jennifer let out a squeal.
"Hey, watch your hands, mister! 
... that's it, just a little lower... there, 
there it is."
As Snake retrieved the level 2 card, 
Jennifer looked into his eyes.
"Hurry back, okay?"
"Don't worry, I will."
Snake put the blindfold back around 
Jennifer's eyes, then made his way out 
of the assembly hall.#C0
EOF
//}

/*
//爆弾無視してファットマン倒した
//分岐
//ファットマン殺した場合：			AF2_1A→AJB2→AF2_1_2→AF2_2→AF2_4→AF2_5A→AF2_6
//ファットマン気絶させた場合：	AF2_1B1→AJB2→AF2_1_2→AF2_1B2→AF2_2→AF2_4→AF2_5B→AF2_6
//倒した {
AJF2_爆弾解体せずにファットマン倒した: << EOF
#C1#C2#C3#C4#C5#C6#C7#C8
EOF
//}
*/


//爆弾解体成功後
//★ＡＪＢ１ {
/*ENGLISH!!
スネークは無線でキャンベルを呼び出した。大統領が既に脱出していたこと、爆弾を冷却処理したことを報告する。キャンベルが言った。
「よくやった。後はファットマンだな。あれだけの数の人質を君一人で脱出させるのは難しい。先にテロリストを制圧した方がいいだろう」
スネークはうなずいた。ジェニファー達のことは心配だが、今はそれが一番の方法だろう。
「エイムズの話によれば、ファットマンはヘリポートらしい」
「Ｅ脚の屋上だな。スネーク、ヘリポートに向かうんだ。ファットマンを倒せ」
「了解」#C0
*/
	AJB1_爆弾解体成功後: << EOF
Snake called Campbell by Codec and 
reported that the President had already 
escaped and that he'd neutralized the 
bombs with the coolant spray.
"Good work," said Campbell. "Now it's 
time to take out Fatman. I don't think 
even you could get all those hostages 
out at once. You'll have to take care of 
the terrorists first."
Snake nodded. He was worried about 
Jennifer and the others, but he had to 
agree with Campbell's assessment. 
"According to Ames, Fatman should be 
at the heliport."
"That's on the roof of Strut E. Snake, 
head for the heliport and take out 
Fatman. Make sure he stays down for 
good this time."
"Roger that, Colonel."#C0
EOF
//}

/*
//じぇにふぁーたんに会って爆弾解体せずにファットマンと会った {
//分岐：AF1_0→AJBF1_1→AF1_2 
AJF1_爆弾解体せずにファットマンに会った: << EOF
#C1#C2#C3
EOF
//}
*/


/*
//爆弾解体してファットマンと対決
//分岐：AF1_0→AJBF1_1→AJBF1_2
//対決 {
AJBF1_爆弾解体してファットマンと対決: << EOF
#C1#C2#C3
EOF
//}
*/


//ファットマン登場（爆弾解体後）ＡＦ０からつながる
//★ＡＪＢ１ {
/*
ENGLISH!!
スネークは答えた。
「お前の手が届かない所だ」
「嘘はためにならんぞ」
「信じる信じないはお前の勝手だ」
*/
AJBF1_1: << EOF
"You'll never find him," answered Snake.
"I don't believe you! You're lying!"
"You go ahead and believe what you 
want."#C0
EOF
//}

//★ＡＪＢF２ {
/*
ENGLISH!!
ファットマンが右手に握った何かを突き出す。
「爆弾の無線式起爆装置だ。大統領を渡さなければ人質の命はない」
スネークは冷静に告げた。
「無駄だ」
ファットマンはスイッチを押した。何も起こらない。何度も押す。何も起こらない。スネークは言った。
「信じる気になったか？」
ファットマンはスイッチを投げ捨て、地団太を踏んだ。
「くそっエイムズめ！高い金取ってオッサン一人捕まえられんのか！？だから素人と組むのは嫌だったんだ！」
ファットマンは紅潮したままスネークに向き直った。
「計画は失敗だ。俺は下りさせてもらう。ただし貴様を始末してからな！」
ファットマンが滑走を始めた。#C0
*/
AJBF1_2: << EOF
Fatman thrust out his right hand to 
reveal a small object with a switch. 
"This is a remote triggering device for 
a bomb. Hand over the President or the 
hostages die."
"It's no use, Fatman," Snake replied 
coolly.
Fatman pushed the switch. Nothing 
happened. He pushed it several more 
times. Still, nothing happened.
"Do you believe me now?" asked 
Snake.
Fatman tossed the switch aside and 
stamped his feet in anger. "That 
bastard Ames! I pay him good money 
and he can't even manage to keep an 
eye on one old man! I never should 
have agreed to work with that amateur 
in the first place!" He turned back to 
face Snake, the color rising to his 
cheeks. "Looks like our little plan has 
failed. I'm getting out of here while the 
getting's good. But first I'll take care of 
you!"
Fatman's skates sprang into motion.#C0
EOF
//}


//★ＡＪＢ３－Ａ０ {
/*
ENGLISH!!
ファットマンは倒れた。巨体の下に血だまりが広がっていく。爆弾王を自称する犯罪者は死んだ。
｜
キャンベルを無線で呼び出し報告する。キャンベルによれば大統領は無事、沿岸警備隊に保護されたらしい。さらに、スネークがファットマンと戦っている間にＳＥＡＬＳが突入、残ったテロリストの制圧を行なっていると言う。
しかしスネークは考え込んだ。話がおかしい。ファットマンはエイムズが大統領を捕まえるはずだと言った。ならばなぜエイムズはスネークに協力しようとしたのか......。
｜
不意に女の声がした。
ジェニファーだった。ＳＥＡＬＳに救出されたと言う。他の人質達も無事だと言う。しかしスネーク自身が助けに来なかったのは不満だと言う。そしてスネークは任務が完了したなら喜ぶべきだと言う。スネークは答えた。
「そんな気にはなれないな。どうにも腑に落ちない......。エイムズを見たか？」
「いいえ？なんで？」
「ファットマンはエイムズがテロリストの一味だと」
ジェニファーは目を丸くした。
「そんなはずないわ！だって、大統領を逃がしたのはリチャードだもの」
「なんだって？」
つじつまが合わない。ファットマンが嘘を言っていたようには思えない。だがエイムズはスネークに協力した。そしてジェニファーはエイムズが大統領を助けたと言う。ファットマンの言葉が嘘でも、ジェニファーの言うことが本当ならば、なぜ最初に会った時、エイムズは大統領が救出されたと言わなかったのか。
黙考するスネークにジェニファーは腹を立てているようだった。
「何考えてるか知らないけど、リチャードは優秀なシークレット・サービスよ。それに彼がテロに協力するなんてありえないわ」
「どうして？」
「......彼は妹さんを爆弾テロで失っているの。３年前に......」
背筋が凍るような感覚があった。単語がつながる。３年前、爆弾テロ、妹、教会、ファットマン。エイムズの言動が脳裏に甦る。ファットマンは無罪になった、と吐き捨てるエイムズ。裁かれるべきだったと主張するエイムズ。必ず殺すんだ、そう言って銃を寄越すエイムズ......。
仮説が組みあがる。エイムズはファットマンに妹を殺された。ファットマンは釈放された。エイムズは怒った。計画を練った。過激派グループと接触した。大統領拉致を提案した。実行部隊としてファットマンを雇わせた。テロを手引きした。大統領は逃がした。警察等には連絡しなかった。テロリスト殺しで悪名高いスネーク達に直接出動要請を出した。『ビッグ・シェル』は処刑場だった。大統領は餌として選ばれた。ファットマンは獲物としておびき出された。スネークは殺し屋として雇われた。
「エイムズは！？奴を見たか！？」
スネークの剣幕にジェニファーはきょとんとした。
「いいえ。まだどこかに隠れてるんじゃない？あの人、結構臆病なとこあるから。でもテロリストが皆捕まったって聞いたらすぐに出てくると思うな......」
出てくるわけがない、とスネークは思った。
その後捜索が行なわれたが、『ビッグ・シェル』はおろか国内のどこにもエイムズの姿はなかったと言う。#C0
*/
AJBF2_A: << EOF
Fatman fell to the ground with a 
sickening thud. A pool of blood began to 
spread under his grotesque body. The 
so-called "Emperor of Explosives" had 
finally met his match.
｜
Snake called Campbell by Codec and 
told him what had happened. Campbell 
reported that the President was safe in 
the custody of the Coast Guard. 
Moreover, the SEALs had entered the 
facility while Snake was fighting Fatman 
and were in the process of cleaning up 
the last remaining terrorists.
Yet something about the whole situation 
seemed odd to Snake. "Fatman said 
that Ames was the one who had 
captured the President. I don't think he 
was lying. But if Ames was involved, 
then why did he ask for my help...?"
｜
Suddenly, Snake heard a woman's 
voice. It was Jennifer. The SEALs had 
rescued her, she said. The other 
hostages were safe as well. But she 
was disappointed that Snake didn't 
come to rescue her personally. She 
asked him if he was happy now that the 
mission was over.
"Yeah, but something's bothering me," 
Snake responded. "Something just... 
doesn't seem right."
"What doesn't seem right?"
"Fatman said that Ames was in league 
with the terrorists. But he -- "
Jennifer's eyes widened. "That's 
impossible! Richard's the one who 
helped the President escape!"
"What are you talking about?"
Something didn't add up. Fatman 
seemed to be telling the truth. But it 
was also true that Ames had cooperated 
with Snake. Now Jennifer was saying 
that Ames had helped the President 
escape. Even if Fatman was lying and 
Jennifer was telling the truth, why would 
Ames neglect to mention that he'd 
already rescued the President?
Snake pondered this riddle in silence. 
Jennifer seemed to take offense at this.
"I don't know what you're thinking, but I 
can assure you that Richard is one of 
our finest agents. There's no way he 
could be working with the terrorists."
"And why's that?"
"Because he lost his sister in a terrorist 
bombing attack. Three years ago..."
A chill ran down Snake's spine. The 
words came together in a rush. Three 
years ago... terrorist bombing... sister... 
church... Fatman. Snake suddenly 
recalled Ames' strange behavior. The 
way he spat as he said that Fatman had 
been acquitted. His passionate 
conviction that the jury did not bring 
justice to Fatman. The look in his eyes 
as he gave Snake his gun and said, 
"Promise me you'll kill him..."
The pieces were all falling into place 
now. Fatman had killed Ames' sister 
and gotten off scot-free. Ames promised 
revenge. He plotted his revenge. He 
approached the extremist group and 
proposed kidnapping the President. He 
hired Fatman to help carry out the job. 
He led the terrorists. He helped the 
President escape, but didn't bother to 
contact the authorities. Finally, he 
called and asked for Solid Snake, the 
notorious terrorist-killer, to intervene 
personally. Big Shell was the execution 
ground. The President was the bait 
used to lure Fatman to his doom. And 
Snake himself was the executioner.
"Where is Ames?! Have you seen 
him?!"
Jennifer looked startled by Snake's 
sudden change in attitude. "No. He's 
probably still hiding somewhere. He's 
actually pretty timid, you know. But I'm 
sure he'll come out once he hears that 
the terrorists have all been captured."
Snake knew Ames wasn't coming out.
Later, they conducted a search for 
Ames, but there was no trace of him 
anywhere in the country, let alone Big 
Shell. He was gone.#C0
EOF
//}



//★ＡＪＢ３－Ａ１ {
/*
ENGLISH!!
ファットマンは倒れた。スネークはその巨体を見下ろした。かすかに胸が上下している。死んではいない。
｜
キャンベルを無線で呼び出し報告する。キャンベルによれば大統領は無事、沿岸警備隊に保護されたらしい。さらに、スネークがファットマンと戦っている間にＳＥＡＬＳが突入、残ったテロリストの制圧を行なっていると言う。
｜
「なぜ殺さなかった？」
いつのまにかエイムズが立っていた。スネークはエイムズに向き直った。
「あんたこそなぜ殺さなかった、大統領を？」
「なに？」
「ファットマンが言った。あんたが大統領を確保する役割だったと」
エイムズは押し黙った。
「あんたはなぜ大統領を殺さずに逃がした？」
スネークが詰め寄る。エイムズが後ずさる。手が懐に伸びる。
「ちょっと、何言ってるの！そんなことあるわけないじゃない！」
ジェニファーが現れた。ＳＥＡＬＳに救出されたのだろう。
スネークはエイムズから目を離さず答えた。
「どうしてそう言える？」
「だって大統領を逃がしたのはリチャードだもの」
「なんだって？」
つじつまが合わない。ファットマンが嘘をついているとはスネークには思えなかった。ならばなぜエイムズはスネークに協力しようとしたのか。そしてジェニファーはエイムズ自身が大統領を逃がしたと言う。ファットマンの言葉が嘘でも、ジェニファーの言うことが本当ならば、なぜ最初に会った時、エイムズは大統領が救出されたと言わなかったのか......。
考え込むスネークをよそにジェニファーが続ける。
「リチャードは優秀なシークレット・サービスよ。それに、そもそもリチャードが爆弾テロなんかに加担するわけないわ。妹さんが......」
「ジェニファー！」
エイムズが遮ったが、ジェニファーは無視した。
「リチャードは妹さんを爆弾テロで亡くしてるのよ。３年前に......」
背筋が凍るような感覚があった。単語がつながる。３年前、爆弾テロ、妹、教会、ファットマン。エイムズの言動が脳裏に甦る。ファットマンは無罪になった、と吐き捨てるエイムズ。裁かれるべきだったと主張するエイムズ。必ず殺すんだ、そう言って銃を寄越すエイムズ......。
スネークはエイムズを見た。目が泳いでいる。狼狽を隠そうとしている。仮説が組みあがる。
「エイムズ、なぜあんたは大統領を助けたことを俺に言わなかった？言えば俺がファットマンを殺す確率が下がると思ったか？」
エイムズは答えない。スネークは続ける。
「なぜあんたは警察ではなく俺達に直接連絡してきた？俺ならファットマンを捕まえずに殺すと考えたからか？」
「何言ってるの！」
ジェニファーが声を上げる。エイムズの手が懐に伸びていく。
「......あんたの狙いは、俺にファットマンを殺させることだったのか？そのために全てを......」
エイムズが懐から拳銃を抜いた。スネークがコンテナの陰に飛び込む。だがエイムズの銃口はファットマンに向けられていた。引き金が絞られる。
「やめて！」
ジェニファーがエイムズの手を払った。拳銃が弾き飛ばされる。宙を飛ぶ。床に落ちる。エイムズが飛びつく。だが一瞬早く拳銃を抑えたのはスネークの足だった。エイムズは転がったままスネークを見上げ、やがて肩を落とした。
ジェニファーがふらふらとエイムズへ歩み寄っる。
「......嘘だと言って......」
エイムズは首を振った。
「本当......なの？」
エイムズはその場に座り込み、空を仰いだ。
「ああ......」
エイムズは語った。
妹が殺され、ファットマンが釈放された後、エイムズは復讐の計画を練った。過激派グループの一つに接近し大統領拉致をもちかけた。実行犯としてファットマンを雇わせた。裏のエージェントを使って奴を探し出し引っ張り込むのには金がかかった。後はスネークの言った通りだ。ファットマンをおびき寄せ、スネークに殺させるつもりだった。妹のために。
ジェニファーはエイムズの正面にしゃがみこんだ。
「......そんなこと、あなたは何も......」
「ベッドで話すには重過ぎる話題だろう？」
ジェニファーがエイムズの頬を叩いた。泣いていた。エイムズは目をそらした。
「......君を巻き込みたくなかったんだ。大統領以外には手を出さない計画だった」
「そういうことじゃない......」
ジェニファーはエイムズの手を取り、両手で包み込んだ。すがるようにエイムズの瞳を覗き込む。しばし見つめあい、やがてジェニファーは絶望したように手を離した。涙が頬をつたう。そこに自分が映っていないと気づいたのかもしれない。ジェニファーが呟いた。
「......あなたのしたことは犯罪よ」
「それはわかってる。だが裁きは与えられるべきだ」
「でも、そんなことしたって！」
「妹は戻らない。それもわかってる。だが君なら許せるか？妹は殺された。だが奴は刑務所に送られることすらなかった。妹の人生は断ち切られた。にも関わらず奴は何事も無かったように酒を飲み、女を買い、人を殺している......間違っているとは思わないか？」
ジェニファーがうちのめされたようにうなだれる。
「......そうかもしれない。でもあなたも同じくらい間違ってる」
「......悪いが、それはよくわからないな」
エイムズは指を差し伸ばし、ジェニファーの涙をぬぐった。
「私をどうする？」
「警察に引き渡すわ」
「終身刑になるな」
「でしょうね」
エイムズは息をついた。
「その前に一つ頼みを聞いてくれるか？」
ジェニファーが顔を上げる。
「なに？」
「奴を殺させてくれ」
ジェニファーはつらそうに首を振った。
「......駄目よ」
「そうか......では別の手が必要になるな」
エイムズはつぶやいた。
「だが考える時間だけはありそうだ」#C0
*/
AJBF2_B: << EOF
Fatman fell with a sickening thud. 
Snake gazed down at the disgustingly 
fat body. Fatman's chest was moving 
faintly up and down. He'd live.
｜
Snake called Campbell by Codec and 
told him what had happened. Campbell 
reported that the President was safe in 
the custody of the Coast Guard. 
Moreover, the SEALs had entered the 
facility while Snake was fighting 
Fatman and were in the process of 
cleaning up the last remaining terrorists.
｜
"Why didn't you kill him?"
Snake turned around to see Ames 
standing behind him.
"Why? I should be asking you the 
same question. Why did you let the 
President go?
"What?"
"Fatman told me everything. You were 
supposed to be guarding the President."
Ames laughed.
"Fatman? You buy what Fatman said?"
Snake did not move his eyes.
"Yeah, I do. Were you and Fatman a 
team?"
"Never."
"Is that so?"
Snake advanced toward Ames. Ames' 
hand reached for his chest.
"Wait a minute! You don't know what 
you're talking about! He'd never do 
anything like that!" Jennifer suddenly 
appeared. The SEALs must have 
rescued her.
"How can you be so sure?" answered 
Snake, keeping his eyes fixed on Ames.
"Because Richard was the one who 
helped the President escape!"
"What are you talking about?"
Something didn't add up. Fatman didn't 
seem to be lying. But then why had 
Ames cooperated with Snake? 
And now Jennifer was saying that Ames 
had helped the President escape. Even 
if Fatman was lying and Jennifer was 
telling the truth, why would Ames 
neglect to mention that the President 
was safe?
Jennifer continued as Snake pondered 
this riddle. "Richard is one of our best 
agents. And besides, there's no way 
he'd be involved in a terrorist bombing 
plot in the first place. After all, his 
sister -- "
"Jennifer!" Ames interrupted. She 
ignored him.
"Richard lost his younger sister in a 
terrorist bombing attack. Three years 
ago..."
A chill ran down Snake's spine. The 
words came together in a rush. Three 
years ago... terrorist bombing... sister... 
church... Fatman. Snake suddenly 
recalled Ames' strange behavior. The 
way he spat as he said that Fatman 
had been acquitted. His passionate 
conviction that the jury did not bring 
justice to Fatman. The look in his eyes 
as he gave Snake his gun and said, 
"Promise me you'll kill him..."  
A hypothesis was starting to take shape.
Snake looked at Ames. He had no 
facial expression. No panic. No fear. 
Not even anger. The hypothesis had 
now become a conviction.
"Ames, you didn't tell me that you'd 
already rescued the President because 
you thought I'd be less likely to kill 
Fatman for you, right?"
Ames didn't respond. Snake continued.
"Did you call us instead of the police 
because I'd be more likely to kill 
Fatman instead of capturing him alive?"
"That's enough!" yelled Jennifer. 
Ames' hand inched closer to Snake's 
chest.
"You planned all along to have me kill 
Fatman, didn't you? And all this is 
just -- "
Ames suddenly pulled a pistol from his 
breast pocket. Snake reacted 
immediately, diving into the shadow of 
a nearby crate. But Ames' gun was 
pointed straight at Fatman's prostate 
body. He squeezed the trigger.
"Stop!"
Jennifer forced Ames' hand aside, 
knocking the pistol from his grasp. 
It arced through the air and fell with a 
clatter to the ground. Ames made a 
desperate dive for it, but Snake's 
trained reflexes were quicker. He 
reached out and grabbed the gun 
before Ames could get ahold of it. 
Lying sprawled on the ground, Ames 
looked up at Snake, then finally lowered 
his shoulders and relaxed.
Jennifer staggered over to Ames.
"Tell me it isn't so..."
Ames slowly shook his head.
"So... it's true, then?"
Ames sat down wearily and stared up 
at the sky.
"Yes..."
He told his story.

After his sister was killed -- and Fatman 
got off scot-free -- Ames had began to 
plot his revenge. He approached an 
extremist group with a scheme to 
kidnap the President. Then he hired 
Fatman to carry out the job. It had cost 
him a small fortune just to hire a secret 
agent to track Fatman down and 
convince him to join the conspiracy. 
After that, it was just as Snake had 
described. Ames' plan was to lure 
Fatman to the Big Shell and then have 
Snake kill him... all for the sake of his 
dear, departed sister. Jennifer crouched
down in front of Ames. "Richard... 
why didn't you tell me..."
"It's a little too heavy for pillow talk, 
wouldn't you agree?"
Jennifer slapped his the face. Tears 
streamed down her cheeks. Ames 
looked away.
"I never meant for you to get involved. 
There weren't supposed to be this 
many hostages. Just the President."
"No, you don't get it."
Jennifer took Ames's hand in her own 
and clasped it tightly. She stared 
straight into his eyes with a pleading 
gaze. But he returned no answer. 
"What you did is a crime."
Drooping her head in silence, Jennifer 
looked as if she were being a bad 
loser. Ames answered.
"I know that. But Fatman must be 
punished."
"Nothing Fatman did could possibly 
justify what you've done!"
"I know I can't bring my sister back 
from the dead. But put yourself in my 
position for a minute. Do you really 
think you could forgive him? He killed 
my sister, for God's sake! And he 
never spent a single day in prison for it. 
My sister never had a chance to live 
out her life. And yet he sits there 
drinking champagne, fooling around 
with beautiful women, and blowing 
people to bits. Doesn't that strike you 
as horribly wrong?"
"Perhaps... but that doesn't make you 
any less of a criminal." 
"Sorry, but I just don't see it the same 
way as you."
Ames reached out with his finger and 
wiped away Jennifer's tears.
"So what are you going to do with me 
now?"
"I'm going to turn you in to the police."
"I'll probably get life in prison."
"Yeah, I guess so."
Ames sighed. "Before we go, there's 
one last favor I need to ask of you."
Jennifer looked up at him. "What?"
"Please, let me kill the bastard."
Jennifer shook her head sadly. "I can't 
let you do that."
"I see... then I guess I'll just have to 
find some other way to do it."
Ames turned away and muttered to 
himself.
"Lord knows I'll have plenty of time to 
think."#C0
EOF
//}



}
