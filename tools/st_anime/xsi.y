/*
	XSI parser yacc

	XSIの文法はBNF記法で書けないため、正常に動作させるためにはすべ
	てのテンプレートに対して各文法を用意する必要がある。ゆえに以下
	のような信じ難いほど汚い文法になる。

	T.Morita  Feb 21 2000
	$Id: xsi.y,v 1.7 2001/11/30 04:12:21 usr04098 Exp $
*/
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alltypes.h"
#include "allvars.h"

int yyerror( char *s ) ;
int yylex( void ) ;
%}

%union
{
    float    num_yy_t ;
    char     str_yy_t[128] ;
    int      typ_yy_t ;
    VAR      var_yy_t ;
    short   *vec_yy_t ;
}

/* key words */
%token HEADER
%token<str_yy_t> IDENTIFIER 
%token<var_yy_t> CONSTANT STRING_LITERAL
%token<typ_yy_t> WORD DWORD FLOAT DOUBLE CHAR UCHAR BYTE  

%token<typ_yy_t> ARRAY

%token<typ_yy_t> ANIMATIONSET ANIMATION FRAME FRAMETRANSFORMMATRIX
%token<typ_yy_t> MESH MESHMATERIALLIST TEXTUREFILENAME

%token<typ_yy_t> SI_AMBIENCE SI_ANGLE SI_ANIMATIONKEY SI_ANIMATIONPARAMKEY
%token<typ_yy_t> SI_CAMERA SI_COORDINATESYSTEM SI_ENVELOPE SI_ENVELOPELIST SI_FOG
%token<typ_yy_t> SI_FRAMEBASEPOSEMATRIX SI_LIGHT SI_MATERIAL SI_MESHFACE
%token<typ_yy_t> SI_MESHNORMALS SI_PATCHSURFACE SI_MESHTEXTURECOORDS
%token<typ_yy_t> SI_MESHVERTEXCOLORS SI_SPOTLIGHTPARAM SI_VERTEXWEIGHT
%token<typ_yy_t> SI_SCENE SI_CAMERAANIMATION SI_FCURVE

/* expressions */
%type<str_yy_t> identifier 

%type<var_yy_t> expr member 
%type<typ_yy_t> anim_paramkey_member anim_paramkey_array mesh_array mesh_array_member

%type<typ_yy_t> template templates template_name ordinal_template specific_template

%start file

%%

/* 汎用 メンバ構成文法 */
expr
        : CONSTANT
        | STRING_LITERAL
        ;
member
        : expr ';'
        ;
list
        : expr ';'
          {
	      add_to_varlist( &($1) ) ;
	  }
	| list expr ';'
          {
	      add_to_varlist( &($2) ) ;
	  }
        ;
list_member
	: list ';'
        ;
array
        : expr
          {
	      add_to_varlist( &($1) ) ;
	  }
        | array ',' expr
          {
	      add_to_varlist( &($3) ) ;
	  }
        ;
array_member
        : array ';'
        ;
list_array
        : list
        | list_array ',' list
        ;
list_array_member
        : list_array ';'
        ;
ordinal_list_member
	:
         {
	     init_varlist( 16 ) ;
	 }
          list ';'
         {
	     free_varlist() ;
         }
        ;


/* SI_ANIMATIONPARAMKEY用 メンバ文法 */
anim_paramkey_member
	: 
         {
	     reset_varlist() ;
         }
          member member array_member ';'
	;
anim_paramkey_array
	: anim_paramkey_member
         {
	     if ( current->para )
	     {
		 $$ = 0 ;
		 if ( !strstr( param_mode, "Camera" ) )
		 {
#if 0
		     get_fvec_sub_vert( &current->para->verts[($1)*3], current->para->org ) ;
#else
		     get_fvec_from_varlist( current->para->org ) ;
#endif
		 }
	     }
         }
	| anim_paramkey_array ',' anim_paramkey_member
         {
	     if ( current->para )
	     {
		 if ( !strstr( param_mode, "Camera" ) )
		 {
#if 0
		     if ( get_fvec_sub_vert( &current->para->verts[($1)*3], &current->para->org[$1] ) )
			 YYABORT ;
		     apply_matrix( current->matrix ) ;
#else
		     get_fvec_from_varlist( &current->para->org[$$ = $1+1] ) ;
#endif
		 }
	     }
         }
	;
