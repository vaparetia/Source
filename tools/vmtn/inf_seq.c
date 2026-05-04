/*
 * inf ファイルを読み、話者情報だけを取り出して、
 * 内部で使用する形式に変換する
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _inf_seq_c_
#include "inf_seq.h"
#include "infdata.h"
#include "code.h"
#include "mem_util.h"

#define VTICK(t)    ( int )( (t) * 100.0 / ( 22050.0 / 1024.0 ) )
#define TICK( t )   ( int )( (t) * 300.0 / ( 22050.0 / 1024.0 ) )

#define BIT_LEN  24

#ifndef _DEBUG_
#define DBG(args...)
#else
#define DBG(args...)    { printf(args);  fflush(stdout); }
#endif

static int cmp_talkarea(const void *va, const void *vb)
{
  TalkArea *a, *b;

  a = va, b = vb;

  if(a->begin > b->begin) return 1;
  if(a->begin < b->begin) return -1;
  return 0;
}

int get_strcode( char * string )
{
  unsigned char c;
  unsigned char *p;
  unsigned int id, mask;
  
  p = ( unsigned char * )string;
  id = 0;
  mask = ( 1 << BIT_LEN ) - 1;
  
  while( ( c = *( p++ ) ) != '\0' ){
    id = ( id << 5 ) | ( id >> (BIT_LEN-5) );
    id += c;
    id &= mask;
  }
  if( id == 0 ) id = 1;
  return id;
}

static int get_int( void *ptr )
{
	unsigned char *buf = ptr;

	return ( buf[ 0 ] << 24 ) | ( buf[ 1 ] << 16 ) | ( buf[ 2 ] << 8 ) | buf[ 3 ];
}

TalkArea * infReadSpeakerByStream(FILE * rfp)
{
  TalkArea * talk;
  void     * vpt;
  int cnt, speaker_cnt;
  INF_BLOCK tag;
  int block_size;
  char * tagstr;
  int current_name_id = -1;
  char * current_name = NULL;

  talk = NULL;
  cnt  = speaker_cnt = 0;

  while(fread(&tag, sizeof(INF_BLOCK), 1, rfp) > 0)
    {
      DBG("***\n");
      block_size = get_int( &tag.block_size );
      tagstr = (char *)&tag.id;
#ifdef _DEBUG_
      {
	int i;
	for(i = 0; i < IDTAG_SIZE; i++) putchar(tagstr[i]);
	printf("\n");
	fflush(stdout);
      }
#endif /* _DEBUG_ */
      if(!strncmp(tagstr, INF_TALK_INF_ID, IDTAG_SIZE))
	/* 話者特定 */
	{
	  void * data;
	  int    tick;

	  INF_TALK_INF *inf;

	  inf = malloc(block_size);
	  data = inf;
	  memcpy(data, &tag, sizeof(INF_BLOCK));
	  fread(data + sizeof(INF_BLOCK), sizeof(char),
		block_size - sizeof(INF_BLOCK), rfp);

	  /* inf.name は、日本語コードが Shift_JIS なので、
	     これを EUC-JP に変換する。 */
	  code_exchange(inf->name);
	  if(NULL != current_name) free(current_name);
	  current_name    = strdup( inf->name );
	  current_name_id = get_strcode( inf->name );
	  DBG("EUC: %s  name_id = %d\n", inf->name, current_name_id);
	  free(inf);
	  speaker_cnt++;
	}
      else if(!strncmp(tagstr, INF_TALK_AREA_ID, IDTAG_SIZE))
	/* 有音部の発話領域情報 */
	{
	  void * data;
	  INF_TALK_AREA area;

	  data = &area;
	  memcpy(data, &tag, sizeof(INF_BLOCK));
	  fread(data + sizeof(INF_BLOCK), sizeof(char),
		block_size - sizeof(INF_BLOCK), rfp);

	  if(NULL == (vpt = mem_add(talk, sizeof(TalkArea), cnt))) goto err;

	  talk = vpt;

	  talk[cnt].begin   = VTICK( get_int(&area.pos) );
	  talk[cnt].speaker = current_name_id;
	  talk[cnt].spname  = strdup(current_name);

	  DBG("%5d: speaker = %s (%d)\n",
	      talk[cnt].begin, talk[cnt].spname, talk[cnt].speaker);

	  cnt++;
	}
#if 0
      else if(!strncmp(tagstr, INF_FILE_HISTS_ID, IDTAG_SIZE))
	/* 無音情報など */
	{
	  
	}
#endif
      else
	{
	  /* 用の無いタグは読み飛ばす */
	  DBG("block_size = %d\n", block_size);
	  /* 読み飛ばしの処理 */
	  fseek(rfp, block_size - sizeof(INF_BLOCK), SEEK_CUR);
	}
      DBG("---\n");
    }
  DBG("@\n");
  /* 話者が一人だけで、なおかつ領域が指定されていない場合は、
     100% その人物が喋っているものとする。 */

  /* 発話領域が設定されていない場合 */
  if(!cnt)
    if(speaker_cnt == 1)   /* 話者が一人だけの場合 */
      {
	/*
	 * 話者が一人だけの場合は、音声の 100% がその話者によるものとする
	 */
	/* 話者が登場していない場合はエラー(理論上あり得ないが念のため) */
	if(current_name_id < 0) goto err;

	if(NULL == (vpt = mem_add(talk, sizeof(TalkArea), cnt))) goto err;
	talk = vpt;
	talk[cnt].begin = 0;  /* 時刻 0 から、その話者の発言が始まる */
	talk[cnt].speaker = current_name_id;
	talk[cnt].spname  = strdup(current_name);
	cnt++;
      }
    else
      /* 話者が複数いる場合、話者がいない場合は、エラーとする */
      goto err;


  DBG("A\n");
  /* 結果を時系列でソートする。 */
  qsort(talk, cnt, sizeof(TalkArea), cmp_talkarea);
  DBG("B\n");

  /* 終了コードを書き込む */
  if(NULL == (vpt = mem_add(talk, sizeof(TalkArea), cnt))) goto err;

  talk = vpt;

  /* 終了ブロックを書き込む */
  talk[cnt].begin   = -1;
  talk[cnt].speaker = -1;
  talk[cnt].spname  = NULL;

#ifdef _DEBUG_
  {
    int i;

    for(i = 0; talk[i].begin >= 0; i++)
      printf("%5d: speaker = %d\n", talk[i].begin, talk[i].speaker);
  }
#endif /* _DEBUG_ */
  if(NULL != current_name) free(current_name);
  return talk;

 err:
  if(talk != NULL) free(talk);

  return NULL;
}

TalkArea * infReadSpeaker(char *fname)
{
  FILE * rfp;
  TalkArea * talk;

  if(NULL == (rfp = fopen(fname, "rb"))) return NULL;

  talk = infReadSpeakerByStream(rfp);
  fclose(rfp);

  return talk;
}

void infReleaseSpeaker(TalkArea * talk)
{
  int i;

  if(NULL == talk) return;

  DBG("talk = %p\n", talk);
  for(i = 0; talk[i].begin >= 0; i++)
    {
      DBG("spname = %s\n", talk[i].spname);
      if(NULL != talk[i].spname) free(talk[i].spname);
    }
  
  free(talk);
}
