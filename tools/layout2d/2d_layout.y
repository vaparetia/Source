%{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "2d_data.h"
#include "util.h"
#include "debug.h"

%}
%union {
       int    i_num; 
       float  f_num;
       char * str;
       sprID  spr_id;
       sprPos point;
       sprRGBA rgba;
}
%token NL
%token <str> STRING
%token <i_num> NUMBER
%token <f_num> FLOAT
%token OP_ADD
%token OP_SUB
%token OP_MUL
%token OP_DIV
%token OP_CUM
%token OP_EXC
%left OP_LBC;
%left OP_RBC;
%left OP_LP
%left OP_RP

%token OBJ
%token OBJ_VERTEX
%token OBJ_PARENT
%token <spr_id> OBJ_ATTRIB

%token REFTRI
%token STAT
%token STAT_MODE
%token STAT_MODE_END
%token STAT_END

%token MODE_TEX
%token MODE_SIZE
%token MODE_CENTER
%token MODE_ANGLE
%token MODE_UV
%token MODE_UVSIZE
%token MODE_TEXOFF
%token MODE_PRI
%token MODE_MAGNI
%token MODE_HIDE
%token MODE_SHOW
%token MODE_H_REV
%token MODE_V_REV

%token MODE_VERTEX
%token MODE_COLOR
%token MODE_ALPHA
%token <i_num> A_COL
%token <i_num> A_ALP

%token ACT
%token ACT_END

%token KEY
%token KEY_END

%token TRK
%token TRK_END
%token TRK_SET
%token TRK_WAIT
%token TRK_MORF
%token TRK_SIGN

%token <i_num> TRK_SPINDIR

%type <i_num> part
%type <i_num> i_expr
%type <f_num> f_expr
%type <str> name
%type <spr_id> attrib

%type <str> obj.prefix
%type <i_num> vertex.prefix
%type <str> parent.prefix

%type <point> vertex.point
%type <rgba>  color.rgba

%type <i_num> track.op.morf.spindir

%start s
%%

s	: command
	;

name	: STRING		{ $$ = $1; DBG("string \"%s\"\n", $1); }
	;
i_expr	: f_expr		{ $$ = (int)$1; }

f_expr	: f_expr OP_ADD f_expr	{ $$ = $1 + $3; }
	| f_expr OP_SUB f_expr	{ $$ = $1 - $3; }
	| f_expr OP_MUL f_expr	{ $$ = $1 * $3; }
	| f_expr OP_DIV f_expr	{ $$ = $1 / $3; }
	| OP_LP f_expr OP_RP	{ $$ = $2; }
	| FLOAT			{ $$ = $1; }
	;

part	: OP_LBC i_expr OP_EXC i_expr OP_RBC
	    {
	      $$ = (($2 & 0xfff) << 16) | ($4 & 0xfff);
	    }
	;
command	: /* empty */
	| command syntaxes NL
	;

syntaxes
	: /* empty */
	| obj.syntax	
	| stat.syntax
	| action.syntax
	;

obj.syntax
	: obj.prefix attrib vertex.prefix parent.prefix
	      {
		sprObject * o;
		DBG("{1}\n");
		o = object_2d_new($1);
		DBG("{2}\n");
		if(NULL == o) l2derror();
		object_2d_set_class(o, $2);
		DBG("{3}\n");
		if($2 == SP_LINESTRIP) object_2d_set_vertex(o, $3);
		DBG("{4}\n");
		if($4 != NULL) object_2d_set_parent(o, $4);
		DBG("{5}\n");
	      }
	;

obj.prefix
	: OBJ name	{ $$ = $2; }
	;

vertex.prefix
	: /* empty */		{ $$ = 0; }
	| OBJ_VERTEX i_expr	{ $$ = $2; }
	;

parent.prefix
	: /* empty */		{ $$ = NULL; }
	| OBJ_PARENT name	{ $$ = $2; }
	;

attrib	: OBJ_ATTRIB		{ $$ = $1; }
	;

stat.syntax
	: tri.define
	| stat.begin stat.modes stat.end
	;

tri.define
	: REFTRI name
			{
			  DBG("tri name [%s]\n", $2);
			  if(NULL == statblock_2d_tri($2)) l2derror();
			  DBG("tri success.\n");
			}
	;

stat.begin
	: STAT name NL
			{
			  DBG("begin status [%s]\n", $2);
			  if(NULL == statblock_2d_new($2)) l2derror();
			}
	;

stat.end
	: STAT_END
			{
			  DBG("end status\n");
			  spr_layout->blockTarget = NULL;
			}
	;

