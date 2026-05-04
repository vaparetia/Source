#include <stdio.h>
#include <stdlib.h>


#define SCN_SCODE_BITLEN	110
#define SCN_CODE_BITLEN		122
#define SCN_CODE_LEN		((SCN_CODE_BITLEN+32-1)/32)
#define SCN_CODE_LEN_S		((SCN_CODE_BITLEN+16-1)/16)
#define SCN_CODE_LEN_C		((SCN_CODE_BITLEN+8-1)/8)
#define CLR_CODE_LEN		26


void Calc26CodeToBit(unsigned char *in,unsigned int *out);
int DecodeClearCode(unsigned int *code);
void PrintParam(unsigned int *code);

void CalcBitTo26Code(unsigned int *in,unsigned char *out);
void EncodeClearCode(unsigned int *code);


#define CHECK_COUNT		100


int main(int argc,char *argv[])
{
	unsigned char s[CLR_CODE_LEN];
	unsigned int code[SCN_CODE_LEN];
	int i,j,k,l;
	int ii,jj,kk,ll;

	for(i=0;i<SCN_CODE_LEN;i++){
		code[i]=0;
	}
	if(argc>1){
		for(i=0;i<atoi(argv[1]);i++){
			j=rand();
		}
	}

#if 0
	for(i=0;i<0x4000;i++){
		j=0;
		do{
			k=0;
			do{
				l=0;
				do{
					code[0]=l;
					code[1]=k;
					code[2]=j;
					code[3]=i;

					printf("%08x %08x %08x %08x\n",code[0],code[1],code[2],code[3]);

					EncodeClearCode(code);

					CalcBitTo26Code(code,s);

#if 1
					{
						int _cnt;

						printf("ClearCode = ");

						for(_cnt=0;_cnt<CLR_CODE_LEN;_cnt++){
							printf("%c",s[_cnt]+'A');
						}
					}

					printf("\n");
#endif

					Calc26CodeToBit(s,code);

					if(!DecodeClearCode(code)){
						printf("Code is Wrong !!\n");
						printf("%08x %08x %08x %08x\n",i,j,k,l);
						return -1;
					}

					if(code[0]!=l || code[1]!=k || code[2]!=j || code[3]!=i){
						printf("Code is Wrong !!\n");
						printf("%08x %08x %08x %08x -> %08x %08x %08x %08x\n",
							   i,j,k,l,code[0],code[1],code[2],code[3]);
						return -1;
					}

					l++;
				}while(l!=0);

				k++;
			}while(k!=0);

			j++;
		}while(j!=0);
	}
#else
	for(i=0;i<CHECK_COUNT;i++){
		ii=rand() & 0x3fff;

		for(j=0;j<CHECK_COUNT;j++){
			jj=rand();

			for(k=0;k<CHECK_COUNT;k++){
				kk=rand();

				for(l=0;l<CHECK_COUNT;l++){
					ll=rand();

					code[0]=ll;
					code[1]=kk;
					code[2]=jj;
					code[3]=ii;

					printf("%08x %08x %08x %08x\n",code[0],code[1],code[2],code[3]);

					EncodeClearCode(code);

					CalcBitTo26Code(code,s);

#if 1
					{
						int _cnt;

						printf("ClearCode = ");

						for(_cnt=0;_cnt<CLR_CODE_LEN;_cnt++){
							printf("%c",s[_cnt]+'A');
						}
					}

					printf("\n");
#endif

					Calc26CodeToBit(s,code);

					if(!DecodeClearCode(code)){
						printf("Code is Wrong !!\n");
						printf("%08x %08x %08x %08x\n",l,k,j,i);
						return -1;
					}

					if(code[0]!=ll || code[1]!=kk || code[2]!=jj || code[3]!=ii){
						printf("Code is Wrong !!\n");
						printf("%08x %08x %08x %08x -> %08x %08x %08x %08x\n",
							   ll,kk,jj,ii,code[0],code[1],code[2],code[3]);
						return -1;
					}
				}
			}
		}
	}
#endif

	return 0;
}
