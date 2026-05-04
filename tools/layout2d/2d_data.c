#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _2d_data_c_
#include "2d_data.h"
#include "yacc_parser.h"
#include "debug.h"

#define BIT_LEN  24    /* StrCode 生成で使用する */

extern int num_line;

static char err_msg[1024];

/*
 * 与えられた文字列の StrCode を求め、返す
 */
int strcode(char * string)
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

/*
 * ファイル名から StrCode を求める
 */
static int file_strcode(char * name)
{
  int begin, len, ret;
  char * label;
  char * buf;

  /* ディレクトリ名を除去する */
  for(begin = strlen(name); begin > 0; begin--)
    if(name[begin - 1] == '/') break;

  label = name + begin;

  /* 最初のピリオド以降を除去する (Suffix にあらず) */
  for(len = 0; *(label + len); len++)
    if(*(label + len) == '.') break;

  if(NULL == (buf = MALLOC(len + 1))) return -1;
  strncpy(buf, label, len);
  buf[len] = 0;
  ret = strcode(buf);
  FREE(buf);
  return ret;
}

/*
 * スクリプトを読み、値を各ワークにセットする
 */
sprLayout * sprReadLayoutScript(FILE * rfp)
{
  int ret;

  /* ワークの確保 */
  if(NULL == (spr_layout = MALLOC(sizeof(sprLayout)))) return NULL;

  /* ワークの初期化 */
  spr_layout->obj_last_id = 0;
  spr_layout->objBegin    = spr_layout->objEnd  = NULL;
  spr_layout->blockBegin  = spr_layout->blockEnd = NULL;
  spr_layout->blockTarget = NULL;
  spr_layout->actBegin    = spr_layout->actEnd   = NULL;
  spr_layout->actTarget   = NULL;

  spr_layout->tri_info = NULL;


  yyin = rfp;
  ret = yyparse();

#ifdef DEBUG
  {
    sprObject * obj;

    for(obj = spr_layout->objBegin; obj != NULL; obj = obj->next)
      {
	printf("Object number: %d\n", obj->serial_id);
	printf("Name: %s ( %06x:%d )\n", obj->label, obj->code, obj->code);
	if(obj->parent != NULL)
	  printf("Parent: %s\n", obj->parent->label);
	printf("Vertexes: %d\n", obj->vertex_num);
	printf("Colors:  %d\n", obj->rgba_num);

	switch(obj->obj_id)
	  {
	  case SP_EMPTY:     printf("Attrib: [ EMPTY ]\n"); break;
	  case SP_POINT:     printf("Attrib: [ POINT ]\n"); break;
	  case SP_LINE:      printf("Attrib: [ LINE ]\n"); break;
	  case SP_LINESTRIP: printf("Attrib: [ LINE STRIP ]\n"); break;
	  case SP_BOX:       printf("Attrib: [ BOX ]\n"); break;
	  case SP_SPRITE:    printf("Attrib: [ SPRITE ]\n"); break;
	  case SP_ZOOM:      printf("Attrib: [ ZOOM ]\n"); break;
	  case SP_SPIN:      printf("Attrib: [ SPIN ]\n"); break;
	  case SP_POLY:      printf("Attrib: [ POLY ]\n"); break;
	  case SP_TRIANGLE:  printf("Attrib: [ TRIANGLE ]\n"); break;
	  defaults: printf("Attrib: [ Unknown ]\n"); break;
	  }

	printf("\n-------------------\n");
      }
  }
  {
    sprStatBlock * block;
    sprStatus * stat;
    int i, j;

    for(block = spr_layout->blockBegin; block != NULL; block = block->next)
      {
	if(block->mode == MODE_REFTRI)
	  printf("Ref TRI: %s [%06x:%d]\n",
		 block->d.tri.label, block->d.tri.code, block->d.tri.code);
	else
	  {
	    /* 状態ブロックの中身を表示 */
	    printf("Block: for %s\n", block->d.st.target->label);
	    for(stat = block->d.st.begin; stat != NULL; stat = stat->next)
	      {
		printf("  Status: \"%s\" [%06x:%d]\n",
		       stat->label, stat->code, stat->code);

		printf("  Disp: %s\n", (stat->disp) ? "SHOW" : "HIDE");
		if(stat->h_rev) printf("H_REV\n");
		if(stat->v_rev) printf("V_REV\n");
		j = 0;
		printf("  Veretx:");
		for(i = 0; i < block->d.st.target->vertex_num; i++)
		  {
		    if(stat->vertex[i].pos)
		      {
			printf("%c(%8.3f, %8.3f)",
			       (j) ? '-' : ' ',
			       stat->vertex[i].x, stat->vertex[i].y);
			j = 1;
		      }
		    else
		      j = 0;
		  }
		printf("\n");
		printf("  RGBA:");
		for(i = 0; i < block->d.st.target->rgba_num; i++)
		  if(stat->vertex[i].col)
		    printf(" {%3d, %3d, %3d, %3d}",
			   stat->vertex[i].r, stat->vertex[i].g,
			   stat->vertex[i].b, stat->vertex[i].a);
		printf("\n");
		  
		if(block->d.st.target->obj_id == SP_SPRITE)
		  printf("  Size: %8.3f *  %8.3f\n",
			 stat->width, stat->height);
		  
		if(stat->tex_sw)
		  {
		    printf("  Texture: \"%s\"\n", stat->tex_name);
		    printf("  UV: (%8.3f, %8.3f)\n  UV-size: (%8.3f, %8.3f)\n",
			   stat->u, stat->v, stat->uw, stat->vh);
		  }
		
		printf("  -----\n");
	      }
	  }
	printf("--------------------\n");
      }
  }
  {
    sprAction * act;
    sprTrack * track;
    int i;

    for(act = spr_layout->actBegin; act != NULL; act = act->next)
      {
	printf("Action: %s [%06x:%d]\n", act->label, act->code, act->code);
	for(track = act->begin; track != NULL; track = track->next)
	  {
	    if(track->target == NULL)
	      printf("  Signal Key Track\n");
	    else
	      printf("  Track for \"%s\"\n", track->target->label);
	    for(i = 0; i < track->seq_step; i++)
	      switch(track->seq[i].op)
		{
		case TR_WAIT:
		  printf("    WAIT (%d)\n", track->seq[i].v_time);
		  break;
		case TR_SET:
		  printf("    SET %s[%02x:%d]\n",
			 track->seq[i].v_stat->label,
			 track->seq[i].v_stat->code,
			 track->seq[i].v_stat->code);
		  break;
		case TR_MORF:
		  printf("    MORF (%d) %s[%02x:%d]\n",
			 track->seq[i].v_time,
			 track->seq[i].v_stat->label,
			 track->seq[i].v_stat->code,
			 track->seq[i].v_stat->code);
		  break;
		case TR_SIGN:
		  printf("    SIGN [%02x:%d] value = %d\n",
			 track->seq[i].signal,
			 track->seq[i].signal,
			 track->seq[i].sigvalue);
		  break;
		case TR_END: printf("    END\n"); break;
		}
	  }
      }
  }
#endif /* DEBUG */

  return (!ret) ? spr_layout : NULL;
}

