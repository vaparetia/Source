#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <libatrac9.h>
#include <windows.h>

#include "streamtool.h"
#include "CStreamWriter_Vita_AT9.h"

#define SONY_ATRAC9_WAVEFORMAT_VERSION (1)
#define AT9_HEADER_CB_SIZE (34)
#define FORMAT_AT9 (9)

int const kGameDiskSectorSize = 2048;

//----------------------------------------------------------------------------
namespace 
{
   struct At9Header
   {
      static const unsigned int kRIFFId = 'FFIR';
      static const unsigned int kWaveId = 'EVAW';
      static const unsigned int kFmtId = ' tmf';
      static const unsigned int kDataId = 'atad';
      static const unsigned int kFactId = 'tcaf';

      unsigned int numChannels;
      unsigned int freq;
      unsigned int avgBytesPerSec; //(blockAlign * samplesPerSec) / samplesPerBlock
      unsigned int bitsPerSample;
      unsigned char configData[4];
      unsigned char* pAt9DataStart;
      unsigned int numAt9Bytes; // Total size in bytes of the compressed audio data.
      unsigned int encodedDelay;
      unsigned int totalEncodedSamples;

      At9Header() : numChannels(0), freq(0), avgBytesPerSec(0), bitsPerSample(0), pAt9DataStart(NULL), 
         numAt9Bytes(0), encodedDelay(0), totalEncodedSamples(0)
      {
         memset(&configData[0], 0, sizeof(configData));
      }

      void ParseAt9Header(unsigned char* pBytes, unsigned int numBytes)
      {
         unsigned int* pWaveData = (unsigned int*)pBytes;
         static const unsigned char kAt9FormatGuid[16] = {
            0xD2, 0x42, 0xE1, 0x47, 0xBA, 0x36, 0x8D, 0x4D, 0x88, 0xFC, 0x61, 0x65, 0x4F, 0x8C, 0x83, 0x6C
         };
         
         if (*pWaveData++ != kRIFFId)
         {
            printf("Not a valid audio file.\n");
            throw false;
         }

         pWaveData++;

         if (*pWaveData++ != kWaveId)
         {
            printf("Not a valid wave file.\n");
            throw false;
         }

         unsigned char* pDataBegin = (unsigned char*)pWaveData;
         unsigned char* pDataEnd = pBytes + numBytes;

         while (pDataBegin < pDataEnd)
         {
            unsigned int chunkId = *(unsigned int*)pDataBegin;
            unsigned int chunkLength = *(unsigned int*)(pDataBegin + sizeof(unsigned int));
            
            pDataBegin += (sizeof(unsigned int) * 2);

            if ((chunkLength % 2) == 1)
               chunkLength++;
            
            switch (chunkId)
            {
            case kFmtId:
               {
                  unsigned short format = *(unsigned short*)pDataBegin;
                  pDataBegin += sizeof(unsigned short);

                  if (format != 0xfffe) // WAVE_FORMAT_EXTENSIBLE
                  {
                     printf("Not a valid at9 file.\n");
                     throw false;
                  }

                  numChannels = *(unsigned short*)pDataBegin;
                  pDataBegin += sizeof(unsigned short);

                  freq = *(unsigned int*)pDataBegin;
                  pDataBegin += sizeof(unsigned int);

                  avgBytesPerSec = *(unsigned int*)pDataBegin;
                  pDataBegin += sizeof(unsigned int);

                  // Skip block align.
                  pDataBegin += sizeof(unsigned short);

                  bitsPerSample = *(unsigned short*)pDataBegin;
                  pDataBegin += sizeof(unsigned short);

                  unsigned short cbSize = *(unsigned short*)pDataBegin;
                  pDataBegin += sizeof(unsigned short);
                  if (cbSize != AT9_HEADER_CB_SIZE)
                  {
                     printf("CBSIZE must be 34 for atrac9 formatted audio files.\n");
                     throw false;
                  }

                  // Skip sample info..
                  pDataBegin += sizeof(unsigned short);

                  // Don't need the channel mask.
                  pDataBegin += sizeof(unsigned int);

                  GUID codecGuid;
                  memcpy(&codecGuid, pDataBegin, sizeof(GUID));
                  pDataBegin += sizeof(GUID);
                  if (memcmp(&kAt9FormatGuid[0], &codecGuid, 16))
                  {
                     printf("Not a valid at9 file. Codec guid does not match expected value.\n");
                     throw false;
                  }

                  // Skip version...
                  unsigned int version = *(unsigned int*)pDataBegin;
                  pDataBegin += sizeof(unsigned int);

                  if (version != SONY_ATRAC9_WAVEFORMAT_VERSION)
                  {
                     printf("Invalid AT9 version.\n");
                     throw false;
                  }

                  memcpy(&configData[0], pDataBegin, 4);
                  pDataBegin += 4;

                  // Skip reserved.. we don't need it.
                  pDataBegin += sizeof(unsigned int);
               }
               break;

            case kFactId:
               totalEncodedSamples = *(unsigned int*)pDataBegin;
               pDataBegin += sizeof(unsigned int);
               if (chunkLength >= 8)
               {
                  // Skip overlap delay samples..
                  pDataBegin += sizeof(unsigned int);
                  encodedDelay = *(unsigned int*)pDataBegin;
                  pDataBegin += sizeof(unsigned int);
               }

               pDataBegin += (chunkLength - 12);
               break;

            case kDataId:
               pAt9DataStart = pDataBegin;
               numAt9Bytes = chunkLength;
               pDataBegin += chunkLength;
               break;

            default:
               pDataBegin += chunkLength;
            }
         }
      }
   };

