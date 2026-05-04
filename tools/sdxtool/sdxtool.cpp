//----------------------------------------------------------------------------
// sdxtool.cpp
// various operations on mgs2 sound pak (.sdx) files
//----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

#include <list>
#include <map>
#include <string>
#include <algorithm>

#include "DataUnifier.h"

#pragma warning(disable:4996)

#define	VAG_LOOPSTART	(0x04)
#define	VAG_LOOPBLOCK	(0x02)
#define	VAG_LOOPEND		(0x01)

#define  SECTOR_SIZE    (2048)
//----------------------------------------------------------------------------

#if MGS_VERSION==2
struct PAK_HEADER {
   unsigned int		offset;
   unsigned int		code;
};
struct PAK_HEADER	pak_header[0x100];

struct WAVE_W {
   unsigned long	addr_le;
   char			sample_note;
   char			sample_tune;
   unsigned char	a_mode;
   unsigned char	ar;
   unsigned char	dr;
   unsigned char	s_mode;
   unsigned char	sr;
   unsigned char	sl;
   unsigned char	r_mode;
   unsigned char	rr;
   unsigned char	pan;
   unsigned char	dec_vol;
};
#endif

//----------------------------------------------------------------------------

#if MGS_VERSION==3

typedef struct {
	int		fsize;
	int		szPvg;
	short	loadDir;
	short	loadPBank;
	short	pad[22];
} PSQ_IPSQ;
typedef struct
{
	unsigned int		adsr;
	unsigned int		addr;	// d31:memory stream / d23-d00:spu addr
	unsigned short		frq;
	unsigned short		size;
	char				pad[4];
} PSQ_IWAV;
typedef struct
{
	short				wav;
	signed char			vol;
	signed char			pan;
	signed char			center;
	signed char			fine;
	signed char			noteL;
	signed char			noteH;
	signed char			pad[8];
} PSQ_IPAT;
typedef struct
{
   union
   {
      struct
      {
         unsigned			no		: 9;
         unsigned			bank	: 7;
         unsigned			nVc		: 9;
         unsigned			prog	: 7;
      }
      bits;
      unsigned bits_uint;
   };
} PSQ_TPAT;
typedef struct
{
	signed char			grp;
	signed char			pri;
	signed char			vol;
	signed char			flg;
	short				vgrp;
	signed char			seqMax;
	signed char			crv;
	char				pad[4];

   union
   {
      struct
      {
         unsigned			fPause			: 1;
         unsigned			fSys			: 1;
         unsigned			fRecv			: 1;
         unsigned			fDisVol			: 1;
         unsigned			fDisPan			: 1;
         unsigned			itr				: 7;
         unsigned			rndVol			: 4;
         unsigned			fRotate			: 1;
         unsigned			noVolDown		: 3;
         unsigned			bpadA			:12;
      }
      bits;
      unsigned bits_uint;
   };
} PSQ_ISEQ;
typedef struct
{
   union
   {
      struct
      {
         unsigned			addr	:24;
         unsigned			nLink	: 3;
         unsigned			fRnd	: 1;
         unsigned			bpadA	: 4;
      }
      bits;
      unsigned bits_uint;
   };
} PSQ_TSEQ;

#define getInt( p ) ( (int)(p)[0] + (int)(p)[1] * 0x100 + (int)(p)[2] * 0x10000 + (int)(p)[3] * 0x1000000 )

#endif

//----------------------------------------------------------------------------

enum EUsageMode
{
   kUsageMode_None,
   kUsageMode_ExtractWavs,
   kUsageMode_HeaderInfo,
   kUsageMode_DuplicateReport,
   kUsageMode_ZeroGarbagePadding,
   kUsageMode_UnifyWavs,
   kUsageMode_DumpUnifiedWavArchive,
};

struct SCommandLineArgs
{
   EUsageMode     mUsageMode;
   const char *   mInputFilename;
   const char *   mExtractTypes;
   bool           mbPrintAllWavDupeSourceLabels;
};

struct SWavDupeInfo
{
   SWavDupeInfo() : mSrcLabelList(), mSize( 0 ), mCount( 0 ) {}

   std::list< std::string >   mSrcLabelList;

   int            mSize;
   int            mCount;
};

typedef std::map< std::string, SWavDupeInfo > TWavDupeMap;

//----------------------------------------------------------------------------