/*
 * 読み込み時に確保されたメモリを開放する
 */
void sprDestroyReadData(sprLayout * layout)
{
  
  /* オブジェクト定義の開放 */
  {
    sprObject * obj;
    sprObject * next;

    obj = layout->objBegin;
    while(obj != NULL)
      {
	next = obj->next;
	free(obj->label);
	FREE(obj);
	obj = next;
      }
  }

  /* 状態領域の開放 */
  {
    sprStatBlock * block;
    sprStatBlock * bnext;
    sprStatus * stat;
    sprStatus * snext;

    block = layout->blockBegin;
    while(block != NULL)
      {
	bnext = block->next;

	/* 個々の状態の開放 */
	if(block->mode == MODE_REFTRI)
	  free(block->d.tri.label);
	else
	  {
	    stat = block->d.st.begin;
	    while(stat != NULL)
	      {
		snext = stat->next;
		free(stat->label);
		if(NULL != stat->tex_name) free(stat->tex_name);
		if(NULL != stat->vertex) FREE(stat->vertex);
		FREE(stat);
		stat = snext;
	      }
	  }
	FREE(block);

	block = bnext;
      }
  }

  /* アクション領域の開放 */
  {
    sprAction * act;
    sprAction * anext;
    sprTrack * track;
    sprTrack * tnext;

    act = layout->actBegin;
    while(act != NULL)
      {
	anext = act->next;
	free(act->label);
	track = act->begin;
	while(NULL != track)
	  {
	    tnext = track->next;
	    FREE(track->seq);
	    track = tnext;
	  }
	FREE(act);
	act = anext;
      }
  }

  /* .tri 生成情報の開放 */
  {
    triInfo * info;
    triInfo * next;
    triTex  * tex;
    triTex  * txnxt;

    info = layout->tri_info;
    while(NULL != info)
      {
	next = info->next;
	tex = info->tex;
	while(NULL != tex)
	  {
	    txnxt = tex->next;
	    FREE(tex);
	    tex = txnxt;
	  }
	FREE(info);
	info = next;
      }
  }

  FREE(layout);
}

