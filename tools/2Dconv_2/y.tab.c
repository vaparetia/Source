
/*  A Bison parser, made from 2d_layout.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	NL	257
#define	STRING	258
#define	NUMBER	259
#define	FLOAT	260
#define	OP_ADD	261
#define	OP_SUB	262
#define	OP_MUL	263
#define	OP_DIV	264
#define	OP_CUM	265
#define	OP_EXC	266
#define	OP_LBC	267
#define	OP_RBC	268
#define	OP_LP	269
#define	OP_RP	270
#define	OBJ	271
#define	OBJ_VERTEX	272
#define	OBJ_PARENT	273
#define	OBJ_ATTRIB	274
#define	REFTRI	275
#define	STAT	276
#define	STAT_MODE	277
#define	STAT_MODE_END	278
#define	STAT_END	279
#define	MODE_TEX	280
#define	MODE_SIZE	281
#define	MODE_CENTER	282
#define	MODE_ANGLE	283
#define	MODE_UV	284
#define	MODE_UVSIZE	285
#define	MODE_TEXOFF	286
#define	MODE_PRI	287
#define	MODE_MAGNI	288
#define	MODE_HIDE	289
#define	MODE_SHOW	290
#define	MODE_VERTEX	291
#define	MODE_COLOR	292
#define	MODE_ALPHA	293
#define	A_COL	294
#define	A_ALP	295
#define	ACT	296
#define	ACT_END	297
#define	TRK	298
#define	TRK_END	299
#define	TRK_SET	300
#define	TRK_WAIT	301
#define	TRK_MORF	302
#define	TRK_SPINDIR	303

#line 1 "2d_layout.y"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "2d_data.h"
#include "util.h"
#include "debug.h"


#line 11 "2d_layout.y"
typedef union {
       int    i_num; 
       float  f_num;
       char * str;
       sprID  spr_id;
       sprPos point;
       sprRGBA rgba;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		156
#define	YYFLAG		-32768
#define	YYNTBASE	50

#define YYTRANSLATE(x) ((unsigned)(x) <= 303 ? yytranslate[x] : 105)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     6,    10,    14,    18,    22,    26,    28,
    29,    33,    34,    36,    38,    40,    45,    48,    49,    52,
    53,    56,    58,    60,    64,    67,    71,    73,    74,    78,
    79,    83,    87,    89,    90,    93,    96,    97,    99,   101,
   103,   105,   107,   109,   111,   113,   115,   117,   119,   121,
   123,   125,   128,   132,   135,   139,   143,   147,   149,   152,
   155,   157,   159,   162,   165,   167,   171,   177,   180,   183,
   185,   188,   198,   204,   211,   215,   219,   221,   222,   226,
   227,   231,   235,   237,   238,   242,   243,   245,   247,   249,
   252,   255,   260,   261
};

static const short yyrhs[] = {    54,
     0,     4,     0,    53,     0,    53,     7,    53,     0,    53,
     8,    53,     0,    53,     9,    53,     0,    53,    10,    53,
     0,    15,    53,    16,     0,     6,     0,     0,    54,    55,
     3,     0,     0,    56,     0,    61,     0,    92,     0,    57,
    60,    58,    59,     0,    17,    51,     0,     0,    18,    52,
     0,     0,    19,    51,     0,    20,     0,    62,     0,    63,
    65,    64,     0,    21,    51,     0,    22,    51,     3,     0,
    25,     0,     0,    65,    66,     3,     0,     0,    67,    69,
    68,     0,    23,    51,     3,     0,    24,     0,     0,    69,
    70,     0,    71,     3,     0,     0,    72,     0,    83,     0,
    75,     0,    73,     0,    74,     0,    76,     0,    77,     0,
    78,     0,    80,     0,    79,     0,    87,     0,    91,     0,
    81,     0,    82,     0,    26,    51,     0,    28,    53,    53,
     0,    29,    53,     0,    27,    53,    53,     0,    30,    53,
    53,     0,    31,    53,    53,     0,    32,     0,    33,    52,
     0,    34,    53,     0,    35,     0,    36,     0,    84,    85,
     0,    37,    52,     0,    86,     0,    85,    12,    86,     0,
    15,    53,    11,    53,    16,     0,    88,    89,     0,    38,
    52,     0,    90,     0,    89,    90,     0,    13,    52,    11,
    52,    11,    52,    11,    52,    14,     0,    39,    40,    40,
    41,    40,     0,    39,    40,    40,    41,    40,    52,     0,
    93,    95,    94,     0,    42,    51,     3,     0,    43,     0,
     0,    95,    96,     3,     0,     0,    97,    99,    98,     0,
    44,    51,     3,     0,    45,     0,     0,    99,   100,     3,
     0,     0,   101,     0,   102,     0,   103,     0,    46,    51,
     0,    47,    52,     0,    48,    52,    51,   104,     0,     0,
    49,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    91,    94,    96,    98,    99,   100,   101,   102,   103,   106,
   107,   111,   112,   113,   114,   118,   135,   139,   140,   144,
   145,   148,   152,   153,   157,   166,   174,   182,   183,   186,
   187,   191,   200,   207,   208,   212,   216,   217,   218,   219,
   220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
   230,   233,   247,   260,   272,   280,   288,   297,   305,   313,
   321,   328,   336,   340,   346,   365,   387,   392,   396,   403,
   424,   446,   455,   465,   480,   484,   491,   498,   499,   503,
   504,   507,   514,   521,   522,   525,   526,   527,   528,   531,
   538,   545,   552,   553
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","NL","STRING",
"NUMBER","FLOAT","OP_ADD","OP_SUB","OP_MUL","OP_DIV","OP_CUM","OP_EXC","OP_LBC",
"OP_RBC","OP_LP","OP_RP","OBJ","OBJ_VERTEX","OBJ_PARENT","OBJ_ATTRIB","REFTRI",
"STAT","STAT_MODE","STAT_MODE_END","STAT_END","MODE_TEX","MODE_SIZE","MODE_CENTER",
"MODE_ANGLE","MODE_UV","MODE_UVSIZE","MODE_TEXOFF","MODE_PRI","MODE_MAGNI","MODE_HIDE",
"MODE_SHOW","MODE_VERTEX","MODE_COLOR","MODE_ALPHA","A_COL","A_ALP","ACT","ACT_END",
"TRK","TRK_END","TRK_SET","TRK_WAIT","TRK_MORF","TRK_SPINDIR","s","name","i_expr",
"f_expr","command","syntaxes","obj.syntax","obj.prefix","vertex.prefix","parent.prefix",
"attrib","stat.syntax","tri.define","stat.begin","stat.end","stat.modes","mode.block",
"mode.begin","mode.end","mode.defines","mode.commands","mode.syntaxes","tex.syntax",
"center.syntax","angle.syntax","size.syntax","uv.syntax","uvsize.syntax","texoff.syntax",
"pri.syntax","magni.syntax","hide.syntax","show.syntax","vertex.syntax","mode.vertex.prefix",
"mode.vertex.array","vertex.point","color.syntax","mode.color.prefix","mode.color.array",
"color.rgba","alpha.syntax","action.syntax","act.begin","act.end","act.tracks",
"track.syntax","track.begin","track.end","track.commands","track.opcode","track.op.set",
"track.op.wait","track.op.morf","track.op.morf.spindir", NULL
};
#endif

static const short yyr1[] = {     0,
    50,    51,    52,    53,    53,    53,    53,    53,    53,    54,
    54,    55,    55,    55,    55,    56,    57,    58,    58,    59,
    59,    60,    61,    61,    62,    63,    64,    65,    65,    66,
    66,    67,    68,    69,    69,    70,    71,    71,    71,    71,
    71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
    71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
    81,    82,    83,    84,    85,    85,    86,    87,    88,    89,
    89,    90,    91,    91,    92,    93,    94,    95,    95,    96,
    96,    97,    98,    99,    99,   100,   100,   100,   100,   101,
   102,   103,   104,   104
};

static const short yyr2[] = {     0,
     1,     1,     1,     3,     3,     3,     3,     3,     1,     0,
     3,     0,     1,     1,     1,     4,     2,     0,     2,     0,
     2,     1,     1,     3,     2,     3,     1,     0,     3,     0,
     3,     3,     1,     0,     2,     2,     0,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     2,     3,     2,     3,     3,     3,     1,     2,     2,
     1,     1,     2,     2,     1,     3,     5,     2,     2,     1,
     2,     9,     5,     6,     3,     3,     1,     0,     3,     0,
     3,     3,     1,     0,     3,     0,     1,     1,     1,     2,
     2,     4,     0,     1
};

static const short yydefact[] = {    10,
     1,     0,     0,     0,     0,     0,    13,     0,    14,    23,
    28,    15,    78,     2,    17,    25,     0,     0,    11,    22,
    18,    30,    80,    26,    76,     0,    20,     0,    27,    24,
     0,    34,    77,     0,    75,     0,    84,     9,     0,    19,
     3,     0,    16,     0,    29,    37,     0,    79,    86,     0,
     0,     0,     0,     0,    21,    32,    33,     0,     0,     0,
     0,     0,     0,    58,     0,     0,    61,    62,     0,     0,
     0,    31,    35,     0,    38,    41,    42,    40,    43,    44,
    45,    47,    46,    50,    51,    39,     0,    48,     0,    49,
    82,    83,     0,     0,     0,    81,     0,    87,    88,    89,
     8,     4,     5,     6,     7,    52,     0,     0,    54,     0,
     0,    59,    60,    64,    69,     0,    36,     0,    63,    65,
     0,    68,    70,    90,    91,     0,    85,    55,    53,    56,
    57,     0,     0,     0,     0,    71,    93,     0,     0,    66,
     0,    94,    92,    73,     0,     0,    74,    67,     0,     0,
     0,     0,    72,     0,     0,     0
};

static const short yydefgoto[] = {   154,
    15,    40,    41,     1,     6,     7,     8,    27,    43,    21,
     9,    10,    11,    30,    22,    31,    32,    72,    46,    73,
    74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
    84,    85,    86,    87,   119,   120,    88,    89,   122,   123,
    90,    12,    13,    35,    23,    36,    37,    96,    49,    97,
    98,    99,   100,   143
};

static const short yypact[] = {-32768,
    20,     3,     3,     3,     3,     5,-32768,    -9,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,     9,    21,-32768,-32768,
    14,    13,   -40,-32768,-32768,    45,    34,     3,-32768,-32768,
    37,-32768,-32768,     3,-32768,    51,-32768,-32768,    45,-32768,
    61,     3,-32768,    54,-32768,    81,    55,-32768,    47,    36,
    45,    45,    45,    45,-32768,-32768,-32768,     3,    45,    45,
    45,    45,    45,-32768,    45,    45,-32768,-32768,    45,    45,
    19,-32768,-32768,    69,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    58,-32768,    63,-32768,
-32768,-32768,     3,    45,    45,-32768,    76,-32768,-32768,-32768,
-32768,    61,    61,    61,    61,-32768,     7,     7,    61,     7,
     7,-32768,    61,-32768,-32768,    42,-32768,    45,    68,-32768,
    45,    63,-32768,-32768,-32768,     3,-32768,    61,    61,    61,
    61,    46,    56,    58,    72,-32768,    39,    57,    45,-32768,
    45,-32768,-32768,    45,    40,    75,-32768,-32768,    45,    85,
    45,    84,-32768,    99,   100,-32768
};

static const short yypgoto[] = {-32768,
    -3,   -60,   -33,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   -32,-32768,-32768,-32768,   -21,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768
};


#define	YYLAST		123


static const short yytable[] = {    16,
    17,    18,    33,    34,   112,    50,    14,    19,   114,   115,
    20,    24,    38,    51,    52,    53,    54,   102,   103,   104,
   105,    39,   -12,    25,    44,   107,   108,   109,   110,   111,
    47,    26,   113,   125,   126,    28,     2,    29,    55,    45,
     3,     4,    51,    52,    53,    54,    51,    52,    53,    54,
    38,   101,    42,    48,   106,   148,    56,    91,   116,    39,
   135,     5,    51,    52,    53,    54,   139,    51,    52,    53,
    54,   117,   118,   128,   129,   121,   130,   131,   127,   134,
   146,   132,   141,   147,   133,   149,   138,   142,   150,   124,
   152,    92,    93,    94,    95,   151,   144,   153,   155,   156,
   136,   140,     0,     0,    57,   145,    58,    59,    60,    61,
    62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
     0,     0,   137
};

static const short yycheck[] = {     3,
     4,     5,    43,    44,    65,    39,     4,     3,    69,    70,
    20,     3,     6,     7,     8,     9,    10,    51,    52,    53,
    54,    15,     3,     3,    28,    59,    60,    61,    62,    63,
    34,    18,    66,    94,    95,    23,    17,    25,    42,     3,
    21,    22,     7,     8,     9,    10,     7,     8,     9,    10,
     6,    16,    19,     3,    58,    16,     3,     3,    40,    15,
   121,    42,     7,     8,     9,    10,    11,     7,     8,     9,
    10,     3,    15,   107,   108,    13,   110,   111,     3,    12,
   141,    40,    11,   144,   118,    11,    41,    49,   149,    93,
   151,    45,    46,    47,    48,    11,    40,    14,     0,     0,
   122,   134,    -1,    -1,    24,   139,    26,    27,    28,    29,
    30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
    -1,    -1,   126
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 2:
#line 94 "2d_layout.y"
{ yyval.str = yyvsp[0].str; DBG("string \"%s\"\n", yyvsp[0].str); ;
    break;}
case 3:
#line 96 "2d_layout.y"
{ yyval.i_num = (int)yyvsp[0].f_num; ;
    break;}
case 4:
#line 98 "2d_layout.y"
{ yyval.f_num = yyvsp[-2].f_num + yyvsp[0].f_num; ;
    break;}
case 5:
#line 99 "2d_layout.y"
{ yyval.f_num = yyvsp[-2].f_num - yyvsp[0].f_num; ;
    break;}
case 6:
#line 100 "2d_layout.y"
{ yyval.f_num = yyvsp[-2].f_num * yyvsp[0].f_num; ;
    break;}
case 7:
#line 101 "2d_layout.y"
{ yyval.f_num = yyvsp[-2].f_num / yyvsp[0].f_num; ;
    break;}
case 8:
#line 102 "2d_layout.y"
{ yyval.f_num = yyvsp[-1].f_num; ;
    break;}
case 9:
#line 103 "2d_layout.y"
{ yyval.f_num = yyvsp[0].f_num; ;
    break;}
case 16:
#line 119 "2d_layout.y"
{
		sprObject * o;
		DBG("{1}\n");
		o = object_2d_new(yyvsp[-3].str);
		DBG("{2}\n");
		if(NULL == o) l2derror();
		object_2d_set_class(o, yyvsp[-2].spr_id);
		DBG("{3}\n");
		if(yyvsp[-2].spr_id == SP_LINESTRIP) object_2d_set_vertex(o, yyvsp[-1].i_num);
		DBG("{4}\n");
		if(yyvsp[0].str != NULL) object_2d_set_parent(o, yyvsp[0].str);
		DBG("{5}\n");
	      ;
    break;}
case 17:
#line 135 "2d_layout.y"
{ yyval.str = yyvsp[0].str; ;
    break;}
case 18:
#line 139 "2d_layout.y"
{ yyval.i_num = 0; ;
    break;}
case 19:
#line 140 "2d_layout.y"
{ yyval.i_num = yyvsp[0].i_num; ;
    break;}
case 20:
#line 144 "2d_layout.y"
{ yyval.str = NULL; ;
    break;}
case 21:
#line 145 "2d_layout.y"
{ yyval.str = yyvsp[0].str; ;
    break;}
case 22:
#line 148 "2d_layout.y"
{ yyval.spr_id = yyvsp[0].spr_id; ;
    break;}
case 25:
#line 158 "2d_layout.y"
{
			  DBG("tri name [%s]\n", yyvsp[0].str);
			  if(NULL == statblock_2d_tri(yyvsp[0].str)) l2derror();
			  DBG("tri success.\n");
			;
    break;}
case 26:
#line 167 "2d_layout.y"
{
			  DBG("begin status [%s]\n", yyvsp[-1].str);
			  if(NULL == statblock_2d_new(yyvsp[-1].str)) l2derror();
			;
    break;}
case 27:
#line 175 "2d_layout.y"
{
			  DBG("end status\n");
			  spr_layout->blockTarget = NULL;
			;
    break;}
case 32:
#line 192 "2d_layout.y"
{
			
		          DBG("mode defined [%s]\n", yyvsp[-1].str);
			  mode_2d_new(yyvsp[-1].str);
			;
    break;}
case 33:
#line 201 "2d_layout.y"
{
				DBG("mode end\n");
			;
    break;}
case 52:
#line 234 "2d_layout.y"
{
		  DBG("tex: %s\n", yyvsp[0].str);
		  spr_layout->blockTarget->d.st.end->tex_name = strdup(yyvsp[0].str);
		  spr_layout->blockTarget->d.st.end->tex_code = strcode(yyvsp[0].str);
		  spr_layout->blockTarget->d.st.end->tex_sw   = 1;

		  /* 現在の TRI のメンバとして、指定されたテクスチャ名称を
		     追加する */
		  tri_add_texture(spr_layout->blockTarget->d.st.end->tex_name);
		;
    break;}
