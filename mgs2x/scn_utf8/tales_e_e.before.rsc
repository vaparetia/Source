//External gazer
/*
	tales_e_j.h
	    スネークテイルズ紙芝居用リソースヘッダ

	2002/06/04 S.Mukaide
	$Id: tales_e_e.before.rsc,v 1.1 2002/08/26 05:15:41 usr03005 Exp $


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
//	テイルズＥ(ごりゅりゅぎょんが出てくる話)
//---------------------------------------------------------------------------
resource テイルズ＿Ｅ__e d:TITLE_FONT_AREA {
//resource テイルズ＿Ｅ {

//最初
//★Ｅ１ {
/*
俺はＢ脚扉を開けた。ＡＢ連絡橋に出る。敵の姿はない。無線機からオタコンの指示が聞こえる。
「いいかい、スネーク。もう一度説明するよ。今回の僕等のターゲットは世間を騒がす正体不明のＵＭＡ、謎の怪獣、通称ゴルルゴンだ」
何を言っている。騒いでいるのはオタコンとメイ・リンだけだし、その珍妙な通称もお前ら二人にしか通用しないだろう。俺はそう思ったが、口にするのはやめた。オタコンの指示が続く。
「分かってるだろうけど、ターゲットといっても傷つけたりしちゃ駄目だ。まずは写真を撮るんだ。逃げられたりする前にね。ようやくここまで辿り着いたんだ。絶対にしくじるんじゃないぞ！」
仕方ない......。いかに馬鹿馬鹿しくとも任務は任務だ。俺は覚悟を決めて、ＡＢ連絡橋に踏み出した。#C0
*/
	E1: << EOF
I opened the door to Strut B. I went 
out on the AB connecting bridge. No 
enemies sighted. Otacon gave me 
instructions via Codec.
"Listen carefully, Snake. I'll explain 
again. Our target this time is that 
mysterious unidentified monster that's 
got the whole world in an uproar. They 
call it the Gurlugon."
What is he talking about? The only 
ones in an uproar are Otacon and 
Mei Ling, and I bet they're the only 
ones using that nickname, too. That's 
what I was thinking, anyway. But I 
didn't say anything. Otacon continued.
"I think you already know this, but do 
NOT hurt the target! Take a picture of 
it before it has a chance to get away. 
You made it this far, so don't do 
anything stupid!"
My hands were tied. Ridiculous or not, 
a mission is a mission. I made up my 
mind, and began walking across the AB
connecting bridge. #C0
EOF
//}

//ゴルルゴンが海中から出現しそうなシナリオデモ後
//★Ｅ２ {
/*
俺は逆巻き、泡立っていく海面を半ば呆然と見つめていた。海面が徐々に隆起していく。その中から巨大な何かが現れようとしているのは間違いなかった。この不可解で滑稽極まる『怪獣探索任務』は、俺の予想を裏切る形で最終局面を迎えようとしている......。
｜
なぜ俺がこんなものに駆り出されることになったのか。
そう、あれは三日前のことだ。俺はＮＹにあるフィランソロピーのアジトで、オタコンが組んだ新しいＶＲシステムとやらを試していた。#C0
*/
	E2: << EOF
I stared at the churning whirlpool the 
sea had become in stunned silence. 
The surface of the sea was beginning 
to rise. No question about it, something
huge was getting ready to make an 
appearance. My mysterious, ludicrous 
quest, my so-called "Monster Search 
Mission," looked to be coming to an 
unexpected conclusion...
｜
How did I get involved in this ridiculous 
mission? It all began three days ago. 
I was testing Otacon's latest invention, 
the VR system, at Philanthropy's 
hideout in New York.#C0
EOF
//}

//回想ＶＲ後
//★Ｅ３－０ {
/*
目の前に広がっていた世界が消えた。全身に本来の感覚が戻り、俺はタンカーで爆弾解体任務を行なっていたのではなく、ＶＲ訓練用シートに座っていたのだということを思い出す。ヘッドセットを外した俺に向かってオタコンが言った。
*/
E3_0: << EOF
The world disappeared before my eyes.
As my body returned to its senses, 
I realized that I wasn't actually on a 
bomb-disposal mission on a tanker; 
I was sitting in the VR training seat. 
I pulled off the headset and Otacon 
turned to speak.#C0
EOF
//}

//★Ｅ３－１Ａ {
/*「イマイチだったね。どうしたんだい？」
*/
E3_1A: << EOF
"That didn't look too good. 
What happened?"#C0
EOF
//}

//★Ｅ３－１Ｂ {
/*
「なかなかいい調子だね」
*/
E3_1B: << EOF
"You were doing just fine."#C0
EOF

//★Ｅ３－１Ｃ {
/*
「やるじゃないか！さすがはスネークだ」
*/
E3_1C: << EOF
"Great job! I knew you could do it, 
Snake."#C0
EOF

//★Ｅ３－２ {
/*
俺はあいまいにうなずいた。オタコンが得意げに聞いてくる。
「どうだい、新しいＶＲシステムは？」
「悪くない」
「それだけかい？」
不満気に首をかしげる。
「いや正直言って驚いている。ここまでよく出来ているとは思わなかった」
「だろ！このＶＲシステムは最近手に入れたコペルソーン・エンジンって仕組みを採用してるんだ」
「コペルソーン・エンジン？」
俺が聞き返すと、オタコンは嬉しそうに眼鏡を直した。
「コペルソーン・エンジンってのは、最近一部で大脚光を浴びてる量子計算システムなんだ。量子コンピュータは知ってるだろ......」
オタコンが語り始めた。キュービットがどうの、テンソル積がこうの。俺はいつもの癖でつい聞き返してしまったことを２時間３７分に渡って後悔することとなった。
「......って感じで、今までとは比較にならない情報量を持つ仮想空間を計算し構築することが可能になったって訳さ。それを信号化してそのヘッドセットから君の脳に流し、各感覚野で情報を直接構成させると同時に肉体からのフィードバック情報はカットする。そうやって光や音だけでなく匂いや手触りまである、現実世界と全く変わらない仮想世界を実現出来るってわけなのさ」
話が途切れた。このチャンスを逃してはいけない。
「なるほど、よくわかった......」
俺はそう言って椅子から立ちあがろうとしたがオタコンに止められた。
「ちょっと待った。もう少しやってみようよ。次の状況は『ビッグ・シェル』で君が人質の退路を確保するために敵を排除するところだ」
オタコンはよほどこの玩具を試したいらしい。俺はしぶしぶシートに掛け直した。オタコンが説明を続ける。
「君は一定時間以内に敵をすべて排除しなければいけない。いいね。じゃあいくよ」
俺はヘッドセットを付け直し、目を閉じた。まばゆい閃光が弾ける。次の瞬間、俺の周囲にはとても作り物とは思えない世界が広がっていた。#C0
*/
E3_2: << EOF
I nodded reluctantly. Otacon asked me 
with pride:
"What did you think about the new VR 
system?"
"Not bad."
"That's all?" Unsatisfied, he cocked 
his head.
"I can't believe it, to tell you the truth. 
I didn't expect it to work so well."
"I know! The VR system incorporates 
something called the Koppelthorn Engine.
I just got it the other day."
"The Koppelthorn Engine?"
Hearing the question, Otacon removed 
his glasses with a smile on his face.
"The Koppelthorn Engine is a quantum 
calculation system that's been getting 
a lot of attention in certain quarters 
recently. You know about quantum 
computing, right?" began Otacon.
He went on and on, q-bits this and 
Tensor products that. For the next two 
hours and thirty-seven minutes, 
I regretted my stupidity in having asked.
"...in short, it allows the construction of 
virtual-reality environments containing 
a hitherto unprecedented amount of 
information. That information is 
converted into a signal and conveyed 
into your brain via the headset, directly 
synthesizing information for each of 
your senses while simultaneously 
cutting off feedback from your actual 
body. That way, it's possible to realize 
a virtual reality environment that is 
completely indistinguishable from the 
real world, down to lights, sounds, even
your sense of smell and touch."
He's taking a breather. I can't miss this
opportunity.
"I see, I understand now..." I said, 
trying to get up out of the seat. 
But Otacon stopped me.
"Wait a second. Let's try it again. 
The next scenario will be eliminating 
the enemies to secure an escape path 
for the hostages on Big Shell."
I guess Otacon just can't wait to try 
out his new toy. Reluctantly, I took the 
seat again. Otacon continued his 
briefing.
"Your mission is to eliminate all 
enemies within a certain period of time.
Ready? Here we go!"
I put the headset back on and closed 
my eyes. A burst of light dazzled my 
eyes. The next moment, I found myself 
surrounded by a world I could hardly 
believe was nothing more than a 
construct.#C0
EOF
//}

//ＶＲ終了後
//★Ｅ４－０ {
/*ＶＲが終了した。仮想空間とは思えぬ仮想空間が消え、体に感覚が戻る。俺はヘッドセットを外した。
「どうだい？」
オタコンが聞いてくる。俺は二回試してみて気づいた点を言ってみた。
「確かによく出来ているが、体の感覚に何と言うか違和感のようなものを感じるな」
「どういう違和感だい？」
「説明しづらいんだが......どこか他人の体に乗り移ったような奇妙な感じがするんだ......」
「脳を疑似信号で満たすからね。理論的には肉体の感覚器からの刺激と同じものが再現されているはずなんだけど......」
俺とオタコンが話し合っていると、突然ドアが開きメイ・リンが入ってきた。
「ちょっと、これ見てよ！！」
メイ・リンは手に持ったタブロイド新聞を投げて寄越した。
*/
E4_0: << EOF
The VR simulation ended. The virtual 
reality that could easily have passed 
for the real thing disappeared, and my 
body came back to its senses. 
I removed the headset. 
"How was it?" asked Otacon. I told him
what I'd noticed during my two trials. 
 "It's well done, no question. But it 
feels kind of unnatural, like something's
wrong with my senses."
"Unnatural? In what way?"
"It's tough to explain, but... I feel like 
I'm possessing someone else's body..."
"That's because of the pseudo-signal 
filling your brain. Theoretically, it should
be possible to replicate the exact same 
stimulus as from the sensory structures
of your physical body."
Midway through my discussion with 
Otacon, the door opened, and in strode 
Mei Ling.
"Hey, check this out!"
Mei Ling threw us the tabloid she'd 
brought with her.#C0
EOF
//}