/*
 * 指定された名称を持つオブジェクトのポインタを得る
 */
static sprObject * search_object(char * label)
{
  sprObject * ptr;

  for(ptr = spr_layout->objBegin; ptr != NULL; ptr = ptr->next)
    if(!strcmp(ptr->label, label)) return ptr;

  return NULL;
}

/*
 * 指定されたオブジェクトに属する、指定された名称を持つ状態を検索する
 */
sprStatus * search_status(sprObject * obj, char * name)
{
  sprStatBlock * block;
  sprStatus * stat;

  for(block = spr_layout->blockBegin; block != NULL; block = block->next)
    {
      if(block->mode == MODE_REFTRI) continue;
      if(block->d.st.target == obj) break;
    }
  if(block == NULL) return NULL;
  for(stat = block->d.st.begin; stat != NULL; stat = stat->next)
    if(!strcmp(name, stat->label)) return stat;

  return NULL;
}



/*
 * オブジェクト構造体の新規作成
 */
sprObject * object_2d_new(char * name)
{
  sprObject * obj;
  char * label;

  /* 既に同じ名前のものがある場合はエラー */
  DBG("{{1}}\n");
  if(NULL != search_object(name))
    {
      sprintf(err_msg, "Duplicated label '%s'", name);
      return NULL;
    }
  DBG("{{2}}\n");
  if(NULL == (obj   = MALLOC(sizeof(sprObject)))) return NULL;
  if(NULL == (label = STRDUP(name))) { FREE(obj); return NULL; }

  obj->label = label;
  obj->code  = strcode(name);
  obj->serial_id = spr_layout->obj_last_id ++;
  obj->parent = NULL;  /* 親オブジェクト無し */

  obj->obj_id     = SP_unknown;
  obj->vertex_num = 0;    /* 頂点無し */
  obj->rgba_num   = 0;    /* RGBA 値無し */

  /*
   * オブジェクトを、登録リンクに登録する
   */
  obj->next = NULL;
  obj->prev = spr_layout->objEnd;
  if(NULL != spr_layout->objEnd) spr_layout->objEnd->next = obj;
  spr_layout->objEnd = obj;
  if(NULL == spr_layout->objBegin) spr_layout->objBegin = obj;

  return obj;
}

/*
 * オブジェクトの親オブジェクトを指定する
 */
sprObject * object_2d_set_parent(sprObject * obj, char * parent)
{
  sprObject * p;

  for(p = spr_layout->objBegin; p != NULL; p = p->next)
    if(!strcmp(parent, p->label))
      {
	obj->parent = p;
	break;
      }
  if(p == NULL) return NULL;
  return obj;
}

/*
 * オブジェクト属性を指定する
 */
sprObject * object_2d_set_class(sprObject * obj, sprID id)
{
  static struct {
    sprID id;
    int ver;
    int rgba;
  } list[] = {
    { SP_EMPTY,      1, 0 },
    { SP_POINT,      1, 1 },
    { SP_LINE,       2, 2 },
    { SP_LINESTRIP,  0, 0 },
    { SP_BOX,        2, 1 },
    { SP_SPRITE,     1, 1 },
    { SP_ZOOM,       2, 1 },
    { SP_SPIN,       1, 1 },
    { SP_POLY,       4, 4 },
    { SP_TRIANGLE,   3, 3 },
    { SP_unknown,    0, 0 }
  };
  int i;

  obj->obj_id = id;
  for(i = 0; list[i].id != SP_unknown; i++)
    if(list[i].id == id)
      {
	obj->vertex_num = list[i].ver;
	obj->rgba_num   = list[i].rgba;
	break;
      }

  return obj;
}

