/*********************************************************************
	
	ﾌｧｲﾙ入出力ﾗｲﾌﾞﾗﾘ
							NISINO Motoaki
	1999/06/16		
	1999/09/02		read_listfileを追加

**********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include "my.h"

#define	WORKBUFMAX	0x100000


/**********************************************************
引数	char	*fname		:ﾌｧｲﾙ名のﾎﾟｲﾝﾀ
		char	*mode				:ﾓｰﾄﾞの設定
返値	FILE*				:ﾌｧｲﾙﾊﾝﾄﾞﾗ
解説	ﾌｧｲﾙを開く
**********************************************************/
FILE *KsFileOpen(char *fname, char *mode)
{
	FILE 	*rfp;

	if ( ( rfp = fopen ( fname, mode )) == NULL ) {
			printf("%s ﾌｧｲﾙが開けません。\n", fname);
			return(0);//exit(1);
	}
#if 0
	printf("%s ﾌｧｲﾙを開きました。\n", fname);
#endif

	return(rfp);
}



/**********************************************************
引数
返値
解説	拡張子を変更して、ﾌｧｲﾙﾈｰﾑを変える
		
		KsChangeFileName(filename,filename2,".bmp");
		fire1.sdt→fire1.bmp
**********************************************************/
void KsChangeFileName(char *pname, char *pcname, char *pext)
{
	strcpy(pcname, pname);
	strcpy(strchr(pcname,'.'),"");			/* ﾌｧｲﾙ名の拡張子を取った物	*/
	strcat(pcname, pext);
}


/**********************************************************
引数
返値
解説	ﾌｧｲﾙのｻｲｽﾞを調べる
**********************************************************/
u_long KsFileSize(FILE *rfp)
{
	u_long	addr;

	fseek(rfp, 0, 2);
	addr = (u_long)ftell(rfp);
	fseek(rfp, 0, 0);
	addr -= (u_long)ftell(rfp);

	return(addr);
}

/**********************************************************
引数	FILE	*fp		: ﾌｧｲﾙﾎﾟｲﾝﾀ
		u_char 	*pbuf	: 補完するﾊﾞｯﾌｧﾎﾟｲﾝﾀ
返値	ﾌｧｲﾙｻｲｽﾞ
解説	指定ﾌｧｲﾙの内容を、ﾊﾞｯﾌｧに取り込む。
**********************************************************/
long KsFileToBuf(FILE *fp, BYTE *pbuf)
{
	int		size;

	size = (int)KsFileSize(fp);
#if 1
	printf("filesize   :%d byte\n", size);
#endif
	//..ｴﾗｰﾁｪｯｸ 
	if(size >= WORKBUFMAX) {
		printf("ﾊﾞｯﾌｧｻｲｽﾞが不足しています！\n");
		printf("filesize   :%d byte\n\n", size);
		exit(1);
	}
	
	fseek(fp, 0, 0);
	fread(pbuf, size, 1, fp);
	return(size);
}


/**********************************************************
引数	FILE	*fp		: ﾌｧｲﾙﾎﾟｲﾝﾀ
		u_char 	*pbuf	: 補完するﾊﾞｯﾌｧﾎﾟｲﾝﾀ
返値	ﾌｧｲﾙｻｲｽﾞ
解説	指定ﾌｧｲﾙの内容を、ﾊﾞｯﾌｧに取り込む。
**********************************************************/
u_long KsFileToBuf2(FILE *fp, u_char *pbuf,u_long work_size)
{
	u_long	size;

	size = (int)KsFileSize(fp);
#if 1
	printf("filesize   :%d byte\n", size);
#endif
	//..ｴﾗｰﾁｪｯｸ 
	if(size >= work_size) {
		printf("バッファのサイズが不足しています！\n");
		printf("filesize   :%d byte\n\n", size);
		exit(1);
	}
	
	fseek(fp, 0, 0);
	fread(pbuf, size, 1, fp);
	return(size);
}


/**********************************************************
関数
引数	ﾘｽﾄﾌｧｲﾙ名、ﾘｽﾄﾌｧｲﾙの文字列要素を格納するﾎﾟｲﾝﾀ配列,ﾜｰｸ
返値	文字列要素数　0は失敗
解説	ﾘｽﾄﾌｧｲﾙを読込み、第二引数のﾎﾟｲﾝﾀ配列に文字列要素
		へのﾎﾟｲﾝﾀを格納する関数
		ﾘｽﾄﾌｧｲﾙを格納するためのﾜｰｸが必要となる
**********************************************************/

