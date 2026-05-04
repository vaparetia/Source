/**
	中間データ->OLAファイルのコンバート(UNIX用)
	2002.11.01 Y.Yano
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"

/* -------------------------------------------- */
/* ＬＡファイル */
#define TRI_CNT_MAX 	16
#define ACTION_NUM_MAX 	256
enum {
	LA_SCMD_END		= 0x00000000,/* 状態定義シーケンス終了     	*/
	LA_SCMD_DISP	= 0x10000000,/* 表示状態指定               	*/
	LA_SCMD_TEX		= 0x20000000,/* 使用テクスチャ指定         	*/
	LA_SCMD_VERTEX	= 0x30000000,/* 頂点座標値指定             	*/
	LA_SCMD_RGBA	= 0x40000000,/* 頂点 RGBA値指定            	*/
	LA_SCMD_SIZE	= 0x50000000,/* 表示サイズ指定             	*/
	LA_SCMD_UV		= 0x60000000,/* UV値指定(ID指定方法)       	*/
	LA_SCMD_ADDVERT = 0x70000000,/* 移動量			       		*/
	LA_SCMD_ALPHA	= 0x80000000,/* アルファブレンディング指定 	*/
	LA_SCMD_SCALE	= 0x90000000,/* スケーリング			 	*/
	LA_SCMD_UVF		= 0xd0000000,/* 小数点ＵＶ指定 */
};
enum {
	LA_ACMD_END 		= 0x00000000,/* 終端コード */
	LA_ACMD_SIGNAL		= 0x10000000,/* シグナル */
	LA_ACMD_ACTALL	 	= 0x20000000,/* 汎用アクション呼び出し */
	LA_ACMD_ACTALLEND 	= 0x30000000,/* 汎用アクションの終端コード */
	LA_ACMD_WAIT		= 0x40000000,/* 待機中 */
	LA_ACMD_ACT			= 0x50000000,/* 別アクション呼び出し */
	LA_ACMD_ACTSTOP		= 0x60000000,/* 別アクション終了 */
	LA_ACMD_SET			= 0x80000000,/* 即時変化 */
	LA_ACMD_MORF		= 0xc0000000,/* 指定移動時間変化 */
	//LA_ACMD_LOOP		= 0xd0000000,/* 終端に来たら最初のフレームに戻る */
};

#define BYTEI 4
#define BYTES 2
#define BYTEC 1

typedef struct {
	unsigned int version;
	unsigned int ofObj;
//	unsigned int ofStat;
//	unsigned int ofAct;
//	unsigned int pad;
	unsigned int ofActTbl;
	unsigned int ofTri;
} LAF_HEADER;

typedef struct {
	unsigned int   strcode;
	unsigned short id;
	unsigned short p_id;
	unsigned char  attr;
	unsigned char  v_count;
	unsigned short pad;
} LAF_OBJECT;

/* -------------------------------------------- */
static int StringToStrcode( char *str )
{
	int i = 0;
	int code = 0;

	while( str[ i ] != '\0' ){
		code = ( code << 5 ) | ( code >> (24-5) );/* 24bit Strcodeの場合 */
		code += str[ i ];
		code &= 0x00ffffff;

		i++;
	}

	if( code == 0 ){
		code = 1;
	}

	return code;
}

static void SearchObject( Object *lpObj, LAF_OBJECT *obj, int *i, int p )
{
	while( lpObj != NULL ){		
		obj->strcode 	= StringToStrcode( lpObj->szName );
		obj->p_id 		= p;
		obj->id 		= lpObj->nObjectID = *i;
		obj->attr 		= lpObj->unType;
		obj->v_count 	= lpObj->unDataCount;
		i ++;
		obj ++;
		
		if( lpObj->lpChild != NULL ){
			SearchObject( lpObj->lpChild, obj, i, (*i-1) );
		}
		
		lpObj = lpObj->lpNext;
	}

	return;
}

static void SetInt( void *b, int *n, int dat )
{
	char *buf = b;
	((int*)buf)[ *n ] = dat;
	*n += BYTEI;
	return;
}
static void SetChar( void *b, int *n, int dat )
{
	char *buf = b;
	buf[ *n ] = dat;
	*n += BYTEC;
	return;
}


