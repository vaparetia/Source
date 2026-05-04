/*--------------------------------------------------------------*/
/*	xbox2win_taka.h												*/
/*					X-BoxからWindowsに変換する為のいろいろ。	*/
/*
	$Id: 
*/
/*--------------------------------------------------------------*/
#ifndef	__XBOX2WIN_YOSI_H__
#define	__XBOX2WIN_YOSI_H__

/*--------------------------------------------------------------*/
/*	とりあえずの置換用											*/
/*--------------------------------------------------------------*/
//typedef	WIN_DUMMY_PARAM_STRUCT	PDM_CMDCONT ;
	
typedef struct _DSI3DL2OBSTRUCTION{
	LONG lHFLevel;
	FLOAT flLFRatio;
} DSI3DL2OBSTRUCTION, *LPDSI3DL2OBSTRUCTION;
typedef struct _DSI3DL2OCCLUSION{
	LONG lHFLevel;
	FLOAT flLFRatio;
} DSI3DL2OCCLUSION, *LPDSI3DL2OCCLUSION;
typedef	struct {
	LONG lDirect;
	LONG lDirectHF;
	LONG lRoom;
	LONG lRoomHF;
	FLOAT flRoomRolloffFactor;
	DSI3DL2OBSTRUCTION Obstruction;
	DSI3DL2OCCLUSION Occlusion;
}DSI3DL2BUFFER,*LPDSI3DL2BUFFER;

typedef struct xbox_adpcmwaveformat_tag {
	WAVEFORMATEX wfx;
	WORD wSamplesPerBlock;
} XBOXADPCMWAVEFORMAT, *PXBOXADPCMWAVEFORMAT, *LPXBOXADPCMWAVEFORMAT;

extern void SoundDebugOutYoshizawa(void);
extern void SoundDebugOutYoshizawa2(void);
extern void SoundDebugMainYoshizawa(int key);
extern BOOL SetPriorityClassMacro(DWORD p);

#endif	/* !defined(__XBOX2WIN_TAKA_H__) */

/*-- End Of File --*/
