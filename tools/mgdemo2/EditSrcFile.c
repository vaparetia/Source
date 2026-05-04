/******************************************************************************
 * System	: DemoAct
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Kelnel
 */

/******************************************************************************
 * included
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef _WINDOWS
#include <windows.h>
#include <SFDWIN.h>
#endif
#ifdef _UNIX
#include <SFDUNIX.h>
#endif
#ifdef _LINUX
#include <SFDLINUX.h>
#endif

#include "Kernel.h"



/******************************************************************************
 * defines
 */
#define KEYWORDCOUNT   30
#define GETPATHCOUNT   20
#define MAXDEMOTMPFILE 30


/******************************************************************************
 * functions
 */
static int GetModelFileList(char* lpszListFilePath, char* lppcModelList[]);
static int DeleteDemoTmp(char* lpszDemoTmpFileName);
static int GetFilePathList(char* lpszSrcFilePath, char* lpszFixPath, char* lppcFilePathList[], char* lpszDemoTmpFile, int nTmpCount);
static int CheckModelFileList(char* lppcModelList[], char* lppcFilePathList[]);
static int CheckModelFile(char* lpszModelFileName, char* lppcFilePath);
static int MakeFilePath(char* lppszModelFileName[], char* lppszFilePath[], char* lpszFixPath);
static int GetFixFilePath(char* lpszModelFileName, char* lpszFixPath, char* lpszSearchFilePath, char* lppszGetPathList[]);
static int CheckCacheFile(char* lpszListDirectory, char* lpcModelList[]);
static void CheckCacheModel(char* lpcModelList[], char* lpcCacheModelList[]);

/******************************************************************************
 * publics
 */
#define MODELCOUNT      100
#define FILEPATHCOUNT   100
#define REG_MODEL_COUNT 100

char* lpszCheckBeginCodeSrc = "# Add Demo Model List by mgdemo. by ";

char* lpszCheckEndCodeSrc = "# End Demo Model List.\n";

char* lpszFixPath = "/u/develop/mj001data/fix";

char* lpszCheckModelCodeList = "/* Demo Model List\n";

char* lpszDemoTmpFileName = "DemoTmp\0";
/******************************************************************************
 * srcファイルを編集する
 */
