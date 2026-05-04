
/*********************************************************************
	
	BMPファイルを扱う関数群
			NISINO Motoaki
	1999/06/16
	2000/01/07	ｺﾋﾟｰ関数等を追加

**********************************************************************/
/*********************************************************************
関数	
引数	
戻値
解説
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "my.h"
#include "bmp.h"
#include "file.h"
#include "wrapfunc.h"


#define	WORKBUFMAX	0x100000

BYTE xbuf[WORKBUFMAX];
//u_char  xbuf[WORKBUFMAX];

//..(0,0,0)のピクセルを下地に書き込むか
#define DRAW_NULL_PIXEL


//extern void bzeroc(char *dst, int size);

void bzeroc(char *dst, int size) { for( ; size; --size, *dst++=0 ); }

void b_hai(char *dst, int size) { for( ; size; --size, *dst++=0x00 ); }

/*********************************************************************
解説	
引数	
戻値
注釈
**********************************************************************/
short bmp_pad(short w)
{
	short pad;
	if((w*3)%4)
		pad=4-((w*3)%4);
	else pad=0;
	//printf("pad=%d\n",pad);
	return(pad);
}

/*********************************************************************
解説	24bitﾃｸｽﾁｬの横ﾋﾟｸｾﾙの実ﾊﾞｲﾄ数を返す
引数	横ﾋﾟｸｾﾙ
戻値	横ﾊﾞｲﾄ数
注釈
**********************************************************************/
short byte_width(short yoko)
{

	return(yoko*3+bmp_pad(yoko));

}

/*
int  byte_width(int w){
	prect->pitch = w * byte;
	prect->pitch = ( prect->pitch + 3 ) & ~3;
}
*/

//..ピクセル幅 ,ピクセルあたりのバイト数
//..4バイトアライメントにそろえた値を返す
int  get_pitch(int w,int byte){
	return( ( w * byte + 3 ) & ~3);
}



/*********************************************************************
解説	BMPﾍｯﾀﾞﾌｧｲﾙの解析 ppifにbmpの情報を代入
引数	BMPを格納しているﾊﾞｯﾌｧの先頭ｱﾄﾞﾚｽ
戻値	1 成功　0　失敗
注釈
**********************************************************************/
#define FILE_M 0

short kaiseki_bmpfile(char *p,BITMAP_INFO_HEADER *ppif)
{
	BITMAP_FILEHEADER *pfh;
	BITMAP_INFO_HEADER *pinfo;
	ENDIAN size,addr;

	pfh=(BITMAP_FILEHEADER*)p;
		
	size.us.l=pfh->bf_size_l;
	size.us.h=pfh->bf_size_h;

	addr.us.l=pfh->bf_off_bits_l;
	addr.us.h=pfh->bf_off_bits_h;

#if FILE_M
	printf("ﾌｧｲﾙﾀｲﾌﾟ=%x\n",pfh->bf_type);
#endif
	if(pfh->bf_type!=0x4d42){
		printf("bmpﾌｧｲﾙではありません\n");
		return(0);
	}
#if FILE_M
	printf("ﾌｧｲﾙｻｲｽﾞ=%dﾊﾞｲﾄ\n",size.ul);//pfh->bf_size);
	printf("ｲﾒｰｼﾞの開始位置=%x\n",addr.ul);
#endif
	pfh++;
	
	pinfo=(BITMAP_INFO_HEADER*)pfh;
#if FILE_M
	printf("ﾍｯﾀﾞのｻｲｽﾞ =%d\n",pinfo->bi_size);		//..ﾍｯﾀﾞのｻｲｽﾞ 40ﾊﾞｲﾄ
	printf("ｲﾒｰｼﾞの幅  =%d\n",pinfo->bi_width);		//..ｲﾒｰｼﾞの幅
	printf("ｲﾒｰｼﾞの高さ=%d\n",pinfo->bi_height);	//..ｲﾒｰｼﾞの高さ
	printf("ｲﾒｰｼﾞﾌﾟﾚｰﾝ数=%d\n",pinfo->bi_planes);	//..ｲﾒｰｼﾞﾌﾟﾚｰﾝ数 必ず1
	printf("ﾋﾟｸｾﾙあたりのﾋﾞｯﾄ数=%d\n",pinfo->bi_bitcount);	//..ﾋﾟｸｾﾙあたりのﾋﾞｯﾄ数 1,4,8,24

	if(pinfo->bi_bitcount==24)
		printf("横実ﾊﾞｲﾄ数=%d\n",byte_width(pinfo->bi_width));
#endif
	//ppif->bi_size=pinfo->bi_size;
	ppif->bi_width=pinfo->bi_width;
	ppif->bi_height=pinfo->bi_height;

	return(1);
}
/*********************************************************************
解説	test
引数	
戻値
**********************************************************************/
short kakikomi_bmpfile(FILE *wfp){
	
	u_char a=0;
	short yoko,tate;
	ENDIAN file_size;
	BITMAP_FILEHEADER fh;
	BITMAP_INFO_HEADER info;
	long i;
	
	bzeroc((char*)&fh,sizeof(BITMAP_FILEHEADER));
	bzeroc((char*)&info,sizeof(BITMAP_INFO_HEADER));

	yoko=3;	tate=3;
	
	file_size.ul=sizeof(BITMAP_FILEHEADER)+sizeof(BITMAP_INFO_HEADER)+
		tate*(yoko*3+bmp_pad(yoko));
	
	//..ﾌｧｲﾙﾍｯﾀﾞ設定
	fh.bf_type=0x4d42;
	fh.bf_size_l=file_size.us.l;
	fh.bf_size_h=file_size.us.h;

	fh.bf_off_bits_l=54;

	fwrite(&fh,sizeof(BITMAP_FILEHEADER),1,wfp);
	
	//..infoの設定
	info.bi_size=40;
	info.bi_width=yoko;
	info.bi_height=tate;
	info.bi_planes=1;
	info.bi_bitcount=24;

	fwrite(&info,sizeof(BITMAP_INFO_HEADER),1,wfp);

	for(i=0; i<tate*(yoko*3+bmp_pad(yoko)); i++)
		fwrite(&a,1,1,wfp);

	fclose(wfp);

	return(0);
}
/*********************************************************************
解説	ﾌｧｲﾙにBMPﾍｯﾀﾞﾌｧｲﾙを書き込む
引数	書き込まれるﾌｧｲﾙﾎﾟｲﾝﾀ
戻値
注釈	24bitﾓｰﾄﾞ限定
**********************************************************************/
short settei_bmpheader(FILE *wfp,short tate,short yoko){
	
	ENDIAN file_size;
	BITMAP_FILEHEADER fh;
	BITMAP_INFO_HEADER info;
	
	bzeroc((char*)&fh,sizeof(BITMAP_FILEHEADER));
	bzeroc((char*)&info,sizeof(BITMAP_INFO_HEADER));

	//..ﾌｧｲﾙｻｲｽﾞの設定
	file_size.ul=sizeof(BITMAP_FILEHEADER)+sizeof(BITMAP_INFO_HEADER)+
		tate*(yoko*3+bmp_pad(yoko));
	
	//..ﾌｧｲﾙﾍｯﾀﾞ設定
	fh.bf_type=0x4d42;
	fh.bf_size_l=file_size.us.l;
	fh.bf_size_h=file_size.us.h;

	fh.bf_off_bits_l=54;

	fwrite(&fh,sizeof(BITMAP_FILEHEADER),1,wfp);
	
	//..infoの設定
	info.bi_size=40;
	info.bi_width=yoko;
	info.bi_height=tate;
	info.bi_planes=1;
	info.bi_bitcount=24;

	fwrite(&info,sizeof(BITMAP_INFO_HEADER),1,wfp);
	
	printf("Ok\n");

	return(0);
}


