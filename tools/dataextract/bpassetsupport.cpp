#include "bpassetsupport.h"

#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <vector>
#include <map>
#include <algorithm>

#include "zlib.h"
#include "kp.h"
#include "dataextract.h"
#include "dumpassets.h"

//----------------------------------------------------------------------------

typedef struct { unsigned char r, g, b, cd ; } CVECTOR ;
typedef struct { short vx, vy, vz, pad ; } SVECTOR ;
typedef struct { int vx, vy, vz, vw ; } IVECTOR ;
typedef struct { float vx, vy, vz, vw ; } FVECTOR ;
typedef struct { float m[4][4] ; } FMATRIX ;

typedef struct _dg_dmatag{
   u_int	qwc ;
   void	*addr ;
   u_int	vifcode[2] ;
} DG_DMATAG ;

typedef struct _dg_gsreg {
   unsigned long long	data ;
   unsigned long long	reg ;
} DG_GSREG ;

typedef struct _dg_giftag{
   unsigned long long	tag ;
   unsigned long long	regs ;
} DG_GIFTAG ;

typedef struct _dg_vifcode {
   u_int	vifcode[4] ;
} DG_VIFCODE ;

typedef struct _dg_tex_trans {
   DG_GIFTAG	giftag ;
   DG_GSREG	clamp ;
   DG_GSREG	tex2 ;
   DG_GSREG	tex0 ;
   DG_GSREG	alpha ;
   DG_GSREG	pad[1] ;
   FVECTOR		vec1 ;
   FVECTOR		vec2 ;
} DG_TEX_TRANS ;

typedef struct _dg_tex {
   float				u_offset ;
   float				v_offset ;
   float				u_scale ;
   float				v_scale ;
   unsigned int		tex_id ;
   unsigned int		tri_id ;
   int					flag ;
   int					pad[1] ;
   DG_TEX_TRANS		tex_trans ;
} DG_TEX ;

typedef struct _tri_fileheader {
   unsigned long		tex_offset ;
   unsigned long		tex_size ;
   unsigned long		clut_offset ;
   unsigned long		clut_size ;
   unsigned long		n_textures ;
   unsigned long		compress_flag ;
   unsigned long		texel_addr ;
   unsigned long		clut_addr ;
} TRI_FILEHEADER ;

typedef struct {
   int			data_offset ;
   int			data_id ;
   int			pad[ 2 ] ;
} ZAR_LIST;

typedef struct {
   int				format_type ;	/*   */
   int				version ;		/* 0 */
   int				type ;			/* 0 */
   int				n_datas ;
   ZAR_LIST	list[ 0 ];
} ZAR_HEADER ;

//----------------------------------------------------------------------------

struct STriInfo
{
   unsigned int                  mTriId;
   std::vector< unsigned int >   mTexIds;
   std::vector< std::string >    mTexFilenames;
};

struct SAssetDupeCount
{
   std::string mFilename;
   int         mCount;

   bool operator < ( const SAssetDupeCount & rhs ) const
   {
      //sort descending!
      return mCount > rhs.mCount;
   }
};

typedef std::map< std::string, STriInfo > TTriInfoMap;
typedef std::map< std::string, std::string > TStringStringMap;
typedef std::map< std::string, SAssetDupeCount >  TAssetDupeMap;
typedef std::map< std::string, TAssetDupeMap >     TAllAssetDupeMaps;

namespace
{
   bool sbUsingTexFlatlistRemapping = false;
   bool sbUsingAssetRemapping = false;
   TStringStringMap sSrcToFlatFilenameMap;
   TStringStringMap sAssetMap;
   TTriInfoMap sTriInfoMap;
   TAllAssetDupeMaps sAllAssetDupeMaps;

   const char * const skGlobalAssetLabel = "_GLOBAL";
   char sPlatformSubfolder[FILENAME_MAX] = "_ps3";
}

void SetPlatformSubfolder( const char * subfolder )
{
   strcpy( sPlatformSubfolder, subfolder );
}

