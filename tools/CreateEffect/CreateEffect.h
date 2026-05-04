/**********************************************************************
 *<
	FILE: CreateEffect.h

	DESCRIPTION: Create effect code for Demo.

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __CREATE_EFFECT__H
#define __CREATE_EFFECT__H

char dest_path[128] ;
LIST *func_ls ;
LIST *decl_list = NULL ;/*関数内の宣言変数*/
LIST *data_list = NULL ;/*関数内の宣言変数*/
NODE *member = NULL ;/*関数内の宣言変数*/
NODE *assign = NULL ;

#endif // __CREATE_EFFECT__H
