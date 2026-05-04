/*
	tales_b_g.rsc
	    スネークテイルズ紙芝居用リソースヘッダ

	2002/06/04 S.Mukaide
	$Id: tales_b_g.rsc,v 1.1 2002/08/26 05:15:41 usr03005 Exp $


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
//	テイルズＢ(エマが出てくる話)
//---------------------------------------------------------------------------
resource テイルズ＿Ｂ__g d:TITLE_FONT_AREA {
//resource テイルズ＿Ｂ {

//最初 
//★Ｂ１{
/*
ENGLISH!!
「スネーク、頼みがあるんだ」
オタコンが言った。スネークは顔をしかめた。オタコンの頼みにはろくなものがない。
「断る。レアだかなんだか知らんが、よくわからんモノのために全米を駆けずり回るのは、もう……」
「そうじゃないよ。妹のことなんだ」
「妹？」
「うん。エマって言って……」
オタコンはいつになく真剣な様子で語り始めた。
長らく音信不通だった義理の妹の消息がつかめたという。ＮＹ湾沖の海上除染施設『ビッグ・シェル』でシステムプログラマをしているらしい。スネークは聞き返した。
「ちょっと待て『ビッグ・シェル』と言えば……」
「そう。例の噂、聞いてるだろ」
最近『ビッグ・シェル』では職員の事故死が相次いでいた。連続殺人、集団うつ病、果ては『ビッグ・シェル』には悪魔がいる、という噂まで流れている。今や『ビッグ・シェル』は環境保護のシンボルではなく、タブロイド紙のネタ供給源となっていた。
「エマが心配なんだ。様子を見に行ってくれないか？」
スネークは手を振った。
「警察にでも頼め」
「動いてくれる訳ないだろ」
「俺なら動くとでも？」
「そうじゃなくて、最近犯罪組織同士の抗争が激化してるだろ？警察はそっちに手いっぱいでこっちなんか構ってくれないよ」
「俺なら構うとでも！？」
「胸騒ぎがするんだ。頼むよ！」
「自分で行け」
「そんなこと言わずに」
「嫌だ！」
「すごく悪い予感がするんだ！頼むよ！！」
「絶対嫌だ！！」


６時間後、スネークは『ビッグ・シェル』のヘリポートに降り立っていた。そこでスネークが見たものは、施設を制圧していく武装集団の姿だった。
スネークはコンテナに身を隠した。無線機からオタコンが呼びかけてくる。
「……テロかな？」
「わからん。だが少々危険な状況なのは確かだ。お前のカンが当たったな」
「嬉しくないよ。エマは大丈夫かな……」
「だといいが。この屋上のハリアーも連中が？」
スネークはヘリポートに駐機されているハリアーを横目で見ながら言った。オタコンが答える。
「それは違うよ。三日前、海兵隊の演習中にトラブルを起こして不時着したものらしい」
スネークはうなずいた。
「とりあえず事態の把握が先決だ。今から内部に潜入する」
「わかった。気をつけて。あ、それと……」
「なんだ？」
「エマを見つけても僕のことには触れないでほしいんだ」
「どうして？」
「うん……長い間会ってないし……いきなりっていうのも……」
オタコンは口ごもった。何か複雑な事情があるのだろう。スネークは了承し、『ビッグ・シェル』内部へと向かった。#C0
*/
	B1＿スタート: << EOF
"Snake, I have a favor to ask," said 
Otacon.
Snake's face creased into a frown. 
Otacon's "favors" always meant trouble, 
usually for Snake.
"Forget it. I don't know what you're up 
to this time, but I'm through running 
around the country on some half-baked 
crusade I don't even understand."
"It's nothing like that. It's... it's my 
sister."
"Your sister?"
"Yeah. Her name's Emma." Otacon's 
voice suddenly became grave as he 
began to relate his story. He'd recently 
discovered the whereabouts of his 
long-estranged sister-in-law. Apparently, 
she was working as a systems 
programmer at an ocean cleanup facility 
in Manhattan Bay called "Big Shell."
"Wait a minute. Big Shell... isn't that
 where...?"
"Exactly. So you've heard the rumors, 
too."
There had recently been a string of 
accidental deaths among the employees 
of Big Shell. A number of rumors were 
circulating about the cause of these 
deaths - there was a serial killer on the 
loose, the employees were going 
stir-crazy, the whole place was haunted. 
Big Shell had gone from being a 
symbol of environmental responsibility 
to being a lucrative source of tabloid 
gossip.
"I'm worried about Emma. Will you go 
and check on her for me?"
Snake waved his hand in dismissal. 
"Go ask the police or something."
"You know they wouldn't do anything 
about it."
"And you assumed that I would?"
"I've got a bad feeling about this thing. 
Just go and check for me, please?"
"You go."
"Come on!"
"No way!"
"I'm telling you, Snake, I've got a 
REALLY bad feeling about this! Please, 
I'm begging you!"
"Leave me alone!"
｜
Six hours later, Snake found himself 
standing on the heliport of Big Shell. 
He watched as a group of armed men 
began to take over the facility before 
his eyes. Snake hid himself behind a 
crate. Just then, Otacon called on the 
Codec.
"...You think they're some kind of 
terrorists?" he asked.
"I dunno. But it's pretty clear that 
the situation is a little more dangerous 
than before. Congratulations. Looks like 
you guessed right."
"That's not exactly reassuring. I hope 
Emma's still okay...."
"Yeah, me too," said Snake. His eye 
drifted over to a Harrier jet parked on 
the heliport. "What about that Harrier? 
Does it belong to them?"
"No, that belongs to the Marines. 
Apparently they made an emergency 
landing here three days ago, after a 
problem occurred during a training 
exercise."
Snake nodded. "First things first. We'd 
better find out what exactly is going on 
here. I'm going in."
"Roger. Be careful in there. Oh, and 
Snake..."
"What?"
"If you do happen to meet Emma, don't 
mention my name, okay?"
"Why not?"
"Well... we haven't seen each other in a 
long time... and it's all kind of sudden..."
Otacon hesitated. Snake realized that 
there was more to the situation than 
Otacon was willing to let on. He decided 
to let it drop as he headed inside Big 
Shell.#C0
EOF
//}

/*
//ＤＥ連絡橋に出た時
//Ｂ２の親リソース {
	B2_ＤＥ連絡橋に出た: << EOF
#C1
#C2
#C3
EOF
//}
*/

//★Ｂ２−０ {
/*
ENGLISH!!
ＤＥ連絡橋に出たスネークは気配を感じ、物陰に身を隠した。足音と共に話し声が聞こえてくる。
男の声が言う。
「逃げた女は？」
別の男の声が答える。
「Ｃ脚で見失ったらしい」
女？エマのことだろうか。スネークは耳を澄ました。
「見失った？そりゃヤバイな」
「ヤバイのか？」
「詳しくは聞いてねぇが、アレを取り返すのに必要らしい。つまりあのクソガキのケツを捕まえないことには俺達もヤバイってことさ」
スネークは男達が話す姿をうかがった。装備は軍隊並みだが、物腰からして軍人ではないようだ。話は続く。
*/
	B2_0: << EOF
Snake emerged onto the DE connecting 
bridge. Sensing a presence nearby, 
he quickly hid himself in the shadows. 
Soon he heard the sound of footsteps, 
accompanied by voices.
"What about that girl who got away?" 
said a man's voice.
"We lost her in Strut C," said another 
man.
Girl? Were they talking about Emma? 
Snake cocked his head to listen more 
closely.
"Lost her? Ooh, that's not good," said 
the first man.
"Why?"
"Well, I don't know all the details, but I 
heard that we need that brat to get it 
back. So until we get our hands on her, 
we're screwed."
Snake stole a peek at the two men. 
They were decked out in full military 
gear, but they didn't seem to carry 
themselves like soldiers.
The first man continued.#C0
EOF
//}

//敵に発見されている場合
//★Ｂ２−１Ａ {
/*
ENGLISH!!
「それと聞いてるか？さっきヘリポートで……」
「ああ、ビッグシェルの悪魔だな」
*/
	B2_1A: << EOF
"Oh yeah, did you hear about what 
happened at the heliport?"
"Yeah, the 'ghost of Big Shell' showed 
up, right? Some people call it the 
'Big Shell Evil'."#C0
EOF
//}