case 53:
#line 248 "2d_layout.y"
{
		  DBG("center: %8.3f %8.3f\n", yyvsp[-1].f_num, yyvsp[0].f_num);
		  if(spr_layout->blockTarget->d.st.target->obj_id != SP_SPIN)
		    {
		      /* error */
		    }

		  spr_layout->blockTarget->d.st.end->center_x = yyvsp[-1].f_num;
		  spr_layout->blockTarget->d.st.end->center_y = yyvsp[0].f_num;
		;
    break;}
case 54:
#line 261 "2d_layout.y"
{
		  DBG("angle: %8.3f\n", yyvsp[0].f_num);
		  if(spr_layout->blockTarget->d.st.target->obj_id != SP_SPIN)
		    {
		      /* error */
		    }

		  spr_layout->blockTarget->d.st.end->angle = yyvsp[0].f_num;
		;
    break;}
case 55:
#line 273 "2d_layout.y"
{
		  DBG("size: %8.3f x %8.3f\n", yyvsp[-1].f_num, yyvsp[0].f_num);
		  spr_layout->blockTarget->d.st.end->width  = yyvsp[-1].f_num;
		  spr_layout->blockTarget->d.st.end->height = yyvsp[0].f_num;
		;
    break;}
case 56:
#line 281 "2d_layout.y"
{
		  DBG("uv: %8.3f x %8.3f\n", yyvsp[-1].f_num, yyvsp[0].f_num);
		  spr_layout->blockTarget->d.st.end->u = yyvsp[-1].f_num;
		  spr_layout->blockTarget->d.st.end->v = yyvsp[0].f_num;
		;
    break;}