const char * GetPlatformSubfolder()
{
   return sPlatformSubfolder;
}

void InsertPlatformSubfolder( char * fullPath )
{
   char justFilename[FILENAME_MAX];
   char * lastSlash = fullPath + strlen(fullPath);
   while( *lastSlash != '/' )
      --lastSlash;
   strcpy( justFilename, lastSlash+1 );
   strcpy( lastSlash+1, GetPlatformSubfolder() );
   strcat( fullPath, "/" );
   strcat( fullPath, justFilename );
}

//----------------------------------------------------------------------------

bool UsingTextureFlatlistRemapping()
{
   return sbUsingTexFlatlistRemapping;
}

static char *fgets_wrapper( char *s, int max_count, FILE *fp )
{
   char *ret = fgets( s, max_count - 1, fp );
   if ( ret )
   {
      int len = strlen( ret );
      if ( len > 0 && ret[len-1] == '\n' )
      {
         ret[ len-1 ] = 0;
      }
   }

   return ret;
}

void InitTriInfo( const char * const texFlatlistFilename )
{
   if( !texFlatlistFilename )
   {
      return;
   }

   sbUsingTexFlatlistRemapping = true;
   FILE * fp = fopen( texFlatlistFilename, "rt" );
   if( !fp )
   {
      printf( "%s could not be opened for reading!\n", texFlatlistFilename );
      throw kAbortException;
   }

   char line[_MAX_PATH * 2 + 1];
   
   if ( fgets_wrapper( line, sizeof( line ), fp ) )
   {
      if ( strcmp( line, "flatmapv2" ) )
      {
         printf( "%s is not the right version of the flat map\n", texFlatlistFilename );
         throw kAbortException;
      }
   }
   else
   {
      printf( "%s contains no data\n", texFlatlistFilename );
      throw kAbortException;
   }

   while ( fgets_wrapper( line, sizeof( line ), fp ) )
   {
      char *space = strchr( line, ' ' );
      if ( space == NULL )
      {
         continue;
      }

      // Turn the space into a NULL to end the string, and we have both sides
      *space = 0;
      char *lhs = line;
      char *rhs = space + 1;

      if ( false == sSrcToFlatFilenameMap.insert( TStringStringMap::value_type( lhs, rhs ) ).second )
      {
         printf( "Error: Same id mapped to multiple flat filenames.\n" );
         throw kAbortException;
      }
   }
}

//----------------------------------------------------------------------------