//敵に発見されていない場合
//★Ｂ２−１Ｂ {
/*
ENGLISH!!
「それで、あっちの方は？」
「ビッグシェルの悪魔か？まだ現れちゃいないらしい」
*/
	B2_1B: << EOF
"What about... you know...?
"The 'ghost of Big Shell'? Nobody's 
seen him yet."#C0
EOF
//}

//★Ｂ２−Ｃ {
/*
ENGLISH!!
「俺達が回収に来たとあれば、奴は必ず出てくるだろう。見つけ次第……」
「ケツを蹴り飛ばす」
「いや、それじゃ足りない。殺すんだ。奴はこっちの人間を三人も殺してる……」
男達は話しながら遠ざかっていった。スネークはオタコンを無線で呼び出した。オタコンが声を荒げる。
「その『逃げた女』ってのがエマに違いないよ！」
「奴等はビッグシェルの悪魔に仲間を殺されたとか言っていた……どういうことだと思う？」
「わからないよ。そんなことよりＣ脚に向かってくれ。エマが……」
オタコンの言う通り、今はエマを助けることが第一だろう。スネークはＣ脚へ向かうことにした。#C0
*/
	B2_2: << EOF
"He must know we're here to pick it up. 
He'll be coming for us as soon as we 
find it."
"If he does, I'll kick his ass."
"Heh, I'd do more than that. I'd kill the 
bastard. He's already iced three of our 
guys..."
The voices trailed off into the distance. 
Snake called Otacon on the Codec.
"That 'girl' that they were talking 
about -- that's Emma! I'm sure of it!"
"They mentioned that some of their 
comrades were killed by the 'ghost of 
Big Shell'.... What do you think they 
were talking about?"
"I have no idea. Don't worry about it. 
Just head for Strut C, quickly. 
Emma's..."
Otacon was right. Right now, saving 
Emma was the top priority. Snake 
decided to head for Strut C.#C0
EOF
//}

//Ｃ脚でエマを見つける
//★Ｂ３ {
/*
ENGLISH!!
スネークは食料庫の中でしゃがみこんでいる女にゆっくり近づいた。
「エマ、だな？」
「誰？なんで私の名前を！？」
エマを落ち着かせるように、スネークは言った。
「安心しろ。俺は味方だ。助けに来た」
「……味方？」
「そうだ」
「助けに？」
「ああ」
「……ホントに？」
「ホントだ」
スネークは言ったが、エマはまだ疑わしげな目つきでこちらを見ている。
「……そんなに信用できない人間に見えるのか？」
「あなたの家、鏡ないの？」
スネークは反論しようとしたが、先にエマがしゃべりだした。
「まあいいわ。で、あなたはどうして私を助けてくれるの？」
自分のことには触れないでほしい、というオタコンの言葉をスネークは思い出した。
「……それは言えない」
「どうして？」
「言えないからだ」
エマはスネークを上から下まで見回した。納得はしていないが、もう警戒もしていないようだ。
「……ヒーローの押し売りなんて聞いたことないけど、悪い人じゃなさそうね……あなた名前は？」
「ソリッド・スネーク」
突然エマは立ち上がり、スネークの耳を引っ張った。
「もしもし？コトバワカリマスカ？私は、なんて名前ですか？って聞いてるの。名前聞かれてニックネーム答える人いる？」
エマは手を離し、スネークの目を覗き込んだ。
「本名は？」
「捨てた」
「はぁ？」
「俺の世界では必要とされないものだ」
エマは軽く顔をしかめた。
「んー……悪い人じゃなさそうだけどちょっと変な人みたいね……まあいいか。じゃあ……ソリッド？」
「スネークと呼んでくれ」
スネークはため息を吐いた。この子と話すと妙に疲れる。
「意外と細かいとこ、こだわるのね……じゃあスネーク？あなた……」
スネークはエマを遮った。
「そろそろ俺にも質問させてくれ」
エマは不満気に口をすぼめた。
「……いいけど？」
「どうして連中は君を探してるんだ？」
「私が可愛いからじゃない？」
言葉を返せないでいるスネークに、エマは少しがっかりしたようだった。
「……もしくは私がパスコードを変えてコンピュータシステムをロックしたせいかも」
「どういうことだ？」
エマが言うには、このビッグシェルを占拠した連中がコンピュータシステムを使って何かをしようとしているのを見たと言う。そして見張りの隙をついて逃げ出した後、システムに変更を加え、奴等が使えないようにしたらしい。
「時間稼ぎになるし、嫌がらせにもなると思ったから」
後者の比重の方が高そうだとスネークは思った。
「で、奴等は何をしようとしてるんだ？」
「知るもんですか」
「ビッグシェルの悪魔とやらが関係しているのか？」
「なんですって？」
エマが大袈裟に驚く。
「奴等はビッグシェルの悪魔に仲間を殺されたと言っていた。『ビッグ・シェル』の悪魔とは一体何者なんだ？」
エマがスネークをすごく嫌そうな顔で睨み付けてくる。
「……なぜそんな目で見る？」
「だってビッグ・シェル』の悪魔だなんて、ばかばかしい」
手をひらひらと振る。
「いい？確かに、この一ヶ月で三人も亡くなってるのは事実よ。とても悲しいことだった。初めに二人が相次いで行方不明になって、その直後に赴任してきた人もすぐに……でもそれは全部事故なの！それを幼稚な『伝説』と結び付けて……」
「『伝説』？ビッグシェルの悪魔の話は前からあったと？」
「そう。昔、息子さんが死んでショックを受けた職員が自殺しようとしたんだけど、死にきれずに大怪我を負ったそのままの姿で住み着いてるとか、そんな話……」
瞳が一瞬震えたような気がしたが、エマはすぐに明るい声をだした。
「ほら、その手の怪談めいた話ってこういう場所にはつきものでしょ？……誰もいないはずの区画で人影を見たとかいう人もいるけど、私は職員がサボってただけだと思ってる」
「だが……」
「思ってるんだってば！！」
エマがムキになる。
「落ち着け。俺もそんなお化け話信じちゃいない」
「そうよね？」
エマはほっとしたような笑みを浮かべた。
「ああ。だが可能性を検討しておきたい。この施設に部外者が入り込むことは可能なのか？」
エマは少し考えて答えた。
「……ビッグシェルは自動化が進んでいてほとんどは無人区画なの。だから可能だとは思うわ。だけど……」
突然食堂の扉が開いた。怒号と共に敵が室内になだれ込んでくる。長話をしすぎたらしい。スネークは舌打ちした。エマといるとどうも調子が狂う。
「エマ、下がってろ！」
スネークは叫んでＵＳＰを抜いた。#C0
*/
	B3_Ｃ脚エマ遭遇: << EOF
