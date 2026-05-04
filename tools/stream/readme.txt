新ストリーミングファイルフォーマット

PS2では、転送するデータは16バイト境界に揃っている必要があるため、
タグ、パケット等のデータサイズを16バイトアライメントとする。

tag:
struct {
	int type;
	int size;
	int time;
	int option2;
};

type は、( ch << 16 ) | ( type );

入力するファイルは以下のようなテキストファイル。

-- ソーススクリプト
block TEST1
{
	sound sd000000.pcm
}

block TEST2
{
	sound sd000001.pcm
}

block TEST3
{
	sound sd000002.pcm
}
--

-- チャンク定義ファイル soundは１固定

sound	0x01
demo	0x02
lip		0x03

--

オフセットはテーブルファイル(lstファイル)として出力されるが、
最上位８ビットをフラグとして使う。
(したがってファイルサイズのMAXは0x00FFFFFF sector = 32G )

0x80000000		符合ビットなので、念のため使わない
0x40000000		8Bit Sound Streamを含む
0x20000000		VAG Sound Streamを含む