/*********************************************************************
解説	ﾌｧｲﾙにBMPﾍｯﾀﾞﾌｧｲﾙを書き込む
引数	書き込まれるﾌｧｲﾙﾎﾟｲﾝﾀ
戻値
注釈	32bitﾓｰﾄﾞ限定
**********************************************************************/
short settei_bmpheader32(FILE *wfp,short tate,short yoko){
	
	ENDIAN file_size;
	BITMAP_FILEHEADER fh;
	BITMAP_INFO_HEADER info;
	
	bzeroc((char*)&fh,sizeof(BITMAP_FILEHEADER));
	bzeroc((char*)&info,sizeof(BITMAP_INFO_HEADER));

	//..ﾌｧｲﾙｻｲｽﾞの設定
	file_size.ul=sizeof(BITMAP_FILEHEADER)+sizeof(BITMAP_INFO_HEADER)+
		tate*(yoko*4);
	
	dprint(file_size.ul);

	//..ﾌｧｲﾙﾍｯﾀﾞ設定
	fh.bf_type=0x4d42;
	fh.bf_size_l=file_size.us.l;
	fh.bf_size_h=file_size.us.h;

	fh.bf_off_bits_l=54;

	fwrite(&fh,sizeof(BITMAP_FILEHEADER),1,wfp);
	
	//..infoの設定
	info.bi_size=40;
	info.bi_width=yoko;
	info.bi_height=tate;
	info.bi_planes=1;
	info.bi_bitcount=32;	//..ここがユニークな点となる

	fwrite(&info,sizeof(BITMAP_INFO_HEADER),1,wfp);
	
	printf("Ok\n");

	return(0);
}


/*********************************************************************
解説	ﾌｧｲﾙにBMPﾍｯﾀﾞﾌｧｲﾙを書き込む
引数	書き込まれるﾌｧｲﾙﾎﾟｲﾝﾀ
戻値
注釈	24bit 32bitのみ対応
**********************************************************************/
short settei_bmpheader2( FILE *wfp, RECT2 *prect ){
	
	ENDIAN file_size;
	BITMAP_FILEHEADER fh;
	BITMAP_INFO_HEADER info;
	
	bzeroc((char*)&fh,sizeof(BITMAP_FILEHEADER));
	bzeroc((char*)&info,sizeof(BITMAP_INFO_HEADER));

	//..ﾌｧｲﾙｻｲｽﾞの設定
	file_size.ul=sizeof(BITMAP_FILEHEADER)+sizeof(BITMAP_INFO_HEADER)+
		prect->pitch * prect->h;
	
	//..ﾌｧｲﾙﾍｯﾀﾞ設定
	fh.bf_type=0x4d42;
	fh.bf_size_l=file_size.us.l;
	fh.bf_size_h=file_size.us.h;

	fh.bf_off_bits_l=54;

	fwrite(&fh,sizeof(BITMAP_FILEHEADER),1,wfp);
	
	//..infoの設定
	info.bi_size=40;
	info.bi_width = prect->w;
	info.bi_height = prect->h;
	info.bi_planes=1;
	info.bi_bitcount=prect->byte*8;

	fwrite(&info,sizeof(BITMAP_INFO_HEADER),1,wfp);

	return(0);
}