void AddTri( const char * const filename )
{
   if( !sbUsingTexFlatlistRemapping )
   {
      return;
   }

   TRI_FILEHEADER	header;
   DG_TEX *tex;
   unsigned int i;
   unsigned int tri_id;

   TAssetDupeMap & allAssetDupeMap = sAllAssetDupeMaps[skGlobalAssetLabel];

   sscanf( filename + strlen( filename ) - 12, "%08x.tri", &tri_id );
   char stagePath[FILENAME_MAX];
   strncpy( stagePath, filename, strlen( filename ) - 13 );
   stagePath[strlen(filename)-13] = '\0';

   TTriInfoMap::iterator foundIt = sTriInfoMap.find( filename );
   if( foundIt != sTriInfoMap.end() )
   {
      //This case can occur when processing both STAGE.DAT and STAGE2.DAT together
      //as they both contain certain stages.  We still want the duplication information
      //to be global so it is recorded below regardless.
//      printf("Already present in tri map: %s\n", filename );
   }
   else
   {
      //Fill in information for this tri file.
      STriInfo & triInfo = sTriInfoMap[filename];
      triInfo.mTriId = tri_id;

      FILE *fp = fopen(filename, "rb");
      if (fp == NULL)
      {
         printf( "TRI file '%s' could not be opened!\n", filename);
         throw kAbortException;
      }
      else
      {
         fread(&header, sizeof(TRI_FILEHEADER), 1, fp);

         triInfo.mTexIds.reserve( header.n_textures );

         tex = (DG_TEX *) malloc(sizeof(DG_TEX)*header.n_textures);
         if (tex == NULL)
         {
            printf( "Could not allocate %d texture headers!\n", header.n_textures);
         }
         else
         {
            fread(tex, sizeof(DG_TEX), header.n_textures, fp);
            for (i = 0; i < header.n_textures; ++i)
            {
               const unsigned int tex_id = tex[i].tex_id;
               triInfo.mTexIds.push_back( tex_id );
               char texFilename[FILENAME_MAX];
               char texIdName[9];
               sprintf( texIdName, "%08x", tex_id );
               strcpy( texFilename, filename );
               strcpy( texFilename + strlen( texFilename ) - 4, "/" );
               strcat( texFilename, texIdName );
               strcat( texFilename, ".tga" );
               triInfo.mTexFilenames.push_back( texFilename );
            }
            free(tex);
         }
         fclose(fp);
      }
   }

   //Handle accounting in tristats
   {
      const STriInfo & triInfo = sTriInfoMap[filename];
      for( int i=0; i < (int)triInfo.mTexFilenames.size(); ++i )
      {
         const std::string & texFilename = triInfo.mTexFilenames[i];
         TStringStringMap::const_iterator found = sSrcToFlatFilenameMap.find( texFilename );
         if( found == sSrcToFlatFilenameMap.end() )
         {
            printf("Error: not found %s\n", texFilename.c_str() );
            continue;
         }
         TAssetDupeMap & stageAssetDupeMap = sAllAssetDupeMaps[stagePath];
         SAssetDupeCount & allAssetCount = allAssetDupeMap[found->second];
         if( allAssetCount.mFilename.empty() )
         {
            allAssetCount.mFilename = found->second;
            allAssetCount.mCount = 0;
         }
         ++allAssetCount.mCount;
         SAssetDupeCount & stageAssetCount = stageAssetDupeMap[found->second];
         if( stageAssetCount.mFilename.empty() )
         {
            stageAssetCount.mFilename = found->second;
            stageAssetCount.mCount = 0;
         }
         ++stageAssetCount.mCount;
      }
   }
}

std::string get_texture_flat_ctxr( const char * const srcFilename )
{
   char texFilename[FILENAME_MAX];
   TStringStringMap::const_iterator found = sSrcToFlatFilenameMap.find( srcFilename );
   if( found == sSrcToFlatFilenameMap.end() )
   {
      printf("Warning: no mapping found for %s\n", srcFilename );
      
      const char* fileName = strrchr( srcFilename, '/' );
      if ( !fileName )
      {
         fileName = srcFilename;
         texFilename[ 0 ] = '\0';
      }
      else
         strcpy( texFilename, "textures/flatlist" );
      strcat( texFilename, fileName );
      char* ext = strrchr( texFilename, '.' );
      strcpy( ext, ".ctxr" );
      //throw kAbortException;
      return texFilename;
   }
   else
   {
      sprintf( texFilename, "textures/flatlist/%s.ctxr", found->second.c_str() );
   }
   return texFilename;
}

static std::string const get_archive_texture_from_input_and_flat( char const * const triFilename, char const * const flatPath )
{
   // Build the "archive path" from the flat path and the triFilename
   // If triFilename is region/blahblah/trifilename
   // then we need to make the archive path blahblah/flatFileName

   char const *flatPathFilenamePart = strrchr( flatPath, '/' ) + 1;
   if ( triFilename[2] != '/' )
   {
      printf( "ERROR: get_archive_texture_from_input_and_flat expected to start with 2 letter rgn: %s\n", triFilename );
      throw kAbortException;
   }

   char const *triFilenameNewBegin = triFilename + 3;
   char const *triFilenameNewEnd = strrchr( triFilename, '/' );

   return std::string( triFilenameNewBegin, triFilenameNewEnd ) + '/' + flatPathFilenamePart;
}