SCommandLineArgs     gCommandLineArgs;
char                 gCurrOutputFolder[FILENAME_MAX];
char                 gCurrInputFilename[FILENAME_MAX];
TWavDupeMap          gWavDupeMap;

static void AddWavToDupeMap( const std::string & srcLabel, const unsigned char * waveBuf, const int wavSize )
{
   if( wavSize % 16 )
   {
      printf("Error: not a multiple of vag packet size!\n");
      throw false;
   }
   const std::string wavHash( sdx::ComputeMD5Hash( waveBuf, wavSize ) );
   SWavDupeInfo & mapEntry = gWavDupeMap[wavHash];
   if( mapEntry.mCount == 0 )
   {
      //First entry for this hash.
      mapEntry.mSize = wavSize;
   }
   else
   {
      if( mapEntry.mSize != wavSize )
      {
         printf("Error: two wavs with same hash but diff. sizes?\n");
         throw false;
      }
   }
   ++mapEntry.mCount;
   mapEntry.mSrcLabelList.push_back( srcLabel );
}

//----------------------------------------------------------------------------

struct SIffChunkHeader
{
   char  mId[4];
   int   mSize;
};

struct SRIffChunk
{
   SIffChunkHeader mHeader;
   char  mWavHeader[4];
};

struct SFmtChunk
{
   SIffChunkHeader   mHeader;
   short             mFormatTag;
   unsigned short    mChannelCount;
   unsigned int      mFrequency;
   unsigned int      mAvgBytesPerSec;
   unsigned short    mBlockAlign;
   unsigned short    mBitsPerSample;
};

void SaveLPCM16WavFile( const char * const filename, const unsigned int channelCount, const unsigned int frequency, const unsigned int dataSize, const unsigned char * const data )
{
   FILE * fp = fopen( filename, "wb" );
   if( !fp )
   {
      printf("Error opening %s for writing!\n", filename );
      throw false;
   }

   printf("Extracting %s.\n", filename );
   const int bytesPerSample = sizeof(short);

   SRIffChunk riffChunk = { { {'R','I','F','F' }, dataSize + 36 }, {'W','A','V','E'} };
   fwrite( &riffChunk, sizeof( riffChunk ), 1, fp );

   SFmtChunk fmtChunk = { { {'f','m','t',' '}, sizeof(SFmtChunk)-sizeof(SIffChunkHeader) }, 1, (unsigned short)channelCount, frequency, frequency * channelCount * bytesPerSample, (unsigned short)(channelCount * bytesPerSample), (unsigned short)(bytesPerSample*8) };
   fwrite( &fmtChunk, sizeof( fmtChunk ), 1, fp );

   SIffChunkHeader dataChunk = { {'d','a','t','a'}, dataSize };
   fwrite( &dataChunk, sizeof( dataChunk ), 1, fp );

   fwrite( data, dataSize, 1, fp );

   fclose( fp );
}

//----------------------------------------------------------------------------

static const double f[5][2] = { { 0.0, 0.0 },
{  60.0 / 64.0,  0.0 },
{  115.0 / 64.0, -52.0 / 64.0 },
{  98.0 / 64.0, -55.0 / 64.0 },
{  122.0 / 64.0, -60.0 / 64.0 } };