Snake slowly approached the girl, who 
shrank into the corner of the food 
storage locker.
"You're Emma, right?"
"Who are you? How do you know 
my name?!"
"Calm down," said Snake, trying to 
reassure her. "I'm on your side. I came 
to rescue you!"
"You're... on my side?"
"That's right."
"You came to save me?"
"Yes."
"...Really?"
"Really."
Emma regarded Snake with a suspicious 
look.
"...Do I really look like I can't be 
trusted?" said Snake.
"What's the matter, don't you have a 
mirror in your house?"
Snake started to protest, but Emma cut 
him off.
"Whatever. So, why exactly did you 
come to rescue me?"
Snake recalled what Otacon had said to 
him -- whatever happens, don't mention 
his name.
"I can't tell you that."
"Why not?"
"Because, I just can't."
Emma looked Snake up and down, from 
head to toe. She didn't exactly look 
convinced, but she seemed to let her 
guard down.
"...You know, for a hero, you don't 
sound very convincing. But I guess you 
don't seem like such a bad guy, either. 
What's your name?"
"Solid Snake."
Suddenly, Emma stood up and grabbed 
Snake by the ear.
"Hel-LO? Habla Ingles? I asked you 
what your NAME was, not what your 
NICKNAME was. God, what a freak!" 
Emma released Snake's ear and looked 
him square in the eye. "What's -- your 
-- REAL -- name?"
"I got rid of it."
"Huh?"
"People in my line of work don't need 
names."
A slight frown crossed Emma's face. 
"...Hmph. You might not be a bad guy, 
but you sure are weird. But whatever. 
So... Solid?"
"Call me Snake." Snake sighed. Talking 
to this kid was making him tired.
"Man, you're really uptight about names, 
aren't you? Okay then, SNAKE, 
you're..."
Snake interrupted her. "Let me ask you 
a few questions, all right?"
"...Sure, fine."
"Why are those terrorists looking for 
you?"
"Maybe it's because they find me 
irresistibly attractive."
Snake couldn't think of anything to say 
to this. Emma seemed a little 
disappointed.
"...Or maybe it's because I changed the 
password for the computer system and 
locked them out."
"What do you mean?"
Emma explained. She'd seen the 
terrorists trying to use Big Shell's 
computer system to do something. 
Then, after creating a diversion and 
making her escape, she'd made a 
change to the system so that they 
couldn't use it anymore. "It bought me 
some time, and I thought it might irritate 
them as an added bonus, too."
She was probably more concerned with 
the latter, Snake thought to himself. 
"So what are the terrorists trying to do 
with the system?"
"How should I know?"
"Does it have something to do with the 
'ghost of Big Shell'?"
"What are you talking about?" Emma 
feigned a look of surprise.
"They said that some of their comrades 
got killed by the 'ghost of Big Shell.' 
So who is this 'ghost' they're talking 
about, anyway?"
Emma glared at Snake in disgust.
"Why are you looking at me like that?"
"The 'ghost of Big Shell'? That's so 
lame!" She waved her hands above her 
head. "Sure, it's true that three of them 
have died in the last month. It was 
actually really sad. Two of them just up 
and disappeared one after another. 
Then this other guy who'd just got here, 
he disappeared, too.... But they were all 
accidents, okay? They're just 
connecting it with some stupid, childish 
legend...."
"'Legend'? So this 'ghost' story has 
been around for a while?"
"Yeah. They say that there was this one 
employee a long time ago whose son 
died, and he tried to kill himself, but he 
didn't really die, he just got really 
mangled and stuff, and now he haunts 
the place.... You know, that sort of 
thing."
Something in Emma's eyes seemed to 
flicker for a moment, but she quickly 
recovered. Her voice became cheerful 
again.
"Hey, these kinds of places always have 
one or two ghost stories associated with 
them, right?... Some people say they've 
seen shadows lurking around in 
supposedly empty rooms, but personally 
I think someone's probably just slacking 
off on the job."
"But..."
"There's no such thing as ghosts!" 
Emma was clearly getting flustered.
"Okay, settle down. I don't believe in 
ghost stories, either."
"I know! Stupid, aren't they?" Emma 
looked relieved.
"Right. But I don't want to rule out any 
possibilities. Do you think anyone could 
have gotten in here without 
authorization?"
Emma thought for a minute. "...Big Shell 
is almost completely automated, so 
most of it's inaccessible to people. 
But yeah, I guess it might be possible. 
Still..."
Suddenly, the door to the Dining Hall 
burst open. Enemy guards started 
pouring into the room, shouting angrily. 
It looked like their conversation had 
dragged on a little bit too long. Snake 
grunted in frustration. It was tough to 
concentrate with Emma around.
"Emma! Get down!" Snake yelled as 
he drew his USP.#C0
EOF
//}

/*
//敵兵をすべて倒した後
//分岐：
//直前のイベント中にダメージを受けた場合：B4_0→B4_1A→B4_2（紙芝居終了後レーション１個増えている）
//直前のイベント中にダメージを受けていない場合：B4_0→B4_1B→B4_2
//Ｂ４の親リソース {
	B4_Ｃ脚敵兵倒した後: << EOF
#C1#C2#C3
EOF
//}
*/

//★Ｂ４ {
/*
ENGLISH!!
突入してきた敵は全て倒した。スネークは、銃を収め、しゃがみ込んだエマに近づいた。肩に手を置いて声をかける。
「エマ、大丈夫か？」
「ええ……」
エマはうつむいたまま答えた。スネークの目を見ようとはしない。
「エマ？」
「……知り合ってまだ１０分もたってないけど、あなたについて、わかったことが一つあるわ」
「なんだ？」
「あいつらと一緒。暴力が得意」
強気を装っているが声は震えている。スネークは諭した。
「やらなければやられる。これが俺の生きてきた世界、そして君が今いる場所だ。生き延びたければ強くなれ」
エマが顔を上げた。
「……もう一つわかったわ。自分を正当化するのも得意」
スネークを見上げたエマは気づいた。
べったりとついた返り血。戦闘に疲弊した顔。
*/
	B4_0: << EOF
The guards all lay silent on the floor. 
Snake put away his gun and moved 
towards Emma, who was cowering in 
the corner again. He put his hand on 
her shoulder.
"Emma, are you all right?"
"Yeah..." Emma hung her head, trying 
to avoid Snake's gaze.
"Emma?"
"...I know I've only known you for 10 
minutes, but I've already learned 
something about you."
"What's that?"
"You're just like them. You're good at 
killing people." She was trying to put a 
strong face on, but her voice was 
wavering.
"In my world," said Snake, "there's only 
one law: kill or be killed. That's what's 
kept me alive so far. And that's the only 
thing that's keeping you alive right now. 
If you want to survive, you'd better 
learn to be strong."
Emma raised her head. "...I just learned 
something else about you. You're also 
good at justifying yourself, aren't you?"
Emma drew closer to Snake.
His face was covered with the blood of 
others. It was a face exhausted by 
battle.#C0
EOF
//}

//★怪我してる場合（Ｂ４−１Ａ）{
/*
ENGLISH!!
全身の怪我。エマは目を伏せた。それらは全てエマを守るために負った傷だと悟った。
「……ごめんなさい」
エマはレーションをスネークに差し出した。
「これ……」
*/
	B4_1A: << EOF
Snake was wounded in numerous places. 
Emma tried to avert her eyes. He'd 
suffered all that, just to protect her.
"...I'm sorry."
Emma handed Snake a ration.
"Here..."#C0
EOF
//}


//★怪我してない場合（Ｂ４−１Ｂ）{
/*
ENGLISH!!
怪我こそしていないが、それらは全てエマを守るために負った危険の証明だった。エマは目を伏せた。
「……ごめんなさい」
*/
	B4_1B: << EOF
Snake wasn't hurt, but that didn't 
change the fact that he'd risked his life 
protecting her. She tried to avert her 
eyes.
"...I'm sorry."#C0
EOF
//}

//★Ｂ４−２ {
/*
ENGLISH!!
「気にするな。なんてことない」
スネークは言った。
「話の途中だったな。他の人質は？」
「シェル２中央棟の１Ｆに集められてる。まだ誰も危害は加えられてないみたいだけど……」
エマは口ごもった。あくまでも『まだ』に過ぎないということだろう。
「わかった。何とかしよう。君はここに……」
「待って！中央棟には行けないの」
「なに？」
「Ｄ脚にあるシェル１シェル２中央棟への扉をロックしたの。カードを使っても開けられない」
「どうすればいい？」
「Ｂ脚のノードにアクセスして、パスコードを入れればいいわ」
エマは１６桁の英数字を口にした。スネークはうなずいた。
「わかった」
エマが驚く。
「メモとかしなくて良いの？」
「俺の世界ではメモはしない」
スネークはパスコードを復唱してみせた。エマが感心する。
「あなたって、暗記も得意なんだ」
「Ｂ脚だったな。君はここにいろ。俺が行く」
「でも……」
「任せろ。『お使い』も得意だ」
エマは微笑んだ。
「わかった。これを持っていって。予備のセキュリティカード。Ｂ脚に行くのに必要なの。私はもう一枚自分の持ってるから」
スネークはエマからカードを受け取った。
「すぐに戻ってくる。何があってもドアを開けるな。音も立てるなよ」
エマはうなずいて食料庫の扉を閉めた。
*/
	B4_2: << EOF
