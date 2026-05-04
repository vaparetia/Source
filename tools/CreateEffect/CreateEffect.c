/**********************************************************************
 *<
	FILE: CreateEffect.cpp

	DESCRIPTION: Create effect for Demo.

	CREATED BY: T. Morita

	HISTORY: 

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ParseAll.h"
#include "ParseGrm.h"
#include "CreateEffect.h"


#define  PERROR(_s)  return(fprintf(stderr,(_s)),1)


/*	-------------------------

	型の生成

  	-------------------------*/
int print_pointer_mark( FILE *fp, int i )
{
	if ( i >= 0 )
		while( --i>=0 )
			fprintf( fp, "*" ) ;
	else
		while( ++i<=0 )
			fprintf( fp, "&" ) ;
	return 0 ;
}
int print_pointer( FILE *fp, NODE* type, int decl )
{
	return print_pointer_mark( fp, type->pointer * decl ) ;
}

int print_type( FILE *fp, NODE* type, int lvl )
{
	static struct type_t
	{
		char *literal ;
		int   type_code ;
	} type_str[] = {
		{ "char"   , TOKEN_CHAR        },{ "int"    , TOKEN_INT         },{ "long"   , TOKEN_LONG        },
		{ "short"  , TOKEN_SHORT       },{ "u_char" , TOKEN_CHAR |0x8000},{ "u_int"  , TOKEN_INT  |0x8000},
		{ "u_long" , TOKEN_LONG |0x8000},{ "u_short", TOKEN_SHORT|0x8000},{ "float"  , TOKEN_FLOAT       },
		{ "void"   , TOKEN_VOID        },
		{ "FVECTOR", TOKEN_FVECTOR     },{ "SVECTOR", TOKEN_SVECTOR     },{ "IVECTOR", TOKEN_IVECTOR     },
		{ "FMATRIX", TOKEN_FMATRIX     },
	} ;
	int i, j ;

	for ( i=sizeof(type_str)/sizeof(struct type_t) ; --i>=0 ; )
		if ( type_str[i].type_code == (type->type & ~0x4000) )
		{
			for ( j=lvl ; --j>=0 ; )
				fprintf( fp, "\t" ) ;
			fprintf( fp,(lvl ? "%-7s " : "%s "), type_str[i].literal ) ;
			return 0 ;
		}
	return 1 ;
}




/*	-------------------------

	インクルードの生成

  	-------------------------*/
int create_effect_include( FILE *fp )
{
	fprintf( fp, "#include <stdio.h>\n" ) ;
	fprintf( fp, "#include <stdlib.h>\n" ) ;
	fprintf( fp, "#ifdef PSX2\n" ) ;
	fprintf( fp, "#include <libgraph.h>\n" ) ;
	fprintf( fp, "#include <libdma.h>\n" ) ;
	fprintf( fp, "#include <libvu0.h>\n" ) ;
	fprintf( fp, "#endif\n" ) ;
	fprintf( fp, "\n" ) ;

	fprintf( fp, "#include \"gameheader.h\"\n" ) ;
	fprintf( fp, "#include \"libutl.h\"\n" ) ;
	fprintf( fp, "#include \"../libdemo.h\"\n" ) ;

	fprintf( fp, "\n\n\n" ) ;

	return 0 ; 
}


/*	-------------------------

	構造体の生成

  	-------------------------*/
int create_effect_struct( FILE *fp, EFFECT *ef )
{
	LIST *ls ;
	int   var ;
	NODE *type_node ;
	char *fmt ;

	/*ワーク構造体を生成*/
	if ( ef->member )
	{
		fprintf( fp, "typedef struct work_t {\n" ) ;
		ls = ef->member->prev ;
		do
		{
			type_node = (NODE*)ls->data ;
			print_type( fp, type_node, 1 ) ;
			print_pointer( fp, type_node, 1 ) ;
			fprintf( fp, "%s ;\n", type_node->value.name ) ;
		}
		while( (ls = ls->prev)->next ) ;
		fprintf( fp, "} Work ;\n\n" ) ;
	}

	/*引数構造体を生成*/
	if ( (func_ls = ef->func_ls) )
	{
		fprintf( fp, "typedef struct data_t {\n" ) ;
		var = 0 ;
		ls = ef->func_ls->prev ;
		do
		{
			type_node = (NODE*)ls->data ;
			if ( strstr( type_node->value.name, "MaxInput" ) == type_node->value.name )
			{
				if ( type_node->pointer ==1 && (type_node->type&0x0fff)==TOKEN_CHAR )
					fmt = "d%d[128] ;\n" ;
				else
				{
					type_node->pointer = 0 ;/*参照の状況で０でないときがあるので強制的に戻す*/
					fmt = "d%d ;\n" ;
				}
				print_type( fp, type_node, 1 ) ;
				fprintf( fp, fmt, var ) ;
			}
			var++ ;
		}
		while( (ls = ls->prev)->next ) ;
		fprintf( fp, "} Data ;\n\n" ) ;
	}
	return 0 ;
}








