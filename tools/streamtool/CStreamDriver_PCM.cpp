//----------------------------------------------------------------------------

#include "CStreamDriver_PCM.h"
#include "CStreamDriver_STV.h"

//----------------------------------------------------------------------------

using namespace System;
using namespace System::Xml;

//----------------------------------------------------------------------------

#define LNR8_BUF_SIZE 0x400 
#define LNR8_SAMPLES_PER_BUF 0x200  //256 left, 256 right

//----------------------------------------------------------------------------

static signed short lnr_val[2] = { 0 };

static void lnr_trans_init(void)
{
   lnr_val[0] = lnr_val[1] = 0;
}

static inline signed short dec_16(unsigned char val8)
{
   signed short	val16;
   int		shift;

   shift = val8 & 7;
   val16 = val8 & 0x78;

   if (shift != 7) {
      val16 |= 0x80;
      val16 = (val16 << 7) >> shift;
   } else {
      val16 = (val16 << 8) >> shift;
   }
   if (val8 & 0x80) val16 = -val16;

   return (val16);
}

static void lnr_trans(signed short *ptr16, const signed char *ptr8, unsigned int sz, bool sound_mono_fg )
{
	unsigned int	i, j;
	signed short	val_l, val_r;
	signed long		val_m;

	if (sound_mono_fg) {
/*MONO Mix*/
		val_l = lnr_val[0];
		val_r = lnr_val[1];
		for (j=0; j<sz; j+=0x200) {
			for (i=0; i<0x100; i++) {
				val_l = dec_16(*ptr8) + val_l;
				ptr8++;
				val_r = dec_16(*ptr8) + val_r;
				ptr8++;
				val_m = (signed long)val_l + (signed long)val_r;
				*(ptr16+i+j) = *(ptr16+i+j+0x100) = (signed short)(val_m / 2);
			}
			lnr_val[0] = val_l;
			lnr_val[1] = val_r;
		}
	} else {
/*STEREO*/
		val_l = lnr_val[0];
		val_r = lnr_val[1];
		for (j=0; j<sz; j+=0x200) {
			for (i=0; i<0x100; i++) {
				val_l = *(ptr16+i+j) = dec_16(*ptr8) + val_l;
				ptr8++;
				val_r = *(ptr16+i+j+0x100) = dec_16(*ptr8) + val_r;
				ptr8++;
			}
			lnr_val[0] = val_l;
			lnr_val[1] = val_r;
		}
	}
}

//----------------------------------------------------------------------------

CStreamDriver_PCM::CStreamDriver_PCM()
: mbInitialized( false )
{
}

void CStreamDriver_PCM::ProcessPacket( const STREAM_TAG & packet, const void * const pBody )
{
   const int bodySize = packet._size - sizeof( packet );
   if( !mbInitialized )
   {
      //Header packet.
      const unsigned char *p = (const unsigned char*)pBody;

      mSize = ( p[ 0 ] << 24 ) | ( p[ 1 ] << 16 ) | ( p[ 2 ] << 8 ) | ( p[ 3 ] );
      mFrequency = ( p[ 6 ] << 8 ) | ( p[ 7 ] );
      mChannel = p[ 8 ];
      mFormat = p[ 10 ];

      if( mFormat == 1 )
      {
         //Ignore stream frequency setting when it's the direct output format.
         //At runtime it was forced to play at the PS2 native output frequency.
         mFrequency = 48000;

         mAdpcmBuffer.reserve( mSize );
         mTrackBuffer.reserve( mSize );
      }
      else
      {
         mAdpcmBuffer.reserve( mSize );

      }

      if( mFormat == 2 )
      {
         printf("TODO: AC3 extraction (not really) %s\n", gCurrStreamName);
      }
      else
      {
         //Write out an xml document with pcm header information.
         XmlDocument^ doc = gcnew XmlDocument();
         doc->AppendChild( doc->CreateXmlDeclaration( "1.0", nullptr, nullptr ) );
         XmlNode^ documentRoot = doc->AppendChild(doc->CreateElement("PCM"));
         {
            XmlElement^ subElement = (XmlElement^)(documentRoot->AppendChild(doc->CreateElement("size")));
            subElement->InnerText = mSize.ToString();

            subElement = (XmlElement^)(documentRoot->AppendChild(doc->CreateElement("frequency")));
            subElement->InnerText = mFrequency.ToString();

            subElement = (XmlElement^)(documentRoot->AppendChild(doc->CreateElement("channels")));
            subElement->InnerText = mChannel.ToString();

            subElement = (XmlElement^)(documentRoot->AppendChild(doc->CreateElement("format")));
            subElement->InnerText = mFormat.ToString();
         }
         char filename[FILENAME_MAX];
         sprintf( filename, "%s/%s/%s.pcm.xml", gOutputFolder, gCurrStreamName, gCurrStreamName );
         doc->Save(gcnew System::String(filename));
      }

      mbInitialized = true;
   }
   else
   {
      //Stream packet.
      const unsigned char * const sampleData = (const unsigned char *)(pBody);

      switch( mFormat )
      {
      case 0:
         //ADPCM
         {
            for( int i=0; i < bodySize; ++i )
            {
               mAdpcmBuffer.push_back( sampleData[i] );
            }
         }
         break;
      case 1:
         {
            signed short decodeBuf[LNR8_BUF_SIZE] = { 0 };
            if( (bodySize % LNR8_BUF_SIZE) != 0 )
            {
               printf("Unexpected str3 packet size!\n");
               throw false;
            }

            for( int i=0; i < bodySize; ++i )
            {
               mAdpcmBuffer.push_back( sampleData[i] );
            }

            for(int i=0; i < bodySize / LNR8_BUF_SIZE; ++i)
            {
               const signed char * decodeSrc = (const signed char*)&sampleData[i*LNR8_BUF_SIZE];
               lnr_trans( decodeBuf, decodeSrc, LNR8_BUF_SIZE, mChannel == 1 );
               for( int j=0; j < sizeof(decodeBuf)/sizeof(*decodeBuf); ++j )
               {
                  mTrackBuffer.push_back( decodeBuf[j] );
               }
            }
         }
         break;
      case 2:
         //AC3
         break;
      default:
         printf("TODO!\n");
         throw false;
      }
   }
}