case 57:
#line 289 "2d_layout.y"
{
		  DBG("uvsize: %8.3f x %8.3f\n", yyvsp[-1].f_num, yyvsp[0].f_num);
		  spr_layout->blockTarget->d.st.end->uw = yyvsp[-1].f_num;
		  spr_layout->blockTarget->d.st.end->vh = yyvsp[0].f_num;
		;
    break;}
case 58:
#line 298 "2d_layout.y"
{
		  DBG("texoff\n");
		  spr_layout->blockTarget->d.st.end->tex_sw = 0;
		;
    break;}
case 59:
#line 306 "2d_layout.y"
{
		  DBG("pri adjust: +%d\n", yyvsp[0].i_num);
		  spr_layout->blockTarget->d.st.end->pri = yyvsp[0].i_num;
		;
    break;}
case 60:
#line 314 "2d_layout.y"
{
		  DBG("magnification: %8.3f\n", yyvsp[0].f_num);
		  spr_layout->blockTarget->d.st.end->magni = yyvsp[0].f_num;
		;
    break;}
case 61:
#line 322 "2d_layout.y"
{
		  DBG("hide\n");
		  spr_layout->blockTarget->d.st.end->disp = 0;
		;
    break;}
case 62:
#line 329 "2d_layout.y"
{
		  DBG("show\n");	
		  spr_layout->blockTarget->d.st.end->disp = 1;
		;
    break;}