   //----------------------------------------------------------------------------
   static void FindReplacementAt9Files(std::vector<std::string>& filePaths)
   {
      char acPath[FILENAME_MAX];
      sprintf(acPath, "%s/%s", gOutputFolder, gCurrStreamName);

      System::String^ srcPath = gcnew System::String(acPath);
      array<System::String^>^ files = System::IO::Directory::GetFiles(srcPath, "*.at9", 
         System::IO::SearchOption::AllDirectories);

      for (int i = 0; i < files->Length; i++)
         filePaths.push_back(BP_ConvertManagedString(files[i]));

      //If there's a at9 file in a BP override folder, choose just that one.
      std::string bestFile;
      for( std::vector<std::string>::iterator it = filePaths.begin(); it != filePaths.end(); ++it )
      {
         char currPath[FILENAME_MAX];
         strcpy( currPath, it->c_str() );
         BP_UnifyPath( currPath );

         if( strstr( currPath, BP_GetPlatformSubfolder( kPlatform_X360 ) ) )
         {
            bestFile = currPath;
            break;
         }
         if( strstr( currPath, BP_GetPlatformSubfolder( kPlatform_Base ) ) )
         {
            bestFile = currPath;
         }
      }

      if( !bestFile.empty() )
      {
         filePaths.clear();
         filePaths.push_back( bestFile );
      }
   }

   //----------------------------------------------------------------------------
   static void CheckAt9Error(HANDLE_ATRAC9 const at9Handle, int const res)
   {
      if (res == 0)
         return;

      printf("Atrac9 error = 0x%x\n", res);
      if (res == SCE_AT9_ERROR_INTERNAL_ERROR) 
      {
         SceAt9InternalErrorInfo internalErrorInfo;
         sceAt9GetInternalErrorInfo(at9Handle, &internalErrorInfo);
         printf("internalErrorInfo [%d, %d, %d]", internalErrorInfo.errorCode, internalErrorInfo.detailError, internalErrorInfo.detailError);
      }

      throw false;
   }

