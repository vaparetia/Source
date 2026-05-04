#include <stdio.h>
#include <stdlib.h>


#define SCN_SCODE_BITLEN	110
#define SCN_CODE_BITLEN		122
#define SCN_CODE_LEN		((SCN_CODE_BITLEN+32-1)/32)
#define SCN_CODE_LEN_S		((SCN_CODE_BITLEN+16-1)/16)
#define SCN_CODE_LEN_C		((SCN_CODE_BITLEN+8-1)/8)
#define CLR_CODE_LEN		26


/* 7 bit CRC 計算
   110bitのデータであるので、7bitのCRC符号があれば充分である。
   計算は、1+x^1+x^2+x^5+x^7を使った。
   既約多項式ではあるが、原始多項式かどうか疑問？ */

#define CRCPOLY		0x74

static unsigned int calc_crc_code( void *data, int len )
{
	int i, j;
	unsigned int r;
	char *p;

	r = 0;
	p = data;

	for( i = len; i > 0; i-=8 ){
		r ^= *( p ++ );
		for( j = (i>=8 ? 8 : i) ; j > 0; j-- ){
			if( r & 1 ){
				r = ( r >> 1 ) ^ CRCPOLY;
			} else {
				r >>= 1;
			}
		}
	}
	return r & 0x7f;
}


#define CRC_INDEX		(SCN_SCODE_BITLEN/32)
#define CRC_SHIFT		(SCN_SCODE_BITLEN & 31)
#define CRC_LEN			7

#define RND_INDEX		((SCN_SCODE_BITLEN+CRC_LEN)/32)
#define RND_SHIFT		((SCN_SCODE_BITLEN+CRC_LEN) & 31)
#define RND_LEN			5

#define XOR_ENCODE0		0x8634f7ad
#define XOR_ENCODE1		0x19e5cb02
#define XOR_ENCODE2		0x7ed10f24
#define XOR_ENCODE3		0x9a36b8c5

#define XOR_ENCODE4		0xb8c59a36
#define XOR_ENCODE5		0x0f247ed1
#define XOR_ENCODE6		0xcb0219e5
#define XOR_ENCODE7		0xf7ad8634


static const unsigned char decode_index[]={
	0,1,119,57,72,107,30,90,
	112,88,71,94,92,89,38,69,
	22,97,24,76,46,55,98,73,
	44,8,25,23,116,78,108,20,
	6,113,111,79,15,106,102,19,
	18,39,21,114,104,51,91,35,
	117,58,54,29,93,17,99,115,
	52,56,50,67,34,74,13,3,
	11,33,10,109,45,7,95,63,
	27,41,68,118,66,37,60,32,
	12,82,47,121,31,103,101,80,
	36,4,87,59,120,48,105,62,
	70,43,86,83,84,61,64,96,
	75,49,2,81,28,9,14,100,
	110,53,16,77,40,65,5,85,
	26,42,
};


static unsigned int rand_seed;

static unsigned int clr_rand( void )
{
	rand_seed = 2100005341U * rand_seed + 3321;
	return rand_seed;
}

static void MoveBit(unsigned int *outcode,unsigned int *incode,int b0,int b1)
{
	int index0=b0>>5;
	int shift0=b0&31;
	int index1=b1>>5;
	int shift1=b1&31;

	int bit0;

	bit0=(*(incode+index0)>>shift0) & 1;
	*(outcode+index1)|=(bit0<<shift1);
}


static const unsigned char encode_index[]={
	0,1,106,63,89,118,32,69,
	25,109,66,64,80,62,110,36,
	114,53,40,39,31,42,16,27,
	18,26,120,72,108,51,6,84,
	79,65,60,47,88,77,14,41,
	116,73,121,97,24,68,20,82,
	93,105,58,45,56,113,50,21,
	57,3,49,91,78,101,95,71,
	102,117,76,59,74,15,96,10,
	4,23,61,104,19,115,29,35,
	87,107,81,99,100,119,98,90,
	9,13,7,46,12,52,11,70,
	103,17,22,54,111,86,38,85,
	44,94,37,5,30,67,112,34,
	8,33,43,55,28,48,75,2,
	92,83,
};

