#pragma once
#ifndef __DATAUNIFIER__H
#define __DATAUNIFIER__H

#include <vector>
#include <string>
#include <map>

namespace sdx
{
   int const kPackageHeaderSize = 256;
   int const kADPCMPacketSize = 16;
   int const kVagLoopStart = 0x04;
   int const kVagLoopBlock = 0x02;
   int const kVagLoopEnd = 0x01;

   int const kSeExpTableSize = 0x800;
   int const kSeDataSize = 0x6000;
   int const kSeHeaderSize = 4096;
   int const kTotalSeDataSize = kSeHeaderSize + kSeDataSize;
   int const kSngDataSize = 0x20000;

   int const kSngDataPresent = 0x0001;
   int const kSeDataPresent = 0x0002;
   
   struct ArchiveHeader
   {
      unsigned int mId;
      unsigned int mVersion;
      unsigned int mNumPackages;
      unsigned int mNumUniqueWavForms;
      unsigned int mWavStartOffset;
   };
   
   struct WavTableEntry
   {
      unsigned int mVoiceTableOffset;
      unsigned int mVoiceTableSize;
      unsigned int mSpuLoadOffset;
      std::vector<unsigned char> mVoiceTable;
      std::vector<unsigned int> mWavFiles;
   };

   struct Package
   {
      std::string mManifestName;
      std::vector<WavTableEntry> mWavTable;
      unsigned int mOffsetInArchive;
      unsigned int mSizeInArchive;
      unsigned int mExtraDataFlags;
      unsigned char mSeExpTable[kSeExpTableSize];
      unsigned char mSeData[kTotalSeDataSize];
      unsigned char mSngData[kSngDataSize];
   };

   std::string ComputeMD5Hash(const unsigned char* const pBuf, int const length);

   class DataUnifier 
   {
   public:

      DataUnifier() : mNumDuplicatedWavs(0) { }
      ~DataUnifier() { }

      void RegisterPackage(const char* const pFullPath, const char* const pPackageName);
      void WriteArchiveAndManifest();
      void DumpArchive(const char* const pFullPath, std::vector<std::vector<unsigned char> >& adpcmData);

      unsigned int GetNumDuplicatedWavs() const { return mNumDuplicatedWavs; }
      unsigned int GetNumUniqueWavs() const { return mUnifiedWavData.size(); }

   private:

      void ParseWavTableMGS2(WavTableEntry* const pWavEntry, FILE* const fp, unsigned int numBytes);
      unsigned int GetWavDataIndex(unsigned char const* const pWavDataStart, unsigned char const* const pWavDataEnd);
      unsigned int CalcAllPackageSizeAndOffsets();

      unsigned int GetSizeOfPackage(Package const& package) const;
      unsigned int GetSizeOfWavTableEntry(WavTableEntry const& entry) const;

      void SaveManifest(FILE* const fp);
      void SaveArchive(FILE* const fp);

      std::vector<Package> mSdxPackages;
      std::map<std::string, std::vector<unsigned char> > mWavForms;
      std::vector<std::string> mUnifiedWavData;
      unsigned int mNumDuplicatedWavs;
   };
}


#endif
