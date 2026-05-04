/* 指定されたテキストに書かれた文字列を、音素列に変換するモジュール */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _text_c_
#include "util.h"
#include "analyze.h"
#include "text.h"
#include "debug.h"

static txtPhono ** txt_adjust_no_voice_vowel(txtPhono **tx_list);
static txtPhono ** txt_adjust_long_vowel(txtPhono **tx_list);

/* 平仮名、カタカナと、音素列の対応表。
   音素列は音韻的な分類をしているので、必ずしも訓令式もしくはヘボン式の
   ローマ字列とは一致しない。 */
static txtList _txt_ref[] = {
  {"きゃ", "キャ", {"k", "ya", NULL}},
  {"きゅ", "キュ", {"k", "yu", NULL}},
  {"きぇ", "キェ", {"k", "ye", NULL}},
  {"きょ", "キョ", {"k", "yo", NULL}},
  {"しゃ", "シャ", {"s", "ya", NULL}},
  {"しゅ", "シュ", {"s", "yu", NULL}},
  {"しぇ", "シェ", {"s", "ye", NULL}},
  {"しょ", "ショ", {"s", "yo", NULL}},
  {"ちゃ", "チャ", {"c", "ya", NULL}},
  {"ちゅ", "チュ", {"c", "yu", NULL}},
  {"ちぇ", "チェ", {"c", "ye", NULL}},
  {"ちょ", "チョ", {"c", "yo", NULL}},
  {"にゃ", "ニャ", {"n", "ya", NULL}},
  {"にゅ", "ニュ", {"n", "yu", NULL}},
  {"にぇ", "ニェ", {"n", "ye", NULL}},
  {"にょ", "ニョ", {"n", "yo", NULL}},
  {"ひゃ", "ヒャ", {"h", "ya", NULL}},
  {"ひゅ", "ヒュ", {"h", "yu", NULL}},
  {"ひぇ", "ひぇ", {"h", "ie", NULL}},
  {"ひょ", "ヒョ", {"h", "yo", NULL}},
  {"みゃ", "ミャ", {"m", "ya", NULL}},
  {"みゅ", "ミュ", {"m", "yu", NULL}},
  {"みぇ", "ミェ", {"m", "ye", NULL}},
  {"みょ", "ミョ", {"m", "yo", NULL}},
  {"りゃ", "リャ", {"r", "ya", NULL}},
  {"りゅ", "リュ", {"r", "yu", NULL}},
  {"りぇ", "リェ", {"r", "ye", NULL}},
  {"りょ", "リョ", {"r", "yo", NULL}},
  {"ぎゃ", "ギャ", {"g", "ya", NULL}},
  {"ぎゅ", "ギュ", {"g", "yu", NULL}},
  {"ぎぇ", "ギェ", {"g", "ye", NULL}},
  {"ぎょ", "ギョ", {"g", "yo", NULL}},
  {"じゃ", "ジャ", {"z", "ya", NULL}},
  {"じゅ", "ジュ", {"z", "yu", NULL}},
  {"じぇ", "ジェ", {"z", "ye", NULL}},
  {"じょ", "ジョ", {"z", "yo", NULL}},
  {"ぢゃ", "ヂャ", {"z", "ya", NULL}},
  {"ぢゅ", "ヂュ", {"z", "yu", NULL}},
  {"ぢぇ", "ヂェ", {"z", "ye", NULL}},
  {"ぢょ", "ヂョ", {"z", "yo", NULL}},
  {"びゃ", "ビャ", {"b", "ya", NULL}},
  {"びゅ", "ビュ", {"b", "yu", NULL}},
  {"びぇ", "ビェ", {"b", "ye", NULL}},
  {"びょ", "ビョ", {"b", "yo", NULL}},
  {"ぴゃ", "ピャ", {"p", "ya", NULL}},
  {"ぴゅ", "ピュ", {"p", "yu", NULL}},
  {"ぴぇ", "ピェ", {"p", "ie", NULL}},
  {"ぴょ", "ピョ", {"p", "yo", NULL}},
  {"ふぁ", "ファ", {"f", "a", NULL}},
  {"ふぃ", "フィ", {"f", "i", NULL}},
  {"ふぇ", "フェ", {"f", "e", NULL}},
  {"ふぉ", "フォ", {"f", "o", NULL}},

  {"うぃ", "ウィ", {"w", "i", NULL}},
  {"うぇ", "ウェ", {"w", "e", NULL}},

  {"", "ヴァ", {"v", "a", NULL}},
  {"", "ヴィ", {"v", "i", NULL}},
  {"", "ヴェ", {"v", "e", NULL}},
  {"", "ヴォ", {"v", "o", NULL}},
  {"あ", "ア", {"a", NULL}},
  {"い", "イ", {"i", NULL}},
  {"う", "ウ", {"u", NULL}},
  {"え", "エ", {"e", NULL}},
  {"お", "オ", {"o", NULL}},
  {"か", "カ", {"k", "a", NULL}},
  {"き", "キ", {"k", "i", NULL}},
  {"く", "ク", {"k", "u", NULL}},
  {"け", "ケ", {"k", "e", NULL}},
  {"こ", "コ", {"k", "o", NULL}},
  {"さ", "サ", {"s", "a", NULL}},
  {"し", "シ", {"s", "i", NULL}},
  {"す", "ス", {"s", "u", NULL}},
  {"せ", "セ", {"s", "e", NULL}},
  {"そ", "ソ", {"s", "o", NULL}},
  {"た", "タ", {"t", "a", NULL}},
  {"ち", "チ", {"c", "i", NULL}},
  {"つ", "ツ", {"ts", "u", NULL}},
  {"て", "テ", {"t", "e", NULL}},
  {"と", "ト", {"t", "o", NULL}},
  {"な", "ナ", {"n", "a", NULL}},
  {"に", "ニ", {"n", "i", NULL}},
  {"ぬ", "ヌ", {"n", "u", NULL}},
  {"ね", "ネ", {"n", "e", NULL}},
  {"の", "ノ", {"n", "o", NULL}},
  {"は", "ハ", {"h", "a", NULL}},
  {"ひ", "ヒ", {"h", "i", NULL}},
  {"ふ", "フ", {"h", "u", NULL}},
  {"へ", "ヘ", {"h", "e", NULL}},
  {"ほ", "ホ", {"h", "o", NULL}},
  {"ま", "マ", {"m", "a", NULL}},
  {"み", "ミ", {"m", "i", NULL}},
  {"む", "ム", {"m", "u", NULL}},
  {"め", "メ", {"m", "e", NULL}},
  {"も", "モ", {"m", "o", NULL}},
  {"や", "ヤ", {"ya", NULL}},
  {"ゆ", "ユ", {"yu", NULL}},
  {"よ", "ヨ", {"yo", NULL}},
  {"ら", "ラ", {"r", "a", NULL}},
  {"り", "リ", {"r", "i", NULL}},
  {"る", "ル", {"r", "u", NULL}},
  {"れ", "レ", {"r", "e", NULL}},
  {"ろ", "ロ", {"r", "o", NULL}},
  {"わ", "ワ", {"wa", NULL}},
  {"ゐ", "ヰ", {"i", NULL}},
  {"ゑ", "ヱ", {"e", NULL}},
  {"を", "ヲ", {"o", NULL}},
  {"ん", "ン", {"N", NULL}},
  {"が", "ガ", {"g", "a", NULL}},
  {"ぎ", "ギ", {"g", "i", NULL}},
  {"ぐ", "グ", {"g", "u", NULL}},
  {"げ", "ゲ", {"g", "e", NULL}},
  {"ご", "ゴ", {"g", "o", NULL}},
  {"ざ", "ザ", {"z", "a", NULL}},
  {"じ", "ジ", {"z", "i", NULL}},
  {"ず", "ズ", {"z", "u", NULL}},
  {"ぜ", "ゼ", {"z", "e", NULL}},
  {"ぞ", "ゾ", {"z", "o", NULL}},
  {"だ", "ダ", {"d", "a", NULL}},
  {"ぢ", "ヂ", {"d", "i", NULL}},
  {"づ", "ヅ", {"d", "u", NULL}},
  {"で", "デ", {"d", "e", NULL}},
  {"ど", "ド", {"d", "o", NULL}},
  {"ば", "バ", {"b", "a", NULL}},
  {"び", "ビ", {"b", "i", NULL}},
  {"ぶ", "ブ", {"b", "u", NULL}},
  {"べ", "ベ", {"b", "e", NULL}},
  {"ぼ", "ボ", {"b", "o", NULL}},
  {"ぱ", "パ", {"p", "a", NULL}},
  {"ぴ", "ピ", {"p", "i", NULL}},
  {"ぷ", "プ", {"p", "u", NULL}},
  {"ぺ", "ペ", {"p", "e", NULL}},
  {"ぽ", "ポ", {"p", "o", NULL}},

  {"っ", "ッ", {"q", NULL}},
  {NULL, NULL, {NULL}}
};

