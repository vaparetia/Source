#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _objact_c_
#include "debug.h"
#include "my_malloc.h"
#include "util.h"
#include "token.h"
#include "label.h"

#include "command.h"
#include "objdef.h"
#include "objstat.h"
#include "objact.h"

static int act_release_step(actTrack * parent, actStep * step)
{

  /* 自分をリンクから除去する */
  if(parent->begin == step) parent->begin = step->next;
  if(parent->end   == step) parent->end   = step->prev;
  if(NULL != step->prev) step->prev->next = step->next;
  if(NULL != step->next) step->next->prev = step->prev;
  FREE(step);
  return 0;
}


static int act_release_track(actAction * parent, actTrack * track)
{
  actStep * step;
  actStep * next;

  FREE(track->trk_name);
  step = track->begin;
  while(step != NULL)
    {
      next = step->next;
      act_release_step(track, step);
      step = next;
    }

  /* 自分をリンクから除去する */
  if(parent->begin == track) parent->begin = track->next;
  if(parent->end   == track) parent->end   = track->prev;

  if(NULL != track->prev) track->prev->next = track->next;
  if(NULL != track->next) track->next->prev = track->prev;

  FREE(track);

  return 0;
}

static int act_release_action(actAction * act)
{
  actTrack * track;
  actTrack * next;

  FREE(act->act_label);
  track = act->begin;
  while(NULL != track)
    {
      next = track->next;
      act_release_track(act, track);
      track = next;
    }

  /* 自分をリンクから除去する */
  if(actLink.begin == act) actLink.begin = act->next;
  if(actLink.end   == act) actLink.end   = act->prev;
  if(act->prev != NULL) act->prev->next = act->next;
  if(act->next != NULL) act->next->prev = act->prev;

  FREE(act);

  return 0;
}



int actInitObjectAction(void)
{
  actAction * act;
  actAction * next;

  if(actLink.begin != NULL)
    {
      act = actLink.begin;
      while(act != NULL)
	{
	  next = act->next;
	  act_release_action(act);
	  act = next;
	}
    }
  actLink.begin = actLink.end = actLink.now = NULL;  
  
  return 0;
}

/*
 * アクション構造体の、リストへの登録
 */
static int act_regist_action(actAction * act)
{
  act->prev = actLink.end;
  act->next = NULL;
  if(NULL != actLink.end) actLink.end->next = act;
  actLink.end = act;
  actLink.now = act;
  if(NULL == actLink.begin) actLink.begin = act;
  return 0;
}

#define TRACK_CONT  0
#define TRACK_END   1
#define TRACK_ERROR 2


static int act_regist_track(actAction * act, actTrack * trk)
{
  trk->prev = act->end;
  trk->next = NULL;
  if(NULL != act->end) act->end->next = trk;
  act->end = trk;
  if(NULL == act->begin) act->begin = trk;
  return 0;
}

static int act_regist_step(actTrack * track, actStep * step)
{
  step->prev = track->end;
  if(NULL != track->end) track->end->next = step;
  step->next = NULL;
  track->end = step;
  if(NULL == track->begin) track->begin = step;
  return 0;
}
#define STEP_CONT   1
#define STEP_END    0
#define STEP_ERROR -1

