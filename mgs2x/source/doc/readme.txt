メタルギアソリッド２ for PlayStation2
    1999/04/15 K.Uehara
    $Id: readme.txt,v 1.1.1.3 2002/11/19 11:41:44 Yoshizawa1 Exp $


■連絡用掲示板

プログラマの連絡掲示板は、
http://london.konami
からリンクされている。必ずそこをチェックすること。

■基本方針

PSでのシステムをほぼそのまま継承。
ただし、システム関数で、その機能が変更されるものは
意図的に名前を変える。

Windowsからアクセスするプログラマも多いと考えられるので、
ソースファイル名は小文字、できるだけ8+3で作成。
タブ幅はシステムは４で作成する。

ソースファイル中の文字コードはEUCのみ、改行コードはLFのみとする。
それ以外の文字コード、改行コードのファイルは、COMMIT時に拒否される。
(1999/11/24から)

■関数命名規則

(ライブラリ識別子)_(関数名)
ライブラリ識別子はシステムは大文字2文字。関数名は大文字小文字まじり。

■ソース構成

ソース、シナリオは以下の構成にする。
ディレクトリ名をmakefileから参照している場合があるので、
ディレクトリ名を変更するのは禁止する。

mgs2---cdrom.img
     |-source
     |-module
     |-scn
     |-list
     |-disc

list にはステージダウンロード用のスクリプトをおく。
cdrom.img以外のディレクトリはCVSによって管理する( ->cvs4mgs.txt 参照 )

sourceはトップに全体のmakefile, システム用obj, 実行ファイルを置く。
ソースは各ディレクトリに分割して作成し、巡回してmakeする。
各ディレクトリには、make logで_ChangeLogファイルが自動生成される。
これは、cvs2cl.plを使用してcvsのログから作成したもの。

discは、CD/DVDに焼くファイルを作成するための環境。

各ディレクトリにreadme.txtをおき、その中の関数について記述する

module は、IOP側のプログラムや、それに対するEE側の
インターフェースライブラリ、開発支援ライブラリを含むディレクトリ。
ここがmakeされていないと、sourceでのmakeが通らないことがあるので、
注意すること。

特殊なディレクトリは以下の通り。

main/       bootup routine
game/       ゲーム進行管理ルーチンとユーティリティ
system/     システムライブラリ
user/       ユーザー作成プログラム
doc/        各種ドキュメント
lib/        各ライブラリ
include/    共通インクルードファイル
stage/      各ステージのキャラクタ登録用(scnで自動生成)

トップディレクトリのファイルは以下の通り
makefile        makefile本体
makefile.def    INCLUDE, OPTIONS等定義ファイル

crt0.o          ソニー製ブートルーチン

mgs2.elf        常駐部実行ファイル
mgs2.map

今回は、あるディレクトリでmakeをかけると、その下のすべての
ディレクトリをコンパイルした後、リンクがかかる。
「make ディレクトリ名」 で、そのディレクトリのコンパイルだけ。
「make ディレクトリ名 link」 でそのディレクトリだけコンパイルした後リンク。

make の後に書いて使うコマンドは以下の通り。
一部をのぞき、どのディレクトリからも使える。

run   : 実行。
go    : run と同じ。
link  : リンクする。
usb   : usbファイルシステムを使用する。
clean : そこから下のディレクトリを巡回してすべてのオブジェクトを消す。
lib   : そのディレクトリ以下をコンパイルのみ行う。
needfull: フルコンパイル予約（下参照）
debug : デバッガ起動(backtrace)
module: source/でやると、moduleの下をコンパイルして、sourceをリンク。
source: scn/でやると、source, moduleをアップデートしてコンパイル。
allnew: mgs2/をすべてupdateして、フルコンパイル。
log   : _ChangeLogを各ディレクトリに生成する。

scn の下で、make stageをやると、stage/*.cを自動生成する。
ソースのclean直後でstageの下に.cが一つもない場合、自動的に
../scn/でmake stageをかけるようになっている。
その結果、１つも.cができないと、makeが無限ループに陥るので注意。

makefileは、トップ用と、中間ディレクトリ用と、ソースディレクトリ用の3種類。
新規にディレクトリを作るときは、その周りの同じようなところからコピーすること。

scnの下でも、link, run を使用することが可能。

■ defines について。

現在、以下のdefinesが定義されている。
makefile.def参照。

PSX2                マシン依存オプション。
NTSC                NTSC/PAL。
DEBUG               DEBUG用LOG構造など。
DEBUG_MODE          DEBUG_MODE用。
JAPANESE            言語オプション。JAPANESE/ENGLISH/...
HEAP_SIZE           printf用HEAP領域のサイズ。
EE                  EE用プログラム
MEDIA_CD            メディアがCDの時指定
MEDIA_DVD           メディアがDVDの時指定

DESIGN_PREVIEW      デザイナー用プレビュー環境の時SET。

■ フルコンパイル予約について

make needfullで、source/.need_fullを作成する。これをCVSでCOMMITすることで、
それ以降ダウンロードした人にmake cleanをしなければコンパイル
できなくすることが可能。

具体的には、CVSでCOMMITされないsource/.done_fullというファイルがあり、
この内容とsource/.need_fullとが異なった場合、make cleanで
cp .need_full .done_fullが実行されない限り、エラーメッセージをだし続ける
というしくみ。

■ ライブラリについて

コンパイルしたファイルはディレクトリ毎にlib/*.a という形でおかれる。
lib/*.aには、登録はされるが、自動的に削除はされないので、
ファイルを消去した場合は、lib/*.aの中にそのファイルの古いオブジェクトが
残ったままになる。ファイルをlib/*.aをまたがる形で移動した場合は
最悪、新しく作ったオブジェクトがリンクされず、古いオブジェクトがリンク
されっぱなしになることがある。
この場合は、lib/*.aを消去して、makeすれば、解決する。
とりあえずファイルをuser/username/*から移動した場合は、
make clean (そしてフルコンパイル予約)した方が無難。

■ R5900 tips

int は 32bit, longは64bit。long longは64bit, pointerは32bit。
long 同士の掛け算は重い。
32bit-float。
レジスタ渡しされる引数は8個(r4-r11),floatの引数も8個(f12-f19)。
関数呼び出しで破壊されるレジスタは r1-r15,r24,r25。
関数呼び出しで保存されるレジスタは r16-r23, r30。

■ program tips

floatを返す関数は必ずプロトタイプ宣言すること。
暗黙に使用するとintをみなされ、確実に変な値を見てしまう。

短い期間に何回も呼び出されるある程度以上の大きさの関数は、
インライン展開するより、通常呼出しする方がI-CACHEミスがおこりにくいので
速いかも。

sin(), cos()などは、doubleを返すので非常に遅くなる。
sinf(), cosf()などを使用すること。

■ その他

ディレクトリのネストが深くなったので、grepが使いにくくなりました。
findと組み合わせて、再帰的に行うgrepを考えてみたので、良かったら、
.cshrc に、

alias rgrep 'find . -regex ".*[ch]" -type f -exec grep -n \!* {} /dev/null \;'

を追加してみてください。

rgrep NewChara
とかやると、そのディレクトリ以下の.c,.hを検索します。

また、makeでディレクトリ名が表示されるのがうっとおしい人は

setenv MAKEFLAGS --no-print-directory

を入れると、かなりましになります。