int EditSrcFile(			/* 0 : 異常 */
	char* lpszListFilePath, /* リストファイルへのパス */
	char* lpszSrcFilePath,	/* srcファイルへのパス */
	char* lpszSrcFileName,	/* 拡張子なしのsrc ファイル名 */
	char* lpszListDirectory)	/* list ディレクトリへのパス */
{
	int   i;
	int   nSize;
	int   nFileSize;
	long  lPointer;
	char  szString[_MAX_TEXT + 1];
	char  szNewFileName[_MAX_TEXT + 1];
	char  szLineString[_MAX_TEXT + 1];
	char  szDemoTmpFile[_MAX_TEXT];
	char  szStartKeyword[_MAX_TEXT];
	char  szListFileName[_MAX_TEXT];
	char* lppcModelList[MODELCOUNT];
	char* lppcFilePathList[FILEPATHCOUNT];
	FILE* fFile;
	FILE* fTmpFile;
#if 0
	/* リストファイルパスからステージ名＋ファイルネームを取り出す */
	for(i = (strlen(lpszListFilePath) - 1); i >= 0; i--)
	{
		if(*(lpszListFilePath + i) == '/')
			break;
	}
	for(i-- ; i >= 0; i--)
	{
		if(*(lpszListFilePath + i) == '/')
			break;
	}
	sprintf(szListFileName, "%s\0", (lpszListFilePath + i + 1));

	/* モデルファイルリストの初期化 */
	for(i = 0; i < MODELCOUNT; i++)
		lppcModelList[i] = NULL;

	/* ファイルパスリストの初期化 */
	for(i = 0; i < FILEPATHCOUNT; i++)
		lppcFilePathList[i] = NULL;

	/* リストファイルからデモで使用するモデルファイル名を獲得 */
	if(GetModelFileList(lpszListFilePath, lppcModelList) == 0)
		return 0;

	/* 取得したモデルファイルから .cache ファイルにあるモデルははずす */
	if(CheckCacheFile(lpszListDirectory, lppcModelList) == 0)
		return 0;

	/* 追加するモデルがないのなら終了 */
	if(lppcModelList[0] == NULL)
		return 1;

	/* mgdemo が作成する src 修正用テンポラリファイルを削除する */
	sprintf(szDemoTmpFile, "%s/%s", lpszListDirectory, lpszDemoTmpFileName);
	if(DeleteDemoTmp(szDemoTmpFile) == 0)
		return 0;

	/* 既存の src ファイルからすでに通っているパスを得る */
	/* lppcFilePathList にはフルパスで入っている */
	if(GetFilePathList(lpszSrcFilePath, lpszFixPath,
		lppcFilePathList, szDemoTmpFile, 0) == 0)
		return 0;

	/* mgdemo が作成する src 修正用テンポラリファイルを削除する */
	if(DeleteDemoTmp(szDemoTmpFile) == 0)
		return 0;

	/* パス先に、追加するモデルファイルがあるかどうか調べる */
	/* 組み込み済みのモデルであればリストから削除する */
	/* 注意！ mdlファイルではなく、kms、evm ファイルである可能性に留意すること */
	if(CheckModelFileList(lppcModelList, lppcFilePathList) == 0)
	{
		printf("Error CheckModelFileList\n");
		return 0;
	}

	/* すべてのモデルファイルが組み込み済みだったか */
	if(lppcModelList[0] == NULL)
		return 1;

	/* まだモデルファイルが残っているのなら追加する */
	printf("\nAdd New Model.\n");
	for(i = 0; lppcModelList[i] != NULL; i++)
		printf("%d : %s\n", i, lppcModelList[i]);

	/* lppcFilePathList を初期化する */
	for(i = 0; i < FILEPATHCOUNT; i++)
	{
		if(lppcFilePathList[i] != NULL)
			freeEx(lppcFilePathList[i]);
		lppcFilePathList[i] = NULL;
	}

	/* モデルファイルのあるディレクトリを検索する */
	if(MakeFilePath(lppcModelList, lppcFilePathList, lpszFixPath) == 0)
		return 0;

	/* もしもリストがないなら終了 */
	if(lppcFilePathList[0] == NULL)
		return 1;

	/* この段階で lppcModelList と lppcFilePathList は一対一に対応している */
	/* src ファイルを開く */
	if((fFile = fopen(lpszSrcFilePath, "r")) == NULL)
	{
		printf("Error! Can\'t open src file.(%s)\n", lpszSrcFilePath);
		return 0;
	}

	/* テンポラリファイルを開く */
	if((fTmpFile = tmpfile()) == NULL)
	{
		printf("Error! Can\'t create tmp file.\n");
		return 0;
	}

	/* src ファイルをテンポラリファイルにコピーする */
	while(fgets(szLineString, _MAX_TEXT, fFile) != NULL)
	{
		/* テンポラリファイルに書き出す */
		if(fputs(szLineString, fTmpFile) < 0)
		{
			printf("Error! Don\'t write tmp file.\n");
			return 0;
		}
	}

	/* スタートキーワードを書き出す */
	sprintf(szStartKeyword, "%s%s\n", lpszCheckBeginCodeSrc, szListFileName);
	if(fputs(szStartKeyword, fTmpFile) < 0)
	{
		printf("Error! Don\'t write tmp file.\n");
		return 0;
	}

	/* エンドキーワードを見つけるまで書き出す */
	while(fgets(szLineString, _MAX_TEXT, fFile) != NULL)
	{
		/* キーワード検索 */
		if(strstr(szLineString, lpszCheckEndCodeSrc) != NULL)
			break;

		/* テンポラリファイルに書き出す */
		if(fputs(szLineString, fTmpFile) < 0)
		{
			printf("Error! Don\'t write tmp file.\n");
			return 0;
		}
	}

	/* 新規に追加するモデルのリストを書き出す */
	for(i = 0; lppcFilePathList[i] != NULL; i++)
	{
		/* テンポラリファイルに書き出す */
		if(fputs(lppcFilePathList[i], fTmpFile) < 0)
		{
			printf("Error! Don\'t write tmp file.\n");
			return 0;
		}
	}

	/* エンドキーワードを書き出す */
	/* スタートキーワードを書き出す */
	if(fputs(lpszCheckEndCodeSrc, fTmpFile) < 0)
	{
		printf("Error! Don\'t write tmp file.\n");
		return 0;
	}

	/* 残りの部分をテンポラリファイルに書き出す */
	while(fgets(szLineString, _MAX_TEXT, fFile) != NULL)
	{
		/* テンポラリファイルに書き出す */
		if(fputs(szLineString, fTmpFile) < 0)
		{
			printf("Error! Don\'t write tmp file.\n");
			return 0;
		}
	}

	/* 元のファイルは閉じる。改めて中を破壊して開く */
	fclose(fFile);

	fseek(fTmpFile, 0, SEEK_SET);
	lPointer = ftell(fTmpFile);
	fseek(fTmpFile, 0, SEEK_END);
	nFileSize =  ftell(fTmpFile) - lPointer;
	fseek(fTmpFile, 0, SEEK_SET);

	/* srcを新規に作成 */
	if((fFile = fopen(lpszSrcFilePath, "w")) == NULL)
	{
		printf("Error! -1. Can\'t open src file.(%s)\n", lpszSrcFilePath);
		return 0;
	}

	/* テンポラリーファイルの中身をコピーする */
	while(nFileSize != 0)
	{
		nSize = (nFileSize > _MAX_TEXT)? _MAX_TEXT : nFileSize;
		if(fread(szLineString, sizeof(char), nSize, fTmpFile) != nSize)
		{
			printf("Error! Don\'t read tmp file.\n");
			return 0;
		}
		if(fwrite(szLineString, sizeof(char), nSize, fFile) != nSize)
		{
			printf("Error! Don\'t write src file.(%s)\n", lpszSrcFilePath);
			return 0;
		}
		nFileSize -= nSize;
	}
	fclose(fFile);
	fclose(fTmpFile);
#else
	/* replace_src を呼び出して返戻値で make するかどうかを決める */
	sprintf(szString, "replace_src %s", lpszListFilePath);
	switch( i=MgdemoSystem(szString) )
	{
	case -1:
	case 0:
	    return 0 ;
	case 1:
	    return 1 ;
	case 2:
	    break ;
	}
#endif

	/* make をかけて終了 */
	sprintf(szString, "list");
	if(chdir(szString) == -1)
	{
		printf("Error! Don't Change directory.(%s)\n", szString);
		return 0;
	}
	sprintf(szString, "make %s", lpszSrcFileName);
	if(system(szString) != 0)
		return 0;

	sprintf(szString, "..");
	if(chdir(szString) == -1)
	{
		printf("Error! Don't Change directory.(%s)\n", szString);
		return 0;
	}
#if 0
	/* リストの開放 */
	for(i = 0; i < MODELCOUNT; i++)
	{
		if(lppcModelList[i] != NULL)
			freeEx(lppcModelList[i]);
	}
	for(i = 0; i < FILEPATHCOUNT; i++)
	{
		if(lppcFilePathList[i] != NULL)
			freeEx(lppcFilePathList[i]);
	}
#endif

	return 2;
}