"It's nothing. Forget about it," said 
Snake. "Now let's get back to the task 
at hand. Where are the other 
hostages?"
"They're all on the first floor of the Shell 
2 Core. I don't think anybody's been 
hurt yet, but still..."
"Don't worry. I'll do everything I can to 
help them. You'd better stay here...."
"Wait! You can't get to the Shell 2 
Core!"
"What?"
"I locked the door in Strut D that links 
the Shell 1 and Shell 2 Cores. It won't 
open, even with a card."
"So how do I open it, then?"
"You've got to access the node in Strut 
B and enter the password." She dictated 
a 16-character string of letters and 
numbers to him.
Snake nodded. "Got it."
Emma looked surprised. "Shouldn't you 
write it down or something?"
"In my line of work, we don't take 
notes." 
Snake demonstrated by reciting the 
password from memory.
"So you're good at memorizing things, 
too, huh?"
"I'm heading for Strut B. You stay put."
"But..."
"No buts. Just leave everything to me. 
I'm good at running errands, too."
Emma smiled. "Okay. Here, take this 
with you. It's an extra security card. 
You'll need it to get into Strut B. I've 
got my own card with me, so it's okay."
Snake took the card from Emma. "I'll be 
back before you know it. No matter 
what happens, don't open the door. And 
try not to make a sound."
Emma nodded and closed the door to 
the locker.#C0
EOF
//}



//Ｂ脚ノードにアクセス
//★Ｂ５ {
/*
ENGLISH!!
スネークはノードへアクセスした。エマから教えられたパスコードを入力する。しばしの間をおいてシェル１シェル２連絡橋の扉が解放された旨のメッセージが表示された。スネークがＣ脚へ引き返そうとしたその時、どこかのスピーカーから声が流れてきた。構内放送のようだ。
「……ビッグシェルの悪魔に告ぐ。女から渡されたコントロールキーを使ってシステムロックを解除しろ。今から１５分以内だ。さもなくば女の命はない。繰り返す……」
声は流れ続ける。背中に冷たいものが走った。女とはエマのことに違いない。敵に捕らわれたのか？だがコントロールキーとは何だ？スネークはオタコンを無線で呼び出した。オタコンが取り乱す。
「エマが！？君は一体……！？」
「落ち着いてくれ。今はどうやってエマを助け出すか考えなければ……」
オタコンの声に平静らしきものが戻った。
「……わかったよ」
「奴等の言うコントロールキーとは、このエマから渡された……」
「いいや、それはセキュリティカードだ。ドアを開けるためだけのものだよ。コントロールキーは『ビッグ・シェル』のシステム管理者だけが持っている認証鍵で文字どおり鍵型の指紋認証……要するにそれがなければビッグシェルのコンピュータシステムを変更できないっていうものだ。システムロックを解除するにはそれがないと……」
「さっきのパスコードで操作することは？」
「無理だ。パスコードで入れるのはレベル３までだ。それより上のアクセス権を得るにはコントロールキーが必須で……」
「詳しいな」
「そんなことより、エマからは他に何も渡されてないのかい？」
「ああ。奴等は一体何を勘違いしてるんだ？」
「……エマがテロリストに嘘を教えたのかもしれない」
「本物のビッグシェルの悪魔に渡したという可能性もあるな」
「それはないだろうね」
オタコンは即答した。スネークが聞き返す。
「なぜそう言い切れる？」
「君だって、そんな奴がいるとは信じてないだろう？そんなことよりエマだ！」
「そうだな。だがエマがどこに捕らわれているか分からなければ……」
オタコンが無線機の向こうで手を打つ音が聞こえた。
「そうだ！それなら分かると思うよ！！」
「なに？」
「エマは自分のカードを持ってるって言ってただろ？『ビッグ・シェル』のセキュリティカードは、無線識別技術、ＲＦＩＤになってるんだ。ＲＦＩＤはＩＣチップと無線データ送信機能を備えていて、無線読取装置が認識できる範囲にあるＩＤを検出することが出来る。『ビッグ・シェル』では全てのデータが統合管理されてるんだけど、それはレベル３でアクセス可能なんだ！」
オタコンが早口でまくしたてる。スネークは聞いた。
「……つまり？」
「つまり、ノードを使えばエマがどの脚部にいるかわかるってことさ！スネーク、さっきのパスコードでノードにアクセスしてみてくれ」
スネークはオタコンの言う通りにノードを操作した。やがて、ディスプレイ上のＦ脚が点滅する。
「エマはＦ脚だ。スネーク、Ｆ脚に向かってくれ！」#C0
*/
	B5_ノードにアクセス後: << EOF
Snake accessed the node. He entered 
the password that Emma had given him. 
After a moment, the monitor displayed 
a message saying that the lock on the 
door to the Shell 1 - 2 connecting bridge 
had been released. Snake turned to 
head back to Strut C, he heard a voice 
emanating from an unseen speaker. It 
sounded like some kind of building-wide 
announcement.
"This is a message for the 'ghost of Big
Shell'. You are hereby ordered to 
remove the lock on the system using the
control key you got from the girl at 
once. You have fifteen minutes to 
comply. If you fail to do so, we will kill 
the girl. I repeat..."
The voice repeated its warning. A chill 
ran up his spine. The girl they were 
talking about had to be Emma. Had 
they really gotten their hands on her? 
And what was this control key they 
were talking about? Snake called Otacon 
via the Codec.
"They got Emma?!" Otacon exploded. 
"What did you..."
"Just calm down for a minute. I'm trying 
to think of a way to save her."
The calm returned to Otacon's voice. 
"All right."
"This control key that they're talking 
about, could it be this card that Emma 
gave me?"
"No, that's just a security card. It's 
only good for opening doors. The 
control key is just what it sounds like -- 
an actual key that only Big Shell's 
system administrator would have access 
to. It acts as a verification device. 
Basically, you can't make any changes 
to Big Shell's computer system without 
it. The terrorists need it to release the 
lock on the system...."
"What if they used a password like I 
just did?"
"That wouldn't work. Passwords are 
only good up to security level 3. Beyond 
that, you'd need the control key to gain 
access...."
"Sounds like you've been doing your 
homework."
"Snake, are you sure Emma didn't give 
you anything else?"
"Yeah. What makes them think I have 
it, anyway?"
"...Maybe Emma lied to them."
"Or maybe she gave it to the real 'ghost 
of Big Shell.'"
"That's not possible," said Otacon 
quickly.
"And how can you be so sure?" 
countered Snake.
"Hey, you said yourself that there's no 
such thing, didn't you? We have to 
hurry and save Emma!"
"You're right. But we don't even know 
where they've taken her...."
Over the Codec, Snake heard the sound 
of Otacon smacking one hand against 
the other.
"That's it! Why didn't I think of it 
before?"
"What?"
"Emma said she had her own card with 
her, right? Security cards in Big Shell 
work on an RFID system -- that's radio 
frequency identification. Each RFID 
card is imbedded with an IC chip that 
sends data via a radio signal. The card 
readers can use these signals to detect 
any IDs within range. All data in Big 
Shell is managed by an integrated 
management system. And that system 
is accessible with a level 3 password!" 
Otacon was speaking rapidly now.
"...Meaning?" said Snake.
"Meaning that you can use the node to 
find out which strut Emma is in! Snake, 
try accessing the node again using that 
password."
Snake did as Otacon said. After a 
moment, the screen displayed a blinking 
light in Strut F.
"That's her. She's in Strut F. Snake, 
you've got to hurry!"#C0
EOF
//}


//途中でシェル１シェル２連絡橋に出た時
//★Ｂ＿シェル１シェル２連絡橋に出た＿エマ救出前 {
/*
ENGLISH!!
シェル１シェル２連絡橋に出たスネークは無線での呼び出しを受けた。
「こちらスネーク」
「何が、こちらスネークだ！」
オタコンの怒声が響く。
「一体どこに行くつもりなんだ！？エマはＦ脚だろ！早くＦ脚に向かってくれ！」#C0
*/
	Ｂ＿シェル１シェル２連絡橋に出た＿エマ救出前: << EOF
As Snake emerged onto the Shell 1 - 2 
connecting bridge, he got a call on 
the Codec."This is Snake."
"Don't give me that 'this is Snake' 
routine!" Otacon was furious. "Where 
do you think you're going, anyway? 
Emma is in Strut F! Get over there 
now!"#C0
EOF
//}


//★Ｂ＿シェル１シェル２連絡橋に出た＿エマ救出後 {
/*
ENGLISH!!
シェル１シェル２連絡橋に出たスネークは無線での呼び出しを受けた。
「こちらスネーク」
「何が、こちらスネークだ！」
オタコンの怒声が響く。
「エマをおいてどこに行くつもりなんだ！？君がそんな人間だとは思わなかったよ！早くエマのところに戻ってくれ！！」#C0
*/
	Ｂ＿シェル１シェル２連絡橋に出た＿エマ救出後: << EOF
