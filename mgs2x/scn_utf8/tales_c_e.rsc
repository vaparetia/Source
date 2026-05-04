/*
	tales_res_b_j.h
	    スネークテイルズ紙芝居用リソースヘッダ

	2002/06/04 S.Mukaide
	$Id: tales_c_e.rsc,v 1.6 2002/08/25 08:45:55 usr03005 Exp $


*/

// フォントチェック用define
/*
-fontcheckは省略可能です。５つのパラメータは順に 
width, height, chara_skip, line_skip, flag　をあらわします。
flagが1の場合は、禁則を行わない。(日本語以外)

-fontcheck width height chara_skip line_skip flag
*/
#define	TITLE_FONT_AREA		-fontcheck 520 120 0 12 0 


//---------------------------------------------------------------------------
//	テイルズＣ(メルリが出てくる話)
//---------------------------------------------------------------------------
resource テイルズ＿Ｃ__e d:TITLE_FONT_AREA {
//resource テイルズ＿Ｃ {
//最初 
//★Ｃ１{
/*

ENGLISH!!
暴風が荒れ狂う。豪雨が叩き付ける。川面がうねる。漆黒に塗り込めたゴムボートは木の葉のように翻弄される。スネークはボートの中から顔を上げた。嵐に混じってヘリの羽音が聞こえる。やがて３機の黒いヘリが前方のタンカーに接近していくのが見えた。スネークはつぶやいた。
「遅かったか……」
｜
三日前。アラスカのスネークの元にキャンベルが訪れた。シャドーモセス事件の後、隠遁したスネークと反対に、キャンベルは国防情報局に職を求めた。旧交を温めに来た訳でないことは初めから分かっていた。
しばしの会話の後、キャンベルは切り出した。
「セルゲイ・ゴルルコビッチを覚えているか？」
スネークはうなずいた。元ＧＲＵ大佐。冷酷非情な策謀家。冷戦時代はＦＯＸＨＯＵＮＤ司令官だったキャンベルに幾度も煮え湯を飲ませたという。ソ連崩壊後は、旧ソ連軍出身者を率いて傭兵部隊を組織、各地の紛争地帯を転戦していると聞いていた。キャンベルが続ける。
「奴が南米のエルデラ共和国で極秘作戦中の海兵隊を襲撃するという情報が入った。海兵隊は機密を盾に情報を明かそうとしないが……どうやらメタルギアが絡んでいるらしい」
「メタルギアだと？」
スネークは唸った。シャドーモセス事件後、メタルギアの技術情報が闇市場にばらまかれた。その結果、世界中でメタルギアの亜種が誕生しようとしているという噂はスネークも聞いている。
エルデラ共和国は南米の小国だ。内戦の国でもある。８０年代から親米的な独裁軍事政権が支配していたが、冷戦終了によって米勢力の影響力が減ると共に、分離独立を目指す国内少数民族が武装蜂起、以来、内戦が続いている。
エルデラには常に傭兵や武器商人が出入りしている。何らかの形で亜種メタルギアを入手できたとしても不思議ではない。というのがキャンベルの分析だった。核による恫喝が安定を産み出すという思想はまだ死んでいない。
スネークは聞いた。
「で、海兵隊はそれを破壊するつもりなのか？」
「いや、彼等はろ獲しようとしているようだ。亜種メタルギアの実態はまだ良く分かっていない。情報を掴む良いチャンスだからな」
つまり、エルデラは亜種メタルギアを手に入れた。海兵隊はそれを強奪する計画を立てている。さらにゴルルコビッチがそれを掠め取ろうとしている。そしてキャンベルは現れたゴルルコビッチを捕えようとしている。スネークは首を振った。
「悪いが他をあたってくれ。英雄稼業はもうやめたんだ」
「スネーク……」
「大佐、時代は変わった。そろそろあんたもやめるべきだと思う」
「軍に関わることをか？」
「奴を憎むことをだ」
口ごもるキャンベルを見て、スネークは確信した。キャンベルはいまだにゴルルコビッチを憎んでいる。冷戦時代からの遺恨。時間は憎しみを流し去らず、増幅している。
「ＤＩＡに入ったのも奴と決着を付けるためなのか？」
「……そうだ。だが今回君を頼ってきたのは奴への私怨を晴らすためじゃない」
キャンベルは拳を握り締めた。
「……その作戦には……メリルが参加しているんだ」
｜
ゴムボートがタンカーに接舷した。スネークはロープを取り出す。
メリル・シルバーバーグ。キャンベルの弟の娘。シャドーモセスで生死を共にした女。一時は共に暮らした女。半年前、彼女はスネークの元を去った。やらなければいけないことがある。そう言った。毅然としていた。その瞳は凛として、とても澄んで見えた。止めることは出来なかった。
｜
スネークはロープで外壁を登り、船尾甲板に降り立った。ロシア傭兵と思しき兵士達が警戒態勢をとっているのが見える。スネークはキャンベルに無線連絡を入れた。
「こちらスネーク。タンカーへの潜入に成功した。船内は既に奴等に制圧されているようだ。まずはメリルとの接触を図る」#C0
*/
	C1_スタート: << EOF
The wind howled as the rain beat down 
in torrential sheets upon the churning, 
undulating surface of the river. A 
single, lonely rubber boat, painted jet 
black, was being tossed back and forth 
upon the waves like the plaything of 
some capricious river god. Snake lifted 
his face to peer over the edge of the 
boat. Over the din of the storm, he 
could hear the sound of helicopters. 
At length, he saw three black choppers 
approach the tanker in front of him. 
"Am I late?" muttered Snake to 
himself.
｜
Three days earlier, Campbell had 
visited Snake at his home in Alaska. 
Unlike Snake, who had gone back into 
retirement after the Shadow Moses 
incident, Campbell had gotten himself a 
position at the Defense Intelligence 
Agency. Snake had known right away 
that Campbell wasn't there to rekindle 
old ties with his former comrade.
After making some brief small talk, 
Campbell had jumped straight to the 
point.
"Do you remember a man by the name 
of Sergei Gurlukovich?"
Snake nodded. A former GRU colonel, 
Gurlukovich was known as a ruthless, 
cold-blooded strategist. During the Cold 
War, when Campbell was the 
commander of FOXHOUND, 
Gurlukovich had humiliated him on 
numerous occasions. After the collapse 
of the Soviet Union, he had organized 
a group of former Red Army soldiers 
into his own personal mercenary army. 
Since then, Gurlukovich and his troops 
had drifted from one war zone to the 
next, selling their services to the 
highest bidder.
"We received a report that Gurlukovich 
and his men attacked a group of 
Marines who were on a secret mission 
in the South American republic of 
Eldera. The Marines are being 
tight-lipped about it -- as usual, they'd 
rather keep a lid on the whole affair.
But from what we've been able to tell, 
it sounds like Metal Gear is involved."
"Metal Gear?" Snake growled. He'd 
heard the rumors, too. Following the 
Shadow Moses incident, the technical 
specs for Metal Gear had been leaked 
onto the black market. As a result, 
bastardized versions of Metal Gear 
were starting to crop up all over the 
world. Eldera was a small country in 
South America that happened to be 
engaged in a civil war. Since the 
1980's, the country had been under 
the control of a pro-American 
dictatorial regime. But as the Cold War 
drew to a close and American influence 
in the region declined, the country's 
minority ethnic population had taken up 
arms against the government, calling 
for the establishment of a separate 
homeland. The country had been torn 
apart by civil war ever since.
Mercenaries and arms merchants were 
constantly coming and going in and out 
of Eldera. Given those circumstances, 
Campbell had figured, it wouldn't be 
too surprising if one side or the other 
had managed to get their hands on a 
new type of Metal Gear. It seemed that 
some people still clung to the belief 
that the threat of nuclear war could 
bring stability to the region.
"And the Marines were there to try and 
destroy it?" asked Snake.
"No. It looks as if they were trying to 
seize it for themselves. There's still a 
lot we don't know about the true nature 
of the new Metal Gear. This would 
have been a good chance to pick up a 
lot of valuable information."
In other words, Eldera had managed to 
get itself a new type of Metal Gear, the 
Marines were plotting to seize it, and 
Gurlukovich was planning to steal it 
from the Marines. And now Campbell 
was proposing to go after Gurlukovich.
Snake shook his head. "Sorry, but 
you'll have to find someone else to do 
your dirty work this time. I'm out of the 
hero business."
"Snake...."
"Times are changing, Colonel. I think 
it's about time you gave it a rest."
"You mean retire from the military?"
"No, I mean time to stop this damn 
vendetta of yours against Gurlukovich."
The look of fury on Campbell's face 
confirmed Snake's suspicions. 
Campbell still hated Gurlukovich with a 
passion. It was an old grudge left over 
from the Cold War. The passage of 
time had done nothing to diminish it. 
Rather, it had grown in intensity.
"Is that why you joined the DIA? To 
settle old scores?"
"...Yes, you're right. But I didn't come 
all the way here to ask for your help 
just for the sake of a personal grudge." 
Campbell's hands clenched into fists.
"...The truth is... Meryl was part of that 
mission."
｜
The rubber boat drew up alongside the 
tanker. Snake gripped the rope tightly.
Meryl Silverburg. Campbell's niece. 
The woman who had risked her life 
alongside Snake on Shadow Moses. 
At one time they'd been living together. 
Then, six months earlier, Meryl had left 
him. She said there was something 
important that she had to take care of. 
Her voice was resolute. Her eyes were 
clear and cold as steel. There was 
nothing Snake could do to stop her.
｜
Snake climbed the rope up the outer 
hull of the tanker and emerged onto 
the aft deck. Nearby, he could see a 
number of soldiers standing guard. 
They appeared to be Russian 
mercenaries. He called Campbell on 
the Codec.
"This is Snake. I've managed to 
infiltrate the tanker. It looks like our 
Russian friends have already taken 
control of the ship. I'll try to get in 
touch with Meryl somehow."#C0
EOF
//}


//船橋に到着
/*
ENGLISH!!
メリルは銃を下ろした。
「なぜあなたがここに？」
スネークは答えた。
「君に会いに」
「冗談はやめて」
「半分は本当だ」
「……伯父さんね？」
スネークはうなずいた。キャンベルがゴルルコビッチの襲撃情報を得ていたことを話す。メリルが吐き捨てた。
「お節介な話！」
「君には負ける」
「なんですって？」
「軍隊で他国に乗り込みメタルギアを強奪する。随分と大掛かりなお節介じゃないか」
責めるような口調になる。
「それが君の言っていた『やるべきこと』なのか？」
「……あなたには分からないわ」
メリルは目を伏せた。
「犬の世話だけしながら、その日その日を暮らしてればいいって人には分からない」
「俺は誰も憎まない生き方を見つけたかっただけだ」
「いいえ。あなたは向き合うことをやめてしまっただけ。世間へも過去へも……私へもね」
「……それが出ていった理由なのか？」
「そうじゃないわ……」
でもそうかもしれない、メリルの目はそう語っているようにスネークには思えた。沈黙が流れる。半年の時間が冷たく横たわる。やがてメリルがつぶやいた。
「……父は……この国で死んだの」
メリルの父親、キャンベルの弟、マット・キャンベル。元ＤＩＡ局員。メリルが幼い頃、任務中に死んだとスネークは聞いていた。
「父の見たものを私も見てみたいと思った……」
メリルは窓の外に目を移した。だが見えたのは嵐だけだった。メリルはスネークに向き直り、まっすぐその目を見据えて言った。
「スネーク、消えて頂戴。ここは私の戦場よ。部外者には立ち入ってもらいたくない」
「無理な話だ。もう関わっている」
「どうしても？」
スネークは断言した。
「どうしてもだ」
メリルが舌打ちする。スネークは無頓着を装う。
「俺達の戦場について教えてくれ。この作戦は極秘だったはずだろう？なぜゴルルコビッチが知っている？」
「それはこっちが知りたいわ」
「内通者が？」
「考えにくいわね。そもそも、あなたが知ってるのなら、誰が知っていてもおかしくないのかも知れない」
スネークは少し考えた。確かにそうかもしれない。だがどちらにせよ、ゴルルコビッチにメタルギアを渡す訳にはいかない。
「メタルギアはどこだ？」
「船倉よ」
「そうか。俺はそちらに向かう。君は……」
メリルが口を挟んだ。
「私はあなたの部下じゃない。別行動をとらせてもらうわ。船倉に向かうつもりなら、１Ｆの階段を塞いでる箱はなんとかしておいてあげる」
制止する間もなく、メリルは船橋から出ていった。#C0
*/
//★Ｃ２ {
	C2_船橋到着: << EOF
Meryl lowered her gun.
"What are you doing here?"
"I just wanted to see you," answered 
Snake.
"You think you're funny?"
"Actually, I was only half-joking."
"...My uncle sent you, didn't he?"
Snake nodded. He explained that 
Campbell had gotten word of 
Gurlukovich's attack.
"You've got a lot of nerve coming 
here!" Meryl snapped.
"Look who's talking," Snake shot back.
"What?!"
"Sneaking into another country under 
the guise of a military operation when 
all you're really after is the Metal Gear. 
Who's got a lot of nerve coming here?"
His voice took on a condemnatory tone.
"Is this that 'something important' that 
you had to take care of?"
"...You wouldn't understand." Meryl 
turned away. "Someone like you, who 
could happily spend the rest of his life 
taking care of sled dogs day after day, 
could never understand."
"I was only trying to live my life free of 
hatred."
"No. You just didn't want to face up to 
reality, that's all. Society, your past... 
even me."
"...Is that why you left?"
"No, that's not it...." But the look in 
Meryl's eyes told Snake that it might 
well have been. A deep silence fell 
between them. Six months of 
separation had created a cold, empty 
gap that was difficult to fill. At last, 
Meryl broke the silence.
"...My father... he died here, in this 
country," she murmured.
Matt Campbell, Meryl's father and 
Campbell's younger brother, had been 
a DIA agent. Snake had heard that 
he'd disappeared during a mission 
when Meryl was very young.
"I wanted to see for myself what my 
father saw...." Meryl's eyes drifted over 
to the window, but the only sight was 
that of the storm raging outside. She 
suddenly turned back to Snake and 
held him firmly in her gaze.
"Snake, please just get out of here. 
This is my battlefield. I don't need 
anyone from the outside to get involved 
in this."
"It's a little too late for that. I'm already 
involved."
"Nothing can change your mind?"
"Nothing," Snake said firmly.
Meryl sighed in exasperation. Snake 
pretended not to notice.
"Now tell me what our situation is. This 
mission was supposed to be top-secret, 
right? So how did Gurlukovich find out 
about it?"
"That's what I'd like to know, too."
"You think there's a leak?"
"I doubt it. But seeing as how you 
know all about it, then I suppose 
anyone might know about it."
Snake thought for a minute. Meryl was 
right -- anyone might know. But 
whatever the case, might be, they had 
to stop Gurlukovich from getting his 
hands on the Metal Gear.
"Where's the Metal Gear?"
"In the cargo hold."
"All right. I'll go check it out. You...."
Meryl cut him off. "I'm not about to 
take orders from you. We'll split up and 
go our own separate ways. If you're 
planning on heading for the cargo hold, 
then you'll need me to do something 
about the boxes that are blocking the 
first deck stairway."
Before Snake could stop her, Meryl 
had left the bridge.#C0
EOF
//}

//長廊下戦前
//★Ｃ３ {
/*
ENGLISH!!
暗闇の中、スネークの視界を何かがよぎった。すかさずコンテナの陰へ身を躍らせる。一瞬の光。銃声。空気が切り裂かれ、背後で跳弾が火花を上げる。
待ち伏せのようだった。しかし考える暇はなさそうだ。敵の数は少なくとも５人以上。銃を構える気配がする。スネークはＵＳＰを抜いた。#C0
*/
	C3_長廊下戦前: << EOF
In the pitch darkness of the corridor, 
something moved in front of Snake's 
line of vision. Reacting quickly, Snake 
dove into the shadow of a nearby crate. 
There was a flash of light. Snake heard 
the sound of a gunshot. A bullet 
whizzed past his head, igniting sparks 
as it ricocheted off the wall behind him.
It was a trap. No time to think about 
the particulars, though. There were at 
least five enemy soldiers up against 
him. He could hear the sound of guns 
being readied. Snake drew his USP.#C0
EOF
//}

//長廊下戦後
//★Ｃ４ {
/*
ENGLISH!!
スネークは通路の奥を伺った。銃弾を浴びせてくる者はもういない。先へ進む。倒した敵兵の脇を通り過ぎる。船倉へ入る。
船倉はシートをかけられた巨大な塊に占領されていた。間違い無いだろう。メタルギア。２１世紀を滅ぼす悪魔の兵器。
人の気配はなかった。メタルギアに近づく。落ちかかる影が視界を圧する。シートの下を覗く。しかし、そこにあったのは悪魔の死骸だった。
装甲に浮いた錆。堆積した埃。ひび割れた旧式アクチュエーター。うち捨てられて１０年以上は経過しているように見える。
「そんな顔をするな」
不意に背後から声が響いた。
「先人が遺した憎悪の遺産だ。もう少し敬意を払ってもいいのではないか？」
スネークは振り向いた。柱の影からセルゲイ・ゴルルコビッチが現れる。
「銃を捨ててもらおう」
スネークが答える。
「お断りだ」
ゴルルコビッチが柱の後ろから人影を引きずり出した。メリルだった。ゴルルコビッチがメリルを引き寄せる。銃をつきつける。銃口が白い喉を這う。口元を撫ぜる。スネークは銃を捨てた。ゴルルコビッチがうなずく。
「良い心がけだ」
「もういいでしょう。離して」
メリルはゴルルコビッチの手を振り払った。ゴルルコビッチがスネークに銃を向け直す。メリルへ向かってニヤリと笑う。スネークは知った。メリルとゴルルコビッチは組んでいる。つまり。
「……君が奴等を手引きしたのか？」
　沈黙がその問いを肯定していた。
「なぜだ？」
「父の遺志……と言ったら納得してくれる？」
メリルは懇願するようにスネークを見つめた。
「……もう一度言うわ。何も聞かずにここから消えて頂戴」
「無理な話だ」
　ゴルルコビッチが撃鉄を起こした。
「ならば私が消してやる」
トリガーが引かれる。
「待って！」
メリルが横からゴルルコビッチの手を払った。銃弾がそれ、スネークの足下で跳ねる。
「殺さないって約束でしょう！」
言い争う二人。スネークはＵＳＰを拾い、駆け出した。
ゴルルコビッチが部下達へ叫ぶのが聞こえる。
「奴を殺せ！」
メリルが制止の声を上げるのが聞こえた。だがスネークは振り返らない。まずはここから脱出するのが先決だった。#C0
*/
	C4_長廊下戦後: << EOF
Snake peered down the corridor. No 
hail of bullets came to greet him this 
time -- the coast was clear. He pressed 
forward, past the bodies of the fallen 
enemy soldiers, and entered the cargo 
hold.
The cargo hold was dominated by a 
massive shape covered by a tarp. 
There was no mistaking it -- it was a 
Metal Gear, the ultimate weapon of the 
21st-century.
Sensing the place to be deserted, 
Snake approached the Metal Gear. Its 
immense shadow loomed over him. He 
peeked under the tarp. But what he 
found was nothing more than a decrepit 
shell.
The armor plating was speckled with 
rust. The body was covered with a 
thick layer of dust. The old, badly 
outdated actuator was a crisscross 
road map of tiny cracks. It looked as if 
it hadn't been used in over 10 years.
"Do not look so disgusted." A voice 
suddenly sounded behind Snake's 
back. "What you see before you is a 
magnificent legacy of hatred left to me 
by my distinguished predecessor. You 
really should treat it with more respect."
Snake turned around in time to see 
Sergei Gurlukovich emerge from the 
shadow of a pillar.
"Drop your weapon," said Gurlukovich.
"Make me," Snake retorted.
Gurlukovich dragged another figure out 
from behind the pillar. It was Meryl. He 
drew her closer and jabbed his gun 
against her throat. Snake watched as 
the barrel slowly traced the contours 
of her slender, white neck and brushed 
gently against the corner of her mouth. 
He threw down his gun.
"A wise choice," said Gurlukovich.
"All right, Gurlukovich. Let her go!"
Meryl shook off Gurlukovich's grasp. 
Gurlukovich pointed his gun at Snake. 
He regarded Meryl with a leer. 
Suddenly, Snake realized what was 
going on. Meryl was working with 
Gurlukovich. Meaning....
"...So you were the leak?" 
Meryl was silent, confirming Snake's 
fears.
"But why?"
"...What if I told you that it was my 
father's dying wish?" Meryl looked 
pleadingly at Snake. "...I'll ask you one 
more time. Please, leave this place."
"I can't do that."
Gurlukovich cocked the hammer of his 
gun. "Then I'll just have to get rid of 
you!"
He pulled the trigger.
"No!" Meryl grabbed Gurlukovich's 
hand from the side. The bullet went 
astray, striking the ground between 
Snake's feet. "We had a deal! You 
promised you wouldn't kill him!"
As the two continued to argue, Snake 
retrieved his USP and made his 
escape. As he ran, he heard the sound 
of Gurlukovich shouting angrily at his 
men.
"Kill him!"
Snake heard Meryl trying to stop him, 
but he didn't turn back. His only thought 
was to escape as quickly as possible.#C0
EOF
//}

//長廊下から機関室に脱出
//★Ｃ５ {
/*
ENGLISH!!
スネークは水密扉を閉めた。ハンドルを回しロックする。これで多少の時間が稼げるはずだった。その間に確かめなければいけないことがある。スネークは無線でキャンベルを呼び出した。ことの次第を説明する。キャンベルの声が震えた。
「メリルがゴルルコビッチと……」
「メリルは『父の遺志』と言った。あんたの弟はこの国で一体何をしていたんだ？」
キャンベルは語った。
冷戦中、このエルデラでは親米的な独裁政権の元、米軍による極秘プロジェクトが行われていた。マット・キャンベルもその一員だった。冷戦終了と共にプロジェクトは打ち切られ、米軍は撤退を決めた。マットは失踪した。直後、後ろ盾を失った独裁政権に対し少数民族が蜂起した。内戦が始まった。マットの死体が発見された。蜂起の後ろにはスペツナズがいた。指揮官の名はセルゲイ・ゴルルコビッチ。
「マットがゴルルコビッチに通じていたという者もいる。だが私は信じていない」
その言葉は、ゴルルコビッチがマットを殺したと信じている、とも聞こえた。
「だからあんたはゴルルコビッチに拘ってきたのか？」
「……そうだ」
キャンベルは認めた。キャンベルはゴルルコビッチを憎んでいる。憎しみは反射し連鎖し増殖する。
「あんたはそのプロジェクトに関わっていなかったのか」
「近親者が関わる作戦には参加しないのが原則だ。私情で判断が鈍るからな」
「それだけか？」
キャンベルの口調に後悔がにじんだ。
「……当時マットは私と距離を置きたがっていたんだ」
だがその理由については答えようとしなかった。
「おそらくあの骨董品のメタルギアが極秘プロジェクトの成果だ。……これ以上は私も知らない。最高機密扱いで、今もその記録は封印されている」
ありうる話だ。とスネークは思った。多くの人間は忌まわしい記憶を忘れようとする。組織も同じだ。しかし忘れようとしない人間もいる。憎しみに拘る人間がいる。ゴルルコビッチのように。キャンベルのように。
キャンベルが言う。
「スネーク、奴等にメタルギアを渡す訳にはいかない。発射可能な核弾頭が装備されている可能性がある。奴等はどこかの海域でメタルギアの回収を試みるはずだ。船の進路を変えてくれ。船橋に向かうんだ」
それについてはスネークにも異論はなかった。#C0
*/
	C5_機関室脱出: << EOF
Snake closed the watertight door 
behind him and turned the handle to 
lock it. That should buy me some time, 
he thought. In the meantime, there was 
something he had to check on. He 
called Campbell on the Codec and 
described what had happened.
"Meryl and Gurlukovich...." Campbell's 
voice quivered.
"Meryl said something about her 
'father's dying wish.' What the hell was 
your brother doing in this country, 
anyway?"
Campbell explained.
During the Cold War, while the country 
was under the control of a 
pro-American dictator, the American 
military had been conducting a 
top-secret project in Eldera. Matt 
Campbell had been a member of that 
project. When the Cold War ended, the 
project was terminated as well, and the 
Americans had prepared to withdraw 
from the country. Matt, however, went 
AWOL. Shortly afterward, the 
pro-American regime, having lost its 
backing, was faced with armed 
uprisings among the country's minority 
groups. Eldera was plunged into civil 
war. Matt's corpse was discovered 
later. Spetsnaz had been behind the 
uprising. And the name of their 
commander was Sergei Gurlukovich.
"Some say that Matt was actually 
collaborating with Gurlukovich. But I 
don't believe it for a minute." 
Campbell's words made it sound as if 
he believed that Gurlukovich himself 
had killed Matt.
"Is that why you've been so obsessed 
with Gurlukovich for all this time?" 
asked Snake.
"...Yes," admitted Campbell. He hated 
Gurlukovich. Hatred spawned hatred in 
kind.
"Were you involved in the project, 
too?"
"No. There's a rule against that. We're 
forbidden to take part in an operation 
involving family members. They say it 
compromises your judgment."
"Is that all?"
"...I guess Matt wanted to put some 
distance between us at the time." 
Campbell's voice was tinged with 
regret. But he ventured no explanation 
as to the reason behind his brother's 
actions. "The antique Metal Gear you 
saw is the end result of that project.... 
That's all I know. The whole thing is 
highly confidential. Even now, the 
records are all sealed."
I believe it, thought Snake to himself. 
After all, it was only natural for people 
to try and forget horrible things that 
happened in the past. Governments 
were no exception. But some people 
held on to these kinds of memories. 
Some people never forgot their hate. 
Like Gurlukovich and Campbell.
"Snake, we can't let Gurlukovich and 
his men escape with that Metal Gear," 
said Campbell. "It could still be armed 
with live nuclear warheads. They're 
probably planning to test the Metal 
Gear's capabilities somewhere out at 
sea. You've got to change the course 
of the ship. Head for the bridge."
Snake had no objections.#C0
EOF
//}


//再び船橋に到着
//★Ｃ６ {
/*
ENGLISH!!
スネークは船橋に辿り着いた。窓の外、嵐の向こうにメリルの姿が見えた。スネークは水密扉を開け、航海ウイングに出た。メリルが振り返る。
「遅かったわね」
「待っていたのか？」
「ここなら邪魔は入らない。聞いてほしいの」
「何をだ？父親を殺した男に手を貸す理由か？」
「伯父さんがそういったの？」
スネークはうなずいた。メリルは首を振った。
「それは事実じゃないわ」
メリルは語った。
エルデラで極秘プロジェクトと行われていたメタルギアの開発。マット・キャンベルの担当任務はエルデラ国内の安定化だった。分離独立を目指す少数民族ゲリラへの情報工作。まず候補者を見繕う。弱みを掴んで脅迫する。餌をちらつかせて懐柔する。仕立てたスパイを組織に潜入させる。情報を提供させる。仲間を売らせる。
だがマットは致命的な過ちを犯した。少数民族に接するうち、徐々に彼等へ共感していった。独立の理想に魅せられた。小国を利用する米国の姿勢に憤るようになった。マットは彼等の国を作る手助けをしたいと考えた。彼等に民族独立の切り札を与えようと思い立った。米軍が極秘かつ違法に開発していた核搭載二足歩行戦車はうってつけだった。計画にはゲリラを支援していたセルゲイ・ゴルルコビッチが協力した。
計画は実行された。米軍基地からのメタルギア搬出には成功した。だが計画は発覚した。マットは米軍の手によって抹殺された。父は理想を抱いた罪によって裁かれた。この世界で良心は高くつく。
その後の内戦に伴う混乱の中でメタルギアは行方不明になった。だが半年前、その存在が確認され、海兵隊が過去の恥を回収に向かうことになった。
全ての真相をゴルルコビッチから知らされたメリルは、彼の指示に従って海兵隊に潜入した。今もゲリラを支援しているゴルルコビッチを介して彼等にメタルギアを渡し、父の遺志と理想を継ぐつもりだった。
スネークは聞いた。
「ゴルルコビッチがそう言ったのか？」
メリルはうなずいた。スネークは首を振った。
「それは事実じゃない、と考えたことはないのか？」
メリルは哀しげにスネークを見た。
「……見逃してはくれないわけね……？」
「だとしたらどうする？」
メリルは銃を抜いた。銃口がスネークを捉える。「……撃てるのか？」
　メリルの親指が安全装置を外す。
「撃てるわ」#C0
*/
	C6_メリル再会: << EOF
Snake made his way to the ship's 
bridge. Outside the window, beyond the 
storm, he could see the figure of Meryl. 
He opened the watertight door and 
stepped into the navigation room. 
Meryl turned around.
"You're late."
"Were you waiting for me?"
"There's no one to interrupt us here. I 
need to tell you something."
"You mean like the reason why you're 
helping the man who killed your 
father?"
"Is that what my uncle told you?"
Snake nodded. Meryl shook her head.
"He was lying."
Meryl explained.
Matt Campbell's part in the top-secret 
Metal Gear development project in 
Eldera had been to ensure the 
country's stability. Part of his job was 
feeding falsified information to the 
minority separatist guerillas, and he 
was good at it. First, he'd pick a 
suitable candidate. Next, he'd find his 
target's weak point and use it to 
blackmail him. Then he'd dangle a 
carrot in front of the victim's eyes and 
convince him to cooperate. Matt would 
then send his now-willing accomplice 
into the ranks of the separatists. Once 
there, the spy provided the Americans 
with useful information, selling out his 
comrades in the process.
But in carrying out his mission, Matt 
made one fatal error. In his dealings 
with the separatists, he came to 
sympathize with their cause. He 
became enchanted with the idea of 
independence. He grew to resent the 
U.S. for the way it exploited smaller, 
weaker nations for its own gain. In the 
end, Matt decided to help the guerillas 
create their own nation. He wanted to 
give them the trump card they needed 
to secure their independence. And 
what better gift than a bipedal, 
nuclear-capable walking tank that was 
conveniently being developed in Eldera 
(albeit secretly and quite illegally) by 
the American military? Sergei 
Gurlukovich, who was also supporting 
the guerillas, agreed to cooperate.
Matt's plan was executed. The guerillas 
succeeded in stealing the Metal Gear 
from the American base. But then their 
plan was discovered. Matt was 
executed by the Americans. Meryl's 
father had been punished for the crime 
of believing in his own ideals. The price 
of having a conscience was high in 
those troubled times. In the tumult of 
the civil war that followed, the Metal 
Gear disappeared without a trace. 
However, six months ago, its existence 
was reconfirmed, and the Marines had 
been sent to recover their long-lost 
dirty secret.
After hearing the true story from 
Gurlukovich, Meryl agreed to follow his 
instructions and infiltrate the Marines. 
Her intent was to transfer the Metal 
Gear over to the guerillas through 
Gurlukovich, who was still providing 
support to the rebels. In this way, she 
could carry on her father's memory and 
the ideals for which he died.
"Is that what Gurlukovich told you?" 
asked Snake.
Meryl nodded. Snake shook his head.
"Didn't it ever occur to you that he 
might be lying?"
Meryl looked at Snake with sorrow in 
her eyes. "...You're not going to let me 
get away, are you?"
"So what are you going to do?"
Meryl drew her gun and pointed it at 
Snake.
"You're not going to shoot me," said 
Snake.
She released the safety with her thumb.
"Watch me."#C0
EOF
//}


//メリル戦後
/*
//メリル戦後親リソース
//分岐：
//メリル殺した場合：C7_1A→C7_2→C7_3A
//メリル気絶させた場合：C7_1B→C7_2→C7_3B
	C7_メリル戦後: << EOF
#C1#C2#C3
EOF
//}
*/

//メリル殺した場合
//★Ｃ７－１Ａ {
/*
ENGLISH!!
スネークはメリルがその場に崩れ落ちるのを見た。すぐに駆け寄り、抱き起こす。大量の出血。かすかな呼吸。……致命傷。スネークは自分が傷つけた女を抱きしめた。体温が失われていくのがわかる。
*/
	C7_1A: << EOF
Snake watched as Meryl crumpled to 
the ground. He rushed to her side and 
cradled her in his arms. She was 
bleeding profusely. Her breath was 
faint... and her wounds were mortal. 
Snake clasped her tightly to his chest, 
covering the wounds he himself had 
given her. He felt her body grow cold 
and lifeless in his grasp.#C0
EOF
//}

//メリル気絶させた場合
//★Ｃ７－１Ｂ {
/*
ENGLISH!!
スネークはメリルがその場に崩れ落ちるのを見た。すぐに駆け寄り、抱き起こす。気絶しているだけだ。スネークはメリルを抱きしめた。
*/
	C7_1B: << EOF
Snake watched as Meryl crumpled to 
the ground. He rushed to her side and 
cradled her in his arms. Fortunately, 
she'd only been temporarily knocked 
out. He clasped her tightly to his chest.#C0
EOF
//}

//★Ｃ７－２ {
/*
ENGLISH!!
「胸が痛むな」
背後から声がした。ゴルルコビッチだった。拳銃を手にしている。怒りを抑えながらスネークは聞いた。
「なぜメリルを利用した？」
ゴルルコビッチが肩をすくめる。
「利用？私は彼女が知りたがっていたことを教えてやっただけだ」
「そうではあるまい」
ゴルルコビッチは少し笑った。策謀家の笑い。嘘で人を操る人間の笑い。
「確かに多少の脚色はしたがな。人間は信じたいものを信じるものだ」
「大佐への報復のつもりか？」
「そうだ。奴はもっと苦しむべきだ。奴の苦しみは、奴が葬った私の部下とその遺族達の苦しみにはまだまだ足りない」
ゴルルコビッチはキャンベルを憎んでいる。キャンベルもゴルルコビッチを憎んでいる。憎しみは反射し連鎖し増殖する。
「だから彼女を選んだ。キャンベルの一番大切な人間だと言うじゃないか。マットから聞いたよ。死ぬ間際に」
「やはりお前がマットを殺したのか？」
ゴルルコビッチの目に一瞬影がよぎった。
「人聞きが悪いな。私は望みを叶えてやっただけだ。奴は誰かに殺されるのを待っていた。……そういうところがあった」
その顔は友を悼んでいるように見えた。あるいは本当に悼んでいるのかもしれない。だがその表情もすぐに消えた。
「あのメタルギアはこの国のゲリラには渡さない。私の部隊で有効に使わせてもらう。本体はガラクタだが、核弾頭とその発射システムさえ修復できればいい。核弾道弾は今の時代でも充分貴重だ」
歯噛みするスネークをゴルルコビッチが笑う。
「そう怒るな。これもマット・キャンベルの遺志だ。奴も最初からそのつもりだったのだから」
「なんだと？」
「嘘だと思うなら奴に直接聞くがいい」
ゴルルコビッチは銃口をスネークに向けた。
「あの世でな」
銃声が轟いた。
｜
ゴルルコビッチが倒れる。撃ったのはメリルだった。
*/
	C7_2: << EOF
"Do not trouble yourself over it," said a 
voice behind Snake's back. It was 
Gurlukovich. He held a pistol in his 
hand.
"Why were you using Meryl?" Snake 
demanded, fighting to control his anger.
"USING her? I merely told her what 
she wanted to know."
"I don't think so."
A slight smile crossed Gurlukovich's 
face. It was the smile of a master 
strategist, a smile that told of lies, 
manipulation, and betrayal. "Perhaps I 
did embellish the truth a bit. After all, 
people will believe what they want to 
believe."
"Is this your idea of revenge on the 
Colonel?"
"Of course. He deserves to suffer. I will 
make him squirm in agony until his pain 
is as great as the suffering of all of the 
men that I have had to bury, and all of 
the widows and orphans they left 
behind."
Gurlukovich hated Campbell. Campbell 
hated Gurlukovich. Hatred spawned 
hatred in kind.
"That is why I chose her. Is she not 
the most important person in 
Campbell's life? This I heard from 
Matt... as he lay dying."
"So it was you that killed Matt?"
Gurlukovich's eyes darkened for an 
instant. "I see my reputation precedes 
me. I simply gave him that which he 
desired. He was only waiting for 
someone to kill him. That is the kind of 
man he was." For a moment, it almost 
looked as if Gurlukovich were truly 
grieving the loss of an old friend. 
Perhaps his grief was real. But the look 
soon passed.
"I will not hand the Metal Gear over to 
the Elderan guerillas. My men and I 
have other plans for it. The machine 
itself may be a wreck, but all we really 
need to do is recover the nuclear 
warheads and the launching system. A 
nuclear-equipped ballistic missile is still 
a precious commodity, even in this day 
and age."
Snake gritted his teeth. Gurlukovich 
laughed.
"Do not be so angry with me. This is 
Matt Campbell's dying wish. This was 
his intention from the beginning."
"What do you mean?"
"If you do not believe me, then ask him 
yourself."
Gurlukovich aimed his gun at Snake.
"In hell!"
The sound of a gunshot ripped through 
the air. 
｜
Gurlukovich fell to the ground. 
The shot had come from Meryl.#C0
EOF
//}

//メリル殺した場合
//★Ｃ７－３Ａ {
/*
ENGLISH!!
スネークの腕の中でメリルの力が抜けていく。
その手から銃がすべり落ちる。
「スネーク……」
メリルがうめいた。何かを言おうとしているようだった。スネークはその口元に耳を寄せた。だがもう息吹すら感じ取ることは出来なかった。
*/
	C7_3A: << EOF
Meryl's body slowly went limp in 
Snake's arms.
The gun slipped from her fingers.
"Snake...." Meryl moaned. She looked 
as if she were trying to say something. 
Snake lowered his head closer to her 
mouth. But his ears could not detect 
the slightest trace of breath.#C0
EOF
//}


//メリル気絶させた場合
//★Ｃ７－３Ｂ {
/*ENGLISH!!
メリルはのろのろと立ち上がった。スネークに背を向ける。雨が体を包む。
「メリル……」
「お願い。何も言わないで」
肩が震えていた。泣いているのかもしれなかった。


タンカーとメタルギアは米軍の手に戻った。メリルは軍の取り調べを受けることになった。
数日後。拘置所へ面会に行ったスネークはキャンベルと出会った。
キャンベルは語った。回収されたメタルギアの調査が行われた。ガラクタ同然の失敗作だった。核弾頭も出来の悪い模擬弾頭だった。マットがそれを知っていたかどうかはわからない。記録は何も残っていない。そしてマットが軍を裏切ったのは自分のせいだ。
「なんだって？」
スネークは聞き返した。
「マットは私を憎んでいた。私だけじゃない。自分の妻も……メリルも……」
「どういうことだ？」
「メリルの父親は私なんだ……」
スネークは絶句した。信じられない面持ちでキャンベルを見やる。
「じゃあ……あんたは弟の妻と……？」
キャンベルはうなずいた。
「……許されないこととわかっていた。だが……彼女は……」
キャンベルはうなだれた。
「マットはそのことを？」
「感づいていた。あいつは逃げるように独りエルデラへ向かい……そこでゴルルコビッチに付け込まれた……」
諜報戦の基本は弱みをつくことだ。恥を調べ上げ、傷をえぐり、同情を示し、信用を得て、心を奪い、操る。奴にとってマットは格好の標的だったことだろう。
「……メリルはそのことを？」
「いや。だが……これから伝えるつもりだ」
キャンベルは接見室へ向かった。その後ろ姿を見送りながらスネークは思った。メリルが事実を受け入れるには時間がかかるだろう。キャンベルを認める日は来るのかどうかすらわからない。だがいつかまた二人が笑いあえる日が来るといい。これ以上誰かを憎んではいけない。#C0
*/
	C7_3B: << EOF
Meryl staggered painfully to her feet 
and turned her back to Snake. The rain 
beat down upon her ragged frame.
"Meryl...."
"Please, just don't say anything."
Her shoulders were trembling. Snake 
wondered if she was crying.
#WThe tanker and the Metal Gear it was 
carrying were recovered by U.S. forces. 
Meryl, meanwhile, found herself the 
subject of a military inquiry.
Several days later, Snake ran into 
Campbell while visiting Meryl at the 
detention center.
According to Campbell, the military had 
conducted a full investigation on the 
recovered Metal Gear. It was judged to 
be a complete failure, little more than 
scrap metal. The nuclear warheads, 
too, were nothing but cheap imitations. 
Whether or not Matt had been aware 
of this was unclear. There were no 
records left. And, Campbell added, it 
was his fault that Matt had betrayed his 
country.
"What?!" Snake exclaimed.
"Matt hated me. And not just me, 
either. He hated his wife, too... and 
Meryl...."
"What are you trying to say?"
"Meryl is my daughter."
Snake was thunderstruck. He stared at 
Campbell in utter disbelief. "So... you 
and your brother's wife...?"
Campbell nodded. "...I knew what I did 
was unforgivable. But I thought she...." 
He hung his head.
"Did Matt know about it?"
"He sensed that something was up. 
That's why he went to Eldera alone -- 
to get away from us. And that's where 
Gurlukovich found him...."
The first rule of intelligence gathering 
was to find your opponent's weak point. 
Dig up the ghosts of his past. Rub salt 
into his old wounds. Then pretend to 
sympathize. Gain his trust. Steal his 
heart. Then he would be yours to 
control. In Matt, Gurlukovich had found 
the perfect target.
"...Does Meryl know about this?"
"No. But... I was about to tell her."
With that, Campbell turned and started 
walking towards the visiting room. As 
Snake watched him recede into the 
distance, he thought to himself. It 
would take time for Meryl to accept the 
truth about what had happened. He 
wasn't even sure whether or not she'd 
ever acknowledge Campbell as her 
father. But it would be enough for 
Snake if, one day, the two could smile 
at each other once again. There wasn't 
any room in the world for more hatred.#C0
EOF
//}


}