int create_effect_expr( FILE *fp, NODE *expr, char *name ) ;


/*	-------------------------

	関数の生成

  	-------------------------*/
int create_effect_func( FILE *fp, NODE *func, char *name )
{
	LIST *ls ;
	int   var = 0 ;

	/*先頭に"MaxInput"の文字があればMaxからのデータ入力関数*/
	if ( strstr( func->value.name, "MaxInput" ) == func->value.name )
	{
		if ( func_ls )
		{
			ls = func_ls->prev ;
			do
				if ( ls->data == func )
					break ;
				else
					var++ ;
			while( (ls = ls->prev)->next ) ;
		}
		fprintf( fp, "data->d%d", var ) ;
	}

	/*データ処理関数*/
	else
	{
		if ( strstr( func->value.name, "sceVu0" ) == func->value.name )
			fprintf( fp, "_%s( ", func->value.name ) ;
		else
			fprintf( fp, "%s( " , func->value.name ) ;
		if ( (ls = func->data) )
		{
			ls = ls->prev ; 
			do
			{
				create_effect_expr( fp, ls->data, name ) ;
				if ( ls->prev->next )
					fprintf( fp, ", " ) ;
			}
			while( (ls = ls->prev)->next ) ;
		}
		fprintf( fp, " )" ) ;
	}
	return 0 ;
}

/*	-------------------------

	式の生成

  	-------------------------*/
int create_effect_expr( FILE *fp, NODE *expr, char *name )
{
	NODE *type ;

	if ( expr->flag & IS_Expression && !(expr->flag & IS_Constant) )
		fprintf( fp, "(" ) ;

	if ( expr->flag & IS_Function )
	{
		if ( assign )
			print_pointer_mark( fp, expr->pointer - assign->pointer ) ;
		create_effect_func( fp, expr, name ) ;
	}
	else if ( expr->flag & IS_Variable )
	{
		if ( !(expr->flag & IS_PointRef) )
		{
			if ( !(expr->flag & IS_Assigned) && assign )
				if ( (type = list_search_node( decl_list, expr, NULL )) )
					print_pointer_mark( fp, type->pointer - assign->pointer ) ;
			if (  expr->flag & IS_Arguments )
				if ( (type = list_search_node( decl_list, expr, NULL )) )
					print_pointer_mark( fp, type->pointer - expr->pointer ) ;
		}
		if ( expr->flag & IS_Assigned )
			assign = expr ;
		fprintf( fp, "%s", expr->value.name ) ;
	}
	else if ( expr->flag & IS_StackVar )
	{
		if ( !(expr->flag & IS_PointRef) )
		{
			if ( assign && !(expr->flag & IS_Assigned) )
				print_pointer_mark( fp, expr->pointer - assign->pointer ) ;
			if ( member &&   expr->flag & IS_Arguments )
				print_pointer_mark( fp, member->pointer - expr->pointer ) ;
		}
		if ( expr->flag & IS_Assigned )
			assign = member ;
		fprintf( fp, "work.%s", name ) ;
	}
	if ( expr->flag & IS_Assigned )
		fprintf( fp, " = " ) ;

	if ( expr->left  )
		create_effect_expr( fp, expr->left , expr->value.name ) ;

	if ( expr->flag & IS_Literal )
		fprintf( fp, " \"%s\" ", expr->value.literal ) ;
	else if ( expr->flag & IS_Constant )
	{
		if ( assign )
		{
			fprintf( fp, "( " ) ;
			print_type( fp, assign, 0 ) ;
			fprintf( fp, ") " ) ;
		}
		fprintf( fp, "%f ", expr->value.constant ) ;
	}
	else if ( expr->flag & IS_Expression )
		fprintf( fp, " %c ", expr->value.operand  ) ;

	if ( expr->right )
		create_effect_expr( fp, expr->right, expr->value.name ) ;

	if ( expr->flag & IS_PointTo )
		fprintf( fp, (expr->right->pointer ? "->%s":".%s"), expr->value.name ) ;

	if ( expr->flag & IS_Expression && !(expr->flag & IS_Constant) )
		fprintf( fp, ")" ) ;
	return 0 ;
}

