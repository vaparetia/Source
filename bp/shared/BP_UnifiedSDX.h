#pragma once
#ifndef __BP_UNIFIEDSDX__H
#define __BP_UNIFIEDSDX__H

#ifdef __cplusplus
extern "C" {
#endif

extern void BP_LoadUnifiedSDXArchive();
extern void BP_BeginBackgroundLoadSDXArchive();
extern void BP_JoinBackgroundLoadSDXArchive();

extern int BP_LoadPakFileFromArchive(int soundCode);

extern int BP_DoesPakFileExistInUnifiedArchive(char const* const pSdxPath);

extern void BP_CloseUnifiedSDXArchive();

// Special one off sound effect that plays when the AP sensor is pinged. Previously, MGS2 relied on vibrating the 
// controller when the AP sensor was pinged. Instead we animate the icon and play a sound effect. The sound effect is 
// stored as a tiny wav file in the misc. bp archive file and is loaded when the application starts. (the wav file is 
// only 5,196 bytes.
extern void BP_PlayAPSensorPing();

#ifdef __cplusplus
};
#endif

#endif