typedef struct{
	fpos_t pos;
	void *stat;
} Offset;
static Offset stat_pos[1024];
//static fpos_t pos[1024];			/* 状態が書き込まれているファイル位置指示子を記憶 */
static unsigned int stat_cnt = 0;	/* 書き込みが終了した状態数 */
static unsigned int tri_code[ TRI_CNT_MAX ] = {0};/* TRIのstrcode名:全て0で初期化 */
static LAF_HEADER head;

static int CheckStatus( Data *stat, char *buf, int *n )
{
	Vertex 	*vert;
	Color 	*col;
	UV 		*uv;
	int 	j;

	if( stat->nVertexFlag ){
		/* 座標 */
		vert = stat->lpvtVertex;
		SetInt( buf, n, ((stat->unVertexCount << 8) | (LA_SCMD_VERTEX)) );
		for( j = 0; j < stat->unVertexCount; j++ ){					
			SetInt( buf, n, ((vert->y << 16) | vert->x) );
			vert++;
		}
	}
	if( stat->nColorFlag ){
		/* 頂点カラー */
		col = stat->lpclrColor;
		SetInt( buf, n, ((stat->unVertexCount << 8) | (LA_SCMD_RGBA)) );
		for( j = 0; j < stat->unVertexCount; j++ ){
			SetChar( buf, n, col->a );
			SetChar( buf, n, col->b );
			SetChar( buf, n, col->g );
			SetChar( buf, n, col->r );
			col ++;
		}
	}
	if( stat->nViewFlag ){
		/* show,hide */
		SetInt( buf, n, (stat->nView | (LA_SCMD_DISP)) );
	}
	if( stat->nMagniFlag ){
		/* 拡大縮小値 */
		int scale;
		scale = (int)(stat->fMagni * 256.0f);
		SetInt( buf, n, (scale | (LA_SCMD_SCALE)) );
	}
	if( stat->nBlendFlag ){
		/* アルファブレンド */
		/* @TODO 未対応！！ */
	}
	if( stat->nPictureFlag ){
		/* テクスチャ */
		int p_code;
		int t_code;
		int k;
		t_code = StringToStrcode( stat->szTRIName );
		for( k = 0; k < TRI_CNT_MAX; k++ ){
			if( t_code == tri_code[k] ){
				break;
			}
		}
		if( k >= TRI_CNT_MAX ){
			printf("tri error!\n");
			return -1;
		}
		p_code = StringToStrcode( stat->szPictureName );
		SetInt( buf, n, ((k << 24) | p_code | LA_SCMD_TEX) );
	}
	if( stat->nUVFlag ){
		/* ＵＶＦ(floatＵＶ設定) ::: @TODO ＩＤでのＵＶ設定も必要！ */
		int u, v;
		uv = stat->lpUV;
		SetInt( buf, n, ((stat->unVertexCount << 8) | (LA_SCMD_UVF)) );
		for( j = 0; j < stat->unVertexCount; j++ ){
			u = (int)(uv->u * 65535.0f);
			v = (int)(uv->v * 65535.0f);
			SetInt( buf, n, ((v << 16) | u) );
			uv ++;
		}
	}
	if( stat->nRotateFlag ){
		/* 回転 */
		/* @TODO 未対応 */
	}
	if( stat->nRotateModeFlag ){
		/* 回転方向 */
		/* @TODO 未対応 */
	}
	if( stat->nSpinCenterFlag ){
		/* 回転中心 */
		/* @TODO 未対応 */
	}
	
	return 0;
}

