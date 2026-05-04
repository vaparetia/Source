/*--------------------------------------------------------------*/
/*	xbox2win.h													*/
/*					X-BoxからWindowsに変換する為のいろいろ。	*/
/*--------------------------------------------------------------*/
#ifndef	__XBOX2WIN_H__
#define	__XBOX2WIN_H__

#include "error_util.h"
#include "error_code.h"

#ifdef	__XBOX2WIN_C__
#define	EXTERN
#else
#define	EXTERN	extern
#endif

/*--------------------------------------------------------------*/
/*	とりあえずの置換用											*/
/*--------------------------------------------------------------*/
typedef	struct	_WIN_DUMMY_PARAM_STRUCT
{
	DWORD	dmy;
} WIN_DUMMY_PARAM_STRUCT ;

typedef	DWORD					D3DVERTEXATTRIBUTEFORMAT ;
typedef	WIN_DUMMY_PARAM_STRUCT	D3DSTREAM_INPUT ;
typedef	WIN_DUMMY_PARAM_STRUCT	D3DBaseTexture ;
typedef	WIN_DUMMY_PARAM_STRUCT	D3DPalette ;
typedef	DWORD					D3DPALETTESIZE ;
typedef	DWORD					LPDIRECT3DPALETTE8 ;
typedef	WIN_DUMMY_PARAM_STRUCT	D3DPushBuffer ;
typedef	WIN_DUMMY_PARAM_STRUCT	D3DPIXELSHADERDEF_FILE ;
typedef	WIN_DUMMY_PARAM_STRUCT	D3DSurface ;
typedef	DWORD					PXCALCSIG_SIGNATURE ;
typedef	WIN_DUMMY_PARAM_STRUCT	PLAUNCH_DATA ;
typedef	WIN_DUMMY_PARAM_STRUCT	D3DResource ;
typedef	DWORD					IDirect3DPushBuffer8 ;
typedef	DWORD					D3DPIXELSHADERDEF ;
typedef	WIN_DUMMY_PARAM_STRUCT	DSMIXBINS, *LPDSMIXBINS ;
typedef	WIN_DUMMY_PARAM_STRUCT	DSMIXBINVOLUMEPAIR, *LPDSMIXBINVOLUMEPAIR;
//typedef	DWORD					LPCDSI3DL2BUFFER ;
//typedef	DWORD					LPCDSMIXBINS ;
//typedef	DWORD					LPCDSENVELOPEDESC ;
//typedef	WIN_DUMMY_PARAM_STRUCT	DSENVELOPEDESC, *LPDSENVELOPEDESC ;
//typedef	WIN_DUMMY_PARAM_STRUCT	XBOXADPCMWAVEFORMAT;
//typedef	WIN_DUMMY_PARAM_STRUCT	DSEFFECTIMAGELOC ;
//typedef	WIN_DUMMY_PARAM_STRUCT	*LPDSEFFECTIMAGELOC ;
//typedef	WIN_DUMMY_PARAM_STRUCT	DSEFFECTIMAGEDESC ;
//typedef	WIN_DUMMY_PARAM_STRUCT	*LPDSEFFECTIMAGEDESC ;

/*--------------------------------------------------------------*/
/*	D3DBLEND													*/
/*--------------------------------------------------------------*/
#define	D3D_AllocContiguousMemory(sz_, al_)	malloc(sz_)

/*--------------------------------------------------------------*/
/*	D3DBLEND													*/
/*--------------------------------------------------------------*/
#define	D3DTILE_ALIGNMENT	(4)
#define	D3D_AllocContiguousMemory(sz_, al_)	malloc(sz_)
#define	D3DPRESENTFLAG_10X11PIXELASPECTRATIO	(0)

/*--------------------------------------------------------------*/
/*	Lock flags													*/
/*--------------------------------------------------------------*/
#define D3DLOCK_TILED	0x00000000L