/******************************************************************************
 * コマンドを実行するシステム関数
 */
#include <errno.h>
#include <sys/wait.h>

int MgdemoSystem( char *command )
{
    int pid, status;

    if ( command == NULL )
	return 1 ;
    pid = fork();
    if ( pid == -1 )
	return -1 ;
    if ( pid == 0  )
    {
	char *argv[4] = { "sh", "-c", command, NULL } ;
	extern char **environ ;

	execve( "/bin/sh", argv, environ ) ;
	exit( 127 ) ;
    }
    while( 1 )
	if ( waitpid(pid, &status, 0) == -1 )
	{
	    if ( errno != EINTR )
		return -1 ;
	}
	else
	    return  WEXITSTATUS( status ) ;
}

/******************************************************************************
 * 文字列から空白文字を削除する
 */
static void FixString(		/* 0 : 異常 */
	char* lpszString)		/* 空白文字を削除する文字列 */
{
	int   i;
	int   nLength;
	char* lpszFixString;

	/* 空白文字を削除する */
	nLength = strlen(lpszString);
	lpszFixString = (char*)callocEx(nLength + 1, sizeof(char));
	for(i = 0; i < nLength; i++)
	{
		/* 空白文字なら非零 */
		if(isspace(*(lpszString + i))  != 0)
			continue;
		
		*(lpszFixString + i) = *(lpszString + i);
	}
	/* 終了文字を入れる */
	*(lpszFixString + i) = '\0';
	/* 書きもどす */
	strcpy(lpszString, lpszFixString);
	freeEx(lpszFixString);
}

/******************************************************************************
 * リストファイルからデモで使用するモデルファイルを得る
 */
static int GetModelFileList(			/* 0 : 異常 */
	char* lpszListFilePath,		/* リストファイルのパス */
	char* lppcModelList[])		/* 発見したモデルファイル名格納する */
{
	int  i;
	int  nLength1;
	int  nLength2;
	char szLineString[_MAX_TEXT];
	FILE *fFile;

	/* リストファイルが開くかチェック */
	if((fFile = fopen(lpszListFilePath, "r")) == NULL)
	{
		printf("Error! Can\'t open list file.(%s)\n", lpszListFilePath);
		return 0;
	}

	/* リストファイルからキーワードを検索 */
	/* "Demo Model List" をキーとする */
	while(fgets(szLineString, _MAX_TEXT, fFile) != NULL)
	{
		/* キーワード検索 */
		if(strncmp(szLineString, lpszCheckModelCodeList, 18) == 0)
			break;
	}

	/* キーワード内にあるモデル名を lppcModelList に格納 */
	while(fgets(szLineString, _MAX_TEXT, fFile) != NULL)
	{
		/* 閉じるマークが着たら終了 */
		if(strncmp(szLineString, "*/", 2) == 0)
			break;

		/* 空白文字を削除する */
		FixString(szLineString);
		nLength1 = strlen(szLineString);
		for(i = 0; lppcModelList[i] != NULL; i++)
		{
			nLength2 = strlen(lppcModelList[i]);
			if((nLength1 == nLength2) && (strncmp(szLineString,
				lppcModelList[i], strlen(lppcModelList[i])) == 0))
				break;
		}
		/* NULLでないということは同一モデル名が登録されていた */
		if(lppcModelList[i] != NULL)
			continue;

		/* リストに登録 */
		lppcModelList[i] = (char*)callocEx(strlen(szLineString) + 1,
			sizeof(char));
		strcpy(lppcModelList[i], szLineString);
	}

	/* ファイルを閉じて終了 */
	fclose(fFile);
	return 1;
}

/******************************************************************************
 * mgdemoが作成した、src編集時のテンポラリファイルを削除する
 */
static int DeleteDemoTmp(			/* 0 : 異常 */
	char* lpszDemoTmpFileName)		/* mgdemo テンポラリファイルの名称 */
{
	int  i;
	char szTmpFileName[_MAX_TEXT];

	for(i = 0; i < MAXDEMOTMPFILE; i++)
	{
		sprintf(szTmpFileName, "%s%d.txt", lpszDemoTmpFileName, i);
		// 存在しなかったら削除
		if(CheckFile(szTmpFileName, 0) == 0) break;
		if(remove(szTmpFileName) != 0) return 0;
	}
	return 1;
}

/******************************************************************************
 * 既存の src ファイルからすでに通っているパスを得る
 */