/*
 * LineStrip の頂点数を指定する
 */
sprObject * object_2d_set_vertex(sprObject * obj, int num)
{
  /* LineStrip 以外、この操作は禁止 */
  if(obj->obj_id != SP_LINESTRIP) return NULL;

  /* LineStrip は、頂点数と RGB 指定数が同じ */
  obj->vertex_num = num;
  obj->rgba_num = num;

  return obj;
}


/*
 * TRI 指定ブロックの作成
 */
sprStatBlock * statblock_2d_tri(char * tri_name)
{
  sprStatBlock * block;
  triInfo * info;

  if(NULL == (block = MALLOC(sizeof(sprStatBlock)))) return NULL;
  if(NULL == (info = MALLOC(sizeof(triInfo))))
    {
      FREE(block);
      return NULL;
    }
  block->mode = MODE_REFTRI;
  if(NULL == (block->d.tri.label = STRDUP(tri_name)))
    {
      FREE(info);
      FREE(block);
      return NULL;
    }
  block->d.tri.code = file_strcode(tri_name);

  info->tex = NULL;
  info->tri_name = block->d.tri.label;  /* 名前の文字列のみを必要とする */

  /*
   * 登録
   */
  block->next = NULL;
  block->prev = spr_layout->blockEnd;
  if(spr_layout->blockEnd != NULL) spr_layout->blockEnd->next = block;
  spr_layout->blockEnd = block;
  if(spr_layout->blockBegin == NULL) spr_layout->blockBegin = block;

  info->next = spr_layout->tri_info;
  spr_layout->tri_info = info;

  return block;
}

/*
 * 最新のTRI 生成情報に、指定されたテクスチャ名称を追加する
 */
int tri_add_texture(char * tex_name)
{
  triTex * tex;

  /* 現在編集中の TRI 生成情報中に同じものが無いかをチェック */
  for(tex = spr_layout->tri_info->tex; tex != NULL; tex = tex->next)
    if(!strcmp(tex->tex_name, tex_name)) return 0;

  if(NULL == (tex = MALLOC(sizeof(triTex)))) return -1;

  tex->tex_name = tex_name;

  /* 現在編集中の TRI 生成情報に登録 */
  tex->next = spr_layout->tri_info->tex;
  spr_layout->tri_info->tex = tex;

  return 0;
}

/*
 * 状態ブロックの作成
 */
sprStatBlock * statblock_2d_new(char * obj_name)
{
  sprObject * obj;
  sprStatBlock * block;
  
  /* 指定されたオブジェクトの名称が見付からなければエラー */
  if(NULL == (obj = search_object(obj_name)))
    {
      sprintf(err_msg, "Object '%s' is not defined.", obj_name);
      return NULL;
    }

#if 0
  /* 指定されたオブジェクトに対応する状態ブロックが既にあるかを確認し、
     無ければ新規作成、あればそれを再利用する。 */
  for(block = spr_layout->blockBegin; block != NULL; block = block->next)
    {
      if(block->mode == MODE_REFTRI) continue;
      if(!strcmp(block->d.st.target->label, obj_name)) break;
    }
#else
  /*
   * 現在と同じ TRI 情報をもつ,同じオブジェクトに対応する状態ブロックが
   * 既にあればそれを再利用する。同じTRI空間になければ、また新しく作成する。
   */
  for(block = spr_layout->blockEnd; block != NULL; block = block->prev)
    {
      if(block->mode == MODE_REFTRI)
	{
	  /* TRI 指定コードを見付けたということはもはや同じTRI空間では
	     ないので、探索を打ち切る。 */
	  block = NULL;
	  break;
	}
      if(!strcmp(block->d.st.target->label, obj_name)) break;
    }

#if 1
  /* 実機再生環境が対応するまでの間,他のTRI空間に同じ名称のオブジェクトの
     ステータスがあれば、エラー終了させる。*/
  if(block == NULL)
    {
      for(block = spr_layout->blockBegin; block != NULL; block = block->next)
	{
	  if(block->mode == MODE_REFTRI) continue;
	  if(!strcmp(block->d.st.target->label, obj_name))
	    {
	      sprintf(err_msg, "Object '%s' has other TRI (not support yet.)", obj_name);
	      return NULL;
	    }
	}
      block = NULL;
    }
#endif

#endif
  if(NULL == block)
   { 
      if(NULL == (block = MALLOC(sizeof(sprStatBlock)))) return NULL;
      /* 新規作成時の初期化 */
      block->mode = MODE_STATUS;
      block->d.st.target = obj;
      block->d.st.begin = NULL;
      block->d.st.end   = NULL;

      /* 新規作成ブロックを、リンクに登録 */
      block->next = NULL;
      block->prev = spr_layout->blockEnd;
      if(spr_layout->blockEnd != NULL) spr_layout->blockEnd->next = block;
      spr_layout->blockEnd = block;
      if(spr_layout->blockBegin == NULL) spr_layout->blockBegin = block;
    }
  /* 編集対象として登録 */
  spr_layout->blockTarget = block;

  return block;
}