/*--------------------------------------------------------------*/
/*	DSBSTOPEX													*/
/*--------------------------------------------------------------*/
#define DSBSTOPEX_IMMEDIATE         0x00000000      // The buffer should stop immediately
#define DSBSTOPEX_ENVELOPE          0x00000001      // The buffer should enter it's release phase
#define DSBSTOPEX_RELEASEWAVEFORM   0x00000002      // The buffer should break out of the loop region 
/*--------------------------------------------------------------*/
/*	DSBPLAY														*/
/*--------------------------------------------------------------*/
//#define DSBPLAY_LOOPING             0x00000001      // The buffer should play in a loop
//#define DSBPLAY_FROMSTART           0x00000002      // Play the buffer from the beginning, regardless of current position

/*--------------------------------------------------------------*/
/*	DSP_IMAGE_DSSTDFX_FX_INDICES								*/
/*--------------------------------------------------------------*/
typedef enum _DSP_IMAGE_DSSTDFX_FX_INDICES {
    I3DL2_CHAIN_I3DL2_REVERB = 0,
    I3DL2_CHAIN_XTALK = 1,
    CHORUS_CHAIN_CHORUS = 2,
    FLANGE_CHAIN_FLANGE = 3,
    AMPMOD_CHAIN_AMPMOD = 4,
    ECHO_CHAIN_ECHO = 5,
    IIR2_CHAIN_IIR2_LEFT = 6,
    IIR2_CHAIN_IIR2_RIGHT = 7,
    REVERB_CHAIN_MINI_REVERB = 8
} DSP_IMAGE_DSSTDFX_FX_INDICES;

/*--------------------------------------------------------------*/
/*	MixBin identifiers											*/
/*--------------------------------------------------------------*/

#define DSMIXBIN_FRONT_LEFT         0
#define DSMIXBIN_FRONT_RIGHT        1
#define DSMIXBIN_FRONT_CENTER       2
#define DSMIXBIN_LOW_FREQUENCY      3
#define DSMIXBIN_BACK_LEFT          4
#define DSMIXBIN_BACK_RIGHT         5
#define DSMIXBIN_SPEAKERS_FIRST     DSMIXBIN_FRONT_LEFT
#define DSMIXBIN_SPEAKERS_LAST      DSMIXBIN_BACK_RIGHT
#define DSMIXBIN_SPEAKERS_COUNT     (DSMIXBIN_SPEAKERS_LAST - DSMIXBIN_SPEAKERS_FIRST + 1)
                                    
#define DSMIXBIN_XTLK_FRONT_LEFT    6
#define DSMIXBIN_XTLK_FRONT_RIGHT   7
#define DSMIXBIN_XTLK_BACK_LEFT     8
#define DSMIXBIN_XTLK_BACK_RIGHT    9
#define DSMIXBIN_XTLK_FIRST         DSMIXBIN_XTLK_FRONT_LEFT
#define DSMIXBIN_XTLK_LAST          DSMIXBIN_XTLK_BACK_RIGHT
#define DSMIXBIN_XTLK_COUNT         (DSMIXBIN_XTLK_LAST - DSMIXBIN_XTLK_FIRST + 1)
                                    
#define DSMIXBIN_I3DL2              10
                                    
#define DSMIXBIN_FXSEND_0           11
#define DSMIXBIN_FXSEND_1           12
#define DSMIXBIN_FXSEND_2           13
#define DSMIXBIN_FXSEND_3           14
#define DSMIXBIN_FXSEND_4           15
#define DSMIXBIN_FXSEND_5           16
#define DSMIXBIN_FXSEND_6           17
#define DSMIXBIN_FXSEND_7           18
#define DSMIXBIN_FXSEND_8           19
#define DSMIXBIN_FXSEND_9           20
#define DSMIXBIN_FXSEND_10          21
#define DSMIXBIN_FXSEND_11          22
#define DSMIXBIN_FXSEND_12          23
#define DSMIXBIN_FXSEND_13          24
#define DSMIXBIN_FXSEND_14          25
#define DSMIXBIN_FXSEND_15          26
#define DSMIXBIN_FXSEND_16          27
#define DSMIXBIN_FXSEND_17          28
#define DSMIXBIN_FXSEND_18          29
#define DSMIXBIN_FXSEND_19          30
#define DSMIXBIN_FXSEND_FIRST       DSMIXBIN_FXSEND_0
#define DSMIXBIN_FXSEND_LAST        DSMIXBIN_FXSEND_19
#define DSMIXBIN_FXSEND_COUNT       (DSMIXBIN_FXSEND_LAST - DSMIXBIN_FXSEND_FIRST + 1)
                                    