static int WriteStatus( Object *lpObj, FILE *laf )
{
	char	*buf;
	int 	i, j, n, cmd_num;
	//int		 stat_code;
	Data 	*stat;

	while( lpObj != NULL ){
		stat = lpObj->lpData;
		n = 0;

		for( i = 0; i < lpObj->unDataCount; i++ ){
			char 	buffer[4096];/* テンポラリ領域 */
			char 	tmp[4096];/* テンポラリ領域 */

			n = 0;
			
			/* データをbufferに書き込んでいく */
			if( CheckStatus( stat, buffer, &n ) < 0 ){
				printf("err!\n");
				return -1;
			}

			/* 他のデータと被っていないかのチェック */
			if( stat_cnt > 0 ){
				for( j = 0; j < stat_cnt; j++ ){
					fflush( laf );
					fsetpos( laf, &(stat_pos[ j ].pos) );
					fread( &cmd_num, sizeof(int), 1, laf );
					//cmd_num = ((cmd_num & 0xff000000) >> 24);
					if( n == cmd_num ){
						fread( tmp, sizeof(char), n , laf );
						if( memcmp( buffer, tmp, n ) == 0 ){
							stat->nOffset = 0;
							stat->lpRefData = stat_pos[ j ].stat;
							break;
						}
					}
				}
				if( j < stat_cnt ){
					stat = stat->lpNext;
					continue;
				}
			}

			if( n > 4096 ){
				printf("設定バイトを超えました\n");
				return -1;
			}
			//stat_code = StringToStrcode( stat->szName );
			//stat_code |= ( n << 24 );/* 設定バイトを入れる */
			fflush( laf );
			/* offsetをセットする */
			stat->nOffset = ftell( laf );
			/* posを記憶 */
			fgetpos( laf, &(stat_pos[ stat_cnt ].pos) );
			stat_pos[ stat_cnt ].stat = stat;
			stat_cnt ++;
			/* 書き込み */
			fwrite( &n, sizeof(int), 1, laf );
			fwrite( buffer, sizeof(char), n, laf );
			
			stat = stat->lpNext;
		}
		
		if( lpObj->lpChild != NULL ){
			/* 再帰で子のチェック */
			if( WriteStatus( lpObj->lpChild, laf ) < 0 ){
				return -1;
			}			
		}

		if( lpObj->lpNext != NULL ){
			lpObj = lpObj->lpNext;
		} else {
			return 0;
		}
	}
	return -1;
}

static void SortAction( int *code, long *pos, int n )
{
	int  i, j;
	int  dmy;
	long ldmy;

	/* codeの昇順でソート */
	for( i = 0; i < (n-1); i ++ ){
		for( j = 0; j < (n-1); j++ ){
			if( code[ j ] > code[ j + 1 ] ){
				dmy = code[ j + 1 ];
				code[ j + 1 ] = code[ j ];
				code[ j ] = dmy;
				/* posの方も入れ替える */
				ldmy = pos[ j + 1 ];
				pos[ j + 1 ] = pos[ j ];
				pos[ j ] = ldmy;
				
			}
		}
	}
	return;
}