void PrintParam(unsigned int *code)
{
	int val;
	int country;

	printf("0x%08x 0x%08x 0x%08x 0x%08x\n",code[0],code[1],code[2],code[3]);

	country=(code[3]>>0) & 3;

	printf("レーダータイプ : ");
	switch(code[0] & 3){
	case 0: printf("OFF\n"); break;
	case 1: printf("TYPE 2\n"); break;
	case 2: printf("TYPE 1\n"); break;
	default: printf("Unknown\n"); break;
	}

	{
		int hour,min,sec;
		val=(code[0]>>2) & 0x7fff;
		min=val/4;
		sec=(val%4)*15;
		hour=min/60;
		min%=60;
		if(val==0x7fff) printf("プレイ時間(15秒単位) : %d:%d:%d 以上\n",hour,min,sec);
		else printf("プレイ時間(15秒単位) : %d:%d:%d\n",hour,min,sec);
	}

	val=(code[0]>>17) & 0x3ff;
	if(val==0x3ff) printf("発砲数 : %d 以上\n",val);
	else printf("発砲数 : %d\n",val);

	val=(code[0]>>27) & 0x1f;
	if(val==0x1f) printf("ダメージ率 : ゲージ %f 本分以上\n",(float)val*0.5f);
	else printf("ダメージ率 : ゲージ %f 本文\n",(float)val*0.5f);

	val=(code[1]>>0) & 0xff;
	if(val==0xff) printf("危険モード回数 : %d 以上\n",val);
	else printf("危険モード回数 : %d\n",val);

	val=(code[1]>>8) & 0xff;
	if(val==0xff) printf("敵兵を殺した数 : %d 以上\n",val);
	else printf("敵兵を殺した数 : %d\n",val);

	val=(code[1]>>16) & 0xff;
	if(val==0xff) printf("クリアリング回数 : %d 以上\n",val);
	else printf("クリアリング回数 : %d\n",val);

	if(country==0){
		/* USA */
		val=(code[1]>>24) & 0xff;
		if(val==0xff) printf("赤びっくり回数 : %d 以上\n",val);
		else printf("赤びっくり回数 : %d\n",val);
	}

	val=(code[2]>>0) & 0x3f;
	if(val==0x3f) printf("コンティニュー回数 : %d 以上\n",val);
	else printf("コンティニュー回数 : %d\n",val);

	val=(code[2]>>6) & 0x1f;
	if(val==0x1f) printf("レーション使用回数 : %d 以上\n",val);
	else printf("レーション使用回数 : %d\n",val);

	val=(code[2]>>11) & 0x7f;
	if(val==0x7f) printf("セーブ回数 : %d 以上\n",val);
	else printf("セーブ回数 : %d\n",val);

	val=(code[2]>>18) & 0xff;
	if(val==0xff) printf("カメラ・ガンカメラ・サイファー破壊数 : %d 以上\n",val);
	else printf("カメラ・ガンカメラ・サイファー破壊数 : %d\n",val);

	if(country==0){
		/* USA */
		val=(code[2]>>26) & 0x1f;
		if(val==0x1f) printf("ポスターを見た回数 : %d 以上\n",val);
		else printf("ポスターを見た回数 : %d\n",val);

		printf("特殊アイテム使用 : ");
		if(code[2] & 0x80000000) printf("使用した\n");
		else printf("使用していない\n");
	}

	if(country!=0){
		/* JAPAN */
		val=(code[1]>>24) & 0x3f;
		if(val==0x3f) printf("タンカー編クリア回数 : %d 以上\n",val);
		else printf("タンカー編クリア回数 : %d\n",val);

		val=(code[2]>>26) & 0x3f;
		if(val==0x3f) printf("プラント編クリア回数 : %d 以上\n",val);
		else printf("プラント編クリア回数 : %d\n",val);

		printf("舟虫がくっついた : ");
		if(code[1] & 0x40000000) printf("はい\n");
		else printf("いいえ\n");

		printf("特殊アイテム使用 : ");
		if(code[1] & 0x80000000) printf("使用した\n");
		else printf("使用していない\n");
	}

	printf("バージョン : ");
	switch(country){
	case 0:
		printf("USA\n");
		break;
	case 1:
		printf("JAPAN\n");
		break;
	case 2:
		printf("EUROPE\n");
		break;
	case 3:
		printf("Unknown\n");
		break;
	}

	printf("ゲームレベル : ");
	val=(code[3]>>2) & 7;

	if(country==0){
		/* USA */
		switch(val){
		case 0: printf("VERY EASY\n"); break;
		case 1: printf("EASY\n"); break;
		case 2: printf("NORMAL\n"); break;
		case 3: printf("HARD\n"); break;
		case 4: printf("EXTREME\n"); break;
		default: printf("Unknown\n"); break;
		}
	}
	else{
		/* JAPAN */
		switch(val){
		case 0: printf("EASY\n"); break;
		case 1: printf("NORMAL\n"); break;
		case 2: printf("HARD\n"); break;
		case 3: printf("VERY HARD\n"); break;
		case 4: printf("EXTREME\n"); break;
		default: printf("Unknown\n"); break;
		}
	}

	val=(code[3]>>5) & 3;
	printf("タンカー編 or プラント編 : ");
	switch(val){
	case 0:
		/* Tanker & Plant */
		printf("Tanker & Plant\n");
		break;
	case 1:
		/* Tanker */
		printf("Tanker\n");
		break;
	case 2:
		/* Plant */
		printf("Plant\n");
		break;
	case 3:
		/* Unknown */
		printf("Unkown\n");
		break;
	}

	val=(code[3]>>7) & 0x7f;
	printf("ドッグタグ回収率 : %d\n",val);
}