static int GetFilePathList(			/* 0 : 異常 */
	char* lpszSrcFilePath,			/* srcファイルのパス */
	char* lpszFixPath,				/* モデルの fix のパス */
	char* lppszFilePathList[],		/* 通っているパスのリスト */
	char* lpszDemoTmpFile,			/* src編集用テンポラリファイル名 */
	int   nTmpCount)				/* テンポラリファイル数 */
{
	int   i;
	int   j;
	int   nNum;
	int   nLength;
	int   nKeyWordCount;
	char  szWord[_MAX_TEXT];
	char  szString[_MAX_TEXT];
	char  szLineString[_MAX_TEXT];
	char  szDirectoryPath[_MAX_TEXT];
	char  szTmpFile[_MAX_TEXT];
	char* lpcChar;
	char* lpcChar2;
	char* lppcKeyWordList[KEYWORDCOUNT];

	DIR*  lpDir;
	FILE* fFile;
	struct dirent* lpDirent;
	struct stat    statBuffer;

	/* リストの初期化 */
	for(i = 0; i < KEYWORDCOUNT; i++)
		lppcKeyWordList[i] = NULL;

	nKeyWordCount = 0;

	/* src ファイルあるのディレクトリへのパス */
	strcpy(szDirectoryPath, lpszSrcFilePath);
	nLength = strlen(szDirectoryPath);
	for(i = nLength - 1; i >= 0; i--)
	{
		if(szDirectoryPath[i] == '/')
		{
			/* ファイル名の部分は要らない */
			szDirectoryPath[i] = '\0';
			break;
		}
	}

	/* fix ディレクトリからキーワードを得る */
	/* ここで言うキーワードとは fix 以下にあるディレクトリ名である */
	/* fix のディレクトリを開く */
	if((lpDir = opendir(lpszFixPath)) == NULL)
		return 0;

	/* fix ディレクトリ内のディレクトリをキーワードとして獲得 */
	while((lpDirent = readdir(lpDir)) != NULL)
	{
		/* カレントと親のディレクトリは飛ばす */
		if((strcmp(lpDirent->d_name, ".") == 0) || (strcmp(lpDirent->d_name, "..") == 0))
			continue;

		sprintf(szString, "%s/%s\0", lpszFixPath, lpDirent->d_name);
		stat(szString, &statBuffer);

		if(S_ISDIR(statBuffer.st_mode))
		{
			/* ディレクトリを発見。キーワードリストに追加する */
			nLength = strlen(lpDirent->d_name);
			lppcKeyWordList[nKeyWordCount] = (char*)callocEx(nLength + 1, sizeof(char));
			strcpy(lppcKeyWordList[nKeyWordCount], lpDirent->d_name);
			nKeyWordCount++;
		}
	}
	closedir(lpDir);

	/* ifdef に対応するためにcpp.exeにかませる */
	sprintf(szTmpFile, "%s%d.txt", lpszDemoTmpFile, nTmpCount);
	sprintf(szString, "cpp %s -o %s\n", lpszSrcFilePath, szTmpFile);
	if(system(szString) != 0)
		return 0;

	/* src ファイルを開く */
	if((fFile = fopen(szTmpFile, "r")) == NULL)
	{
		printf("Error! Can\'t open src file.(%s)\n", lpszSrcFilePath);
		return 0;
	}

	/* リストファイルからキーワードを検索 */
	while(fgets(szLineString, _MAX_TEXT, fFile) != NULL)
	{
		lpcChar = szLineString;

		while(lpcChar != '\0')
		{
			if(isspace((int)(*lpcChar)) != 0)		/* 空白文字は飛ばす */
				lpcChar++;
			else
				break;
		}

		if(*lpcChar == '\0')	/* 空行は飛ばす(改行文字も空白文字の一つ) */
			continue;

		if(*lpcChar == '#')		/* コメント行は飛ばす */
			continue;

		lpcChar2 = lpcChar;

		/* インクルード文は再帰する */
		if(strncmp(lpcChar, "include", 7) == 0)
		{
			/* インクルードの文字を飛ばす */
			while(*lpcChar != '\0')
			{
				if(isspace((int)(*lpcChar)) == 0)	/* 空白文字まで飛ばす */
					lpcChar++;
				else
					break;
			}
			/* include の引数を得たいので空白を飛ばす */
			while(*lpcChar != '\0')
			{
				if(isspace((int)(*lpcChar)) != 0)	/* 空白文字を飛ばす */
					lpcChar++;
				else
					break;
			}
			sprintf(szString, "%s/%s\0", szDirectoryPath, lpcChar);
			/* 最後に終端文字を入れる */
			lpcChar = szString;
			while(*lpcChar != '\0')
			{
				if(isspace((int)(*lpcChar)) == 0)	/* 空白文字まで飛ばす */
					lpcChar++;
				else
					break;
			}
			*lpcChar = '\0';

			GetFilePathList(szString, lpszFixPath,
				lppszFilePathList, lpszDemoTmpFile, (nTmpCount + 1));
			continue;
		}

		/* キーワードとマッチング */
		for(i = 0; i < nKeyWordCount; i++)
		{
			lpcChar = lpcChar2;
			if(strncmp(lppcKeyWordList[i], lpcChar, strlen(lppcKeyWordList[i])) == 0)
			{
				/* キーワード発見 : ただしその次の文字が空白文字でないとダメ */
				if(isspace((int)(*(lpcChar + strlen(lppcKeyWordList[i])))) == 0)
					continue;

				/* キーワードを飛ばす */
				while(*lpcChar != '\0')
				{
					if(isspace((int)(*lpcChar)) == 0)	/* 空白文字まで飛ばす */
						lpcChar++;
					else
						break;
				}
				/* 引数まで飛ばす */
				while(*lpcChar != '\0')
				{
					if(isspace((int)(*lpcChar)) != 0)	/* 空白文字を飛ばす */
						lpcChar++;
					else
						break;
				}

				do
				{
					/* 複数個記述されている可能性があるので調べる */
					/* 一単語取り出す */
					for(j = 0;		/* 空白文字まで飛ばす */
						(isspace((int)(*lpcChar)) == 0) || (*lpcChar == '\0'); j++)
					{
						szWord[j] = *lpcChar;
						lpcChar++;
					}
					szWord[j] = '\0';

					/* 追加する */
					/* パスの作成 */
					sprintf(szString, "%s/%s/%s\0", lpszFixPath,
						lppcKeyWordList[i], szWord);

					/* リストにすでに追加されているかチェック */
					nLength = strlen(szString);
					for(j = 0; lppszFilePathList[j] != NULL; j++)
					{
						if((nLength == strlen(lppszFilePathList[j])) &&
							strncmp(lppszFilePathList[j], szString, strlen(szString)) == 0)
							break;
					}
					if(lppszFilePathList[j] == NULL)
					{
						/* 追加する */
						nLength = strlen(szString);
						lppszFilePathList[j] = (char*)callocEx(nLength + 1, sizeof(char));
						strcpy(lppszFilePathList[j], szString);
					}

					/* 更なる単語があるか */
					/* 空白文字で、かつ改行や終端文字ではない */
					if((isspace((int)(*lpcChar)) != 0) && (*lpcChar != '\n') &&
						(*lpcChar != '\0'))
						lpcChar++;

					if(*lpcChar == '\\')
					{
						/* 新しい行を読み込む */
						if(fgets(szLineString, _MAX_TEXT, fFile) != NULL)
						{
							lpcChar = szLineString;
							while(*lpcChar != '\0')
							{
								if(isspace((int)(*lpcChar)) != 0)	/* 空白文字を飛ばす */
									lpcChar++;
								else
									break;
							}
							continue;
						}
						else
						{
							fclose(fFile);
							return 1;
						}
					}

					/* 連続したテキストの終了 */
					if((*lpcChar == '\n') || (*lpcChar == '\0'))
						break;
				} while(1);
			}
		}
	}
	fclose(fFile);

	/* キーワードリストを開放 */
	for(i = 0; lppcKeyWordList[i] != NULL; i++)
		freeEx(lppcKeyWordList[i]);

	return 1;
}

