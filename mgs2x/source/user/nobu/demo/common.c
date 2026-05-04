//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	common.c
	    汎用関数群

	2000/01/24 N.Tanaka
	$Id: common.c,v 1.1.1.3 2002/11/19 11:46:41 Yoshizawa1 Exp $
*/

/******************************************************************************
 * included
 */
#include "common.h"

/******************************************************************************
 * publics
 */

/******************************************************************************
 * チェーンの管理
 * チェーン構造体は管理用と接続用の2つを用意して下さい。
 * 構造体のメンバの先頭には前方検索用と後方検索用の2つの void* 定数メンバを用意
 * して下さい。
 */

/******************************************************************************
 * チェーン構造体の初期化
 */
void InitChain(
	void* lpAddress)	/* 構造体のポインタ */
{
	if(lpAddress == NULL)
		return;

	*(void**)lpAddress = lpAddress;
	*(void**)((char*)lpAddress + sizeof(void*)) = lpAddress;
}

/******************************************************************************
 * 構造体をチェーンの後方に接続
 */
void NextChain(
	void* lpManeger,	/* 管理用構造体のポインタ */
	void* lpChain)		/* 接続する構造体のポインタ */
{
	if((lpManeger == NULL) || (lpChain == NULL))
		return;

	*(void**)lpChain = *(void**)lpManeger;
	*(void**)((char*)lpChain + sizeof(void*)) = lpManeger;

	if(*(void**)lpManeger == lpManeger)
		*(void**)((char*)lpManeger + sizeof(void*)) = lpChain;
	else
		*(void**)((char*)(*(void**)lpManeger) + sizeof(void*)) = lpChain;

	*(void**)lpManeger = lpChain;
}

/******************************************************************************
 * 構造体をチェーンから削除
 */
void DeleteChain(
	void* lpManeger,	/* 管理用構造体のポインタ */
	void* lpDelete)		/* 削除する構造体のポインタ */
{
	if((lpManeger == NULL) || (lpDelete == NULL))
		return;

	if(*(void**)lpDelete == lpManeger)
		*(void**)((char*)lpManeger + sizeof(void*))
			= *(void**)((char*)lpDelete + sizeof(void*));
	else
		*(void**)((char*)(*(void**)lpDelete) + sizeof(void*))
			= *(void**)((char*)lpDelete + sizeof(void*));

	if(*(void**)((char*)lpDelete + sizeof(void*)) == lpManeger)
		*(void**)lpManeger = *(void**)lpDelete;
	else
		*(void**)(*(void**)
			((char*)lpDelete + sizeof(void*))) = *(void**)lpDelete;

	InitChain(lpDelete);
}