void GetTriCtxrFilenames( const char * const triFilename, std::vector< SBPAssetLine > & outAssets )
{
   TTriInfoMap::const_iterator found = sTriInfoMap.find( triFilename );
   if( found == sTriInfoMap.end() )
   {
      printf("Internal error: tri file not found: %s\n", triFilename );
      throw kAbortException;
   }
   const STriInfo & triInfo = found->second;
   outAssets.assign( triInfo.mTexIds.size(), SBPAssetLine() );
   
   for (int i=0; i < (int)triInfo.mTexIds.size(); ++i )
   {
      const unsigned int tex_id = triInfo.mTexIds[i];
      const std::string & texFilename = triInfo.mTexFilenames[i];

      std::string flatPath = get_texture_flat_ctxr( texFilename.c_str() );


      char stagePath[FILENAME_MAX];
      strcpy( stagePath, found->first.c_str() );
      stagePath[found->first.size() - 4] = '\0';
      strcat( stagePath, "/" );
      sprintf( stagePath + strlen( stagePath ), "%08x.ctxr", tex_id );

      outAssets[i].mArchivePath = get_archive_texture_from_input_and_flat( triFilename, flatPath.c_str() );
      outAssets[i].mFlatPath = flatPath;
      outAssets[i].mStagePath = stagePath;
   }
}

void GetImgCtxrFilenames( const char * const imgFilename, SBPAssetLine &outAsset )
{
   char stagePath[FILENAME_MAX];
   strcpy( stagePath, imgFilename );
   stagePath[strlen( imgFilename ) - 12] = '\0';
   strcat( stagePath, imgFilename + strlen( imgFilename ) - 12 );
   strcat( stagePath, ".ctxr" );
   
   outAsset.mStagePath = stagePath;

   std::string texFilename = std::string( imgFilename ) + ".tga";

   outAsset.mFlatPath = get_texture_flat_ctxr( texFilename.c_str() );

   outAsset.mArchivePath = get_archive_texture_from_input_and_flat( imgFilename, outAsset.mFlatPath.c_str() );

}

std::string GetCmdlFilename( const char * const mdlFilename )
{
   char const *extstart = strrchr( mdlFilename, '.' );
   if ( extstart == NULL )
   {
      printf( "ERROR: Couldn't get extension for %s", mdlFilename );
      throw kAbortException;
   }

   return std::string( mdlFilename, extstart ) + ".cmdl";
}

void DumpTriStats( const char * const texDupeStatsFilename )
{
   //Write out an xml file summarizing texture duplication counts, both per stage and globally.
   FILE * fp = fopen( texDupeStatsFilename, "wt" );
   if( !fp )
   {
      printf( "tristats.xml could not be opened for writing!\n");
      throw kAbortException;
   }
   fprintf( fp, "<stats>\n" );
   for( TAllAssetDupeMaps::const_iterator it = sAllAssetDupeMaps.begin(); it != sAllAssetDupeMaps.end(); ++it )
   {
      const TAssetDupeMap & assetMap = it->second;
      fprintf( fp, "\t<stage name=\"%s\">\n", it->first.c_str() );

      std::vector< SAssetDupeCount > sortedAssets;
      sortedAssets.reserve( assetMap.size() );
      for( TAssetDupeMap::const_iterator at = assetMap.begin(); at != assetMap.end(); ++at )
      {
         const SAssetDupeCount & assetCount = at->second;
         sortedAssets.push_back( assetCount );
      }
      std::sort( sortedAssets.begin(), sortedAssets.end() );
      for( std::vector< SAssetDupeCount >::const_iterator at = sortedAssets.begin(); at != sortedAssets.end(); ++at )
      {
         const SAssetDupeCount & assetCount = *at;
         fprintf( fp, "\t\t<tex count=\"%d\" name=\"%s\"/>\n", assetCount.mCount, assetCount.mFilename.c_str() );
      }

      fprintf( fp, "\t</stage>\n", it->first.c_str() );

      if( !strcmp( it->first.c_str(), skGlobalAssetLabel ) )
      {
         //Build a csv file for just the global section so we can look at numbers most easily in a spreadsheet app.
         std::string csvFilename( texDupeStatsFilename );
         csvFilename += ".global.csv"; //eh

         if( const char * const bpe_repository = getenv( "BPE_REPOSITORY" ) )
         {
            if( FILE * csvfp = fopen( csvFilename.c_str(), "wt" ) )
            {
               for( std::vector< SAssetDupeCount >::const_iterator at2 = sortedAssets.begin(); at2 != sortedAssets.end(); ++at2 )
               {
                  const SAssetDupeCount & assetCount = *at2;
                  int compressedSize = 0;
                  char compressedFullPath[FILENAME_MAX];
                  sprintf( compressedFullPath, "%s\\xbarc_temp\\textures\\flatlist\\_360\\%s.ctxr.xbc", bpe_repository, assetCount.mFilename.c_str() );

                  WIN32_FILE_ATTRIBUTE_DATA fileAttr = { 0 };
                  BOOL b = GetFileAttributesExA( compressedFullPath, GetFileExInfoStandard, &fileAttr );
                  if( b )
                  {
                     compressedSize = fileAttr.nFileSizeLow;
                  }

                  fprintf( csvfp, "%d,%s,%d\n", assetCount.mCount, assetCount.mFilename.c_str(), compressedSize );
               }
               fclose( csvfp );
            }
         }
      }
   }
   fprintf( fp, "</stats>\n" );
   fclose( fp );
}