   //----------------------------------------------------------------------------
   static void CalculateTotalStreamSizeWithPadding(CStreamWriter_Vita_AT9::AudioStream* const pAudioStream)
   {
      unsigned int totalPacketSize = 0;
      unsigned int numFrames = pAudioStream->mTracks[0]->mAudioFrames.size();

      for (unsigned int i = 0; i < numFrames; i++)
      {
         unsigned int packetSize = 0;
         
         for (unsigned int j = 0; j < pAudioStream->mTracks.size(); j++)
         {
            packetSize += pAudioStream->mTracks[j]->mAudioFrames[i].mAt9NumBytes;
         }

         totalPacketSize += (packetSize + 15) & ~15;
      }

      pAudioStream->mStreamHeader.mStreamDataSize = totalPacketSize;
   }

   //----------------------------------------------------------------------------
   static void GetAudioHeaderInfo(CStreamWriter_Vita_AT9::AudioTrack* const pAudioTrack, 
      CStreamWriter_Vita_AT9::At9AudioHeader* pAudioHeader)
   {
      At9Header at9Header;
      at9Header.ParseAt9Header(pAudioTrack->mpData, pAudioTrack->mBufferSizeInBytes);

      pAudioHeader->mCookie = 'VAT9';
      pAudioHeader->mFormat = FORMAT_AT9;
      pAudioHeader->mChannels = at9Header.numChannels; // Number of channels is in the lower 16 bits
      pAudioHeader->mFrequency = at9Header.freq;
      pAudioHeader->mTotalStreamSamples = at9Header.totalEncodedSamples;
      pAudioHeader->mAT9DataSize = at9Header.numAt9Bytes;
      pAudioHeader->mStreamDataSize = 0; // Set after parsing all tracks.
      memcpy(&pAudioHeader->mConfigData[0], &at9Header.configData[0], sizeof(unsigned int));
   }

   //----------------------------------------------------------------------------
   static bool CompareAudioHeaders(CStreamWriter_Vita_AT9::At9AudioHeader* const headerA, 
      CStreamWriter_Vita_AT9::At9AudioHeader* const headerB)
   {
      return headerA->mCookie == headerB->mCookie && 
         headerA->mFormat == headerB->mFormat && 
         headerA->mChannels == headerB->mChannels && 
         headerA->mFrequency == headerB->mFrequency && 
         headerA->mStreamDataSize == headerB->mStreamDataSize && 
         headerA->mTotalStreamSamples == headerB->mTotalStreamSamples &&
         !memcmp(&headerA->mConfigData[0], &headerB->mConfigData[0], sizeof(headerA->mConfigData));
   }