static int DecodeADPCM(const signed char* inputData,int inputDataSize,char* outputData,int nrChannels, int interleave)
{
   int blockCount = inputDataSize / 16;   //block = vag packet
   if( blockCount * 16 != inputDataSize )
   {
      printf("DecodeADPCM: unaligned source data size: %d!\n", inputDataSize );
      throw false;
   }

   int blockInterleave = interleave / 16;
   if( blockInterleave * 16 != interleave || interleave == 0 )
   {
      printf("DecodeADPCM: unaligned or zero interleave size: %d (%d)!\n", inputDataSize, interleave );
      throw false;
   }

   int predict_nr, shift_factor;
   int i, block = 0;
   unsigned int d;
   int s;
   struct {
      double s_1;
      double s_2;
   } ctxs[2] = {0}, *ctx = ctxs;

   signed short int * left = (signed short int *)_alloca(sizeof(short)*28*blockInterleave);
   signed short int * right = (signed short int *)_alloca(sizeof(short)*28*blockInterleave);
   signed short int *chan = left;
   double samples[28];
   char* tempOutputData=outputData;


   for (const signed char* data=inputData;data<inputData+inputDataSize && !(data[1] & VAG_LOOPEND) && blockCount>0;data+=16)
   {
      predict_nr = data[0];
      shift_factor = predict_nr & 0xf;
      predict_nr >>= 4;

      for (i = 0; i < 28; i += 2) {
         d = data[i / 2 + 2];
         s = (d & 0xf) << 12;
         if (s & 0x8000)
            s |= 0xffff0000;
         samples[i] = (double) (s >> shift_factor);
         s = (d & 0xf0) << 8;
         if (s & 0x8000)
            s |= 0xffff0000;
         samples[i+1] = (double) (s >> shift_factor);
      }

      for (i = 0; i < 28; i++) {
         int d2;
         samples[i] += ctx->s_1 * f[predict_nr][0] + ctx->s_2 * f[predict_nr][1];
         ctx->s_2 = ctx->s_1;
         ctx->s_1 = samples[i];
         d = (int) (samples[i] + 0.5);
         d2=d;
         if(d2>32767)
            d2=32767;
         if(d2<-32767)
            d2=-32767;
         chan[i + block*28] = (short)d2;

      }
      if(nrChannels==1)
      {
         for (i = 0; i < 28; ++i) {
            *(tempOutputData++)=left[i]&0xff;
            *(tempOutputData++)=left[i] >> 8;
         }

      }
      else
      {
         block++;
         if (block == blockInterleave) 
         {
            if (chan == left)
            {
               chan = right;
               ctx = &ctxs[1];
            }
            else 
            {
               chan = left;
               ctx = &ctxs[0];

               for (i = 0; i < 28*blockInterleave; ++i) {
                  *(tempOutputData++)=left[i]&0xff;
                  *(tempOutputData++)=left[i] >> 8;
                  *(tempOutputData++)=right[i]&0xff;
                  *(tempOutputData++)=right[i] >> 8;
               }
               blockCount--;
            }
            block = 0;
         }
      }
   }
   if(block!=0)
   {
      printf("DecodeADPCM: ADPCM decoding ended in the middle of a block - interleaved data is wrong");
      throw false;
   }
   return tempOutputData-outputData;
}

//----------------------------------------------------------------------------

void usage()
{
   fprintf(stderr, "sdxtool [-x[a|w]|i|d|z] [/a] [<.sdx file>|<@.sdx file list>]\n");
   fprintf(stderr, "-x[a|w]: extract .adpcm and / or .wav files (use -xaw for both)\n");
   fprintf(stderr, "-u: Unify all wav data into a single sdx archive removing any duplicates.\n");
   fprintf(stderr, "-i: print header info of sdx files\n");
   fprintf(stderr, "-d: print report on duplicated waveform data across sdx files\n");
   fprintf(stderr, "-z: zero out garbage padding at end of MGS3 sdx files\n");
   fprintf(stderr, "/a: include all wav source labels in dupe report\n");
   fprintf(stderr, "--dump: dump unified wave table from a unified archive\n");
}

