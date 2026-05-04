/*
	ReadLefPicture.c

   2002/12/11 Nobumitsu Tanaka
   $Id: ReadLefPicture.c,v 1.2 2002/12/11 06:28:37 usr03635 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "func.h"
#include "LESceneStruct20.h"

/* LEFピクチャー部読み込み */
int ReadLefPicture(
	FILE      *fp,
	SceneData *lpSceneData,
	LEFLESCENEHEADER *lpLEFSceneHeader)
{
	int           i;
	char          szName[MAX_TEXT];
	char          szTRIName[MAX_TEXT];
	char          szPath1[MAX_TEXT];
	char          szPath2[MAX_TEXT];
	void         *lpFile;
	Picture      *lpPicture;
	LEFLEPICTURE  lefPicture;

	LEFLEPICTURETREEHEADER lefPictureHeader;

	/* ピクチャーヘッダー読み込み */
	fseek(fp, lpLEFSceneHeader->lLEPictures, SEEK_SET);
	fread(&lefPictureHeader, sizeof(LEFLEPICTURETREEHEADER), 1, fp);

#ifdef DEBUG_DUMP
	printf("\n");
	printf("Picture\n");
	printf("\tFile Count = %d\n", lefPictureHeader.nPictureCount);
#endif

	/* 中間データピクチャー配列作成 */
	if((lpSceneData->lpPicture = (Picture*)Malloc(
		sizeof(Picture) * lefPictureHeader.nPictureCount)) == NULL){
		printf("ReadLefPicture malloc error(0)!\n");
		return 1;
	}

	/* ピクチャーファイル読み込み */
	for(i = 0; i < lefPictureHeader.nPictureCount; i++){
		/* LEFピクチャー情報の読み込み */
		fread(&lefPicture, sizeof(LEFLEPICTURE), 1, fp);
		/* ファイル名読み込み */
		fread(szName, sizeof(char), lefPicture.nNameLength, fp);
		szName[lefPicture.nNameLength] = '\0';
		/* フルパス読み込み */
		fread(szPath1, sizeof(char), lefPicture.nTexPathLength1, fp);
		szPath1[lefPicture.nTexPathLength1] = '\0';
		/* 相対パス読み込み */
		fread(szPath2, sizeof(char), lefPicture.nTexPathLength2, fp);
		szPath2[lefPicture.nTexPathLength2] = '\0';
		/* TRI名読み込み */
		fread(szTRIName, sizeof(char), lefPicture.nTRINameLength, fp);
		szTRIName[lefPicture.nTRINameLength] = '\0';
		/* ファイルデータ読み込み */
		if((lpFile = Malloc(lefPicture.unFileSize)) == NULL){
			printf("ReadLefPicture malloc error(1)!\n");
			return 1;
		}
		fread(lpFile, lefPicture.unFileSize, 1, fp);

		/* 文字列長さチェック */
		if(lefPicture.nNameLength >= MAX_NAME){
			printf("Picture Name length over!\n");
			return 1;
		}
		if(lefPicture.nTRINameLength >= MAX_NAME){
			printf("TRI Name length over!\n");
			return 1;
		}
		if(lefPicture.nTexPathLength1 >= MAX_TEXT){
			printf("Picture Path length over!\n");
			return 1;
		}

		/* データ設定 */
		lpPicture = lpSceneData->lpPicture + i;

		lpPicture->nYear  = lefPicture.nYear;
		lpPicture->nMonth = lefPicture.nMonth;
		lpPicture->nDay   = lefPicture.nDay;
		lpPicture->nHour  = lefPicture.nHour;
		lpPicture->nMin   = lefPicture.nMin;
		lpPicture->nSec   = lefPicture.nSec;

		lpPicture->nWidth  = lefPicture.nWidth;
		lpPicture->nHeight = lefPicture.nHeight;

		strcpy(lpPicture->szName, szName);
		strcpy(lpPicture->szTRIName, szTRIName);
		strcpy(lpPicture->szPath, szPath1);

		lpPicture->unFileSize = lefPicture.unFileSize;
		lpPicture->lpFile = lpFile;

#ifdef DEBUG_DUMP
		printf("\n");
		printf("\tFile Name = %s\n", lpPicture->szName);
		printf("\tTRI Name  = %s\n", lpPicture->szTRIName);
		printf("\tFile Path = %s\n", lpPicture->szPath);
		printf("\tFile Size = %d\n", lpPicture->unFileSize);
		printf("\tWidth  = %d\n", lpPicture->nWidth);
		printf("\tHeight = %d\n", lpPicture->nHeight);
		printf("\tDate(%d/%d/%d %d.%d.%d)\n", lpPicture->nYear, lpPicture->nMonth, 
			lpPicture->nDay, lpPicture->nHour, lpPicture->nMin, lpPicture->nSec);
#endif

		lpSceneData->unPictureCount++;
	}

	return 0;
}