   //----------------------------------------------------------------------------
   static void GetAudioFrameInfo(CStreamWriter_Vita_AT9::AudioTrack* const pAudioTrack)
   {
      At9Header at9Header;
      at9Header.ParseAt9Header(pAudioTrack->mpData, pAudioTrack->mBufferSizeInBytes);

      pAudioTrack->mpAudioDataStart = at9Header.pAt9DataStart;

      HANDLE_ATRAC9 at9Handle = sceAt9GetHandle();
      if (!at9Handle)
      {
         printf("Error getting handle to ATrac9 decoder.\n");
         throw false;
      }

      int res = sceAt9DecInit(at9Handle, at9Header.configData, SCE_AT9_WORD_LENGTH_16BIT);
      CheckAt9Error(at9Handle, res);

      SceAt9CodecInfo codecInfo;
      res = sceAt9GetCodecInfo(at9Handle, &codecInfo);
      CheckAt9Error(at9Handle, res);
      
      unsigned int startSample = at9Header.encodedDelay;
      if (startSample == 0)
         startSample = codecInfo.frameSamples;
      
      // Each "packet" of at9 data is a super frame consisting of 1..4 consecutive frames. The total byte size of a 
      // super frame is fixed, however, the byte size of child frames within a super frame can vary. Decode each 
      // child frame to get a total time for the current super frame.
      unsigned char* pAt9Data = at9Header.pAt9DataStart;
      unsigned int at9BytePos = 0;
      unsigned int samplesDecoded = 0;
      unsigned int totalSamples = at9Header.totalEncodedSamples;
      unsigned int pcmBytePos = 0;

      while (at9BytePos < at9Header.numAt9Bytes)
      {
         for (int frameIndex = 0; frameIndex < codecInfo.framesInSuperframe; frameIndex++)
         {
            int bytesUsed = 0;
            int skipSample;
            int outSample;

            if ((samplesDecoded + codecInfo.frameSamples) <= startSample) 
            {
               skipSample = 0;
               outSample  = 0;
            } 
            else if (samplesDecoded <= startSample)
            {
               skipSample = startSample % codecInfo.frameSamples;
               outSample = codecInfo.frameSamples - skipSample;
            } 
            else if (totalSamples < (unsigned int)codecInfo.frameSamples) 
            {
               skipSample = 0;
               outSample = totalSamples;
            } 
            else 
            {
               skipSample = 0;
               outSample = codecInfo.frameSamples;
            }

            short decodedPCMData[SCE_AT9_MAX_FRAME_SAMPLES * SCE_AT9_MAX_CHANNELS];
            res = sceAt9DecDecode(at9Handle, pAt9Data, &bytesUsed, &decodedPCMData[0], skipSample, outSample);
            CheckAt9Error(at9Handle, res);

            CStreamWriter_Vita_AT9::AudioFrame newFrame;

            newFrame.mAt9BytePos = at9BytePos;
            newFrame.mAt9NumBytes = bytesUsed;
            newFrame.mPcmBytePos = pcmBytePos;
            newFrame.mPcmNumBytes = codecInfo.frameSamples * sizeof(short) * at9Header.numChannels;

            float denom = (float)at9Header.freq;
            newFrame.mPacketStartTime = (float)samplesDecoded / denom;
            newFrame.mPacketEndTime = (float)(samplesDecoded + codecInfo.frameSamples) / denom;

            pAudioTrack->mAudioFrames.push_back(newFrame);
            pcmBytePos += codecInfo.frameSamples * sizeof(short) * at9Header.numChannels;

            at9BytePos += bytesUsed;
            pAt9Data += bytesUsed;
            samplesDecoded += codecInfo.frameSamples;
            totalSamples -= outSample;
         }
      }

      sceAt9ReleaseHandle(at9Handle);
   }

   //----------------------------------------------------------------------------
   static void MergeAudioFrames(CStreamWriter_Vita_AT9::AudioTrack* pAudioTrack, int numFramesToMerge)
   {
      std::vector<CStreamWriter_Vita_AT9::AudioFrame> newAudioFrames;
      std::vector<CStreamWriter_Vita_AT9::AudioFrame> oldAudioFrames = pAudioTrack->mAudioFrames;

      int startFrameIndex = 0;
      int endFrameIndex = 0;
      int frameIndex = 0;
      int totalFrames = oldAudioFrames.size();

      while (frameIndex < totalFrames)
      {
         startFrameIndex = frameIndex;
         endFrameIndex = frameIndex;
         int count = 0;

         while (count < numFramesToMerge && frameIndex < totalFrames)
         {
            frameIndex++;
            endFrameIndex++;
            count++;
         }

         const CStreamWriter_Vita_AT9::AudioFrame& startFrame = oldAudioFrames[startFrameIndex];
         const CStreamWriter_Vita_AT9::AudioFrame& endFrame = oldAudioFrames[endFrameIndex - 1];
         CStreamWriter_Vita_AT9::AudioFrame combinedFrame;

         combinedFrame.mAt9BytePos = startFrame.mAt9BytePos;
         combinedFrame.mPcmBytePos = startFrame.mPcmBytePos;
         combinedFrame.mPacketStartTime = startFrame.mPacketStartTime;
         combinedFrame.mPacketEndTime = endFrame.mPacketEndTime;

         for (int i = startFrameIndex; i < endFrameIndex; i++)
         {
            combinedFrame.mPcmNumBytes += oldAudioFrames[i].mPcmNumBytes;
            combinedFrame.mAt9NumBytes += oldAudioFrames[i].mAt9NumBytes;
         }

         newAudioFrames.push_back(combinedFrame);
      }

      // Ok to do this, we are just copying structs..
      pAudioTrack->mAudioFrames = newAudioFrames;
   }

} // End of namespace