As Snake emerged onto the Shell 1 - 2 
connecting bridge, he got a call on 
the Codec.
"This is Snake."
"Don't give me that 'this is Snake' 
routine!" Otacon was furious. 
"Are you just gonna leave Emma there 
to fend for herself? You ought to be 
ashamed of yourself! Get back there 
right away!"#C0
EOF
//}




//Ｆ脚侵入時
//★Ｂ６ {
/*
ENGLISH!!
スネークはＦ脚への扉をくぐった。エマはこのＦ脚のどこかに捕らわれているはずだ。ここからはより慎重にいかねばならない。敵に発見されれば、奴等は容赦なくエマを殺すだろう。敵に見つかることなく無くエマを探し出さなければならない。#C0
*/
	B6_Ｆ脚侵入: << EOF
Snake passed through the door to Strut 
F. Emma was being held captive 
somewhere in this part of the facility. 
He'd have to be extra cautious from 
here on out. If the enemy spotted him, 
they'd kill Emma without hesitation. 
Snake would have to find her without 
being detected.#C0
EOF
//}


//エマと接触
//★Ｂ７ {
/*
ENGLISH!!
スネークはエマを見つけた。
「エマ！」
エマはしゃがみこんだまま、顔を上げた。
「遅かったじゃない！」
ふくれっ面をしてみせる。怪我はないようだ。
「悪かった。『コントロールキー』を探すのに手間取ってな」
スネークの皮肉に、エマはぺろっと舌を出す。「ああ、あれね……いい時間稼ぎになったでしょ？」
全く悪びれた様子はない。オタコンの推測通り、奴等に嘘をついたということなのだろう。スネークはエマを咎めた。
「どうしてあんな危険なことを？一つ間違ったら……」
「あいつらの好きにさせるのは嫌だったから。それに、信じてたし」
「何を？」
「ちゃんと助けに来てくれるって」
スネークは首を振った。
「そういう問題じゃない。とにかくああいう危険なマネは……」
「わかってる。それより聞きたくない？奴等が何を探してるのか、わかったの」
「なんだ？」
「麻薬よ」
エマは捕らわれながら見張りから聞き出したという話を語った。
『ビッグ・シェル』を占拠している武装集団はロシア系犯罪組織の構成員らしい。組織は職員の何人かを抱き込み、『ビッグ・シェル』を麻薬密輸の中継地として利用していた。夜間に海上の密輸船から小型艇で『ビッグ・シェル』へ麻薬を搬入し、施設からの搬出物に紛れて国内に持ち込む。近年強化されている港での摘発をかいくぐるための手口だったという。
一ヶ月前、約６００ｋｇ、末端価格７００万ドル以上に及ぶ大量の麻薬が運び込まれた。そしてその次の日、組織の手引きをしていた職員二人が相次いで事故死した。組織は即刻調査のために組織の人間を派遣したが、その人物も到着後間もなく事故で死んだ。
組織の幹部達はこの事実を、近年麻薬を巡って対立していた別の犯罪組織が麻薬を強奪しようとしていると判断し、その奪回のために武装集団を投入した、というのがことの顛末だという。
スネークは聞いた。
「つまり、『ビッグ・シェル』の悪魔とは……」
エマはなぜかうれしそうに微笑んだ。
「そう。お化けなんかじゃない。麻薬を横取りするために『ビッグ・シェル』へ入り込んだ対立組織の殺し屋だったってこと」
「……ということは、君は奴等に、俺が組織の殺し屋だと言ったってことだな？」
「ま、まあいいじゃない、過ぎたことは……」
スネークはそれ以上追求せず別の疑問を口にした。
「で、それだけ大量の麻薬がこのビッグシェルのどこにあるんだ？」
エマがかぶりを振る。
「見当もつかない。ちっとも気づかなかったもの。今でも信じられない……」
スネークも首をひねった。他の職員に知られないように、数百キロに及ぶ麻薬を隠し続けることが可能なのだろうか。だが考え込んでいる時間はなさそうだった。まだ人質が捕らわれたままだ。スネークはエマに確認した。
「人質はシェル２中央棟だったな？」
「ええ……」
エマは立ち上がろうとしてよろけた。足元がふらついている。
「大丈夫か？」
「うん、ちょっと……実は奴等に何か打たれたの。今度は逃げ出られないようにって……多分鎮静剤か何かだと思うけど……お陰で足が……」
エマがまたバランスを崩す。スネークはエマを支えた。こんな状態になりながらも敵から情報を収集し、なお悪態をつくことも忘れない。芯の強いところが義理の兄とよく似ている。
「もう充分だ。君は隠れていろ」
「いいえ。連れていって」
「だが……」
「お願い」
エマはまっすぐスネークを見つめた。
「あなたには二回も助けてもらったわ。今度は私が手助けしたいの。きっと役に立つから……お願い！」
結局スネークは折れた。シェル２中央棟へはＤ脚からシェル１シェル２連絡橋を渡ればいい。二人はＤ脚へ向かうことにした。#C0
*/
	B7_エマ再会: << EOF
Snake had found Emma.
"Emma!"
Emma looked up without rising from 
her crouched position. "You're late!" 
She had a sullen look on her face, but 
she didn't appear to be hurt.
"Sorry. It took me a while to find the 
control key."
Emma stuck out her tongue at Snake's 
sarcasm. "Oh, that.... You have to admit 
it did buy us some time, didn't it?"
She showed no sign of remorse. 
Just as Otacon had guessed, Emma 
had indeed lied to the terrorists.
"What in the world made you do a thing 
like that? If you'd made one false 
move..."
"Hey, I just didn't want them to get 
away with it, that's all. And... I believed 
in you."
"Believed what?"
"That you'd come and rescue me, of 
course."
Snake shook his head. "That's not the 
point. The point is, you shouldn't be 
putting yourself in so much danger...."
"I know. But listen, you're never gonna 
believe this! I found out what the 
terrorists are looking for!"
"What is it?"
"Drugs!"
Emma related a story she'd heard from 
one of the guards while she was being 
held captive.
The armed group that had taken over 
Big Shell belonged to a Russian crime 
syndicate. With the collaboration of a 
number of employees in the facility, the 
syndicate was using Big Shell as a 
waypoint in their drug smuggle 
operations. At night they'd sneak the 
drugs from smuggling ships out at sea 
into Big Shell using small boats. Then 
they'd slip the drugs in with the 
outgoing cargo from Big Shell and get 
them into the country that way. Security 
in the harbor had been getting tighter in 
recent years, and this was a convenient 
and effective way of getting the goods 
through without being exposed.
One month earlier, they'd brought in a 
600 kg shipment of drugs with a street 
value of over $7 million. The following 
day, two of the employees who were 
working with the syndicate were killed 
in two consecutive accidents. The 
syndicate immediately sent one of their 
men in to investigate, but he too was 
killed in an accident shortly after 
arriving.
The leaders of the syndicate decided 
that these incidents were the work of a 
rival syndicate that had been competing 
with them in the drug trade and that 
were now trying to seize their shipment. 
They'd sent in their troops in to recover
the drugs, leading to the current 
situation. 
"So the 'ghost of Big Shell' is..."
A mischievous grin spread across 
Emma's face. "Exactly. It's not a ghost 
at all. It's just a plain old hitman who's 
been sent by the rival syndicate to 
steal the drugs."
"...And you told them that I'm the 
hitman."
"Well... yeah, but what's done is done, 
right? No use arguing about it."
Snake decided not to pursue the matter 
any further. He changed the subject. 
"So where could they be hiding such a 
huge amount of drugs in Big Shell?"
"I have no clue. I didn't have any idea 
what was going on. I'm still having 
trouble believing it even now...."
Snake, too, was puzzled. How could it 
be possible for a few employees to 
keep hundreds of kilograms of drugs 
hidden in Big Shell without the others 
finding out? He couldn't afford to waste 
any more time thinking about it, though. 
The hostages' lives were still in danger.
"The hostages are still in the Shell 2 
Core, aren't they?"
"Yeah...." Emma tried to get up and 
stumbled. She slowly staggered to her 
feet.
"Are you all right?" Snake asked.
"Yeah, I'm fine.... Actually, the 
terrorists injected me with something. 
They said it was to keep me from 
running away again. I think it was some 
kind of tranquilizer.... Now my feet feel 
like they weigh about a ton each...."
Emma started to lose her balance 
again. Snake caught her as she fell. 
Even in her incapacitated state, she'd 
managed to extract information from the 
enemy, and never lost sight of her 
purpose for being there. This girl was 
full of surprises. In that respect, at 
least, she resembled her stepbrother.
"That's enough. You'd better go hide 
somewhere."
"No way. I'm coming with you."
"But..."
"Please, take me with you." Emma's 
eyes leveled with Snake's.
"You've saved my life twice now. I want 
to return the favor if I can. I know 
I'll come in handy somehow.... Please!"
Snake gave in at last. To get to the 
Shell 1 Core, he'd have to cross the 
Shell 1 - 2 connecting bridge from 
Strut D. The two set out for Strut D.#C0
EOF
//}


