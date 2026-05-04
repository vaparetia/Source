// JPEGRowConverter.cpp : Very simple application to open a JPEG and add some unique identifiers to it and a filesize field

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int JPEG(int argc, char* argv[]);
int Movie(int argc, char* argv[]);

// Usage notes:
// p4 edit ...celgei_snapshot_bigsize.row
// p4 edit ...reflact02.row
// run programe
// revert unchanged, check in

enum ERowFileType
{
   kJPG,
   kMovie,
};

struct SFileInfo
{
   char* pFileNameSource;
   char* pExtensionSource;
   char* pUniqueHeader;
};

SFileInfo gArrayFiles[] = 
{
   "assets\\row\\us\\_bp\\celgei_snapshot_bigsize", "jpg", "BP_MEMJPEG",
   "assets\\row\\eu\\_bp\\celgei_snapshot_bigsize", "jpg", "BP_MEMJPEG",

   "assets\\row\\us\\ovr_ps3\\reflact02", "m2v", "BP_MEMMPEG",
   "assets\\row\\us\\ovr_360\\reflact02", "wmv", "BP_MEMMPEG",
};

int DoFile(SFileInfo* pFileInfo);

int main(int argc, char* argv[])
{
   for( int ii=0; ii<sizeof(gArrayFiles)/sizeof(gArrayFiles[0]); ++ii )
   {
      DoFile( &gArrayFiles[ii] );
   }
}

int DoFile(SFileInfo* pFileInfo)
{
   char fileNameSource[1024];
   sprintf( fileNameSource, "%s.%s", pFileInfo->pFileNameSource, pFileInfo->pExtensionSource );
   FILE* Input_fp = NULL;
   fopen_s(&Input_fp, fileNameSource, "rb");
   if(!Input_fp)
   {
      printf("Error opening source file: %s\n", fileNameSource);
      return -1;
   }

   char fileNameDest[1024];
   sprintf( fileNameDest, "%s.row", pFileInfo->pFileNameSource );
   FILE* Ouput_fp = NULL;
   fopen_s(&Ouput_fp, fileNameDest, "wb");
   if(!Ouput_fp)
   {
      printf("Error opening output file: %s\n", fileNameDest);
      return -1;
   }

   fseek(Input_fp, 0, SEEK_END);
   int Input_TotalFileLengthBytes = ftell(Input_fp);
   fseek(Input_fp, 0, SEEK_SET);

   //
   char* const pUniqueID = pFileInfo->pUniqueHeader;
   int uniqueIDLength = strlen(pUniqueID);
   int totalHeaderSize = uniqueIDLength + 1 + sizeof(int); // length + NULL character + sizeof JPEG

   char* memBuffer = new char[Input_TotalFileLengthBytes+totalHeaderSize];
   unsigned int bytesRead = fread(&memBuffer[totalHeaderSize], 1, Input_TotalFileLengthBytes, Input_fp);
   if(bytesRead != Input_TotalFileLengthBytes)
   {
      printf("Error reading source file: %s\n", fileNameSource);
      return -1;
   }
   fclose(Input_fp);
   //fill in header info
   strcpy(&memBuffer[0], pUniqueID);
   memcpy(&memBuffer[uniqueIDLength+1], &Input_TotalFileLengthBytes, sizeof(int));
   //write out our new data
   fwrite(&memBuffer[0], Input_TotalFileLengthBytes+totalHeaderSize, 1, Ouput_fp);
   //cleanup
   delete [] memBuffer;
   fclose(Ouput_fp);
   return 0;
}
