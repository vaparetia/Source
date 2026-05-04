#include "DataUnifier.h"

#include <direct.h>

namespace sdx
{
   static unsigned char const kEmptyADPCMPacket[kADPCMPacketSize];

   //----------------------------------------------------------------------------
   std::string ComputeMD5Hash(const unsigned char* const pBuf, int const length)
   {
      array<unsigned char>^ stringBytes = gcnew array<unsigned char>(length);

      System::Runtime::InteropServices::Marshal::Copy( (System::IntPtr)(void*)pBuf, stringBytes, 0, length);

      array<unsigned char>^ md5Hash = 
         (gcnew System::Security::Cryptography::SHA1CryptoServiceProvider())->ComputeHash(stringBytes);

      System::String^ hashString = gcnew System::String("");
      for (int i=0; i < md5Hash->Length; ++i)
      {
         hashString += (gcnew System::Byte(md5Hash[i]))->ToString( "X2");
      }

      System::IntPtr pIntPtr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(hashString);
      
      char const* pStr = (char const*)(void*)pIntPtr;
      std::string ret(pStr);

      System::Runtime::InteropServices::Marshal::FreeHGlobal(pIntPtr);

      return ret;
   }

   //----------------------------------------------------------------------------
   void DataUnifier::RegisterPackage(const char* const pFullPath, const char* const pPackageName)
   {
      FILE* fp = NULL;

      fopen_s(&fp, pFullPath, "rb");
      if (!fp)
         throw false;

#if MGS_VERSION == 2
      struct SPackageHeader
      {
         unsigned int offset;
         unsigned int code;
      };
      
      SPackageHeader pkgHeader[kPackageHeaderSize];
      Package newPkg;
      
      newPkg.mManifestName = pPackageName;
      newPkg.mExtraDataFlags = 0;

      fread(&pkgHeader[0], sizeof(pkgHeader), 1, fp);
      if (pkgHeader[0].offset)
      {
         // Offsets are in sector where the sector size is 2048 bytes.
         unsigned int numBytes = (pkgHeader[1].offset - pkgHeader[0].offset) * 2048;
         WavTableEntry newWavTableEntry;
         ParseWavTableMGS2(&newWavTableEntry, fp, numBytes);
         newPkg.mWavTable.push_back(newWavTableEntry);
      }
      else
      {
         fseek(fp, (pkgHeader[1].offset - pkgHeader[0].offset) * 2048, SEEK_CUR);
      }

      if (pkgHeader[1].offset && pkgHeader[1].code != 0xff)
      {
         // Game code does not load wav files with a code of 0xff
         unsigned int numBytes = (pkgHeader[2].offset - pkgHeader[1].offset) * 2048;
         WavTableEntry newWavTableEntry;
         ParseWavTableMGS2(&newWavTableEntry, fp, numBytes);
         newPkg.mWavTable.push_back(newWavTableEntry);
      }
      else
      {
         fseek(fp, (pkgHeader[2].offset - pkgHeader[1].offset) * 2048, SEEK_CUR);
      }

      // Load the sound effect table data.

      memset(&newPkg.mSeExpTable[0], 0, kSeExpTableSize);
      memset(&newPkg.mSeData[0], 0, kTotalSeDataSize);

      if (pkgHeader[2].offset)
      {
         if (pkgHeader[2].code != 0xff)
         {
            fread(&newPkg.mSeExpTable[0], 1, kSeExpTableSize, fp);
            unsigned int se_size = (pkgHeader[3].offset - (pkgHeader[2].offset + 1)) * 2048;
            fread(&newPkg.mSeData[0], 1, se_size, fp);

            newPkg.mExtraDataFlags |= kSeDataPresent;
         }
         else
         {
            fseek(fp, (pkgHeader[3].offset - pkgHeader[2].offset) * 2048, SEEK_CUR);
         }
      }

      memset(&newPkg.mSngData[0], 0, kSngDataSize);
      if (pkgHeader[3].offset && pkgHeader[3].code != 0xff)
      {
         fread(&newPkg.mSngData[0], 1, kSngDataSize, fp);
         newPkg.mExtraDataFlags |= kSngDataPresent;
      }

      mSdxPackages.push_back(newPkg);
#else
#error MGS3 is not yet supported.
#endif

      fclose(fp);
   }

