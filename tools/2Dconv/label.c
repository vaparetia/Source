#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _label_c_
#include "debug.h"
#include "my_malloc.h"
#include "util.h"
#include "label.h"


lblBlock * lblCreateLabelBlock(void)
{
  lblBlock * block;

  if(NULL == (block = MALLOC(sizeof(lblBlock)))) return NULL;

  block->begin = NULL;
  block->end   = NULL;
  block->newer_num = 0;  /* 最初から順に番号を振っていく */

  return block;
}

int lblReleaseLabelBlock(lblBlock * block)
{
  lblLabel * lbl;
  lblLabel * nxt;

  if(NULL == block) return LBLERR_UNDEFINED;

  lbl = block->begin;
  do {
    nxt = lbl->next;
    FREE(lbl->label);
    FREE(lbl);
  } while(NULL != (lbl = nxt));

  FREE(block);

  return 0;
}


static int lbl_check_same_label(lblBlock * block, char * label)
{
  lblLabel * lbl;

  for(lbl = block->begin; lbl != NULL; lbl = lbl->next)
    /* 該当するラベルが登録されている場合は、そのラベルの持つ値を返す */
    if(!strcmp(label, lbl->label)) return lbl->id;

  /* 該当するものが無い場合は、定義されていないというステータスを返す */
  return LBLERR_UNDEFINED;
}

/*
 * ラベルの登録
 */
int lblDefineLabel(lblBlock * block, char * label)
{
  lblLabel * lbl;

  /* 登録前に、既に同じものが登録されていないかをチェックする */
  if(lbl_check_same_label(block, label) >= 0) return LBLERR_REGISTED;

  /* メモリが確保できなければエラー */
  if(NULL == (lbl = MALLOC(sizeof(lblLabel)))) return LBLERR_NO_MEMORY;
  if(NULL == (lbl->label = strdup(label)))
    {
      FREE(lbl);
      return LBLERR_NO_MEMORY;
    }

  lbl->id = block->newer_num++;
  lbl->prev = block->end;
  lbl->next = NULL;
  if(block->end != NULL) block->end->next = lbl;
  block->end = lbl;
  if(NULL == block->begin) block->begin = lbl;

  return lbl->id;
}

/*
 * ラベルの抹消
 */
int lblUndefLabel(lblBlock * block, char * label)
{
  lblLabel * lbl;

  for(lbl = block-> begin; lbl != NULL; lbl = lbl->next)
    if(!strcmp(lbl->label, label))
      {
	/* リンクから自分を外す */
	lbl->next->prev = lbl->prev;
	lbl->prev->next = lbl->next;

	FREE(lbl->label);
	FREE(lbl);

	return 0;
      }
  
  return LBLERR_UNDEFINED;
}

/*
 * ラベル名から、その値を得る
 */
int lblRefLabel(lblBlock * block, char * label)
{
  return lbl_check_same_label(block, label);  
}

#ifdef _DEBUG_
void lblDbgShow(FILE * output, lblBlock * block)
{
  lblLabel * lbl;

  fprintf(output, "Block: %p\n---------------------------------\n", block);

  for(lbl = block->begin; lbl != NULL; lbl = lbl->next)
    fprintf(output, "%5d  \"%s\"\n", lbl->id, lbl->label);

  fprintf(output, "---------------------------------\n");
}
#endif /* _DEBUG_ */
