/*--------------------------------------------------------------*/
/*	xbox2win.c													*/
/*					X-BoxからWindowsに変換する為のいろいろ。	*/
/*--------------------------------------------------------------*/
#define	__XBOX2WIN_C__

#include	<xtl.h>
#include	"game.h"


/*--------------------------------------------------------------*/
/*	DirectSound8												*/
/*--------------------------------------------------------------*/
STDAPI IDirectSound_CommitDeferredSettings(LPDIRECTSOUND pDirectSound)
{
	return(S_OK) ;
}


STDAPI_(void) DirectSoundDoWork(void)
{
}

STDAPI_(void) DirectSoundUseFullHRTF(void)
{
}

static unsigned char stage_file_tbl[16][256];  // 適当
static unsigned char stage_dir_tbl[512][256];  // 適当
static int stage_file_ent = 0;
static int stage_ent = 0;
static int	stage_cnt  = 0;
static int	stage_file_cnt  = 0;
static int	stage_nest_cnt  = 0;

static int 	cursor_pos = 0;
static int test_pgm_no = 0;
extern DSFXI3DL2Reverb	test_ds3dl2;
extern DSFXWavesReverb	test_dswr;
extern int 			test_dswr_req;
void SoundDebugOutYoshizawa(void)
{
	extern char	sound_debug_out[4][256];
	char	pFileName[MAX_PATH];
	int	i;
	for(i = 0; i< 4 ;i++){
		DEBUG_Locate( 16, 256 + i *16, 0 );
		DEBUG_Printf( "%s", sound_debug_out[i]) ;
		sound_debug_out[i][0] = 0;
	}

	DEBUG_Locate( 32, 256 + 4 *16, 0 );
	DEBUG_Printf( "SE  (Q)<(%3x)>(W) Play(E) ", test_pgm_no) ;
	
	
	DEBUG_Locate( 32, 256 + 5 *16, 0 );
	DEBUG_Printf( "fInGain     %f", test_dswr.fInGain) ;
	DEBUG_Locate( 32, 256 + 6 *16, 0 );
	DEBUG_Printf( "fReverbMix  %f", test_dswr.fReverbMix ) ;
	DEBUG_Locate( 32, 256 + 7 *16, 0 );
	DEBUG_Printf( "fReverbTime %f", test_dswr.fReverbTime ) ;
	DEBUG_Locate( 32, 256 + 8 *16, 0 );
	DEBUG_Printf( "fHighFreqRTRatio %f", test_dswr.fHighFreqRTRatio) ;
	DEBUG_Locate( 32, 256 + 9 *16, 0 );
	if(stage_nest_cnt){
		sprintf(pFileName,"%s\\%s",stage_dir_tbl[stage_cnt],stage_file_tbl[stage_file_cnt]);
		DEBUG_Printf( pFileName) ;
	}else{
		DEBUG_Printf( stage_dir_tbl[stage_cnt]) ;
	}


	
	DEBUG_Locate( 16, 256 + (4+cursor_pos ) *16, 0 );
	DEBUG_Printf( ">") ;
}
extern DMUS_SYNTHSTATS8	debug_status;
void SoundDebugOutYoshizawa2(void)
{
	extern char	sound_debug_out[4][256];

	DEBUG_Locate( 8 * (40 + 5), 256 + 4 *16, 0 );
	DEBUG_Printf( "VS      (%3x)", debug_status.dwValidStats) ;
	DEBUG_Locate( 8 * (40 + 5), 256 + 5 *16, 0 );
	DEBUG_Printf( "Voices  (%3d)", debug_status.dwVoices) ;
	DEBUG_Locate( 8 * (40 + 5), 256 + 6 *16, 0 );
	DEBUG_Printf( "TotalCPU (%3d)", debug_status.dwTotalCPU) ;
	DEBUG_Locate( 8 * (40 + 5), 256 + 7 *16, 0 );
	DEBUG_Printf( "CPUPerVoice (%3d)", debug_status.dwCPUPerVoice) ;
	DEBUG_Locate( 8 * (40 + 5), 256 + 8 *16, 0 );
	DEBUG_Printf( "LostNotes  (%3d)", debug_status.dwLostNotes) ;
	DEBUG_Locate( 8 * (40 + 5), 256 + 9 *16, 0 );
	DEBUG_Printf( "FreeMemory (%xh)", debug_status.dwFreeMemory) ;
	DEBUG_Locate( 8 * (40 + 5), 256 + 10 *16, 0 );
	DEBUG_Printf( "lPeakVolume  (%d)", debug_status.lPeakVolume) ;
	DEBUG_Locate( 8 * (40 + 5), 256 + 11 *16, 0 );
	DEBUG_Printf( "SynthMemUse  (%xh)", debug_status.dwSynthMemUse) ;

}