case 64:
#line 341 "2d_layout.y"
{
		  /* モードのワークに頂点番号を設定 */
		  spr_layout->blockTarget->d.st.end->vertex_index = yyvsp[0].i_num;
		;
    break;}
case 65:
#line 347 "2d_layout.y"
{
		  sprStatus * stat = spr_layout->blockTarget->d.st.end;

		  DBG("setup vertex %d = %8.3f,%8.3f\n", stat->vertex_index, yyvsp[0].point.x, yyvsp[0].point.y);
		  if(spr_layout->blockTarget->d.st.target->vertex_num
		     <= stat->vertex_index)
		    {
		      /* error */

		    }


		  /* 座標値を、現在の頂点に設定し、頂点番号を更新 */
		  stat->vertex[stat->vertex_index].x = yyvsp[0].point.x;
		  stat->vertex[stat->vertex_index].y = yyvsp[0].point.y;
		  stat->vertex[stat->vertex_index].pos = 1; /* 座標を使用 */
		  stat->vertex_index++;
		;
    break;}
case 66:
#line 366 "2d_layout.y"
{
		  /* 座標値を、現在の頂点に設定し、頂点番号を更新 */
		  sprStatus * stat = spr_layout->blockTarget->d.st.end;
		  DBG("setup vertex %d = %8.3f,%8.3f\n", stat->vertex_index, yyvsp[0].point.x, yyvsp[0].point.y);
		  if(spr_layout->blockTarget->d.st.target->vertex_num
		     <= stat->vertex_index)
		    {
		      /* error */

		    }


		  /* 座標値を、現在の頂点に設定し、頂点番号を更新 */
		  stat->vertex[stat->vertex_index].x = yyvsp[0].point.x;
		  stat->vertex[stat->vertex_index].y = yyvsp[0].point.y;
		  stat->vertex[stat->vertex_index].pos = 1; /* 座標を使用 */
		  stat->vertex_index++;
		;
    break;}
