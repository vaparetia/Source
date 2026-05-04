#include <stdio.h>
#include <string>

namespace
{
   static const int skLineTooLong = -1;
}

char const *MDU_TailName( char const *filename )
{
   char	const *tail ;
   char const *cp;
   char	c;

   tail = cp = filename ;
   while ( ( c = *( cp ++ ) ) != '\0' ) {
      if ( c == '/' || c == '\\' ) tail = cp ;
   }
   return tail ;
}

#define BIT_LEN		24
int 	MDU_GetStrCode( char const *string )
{
   unsigned char c ;
   unsigned char const *p ;
   unsigned int id, mask ;

   //    return MDU_GetStrCodeOld( string ) ;

   //    p = ( unsigned char * )string ;
   p = (unsigned char const *) MDU_TailName( string ) ;
   id = 0 ;
   mask = ( 1 << BIT_LEN ) - 1 ;

   while( ( c = *( p++ ) ) != '\0' ) {
      if ( c == '.' ) break ;
      id = ( id << 5 ) | ( id >> ( BIT_LEN - 5 ) ) ;
      id += c ;
      id &= mask ;
   }
   if( id == 0 ) id = 1 ;
   return id;
}

int trim_end_return_len( char *str )
{
   // Since fgets returns a string with the newline intact, this mutates
   // the input string to remove the newline and returns the resultant string
   // length.
   // It there is no newline, then it returns skLineTooLong so the tool can know that the
   // line was too long

   // Also, we should trim any excess spaces, as some of the other parts of this 
   // tool can't deal

   int len = strlen( str );

   if ( len == 0 )
   {
      return skLineTooLong;
   }

   if ( str[ len - 1 ] == '\n' )
   {
      // Trimming whitespace off the end
      --len;
      while ( len && strchr( "\t\r ", str[ len - 1 ] ) )
      {
         --len;
      }

      str[len] = 0;
      return len;
   }
   else
   {
      return skLineTooLong;
   }
}

std::string const get_dar_filename( char const *line )
{
   // Parses a line of text from DAR such as...
   // filename.ext ( size #### )
   // and returns filename.ext

   char const *foundDot = strchr( line, '.' );
   char const *foundSize = strstr( line, " ( size " );

   if ( foundSize != NULL && foundDot != NULL && foundDot < foundSize )
   {
      return std::string( line, foundSize );
   }
   else
   {
      return std::string();
   }
}

std::string const get_qar_filename( char const *line )
{
   // Parses a line of text from QAR such as...
   // ID ######## : filename.ext
   // and returns filename.ext

   if ( strncmp( line, "ID ", 3 ) == 0 )
   {
      // Starts with ID...
      static char const *skColon = "\t: ";
      static const int skColonLen = strlen( skColon );
      char const *foundDot = strchr( line, '.' );
      char const *foundColon = strstr( line, skColon );
      char const *end = strchr( line, 0 );

      if ( foundDot && foundColon && foundDot > foundColon && end > foundColon + skColonLen )
      {
         return std::string( foundColon + skColonLen );
      }
   }

   return std::string();
}

void PrintUsage()
{
   printf( 
      "Scans stdin for MGS ID's and prints to stdout\n"
      "\n"
      "Usage:\n"
      "id_scan [-s]\n"
      " -s - Simple scan of filenames, one per line.  Without this flag, it will\n"
      "      only find id's from the output of dar and qar tools\n"
      " -i - Output's id's only\n"
      " -l - Output for flatlisttool lookup file\n"
      " -a - Every arg after this is parsed for id instead of file (implies -i and -s)\n"
      " -? - Prints this help\n"
      "\n" );
}

enum EOutputType
{
   kOT_MGS2,
   kOT_Simple,
   kOT_Lookup
};

int main( int argc, char const *argv[] )
{
   char line[ 255 ];
   bool ignore_until_eol = false;

   bool simple_parse = false;
   EOutputType outputType = kOT_MGS2;
   int first_scan_arg = -1;

   for ( int arg = 1; arg < argc; ++arg )
   {
      if ( strcmp( argv[arg], "-s" ) == 0)
      {
         simple_parse = true;
      }
      else if ( strcmp( argv[arg], "-i" ) == 0)
      {
         outputType = kOT_Simple;
      }
      else if ( strcmp( argv[arg], "-l" ) == 0 )
      {
         outputType = kOT_Lookup;
      }
      else if ( strcmp( argv[arg], "-a" ) == 0)
      {
         // All args after this are id's
         first_scan_arg = arg + 1;
         outputType = kOT_Simple;
         simple_parse = true;
         break;
      }
      else if ( strcmp( argv[arg], "-?" ) == 0 )
      {
         PrintUsage();
         return 0;
      }
      else
      {
         printf( "Unknown arg: %s\n\n", argv[arg] );
         PrintUsage();
         return 1;
      }
   }

   bool printed_simple_line = false;

   for ( ;; )
   {
      char *str;
      int len;

      if ( first_scan_arg == argc )
      {
         break;
      }

      if ( first_scan_arg == -1 )
      {
         str = fgets( line, sizeof( line ), stdin );

         if ( str == NULL )
         {
            break;
         }

         len = trim_end_return_len( str ); 
      }
      else
      {
         str = line;
         memset( line, 0, sizeof( line ) );
         strncpy( line, argv[first_scan_arg], sizeof( line ) );
         ++first_scan_arg;
         len = strlen( str );
      }

      if ( len == skLineTooLong )
      {
         // If the line is too long, then just punt on this line
         // and any other line until we get an end of line
         ignore_until_eol = true;
      }
      else if ( ignore_until_eol )
      {
         // We were ignoring until we hit EOL.  Well, we're done.
         // We will pay attention to the next line
         ignore_until_eol = false;
      }
      else
      {
         // Otherwise we have a line from either QAR, dar, or whatever
         std::string filename;

         if ( simple_parse )
         {
            filename = line;
         }
         else
         {
            filename = get_dar_filename( line );
            if ( filename.empty() )
            {
               filename = get_qar_filename( line );
            }
         }

         if ( !filename.empty() )
         {
            int id = MDU_GetStrCode( const_cast<char *>( filename.c_str() ) );

            switch ( outputType )
            {
            case kOT_Lookup:
               // Outputting for a flatlisttool lookup table
               {
                  printf( "0x%8.8x %s\n", id, MDU_TailName( filename.c_str() ) );
               }
               break;
            case kOT_Simple:
               // When outputting simple output, don't put a newline out unless we have to. 
               // It convenient when piping into clip
               printf( "%s%08x", printed_simple_line ? "\n" : "", id );
               printed_simple_line = true;
               break;
            case kOT_MGS2:
               // Encode it like MGS2's idlist.txt files
               printf( "%-10s\t=> 0x%08x : %8d\n", filename.c_str(), id, id ) ;
               break;
            }
         }
      }
   }
}