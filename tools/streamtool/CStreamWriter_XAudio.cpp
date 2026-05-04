#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "streamtool.h"
#include "CStreamWriter_XAudio.h"

#include <windows.h>

#ifndef WAVE_FORMAT_WMAUDIO2 
#define WAVE_FORMAT_WMAUDIO2        0x0161 // Windows Media Audio
#define WAVE_FORMAT_WMAUDIO3        0x0162 // Windows Media Audio
#endif

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

const float gTimeStepAdjust = 0.5f;

// Note, these are little endian format...
const DWORD ATG_FOURCC_RIFF = 'FFIR';
const DWORD ATG_FOURCC_WAVE = 'EVAW';
const DWORD ATG_FOURCC_XWMA = 'AMWX';
const DWORD ATG_FOURCC_DPDS = 'sdpd';
const DWORD ATG_FOURCC_FORMAT = ' tmf';
const DWORD ATG_FOURCC_DATA = 'atad';

//#define BP_DECODE_TO_DETERMINE_TIME 1

//----------------------------------------------------------------------------
namespace 
{
   static void FindReplacementXWMFiles(std::vector<std::string>& filePaths)
   {
      char acPath[FILENAME_MAX];
      sprintf(acPath, "%s/%s", gOutputFolder, gCurrStreamName);

      System::String^ srcPath = gcnew System::String(acPath);
      array<System::String^>^ files = System::IO::Directory::GetFiles(srcPath, "*.xwm", System::IO::SearchOption::AllDirectories);

      for (int i = 0; i < files->Length; i++)
         filePaths.push_back(BP_ConvertManagedString(files[i]));

      //If there's a xwm file in a BP override folder, choose just that one.
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

   static void ByteSwap_Int_Always(int *n)
   {
      unsigned int u = (unsigned int)*n;
      *n = (u >> 24) | ((u >> 8) & 0xff00) | ((u & 0xff00) << 8) | (u << 24);
   }

   static void GetAudioFrameInfo(const char* pFilePath, CStreamWriter_XAudio::AudioTrack* pAudioTrack)
   {
      AVFormatContext* pFormatCtx;

      av_register_all();
      
      int res = av_open_input_file(&pFormatCtx, pFilePath, NULL, 0, NULL);
      if (res)
      {
         printf("libav: Unable to open input file %s\n", pFilePath);
         throw false;
      }

      av_dump_format(pFormatCtx, 0, pFilePath, 0);

      if (pFormatCtx->nb_streams != 1)
      {
         printf("libav: Input file should only consist of an audio stream.\n");
         throw false;
      }

      AVStream* pAudioStream = pFormatCtx->streams[0];
      AVCodecContext* pCodecCtx = pAudioStream->codec;

#if BP_DECODE_TO_DETERMINE_TIME
      AVCodec* pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
      if (!pCodec)
      {
         printf("libav: Unable to find codec decoder for xWMA audio stream.\n");
         throw false;
      }

      if (avcodec_open(pCodecCtx, pCodec) < 0)
      {
         printf("Could not open audio codec.\n");
         throw false;
      }
#endif

      pAudioTrack->audioHeader.mCookie = 'XWMA';

      // Set format
      {
         switch(pCodecCtx->codec_tag)
         {
         case WAVE_FORMAT_WMAUDIO2:
         case WAVE_FORMAT_WMAUDIO3:
            break;
         
         default:
            throw false;
         }

         pAudioTrack->audioHeader.mFormat = pCodecCtx->codec_tag;
      }

      pAudioTrack->audioHeader.mChannels = pCodecCtx->channels;
      pAudioTrack->audioHeader.mFrequency = pCodecCtx->sample_rate;
      pAudioTrack->audioHeader.mAvgBytesPerSec = pCodecCtx->bit_rate / 8;
      pAudioTrack->audioHeader.mBlockAlign = pCodecCtx->block_align;

#if BP_DECODE_TO_DETERMINE_TIME
      AVPacket audioPacket;
      const int kMaxBufSize = 2 * AVCODEC_MAX_AUDIO_FRAME_SIZE;
      int16_t* pDecodeBuffer = new int16_t[kMaxBufSize];   
      unsigned int oldPCMBytePos = 0;

      while (true)
      {
         av_init_packet(&audioPacket);
         if (av_read_frame(pFormatCtx, &audioPacket) < 0)
            break; // end of audio stream.

         CStreamWriter_XAudio::AudioFrame newFrame;
         
         newFrame.xwmBytePos = (unsigned int)audioPacket.pos;
         newFrame.xwmLen = audioPacket.size;

         // Decode the packet and add an entry to our packet list to contain encoded data size and decoded data size.
         // prepare a buffer to store the decoded samples in          
         int bufSize = kMaxBufSize;
         int bytesDecoded = avcodec_decode_audio3(pCodecCtx, &pDecodeBuffer[0], &bufSize, &audioPacket);
         if (bytesDecoded < 0)
         {
            printf("Error decoding audio stream %s\n", pFilePath);
            throw false;
         }

         newFrame.pcmBytePos = oldPCMBytePos;
         newFrame.pcmLen = bufSize;

         //printf("FrameStats[%d]: xwmPos: %d xwmLen: %d pcmPos: %d pcmLen: %d\n", pAudioTrack->audioFrames.size(), newFrame.xwmBytePos, newFrame.xwmLen, newFrame.pcmBytePos, newFrame.pcmLen);

         unsigned int bytesPerSample = pCodecCtx->bits_per_coded_sample / 8;
         unsigned int numSamplesStart = newFrame.pcmBytePos / bytesPerSample;
         unsigned int numSamplesEnd = (newFrame.pcmBytePos + newFrame.pcmLen) / bytesPerSample;
         newFrame.packetStartTime = (float)numSamplesStart / ((float)(pCodecCtx->sample_rate * pCodecCtx->channels));
         newFrame.packetEndTime = (float)numSamplesEnd / ((float)(pCodecCtx->sample_rate * pCodecCtx->channels));
         pAudioTrack->audioFrames.push_back(newFrame);
         oldPCMBytePos += bufSize;

         av_free_packet(&audioPacket);
      }

      delete [] pDecodeBuffer;
      avcodec_close(pCodecCtx);
#else
      {
         unsigned char* pCurrentTag = pAudioTrack->pBuffer;
         unsigned char* pBufferEnd = pAudioTrack->pBuffer + pAudioTrack->bufferSize;

         unsigned char* dataChunkStart = NULL;

         unsigned int* dpdsTable = NULL;
         int dpdsPacketCount = 0;

         while(pCurrentTag < pBufferEnd)
         {
            char fourCCString[5] = { pCurrentTag[0], pCurrentTag[1], pCurrentTag[2], pCurrentTag[3], 0 };

            unsigned int chunkFourCC = ((unsigned int*)pCurrentTag)[0];
            unsigned int chunkSize = ((unsigned int*)pCurrentTag)[1];

            pCurrentTag += 8;

            switch(chunkFourCC)
            {
            case ATG_FOURCC_RIFF:
               break;

            case ATG_FOURCC_XWMA:
               // XWMA has no size field
               pCurrentTag -= 4;
               // Step into chunk
               break;

            case ATG_FOURCC_FORMAT:
               pCurrentTag += chunkSize;
               break;

            case ATG_FOURCC_DPDS:
               {
                  dpdsTable = (unsigned int*)pCurrentTag;
                  dpdsPacketCount = chunkSize / 4;

                  pCurrentTag += chunkSize;
               }
               break;

            case ATG_FOURCC_DATA:
               {
                  dataChunkStart = pCurrentTag;
                  // terminate parsing of file, we have all the information we need!
                  pCurrentTag = pBufferEnd;
               }
               break;

            default:
               pCurrentTag += chunkSize;
               break;
            }
         }

         if( dpdsTable )
         {
            unsigned int lastPCMBytePos = 0;

            // We don't start at the beginning of the file, our XWM byte pos starts at the beginning of the data chunk!
            unsigned int lastXWMBytePos = dataChunkStart - pAudioTrack->pBuffer;

            for( int i = 0; i < dpdsPacketCount; ++i )
            {
               CStreamWriter_XAudio::AudioFrame newFrame;

               newFrame.xwmBytePos = lastXWMBytePos;
               newFrame.xwmLen = pAudioTrack->audioHeader.mBlockAlign;
               
               lastXWMBytePos += newFrame.xwmLen;

               int newPCMBytePos = dpdsTable[i];

               int const pcmLength = newPCMBytePos - lastPCMBytePos;

               newFrame.pcmBytePos = lastPCMBytePos;
               newFrame.pcmLen = pcmLength;

               lastPCMBytePos = newPCMBytePos;

               //printf("FrameStats[%d]: xwmPos: %d xwmLen: %d pcmPos: %d pcmLen: %d\n", i, newFrame.xwmBytePos, newFrame.xwmLen, newFrame.pcmBytePos, newFrame.pcmLen);

               unsigned int bytesPerSample = pCodecCtx->bits_per_coded_sample / 8;
               unsigned int numSamplesStart = newFrame.pcmBytePos / bytesPerSample;
               unsigned int numSamplesEnd = (newFrame.pcmBytePos + newFrame.pcmLen) / bytesPerSample;
               newFrame.packetStartTime = (float)numSamplesStart / ((float)(pCodecCtx->sample_rate * pCodecCtx->channels));
               newFrame.packetEndTime = (float)numSamplesEnd / ((float)(pCodecCtx->sample_rate * pCodecCtx->channels));

               pAudioTrack->audioFrames.push_back(newFrame);
            }
         }
      }
#endif

      av_close_input_file(pFormatCtx);

      // Finally set the number of bytes of xWMA data.
      CStreamWriter_XAudio::AudioFrame lastFrame = pAudioTrack->audioFrames[pAudioTrack->audioFrames.size() - 1];
      pAudioTrack->audioHeader.mSampleSize = lastFrame.xwmBytePos + lastFrame.xwmLen - 
         pAudioTrack->audioFrames[0].xwmBytePos;
   }
}

//----------------------------------------------------------------------------
CStreamWriter_XAudio::CStreamWriter_XAudio() : m_pAudioTrack(NULL)
{
}

//----------------------------------------------------------------------------
bool CStreamWriter_XAudio::ReplacementFileExistsForCurrStream(EPlatform curPlatform) const
{
   return ReplacementFileExistsForCurrStream_Static(curPlatform);
}

//----------------------------------------------------------------------------
bool CStreamWriter_XAudio::ReplacementFileExistsForCurrStream_Static(EPlatform curPlatform)
{
   if (curPlatform != kPlatform_X360)
      return false;

   std::vector<std::string> xwmFiles;
   FindReplacementXWMFiles(xwmFiles);

   if (xwmFiles.empty())
   {
      return false;
   }
   else
   {
      if (xwmFiles.size() > 1)
      {
         printf("Found a stream with multiple tracks. Only one track is supported.\n");
         throw false;
      }

      printf("Found %d replacement XAudio file(s)\n", xwmFiles.size());
      return true;
   }
}

//----------------------------------------------------------------------------
unsigned int CStreamWriter_XAudio::GetRebuildPlatformMask() const
{
   return kPlatformMask_X360;
}

//----------------------------------------------------------------------------
void CStreamWriter_XAudio::WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick)
{
   if (curPlatform != kPlatform_X360)
      return;

   STREAM_TAG streamTag;
   if (!m_pAudioTrack)
   {
      std::vector<std::string> xwmFiles;

      FindReplacementXWMFiles(xwmFiles);
      if (xwmFiles.empty())
         return;

      FILE* fp = NULL;
      fopen_s(&fp, xwmFiles[0].c_str(), "rb");
      if (!fp)
      {
         printf("Error opening input file %s.\n", xwmFiles[0].c_str());
         throw false;
      }

      m_pAudioTrack = new AudioTrack();

      fseek(fp, 0, SEEK_END);
      m_pAudioTrack->bufferSize = ftell(fp);
      fseek(fp, 0, SEEK_SET);

      m_pAudioTrack->pBuffer = new unsigned char[m_pAudioTrack->bufferSize];
      unsigned int bytesRead = fread(&m_pAudioTrack->pBuffer[0], 1, m_pAudioTrack->bufferSize, fp);
      if (bytesRead != m_pAudioTrack->bufferSize)
      {
         printf("Error reading source file: %s\n", xwmFiles[0].c_str());
         fclose(fp);
         throw false;
      }

      fclose(fp);

      // Parse the input audio file to determine uncompressed frame sizes and header information. 
      // This is used to ensure that packets of compressed data will match the stream time when uncompressed.
      GetAudioFrameInfo(xwmFiles[0].c_str(), m_pAudioTrack);

      //Write a stream driver startup packet.
      streamTag._type = CHUNK_TYPE_SYSTEM;
      streamTag._size = sizeof(streamTag);
      streamTag._time = 0;
      streamTag._option = STREAM_TYPE_XAUDIO;
      fwrite(&streamTag, sizeof(streamTag), 1, streamfp);

      streamTag._type = STREAM_TYPE_XAUDIO;
      streamTag._size = sizeof(streamTag) + sizeof(XAudioStreamHeader);
      streamTag._time = 0;
      streamTag._option = 0;
      
      // Byte swap header values so they are in big endian.
      XAudioStreamHeader bigEndianHeader = m_pAudioTrack->audioHeader;
      ByteSwap_Int_Always((int*)&bigEndianHeader.mCookie);
      ByteSwap_Int_Always(&bigEndianHeader.mFormat);
      ByteSwap_Int_Always(&bigEndianHeader.mChannels);
      ByteSwap_Int_Always(&bigEndianHeader.mFrequency);
      ByteSwap_Int_Always(&bigEndianHeader.mSampleSize);
      ByteSwap_Int_Always(&bigEndianHeader.mAvgBytesPerSec);
      ByteSwap_Int_Always(&bigEndianHeader.mBlockAlign);
      bigEndianHeader.mUnused = 0;

      fwrite(&streamTag, sizeof(streamTag), 1, streamfp);
      fwrite(&bigEndianHeader, sizeof(XAudioStreamHeader), 1, streamfp);
   }

   if (m_pAudioTrack->HasRemainingPackets())
   {
      const int kMaxAudioLeadTicks = STREAM_TICKS_PER_SECOND / 2; //allow audio to write packets 1/2 second ahead of current tick.
      const float desiredAudioTime = float( currTick + kMaxAudioLeadTicks ) / STREAM_TICKS_PER_SECOND;

      //float streamTime = (float)currTick / (float)STREAM_TICKS_PER_SECOND;
      AudioFrame& audioFrame = m_pAudioTrack->audioFrames[m_pAudioTrack->currAudioPacket];

      if (audioFrame.packetStartTime < desiredAudioTime)
      {
         WriteSoundPacket(streamfp, currTick);
         m_pAudioTrack->currAudioPacket++;
      }
   }
}

