#include "Engine/StdAfx.h"

#include "BP_BuildDefines.h"
#include "BP_FileSupport.h"
#include "BP_SoundSupport.h"

#include "BP_UnifiedSDX.h"
#include "Engine/System/CStopWatch.h"

#if BP_VITA
#  include "kernel/threadmgr.h"
#endif

#if MGS_VERSION == 2
extern "C" 
{
   extern char gCurrStageSoundDir[];
   extern unsigned char gBP_SpuMemory[];
   extern unsigned char *se_exp_table;
   extern unsigned char *se_data;
   extern unsigned char *jo_data;
   extern struct SETBL *jo_header;
   extern struct SETBL *se_header;
   extern unsigned char sng_data[];
   extern char mem_str_buf[];
   extern unsigned long spu_wave_start_ptr;
   extern void set_voice_tbl(struct WAVE_W *p, unsigned int size, unsigned int offset);

   extern int gBP_WavFormPitch;
}

#if BP_VITA
namespace
{
   SceUID sBackgroundLoadThread = -1;
}
#endif

//----------------------------------------------------------------------------
namespace sdx
{

//----------------------------------------------------------------------------
class UnifiedArchive* gpSDXArchive = NULL;

uint32 const kArchiveFileVersion = 1;
uint32 const kArchiveFileId = 'ASDX';
uint32 const kSeExpTableSize = 0x800;
uint32 const kSeDataSize = 0x6000;
uint32 const kSeHeaderSize = 4096;
uint32 const kTotalSeDataSize = kSeHeaderSize + kSeDataSize;
uint32 const kSngDataSize = 0x20000;

uint32 const kSngDataPresent = 0x0001;
uint32 const kSeDataPresent = 0x0002;

// See sd_incl, MEM_STR_BUF_SIZE
uint32 const kMemStrMaxSize = 0x80000;

// Custom AP sensor sound effect file name.
char const *const kAPSensorSoundEffectFileName = "misc/ap_sensor_ding.wav";

/// Used to identify the different asynchronous loading states when reading a package.
enum ELoadStatus
{
   kLoadStatus_SeTableData,
   kLoadStatus_SeData,
   kLoadStatus_SngData,
   kLoadStatus_Wav_Files,
   kLoadStatus_Done,
};

/// Used for verifying the target of wav copy operations.
enum EWavMemTarget
{
   kMemTarget_SpuMem,
   kMemTarget_MemStream,
};

/// Header for the unified archive.
struct ArchiveHeader
{
   uint32 mId;
   uint32 mVersion;
   uint32 mNumPackages;
   uint32 mNumUniqueWavForms;
   uint32 mWavStartOffset;
};

/// Each sound package has a table of wav data stored in the package. The wav table in this case stores sound voice 
/// config data and an array of indices which index into the unified wav table.
struct WavTableEntry
{
   uint32 mVoiceTableOffset;
   uint32 mVoiceTableSize;
   uint32 mSpuLoadOffset;
   std::vector<unsigned char> mVoiceTable; // array of bytes we copy into the current active voice table.
   std::vector<uint32> mWavFiles; // Indices into the unified wav table.
};

/// The manifest file has an entry for each of the original package files loaded by the game. At game init time, we 
/// pre-load all the package information (~2MB) and keep it cached.
struct PackageEntry
{
   uint32 mOffset; // Offset from beginning of archive file.
   uint32 mNumBytes; // Size of the entry.
   uint32 mExtraDataFlags; // Indicates what data the package contains.
   std::vector<WavTableEntry> mWavTable;
};

/// Simple table of wav data file offsets and sizes. Each wav table entry has an index into this table which points to
/// the actual unified wav data.
struct WavData
{
   uint32 mOffset;
   uint32 mNumBytes;
};

/// Map the original package file path to it's new location in the unified archive.
typedef std::map<std::string, PackageEntry> TPackageMap;

class UnifiedArchive
{
public:
   
   UnifiedArchive() 
      : mCurrLoadStatus(kLoadStatus_Done)
      , mpCurrPakFile(NULL)
      , mpCurrFileOp(NULL)
      , mpArchiveFile(NULL) 
      , mpCurrSoundBufferPos(NULL) 
      , mCurrWavLoadIndex(0)
   {
      memset(&mLastSdxLoaded[0], 0, sizeof(mLastSdxLoaded));
   }
 
