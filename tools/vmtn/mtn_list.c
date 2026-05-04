/*
  音素とモーションの関連を、ファイルから読み込む
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _mtn_list_c_
#include "mtn_list.h"
#include "mem_util.h"

#ifdef _DEBUG_
#define DBG(args...)  fprintf(stderr, args); fflush(stderr)
#else
#define DBG(args...)
#endif /* _DEBUG_ */



static int skip_blank(char * line, int idx)
{
  while(line[idx] == ' ' || line[idx] == '\t') idx++;
  return idx;
}

static int read_float(float * ret, char * line, int idx)
{
  float f_tmp, a, b, c;
  int j;

  idx = skip_blank(line, idx);

  f_tmp = 0.0F;
  a = 10.0F, b = 1.0F, c = 1.0F;
  for(j = 0; line[idx + j] &&
	line[idx + j] != ' ' &&
	line[idx + j] != '\t'; j++)
    {
      if(line[idx + j] == '.')
	{
	  a = 1.0F;
	  b = c = 0.1F;
	  continue;
	}
      f_tmp *= a;
      f_tmp += (float)(line[idx + j] - '0') * b;
      b *= c;
    }
  *ret = f_tmp;
  return idx + j;
}

static int read_voice_set(mtn_cel * cel, char * line, int idx)
{
  int i, id;
  static struct {
    char *label;
    int id;
  } list[] = {
    { "end", -1}, { "sil", 0 }, { "s",   0 }, { "a",   1 }, { "i",   2 },
    { "u",   3 }, { "e",   4 }, { "o",   5 }, { "N",   6 }, { "v",   7 },
    { NULL, -1 }
  };

  /* 空白を読み飛ばす */
  idx = skip_blank(line, idx);

  /* 前半を読む */
  for(i = 0; list[i].label != NULL; i++)
    if(!strncmp(list[i].label, line + idx, strlen(list[i].label)))
      {
	cel->mtn_a = list[i].id;
	DBG("mtn_a = %d\n", cel->mtn_a);
	idx += strlen(list[i].label);
	break;
      }
  if(list[i].label == NULL) return -2;

  if(cel->mtn_a < 0) return -1;   /* end コードの場合 */

  if(line[idx] != ':') return -2;
  idx++;

  /* 後半を読む */
  for(i = 0; list[i].label != NULL; i++)
    if(!strncmp(list[i].label, line + idx, strlen(list[i].label)))
      {
	cel->mtn_b = list[i].id;
	DBG("mtn_b = %d\n", cel->mtn_b);
	idx += strlen(list[i].label);
	break;
      }
  if(list[i].label == NULL) return -2;

  return idx;
}

/*
 * 既にオープンされているストリームから、
 * モーションブレンド情報を読み込む
 */
static lipMotionList * lip_load_motion_list_by_stream(FILE *rfp)
{
  char line[ LINE_MAX ];
  lipMotionList * list;
  mtn_voice * mv;
  void * vpt;
  int i, j, cnt, step;
  int vowel;

  /* 全言語で共通な、無音部音素を作成する */
  cnt = step = 0;
  if(NULL == (vpt = mem_add(mv, sizeof(mtn_voice), cnt))) return NULL;
  mv = vpt;
  mv[0].vid = 0;
  mv[0].vow_flg = 0l;
  strcpy(mv[0].label, "sil");

  mv[0].mtn[0].time  = 0.0F;
  mv[0].mtn[0].rate  = 0.0F;
  mv[0].mtn[0].mtn_a = 0; 
  mv[0].mtn[0].mtn_b = 0;

  mv[0].mtn[1].time  = 1.0F;
  mv[0].mtn[1].rate  = 0.0F;
  mv[0].mtn[1].mtn_a = -1; 
  mv[0].mtn[1].mtn_b = 0;

  cnt++;

  while(NULL != fgets(line, LINE_MAX, rfp))
    {
      /*
       * 注釈を除去する
       */
      /* 先頭から検索し、最初に見付けた '#' 以降を注釈とみなす */
      /* ついでなので、改行コードも除去 */
      for(i = 0; line[i]; i++)
	if(line[i] == '#' || line[i] == '\n')
	  {
	    line[i] = 0;
	    break;
	  }
      /* 空白以外の最初の文字を探す */
      for(i = 0; line[i]; i++)
	if(line[i] != ' ' && line[i] != '\t') break;
      if(!line[i]) continue;  /* 行に文字が無ければ、次の行を処理 */

      DBG("line: [%s]\n", line);
      /*
       * 音素記号で開始されていれば、音素に対応する
       * モーションシーケンス情報領域を新たに確保する 
       */
      vowel = 0;
      if(line[i] == 'v' && line[i + 1] == ':')
	{
	  vowel = 1;
	  i += 2;
	}
      if(line[i] == '/')
	{
	  DBG("New Voice(%d)\n", cnt);
	  i++;

	  if(NULL == (vpt = mem_add(mv, sizeof(mtn_voice), cnt))) break;
	  mv = vpt;

	  mv[cnt].vid = cnt;
	  mv[cnt].vow_flg = vowel;
	  for(j = 0; line[j + i] && line[j + i] != '/'; j++)
	    mv[cnt].label[j] = line[j + i];
	  mv[cnt].label[j] = 0;
	  step = 0;
	  i += 1 + j;  /* '/' の直後 */
	}
      
      /*
       * 時系列とモーション組み合わせ情報を読み込む
       */
      DBG("[1]\n");
      /* 正規化時系列読み込み */
      i = read_float(&mv[cnt].mtn[step].time, line, i);
      
      DBG("[2]\n");
      /* 組み合わせ読み込み */
      i = read_voice_set(&mv[cnt].mtn[step], line, i);
      
      DBG("[3]\n");
      if(i < 0)   /* 負の値が返ってきたら音素シーケンスの終端とみなす */
	{
	  if(i != -1)  /* 負の値が -1 以外であればエラー */
	    {
	      if(cnt)free(mv);
	      return NULL;
	    }
	  DBG("----------\n");
	  cnt++;
	  continue;
	}

      DBG("[4]\n");

      /* ブレンドレート読み込み */
      i = read_float(&mv[cnt].mtn[step].rate, line, i);
      
      DBG("[5]\n");
	 
      step++;
    }
  
  if(NULL == (list = malloc(sizeof(lipMotionList))))
    {
      free(mv);
      return NULL;
    }

  list->mtn = mv;
  list->voices = cnt;

  return list;
}

lipMotionList * lipLoadMotionList(char * fname)
{
  FILE *rfp;
  lipMotionList * list;

  if(NULL == (rfp = fopen(fname, "r"))) return NULL;
 
  list = lip_load_motion_list_by_stream(rfp);

  fclose(rfp);
  return list;
}

/* モーションブレンドリストを開放する */
void lipReleaseMotionList(lipMotionList * list)
{
  free(list->mtn);
  free(list);
}