int create_effect_exprs( FILE *fp, NODE *expr )
{
	LIST *ls ;

	member = expr ;
	if ( expr->data )
	{
		ls = expr->data->prev ;
		do
		{
			assign = NULL ;
			fprintf( fp, "\t" ) ;
			create_effect_expr( fp, ls->data, expr->value.name ) ;
			fprintf( fp, " ;\n" ) ;
		}
		while( (ls = ls->prev)->next ) ;
	}
	return 0 ;
}



/*	-------------------------

	宣言の生成

  	-------------------------*/
int regist_decl( FILE *fp, NODE *expr, int decl )
{
	if ( !list_search_node( decl_list, expr, NULL ) )
	{
		decl_list = list_regist( decl_list, expr, sizeof(NODE) ) ;

		print_type( fp, expr, 1 ) ;
		print_pointer( fp, expr, decl ) ;
		if ( !decl )
			((NODE*)decl_list->data)->pointer = 0 ;
		fprintf( fp, " %s ;\n", expr->value.name ) ;
		return 1 ;
	}
	return 0 ;
}

int create_declare( FILE *fp, NODE *expr )
{
	LIST *ls ;

	if ( expr->flag & IS_Assigned && expr->flag & IS_Variable)
		regist_decl( fp, expr, 1 ) ;
	else if ( expr->flag & IS_Function )
		if ( strstr( expr->value.name, "MaxInput" ) != expr->value.name )
			if ( (ls = expr->data) )
			{
				ls = ls->prev ; 
				do
					if ( ((NODE*)ls->data)->flag & IS_Variable )
						regist_decl( fp, ls->data, 0 ) ;
				while( (ls = ls->prev)->next ) ;
			}

	if ( expr->left  )
		create_declare( fp, expr->left  ) ;
	if ( expr->right )
		create_declare( fp, expr->right ) ;
	return 0 ;
}

int create_declares( FILE *fp, NODE *expr )
{
	LIST *ls ;

	member = expr ;
	if ( expr->data )
	{
		ls = expr->data->prev ;
		do
			create_declare( fp, ls->data ) ;
		while( (ls = ls->prev)->next ) ;
	}
	return 0 ;
}




/*	-------------------------

	ランチャーの生成

  	-------------------------*/
int create_effect_lancher( FILE *fp, EFFECT *ef, char *name )
{
	LIST *ls ;
	int len = 0 ;

	/*ワーク、データを生成*/
	fprintf( fp, "void *%sLaunch( int id, void *argv )\n{\n", name ) ;
	if ( ef->member )
		fprintf( fp, "\tWork  work ;\n" ) ;
	if ( ef->func_ls )
		fprintf( fp, "\tData *data = (Data *)argv ;\n" ) ;
	fprintf( fp, "\n" ) ;

	/*関数宣言*/
	fprintf( fp, "\textern void *%s( ", ef->func_nm ) ;
	if ( ef->member )
	{
		ls = ef->member->prev ;
		do
		{
			print_type( fp, (NODE*)ls->data, 0 ) ;
			print_pointer( fp, (NODE*)ls->data, 1 ) ;
			fprintf( fp, (ls->prev->next ? "," : " ") ) ;
		}
		while( (ls = ls->prev)->next ) ;
	}
	fprintf( fp, ") ;\n\n" ) ;


	if ( decl_list )
		list_delete( decl_list ), decl_list = NULL ;

	/*宣言を生成*/
	if ( ef->member )
	{
		ls = ef->member->prev ;
		do
			create_declares( fp, (NODE *)ls->data ) ;
		while( (ls = ls->prev)->next ) ;
	}
	fprintf( fp, "\n" ) ;

	/*式を生成*/
	if ( ef->member )
	{
		ls = ef->member->prev ;
		do
			create_effect_exprs( fp, (NODE *)ls->data ) ;
		while( (ls = ls->prev)->next ) ;
	}
	fprintf( fp, "\n" ) ;

	/*呼び出しを生成*/
	fprintf( fp, "\treturn %s( ", ef->func_nm ) ;
	if ( ef->member )
	{
		ls = ef->member->prev ;
		do
		{
			fprintf( fp, (len ? "\t%*s%s%s\n" : "%*s%s%s\n"),
							len, "work.",
							((NODE*)ls->data)->value.name,
							(ls->prev->next ? "," : "") ) ;
			len = strlen( ef->func_nm ) + 14 ;
		}
		while( (ls = ls->prev)->next ) ;
	}
	fprintf( fp, ") ;\n" ) ;
	fprintf( fp, "}\n\n\n" ) ;

	return 0 ;
}