//----------------------------------------------------------------------------
CStreamWriter_Vita_AT9::CStreamWriter_Vita_AT9() 
   : mpAudioStream(NULL)
   , mLoopedSector(0)
   , mNumSourceAudioDataBytes(0)
   , mNumSourceDataBytes(0)
   , mIsBGMStream(false)
   , mIsMTAHeaderValid(false)
{
}

//----------------------------------------------------------------------------
bool CStreamWriter_Vita_AT9::ReplacementFileExistsForCurrStream(EPlatform curPlatform) const
{
   return ReplacementFileExistsForCurrStream_Static(curPlatform);
}

//----------------------------------------------------------------------------
bool CStreamWriter_Vita_AT9::ReplacementFileExistsForCurrStream_Static(EPlatform curPlatform)
{
   if (curPlatform != kPlatform_Vita)
      return false;

   std::vector<std::string> at9Files;
   FindReplacementAt9Files(at9Files);

   if (at9Files.empty())
   {
      return false;
   }
   else
   {
      printf("Found %d replacement ATrac9 file(s)\n", at9Files.size());
      return true;
   }
}

//----------------------------------------------------------------------------
void CStreamWriter_Vita_AT9::ModifyCurrentSourcePacket(STREAM_TAG& streamTag, unsigned char * const packetBuf, 
   EPlatform curPlatform)
{
   (void)packetBuf;
   (void)curPlatform;

   mNumSourceDataBytes += streamTag._size;

   if (streamTag._type == STREAM_TYPE_MTA)
   {
      // Need to write out this data structure for BGM streams so we can load it when a BGM type stream is played back.
      if (!mIsMTAHeaderValid)
      {
         int headerSize = streamTag._size - sizeof(streamTag);
         if (headerSize != sizeof(mMTAHeader))
         {
            printf("Error processing BGM MTA header.\n");
            throw false;
         }

         memcpy(&mMTAHeader, packetBuf + sizeof(streamTag), sizeof(mMTAHeader));
         mIsMTAHeaderValid = true;

         // Accumulate the total size of audio data. This is used later when processing BGM loop points.
         mNumSourceAudioDataBytes += streamTag._size - sizeof(STREAM_TAG);
      }
   }
}

//----------------------------------------------------------------------------
unsigned int CStreamWriter_Vita_AT9::GetRebuildPlatformMask() const
{
   return kPlatformMask_Vita;
}