case 67:
#line 387 "2d_layout.y"
{ yyval.point.x = yyvsp[-3].f_num; yyval.point.y = yyvsp[-1].f_num; ;
    break;}
case 69:
#line 397 "2d_layout.y"
{
		  /* モードのワークに頂点番号を設定 */
		  spr_layout->blockTarget->d.st.end->vertex_index = yyvsp[0].i_num;
		;
    break;}
case 70:
#line 404 "2d_layout.y"
{
		  sprStatus * stat = spr_layout->blockTarget->d.st.end;

		  DBG("setup rgba %d\n", stat->vertex_index);

		  if(spr_layout->blockTarget->d.st.target->rgba_num
		     <= stat->vertex_index)
		    {
		      /* error */

		    }

		  /* 座標値を、現在の頂点に設定し、頂点番号を更新 */
		  stat->vertex[stat->vertex_index].r = yyvsp[0].rgba.r;
		  stat->vertex[stat->vertex_index].g = yyvsp[0].rgba.g;
		  stat->vertex[stat->vertex_index].b = yyvsp[0].rgba.b;
		  stat->vertex[stat->vertex_index].a = yyvsp[0].rgba.a;
		  stat->vertex[stat->vertex_index].col = 1; /* RGBAを使用 */
		  stat->vertex_index++;
		;
    break;}
