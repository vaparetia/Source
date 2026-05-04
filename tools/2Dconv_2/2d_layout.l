%{
#include "util.h"

int num_line = 0;
char lexstrbuf[ 256 ] = {0};
int strcnt = 0;
%}
%start DEF DEFSTR TRI TRISTR ST STSTR MODE MDSTR ALPHA AC ACSTR TR TRSTR MORF
%start COMM COMM_DEF COMM_TRI COMM_ST COMM_MD COMM_ALP COMM_AC COMM_TR COMM_MF
%%
<INITIAL>"/*"		{ BEGIN COMM; }
<COMM>"*/"		{ BEGIN INITIAL; }

<DEF>"/*"		{ BEGIN COMM_DEF; }
<COMM_DEF>"*/"		{ BEGIN DEF; }

<ST>"/*"		{ BEGIN COMM_ST; }
<COMM_ST>"*/"		{ BEGIN ST; }

<MODE>"/*"		{ BEGIN COMM_MD; }
<COMM_MD>"*/"		{ BEGIN MODE; }

<ALPHA>"/*"		{ BEGIN COMM_ALP; }
<COMM_ALP>"*/"		{ BEGIN ALPHA; }

<AC>"/*"		{ BEGIN COMM_AC; }
<COMM_AC>"*/"		{ BEGIN AC; }

<TR>"/*"		{ BEGIN COMM_TR; }
<COMM_TR>"*/"		{ BEGIN TR; }

<MORF>"/*"		{ BEGIN COMM_MF; }
<COMM_MF>"*/"		{ BEGIN MORF; }

<COMM,COMM_DEF,COMM_TRI,COMM_ST,COMM_MD,COMM_ALP,COMM_AC,COMM_TR,COMM_MF>.  ;


<INITIAL>obj		{
			  BEGIN DEF;
			  return OBJ;
			}
<INITIAL>status		{
			  BEGIN ST;
			  return STAT;
			}

<INITIAL>reftri		{
			  BEGIN TRI;
			  return REFTRI;
			}
<TRI>\"			{
			  BEGIN TRISTR;
			  strcnt = 0;
			}
<TRISTR>\\\"		{ lexstrbuf[strcnt++] = '\"'; }
<TRISTR>\\n		{ lexstrbuf[strcnt++] = '\n'; }
<TRISTR>\"		{
			  lexstrbuf[strcnt] = 0;
			  yylval.str = &poolbuf[pool(lexstrbuf)];
			  BEGIN INITIAL;
			  return STRING;
			}
<TRISTR>.		{ lexstrbuf[strcnt++] = yytext[0]; }

<INITIAL>action		{
			  BEGIN AC;
			  return ACT;
			}
<AC>actend		{
			  BEGIN INITIAL;
			  return ACT_END;
			}
<AC>track		{
			  BEGIN TR;
			  return TRK;
			}
<TR>trend		{
			  BEGIN AC;
			  return TRK_END;
			}
<TR>set			return TRK_SET;
<TR>wait		return TRK_WAIT;
<TR>morf		return TRK_MORF;
<TR>right		{
			  yylval.i_num = 0;
			  return TRK_SPINDIR;
			}
<TR>left		{
			  yylval.i_num = 1;
			  return TRK_SPINDIR;
			}
<TR>\"			{
			  BEGIN TRSTR;
			  strcnt = 0; /* 文字列を最初から */
			}
<TRSTR>\\\"		{ lexstrbuf[strcnt++] = '\"'; }
<TRSTR>\\n		{ lexstrbuf[strcnt++] = '\n'; }
<TRSTR>\"		{
			  lexstrbuf[strcnt] = 0;
			  yylval.str = &poolbuf[pool(lexstrbuf)];
			  BEGIN TR;
			  return STRING;
			}
<TRSTR>.		{ lexstrbuf[strcnt++] = yytext[0]; }

<AC>\"			{
			  BEGIN ACSTR;
			  strcnt = 0; /* 文字列を最初から */
			}
<ACSTR>\\\"		{ lexstrbuf[strcnt++] = '\"'; }
<ACSTR>\\n		{ lexstrbuf[strcnt++] = '\n'; }
<ACSTR>\"		{
			  lexstrbuf[strcnt] = 0;
			  yylval.str = &poolbuf[pool(lexstrbuf)];
			  BEGIN AC;
			  return STRING;
			}
<ACSTR>.		{ lexstrbuf[strcnt++] = yytext[0]; }

<DEF>\n			{
			  num_line++;
			  BEGIN INITIAL;
			  return NL;
			}
<DEF>empty		{
			  yylval.spr_id = SP_EMPTY;
			  return OBJ_ATTRIB;
			}
<DEF>point		{
			  yylval.spr_id = SP_POINT;
			  return OBJ_ATTRIB;
			}
<DEF>line		{
			  yylval.spr_id = SP_LINE;
			  return OBJ_ATTRIB;
			}
<DEF>lstrip		{
			  yylval.spr_id = SP_LINESTRIP;
			  return OBJ_ATTRIB;
			}