static int act_get_step(tknFILE * tkn, actTrack * track)
{
  actStep * step;
  actMode   mode;
  char * cmd;
  char * lf;
  char * str_time;
  char * str_name;

  step = NULL;
  cmd = str_time = str_name = NULL;
  if(NULL == (cmd = cmdGetCommand(tkn))) goto err_end;
  cmdStatus = 0;
  DBG("cmd = %s\n", cmd);
  if(!strcmp(cmd, "wait"))
    {
      mode = STEP_WAIT;
      if(NULL == (str_time = cmdGetToken(tkn, TKN_TOKEN))) goto err_end;
    }
  else if(!strcmp(cmd, "set"))
    {
      mode = STEP_SET;
      if(NULL == (str_name = cmdGetToken(tkn, TKN_QUOTED))) goto err_end;
      DBG("str_name = %s\n", str_name);
    }
  else if(!strcmp(cmd, "morf"))
    {
      mode = STEP_MORF;
      if(NULL == (str_time = cmdGetToken(tkn, TKN_TOKEN))) goto err_end;
      if(NULL == (str_name = cmdGetToken(tkn, TKN_QUOTED))) goto err_end;
    }
  else if(!strcmp(cmd, "trend"))
    {
      FREE(cmd);
      return STEP_END;
    }
  else
    {
      FREE(cmd);
      cmdStatus = CMD_ERR_SYNTAX;
      goto err_end;
    }
  FREE(cmd);


  if(NULL == (lf = cmdGetToken(tkn, TKN_DELIM))) goto err_end;
  if(*lf != '\n')
    {
      FREE(lf);
      cmdStatus = CMD_ERR_SYNTAX;
      goto err_end;
    }
  FREE(lf);

  if(NULL == (step = cmdMalloc(sizeof(actStep)))) goto err_end;
  step->mode = mode;
  if(NULL != str_time)
    if(str_to_int(&step->st_time, str_time)) goto err_end;
  step->st_name = str_name;
  act_regist_step(track, step);

  if(NULL != str_time) FREE(str_time);

  return STEP_CONT;
  
 err_end:
  if(str_name != NULL) FREE(str_name);
  if(str_time != NULL) FREE(str_time);
  if(step != NULL) FREE(step);
  if(cmd != NULL) FREE(cmd);
  return STEP_ERROR;
}

static int act_get_track(tknFILE * tkn, actAction * act)
{
  actTrack * trk;
  char * str_track;
  char * trk_name;
  char * lf;
  int s;

  cmdStatus = 0;
  if(NULL == (str_track = cmdGetCommand(tkn))) return TRACK_ERROR;
  if(!strcmp(str_track, "actend"))
    {
      FREE(str_track);
      return TRACK_END;
    }
  if(strcmp(str_track, "track"))
    {
      FREE(str_track);
      goto syntax_err;
    }
  FREE(str_track);

  /* トラック名称の取得 */
  if(NULL == (trk_name = cmdGetToken(tkn, TKN_QUOTED))) goto err;

  /* 改行の確認 */
  if(NULL == (lf = cmdGetToken(tkn, TKN_DELIM)))
    {
      FREE(trk_name);
      goto syntax_err;
    }
  if(*lf  != '\n')
    {
      FREE(lf);
      FREE(trk_name);
      goto syntax_err;
    }
  FREE(lf);

  if(NULL == (trk = cmdMalloc(sizeof(actTrack))))
    {
      FREE(trk_name);
      goto no_memory_err;
    }

  trk->trk_name= trk_name;
  trk->begin = trk->end = NULL;
  do {
    s = act_get_step(tkn, trk);
  } while(s == STEP_CONT);

  /* 最後の改行を取得する */
  if(NULL == (lf = cmdGetToken(tkn, TKN_DELIM)))
    {
      act_release_track(act, trk);
      goto syntax_err;
    }
    
  act_regist_track(act, trk);
  return TRACK_CONT;

 syntax_err:
  cmdStatus = CMD_ERR_SYNTAX;
  return TRACK_ERROR;
 no_memory_err:
  cmdStatus = CMD_ERR_NO_MEMORY;
 err:
  return TRACK_ERROR;
}


/*
 * 
 */
int actDefAction(char * cmd, tknFILE * tkn, lblBlock * block, FILE *wfp)
{
  char * act_label;   /* アクション名 */
  char * lf;
  actAction * act;
  int s;

  /* アクション名ラベルを取得 */
  if(NULL == (act_label = cmdGetToken(tkn, TKN_QUOTED))) return cmdStatus;
  if(NULL == (lf = cmdGetToken(tkn, TKN_DELIM)))
    {
      FREE(act_label);
      return cmdStatus;
    }
  if(*lf != '\n')
    {
      FREE(act_label);
      return cmdStatus;
    }
  FREE(lf);
  if(NULL == (act = cmdMalloc(sizeof(actAction))))
    {
      FREE(act_label);
      return cmdStatus;
    }

  act->act_label = act_label;
  act->begin = act->end = NULL;

  do {
    s = act_get_track(tkn, act);
  } while(TRACK_CONT == s);

  /* 改行の取得 */
  if(NULL == (lf = cmdGetToken(tkn, TKN_DELIM)))
    {
      act_release_action(act);
      return cmdStatus;
    }
  if(*lf != '\n')
    {
      FREE(lf);
      act_release_action(act);
      return (cmdStatus = CMD_ERR_SYNTAX);
    }
  FREE(lf);
  act_regist_action(act);
  return 0;
}
