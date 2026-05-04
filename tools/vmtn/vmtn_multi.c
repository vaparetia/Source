/*
 * 複数キャラクター分のモーション素材を管理するモジュール
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _vmtn_multi_c_
#include "vmtn_multi.h"
#include "vmotion.h"
#include "mem_util.h"

#ifdef _DEBUG_
#define DBG(args...)  { fprintf(stderr, args);  fflush(stderr); }
#else
#define DBG(args...)
#endif /* _DEBUG_ */


/*
 * vmmSpeaker 構造体に登録されている話者の中で、指定された話者コードを持つ
 * 話者のモーション素材セットを、指定されたラベルのものに変更する
 */
int vmmSelectMotionSet(vmtnContainer * container,
		       int speaker_code, char * set_label)
{
  vmtnSpeaker * speaker;
  vmtnSet    * set;
  vmtnVowel  * vowel;
  int i;

  DBG("entering vmmSelectMotionSet() - [%s]\n", set_label);
  for(i = 0; i < container->nums; i++)
    if(container->mtn_block[i]->code == speaker_code)
      {
	speaker = container->mtn_block[i];
	break;
      }
  if(i == container->nums) return -1;  /* 指定された話者が見付からない */

  set = speaker->set;

  for(vowel = set->begin; vowel != NULL; vowel = vowel->next)
    if(!strcmp(set_label, vowel->set_label))
      {
	speaker->motions = vowel;
	DBG("vmmSelectMotionSet(): SUCCESS\n");
	return 0;   /* 変更は正常に終了 */
      }
  return -1;    /* 指定されたセット名が見付からない */
}


vmtnSpeaker * vmmReadSpeakerMotion(char *fname)
{
  vmtnSpeaker * speaker;
  vmtnSet     * set;
  vmtnVowel   * motions;

  DBG("entering vmmReadSpeakerMotion()...\n");
  if(NULL == (set = vmtnReadMotion(fname))) return NULL;

  DBG("<----------->\n");
  if(NULL == (speaker = malloc(sizeof(vmtnSpeaker))))
    {
      vmtnReleaseMotion(set);
      return NULL;
    }

  motions = set->begin;
  speaker->code    = set->speaker_code;
  speaker->name    = set->speaker_name;
  //speaker->motions = motions;
  speaker->motions = set->begin;
  speaker->set     = set;

  DBG("vmmReadSpeakerMotion() is success.\n");
  return speaker;
}

void  vmmReleaseSpeakerMotion(vmtnSpeaker * speaker)
{
  DBG("entering vmmReleaseSpeakerMotion()\n");
  vmtnReleaseMotion(speaker->set);
  free(speaker);
}

vmtnContainer * vmmCreateContainer(void)
{
  vmtnContainer *  cont;

  if(NULL == (cont = malloc(sizeof(vmtnContainer)))) return NULL;
  cont->nums = 0;
  cont->mtn_block = NULL;

  return cont;
}


/*
 * ある話者のモーションセットを、モーションコンテナに追加する
 */
vmtnContainer * vmmRegistSpeaker(vmtnContainer * cont, vmtnSpeaker * speaker)
{
  void * vpt;
  int i;

  /* おなじ話者名コードを持つモーションセットがあれば、
     新しいものと差し替える */
  for(i = 0; i < cont->nums; i++)
    if(cont->mtn_block[i]->code == speaker->code)
      {
	vmmReleaseSpeakerMotion(cont->mtn_block[i]);
	cont->mtn_block[i] = speaker;
	return cont;
      }


  /* 新規の登録 */
  if(NULL == (vpt = mem_add(cont->mtn_block,
			    sizeof(vmtnSpeaker *), cont->nums))) return NULL;

  cont->mtn_block = vpt;
  cont->mtn_block[cont->nums] = speaker;
  cont->nums++;

  return cont;
}

void vmmReleaseContainer(vmtnContainer * cont)
{
  int i;

  DBG("entering vmmReleaseContainer()\n");
  for(i = 0; i < cont->nums; i++)
    vmmReleaseSpeakerMotion(cont->mtn_block[i]);
  free(cont->mtn_block);
  free(cont);
}
