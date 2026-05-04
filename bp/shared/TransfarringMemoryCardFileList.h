//----------------------------------------------------------------------------
// TransfarringMemoryCardFileList.h
//
// A list of files that can manipulate the save game system
//----------------------------------------------------------------------------

#pragma once
#include "TransfarringFileList.h"
#include "TransfarringID.h"

#if MGS_VERSION == 2
#define MEMORY_CARD_ACTOR MCMAN_WORK
#endif

#if MGS_VERSION == 3
#define MEMORY_CARD_ACTOR void
#endif

class TransfarringMemoryCardFileList : public TransfarringFileList
{
public:
   TransfarringMemoryCardFileList( MEMORY_CARD_ACTOR* memcardActor );

   void SetMemoryCardActor( MEMORY_CARD_ACTOR* memcardActor );
   ESaveType GetMemoryCardActorType() const;

   // Anything marked as _SubThread will stall the main thread; call it from a worker thread

   // Populates the file list for every save type
   STransfarringError LoadAllFileLists_SubThread();
   STransfarringError LoadFileList_SubThread(ESaveType saveType);
   STransfarringError SaveFile_SubThread(TRANSFARRING_GAME_DATA& data, ESaveType saveType, int saveIndex);
   STransfarringError LoadFile_SubThread(TRANSFARRING_GAME_DATA& data, ESaveType saveType, int loadIndex);
   STransfarringError DeleteFile_SubThread(ESaveType gameType, int deleteIndex);

   // Support for game-thread operations
   STransfarringError LoadAllFileLists_GameThread();
   STransfarringError LoadFile_GameThread(TRANSFARRING_GAME_DATA& data, ESaveType saveType, int loadIndex);
   STransfarringError SaveFile_GameThread(TRANSFARRING_GAME_DATA& data, ESaveType saveType, int saveIndex);
   STransfarringError DeleteFile_GameThread(ESaveType gameType, int deleteIndex);
   STransfarringError DeleteNonGameFile_GameThread(ENonGameSaveType gameType, int deleteIndex);

private:
   // Stall the thread until the memory card system is idle
   int WaitForMemoryCard_SubThread();
   void SetMemoryCardActorType(ESaveType saveType);
   int CreateSaveFileFromCodedString(const char* name, TransfarringSaveFile* file, ESaveType saveType);

   MEMORY_CARD_ACTOR* m_memcardActor;

   // Support for game-thread loading

};