/*********************************************************************
解説	24bitBmpﾌｧｲﾙのｲﾒｰｼﾞをﾒﾓﾘに確保する関数
引数	BMPﾌｧｲﾙ名,ﾃｸｽﾁｬ縦横情報を代入する構造体
戻値	先頭ｱﾄﾞﾚｽ,0は失敗
注釈	prectにﾋﾞｯﾄﾏｯﾌﾟﾌｧｲﾙの情報が格納される
**********************************************************************/

char* bmp2buffer(char *bmpfilename,RECT *prect)
{
	char *pw,*pp;
	short w;
	long i,j;
	FILE *rfp;
	BITMAP_INFO_HEADER ppif;
	

	rfp = KsFileOpen(bmpfilename, "rb");
	
	//..いったんﾜｰｸに確保
	KsFileToBuf(rfp,xbuf);
	fclose(rfp);

	//..ﾃｸｽﾁｬ情報解析
	kaiseki_bmpfile(xbuf,&ppif);

	prect->w=ppif.bi_width;
	prect->h=ppif.bi_height;

	//..実際の幅..ﾊﾞｲﾄ単位　3pixel=12byte
	w=ppif.bi_width*3 + bmp_pad(ppif.bi_width);//*3);

	printf("ｲﾒｰｼﾞｻｲｽﾞ=%d*%d=%d\n", w, (int)ppif.bi_height,(int)(w*ppif.bi_height) );

	pp=(char*)malloc(w * ppif.bi_height);
	if(pp==NULL){
		printf("メモリが確保できません。\n");
		exit(1);
	}
	//..黒ｸﾘｱ
	bzeroc((char*)pp,w * ppif.bi_height);

	//..ﾍｯﾀﾞを飛ばしｲﾒｰｼﾞ開始位置までﾎﾟｲﾝﾀ進める
	pw=xbuf+54;
	
	//..mallocでﾒﾓﾘを確保した領域にｲﾒｰｼﾞｺﾋﾟｰ
	for(i=0; i<ppif.bi_height; i++)
		for(j=0; j<w; j++)
			*(pp+i*w+j)=*(pw + (ppif.bi_height-1-i)*w + j);

	return(pp);

}

/*********************************************************************
解説	下地24bitﾃｸｽﾁｬに上絵24bitﾃｸｽﾁｬをｺﾋﾟｰする
引数	下地ｲﾒｰｼﾞ情報構造体、下地ｲﾒｰｼﾞ先頭ﾎﾟｲﾝﾀ、
		上絵ｲﾒｰｼﾞ情報構造体、上絵ｲﾒｰｼﾞ先頭ﾎﾟｲﾝﾀ
		下地への書込み位置x、下地への書込み位置
戻値
注釈	書込み位置は
**********************************************************************/

short paste_bmpimg(
	RECT *prect_s,u_char *ps,
	RECT *prect_u,u_char *pu,
	short x,short y
	){

	u_char *pp;
	short i,j,k;
	
	//..安全処理
	if( x<0 || y<0 || x>prect_s->w || y>prect_s->h){
		printf("paste_bmpimg:書込み位置が範囲外にあります \n");
		printf("xy=%d,%d  wh=%d,%d\n",x,y,prect_s->w,prect_s->h);
		return(0);
	}

	//..下地の書込み位置求める
	pp = ps +  x*3 + byte_width(prect_s->w)*y;

	//..ｲﾒｰｼﾞｺﾋﾟｰ
	for(i=0; i<prect_u->h && i+y<prect_s->h; i++){
		for(j=0; (j < prect_u->w*3)&&(x+j/3 < prect_s->w); j+=3){
			
			//..黒は書かない
			if(
				*(pu + j+0 + byte_width(prect_u->w)*i)+
				*(pu + j+1 + byte_width(prect_u->w)*i)+
				*(pu + j+2 + byte_width(prect_u->w)*i)
				){
				for(k=0; k<3; k++)
					*(pp + j+k + byte_width(prect_s->w)*i)=*(pu + j+k + byte_width(prect_u->w)*i);
			}
		}
	}
	return(1);
}

/*********************************************************************
解説	下地24bitﾃｸｽﾁｬに上絵24bitﾃｸｽﾁｬを半透明指定でｺﾋﾟｰする
引数	下地ｲﾒｰｼﾞ情報構造体、下地ｲﾒｰｼﾞ先頭ﾎﾟｲﾝﾀ、
		上絵ｲﾒｰｼﾞ情報構造体、上絵ｲﾒｰｼﾞ先頭ﾎﾟｲﾝﾀ
		下地への書込み位置x、下地への書込み位置
戻値
注釈	書込み位置はx,y
		0 不透明
		1 下地50%+上絵50%
		2 下地100%+上絵100%..未完成
		3
		4
**********************************************************************/

