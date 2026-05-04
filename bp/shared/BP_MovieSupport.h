//----------------------------------------------------------------------------
// BP_MovieSupport.h
//----------------------------------------------------------------------------

#pragma once

#include "MGS_Common.h"

#ifdef __cplusplus
extern "C" {
#endif

enum MovieState 
{
   MOVIE_STATE_VALID,
   MOVIE_STATE_WAITING,
   MOVIE_STATE_READY,
   MOVIE_STATE_BUFFER_INIT,
   MOVIE_STATE_BUFFER,
   MOVIE_STATE_PLAYING,
   MOVIE_STATE_PAUSED,
   MOVIE_STATE_STOPPED,
   MOVIE_STATE_DESTROYED,
   MOVIE_STATE_INVALID = -1
};

typedef unsigned int BP_Movie_Handle;
typedef void(*BP_Movie_Render_Callback)(unsigned char* pBuffer, int* pOutBytesRead);

extern void BP_Movie_InitDebugMenu();

extern void BP_Movie_Init();
extern void BP_Movie_Shutdown();

extern BP_Movie_Handle BP_Movie_Create(int x, int y, int width, int height, DG_TEX_TRANS* pTexTrans, BP_Movie_Render_Callback renderCallback, int isFullscreenMovie, char* pMemoryStream, int memorySize);

extern void BP_Movie_RenderActiveMovies(int const shouldDraw);

extern void BP_Movie_Destroy(BP_Movie_Handle hMovie);

extern void BP_Movie_Set_Time(BP_Movie_Handle hMovie, float time);
extern float BP_Movie_Get_Frame_Time(BP_Movie_Handle hMovie);

extern void BP_Movie_Start(BP_Movie_Handle hMovie, unsigned int movieDataSizeInBytes);

extern void BP_Movie_Stop(BP_Movie_Handle hMovie);
extern void BP_Movie_Reset(BP_Movie_Handle hMovie);

extern int BP_Movie_Get_State(BP_Movie_Handle hMovie);

extern DG_TEX* BP_Movie_Get_Texture(BP_Movie_Handle hMovie);
extern void BP_Movie_ResetTextureToBlack(BP_Movie_Handle hMovie);

// Will return 1 if the movie is a fullscreen movie.
extern int BP_Movie_GetMovieCoordinates(int x, int y, int width, int height, float* x0, float* y0, float* x1, float* y1);

extern void BP_Movie_SetCinemabarsDisabled(int moviePlayerUniqueId, int enabled);
extern int BP_Movie_IsCinemabarsDisabled();

extern BP_Movie_Handle BP_Movie_CreateStandalonePlayer(const char* pFilename);
void BP_Movie_DestroyStandalonePlayer(BP_Movie_Handle handle);
void BP_Movie_StandalonePlayer_Play(BP_Movie_Handle handle);

extern int BP_Movie_IsPlayingFullscreen();

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