/******************************************************************************
 * 検索パスリストとモデルファイルリストをマッチングしてパス先に追加する
 * モデルファイルがあるか調べる
 */
static int CheckModelFileList(		/* 0 : 異常 */
	char* lppcModelList[],		/* リストファイルから得たモデルファイル名 */
	char* lppcFilePathList[])	/* src ファイルから得た検索パスリスト */
{
	int i, j, k;
	int nResult;

	for(i = 0; lppcModelList[i] != NULL; i++)
	{
		for(j = 0; lppcFilePathList[j] != NULL; j++)
		{
			if((nResult = CheckModelFile(lppcModelList[i], lppcFilePathList[j])) == 1)
			{
				/* パス先にモデルファイルの存在を確認 */
				/* モデルファイルの削除を行う */
				freeEx(lppcModelList[i]);
				for(k = i; lppcModelList[k] != NULL; k++)
					lppcModelList[k] = lppcModelList[k + 1];
 				i--;
				break;
			}
			else if(nResult == 0)
			{
				/* ファイル発見できず */
				continue;
			}
			else
			{
				/* エラー！ */
				printf("Error in CheckModelFileList nResult = %d", nResult);
				return 0;
			}
		}
	}

	return 1;
}

/******************************************************************************
 * パス先に追加するモデルファイルがあるか調べる
 */
static int CheckModelFile(	/*  1 : パス先にモデルファイル発見 */
							/*  0 : 発見できず */
							/* -1 : 異常 */
	char* lpszModelFileName,	/* リストファイルから得たモデルファイル名 */
	char* lpszFilePath)			/* src ファイルから得た検索パスリスト */
{
	int  i, j;
	int  nResult;
	int  nLength;
	char szText[_MAX_TEXT];
	char szMDLFileName[_MAX_TEXT];
	char szKMSFileName[_MAX_TEXT];
	char szEVMFileName[_MAX_TEXT];

	DIR*          lpDir;
	struct dirent *dirent;
	struct stat   stbuf;

	/* チェック用のファイル名を作成 */
	nLength = strlen(lpszModelFileName);
	for(i = 0; i < nLength; i++)
	{
		if(*(lpszModelFileName + i) == '.')
			break;

		szMDLFileName[i] = *(lpszModelFileName + i);
		szKMSFileName[i] = *(lpszModelFileName + i);
		szEVMFileName[i] = *(lpszModelFileName + i);
	}
	/* デフォルトは mdl ファイル */
	szMDLFileName[i + 0] = '.';
	szMDLFileName[i + 1] = 'm';
	szMDLFileName[i + 2] = 'd';
	szMDLFileName[i + 3] = 'l';
	szMDLFileName[i + 4] = '\0';
	/* kms ファイル */
	szKMSFileName[i + 0] = '.';
	szKMSFileName[i + 1] = 'k';
	szKMSFileName[i + 2] = 'm';
	szKMSFileName[i + 3] = 's';
	szKMSFileName[i + 4] = '\0';
	/* evm ファイル */
	szEVMFileName[i + 0] = '.';
	szEVMFileName[i + 1] = 'e';
	szEVMFileName[i + 2] = 'v';
	szEVMFileName[i + 3] = 'm';
	szEVMFileName[i + 4] = '\0';

	/* もしも検索パスがファイル込みならば直接比較する */
	stat(lpszFilePath, &stbuf);
	if(S_ISDIR(stbuf.st_mode))
	{
		/* 検索パスリスト先のディレクトリを開く */
		if((lpDir = opendir(lpszFilePath)) == NULL)
			return 0;

		while((dirent = readdir(lpDir)) != NULL)
		{
			if((strcmp(dirent->d_name, ".") == 0) || (strcmp(dirent->d_name, "..") == 0))
				continue;

			/* ディレクトリ内のファイル情報を得る */
			sprintf(szText, "%s/%s\0", lpszFilePath, dirent->d_name);
			stat(szText, &stbuf);

			/* シンボリックリンクは飛ばす */
			if(S_ISLNK(stbuf.st_mode))
				continue;

			/* ディレクトリか否か */
			if(S_ISDIR(stbuf.st_mode))
			{
				/* 先頭がピリオドのディレクトリは検索対象外 */
				if(dirent->d_name[0] == '.')
					continue;

				/* さらにもぐる */
				/* 返り値が０以外なら値を返す */
				if((nResult = CheckModelFile(lpszModelFileName, szText)) != 0)
					return nResult;
				else
					continue;
			}
			else
			{
				/* ファイルである */

				/* 先頭がピリオドのファイルは比較対象外 */
				if(dirent->d_name[0] == '.')
					continue;

				/* ディレクトリ内のファイルと、モデルファイルリストのマッチング */
				nLength = strlen(dirent->d_name);
				if((nLength == strlen(szMDLFileName)) && (strncmp(szMDLFileName,
					dirent->d_name, strlen(szMDLFileName)) == 0))
					return 1;
				if((nLength == strlen(szKMSFileName)) && (strncmp(szKMSFileName,
					dirent->d_name, strlen(szKMSFileName)) == 0))
					return 1;
				if((nLength == strlen(szEVMFileName)) && (strncmp(szEVMFileName,
					dirent->d_name, strlen(szEVMFileName)) == 0))
					return 1;
			}
		}
		closedir(lpDir);
	}
	else
	{
		nLength = strlen(lpszFilePath);
		/* ファイル同士の比較 */
		for(j = nLength - 1; j >= 0; j--)
		{
			if(*(lpszFilePath + j) == '/')
				break;
		}
		strcpy(szText, (lpszFilePath + j + 1));
		nLength = strlen(szText);

		if((nLength == strlen(szMDLFileName)) && (strncmp(szMDLFileName,
			szText, strlen(szMDLFileName)) == 0))
			return 1;
		if((nLength == strlen(szKMSFileName)) && (strncmp(szKMSFileName,
			szText, strlen(szKMSFileName)) == 0))
			return 1;
		if((nLength == strlen(szEVMFileName)) && (strncmp(szEVMFileName,
			szText, strlen(szEVMFileName)) == 0))
			return 1;
	}
	
	return 0;
}