static txtPhono _tx_phono_silent = {"sil", 1, {V_S}};

static txtPhono _tx_phono[] = {
  /*
   * 有声音
   */
  /* 母音 */
  {"a",  1, {V_W}},
  {"e",  1, {V_W}},
  {"o",  1, {V_W}},

  {"i",  2, {V_W, V_U}},
  {"u",  2, {V_W, V_U}}, /* /i/, /u/ は、無声化母音となりうる */

  /* 半母音 */
  {"ya", 1, {V_W}}, {"yu", 1, {V_W}}, {"ye", 1, {V_W}}, {"yo", 1, {V_W}},
  {"wa", 1, {V_W}}, {"w",  1, {V_W}}, {"ie", 1, {V_W}},
  
  /* 鼻音 */
  {"m",  1, {V_C}}, {"n",  1, {V_C}}, {"N",  1, {V_W}},
  
  /* 流音 */
  {"r",  1, {V_V}},

  /* 有声破裂音 */
  {"b",  2, {V_D, V_U}}, {"d",  2, {V_D, V_U}},
  {"g",  2, {V_D, V_S}}, /* /g/ は、D のほか C, S としても分類される */

  /* 有声摩擦音 */
  {"h",  1, {V_V}},
  {"z",  1, {V_C}},      /* /z/ は無声化摩擦音となりうる             */
  {"f",  2, {V_V, V_F}}, /* /f/ は、V のほか F, U としても分類される */


  /*
   * 無声音
   */
  /* 無声破裂音 */
  {"p",  1, {V_U}}, {"t",  1, {V_U}},
  {"k",  1, {V_F}},     /* /k/ は、無声破裂音だが、
			   無声摩擦音的音素として扱われる。 */

  /* 無声摩擦音 */
  {"s",  1, {V_F}}, {"c",  1, {V_F}}, {"ts",  1, {V_F}},

  {NULL, 0, {V_unknown}}  /* 終了コード */
};


