#include <sifdev.h>
#include <libipu.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <libeenet.h>
#include <math.h>

#define DTD(flags) (((flags) >> 2) & 0x1)
#define IPU_CTRL_VAL(flags) (((flags) & ~0x00000004) << 16)

struct IPUHeader {
   char id[4];
   unsigned int numBytes;
   unsigned short width;
   unsigned short height;
   unsigned int numFrames;
};

////////////////////////////////////////////////////////////////////////////////////
void WriteTGAFrame(unsigned short width, unsigned short height, unsigned int frame,
   unsigned char* pcDecodeBuffer);
   
void ExtractMacroBlock(unsigned char* pDst, unsigned int dstWidth, unsigned int dstHeight,
   unsigned int* pSrc);

////////////////////////////////////////////////////////////////////////////////////
const unsigned int kPixelSizeInBytes = 4;
const char* kIPUID = "ipum";
const char kIpuFrameEndMarker[] = { 0x00, 0x00, 0x01, 0xb0 };
const unsigned int kBlocksize = 16 * 16 * kPixelSizeInBytes / 16;

////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{   
   if (argc != 2)
   {
      printf("Usage: ipu2mpeg <in_file.ipu>\n");
      return 1;
   }
   
   char acFileName[255];
   sprintf(acFileName, "host:%s", argv[1]);
   int fd;

   fd = sceOpen(acFileName, SCE_RDONLY);
   if (fd < 0)
   {
      printf("Cannot open file: <%s>\n", acFileName);
      return 1;
   }

   unsigned int fileSize = sceLseek(fd, 0, SCE_SEEK_END);
   unsigned char* pcFileData = (unsigned char*)malloc(fileSize);

   sceLseek(fd, 0, SCE_SEEK_SET);
   unsigned int numBytesRead = sceRead(fd, pcFileData, fileSize);
   if (numBytesRead != fileSize)
   {
      free(pcFileData);
      sceClose(fd);
      printf("Error reading file.\n");
      return 1;
   }
   else
   {
      sceClose(fd);
   }

   unsigned char* pcMem = pcFileData;
   IPUHeader* pkIpuHeader = (IPUHeader*)pcMem;
   pcMem += 16;

   if (memcmp(kIPUID, pkIpuHeader, 4))
   {
      printf("File is not an IPU stream!\n");
      free(pcFileData);
      return 1;
   }
   
   printf("Converting IPU file %s\n", acFileName);
   printf("\tWidth: %d\n", pkIpuHeader->width);
   printf("\tHeight: %d\n", pkIpuHeader->height);
   printf("\tNumFrames: %d\n", pkIpuHeader->numFrames);
   printf("\tNumBytes: %d\n", pkIpuHeader->numBytes);

   unsigned char* pcDecodeBuffer = (unsigned char*)memalign(128, kBlocksize * 16 *
         (pkIpuHeader->width / 16) * (pkIpuHeader->height / 16));
   unsigned int currFrame = 0;
   
   while (currFrame < pkIpuHeader->numFrames)
   {
      unsigned char ipuFlags = *pcMem;
      unsigned char* pcFrameStart = pcMem;
      
      while (1)
      {
         if (!memcmp(pcMem, &kIpuFrameEndMarker[0], 4))
            break;
         pcMem++;
      }

      pcMem += 4;
      
      // Past the end of frame marker, there could be additional bytes to force 16-byte
      // alignment.
      unsigned int frameNumBytes = pcMem - pcFrameStart;
      unsigned int alignedNumBytes = (frameNumBytes + 15) & ~15;
      pcMem += (alignedNumBytes - frameNumBytes);

      if ((currFrame % 25) == 0)
         printf("Writing frame %d of %d\n", currFrame + 1, pkIpuHeader->numFrames);

      FlushCache(0);

      DPUT_D4_CHCR(0x000);
      DPUT_D3_CHCR(0x000);

      sceIpuReset();
      sceIpuSync(0, 0);
      sceIpuBCLR(8);
      sceIpuSync(0, 0);

      DPUT_IPU_CTRL(IPU_CTRL_VAL(ipuFlags));
      sceIpuIDEC(0, 0, 0, DTD(ipuFlags), 1, 0);

      // Dma to the ipu for decoding...
      DPUT_D4_MADR((u_int)pcFrameStart);
      DPUT_D4_QWC(alignedNumBytes / 16);
      DPUT_D4_CHCR(0x101);
      DPUT_D_STAT(1 << 3);

      // Post a dma request to get the decoded data back from the ipu.
      DPUT_D3_MADR((u_int)pcDecodeBuffer);
      DPUT_D3_QWC(kBlocksize * (pkIpuHeader->width / 16) * (pkIpuHeader->height / 16));
      DPUT_D3_CHCR(0x100);

      // Wait until the ipu is done decoding the frame data.
      while (DGET_D3_CHCR() & 0x100);
      DPUT_D4_CHCR(0x000);
      
      WriteTGAFrame(pkIpuHeader->width, pkIpuHeader->height, currFrame, pcDecodeBuffer);
      currFrame++;
   }

   printf("\n");

   DPUT_D4_CHCR(0x000);
   DPUT_D3_CHCR(0x000);
   sceIpuReset();
   
   free(pcFileData);
   return 0;
}