short paste_bmpimg_semi(
	RECT *prect_s,u_char *ps,
	RECT *prect_u,u_char *pu,
	short x,short y,short semi
	){

	u_char *pp;
	short i,j,k;
	
	//..安全処理
	if( x<0 || y<0 || x>prect_s->w || y>prect_s->h){
		printf("paste_bmpimg_semi:書込み位置が範囲外にあります!! \n");
		printf("xy=%d,%d  wh=%d,%d\n",x,y,prect_s->w,prect_s->h);
		return(0);
	}

	//..下地の書込み位置求める
	pp = ps +  x*3 + byte_width(prect_s->w)*y;

	//..ｲﾒｰｼﾞｺﾋﾟｰ
	for(i=0; i<prect_u->h && i+y<prect_s->h; i++){
		for(j=0; (j < prect_u->w*3)&&(x+j/3 < prect_s->w); j+=3){
			
			//..黒は書かない
			if(
				*(pu + j+0 + byte_width(prect_u->w)*i)+
				*(pu + j+1 + byte_width(prect_u->w)*i)+
				*(pu + j+2 + byte_width(prect_u->w)*i)
				){
				for(k=0; k<3; k++){
					switch(semi){
					case 0:
						*(pp+j+k+byte_width(prect_s->w)*i)=
							*(pu+j+k+byte_width(prect_u->w)*i);
						break;
					case 1:
						//..75%+25%
						*(pp+j+k+byte_width(prect_s->w)*i)=
							*(pu+j+k+byte_width(prect_u->w)*i)*3/4
							+*(pp+j+k+byte_width(prect_s->w)*i)/4;
						break;
					default:
						break;
					}
				}
			}
		}
	}
	return(1);
}

/*********************************************************************
解説	下地24bitﾃｸｽﾁｬに上絵24bitRGBを半透明指定でｺﾋﾟｰする
引数	下地ｲﾒｰｼﾞ情報構造体、下地ｲﾒｰｼﾞ先頭ﾎﾟｲﾝﾀ、
		上絵ｲﾒｰｼﾞ情報構造体、上絵ｲﾒｰｼﾞ先頭ﾎﾟｲﾝﾀ
		下地への書込み位置x、下地への書込み位置
戻値
注釈	書込み位置はx,y
		0 不透明
		1 下地50%+上絵50%
		2 下地100%+上絵100%..未完成
		3
		4
**********************************************************************/

short paste_rgb_semi(
	short x,short y,
	RECT *prect_s,u_char *ps,
	RECT *prect_u,
	RGB_QUAD rgb,short semi
	){

	u_char *pp;
	short i,j;
	
	//..安全処理
	if( x<0 || y<0 || x>prect_s->w || y>prect_s->h){
		printf("Error! 書込み位置が範囲外にあります rgb_semi\n");
		printf("xy=%d,%d  wh=%d,%d\n",x,y,prect_s->w,prect_s->h);
		return(0);
	}

	//..下地の書込み位置求める
	pp = ps +  x*3 + byte_width(prect_s->w)*y;

	//..ｲﾒｰｼﾞｺﾋﾟｰ
	for(i=0; i<prect_u->h && i+y<prect_s->h; i++){
		for(j=0; (j < prect_u->w*3)&&(x+j/3 < prect_s->w); j+=3){
			
			//..黒は書かない
			/*
			if(
				*(pu + j+0 + byte_width(prect_u->w)*i)+
				*(pu + j+1 + byte_width(prect_u->w)*i)+
				*(pu + j+2 + byte_width(prect_u->w)*i)
				){
			*/	
					switch(semi){
					case 0:
						*(RGB_QUAD*)(pp+j+byte_width(prect_s->w)*i)=rgb;
						break;
					case 1:
						/*
						//..75%+25%
						*(RGB_QUAD)(pp+j+byte_width(prect_s->w)*i)=	
							*(pu+j+byte_width(prect_u->w)*i)/2+*(pp+j+k+byte_width(prect_s->w)*i)/2;
						*/
						break;
					default:
						break;
					}
				
			//}
		}
	}
	return(1);
}


/*********************************************************************
解説	24bitBmpﾌｧｲﾙのｲﾒｰｼﾞをﾒﾓﾘに確保する関数
引数	BMPﾌｧｲﾙ名,ﾃｸｽﾁｬ縦横情報を代入する構造体
戻値	先頭ｱﾄﾞﾚｽ,0は失敗
注釈	prectにﾋﾞｯﾄﾏｯﾌﾟﾌｧｲﾙの情報が格納される
		第二引数をRECT2に変更
**********************************************************************/