txtPhono ** txtSplitText(char * fname)
{
  txtPhono ** tx_list = NULL;
  int nums            = 0;
  txtPhono  * phono;
  FILE *rfp;
  int i, j, k;
  char *line;
  int pos;
  char *hira, *kata;
  void *vpt;
  int len;
  int flg;
  int c;

  rfp = fopen(fname, "rb");
  if(NULL == rfp) return NULL;

  /*
   * 先頭には明記されない無音部分があると考えられるため、
   * 自動的に '*' を配置する。
   */
  if(NULL == (tx_list = utilAddBuf(tx_list, sizeof(txtPhono *), nums)))
    goto err_1;
  tx_list[0] = &_tx_phono_silent;
  nums ++;
  while(NULL != (line = utilReadLine(rfp)))
    {
      pos = 0;
      while(line[pos])
	{
	  phono = NULL;
	  
	  /* 空白節をあらわす '*' がきたら、空白要素を追加する */
	  if(line[pos] == '*')
	    {
	      if(NULL == (vpt = utilAddBuf(tx_list, sizeof(txtPhono *), nums)))
		goto err_1;
	      tx_list = vpt;
	      tx_list[nums] = &_tx_phono_silent;
	      nums++;
	      pos++;
	    }
	  else if(line[pos] & 0x80)
	    {
	      for(i = 0; _txt_ref[i].hira != NULL; i++)
		{
		  hira = _txt_ref[i].hira;
		  kata = _txt_ref[i].kata;
		  flg = 0;
		  if((len = strlen(hira)) > 0)
		    if(!strncmp(hira, line + pos, strlen(hira))) flg = 1;

		  if((len = strlen(kata)) > 0)
		    if(!strncmp(hira, line + pos, strlen(kata))) flg = 1;
		  
		  if(flg)
		    {
		      /* 一致する音節があれば、その音素列を登録する */
		      for(j = 0; _txt_ref[i].phono[j] != NULL; j++)
			for(k = 0; _tx_phono[k].label != NULL; k++)
			  if(!strcmp(_tx_phono[k].label,
				     _txt_ref[i].phono[j]))
			    {
			      vpt = utilAddBuf(tx_list,
					       sizeof(txtPhono *), nums);
			      if(NULL == vpt) goto err_1;
			      tx_list = vpt;
			      tx_list[nums] = &_tx_phono[k];
			      nums++;
			    }
		      pos += len;
		      break;
		    }
		}
	      if(_txt_ref[i].hira == NULL)
		if(!strncmp("ー", line + pos, 2))
		  {
		    /*
		     * 直前の音素が母音/半母音であることを確認する
		     * /a/ /i/ /u/ /e/ /o/ /ya/ /yu/ /yo/ /wa/ のみ許可
		     */
		    len = strlen(tx_list[nums - 1]->label);
		    c = tx_list[nums - 1]->label[len - 1];
		    if(utilCheckChars(c, "aiueo"))
		      /* 直前の母音と同じラベルを持つ母音を
			 音素として割り当てる */
		      for(i = 0; _tx_phono[i].label != NULL; i++)
			if(*(_tx_phono[i].label) == c)
			  {
			    vpt = utilAddBuf(tx_list,
					     sizeof(txtPhono *), nums);
			    if(NULL == vpt) goto err_1;
			    tx_list = vpt;
			    tx_list[nums] = &_tx_phono[i];
			    nums++;
			    break;
			  }
		    pos += 2;
		  }
	    }
	  else pos++;
	}
    }

  /* 終了間際の無音(sil)を書き込む */
  if(NULL == (vpt = utilAddBuf(tx_list, sizeof(txtPhono *), nums))) goto err_1;
  tx_list = vpt;
  tx_list[nums] = &_tx_phono_silent;
  nums++;

  /* 終了コードとなる NULL を書き込む */
  if(NULL == (vpt = utilAddBuf(tx_list, sizeof(txtPhono *), nums))) goto err_1;
  tx_list = vpt;
  tx_list[nums] = NULL;

  fclose(rfp);

  /* 「無声化」を考慮し、音素列を修正 */
  tx_list = txt_adjust_no_voice_vowel(tx_list);

  /* 「子音脱落」を考慮し、音素列を修正 */

  /* 「子音母音融合」を考慮し、音素列を修正 */


  /* 「長母音化」を考慮し、音素列を修正 */
  tx_list = txt_adjust_long_vowel(tx_list);

#ifdef _DEBUG_
  for(i = 0; tx_list[i] != NULL; i++)
    printf(" %s", tx_list[i]->label);

  printf("\n");
#endif /* _DEBUG_ */

  return tx_list;

 err_1:
  free(tx_list);
  fclose(rfp);
  return NULL;
}


