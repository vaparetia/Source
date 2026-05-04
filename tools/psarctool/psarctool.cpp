//----------------------------------------------------------------------------
//psarc tool
//examine a psarc archive
//----------------------------------------------------------------------------

#include "CPsarcFile.h"

//----------------------------------------------------------------------------

int main( int argc, char const *argv[] )
{
   if( argc != 2 )
   {
      printf("Usage: psarctool.exe psarcfilename\n");
      return -1;
   }

   const char * const psarcFilename = argv[1];
   CPsarcFile psarcFile;
   int ret = psarcFile.ReadPsarcFile( psarcFilename );
   return ret;
}

//----------------------------------------------------------------------------