char* bmp2buffer2(char *bmpfilename,RECT2 *prect)
{
	char *pw,*pp;
	short w;
	long i,j;
	FILE *rfp;
	BITMAP_INFO_HEADER ppif;
	

	rfp = KsFileOpen(bmpfilename, "rb");
	
	//..いったんﾜｰｸに確保
	KsFileToBuf(rfp,xbuf);
	fclose(rfp);

	//..ﾃｸｽﾁｬ情報解析
	kaiseki_bmpfile(xbuf,&ppif);

	prect->w=(u_short)ppif.bi_width;
	prect->h=(u_short)ppif.bi_height;

	//..実際の幅..ﾊﾞｲﾄ単位　3pixel=12byte
	w=byte_width(ppif.bi_width);	
	
#if 0
	printf("ｲﾒｰｼﾞｻｲｽﾞ=%d*%d=%d\n",w,ppif.bi_height,w * ppif.bi_height);
#endif

	pp=(char*)malloc(w * ppif.bi_height);
	if(pp==NULL){
		printf("メモリが確保できません。bmp2buffer2_p\n");
		exit(1);
	}

	//..ｱﾄﾞﾚｽ保持
	prect->pimg=(u_char*)pp;

	//..黒ｸﾘｱ
	bzeroc((char*)pp,w * ppif.bi_height);

	//..ﾍｯﾀﾞを飛ばしｲﾒｰｼﾞ開始位置までﾎﾟｲﾝﾀ進める
	pw=xbuf+54;
	
	//..mallocでﾒﾓﾘを確保した領域にｲﾒｰｼﾞｺﾋﾟｰ
	for(i=0; i<ppif.bi_height; i++)
		for(j=0; j<w; j++)
			*(pp+i*w+j)=*(pw + (ppif.bi_height-1-i)*w + j);

	return(pp);

}

/*********************************************************************
解説	下地24bitﾃｸｽﾁｬに上絵24bitﾃｸｽﾁｬを半透明指定でｺﾋﾟｰする
引数	下地ｲﾒｰｼﾞ情報構造体、
		上絵ｲﾒｰｼﾞ情報構造体、
		下地への書込み位置x、下地への書込み位置y
戻値
注釈	書込み位置はx,y
		0 不透明
		1 下地25%+上絵75%
		2 下地100%+上絵100%..未完成
		3
		4
**********************************************************************/

short paste_bmpimg_semi2(RECT2 *prect_s, RECT2 *prect_u,short x,short y,short semi){

	u_char *pp;
	short i,j,k;
	
	//..安全処理
	if( x<0 || y<0 || x>prect_s->w || y>prect_s->h){
		printf("paste_bmpimg_semi:書込み位置が範囲外にあります!! \n");
		printf("xy=%d,%d  wh=%d,%d\n",x,y,prect_s->w,prect_s->h);
		return(0);
	}

	//..下地の書込み位置求める
	pp = prect_s->pimg + x*3 + byte_width(prect_s->w)*y;

	//..ｲﾒｰｼﾞｺﾋﾟｰ
	for(i=0; i<prect_u->h && i+y<prect_s->h; i++){
		for(j=0; (j < prect_u->w*3)&&(x+j/3 < prect_s->w); j+=3){

#ifndef DRAW_NULL_PIXEL
			//..黒は書かない
			if(
				*(prect_u->pimg + j+0 + byte_width(prect_u->w)*i)+
				*(prect_u->pimg + j+1 + byte_width(prect_u->w)*i)+
				*(prect_u->pimg + j+2 + byte_width(prect_u->w)*i)
				){
#endif
				
				for(k=0; k<3; k++){
					switch(semi){
					case 0:
						*(pp+j+k+byte_width(prect_s->w)*i)=
							*(prect_u->pimg+j+k+byte_width(prect_u->w)*i);
						break;
					case 1:
						//printf("a");
						//..75%+25%
						*(pp+j+k+byte_width(prect_s->w)*i)=
							(*(prect_u->pimg+j+k+byte_width(prect_u->w)*i))*3/4
							+
							*(pp+j+k+byte_width(prect_s->w)*i)/4;
						break;
					default:
						break;
					}
				}
#ifndef DRAW_NULL_PIXEL
			}
#endif
		}
	}
	return(1);
}


/*********************************************************************
解説	下地32bitﾃｸｽﾁｬに上絵32bitﾃｸｽﾁｬを半透明指定でｺﾋﾟｰする
引数	下地ｲﾒｰｼﾞ情報構造体、
		上絵ｲﾒｰｼﾞ情報構造体、
		下地への書込み位置x、下地への書込み位置y
戻値
注釈	書込み位置はx,y
		0 不透明
		1 下地25%+上絵75%
		2 下地100%+上絵100%..未完成
		3
		4
**********************************************************************/

short paste_bmpimg_semi32(RECT2 *prect_s, RECT2 *prect_u,short x,short y,short semi){

	u_char *pp;
	short i,j,k;
	
	//..安全処理
	if( x<0 || y<0 || x>prect_s->w || y>prect_s->h){
		printf("paste_bmpimg_semi:書込み位置が範囲外にあります!! \n");
		printf("xy=%d,%d  wh=%d,%d\n",x,y,prect_s->w,prect_s->h);
		return(0);
	}

	//..下地の書込み位置求める
	pp = prect_s->pimg + x*4 + prect_s->w*4*y;

	//..ｲﾒｰｼﾞｺﾋﾟｰ
	for(i=0; i<prect_u->h && i+y<prect_s->h; i++){
		for(j=0; (j < prect_u->w*4)&&(x+j/4 < prect_s->w); j+=4){

#ifndef DRAW_NULL_PIXEL
			//..黒を書かない場合
			if(
				*(prect_u->pimg + j+0 + prect_u->w * 4 * i)+
				*(prect_u->pimg + j+1 + prect_u->w * 4 * i)+
				*(prect_u->pimg + j+2 + prect_u->w * 4 * i)
				){
#endif
				for(k=0; k<4; k++){
					switch(semi){
					case 0:
						*(pp+j+k+ prect_s->w * 4 * i)=
							*(prect_u->pimg+j+k+ prect_u->w * 4 * i);
						break;
					case 1:
						//printf("a");
						//..75%+25%
						*(pp+j+k+ prect_s->w*4*i)=
							(*(prect_u->pimg+j+k+prect_u->w*4*i))*3/4
							+
							*(pp+j+k+prect_s->w*4*i)/4;
						break;
					default:
						break;
					}
				}
#ifndef DRAW_NULL_PIXEL
			}
#endif
		}
	}
	return(1);
}