stat.modes
	: /* empty */
	| stat.modes mode.block NL

mode.block
	: /* empty */
	| mode.begin mode.defines mode.end
	;

mode.begin
	: STAT_MODE name NL
			{
			
		          DBG("mode defined [%s]\n", $2);
			  {
			    void * ptr = mode_2d_new($2);
			    if((ptr == NULL) && (trad_flag == 0)) l2derror();
			  }
			}
	;

mode.end
	: STAT_MODE_END
			{
				DBG("mode end\n");
			}
	;

mode.defines
	: /* empty */
	| mode.defines mode.commands
	;

mode.commands
	: mode.syntaxes NL
	;

mode.syntaxes
	: /* empty */
	| tex.syntax
	| vertex.syntax
	| size.syntax
	| center.syntax
	| angle.syntax
	| uv.syntax
	| uvsize.syntax
	| texoff.syntax
	| magni.syntax
	| pri.syntax
	| color.syntax
	| alpha.syntax
	| hide.syntax
	| show.syntax
	| hrev.syntax
	| vrev.syntax
	;
tex.syntax
	: MODE_TEX name
		{
		  DBG("tex: %s\n", $2);
		  spr_layout->blockTarget->d.st.end->tex_name = strdup($2);
		  spr_layout->blockTarget->d.st.end->tex_code = strcode($2);
		  spr_layout->blockTarget->d.st.end->tex_sw   = 1;

		  /* 現在の TRI のメンバとして、指定されたテクスチャ名称を
		     追加する */
		  tri_add_texture(spr_layout->blockTarget->d.st.end->tex_name);
		}
	;

center.syntax
	: MODE_CENTER f_expr f_expr
		{
		  DBG("center: %8.3f %8.3f\n", $2, $3);
		  if(spr_layout->blockTarget->d.st.target->obj_id != SP_SPIN)
		    {
		      /* error */
		    }

		  spr_layout->blockTarget->d.st.end->center_x = $2;
		  spr_layout->blockTarget->d.st.end->center_y = $3;
		}
	;
angle.syntax
	: MODE_ANGLE f_expr
		{
		  DBG("angle: %8.3f\n", $2);
		  if(spr_layout->blockTarget->d.st.target->obj_id != SP_SPIN)
		    {
		      /* error */
		    }

		  spr_layout->blockTarget->d.st.end->angle = $2;
		}
	;
size.syntax
	: MODE_SIZE f_expr f_expr
		{
		  DBG("size: %8.3f x %8.3f\n", $2, $3);
		  spr_layout->blockTarget->d.st.end->width  = $2;
		  spr_layout->blockTarget->d.st.end->height = $3;
		}
	;
uv.syntax
	: MODE_UV f_expr f_expr
		{
		  DBG("uv: %8.3f x %8.3f\n", $2, $3);
		  spr_layout->blockTarget->d.st.end->u = $2;
		  spr_layout->blockTarget->d.st.end->v = $3;
		  spr_layout->blockTarget->d.st.end->iu = 0;
		  spr_layout->blockTarget->d.st.end->iv = 0;
		}
	| MODE_UV part part
		{
		  DBG("uv: %d/%d , %d/%d\n", ($2 >> 16) & 0xfff, $2 & 0xfff, ($3 >> 16) & 0xfff, $3 & 0xfff);
		  spr_layout->blockTarget->d.st.end->iu = $2;
		  spr_layout->blockTarget->d.st.end->iv = $3;
		}
	;
uvsize.syntax
	: MODE_UVSIZE f_expr f_expr
		{
		  DBG("uvsize: %8.3f x %8.3f\n", $2, $3);
		  spr_layout->blockTarget->d.st.end->uw = $2;
		  spr_layout->blockTarget->d.st.end->vh = $3;
		  spr_layout->blockTarget->d.st.end->iuw = 0;
		  spr_layout->blockTarget->d.st.end->ivh = 0;
		}
	| MODE_UVSIZE part part
		{
		  DBG("uvsize: %d/%d , %d/%d\n", ($2 >> 16) & 0xfff, $2 & 0xfff, ($3 >> 16) & 0xfff, $3 & 0xfff);
		  spr_layout->blockTarget->d.st.end->iuw = $2;
		  spr_layout->blockTarget->d.st.end->ivh = $3;
		}
	;

texoff.syntax
	: MODE_TEXOFF
		{
		  DBG("texoff\n");
		  spr_layout->blockTarget->d.st.end->tex_sw = 0;
		}
	;