/* -------------------------------------------- */
/**
	中間ファイルフォーマットから、LAファイルの変換
*/
int SceneData2Ola( ConvertInfo *info )
{
	SceneData 	*data = &info->scnData;
	char 		*filename = info->szOutputPath;
	FILE 		*laf;
	int  		i, j, k;
	long		action_pos[ ACTION_NUM_MAX ];

printf("start\n");
	
	laf = fopen( filename, "wb+" );
	if( laf == NULL ){
		fprintf(stderr, "LAFファイルオープン失敗です\n");
		return -1;
	}
	/* ヘッダ部分の書き込みは後で行う */
	fseek( laf, sizeof(LAF_HEADER), SEEK_SET );
	
printf("a\n");
	{
		/* ＴＲＩ */
		unsigned int tri_count = 0;
		unsigned int code = 0;
		Picture  *pict = data->lpPicture;

		for( i = 0; i < data->unPictureCount; i++ ){
			code = StringToStrcode( pict->szTRIName );
			for( j = 0; j < TRI_CNT_MAX; j++ ){
				if( tri_code[ j ] == 0 ){
					tri_code[ j ] = code;
					tri_count = (j + 1);
					break;
				} else if( tri_code[ j ] == code ){
					break;
				}
			}
			pict ++;
		}
		/* ＴＲＩ書き込み */
		head.ofTri = ftell( laf );
		if( tri_count > 0 && tri_count < TRI_CNT_MAX ){
			/* ＴＲＩ数の書き込み */
			fwrite( &tri_count, sizeof(int), 1, laf );
			/* strcodeの書き込み */
			fwrite( tri_code, sizeof(int), tri_count, laf );
		}
	}
printf("b\n");
	{
		/* オブジェクト定義 */
		void	*obj;

		/* オブジェクト用メモリ確保 */
		obj = malloc( sizeof(LAF_OBJECT) * data->unObjectCount );

		/* 再帰呼び出しでオブジェクト構造をサーチ */
		i = 0;
		SearchObject( data->lpObject, obj, &i, 0xffff );

		/* オブジェクト定義の書き込み */	 
		/* オブジェクト数の書き込み */
		head.ofObj = ftell( laf );
		fwrite( &(data->unObjectCount), sizeof(int), 1, laf );
		/* オブジェクトの書き込み */
		fwrite( obj, sizeof(LAF_OBJECT), data->unObjectCount, laf );
		
		/* オブジェクト用メモリ開放 */
		free( obj );
	}
printf("e\n");
	{
		/* 状態定義 */
		/* 状態をオプジェクトのツリーに従って書き込む */
		if( WriteStatus( data->lpObject, laf ) < 0 ){
			return -1;
		}
	}
printf("c\n");
	{
		/* アクション定義 */
		Action *act = data->lpAction;
		ActObj *actobj = act->lpActObj;
		Key	   *key = actobj->lpKey;
		int t_num;
		int id;
		int cmd[ 256 ], n, offset, f;

		for( i = 0; i < data->unActionCount; i++ ){
			
			t_num = act->unActObjCount;
			fflush( laf );
			/* アクション毎のオフセット値を記憶 */
			action_pos[ i ] = ftell( laf );
			/* トラック数の書き込み */
			fwrite( &t_num, sizeof(int), 1, laf );
			
			for( j = 0; j < t_num; j++ ){
				n = 0;
				f = 0;
				id = actobj->lpObject->nObjectID;
				key = actobj->lpKey;

				/* キー情報がないオブジェクトはハジく */
				if( actobj->unKeyCount == 0 ){
					actobj ++;
					continue;
				}
				/* オブジェクトのＩＤ書き込み */
				fwrite( &id, sizeof(int), 1, laf );
				/* キー命令の書き込み */
				for( k = 0; k < actobj->unKeyCount; k++ ){
					switch( key[ k ].unType ){
					  case 0:
						/* morf */
						SetInt( cmd, &n, ((key[ k ].nFrame - f) | LA_ACMD_MORF ) );
						offset = key[ k ].lpData->nOffset;
						if( offset == 0 ){
printf("ceee %p %p\n", key[ k ].lpData->lpRefData, key[ k ].lpData);
							/* 参照データのオフセット値を入れる */
							offset = ((Data*)(key[ k ].lpData->lpRefData))->nOffset;
						}
						SetInt( cmd, &n, offset );
						f = key[ k ].nFrame + 1;
						break;
					  case 1:
						/* set */
						SetInt( cmd, &n, key[ k ].nFrame | LA_ACMD_SET );
						offset = key[ k ].lpData->nOffset;
						if( offset == 0 ){
printf("cefe %p %p\n", key[ k ].lpData->lpRefData, key[ k ].lpData);
							/* 参照データのオフセット値を入れる */
							offset = ((Data*)key[ k ].lpData->lpRefData)->nOffset;
						}
						SetInt( cmd, &n, offset );
						break;

						/* ///TODO: この他にact, actobjなども入る */
					}

				}
				/* 終了キー */
				SetInt( cmd, &n, 0 );
				/* アクション内容書き込み */
				fwrite( cmd, sizeof(int), n, laf );
				actobj ++;
			}
			act ++;
		}
	}
printf("d\n");
	{
		/* アクションテーブル */
		/* sortして書き込む */
		Action *act = data->lpAction;
		int 	action_code[ ACTION_NUM_MAX ];
		for( i = 0; i < data->unActionCount; i++ ){
			action_code[ i ] = StringToStrcode( act[ i ].szName );
		}
		SortAction( action_code, action_pos, data->unActionCount );
		head.ofActTbl = ftell( laf );
		fwrite( action_code, sizeof(int), data->unActionCount, laf );
		fwrite( action_pos , sizeof(int), data->unActionCount, laf );
	}

	{
		/* ヘッダ書き込み */
		head.version = 0x00010000;/* 固定小数点(上位16bit整数部:下位16bit小数部) */
		fseek( laf, 0L, SEEK_SET );
		fwrite( &head, sizeof(LAF_HEADER), 1, laf );
	}

	fclose( laf );
printf("f\n");
	return 0;
}
