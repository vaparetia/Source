zlib デコードライブラリ

zlibでエンコードされたデータをデコードするライブラリ。
zlibのデコード部分をほぼそのまま移植。

mallocをライブラリ内部で一切使わないようにした。

使用法は以下のようにする。

z_stream z;

z.buffer = ( inflateGetWorkSize() の大きさのバッファ。たぶん45656 byte )

/* 初期化 */
if( inflateInit( &z ) != Z_OK ){
	// Error
}

z.next_in = 圧縮データの先頭アドレス
z.avail_in = 圧縮データの有効サイズ

z.next_out = 展開バッファの先頭アドレス
z.avail_out = 0;

status = inflate( &z, Z_NO_FLUSH );

inflateEnd( &z );