   void LoadManifest();
   bool LoadSoundPak(int const soundCode);
   bool DoesSoundPakFileExistInManifest(char const* const pSdxFilePath);
   void Close();

   void PlayAPSensorPing();

private:
   bool LoadSeTable();
   bool LoadSeData();
   bool LoadSngData();

   bool LoadNextWavFile();
   bool GetNextWavOffsetAndSize(uint32* pOffset, uint32* pNumBytes);
   void SetWavLoadPtr();

   TPackageMap mSdxPackageManifest;
   ArchiveHeader mSdxArchiveHeader;
   std::vector<WavData> mUnifiedWavTable;

   // Stores a sound effect played when the AP sensor is pinged. (5,196 bytes).
   std::vector<unsigned char> mAPSensorSound;

   // Used to track asynchronous load state.
   ELoadStatus mCurrLoadStatus;
   PackageEntry* mpCurrPakFile; // Pak file we are currently loading.
   
   void* mpCurrFileOp;
   void* mpArchiveFile;

   unsigned char* mpCurrSoundBufferPos;

   // Each package has 1 or more wav tables. (2 most of the time) This index is the one we are
   // currently loading.
   uint32 mCurrPakWavTable; 
   
   // Index of the wav data within the pak's wav table we are currently loading.
   uint32 mCurrWavLoadIndex;

   EWavMemTarget mCurrWavMemTarget;