/*********************************************************************
解説	下地24bitﾃｸｽﾁｬに上絵24bitRGBを半透明指定で塗りつぶし
引数	下地への書込み位置(x,y),下地ｲﾒｰｼﾞ情報構造体、上絵ｲﾒｰｼﾞ情報構造体、
		
戻値
注釈	書込み位置はx,y
		0 不透明
		1 下地50%+上絵50%
		2 下地100%+上絵100%..未完成
		3
		4
**********************************************************************/

short paste_rgb_semi2(short x,short y,RECT2 *prect_s,RECT2 *prect_u,RGB_QUAD rgb,short semi){

	u_char *pp;
	short i,j;
	
	//..安全処理
	if( x<0 || y<0 || x>prect_s->w || y>prect_s->h){
		printf("Error! 書込み位置が範囲外にあります rgb_semi\n");
		printf("xy=%d,%d  wh=%d,%d\n",x,y,prect_s->w,prect_s->h);
		return(0);
	}

	//..下地の書込み位置求める
	pp = prect_s->pimg/*ps*/ +  x*3 + byte_width(prect_s->w)*y;

	//..ｲﾒｰｼﾞｺﾋﾟｰ
	for(i=0; i<prect_u->h && i+y<prect_s->h; i++){
		for(j=0; (j < prect_u->w*3)&&(x+j/3 < prect_s->w); j+=3){
			switch(semi){
			case 0:
				*(RGB_QUAD*)(pp+j+byte_width(prect_s->w)*i)=rgb;
				break;
			case 1:
				/*
				//..75%+25%
				*(RGB_QUAD)(pp+j+byte_width(prect_s->w)*i)=	
					*(pu+j+byte_width(prect_u->w)*i)/2+*(pp+j+k+byte_width(prect_s->w)*i)/2;
				*/
				break;
			default:
				break;
			}
		}
	}
	return(1);
}



/*********************************************************************
解説
引数	小節をひく位置x,y、ワーク先頭ポインタ
戻値
解説	小節の線をひく
**********************************************************************/
short draw_yoko_line(short x,short y,short nagasa,RGB_QUAD quad,RECT2 *prect){
	
	u_char *p;
	short j,w;

	//..安全処理
	if( x<0 || y<0 || x > prect->w || y > prect->h){
		printf("Warning! 書込み位置が範囲外にあります draw_yoko_line\n");
		printf("(x,y)=(%d,%d) (w,h)=(%d,%d)\n",x,y,prect->w,prect->h);
		return(0);
	}

	if(nagasa+x > prect->w)
		 nagasa=prect->w-x;
	
	//..ﾒﾓﾘ上の幅..ﾊﾞｲﾄ単位
	w=byte_width(prect->w);

	//..ｱﾄﾞﾚｽを書込み位置に進める
	p=prect->pimg+x*3+y*w;

	for(j=0; j<nagasa; j++)
		*(RGB_QUAD*)(p+j*3)=quad;

	return(1);
}

/*********************************************************************
解説
引数	小節をひく位置x,y、ワーク先頭ポインタ
戻値
解説	小節の線をひく
**********************************************************************/
short draw_dot(short x,short y,RGB_QUAD quad,RECT2 *prect){
	
	u_char *p;
	short w;

	//..安全処理
	if( x<0 || y<0 || x > prect->w || y > prect->h){
		printf("Warning! 書込み位置が範囲外にあります draw_dot\n");
		printf("(x,y)=(%d,%d) (w,h)=(%d,%d)\n",x,y,prect->w,prect->h);
		return(0);
	}
	
	//..ﾒﾓﾘ上の幅..ﾊﾞｲﾄ単位
	w=byte_width(prect->w);

	//..ｱﾄﾞﾚｽを書込み位置に進める
	p=prect->pimg+x*3+y*w;

	*(RGB_QUAD*)p=quad;

	return(1);
}

/*********************************************************************
関数	
引数	
戻値
解説	RECT2をﾌｧｲﾙ出力
**********************************************************************/

short rect2file(RECT2 *prect,u_char *filename){
	
	short i,j;
	long w;
	FILE *wfp;
	
	if(!(wfp = KsFileOpen((char*)filename,"wb+")))	return(0);

	//..ﾒﾓﾘ上の幅..ﾊﾞｲﾄ単位
	w=byte_width(prect->w);
	
	settei_bmpheader(wfp,prect->h,prect->w);
	
	//..ﾌｧｲﾙに出力
	for(i=prect->h-1; i>=0; i--)
		for(j=0; j<w; j++)
			fwrite(prect->pimg+i*w+j,1,1,wfp);
	
	fclose(wfp);
	return(1);
}


/*********************************************************************
関数	
引数	
戻値
解説	RECT2のイメージにヘッダをつけて引数のファイル名で出力する
	
**********************************************************************/