//インチキ新聞ＰＩＣＴ表示後
//★Ｅ４－１ {
/*
それは『ビッグ・シェル』近辺に巨大な生物のようなものが現れたとかいう記事だった。見出しは「『ビッグ・シェル』に怪獣出現！？」全くもってくだらない。
「これがどうしたというんだ？」
俺は率直な感想を述べた。メイ・リンが目を丸くする。
「どうしたって、スネーク、怪獣よ！？」
オタコンが机を叩く。
「そうだよ、怪獣だぞ！」
「......どうせ合成写真だろ？」
しごく真っ当な見解を述べると、二人にものすごい目つきで睨まれた。二人は俺を無視することにしたようだ。
「しかしメイ・リン、信じられるかい？ＮＹに怪獣なんて！スゴい！」
「ホント、スゴいわよね！スゴすぎる！！」
「『ビッグ・シェル』かぁ......見に行きたいなぁ」
「ね、オタコン、思いついたんだけど、この怪獣捕まえるってのはどう？」
「いいね！是非捕まえよう！生物学いや全科学にとって大きな進歩になるかもしれない！」
「そうよね！で、捕まえて怪獣園作ったらお客さんいっぱい来るかしら？」
「勿論！やはりこの背ビレからして地底から来たんじゃないかと思うんだけど......どう思う？」
「そうかもね。人形とかＴシャツとかも作ってみたらいけるかな？」
「悪くないよ！でも宇宙からの使者という線も捨て難いよな......」
「確かに。でもまず商標登録しちゃった方がいいかもね」
話がかみ合っていないと俺には思えるのだが、二人は多いに盛り上がっている。俺はまさかと思いつつ聞いてみた。
「おい、お前ら、本気でこいつを捕まえようとか考えてるんじゃ......」
オタコンが目をむいた。
「スネーク！何言ってるんだ！怪獣だぞ！」
メイ・リンが椅子を蹴った。
「そうよ！怪獣なんだから！」
「だが......」
オタコンが身を乗り出す。
「未知の怪獣が現れたんだぞ！科学者の名にかけて黙ってはいられない！」
メイ・リンが目を吊り上げる。
「フィランソロピーの資金繰りがどうなってるか知らないの？今月の家賃も危ないのよ！」
「しかし......」
「捕まえないでどうするんだ！？」
「黙って見過ごすつもりなの！？この意気地なし！！」
「とは言っても......」
なお渋る俺に向かってオタコンとメイ・リンが同時に叫んだ。
「やるんだよ！！」
「やるのよ！！」
もはや、俺にはわかったと言うより他なかった。
｜
という次第で、俺は『ビッグ・シェル』に降り立った。３時間前のことだ。#C0
*/
E4_1: << EOF
It was an article describing the 
appearance of some kind of huge 
creature near Big Shell. The title 
shouted, "A Monster at Big Shell?!" 
Completely ridiculous.
"So what?" I said without hesitation. 
Mei Ling blinked in surprise.
"'So what?! It's a monster, Snake!"
Otacon hit the table.
"That's right! It's a monster!"
"That picture has to be fake." 
It was the most normal opinion in the 
world, but you wouldn't have known by 
the way they glared at me. Finally they 
simply decided to ignore me.
"It sounds incredible, doesn't it, 
Mei Ling? A monster in New York? 
It's amazing!"
"I agree! It's amazing! Too 
amazing!!"
"So it's at Big Shell... I want to go and 
see it..."
"Hey, Otacon, I have an idea. How 
about catching the monster? What do 
you say?"
"Great idea! Let's get him! It'd be a 
real boon to biology -- no, I take that 
back! To ALL of the sciences!"
"That's right! Hey, do you think we 
could get a lot of customers if we built 
a monster zoo?"
"Of course! Judging from the dorsal 
fin, 
I think this monster must have come 
from underground... What do you 
think?"
"Maybe. What about making stuffed 
animals or T-shirts?"
"Not bad! But I can't help thinking that 
it might be a messenger from space, 
you know?" 
"Oh, totally. But we'd better register a 
trademark first."
They excitedly kept on with what 
seemed to me to be a totally disjointed 
conversation. I couldn't believe they 
were actually considering it, but I had 
to ask.
"Hey, I hope you're not serious about 
catching this thing..."
Otacon regarded me with utter shock.
"Wake up, Snake! We're talking about 
a monster here!"
Mei Ling kicked a chair.
"He's right! This is about a monster!"
"But..."
Otacon leaned toward me. 
"A monster of unknown origin has 
appeared! As a scientist, I can't 
simply ignore it!"
"Do I have to remind you about 
Philanthropy's financial situation?! 
We don't even know if we have enough
to cover our rent this month!" snapped
Mei Ling with fire in her eyes.
"But..."
"What do you propose instead of 
catching it?!"
"Are you just going to quietly let this 
opportunity slip by?! You coward!"
"But, but..."
Otacon and Mei Ling kept up their 
simultaneous tirade as I hesitated.
"We've got to do it!"
"We've got to do it!"
At this point, there was nothing to say 
except "Okay."
That's the story. And so I came to Big 
Shell. That was three hours ago.#C0
EOF
//}


//Ｃ脚にはいった
//★Ｅ５ {
/*
Ｃ脚に入った俺はオタコンに無線連絡を入れた。オタコンが聞く。
「どうだい、スネーク。何か手がかりはつかめたかい？」
「いや。だが妙な奴等がうろついてる」
俺はＣＤ連絡橋で見た兵士のことを話した。オタコンが言う。
「きっと僕等の他にもゴルルゴンを狙ってる奴等がいるんだよ」
「そうとは思えないが......」
「いやそうに違いない！」
オタコンは自信たっぷりに断言する。
「とにかく奴等に見つからないように進むんだ。ゴルルゴンの目撃情報はビッグシェルの南西部に集中している。ＡＢ連絡橋に向かってくれ」
ここまで来るとオタコンに従うしかない。俺はＡＢ連絡橋に向かうことにした。#C0
*/
E5: << EOF
I contacted Otacon via Codec as 
I entered Strut C. 
"What's up, Snake? Found any 
clues?" asked Otacon.
"Negative. But I've seen some strange 
people wandering around."
I told Otacon about the soldiers I'd 
seen on the CD connecting bridge. 
Otacon replied:
"I bet they're trying to get Gurlugon, 
just like we are."
"I don't think..."
"It has to be!" he swore with 
confidence.
"Anyway, just move ahead without 
getting caught. Most of the sightings 
of Gurlugon have been southwest of 
Big Shell. Head for the AB connecting 
bridge."
I'd come this far, so there was nothing 
to do but follow Otacon. I started 
moving towards the AB connecting 
bridge.#C0
EOF
//}

//ＡＢ連絡橋入った後
//★Ｅ６ {
/*
以上がことの次第だ。かくのごとく、俺は怪獣探しに駆り出され、そして今、俺の眼前に何かが姿を現そうとしている。
こうなれば覚悟を決めるしかない。俺はカメラを確認した。何が出てくるのかはわからない。だが必ず写真を収めてやる。#C0
*/
E6: << EOF
Anyway, that's the story. Here I am, 
forced to hunt down a monster, and 
now it looks as though something's 
about to appear before my very eyes.
At this point, there was nothing to do 
but get ready. I checked over the 
camera. I have no idea what I'm about 
to see, but I'm going to shoot first and 
ask questions later.#C0
EOF
//}

//敵兵ラッシュイベントの後
//★Ｅ７－０ {
/*
「邪魔をしないで！」
オルガが叫んだ。武器は手にしていない。攻撃の意志はないようだ。俺は銃を収めながら聞いた。
「なぜお前がここに？」
オルガが少し笑った。その笑みに俺は違和感のようなものを覚えた。雰囲気が違う。以前あった影や悲壮さのようなものが感じられない。そう、まるで別人のようだった。
「そのお前、というのが問題ね。私はあなたの知ってる私じゃないし、あなたの知ってる私を私は知らない」
「何を言ってる？」
「私は私の宇宙へ帰るための鍵を探している。それがあのおかしな生き物なのよ。アレはあなたの宇宙のモノじゃない。......私もね。私達はそれぞれ別の宇宙から来た。このままでは、あなたの宇宙も大変なことになるわ！」
オルガがよく分からない言葉を並べ立てる。俺が戸惑っていると、オルガは眼下の海面を見下ろし、歯噛みした。
「いけない......奴が逃げる。いい！？ＶＲシステムに気を付けなさい！あれは他の宇宙を覗く。あれが全ての元凶なのよ！」
オルガは連絡橋から身を躍らせた。
「待て！」
俺が橋の手すりから身を乗り出すと、既にオルガは海中に消えていた。オルガの部下達も次々とオルガの後を追って海へ飛び込んでいく。
俺は一人連絡橋に取り残された。もうゴルルゴンは現れなかった。#C0
*/
E7_0: << EOF
"Get out of my way!" screamed Olga.
She isn't armed. She doesn't seem to 
be making a move to attack. I begin 
talking to her as I holster my gun.
"What are you doing here?"
Olga smiled a little. But something 
about her smile isn't right. She looks 
so different. She doesn't carry the 
sense of darkness and tragedy 
I remember. In fact, she looks like a 
totally different woman.
"Don't get too friendly. I'm not the me 
that you know, and I don't know the 
me that you do know."
"What are you talking about?"
"I'm looking for the key to get back to 
my universe. And that strange creature 
is it. It doesn't belong in your universe. 
Neither do I. We're from a different 
place. And your universe is in for 
trouble, too, if you just ignore things!"
I just couldn't wrap my brain around 
Olga's story. I was lost. Grinding her 
teeth, Olga looked down to the surface 
of the sea.
"Damn. He's getting away. Listen! 
Check the VR system! It's allowing 
you to peek into another universe. 
That's what's causing all this trouble!" 
shouted Olga as she jumped from the 
connecting bridge.
"Wait!"
I leaned over the railing, but Olga had 
already disappeared into the sea. 
Olga's henchmen were jumping into the 
sea one after the other.
I was left on the connecting bridge 
alone. I couldn't see Gurlugon 
anymore.#C0
EOF
//}