   //----------------------------------------------------------------------------
   void DataUnifier::ParseWavTableMGS2(WavTableEntry* const pWavEntry, FILE* const fp, unsigned int numBytes)
   {
      unsigned char* pFileData = new unsigned char[numBytes];
      fread(pFileData, 1, numBytes, fp);
      unsigned char* pBuf = pFileData;

      pWavEntry->mVoiceTableOffset = ((unsigned int)*(pBuf)) << 24;
      pWavEntry->mVoiceTableOffset |= ((unsigned int)*(pBuf + 1)) << 16;
      pWavEntry->mVoiceTableOffset |= ((unsigned int)*(pBuf + 2)) << 8;
      pWavEntry->mVoiceTableOffset |= ((unsigned int)*(pBuf + 3));

      pWavEntry->mVoiceTableSize = ((unsigned int)*(pBuf + 4)) << 24;
      pWavEntry->mVoiceTableSize |= ((unsigned int)*(pBuf + 5)) << 16;
      pWavEntry->mVoiceTableSize |= ((unsigned int)*(pBuf + 6)) << 8;
      pWavEntry->mVoiceTableSize |= ((unsigned int)*(pBuf + 7));

      pBuf = &pBuf[16];
      pWavEntry->mVoiceTable.resize(pWavEntry->mVoiceTableSize);
      memcpy(&pWavEntry->mVoiceTable[0], pBuf, pWavEntry->mVoiceTableSize);

      pBuf += pWavEntry->mVoiceTableSize;

      pWavEntry->mSpuLoadOffset = ((unsigned int)*(pBuf)) << 24;
      pWavEntry->mSpuLoadOffset |= ((unsigned int)*(pBuf + 1)) << 16;
      pWavEntry->mSpuLoadOffset |= ((unsigned int)*(pBuf + 2)) << 8;
      pWavEntry->mSpuLoadOffset |= ((unsigned int)*(pBuf + 3));
      
      // Skip the load offset we don't need it
      unsigned int wavDataSize = ((unsigned int)*(pBuf + 4)) << 24;
      wavDataSize |= ((unsigned int)*(pBuf + 5)) << 16;
      wavDataSize |= ((unsigned int)*(pBuf + 6)) << 8;
      wavDataSize |= ((unsigned int)*(pBuf + 7));

      if ((wavDataSize % kADPCMPacketSize) != 0)
      {
         printf("Wav data size should be a multiple of 16.\n");
         throw false;
      }

      pBuf += 16;

      unsigned char const* const pStart = pBuf;
      unsigned char const* const pEnd = pBuf + wavDataSize;
      unsigned char const* pCurr = pStart;

      for (unsigned char const* pItor = pStart + kADPCMPacketSize; pItor != pEnd; pItor += kADPCMPacketSize)
      {
         if (!memcmp(kEmptyADPCMPacket, pItor, kADPCMPacketSize))
         {
            unsigned int wavDataIndex = GetWavDataIndex(pCurr, pItor);
            pWavEntry->mWavFiles.push_back(wavDataIndex);
            pCurr = pItor;
         }
      }

      unsigned int wavDataIndex = GetWavDataIndex(pCurr, pEnd);
      pWavEntry->mWavFiles.push_back(wavDataIndex);
      
      delete [] pFileData;
   }

   //----------------------------------------------------------------------------
   unsigned int DataUnifier::GetWavDataIndex(unsigned char const* const pWavDataStart, unsigned char const* const pWavDataEnd)
   {
      // Some streams could start with empty packets so skip them.
      unsigned int const numBytes = pWavDataEnd - pWavDataStart;
      if (numBytes == 0)
      {
         printf("Empty wav data found!\n");
         throw false;
      }

      std::string hashValue = ComputeMD5Hash(pWavDataStart, numBytes);
      std::map<std::string, std::vector<unsigned char> >::iterator itor = mWavForms.find(hashValue);

      if (itor == mWavForms.end())
      {
         // Encountered unique waveform data.
         std::vector<unsigned char>& wavData = mWavForms[hashValue]; // insert if not found and fill in the wav data.
         wavData.resize(numBytes);
         memcpy(&wavData[0], pWavDataStart, numBytes);

         mUnifiedWavData.push_back(hashValue);
      }
      else
      {
         mNumDuplicatedWavs++;
      }

      unsigned int numUniqueWavs = mUnifiedWavData.size();
      for (unsigned int ui = 0; ui < numUniqueWavs; ui++)
      {
         if (mUnifiedWavData[ui] == hashValue)
            return ui;
      }

      printf("Could not find wav data index!\n");
      throw false;

      return 0xffffffff;
   }