/******************************************************************************
 * 正式な階乗関数ではないので注意すること
 */
static int Power(
	int nBase,
	int nLevel)
{
	int i;
	int nResult;

	nResult = 1;
	for(i = 0; i < nLevel; i++)
		nResult *= nBase;
	return nResult;
}

/******************************************************************************
 * src ファイルに追加するパスを作成する
 */
static int MakeFilePath(	/*  0 : 異常 */
	char* lppszModelFileName[],	/* リストファイルから得たモデルファイル名 */
	char* lppszFilePath[],		/* src ファイルから得た検索パスリスト */
	char* lpszFixPath)
{
	int   i, j, k;
	int   nCount;
	int   nChar;
	int   nLength;
	int   nSelectNumber;
	char  szInputNumber[_MAX_TEXT];
	char* lppszGetPathList[GETPATHCOUNT];

	/* リストの初期化 */
	for(j = 0; j < GETPATHCOUNT; j++)
		lppszGetPathList[j] = NULL;

	for(i = 0; lppszModelFileName[i] != NULL; i++)
	{
		/* 取得リストを開放する */
		for(j = 0; j < GETPATHCOUNT; j++)
		{
			if(lppszGetPathList[j] != NULL)
				freeEx(lppszGetPathList[j]);
			lppszGetPathList[j] = NULL;
		}

		/* fix のパスリストを取得する */
		if(GetFixFilePath(lppszModelFileName[i], lpszFixPath, NULL,
			lppszGetPathList) == 0)
			return 0;

		if(lppszGetPathList[0] == NULL)
		{
			/* fix にモデルファイルを発見できなかった */
			printf("\nError! %s not Found in Fix directory.\n", lppszModelFileName[i]);
			return 0;
		}

		/* パスが１つ以上見つかったら処理を変える */
		if(lppszGetPathList[1] != NULL)
		{
			nSelectNumber = -1;
			nCount = 0;
			szInputNumber[0] = '\0';
			while(nSelectNumber < 0)
			{
				/* パスが２つ以上あるので選択を促す */
				printf("\nWarning! More than one model files found.\n");
				for(j = 0; lppszGetPathList[j] != NULL; j++)
				{
					printf("%d : %s\n", j, lppszGetPathList[j]);
				}
				/* 標準入力 */
				printf("Please Select Number : ");
				while(((nChar = getchar()) != EOF) && (nChar != '\n'))
					szInputNumber[nCount++] = (char)nChar;

				if(nChar == EOF)
				{
					printf("Error! Please run mgdemo once more.\n");
					return 0;
				}
				/* 数字文字列を整数に変換する */
				nSelectNumber = 0;
				for(j = 0; j < nCount; j++)
				{
					if(isdigit((int)szInputNumber[j]) == 0)
					{
						printf("Please Input Number.\n");
						nSelectNumber = -1;
						nCount = 0;
						szInputNumber[0] = '\0';
						break;
					}
					else
						nSelectNumber += (int)((int)(szInputNumber[j] - '0')) * Power(10, j);
				}

				if(nSelectNumber < 0)
				{
					nSelectNumber = -1;
					nCount = 0;
					szInputNumber[0] = '\0';
					continue;
				}

				printf("Select No.%d Model.\n", nSelectNumber);
			}

			/* 追加する */
			nLength = strlen(lppszGetPathList[nSelectNumber]);
			for(k = 0; k < nLength; k++)
			{
				if(lppszGetPathList[nSelectNumber][k] == '/')
				{
					lppszGetPathList[nSelectNumber][k] = '\t';
					break;
				}
			}

			for(j = 0; lppszFilePath[j] != NULL; j++) ;
			lppszFilePath[j] = (char*)callocEx(nLength + 2, sizeof(char));
			strcpy(lppszFilePath[j], lppszGetPathList[nSelectNumber]);
			lppszFilePath[j][nLength]     = '\n';
			lppszFilePath[j][nLength + 1] = '\0';
		}
		else
		{
			/* パスが１つしかなかったのでこれを追加する */
			/* src ファイルにそのまま書き込めるようにしておくこと */
			/* 追加する */
			nLength = strlen(lppszGetPathList[0]);
			for(k = 0; k < nLength; k++)
			{
				if(lppszGetPathList[0][k] == '/')
				{
					lppszGetPathList[0][k] = '\t';
					break;
				}
			}

			for(j = 0; lppszFilePath[j] != NULL; j++) ;
			lppszFilePath[j] = (char*)callocEx(nLength + 2, sizeof(char));
			strcpy(lppszFilePath[j], lppszGetPathList[0]);
			lppszFilePath[j][nLength]     = '\n';
			lppszFilePath[j][nLength + 1] = '\0';
		}
	}

	for(j = 0; j < GETPATHCOUNT; j++)
	{
		if(lppszGetPathList[j] != NULL)
			freeEx(lppszGetPathList[j]);
	}

	return 1;
}