//★Ｅ７－１ {
/*
アジトのテーブルの上に俺が撮ったゴルルゴンの写真が載っている。俺はメイ・リンに聞いた。
「やはり今日も？」
「ええ。出なかったみたい」
あれから一週間。ゴルルゴンの目撃情報は絶えていた。次はいつ現れるか分からない。いやもう現れないかもしれない。
｜
俺はオルガの言葉を信じた。嘘を言っているとは思えなかったし、何より、あの馬鹿げた怪獣の存在自体が証拠とは言えまいか。あんなものは俺達の生きるこの宇宙ではありえない。だが他の宇宙でならありうるのかもしれない。オタコンは「並行宇宙」という言葉を口にした。
｜
オタコンは例によって必要以上に詳細な説明を行なってくれたが、要するに宇宙は単一の存在ではなく可能性の数だけ無数に並行して存在すると言う考え方だ。その中には俺達の宇宙とは俺の髪の毛の数が一本違うだけのものもあれば、ゾウリムシが文明を築いている宇宙もあるかもしれない。怪獣が当たり前のように存在する宇宙も。
オタコンによれば、その本来お互いに交わらないはずの並行宇宙が入り乱れつつあるのではないか、ということだった。だから俺達の知っているオルガは別の並行宇宙のオルガと入れ替わり、また別の宇宙からあの怪獣がやってきたのではないかという。そしてその現象は現在も進行中なのではないかとオタコンは危惧していた。
｜
もしその推測が正しければ大変なことになる。例えば一時間後、俺は別の宇宙の俺と入れ替わってしまうかもしれない。もしかしたらオタコンは既に入れ替わっているかもしれない。それ以前に、月が地球に衝突する宇宙と入れ替わったらどうなる？何が起こるか想像もつかなかった。
幸いにして今までのところ、月は落ちてきていない。少なくとも俺達のいるこの宇宙には。だが明日もそれが保たれると言う保証はなかった。
｜
「ＶＲシステムの解析は？」
俺の質問にオタコンは首を振った。
「さっぱりだ......」
ＶＲシステムが全ての元凶だ、というオルガの言葉を聞いたオタコンは青くなった。新しいＶＲシステムの基幹となっているコペルソーン・エンジンとかいうモノは、Ｄｒ．コペルソーンなる人物が発表した量子計算機だというが、その原理は全く不明であるという。事実、その文字どおりのブラックボックスを開けてみると、中では材質不明の鉄球や理解不能な回路らしきもの、怪しげな歯車などがガチャガチャと動いているだけだった。原理は不明。だがなぜか動く。「一部で話題沸騰」とはそういう意味だったらしい。多数の疑似科学愛好家が研究にあたっているというが、ＵＦＯの破片を利用したものだ、太古のオーパーツだ、などという説が出るだけで、その仕組みはいまだ誰にも解明出来ていないという。その上調べてみるとＤｒ．コペルソーンという人物自身が全くの正体不明で、その姿を見た者すらいないらしい。
｜
しかし、その謎のＶＲシステムが並行宇宙の混乱と何の関係があるのか。オルガはＶＲシステムが他の宇宙を覗くことが原因だと言った。メイ・リンの仮説によればこうだ。コペルソーン・エンジンを用いたＶＲシステムは、計算によって仮想空間を構築しているのではなく、何らかの方法で並行宇宙の壁を越え、与えられた条件を満たす他の並行宇宙を「覗いて」、その情報を仮想空間としてＶＲシステムに渡しているのではないか。その際の観測と干渉が並行宇宙間の混乱を引き起こしているのではなかろうか。
また、メイ・リンはこうも言った。オルガが例のゴルルゴンを探していたのは、それこそが並行宇宙の歪みの原点だからではなかろうか。ゴルルゴンを中心に宇宙は歪み混乱していく。そうであるならばゴルルゴンを退治しなければ、宇宙が入り乱れていくのを止めることは出来ない。
｜
そんな理由で俺達はゴルルゴン探索を続けていた。だが怪獣の行方は一向につかめず、ＶＲシステムの解析も進まないでいる。だがこうしている間にも並行宇宙の混乱は続いているのだろう。幾つかの宇宙はまさに今破滅を迎えているかもしれない。もしかしたら数秒後には俺達の宇宙もそうなるかもしれない。俺達は焦っていた。
｜
突然メイ・リンが言った。
「でも......ゴルルゴンを倒せばいいのなら、方法はあるかもしれない」
「どういうことだい？」
目を上げたオタコンに、メイ・リンは自説を披露した。
「ＶＲシステムが本当に他の並行宇宙に干渉することが出来るのなら、ＶＲシステム内でゴルルゴンを倒せばいいんじゃないかしら？そうすれば元の宇宙にいたゴルルゴンを倒したことになって、私達の宇宙にいるゴルルゴンも消えるはず......」
オタコンが控えめに異議を唱えた。
「それはちょっと無理があるんじゃないかな？」
メイ・リンが反論する。
「でも！ここでずーっと写真眺めてるわけにはいかないでしょ？何かしないと！そうでしょ、スネーク！」
「それはどうかな......」
俺も異議を唱えてみた。いくら何でも論理が乱暴過ぎるように思える。しかし結局やることになるだろうとも感じていた。何かすればいいというものでもないが、他に解決手段らしきものがないのも事実なのだ。
そして数時間後、俺はＶＲ空間の中でゴルルゴンと対峙していた。#C0
*/
E7_1: << EOF
My photo of Gurlugon sat on the table 
at the hideout. 
"No sign today?" I asked Mei Ling.
"No. Looks like it hasn't shown up 
today, either."
It's been a week. No new sightings of 
Gurlugon. No one knows where he'll 
show up next. 
Maybe he'll never show up ever again.
I believed Olga. It didn't seem like she 
was lying, and what's more, the 
existence of that ridiculous monster 
was proof in and of itself. There's no 
way something like that could live in 
our world, but it might be possible in 
another. Otacon brought up a theory 
about "parallel universes."
As usual, Otacon gave far more of an 
explanation than necessary. But in a 
nutshell, the idea is that rather than a 
single universe, there are countless 
universes existing in parallel. That 
means one universe might be exactly 
the same as mine, except the number 
hairs on my head is off by one. In 
another universe, civilization might 
have sprung from paramecia. There 
could even be a universe full of 
monsters. Otacon thinks these 
universes are supposed to be mutually 
exclusive, but for some reason have 
begun randomly intersecting. That's 
how the Olga we know became 
switched with an Olga from a parallel 
universe, and how that monster came 
to be in ours. And Otacon warned 
that the phenomenon seemed to be 
progressing as we spoke.
But if his theory is correct, we're in for 
big trouble. For example, it's possible 
that an hour from now, I may switch 
with another version of myself from a 
different universe. Otacon might have 
already been switched with a different 
Otacon by now. What would happen to 
us if we switched to a universe, say, 
where the Moon is about to collide with 
the Earth or something? It was simply 
impossible to predict what might 
happen next.
Fortunately, the moon hasn't smashed 
into us yet - or at least not in our 
universe. Of course, that doesn't 
guarantee we'll be safe tomorrow.
"How's the analysis of the VR system 
coming?"
Otacon shook his head.
"I'm not getting anything at all..."
｜
Otacon looked sick when he heard Olga
said that the VR system was the root 
of the problem. He told me that the 
Koppelthorn Engine, the heart of the 
new VR system, is a quantum 
computational device designed by 
Dr. Koppelthorn, but the principles 
behind it remain unknown. He opened 
the housing, a literal black box, finding 
nothing inside except some mysterious 
circuits, clattering gears, and what 
appeared to be some iron spheres. 
But somehow, it worked. That's 
probably why it's been such a source 
of excitement for some people. 
Pseudo-science aficionados have 
examined the thing, but the only 
conclusions so far have centered 
around the possibility that it makes use 
of components from UFOs or artifacts 
from a lost civilization. Meaning, 
nobody's figured out what makes it tick.
And what's more, nobody has ever 
seen this Dr. Koppelthorn 
himself or herself, either.
So what's the connection between this 
mysterious VR system and the collision 
of the parallel universes? Olga had 
said that the root of the problem was 
the VR system's "peeking" into other 
universes. But Mei Ling had her own 
theory. The Koppelthorn Engine-based 
VR system wasn't simply constructing 
a virtual reality through its calculations, 
but managed to get around the wall 
separating the parallel universes, 
peering into parallel realities that 
satisfy certain requirements, and 
incorporating that information into the 
VR system's virtual reality construct. 
Perhaps that observation and 
interference was creating the 
disturbance in the parallel universes. 
But that wasn't all, said Mei Ling. 
Perhaps Olga was searching for the 
Gurlugon because it was the cause of 
the distortion among the parallel 
universes. The Gurlugon had become 
the fulcrum around which space had 
warped. And if that was true, there 
would be no way to stop the distortion 
without getting rid of the Gurlugon 
itself.
And so for that reason, we continued 
searching for the Gurlugon. We'd lost 
all trace of the monster, and hadn't 
made any further progress into 
analyzing the VR system. But the 
disturbance of the parallel universes 
might well be continuing. It was 
possible that some of the universes 
might even be in peril. And it was 
equally possible that our universe 
could follow the same path moments 
later. We were completely frazzled.
｜
Suddenly, Mei Ling spoke.
"If killing Gurlugon will make things 
right, there might be a way."
"What do you mean?" 
Otacon raised his face, and Mei Ling 
began her explanation.
"If it's true that the VR system can 
interfere with other parallel universes, 
why not try killing Gurlugon within the 
VR system? That could mean we're 
actually killing the original Gurlugon, 
and the Gurlugon in our universe could 
disappear..."
"Isn't that just a little too extreme?" 
objected Otacon.
To which Mei Ling shot back:
"We can't sit back and keep staring at 
the photo! We've got to do something!
You agree with me, right, Snake?!"
"I'm not sure..." I answered. Her logic 
seemed extreme no matter how I 
looked at it. But at the same time, I had
the feeling that I'd end up doing it 
anyway. Doing something for the sake 
of doing it wasn't the answer, but I had 
to agree we still hadn't come up with a 
solution for the problem.
A few hours later, I squared off against 
Gurlugon inside the VR system.#C0
EOF
//}

//★ＶＲＥＸＩＴ紙芝居 {
/*
俺はＶＲを終了した。ヘッドセットを外すと、オタコンの怒声が飛び込んできた。
「スネーク、何でやめちゃうんだ！？」
「気が進まなくてな」
「何言ってるんだ！今がどういう状況だかわかってるのか！？もう一度行くよ！」
俺はヘッドセットを付け直した。オタコンが機器を操作する。再びＶＲが始まった。#C0
*/
E_VR_EXIT: << EOF
I stopped the VR system. As I removed 
the headset, Otacon yelled at me.
"Snake, what are you doing?!"
"I just didn't feel like it."
"What are you talking about?! Do you 
realize the trouble we're in?! We're 
doing it again!"
I replaced the headset on my head. 
Otacon took control of the system. 
The VR simulation began again.#C0
EOF
//}