anim_paramkey_array_member
	: member member member
         {
	     if ( current->para )
	     {
		 init_varlist( 3 ) ;
		 if ( !strstr( param_mode, "Camera" ) )
		 {
		     if ( n_frame && 0 )
			 if ( n_frame > (int)$3.f )
			 {
			     fprintf( stderr, "\nSI_ANIMATIONPARAMKEY number of frame unmached %d(n_frame)!=%d(para)\n", n_frame, (int)$3.f ) ;
			     yyerror( "" ) ;
			     YYABORT ;
			 }
		     n_frame = (int)$3.f ;
		     current->para->org = malloc( n_frame * sizeof(FVECTOR) ) ;
		 }
	     }
         }
          anim_paramkey_array ';'
         {
	     if ( current->para )
	     {
		 if ( !strstr( param_mode, "Camera" ) )
		 {
		     int i ;
		     FVECTOR *m ;

		     for ( i=0, m=current->mesh ; i<current->n_mesh ; i++,m++ )
			 if ( m->vw == $1.f )
			 {
			     printf( "o" ) ;
			     current->n_anime++ ;
			     current->para->vert_id = i ;
			     current->para++ ;
			     break ;
			 }
		     if ( i<0 )
		     {
			 free( current->para->org ) ;
			 printf( "." ) ;
		     }
		     fflush( stdout ) ;
		 }
		 free_varlist() ;
	     }
         }
        ;



/* MESH用 メンバ文法 */
mesh_ar_member
	:
         {
	     reset_varlist() ;
         }
          member array_member
	;
mesh_array
	: mesh_ar_member
         {
	     get_poly_from_varlist( &current->poly[$$ = 0   ] ) ;
         }
	| mesh_array ',' mesh_ar_member
         {
	     get_poly_from_varlist( &current->poly[$$ = $1+1] ) ;
         }
	;
mesh_array_member
	: mesh_array ';' 
	;
mesh_member
	: member
         {
	     current->n_mesh = (int)$1.f ;
	     init_varlist( 3*current->n_mesh ) ; /* at least 4 elements needed */
	     if ( !(current->mesh = malloc( sizeof(FVECTOR) * current->n_mesh )) )
	     {
		 fprintf( stderr, "Out of memory\n" ) ;
		 YYABORT ;
	     }
	     printf( "%s\t%d vertexes by MESH\n", current->name, current->n_mesh ) ;
         }
	  list_array_member
         {
	     int i ;
	     FVECTOR *f ;

	     for ( i=current->n_mesh, f=current->mesh ; --i>=0 ; f++ )
		 get_fvec_from_varlist( f ) ;
	     free_varlist() ;
         }
          member
         {
	     current->n_poly = (int)$5.f ;
	     init_varlist( 5 ) ;
	     if ( !(current->poly = malloc( sizeof(POLY) * current->n_poly )) )
	     {
		 fprintf( stderr, "Out of memory\n" ) ;
		 YYABORT ;
	     }
	     printf( "%s\t%d polygons by POLY\n", current->name, current->n_poly ) ;
         }
          mesh_array_member
         {
	     free_varlist() ;
         }
          templates
        ;


/* MESHNORMALS用 メンバ文法 */
meshnormals_member
	: 
         {
	     reset_varlist() ;
         }
          member member array_member
	;
meshnormals_array
	: meshnormals_member
	| meshnormals_array ',' meshnormals_member
	;
meshnormals_array_member
	: 
	  member
         {
	     init_varlist( 3*(int)$1.f ) ;
         }
          list_array_member member meshnormals_array ';'
         {
	     free_varlist() ;
         }
	;



/* FRAMETRANSFORMMATRIX用 メンバ文法 */
frametransmtx_member
        : 
         {
	     init_varlist( 16 ) ;
         }
	  array_member ';'
         {
	     int  i, j ;

	     for ( i=0 ; i<4 ; i++ )
		 for ( j=0 ; j<4 ; j++ )
		     current->matrix.m[i][j] = get_float_from_varlist() ;
	     free_varlist() ;
         }
	;