short rect2file32(RECT2 *prect,u_char *filename){
	
	short i,j;
	long w;
	FILE *wfp;
	
	if(!(wfp = KsFileOpen((char*)filename,"wb+")))	return(0);

	//..ﾒﾓﾘ上の幅..ﾊﾞｲﾄ単位
	w=prect->w*4;
	
	settei_bmpheader32(wfp,prect->h,prect->w);
	
	//..ﾌｧｲﾙに出力
	for(i=prect->h-1; i>=0; i--)
		for(j=0; j<w; j++)
			fwrite(prect->pimg+i*w+j,1,1,wfp);
	
	fclose(wfp);
	return(1);
}

/*
解説	prectのイメージをファイルとして出力する
		pitchが設定されていなければならないので注意
引数	
戻値
*/

int  RectImg4Outputfile(RECT2 *prect,u_char *filename){
	
	short i,j;
	FILE *wfp;
	
	if(!(wfp = KsFileOpen((char*)filename,"wb+")))	return(1);

	settei_bmpheader2(wfp,prect);
	
	//..ﾌｧｲﾙに出力
	for(i=prect->h-1; i>=0; i--)
		for(j=0; j < prect->pitch; j++)
			fwrite(prect->pimg + i*prect->pitch + j,1,1,wfp);
	
	fclose(wfp);
	return(0);
}





/*********************************************************************
関数	
引数	
戻値
解説	RECT2をﾌｧｲﾙ出力 上下逆
**********************************************************************/

short rect2file_revers(RECT2 *prect,u_char *filename){
	
	short i,j;
	long w;
	FILE *wfp;
	
	if(!(wfp = KsFileOpen((char*)filename,"wb+")))	return(0);

	//..ﾒﾓﾘ上の幅..ﾊﾞｲﾄ単位
	w=byte_width(prect->w);
	
	settei_bmpheader(wfp,prect->h,prect->w);
	
	//..ﾌｧｲﾙに出力
	for(i=0; i< prect->h; i++)
	//for(i=prect->h-1; i>=0; i--)
		for(j=0; j<w; j++)
			fwrite(prect->pimg+i*w+j,1,1,wfp);
	
	fclose(wfp);
	return(1);
}


/*********************************************************************
関数	
引数	
戻値
解説	RECT2をﾌｧｲﾙ出力　書込み側がｵｰﾌﾟﾝ済み
**********************************************************************/

short rect2wfp(RECT2 *prect,FILE *wfp){
	
	short i,j;
	long w;

	fseek(wfp, 0, 0);
	
	//..ﾒﾓﾘ上の幅..ﾊﾞｲﾄ単位
	w=byte_width(prect->w);
	
	settei_bmpheader(wfp,prect->h,prect->w);
	
	//..ﾌｧｲﾙに出力
	for(i=prect->h-1; i>=0; i--)
		for(j=0; j<w; j++)
			fwrite(prect->pimg+i*w+j,1,1,wfp);
	
	return(1);
}

/*********************************************************************
関数	
引数	
戻値
解説	RECT2をﾌｧｲﾙ出力　書込み側がｵｰﾌﾟﾝ済み 32bit版
**********************************************************************/

short rect2wfp32(RECT2 *prect,FILE *wfp){
	
	short i,j;
	long w;
	//char alpha=0x41;

	fseek(wfp, 0, 0);
	
	//..ﾒﾓﾘ上の幅..ﾊﾞｲﾄ単位
	w=prect->w*4;

	settei_bmpheader32(wfp,prect->h,prect->w);

	//..ﾌｧｲﾙに出力
	for(i=prect->h-1; i>=0; i--){
		for(j=0; j < prect->w; j++){
			fwrite(prect->pimg + i * (w) + 4*j    ,1,1,wfp);	//..B
			fwrite(prect->pimg + i * (w) + 4*j + 1,1,1,wfp);//..G
			fwrite(prect->pimg + i * (w) + 4*j + 2,1,1,wfp);//..R
			fwrite(prect->pimg + i * (w) + 4*j + 3,1,1,wfp);//..A
			//fwrite(&alpha,1,1,wfp);							//..alpha
		}
	}
	return(1);
}


/*********************************************************************
関数	
引数	
戻値
解説
**********************************************************************/
short copy_bmpimg(short u,short v,short w,short h,RECT2 *pmoto,RECT2 *psaki){

	u_char *pp;
	short i,j,k,bw,semi=0;
	
	//..安全処理
	if( u<0 || v<0 || u>=pmoto->w || v>=pmoto->h){
		printf("copy_bmpimg:書込み位置が範囲外にあります!! \n");
		printf("uv=%d,%d  wh=%d,%d\n",u,v,pmoto->w,psaki->h);
		return(0);
	}

	if(u+w > pmoto->w) w=pmoto->w-u;
	if(v+h > pmoto->h) h=pmoto->h-v;
	
	psaki->w=w;	psaki->h=h;
	
	//printf("wh=%d %d\n",w,h);

	//..ﾒﾓﾘ上の幅..ﾊﾞｲﾄ単位
	bw=byte_width(w);
	//..ﾒﾓﾘ確保
	psaki->pimg=(u_char*)malloc(bw * h);
	if(psaki->pimg==NULL){
		printf("メモリが確保できません。\n");exit(1);
	}
	
	//..下地の読み込み位置求める
	pp = pmoto->pimg + u*3 + byte_width(pmoto->w)*v;
	
	//..ｲﾒｰｼﾞｺﾋﾟｰ
	for(i=0; i < psaki->h && i+v < pmoto->h; i++){
	    for(j=0; (j < psaki->w*3)&&(u+j/3 < pmoto->w); j+=3){   
			for(k=0; k<3; k++){
				switch(semi){
				case 0:
					*(psaki->pimg+j+k+byte_width(psaki->w)*i)=
						*(pp+j+k+byte_width(pmoto->w)*i);
					//*(pp+j+k+byte_width(prect_s->w)*i)=
					//	*(prect_u->pimg+j+k+byte_width(prect_u->w)*i);
					break;
				
				default:
					break;
				}
			}
			//}
		}
	}
	return(1);

}