/*	-------------------------

	エフェクト毎のコード生成

  	-------------------------*/
int create_effects()
{
	FILE   *fp ;
	EFFECT *ef ;
	LIST   *ls ;
	char   *path_end ;
	NODE   *id ;
	char    name[128] ;

	fprintf( stderr, "Enter Launcher Creation\n" ) ;
	if ( strlen(dest_path) )
#ifdef _LINUX
		if ( dest_path[ strlen(dest_path)-1 ] != '/' )
			strcat( dest_path, "/" ) ;
#else
		if ( dest_path[ strlen(dest_path)-1 ] != '\\' )
			strcat( dest_path, "\\" ) ;
#endif

	path_end = &dest_path[ strlen(dest_path) ] ;
	for( ls = parse_work.effect ; ls ; ls = ls->next )
	{
		ef = (EFFECT *)ls->data ;
		if ( !(id = list_search_option( ef->option, "id" )) )
		{
			fprintf( stderr, "エフェクト(%s)にidのオプションがありません<id:num>\n", ef->name ) ;
			continue ;
		}

		*path_end = '\0' ;

		sprintf( name, "%04x_%s", (int)id->value.constant, ef->func_nm ) ;
		strcat( dest_path, name ) ;
		strcat( dest_path, ".c" ) ;

		sprintf( name, "%s_%04x", ef->func_nm, (int)id->value.constant ) ;

		if ( (fp = fopen( dest_path, "w" )) )
		{
			fprintf( stderr, "  %s ", ef->func_nm ) ;

			/*コメント生成*/
			fprintf( fp, "/*********************************************\n\n" ) ;
			fprintf( fp, "\tエフェクト　%s　のランチャー関数\n\n", ef->name   ) ;
			fprintf( fp, "\tCreated by DEMO Composer.\n" ) ;
			fprintf( fp, " *********************************************/\n"  ) ;
			fprintf( fp, "\n\n" ) ;

			create_effect_include( fp ) ;
			create_effect_struct( fp, ef ) ;
			create_effect_lancher( fp, ef, name ) ;

			/*コメント生成*/
			fprintf( fp, "/*********************************************\n\n" ) ;
			fprintf( fp, "\tエフェクト　%s　のランチャー関数\n\n", ef->name   ) ;
			fprintf( fp, "\tCreated by DEMO Composer.\n" ) ;
			fprintf( fp, " *********************************************/\n"  ) ;

			fprintf( stderr, "done.\n" ) ;

			fclose( fp ) ;
		}
		else
			fprintf( stderr, "Cannot open %s !!\n", dest_path ) ;
	}
	fprintf( stderr, "Done.\n" ) ;

	return 0 ;
}







/*	-------------------------

	メイン関数

  	-------------------------*/
int main( int argc, char *argv[] )
{
	int flag = 1 ;

	if ( argc > 1 )
	{
		flag = 0 ;
		file_path[0] = '\0' ;
		dest_path[0] = '\0' ;
		for ( argv++ ; --argc> 0 ; argv++ )
		{
			if ( argv[0][0] == '-' )
				switch( argv[0][1] )
				{
				case 'h':
					flag = 1 ;
					break ;
				default:
					fprintf( stderr, "Illeagal option(-%c) found.\n", argv[0][1] ) ;
					return 1 ;
				}
			else if ( !strlen( file_path ) && strstr( *argv, ".cnf" ) )
				strcpy( file_path, *argv ) ;
			else if ( !strlen( dest_path ) )
				strcpy( dest_path, *argv ) ;
		}
	}
	if ( flag & 1 )
		fprintf( stderr, "ef_create [-h] conf-file [directory]\nDesc: Create source files for Demo Effects.\n" ) ;
	else if ( yyparse() )
		fprintf( stderr, "%s\nParse ERROR!!\n", debug_buff ) ;
	else if ( create_effects() )
		fprintf( stderr, "Effect Creation ERROR!!\n" ) ;
	else
		return 0 ;
	return 1 ;
}