   //----------------------------------------------------------------------------
   void DataUnifier::WriteArchiveAndManifest()
   {
      FILE* fp = NULL;
      char* const kArchiveName = "unified_sdx_archive.sdx";
      char* const kArchiveManifestName = "unified_sdx_archive.sdx.manifest";

      fopen_s(&fp, kArchiveName, "wb");
      if (!fp)
      {
         printf("Unable to open output archive for writing.\n");
         throw false;
      }

      SaveArchive(fp);
      fclose(fp);
      fp = NULL;

      fopen_s(&fp, kArchiveManifestName, "wt");
      if (!fp)
      {
         printf("Unable to open output manifest file for writing.\n");
         throw false;
      }

      SaveManifest(fp);
      fclose(fp);
   }

   //----------------------------------------------------------------------------
   void DataUnifier::SaveManifest(FILE* const fp)
   {
      unsigned int numPackages = mSdxPackages.size();
      for (unsigned int ui = 0; ui < numPackages; ++ui)
      {
         Package& currPkg = mSdxPackages[ui];
         fprintf(fp, "%s %d %d\n", currPkg.mManifestName.c_str(), currPkg.mOffsetInArchive, currPkg.mSizeInArchive);
      }
   }

   //----------------------------------------------------------------------------
   void DataUnifier::SaveArchive(FILE* const fp)
   {
      unsigned int const kFileVersion = 1;

      ArchiveHeader archiveHeader;

      archiveHeader.mId = 'ASDX';
      archiveHeader.mVersion = kFileVersion;
      archiveHeader.mNumPackages = mSdxPackages.size();
      archiveHeader.mNumUniqueWavForms = mUnifiedWavData.size();
      archiveHeader.mWavStartOffset = CalcAllPackageSizeAndOffsets();

      fwrite(&archiveHeader, sizeof(ArchiveHeader), 1, fp);

      // Write out all the package information.
      unsigned int numPackages = mSdxPackages.size();
      for (unsigned int ui = 0; ui < numPackages; ++ui)
      {
         Package& currPkg = mSdxPackages[ui];

         fwrite(&currPkg.mExtraDataFlags, sizeof(unsigned int), 1, fp);

         if (currPkg.mExtraDataFlags & kSeDataPresent)
         {
            fwrite(&currPkg.mSeExpTable[0], 1, kSeExpTableSize, fp);
            fwrite(&currPkg.mSeData[0], 1, kTotalSeDataSize, fp);
         }

         if (currPkg.mExtraDataFlags & kSngDataPresent)
         {
            fwrite(&currPkg.mSngData[0], 1, kSngDataSize, fp);
         }

         unsigned int numTableEntries = currPkg.mWavTable.size(); 
         fwrite(&numTableEntries, sizeof(unsigned int), 1, fp);

         for (unsigned int uj = 0; uj < numTableEntries; ++uj)
         {
            WavTableEntry const& entry = currPkg.mWavTable[uj];
            
            fwrite(&entry.mVoiceTableOffset, sizeof(unsigned int), 1, fp);
            fwrite(&entry.mVoiceTableSize, sizeof(unsigned int), 1, fp);
            fwrite(&entry.mSpuLoadOffset, sizeof(unsigned int), 1, fp);

            unsigned int numVoiceTableBytes = entry.mVoiceTable.size();
            fwrite(&numVoiceTableBytes, sizeof(unsigned int), 1, fp);

            unsigned int numWavFiles = entry.mWavFiles.size();
            fwrite(&numWavFiles, sizeof(unsigned int), 1, fp);

            fwrite(&entry.mWavFiles[0], sizeof(unsigned int), numWavFiles, fp);
            fwrite(&entry.mVoiceTable[0], 1, numVoiceTableBytes, fp);
         }
      }

      // Write out the unified wave table. First write out a toc containing s
      unsigned int filePos = ftell(fp);
      if (filePos != archiveHeader.mWavStartOffset)
      {
         printf("Error writing archive.\n");
         throw false;
      }

      unsigned int numWavForms = mUnifiedWavData.size();
      fwrite(&numWavForms, sizeof(unsigned int), 1, fp);

      // Iterate through the vector not the map to ensure the same ordering used when created the wave table entries.
      filePos = 0; // Offset of each waveform is the offset from the end of the unified wave table toc
      std::map<std::string, std::vector<unsigned char> >::const_iterator itor;
      for (unsigned int ui = 0; ui < numWavForms; ui++)
      {
         itor = mWavForms.find(mUnifiedWavData[ui]);
         if (itor == mWavForms.end())
            throw false;

         unsigned int numBytes = itor->second.size();
         fwrite(&filePos, sizeof(unsigned int), 1, fp);
         fwrite(&numBytes, sizeof(unsigned int), 1, fp);

         filePos += numBytes;
      }

      for (unsigned int ui = 0; ui < numWavForms; ui++)
      {
         itor = mWavForms.find(mUnifiedWavData[ui]);
         if (itor == mWavForms.end())
            throw false;

         fwrite(&itor->second[0], 1, itor->second.size(), fp);
      }
   }