////////////////////////////////////////////////////////////////////////////////////
void WriteTGAFrame(unsigned short width, unsigned short height, unsigned int frame,
   unsigned char* pcDecodeBuffer)
{
   struct TgaHeader
   {
      unsigned char identsize;
      unsigned char colormaptype;
      unsigned char imagetype;

      short colormapstart;
      short colormaplength;
      unsigned char colormapbits;

      short xstart;
      short ystart;
      short width;
      short height;
      unsigned char bits;
      unsigned char descriptor;
   } __attribute__((__packed__));
   
   int fd;
   char acFileName[255];
   
   sprintf(acFileName, "host:frame%d.tga", frame);
   fd = sceOpen(acFileName, SCE_CREAT | SCE_TRUNC | SCE_WRONLY);
   if (fd < 0)
   {
      printf("Cannot write frame data to file: <%s>\n", acFileName);
      return;
   }
   
   TgaHeader tgaHeader;
   memset(&tgaHeader, 0, sizeof(TgaHeader));

   tgaHeader.imagetype = 2;
   tgaHeader.width = width;
   tgaHeader.height = height;
   tgaHeader.bits = 24;
   tgaHeader.descriptor = 0x20;

   sceWrite(fd, &tgaHeader, 18);

   unsigned char* pcRasterImage = (unsigned char*)malloc(width * height * 3);
   memset(pcRasterImage, 0, width * height * 3);

   unsigned char* pcRasterStart = pcRasterImage;
   unsigned char* pcMacroBlockStart = pcDecodeBuffer;
   for (int i = 0; i < height; i += 16)
   {
      for (int j = 0; j < width; j += 16)
      {
         pcRasterStart = pcRasterImage + ((i * width + j) * 3);
         ExtractMacroBlock(pcRasterStart, width, height, (unsigned int*)pcMacroBlockStart);
         pcMacroBlockStart += (16 * 16 * kPixelSizeInBytes);
      }
   }

   sceWrite(fd, &pcRasterImage[0], width * height * 3);
   free(pcRasterImage);
   sceClose(fd);
}

void ExtractMacroBlock(unsigned char* pDst, unsigned int dstWidth, unsigned int dstHeight,
   unsigned int* pSrc)
{
   for (int i = 0; i < 16; i++)
   {
      unsigned char* pDstPixel = pDst + (i * dstWidth * 3);
      
      for (int j = 0; j < 16; j++)
      {
         unsigned int srcPixel = *pSrc++;
         unsigned char blue = (srcPixel & 0x00ff0000) >> 16;
         unsigned char green = (srcPixel & 0x0000ff00) >> 8;
         unsigned char red = (srcPixel & 0x000000ff);

         *pDstPixel++ = blue;
         *pDstPixel++ = green;
         *pDstPixel++ = red;
      }
   }
}