<DEF>linestrip		{
			  yylval.spr_id = SP_LINESTRIP;
			  return OBJ_ATTRIB;
			}
<DEF>box		{
			  yylval.spr_id = SP_BOX;
			  return OBJ_ATTRIB;
			}
<DEF>sprite		{
			  yylval.spr_id = SP_SPRITE;
			  return OBJ_ATTRIB;
			}
<DEF>zoom		{
			  yylval.spr_id = SP_ZOOM;
			  return OBJ_ATTRIB;
			}
<DEF>spin		{
			  yylval.spr_id = SP_SPIN;
			  DBG("SPIN!!\n");
			  return OBJ_ATTRIB;
			}
<DEF>vertex		return OBJ_VERTEX;
<DEF>parent		return OBJ_PARENT;
<DEF>\"			{
			  BEGIN DEFSTR;
			  strcnt = 0; /* 文字列を最初から */
			}
<DEFSTR>\\\"		{ lexstrbuf[strcnt++] = '\"'; }
<DEFSTR>\\n		{ lexstrbuf[strcnt++] = '\n'; }
<DEFSTR>\"		{
			  lexstrbuf[strcnt] = 0;
			  yylval.str = &poolbuf[pool(lexstrbuf)];
			  BEGIN DEF;
			  return STRING;
			}
<DEFSTR>.		{
			  DBG("str: %c\n", yytext[0]);
			  lexstrbuf[strcnt++] = yytext[0];
			}
<ST>\"			{
			  BEGIN STSTR;
			  strcnt = 0;
			}
<STSTR>\\\"		{ lexstrbuf[strcnt++] = '\"'; }
<STSTR>\\n		{ lexstrbuf[strcnt++] = '\n'; }
<STSTR>\"		{
			  lexstrbuf[strcnt] = 0;
			  yylval.str = &poolbuf[pool(lexstrbuf)];
			  BEGIN ST;
			  return STRING;
			}
<STSTR>.		{ lexstrbuf[strcnt++] = yytext[0]; }

<ST>endstat		{
			  BEGIN INITIAL;
			  return STAT_END;
			}

<ST>mode		{
			  BEGIN MODE;
			  return STAT_MODE;
			}

<MODE>endmode		{
			  BEGIN ST;
			  return STAT_MODE_END;
			}

<MODE>tex		return MODE_TEX;
<MODE>vertex		return MODE_VERTEX;
<MODE>size		return MODE_SIZE;
<MODE>center		return MODE_CENTER;
<MODE>angle		return MODE_ANGLE;
<MODE>uv		return MODE_UV;
<MODE>uvsize		return MODE_UVSIZE;
<MODE>texoff		return MODE_TEXOFF;
<MODE>color		return MODE_COLOR;
<MODE>alpha		{
			  BEGIN ALPHA;
			  return MODE_ALPHA;
			}
<ALPHA>Cs		{
			  yylval.i_num = 0;
			  return A_COL;
			}
<ALPHA>Cd		{
			  yylval.i_num = 1;
			  return A_COL;
			}
<ALPHA>0		{
			  yylval.i_num = 2;
			  return A_COL;
			}
<ALPHA>As		{
			  yylval.i_num = 0;
			  return A_ALP;
			}
<ALPHA>Ad		{
			  yylval.i_num = 1;
			  return A_ALP;
			}
<ALPHA>FIX		{
			  yylval.i_num = 2;
			  return A_ALP;
			}
<ALPHA>\n		{
			  num_line++;
			  BEGIN MODE;
			  return NL;
			}

<MODE>pri		return MODE_PRI;
<MODE>magni		return MODE_MAGNI;
<MODE>hide		return MODE_HIDE;
<MODE>show		return MODE_SHOW;
<MODE>\"		{
			  BEGIN MDSTR;
			  strcnt = 0;
			  DBG("mode string\n");
			}
<MDSTR>\\\"		{ lexstrbuf[strcnt++] = '\"'; }
<MDSTR>\\n		{ lexstrbuf[strcnt++] = '\n'; }
<MDSTR>\"		{
			  lexstrbuf[strcnt] = 0;
			  yylval.str = &poolbuf[pool(lexstrbuf)];
			  BEGIN MODE;
			  DBG("mode string end\n");
			  return STRING;
			}
<MDSTR>.		{ lexstrbuf[strcnt++] = yytext[0]; }

<INITIAL,ST,MODE,AC,TR>\n	{
				  num_line++;
				  return NL;
				}
"+"			return OP_ADD;
"-"			{ DBG("----\n"); return OP_SUB; }
"*"			return OP_MUL;
"/"			return OP_DIV;
"("			return OP_LP;
")"			return OP_RP;
","			return OP_CUM;
"!"			return OP_EXC;
"{"			return OP_LBC;
"}"			return OP_RBC;
<INITIAL,DEF,TR,MODE>[-\.0-9]+		{
			  DBG("FLOAT\n");
			  yylval.f_num = atof(yytext);
			  return FLOAT;
			}
[ \t]			;
.			;

%%
