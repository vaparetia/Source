//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	clearcode.c
		クリアコード表示関連

	2001/08/09  K.Kano
	$Id: clearcode.c,v 1.1.1.3 2002/11/19 11:43:07 Yoshizawa1 Exp $
 */


#include "clearcode.h"


#define DOGTAG_RESOURCE		0x00be6566		/* ドッグタグデータ */
#define MAX_LEVEL			5

// ドッグタグデータ多重度
#ifdef PSX2
#define	DOGTAG_N_PERSON		2
#else
#define	DOGTAG_N_PERSON		1
#endif

typedef struct {
	char *name;
	int month;
	int day;
	char *region;
	int where;
	int level;
	int blood;
} DOGTAG_PERSON;

typedef struct {
	int id;
	int location;
	DOGTAG_PERSON	person[ DOGTAG_N_PERSON ];
} DOGTAG_RDATA;


int SIG_CheckDogTagFlag(int num);


static int GetADogtag(DOGTAG_RDATA *r)
{
	int i;

	r->id = GCL_GetNextInt();
	r->location = GCL_GetNextInt();

	for( i = 0 ; i < DOGTAG_N_PERSON ; i++ ) {
		DOGTAG_PERSON* p = &r->person[ i ];
		p->name = GCL_GetNextString();
		p->month = GCL_GetNextInt();
		p->day = GCL_GetNextInt();
		p->region = GCL_GetNextString();
		p->where = GCL_GetNextInt();
		p->level = GCL_GetNextInt();
		p->blood = GCL_GetNextInt();
	}

	if(r->id!=-1 &&
	   (r->person[0].where==-1 || r->person[0].level<0 || r->person[0].level>=MAX_LEVEL || r->person[0].blood==-1)) return 0;

	else return 1;
}

static inline int CheckFlag(int id)
{
	return SIG_CheckDogTagFlag(id);
}

int ComNTankerDogtags(void)
{
	void *res;
	DOGTAG_RDATA d;
	int cnt=0;
	int mode = (GM_Configuration2 & GM_CONFIG_DOGTAGS_2002 ) ? 1 : 0;

	res=GetLocalResource(DOGTAG_RESOURCE,0);
	ASSERT(res);

	GCL_SetArgTop(res);

	while(GetADogtag(&d)){
		// printf("cnt = %d\n",cnt);

		if(d.id==-1) continue;

		if(!d.person[ mode ].where){
			if(CheckFlag(d.id)) cnt++;
		}
	}

	// printf("NTanker = %d\n",cnt);

	return cnt;
}

int ComMaxTankerDogtags(void)
{
	void *res;
	DOGTAG_RDATA d;
	int cnt=0;
	int mode = (GM_Configuration2 & GM_CONFIG_DOGTAGS_2002 ) ? 1 : 0;

	res=GetLocalResource(DOGTAG_RESOURCE,0);
	ASSERT(res);

	GCL_SetArgTop(res);

	while(GetADogtag(&d)){
		// printf("cnt = %d\n",cnt);

		if(d.id==-1) continue;

		if(!d.person[ mode ].where) cnt++;
	}

	return cnt;
}

int ComNPlantDogtags(void)
{
	void *res;
	DOGTAG_RDATA d;
	int cnt=0;
	int mode = (GM_Configuration2 & GM_CONFIG_DOGTAGS_2002 ) ? 1 : 0;

	res=GetLocalResource(DOGTAG_RESOURCE,0);
	ASSERT(res);

	GCL_SetArgTop(res);

	while(GetADogtag(&d)){
		// printf("cnt = %d\n",cnt);

		if(d.id==-1) continue;

		if(d.person[ mode ].where){
			if(CheckFlag(d.id)) cnt++;
		}
	}

	// printf("NPlant = %d\n",cnt);

	return cnt;
}

