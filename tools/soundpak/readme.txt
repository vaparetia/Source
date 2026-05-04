soundpak

サウンドデータをパッキングする。

以下のような形式でsrcファイルを記述する。

pack w01a/sd000000.pak
{
	wvx wvx1/wv000000.wvx
	wvx wvx1/wv000001.wvx
	efx efx1/se000000.efx
	mdx mdx1/sg000010.mdx
}

上記のパラメータでパックツールを起動すると、
stage/w01a/sd000000.pak
を作成。

cd作成ツールの方で、
sd%06d.pak
を0から順に検索して、各ステージに振り分ける。