#ifdef PAL
	#define TICK_TIME (50.0f)
#else
	#define TICK_TIME (60.0f)
#endif	

static void TimeDisp( char *str , unsigned int time )
{
	int min , sec , msec;

	time = ( ( time * 100.0f ) / TICK_TIME );
	min = time / 60 / 100;
	sec = ( time - ( min * 60 * 100 ) ) / 100;
	msec = ( time - ( min * 60 * 100 ) - ( sec * 100 ) );
#if 0
	sec = time;
	min = sec / 60;
	sec = ( sec % 60 );
	hour= min / 60;
	min %= 60;
#endif
	printf( "%s : %d:%d:%d\n",str , min , sec , msec );
}

void PrintParamBossRush( unsigned int *code )
{
	int code_tmp;
	int tmp;

	printf("0x%08x 0x%08x 0x%08x 0x%08x\n",code[0],code[1],code[2],code[3]);

	code_tmp = code[ 0 ] & 0x1ffff;
	TimeDisp( "オルガ      \0" , code_tmp );
	//
	code_tmp = ( ( code[ 1 ] & 0x3 ) << 15 ) | ( ( code[ 0 ] >> 17 ) & 0x7fff );
	TimeDisp( "ファットマン\0" , code_tmp );
	//
	code_tmp = ( code[ 1 ] >> 2 ) & 0x1ffff;
	TimeDisp( "ハリアー    \0" , code_tmp );
	//
	code_tmp = ( ( code[ 2 ] & 0xf ) << 13 ) | ( ( code[ 1 ] >> 19 ) & 0x1fff );
	TimeDisp( "ヴァンプ    \0" , code_tmp );
	//
	code_tmp = ( code[ 2 ] >> 4 ) & 0x1ffff;
	TimeDisp( "RAY         \0" , code_tmp );
	//
	code_tmp = ( ( code[ 3 ] & 0x3f ) << 11 ) | ( ( code[ 2 ] >> 21 ) & 0x7ff );
	TimeDisp( "ソリダス    \0" , code_tmp );

//	code_tmp = ( code[ 3 ] >> 12 ) & 0x1;
//	printf("オルガ       気絶 or 殺した : %s\n" , ( ( code[ 3 ] >> 12 ) & 0x1 ) == 1 ? "気絶\0" : "殺した\0" );
	printf("ファットマン 気絶 or 殺した : %s\n" , ( ( code[ 3 ] >> 6 ) & 0x1 ) == 1 ? "気絶\0" : "殺した\0" );
//	printf("ハリアー     気絶 or 殺した : %s\n" , ( ( code[ 3 ] >> 14 ) & 0x1 ) == 1 ? "気絶\0" : "殺した\0" );
	printf("ヴァンプ     気絶 or 殺した : %s\n" , ( ( code[ 3 ] >> 7 ) & 0x1 ) == 1 ? "気絶\0" : "殺した\0" );
//	printf("RAY          気絶 or 殺した : %s\n" , ( ( code[ 3 ] >> 16 ) & 0x1 ) == 1 ? "気絶\0" : "殺した\0" );
	printf("ソリダス     気絶 or 殺した : %s\n" , ( ( code[ 3 ] >> 8 ) & 0x1 ) == 1 ? "気絶\0" : "殺した\0" );

	printf("Version %x\n" , ( code[ 3 ] >> 9 ) & 0x3 );
	printf("難易度 ");
	tmp = ( code[ 3 ] >> 11 ) & 0x3;
#if 0
	switch ( tmp ){
	case 0 : // Very Easy
		printf("VERY_EASY\n");
		break;
	case 1 : // Easy
		printf("EASY\n");
		break;
	case 2 : // Normal
		printf("Normal\n");
		break;
	case 3 : // Hard
		printf("Hard\n");
		break;
	case 4 : // Very Hard
		printf("VERY_Hard\n");
		break;
	case 5 : // Extreme
		printf("Extreme\n");
		break;
	}
#endif
	switch ( tmp ){
	case 1 : // Very Easy
		printf("VERY_EASY\n");
		break;
	case 2 : // Easy
		printf("EASY\n");
		break;
	case 3 : // Hard
		printf("Hard\n");
		break;
	}
	printf("Player %s\n" , ( ( code[ 3 ] >> 13 ) & 0x1 ) == 0 ? "スネーク" : "ライデン" );
}