/******************************************************************************
 * fix からモデルファイルを探し出す
 */
static int GetFixFilePath(	/*  0 : 異常 */
	char* lpszModelFileName,	/* モデルファイル名 */
	char* lpszFixPath,			/* fix パス */
	char* lpszSearchFilePath,		/* 検索パス:このパスと fix パスを調べていく */
	char* lppszGetPathList[])	/* 発見したパス */
{

	int  i, j;
	int  nLength;
	char szString[_MAX_TEXT];
	char szDirectory[_MAX_TEXT];
	char szSearchDirectory[_MAX_TEXT];
	char szMDLFileName[_MAX_TEXT];
	char szKMSFileName[_MAX_TEXT];
	char szEVMFileName[_MAX_TEXT];

	DIR*          lpDir;
	struct dirent *dirent;
	struct stat   stbuf;

	/* チェック用のファイル名を作成 */
	nLength = strlen(lpszModelFileName);
	for(i = 0; i < nLength; i++)
	{
		if(*(lpszModelFileName + i) == '.')
			break;

		szMDLFileName[i] = *(lpszModelFileName + i);
		szKMSFileName[i] = *(lpszModelFileName + i);
		szEVMFileName[i] = *(lpszModelFileName + i);
	}

	/* デフォルトは mdl ファイル */
	szMDLFileName[i + 0] = '.';
	szMDLFileName[i + 1] = 'm';
	szMDLFileName[i + 2] = 'd';
	szMDLFileName[i + 3] = 'l';
	szMDLFileName[i + 4] = '\0';
	/* kms ファイル */
	szKMSFileName[i + 0] = '.';
	szKMSFileName[i + 1] = 'k';
	szKMSFileName[i + 2] = 'm';
	szKMSFileName[i + 3] = 's';
	szKMSFileName[i + 4] = '\0';
	/* evm ファイル */
	szEVMFileName[i + 0] = '.';
	szEVMFileName[i + 1] = 'e';
	szEVMFileName[i + 2] = 'v';
	szEVMFileName[i + 3] = 'm';
	szEVMFileName[i + 4] = '\0';

	/* ディレクトリ作成 */
	if(lpszSearchFilePath != NULL)
		sprintf(szDirectory, "%s/%s\0", lpszFixPath, lpszSearchFilePath);
	else
		sprintf(szDirectory, "%s\0", lpszFixPath);

	/* 検索パスリスト先のディレクトリを開く */
	if((lpDir = opendir(szDirectory)) == NULL)
		return 0;

	while((dirent = readdir(lpDir)) != NULL)
	{
		if((strcmp(dirent->d_name, ".") == 0) || (strcmp(dirent->d_name, "..") == 0))
			continue;

		/* ディレクトリ内のファイル情報を得る */
		sprintf(szString, "%s/%s\0", szDirectory, dirent->d_name);
		stat(szString, &stbuf);

		/* シンボリックリンクは飛ばす */
		if(S_ISLNK(stbuf.st_mode))
			continue;

		/* ディレクトリか否か */
		if(S_ISDIR(stbuf.st_mode))
		{
			/* 先頭がピリオドのディレクトリは検索対象外 */
			if(dirent->d_name[0] == '.')
				continue;

			/* szSearchDirectory に追加 */
			if(lpszSearchFilePath != NULL)
				sprintf(szSearchDirectory, "%s/%s\0", lpszSearchFilePath, dirent->d_name);
			else
				sprintf(szSearchDirectory, "%s\0", dirent->d_name);

			/* さらにもぐる */
			if(GetFixFilePath(lpszModelFileName, lpszFixPath, szSearchDirectory,
				lppszGetPathList) == 0)
				return 0;
		}
		else
		{
			/* ファイルである */

			/* 先頭がピリオドのファイルは比較対象外 */
			if(dirent->d_name[0] == '.')
				continue;

			/* ディレクトリ内のファイルと、モデルファイルリストのマッチング */
			szString[0] = '\0';
			nLength = strlen(dirent->d_name);
			if((nLength == strlen(szMDLFileName)) && (strncmp(szMDLFileName,
				dirent->d_name, strlen(szMDLFileName)) == 0))
			{
				if(lpszSearchFilePath != NULL)
				{
					sprintf(szString, "%s/%s\0", lpszSearchFilePath, szMDLFileName);
				}
				else
				{
					sprintf(szString, "%s\0", szMDLFileName);
				}
			}

			if((nLength == strlen(szKMSFileName)) && (strncmp(szKMSFileName,
				dirent->d_name, strlen(szKMSFileName)) == 0))
			{
				if(lpszSearchFilePath != NULL)
				{
					sprintf(szString, "%s/%s\0", lpszSearchFilePath, szKMSFileName);
				}
				else
				{
					sprintf(szString, "%s\0", szKMSFileName);
				}
			}

			if((nLength == strlen(szEVMFileName)) && (strncmp(szEVMFileName,
				dirent->d_name, strlen(szEVMFileName)) == 0))
			{
				if(lpszSearchFilePath != NULL)
				{
					sprintf(szString, "%s/%s\0", lpszSearchFilePath, szEVMFileName);
				}
				else
				{
					sprintf(szString, "%s\0", szEVMFileName);
				}
			}

			if(szString[0] != '\0')
			{
				/* 追加する */
				nLength = strlen(szString);
				for(i = 0; lppszGetPathList[i] != NULL; i++) ;
				lppszGetPathList[i] = (char*)callocEx(nLength + 1, sizeof(char));
				strcpy(lppszGetPathList[i], szString);
			}
		}
	}
	closedir(lpDir);

	return 1;
}