static void SoundDebugFileInitYoshizawa(char *fpath)
{
	HANDLE	fh;
	int rc;
	
	WIN32_FIND_DATA 	find_data;
	char	pFileName[MAX_PATH];
	
	stage_file_ent = 1;
	sprintf(stage_file_tbl[0],"..");
	
	sprintf(pFileName,"%s\\%s\\*.sdx",pcGetFilePath(),fpath);
	fh = FindFirstFile(pFileName,&find_data);
	if(fh != INVALID_HANDLE_VALUE){

		do{
			sprintf(stage_file_tbl[stage_file_ent],"%s"
					,find_data.cFileName);
				
			stage_file_ent ++;
		}while(FindNextFile(fh ,&find_data));

		FindClose(fh);
	}
}

static void SoundDebugInitYoshizawa(void)
{
	HANDLE	fh;
	int rc;
	
	WIN32_FIND_DATA 	find_data;
	char	pFileName[MAX_PATH];
	
	
	sprintf(pFileName,"%s\\stage\\*",pcGetFilePath());
	fh = FindFirstFile(pFileName,&find_data);
	if(fh != INVALID_HANDLE_VALUE){

		do{
			if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				if(find_data.cFileName[0] == '.'
				 &&(find_data.cFileName[1] == 0
				  ||find_data.cFileName[1] == '.'
				  &&find_data.cFileName[2] == 0)
				  ){
				}else{
					sprintf(stage_dir_tbl[stage_ent],"stage\\%s"
							,find_data.cFileName);
					
					stage_ent ++;
				}
			}
		}while(FindNextFile(fh ,&find_data));

		FindClose(fh);
	}
}
extern void	GM_SdSet( code );
void SoundDebugMainYoshizawa(int key)
{
	static int		file_init_flag  = 0;
	static int		last_key = 0;
	static DWORD 		last_tick = 0;
	static int		key_cnt=0;
	static float	rev_mms_tbl[][3] = {
			{DSFX_WAVESREVERB_INGAIN_MIN,DSFX_WAVESREVERB_INGAIN_MAX,0.5f},
			{DSFX_WAVESREVERB_REVERBMIX_MIN,DSFX_WAVESREVERB_REVERBMIX_MAX,0.5f},
			{DSFX_WAVESREVERB_REVERBTIME_MIN,DSFX_WAVESREVERB_REVERBTIME_MAX,100.0f},
			{DSFX_WAVESREVERB_HIGHFREQRTRATIO_MIN,DSFX_WAVESREVERB_HIGHFREQRTRATIO_MAX,0.1f}};
		
	BYTE		now_key[256];	
	int		rep_step;
	float	*rev = (float*)&test_dswr;
	int		inx = cursor_pos -1;
	DWORD  	now_tick;
	int		i;
	
	if(file_init_flag==0){
		SoundDebugInitYoshizawa();
		file_init_flag = 1;
	}

	now_tick = GetTickCount();
	rep_step = 1;

	if((now_tick - last_tick) < 1000 / 60){
		return;
	}else if((now_tick - last_tick) > 4 * 1000 / 60){
		key_cnt = 0;
	}
	if(last_key == key){
		key_cnt ++;
		if(		 key_cnt <  5){
			rep_step = 1;
		}else if(key_cnt < 10){
			rep_step = 1;
		}
	}else{
		key_cnt = 0;
	}
	GetKeyboardState(now_key);
	
	if(now_key[VK_LSHIFT] & 0x80 || now_key[VK_RSHIFT] & 0x80){
		rep_step = 0x10;
	}else if(now_key[VK_LCONTROL] & 0x80 || now_key[VK_RCONTROL] & 0x80){
		rep_step = 0x100;
	}

	last_key = key;
	last_tick = now_tick;
	switch(key){
	case	'W':
		if(cursor_pos==0){
			test_pgm_no += rep_step;
			if(test_pgm_no > 0x620){
				test_pgm_no = 0;
			}
		}else if(cursor_pos<5){
			rev[inx] += rev_mms_tbl[inx][2] * rep_step;
			if(rev[inx] > rev_mms_tbl[inx][1]){
				rev[inx] = rev_mms_tbl[inx][0];
			}
			test_dswr_req = 1;
		}else{
			if(stage_nest_cnt==0){
				stage_cnt += rep_step;
				if(stage_cnt >= stage_ent){
					stage_cnt = 0;
				}
			}else{
				stage_file_cnt += rep_step;
				if(stage_file_cnt >= stage_file_ent){
					stage_file_cnt = 0;
				}
			}
		}
		break;
		
	case	'Q':
		if(cursor_pos==0){
			test_pgm_no -= rep_step;
			if(test_pgm_no < 0){
				test_pgm_no = 0x620;
			}
		}else if(cursor_pos<5){
			rev[inx] -= rev_mms_tbl[inx][2] * rep_step;
			if(rev[inx] < rev_mms_tbl[inx][0]){
				rev[inx] = rev_mms_tbl[inx][1];
			}
			test_dswr_req = 1;
		}else{
			if(stage_nest_cnt==0){
				stage_cnt -= rep_step;
				if(stage_cnt < 0){
					stage_cnt = stage_ent-1;
				}
			}else{
				stage_file_cnt -= rep_step;
				if(stage_file_cnt < 0){
					stage_file_cnt = stage_file_ent-1;
				}
			}
		}
		break;

	case	'E':
//@		GM_SdSet( test_pgm_no);
		Sleep(8);
		GM_SdSet( test_pgm_no | (0x20 << 18) | 0x3F000);
		
		break;

	case	'R':
		cursor_pos --;
		if(cursor_pos <0){
			cursor_pos = 6-1;
		}
		break;

	case	'T':
		cursor_pos ++;
		if(cursor_pos >= 6){
			cursor_pos = 0;
		}
		break;

	case	'Y':
	
		if(stage_nest_cnt==0){
			stage_file_cnt = 0;
			SoundDebugFileInitYoshizawa(stage_dir_tbl[stage_cnt]);
			
			stage_nest_cnt = 1;
		}else{
			if(stage_file_cnt == 0){
				stage_nest_cnt = 0;
			}else{
				DWORD	send_code;
				GM_SdSet( 0x1FFFFFFE );
				GM_SdSet( 0x1FFFFFFF );
				GM_SdSet( 0x80000000  | (DWORD)stage_dir_tbl[stage_cnt]);
				{
					char	pFileName[MAX_PATH];
					sprintf(pFileName,"%s\\",stage_dir_tbl[stage_cnt]);
					sd_change_directory( pFileName );
				
				}
		
				Sleep(1000);
				
				send_code = 0;
				for( i = 2  ; i < 8 ; i++){
					send_code *= 16;
					if(stage_file_tbl[stage_file_cnt][i] >= 'a'
					 &&stage_file_tbl[stage_file_cnt][i] <= 'f'){

						send_code += stage_file_tbl[stage_file_cnt][i] - 'a' + 10;

					}else if( stage_file_tbl[stage_file_cnt][i] >= 'A'
					 		&&stage_file_tbl[stage_file_cnt][i] <= 'F'){

						send_code += stage_file_tbl[stage_file_cnt][i] - 'A' + 10;

					}else{

						send_code += stage_file_tbl[stage_file_cnt][i] - '0';

					}
				}

				GM_SdSet( 0xFE800000 | send_code );
			}
		}

		break;
	}
}


