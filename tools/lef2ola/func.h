/*
	func.h

   2002/12/11 Nobumitsu Tanaka
   $Id: func.h,v 1.2 2002/12/11 06:28:37 usr03635 Exp $
*/

#ifndef __FUNC_H__
#define __FUNC_H__

#include "data.h"
#include "LESceneStruct20.h"

#ifdef DEBUG

typedef struct tagAllocTable{
	int  nID;
	int  nSize;
	long lPointer;
} AllocTable;

extern int nMemerySize;
void* Malloc(size_t size);
void  Free(void* p);

#else

#define Malloc(a) (malloc(a))
#define Free(a) (free(a))

#endif


extern int Analyze(int argc, char *argv[], ConvertInfo *lpInfo);
extern int ReadLefPicture(FILE *fp, SceneData *lpSceneData, LEFLESCENEHEADER *lpLEFSceneHeader);
extern int ReadLefAction(FILE *fp, SceneData *lpSceneData, LEFLESCENEHEADER *lpLEFSceneHeader, IdDataTable *lpActorTable);
extern Picture* SearchPicture(SceneData *lpSceneData, char *lpszTexName);
extern int ReadLefObject(FILE *fp, SceneData *lpSceneData, LEFLESCENEHEADER *lpLEFSceneHeader, IdDataTable *lpActorTable, AnimeObjectHeader *lpAnmObjHeader);
extern int ReadLefAnime(FILE *fp, SceneData *lpSceneData, Object *lpObject, ObjectDummy *lpObjectDummy, int nAnimeObjectCount);
extern int SetKeyData(SceneData *lpSceneData, IdDataTable *lpTblActor, AnimeObjectHeader *lpAnmObjectHeader);
extern void DumpSceneData(ConvertInfo *lpInfo);
extern void DeleteSceneData(SceneData *lpSceneData);

#endif