//ゴルルゴン戦終了後
//★Ｅ８－０ {
/*
ゴルルゴンは倒れ、ＶＲは終了した。だが何か手応えのようなものが無い。ヘッドセットを外した俺は、オタコンに言った。
「なあ、本当にこれで成功したのか......」
オタコンが肩をすくめる。俺と同じように感じているようだ。
その時、メイ・リンが言った。
「勿論。大成功よ......」
その口元には邪悪とも言える笑みが浮かんでいる。
「これで並行宇宙を元に戻す手段はなくなったんだから......」
「なんだと？」
俺はシートから立ち上がった。メイ・リンが答える。
「確かにあのゴルルゴンは並行宇宙の歪みを象徴する特異点なの。でも宇宙を元に戻す唯一の方法は特異点を消去することじゃない。特異点を捉え、コペルソーン・エンジンを通して逆ユニタリー変換をかけ、波動関数を収束前の状態に拡散させることなのよ......」
メイ・リンは愉悦に満ちた表情で語り続けた。「つまり、特異点が消滅したということは、宇宙を元に戻す方法もなくなったってことになるの」
「君は一体！？」
オタコンの叫びをメイ・リンは鼻で笑った。
「オルガ風に言えば、私はあなた達の知ってる私じゃないってことになるわ。悪く思わないで。ある人物のもとで、ある目的のために動いていただけなの」
「ある人物だと！？」
俺は聞き返した。
「旦那よ」
メイ・リンが甘い声で答えた。
「あなた達の宇宙ではソリダスって呼ばれてるみたいだけど」
「ソリダス！？」
思わず声を上げる俺達に、メイ・リンはゆっくりと壁を指差しながら言った。
「ほら、ウワサをすれば......」
突然壁が吹き飛んだ。降り注ぐ瓦礫。たちこめる粉塵。その中に強化服を着込んだソリダス・スネークが立っている。ソリダスはメイ・リンに言った。
「待たせたな！」
「ハニー！」
メイ・リンがソリダスに抱き付く。
俺はソリダスに向かって駆け出した。ソリダスが蛇手からミサイルを発射する。閃光。爆発。俺は壁に叩き付けられた。一瞬意識が遠のく。何とか起き上がった時、爆炎の向こうにメイ・リンを横抱きにしたソリダスが壁に開けた穴から飛び出していくのが見えた。#C0
*/
E8_0: << EOF
Gurlugon defeated, we turned the VR 
system off. But somehow, I still wasn't 
convinced. I removed the headset and 
spoke to Otacon.
"So. Do you think we really did it?"
Otacon shrugged. Looks like he feels 
the same way I do.
Just then, Mei Ling said:
"Of course the mission has succeeded."
I saw a wicked smile on her face.
"We've just lost the only way to restore 
the parallel universes to normal."
"What did you say?!" I said, standing 
from the seat. Mei Ling responded:
"There's no question that the Gurlugon 
was the singularity that represented the
warp in the parallel universes. But the 
only way to restore space as we know 
it isn't destroying the singularity. The 
solution is to capture the singularity, 
perform a counter-unitary conversion 
via the Koppelthorn Engine, dispersing 
the wave function to its state before 
convergence..."
Mei Ling continued, her voice full of 
satisfaction.
"Anyway, the disappearance of the 
singularity means the disappearance of 
the only method to restore space, too."
"Who are you?!"
Mei Ling sneered at Otacon's cry.
"To borrow Olga's words, I'm not the 
Mei Ling you know. But no hard 
feelings, okay? I was just following 
someone's orders to complete a certain 
objective."
"Someone? Who?" I asked her back.
"My husband." Mei Ling replied sweetly.
"I believe he's called Solidus in your 
universe."
"Solidus?!" we screamed without 
thinking. Mei Ling slowly pointed at a 
wall and said: 
"Oh, speak of the devil..."
Just then, the wall exploded. Debris 
poured from the gap. Dust enveloped 
the area. And Solidus Snake, clad in a 
powered suit, stood in the middle. 
Solidus said to Mei Ling, 
"Kept you waiting, huh?"
"Honey!"
Mei Ling threw herself into Solidus' 
arms.
I dove for Solidus. He fired a missile 
from his snake hand. A dazzling flash. 
An explosion. I was thrown against the 
wall. I lost consciousness momentarily. 
By the time I managed to get to my 
feet, I saw Solidus cradling Mei Ling 
and escaping through the hole he'd just 
blown in the wall. #C0
EOF
//}

//★Ｅ８－１ {
/*
結局逃走したソリダスに追いつくことは出来なかった。俺とオタコンは半ば以上崩壊したアジトで善後策を話し合った。奴等の言うことが本当ならば、並行宇宙の混乱はもはや阻止できなくなったということになる。何か他に方法はないものか......。やがてオタコンが思い付いたように言った。
「オルガはどうかな？」
確かに、『ビッグ・シェル』で会ったオルガは事態をかなりの程度把握しているようだった。彼女なら何か方法を思い付くかもしれない。だが問題が一つある。彼女は今、どこにいるのか。
俺がその点を口にすると、オタコンは自信たっぷりにうなずいた。
「考えがある」#C0
*/
E8_1: << EOF
In the end, I couldn't keep up with 
Solidus. Otacon and I discussed what 
to do next as we stood in our 
half-destroyed hideout. If they were 
telling the truth, we'd just lost the way 
to restore the chaos of the parallel 
universes. But there still had to be 
something we could do... Finally, 
Otacon spoke.
"What about Olga?"
It wasn't a bad idea. The Olga we'd run 
into at Big Shell seemed to have a 
good grasp of the situation. She might 
be able to come up with some kind of 
solution. There's just one problem. 
Where could she be?
I told Otacon my concerns, and he 
nodded with confidence.
"I have an idea."#C0
EOF
//}

//★Ｅ８－２
/*
「で、あなたの特技は？」
オタコンが聞く。目の前にはスレンダーな黒人美女が座っている。よく笑うやたらに朗らかな女だった。
「運が良いことでぇ～す」
「運が良いって？それ特技なんですか？」
「はいぃ～」
質問を続けいくオタコンを、俺は呆れた思いで眺めていた。
オタコンは主要新聞に「ゴルルゴン探検隊募集！」という広告を出した。謎の怪獣を我々の手で捕えよう！当方、怪獣の出現場所について有力情報アリ。経験者優遇。委細面談......。
面接には意外なほど多くの人間がやってきた。俺達は１０人以上と会ったが、当然と言うべきか、その中にオルガの姿はなかった。
「じゃあこちらから連絡しますから」
「よろしくお願いしますぅ～」
立ち上がった女は、にっこり笑って出ていった。俺はオタコンに話し掛けた。
「なあ、お前ホントにこんな方法で......」
「彼女の方もゴルルゴンの情報を探してるはずだろ。きっとやってくるって」
「だが来たのは変人ばかりじゃないか。スライディングとバック転が得意なオヤジとか......」
「ああ、あの人おもしろかったねぇ」
「おい！」
オタコンは明らかに楽しんでいる様子だ。
「まあまあ。もう少し待ってみようよ。はい、次の人！」
オタコンが声をかけた。ドアが開く。入ってきたのはオルガだった。#C0
*/
E8_2: << EOF
"So, what's your skill?" asked Otacon. 
A beautiful black woman with a slender 
build was sitting in front of us. She 
was too cheerful. She laughed far too 
often.
"I've got, like, really good luck!"
"Good luck?! Is that a skill?"
"You betcha!"
Somewhat dismissively, I watched 
Otacon continue questioning the 
applicant. 
Otacon had placed an ad entitled 
"Wanted: Gurlugon Search Team 
Members!" in the city's main 
newspaper the other day. Let's catch 
the mysterious monster with our own 
hands! We have credible information 
about where the creature will appear. 
Experience a plus. Details to be 
discussed during an interview...
Unbelievably, quite a few people had 
shown up. We'd already met with more 
than ten people, but still no sign of 
Olga.
"We'll contact you later."
"I really look forward to hearing from 
ya!"
The woman stood up, gave him a big 
smile, and left. I said to Otacon, 
"Do you really think this is going to 
work?!"
 "She has to be on the lookout for 
information about Gurlugon herself. 
She'll show up."
"But all we've gotten so far are 
weirdos! Like that old man who's skill 
was back flips and sliding, and..."
"Yeah, he was funny!"
"Hey!"
Obviously, Otacon was getting a kick 
out of the whole scene.
"Relax. Let's wait a little bit longer. 
Next!" yelled Otacon. The door 
opened. Lo and behold, in walked Olga.#C0
EOF
//}

//★Ｅ８－３ {
/*
俺達は半壊したアジトでオルガに事情を説明した。ことの顛末を聞いたオルガは、まだ手はある、と言った。
「確かにゴルルゴンの特異点は消されてしまったようだけど、並行宇宙に存在する全ての特異点がなくなったとは限らないわ」
オタコンが目を輝かせた。
「他にも特異点はあるってこと？」
「おそらくね。でもどこの並行宇宙にどういう形を取って存在しているのかはわからない。探すのは簡単じゃないと思う」
「僕等にはゴルルゴンのデータがある。そのパターンを解析してストレンジ・アトラクタ近辺をＶＲで走査すれば......」
オタコンとオルガは技術的な検討を重ねた。その結果、ＶＲシステムの波動関数パラメータを特異点の固有パターンに近づくよう微調整しながら連続稼動させることで、他の並行宇宙にある特異点を探し当てるという方法に落ち着いたとオタコンは言った。......要するに俺がＶＲを連続してクリアしていけばいいということらしい。
準備は意外なほど早く済んだ。
再びＶＲシステムのシートに座った俺にオタコンが言う。
「特異点が観測できたらすぐに教える。それまでＶＲをクリアし続けてくれ。でも気を付けるんだ。ソリダスが黙っているとも思えない。何か罠を張っている危険性もある。充分に注意してくれよ！」
オタコンがＶＲシステムを動かした。目の前に仮想空間が広がっていく。#C0
*/
E8_3: << EOF
Back in the remains of our hideout, 
we explained the situation to Olga. 
After hearing our full account, Olga 
told us that there was still a way out. 
"It's true that the Gurlugon singularity 
might have been erased, but that 
doesn't necessarily mean that all of 
singularities in all of the parallel 
universes have been."
A glimmer of hope flashed through 
Otacon's eyes.
"You mean there are other 
singularities?"
"Most likely. But their form and location 
in the parallel universes are unknown. 
They won't be easy to find." 
"We have some data about Gurlugon. 
If we analyze its pattern, scan the 
vicinity of the strange attractor with VR,
maybe we can..."
Otacon and Olga continued their 
technical discussion. The end result, 
said Otacon, was settling on a method 
to find the singularities of other parallel 
universes by continuously and finely 
adjusting the wave function parameter 
of the VR system to mimic the unique 
pattern generated by a singularity. 
In other words, it was looking like I'd 
have to make it through a series of VR 
constructs. Preparations were made 
faster than I expected.
I returned to the seat of the VR system.
Otacon said, "I'll tell you as soon I 
locate a singularity. Until then, I want 
you to continue clearing VR stages. 
But be careful. I don't think Solidus will 
just take this sitting down. You may 
run into traps. Stay alert!"
Otacon initialized the VR system. 
A virtual reality spread before my eyes.#C0
EOF
//}

//ＶＲクリア後
//★Ｅ９ {
/*
「どうだ、特異点は観測できたか？」
俺はオタコンに聞いた。
「いや、まだだ」
ＶＲ空間にオタコンの声が聞こえてくる。
「別の状況を試してみよう。準備する。少し待っててくれ」
俺はＶＲ空間に取り残された。
「スネークよ......ＶＲを使って特異点を検索するとは考えたものだな......だが貴様が特異点に辿り着くことはない」
突然どこかから声が聞こえてきた。ソリダスだ。俺はオタコンを呼び出した。
「オタコン、ソリダスが......」
「わかってる。こっちでも計測した。奴はＶＲシステムに介入してきてる。でもまだシステムの制御を支配されてるわけじゃない。急いで次のＶＲを始めよう。準備はいいかい？」
目の前に新しいＶＲ空間が開けた。#C0
*/
E9: << EOF
"Located any singularities?" I asked 
Otacon.
"No, not yet." echoed Otacon's voice 
through the VR construct.
"Let me try a different setting. I'm 
getting ready. Hold on a minute."
I was left alone in the virtual reality.
"You must have figured out how to find 
singularities using VR, Snake. But you 
won't find one." boomed a voice 
suddenly from out of nowhere. It was 
Solidus.
I called Otacon.
"Otacon, it's Solidus."
"I know. I detected him from my end, 
too. He's hacking into the VR system, 
but he hasn't gotten full control yet. 
Let's hurry into the next VR construct. 
Are you ready?"
A new VR space spread in front of my 
eyes.#C0
EOF
//}