//----------------------------------------------------------------------------
void CStreamWriter_Vita_AT9::WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick)
{
   if (curPlatform != kPlatform_Vita)
      return;

   std::vector<std::string> at9Files;
   FindReplacementAt9Files(at9Files);
   if (at9Files.empty())
      return;

   // Check for BGM stream processing. For BGM streams we need extra meta-data in addition to the replacement atrac9 
   // data so do special processing.
   if (at9Files[0].find("us\\bgm") != std::string::npos || at9Files[0].find("us/bgm") != std::string::npos)
      mIsBGMStream = true;
   else
      mIsBGMStream = false;

   // Wait until we have a valid MTA header before writing out audio packets if this is a bgm stream replacement.
   if (mIsBGMStream && !mIsMTAHeaderValid)
      return;

   STREAM_TAG streamTag;
   if (!mpAudioStream)
   {
      mpAudioStream = new AudioStream();

      for (unsigned int i = 0; i < at9Files.size(); i++)
      {
         FILE* fp = NULL;
         fopen_s(&fp, at9Files[i].c_str(), "rb");
         if (!fp)
         {
            printf("Error opening input file %s.\n", at9Files[i].c_str());
            throw false;
         }

         AudioTrack* pNewTrack = new AudioTrack();
         mpAudioStream->mTracks.push_back(pNewTrack);

         fseek(fp, 0, SEEK_END);
         pNewTrack->mBufferSizeInBytes = ftell(fp);
         fseek(fp, 0, SEEK_SET);

         pNewTrack->mpData = new unsigned char[pNewTrack->mBufferSizeInBytes];
         unsigned int bytesRead = fread(&pNewTrack->mpData[0], 1, pNewTrack->mBufferSizeInBytes, fp);
         if (bytesRead != pNewTrack->mBufferSizeInBytes)
         {
            printf("Error reading source file: %s\n", at9Files[0].c_str());
            fclose(fp);
            throw false;
         }

         fclose(fp);

         // Parse the input audio file to determine uncompressed frame sizes and header information. 
         // This is used to ensure that packets of compressed data will match the stream time when uncompressed.
         GetAudioFrameInfo(pNewTrack);

         // Once the individual frames are created, merge them into larger chunks so we minimize the number of small
         // packets written to the stream. (Each packet has an alignment restriction so more packets mean potentially more
         // wasted space due to alignment requirements).
         if (mIsBGMStream)
            MergeAudioFrames(pNewTrack, 32);
         else
            MergeAudioFrames(pNewTrack, 256);

         // Set the stream's audio header using the first track. Tracks should always have the same header information 
         // so verify this if it's not the first track.
         if (i == 0)
         {
            GetAudioHeaderInfo(pNewTrack, &mpAudioStream->mStreamHeader);
         }
         else
         {
            At9AudioHeader audioHeader;
            GetAudioHeaderInfo(pNewTrack, &audioHeader);

            if (!CompareAudioHeaders(&audioHeader, &mpAudioStream->mStreamHeader))
            {
               printf("Audio formats for each track must be the same.");
               throw false;
            }
         }
      }

      // Need to know the total size of the packets put into the stream including padding for the game system to play 
      // back the stream correctly.
      CalculateTotalStreamSizeWithPadding(mpAudioStream);

      // Number of tracks is in the upper 16 bits of the channels field.
      mpAudioStream->mStreamHeader.mChannels |= ((mpAudioStream->mTracks.size() << 16) & 0xffff0000);
      mpAudioStream->mTotalAudioPackets = mpAudioStream->mTracks[0]->mAudioFrames.size();

      // Sanity check...
      for (unsigned int i = 1; i < mpAudioStream->mTracks.size(); i++)
      {
         if (mpAudioStream->mTracks[i]->mAudioFrames.size() != mpAudioStream->mTotalAudioPackets)
         {
            printf("Each audio track must have the same number of audio frames.");
            throw false;
         }
      }

      //Write a stream driver startup packet.
      streamTag._type = CHUNK_TYPE_SYSTEM;
      streamTag._size = sizeof(streamTag);
      streamTag._time = 0;
      streamTag._option = STREAM_TYPE_VITA_AT9;
      fwrite(&streamTag, sizeof(streamTag), 1, streamfp);

      // If we are working with BGM streams align the header packet to the streams sector size.

      char paddingBytes[2048] = { 0 };
      int sectorPadding = 0;
      if (mIsBGMStream)
      {
         int numHeaderBytes = sizeof(streamTag) + sizeof(At9AudioHeader) + sizeof(mMTAHeader);

         // Take into account first system tag. Note, the data spans two sectors so calculate padding using 2 times the 
         // sector size.
         sectorPadding = (2*kGameDiskSectorSize) - numHeaderBytes - sizeof(streamTag); 
         streamTag._type = STREAM_TYPE_VITA_AT9;
         streamTag._size = numHeaderBytes + sectorPadding;
         streamTag._time = 0;
         streamTag._option = 0;
      }
      else
      {
         streamTag._type = STREAM_TYPE_VITA_AT9;
         streamTag._size = sizeof(streamTag) + sizeof(At9AudioHeader);
         streamTag._time = 0;
         streamTag._option = 0;
      }

      fwrite(&streamTag, sizeof(streamTag), 1, streamfp);
      fwrite(&mpAudioStream->mStreamHeader, sizeof(At9AudioHeader), 1, streamfp);
      if (mIsBGMStream)
      {
         if (!mIsMTAHeaderValid)
         {
            printf("Trying to write an MTA header that is invalid.\n");
            throw false;
         }

         // Write the mta header first so the game can grab it.
         fwrite(&mMTAHeader, sizeof(mMTAHeader), 1, streamfp);
      }

      if (sectorPadding)
         fwrite(&paddingBytes[0], 1, sectorPadding, streamfp);
   }

   WriteSoundPacket(streamfp, currTick, false);
}