/* 文字列の末尾と、文字列を比較する */
static int txt_cmptail(char *tester, char *sample)
{
  size_t len;

  len = strlen(sample);
  return strcmp(tester + strlen(tester) - len, sample);
}

/*
 * 指定した音素の末尾と同じ音素がリスト中にあるかどうかをチェックする
 */
static int txt_check_phonome(char *phonome, char **list)
{
  int i, ret;

  for(i = 0; list[i] != NULL; i++)
    if(!(ret = txt_cmptail(phonome, list[i]))) return 1;

  return 0;
}

/*
 * 「無声化」を考慮し、音素列を修正する。
 *
 * VC 表記において、C1 V C2 の場合、
 * C1, C2 が {/p/, /t/, /k/, /s/, /h/} であり、
 * V が { /i/, /u/ } の場合、V は無声化する。
 */
static txtPhono ** txt_adjust_long_vowel(txtPhono **tx_list)
{
  static char *listC[] = {"p", "t", "k", "s", "h", NULL};
  static char *listV[] = {"i", "u", NULL};
  int fC1, fC2;
  int fV;
  int i, tx_size, tx_pos;
  txtPhono ** new_phono;

  /* 現在の音素数をカウントする */
  tx_size = 0;
  for(i = 0; tx_list[i] != NULL; i++) tx_size++;
  
  if(NULL == (new_phono = malloc(sizeof(txtPhono *) * (tx_size + 1))))
    return tx_list;

  tx_pos = 0;
  fC1 = fC2 = fV = 0;  /* 初期値 */
  for(i = 0; tx_list[i + 1] != NULL; i++)
    {
      fV = txt_check_phonome(tx_list[i]->label, listV);
      fC2 = txt_check_phonome(tx_list[i + 1]->label, listC);

      /* すべての条件が揃えば、現在の音素を
	 無声化する母音であると判断し、新規リストに追加しないものとする。 */
      if(!(fC1 && fC2 && fV)) new_phono[tx_pos++] = tx_list[i];

      fC1 = txt_check_phonome(tx_list[i]->label, listC);
    }

  new_phono[tx_pos++] = NULL;
  new_phono = realloc(new_phono, sizeof(txtPhono *) * tx_pos);
  free(tx_list);
  return new_phono;
}