//★Ｅ１０－０ {
/*
「どうだ、オタコン？」
俺は虚空に尋ねた。オタコンの声が答える。
*/
E10_0: << EOF
"How are you doing, Otacon?" 
I asked empty air. Otacon's voice 
answered.#C0
EOF
//}

//★Ｅ１０－１Ａ {
/*
「それらしいパターンは感知した。でもまだ遠い。状況変数を変えてみてくれ。今よりももう少し高いスコアを取るんだ。もう一度いくよ！」
俺は身構えた。今度はより高いスコアを取らなければならないということらしい。もう一度ＶＲが始まった。#C0
*/
E10_1A: << EOF
"I detected a likely pattern. But it's too 
far away. Change the settings of your 
variables and aim for a higher score. 
Let's try again!"
I squared off. Looks like I have to get 
a higher score this time. The VR 
simulation began again.#C0
EOF
//}

//★Ｅ１０－１Ｂ {
/*
「だいぶ近づいてきたよ！すぐに次の状況を用意する」
俺がうなずいた時、メイ・リンの声が聞こえてきた。
「奴等......思ったよりやるじゃない？」
ソリダスの声が答える。
「ああ。認識を改めねばならんな。奴等が特異点に近づけんよう手を打つ必要がある」
「『奴等』を使ったらどうかしら？」
「それは危険すぎる。例えスネークを止めることが出来たとしても、世界そのものが破壊されてしまっては意味が無い」
「確かにそうね......」
「『奴等』を使うのは最終手段だ」
「まかせるわ、ハニー」
二人の声は先ほどよりもはっきりと聞こえた。間違いなく奴等は近づいてきている。
俺はオタコンに叫んだ。
「オタコン、ソリダス達が！」
「わかってる。介入がかなり強くなってきてる。奴等に完全に捕捉される前に特異点を見つけるんだ。次いくよ！」#C0
*/
E10_1B: << EOF
"We're getting a lot closer! I'll get 
ready for the next setting."
As soon as I nodded, I heard Mei Ling's
voice. "They're doing a lot better than 
we expected, aren't they?"
"Indeed. We'd better change our 
strategy. We're going to have to 
somehow block the singularity so they 
can't get any closer." answered 
Solidus' disembodied voice.
"How about using 'them'?"
"It's too dangerous. Even if they did 
stop Snake, it'd be pointless if the 
world was destroyed in the process."
"True enough."
"Using 'them' has to be our last resort."
"Everything is in your hands, honey."
Their voices were a lot clearer than 
before. They're definitely getting closer.
I yelled to Otacon: "Otacon, it's 
Solidus!"
"I know. Their intrusion is progressing. 
We have to find the singularity before 
they get full control. Let's go!"#C0
EOF
//}

//ＶＲクリア後
//★Ｅ１１ {
/*
俺はＶＲ空間の中からオタコンに呼びかけた。
「オタコン、特異点は観測できたか？」
「それらしい反応を見つけたよ！次のゴールに飛び込んでくれ！」
「ハニー！スネークが......」
メイ・リンの舌打ちが聞こえた。まるで耳元でささやかれているような近さから。ソリダスが答える。
「わかっている。もう『奴等』を使うしかないだろう。準備をしてくれ」
「わかったわ......」
オタコンの上ずった声が聞こえてきた。
「スネーク、聞こえるか！ソリダス達がＶＲを支配し始めた！もう時間がない。すぐに始めるよ！」#C0
*/
E11: << EOF
"Otacon, have you sighted a 
singularity?" I asked from inside the 
VR.
"I'm picking up a reaction! Hurry to 
the next goal!"
"Honey! Snake is..."
I heard Mei Ling click her tongue. 
I heard it as if she was whispering to 
me. Solidus answered,
"I know. It may be time to use 'them.' 
Get ready." 
"Roger that."
I heard Otacon's excited voice. 
"Come in, Snake! Solidus has taken 
control of the VR system! We don't 
have much time. Let's begin!"#C0
EOF
//}


//ＶＲクリア後
//★Ｅ１２－０ {
/*
そこには何も無かった。光も暗黒も音も温度も上下の感覚も無い。ここが特異点なのだろうか？俺はオタコンを呼んだ。だが答えたのはオタコンではなく、ソリダスの声だった。
「そこは......まあ貴様に理解しやすいように言うならば、並行宇宙の狭間、と言ったところだ」
俺は叫んだ。
「ソリダス！？貴様の狙いは何だ！？なぜ宇宙を破壊しようとする！？」
「破壊？馬鹿を言うな。訂正だよ。あるべき姿へのな......」
「どういうことだ！？」
「コペルソーン・エンジンを、いつ、どこで、誰が、何のために作ったかはわからない。だがそれはおそらく並行宇宙間をまたがるグリッド・コンピューティングシステムの一部として設計されたのではないかと私は推測している」
オタコンから聞いたことがある。グリッド・コンピューティング。多数のサーバを接続することで、莫大な演算能力を持つ共有型のスーパーコンピュータを作り出す技術だ。それを文字どおり無数に存在する並行宇宙の間で行なうことが出来たならば、無限に近い演算能力が得られるのかもしれない。
ソリダスが続ける。
「だが並行宇宙にまたがるコペルソーンエンジンがＶＲシステムに取り入れられた時、それは可能性を支配する装置となった」
「支配だと？」
「そうだ。コペルソーン・エンジンを用いたＶＲシステムは、ある条件を持った仮想空間を構築するよう命令されると、いまだ波動関数が収束せず状態が確定していない並行宇宙の中から、その条件を満たす可能性のある宇宙群を抜き出し、貴様の前にＶＲ空間として差し出す。貴様らがＶＲ訓練と呼んでいたものは、それらの並行宇宙群に対し外部から干渉・観測を行なう行為に他ならない」
俺はＶＲ訓練を行なっている時の、まるで他人の体に乗り移り動かしているような奇妙な感覚を思い出した。あれが他の宇宙に干渉する感覚だったというのか。
ソリダスの声が響く。
「貴様がＶＲ訓練を行なうと、ＶＲ内で貴様が取った行動が、それに応じた幾つかの並行宇宙に反映される。そしてその干渉は同時に、干渉を受けた宇宙以外のいまだ確定していない並行宇宙群に対する観測行為ともなるのだ。それはつまり貴様が取った行動と同じ状態へ、宇宙の可能性が収束することを意味する。例えば貴様がＶＲ内で、ある敵兵を殺したとしよう。するとその敵兵が殺された宇宙が観測され確定する。言い換えればそれ以外の可能性を持っていた宇宙、つまりその敵兵が殺されることのない並行宇宙群は未確定の可能性を失い、ありえなかったものとして、その存在自体を抹消されてしまうのだ」
ソリダスは語り続ける。
「最終的に生き残るのを許される宇宙は、貴様のＶＲ内での行動と矛盾しない宇宙だけだ。これがどういうことかわかるか？もしＶＲの中で貴様がゲームオーバーになり、リトライを行なったならば、その宇宙は存在の可能性自体を否定され、無かったものとして抹殺されるということだ。つまり貴様がリトライを選択する度に無数の宇宙が虐殺されてきたということなのだよ！」
俺は愕然とした。
「貴様が見てきた並行宇宙の混乱は外部から干渉されることによって生じる歪みだ。殺されゆく宇宙が上げる断末魔の叫びと言っていいだろう。特異点はそれが凝集したものと言える」
オルガやゴルルゴン、メイ・リンの姿が俺の脳裏をよぎった。それらの混乱が外部からの干渉によって引き起こされるとするならば、それはつまり......
「それはつまり、俺の宇宙も別の並行宇宙にあるＶＲシステムからの干渉を受けていたということか？」
「勿論だ。貴様自身、自分が自分でないように感じたり、後から考えると不可解な行動をとってしまったことはなかったか？あるいは、出来るはずがないと思っていたことができてしまったことは？それらが他宇宙からの干渉によるものだ。貴様がシャドーモセスを切り抜けられたのも、他の並行宇宙の誰かのお陰かも知れんな」
ソリダスの嘲笑がこだまする。
「私は並行宇宙を研究するうち、多くの宇宙で私が殺されていることを知った。私は私の同胞達を守ろうと決意した。私が殺される宇宙など私は絶対に認めん！......だから私はＶＲシステムでその間違いを正すことにした。ＶＲシステムを使って他の並行宇宙へ干渉し、その宇宙の私が死なないよう操作することにな......そのために多少の混乱が起きようと、それと矛盾する並行宇宙が消え去ろうと知ったことではない。重要なのは私が生き残ること、それだけだ」
*/
E12_0: << EOF
There was nothing there. There was no 
light, no sound, no feeling of 
temperature or even of height. Was 
this the singularity? I called Otacon. 
But it was Solidus who answered.
"You're in what's called - how to put 
this so you can understand it? - a gap 
between parallel universes."
"Solidus?! What are you trying to 
do?! Why are you trying to destroy 
the universe?!" I screamed.
"Destroy? Don't be stupid. This is a 
correction. Back to how things should 
be..."
"What do you mean?!"
"I don't know who created the 
Koppelthorn Engine, or where or when, 
or even why. But my guess is that it 
was designed as part of a grid 
computing system spread between the 
parallel universes."
Grid computing. I'd heard the term from
Otacon. It was a technique for creating 
a networked supercomputer with 
massive computational abilities by 
connecting numerous servers. If one 
could be built spanning across 
countless parallel universes, it would 
be possible to obtain near-limitless 
computing power.
"But when the universe-spanning 
Koppelthorn Engine was applied to the 
VR system, it became a device for 
controlling possibility itself." Solidus 
continued.
"Controlling?"
"That's correct. When a VR system 
using the Koppelthorn Engine is 
instructed to build a virtual reality 
having a given set of conditions, it 
selects a group of universes having the 
potential to satisfy those conditions 
from among a set of parallel universes 
that are still indefinite and that still 
have wave functions that have not 
converged. It then presents these as 
the VR construct you see before you. 
What you call VR training is nothing 
less than the external observation of 
and interference with that group of 
parallel universes!"
I recalled the bizarre sensation of 
possessing another person's body 
when I was engaged in VR training. 
I suppose that was the sensation of 
interfering with another universe. 
Solidus' voice echoed:
"When you go through VR training, 
your actions within virtual reality are 
reflected in a set of corresponding 
universes. The resulting interference 
simultaneously acts as observation with
regards to a group of still-indefinite 
parallel universes besides the one that 
is receiving the interference. That 
means that the possibilities of the 
universe converge on the same 
conditions corresponding to the actions 
you took. For example, let's say you 
just killed an enemy soldier within the 
VR simulation. That means that the 
universe containing the killed soldier 
was observed and made definite. 
In other words, universes having 
possibilities other than that, in short 
parallel universes where that soldier 
had not been killed, lose their indefinite 
potential and are erased from 
existence."
Solidus continued.
 "The only universe permitted to exist 
