/*
	tales_comment_j.h
	  

	2002/06/04 S.Mukaide
	$Id: tales_comment_e.before.rsc,v 1.1 2002/08/26 05:15:41 usr03005 Exp $


*/

// フォントチェック用define
/*
-fontcheckは省略可能です。５つのパラメータは順に 
width, height, chara_skip, line_skip, flag　をあらわします。
flagが1の場合は、禁則を行わない。(日本語以外)

-fontcheck width height chara_skip line_skip flag
*/
#define	TITLE_FONT_AREA		-fontcheck 520 120 0 12 0 


resource テイルズ＿独り言__e d:TITLE_FONT_AREA {

//最初〜エイムズ会うまで
/*
俺は『ビッグ・シェル』への潜入に成功した。まずは内部協力者のエイムズと接触して情報を入手するのが先決だ。大佐によれば、エイムズはＦ脚にいるという。
*/
	A_エイムズ会うまで: << EOF
I successfully made it into "Big Shell." 
I should first get in touch with informant 
Ames and obtain information. According to 
Colonel, Ames is in Strut F.
EOF

/*
エイムズはこのＦ脚内のどこかにいる。だが彼は敵から姿を隠しているはずだ。俺が敵に追われている状態では姿を現さないだろう。見つからないようにエイムズを探さなければならない。
*/
	A_エイムズ会うまで＿Ｆ脚: << EOF
Ames is somewhere here in Strut F. 
He must be hiding himself from the enemy. 
He won't come out if I'm being chased by 
the enemy. I have to find him without being 
spotted.
EOF

//エイムズ会った後〜人質部屋前
/*
エイムズはファットマンがヘリポートにいると語った。また大佐によれば人質がシェル１中央棟Ｂ１に捕えられていると言う。どちらを優先するかは俺の判断に任された。
*/
A_エイムズ会った後人質部屋前: << EOF
Ames told me that Fatman is on the heliport. 
And according to Colonel, there are hostages 
tied up on Level B1, Shell 1 Core. Which site 
I choose is up to my judgment.
EOF

/*
人質を爆殺したファットマンは、次の標的を俺に定めた。迫り来るファットマンの巨体。奴を倒すしかない。
*/
A_爆弾未処理ファットマン戦中: << EOF
After killing the hostages with his bomb, 
Fatman has selected me as his next target. 
This massive body of his -- I have no choice 
but to take him out.
EOF

/*
俺は人質の捕えられている部屋に辿り着いた。しかし大佐は、ジェニファーを特定するには集音マイクが必要だと言う。まずは集音マイクを探さなければならないようだ。
*/
A_人質部屋内＿マイクなし: << EOF
I arrived at the room with the hostages. 
Colonel says that I need a directional mic 
to identify Jennifer. I guess I must find a 
directional mic first.
EOF

/*
この部屋のどこかにジェニファーがいる。ジェニファーは今もメールを打っているらしい。集音マイクで彼女を探し出し、そのままアクションボタンで呼びかければ接触できるはずだ。
*/
A_人質部屋内＿マイクあり: << EOF
Jennifer is in this room somewhere. 
Supposedly, Jennifer is typing up an e-mail 
right now. If I find her with the directional mic 
and press the Action button, I think I can get 
in touch with her.
EOF

/*
俺はシェル１中央棟Ｂ１で人質達が捕らわれているのを確認した。集音マイクを使えばジェニファーと接触できるはずだ。
*/
A_人質部屋後人質部屋外: << EOF
I have confirmed that the hostages are kept 
on Level B1, Shell 1 Core. I think I can 
contact Jennifer with the directional mic.
EOF

/*
ジェニファーは、ファットマンがシェル１中央棟に爆弾を仕掛け、人質の動きを封じていると語った。ファットマンはヘリポートにいるはずだ。
*/
A_ジェニファー後爆弾無線前: << EOF
Jennifer said that Fatman has set explosives 
in the Shell 1 Core and is preventing the 
hostages from moving. Fatman must be on 
the heliport.
EOF

/*
爆弾は処理した。人質はこれで安全と言えるだろう。残る任務はテロリストの制圧となる。ファットマンはヘリポートにいるはずだ。
*/
A_爆弾無線後ファットマン前: << EOF
I have defused the explosives. The 
hostages are all safe now. The other 
half of the mission is suppressing the 
terrorist.Fatman must be on the heliport.
EOF

/*
計画の失敗を悟ったファットマンは、次の標的を俺に定めた。迫り来るファットマンの巨体。奴を倒すしかない。
*/
A_爆弾処理済みファットマン戦中: << EOF
Sensing the failure of his plan, Fatman 
has selected me has his next target. 
This massive body of his -- I have no 
choice but to take him out.
EOF

/*
『ビッグ・シェル』は正体不明の武装集団に占拠されていた。エマの身が気がかりだ。彼女はどこにいるのだろうか。
*/

B_立ち聞き前: << EOF
"Big Shell" has been taken over by an 
unidentified armed group. I'm really worried 
about Emma. Where the hell can she be?
EOF

/*
『ビッグ・シェル』は正体不明の武装集団に占拠されていた。エマの身が気がかりだ。彼女はＣ脚のどこかに逃げ込んだという。
*/
B_立ち聞き後＿エマ接触前: << EOF
"Big Shell" has been taken over by an 
unidentified armed group. I'm really worried 
about Emma. She supposedly hid herself 
somewhere in Strut C.
EOF

/*
エマの無事を確認したのもつかの間、気がつくと俺達は敵に包囲されていた。なだれ込んでくる敵兵達。エマを守るには奴等を全て倒さなければならない。
*/
B_エマ接触後＿敵兵ラッシュ中: << EOF
Only moments after I confirmed Emma's 
safety, we were surrounded by enemies. 
Enemy soldiers keep on flooding in. I must 
take out every single one of them to save 
Emma.
EOF

/*
人質が捕らわれているシェル２へ行くには、シェル１シェル２連絡橋の扉を解放する必要があるとエマは語った。そのためにはＢ脚のノードにアクセスすればいいらしい。
*/
B_敵兵ラッシュ後＿ノード前: << EOF
Emma told me that I must open the door to 
the Shell 1-Shell 2 connecting bridge to go to 
Shell 2 where the hostages are kept.I must 
access the node in Strut B to do so.
EOF

/*
エマが敵に捕らわれてしまった。オタコンの協力で、エマはＦ脚のどこかにいることが判明する。何としてもエマを救出しなければならない。
*/
B_ノード後＿再会前＿Ｆ脚以外: << EOF
Emma has been captured. With Otacon's 
help, I found out thta Emma is somewhere 
in Strut F. I'll do anything it takes to rescue 
Emma.
EOF

/*
エマはこのＦ脚のどこかにいるはずだ。だが俺の潜入が知られれば、敵は容赦なくエマを殺すだろう。敵に見つかることなくエマを探し出さなければならない。
*/
B_ノード後＿再会前＿Ｆ脚内: << EOF
Emma is here somewhere in Strut F. If they 
find out I'm here, the enemy will kill Emma 
without mercy. I must find Emma without 
being spotted by the enemy.
EOF

/*
エマはなんとか無事だった。だがまだ多くの職員がシェル２に捕らわれている。人質を救出するため、俺とエマはＤ脚からシェル２へ向かうことにした。
*/
B_再会後＿Ｅ脚前: << EOF
Emma seems fine. But there still are a lot 
of those who work here tied up in Shell 2. 
Emma and I decided to go from Strut D to 
Shell 2 to save the hostages.
EOF

/*
麻薬の隠し場所を見つけた俺達。だがそこへ敵の大部隊が突入してきた。奴等の包囲を突破し、必ずエマと共にＤＥ連絡橋へ脱出してみせる。
*/
B_Ｅ脚脱出中: << EOF
We found where the drugs have been 
hidden. But then rushed in a large enemy 
unit. I WILL get past the surrounding 
enemies and escape to the DE connecting 
bridge together with Emma.
EOF

/*
エマは敵の大部分をＥ脚内に閉じ込めることに成功した。人質を救出するためシェル２へ向かう俺達。だがその前にハリアーが現れた。ハリアーを撃墜しなければ道は開けない。
*/
B_ハリアー戦中: << EOF
Emma successfully locked most of the 
enemies inside Strut E. The two of us head 
for Shell 2 to rescue the hostages. Then 
appears a Harrier right in front of us. There 
is no choice but to bring the Harrier down.
EOF

/*
俺は偽装タンカーへの潜入に成功する。だが既に船はゴルルコビッチの部隊によって制圧されていた。まずはメリルの安全を確かめねばならない。彼女はこの船のどこかにいるはずだ。
*/
C_艦橋メリル前: << EOF
I successfully infiltrated the disguised tanker. 
But it seems like the ship has already been 
occupied by Gurlukovich's army. I must first 
confirm Meryl's safety. She must be 
somewhere on this ship.
EOF

/*
メリルは別行動をとるとして去ってしまった。だがゴルルコビッチにメタルギアを渡す訳にはいかない。メリルによればメタルギアは船倉にあるという。
*/
C_艦橋メリル後＿長廊下戦前: << EOF
Meryl has left to pursue action on her own. 
I cannot make Gurlukovich take Metal Gear. 
According to Meryl, Metal Gear is in the 
holds.
EOF

/*
船倉を目指す俺は敵部隊の待ち伏せを受ける。行動が読まれていたのか。だが今は考える余裕はなかった。まずはこの窮地を切り抜けなければならない。
*/
C_長廊下戦中: << EOF
As I headed for the holds, I fell into enemy 
ambush. Did they know where I was heading? 
Now is no time for questions. I must first get 
out of this situation.
EOF

/*
メリルはゴルルコビッチと組んでいた。衝撃を受けつつも何とか船倉を脱出した俺に迫る敵の追っ手。まずはこの第二甲板から抜け出さねばならない。機関室まで戻れば何とかなるはずだ。

ＸＸＸＸＸＸＸＸｘ長い！ＸＸＸＸＸｘ
*/
C_長廊下脱出中: << EOF
I managed to escape the holds, but the 
enemies are still after me. I must first get 
out of Deck 2 here. Maybe if I go back to the 
engine room there is a way out.
EOF

/*
大佐はゴルルコビッチが仲間と合流しメタルギアを回収する可能性があるという。その前に船橋に辿り着き、船の進路を変えなければならない。
*/
C_長廊下脱出後メリル戦前: << EOF
Colonel says that Gurlukovich might join up 
with his people and get a hold of Metal Gear. 
I must get to the bridge before that and 
change the ship's course.
EOF

/*
全ては亡き父親の遺志を継ぐためだとメリルは言う。ゴルルコビッチの言う事実に疑念を挟む俺に、メリルは銃を向けた。
*/

C_メリル戦中: << EOF
Meryl says all this is for her to carry out the 
intention her late father. As I expressed my 
doubts toward the facts Gurlukovich spoke 
of, Meryl pointed her gun at me.
EOF




/*
ＳＥＡＬＳとの合同演習中、デッドセルが反乱を起こした。ヴァンプの意図は不明だが、まずは襲われたＳＥＡＬＳを助けなければならない。俺はＢ脚へ急ぐことにした。
*/
D_ＳＥＡＬＳ死体前: << EOF
Dead Cell revolted during the joint exercise 
with the SEALS. Vamp's intentions are not 
clear. But I must save the SEALS under 
attack. I am now heading toward Strut B.
EOF

/*
ＳＥＡＬＳの言い残した言葉によれば、ヴァンプの次の狙いはシェル２にいるオブライエンだという。俺はシェル２に渡るため、Ｄ脚へ向かう。
*/
D_ＳＥＡＬＳ死体後Ｄ脚前: << EOF
According to the last words of the SEAL, 
Vamp's next target is O'Brien in Shell 2. 
I head for Strut D to get to Shell 2.
EOF

/*
ＳＥＡＬＳの言い残した言葉によれば、ヴァンプの次の狙いはシェル２にいるオブライエンだという。だがＤ脚からシェル２への扉は閉鎖されている。残る道は海上のオイルフェンスを渡ることだ。オイルフェンスへのハシゴはＥ脚Ｂ１にある。
ＸＸＸＸＸＸＸＸｘ長い！ＸＸＸＸＸｘ
*/
D_Ｄ脚後Ｅ脚前: << EOF
Vamp's next target is O'Brien in Shell 2. 
The door from Strut D to Shell 2 is shut. 
The only way now is to cross the oil fence 
on the water. The ladder to the oil fence is 
on Level B1, Strut E.
EOF

/*
俺はシェル２に到着した。ヴァンプは既にシェル２へ到達している。オブライエンが危ない。オブライエンはシェル２中央棟１階のはずだ。
*/
D_Ｅ脚後シェル２中央棟１Ｆ前: << EOF
I arrived at Shell 2. Vamp has already been 
in Shell 2. O'Brien is in danger. O'Brien 
must be on the first floor in the Shell 2 Core.
EOF

/*
ドルフはオブライエンと連絡が途絶したという。だがオブライエンがいる空気清浄室の入口には、床へ高圧電流を流すトラップが設置されていた。オブライエンの安否を確かめるには、ダクトからリモコンミサイルを侵入させ、室内の配電盤を破壊しなければならない。

長い

*/
D_シェル２中央棟１Ｆ後配電盤破壊前: << EOF
Dolph says he lost contact with O'Brien. 
To confirm O'Brien's safety, I must shoot a 
remote-controlled missile through the ducts 
and destroy the power source in the room.
EOF

/*
ドルフはオブライエンと連絡が途絶したという。オブライエンの安否を確かめなければならない。俺は配電盤の破壊に成功した。オブライエンは空気清浄室だ。道を阻むトラップはもうない。
*/
D_シェル２中央棟１Ｆ後配電盤破壊後大佐接触前: << EOF
Dolph says he lost contact with O'Brien. 
I must confirm O'Brien's safety. 
I successfully destroyed the power source. 
O'Brien is in the air purification room. 
There is no trap blocking the way anymore.
EOF

/*
オブライエンは死んだ。ヴァンプはシェル２中央棟Ｂ１に向かったという。奴の次の狙いはドルフなのか。ヴァンプを止めねばならない。Ｂ１へはエレベータを使えば行けるはずだ。
*/
D_大佐接触後ヴァンプ戦前: << EOF
O'Brien is dead. Supposedly Vamp had 
headed for Level B1, Shell 2 Core. 
Is Dolph his next target? I must stop Vamp. 
The elevator should take me to Level B1.
EOF

/*
Ｂ１で俺はヴァンプと遭遇した。ジャクソンの汚職が事実だと告げる俺。だがヴァンプは問答無用で襲いかかってきた。
*/
D_ヴァンプ戦中: << EOF
I encountered Vamp on Level B1. I told him 
that Jackson's corruption accusation was 
true. But Vamp did not waste his breath and 
attacked me.
EOF

/*
今回の任務は、オタコン曰く謎の怪獣ゴルルゴンの写真を撮影することだ。ゴルルゴンはこのＡＢ連絡橋でしばしば目撃されているという。
*/
E_ＡＢ連絡橋ゴルルゴン登場前: << EOF
My mission is to take a photo of "Gurlugon" 
-- the mysterious monster according to 
Otacon. Supposedly, Gurlugon has been 
spotted many times by this AB connecting 
bridge.
EOF

/*
俺はゴルルゴン探索のため『ビッグ・シェル』に降り立つことになった。まずはゴルルゴンを見つけなければならない。
*/
E_回想Ｃ脚前: << EOF
I have come to the "Big Shell" for the search 
of Gurlugon. I must first find Gurlugon.
EOF

/*
俺はゴルルゴン探索のため『ビッグ・シェル』に降り立つことになった。まずはゴルルゴンを見つけなければならない。オタコンが言うには、ゴルルゴンはＡＢ連絡橋でよく目撃されるらしい。
*/
E_回想Ｃ脚後ゴルルゴン登場前: << EOF
I have come to the "Big Shell" for the search 
of Gurlugon. I must first find Gurlugon. 
According to Otacon, Gurlugon is often 
spotted by the AB connecting bridge.
EOF

/*
眼下で巨大な怪獣が泳ぎ回る。信じがたいことだが、こうなったら写真を撮るしかない。奴が浮上した瞬間がいいだろう。近すぎても遠すぎてもいけない。オタコンも唸るような写真を撮ってやる。

ＸＸＸＸＸＸＸＸｘ長い！ＸＸＸＸＸｘ
*/
E_ゴルルゴン撮影中: << EOF
This is unbelievable, but I guess I should be 
taking photos. The best moment is when it 
comes above the surface. I shouldn't be too 
near or far from it. I'm gonna take a photo 
good enough to make Otacon grunt.
EOF

/*
写真を撮り終えた俺は武装集団の襲撃を受けた。奴等は何者なのか？だが考えている時間はない。今は奴等を倒すしかないだろう。
*/
E_敵兵ラッシュ中: << EOF
After I took a photo, I have been attacked 
by an armed group. Who the hell are they? 
Now is no time for questions. I must first 
take them out.
EOF

/*
メイ・リンは、ＶＲ内でゴルルゴンを倒せば現実のゴルルゴンも消滅するのではないかという。乱暴過ぎる理論だとは思うが、結局俺はＶＲでゴルルゴンと戦うことになった。
*/
E_ゴルルゴン戦中: << EOF
Mei Ling suggests that Gurlugon in real life 
will disappear if Gurlugon is defeated in VR. 
I thought her logic was too radical, 
but I ended up facing Gurlugon in VR.
EOF


/*
特異点へ辿り着いた俺の前に、ソリダスが放ったＲＡＹの大群が現れる。オタコンが作業を完了するまで後少し。それまでＲＡＹを倒し続けるしかない。
*/
E_ＲＡＹ戦中: << EOF
When I arrived at the singularity, a bunch of 
RAYs sent by Solidus have appeared in front 
of me. Otacon needs some more time to 
complete his operation. I have no choice but 
to keep on fighting these RAYs.
EOF

/*
宇宙は修復されはじめた。だが帰還を待つ俺の前に、ソリダスが最後の闘いを挑むべく立ちふさがる。俺の制止の言葉ももはや奴には届かない。
*/
E_ソリダス戦中: << EOF
The universe is now being repaired. 
As I wait for my moment of return, Solidus 
stands in front of me to challenge me to the 
final battle. He no longer lends his ear to my 
words to hold him back.
EOF

}