//----------------------------------------------------------------------------
void CStreamWriter_XAudio::WriteSoundPacket(FILE * const streamfp, const int currTick)
{   
   if (m_pAudioTrack->currAudioPacket >= m_pAudioTrack->audioFrames.size())
   {
      printf("Overflowed input audio buffer!\n");
      throw false;
   }

   AudioFrame& audioFrame = m_pAudioTrack->audioFrames[m_pAudioTrack->currAudioPacket];
   
   unsigned int bytesToWrite = audioFrame.xwmLen;
   unsigned int paddingBytes = ((bytesToWrite + 15) & ~15) - bytesToWrite;
   STREAM_TAG streamTag;

   streamTag._type = STREAM_TYPE_XAUDIO;
   streamTag._size = bytesToWrite + paddingBytes + sizeof(streamTag);
   streamTag._time = currTick;
   streamTag._option = bytesToWrite;

   fwrite(&streamTag, sizeof(streamTag), 1, streamfp);

   unsigned char* pXWMBuf = &m_pAudioTrack->pBuffer[audioFrame.xwmBytePos];
   fwrite(pXWMBuf, audioFrame.xwmLen, 1, streamfp);
  
   char c = 0;
   for (unsigned int i = 0; i < paddingBytes; i++)
      fwrite(&c, 1, 1, streamfp);
}

//----------------------------------------------------------------------------
void CStreamWriter_XAudio::EndStream(FILE * const streamfp, EPlatform curPlatform, STREAM_TAG* pEndTag)
{
   (void)curPlatform;
   (void)streamfp;
   (void)pEndTag;

   if (!m_pAudioTrack)
      return;

   for (int i = m_pAudioTrack->currAudioPacket; i < m_pAudioTrack->audioFrames.size(); i++)
   {  
     WriteSoundPacket(streamfp, 0);
     m_pAudioTrack->currAudioPacket++;
   }

   delete m_pAudioTrack;
   m_pAudioTrack = NULL;
}

//----------------------------------------------------------------------------
bool CStreamWriter_XAudio::ReplacesDataForCurrStream(const unsigned int type)
{
   if (type == STREAM_TYPE_STV || type == STREAM_TYPE_MTA || type == STREAM_TYPE_VAG || type == CHUNK_TYPE_PCM)
   {
      return m_pAudioTrack != NULL;
   }
   else
   {
      return false;
   }
}
