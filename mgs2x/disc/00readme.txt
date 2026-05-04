MGS2 ディスク作成環境
2000/07/06 K.Uehara
$Id: 00readme.txt,v 1.2 2000/12/06 06:32:51 usr01475 Exp $

CD/DVDに焼くためのデータを作成する。

環境変数によって、以下の挙動が変わる。
MGS2_VERSION		: version情報をその数字にする。
MGS2_VMODE			: PAL or NTSC system.cnfの記述も変更


disc -  makefile    : image作成のためのmakefile
        image       : CD/DVDに焼くデータが作成されるディレクトリ
        demo        : デモストリーミング作成
        vox         : 音声ストリーミング作成
        stage       : ステージファイル作成

サウンドやデモのファイルが格納されているディレクトリ等、環境変数は
disc/makefile中で定義される。変更があった場合は、ここだけを変えればいい。

makefileで行なう流れは以下のようになる。

    demo, soundストリームファイルを作成。
    できたdemo.sym, vox.symでシナリオ、無線を再コンパイル。
    sourceをCDモードで再リンク。
    stageファイルを作成。
    実行ファイル、無線ファイルをコピー。