pri.syntax
	: MODE_PRI i_expr
		{
		  DBG("pri adjust: +%d\n", $2);
		  spr_layout->blockTarget->d.st.end->pri = $2;
		}
	;

magni.syntax
	: MODE_MAGNI f_expr
		{
		  DBG("magnification: %8.3f\n", $2);
		  spr_layout->blockTarget->d.st.end->magni = $2;
		}
	;

hide.syntax
	: MODE_HIDE
		{
		  DBG("hide\n");
		  spr_layout->blockTarget->d.st.end->disp = 0;
		}
	;
show.syntax
	: MODE_SHOW
		{
		  DBG("show\n");	
		  spr_layout->blockTarget->d.st.end->disp = 1;
		}
	;
hrev.syntax
	: MODE_H_REV
		{
		  DBG("hrev\n");
		  spr_layout->blockTarget->d.st.end->h_rev = 1;
		}
	;

vrev.syntax
	: MODE_V_REV
		{
		  DBG("vrev\n");
		  spr_layout->blockTarget->d.st.end->v_rev = 1;
		}
	;

vertex.syntax
	: mode.vertex.prefix mode.vertex.array
	;

mode.vertex.prefix
	: MODE_VERTEX i_expr
		{
		  /* モードのワークに頂点番号を設定 */
		  spr_layout->blockTarget->d.st.end->vertex_index = $2;
		}
mode.vertex.array
	: vertex.point
		{
		  sprStatus * stat = spr_layout->blockTarget->d.st.end;

		  DBG("setup vertex %d = %8.3f,%8.3f\n", stat->vertex_index, $1.x, $1.y);
		  if(spr_layout->blockTarget->d.st.target->vertex_num
		     <= stat->vertex_index)
		    {
		      /* error */

		    }


		  /* 座標値を、現在の頂点に設定し、頂点番号を更新 */
		  stat->vertex[stat->vertex_index].x = $1.x;
		  stat->vertex[stat->vertex_index].y = $1.y;
		  stat->vertex[stat->vertex_index].pos = 1; /* 座標を使用 */
		  stat->vertex_index++;
		}
	| mode.vertex.array OP_EXC vertex.point
		{
		  /* 座標値を、現在の頂点に設定し、頂点番号を更新 */
		  sprStatus * stat = spr_layout->blockTarget->d.st.end;
		  DBG("setup vertex %d = %8.3f,%8.3f\n", stat->vertex_index, $3.x, $3.y);
		  if(spr_layout->blockTarget->d.st.target->vertex_num
		     <= stat->vertex_index)
		    {
		      /* error */

		    }


		  /* 座標値を、現在の頂点に設定し、頂点番号を更新 */
		  stat->vertex[stat->vertex_index].x = $3.x;
		  stat->vertex[stat->vertex_index].y = $3.y;
		  stat->vertex[stat->vertex_index].pos = 1; /* 座標を使用 */
		  stat->vertex_index++;
		}
	;

vertex.point
	: OP_LP f_expr OP_CUM f_expr OP_RP { $$.x = $2; $$.y = $4; }
	;


color.syntax
	: mode.color.prefix mode.color.array
	;

mode.color.prefix
	: MODE_COLOR i_expr
		{
		  /* モードのワークに頂点番号を設定 */
		  spr_layout->blockTarget->d.st.end->vertex_index = $2;
		}

mode.color.array
	: color.rgba
		{
		  sprStatus * stat = spr_layout->blockTarget->d.st.end;

		  DBG("setup rgba %d\n", stat->vertex_index);

		  if(spr_layout->blockTarget->d.st.target->rgba_num
		     <= stat->vertex_index)
		    {
		      /* error */

		    }

		  /* 座標値を、現在の頂点に設定し、頂点番号を更新 */
		  stat->vertex[stat->vertex_index].r = $1.r;
		  stat->vertex[stat->vertex_index].g = $1.g;
		  stat->vertex[stat->vertex_index].b = $1.b;
		  stat->vertex[stat->vertex_index].a = $1.a;
		  stat->vertex[stat->vertex_index].col = 1; /* RGBAを使用 */
		  stat->vertex_index++;
		}
	| mode.color.array color.rgba
		{
		  sprStatus * stat = spr_layout->blockTarget->d.st.end;

		  DBG("setup rgba %d\n", stat->vertex_index);
		  if(spr_layout->blockTarget->d.st.target->rgba_num
		     <= stat->vertex_index)
		    {
		      /* error */

		    }

		  /* 座標値を、現在の頂点に設定し、頂点番号を更新 */
		  stat->vertex[stat->vertex_index].r = $2.r;
		  stat->vertex[stat->vertex_index].g = $2.g;
		  stat->vertex[stat->vertex_index].b = $2.b;
		  stat->vertex[stat->vertex_index].a = $2.a;
		  stat->vertex[stat->vertex_index].col = 1; /* RGBAを使用 */
		  stat->vertex_index++;
		}
	;