int ComMaxPlantDogtags(void)
{
	void *res;
	DOGTAG_RDATA d;
	int cnt=0;
	int mode = (GM_Configuration2 & GM_CONFIG_DOGTAGS_2002 ) ? 1 : 0;

	res=GetLocalResource(DOGTAG_RESOURCE,0);
	ASSERT(res);

	GCL_SetArgTop(res);

	while(GetADogtag(&d)){
		// printf("cnt = %d\n",cnt);

		if(d.id==-1) continue;

		if(d.person[ mode ].where) cnt++;
	}

	return cnt;
}


/* ----------------------------------------------------------- */


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


static unsigned int rand_seed;

static unsigned int clr_rand( void )
{
	rand_seed = 2100005341U * rand_seed + 3321;
	return rand_seed;
}


#ifdef DEBUG_MODE

static void PrintParam(unsigned int *code)
{
	printf("0x%08x 0x%08x 0x%08x 0x%08x\n",code[0],code[1],code[2],code[3]);

	printf("レーダータイプ : ");
	switch(code[0] & 3){
	case 0: printf("OFF\n"); break;
	case 1: printf("TYPE 1\n"); break;
	case 2: printf("TYPE 2\n"); break;
	default: printf("Unknown\n"); break;
	}

	{
		int hour,min,sec;
		sec=(code[0]>>2) & 0x7fff;
		min=sec/4;
		sec=(sec%4)*15;
		hour=min/60;
		min%=60;
		printf("プレイ時間 : %d:%d:%d\n",hour,min,sec);
	}

	printf("発砲数 : %d\n",(code[0]>>17) & 0x3ff);

	printf("ダメージ率 : %f\n",(float)((code[0]>>27) & 0x1f)*0.5f);

	printf("危険モード回数 : %d\n",(code[1]>>0) & 0xff);

	printf("敵兵を殺した数 : %d\n",(code[1]>>8) & 0xff);

	printf("クリアリング回数 : %d\n",(code[1]>>16) & 0xff);

	printf("赤びっくり回数 : %d\n",(code[1]>>24) & 0xff);

	printf("コンティニュー回数 : %d\n",(code[2]>>0) & 0x3f);

	printf("レーション使用回数 : %d\n",(code[2]>>6) & 0x1f);

	printf("セーブ回数 : %d\n",(code[2]>>11) & 0x7f);

	printf("カメラ・ガンカメラ・サイファー破壊数 : %d\n",(code[2]>>18) & 0xff);

	printf("ポスターを見た回数 : %d\n",(code[2]>>26) & 0x1f);

	printf("特殊アイテム使用 : ");
	if(code[2] & 0x80000000) printf("使用した\n");
	else printf("使用していない\n");

	printf("バージョン : %d\n",(code[3]>>0) & 3);

	printf("ゲームレベル : ");
	switch((code[3]>>2) & 7){
	case 0: printf("VERY EASY\n"); break;
	case 1: printf("EASY\n"); break;
	case 2: printf("NORMAL\n"); break;
	case 3: printf("HARD\n"); break;
	case 4: printf("EXTREME\n"); break;
	default: printf("Unknown\n"); break;
	}

	printf("タンカー編 or プラント編 : %d\n",(code[3]>>5) & 3);

	printf("ドッグタグ回収率 : %d\n",(code[3]>>7) & 0x7f);
}

#endif


/* クリアコードの暗号化 */
static void _EncodeClearCode(unsigned int *code)
{
	unsigned int outcode[SCN_CODE_LEN];
	unsigned char s[SCN_CODE_LEN_C];
	int i;
	int crc_code;
	int rnd_code;

#ifdef DEBUG_MODE
	PrintParam(code);
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
	rnd_code=(irnd()>>16) & 0x1f;

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

#ifdef DEBUG_MODE
	printf("ClearCode = ");

	for(i=0;i<CLR_CODE_LEN;i++){
		printf("%c",out[i]+'A');
	}

	printf("\n");
#endif

}