case 71:
#line 425 "2d_layout.y"
{
		  sprStatus * stat = spr_layout->blockTarget->d.st.end;

		  DBG("setup rgba %d\n", stat->vertex_index);
		  if(spr_layout->blockTarget->d.st.target->rgba_num
		     <= stat->vertex_index)
		    {
		      /* error */

		    }

		  /* 座標値を、現在の頂点に設定し、頂点番号を更新 */
		  stat->vertex[stat->vertex_index].r = yyvsp[0].rgba.r;
		  stat->vertex[stat->vertex_index].g = yyvsp[0].rgba.g;
		  stat->vertex[stat->vertex_index].b = yyvsp[0].rgba.b;
		  stat->vertex[stat->vertex_index].a = yyvsp[0].rgba.a;
		  stat->vertex[stat->vertex_index].col = 1; /* RGBAを使用 */
		  stat->vertex_index++;
		;
    break;}
case 72:
#line 447 "2d_layout.y"
{
		  yyval.rgba.r = yyvsp[-7].i_num;
		  yyval.rgba.g = yyvsp[-5].i_num;
		  yyval.rgba.b = yyvsp[-3].i_num;
		  yyval.rgba.a = yyvsp[-1].i_num;
		;
    break;}
case 73:
#line 456 "2d_layout.y"
{
		  sprStatus * stat = spr_layout->blockTarget->d.st.end;
		  stat->alpha = (yyvsp[0].i_num << 6) | (yyvsp[-1].i_num << 4) | (yyvsp[-2].i_num << 2) | yyvsp[-3].i_num;
		  if(yyvsp[-1].i_num == 2)
		    {
		      /* error */
		    }
		  DBG("alpha %d %d %d %d\n", yyvsp[-3].i_num, yyvsp[-2].i_num, yyvsp[-1].i_num, yyvsp[0].i_num);
		;
    break;}