/*
 * 編集対象となっている状態ブロックの下に、状態構造体を作成する。
 */
sprStatus * mode_2d_new(char * name)
{
  sprStatBlock * block;
  sprStatus * stat;
  int num, i;

  /* ステータスを追加する構造体がなければエラー */
  if(NULL == (block = spr_layout->blockTarget))
    {
      sprintf(err_msg, "Not enough status to add mode.\n");
      return NULL;
    }

  /* 同じブロック以下に、同一名称の状態が登録されていないかをチェックする */
  for(stat = block->d.st.begin; stat != NULL; stat = stat->next)
    /* 同じ名称があればエラー */
    if(!strcmp(stat->label, name))
      {
	sprintf(err_msg, "mode '%s' is already defined.\n", name);
	return NULL;
      }
  
  /* メモリを確保できなければエラー */
  if(NULL == (stat = MALLOC(sizeof(sprStatus))))
    {
      sprintf(err_msg, "Not enough memory.\n");
      return NULL;
    }


  /* 名称の登録 */
  if(NULL == (stat->label = STRDUP(name)))
    {
      FREE(stat);
      sprintf(err_msg, "Not enough memory.\n");
      return NULL;
    }
  stat->code = strcode(name);

  /* 頂点情報領域を確保しておく。*/
  num = block->d.st.target->vertex_num;
  if(NULL == (stat->vertex = MALLOC(sizeof(sprVertex) * num)))
    {
      free(stat->label);
      FREE(stat);
      return NULL;
    }
  /* 状態の情報を初期化する */
  stat->tex_name = NULL;
  stat->tex_code = -1;

  stat->width    = stat->height = 0.0F;
  stat->center_x = stat->center_y = 0.0F;
  stat->angle    = 0.0F;
  stat->u        = stat->v      = 0.0F;
  stat->uw       = stat->vh     = 1.0F;
  stat->alpha    = -1;
  stat->v_alpha  = 128;
  stat->pri      = 0;
  stat->magni    = 1.0F;  /* 倍率はデフォルトで 1.0F */
  stat->tex_sw   = 0;
  stat->disp     = 0;
  stat->h_rev    = 0;
  stat->v_rev    = 0;

  for(i = 0; i < num; i++)
    {
      stat->vertex[i].pos = 0;
      stat->vertex[i].col = 0;
    }

  /* 状態構造体をブロックに登録 */
  stat->next = NULL;
  stat->prev = block->d.st.end;
  if(block->d.st.end != NULL) block->d.st.end->next = stat;
  block->d.st.end = stat;
  if(block->d.st.begin == NULL) block->d.st.begin = stat;
  return stat;
}

/*
 * アクションブロックを新規作成する
 */