#define DSMIXBIN_SUBMIX             31

#define DSMIXBIN_FIRST              DSMIXBIN_FRONT_LEFT
#define DSMIXBIN_LAST               DSMIXBIN_SUBMIX
#define DSMIXBIN_COUNT              (DSMIXBIN_LAST - DSMIXBIN_FIRST + 1)
                                    
#define DSMIXBIN_3D_FRONT_LEFT      DSMIXBIN_XTLK_FRONT_LEFT
#define DSMIXBIN_3D_FRONT_RIGHT     DSMIXBIN_XTLK_FRONT_RIGHT
#define DSMIXBIN_3D_BACK_LEFT       DSMIXBIN_XTLK_BACK_LEFT
#define DSMIXBIN_3D_BACK_RIGHT      DSMIXBIN_XTLK_BACK_RIGHT
#define DSMIXBIN_3D_FIRST           DSMIXBIN_XTLK_FIRST
#define DSMIXBIN_3D_LAST            DSMIXBIN_XTLK_LAST
#define DSMIXBIN_3D_COUNT           DSMIXBIN_XTLK_COUNT






/*--------------------------------------------------------------*/
/*	XC_LANGUAGE													*/
/*--------------------------------------------------------------*/
enum {
	XC_LANGUAGE_JAPANESE = 1
	,XC_LANGUAGE_GERMAN
	,XC_LANGUAGE_FRENCH
	,XC_LANGUAGE_SPANISH
	,XC_LANGUAGE_ITALIAN
	,XC_LANGUAGE_ENGLISH
};
/*--------------------------------------------------------------*/
/*	個人作業用													*/
/*--------------------------------------------------------------*/
#include "xbox2win_taka.h"
#include "xbox2win_yosi.h"

/*--------------------------------------------------------------*/
/*	Dummy Func													*/
/*--------------------------------------------------------------*/
EXTERN DWORD XGetLanguage(void);

EXTERN HANDLE WINAPI XGetSectionHandleA(LPCSTR pSectionName);
#define XGetSectionHandle  XGetSectionHandleA
EXTERN PVOID WINAPI XLoadSectionByHandle( HANDLE hSection );
EXTERN DWORD WINAPI XGetSectionSize( HANDLE hSection );
EXTERN BOOL WINAPI XFreeSectionByHandle( HANDLE hSection );

EXTERN HANDLE	XCalculateSignatureBegin( DWORD dwFlags );
EXTERN DWORD	XCalculateSignatureEnd(  HANDLE hCalcSig,  PXCALCSIG_SIGNATURE pSignature);
EXTERN DWORD	XCalculateSignatureUpdate( HANDLE hCalcSig, const BYTE *pbData, ULONG cbData);
EXTERN VOID		IDirect3DTexture8_Register( LPDIRECT3DTEXTURE8* ppTex, void *pBase );

EXTERN DWORD	WINAPI XGSetTextureHeader(UINT Width, UINT Height, UINT  Levels, DWORD Usage,
							D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8 *pTexture,
							UINT Data, UINT Pitch);
EXTERN DWORD	WINAPI	XGSetSurfaceHeader( UINT Width, UINT Height, D3DFORMAT Format,
							IDirect3DSurface8 *pSurface, UINT Data,  UINT Pitch);

EXTERN DWORD XLaunchNewImage( LPCSTR lpTitlePath, PLAUNCH_DATA  pLaunchData );


EXTERN LPVOID XPhysicalAlloc(SIZE_T dwSize, ULONG_PTR ulPhysicalAddress,
							ULONG_PTR ulAlignment, DWORD flProtect) ;

EXTERN VOID WINAPI XPhysicalProtect(LPVOID lpAddress, SIZE_T dwSize,DWORD flNewProtect);