//----------------------------------------------------------------------------
void CStreamWriter_Vita_AT9::WriteSoundPacket(FILE * const streamfp, const int currTick, bool forceWrite)
{   
   const int kMaxAudioLeadTicks = STREAM_TICKS_PER_SECOND;
   const float desiredAudioTime = float(currTick + kMaxAudioLeadTicks) / STREAM_TICKS_PER_SECOND;

   // BGM streams do not have a time and are not interleaved with any other data. Wait and write them all out when 
   // the stream has ended so we can capture any possible looped frames.
   if (!forceWrite && mIsBGMStream)
      return;

   if (mpAudioStream->mCurrAudioPacket >= mpAudioStream->mTotalAudioPackets)
      return;

   int const kBuffSize = 128 * 1024;
   std::vector<unsigned char> byteBuffer(kBuffSize);
   unsigned int byteBufferPos = 0;
   bool isLoopedPacket = false;
   
   for (unsigned int i = 0; i < mpAudioStream->mTracks.size(); i++)
   {
      AudioTrack* const pCurrTrack = mpAudioStream->mTracks[i];

      AudioFrame* const pAudioFrame = &pCurrTrack->mAudioFrames[mpAudioStream->mCurrAudioPacket];
      if (pAudioFrame->mPacketStartTime >= desiredAudioTime && !forceWrite)
         return;

      if (byteBufferPos >= kBuffSize)
         throw false;

      if (pAudioFrame->mIsLoopFrame)
         isLoopedPacket = true;

      memcpy(&byteBuffer[byteBufferPos], &pCurrTrack->mpAudioDataStart[pAudioFrame->mAt9BytePos], 
         pAudioFrame->mAt9NumBytes);
      byteBufferPos += pAudioFrame->mAt9NumBytes;
   }
   

   STREAM_TAG streamTag;

   unsigned char padding[kGameDiskSectorSize];
   memset(&padding[0], 0, kGameDiskSectorSize);

   long paddingBytes = 0;

   // If this packet is marked as the start of a loop, we must ensure it starts on a disk sector aligned boundary.
   if (isLoopedPacket)
   {
      long filePos = ftell(streamfp);
      if ((filePos % kGameDiskSectorSize) != 0)
      {
         filePos += sizeof(streamTag);
         paddingBytes = ( (filePos + (kGameDiskSectorSize - 1)) & ~(kGameDiskSectorSize - 1) ) - filePos;

         streamTag._type = STREAM_TYPE_VITA_AT9;
         streamTag._size = sizeof(streamTag) + paddingBytes;
         streamTag._time = 0;
         streamTag._option = paddingBytes;

         fwrite(&streamTag, sizeof(streamTag), 1, streamfp);

         unsigned int magicId = 0xdeadbeef;

         if (paddingBytes < sizeof(unsigned int))
            throw false;

         fwrite(&magicId, sizeof(unsigned int), 1, streamfp);
         if ((paddingBytes - sizeof(unsigned int)) > 0)
            fwrite(&padding[0], 1, paddingBytes - sizeof(unsigned int), streamfp);
      }

      filePos = ftell(streamfp);
      if ((filePos % kGameDiskSectorSize) != 0)
         throw false;

      mLoopedSector = filePos / kGameDiskSectorSize;
   }

   paddingBytes = ((byteBufferPos + 15) & ~15) - byteBufferPos;

   streamTag._type = STREAM_TYPE_VITA_AT9;
   streamTag._size = byteBufferPos + paddingBytes + sizeof(streamTag);
   streamTag._time = currTick;
   streamTag._option = byteBufferPos;

   fwrite(&streamTag, sizeof(streamTag), 1, streamfp);
   fwrite(&byteBuffer[0], byteBufferPos, 1, streamfp);
   fwrite(&padding[0], paddingBytes, 1, streamfp);

   mpAudioStream->mCurrAudioPacket++;
}

