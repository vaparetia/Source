/*
	$Id: main.c,v 1.3 2001/02/07 05:15:39 usr02774 Exp $
 */
/*
		Texture-Row-Image File create program
		1999.4.9  By K.Takabe
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loadinc.h"
#include "tex_unpack.h"

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (-1)
#endif

typedef enum
{
   kCLMode_None,
   kCLMode_InputFiles,
   kCLMode_OutputFiles,
   kCLMode_GetTextures
}
ECLMode;

int ProgMain( char *out_name, int n_files, char **in_files, int options )
{
   int	ret = 0 ;

   for ( ; n_files > 0 ; n_files--, in_files++ )
   {
      fprintf( stdout, "now loading...(%s)\n", *in_files );
      if ( LoadCm2File( *in_files ) == -1 )
      {
         ret = -1 ;
      }
   }
   fprintf( stdout, "write file to <%s> \n", out_name );
   WriteTextureRowImageFile( out_name, options );
   return ret ;
}

void usage(void)
{
   fprintf( stderr, "Texture-Row-Image file create program  By K.Takabe\n" );
   fprintf( stderr, "Usage: make_tri [options] -o <outputfile(*.tri)> -i <inputfile(*.cm2)> ... \n" );
   fprintf( stderr, "(Options)  -t ... transparent texture mode \n" );
   fprintf( stderr, "           -s ... static palette texture mode \n" );
   fprintf( stderr, "           -l ... latter draw flag on \n" );
   /*fprintf( stderr, "           -c ... data compression \n" );*/
   fprintf( stderr, "           -x ... extract TGAs from input tri file(s) \n" );
   fprintf( stderr, "           -3 ... extract to MGS3 format (default is MGS2) \n" );
   fprintf( stderr, "           -q ... get texture stats (do not create or extract) \n" );
   fprintf( stderr, "           -u ... get unique texture stats (do not create or extract) \n" );
   fprintf( stderr, "           -d ... use id as extracted texture name \n" );
   fprintf( stderr, "           -p ... extract texture pages instead of textures \n" );
   fprintf( stderr, "           -b ... BP! rebuild tri file in _bp/ subfolder with black bitmap data\n" );
   fprintf( stderr, "           -a ... AS! add texture(s) to input tri file(s) \n" );
   fprintf( stderr, "           -f ... AS! fix duplicate textures and delete textures in tri file(s) \n" );
   fprintf( stderr, "                  pass texture id's as id:01234567 \n" );
   fprintf( stderr, "\n" );
}

int main( int argc, char **argv )
{
	int      get_output_file = 0;
   int      get_input_files = 0;
   unsigned int get_added_textures = 0;
	char	   *output_file;
   char     *input_files[256];
   char     *added_textures[256];
	ECLMode  mode = kCLMode_None;
   int      option_flag = 0 ;
   int      extract = FALSE;
   EExtractWhat extractWhat = kExtractTextures;
   EGame    game = kGame_MGS2;
   EGetStats getStats = kGetStats_None;
   ETextureName useWhat = kUseName;
   int      bp_rebuild = FALSE;
   int      as_fixdupes = FALSE;
   int      as_addtextures = FALSE;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ )
   {
		if ( argv[0][0] == '-' && strlen(argv[0]) > 1 )
      {
         mode = kCLMode_None;
         switch (argv[0][1])
         {
         case 'o':   mode = kCLMode_OutputFiles; break;
         case 'i':   mode = kCLMode_InputFiles; break;
         case 't':   option_flag = TRI_FLAG_TRANS; break;
         case 's':   option_flag = TRI_FLAG_STATIC; break;
         case 'l':   option_flag = TRI_FLAG_LATTERDRAW; break;
         case 'c':   option_flag = TRI_FLAG_COMPRESS; break;
         case 'x':   extract = TRUE; break;
         case '3':   game = kGame_MGS3; break;
         case 'q':   getStats = kGetStats_Tex; break;
         case 'u':   getStats = kGetStats_UniqueTex; break;
         case 'd':   useWhat = kUseId; break;
         case 'p':   extractWhat = kExtractSheets; break;
         case 'b':   bp_rebuild = TRUE; break;
         case 'a':   as_addtextures = TRUE; mode = kCLMode_GetTextures; break;
         case 'f':   as_fixdupes = TRUE; mode = kCLMode_GetTextures; break;
         }
		}
      else
      {
			switch ( mode )
         {
         case kCLMode_OutputFiles:/* 出力ファイル取得 */
				output_file = argv[0];
				get_output_file += 1;
				break;
         case kCLMode_InputFiles:/* 入力ファイル取得 */
				if ( get_input_files < 256 )
            {
               input_files[get_input_files++] = argv[0];
            }
				break;
         case kCLMode_GetTextures:
            if (get_added_textures < 256)
            {
               added_textures[get_added_textures++] = argv[0];
            }
				break;
			}
		}
	}
   if (getStats != kGetStats_None)
   {
      int i;
      if (get_input_files < 1)
      {
         GatherTextureStatsFromTRIFiles(game, getStats);
      }
      else
      {
         for (i = 0; i < get_input_files; ++i)
         {
            GatherTextureStatsFromTRIFile(input_files[i], game, getStats);
         }
      }
      return 0;
   }
   if (extract)
   {
      int i;
      if (get_input_files < 1)
      {
         ReadTextureRowImageFiles(game, useWhat, extractWhat);
         return -1;
      }
      for (i = 0; i < get_input_files; ++i)
      {
         ReadTextureRowImageFile(input_files[i], game, useWhat, extractWhat);
      }
      return 0;
   }
   else if( bp_rebuild )
   {
      int i;
      for (i = 0; i < get_input_files; ++i)
      {
         BP_RebuildTextureRowImageFile(input_files[i], game);
      }
      return 0;
   }
   if (as_addtextures && get_added_textures > 0 && get_input_files > 0)
   {
      int i;
      for (i = 0; i < get_input_files; ++i)
      {
         BP_AddTextureToTextureRowImageFile(input_files[i], added_textures, get_added_textures, game);
      }
      return 0;
   }
   if ( as_fixdupes && get_input_files > 0 )
   {
      int i;
      for ( i = 0; i < get_input_files; ++i )
      {
         printf( "Checking: %s\n", input_files[i] );
         BP_FixDupesAndRemoveFilesInTRI( input_files[i], added_textures, get_added_textures );
      }
      return 0;
   }

	/* エラーチェック */
	if ( get_output_file != 1 || get_input_files < 1 || get_input_files > 255 )
   {
      usage();
      return -1;
	}
   return ProgMain( output_file, get_input_files, input_files, option_flag );
}