EXTERN BOOL __stdcall DmIsDebuggerPresent(void);
EXTERN VOID WINAPI XSetProcessQuantumLength( DWORD dwMilliseconds );

	/*----------------------------------------------------------*/
	/*	Direct3DDevice8											*/
	/*----------------------------------------------------------*/

EXTERN HRESULT	IDirect3DDevice8_Begin( LPDIRECT3DDEVICE8 g_pd3dDevice,
							D3DPRIMITIVETYPE PrimitiveType );
EXTERN HRESULT	IDirect3DDevice8_End(LPDIRECT3DDEVICE8 g_pd3dDevice );

EXTERN HRESULT	IDirect3DDevice8_SetVertexData2s(LPDIRECT3DDEVICE8 g_pd3dDevice,
							INT Register, SHORT a, SHORT b );
EXTERN HRESULT	IDirect3DDevice8_SetVertexData2f(LPDIRECT3DDEVICE8 g_pd3dDevice,
							INT Register, FLOAT a, FLOAT b );
EXTERN HRESULT	IDirect3DDevice8_SetVertexData4ub(LPDIRECT3DDEVICE8 g_pd3dDevice,
							INT Register, BYTE a, BYTE b, BYTE c, BYTE d );
EXTERN VOID		IDirect3DDevice8_BlockUntilIdle(LPDIRECT3DDEVICE8 g_pd3dDevice) ;
EXTERN VOID		IDirect3DDevice8_BlockUntilVerticalBlank(LPDIRECT3DDEVICE8 g_pd3dDevice);

EXTERN HRESULT WINAPI IDirect3DDevice8_EndPush(LPDIRECT3DDEVICE8 pThis, DWORD *pPush) ;
EXTERN HRESULT WINAPI IDirect3DDevice8_GetVertexShaderSize(LPDIRECT3DDEVICE8 pThis,
							DWORD Handle, UINT* pSize) ;
EXTERN HRESULT WINAPI IDirect3DDevice8_LoadVertexShaderProgram(LPDIRECT3DDEVICE8 pThis,
							CONST DWORD *pFunction, DWORD Address) ;
EXTERN HRESULT WINAPI IDirect3DDevice8_LoadVertexShaderProgram(LPDIRECT3DDEVICE8 pThis,
									CONST DWORD *pFunction, DWORD Address) ;

EXTERN HRESULT WINAPI IDirect3DDevice8_CreatePalette(LPDIRECT3DDEVICE8 pThis,
									D3DPALETTESIZE Size, D3DPalette **ppPalette) ;

EXTERN HRESULT WINAPI IDirect3DDevice8_SetPalette(LPDIRECT3DDEVICE8 pThis,
									DWORD Stage, D3DPalette *pPalette) ;

EXTERN HRESULT WINAPI IDirect3DDevice8_SetPixelShaderProgram(LPDIRECT3DDEVICE8 pThis,
									CONST D3DPIXELSHADERDEF *pPSDef) ;

EXTERN HRESULT WINAPI IDirect3DDevice8_BeginPush(LPDIRECT3DDEVICE8 pThis, DWORD Count, DWORD **ppPush) ;


	/*----------------------------------------------------------*/
	/*	Direct3D8												*/
	/*----------------------------------------------------------*/

	/*----------------------------------------------------------*/
	/*	Direct3DSurface8										*/
	/*----------------------------------------------------------*/
EXTERN HRESULT	IDirect3DSurface8_Register(LPDIRECT3DSURFACE8 g_pD3DSurface, void *addr );

	/*----------------------------------------------------------*/
	/*	Direct3DResource8										*/
	/*----------------------------------------------------------*/
EXTERN void    WINAPI IDirect3DResource8_Register(D3DResource *pThis, void *pBase) ;
EXTERN void    WINAPI D3DResource_Register(D3DResource *pThis, void *pBase);

	/*----------------------------------------------------------*/
	/*	Direct3DPushBuffer8										*/
	/*----------------------------------------------------------*/
EXTERN HRESULT WINAPI IDirect3DPushBuffer8_GetSize(D3DPushBuffer* pPushBuffer,
							DWORD* pSize) ;


	/*----------------------------------------------------------*/
	/*	DirectSound8											*/
	/*----------------------------------------------------------*/