   //----------------------------------------------------------------------------
   unsigned int DataUnifier::CalcAllPackageSizeAndOffsets()
   {
      unsigned int numPackages = mSdxPackages.size();
      unsigned int numBytes = sizeof(ArchiveHeader); // First package starts just after the archive header.

      for (unsigned int ui = 0; ui < numPackages; ++ui)
      {
         Package& currPkg = mSdxPackages[ui];

         currPkg.mOffsetInArchive = numBytes;
         currPkg.mSizeInArchive = GetSizeOfPackage(currPkg);
         
         numBytes += currPkg.mSizeInArchive;
      }

      return numBytes;
   }

   //----------------------------------------------------------------------------
   unsigned int DataUnifier::GetSizeOfPackage(Package const& package) const
   {
      unsigned int numBytes = 0;

      numBytes += sizeof(unsigned int); // num table entries
      numBytes += sizeof(unsigned int); // Extra data flags.

      if (package.mExtraDataFlags & kSeDataPresent)
      {
         numBytes += kSeExpTableSize;
         numBytes += kTotalSeDataSize;
      }
      
      if (package.mExtraDataFlags & kSngDataPresent)
      {
         numBytes += kSngDataSize;
      }

      for (unsigned int ui = 0; ui < package.mWavTable.size(); ui++)
         numBytes += GetSizeOfWavTableEntry(package.mWavTable[ui]);

      return numBytes;
   }

   //----------------------------------------------------------------------------
   unsigned int DataUnifier::GetSizeOfWavTableEntry(WavTableEntry const& entry) const
   {
      unsigned int numBytes = 0;

      numBytes += sizeof(unsigned int); // offset
      numBytes += sizeof(unsigned int); // size
      numBytes += sizeof(unsigned int); // spu load offset
      numBytes += sizeof(unsigned int); // voice table size
      numBytes += sizeof(unsigned int); // num wav file indices
      numBytes += entry.mVoiceTable.size();
      numBytes += entry.mWavFiles.size() * sizeof(unsigned int);

      return numBytes;
   }

   //----------------------------------------------------------------------------
   void DataUnifier::DumpArchive(const char* const pFullPath, std::vector<std::vector<unsigned char> >& adpcmData)
   {
      FILE* fp = NULL;
      fopen_s(&fp, pFullPath, "rb");
      if (!fp)
         throw false;

      ArchiveHeader header;
      fread(&header, sizeof(ArchiveHeader), 1, fp);

      fseek(fp, header.mWavStartOffset, SEEK_SET);
      
      unsigned int numWavForms = 0;
      fread(&numWavForms, sizeof(unsigned int), 1, fp);
      
      unsigned int* pWavFormToc = new unsigned int[numWavForms * 2];
      fread(&pWavFormToc[0], sizeof(unsigned int), numWavForms * 2, fp);

      unsigned int wavDataStart = ftell(fp); // wav data offsets are from the start of the wav data not the file.
      for (unsigned int ui = 0; ui < numWavForms; ui++)
      {
         unsigned int offset = pWavFormToc[ui * 2];
         unsigned int numBytes = pWavFormToc[ui * 2 + 1];

         std::vector<unsigned char> wavBytes;
         wavBytes.clear();
         wavBytes.resize(numBytes);
         
         fseek(fp, wavDataStart + offset, SEEK_SET);
         fread(&wavBytes[0], 1, numBytes, fp);

         adpcmData.push_back(wavBytes);
      }

      delete [] pWavFormToc;
      fclose(fp);
   }

}
