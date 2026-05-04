//----------------------------------------------------------------------------

#include "CStreamDriver_STV.h"

//----------------------------------------------------------------------------

using namespace System;
using namespace System::Xml;

//----------------------------------------------------------------------------

static const double f[5][2] = { { 0.0, 0.0 },
{  60.0 / 64.0,  0.0 },
{  115.0 / 64.0, -52.0 / 64.0 },
{  98.0 / 64.0, -55.0 / 64.0 },
{  122.0 / 64.0, -60.0 / 64.0 } };

int DecodeADPCM(const signed char* inputData,int inputDataSize,char* outputData,int nrChannels, int interleave)
{
   int blockCount = inputDataSize / 16;   //block = vag packet
   if( blockCount * 16 != inputDataSize )
   {
      printf("DecodeADPCM: unaligned source data size: %d!\n", inputDataSize );
      throw false;
   }

   int blockInterleave = interleave / 16;
   if( blockInterleave * 16 != interleave )
   {
      printf("DecodeADPCM: unaligned interleave size: %d (%d)!\n", inputDataSize, interleave );
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


   for (const signed char* data=inputData;data<inputData+inputDataSize /*&& (data[1] != 7 && data[1] != 3) */ && blockCount>0;data+=16)
   {
      //Erase special "end marker" pattern before decoding.  This manifests itself as a pop of static in the extracted lpcm data otherwise.
      static const unsigned char kMagicVagPattern[16] = { 0x00,0x01,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77 };
      static const unsigned char kFixedVagPattern[16] = { 0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
      const unsigned char * p = (const unsigned char*)data;
      if( !memcmp( data, kMagicVagPattern, sizeof( kMagicVagPattern ) ) )
      {
         p = kFixedVagPattern;
      }

      predict_nr = p[0];
      shift_factor = predict_nr & 0xf;
      predict_nr >>= 4;

      for (i = 0; i < 28; i += 2) {
         d = p[i / 2 + 2];
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

CStreamDriver_STV::CStreamDriver_STV()
: mbInitialized( false )
{
}

static inline int get_big32( void *ptr )
{
   unsigned char *p;
   p = ( unsigned char * )ptr;
   return ( p[ 0 ] << 24 ) | ( p[ 1 ] << 16 ) | ( p[ 2 ] << 8 ) | ( p[ 3 ] );
}

void CStreamDriver_STV::ProcessPacket( const STREAM_TAG & packet, const void * const pBody )
{
   const int bodySize = packet._size - sizeof( packet );
   if( !mbInitialized )
   {
      //Header packet.
      mChannel = ( ( ( char * )pBody )[ 3 ] == '1' ) ? 1 : 2;
      mFrequency = get_big32( ( char * )pBody + 0x10 );
      mSize = get_big32( ( char * )pBody + 0x0C ) * mChannel;
      mDuration = (int)( ( mSize / 16 * 28 ) * 300.0F / ( mFrequency * mChannel ) );
      mPacketSize = 0x800 * mChannel;

      mStvBuffer.reserve( mSize );
      mbInitialized = true;

      //Write out an xml document with pcm header information.
      XmlDocument^ doc = gcnew XmlDocument();
      doc->AppendChild( doc->CreateXmlDeclaration( "1.0", nullptr, nullptr ) );
      XmlNode^ documentRoot = doc->AppendChild(doc->CreateElement("STV"));
      {
         XmlElement^ subElement = (XmlElement^)(documentRoot->AppendChild(doc->CreateElement("size")));
         subElement->InnerText = mSize.ToString();

         subElement = (XmlElement^)(documentRoot->AppendChild(doc->CreateElement("frequency")));
         subElement->InnerText = mFrequency.ToString();

         subElement = (XmlElement^)(documentRoot->AppendChild(doc->CreateElement("channels")));
         subElement->InnerText = mChannel.ToString();
      }
      char filename[FILENAME_MAX];
      sprintf( filename, "%s/%s/%s.stv.xml", gOutputFolder, gCurrStreamName, gCurrStreamName );
      doc->Save(gcnew System::String(filename));
   }
   else
   {
      //Stream packet.
      const int blockCount = packet._option;
      const int VAGST_SPU_SIZE = 0x800;
      const int blockSize = mChannel * VAGST_SPU_SIZE;
      if( blockSize * blockCount > bodySize )
      {
         printf("Error: STV stream packet size too small!  Expected at least %d, got %d\n", blockSize * blockCount, bodySize );
         throw false;
      }

      for( int i=0; i < blockSize * blockCount; ++i )
      {
         mStvBuffer.push_back( *((const unsigned char*)pBody + i) );
      }
   }
}

void CStreamDriver_STV::EndStream()
{
   if( mbInitialized )
   {
      char fullpath[FILENAME_MAX];
      const unsigned char * const stvBuffer = &mStvBuffer[0];
      const int stvBufferSize = mStvBuffer.size() * sizeof(mStvBuffer[0]);

#if 0
      //disabled writing out raw vag data
      sprintf( fullpath, "%s/%s/%s.stv", gOutputFolder, gCurrStreamName, gCurrStreamName );
      FILE * fp = fopen( fullpath, "wb" );
      if( !fp )
      {
         printf("Error opening output file: %s\n", fullpath );
         throw false;
      }

      fwrite( stvBuffer, stvBufferSize, 1, fp );
      fclose( fp );
#endif
      //Convert to wav format and write out a wav file.
      //N.B. Stereo is interleaved at 0x800 bytes.

      sprintf( fullpath, "%s/%s/%s.wav", gOutputFolder, gCurrStreamName, gCurrStreamName );
      const int wavBufferSize = sizeof( short ) * stvBufferSize * 28 / 16;
      std::vector< unsigned char > wavBuffer( wavBufferSize, 0 );

      unsigned char * const pWavBuffer = &wavBuffer[0];
      DecodeADPCM( (const signed char*)stvBuffer, stvBufferSize, (char*)pWavBuffer, mChannel, 0x800 );

      SaveLPCM16WavFile( fullpath, mChannel, mFrequency, wavBufferSize, pWavBuffer );

      mStvBuffer.clear();
   }
   mbInitialized = false;
}