//
//STDAPI IDirectSound_CommitDeferredSettings(LPDIRECTSOUND pDirectSound);
//STDAPI IDirectSound_DownloadEffectsImage(LPDIRECTSOUND pDirectSound,
//							LPCVOID pvImageBuffer, DWORD dwImageSize,
//							LPDSEFFECTIMAGELOC pImageLoc, LPDSEFFECTIMAGEDESC *ppImageDesc);
//
//STDAPI_(void) DirectSoundDoWork(void);
//STDAPI_(void) DirectSoundUseFullHRTF(void);
//STDAPI IDirectSound_SetMixBinHeadroom(LPDIRECTSOUND pDirectSound, DWORD dwMixBin, DWORD dwHeadroom);
//STDAPI IDirectSound_SetDistanceFactor(LPDIRECTSOUND pDirectSound, FLOAT flDistanceFactor, DWORD dwApply);
//STDAPI IDirectSound_SetRolloffFactor(LPDIRECTSOUND pDirectSound, FLOAT flRolloffFactor, DWORD dwApply);
//STDAPI IDirectSound_SetPosition(LPDIRECTSOUND pDirectSound, FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
//STDAPI IDirectSound_SetOrientation(LPDIRECTSOUND pDirectSound, FLOAT xFront, FLOAT yFront, FLOAT zFront, FLOAT xTop, FLOAT yTop, FLOAT zTop, DWORD dwApply);
//STDAPI IDirectSound_SetI3DL2Listener(LPDIRECTSOUND pDirectSound, LPCDSI3DL2LISTENER pds3dl, DWORD dwApply);
////STDAPI IDirectSoundBuffer_SetMode(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwMode, DWORD dwApply);
////STDAPI IDirectSoundBuffer_SetPosition(LPDIRECTSOUNDBUFFER pBuffer, FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
//
//
//	/*----------------------------------------------------------*/
//	/*	DirectSoundBuffer										*/
//	/*----------------------------------------------------------*/
//STDAPI IDirectSoundBuffer_StopEx(LPDIRECTSOUNDBUFFER pBuffer, REFERENCE_TIME rtTimeStamp, DWORD dwFlags);
//STDAPI IDirectSoundBuffer_SetMixBins(LPDIRECTSOUNDBUFFER pBuffer, LPCDSMIXBINS pMixBins);
//STDAPI IDirectSoundBuffer_SetEG(LPDIRECTSOUNDBUFFER pBuffer, LPCDSENVELOPEDESC pEnvelopeDesc);
//STDAPI IDirectSoundBuffer_SetPlayRegion(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwPlayStart, DWORD dwPlayLength);
//STDAPI IDirectSoundBuffer_SetLoopRegion(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwLoopStart, DWORD dwLoopLength);
////STDAPI IDirectSoundBuffer_SetBufferData(LPDIRECTSOUNDBUFFER pBuffer, LPVOID pvBufferData, DWORD dwBufferBytes);
////STDAPI IDirectSoundBuffer_SetDistanceFactor(LPDIRECTSOUNDBUFFER pBuffer, FLOAT flDistanceFactor, DWORD dwApply);
////STDAPI IDirectSoundBuffer_SetMinDistance(LPDIRECTSOUNDBUFFER pBuffer, FLOAT flMinDistance, DWORD dwApply);
////STDAPI IDirectSoundBuffer_SetMaxDistance(LPDIRECTSOUNDBUFFER pBuffer, FLOAT flMaxDistance, DWORD dwApply);
////STDAPI IDirectSoundBuffer_SetHeadroom(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwHeadroom);
////STDAPI IDirectSoundBuffer_SetRolloffFactor(LPDIRECTSOUNDBUFFER pBuffer, FLOAT flRolloffFactor, DWORD dwApply);

#define IDirectSoundBuffer_StopEx(pBuffer, rtTimeStamp, dwFlags)	IDirectSoundBuffer_Stop(pBuffer)

#undef	EXTERN
#endif	/* !defined(__XBOX2WIN_H__) */

/*-- End Of File --*/
