#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _voice_c_
#include "voice.h"
#include "vmotion.h"

static struct {
  char * label;
  int    id;
} id_list[] = {
  {"a", VOWEL_A}, {"i", VOWEL_I}, {"u", VOWEL_U}, {"e", VOWEL_E},
  {"o", VOWEL_O}, {"N", VOWEL_N}, {"sil", VOWEL_S},

  /* 以下の音素は、後続母音の形状になる。*/
  {"k", VOWEL_unknown}, {"h", VOWEL_unknown},
  {"r", VOWEL_unknown}, {"g", VOWEL_unknown},

  /* 以下の音素は、/u/ の形状から母音形状へ変化する
     (音素の形状は /u/ として扱う ) */

  {"s", VOWEL_U}, {"t", VOWEL_U}, {"n", VOWEL_U}, {"z", VOWEL_U},
  {"ts", VOWEL_U},{"d", VOWEL_U}, {"ya", VOWEL_A},{"yu", VOWEL_U},
  {"yo", VOWEL_O},

  /* 以下の音素は、閉じ形状から母音形状へ変化する */
  {"m", VOWEL_S}, {"b", VOWEL_S}, {"p", VOWEL_S},
  
  /* 以下の半母音は、特殊な形状を持つ
     (扱い上の一音素で、二つのモーションを繋いだ動きをする) */
  {"wa", VOWEL_uA},

  {NULL, VOWEL_unknown}
};

/* 音素ラベルから、口の形をあらわす母音ID を返す*/
int voiLabel2ID(char *label)
{
  int i;

  for(i = 0; id_list[i].label != NULL; i++)
    if(!strcmp(id_list[i].label, label)) return id_list[i].id;
  return VOWEL_unknown;
}
