/*
	XSI parser lex

	XSIの文法用の字句解析

	T.Morita  Feb 21 2000
	$Id: xsi.l,v 1.6 2000/09/11 11:58:19 usr04098 Exp $
*/
D                       [0-9]
L                       [a-zA-Z_-]
H                       [a-fA-F0-9]
E                       [Ee][+-]?{D}+

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "alltypes.h"
#include "allvars.h"

#include "y.tab.h"

#define SET_TYP(_x) (yylval.typ_yy_t = (_x) )
void count() ;

%}

%%

WORD  			{ count(); return WORD   ; }
DWORD 			{ count(); return DWORD  ; }
FLOAT 			{ count(); return FLOAT  ; }
DOUBLE			{ count(); return DOUBLE ; }
CHAR  			{ count(); return CHAR   ; }
UCHAR 			{ count(); return UCHAR  ; }
BYTE  			{ count(); return BYTE   ; }

array	 		{ count(); return ARRAY                  ; }

Animation 		{ count(); return SET_TYP( ANIMATION              ); }
AnimationSet 		{ count(); return SET_TYP( ANIMATIONSET           ); }
Frame	 		{ count(); return SET_TYP( FRAME                  ); }
FrameTransformMatrix 	{ count(); return SET_TYP( FRAMETRANSFORMMATRIX   ); }
Mesh 			{ count(); return SET_TYP( MESH                   ); }
MeshMaterialList 	{ count(); return SET_TYP( MESHMATERIALLIST       ); }
TextureFilename 	{ count(); return SET_TYP( TEXTUREFILENAME        ); }

SI_Ambience 		{ count(); return SET_TYP( SI_AMBIENCE            ); }
SI_Angle 		{ count(); return SET_TYP( SI_ANGLE               ); }
SI_AnimationKey 	{ count(); return SET_TYP( SI_ANIMATIONKEY        ); }
SI_AnimationParamKey 	{ count(); return SET_TYP( SI_ANIMATIONPARAMKEY   ); }
SI_Camera 		{ count(); return SET_TYP( SI_CAMERA              ); }
SI_CameraAnimation	{ count(); return SET_TYP( SI_CAMERAANIMATION     ); }
SI_CoordinateSystem 	{ count(); return SET_TYP( SI_COORDINATESYSTEM    ); }
SI_Envelope 		{ count(); return SET_TYP( SI_ENVELOPE            ); }
SI_EnvelopeList 	{ count(); return SET_TYP( SI_ENVELOPELIST        ); }
SI_Fog	 		{ count(); return SET_TYP( SI_FOG                 ); }
SI_FrameBasePoseMatrix	{ count(); return SET_TYP( SI_FRAMEBASEPOSEMATRIX ); }
SI_Light 		{ count(); return SET_TYP( SI_LIGHT               ); }
SI_Material 		{ count(); return SET_TYP( SI_MATERIAL            ); }
SI_MeshFace 		{ count(); return SET_TYP( SI_MESHFACE            ); }
SI_MeshNormals 		{ count(); return SET_TYP( SI_MESHNORMALS         ); }
SI_MeshTextureCoords 	{ count(); return SET_TYP( SI_MESHTEXTURECOORDS   ); }
SI_MeshVertexColors 	{ count(); return SET_TYP( SI_MESHVERTEXCOLORS    ); }
SI_PatchSurface 	{ count(); return SET_TYP( SI_PATCHSURFACE        ); }
SI_SpotLightParam 	{ count(); return SET_TYP( SI_SPOTLIGHTPARAM      ); }
SI_VertexWeight 	{ count(); return SET_TYP( SI_VERTEXWEIGHT        ); }
SI_Scene	 	{ count(); return SET_TYP( SI_SCENE               ); }
SI_FCurve	 	{ count(); return SET_TYP( SI_FCURVE              ); }

"xsi "{D}+("txt "|"bin "|"com ")("lzw "|"zip ")?{D}+	{ count(); return HEADER ; }

{L}({L}|{D})*           {
			    count();
			    strncpy( yylval.str_yy_t, yytext, yyleng ) ;
			    yylval.str_yy_t[yyleng]='\0' ;
			    return IDENTIFIER     ;
			}
{D}+                    { count(); yylval.var_yy_t.f = atof(yytext) ; return CONSTANT       ; }
'(\\.|[^\\'])+'         { count(); yylval.var_yy_t.f = atof(yytext) ; return CONSTANT       ; }
[+-]?{D}*"."{D}+({E})?  { count(); yylval.var_yy_t.f = atof(yytext) ; return CONSTANT       ; }
\"(\\.|[^\\"])*\"       { count(); yylval.var_yy_t.s = yytext       ; return STRING_LITERAL ; }

","                     { count(); return ',' ; }
";"                     { count(); return ';' ; }
"{"                     { count(); return '{' ; }
"}"                     { count(); return '}' ; }
"["                     { count(); return '[' ; }
"]"                     { count(); return ']' ; }

[ \t\v\f]               { count(); }
\n                      { count(); lines++ ; }
.                       { /* ignore bad characters */ }

%%


int yywrap()
{
    return 1 ;
}


void comment()
{
    char c, c1 ;

loop:
    while ( (c=input())!='*' && c!=0 )
        putchar( c ) ;

    if ((c1 = input())!='/' && c!=0 )
    {
        unput( c1 ) ;
        goto loop ;
    }

    if ( c != 0 )
        putchar(c1) ;
}


int column = 0 ;

void count()
{
    int i ;

    for ( i=0 ; yytext[i] != '\0' ; i++)
        if (yytext[i] == '\n')
            column = 0 ;
        else if (yytext[i] == '\t')
            column += 8 - (column % 8) ;
        else
            column++ ;
    //ECHO ;
}