/* クリアコードの暗号化 */
static void _EncodeClearCode(unsigned int *code)
{
	unsigned int outcode[SCN_CODE_LEN];
	unsigned char s[SCN_CODE_LEN_C];
	int i;
	int crc_code;
	int rnd_code;

#if 0

#ifdef DEBUG_MODE
	PrintParam(code);
//	PrintParamBossRush( code );
#endif

#endif

	/* コード部のみ抽出 */
	*(code+CRC_INDEX)&=~(0xffffffff<<CRC_SHIFT);

	/* 無いとは思うが念のためリトルエンディアン形式で
	   データをchar型に展開 */
	for(i=0;i<SCN_CODE_LEN_C;i++){
		int index=i>>2;
		int shift=(i & 3)*8;
		s[i]=(*(code+index)>>shift) & 0xff;
	}

	crc_code=calc_crc_code(s,SCN_SCODE_BITLEN);

	/* リトルエンディアン形式でデータをint型に展開 */
	for(i=0;i<SCN_CODE_LEN_C;i++){
		int index=i>>2;
		int shift=(i & 3)*8;
		*(code+index)&=~(0xff<<shift);
		*(code+index)|=((unsigned int)(s[i]))<<shift;
	}

	/* CRC組み込み */
	*(code+CRC_INDEX)&=~(0x7f<<CRC_SHIFT);
	*(code+CRC_INDEX)|=crc_code<<CRC_SHIFT;

	// printf("%08x %08x %08x %08x\n",code[0],code[1],code[2],code[3]);

	/* ランダム化 */
	rnd_code=(rand()>>8) & 0x1f;

	// printf("rnd_code = %d\n",rnd_code);

	rand_seed=(unsigned int)(1<<rnd_code)-1;
	for(i=0;i<SCN_CODE_LEN;i++){
		unsigned int mask=clr_rand();
		*(code+i) ^= mask;
	}

	/* RND組み込み */
	*(code+RND_INDEX)&=~(0x1f<<RND_SHIFT);
	*(code+RND_INDEX)|=rnd_code<<RND_SHIFT;

	/* 暗号化 */
	for(i=0;i<SCN_CODE_LEN;i++){
		outcode[i]=0;
	}
	for(i=0;i<SCN_CODE_BITLEN;i++){
		MoveBit(outcode,code,i,encode_index[i]);
	}
	for(i=0;i<SCN_CODE_LEN;i++){
		code[i]=outcode[i];
	}
}

void EncodeClearCode(unsigned int *code)
{
	_EncodeClearCode(code);

	*(code+0)^=XOR_ENCODE0;
	*(code+1)^=XOR_ENCODE1;
	*(code+2)^=XOR_ENCODE2;
	*(code+3)^=XOR_ENCODE3;

	/* コード部のみ抽出 */
	*(code+RND_INDEX)&=~(0xffffffff<<(RND_SHIFT+RND_LEN));
}

void EncodeClearCode2(unsigned int *code)
{
	_EncodeClearCode(code);

	*(code+0)^=XOR_ENCODE4;
	*(code+1)^=XOR_ENCODE5;
	*(code+2)^=XOR_ENCODE6;
	*(code+3)^=XOR_ENCODE7;

	/* コード部のみ抽出 */
	*(code+RND_INDEX)&=~(0xffffffff<<(RND_SHIFT+RND_LEN));
}