short read_listfile(char *filename,char **pstr2,char *pwbuf){

	short i=0;
	FILE 	*rfp;

	if((rfp = KsFileOpen(filename,"r"))){
		KsFileToBuf(rfp,pwbuf);
	}else return(0);
	//..ﾎﾟｲﾝﾀ配列にﾄｰｸﾝを格納
	pstr2[i]=strtok(pwbuf,"\t/: ,\n");
	//printf(" %d %s\n",i,pstr2[i]);
	i++;
	while((pstr2[i]=strtok(NULL,"\t/: ,\n"))!=NULL){
		//printf(" %d %s \n",i,pstr2[i]);
		i++;
	}
	//printf("return(%d)\n",i);
	return(i);
}

/**********************************************************
引数
返値
解説	第１引数を大文字化し、コンマをｱﾝﾀﾞｰﾊﾞｰに変え、
		第２引数に代入
		例: test.txt→TEST_TXT
		
**********************************************************/
void ChangeFileNameUpper(char *pname, char *pname2)
{
	int i,num;
	num=strlen(pname);
	//printf("--%s\n",pname);

	//memset(pname2,0,num);
	for(i=0;i<num;i++){
		if(pname[i]=='.')
			pname2[i]='_';
		else pname2[i]=toupper(pname[i]);

		//printf("(%d)%3x ",i,pname2[i]);

	}
	
	pname2[i]='\0';
	//printf("(%d)%3x ",i,pname2[i]);
	//printf("\n");

}

/**********************************************************
関数
引数
返値
解説	拡張子を取ったﾌｧｲﾙ名
		'.' を '\0' に置き換える	
		
**********************************************************/
void DeleteKakutyousi(char *ppname, char *ppname2)
{
	int i,num;
	
	num=strlen(ppname);

	for(i=0;i<num;i++){
		if(ppname[i]=='.'){
			ppname2[i]='\0';break;
		}else ppname2[i]=ppname[i];
		
	}
}
/**********************************************************
関数
引数
返値
解説	pnameからパスを取り出す
		
**********************************************************/
short get_path(char *pname, char *pname2)
{
	int i,num;
	
	num=strlen(pname);

	for(i=num-1 ; i>=0 ; i--){
		//printf("(%d) %c ",i,pname[i]);
		if(pname[i]=='\\' || pname[i]=='/')
			break;
	}
	if(i==0) return(0);

	strncpy(pname2,pname,i+1);
	pname2[i+1]='\0';

	printf("%s\n",pname2);
	
	return(1);

}
/**********************************************************
関数
引数
返値
解説	ファイル名を取り出す
		in &fname ファイル名 &dst 出力先
		out 出力先アドレス
		例 c:\test\test.lst → test.lst
**********************************************************/

char *get_file(char *pname,char *pname2){

	int i,num,top;
	
	num=strlen(pname);

	for(i=num-1 ; i>=0 ; i--){
		//printf("(%d) %c ",i,pname[i]);
		if(pname[i]=='\\' || pname[i]=='/')
			break;
	}
	if(i==0){
		return(pname);
	}

	
	for(top=i;i<num;i++)
		pname2[i-top]=pname[i+1];
	
	//printf("getfile=%s\n",pname2);

	return(pname2);
}


/*
AnsiString (AnsiString &fName, AnsiString &dst){
	int i;
	for(i = fName.Length(); i >= 1; i--){
  	if(fName[i] == '\\' || fName[i] == '/')
    	break;
  }

  dst = fName.SubString(i + 1, fName.Length()+1 - (i+1));
// printf("filename dst %s\n", dst);
  return &dst;
*/


/**********************************************************
関数
引数
返値
解説	s1+s2=s3	
		
**********************************************************/

char *cat_string(char *s1,char *s2, char *s3){

	strcpy(s3,s1);
	strcat(s3,s2);
	return(s3);
}

/**********************************************************
関数
引数
返値
解説	ﾌｧｲﾙﾎﾟｼﾞｼｮﾝを出力	
		
**********************************************************/
void print_ftell(FILE *nfp){

	long pos=0;
	pos=ftell(nfp);
	printf("File Pos=%x\n",pos);

}
/*********************************************************************
関数
引数	
戻値	進めたカウント
解説	bufをﾜｰｸtbufへ書込み	
**********************************************************************/	

int fwrite2work(const void *buf,int size,int n,void *tbuf){

	memcpy(tbuf,buf,size*n);

	return(size*n);
	
	/*
	//..TIM_ID部書込み
	memcpy(tp,&tim_id2,sizeof(TIM_ID2));
	count+=sizeof(TIM_ID2);
	//fwrite(&tim_id2,sizeof(TIM_ID2),1,gfp);
	*/

}