in the end is the one that doesn't 
contradict the actions you took in 
the VR simulation. Do you realize what 
this means? When your game is over 
in the VR system and you try again, 
the potential for existence for that very 
universe is denied and eliminated as 
never having existed. That means that 
every single time you replay the game, 
an uncounted number of universes are 
destroyed!"
I was in total shock.
"The disturbance of the parallel 
universes you've seen is distortion 
generated by external interference. 
I suppose you could see it as the death 
cries of the universes that have been 
killed off. The singularities are a 
condensed form of that distortion."
I had a flashback of Olga, the 
Gurlugon, and Mei Ling. If this 
disturbance had really been caused by 
external interference, that means...
"That means my own universe is 
receiving interference from VR systems 
in other universes?"
"Of course. Haven't you ever felt like 
your body wasn't your own, like you 
took an unexpected action after the 
fact? Or you were able to do 
something you didn't think was 
possible? That's because of 
interference from other universes. 
Perhaps the reason you were able to 
topple Shadow Moses is because of a 
helping hand from someone in another 
universe." Solidus' laugh echoed 
through the space.
"Through my research into parallel 
universes, I learned that I was killed 
many times in different worlds. 
I decided to protect my selves. I refuse 
to recognize any universe where I am 
killed! And that's why I decided to 
make things right with the VR system. 
I interfere with other parallel universes 
using the VR system and control them 
such that I won't die there. I don't care 
how much of a disturbance I create or 
how many contradictory universes are 
eliminated. The only thing that's 
important is my survival. Period."#C0
EOF
//}

//コンティニューを一回でもした場合
//★１２－１Ａ {
/*
「馬鹿な！」
「ほほう、貴様にそれが非難できるのか？貴様もＶＲ内で自分の失敗を認めずリトライを繰り返してきたではないか？」
*/
E12_1A: << EOF
"That's crazy!"
"Do you really have the right to criticize
me? Haven't you refused to recognize 
your own failures within the VR 
simulation by retrying a level again?"#C0
EOF
//}

//★１２－２ {
/*
返す言葉のない俺を、ソリダスが嘲笑う。
「話はここまでだ。貴様に特異点は渡さない。私が訂正してきた宇宙を元に戻させる訳にはいかん。貴様にはここにとどまってもらう。永久にな！」
ソリダスは傲然と言い放った。
「私に封印を解かせたことを後悔しろ。もうすぐここに『奴等』がやってくる......幾多の宇宙で悪名を轟かせ、蛇蝎のごとく忌み嫌われている史上最凶のユニットだ。その存在のみで世界を破滅させると恐れられ、全宇宙の憎悪を一身に集めてきた暗黒と邪悪の申し子達。地獄のプリンセスと混沌の貴公子......。紹介しよう。ローズとジャックだ！！」
閃光が全てを包んだ。#C0
*/
E12_2: << EOF
Solidus laughed at my lack of a 
comeback.
"This conversation is over. I won't give 
up the singularity. And I can't let you 
restore the universes that I have 
corrected. So I'd like you to stay 
here... FOREVER!" He spat.
"You're going to regret forcing me to 
break the seal. 'They' will be here 
soon... A loathsome unit, infamous in 
uncountable universes, hated and 
feared by all. Their presence alone has 
the ability to destroy a world. They are 
children of darkness on whom is 
focused the combined hate of the 
entire universe. The princess of hell 
and the prince of chaos. Allow me to 
introduce...Rose and Jack!!" 
A dazzling light enveloped everything.#C0
EOF
//}


//雷電テイルズ１話
//★Ｅ１３－１ {
/*
「ジャック！ねぇジャック！！」
ローズがテーブルに身を乗り出した。俺の目を覗き込んで首をかしげる。
「どうしたの？ぼーっとしちゃって」
俺はローズの髪を撫ぜた。
「ああ......君があまりにキレイだからつい見とれてしまったんだ」
「もうジャックったら、冗談ばっかり」
ローズは少女のようにはにかんだ。
「冗談なんかじゃないさ」
ローズが俺の手に触れる。
「ねぇ、明日何の日だか覚えてる？」
「３月２１日が？」
「ええ」
俺は答える。
「勿論さ。俺の人生で最高の幸運が訪れた日だ。３月２１日......俺は君と初めて出会った」
「違うわよ！」
「......そうだったか？」
「そうよ」
ローズがぷーっと頬をふくらませる。
「すまない。君との出会いを毎日感謝してるから、つい明日もその日だと思ってしまったんだ」
「まあ、意味はよくわからないけど、なんだかとってもうれしいわ」
ローズが微笑んだ。
「ところで、今回のあなたの任務は爆弾を処理することよ。しっかりね」
*/
E13_1: << EOF
"Jack! Earth to Jack!" Rose leaned 
over the table. She cocked her head 
and looked into my eyes.
"What's wrong? You looked out of it."
I gently stroked Rose's hair.
"Uh... I was enchanted by your eternal 
beauty."
"Very funny, Jack."
Rose became as bashful as a little girl.
"I'm being serious."
Rose touched my hand.
"Do you remember what day tomorrow 
is?"
"What, March 21st?"
"Yeah."
"Of course. It's the luckiest day of my 
entire life. The day I met you." I said.
"Wrong!"
"...Really?"
"Yep."
Rose puffed out her cheeks.
"I'm sorry. I think of how lucky I am to 
have met you every day, so I always 
think of the anniversary as being right 
around the corner."
"That doesn't exactly make sense, but 
I'm happy to hear it anyway," smiled 
Rose. "By the way, your mission this 
time is to defuse all bombs. Good luck."#C0
EOF
//}


//雷電テイルズ２話
//★Ｅ１３－２ {
/*
俺は任務で戦場のど真ん中にいた。砲声が轟き、銃火が闇を切り裂く。戦況は熾烈を極めていた。敵も味方も全員が傷つき、いたるところに屍をさらしている。まさに地獄だ。銃弾が体をかすめるのを感じて身をすくめたその時、無線連絡が入った。ローズからだ。
「ジャック......したいの？任務？」
「今してるところだ！」
「ああ、そうだったわね」
「何の用だ？」
ローズはしばし黙り込んだ後、戸惑いながら切り出した。
「......ジャック、話があるの」
「話？」
「そう、重要な話。それも明日、話したい」
なぜ明日話したいことを今日言い出すのだろうか。さっぱり分からないので俺は聞いてみた。
「今じゃダメなのか？」
ローズはあっさり言った。
「そうね。じゃあ今話すわ。ジャック、あのね、私......私、お腹に......お腹に......」
重い沈黙が流れる。やがてローズは意を決したように口を開いた。
「お腹に脂肪がついちゃって。どうしたらいい？」
「どうって......別に俺は気づかなかったが......」
「そう？でも昔買ったスカートとか微妙にキツイのよね......」
「そんなに気になるんだったら、エクササイズでも始めたらどうだ？何なら俺も付き合うよ」
「うれしい！じゃあとりあえずジョギングでも始めましょ。あ、それと、今回のあなたの任務だけど、敵兵を武装解除することだから。がんばってね」
*/
E13_2: << EOF
I'm smack in the middle of the 
battleground on a mission. The roar of 
cannons and gunfire punctuates the 
darkness. Conditions are unbelievably 
harsh this time around. No one on 
either side is without injury, and the 
bodies of the dead lie sprawled 
everywhere. It's hell on Earth. 
I instinctively duck, feeling a bullet cut 
through the air inches away. Just then, 
the Codec rings. It's Rose. 
"Do you want to... Jack? What 
happened to the mission?"
"I'm in the middle of it!"
"Oh, that's right."
"What is it?"
Rose hesitated for a moment before 
she spoke.
"...Jack, I need to talk."
"Talk?"
"Yes, it's very important. And we'll talk 
about it,tomorrow."
Why is she telling me now about 
something she wants to discuss 
tomorrow? Confused, I put the 
question to her.
"Why not now?"
Rose eagerly said:
"All right, I'll tell you. Jack... I'm... 
...I'm carrying..."
A heavy silence filled the air. Finally, 
Rose seemed to make up her mind.
"...I think I'm gaining weight! I'm 
getting love handles. What should I 
do?"
"What should you...? I didn't notice at 
all!"
"Really? But my old skirt is getting 
kind of tight on me..."
"If it really bothers you that much, why 
don't you start working out? I'll even 
join you if you like."
"That's great! How about jogging? 
Oh, before I forget, your mission this 
time is to hold up all enemies and get 
to the goal. Good luck."#C0
EOF
//}

//雷電テイルズ３話
//★Ｅ１３－３ {
/*
俺はリバーサイドのレストランでローズとディナーを取っていた。ローズは今晩もキレイだ。抑えられた照明の下、少量のワインにほんのり上気した頬が可愛すぎる。テーブルにメインディッシュが並べられた時、突然大佐が駆け寄ってきた。
「雷電、聞いてくれ......先週の木曜のことだ......」
「どうしたんだ、大佐。また宇宙人に誘拐でもされたのか？」
大佐は半ばムキになった。
「宇宙人だと！？何を言ってるんだ、そんなものいるわけないだろう！全く度し難いな......」
「じゃあなんなんだ？」
大佐は落ち着きを取り戻そうと努力しながら語った。
「うむ。聞いてくれ。実は私の家の裏は麦畑になっているんだが、木曜の夜、畑の方に何か光るものを見た気がしたんだ。いくつも、いくつもな......。で、朝になって畑に行ってみると畑中の麦が倒されて......巨大な円形の模様が出来ていたんだ。とても複雑な......。どう思う？」
「どうって......」
言葉に詰まる俺に代わってローズが言った。
「それはやっぱり......何かのメッセージじゃないですか？」
大佐はうろたえながら言った。
「......や、やはりそうか......ハサミが必要かも知れんな......。ま、まあとにかく今回の君の任務はターゲットを全て破壊することだ。油断するなよ」
*/
E13_3: << EOF
Rose and I are having dinner at a 
riverside restaurant. Rose is beautiful 
tonight, as usual. A bit of wine had 
made her already cute cheeks all rosy. 
Just as the main dish was being 
served, the Colonel ran up to the table.
"Raiden, something happened to me 
last Thursday..."
"Colonel! What's wrong? Were you 
abducted by aliens again?" 
The Colonel became upset.
"Aliens?! What the hell -- There's no 
such thing as aliens! I highly 
disapprove."
"What is it, then?"
Making a visible effort to calm himself, 
the Colonel began his story.
"Listen carefully. There's a barley field 
behind my house. Thursday night, 
I saw something, many things, flashing 
in there. When I went into the field the 
next morning, I discovered that the 
stalks of barley had been flattened into 
huge circular patterns. They were very 
complex... What do you think?"
"Huh?"
I was at a loss for words, but Rose 
answered on my behalf.
"You know, I think it's some kind of a 
message."
The Colonel said in a shaken voice:
"I...I thought so. I guess I'll need some 
scissors, then... Anyway, your mission 
this time is to destroy all targets and 
head to the goal. Stay focused."#C0
EOF
//}



