/*
	tales_d_j.rsc
	    スネークテイルズ紙芝居用リソースヘッダ

	2002/06/04 S.Mukaide
	$Id: tales_d_e.before.rsc,v 1.1 2002/08/26 05:15:41 usr03005 Exp $


*/

// フォントチェック用define
/*
-fontcheckは省略可能です。５つのパラメータは順に 
width, height, chara_skip, line_skip, flag　をあらわします。
flagが1の場合は、禁則を行わない。(日本語以外)

-fontcheck width height chara_skip line_skip flag
*/
#define	TITLE_FONT_AREA		-fontcheck 520 120 0 12 0 


//---------------------------------------
//	テイルズＤ(死人が暴れる話)
//---------------------------------------

resource テイルズ＿Ｄ__e d:TITLE_FONT_AREA {

//最初
//★Ｄ１ {
/*
ENGLISH!!
ＮＹ湾沖、『ビッグ・シェル』。この海洋除染施設がＳＥＡＬＳ・デッドセルによる極秘合同演習の舞台に選ばれたのは、その入り組んだ構造と孤立した環境による。演習はデッドセルとその配下の部隊がテロリストを演じ、ＳＥＡＬのチームが鎮圧するという筋書きで行われていた。
｜
デッドセルは海軍最凶と恐れられた特殊部隊だ。政府重要施設へ抜打ち襲撃をかけ、その警備を監査する対テロ訓練が主な任務とされている。その襲撃の容赦無さは有名だったが、奴等が恐れられた本当の理由は他にあった。対テロ訓練任務は彼等にとってはむしろ演習であり、部隊の本当の目的はアメリカの国益を損なう対象への超法規的な破壊活動であるという噂だ。国内外を問わず、反米勢力の不審な壊滅や反米活動を行なう指導者の変死があると、軍関係者の間では決まってデッドセルの名がささやかれた。噂の真偽は定かではない。しかしデッドセルの名は死と災厄そのものとして畏怖されてきたのは確かだった。......一年前までは。
一年前、デッドセルを束ねていたジャクソン大佐が汚職と横領の罪で逮捕された。以来、演習での度を越した攻撃ぶりや、隊員達の素性の不確かさ、素行の悪さとあいまって、デッドセルは軍の汚点・不名誉の代名詞となっている。今やその戦闘力自体を疑問視する声すら上がっている状況だ。今回の演習もデッドセルへ久しぶりに与えられた正式任務だったが、デッドセルから参加を許されたのはリーダー格のヴァンプ一人だけだった。
｜
合同演習は進み、終盤に差し掛かっていた。テロリスト側の大半が『鎮圧』されたという報告を受けた直後、Ｆ脚で持ち場を警戒していた俺の無線機に緊急を意味するコードの通信が入った。
「......誰か応答してくれ！こちらアルファ・チーム、攻撃を受けている！」
俺は応答した。
「こちらプリスキン、どうした！？」
「......デッドセル......ヴァンプだ！奴が反逆を......こっちの部隊は......ほとんど奴に......」
背後に悲鳴と銃声が聞こえる。俺は聞いた。
「場所はどこだ！？」
「Ｂ脚......」
絶叫と共に通信は途絶した。
俺はすぐに別の周波数へ無線した。低い声が応える。
「ドルフだ。スネークか？」
海兵隊司令官、スコット・ドルフ。今回の演習に招かれ、『ビッグ・シェル』を訪れている。ドルフとデッドセルは縁があった。ドルフの娘、ヘレナはジャクソンの妻であり、ジャクソン逮捕の影響で、ドルフ自身の立場も危うくなっていると聞く。
俺は事態を報告した。ドルフが答える。
「ああ。こちらでも始まった。シェル２中央棟の内部が爆破されたんだ。Ｂ１はほぼ水没してしまった。ヴァンプの仕業だろう......」
俺は言った。
「......あんたが恐れていたのはこれだったのか？」
「ああ......」
三日前だ。俺はドルフから演習に加わるよう依頼を受けた。ドルフは何かが起こるのを恐れていた。俺に止めてほしいと言った。理由は語ろうとしなかった。ただ頼むと言った。俺は引き受けた。それ以上は聞かなかった。ドルフには昔の借りがある。
ドルフは俺のＩＤを偽造した。俺は『プリスキン中尉』として演習に編入され、ここにいる。
ドルフが言った。
「スネーク、我々は瓦礫と浸水で第二濾過室の北に閉じ込められている。脱出するには時間が掛かりそうだ。君にはＳＥＡＬＳの救援を頼む。Ｂ脚に向かってくれ」
「わかった」
「慎重にな。実弾は持っていないだろう？」
ドルフが言った。その通りだ。俺が今持っているのは演習用装備で、その中に実包は含まれていない。俺は答えた。
「ああ。だが何とでもなる」#C
*/
	D1: << EOF
"Big Shell", off the coast of New York. 
The offshore decontamination facility 
was selected as the setting for a 
top-secret joint exercise between the 
Navy SEALs and Dead Cell due to its 
complex structure and isolated 
environment. During the exercise, 
Dead Cell and a team under their 
command portrayed terrorists, while 
the SEAL team acted to suppress them.
｜
Dead Cell was a specialized naval 
combat team feared for their brutality. 
Their main function was anti-terrorist 
training, launching unannounced 
assaults on important government 
installations to determine their level of 
security. Although famous for the 
ferocity of their attacks, there was 
another reason for their reputation. It 
was rumored that their anti-terrorist 
training was cover for their true 
mission: operating above the law to 
undermine threats to American 
national interests. Whether inside the 
country or out, whenever anti-American 
sentiment was suspiciously crushed or 
leaders died unnatural deaths, military 
insiders inevitably associated Dead 
Cell's name with the act. The rumors 
were never confirmed. However, that 
the name of Dead Cell had become 
equated with death, disaster, and 
calamity was undeniable. Until one 
year ago...
｜
One year ago, Dead Cell commander 
Colonel Jackson was arrested on 
charges of graft and corruption. From 
that point on, the overzealous tactics 
during attacks, the shady pasts of the 
team members, and their combined 
inhumanity made Dead Cell's name 
synonymous with military corruption 
and dishonor. Even their very fighting 
ability had come under question. This 
exercise marked their first official 
assignment in some time, but only 
Dead Cell leader Vamp was granted 
permission to participate.
｜
The joint exercises were drawing to a 
close. I was guarding my assigned 
area in Strut F. Immediately after 
receiving a report that half of the 
terrorists had been subdued, a coded 
transmission signifying an emergency 
situation came through my Codec.
"...Come in, anyone! This is Alpha 
team. We're under attack!"
"This is Pliskin. What happened?!" 
I replied.
"...Dead Cell... it's Vamp! He 
double-crossed... The team is almost... 
by him..."
I could hear screams and the sound of 
gunfire in the background. "What is 
your location?" I asked.
"Strut B..."
I heard a scream, and the line went 
dead.
I immediately punched in a different 
frequency. A low voice answered:
"This is Dolph. Is this Snake?"
It was Marine Commander Scott Dolph.
He was on Big Shell as part of the 
exercise. 
I explained the situation to Dolph. He 
answered from all the noise:
"Looks like things just started on my 
end too. "
"What's the status?"
"There was an explosion in the Shell 2 
core. B1's almost totally flooded. "
"...Is this what you were worried 
about?"
Three days ago, I accepted a request 
from Dolph to join the exercise. Dolph 
was worried something might happen, 
and he asked me to put a stop to it if 
it did. But he didn't say why. All he 
told me was "I'm counting on you."
Dolph and Dead Cell were connected.  
Dolph's daughter Helena was 
Jackson's wife.  When Jackson was 
arrested, Dolph was the only one who 
defended Dead Cell and Jackson.  As 
a result, he risked his own position. 
Dolph seemed to be worried abou t 
something concerning Dead Cell.  
I accepted without any questions. 
I owed Dolph.
｜
Dolph created a fake identity for me. 
And here I was, along for the exercise 
as Lieutenant Junior Grade Pliskin.  I 
then asked him.
"What is Vamp up to?"
"I have no idea...."
Dolph said in a deeply troubled voice.
"Snake, we must figure out what's 
happening. We're trapped in the north 
part of the Filtration Chamber No.2 by 
debris and flooding. Looks like it'll take 
us some time to get out of here. I 
want you to rescue the SEALs. Head 
over to Strut B."
"Roger that."
"Be careful! You aren't carrying live 
ammunition, are you?"
Dolph was right. I was equipped for 
exercise and didn`t have any live 
rounds. "No. But I'll be okay." I replied.#C0
EOF
//}


//Ｂ脚到着時
//★Ｄ２ {
/*
ENGLISH!!
Ｂ脚に入った俺が見たものは惨状だった。あたり一面に転がるＳＥＡＬＳ隊員の死体。何もかもが血まみれで切り裂かれ、刻まれている。血だまりの中からうめき声が聞こえた。俺は声の元へ駆け寄った。
「しっかりしろ......」
まだ息のある隊員を抱き起こす。だがその腹に開いた傷は内臓まで達していた。手の施しようがないのは明らかだ。隊員が焦点の合わない目を俺に向けた。
「ヴァンプだ......奴は『私刑』だと......ジャクソンをはめた人間への報復......汚職は冤罪......謀略だと......」
俺は隊員を制止した。
「話さない方がいい」
だが隊員はそれが果たすべき任務であるかのようにしゃべり続けた。
「奴の狙いはオブライエン大佐だ......大佐は謀略など仕掛ける訳が無い......そんな人じゃないんだ......大佐は......シェル２中央棟１Ｆだ......」
隊員は俺の胸ぐらをつかんだ。死に瀕しているとは思えない強さだった。
「大佐を助けてくれ！......頼む......！！」
その手が力を失い下に落ちた。俺はそっとその目を閉じた。
オブライエン海軍大佐。今回の演習の責任者だ。湾岸や欧州、アフリカ等、各地の紛争で数々の戦績を上げている。有能で高潔な軍人としてマスコミにもしばしば取り上げられ、兵からの信望も厚いと言われる男だ。
俺はドルフと連絡を取った。今聞いた話を伝える。ドルフがつぶやいた。
「そうか......」
その声に驚きはなかった。悪い知らせを確認したような響きがあった。俺は聞いた。
「知っていたのか？」
ドルフはしばし逡巡してから答えた。
「......ここ数ヶ月で政府・軍の要人が変死する事件が相次いでいる。それがヴァンプの仕業だという噂がたっていた」
「ヴァンプが私刑を行なっていると？」
「信じたくはないが......」
「オブライエンもジャクソンをはめた一人だということか？」
「いや、それはないと思う。彼の評判は私も聞いている......」
ドルフもよくわからない様子だった。
「だがいずれにせよ私刑など許すわけにはいかない。スネーク、ヴァンプを止めてくれ」
俺は『ビッグ・シェル』の構造を思い返した。シェル２へ行くにはＤ脚からシェル１シェル２連絡橋を渡ればいい。まずはＤ脚へ向かわねばならない。#C0
*/
	D2: << EOF
I was greeted at Strut B by the sight of 
a total massacre. The bodies of dead 
SEALs littered the entire area. 
Everything had been torn apart and 
splattered with blood. I heard a moan 
coming from a body in a pool of blood.
"Hold on!"
I ran over and lifted the soldier in my 
arms. He was still breathing, but his 
wound was deep, reaching his internal 
organs. It was clear that he wouldn't 
make it. He looked at me with 
unfocused eyes.
"It was Vamp... He called it a 
'lynching'... revenge for those who 
backstabbed Jackson. He called... the 
bribery charge... a sham..."
"You'd better not talk now." I said, 
trying to quiet him.
But he keeps on talking.
"Vamp's going after Captain O'Brien... 
But that's nonsense. There's no way 
the Captain's involved... He's just... 
not like that. "
Naval Captain O'Brien, multiply 
decorated in the Persian Gulf, Europe, 
and Africa, was commander in chief for 
the entire exercise. The mass media 
praised the Captain as a competent 
and noble-minded military man who is 
well liked by his fellow soldiers.
He grabs my chest. His grip was 
surprisingly strong for a man on his 
deathbed. 
" The Captain is... on the first floor of 
the Shell 2 core....Save the Captain! 
Please..."
His body lost its strength. I closed his 
eyes. That's all I could do for him now.
I contacted Dolph, telling him what I'd 
just heard.
"I see..." mumbled Dolph.
His voice didn't sound surprised at all. 
In fact, he sounded like he'd just 
confirmed some bad news. 
"Did you know about this?" I asked.
He became quiet, answering hesitantly.
"There've been a lot of cases involving 
murdered government employees and 
servicemen lately. Rumor has it Vamp 
is behind it all."
"Is Jackson really innocent?"
"I don't know for sure, but perhaps. He 
was a real proud soldier.  He was not 
the corrupt kind."
"You think O'Brien backstabbed 
Jackson?"
"No, I don't think so. I know his 
reputation..."
"How can you be so sure?"
"It was O'Brien who planned this 
training. Ifhe was involved in Jackson's 
case, there's no way he would involve 
Vamp."
"That's only if O'Brien know of the 
rumors of the lynching, right?"
"He knew."
"Are you sure?"
"Oh yeah.  I heard of the rumors from 
O'Brien."
"...You and O'Brien talked about the 
rumors?"
I was a bit surprised.  O'Brien and 
Dolph... political Navy captain and 
head of the Marines. That just doesn't 
sound like a happy couple to talk about 
stuff.
Dolph answered as if he knew of my 
doubts.
"O'Brien and I didn't always get along 
because of our positions.  I don't think 
he liked me either.  But Idon't think he 
hated me that much to not give me 
advice, if that advice were to save my 
life."
"What do you mean?"
"O'Brien came to warn me - that I was 
Vamp's next target."
I then knew why Dolph really 
participated in this exercise.
"So you tried to rid Dead Cell of its 
infamous rumors by revealing yourself 
to vamp..."
"Correct. If nothing happens to me 
during the exercise, I could prove that 
the rumors were false."
"...You do trust Vamp."
Dolph scorns at himself.
"I wouldn't go that far.  If I trusted him 
completely, I wouldn't have called you...
I'm a coward."
I said to him that trusting something or 
someone isn't that easy.  Dolph did not 
respond.  I then asked him.
"But then it's not you but O'Brien that 
Vamp's after?"
"Yeah...Either O'Brien's information 
was wrong or Vamp's wrong..."
It seemed Dolph was confused too.
"The only thing that's certain is that 
Vamp is trying to kill O'Brien...Snake, 
you've got to stop Vamp."
"Got it."
I considered the structure of Big Shell 
again. I'd need to use the Shell 1 - 2 
connecting bridge from Strut D to get 
to Shell 2 where Vamp should be 
heading. That meant I'd have to head 
for Strut D first.#C0
EOF
//}

//Ｄ脚到着
//★Ｄ３ {
/*
ENGLISH!!
俺がＤ脚へ到着すると、ドルフから無線連絡が入った。オブライエンと連絡が取れたらしい。オブライエンはヴァンプの侵入を防ぐためにＤ脚からシェル１シェル２連絡橋への扉を封鎖。残ったＳＥＡＬＳを組織してＧ脚を中心に防御を固めているという。ドルフが言った。
「スネーク、君もＳＥＡＬＳと合流してほしい。相手はヴァンプだ。ＳＥＡＬＳと言えどそう簡単に止められるとは思えん」
「だがシェル１シェル２連絡橋が閉鎖されているならどうやってシェル２に行けばいい？」
「もう一つルートがある。『ビッグ・シェル』下部のオイルフェンスを渡るんだ。Ｅ脚Ｂ１にあるハシゴからオイルフェンスに下りることが出来る。そのまま渡ればシェル２Ｌ脚に出られるはずだ」
ヴァンプは既にシェル２へ向かっているはずだ。急がなくてはならない。#C0
*/
D3: << EOF
No sooner had I arrived at Strut D 
when Dolph contacted me. Looks like 
he had managed to get a hold of 
O'Brien. O'Brien had sealed off the 
door to the Shell 1 - 2 connecting 
bridge from Strut D to prevent Vamp 
from getting through. He'd also 
reorganized the surviving SEALs to 
defend Strut G. Dolph said:
"Snake, join the SEALs. Vamp is the 
enemy. Those men might be SEALs, 
but I doubt they can stop him on their 
own." 
"But how can I get to Shell 2? The 
Shell 1 - 2 connecting bridge is closed 
off."
"There is another way. Go across the 
oil fence in the lower part of Big Shell. 
Use the ladder on B1 of Strut E to get 
down to the oil fence. You should be 
able to get to Strut L of Shell 2 by 
crossing it." 
Vamp must have already left for 
Shell 2. I needed to hurry.#C0
EOF
//}

//Ｅ脚ハシゴ到着
//★Ｄ４ {
/*
ENGLISH!!
俺はＥ脚のハシゴに辿り着いた。４０メートルに及ぶハシゴを下りる。ようやくオイルフェンスに降り立った時、上方から銃声が聞こえた。シェル１シェル２連絡橋からだった。ヴァンプとＳＥＡＬＳが交戦しているのが見える。いやそれは交戦とは呼べない。一方的な殺戮だった。ヴァンプが走る。ＳＥＡＬＳが撃つ。ヴァンプには当たらない。ヴァンプがナイフを投げる。ＳＥＡＬＳが倒れる。ＳＥＡＬＳが応戦する。ヴァンプがよける。ＳＥＡＬＳの心臓を突く。腹を裂く。喉を掻き切る。瞬く間にＳＥＡＬＳは倒れていった。無傷のヴァンプへＳＥＡＬＳの最後の一人が何かを叫ぶ。次の瞬間、連絡橋が爆発した。橋に爆薬を仕掛けておいたのだろう。連続する爆発が連絡橋を炎に包み、橋は轟音と共に崩落を始めた。瓦礫が海面に落下する。俺はオイルフェンスを走った。背後で巨大な水柱が上がる。波が押し寄せ、オイルフェンスが跳ね上がる。何とかＬ脚下部に辿り着いた俺が振り向くと、宙を舞いながら、シェル２側に着地したヴァンプと目があった。ヴァンプがニヤリと笑う。長い舌でナイフをなめる。奴はナイフを俺に向かって突き出し、Ｇ脚内部へ消えた。
俺はハシゴをのぼり、シェル２Ｌ脚内部に到達した。ヴァンプはもうシェル２内に侵入している。オブライエンが危ない。俺はシェル２中央棟へ駆け出した。#C0
*/
D4: << EOF
I reached the Strut E ladder and 
climbed down about 40 meters. When I 
finally made it to the oil fence, I heard 
the sound of gunfire from above. It 
was coming from the Shell 1 - 2 
connecting bridge. I could see the 
SEALs fighting Vamp. But it wasn't a 
normal fight. It was a one-sided 
massacre. Vamp ran. The SEALs fired. 
But the bullets didn`t hit him. Vamp 
threw knives. Some of the men went 
down. Other SEALs fought back. Vamp 
wove and dodged. He slit the throats 
of the SEAL men, slicing their 
stomachs open and stabbing deep into 
their hearts. The SEALs went down in 
a split second. I saw the last man 
screaming at the totally unhurt Vamp. 
Suddenly, the connecting bridge 
exploded. It must have been wired up 
with bombs beforehand. A series of 
explosions enveloped the connecting 
bridge in flames, and it began to 
collapse with a roar. Debris rained on 
the surface of the sea. I made a run 
across the oil fence. A huge column of 
water rose behind my back. Waves 
churned, throwing the oil fence up with 
every undulation. I struggled along and 
finally made it to the lower part of 
Strut L. As I turned around, my path of 
vision crossed with Vamp's, who had 
tumbled from the sky and landed on 
the Shell 2 side. Vamp sneered, licked 
a knife with his long tongue and hurled 
it at me. And then he disappeared into 
the interior of Strut G.
I climbed the ladder and made it into 
Strut L of Shell 2. Vamp had already 
penetrated Shell 2. O'Brien was in 
danger. I ran for the Shell 2 core.#C0
EOF
//}

//中央棟入った
//★Ｄ５ {
/*
ENGLISH!!
俺はシェル２中央棟に侵入した。銃声はおろか人の気配もない。その静けさに俺は胸騒ぎを覚えた。ドルフから通信が入る。
「スネーク、今どこだ？」
「あんたの真上。シェル２中央棟１Ｆに到着したところだ。オブライエンは？」
「連絡が途絶した」
俺は舌打ちした。間に合わなかったか。
「いや、まだそうと決まった訳ではない......」
ドルフはそう言ったが、自分の言葉を信じることができないでいるようだった。俺は聞いた。
「オブライエンはこのフロアだったな？」
「ああ。空気清浄室の中に立て篭もっていたはずだ。ヴァンプの侵入を防ぐために床に高圧電流を流すと言っていた」
俺は通路の奥を見やった。床が鈍い光を発している。
「そのトラップは今も動いているようだが？」
「そうか......だが相手はヴァンプだ。役に立つとは限らない」
オブライエンの安否を確かめるには中に入るしかないようだった。俺は聞いた。
「トラップを無効にする方法は？」
「室内の電源パネルを破壊するしかないだろうな。リモコンミサイルならダクトから部屋の中に侵入させることができるはずだ。まずはリモコンミサイルを探してくれ」#C0
*/
D5: << EOF
I entered the Shell 2 core. There 
wasn't any gunfire, or any sign of 
people. The silence made me uneasy. 
The Codec rang. It was Dolph.
"Snake, what is your location?"
"Right above your head. I've just 
entered the first floor of the Shell 2 
core. How's O'Brien?"
"I've lost contact."
I clicked my tongue. Were we too late?
"We still don't know what's happened 
for sure..." said Dolph. But he didn't 
sound too sure of himself.
"O'Brien should be on this floor, 
right?" I asked.
"That's right. He should be barricaded 
in the Air Purification Room. He told 
me he was going to electrify the floor 
to stop Vamp from getting inside."
I looked over the passageway. The 
floor there was glowing dimly.
"Looks like the trap's still working."
"Yeah? ...But we're up against Vamp. 
Who knows if it'll help."
There was only one way to confirm if 
O'Brien was dead or alive: going inside.
"How do I disarm the trap?" I asked.
"Destroying the circuit breaker in the 
room is probably the only way. You 
should be able to do it with a 
remote-controlled missile from a duct. 
You're going to have to find a 
remote-controlled missile first."#C0
EOF
//}

//配電盤破壊して大統領部屋入った。
//分岐：
//時間掛かった：D6_0→D6_1A→D6_2→D6_3A→D6_4
//時間かからなかった：D6_0→D6_1B→D6_2→D6_3B→D6_4

//★Ｄ６−０ {
/*
ENGLISH!!
空気清浄室に入った俺は、壁にもたれるように座り込んでいるオブライエンを見つけた。体中が切り裂かれている。殺すより苦痛を与えることを目的とした傷付け方だった。オブライエンがこちらを見た。俺の兵装を認める。血にまみれながらもオブライエンは安堵したような笑みを浮かべた。
「全滅ではなかったのだな......？」
俺はそれを手で制し、周囲の気配を探った。オブライエンが言う。
「ヴァンプならもういない......ダクトから......Ｂ１に向かったようだ」
その言葉通り、部屋の中には俺と死にかけた男以外誰もいないようだった。俺はオブライエンへ近づいた。
*/
D6_0: << EOF
As soon as I entered the Air 
Purification Room, I found O'Brien on 
the floor slumped against a wall. His 
body was covered with cuts. Judging 
from them, it seemed Vamp was 
torturing him instead of killing him 
instantly. O'Brien looked at me, 
recognizing my uniform. He was 
covered with blood, but he smiled in 
relief.
"I guess we haven't been wiped out 
yet...?"
I signaled for him to keep quiet and 
examined the surroundings. O'Brien 
said,
"Vamp isn't here anymore... Looks 
like he left for B1 from the duct."
It seemed nobody was in the room 
except the dying man and myself. I 
went up to O'Brien.#C0
EOF
//}

//電源パネル破壊まで時間掛かった場合
//★Ｄ６−１Ａ {
/*
ENGLISH!!
出血がひどい。もう長くは持たないように見える。
*/
D6_1A: << EOF
He's bleeding pretty badly. Looks like 
he doesn't have much time.#C0
EOF
//}

//電源パネル破壊まで時間掛からなかった場合
//★Ｄ６−１Ｂ {
/*
ENGLISH!!
出血がひどい。一見して助からないとわかった。だがその目にはまだ力がある。
*/
D6_1B: << EOF
He's bleeding badly. I can tell he won't 
make it. But his eyes still show some 
strength.#C0
EOF
//}

//共通
//★Ｄ６−２ {
/*
ENGLISH!!
俺は手当てをしようとしたが、オブライエンは拒否した。
「よせ......もう助からない。それくらいわかる......」
オブライエンが俺に手を伸ばした。
「それより早くここを脱出しろ。応援を要請するんだ」
俺の腕を掴もうとする。だが出来なかった。腱を切られているのだろう。俺はその手を戻してやりながら答えた。
「いや。そうもいかない。......奴はＢ１と言ったな？」
Ｂ１にはドルフがいる。ヴァンプの次の目標はドルフなのかもしれない。ではドルフもジャクソンをはめた一人だというのか。しかしドルフはジャクソンの義理の父であり、そもそも誰かを罠にかけるような人間ではない......。考えを巡らせる俺にオブライエンが言った。
「中尉、やめろ。君は逃げるんだ。ヴァンプには関わるな」
「だが奴に見当違いの私刑ごっこを続けさせておくわけにはいくまい？」
「私刑だと？」
オブライエンが笑おうとした。だが血を吐くだけに終わった。俺は言った。
「奴はあんたがジャクソンに冤罪をかぶせたと......」
「冤罪？馬鹿を言うな。私はそんなことはしていない。......誰にもできはしない」
「どういうことだ？」
「ジャクソンの汚職は事実だからだ。多くの証拠がある......」
「なんだって！？」
*/
D6_2: << EOF
I tried to treat him, but O'Brien refused.
"Stop it... I'm not going to make it 
anyway. I know that...Get out of here 
now. Call for backup."
I said to him.
"No, I can't do that. You said he went 
to B1, right?" I said.
Dolph was on B1. As O'Brien told 
Dolph, he might be the next target. 
But Dolph is Jackson's father-in-law. 
And besides, he's not the kind of 
person to backstab someone...
Something was wrong.
While I was turning this over in my 
head, O'Brien said,
 "Don't do it, Lieutenant. You'd better 
leave. Don't get involved with Vamp." 
"But we can't let him continue his little 
lynching game, right?"
"Lynching?"
O'Brien tried to laugh. But he coughed 
up blood instead. 
"Vamp said you falsely charged 
Jackson..."
"That's crazy. I didn't do that. Nobody 
could."
"What do you mean?"
"The corruption case is airtight. 
There's plenty of proof..."
"What?!"#C0
EOF
//}

//電源パネル破壊まで時間掛かった場合
//★Ｄ６−３Ａ {
/*
ENGLISH!!
オブライエンは何かを言おうとして咳き込んだ。さらに多くの血を吐く。そしてもう口を開くことはなかった。
*/
D6_3A: << EOF
O'Brien tried to say something, but 
only coughed. He coughed up more 
blood. Then he was quiet for good.#C0
EOF
//}

//電源パネル破壊まで時間掛からなかった場合
//★Ｄ６−３Ｂ {
/*
ENGLISH!!
オブライエンは答えた。
「ヴァンプは騙されているんだ。ヴァンプに吹き込んだ人間がいる......『ジャクソン大佐は汚職などしていない、陥れられたのだ』とな......」
ヴァンプに殺されたといわれる人間は軍や政府の要人だという。政敵も多いだろう。自分に敵対する者を『ジャクソンをはめた人間』であると名指しして、ヴァンプに殺させている人間がいるということなのか。
「あんた、それが誰だか知ってるのか？」
俺は聞いたが、オブライエンにはもう聞こえていないようだった。
「私は......ヴァンプを止めようとした......彼等の能力を生かせる場を与えようと......。今回の演習が終われば、デッドセルは私の直下に再編成される予定だった。だが彼は......私を......裏切り......」
それ以上は言葉にならなかった。オブライエンはさらに多くの血を吐き、虚空を睨みながら死んだ。
*/
D6_3B: << EOF
"Vamp has been deceived. Somebody 
lied to him, telling him Colonel Jackson 
wasn't bribed. He was falsely 
accused..." answered O'Brien.
I heard Vamp's victims included 
important men from the government 
and military. They must have had 
plenty of political enemies. Maybe 
someone was claiming they 
backstabbed Jackson and manipulating 
Vamp to murder them?
"Do you know who it was?" I asked. 
But he didn't seem to hear it.
"I... tried to stop Vamp. I wanted to put 
him where he could make the most of 
his abilities... Once this exercise was 
over, Dead Cell was going to be 
reorganized, coming under my direct 
command... But he betrayed me..."
Those were his final words. He 
coughed up more blood, his eyes 
grew distant, and he died.#C0
EOF
//}

//共通
//★Ｄ６−４ {
/*
ENGLISH!!
俺はドルフと連絡を取ろうとした。しかし応答はなかった。何度呼びかけても無駄だった。ヴァンプは既にＢ１へ向かっているという。一刻も早くＢ１へ辿り着かねばならない。#C0
*/
D6_4: << EOF
I tried to contact Dolph numerous 
times by Codec without any response. 
I heard Vamp had already left for B1. 
I had to get to B1 right away.#C0
EOF
//}


//エレベータ乗った
//★Ｄ７ {
/*
ENGLISH!!
俺はエレベータに乗り込んだ。Ｂ１に着く。ドルフから聞いた通りだった。爆破によって通路はふさがれ、破壊された浄化槽から流れ込んだ大量の水がフロアを水没させている。俺は瓦礫をくぐりぬけ、水中を潜り、先に進んだ。ドルフはこの北にいるはずだ。扉を開け、第二濾過室に入る。身も凍るような静寂。その暗がりの中に一人立つ影があった。ヴァンプだ。奴はゆっくりと目を開いた。
「待ちかねたぞ......」
「待っていただと？」
「そうだ。デッドセルの終焉を飾るに相応しい生け贄をな......つまり貴様だ。ソリッド・スネーク」
「狙いはドルフじゃなかったのか？」
「いいや。俺が最後に殺すのは貴様だ」
ヴァンプが手首をひらめかせる。次の瞬間、その手にナイフが握られていた。
「ヴァンプ、やめろ。ジャクソンの汚職は......」
「知った風な口を聞くな！」
ヴァンプが激昂する。
「貴様は何もわかっていない。だがわかる必要も無い。ただデッドセルの歴史に加わることを知ればいい。貴様の死がな。いくぞ！」#C0
*/
D7: << EOF
I got on an elevator. I arrived at B1. 
B1 was exactly as Dolph had described.
The passageway had been closed off 
by an explosion and flooded from a 
damaged water-purification tank. 
I swam through the water and went 
through the debris to move forward. 
Dolph must be to the north. I opened a 
door and entered the second filtration 
room. Dead silence. Suddenly I spied 
someone standing in the darkness. 
Vamp. He slowly opened his closed 
eyes.
"I was waiting for you..."
"Waiting for me?"
"That's right. I was waiting for the 
perfect opponent to commemorate the 
end of Dead Cell. And you're it...Solid 
Snake."
"What are you saying!? Vamp, knock 
it off.  No one backstabbed Jackson. 
His corruption..."
Vamp spoke calmly.
"I know."
"What!?
Vamp stared at me with sad eyes.
"Snake, You don't understand anything,
and you don't have to. All you need to 
know is that you'll be gracing Dead 
Cell's legacy... as the final victim. 
C'mon!"#C0
EOF
//}





//分岐
//海軍大佐時間掛かった＆ヴァンプ殺した:
//D8_1XA→D8_2→D8_3AX→D8_4AA→D8_5XA→D8_6
//海軍大佐時間掛かった＆ヴァンプ気絶:
//D8_1XB→D8_2→D8_3AX→D8_4AB→D8_5AB→D8_6
//海軍大佐時間掛らない＆ヴァンプ殺した:
//D8_1XA→D8_2→D8_3BX→D8_4BA→D8_5XA→D8_6
//海軍大佐時間掛らない＆ヴァンプ気絶:
//D8_1XB→D8_2→D8_3BX→D8_4BB→D8_5BB→D8_6

//ヴァンプ殺した
//★Ｄ８−１ＸＡ {
/*
ENGLISH!!
俺は血だまりに沈んだヴァンプに近づいた。
*/
D8_1XA: << EOF
I approached Vamp as he lay in a 
pool of his own blood.#C0
EOF
//}

//ヴァンプ気絶させた
//★Ｄ８−１ＸＢ {
/*
ENGLISH!!
俺は倒れたヴァンプに近づいた。胸がかすかに上下している。気絶しているだけだ。
間もなくヴァンプが目を開けた。
「殺してくれ」
「俺はお前を止めろと頼まれただけだ」
俺は言った。
*/
D8_1XB: << EOF
I approached Vamp. He was still 
breathing. Just unconscious. Soon he 
opened his eyes.
"Kill me."
"They only asked me to stop you." 
I said.#C0
EOF
//}

//共通
//★Ｄ８−２ {
/*
ENGLISH!!
「......お前の『私刑』は無意味だった」
「いいや。意味はあった......」
「なかったんだ。ジャクソンは本当に......」
「知っていた」
「何だと？」
ヴァンプは静かに言った。
「大佐の汚職の件なら知っていた」
「ならばどうして！？」
「名誉を取り戻すためだ......」
ヴァンプは語った。
ジャクソンが逮捕され、デッドセルの名は地に落ちた。全米軍の面汚し、軍人の誇りを失ったクズの集まり......。軍はジャクソンを醜聞として切って捨てた。それだけではなかった。デッドセルの功績自体も無視しようとした。それは死よりも耐え難いことだった。ヴァンプは言った。
「確かに俺達は多くを殺した。民間人も殺した。女も殺した。子供も殺した。老人も殺した。丸腰でも殺した。無抵抗でも殺した。命乞いされても殺した。......だが喜んで殺したわけじゃない。全て任務だった。表に出せない任務。誰もが厭う任務。だが果たさればならない任務だ。俺達が全て引き受けた。オブライエンのような男が称賛を浴びられるのは俺達が手を汚し、泥水をすすり、悪夢にうなされてきたからだ」
ヴァンプは大きく息をついた。
「俺達は最低の人でなしか？おそらくそうだろう。では俺達は最低の軍人か？断じて違う。軍務に対する俺達の誠実と純潔は誰にも否定させない。それだけが俺達の拠り所だった。しかし今......最後の誇りまでが打ち消されようとしている。だから俺は行動を起こした」
「それがあの『私刑』だと？」
「そうだ。俺が『私刑』として人を殺せば、世間は疑問を抱くだろう。ジャクソン大佐は、もしかしたら無実だったのではなかろうか、と。さらに殺しつづければ、いつか世間は信じるはずだ。本当に無実だったに違いない、とな......そうなればデッドセルの名誉も戻ってくる......」
*/
D8_2: << EOF
"Vamp, tell my why you went on with 
your lynching when you knew it was 
meaningless?"
Vamp twitches his mouth.
"Because it wasn't meaningless."
Jackson's arrest had been the final 
blow to Dead Cell's reputation. They 
called it a stain on the entire U.S. 
military, called them a group of 
disgraced losers... The military had cut 
Jackson off as a liability. But that 
wasn't all. The brass tried to bury 
Dead Cell's previous record of success 
as well. It was a fate worse than death.
"We killed more than a few people. We 
killed civilians, women, children, the 
elderly. Some were unarmed. Some 
didn't resist. Some even begged for 
their lives. But we killed them all. Of 
course, we didn't kill them for pleasure. 
It was our mission. Dirty work, those 
types of missions. Everyone hated 
them, but they had to be done. So we 
took them all. A man like O'Brien owes 
his spotless reputation to the fact that 
we got our hands dirty, crawled 
through mud, and woke up screaming 
from nightmares..." Vamp clinched his 
fist.
"Are we the lowest of men? Perhaps. 
But the lowest of soldiers? Definitely 
not. I won't allow our honor and purity 
to be stained. That was all we had! 
But now... this last bit of pride has 
been denied us as well. That's why I 
went on this mission."
"You mean your 'lynching'?"
"That's right. If I kill people out of 
revenge, people may start wondering. 
They may even think that perhaps 
Colonel Jackson could be innocent. 
The more I keep on killing, the more 
the public will have to believe. Believe 
in his innocence..."#C0
EOF
//}




//海軍大佐助けるまで遅かった
//★Ｄ８−３ＡＸ {
/*
ENGLISH!!
「そんなものを人が信じると思うのか？」
「信じるさ。世界のほとんどは貴様の言う『そんなもの』で成り立ち、動いている。そうだろう？」
ヴァンプは冷たく笑った。だがその目は妙にうつろに見えた。俺は言った。
「だがお前は信じていないんじゃないか？名誉が戻るとしても、お前の元に戻ってくるとは......」
ヴァンプは自嘲に口元を歪ませた。
「......そうかもしれんな。俺はただの人殺しになってしまった......」
デッドセルに純潔があったとしてもそれは失われた。ヴァンプの手によって。俺の目の前に誇りある軍人の姿はなかった。哀れなテロリストが横たわっているだけだった。奴自身がそれを知っている。『私刑』に対する罰は既に下っているのかもしれなかった。ヴァンプは言った。
「だがそれでもいい。守ることが出来る」
「守る？」
「ジャクソン大佐、ヘレナとお腹の子供......そしてスコット......。そのためならば俺は......」
*/
D8_3AX: << EOF
"Did you really expect anyone to 
believe that crap?"
"Of course. The world lives and 
breathes on what you call 'crap'. Don't 
you think?"
Vamp laughed coldly. But his eyes 
looked empty.  His eyes weren't those 
of someone who regained what they 
lost. They werer of those who lost 
something instead.  Then I realized.
"But you don't believe it, do you? 
Even if Dead Cell's reputation is 
restored, it won't matter for you..."
Vamp shut his eyes as if he was in 
pain.
"Maybe you're right."
Any 'purity' Dead Cell had possessed 
in its mission had been lost at Vamp's 
hand. The person in front of me wasn't 
a proud soldier. He was nothing more 
than a pathetic terrorist. And he knew 
it. Perhaps that was the punishment 
for his "lynching." Vamp said:
"I don't care. At least I can protect 
someone."
"Protect?"
"I'll do anything for Colonel Jackson, 
Helena, their unborn baby... and 
Scott..."#C0
EOF
//}


//ヴァンプ殺した場合
//★Ｄ８−４ＡＡ {
/*
ENGLISH!!
「馬鹿めが！」
怒声が響いた。ドルフだった。戦闘を聞きつけて来たのだろう。その姿を認めたヴァンプの口元がほころんだ。同時にその目から光が急速に失われていく。ドルフはヴァンプに歩み寄りながら言った。
「そんなことを求めるものか。へレナもジャクソンも......私も。わかっているだろう......」
ドルフは冷たくなっていくヴァンプの手をとった。
「私はお前がいてくれるだけで......」
ヴァンプは微笑んだまま死んだ。
*/
D8_4AA: << EOF
"You fool!"
The shout echoed through the air. 
It was Dolph. He must have heard the 
battle and found us. Vamp smiled as 
he saw Dolph. His eyes quickly began 
to cloud. Dolph came up to Vamp and 
said:
"Nobody asked you to do what you did. 
Not Helena, not Jackson...not even 
me. You knew that, didn't you?"
Dolph picked up Vamp's hand.
"Just having you around was good 
enough for me..."
Vamp died with a smile on his face.#C0
EOF
//}

//★Ｄ８−５ＸＡ {
/*
ENGLISH!!
ヴァンプは死に、『ビッグ・シェル』はＳＥＡＬＳの応援によって鎮圧された。後日、デッドセルは正式に解散。ドルフは軍を退き、ジャクソンは獄中で病死した。精神衰弱が原因だったという。
*/
D8_5XA: << EOF
Vamp died. A backup SEAL team 
secured Big Shell. Dead Cell was 
officially dissolved some time after 
that. Dolph retired from the military, 
and Jackson died in prison. They said 
it was due to mental exhaustion.#C0
EOF
//}

//★Ｄ８−６ {
/*
ENGLISH!!
そして時は流れ、いつしか噂が流れ始めた。噂はこう言っていた。
ジャクソンの汚職は冤罪だった、デッドセルは何者かに陥れられたのだ、と。#C0
*/
D8_6: << EOF
Some time later, a rumor began 
making the rounds. It said Jackson 
had been falsely accused, and Dead 
Cell had been framed.#C0
EOF
//}


//ヴァンプ気絶させた場合
//★Ｄ８−４ＡＢ {
/*
ENGLISH!!
「馬鹿めが！」
怒声が響いた。ドルフだった。戦闘を聞きつけて来たのだろう。その姿を認めたヴァンプの口元がほころんだ。ドルフはヴァンプに歩み寄りながら言った。
「そんなことを求めるものか。ヘレナもジャクソンも......私も。わかっているはずだ。私は......」
ヴァンプがドルフを遮った。
「スコット、やらなければならなかったんだ」
「なぜだ？」
ヴァンプは目をそらし、黙り込んだ。ドルフが言った。
「話してはくれないのか」
しばしの沈黙の後、ヴァンプは言った。
「......あなたには関係のない話だからな」
「本気で言ってるのか？」
「そうだ」
「変わったな......」
ヴァンプは何かに耐えるように唇を噛んだ。
「だと思う。変わらないのはあなただけだ。......俺にはもう関わらないでくれ。二度とあなたの顔は見たくない」
ドルフの瞳が一瞬震えた。
「そうか......」
*/
D8_4AB: << EOF
"You fool!"
The shout echoed through the air. 
It was Dolph. He must have heard the 
battle and found us. Vamp smiled as 
he saw Dolph. His eyes quickly began 
to cloud. Dolph came up to Vamp and 
said:
"Nobody asked you to do what you did. 
Not Helena, not Jackson...not even me.
You knew that, didn't you?"
Vamp interrupted Dolph.
"Scott. I had to do it."
"Why?"
Vamp turned his eyes away and 
became quiet. 
"You won't tell me?" asked Dolph.
After a moment of silence, Vamp said:
"...It's got nothing to do with you."
"Are you being serious?"
"Yes."
"You've changed..."
"I think so too. You're the only one who
hasn't..."
Vamp bit his lip as though trying to 
endure something and said,
"...Just leave me alone. I don't want to 
see you ever again."
Dolph's eyes clouded for a moment.
"I see..."#C0
EOF
//}

//★Ｄ８−５ＡＢ {
/*
ENGLISH!!
ヴァンプは拘束され、『ビッグ・シェル』はＳＥＡＬＳの応援によって鎮圧された。
後日、デッドセルは正式に解散。ドルフは軍を退き、ジャクソンは獄中で病死した。精神衰弱が原因だという。
ヴァンプは軍刑務所に収監されたが間もなく脱走した。その後も政府・軍関係者の変死が散発している。奴の仕業だという噂があるが、真相は定かではない。
*/
D8_5AB: << EOF
Vamp was taken into custody. 
A backup SEAL team secured Big 
Shell. Dead Cell was officially dissolved
some time after that. Dolph retired from
the military, and Jackson died in prison.
They said it was due to mental 
exhaustion.
Vamp escaped shortly after being 
confined to a military prison. Shortly 
thereafter, various government and 
military insiders began dying suspicious 
deaths. It was rumored that he was 
behind it, but never confirmed.#C0
EOF
//}



//海軍大佐助けるまで早かった
//★Ｄ８−３Ｂ {
/*
ENGLISH!!
「......そう言われたのか？」
「何だと？」
「オブライエンが言った。ジャクソンに冤罪をかけた人間がいるとお前に告げ、自分の政敵を殺させている人間がいるとな」
ヴァンプが息を呑む。俺は続けた。
「だが奴は間違っていた。お前は騙されて利用されていたんじゃない。デッドセルの名誉を取り戻す方法があると持ち掛けられ、それを実行していたんだ。違うか？」
ヴァンプは押し黙った。否定はしなかった。俺は聞いた。
「誰だ？」
*/
D8_3BX: << EOF
"Is that what you were told?"
"What?"
"O'Brien told me. That someone told 
you Jackson had been framed to 
manipulate you to kill their political 
enemies."
Vamp gasped in surprise. I continued.
"But he was wrong. You weren't fooled 
or manipulated. You were told that 
there was a way to restore Dead Cell's 
reputation. And that's why you took the 
mission. Am I right?"
Vamp sat quietly, not denying anything.
"Who was it?" I asked.#C0
EOF
//}

//ヴァンプ殺した
//★Ｄ８−４ＢＡ {
/*
ENGLISH!!
ヴァンプが何か言おうとする。だがもうそれだけの力は残っていないようだった。
「なんと愚かなことを......」
声がした。ドルフだった。戦闘を聞きつけて来たのだろう。その姿を認めたヴァンプの口元がほころんだ。同時にその目から光が急速に失われていく。歩み寄ったドルフは冷たくなっていくヴァンプの手を取った。
「なぜ何も言わってくれなかった......？」
ヴァンプは微笑みながら死んだ。
*/
D8_4BA: << EOF
Vamp was trying to say something. 
But he didn't seem to have the 
strength.
"What have you done..."
Said a voice. It was Dolph. He must 
have heard the battle and found us. 
Vamp smiled as he saw Dolph. His 
eyes quickly began to cloud. Dolph 
came up to Vamp and took his hand.
"Why didn't you say anything to 
me...?"
Vamp died with a smile on his face.#C0
EOF
//}


//ヴァンプ気絶させた
//★Ｄ８−４ＢＢ {
/*
ENGLISH!!
ヴァンプは言った。
「言うものか......それが言えればこんなことにはなっていない......」
「ジャクソンか？」
声がした。ドルフだった。戦闘を聞きつけて来たのだろう。
ヴァンプは答えない。ドルフはヴァンプの傍らに膝をつき、もう一度言った。
「お前が守ろうとする人間など一人しかいまい。ジャクソンなのか？」
ヴァンプが目をそらす。ドルフは両手でヴァンプの顔を挟んだ。正面を向かせて問い質す。
「あいつに確かめるぞ。ジャクソンなのか！？」
ヴァンプはドルフの手を払った。
「......その通りだ。だが大佐を責めないでくれ......」
ヴァンプは血を吐くように語った。
「大佐は責任を感じていた。傷ついていた。自分を責めていた。デッドセルの名誉を回復する方法を探していた。そしてある日言い出したんだ。あの『私刑』の話を。聡明だった大佐が見る影もなかった......。憔悴しきっていた。壊れる寸前だった。いやもう壊れていたのかもしれない。......俺は従うしかなかった」
俺は聞いた。
「この事件もジャクソンが指示したのか？」
「いいや。俺の一存だ。もう終わりにするしかなかった。奴に知られた以上は......」
どうやってか、オブライエンはジャクソンが偽りの『私刑』を指示していたことを嗅ぎ付けたという。オブライエンはヴァンプに、デッドセルとして配下に加わることを求めた。問題の多いデッドセルを制御出来ることを内外に示して己の権威を高め、さらに子飼いの非合法活動部隊を手にすることで権力を拡張しようという企みが見えた。ヴァンプは拒んだ。デッドセルは魂は売らない。するとオブライエンは、ジャクソンがヴァンプに殺人を指示していたことを公表すると脅した。そうなればデッドセルの名は今度こそ永久に地に落ちる。
残る道はオブライエンを殺すことだけだった。ヴァンプは服従した振りをし、演習中にオブライエンを殺すことにした。大事件になるだろう。デッドセルは解体され、自分も死ぬことになるに違いない。それでも誇りを売り渡すよりはましだった、とヴァンプは語った。
ドルフが怒鳴った。
「馬鹿な！デッドセルなどくれてやればいい。そんなもののためにお前が死ぬことはない！」
ヴァンプが目をそらす。それを見たドルフは何かに気づいたようだった。
「......他にも何かあったんだな？」
ドルフの視線に射すくめられ、ヴァンプは諦めたように言った。
「奴は配下に加わるならば忠誠の証として......この演習中にスコット、あなたを殺せと。俺には......それだけは出来なかった......」
ドルフは呆然とつぶやいた。
「馬鹿な......」
ヴァンプが俺を哀願するように見る。
「さあ殺してくれ」
俺は言った。
「断る。お前の指図は受けない」
うなだれるヴァンプに俺は続けた。
「......それにお前は生きるべきだ。これ以上お前を気にかける人間を悲しませるな」
ドルフが言った。
「ありがとう......」
*/
D8_4BB: << EOF
Vamp groaned.
"I couldn't say anything..." 
"Was it Jackson?" asked a voice. 
It was Dolph. He must have heard the 
battle and found us. 
Vamp made no response. Dolph 
kneeled next to Vamp and repeated 
the question.
"There's only one person you're trying 
to protect. Is it Jackson?"
Vamp turned away. Dolph took Vamp's 
face in both hands and brought it to 
face forward again.
"I'm going to ask him myself anyway. 
Was it Jackson?!"
Vamp knocked Dolph's hands away.
"Yes...." 
Vamp said as he was coughing up 
blood. 
"The Captain blamed himself all the 
way. He was determined to find a way 
to restore Dead Cell and it's honor. 
That's all he talked about in jail - day 
after day as if he was possessed. One 
day he called on me. About this 
'lynching.' "
Vamp twitches his face.  I don't think 
he's doing it from the pain.
"There was no sign of the brilliant 
Captain I had known. He was on the 
edge. Or perhaps he had already gone 
over... There was nothing I could do 
but obey."
"...So Jackson ordered you to do 
this?" I asked.
"...No. It was all me. I had to end it. 
Because he had found out..."
"He?"
"O'Brien."
Vamp continued.
Somehow, O'Brien had discovered that 
Jackson had been behind the 
"lynching." So O'Brien had been 
demanding that Vamp be placed 
under his command as a member of 
Dead Cell. Vamp saw through O'Brien's
plan: strengthening his authority by 
showing the military he could control 
the problematic Dead Cell, while 
expanding his powers by taking charge 
of the extra-legal operations team. 
Vamp refused. Dead Cell would not be 
used as a tool of the government. 
So O'Brien threatened to expose the 
fact that Jackson was ordering Vamp 
to carry out the murders. And if he had,
Dead Cell's name would be tainted for 
good.  That's how O'Brien did things.
The only way left open to Vamp was to 
kill O'Brien. Vamp pretended to go 
along so that he could kill him during 
the exercise. He couldn't fake an 
accident.  It would be a revolt. Dead 
Cell would be dissolved, and he would 
probably die. But it was better than 
selling his own pride. 
Dolph exploded.
"Nonsense! If he'd wanted it, you 
should have given that guy Dead Cell! 
You didn't have to throw your life away 
for that! Why didn't you do so?"
Vamp looked away. Watching him, 
Dolph seemed to notice something.
"...There's something else, isn't there?"
Paralyzed by Dolph's gaze, Vamp 
spoke as if defeated.
"O'Brien dragged his political enemy 
into this exercise. O'Brien ordered me 
to kill this political enemy, you...Scott, 
as proof that I should be under his 
command.... But that was the last thing
I wanted to do..."
Dolph mumbled in shock,
"That's crazy..."
Vamp looked at me with pleading eyes.
"Now, kill me. There's nothing left in 
this world that I want to see."
"No way. I don't take orders from you," 
I said.
Vamp hung his head. I continued.
"...In fact, I think you should live. Don't 
make the people who care about you 
any sadder than you already have."
"Thank you..." said Dolph.#C0
EOF
//}

//★Ｄ８−５ＢＢ {
/*
ENGLISH!!
ヴァンプは拘束され、『ビッグ・シェル』はＳＥＡＬＳの応援によって鎮圧された。
後日、デッドセルは正式に解散。ドルフは軍を退いた。ヴァンプは軍刑務所に収監された。ジャクソンも非公開の軍事裁判で終身刑を受けた。共に今も服役している。#C0
*/
D8_5BB: << EOF
Vamp was taken into custody. 
A backup SEAL team secured Big 
Shell. Dead Cell was officially dissolved 
some time after that. Dolph retired from
the military, and Vamp was placed in 
a military prison. Jackson received 
a life sentence from a closed-door 
court martial. He remains in prison to 
this day.#C0
EOF
//}



}