//Ｅ脚に到着
//★Ｂ８ {
/*
ENGLISH!!
スネークとエマはＥ脚に到着した。
「わかった！」
ベルトコンベアを目にしたエマが突然声を上げた。
「どうしたんだ？」
「わかったの。奴等がどこに麻薬を隠していたのか」
エマはノードに近づくとすぐさま操作を始めた。
「一体……」
「ちょっと待ってて」
しばらくするとベルトコンベアからコンテナが吐き出されて来た。エマがそれを開ける。中には白い粉が入ったビニール袋がぎっしり詰まっていた。次々と運び込まれてくるコンテナを眺めながらエマは言った。
「どんなにうまく隠しても、一ヶ所に置いておけば必ず誰かに見つかる。だからどこにも行きつかずにビッグシェル中を周回する荷物をいくつも設定して、その中に麻薬を隠していたのよ」
感心するスネークを傍目にエマはまだノードをいじっていた。ＰＤＡを取り出してノードへ接続する。
「今度は何をしてるんだ？」
「うん。ちょっと気になるものを見つけたの。……これは……」
その時、ベルトコンベアの後ろから人影が一人現れた。敵だ。敵は二人の姿を認めるなり駆け去った。スネークはすぐにその後を追おうとしたが、エマにその手を掴まれた。
「待って！」
「離せ！仲間を呼ばれるぞ！」
「わかってる。だから待つの！」
エマはノードをすごい速さで操作し始めた。
「一体何をする気だ？」
「このまま逃げてもすぐに追いつかれちゃう。……だから……」
間もなく扉の外から大勢が集結する気配が流れてきた。足音、指示を飛ばす声、遊底を滑らせる音。間もなく突入してくるはずだ。スネークはまだノードにかじりついているエマへ叫んだ。
「エマ！」
エマがノードから離れる。
「これでなんとか。さあ、逃げましょう！」
扉が開いた。#C0
*/
	B8_エマとＥ脚到着: << EOF
Snake and Emma arrived at Strut E. 
Emma's eyes were drawn to a nearby 
conveyor belt.
"I've got it!" she exclaimed.
"Got what?"
"I know where they hid the drugs." 
Emma went over to the node and 
immediately began punching in 
commands.
"How do you..."
"Just wait and see."
A second later, the conveyor belt spat 
out a crate. Emma opened it. It was 
packed tightly with plastic bags full of 
white powder.
"The syndicate guys knew that no 
matter how good the hiding spot, 
someone was sure to find the drugs if 
they hid them in just one place," 
said Emma as they watched more and 
more crates arrive via the conveyor 
belt. "So they hid the drugs in these 
crates and set them up so they'd just 
go around and around Big Shell without 
actually coming out anywhere."
Snake couldn't help but admire the 
girl's brilliance. He watched as Emma 
took out her PDA and connected it to 
the node.
"Now what are you doing?" he said.
"Hmm. I think I might have found 
something interesting here.... this must 
be..."
Just then, a figure emerged from 
behind the conveyor belt. It was one of 
the terrorists. As soon as he spotted 
Snake and Emma standing there, he 
ran away. Snake moved to pursue the 
escaping terrorist, but Emma grabbed 
his hand.
"Wait!"
"Let go! He's going to go call his 
friends!"
"I know that! Just wait one more 
minute!" Emma began to punch 
commands into the node with incredible 
speed.
"What the hell are you up to now?"
"If we run now, they'll just come after 
us. So..."
Within seconds, Snake could hear the 
sounds of a crowd beginning to gather 
outside the door. Footsteps, voices 
barking orders, rifles being cocked. 
They'd come crashing into the room 
any minute now. Emma, however, 
remained glued to the screen of the 
node.
"Emma!" yelled Snake.
She finally detached herself from the 
node. "There, that should do it. Okay, 
then, let's get going!"
The door burst open. #C0
EOF
//}


//★エマをおいて一人ＤＥ連絡橋に脱出 {
/*
ENGLISH!!
スネークは独りＤＥ連絡橋に脱出した。Ｅ脚扉が閉まる。スネークは息をついた。直後、背後からいくつもの銃声がした。スネークは振り返った。エマの悲鳴が聞こえた。#C0
*/
	Ｂ＿エマをおいて一人ＤＥ連絡橋に脱出: << EOF
Leaving Emma behind, Snake escaped 
to the DE connecting bridge. The door 
of Strut E closed behind him. He let out 
a sigh of relief. Moments later, the 
sound of several gunshots rang out 
behind him. Snake whirled around. The 
sound of Emma's screams pierced his 
ears.#C0
EOF
//}



//★Ｂ９ {
/*
ENGLISH!!
二人はなんとかＥ脚からＤＥ連絡橋へ抜け出た。スネークはＥ脚を振り返った。後方を警戒する。奴等はすぐに追ってくるだろう。銃把へ新しい弾倉を叩き込み、Ｄ脚へ急ぐようエマをうながす。だがエマはのんびりと答えた。
「そんなに急がなくても大丈夫だと思うな」
しばらく待ったがＥ脚のドアが開くことはなかった。閉じた扉の向こう側からドアを叩く音と喧騒が聞こえてくる。スネークはエマを見た。
「君が何かやったのか？」
「ノードを使って、私達が脱出したらＥ脚の扉全てが閉鎖されるように設定したの。あの人達、もうどこへも行けないわ」
エマは胸を張った。Ｃ脚でスネークに助けられた時の経験がヒントになっているという。
二人はＤ脚に入った。スネークはあたりを見回したが、ここにも敵の姿は見えない。奴等はＥ脚に総動員をかけたようだった。おそらくＥ脚以外の脚部に残っている敵はわずかだろう。人質を解放するのも難しいことではなさそうだ。
二人はＤ脚を抜け、シェル１シェル２連絡橋に出た。ここにも敵の姿はない。エマが得意げに微笑む。
「言ったでしょ、大丈夫だって。役に立った？」
「上出来だ」
スネークはうなずきながら、エマがノードにＰＤＡをつなげていたことを思い出した。
「ところで、他にも何かしてなかったか？」
「ばれてた？」
エマはＰＤＡを取り出した。
「ベルトコンベアを操作してた時、貨物制御設定ファイルの近くに変な隠しファイルがあるのを見つけたの。で、ダウンロードしてきたんだけど……」
ＰＤＡにファイルが表示される。ファイルは最初に事故死した職員、つまり麻薬密輸の手引きをしていた職員の手記のようなものだった。
それによると、彼が密輸に協力していたのは、妻子を人質に取られていたからだったという。だが一年前、既に妻子が逃亡を図った末に殺されていたことを知った。彼は絶望した。死のうと考えた。だが思い直した。ただ死ぬわけにはいかない。復讐を果たさなければならない。奴等に出来るだけ多くの血を流させてやらなければならない。
彼は一年かけて計画を完成させた。それは、自分が対立組織に殺され麻薬を奪われたと見せ掛けて、組織同士を抗争させ共倒れさせるという謀略だった。彼は行方不明になった振りをした。直後に自分のお目付け役として組織から派遣されていた男をクレーンで押し潰した。その後、マフィアが調査のために送り込んできた男も海へ突き落とした。思惑通り、組織同士の抗争が始まった。もう思い残すことはない。これから妻子の元にいく。最後に、これを読んでいる人間に対して、もしよければ殺された妻子のために祈ってやってほしい、と綴ってファイルは終わっている。その日付は三日前だった。
スネークはＰＤＡから目を上げた。
「……つまりビッグシェルの悪魔は死んでいる、と？」
エマも驚きは隠せない様子だった。
「そういうことになるわね……実はもう一ファイルを取ってきたんだけど……」
スネークの耳がかすかな飛来音を捉えた。とっさにエマを抱きかかえる。爆発。爆風がスネークをエマもろとも床に叩き付ける。エマがスネークの腕の中でうめいた。
「何！？」
轟音と暴風が頬を叩く。連絡橋の上空、ゆらめく陽炎の向こうにハリアーが滞空していた。
エマが呆然とつぶやく。
「そういえば、Ｅ脚から出られるモノが一つあったっけ……」
機銃掃射が襲いかかる。二人はＤ脚への入口まで駆け戻った。
「どうするの？」
「やるしかないな」
「どうやって？」
「これから考える」
「そんな！」
声を上げるエマを、スネークは有無を言わせずＤ脚扉の向こうに突き飛ばした。
「何するの！？」
「そこからこちらには来るな。決してだ」
「でも！」
「一つくらいは言うことを聞いてくれ」
スネークは笑ってみせた。
「約束したんだ。君をちゃんと連れてかえるとな」
「え！？」
扉が閉まった。
ヘリの羽音が近づいてくるのが聞こえる。オタコンのカサッカだ。
「スネーク、これを使うんだ！」
オタコンが操縦席から箱を投げて寄越した。スティンガーだ。これで何とか勝負になるかもしれない。スネークはハリアーへ向き合った。#C0
*/
	B9_ハリアー戦前: << EOF