   char mLastSdxLoaded[512];
};

//----------------------------------------------------------------------------
void UnifiedArchive::LoadManifest()
{
   const char* const kManifestFileName = "misc/unified_sdx_archive.sdx.manifest";
   const char* const kArchiveFileName = "misc/unified_sdx_archive.sdx";

   CStopWatch sdxLoadStopwatch;

   printf("Loading unified SDX manifest file....\n");

   void* pFile = BP_OpenFile(kManifestFileName, NULL);
   BPE_VERIFY(pFile, false, "Error opening sdx manifest file.");

   long numBytes = BP_GetFileSize(pFile);
   char* pManifestContents = new char[numBytes + 1];
   pManifestContents[numBytes] = '\0';

   BP_ReadFile(pFile, pManifestContents, numBytes);
   
   char* pStart = pManifestContents;
   size_t pos = strcspn(pStart, "\n");

   while (pos)
   {
      PackageEntry newEntry;
      char origName[256];
      uint32 offset;
      uint32 size;
      int ret = 0;

      ret = sscanf(pStart, "%s %d %d", origName, &offset, &size);
      BPE_VERIFY(ret == 3, false, "Error parsing manifest file.");
      
      newEntry.mNumBytes = size;
      newEntry.mOffset = offset;
      mSdxPackageManifest.insert(std::make_pair<std::string, PackageEntry>(origName, newEntry));

      pStart += pos + 1;
      pos = strcspn(pStart, "\n");
   }

   delete [] pManifestContents;
   BP_CloseFile(pFile);

   // Open the archive file so it's ready to read from. Just keep it open instead of opening and closing it each time a 
   // pak file is read.
   mpArchiveFile = BP_OpenFile(kArchiveFileName, NULL);
   BPE_VERIFY(mpArchiveFile, false, "Unable to open sdx archive file.");

   // Read the archive header information to make sure the code and tools are in sync.
   BP_ReadFile(mpArchiveFile, &mSdxArchiveHeader, sizeof(mSdxArchiveHeader));
   BPE_VERIFY(mSdxArchiveHeader.mId == kArchiveFileId, false, "Invalid archive.");
   BPE_VERIFY(mSdxArchiveHeader.mVersion == kArchiveFileVersion, false, "Invalid archive version.");

   // Pre-cache the sound packages. Does not load any wav data, just references to the unified wav table. We also skip
   // the se data at the beginning of the package data since it totals around 20MB and we don't want to keep 20MB of 
   // data always loaded.
   int const kLoadBufSize = 8 * 1024;
   std::vector<unsigned char> loadBuf(kLoadBufSize);
   for (TPackageMap::iterator itor = mSdxPackageManifest.begin(); itor != mSdxPackageManifest.end(); ++itor)
   {
      PackageEntry& currPkg = itor->second;
      unsigned char* pLoadBuf = &loadBuf[0];
      uint32 const kNumSeDataBytes = kTotalSeDataSize + kSeExpTableSize;

      BP_SeekFile(mpArchiveFile, currPkg.mOffset, SEEK_SET);
      BP_ReadFile(mpArchiveFile, &currPkg.mExtraDataFlags, sizeof(uint32));

      uint32 skipBytes = sizeof(uint32); // extra data flags
      if (currPkg.mExtraDataFlags & kSeDataPresent)
         skipBytes += kNumSeDataBytes;
      if (currPkg.mExtraDataFlags & kSngDataPresent)
         skipBytes += kSngDataSize;

      BPE_VERIFY((currPkg.mNumBytes - skipBytes) < kLoadBufSize, false, "Increase buffer size.");
      BP_SeekFile(mpArchiveFile, currPkg.mOffset + skipBytes, SEEK_SET);
      BP_ReadFile(mpArchiveFile, pLoadBuf, currPkg.mNumBytes - skipBytes);

      uint32 numWavTableEntries = *(uint32*)pLoadBuf;
      pLoadBuf += sizeof(uint32);

      currPkg.mWavTable.resize(numWavTableEntries);
      for (uint32 ui = 0; ui < numWavTableEntries; ui++)
      {
         WavTableEntry& currTable = currPkg.mWavTable[ui];
         
         currTable.mVoiceTableOffset = *(uint32*)pLoadBuf;
         pLoadBuf += sizeof(uint32);

         currTable.mVoiceTableSize = *(uint32*)pLoadBuf;
         pLoadBuf += sizeof(uint32);

         currTable.mSpuLoadOffset = *(uint32*)pLoadBuf;
         pLoadBuf += sizeof(uint32);

         uint32 numVoiceTableBytes = *(uint32*)pLoadBuf;
         pLoadBuf += sizeof(uint32);

         uint32 numWavs = *(uint32*)pLoadBuf;
         pLoadBuf += sizeof(uint32);

         currTable.mWavFiles.resize(numWavs);
         memcpy(&currTable.mWavFiles[0], pLoadBuf, sizeof(uint32) * numWavs);
         pLoadBuf += sizeof(uint32) * numWavs;

         currTable.mVoiceTable.resize(numVoiceTableBytes + (4 * 1024));
         memcpy(&currTable.mVoiceTable[0], pLoadBuf, numVoiceTableBytes);
         pLoadBuf += numVoiceTableBytes;
      }
   }

   // Load the wav table which contains offsets and sizes to unified wav data.
   BP_SeekFile(mpArchiveFile, mSdxArchiveHeader.mWavStartOffset, SEEK_SET);
   
   uint32 numUniqueWavs;
   BP_ReadFile(mpArchiveFile, &numUniqueWavs, sizeof(uint32));

   mUnifiedWavTable.resize(numUniqueWavs);
   BP_ReadFile(mpArchiveFile, &mUnifiedWavTable[0], numUniqueWavs * sizeof(WavData));

   // Set wav offset to be the start of the first chunk of actual wav data. This is used later when calculating the 
   // start of a wav file.
   mSdxArchiveHeader.mWavStartOffset = mSdxArchiveHeader.mWavStartOffset + sizeof(uint32) + 
      numUniqueWavs * sizeof(WavData);

   // Finally cache the ap sensor sound effect.
   void* pSoundFile = BP_OpenFile(kAPSensorSoundEffectFileName, NULL);
   BPE_VERIFY(pSoundFile, false, "Unable to open AP sensor sound file.");

   long fileSize = BP_GetFileSize(pSoundFile);
   mAPSensorSound.resize(fileSize);
   BP_ReadFile(pSoundFile, &mAPSensorSound[0], fileSize);
   BP_CloseFile(pSoundFile);

   printf("Loaded %d entries from SDX manifest in %g seconds.\n", mSdxPackageManifest.size(), sdxLoadStopwatch.GetElapsedTime() );
}

//----------------------------------------------------------------------------
bool UnifiedArchive::DoesSoundPakFileExistInManifest(char const* const pSdxFilePath)
{
   if (!pSdxFilePath)
      return false;

   TPackageMap::iterator itor = mSdxPackageManifest.find(pSdxFilePath);
   if (itor == mSdxPackageManifest.end())
   {
      return false;
   }
   else
   {
      return true;
   }
}

//----------------------------------------------------------------------------
bool UnifiedArchive::LoadSoundPak(int const soundCode)
{
   //JP and EU sound paks match US versions.
   const char* const kRegionDir = "us";

   if (mCurrLoadStatus == kLoadStatus_Done)
   {
      BPE_VERIFY(!mpCurrFileOp, false, "File op in progress!");
      BPE_VERIFY(!mpCurrPakFile, false, "There should not be a current sound pak.");

      char sdxFilePath[512];
      sprintf(sdxFilePath, "%s/stage/%s/pk%06x.sdx", kRegionDir, gCurrStageSoundDir, soundCode);
      if (!strcmp(sdxFilePath, mLastSdxLoaded))
      {
         return true; // If we already have the sdx contents in memory no need to load again.
      }
      else
      {
         strncpy(mLastSdxLoaded, sdxFilePath, 512);
      }

      TPackageMap::iterator itor = mSdxPackageManifest.find(sdxFilePath);

      // See stage_flatfs.cpp:404 Apparently 1 or more scripts have a bug that tries to load invalid sound data.
      if (itor == mSdxPackageManifest.end())
      {
         memset(&mLastSdxLoaded[0], 0, 512);
         return true;
      }

      mpCurrPakFile = &itor->second;
      mCurrWavLoadIndex = 0;
      mCurrPakWavTable = 0;

      set_voice_tbl((struct WAVE_W*)&mpCurrPakFile->mWavTable[mCurrPakWavTable].mVoiceTable[0], 
         mpCurrPakFile->mWavTable[mCurrPakWavTable].mVoiceTableSize,
         mpCurrPakFile->mWavTable[mCurrPakWavTable].mVoiceTableOffset);

      SetWavLoadPtr();
      mCurrLoadStatus = kLoadStatus_Wav_Files;
   }

   bool finishedLoading = false;
   switch (mCurrLoadStatus)
   {
   case kLoadStatus_Wav_Files:
      if (LoadNextWavFile())
      {
         BPE_VERIFY(!mpCurrFileOp, false, "Did not complete a file op");
         mCurrLoadStatus = kLoadStatus_SeTableData;
      }
      break;

   case kLoadStatus_SeTableData:
      if (LoadSeTable())
      {
         BPE_VERIFY(!mpCurrFileOp, false, "Did not complete a file op");
         mCurrLoadStatus = kLoadStatus_SeData;
      }
      break;

   case kLoadStatus_SeData:
      if (LoadSeData())
      {
         BPE_VERIFY(!mpCurrFileOp, false, "Did not complete a file op");
         mCurrLoadStatus = kLoadStatus_SngData;
      }
      break;

   case kLoadStatus_SngData:
      if (LoadSngData())
      {
         BPE_VERIFY(!mpCurrFileOp, false, "Did not complete a file op");
         mpCurrPakFile = NULL;
         finishedLoading = true;
         mCurrLoadStatus = kLoadStatus_Done;
      }
      break;

   case kLoadStatus_Done:
      finishedLoading = true;
      break;
   }

   return finishedLoading;
}

//----------------------------------------------------------------------------
bool UnifiedArchive::LoadSeTable()
{
   if (mpCurrPakFile->mExtraDataFlags & kSeDataPresent)
   {
      if (mpCurrFileOp)
      {
         if (BP_TryFinishFileOp(mpCurrFileOp))
         {
            mpCurrFileOp = NULL;
            return true;
         }
      }
      else
      {
         // Skip the extra data flags.
         BP_SeekFile(mpArchiveFile, mpCurrPakFile->mOffset + sizeof(uint32), SEEK_SET);
         mpCurrFileOp = BP_ReadFileAsync(mpArchiveFile, se_exp_table, kSeExpTableSize);
      }

      return false;
   }
   else
   {
      return true;
   }
}

//----------------------------------------------------------------------------
bool UnifiedArchive::LoadSeData()
{
   if (mpCurrPakFile->mExtraDataFlags & kSeDataPresent)
   {
      if (mpCurrFileOp)
      {
         if (BP_TryFinishFileOp(mpCurrFileOp))
         {
            mpCurrFileOp = NULL;
            return true;
         }
         else
         {
            return false;
         }
      }
      else
      {
         // Skip the extra data flags and the table we just read.
         BP_SeekFile(mpArchiveFile, mpCurrPakFile->mOffset + sizeof(uint32) + kSeExpTableSize, SEEK_SET);

         if (se_exp_table[0] == 0xfe && se_exp_table[1] == 0xfe)
         {
            mpCurrFileOp = BP_ReadFileAsync(mpArchiveFile, jo_header, kTotalSeDataSize);
         }
         else
         {
            mpCurrFileOp = BP_ReadFileAsync(mpArchiveFile, se_header, kTotalSeDataSize);
         }

         return false;
      }
   }
   else
   {
      return true;
   }
}

//----------------------------------------------------------------------------
bool UnifiedArchive::LoadSngData()
{
   if (mpCurrPakFile->mExtraDataFlags & kSngDataPresent)
   {
      if (mpCurrFileOp)
      {
         if (BP_TryFinishFileOp(mpCurrFileOp))
         {
            mpCurrFileOp = NULL;
            return true;
         }
         else
         {
            return false;
         }
      }
      else
      {
         // If se data was present seek past it else just skip the extra data flags.
         if (mpCurrPakFile->mExtraDataFlags & kSeDataPresent)
         {
            BP_SeekFile(mpArchiveFile, mpCurrPakFile->mOffset + sizeof(uint32) + kSeExpTableSize + kTotalSeDataSize, 
               SEEK_SET);
         }
         else
         {
            BP_SeekFile(mpArchiveFile, mpCurrPakFile->mOffset + sizeof(uint32), SEEK_SET);
         }

         mpCurrFileOp = BP_ReadFileAsync(mpArchiveFile, sng_data, kSngDataSize);
         return false;
      }
   }
   else
   {
      return true;
   }
}

//----------------------------------------------------------------------------
void UnifiedArchive::SetWavLoadPtr()
{
   BPE_VERIFY(mpCurrPakFile, false, "Invalid pak file.");

   if (mpCurrPakFile->mWavTable[mCurrPakWavTable].mVoiceTableOffset >= 0x1000)
   {
      mpCurrSoundBufferPos = (unsigned char*)(&mem_str_buf[0]);
      mCurrWavMemTarget = kMemTarget_MemStream;
   }
   else
   {
      mpCurrSoundBufferPos = &gBP_SpuMemory[0] + spu_wave_start_ptr + mpCurrPakFile->mWavTable[mCurrPakWavTable].mSpuLoadOffset;
      mCurrWavMemTarget = kMemTarget_SpuMem;
   }
}

//----------------------------------------------------------------------------
bool UnifiedArchive::LoadNextWavFile()
{
   BPE_VERIFY(mpCurrPakFile, false, "Invalid pak file.");

   uint32 offset;
   uint32 numBytes;
   
   // If we have a valid file op then we are currently loading a wav form. If the file op is null and we still have 
   // more wav files to load from the package continue loading.
   if (mpCurrFileOp)
   {
      if (BP_TryFinishFileOp(mpCurrFileOp))
      {
         // If this returns false, we have loaded all the wav files.
         if (GetNextWavOffsetAndSize(&offset, &numBytes))
         {
            unsigned char* pMemEnd = NULL;
            if (mCurrWavMemTarget == kMemTarget_SpuMem)
            {
               pMemEnd = &gBP_SpuMemory[0] + BP_SPU_MEMORY_SIZE;
            }
            else
            {
               pMemEnd = (unsigned char*)(&mem_str_buf[0]) + kMemStrMaxSize;
            }

            BPE_VERIFY(mpCurrSoundBufferPos + numBytes < pMemEnd, false, "Overflowed audio buffer.");
          
            BP_SeekFile(mpArchiveFile, offset, SEEK_SET);
            mpCurrFileOp = BP_ReadFileAsync(mpArchiveFile, mpCurrSoundBufferPos, numBytes);
            mpCurrSoundBufferPos += numBytes;
         }
         else
         {
            mpCurrFileOp = NULL;
            return true;
         }
      }
   }
   else
   {
      if (!GetNextWavOffsetAndSize(&offset, &numBytes))
         return true; // No more wav files left we are done loading.

      BPE_VERIFY(numBytes > 0, false, "Invalid wav entry."); // There should be at least one valid wav file in there.

      BP_SeekFile(mpArchiveFile, offset, SEEK_SET);
      mpCurrFileOp = BP_ReadFileAsync(mpArchiveFile, mpCurrSoundBufferPos, numBytes);
      mpCurrSoundBufferPos += numBytes;
   }

   return false;
}

//----------------------------------------------------------------------------
bool UnifiedArchive::GetNextWavOffsetAndSize(uint32* pOffset, uint32* pNumBytes)
{
   BPE_VERIFY(mpCurrPakFile, false, "Invalid package file.");
   uint32 numPakWavTables = mpCurrPakFile->mWavTable.size();

   if (mCurrWavLoadIndex == mpCurrPakFile->mWavTable[mCurrPakWavTable].mWavFiles.size())
   {
      mCurrWavLoadIndex = 0;
      mCurrPakWavTable++;

      if (mCurrPakWavTable == mpCurrPakFile->mWavTable.size())
      {
         // We have read all of the wav data for the package.
         return false;
      }

      set_voice_tbl((struct WAVE_W*)&mpCurrPakFile->mWavTable[mCurrPakWavTable].mVoiceTable[0], 
         mpCurrPakFile->mWavTable[mCurrPakWavTable].mVoiceTableSize,
         mpCurrPakFile->mWavTable[mCurrPakWavTable].mVoiceTableOffset);

      // This is the first wav file we have loaded for the current wav table so set the location in the emulated SPU 
      // memory now.
      SetWavLoadPtr();
   }

   WavData& newWavData = mUnifiedWavTable[mpCurrPakFile->mWavTable[mCurrPakWavTable].mWavFiles[mCurrWavLoadIndex]];
   *pOffset = newWavData.mOffset + mSdxArchiveHeader.mWavStartOffset;
   *pNumBytes = newWavData.mNumBytes;
   
   mCurrWavLoadIndex++;
   return true;
}

//----------------------------------------------------------------------------
void UnifiedArchive::Close()
{
   if (mpArchiveFile)
      BP_CloseFile(mpArchiveFile);
   mpArchiveFile = NULL;
}

//----------------------------------------------------------------------------
void UnifiedArchive::PlayAPSensorPing()
{
   BP_PlayWaveform(&mAPSensorSound[0], mAPSensorSound.size(), gBP_WavFormPitch, 1);
}

} // end namespace sdx