void CStreamDriver_PCM::EndStream()
{
   if( mbInitialized )
   {
      char fullpath[FILENAME_MAX];

      switch( mFormat )
      {
      case 0:
         {
            const unsigned char * const pAdpcmBuffer = &mAdpcmBuffer[0];
            const int adpcmBufferSize = mAdpcmBuffer.size() * sizeof(mAdpcmBuffer[0]);
#if 0
            sprintf( fullpath, "%s/%s/%s.stv", gOutputFolder, gCurrStreamName, gCurrStreamName );
            FILE * fp = fopen( fullpath, "wb" );
            if( !fp )
            {
               printf("Error opening output file: %s\n", fullpath );
               throw false;
            }

            fwrite( pAdpcmBuffer, adpcmBufferSize, 1, fp );
            fclose( fp );
#endif
            //Convert to wav format and write out a wav file.
            //N.B. Stereo is interleaved at 0x800 bytes.

            sprintf( fullpath, "%s/%s/%s.wav", gOutputFolder, gCurrStreamName, gCurrStreamName );
            const int wavBufferSize = sizeof( short ) * adpcmBufferSize * 28 / 16;
            std::vector< unsigned char > wavBuffer( wavBufferSize, 0 );

            unsigned char * const pWavBuffer = &wavBuffer[0];
            int channels = ( mChannel == 1 ) ? 1 : 2;
            DecodeADPCM( (const signed char*)pAdpcmBuffer, adpcmBufferSize, (char*)pWavBuffer, channels, 0x800 );

            SaveLPCM16WavFile( fullpath, channels, mFrequency, wavBufferSize, pWavBuffer );
         }
         break;
      case 1:
         {
            const unsigned char * const pAdpcmBuffer = &mAdpcmBuffer[0];
            const int adpcmBufferSize = mAdpcmBuffer.size() * sizeof(mAdpcmBuffer[0]);

#if 0
            sprintf( fullpath, "%s/%s/%s.pcm", gOutputFolder, gCurrStreamName, gCurrStreamName );
            FILE * fp = fopen( fullpath, "wb" );
            if( !fp )
            {
               printf("Error opening output file: %s\n", fullpath );
               throw false;
            }

            fwrite( pAdpcmBuffer, adpcmBufferSize, 1, fp );
            fclose( fp );
#endif
            //Data is already decoded.  Need to re-interleave at 1 sample rather than 256.
            std::vector< short > wavBuffer( mTrackBuffer.size(), 0 );

            short * pDst = &wavBuffer[0];
            const short * pSrcL = &mTrackBuffer[0];
            const short * pSrcR = &mTrackBuffer[LNR8_SAMPLES_PER_BUF/2];
            for( unsigned int i=0; i < wavBuffer.size(); )
            {
               *pDst++ = *pSrcL++;
               *pDst++ = *pSrcR++;

               i += 2;
               if( ( i & (LNR8_SAMPLES_PER_BUF-1) ) == 0 )
               {
                  //Next 1024byte / 512sample packet.  (we're already halfway there from incrementing during loop)
                  pSrcL += LNR8_SAMPLES_PER_BUF/2;
                  pSrcR += LNR8_SAMPLES_PER_BUF/2;
               }
            }

            sprintf( fullpath, "%s/%s/%s.wav", gOutputFolder, gCurrStreamName, gCurrStreamName );
            unsigned char * const pWavBuffer = (unsigned char * const)(&wavBuffer[0]);
            const unsigned int wavBufferSize = wavBuffer.size() * sizeof(wavBuffer[0] );
            SaveLPCM16WavFile( fullpath, mChannel, mFrequency, wavBufferSize, pWavBuffer );
         }
         break;
      case 2:
         //AC3
         break;
      default:
         printf("TODO!\n");
         throw false;
      }
      mAdpcmBuffer.clear();
      mTrackBuffer.clear();
      lnr_trans_init();
   }
   mbInitialized = false;
}