//雷電テイルズ４話
//★Ｅ１３－４ {
/*
俺とローズは昼下がりの公園を散歩していた。木漏れ日がキラキラと降り注ぐ。俺達は池のほとりのベンチにならんで腰を下ろした。俺がローズの肩に手を回した時、池の中から奇声と共に人間が飛び出した。ヴァンプだ。奴はナイフを抜きざま投げつけてきた。俺はローズをかばいながら銃を抜き、ナイフを撃ち落とす。続けて連射。銃弾はあやまたずヴァンプの眉間を貫き、奴を地面に打ち倒した。だが奴はかすかなうめき声を上げただけですぐに起き上がった。ローズが震える声でつぶやく。
「信じられない......どうして......？」
ヴァンプは長い舌で流れ落ちてくる自らの血をぺろりとなめると、言った。
「俺は一度死んだ男。死にはしない」
ローズがうなずいた。
「じゃあ、しょうがないわね」
ヴァンプはまた奇声を上げると池に向かって飛んだ。着水するとそのまま水面を走りながら俺に叫ぶ。
「今回の貴様の任務は敵兵をすべて倒してゴールへ向かうことだ。さっさと行け！」
*/
E13_4: << EOF
Rose and I were taking a walk in the 
park early one afternoon. The sunlight 
filtered down through the treetops. 
We walked to a pond and sat down on 
a bench. As I put my arm around Rose,
somebody jumped out of the pond with 
an odd squeal. It was Vamp! Pulling 
out a knife, he threw it directly at us. 
Protecting Rose with my body, I drew 
my gun and shot down the knife. 
I continued firing. A bullet struck Vamp 
between the eyes, knocking him to the 
ground. But groaning slightly, he stood 
back up again. I heard Rose's trembling
voice.
"I don't believe it... How...?"
Licking a trickle of his blood with his 
long tongue, Vamp replied:
"I died once already, I can't die twice."
Rose nodded.
"Well, I guess it can't be helped, then." 
Squealing again, Vamp lunged toward 
the pond. Landing directly on the 
pond's surface, he ran across while 
yelling: 
"Your mission this time is to take out 
all enemies and get to the goal! 
Go, NOW!"#C0
EOF
//}



//雷電テイルズ５話
//★Ｅ１３－５ {
/*
俺がローズと彼女の部屋で怪獣映画のビデオを見ていると、いきなり画面がノイズになってしまった。
「故障か？」
俺がベッドから起き上がろうとした時、唐突にノイズのままのテレビ画面がしゃべりだした。
「雷電、聞こえるか？我々だ......」
「らりるれろ！？」
ローズが驚きの声を上げる。らりるれろが言う。
「雷電、君は我々の正体が知りたいのだろう？」
思わず飛び起きた俺に、らりるれろが含み笑いのような音を出す。
「ふふふ......君も知っての通り、我々は正確には......人ではない。だが君は知っているか？便器には......色々な汚れが付着する。尿石や黄ばみ、水垢......。こまめな清掃が重要だ......違うかね？」
「何を言ってる？」
その時、電源を切ってあるはずのラジオから声が流れ出した。
「雷電、聞こえるか？我々だ......」
「たちつてと！ジャック、たちつてとよ！！」
ローズが悲鳴を上げる。たちつてとが言う。
「ふふふ......黄ばみに対してはこうするといい。まず便器の汚れた部分にティッシュ・ペーパーをひくのだ......。そしてその上からトイレ用洗剤をかけろ......。それから数分待ちブラシでこするのだ。......ペーパーごとな......」
「何の話だ！？」
俺が叫んだ時、突然留守番電話がしゃべりだした。
「雷電、聞こえるか？我々だ......」
「いろはにほへと！？いろはにほへとまで！ジャック、どうしよう！？」
ローズが俺の腕にすがりつく。俺は震えるローズを抱きしめた。いろはにほへとが言う。
「便器の内側周囲、いわゆるリムにも気を配れよ......。飛び跳ねた尿が付着しがちだからな......」
らりるれろが言う。
「ブラシでこすりにくいところはゴム手袋を付けてスポンジや布でこすればいい」
たちつてとが言う。
「使用済みの歯ブラシを使うのもいいだろうな......」
いろはにほへとが言う。
「便器の外側や床も忘れるなよ......尿が飛びちっていると匂いが残ってしまう......」
俺は叫んだ。
「それがお前等の正体と何の関係があるんだ！？」
「何の関係も無い！！！」
らりるれろの怒声が轟いた。俺はのけぞった。
「ええと、じゃあ結局......何が言いたいんだ？」
三者が同時に言った。
「ふふふ......我々が言いたいのはこれだけだ」
らりるれろが言う。
「今回の」
たちつてとが言う。
「君の」
いろはにほへとが言う。
「任務は」
ローズが言う。
「見つからずにゴールを目指すことよ。がんばってね！」
*/
E13_5: << EOF
Rose and I were watching a monster 
movie in her bedroom. Suddenly, the 
screen filled with static.
"Is it broken?"
When I got up from the bed, the 
static-filled television screen began to 
speak.
"Raiden, are you receiving? We're still 
here."
"The La-li-lu-le-lo?!"
Said Rose in shock. The La-li-lu-le-lo 
continued:
"Raiden. You want to know who we 
are, don't you?" 
I jumped from the bed in surprise. 
The La-li-lu-le-lo let out something 
approximating a laugh.
"Heh, heh, heh. As you already know, 
we're not what you'd call -- human. 
But you know what? Toilets get all 
kinds of stains: dried urine, yellow 
stains, water stains... They need 
constant cleaning. Don't you agree?"
"What are you talking about?!"
Just then, I heard another voice from 
the radio, in spite of it being turned off.
"Raiden, are you receiving? We're still 
here."
"Ta-ti-tu-te-to! Jack! It's the 
Ta-ti-tu-te-to!" shrieked Rose.
The Ta-ti-tu-te-to said,
"Heh, heh, heh. We have some advice 
for you about those yellow stains. 
First, lay some toilet paper over the 
stain. Then sprinkle on the detergent. 
Let it sit for a few minutes, and then 
scrub hard...with the paper." 
"What the--?!"
As soon as the words left my mouth, 
the answering machine began to speak.
"Raiden, are you receiving? We're still 
here."
"I-ro-ha-ni-ho-he-to?! Oh, no, Jack! 
What should we do?!"
Rose held on to my arm. I held her as 
she trembled. The I-ro-ha-ni-ho-he-to 
said,
"Make sure to pay attention to the rim, 
inside the toilet. The urine sprays onto 
it."
The La-li-lu-le-lo jumped in.
"Wear rubber gloves, and use a sponge
or a piece of cloth to rub dirty spots 
you can't reach with a brush."
The Ta-ti-tu-te-to continued.
"It wouldn't be a bad idea to use an old
toothbrush."
The I-ro-ha-ni-ho-he-to spoke up.
"Don't forget to wipe the outside of the 
toilet or the floor, either. If you leave 
any urine stains behind, it'll smell later."
I yelled a question.
"What in the hell does this have to do 
with who you are?!"
"NOTHING!!!" La-li-lu-le-lo yelled 
back. I was floored.
"Uh, so... What's your point, then?"
The three of them answered at the 
same time.
 "Heh, heh, heh... Our point is very 
simple."
The La-li-lu-le-lo said:
"This time..."
Ta-ti-tu-te-to said:
"...Your..."
The I-ro-ha-ni-ho-he-to said:
"...mission is..."
Rose said:
"...to get to the goal without being 
discovered. Good luck!"#C0
EOF
//}




//ヒント用
//★Ｅ１３－Ｈ０ {
/*
視界が暗転する。その時、どこか遠くから呼ばれたような気がした。なぜかそれはとても懐かしい声に思えた。
*/
E13_H0: << EOF
My vision darkened. Just then, 
I thought I heard someone calling me 
from far away. For some reason, the 
voice seemed very familiar.#C0
EOF
//}

//★Ｅ１３－Ｈ１
/*
「スネーク、それはソリダスの罠だ！目を覚ましてくれ！」#C0
*/
E13_H1: << EOF
"Snake, that's Solidus' trap! 
Wake up!"#C0
EOF
//}

//★Ｅ１３－Ｈ２ {
/*「スネーク、ソリダスの罠から抜けるんだ！」
*/
E13_H2: << EOF
"Snake! Get out of Solidus' trap!"#C0
EOF
//}

//★Ｅ１３－Ｈ３ {
/*
「スネーク、君はソリダスの罠に捕らわれてる！ＶＲから脱出するんだ！」#C0
*/
E13_H3: << EOF
"Snake! You're caught in Solidus' 
trap! Get out of VR mode!"#C0
EOF
//}

//★Ｅ１３－Ｈ４ {
/*
「スネーク、ＶＲから抜け出してくれ！ＶＲを終了しろ！脱出するんだ！」#C0
*/
E13_H4: << EOF
"Snake! Get out of VR! Turn it off! 
Exit!"#C0
EOF
//}

//★Ｅ１３－Ｈ５ {
/*
「スネーク、大丈夫か！？ソリダスからＶＲの制御を奪い返したよ！今、解放する！」#C0
*/
E13_H5: << EOF
"Snake, are you okay!? I took back 
control of the VR from Solidus! I'm 
going to release now!"#C0
EOF
//}

//ループ脱出後
//★Ｅ１４ {
/*
視界がまばゆい光に包まれた。次の瞬間、俺は床に奇妙な光が走る広い空間に投げ出されていた。オタコンの声が聞こえてくる。
「......スネーク......スネーク！大丈夫かい！？」
軽い頭痛に俺は頭を振った。何かとても長い夢を見ていたような気がする。オタコンの声が弾んでいる。
「ソリダスからＶＲの制御を完全に奪い返したよ！」
「そうか......で、特異点は......」
「見つけた。そこだ。そこが特異点だ。今から宇宙を元に戻すための作業を始めるから......」
その時、俺は前方から迫る巨大な何かの気配を感じた。暗闇からメタルギアＲＡＹが現れる。一体、二体......次々と。オタコンが叫んだ。
「ソリダスの最後の妨害だ！こっちの作業が終了するまでもう少し時間が掛かる。何とか持ちこたえてくれ！」
迫り来るメタルギアＲＡＹの軍勢。俺は身構えた。オタコンが作業を終えるまで戦い抜くしかない。#C0
*/
E14: << EOF
I was enveloped in an overpoweringly 
bright light. The next moment, I was 
thrown into a huge space with a 
mysteriously lit floor surface. I could 
hear Otacon's voice.
"Snake! Snake!! Are you okay?!"
I shook my head to clear a mild 
headache. It feels like I've been 
dreaming for a long time. 
"I've completely retaken control of the 
VR system from Solidus!" Said Otacon
excitedly.
"That's good. So where's the 
singularity...?"
"I've found it! It's there! You're in the 
singularity! Now we can start returning 
the universe to normal!" 
Just then, I felt something huge closing 
in from the front. A Metal Gear RAY 
emerged from the darkness. Then a 
second one, then more, right after the 
other. Otacon screamed:
"It's Solidus' final attempt to stop us! 
I need a little more time to finish things 
on my end. Just hold on for a little bit!"
The Metal Gear RAY units closed in. 
I squared off against them. I'd have to 
fight until Otacon finished his work.#C0
EOF
//}