void CalcBitTo26Code(unsigned int *in,unsigned char *out)
{
	/* 26進数に変換 */
	unsigned short s[SCN_CODE_LEN_S];
	int i,j;
	unsigned int t;

	for(i=0;i<SCN_CODE_LEN_S;i++){
		int index=i>>1;
		if(i & 1) s[i]=in[index]>>16;
		else s[i]=in[index] & 0x0000ffff;
	}

	// printf("%08x %08x %08x %08x %08x %08x %08x %08x\n",
	//	   s[0],s[1],s[2],s[3],s[4],s[5],s[6],s[7]);

	for(j=0;j<CLR_CODE_LEN;j++){
		t=0;
		for(i=SCN_CODE_LEN_S-1;i>=0;i--){
			t=(t<<16) | s[i];
			s[i]=t/26;
			t%=26;
		}
		out[j]=t;
	}

#if 0

#ifdef DEBUG_MODE
	printf("ClearCode = ");

	for(i=0;i<CLR_CODE_LEN;i++){
		printf("%c",out[i]+'A');
	}

	printf("\n");
#endif

#endif

}

static int _DecodeClearCode(unsigned int *code)
{
	unsigned int outcode[SCN_CODE_LEN];
	unsigned char s[SCN_CODE_LEN_C];
	int i;
	int crc,rcrc;
	int rnd_code;

	/* 復号化 */
	for(i=0;i<SCN_CODE_LEN;i++){
		outcode[i]=0;
	}
	for(i=0;i<SCN_CODE_BITLEN;i++){
		MoveBit(outcode,code,i,decode_index[i]);
	}
	for(i=0;i<SCN_CODE_LEN;i++){
		code[i]=outcode[i];
	}

	/* ランダムコード読込み */
	rnd_code=(*(code+RND_INDEX)>>RND_SHIFT) & 0x1f;

	// printf("rnd_code = %d\n",rnd_code);

	rand_seed=(unsigned int)(1<<rnd_code)-1;
	for(i=0;i<SCN_CODE_LEN;i++){
		unsigned int mask=clr_rand();
		*(code+i) ^= mask;
	}

	// printf("%08x %08x %08x %08x\n",code[0],code[1],code[2],code[3]);

	/* CRC読込み */
	rcrc=(*(code+CRC_INDEX)>>CRC_SHIFT) & 0x7f;

	/* コード部のみ抽出 */
	*(code+CRC_INDEX)&=~(0xffffffff<<CRC_SHIFT);

	/* 無いとは思うが念のためリトルエンディアン形式で
	   データをchar型に展開 */
	for(i=0;i<SCN_CODE_LEN_C;i++){
		int index=i>>2;
		int shift=(i & 3)*8;
		s[i]=(*(code+index)>>shift) & 0xff;
	}

	crc=calc_crc_code(s,SCN_SCODE_BITLEN);

	/* リトルエンディアン形式でデータをint型に展開 */
	for(i=0;i<SCN_CODE_LEN_C;i++){
		int index=i>>2;
		int shift=(i & 3)*8;
		*(code+index)&=~(0xff<<shift);
		*(code+index)|=((unsigned int)(s[i]))<<shift;
	}

	// printf("crc = 0x%02x 0x%02x\n",crc,rcrc);

	if(crc==rcrc) return 1;
	else return 0;
}

int DecodeClearCode(unsigned int *code)
{
	*(code+0)^=XOR_ENCODE0;
	*(code+1)^=XOR_ENCODE1;
	*(code+2)^=XOR_ENCODE2;
	*(code+3)^=XOR_ENCODE3;

	return _DecodeClearCode(code);
}

int DecodeClearCode2(unsigned int *code)
{
	*(code+0)^=XOR_ENCODE4;
	*(code+1)^=XOR_ENCODE5;
	*(code+2)^=XOR_ENCODE6;
	*(code+3)^=XOR_ENCODE7;

	return _DecodeClearCode(code);
}

void Calc26CodeToBit(unsigned char *in,unsigned int *out)
{
	/* 26進数から変換 */
	unsigned int s[SCN_CODE_LEN_S];
	int i,j;
	unsigned int t;

	for(i=0;i<SCN_CODE_LEN_S;i++){
		s[i]=0;
	}

	for(j=CLR_CODE_LEN-1;j>=0;j--){
		t=in[j];
		for(i=0;i<SCN_CODE_LEN_S;i++){
			s[i]=s[i]*26+t;
			t=s[i]>>16;
			s[i]&=0x0000ffff;
		}
	}

	// printf("%08x %08x %08x %08x %08x %08x %08x %08x\n",
	//		  s[0],s[1],s[2],s[3],s[4],s[5],s[6],s[7]);

	for(i=0;i<SCN_CODE_LEN_S;i++){
		int index=i>>1;

		if(i & 1){
			*(out+index)&=0x0000ffff;
			*(out+index)|=s[i]<<16;
		}
		else{
			*(out+index)&=0xffff0000;
			*(out+index)|=s[i];
		}
	}
}