//----------------------------------------------------------------------------

void InitAssetRemap( const char * const assetRemapFilename )
{
   if( !assetRemapFilename )
   {
      return;
   }

   sbUsingAssetRemapping = true;
   FILE * fp = fopen( assetRemapFilename, "rt" );
   if( !fp )
   {
      printf( "%s could not be opened for reading!\n", assetRemapFilename );
      throw kAbortException;
   }

   char line[_MAX_PATH * 2 + 1];

   if ( fgets_wrapper( line, sizeof( line ), fp ) )
   {
      if ( strcmp( line, "assetmapv2" ) )
      {
         printf( "%s is not the right version of the asset map\n", assetRemapFilename );
         throw kAbortException;
      }
   }
   else
   {
      printf( "%s contains no data\n", assetRemapFilename );
      throw kAbortException;
   }

   while ( fgets_wrapper( line, sizeof( line ), fp ) )
   {
      char *space = strchr( line, ' ' );
      if ( space == NULL )
      {
         continue;
      }

      // Turn the space into a NULL to end the string, and we have both sides
      *space = 0;
      char *lhs = line;
      char *rhs = space + 1;

      if ( false == sAssetMap.insert( TStringStringMap::value_type( lhs, rhs ) ).second )
      {
         printf( "Error: Same id mapped to multiple flat filenames.\n" );
         throw kAbortException;
      }
   }
}

std::string const GetAssetRemappedName( char const * const srcFilename )
{
   TStringStringMap::const_iterator found = sAssetMap.find( srcFilename );
   if( found == sAssetMap.end() )
   {
      printf("Error: no mapping found for %s\n", srcFilename );
//      throw kAbortException;
      return srcFilename;
   }
   else
   {
      return "assets/" + found->second;
   }

}

static inline char const *strchr_throw( char const *s, char c )
{
   char const *ret = strchr( s, c );

   if ( ret == NULL )
   {
      printf( "ERROR: Couldn't find '%c' in '%s'\n", c, s );
      throw kAbortException;
   }

   return ret;
}