//ＲＡＹ戦後
//★Ｅ１５ {
/*
気がつくと俺は再びソリダスの言う「並行宇宙の狭間」に放り出されていた。どこかからオタコンの喜ぶ声が聞こえてくる。
「スネーク、成功だ！こちらの作業は全てうまくいったよ。宇宙は修復されはじめた！」
俺は虚空に問い掛けた。
「全て元どおりに？」
「そのはずだ。君ももうすぐこちらの宇宙に引き寄せられるはずだ。だけど宇宙の修復は順番に行なわれていくようなんだ。全てが完全に直るには少し時間がかかるかもしれないけど......」
「スネーク！！」
突然ソリダスの怒声が響いた。
#W
閃光が走り、目を開けると俺は破壊されたフェデラルホールの上に立っていた。ソリダスと共に。
ソリダスが刀を抜く。俺は言った。
「やめろ、もう勝負はついた。宇宙は修復されはじめている」
「いいや。まだ勝負はついていない。ここで貴様を倒せば、私が生き残る宇宙が少なくとも一つは確定する」
「やめろ！他にも方法はあるはずだ！俺は俺の宇宙に帰る。お前の宇宙に干渉するつもりはない。俺達が闘う理由はない！！」
だがソリダスは俺の言葉を笑い飛ばし、二刀を構えた。強烈な殺気が俺を射抜く。
「繰り言はそこまでだ！いくぞ！！」#C0
*/
E15: << EOF
When I came to, I found myself in what 
Solidus had called the gap between the 
parallel universes again. I heard 
Otacon's happy voice from somewhere.
"Snake! We did it! Everything went 
perfectly on my end! Space has begun 
to repair itself!"
"Exactly like it was before?" I spoke to
thin air.
"Should be. You should be pulled into 
our universe soon. But it looks like the 
repairs are occurring sequentially. 
It could be some time before everything
goes back to normal..."
"SNAKE!!" Echoed Solidus' angry 
voice.
There was a flash of light, and when 
I opened my eyes I found myself 
standing atop the destroyed Federal 
Hall along with Solidus. Solidus drew 
his swords. I said:
"Just stop! It's over. The universe is 
repairing itself."
"No, it's not over. If I manage to kill 
you here, at least one universe where 
I survive will be made definite!"
"Stop it! There has to be another way!
I'm returning to my universe. I have no 
intention of interfering with yours. 
There's no reason for us to fight!!"
But Solidus merely laughed at my 
words, readying his swords. He fixed 
me with a piercing, menacing stare. 
"Enough talk! FIGHT ME!!"#C0
EOF
//}

//ソリダス戦後
//★Ｅ１６－１ {
/*
俺の一撃を受けたソリダスがフェデラルホールから落下する。とっさに延ばした俺の手は空を掴んだだけだった。ソリダスが悲鳴もなく落ちていく。それを俺は見送るしかなかった。ソリダスの体が地面に激突したその瞬間、凄まじい閃光が俺を襲った。#C0
*/
E16_1: << EOF
Solidus, felled by my shot, tumbled 
from the roof of Federal Hall. I threw 
my hand out for him, but caught 
nothing but air. Solidus' body fell in 
silence. I couldn't do anything but 
watch. The moment his body hit the 
ground, I was enveloped in a blindingly 
powerful light.#C0
EOF
//}

//★Ｅ１６－２ {
/*
光が消えると俺はアジトにいた。目の前にはオタコンとオルガがいる。オタコンが手を上げた。
「やあ。おかえり、スネーク」
オルガが言った。
「あなたが帰ってきたってことは、次は私の番かもしれないわね」
「なんだか名残惜しいね」
オルガが優しく微笑む。
「そうね。でもやっぱり帰らなきゃ。会いたい人だっているもの」
「子供か？」
俺の言葉にオルガがきょとんとした顔をする。
「え？子供はいないわ......まだ、ね」
そして少し照れたようにうつむき言った。
「旦那が待ってるの」
「旦那！？」
俺とオタコンは思わず声を上げた。
「ええ、こっちの宇宙では......」
俺は慌ててオルガを遮った。
「いや、聞かないでおこう」
オルガはにっこりと微笑んだ。同時にオルガの姿は光に包まれ、掻き消えた。
オタコンが寂しそうにつぶやいた。
「......行っちゃったね」
「ああ」
「......でも考えてみると、今僕の前にいるスネークが僕の宇宙のスネークとは限らないんだよね......」
オタコンの言う通りだった。無数に存在する並行宇宙には非常に似通った宇宙も存在するはずだ。今までお互いに違和感は感じなかったが、俺の目の前にいるオタコンは、俺の知っているオタコンよりも１ミリ背が高いのかもしれないし、犬好きではないかもしれない。もしかしたら実父は健在かもしれない。俺達が分かち合ったと信じている思い出は存在しないのかもしれない。だがそれを確かめる術はない。
俺が考え込んでいると、オタコンが手の平を差し出してきた。
「......僕の宇宙のスネークは僕に１００ドル貸しがあったはずだけど」
俺はそれを無視して言った。
「お前が俺の宇宙のオタコンなら、俺がお前のフィギュアを壊しちまったことを白状しても怒らないはずだ」
オタコンの目の色が変わる。
「なんだって！？どれのことだよ！！」
俺は思わず吹き出した。しばらく二人で笑いあう。俺達は気づいた。同じ思い出はもっていないかもしれない。この後別の宇宙に別れるのかもしれない。だが何の疑いもなく確かなことが一つある。俺達は親友だ。
世界が閃光で満たされる。
俺とオタコンはがっちりと握手をかわした。#C0
*/
E16_2: << EOF
When the light cleared, I found myself 
in the hideout. Otacon and Olga stood 
right in front of me. Otacon raised his 
hand.
"Hi. Welcome back, Snake."
"Your return could mean that I'll be 
next to go." said Olga.
"We'll miss you."
Olga smiled gently.
"I'll miss you, too. But I have to go. 
There's somebody I want to see."
"Your kid?"
Olga looked at me in surprise.
"What? I don't have any children... 
not yet."
She lowered her eyes as if 
embarrassed and said, 
"My husband is waiting for me."
"H-Husband?!" Otacon and I 
exclaimed in shock.
"That's right. In my universe..."
I quickly interrupted her.
"No. We'd probably be better off not 
knowing."
Olga smiled broadly. A bright light 
enveloped her body and she was gone.
Otacon said sadly:
"She's gone."
"Yeah."
"...But now that I think of it, the Snake 
standing in front of me seems different 
from the Snake I know from my 
universe..."
It was just as Otacon said. In an 
uncountable number of parallel 
universes, there have to be some that 
are incredibly similar. We hadn't felt 
anything out of the ordinary, but it's 
possible that the Otacon in front of me 
is a millimeter taller than the Otacon I 
know. Or maybe he doesn't like dogs. 
Perhaps his father is still alive. 
The memories we thought we shared 
might have been erased from existence.
But there was no way to check for sure.
As I stood engrossed in thought, 
Otacon extended an open palm.
"... The Snake in my universe owes me 
a hundred bucks."
Ignoring him, I said:
"The Otacon from my universe wouldn't 
be angry even if I told him I accidentally
broke one of his action figure toys."
Otacon's eyes widened in shock.
"What?! Which one did you break?!"
I let out a laugh. Soon both of us were 
laughing. We realized something. 
We might not have the same shared 
memories. And perhaps we'd be going 
to different universes shortly. But one 
thing was clear. We were friends.
The world filled with bright light.
Otacon and I shook hands.#C0
EOF
//}


//★Ｅ１６－３ {
/*
気がつくと俺はアジトの椅子に座っていた。あたりを見回す。いつものアジトだ。ソリダスに破壊された跡は全く無い。あの事件以前に時間が戻ったのだろうか。正面に座っていたオタコンと目が会った。
「戻ってきたのかな？」
オタコンが言う。口振りからしてどうやら俺と同じような体験をしたようだ。しかしほぼ同じ事件が起こっていた並行宇宙も無数にあるはずで、俺とこのオタコンが同じ宇宙にいたのかどうかは分からないし、確かめようも無い。だがそんなことはどうでもよかった。俺の目の前にいるのはオタコンなのだ。
「全部終わったね......」
オタコンが大きくのびをする。
俺は少し考えてから言った。
「......そうだろうか？」
「なんだって？」
「......本当にこれで終わりなのだろうか？」
俺は自分が感じていたちょっとした引っかかりのようなものをオタコンに聞かせた。
最後にソリダスと対峙した時、俺は奴と闘いたくなかった。だが実際には闘い、倒してしまった。まるで俺自身の意志ではないように......。ソリダスはコペルソーン・エンジンとＶＲシステムが並行宇宙に干渉すると言ったが、もしそれが間違っていたとしたら？あるいはそれ以外に他の並行宇宙を外部から観測し干渉する存在があるとしたら......？
「考え過ぎだよ」
オタコンは一笑に付した。
「それよりも......」
俺の方に体を乗り出す。
「僕のフィギュア壊したって！？」
「いや、それは......」
その時、突然ドアが開き、メイ・リンが入ってきた。彼女には何事も起こらなかったようだ。
「ちょっと、これ見てよ！！」
メイ・リンは手に持ったタブロイド新聞を投げて寄越した。#C0
*/
E16_3: << EOF
When I came to, I was sitting in a chair 
in the hideout. I looked around the 
room. It was the same hideout as ever. 
There was no trace of the damage 
Solidus had caused. Had I returned to 
just before the whole incident even 
happened? My path of vision crossed 
with Otacon, who was sitting across 
from me.
"Did we come back?" asked Otacon.
Judging from his tone of voice, it 
seems he'd been through a similar 
experience to mine. But there have to 
be an uncountable number of universes 
where nearly the same thing had 
occurred. I don't know if this Otacon 
had been in the same universe with 
me, and there was no way to check. 
But I don't really care. The person 
sitting in front of me is Otacon.
"It's all over..." said Otacon, stretching.
Something occurred to me and I said: 
"Are you sure?"
"What do you mean?"
"...I just wonder if it's really over yet."
I told Otacon about what I couldn't get 
out of my head. When I faced Solidus 
at the end, I really hadn't wanted to 
fight him. But I did fight, and I killed 
him. It was as if I hadn't been in 
control of my own body... Solidus said 
the Koppelthorn Engine and the VR 
system interfered with the parallel 
universes, but what if he's wrong? Or 
what if there's another device that 
allows one to externally observe and 
interfere with a parallel universe...? 
"You're thinking too much." Otacon 
said, laughing it off. "And anyway..."
He leaned towards me.
"Did you say you broke one of my 
action figures?!"
"No, I was just..."
Suddenly the door opened, and in 
walked Mei Ling. She looked like 
nothing had happened to her at all.
"Hey, check this out!"
Mei Ling threw us the tabloid she'd 
brought with her.#C0
EOF
//}





}