case 74:
#line 466 "2d_layout.y"
{
		  sprStatus * stat = spr_layout->blockTarget->d.st.end;
		  if(yyvsp[-2].i_num != 2)
		    {
		      /* error */
		    }
		  stat->alpha = (yyvsp[-1].i_num << 6) | (yyvsp[-2].i_num << 4) | (yyvsp[-3].i_num << 2) | yyvsp[-4].i_num;
		  stat->v_alpha = yyvsp[0].i_num;
		  DBG("alpha %d %d %d %d %d\n", yyvsp[-4].i_num, yyvsp[-3].i_num, yyvsp[-2].i_num, yyvsp[-1].i_num, yyvsp[0].i_num);
		;
    break;}
case 76:
#line 485 "2d_layout.y"
{
		  DBG("action [%s]\n", yyvsp[-1].str);
		  if(NULL == action_2d_new(yyvsp[-1].str)) l2derror();
		;
    break;}
case 77:
#line 492 "2d_layout.y"
{
		  DBG("actend\n");
		  spr_layout->actTarget = NULL;
		;
    break;}
case 82:
#line 508 "2d_layout.y"
{
		  if(NULL == track_2d_new(yyvsp[-1].str))
		    l2derror();
		;
    break;}
case 83:
#line 515 "2d_layout.y"
{
		  DBG("track end\n");
		  track_2d_gencode(TR_END, 0, NULL, 0);
		;
    break;}
case 90:
#line 532 "2d_layout.y"
{
		  DBG("set [%s]\n", yyvsp[0].str);
		  track_2d_gencode(TR_SET, 0, yyvsp[0].str, 0);
		;
    break;}
case 91:
#line 539 "2d_layout.y"
{
		  DBG("wait [%d]\n", yyvsp[0].i_num);
		  track_2d_gencode(TR_WAIT, yyvsp[0].i_num, NULL, 0);
		;
    break;}
case 92:
#line 546 "2d_layout.y"
{
		  DBG("morf [%d] - [%s](%d)\n", yyvsp[-2].i_num, yyvsp[-1].str, yyvsp[0].i_num);
		  track_2d_gencode(TR_MORF, yyvsp[-2].i_num, yyvsp[-1].str, yyvsp[0].i_num);
		;
    break;}
case 93:
#line 552 "2d_layout.y"
{ yyval.i_num = 0; ;
    break;}
case 94:
#line 553 "2d_layout.y"
{ yyval.i_num = yyvsp[0].i_num; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 555 "2d_layout.y"

#include "lex.yy.c"

int yyerror(char * s)
{
  fprintf(stderr, "Error(%d): %s\n", num_line + 1, s);
  exit(EXIT_FAILURE);
}