/*
 *
 */
static txtPhono ** txt_adjust_no_voice_vowel(txtPhono **tx_list)
{
  int i, j, tx_size, tx_pos;
  txtPhono ** new_phono;
  txtPhono * tmp;
  char * pre;

  /* 現在の音素数をカウントする */
  tx_size = 0;
  for(i = 0; tx_list[i] != NULL; i++) tx_size++;
  
  if(NULL == (new_phono = malloc(sizeof(txtPhono *) * (tx_size + 1))))
    return tx_list;

  tx_pos = 0;
  pre = NULL;
  for(i = 0; tx_list[i] != NULL; i++)
    {
      /* 直前の音素と、長母音開始音素を比較する */
      tmp = tx_list[i];
      if(NULL != pre)
	{
	  if(!strcmp(pre, "e"))
	    {
	      if(!strcmp(tx_list[i]->label, "i"))
		/* 母音 /i/ を、母音 /e/ で置き換える */
		for(j = 0; _tx_phono[j].label != NULL; j++)
		  if(!strcmp("e", _tx_phono[j].label))
		    {
		      tmp = &_tx_phono[j];
		      break;
		    }
	    }
	  else if(!strcmp(pre, "o"))
	    {
	      if(!strcmp(tx_list[i]->label, "u"))
		/* 母音 /u/ を、母音 /o/ で置き換える */
		for(j = 0; _tx_phono[j].label != NULL; j++)
		  if(!strcmp("o", _tx_phono[j].label))
		    {
		      tmp = &_tx_phono[j];
		      break;
		    }
	    }
	}

      new_phono[tx_pos++] = tmp;
      pre = tmp->label;
    }

  new_phono[tx_pos++] = NULL;
  new_phono = realloc(new_phono, sizeof(txtPhono *) * tx_pos);
  free(tx_list);
  return new_phono;
}