/* SI_PATCHSURFACE用 メンバ文法 */
patchsurface_member
        :
         {
	     init_varlist( 9 ) ;
         }
          list_member list_member list_member list_member
         {
	     current->n_patch = (int)get_nth_float_from_varlist( 6 ) ;
	     if ( !(current->patch = malloc( sizeof(FVECTOR) * current->n_patch )) )
		 YYABORT ;
	     printf( "%s\t%d vertexes by PATCH\n", current->name, current->n_patch ) ;

	     free_varlist() ;
	     init_varlist( 3 * current->n_patch ) ;
         }
	  list_array_member
         {
	     int i ;
	     FVECTOR *f ;

	     for ( i=current->n_patch, f=current->patch ; --i>=0 ; f++ )
		 get_fvec_from_varlist( f ) ;

	     free_varlist() ;
         }
          templates
        ;


/* MESHMATERIALLIST用 メンバ文法 */
meshmaterlst_member
	: member member
         {
	     init_varlist( (int)$2.f ) ;
         }
	  array_member
         {
	     free_varlist() ;
         }
          templates
	;


/* SI_ANIMATIONKEY用 メンバ文法 */
anim_key_array_member
	: member member
	  member member array_member ';' ','
	  member member array_member ';' ';'
	{}
	;




/* 汎用メンバ文法 */
identifier
        : /* null term */
         {
	     $$[0] = '\0' ;
         }
        | IDENTIFIER
         {
	     if ( $1 )
		 if ( !(current = search_all_list( $1 )) )
		     current = newnode_to_list( $1 ) ;
         }
        ;

template_member
        : '{' IDENTIFIER '}'
        | template
          {}
        | expr ';'
          {}
        | expr ','
          {}
        ;

template_members
        : template_member
        | template_members template_member
        ;

ordinal_template
        : template_name identifier
          '{'
         {
	     int (*action)( char *name ) = pre_actions[$1-ANIMATIONSET] ;

	     if ( action )
		 if ( (*action)( $2 ) )
		     YYABORT ;
	 }
                template_members
         {
	     int (*action)( char *name ) = post_actions[$1-ANIMATIONSET] ;

	     if ( action )
		 if ( (*action)( $2 ) )
		     YYABORT ;
	 }
          '}'
        ;

template_name
	: ANIMATION
	| ANIMATIONSET
	| FRAME
	| TEXTUREFILENAME
	| SI_ANGLE
	| SI_COORDINATESYSTEM
	| SI_FOG
	| SI_ENVELOPE 
	| SI_ENVELOPELIST
	| SI_FRAMEBASEPOSEMATRIX
	| SI_MESHFACE
	| SI_MESHTEXTURECOORDS
	| SI_MESHVERTEXCOLORS
	| SI_SPOTLIGHTPARAM
	| SI_VERTEXWEIGHT
	| SI_CAMERAANIMATION
	| SI_SCENE
	| SI_FCURVE
        ;

specific_template
	: SI_AMBIENCE identifier
	 '{'
		ordinal_list_member
	 '}'
	| SI_LIGHT identifier
	 '{'
		member ordinal_list_member ordinal_list_member
	 '}'
	| MESH identifier
	 '{'
                mesh_member
	 '}'
	| MESHMATERIALLIST identifier
	 '{'
                meshmaterlst_member
	 '}'
	| FRAMETRANSFORMMATRIX identifier
	 '{'
		frametransmtx_member
	 '}'
	| SI_CAMERA identifier
	 '{'
		ordinal_list_member ordinal_list_member template_members
	 '}'
        | SI_PATCHSURFACE identifier
	 '{'
                patchsurface_member
	 '}'
	| SI_MATERIAL identifier
	 '{'
		ordinal_list_member member
                ordinal_list_member ordinal_list_member
		member ordinal_list_member
	 '}' 
	| SI_MESHNORMALS identifier
	 '{'
	        meshnormals_array_member
	 '}' 
	| SI_ANIMATIONPARAMKEY identifier
	 '{'
		'{' IDENTIFIER '}'
	{
		param_mode = $5 ;
	}
		anim_paramkey_array_member
	 '}'
	| SI_ANIMATIONKEY
	 '{'
		anim_key_array_member
	 '}'
        ;

template
        : ordinal_template
        | specific_template
        ;

templates
        : template
        | templates template
        ;

file
        : HEADER templates
         {
	     printf( "\nSuccessfully parsed.\n" ) ;
         }
        ;

%%

extern char yytext[];
extern int column;

int yyerror( char *s )
{
    fprintf( stderr, "\n%*s\n%*s in line %d\n", column, "^", column, s, lines ) ;
    return 0 ;
}