/*********************************************************************
関数	数字ﾌｫﾝﾄをﾒﾓﾘにﾛｰﾄﾞ
引数	
戻値	
解説	
**********************************************************************/

static RECT2 font;
static RECT2 num[20];

void number_font_load(void){

	int i=0;
	
	bmp2buffer2("font35B.bmp",&font);
	
	for(i=0; i<16; i++)
		copy_bmpimg(i*3,0,3,5,&font,&num[i]);

	printf("number_font_load OK!\n");

}


/*********************************************************************
関数	数字を出力
引数	
戻値	
解説	0=DEC,1=HEX,
**********************************************************************/

void print_number(short px,short py,RECT2 *pr,int number,int type){

	u_char a[256];
	int i=0,x,y;
	
	x=px;	y=py;

	//printf("1\n");
	if(type==1)
	 sprintf((char*)a, "%x", number);
	else sprintf((char*)a,"%d",number);

	printf("%s\n",a);
	
	while(a[i]){
		switch(a[i]){
		case '0':
			paste_bmpimg_semi2(pr,&num[0],x,y,0);
			break;
		case '1':
			paste_bmpimg_semi2(pr,&num[1],x,y,0);
			break;
		case '2':
			paste_bmpimg_semi2(pr,&num[2],x,y,0);
			break;
		case '3':
			paste_bmpimg_semi2(pr,&num[3],x,y,0);
			break;
		case '4':
			paste_bmpimg_semi2(pr,&num[4],x,y,0);
			break;
		case '5':
			paste_bmpimg_semi2(pr,&num[5],x,y,0);
			break;
		case '6':
			paste_bmpimg_semi2(pr,&num[6],x,y,0);
			break;
		case '7':
			paste_bmpimg_semi2(pr,&num[7],x,y,0);
			break;
		case '8':
			paste_bmpimg_semi2(pr,&num[8],x,y,0);
			break;
		case '9':
			paste_bmpimg_semi2(pr,&num[9],x,y,0);
			break;
		case 'a':
		case 'A':
			paste_bmpimg_semi2(pr,&num[10],x,y,0);
			break;
		case 'b':
		case 'B':
			paste_bmpimg_semi2(pr,&num[11],x,y,0);
			break;
		case 'c':
		case 'C':
			paste_bmpimg_semi2(pr,&num[12],x,y,0);
			break;
		case 'd':
		case 'D':
			paste_bmpimg_semi2(pr,&num[13],x,y,0);
			break;
		case 'e':
		case 'E':
			paste_bmpimg_semi2(pr,&num[14],x,y,0);
			break;
		case 'f':
		case 'F':
			paste_bmpimg_semi2(pr,&num[15],x,y,0);
			break;
		default:
			printf("default in print_number\n");
			break;
		}
		x+=4;
		i++;
	}
}

/*
	画像を上下反転させる
*/

void RECT2UpsideDown( RECT2 *prect )
{
	void *buff;
	int j;
	buff = (void *)Malloczero( prect->pitch * prect->h);
	memcpy(buff, prect->pimg, prect->pitch * prect->h);

	for(j=0; j<prect->h; j++){
		memcpy(prect->pimg + j * prect->pitch, buff + (prect->h-1-j) * prect->pitch, prect->pitch);

	}
	Free(buff);
}


/*
	RECT2オブジェクト作成
	byte:	1ピクセルあたりのバイト数 (24bit==3, 32bit==4)

*/
void RECT2Create(RECT2 *prect, int w, int h, int byte)
{
	ASSERT(w > 0);
	ASSERT(h > 0);

	memset( prect, 0, sizeof(RECT2) );
	prect->x = 0;
	prect->y = 0;
	prect->w = w;
	prect->h = h;
	prect->pitch = w * byte;
	prect->pitch = ( prect->pitch + 3 ) & ~3;
	prect->byte  = byte;
	prect->pimg=(u_char*)Malloczero( prect->pitch * h);
	prect->buffer_size = prect->pitch * h;
}

/*
	RGBが(0,0,0)ならアルファを強制的に0にする。MGS2X用。
*/

int RECT2PenetratetionCheckRGBA(RECT2 *prect)
{
	int i,j,flg=0;
	RGBA *prgb;
	for(j=0; j < prect->h; j++){
		for(i=0; i< prect->w; i++){
			prgb=(RGBA*)(prect->pimg + prect->pitch*j + prect->byte*i);
			
			if( !prgb->b && !prgb->g && !prgb->r ){
				prgb->a=0;
				flg=1;
				//printf(".");printf("%x",prgb->a);
			}
		}
	}
	return(flg);
}