#endif // MGS_VERSION == 2

//----------------------------------------------------------------------------
void BP_LoadUnifiedSDXArchive()
{
#if MGS_VERSION == 2
   BPE_VERIFY(sdx::gpSDXArchive == NULL, false, "Double initialization.");
   
   sdx::gpSDXArchive = new sdx::UnifiedArchive();
   sdx::gpSDXArchive->LoadManifest();
#endif
}

#if BP_VITA

static SceInt32 vita_loadunifiedsdx_entry(
   SceSize argSize,
   void *pArgBlock
   )
{
   BP_LoadUnifiedSDXArchive();

   return 0;
}

#endif

void BP_BeginBackgroundLoadSDXArchive()
{
#if BP_VITA && MGS_VERSION == 2
   sBackgroundLoadThread = BPE_CHECK_SCE( sceKernelCreateThread( "BackgroundLoadSDX", 
      vita_loadunifiedsdx_entry,
      SCE_KERNEL_DEFAULT_PRIORITY_USER,
      64 * 1024,
      0,
      SCE_KERNEL_CPU_MASK_USER_ALL,
      NULL ) );

   BPE_CHECK_SCE( sceKernelStartThread( sBackgroundLoadThread, 0, NULL ) );
#else
#endif
}

void BP_JoinBackgroundLoadSDXArchive()
{
#if BP_VITA && MGS_VERSION == 2
   SceInt32 exitStatus = 0;

   sceKernelWaitThreadEnd( sBackgroundLoadThread, &exitStatus, NULL );
   sceKernelDeleteThread( sBackgroundLoadThread );
   sBackgroundLoadThread = -1;
#else
   BP_LoadUnifiedSDXArchive();
#endif
}


//----------------------------------------------------------------------------
int BP_DoesPakFileExistInUnifiedArchive(char const* const pSdxPath)
{
#if MGS_VERSION == 2
   return sdx::gpSDXArchive->DoesSoundPakFileExistInManifest(pSdxPath);
#else
   return 0;
#endif
}

//----------------------------------------------------------------------------
int BP_LoadPakFileFromArchive(int const soundCode)
{
#if MGS_VERSION == 2
   return sdx::gpSDXArchive->LoadSoundPak(soundCode);
#else
   return 0;
#endif
}

//----------------------------------------------------------------------------
void BP_CloseUnifiedSDXArchive()
{
#if MGS_VERSION == 2
   sdx::gpSDXArchive->Close();
   delete sdx::gpSDXArchive;
#endif
}

//----------------------------------------------------------------------------
void BP_PlayAPSensorPing()
{
#if MGS_VERSION == 2
   sdx::gpSDXArchive->PlayAPSensorPing();
#endif
}
