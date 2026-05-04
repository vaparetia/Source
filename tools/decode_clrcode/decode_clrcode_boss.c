#include <stdio.h>
#include <stdlib.h>


#define SCN_SCODE_BITLEN	110
#define SCN_CODE_BITLEN		122
#define SCN_CODE_LEN		((SCN_CODE_BITLEN+32-1)/32)
#define SCN_CODE_LEN_S		((SCN_CODE_BITLEN+16-1)/16)
#define SCN_CODE_LEN_C		((SCN_CODE_BITLEN+8-1)/8)
#define CLR_CODE_LEN		26


void Calc26CodeToBit(unsigned char *in,unsigned int *out);
int DecodeClearCode2(unsigned int *code);
void PrintParamBossRush( unsigned int *code );

unsigned char master[CLR_CODE_LEN];

static int DecodeClearcode(unsigned char *s,unsigned int *code)
{
	int i;

	for(i=0;i<SCN_CODE_LEN;i++){
		code[i]=0;
	}

	Calc26CodeToBit(s,code);

	// printf("%08x %08x %08x %08x\n",code[0],code[1],code[2],code[3]);

	if(!DecodeClearCode2(code)){
		return 0;
	}

	return 1;
}

static void PrintEncodedClearcode(unsigned char *s)
{
	int i;
	for(i=0;i<CLR_CODE_LEN;i++){
		if(master[i]==s[i]) printf("%c",*(s+i)+'A');
		else printf("%c",*(s+i)+'a');
	}
	printf("\n");
}

static int ChangeAndCheckCodeARDO(unsigned char *s,int index,int level)
{
	unsigned int code[SCN_CODE_LEN];
	int reccode=*(s+index);
	int ans=0;

	if(index>=CLR_CODE_LEN) return 0;

	switch(reccode){
	case 'A'-'A':
		*(s+index)='R'-'A';
		if(DecodeClearcode(s,code)){
			printf("Success : ");
			PrintEncodedClearcode(s);
			PrintParamBossRush(code);
			printf("--------\n");

			ans=1;
		}
		break;
	case 'R'-'A':
		*(s+index)='A'-'A';
		if(DecodeClearcode(s,code)){
			printf("Success : ");
			PrintEncodedClearcode(s);
			PrintParamBossRush(code);
			printf("--------\n");

			ans=1;
		}
		break;

	case 'D'-'A':
		if(level<1) break;

		*(s+index)='O'-'A';
		if(DecodeClearcode(s,code)){
			printf("Success : ");
			PrintEncodedClearcode(s);
			PrintParamBossRush(code);
			printf("--------\n");

			ans=1;
		}
		break;
	case 'O'-'A':
		if(level<1) break;

		*(s+index)='D'-'A';
		if(DecodeClearcode(s,code)){
			printf("Success : ");
			PrintEncodedClearcode(s);
			PrintParamBossRush(code);
			printf("--------\n");

			ans=1;
		}
		break;
	}

	if(*(s+index)!=reccode){
		if(ChangeAndCheckCodeARDO(s,index+1,level)) ans=1;
		*(s+index)=reccode;
	}
	if(ChangeAndCheckCodeARDO(s,index+1,level)) ans=1;

	return ans;
}

int main(int argc,char *argv[])
{
	unsigned int code[SCN_CODE_LEN];
	unsigned char s[CLR_CODE_LEN];
	int i;

	if(argc!=2){
		printf("usage : decode_clrcode <clear code>\n");
		return -1;
	}

	if(strlen(argv[1])>CLR_CODE_LEN){
		printf("Input code is too long !!\n");
		return -1;
	}

	if(strlen(argv[1])<CLR_CODE_LEN){
		printf("Input code is too short !!\n");
		return -1;
	}

	for(i=0;i<CLR_CODE_LEN;i++){
		if(argv[1][i]>='a' && argv[1][i]<='z') s[i]=argv[1][i]-'a';
		else if(argv[1][i]>='A' && argv[1][i]<='Z') s[i]=argv[1][i]-'A';
		else{
			printf("Input code has a wrong letter !!\n");
			return -1;
		}
		// printf("%02x ",s[i]);
	}
	// printf("\n");

	for(i=0;i<CLR_CODE_LEN;i++){
		master[i]=s[i];
	}

	if(DecodeClearcode(s,code)){
		printf("Success : ");
		PrintEncodedClearcode(s);
		PrintParamBossRush(code);
		return 0;
	}

	printf("Code is Wrong !! : ");
	PrintEncodedClearcode(s);

	printf("\n\nLevel 0 : Try to change code , A -> R , R -> A\n");
	printf("--------\n");

	if(ChangeAndCheckCodeARDO(s,0,0)){
		return -1;
	}

	printf("\n\nLevel 1 : Try to change code , A -> R , R -> A , D -> O , O -> D\n");
	printf("--------\n");

	if(ChangeAndCheckCodeARDO(s,0,1)){
		return -1;
	}

	return -1;
}