sprAction * action_2d_new(char * name)
{
  sprAction * act;

  /* 既に同じ名前のものが登録されていないかどうかを確認し、
     同じものが登録されていれば、続きとして登録する */
  for(act = spr_layout->actBegin; act != NULL; act = act->next)
    if(!strcmp(act->label, name))
      {
	printf("action '%s' is exists.\n", name);
	spr_layout->actTarget = act;
	return act;
      }
  /* 同じ名称のものがなければ新規作成する */
  if(NULL == (act = MALLOC(sizeof(sprAction)))) return NULL;
  act->code = strcode(name);
  if(NULL == (act->label = STRDUP(name)))
    {
      FREE(act);
      return NULL;
    }
  spr_layout->actTarget = act;

  /* アクション定義ブロックの初期化 */
  act->track_nums = 0;
  act->begin = act->end = NULL;

  /* リンクに追加 */
  act->next = NULL;
  act->prev = spr_layout->actEnd;
  if(NULL != spr_layout->actEnd) spr_layout->actEnd->next = act;
  spr_layout->actEnd = act;
  if(NULL == spr_layout->actBegin) spr_layout->actBegin = act;
  
  return act;
}

/*
 * トラックの定義。name が NULL の場合はシグナルトラックの定義になる。
 */
sprTrack * track_2d_new(char * name)
{
  sprObject * obj;
  sprTrack * track;

  /* アクション定義前であればエラー */
  if(spr_layout->actTarget == NULL)
    {
      sprintf(err_msg, "No action defined\n");
      return NULL;
    }

  /* 指定された名前のオブジェクトが定義されていなければエラー */
  if(NULL != name)
    {
      if(NULL == (obj = search_object(name)))
	{
	  sprintf(err_msg, "Object '%s' is not defined.", name);
	  return NULL;
	}
    }
  else
    obj = NULL;

  /* 同じオブジェクトに対するトラックが既に存在していたらエラー */
  for(track = spr_layout->actTarget->begin;
      track != NULL; track = track->next)
    if(track->target == obj)
      {
	sprintf(err_msg, "Track for object '%s' is ready.\n", name);
	return NULL;
      }


  if(NULL == (track = MALLOC(sizeof(sprTrack)))) return NULL;
  track->target = obj;  /* シグナルトラックでは NULL */

  track->seq_step = 0;
  track->seq = NULL;

  /* トラックをアクションに登録 */ 
  spr_layout->actTarget->track_nums++;  /* 保持トラック数を追加 */
  track->next = NULL;
  track->prev = spr_layout->actTarget->end;

  if(spr_layout->actTarget->end != NULL)
    spr_layout->actTarget->end->next = track;

  spr_layout->actTarget->end = track;

  if(NULL == spr_layout->actTarget->begin)
    spr_layout->actTarget->begin = track;
  
  return track;
}

sprTrack * track_2d_gencode(sprOPCODE op, int v_time,
			    char * form, int spin, int sigvalue)
{
  sprTrack * track;
  sprStatus * stat;
  struct track_code * seq;
  int signal;

  if(spr_layout->actTarget == NULL) return NULL;
  track = spr_layout->actTarget->end;
  stat = NULL;
  if(form != NULL)
    if(op != TR_SIGN)
      if(NULL == (stat = search_status(track->target, form))) return NULL;
  
  DBG("[1]\n");
  if(track->seq == NULL)
    {
      if(NULL == (seq = MALLOC(sizeof(struct track_code)))) return NULL;
    }
  else
    if(NULL == (seq = REALLOC(track->seq,
			      sizeof(struct track_code) *
			      (track->seq_step + 1)))) return NULL;

  signal = (op == TR_SIGN) ? strcode(form) : -1;
  DBG("[2]\n");
  
  seq[track->seq_step].op       = op;
  seq[track->seq_step].v_time   = v_time;
  seq[track->seq_step].v_stat   = stat;
  seq[track->seq_step].signal   = signal;
  seq[track->seq_step].sigvalue = sigvalue;
  seq[track->seq_step].spin     = spin;
  DBG("[3]\n");

  track->seq = seq;
  track->seq_step++;
  return track;
}

char * my_strdup(char * str)
{
  char * ret;
  if(NULL == (ret = strdup(str)))
    sprintf(err_msg, "Not enough memory.");
  return ret;
}

void * my_malloc(size_t size)
{
  void * vpt;

  if(NULL == (vpt = malloc(size)))
    sprintf(err_msg, "Not enough memory.");

  return vpt;
}

void l2derror(void)
{
  fprintf(stderr, "Error(%d): %s\n", num_line , err_msg);
  exit(EXIT_FAILURE);
}