//----------------------------------------------------------------------------
void CStreamWriter_Vita_AT9::EndStream(FILE * const streamfp, EPlatform curPlatform, STREAM_TAG* pEndTag)
{
   (void)curPlatform;
   (void)streamfp;

   if (!mpAudioStream)
      return;

   // For BGM looping streams we need to modify the sector loop offset to match our new data. The old sector offset is 
   // invalid since we have rebuilt the data.
  
   // If the option field is non-zero, this means the bgm stream should loop. We need to get the old loop offset
   // which is in sector units, convert to audio samples in order to get the loop time. Note, we also need to write
   // all BGM packets here instead of relying on EndStream to be called to make sure we calculate the correct 
   // sector offset.
   if (pEndTag->_option > 0)
   {
      int offsetInBytes = pEndTag->_option * kGameDiskSectorSize;
      int numTracks = mpAudioStream->mTracks.size();

      float streamOffsetPercentage = offsetInBytes / (float)mNumSourceDataBytes;
      float bgmAudioDuration = (mNumSourceAudioDataBytes / 272 * 256) / (48000.0f * numTracks);
      float offsetTime = bgmAudioDuration * streamOffsetPercentage;

      printf("Found looping stream. Start of loop is %.4f seconds.\n", offsetTime);

      // Using the first track, find the first audio frame within the specified offset time.
      unsigned int numAudioFrames = mpAudioStream->mTracks[0]->mAudioFrames.size();
      for (unsigned int ui = 0; ui < numAudioFrames; ui++)
      {
         AudioFrame& currFrame = mpAudioStream->mTracks[0]->mAudioFrames[ui];
         if (offsetTime >= currFrame.mPacketStartTime && offsetTime < currFrame.mPacketEndTime)
         {
            currFrame.mIsLoopFrame = true;
            break;
         }
      }
   }

   int numRemaining = mpAudioStream->mTotalAudioPackets - mpAudioStream->mCurrAudioPacket;
   for (int i = 0; i < numRemaining; i++)
   {
      WriteSoundPacket(streamfp, 0, true);
   }

   // If we had a looped sector then set it on the end tag before it's written.
   if (pEndTag->_option > 0)
   {
      if (!mLoopedSector)
      {
         printf("Should have had a looped sector value here!\n");
         throw false;
      }

      pEndTag->_option = mLoopedSector;
   }

   delete mpAudioStream;
   mpAudioStream = NULL;
   mNumSourceAudioDataBytes = 0;
   mNumSourceDataBytes = 0;
   mLoopedSector = 0;
   mIsBGMStream = false;
   mIsMTAHeaderValid = false;
   memset(&mMTAHeader, 0, sizeof(mMTAHeader));
}

//----------------------------------------------------------------------------
bool CStreamWriter_Vita_AT9::ReplacesDataForCurrStream(const unsigned int type)
{
   if (type == STREAM_TYPE_STV || type == STREAM_TYPE_MTA || type == STREAM_TYPE_VAG || type == CHUNK_TYPE_PCM)
   {
      return mpAudioStream != NULL || mIsBGMStream;
   }
   else
   {
      return false;
   }
}