BOOL SetPriorityClassMacro(DWORD p)
{
    HANDLE op = OpenProcess(PROCESS_ALL_ACCESS,TRUE,GetCurrentProcessId());
    SetPriorityClass(op,p);
    CloseHandle(op);
}


	/*----------------------------------------------------------*/
	/*	DirectSoundBuffer										*/
	/*----------------------------------------------------------*/



HANDLE WINAPI XGetSectionHandleA(LPCSTR pSectionName)
{
	return(0) ;
}

PVOID WINAPI XLoadSectionByHandle( HANDLE hSection )
{
	return(NULL) ;
}

DWORD WINAPI XGetSectionSize( HANDLE hSection )
{
	return(0) ;
}

BOOL WINAPI XFreeSectionByHandle( HANDLE hSection )
{
	return(TRUE) ;
}

BOOL __stdcall DmIsDebuggerPresent(void)
{
}

VOID WINAPI XSetProcessQuantumLength( DWORD dwMilliseconds )
{
}


DWORD XGetLanguage(void)
{
#ifdef	RELAREA_US
	/*-- 米国Release --*/
	switch(PRIMARYLANGID(GetUserDefaultLangID())){
#ifdef DEBUG_MODE
	case	LANG_JAPANESE:
		printf("XGetLanguage():LANG_JAPANESE\n") ;
		return(XC_LANGUAGE_JAPANESE);
#endif
	default:
	case	LANG_ENGLISH:
		printf("XGetLanguage():LANG_ENGLISH\n") ;
		return(XC_LANGUAGE_ENGLISH);
	}
#else
	/*-- 欧州Release --*/
	switch(PRIMARYLANGID(GetUserDefaultLangID())){
	case	LANG_ITALIAN:
		printf("XGetLanguage():LANG_ITALIAN\n") ;
		return(XC_LANGUAGE_ITALIAN);
#ifdef DEBUG_MODE
	case	LANG_JAPANESE:
		printf("XGetLanguage():LANG_JAPANESE\n") ;
		return(XC_LANGUAGE_JAPANESE);
#endif
	case	LANG_SPANISH:
		printf("XGetLanguage():LANG_SPANISH\n") ;
		return(XC_LANGUAGE_SPANISH);
	case	LANG_FRENCH:
		printf("XGetLanguage():LANG_FRENCH\n") ;
		return(XC_LANGUAGE_FRENCH);
	case	LANG_GERMAN:
		printf("XGetLanguage():LANG_GERMAN\n") ;
		return(XC_LANGUAGE_GERMAN);
	default:
	case	LANG_ENGLISH:
		printf("XGetLanguage():LANG_ENGLISH\n") ;
		return(XC_LANGUAGE_ENGLISH);
	}
#endif
}


/*-- End Of File --*/