color.rgba
	: OP_LBC i_expr OP_CUM i_expr OP_CUM i_expr OP_CUM i_expr OP_RBC
		{
		  $$.r = $2;
		  $$.g = $4;
		  $$.b = $6;
		  $$.a = $8;
		}
	;
alpha.syntax
	: MODE_ALPHA A_COL A_COL A_ALP A_COL
		{
		  sprStatus * stat = spr_layout->blockTarget->d.st.end;
		  stat->alpha = ($5 << 6) | ($4 << 4) | ($3 << 2) | $2;
		  if($4 == 2)
		    {
		      /* error */
		    }
		  DBG("alpha %d %d %d %d\n", $2, $3, $4, $5);
		}
	| MODE_ALPHA A_COL A_COL A_ALP A_COL i_expr
		{
		  sprStatus * stat = spr_layout->blockTarget->d.st.end;
		  if($4 != 2)
		    {
		      /* error */
		    }
		  stat->alpha = ($5 << 6) | ($4 << 4) | ($3 << 2) | $2;
		  stat->v_alpha = $6;
		  DBG("alpha %d %d %d %d %d\n", $2, $3, $4, $5, $6);
		}
	;


action.syntax
	: act.begin act.tracks act.end
	;

act.begin
	: ACT name NL
		{
		  DBG("action [%s]\n", $2);
		  if(NULL == action_2d_new($2)) l2derror();
		}
	;
act.end
	: ACT_END
		{
		  DBG("actend\n");
		  spr_layout->actTarget = NULL;
		}
	;
act.tracks
	: /* empty */
	| act.tracks track.syntax NL
	;

track.syntax
	: /* empty */
	| track.begin track.commands track.end
	| key.begin key.commands key.end
	;

key.begin
	: KEY NL
	        {
		  DBG("keytrack\n");
		  /* NULL指定でシグナルトラック */
		  if(NULL == track_2d_new(NULL))
		    l2derror();
		}
	;
key.end
	: KEY_END
		{
		  DBG("keytrack end\n");
		  track_2d_gencode(TR_END, 0, NULL, 0, 0);
		}
	;

track.begin
	: TRK name NL
		{
		  if(NULL == track_2d_new($2))
		    l2derror();
		}
	;
track.end
	: TRK_END
		{
		  DBG("track end\n");
		  track_2d_gencode(TR_END, 0, NULL, 0, 0);
		  DBG("------\n");
		}
	;
track.commands
	: /* empty */
	| track.commands track.opcode NL
	;

key.commands
	: /* empty */
	| key.commands key.opcode NL
	;

track.opcode
	: /* empty */
	| track.op.set
	| track.op.wait
	| track.op.morf
	;

key.opcode
	: /* empty */
	| track.op.wait
	| track.op.signal
	;

track.op.set
	: TRK_SET name
		{
		  DBG("set [%s]\n", $2);
		  track_2d_gencode(TR_SET, 0, $2, 0, 0);
		}
	;
track.op.signal
	: TRK_SIGN name name
		{
		  int value;

		  value = strcode($3);
		  DBG("signal [%s] value = \"%s\"\n", $2, $3);
		  track_2d_gencode(TR_SIGN, 0, $2, 0, value);
		}
	| TRK_SIGN name i_expr
		{
		  DBG("signal [%s] value = %d\n", $2, $3);
		  track_2d_gencode(TR_SIGN, 0, $2, 0, $3);
		}
	;
track.op.wait
	: TRK_WAIT i_expr
		{
		  DBG("wait [%d]\n", $2);
		  track_2d_gencode(TR_WAIT, $2, NULL, 0, 0);
		}
	;
track.op.morf
	: TRK_MORF i_expr name  track.op.morf.spindir
		{
		  DBG("morf [%d] - [%s](%d)\n", $2, $3, $4);
		  track_2d_gencode(TR_MORF, $2, $3, $4, 0);
		}
	;
track.op.morf.spindir
	: /* empty */	{ $$ = 0; }
	| TRK_SPINDIR	{ $$ = $1; }
	;
%%
#include "lex.yy.c"

int yyerror(char * s)
{
  fprintf(stderr, "Error(%d): %s\n", num_line + 1, s);
  exit(EXIT_FAILURE);
}
