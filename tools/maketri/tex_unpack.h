#ifndef __TEX_UNPACK_H__
#define __TEX_UNPACK_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
   kGame_MGS2,
   kGame_MGS3
}
EGame;

typedef enum
{
   kUseId,
   kUseName
}
ETextureName;

typedef enum
{
   kExtractTextures,
   kExtractSheets
}
EExtractWhat;

typedef enum
{
   kGetStats_None,
   kGetStats_Tex,
   kGetStats_UniqueTex
}
EGetStats;

void ReadTextureRowImageFiles( EGame game, ETextureName useWhat, EExtractWhat extractWhat );
void ReadTextureRowImageFile( char *filename, EGame game, ETextureName useWhat, EExtractWhat extractWhat );

void GatherTextureStatsFromTRIFiles( EGame game, EGetStats getStats );
void GatherTextureStatsFromTRIFile( char *filename, EGame game, EGetStats getStats );
void PrintTextureStats();
void BP_RebuildTextureRowImageFile( char *filename, EGame game );

void BP_AddTextureToTextureRowImageFile( char *filename, char *textureFilenames[], unsigned int numTextures, EGame game );
void BP_FixDupesAndRemoveFilesInTRI( char const *filename, char *textureFilenames[], unsigned int numTextures );

#ifdef __cplusplus
};
#endif

#endif // __TEX_UNPACK_H__