std::string const GetAssetArchiveFilename( char const * const srcFilename, char const * const remapped )
{
   // We want to grab the region from the remapping

   char const *srcFilenameSlash = strchr_throw( srcFilename, '/' );

   if ( srcFilename + 2 != srcFilenameSlash )
   {
      printf( "ERROR: '%s' doesn't start with region/\n", srcFilename );
      throw kAbortException;
   }

   int const tokenCount = 2;
   char const *currentRemapBegin = remapped;
   char const *currentRemapEnd = strchr_throw( remapped, '/' );

   for ( int i = 0; i < tokenCount; ++i )
   {
      currentRemapBegin = currentRemapEnd + 1;
      currentRemapEnd = strchr_throw( currentRemapBegin, '/' );
   }

   if ( currentRemapBegin + 2 != currentRemapEnd )
   {
      printf( "ERROR: '%s' 's third token '%s' is not 2 chars\n", remapped, std::string( currentRemapBegin, currentRemapEnd ).c_str() );
      throw kAbortException;
   }

   // Now use the remapped region for the "archive" name

   return std::string( currentRemapBegin, currentRemapEnd ) + srcFilenameSlash;
}

bool UsingAssetRemapping()
{
   return sbUsingAssetRemapping;
}


bool SBPAssetLine::operator < ( const SBPAssetLine & rhs ) const
{
   if ( mFlatPath == rhs.mFlatPath )
   {
      if ( mArchivePath == rhs.mArchivePath )
      {
         return mStagePath < rhs.mStagePath;
      }
      else
      {
         return mArchivePath < rhs.mArchivePath;
      }
   }
   else
   {
      // Different flat paths, let's sort by ext

      const std::string lhsExt = mFlatPath.substr( mFlatPath.find_last_of( '.' ) );
      const std::string rhsExt = rhs.mFlatPath.substr( rhs.mFlatPath.find_last_of( '.' ) );
      
      if ( lhsExt != rhsExt )
      {
         if( lhsExt == ".ctxr" )
         {
            //always sort ctxr's at the top.
            return true;
         }
         else if( rhsExt == ".ctxr" )
         {
            //always sort ctxr's at the top.
            return false;
         }
      }

      // After extension, sort by flat name
      return mFlatPath < rhs.mFlatPath;
   }
}

#if MGS_VERSION == 2

std::string GetZmsCmdlFilePath(const char * const zmsPath, int kmsId)
{
   char cmdlPath[FILENAME_MAX];

   char kmsIdName[9];
   sprintf( kmsIdName, "%08x", kmsId );

   strcpy( cmdlPath, zmsPath );
   strcpy( cmdlPath + strlen( zmsPath ) - 4, "/" );
   strcat( cmdlPath, kmsIdName );
   strcat( cmdlPath, ".cmdl");
   
   return cmdlPath;
}

void GetZmsCmdlFilenames( const char * const filename, std::vector< SBPAssetLine > & outAssets )
{
   FILE *fp = fopen(filename, "rb");

   if (fp == NULL)
   {
      printf( "ZMS file '%s' could not be opened!\n", filename);
      throw kAbortException;
   }
   else
   {
      ZAR_HEADER header;
      fread(&header, sizeof(ZAR_HEADER), 1, fp);

      outAssets.assign( header.n_datas, SBPAssetLine() );

      ZAR_LIST* entries = (ZAR_LIST*)malloc(sizeof(ZAR_LIST)*header.n_datas);

      fread(entries, sizeof(ZAR_LIST), header.n_datas, fp);

      for (int i = 0; i < header.n_datas; ++i)
      {
         unsigned int kmsId = entries[i].data_id;

         std::string archivePath = GetZmsCmdlFilePath(GetAssetArchiveFilename( filename, GetAssetRemappedName( filename ).c_str() ).c_str(), kmsId);
         std::string flatPath = GetZmsCmdlFilePath(GetAssetRemappedName( filename ).c_str(), kmsId);
         std::string stagePath = GetZmsCmdlFilePath(filename, kmsId);

         //FLAT: textures/flatlist/000385a3.ctxr
         //STAGE: stage/a00a/cache/000385a3.ctxr
         //ARCHIVE: us/stage/a00a/cache/00fc06e8/000385a3.ctxr

         outAssets[i].mArchivePath = archivePath;
         outAssets[i].mFlatPath = flatPath;
         outAssets[i].mStagePath = stagePath;
      }
      
      free(entries);

      fclose(fp);
   }
}

#endif