/******************************************************************************
 * fix からモデルファイルを探し出す
 */
static int CheckCacheFile(
	char* lpszListDirectory,
	char* lpcModelList[])
{
	int   i;
	char  szCacheFilePath[_MAX_TEXT];
	char  szLineString[_MAX_TEXT];
	char* lppcCacheModelList[REG_MODEL_COUNT];
	FILE* fFile;

	/* キャッシュファイルリストを初期化 */
	for(i = 0; i < REG_MODEL_COUNT; i++)
		lppcCacheModelList[i] = NULL;

	/* キャッシュファイルのパス生成 */
	sprintf(szCacheFilePath, "%s/.cache\0", lpszListDirectory); 

	/* キャッシュファイルを開く */
	if((fFile = fopen(szCacheFilePath, "r")) == NULL)
		return 1;

	/* キャッシュファイルが開いたのでモデル情報を取り出す */
	while(fgets(szLineString, _MAX_TEXT, fFile) != NULL)
	{
		if(isspace(szLineString[0]) != 0)		/* 空白文字が先頭にある行は飛ばす */
			continue;

		/* 文字列解析 */
		for(i = 0; i < strlen(szLineString); i++)
		{
			if(szLineString[i] == '.')	/* ピリオドがあるところまで飛ばす */
				break;
		}

		/* もし最後まで言ったのなら追加しない */
		if((i == strlen(szLineString)) || ( (i + 3) > strlen(szLineString)))
			continue;

		/* ３文字比較してモデルファイルかどうか調べる */
		i++;
		if( (strncmp((szLineString + i), "kms", 3) != 0) &&
			(strncmp((szLineString + i), "evm", 3) != 0) )
			continue;

		/* モデルを発見したのでリストに追加する */
		for(i = 0; i < strlen(szLineString); i++)
		{
			if(szLineString[i] == '\n')
			{
				szLineString[i] = '\0';
				break;
			}
		}
		for(i = 0; lppcCacheModelList[i] != NULL; i++);
		lppcCacheModelList[i] = (char*)calloc((strlen(szLineString) + 1), sizeof(char));
		sprintf(lppcCacheModelList[i], "%s\0", szLineString);
	}

	CheckCacheModel(lpcModelList, lppcCacheModelList);

	/* リストを開放 */
	for(i = 0; lppcCacheModelList[i] != NULL; i++)
		free(lppcCacheModelList[i]);

	return 1;
}

/******************************************************************************
 * マッチング
 */
static void CheckCacheModel(	/*  1 : パス先にモデルファイル発見 */
							/*  0 : 発見できず */
							/* -1 : 異常 */
	char* lpcModelList[],
	char* lpcCacheModelList[])
{
	int  i, j, k, l;
	int  nResult;
	int  nLength;
	char szText[_MAX_TEXT];
	char szKMSFileName[_MAX_TEXT];
	char szEVMFileName[_MAX_TEXT];
	char* lpszModelFileName;

	for(i = 0; lpcModelList[i] != NULL; i++)
	{
		lpszModelFileName = lpcModelList[i];

		/* チェック用のファイル名を作成 */
		nLength = strlen(lpszModelFileName);
		for(j = 0; j < nLength; j++)
		{
			if(*(lpszModelFileName + j) == '.')
				break;

			szKMSFileName[j] = *(lpszModelFileName + j);
			szEVMFileName[j] = *(lpszModelFileName + j);
		}

		/* kms ファイル */
		szKMSFileName[j + 0] = '.';
		szKMSFileName[j + 1] = 'k';
		szKMSFileName[j + 2] = 'm';
		szKMSFileName[j + 3] = 's';
		szKMSFileName[j + 4] = '\0';
		/* evm ファイル */
		szEVMFileName[j + 0] = '.';
		szEVMFileName[j + 1] = 'e';
		szEVMFileName[j + 2] = 'v';
		szEVMFileName[j + 3] = 'm';
		szEVMFileName[j + 4] = '\0';

		/* キャッシュファイルから取り出したモデルリストとマッチング */
		for(k = 0; lpcCacheModelList[k] != NULL; k++)
		{
			nLength = strlen(lpcCacheModelList[k]);

			/* マッチするものがあったか */
			if(nLength != strlen(szKMSFileName))
				continue;

			if((strncmp(szKMSFileName, lpcCacheModelList[k], strlen(lpcCacheModelList[k])) != 0) &&
				(strncmp(szEVMFileName, lpcCacheModelList[k], strlen(lpcCacheModelList[k])) != 0))
				continue;

			/* キャッシュファイルにモデルが登録済み */
			free(lpcModelList[i]);
			for(l = i; lpcModelList[l] != NULL; l++)
			{
				/* モデルリストを前詰 */
				lpcModelList[l] = lpcModelList[l + 1];
			}
			i--;
			break;
		}
	}
}