Somehow, the two managed to escape 
from Strut E onto the DE connecting 
bridge. Snake turned back towards 
Strut E, keeping an eye on their rear. 
The terrorists would be coming after 
them any moment now. He slapped a 
new cartridge into the chamber of his 
gun and shouted at Emma, urging her 
to move quickly towards Strut D. She 
responded with a nonchalant look.
"There's no need to be in such a hurry, 
you know," she said.
They waited a while, but the door to 
Strut E remained shut. From the other 
side, they could hear a tumult as the 
soldiers bashed in vain against thetightly
sealed door. Snake turned to Emma.
"What did you do?"
"I programmed the node to seal off all 
the doors to Strut E once we were out 
safely. Those guys aren't going 
anywhere." Emma looked extremely 
proud of herself. It seemed she'd 
picked up a thing or two from her 
experience with Snake in Strut C.
They entered Strut D. Snake looked 
around for signs of the enemy, but 
there was no one in sight. The 
terrorists must have sent the bulk of 
their forces to Strut E. If that were 
true, then the rest of the struts would 
be nearly deserted. Rescuing the 
hostages would be a breeze.
Snake and Emma passed through Strut 
D and emerged onto the Shell 1 - 2 
connecting bridge. Here, too, the 
enemy was nowhere to be seen. Emma 
smiled her characteristic smile.
"See, no problems at all. Didn't I say 
I'd come in handy?"
"I have to admit, you were pretty good 
back there," Snake said with a nod. He 
recalled the way Emma had connected 
to the node using her PDA. "By the 
way, weren't you downloading 
something back there?"
"Yeah. Was it that obvious?" Emma 
took out her PDA. "When I was 
activating the conveyor belt, I found a 
suspicious-looking hidden file next to 
the conveyor belt control file. I decided 
to download it, and this is what I 
found...."
The screen of her PDA displayed the 
file. It appeared to be some kind of note
from one of the two employees who'd 
been killed in the accidents -- the same 
ones who were helping the syndicate 
smuggle drugs into Big Shell. In it, he 
explained that he was only cooperating 
with the Russians because his wife and 
child were being held hostage. But last 
year, he found out that they'd already 
been killed while trying to escape. He 
fell into a deep despair, and at one 
point he even thought of killing himself. 
But then he had a vision. It was still too 
early for him to die. No, he had to get 
his revenge on the syndicate first. 
Indeed, he'd make them pay in blood 
for what they did to his wife and child.
Over the course of the next year, he 
devised an ingenious plan. He'd make it 
look like he was killed by a rival 
syndicate that was trying to steal the 
drugs. This would start a war between 
the two syndicates that would end up 
destroying both. First, he faked his own 
disappearance.  Shortly afterward, 
he used a crane to crush the man the 
syndicate had sent to watch him. 
When the Mafia sent a man to 
investigate both incidents, he threw him 
into the ocean. 
Just as he'd expected, a war erupted 
between the two rival syndicates. With 
everything in place, he could finally rest 
in peace. He said he was going to join 
his wife and child. 
Finally, he implored the person reading 
the memo to say a prayer for his 
departed wife and child. The file 
ended there. It was dated three days 
ago.
Snake looked up from the PDA. 
"...I guess this means the 'ghost of Big 
Shell' is dead?"
Emma was unable to hide her shock. 
"I... I guess so.... Actually, I 
downloaded one more file...."
Snake's ears picked up a faint sound of 
something flying through the air. 
Instantly, he grabbed Emma and held 
her tight. There was an explosion. The 
force of the blast knocked Snake and 
Emma off their feet.
"What was that?" Emma groaned in 
Snake's arms.
A thunderous gale of wind beat against 
their cheeks. Hovering over the bridge, 
beyond the wavering heat from the 
blast, was the Harrier.
"Oh yeah, I guess there WAS 
something that could get out of Strut 
E...." Emma muttered, a look of blank 
amazement on her face.The Harrier 
strafed the bridge with waves of 
machine-gun fire. Snake and Emma 
fled back to the entrance to Strut D.
"Now what do we do?"
"I guess we'll just have to kill it."
"How?"
"I'll think of something."
"Are you serious?!" Emma began to 
protest, but before she could finish, 
Snake thrust her through the door of 
Strut D.
"What are you doing?" she demanded.
"Stay there! No matter what happens, 
don't come out here!"
"But!"
"Just listen to me for once, okay?" 
Snake smiled at her. "I already 
promised someone I'd bring you back 
with me."
"Wha-"
The door slammed shut.
Snake heard the sound of a helicopter 
approaching. It was Otacon in his 
Kasatka.
"Snake, use this!"
Otacon tossed down a box from the 
cockpit. Stinger missiles. With these, 
it became somewhat of an even fight. 
Snake turned and prepared to face the 
Harrier in battle.#C0
EOF
//}


/*
//ハリアー戦後
//分岐：
//エマがＥ脚脱出イベントで大量ダメージを受けている場合：
//	B10_0→B10_1A→B10_2
//エマがＥ脚脱出イベントで大量ダメージを受けていない場合：
//	B10_0→B10_1B→B10_2

//Ｂ１０の親リソース {
	B10_エンディング: << EOF
#C1#C2
｜
#C3
EOF
//}
*/

//★Ｂ１０−Ａ {
/*
ENGLISH!!
制御を失ったハリアーはシェル１の下部に激突した。それを見届けたスネークはＤ脚の扉を開ける。今にも泣き出しそうなエマの顔が覗いた。
「終わったぞ」
「遅い！」
「悪かった」
「大丈夫？怪我は？」
「ああ、なんてことない」
スネークとエマは人質となっていたビッグシェルの職員達を解放し、奪回したビッグシェルの回線で警察に通報した。すぐに警察隊がやってくるだろう。敵は相変わらずＥ脚に閉じ込められている。二人は増援の到着をＩ脚のヘリポートで待つことにした。
「そういえば、さっき言いかけた『もう一つのファイル』というのは？」
「そうそう、組織とつるんでた職員の個人ディレクトリを漁ったんだけど、最初に死んだ人、つまり例の手記を書いた人の方から暗号のかかったファイルが見つかって。で、ダウンロードしてみたんだけど……」
エマはＰＤＡを操作した。
*/
	B10_0: << EOF
