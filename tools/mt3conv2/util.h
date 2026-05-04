/* その数字の符号を返す */
int GetSign( int a );
/* 有効ビット数を返す */
int GetEffectiveBit( int data );
/* 指定された数のshort型データをリトルエンディアンに揃える */
void ChangeLittleEndianShort( unsigned short *src, int num );
/* 指定された数のlong型データをリトルエンディアンに揃える */
void ChangeLittleEndianLong( unsigned long *src, int num );
/* 指定された数のリトルエンディアンデータをshort型に揃える */
void RecoverLittleEndianShort( unsigned short *dst, int num );
/* 指定された数のリトルエンディアンデータをlong型に揃える */
void RecoverLittleEndianLong( unsigned long *dst, int num );