static bool parse_command_line( int argc, char const *argv[] )
{
   gCommandLineArgs.mUsageMode = kUsageMode_None;
   gCommandLineArgs.mInputFilename = NULL;
   gCommandLineArgs.mbPrintAllWavDupeSourceLabels = false;

   for( int i=1; i < argc; ++i )
   {
      const char * arg = argv[i];
      if ( !strncmp( arg, "-x", 2 ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_ExtractWavs;
         gCommandLineArgs.mExtractTypes = arg+2;
      }
      else if ( !strcmp( arg, "-i" ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_HeaderInfo;
      }
      else if ( !strcmp( arg, "-d" ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_DuplicateReport;
      }
      else if ( !strcmp( arg, "-z" ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_ZeroGarbagePadding;
      }
      else if ( !strcmp( arg, "-u" ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_UnifyWavs;
      }
      else if ( !strcmp( arg, "/a" ) )
      {
         gCommandLineArgs.mbPrintAllWavDupeSourceLabels = true;
      }
      else if ( !strcmp( arg, "--dump" ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_DumpUnifiedWavArchive;
      }
      else if ( !gCommandLineArgs.mInputFilename )
      {
         gCommandLineArgs.mInputFilename = arg;
      }
      else
      {
         printf("Unknown command line argument: %s\n", arg );
         return false;
      }
   }

   return gCommandLineArgs.mUsageMode != kUsageMode_None && gCommandLineArgs.mInputFilename != NULL;
}

void set_voice_tbl(struct WAVE_W *p, unsigned int size, unsigned int offset)
{
   unsigned int	i;
   int				drum_on=0;
   signed int		addr_new, addr_old=-1;

   for (i = (offset/0x10); i < (size/0x10); i++) {
      
      addr_new = (signed int)( ( p[i].addr_le ) );
      if (drum_on == 0) {
         if (addr_new < (addr_old)) {
            drum_on = 1;
            break;
         }
         else addr_old = addr_new;
      }
   }
   //memcpy( (char *)((u_int)voice_tbl+offset), p, i*0x10 );
   //if(drum_on) memcpy( drum_tbl, &p[i], size-(i*0x10) );
}

static void process_wav( const int wvx_index, const unsigned char * const org_wave_start, const unsigned char * const wave_end, const int index )
{
   if( wave_end <= org_wave_start )
   {
      printf("Error: nonpositive sized waveform!\n");
      throw false;
   }
   const unsigned char * actual_wave_start = org_wave_start;
   //Strip out any leading empty VAG packets, they do no good to the extraction or to the duplicate checking.
   unsigned char emptyVagPacket[16] = { 0 };
   while( ( actual_wave_start < wave_end ) && !memcmp( emptyVagPacket, actual_wave_start, 16 ) )
   {
      actual_wave_start += 16;
   }

   const int wave_size = ( wave_end - actual_wave_start );
   if( wave_size == 0 )
   {
      printf("Error: empty waveform!\n");
      throw false;
   }

   char fullPath_adpcm[FILENAME_MAX];
   sprintf( fullPath_adpcm, "%s/%d.%03d.adpcm", gCurrOutputFolder, wvx_index, index );
   char fullPath_wav[FILENAME_MAX];
   sprintf( fullPath_wav, "%s/%d.%03d.wav", gCurrOutputFolder, wvx_index, index );

   AddWavToDupeMap( fullPath_adpcm, actual_wave_start, wave_size );

   switch ( gCommandLineArgs.mUsageMode )
   {
   case kUsageMode_ExtractWavs:
      {
         if( strchr( gCommandLineArgs.mExtractTypes, 'a' ) )
         {
            //extract an .adpcm file.
            FILE * fp = fopen( fullPath_adpcm, "wb" );
            if( !fp )
            {
               printf("Error: could not open %s for writing.\n", fullPath_adpcm );
               throw false;
            }
            fwrite( actual_wave_start, wave_size, 1, fp );
            fclose( fp );
         }
         if( strchr( gCommandLineArgs.mExtractTypes, 'w' ) )
         {
            //extract a wav file.
            const int decodedWavSize = sizeof( short ) * wave_size * 28 / 16;
            unsigned char * waveBuf = new unsigned char[decodedWavSize];
            DecodeADPCM((const signed char*)actual_wave_start,wave_size,(char*)waveBuf,1, 16);
            SaveLPCM16WavFile( fullPath_wav, 1, 48000, decodedWavSize, waveBuf );
            delete waveBuf;
         }
      }
      break;
   case kUsageMode_HeaderInfo:
      {
         //Nothing to do per wav file.
      }
      break;
   }
}

static void process_wavs( const int wvx_index, const unsigned char * const wave_load_ptr, const int wave_load_size )
{
   if( wave_load_size % 16 )
   {
      printf("Bad waveform data size, should be 16 byte packets!\n");
      throw false;
   }
   const unsigned char * const begin = wave_load_ptr;
   const unsigned char * const end = (wave_load_ptr+wave_load_size);
   const unsigned char * curr = begin;
   int index = 0;
   unsigned char emptyVagPacket[16] = { 0 };
   for( const unsigned char * p = begin+16; p != end; p += 16 )
   {
#if MGS_VERSION==2
      if( !memcmp( emptyVagPacket, p, 16 ) )
      {
         //Start of another waveform.  Save the last one.
         process_wav( wvx_index, curr, p, index++ );
         curr = p;
      }
#elif MGS_VERSION==3
      //Go by end tags instead.
      if( ( p - curr ) >= 16 )
      {
         const unsigned char * const p0 = p - 16;
         if( p0[1] & VAG_LOOPEND )
         {
            //Looped or unlooped end tag.
            process_wav( wvx_index, curr, p, index++ );
            curr = p;
         }
      }
#endif
   }

   //Save the final one.
   process_wav( wvx_index, curr, end, index++ );
}

#if MGS_VERSION==2

static void process_wvx( const int wvx_index, const int size_x, FILE * fp )
{
   int wave_load_size;
   int spu_load_offset;
   int offset;
   int size;
   unsigned char * voice_tbl_buf = new unsigned char[size_x];
   fread(voice_tbl_buf, size_x, 1, fp);
   offset = ((unsigned long)voice_tbl_buf[0]) << 24;
   offset |= ((unsigned long)voice_tbl_buf[1]) << 16;
   offset |= ((unsigned long)voice_tbl_buf[2]) << 8;
   offset |= (unsigned long)voice_tbl_buf[3];
   size = ((unsigned long)voice_tbl_buf[4]) << 24;
   size |= ((unsigned long)voice_tbl_buf[5]) << 16;
   size |= ((unsigned long)voice_tbl_buf[6]) << 8;
   size |= (unsigned long)voice_tbl_buf[7];
//   printf("SUP HEADER OFFSET=%x:HEADER SIZE=%x\n", (unsigned int)offset, (unsigned int)size);
   unsigned char * wave_load_ptr = &voice_tbl_buf[0x10];
   set_voice_tbl((struct WAVE_W *)wave_load_ptr, size, offset);
   /*--------------------- Load SPU Data ----------------------------------*/
   wave_load_ptr += size;
   spu_load_offset = ((unsigned long)*wave_load_ptr) << 24;
   spu_load_offset |= ((unsigned long)*(wave_load_ptr+1) ) << 16;
   spu_load_offset |= ((unsigned long)*(wave_load_ptr+2) ) << 8;
   spu_load_offset |= (unsigned long)*(wave_load_ptr+3);
   wave_load_size = ((unsigned long)*(wave_load_ptr+4) ) << 24;
   wave_load_size |= ((unsigned long)*(wave_load_ptr+5) ) << 16;
   wave_load_size |= ((unsigned long)*(wave_load_ptr+6) ) << 8;
   wave_load_size |= (unsigned long)*(wave_load_ptr+7);
//   printf("SPU OFFSET=%x:SIZE=%x\n", (unsigned int)spu_load_offset, (unsigned int)wave_load_size);
   wave_load_ptr += 0x10;

   //Process wavs.  Assume that each waveform starts with an empty VAG packet.
   process_wavs( wvx_index, wave_load_ptr, wave_load_size );

   delete voice_tbl_buf;
}

#elif MGS_VERSION==3

static void process_ipsq( const int ipsq_index, const int ipsq_size, FILE * fp )
{
#if 1
   //Don't need to parse over this junk ATM
   fseek( fp, ipsq_size, SEEK_CUR );
#else
   unsigned char * psqBuff = new unsigned char[ipsq_size];
   fread( psqBuff, ipsq_size, 1, fp );
   unsigned char * psq = psqBuff;

   while( memcmp( psq, "TERM\0\0\0\0", 8 ) != 0 )
   {
      if ( memcmp( psq, "IWAV", 4 ) == 0 )
      {
         //         BP_psqLoadIwav( sdfile.loadBank, sdfile.psq );
      }
      else if ( memcmp( psq, "IPAT", 4 ) == 0 )
      {
         //         BP_psqLoadIpat( sdfile.loadBank, sdfile.psq );
      }
      else if ( memcmp( psq, "TPAT", 4 ) == 0 )
      {
         //         psqLoadTpat( sdfile.loadBank, sdfile.psq );
      }
      else if ( memcmp( psq, "ISEQ", 4 ) == 0 )
      {
         //         BP_psqLoadIseq( sdfile.loadBank, sdfile.psq );
      }
      else if ( memcmp( psq, "TSEQ", 4 ) == 0 )
      {
         //         psqLoadTseq( sdfile.loadBank, sdfile.psq );
      }
      else if ( memcmp( psq, "BVAG", 4 ) == 0 )
      {
         //         BP_psqLoadBvag( sdfile.loadBank, sdfile.psq );
      }
      else
      {
         printf("Error: unhandled ipsq section!\n");
         throw false;
      }
      psq += getInt(psq + 4) + 8;
   }
   delete psqBuff;
#endif
   unsigned char bpvgBuf[8];
   fread( bpvgBuf, sizeof( bpvgBuf ), 1, fp );
   if( memcmp( bpvgBuf, "BPVG", 4 ) != 0 )
   {
      printf("Error: expected BPVG section!\n");
      throw false;
   }
   const int wav_size = getInt(bpvgBuf + 4);
   if( wav_size > 0 )
   {
      unsigned char * wavBuf = new unsigned char[wav_size];
      fread( wavBuf, wav_size, 1, fp );
      process_wavs( ipsq_index, wavBuf, wav_size );
      delete wavBuf;
   }
}

#endif

static void parse_input_file()
{
   FILE * fp = fopen( gCurrInputFilename, "rb" );
   if( !fp )
   {
      printf("Error: could not open %s for reading.\n", gCurrInputFilename );
      throw false;
   }

#if MGS_VERSION==2
   struct PAK_HEADER	*p = pak_header;
   fread((char *)pak_header, sizeof(pak_header), 1, fp);

   switch ( gCommandLineArgs.mUsageMode )
   {
   case kUsageMode_HeaderInfo:
      {
         printf("wvx1 [%4d,%2x] wvx2 [%4d,%2x] evx [%4d,%2x] mdx [%4d,%2x] %s\n", p[0].offset, p[0].code, p[1].offset, p[1].code, p[2].offset, p[2].code, p[3].offset, p[3].code, gCurrInputFilename );
      }
      break;
   }

   if (p[0].offset)
   {
      //wvx1
      int size_x = (p[1].offset-p[0].offset)*0x800;//wvx size
      process_wvx( 0, size_x, fp);
   }
   if( ( p[1].offset ) && (p[1].code != 0xFF) )
   {
      //wvx2
      //N.B. any section with an 0xff code is apparently never loaded by the game!
      //This includes half the waveform data in many of the sdx files.  Haven't looked into the reason why,
      //but we can zero out this data when rebuilding the sdx files if so.
      int size_x = (p[2].offset-p[1].offset)*0x800;
      process_wvx( 1, size_x, fp);
   }

#elif MGS_VERSION==3

   unsigned char loadBuff[8+sizeof(PSQ_IPSQ)];
   fread( loadBuff, sizeof( loadBuff ), 1, fp );
   PSQ_IPSQ * pPsq = (PSQ_IPSQ *)(loadBuff + 8);
   enum { EFX_SZ_PSQBUFF = 0x80000 };

   int ipsqIndex = 0;
   int garbagePaddingStartsAt = 0;
   while( memcmp( loadBuff, "IPSQ", 4 ) == 0 )
   {
      int ipsqSize = pPsq->fsize + 8 - sizeof(loadBuff);
      process_ipsq( ipsqIndex++, ipsqSize, fp );
      garbagePaddingStartsAt = ftell( fp );
      memset( loadBuff, 0, sizeof( loadBuff ) );
      fread( loadBuff, sizeof( loadBuff ), 1, fp );
   }
   //As soon as we fail to read the next 'IPSQ' section we've hit garbage padding.
   if( gCommandLineArgs.mUsageMode == kUsageMode_ZeroGarbagePadding )
   {
      //Reopen it for writing and overwrite the garbage end padding with zeroes.
      //This allows us to properly check redundancy across SKU.
      fclose( fp );
      FILE * fp = fopen( gCurrInputFilename, "r+b" );
      if( !fp )
      {
         printf("Error: could not open %s for r/w.\n", gCurrInputFilename );
         throw false;
      }
      fseek( fp, 0, SEEK_END );
      int fsize = ftell( fp );
      int garbageSize = fsize - garbagePaddingStartsAt;
      if( garbageSize >= SECTOR_SIZE )
      {
         printf("Error: >= 2K garbage padding encountered?\n");
         throw false;
      }
      fseek( fp, garbagePaddingStartsAt, SEEK_SET );
      char zeroPadBuf[SECTOR_SIZE] = { 0 };
      if( garbageSize != 0 )
      {
         fwrite( zeroPadBuf, garbageSize, 1, fp );
      }
   }

#endif

   fclose( fp );
}

static void process_input_file( const char * const inputFilename )
{
   strcpy( gCurrInputFilename, inputFilename );
   strcpy( gCurrOutputFolder, inputFilename );
   if( char * ext = strstr( gCurrOutputFolder, ".sdx" ) )
   {
      *ext = 0;
   }
   else
   {
      printf("Error: input file %s is not an .sdx file!\n");
      throw false;
   }

   switch (gCommandLineArgs.mUsageMode)
   {
   case kUsageMode_ExtractWavs:
      _mkdir( gCurrOutputFolder );
      break;
   }

   parse_input_file();
}

static std::string GetSdxPackageName(const std::string& packagePath)
{
   std::string result = packagePath.substr(packagePath.rfind("us\\"));
   std::replace(result.begin(), result.end(), '\\', '/');

   return result;
}

int main(int argc, char const *argv[])
{
   if( !parse_command_line( argc, argv ) )
   {
      usage();
      return -1;
   }

   try
   {
      sdx::DataUnifier wavDataUnifier;

      if( gCommandLineArgs.mInputFilename[0] != '@' )
      {
         if (gCommandLineArgs.mUsageMode == kUsageMode_DumpUnifiedWavArchive)
         {
            std::vector<std::vector<unsigned char> > adpcmData;
            wavDataUnifier.DumpArchive(gCommandLineArgs.mInputFilename, adpcmData);

            printf("Dumping archive contents from archive file: %s to directory <sdx_archive_conents>\n", gCommandLineArgs.mInputFilename);

            const char* pDirName = "sdx_archive_contents";
            _mkdir(pDirName);

            for (unsigned int ui = 0; ui < adpcmData.size(); ui++)
            {
               std::vector<unsigned char>& currData = adpcmData[ui];

               const int decodedWavSize = sizeof( short ) * currData.size() * 28 / 16;

               unsigned char* pWavBuf = new unsigned char[decodedWavSize];
               DecodeADPCM((const signed char*)&currData[0], currData.size(), (char*)pWavBuf, 1, 16);

               char filePath[512];
               sprintf(filePath, "%s\\wav%d.wav", pDirName, ui);
               SaveLPCM16WavFile(filePath, 1, 24000, decodedWavSize, pWavBuf);

               delete pWavBuf;
            }
         }
         else if (gCommandLineArgs.mUsageMode == kUsageMode_UnifyWavs)
         {
            std::string pkgName = GetSdxPackageName(gCommandLineArgs.mInputFilename);
            wavDataUnifier.RegisterPackage( gCommandLineArgs.mInputFilename, pkgName.c_str());
         }
         else
         {
            process_input_file( gCommandLineArgs.mInputFilename );
         }
      }
      else
      {
         FILE * textfp = fopen( gCommandLineArgs.mInputFilename+1, "rt" );
         if( !textfp )
         {
            printf("Error: could not open %s for reading.\n", gCommandLineArgs.mInputFilename+1 );
            throw false;
         }
         char processFilename[FILENAME_MAX];
         while( 1 == fscanf( textfp, "%s", processFilename ) )
         {
            if (gCommandLineArgs.mUsageMode != kUsageMode_UnifyWavs)
            {
               process_input_file( processFilename );
            }
            else
            {
               std::string pkgName = GetSdxPackageName(processFilename);
               wavDataUnifier.RegisterPackage( processFilename, pkgName.c_str());
            }
         }
         fclose( textfp );
      }

      switch( gCommandLineArgs.mUsageMode )
      {
      case kUsageMode_UnifyWavs:
         printf("Writing unified archive and manifest.\n");
         printf("Number of unique wave forms: %d\n", wavDataUnifier.GetNumUniqueWavs());
         wavDataUnifier.WriteArchiveAndManifest();
         break;

      case kUsageMode_DuplicateReport:
         {
            //Print out info in a spreadsheet-friendly format.
            printf("DUP COUNT: %d\n", gWavDupeMap.size());
            for( TWavDupeMap::const_iterator it = gWavDupeMap.begin(); it != gWavDupeMap.end(); ++it )
            {
               const SWavDupeInfo & wavDupeInfo = it->second;
               for( std::list< std::string >::const_iterator at = wavDupeInfo.mSrcLabelList.begin(); at != wavDupeInfo.mSrcLabelList.end(); ++at )
               {
                  const std::string & srcLabel = *at;

                  if( at == wavDupeInfo.mSrcLabelList.begin() )
                  {
                     //First encountered entry
                     printf( "%s\t%d\t%d\t%s\n", it->first.c_str(), wavDupeInfo.mCount, wavDupeInfo.mSize, srcLabel.c_str() );
                  }
                  else
                  {
                     if( gCommandLineArgs.mbPrintAllWavDupeSourceLabels )
                     {
                        //print remaining source labels for reference
                        printf( "\t\t\t%s\n", srcLabel.c_str() );
                     }
                  }
               }
            }
         }
         break;
      }
   }
   catch(...)
   {
      printf("Operation failed.\n" );
      return -1;
   }

   return 0;
}