The Harrier lost control and crashed 
into the bottom of Shell 1. As Snake 
watched it plummet to the earth, the 
door to Strut D opened. Emma peeked 
out cautiously. She looked as if she'd 
been crying.
"It's over."
"What took you so long?"
"Sorry."
"Are you okay? You're not hurt or 
anything?"
"I'm fine, really."
Snake and Emma freed the Big Shell 
employees who had been taken hostage 
and used Big Shell's radio to contact 
the police. The cops would no doubt be 
arriving soon. The terrorists were still 
trapped in Strut E. Snake and Emma 
decided to go to the heliport on Strut I 
to wait for reinforcements to arrive.
"By the way, Emma, weren't you saying 
something about a second file earlier?"
"Oh, right. I managed to dig up the 
personal folders of the guys who were 
working with the syndicate. The first 
guy who died, the guy who wrote the 
memo, had a password-encrypted file 
in his folder. I downloaded it, and..." 
Emma pulled something up on her PDA.#C0
EOF
//}

//Ｅ脚でエマがダメージを受けている場合
//★Ｂ１０−Ｂ０ {
/*
ENGLISH!!
「あれ？」
エマが首をひねる。
「どうした？」
「出ない……」
エマによれば、ファイルを収めた記録用ＩＣが被弾した際に破損したということらしい。エマはファイルを別の記録用ＩＣにダウンロードし直そうとしたが、叶わなかった。ファイルがおいてあったサーバが壊れてしまったようだという。ハリアーが激突した衝撃で、シェル１中央棟の電算室が被害を受けたためだろうということだった。
*/
	B10_1A: << EOF
"Huh?" Emma tilted her head to one 
side.
"What's wrong?"
"It's not coming up...."
The memory IC that was storing the file 
had been shot and damaged. She tried 
to download the file onto another IC, 
but it was no use. The server that the 
file was on had been damaged, she 
said. The data room in the Shell 1 Core 
must have been hit by the shock of the 
Harrier's crash.#C0
EOF
//}



//Ｅ脚でエマがダメージを受けていない場合
//★Ｂ１０−Ｂ１ {
/*
ENGLISH!!
液晶画面にファイルが表示される。
それは麻薬密輸に関する組織への報告書だった。提出前の下書きのようだったが、日付、数量、その他詳細な密輸の記録と今後の計画が記述されている。それによると、彼は『ビッグ・シェル』が麻薬密輸に使われていることを、エマが感づいていると考えていたらしい。
「え？私ちっとも気づいてなかったけど」
エマが自信たっぷりに言う。
「だがこいつは、そうは思ってなかったようだ」
「知性的に見えすぎるのも考えものね」
「人を見る目がなかったんだろうな」
エマが何か言おうとしたが、その前にスネークはファイルの続きを表示した。
そこにはエマに告発される前に彼女を始末するつもりだと書かれていた。
「そんな！」
エマが思わず声を上げる。
「何か危ない目にあったか？」
「いいえ、何にも……ちょっと貸して」
エマはＰＤＡをいじり始めた。
「このファイルの日付……彼が行方不明になる前日になってる……」
「……君を殺す前に誰かに殺されたと？」
「わからないけど……そうかも」
「ではさっきの手記はどうなる？」
エマはしばらく考え込んでから言った。
「……でっちあげかもしれない……あの手記、置かれ方が不自然だった。いかにも発見してくれって感じで……」
二人は考え込んだ。確かに記述内容の細かさから言って報告書の方に信憑性がある。だとすると、誰が組織とつながっている職員達を殺したのか。あの手記を偽造し、システムの中に隠したのは誰なのか……。
やがてエマがぽつりとつぶやいた。
「……私、聞いたことある。『ビッグ・シェル』の悪魔……息子さんは麻薬で死んだんだって……」
エマが憶測を口にする。息子の死にショックを受けて自殺を図った職員。彼は死ぬことが出来ずに『ビッグ・シェル』内を徘徊する怪人となった。その心にはいまも消えない麻薬に対する怒りがある。ある日、『ビッグ・シェル』が麻薬密輸に利用されていることを知り……。
「そんなこと、あるわけないよね！」
エマは笑おうとしたようだが、口元をひきつらせるだけに終わった。
*/
	B10_1B: << EOF
A file appeared on the PDA's LCD 
screen.
It was a report addressed to the 
syndicate concerning their drug 
smuggling activities. Judging by its 
appearance, it was still just a draft copy 
and probably hadn't been sent yet. 
Nevertheless, it contained a wealth of 
information related to the syndicate's 
drug operations -- dates, amounts, and 
other key details. The report also made 
mention of the man's suspicions that 
Emma had caught onto the fact that 
they were using Big Shell to smuggle 
drugs.
"Huh? But-- I had no idea!" Emma 
exclaimed.
"That's not what he thought."
"I suppose I do radiate an aura of 
intelligence, don't I?"
"Guess he wasn't a very good judge of 
character."
Emma's mouth opened in indignation, 
but before she could come up with a 
retort, Snake displayed the next part of 
the file. It described the man's 
intentions to dispose of Emma before 
she exposed them to the police.
"No way!" said Emma without realizing 
it.
"Has anyone tried to hurt you before 
this?"
"No, not that I know of.... Let me see 
that for a minute." Emma began to 
examine the PDA "This file is dated the 
day before he disappeared...."
"...Implying that someone killed him 
before he could get a chance to kill 
you?"
"I'm not sure.... Yeah, maybe."
"In that case, why did he leave that file 
behind?"
Emma looked pensive for a moment. 
"...Maybe it's all a big set-up. Why
 would he put the memo in such a 
conspicuous place, anyway? It's 
almost like someone wanted us to find 
it...."
Both Snake and Emma were lost in 
thought. The report certainly looked 
authentic enough, judging by its 
meticulous attention to detail. But if the 
report were real, then who was killing 
off the employees who were working 
for the syndicate? Who had forged the 
memo and hidden it in the system?
Finally, Emma murmured softly to 
herself. "I heard something once... 
about the ghost of Big Shell.... they 
say his son died of a drug overdose...."
Emma outlined her speculation to 
Snake. One of the employees, shaken 
by the death of his son, had planned to 
commit suicide. But he failed, and 
instead began wandering around Big 
Shell as a specter of his former self. 
It is believed that he wanders around 
Big Shell even now, maintaining a deep 
hatred of drugs and the people who sell 
them. One day, he discovered that Big 
Shell was being used to smuggle 
drugs....
"But no one would ever believe a story 
like that, right?" Emma tried to smile, 
but all she could manage was a small 
twitch at the corner of her mouth.#C0
EOF
//}



//★Ｂ１０−Ｃ {
/*
ENGLISH!!
ヘリの羽音が聞こえてきた。エマが空を仰ぐ。
「警察？」
カサッカが姿を現した。その黒い機影を見てエマは首を傾げた。
「……違うみたい」
スネークはカサッカを見上げた。
「あれは俺の迎えだ」
操縦席の窓からオタコンの顔が覗いている。その目には緊張と恐れ共に、何か覚悟のようなものが見えた。
「どうやら君の迎えでもあるようだな」
「え？」
「すぐにわかる」
カサッカは降下を始めた。風が強くなる。エマは髪を抑えた。カサッカがさらに近づく。エマは操縦席の人影を認めた。息を呑む。もう一度目を凝らす。立ち尽くす。涙がこぼれる。着陸したカサッカの回転翼が止まるのも待たず、エマは兄の元へ駆け出した。#C0
*/
	B10_2: << EOF
Snake heard the sound of a helicopter 
approaching.
Emma gazed up at the sky. "Is that the 
police?"
The Kasatka came into view. Upon 
seeing its dark shadow, Emma tilted 
her head to one side, puzzled. "That's 
not the police. Who is that?"
Snake looked up at the Kasatka. 
"That's my ride."
Otacon's face peered out of the 
cockpit. Snake could detect fear and 
apprehension in his eyes, but there 
was something else, as well -- a kind 
of grim determination.
"And I think he's got room for you, too."
"Me?"
"You'll understand soon enough."
The Kasatka began to descend. The 
wind from its rotors beat fiercely upon 
their faces. Emma brushed her hair 
back from her cheeks. As the helicopter 
drew closer, Emma recognized the 
figure in the pilot's seat. She gasped. 
Her eyes remained frozen. She stood 
perfectly still, as if rooted in place. 
Tears began to roll down her cheeks. 
The Kasatka touched down at last. 
Without even waiting for the rotors to 
stop spinning, Emma rushed to meet 
her long-lost brother.#C0
EOF
//}


}
