//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    vib_edit.c
	振動ツール
	2000/06/28 T.Shibata

	$Id: vib_edit.c,v 1.1.1.3 2002/11/19 11:48:55 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"../util/ts_util.h"
#include	"gameheader.h"
#include	"../../kano/preview/prekey.h"
#include	"../../kano/preview/preview_def.h"
#include	"../../kano/preview/prefile.h"

#ifndef PSX2	///madakore
#define SCE_WRONLY 0
#define SCE_TRUNC 0
#define SCE_CREAT 0
#define SCE_RDONLY 0
#define SCE_SEEK_END 0
#define SCE_SEEK_SET 0
#endif


//#define      LOCATE_X        0x10
//#define      LOCATE_Y        (0x20<<1)

#define LOSS_TIME (16)
#define	COM_MAX		(4)
#define MAX_BANK	(32)

#define	MAX_VIBFILE_NUM	(16)
#define	MAX_PARAM		(255)
#define STATUS_MASK	( STATE_MENU_DISABLE | STATE_LIFE_OFF | STATE_RADAR_OFF | STATE_RADIO_OFF )

#define FILE_NAME_VIB	"host0:./default.vib"

#define sceOpen( a, b )		pcOpen( a, b )
#define sceRead( a, b, c )	pcRead( a, b, c )
#define sceWrite( a, b, c ) pcWrite( a, b, c )
#define sceClose( a )		pcClose( a )
#define sceLseek( a, b, c )	pcLseek( a, b, c )


#if 0
extern int  pcOpen(char *filename, int flag);
extern int  pcClose(int fd);
extern int  pcRead(int fd, void *buf, int nbyte);
extern int  pcWrite(int fd, void *buf, int nbyte);
extern int  pcLseek(int fd, unsigned int offset, int whence);
#endif

extern void *NewPadVibration( char *script, int type );
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
//NewPadVibration( unsigend char *script, long type )
/*----------------------------------------------------------------*/

typedef struct {
	u_char	pow ;
	u_char	len ;
} SCRIPT_PARAM ;

typedef struct {
	int				n_h_param;
	int				n_l_param;
	SCRIPT_PARAM	*h_param;
	SCRIPT_PARAM	*l_param;
}VIB_PACK;

typedef struct {
	int				n_h_param;
	int				n_l_param;
	SCRIPT_PARAM	h_param[255+1];
	SCRIPT_PARAM	l_param[255+1];
} VIB_PACK_WORK;

typedef struct {
	u_short	pow ;
	short	len ;
	u_short from;
	u_short	to;
} EDIT_PARAM ;

typedef struct {
	EDIT_PARAM		h_param[255+3];
	EDIT_PARAM		l_param[255+3];
} EDIT_PACK ;

#define MAX_TEMP 512

//モーションと一緒に再生用
VIB_PACK_WORK G_With_Motion_Vib;

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX			actor ;
	DG_PRIM2			*prim0;			//画面クリア
	DG_PRIM2			*prim1;			//フレーム
	DG_PRIM2			*prim2;			//振動ライン
	int					flag ;			//
	
	int					mode ;			// 編集モード
	int					cursor ;
	int					com_num;
	int 				file_num;
	
	int 				copy_num;
	int					play_time ;		// 振動再生時間
	int					play_cur_x ;
	int					pos ;			// ターゲットパラメータ番号
	
	int					vib_type ;		// 振動タイプ
	int					int_invi;
	int					test;
	void				*vib_chara_work;	// 振動くんのワークポインタ

	
	EDIT_PARAM			temp;
	EDIT_PACK			edit_temp;
	EDIT_PARAM			buf[255+3];
	
	VIB_PACK_WORK		vib_pack_work;
	VIB_PACK_WORK		vib_list[MAX_VIBFILE_NUM];

	int					debug_print_pos;
	float				line_prim_pos;
	
//	VIB_PACK			vib_list[MAX_VIBFILE_NUM];
} Work ;


#define DISP_Y_LOW			(155)
#define DISP_Y_SCALE_INV	(3)
#define DISP_X_LOW			(16)
#define DISP_X_SCALE_INV	(1)


static char	*menu_list_1[] = {"INS PARAM","DEL PARAM","SWAP BACK","SWAP FORWARD"};
//static char	*menu_list_2[] = {"CHANGE BANK","QUICK SAVE HIGH","QUICK SAVE LOW","QUICK LOAD HIGH","QUICK LOAD LOW"};
static Work *vib_work = NULL;
static char *comstr[] = {"PARST","RLEN_PARST","RPOW_PARST"};
static char *ftype[2] = { "HIGH","LOW "};

enum {
	MODE_MAIN = 0,
	MODE_FILE,
	MODE_PLAY,
	MODE_COPY,
	MODE_LOAD,
	MODE_SAVE,
	MODE_MAX,
	MODE_EDIT,
	MODE_EDIT2,
};

#define WARU_Y 	(4)
#define WARU_X	(16*10)

#define N_VERTS1 (6+(WARU_X+1)*2+(WARU_Y+1)*2)

#define N_VERTS2 (60)
#define N_PRIMS2 (MAX_PARAM * 2 * 2/N_VERTS2)

//5*51*4
//5*4*3*17
#ifdef BP_PS2
#define		MEM_SCR_POS		((void *)(SCRPAD_ADDR))
#define		MEM_SCR_UV		((void *)(MEM_SCR_POS + sizeof(FVECTOR) * MAX_PARAM * 2 * 2))
#else
#define		MEM_SCR_POS		((void *)(SCRPAD_ADDR))
#define		MEM_SCR_UV		((void *)((char *)MEM_SCR_POS + sizeof(FVECTOR) * MAX_PARAM * 2 * 2))
#endif

static PreviewKey_Param PreviewKey0;

static void PreKeyInit0(void)
{
    PreviewKey0.status=PreviewKey0.auto_status
		=PreviewKey0.press=0;
    PreviewKey0.repeat_count=0;
}

static void PreKeyAct0(void)
{
    PreviewKey0.press=GV_PadData[0].press;

    if(GV_PadData[0].status && PreviewKey0.status==GV_PadData[0].status){
		if(PreviewKey0.repeat_count>PreviewKey0.repeat_next_count){
			if(PreviewKey0.repeat_next_count>REPEAT_SECOND){
				PreviewKey0.repeat_count=PreviewKey0.repeat_next_count;
			}
			else{
				PreviewKey0.repeat_next_count+=REPEAT_NEXT;
			}
			PreviewKey0.auto_status=PreviewKey0.status;
		}
		else PreviewKey0.auto_status=0;

		PreviewKey0.repeat_count++;
    }
    else{
		PreviewKey0.auto_status
			=PreviewKey0.status=GV_PadData[0].status;
		PreviewKey0.repeat_count=0;
		PreviewKey0.repeat_next_count=REPEAT_FIRST;
    }
}

static void DelParam( SCRIPT_PARAM *param_src, int no )
{
	int		i ;
	SCRIPT_PARAM	*param ;

	param = &param_src[ no ] ;
	for ( i = ( MAX_PARAM - no ) ; i > 0 ; i-- ){
		param[0] = param[1];
		param++ ;
	}
	param = &param_src[ MAX_PARAM-1 ] ;
	param->len = 0;
	param->pow = 0;
}

static void InsParam( SCRIPT_PARAM *param_src, int no )
{
	int		i ;
	SCRIPT_PARAM	*param ;

	param = &param_src[ MAX_PARAM-1 ] ;
	for ( i = ( MAX_PARAM -1 - no ) ; i > 0 ; i-- ){
		param[0] = param[-1];
		param-- ;
	}
	param = &param_src[ no ] ;
	param->len = 0;
	param->pow = 0;
}

static void SwapBackParam( SCRIPT_PARAM *param, int no )
{
	SCRIPT_PARAM	tmp ;

	if ( no >= MAX_PARAM-1 ) return ;
	tmp = param[ no + 1 ] ;
	param[ no + 1 ] = param[ no ] ;
	param[ no ] = tmp ;
}

static void SwapForwardParam( SCRIPT_PARAM *param, int no )
{
	SCRIPT_PARAM	tmp ;

	if ( no <= 0 ) return ;
	tmp = param[ no - 1 ] ;
	param[ no - 1 ] = param[ no ] ;
	param[ no ] = tmp ;
}

static void (*CommamdList0[])(SCRIPT_PARAM*,int) = {
	InsParam,
	DelParam,
	SwapBackParam,
	SwapForwardParam,
};

static void SetPrim0( DG_PRIM2 *prim )
{
	FVECTOR 		*pos = MEM_SCR_POS;
	float			screen_near_x,screen_near_y;

	DG_SwitchBuffPrim2( prim );
	
	screen_near_x = 51.0f / (ASPECT_X()                                           ) / DG_Chanls[0].screen;
	screen_near_y = 51.0f / (ASPECT_Y() * DG_Chanls[0].width / DG_Chanls[0].height) / DG_Chanls[0].screen;
//printf("x = %f:y = %f\n",screen_near_x,screen_near_y);
	
	pos[0].vx = -1.0f * screen_near_x;
	pos[0].vy =  0.0f * screen_near_y;
	pos[0].vz = 51.0f;
	pos[1].vx =  1.0f * screen_near_x;
	pos[1].vy =  0.0f * screen_near_y;
	pos[1].vz = 51.0f;
	pos[2].vx = -1.0f * screen_near_x;
	pos[2].vy =  1.0f * screen_near_y;
	pos[2].vz = 51.0f;
	pos[3].vx =  1.0f * screen_near_x;
	pos[3].vy =  1.0f * screen_near_y;
	pos[3].vz = 51.0f;
	
	TS_Scr_Mem( prim->pos[prim->buffer_clock], MEM_SCR_POS, sizeof(FVECTOR), 4 );

}

#define VIEW_FRAME_NUM (512)

#define SCL_X ( 1.9f / 512.0f )
#define SCL_Y ( -0.9f / 256.0f )
#define MIN_X ( -0.95f )
#define MIN_Y (  0.95f )

static void SetPrim1( Work *work, DG_PRIM2 *prim, int mode )
{
	FVECTOR 		*pos = MEM_SCR_POS;
	float			screen_near_x,screen_near_y;
	int 			i,j,len = 0;
	VIB_PACK_WORK  	*edit_vib = &vib_work->vib_list[vib_work->file_num];
	SCRIPT_PARAM 	*param;// = edit_vib->param[vib_work->vib_type];
	DG_PRIM2_UVRGB	*uvrgb;
	float			x,y,dx,dy;
	int				print_x,print_y;
	
	if(vib_work->vib_type) param = edit_vib->l_param;
	else				   param = edit_vib->h_param;
	
	DG_SwitchBuffPrim2( prim );
	
	screen_near_x = 51.0f / (ASPECT_X()                                           ) / DG_Chanls[0].screen;
	screen_near_y = 51.0f / (ASPECT_Y() * DG_Chanls[0].width / DG_Chanls[0].height) / DG_Chanls[0].screen;
//printf("x = %f:y = %f\n",screen_near_x,screen_near_y);

	uvrgb = prim->uvrgb[prim->buffer_clock];
		
	x = -1.0f;
	y = 0.05f;
	dx = 1.0f - (-1.0f);
	//dy = 0.95f - (0.05f)/(float)WARU_Y;
	dy = 0.90f / 4.0f;
//	printf("dx %f: dy %f\n",dx,dy);
	for( i = 0; i < WARU_Y + 1; i++ ){
		for( j = 0; j < 2; j++ ){
			//printf("x %f: y %f\n",x,y);
			
			pos->vx = x*screen_near_x;
			pos->vy = y*screen_near_y;
			pos->vz = 51.0f;
			
			pos++;
			uvrgb++;
			x += dx;
		}
		x = -1.0f;
		y += dy;
	}
//printf("\n");
	
	x = work->line_prim_pos;
	y = 0.0f;
//	dx = 0.95f - (-0.95f)/(float)WARU_X;
	dx = 1.9f/16.0f;
	dy = 1.0f - 0.0f;

	//printf("dx %f: dy %f\n",dx,dy);	
	for( i = 0; i < WARU_X + 1; i++ ){
		for( j = 0; j < 2; j++ ){
			//printf("x %f: y %f\n",x,y);
			pos->vx = x*screen_near_x;
			pos->vy = y*screen_near_y;
			pos->vz = 51.0f;
			if( i % 8 == 0 ){
				print_x = DG_FTOI(x*256.0f + 256.0f);
				print_y = DG_FTOI(y*224.0f + 224.0f);
				if(!(print_x < 0 || 512 < print_x || print_y < 0 || 224 < print_y )){
					DEBUG_Locate( print_x, print_y, 0 );
					DEBUG_Printf( "%d",i*32);
				}
			}
			pos++;
			uvrgb++;
			y += dy;
		}
		x += dx;
		y = 0.0f;
	}
//	printf("\n");

	//選択ライン
	if(!mode){
		for( i = 0; i < vib_work->pos; i++){
			len += param->len;
			param++;
		}
	
		for( i = 0; i < 6; i++ ){
			if(i<4){
				pos->vx = (i<2)?(work->line_prim_pos+SCL_X*(float)len):(work->line_prim_pos+SCL_X*(float)(len+param->len));
				pos->vy = (i&1)?1.0f:0.0f;
			}else{
				pos->vx = (i&1)?1.0f:-1.0f;
				pos->vy = MIN_Y+SCL_Y*(float)param->pow;

				if(work->vib_type){ //ロウ
					uvrgb->r = 0;
					uvrgb->g = 0x80;
					uvrgb->b = 0;
				}else{ //ハイ
					uvrgb->r = 0;
					uvrgb->g = 0x80;
					uvrgb->b = 0x80;
				}
			}
			pos->vx *= screen_near_x;
			pos->vy *= screen_near_y;
			pos->vz = 51.0f;
			
			uvrgb++;
			pos++;
		}
	}else{
		for( i = 0; i < 6; i++ ){
			if(i<4){
				pos->vx = (i<2)?(work->line_prim_pos+SCL_X*(float)(vib_work->temp.from)):(work->line_prim_pos+SCL_X*(float)(vib_work->temp.from+vib_work->temp.len));
				pos->vy = (i&1)?1.0f:0.0f;
			}else{
				pos->vx = (i&1)?1.0f:-1.0f;
				pos->vy = MIN_Y+SCL_Y*(float)vib_work->temp.pow;

				if(work->vib_type){ //ロウ
					uvrgb->r = 0;
					uvrgb->g = 0x80;
					uvrgb->b = 0;
				}else{ //ハイ
					uvrgb->r = 0;
					uvrgb->g = 0x80;
					uvrgb->b = 0x80;
				}
			}
			pos->vx *= screen_near_x;
			pos->vy *= screen_near_y;
			pos->vz = 51.0f;
			uvrgb++;
			pos++;
		}
	}
	TS_Scr_Mem( prim->pos[prim->buffer_clock], MEM_SCR_POS, sizeof(FVECTOR), N_VERTS1 );
}

static void SetPrim2( Work *work, DG_PRIM2 *prim, int mode )
{
	FVECTOR 		*pos = MEM_SCR_POS;
	float			screen_near_x,screen_near_y,min_x;
	int 			i,l_len = 0,h_len = 0;
	VIB_PACK_WORK 	*edit_vib = &vib_work->vib_list[vib_work->file_num];
	SCRIPT_PARAM 	*h_param = edit_vib->h_param;
	SCRIPT_PARAM 	*l_param = edit_vib->l_param;
	EDIT_PARAM 		*he_param = vib_work->edit_temp.h_param;
	EDIT_PARAM 		*le_param = vib_work->edit_temp.l_param;
	
	DG_SwitchBuffPrim2( prim );
	
	screen_near_x = 51.0f / (ASPECT_X()                                           ) / DG_Chanls[0].screen;
	screen_near_y = 51.0f / (ASPECT_Y() * DG_Chanls[0].width / DG_Chanls[0].height) / DG_Chanls[0].screen;
//printf("x = %f:y = %f\n",screen_near_x,screen_near_y);

	min_x = work->line_prim_pos;
	
	for( i = 0; i < MAX_PARAM * 4; i++ ){
		if(!mode){

			if( i < MAX_PARAM * 2 ){
				if(i&1) h_len += h_param->len;
				pos->vx = min_x+SCL_X*(float)h_len;
				pos->vy = MIN_Y+SCL_Y*(float)h_param->pow;
				if(i&1) h_param++;
			}else{
				if(i&1) l_len += l_param->len;
				pos->vx = min_x+SCL_X*(float)l_len;
				pos->vy = MIN_Y+SCL_Y*(float)l_param->pow;
				if(i&1) l_param++;
				//printf("[%d]pos vx %f:vy %f\n",i -MAX_PARAM * 2, pos->vx,pos->vy);
			}
		}else{
			if( i < MAX_PARAM * 2 ){
				pos->vx = min_x+SCL_X*((i&1)?((float)(he_param->from+he_param->len)):((float)he_param->from));
				pos->vy = MIN_Y+SCL_Y*(float)he_param->pow;
				if(i&1) he_param++;
			}else{
				pos->vx = min_x+SCL_X*((i&1)?((float)(le_param->from+le_param->len)):((float)le_param->from));
				pos->vy = MIN_Y+SCL_Y*(float)le_param->pow;
				if(i&1) le_param++;
				//printf("[%d]pos vx %f:vy %f\n",i -MAX_PARAM * 2, pos->vx,pos->vy);
			}
		}
		pos->vx *= screen_near_x;
		pos->vy *= screen_near_y;
		pos->vz = 51.0f;
		pos++;
	}
	
	TS_Scr_Mem( prim->pos[prim->buffer_clock], MEM_SCR_POS, sizeof(FVECTOR), MAX_PARAM * 4 );
}

static void SetPrims( Work *work )
{
	SetPrim0( work->prim0 );
	SetPrim1( work, work->prim1,((work->mode==MODE_EDIT2)?1:0) );
	SetPrim2( work, work->prim2,((work->mode==MODE_EDIT2)?1:0) );
}

static	void	Act( Work *work )
{
	if(work->flag){
		work->flag = 0;
		if(work->int_invi&2){
			DG_InvisiblePrim2(work->prim0);
			DG_InvisiblePrim2(work->prim1);
			DG_InvisiblePrim2(work->prim2);
		}else{
			DG_VisiblePrim2(work->prim0);
			DG_VisiblePrim2(work->prim1);
			DG_VisiblePrim2(work->prim2);
			SetPrims( work );
		}
		//SetPrims( work );
		//printf("Visible\n");
	}else{
		DG_InvisiblePrim2(work->prim0);
		DG_InvisiblePrim2(work->prim1);
		DG_InvisiblePrim2(work->prim2);
		//printf("Invisible\n");
	}
	PreKeyAct0();
}

static	void	Die( Work *work )
{
	
	if(work->prim0) GM_FreePrim2(work->prim0);
	if(work->prim1) GM_FreePrim2(work->prim1);
	if(work->prim2) GM_FreePrim2(work->prim2);
	
	vib_work = NULL;
}

enum {
	EDIT_FILE = 0,
	EDIT_PLAY,
	EDIT_PRAM,
	EDIT_POW,
	EDIT_LEN,
	EDIT_COMMAND,
	EDIT_MAX,
};

static void ReSetTemp( EDIT_PARAM *param_src )
{
	memset(param_src,0,sizeof(EDIT_PARAM)*(MAX_PARAM+3));
}

static void DelTemp( EDIT_PARAM *param_src, int no )
{
	int		i ;
	EDIT_PARAM	*param ;

	param = &param_src[ no ] ;
	for ( i = ( MAX_PARAM - no ) ; i > 0 ; i-- ){
		param[0] = param[1];
		param++ ;
	}
	param = &param_src[ MAX_PARAM-1 ] ;
	param->len = 0;
	param->pow = 0;
	param->from = 0;
}

static void InsTemp( EDIT_PARAM *param_src, int no )
{
	int		i ;
	EDIT_PARAM	*param ;

	param = &param_src[ MAX_PARAM-1 ] ;
	for ( i = ( MAX_PARAM-1 - no ) ; i > 0 ; i-- ){
		param[0] = param[-1];
		param-- ;
	}
	param = &param_src[ no ] ;
	param->len = 0;
	param->pow = 0;
	param->from = 0;
}

static void AddTemp(EDIT_PARAM *temp,int type)
{
	EDIT_PARAM 		*param;
	EDIT_PARAM 		*init_param;
	int 			pfrom,pto;
	int 			tfrom,tto;
	int 			i,flag=1;
	
	init_param = param = (type)?vib_work->edit_temp.l_param:vib_work->edit_temp.h_param;

	for( i = 0; param->len && i<MAX_PARAM; ){
		pfrom = param->from;
		pto = pfrom + param->len;
		tfrom = temp->from;
		tto = tfrom + temp->len;
		
		if(tfrom <= pfrom && pfrom <= tto){
			if(tfrom <= pto && pto <= tto){
				DelTemp( init_param, i );
				continue;
			}
		}
	
		if(pfrom < tfrom && tfrom < pto ){
			if( pfrom <= tto && tto <= pto ){
				param->len = tfrom - pfrom;
				InsTemp(init_param,i+1);
				InsTemp(init_param,i+1);
				//param[1] = temp;
				param[2].len = pto-tto;
				param[2].pow = param->pow;
				param[2].from = tto;
				i++;
				flag=0;
				break;
			}else{
				param->len = tfrom - pfrom;
				continue;
			}
		}else if(pfrom < tto && tto < pto){
			param->len = param->len + param->from - tto;
			param->from = tto;
			//param->len = param->len + param->from - tto;
			InsTemp(init_param,i);
			flag=0;
			break;
		}
		i++;
		param++;
	}

//	if(flag && i >= MAX_PARAM) return;
		
	init_param[i] = *temp;
	//ソート
	if(flag){
		init_param[i].len = 0;
		init_param[i].pow = 0;
		init_param[i].from = 0;
		param = init_param;	i=0;
		while(param->len){
			if(param->from > temp->from){
				InsTemp(init_param,i);
				*param = *temp;
				break;
			}
			param++;
			i++;
		}
		if(!param->len){
			*param = *temp;
		}
//		printf("sort\n");
	}
	//合成
	param = init_param;	i=0;
	while(param[0].len&&param[1].len){
		if(param[0].pow == param[1].pow &&
		   param[0].from+param[0].len >= param[1].from){
			param[1].len += param[1].from - param[0].from;
			param[1].from = param[0].from;
			DelTemp( init_param, i );
		}else{
			param++;
			i++;
		}
//		printf("conv\n");
	}
	if(!param[0].len){
		param[0].len = 0;
		param[0].from = 0;
		param[0].pow = 0;
	}else{
		param[1].len = 0;
		param[1].from = 0;
		param[1].pow = 0;
	}

	param = init_param;	i = 0;	
	while(param->len && i < MAX_PARAM){
		//printf("[%d]len = %d pow = %d\n",i,param->len,param->pow);
		if(!param->pow){
			DelTemp( init_param, i );
			//printf("del_param\n");
		}else{
			i++;
			param++;
		}
	}
//	printf("\n");
#if 0
	for(i=0;i<MAX_PARAM+3;i++){
		printf("[%2d] pow %3d:from %5d:to %5d:len %5d\n",i,init_param->pow,init_param->from,init_param->from+init_param->len,init_param->len);
		//if(!init_param->len)break;
		init_param++;
	}
	printf("\n");
#endif
}

static void ConvParam_Edit_Vib()
{
	int 			i,lcon=0,hcon=0,h_len=0,l_len=0;
	EDIT_PARAM 		*el_param = vib_work->edit_temp.l_param;
	EDIT_PARAM 		*eh_param = vib_work->edit_temp.h_param;
	VIB_PACK_WORK 	*edit_vib = &vib_work->vib_list[vib_work->file_num];
	SCRIPT_PARAM 	*l_param = edit_vib->l_param;
	SCRIPT_PARAM 	*h_param = edit_vib->h_param;
	EDIT_PARAM temp;
	
//	ReSetTemp( el_param );
//	ReSetTemp( eh_param );
	memset(edit_vib,0,sizeof(VIB_PACK_WORK));

	for(i=0;i<MAX_PARAM;){
		if( el_param->from > l_len ){
			temp.len = el_param->from - l_len;
			temp.pow = 0;
			temp.from = l_len;
		}else{
			temp = *el_param;
			el_param++;
			i++;
		}
		l_len+=temp.len;
		while(temp.len){
			if(lcon >= MAX_PARAM)break;
			if(temp.len >= 128){
				temp.len -= 127;
				l_param->len = 127;
			}else{
				l_param->len = temp.len;
				temp.len = 0;
			}
			l_param->pow = temp.pow;
			l_param++;
			lcon++;
		}
	}
	
	for(i=0;i<MAX_PARAM;){
		if( eh_param->from > h_len ){
			temp.len = eh_param->from - h_len;
			temp.pow = 0;
			temp.from = h_len;
		}else{
			temp = *eh_param;
			eh_param++;
			i++;
		}
		h_len+=temp.len;
		while(temp.len){
			if(hcon >= MAX_PARAM)break;

			if(temp.len >= 128){
				temp.len -= 127;
				h_param->len = 127;
			}else{
				h_param->len = temp.len;
				temp.len = 0;
			}
			h_param->pow = temp.pow;
			h_param++;
			hcon++;
		}
	}
	
}

static void ConvParam_Vib_Edit()
{
	int 			i,l_len = 0,h_len = 0;
	VIB_PACK_WORK 	*edit_vib = &vib_work->vib_list[vib_work->file_num];
	SCRIPT_PARAM 	*l_param = edit_vib->l_param;
	SCRIPT_PARAM 	*h_param = edit_vib->h_param;
	EDIT_PARAM 		temp;

	ReSetTemp( vib_work->edit_temp.l_param );
	ReSetTemp( vib_work->edit_temp.h_param );

	for(i=0;i<MAX_PARAM;i++){
		temp.from = h_len;
		temp.pow = h_param->pow;
		temp.len = h_param->len;
		AddTemp(&temp,0);
		
		temp.from = l_len;
		temp.pow = l_param->pow;
		temp.len = l_param->len;
		AddTemp(&temp,1);
		
		h_len += h_param->len;
		l_len += l_param->len;
		
		l_param++;
		h_param++;
	}
	
}

static void GetSelectSphere( EDIT_PARAM *area, int type )
{
	EDIT_PARAM *param,*buf = vib_work->buf;
	EDIT_PARAM *init_param;
	int pfrom,pto;
	int afrom,ato,preto = 0;
//	int i;

	memset(vib_work->buf,0,sizeof(EDIT_PARAM)*(MAX_PARAM+3));
	
	init_param = param = (type)?vib_work->edit_temp.l_param:vib_work->edit_temp.h_param;

	afrom = area->from;
	ato = afrom + area->len;
	
	while(param->len){
		pfrom = param->from;
		pto = pfrom + param->len;
		
		if( preto <= afrom && afrom < pto ){
			break;
		}		
		preto = pto;
		param++;
	}
	if(!param->len){
		buf->pow = 0;
		buf->len = area->len;
		buf->from = 0;
		return;
	}
	if( pfrom < ato && ato <= pto){
	//一本で完結
		if(pfrom <= afrom && afrom < pto){
			buf->pow = param->pow;
			buf->len = area->len;
			buf->from = 0;
		}else{
			buf->pow = param->pow;
			buf->len = ato - pfrom;
			buf->from = pfrom - afrom;
		}
	}else{
		if(afrom >= pfrom){
			buf->pow = param->pow;
			buf->len = pto - afrom;
			buf->from = 0;
		}else{
			buf->pow = 0;
			buf->len = pfrom - afrom;
			buf->from = 0;
			buf++;
			buf->pow = param->pow;
			buf->len = param->len;
			buf->from = pfrom - afrom;
		}
		buf++;
		preto = pto;
		param++;
		while(param->len){
			pfrom = param->from;
			pto = pfrom + param->len;
			
			if(preto <= ato && ato < pto){
				if(ato <= pfrom){
					buf->pow = 0;
					buf->len = pfrom - ato;
					buf->from = preto - afrom;
				}else{
					buf->pow = param->pow;
					buf->len = ato - pfrom;
					buf->from = pfrom - afrom;
				}
				break;
			}
			
			if(preto!=pfrom){
				buf->pow = 0;
				buf->len = pfrom - preto;
				buf->from = preto - afrom;
				buf++;
			}
			
			buf->pow = param->pow;
			buf->from = pfrom - afrom;
			buf->len = param->len;
			buf++;
			preto = pto;
			param++;
		}
		
	}
	
	if(!param->len){
		buf->pow = 0;
		buf->len = ato - preto;
		buf->from = preto - afrom;
		buf++;
	}

	
#if 1
	{
		int i;
		init_param = vib_work->buf;
		for(i=0;i<MAX_PARAM+3;i++){
			printf("[%2d] pow %3d:from %5d:to %5d:len %5d\n",
				   i,init_param->pow,init_param->from,init_param->from+init_param->len,init_param->len);
			if(!init_param->len)break;
			init_param++;
		}
		printf("\n");
	}
#endif
}

static int Paste( int from, int type )
{
	EDIT_PARAM *buf = vib_work->buf;
	EDIT_PARAM temp;
	int ret=from;

	while(buf->len){
		temp = *buf;
		temp.from+=from;
		if(!type){ if(temp.pow)temp.pow = 127; }
		
		AddTemp(&temp,type);
		ret = temp.from + temp.len;
		buf++;
	}

	return ret;
}

static void ParamShift( int from, int len, int type )
{
	EDIT_PARAM 		*param;
	int 			flag = 0,to = 0;
	EDIT_PARAM		temp;

	param = (type)?vib_work->edit_temp.l_param:vib_work->edit_temp.h_param;
	
	while(param->len){
		if( param->from <= from && from < param->from + param->len ){
			flag = 1;
			break;
		}
		//if( param->from + param->len <= from && from < (param+1)->from ){
		if( to <= from && from < param->from ){
			flag = 2;
			break;
		}
		to = param->from + param->len;
		param++;
	}
	//printf("flag %d\n",flag);
	if(flag == 1) {
		temp = *param;
		temp.from = from + len;
		temp.len = (param->from + param->len) - from;
		param->len = from - param->from;

		param++;

		while(param->len){
			param->from += len;
			param++;
		}
		AddTemp(&temp,type);
	} else if(flag == 2) {
		param++;
		while(param->len){
			param->from += len;
			param++;
		}

	}
}

static int Insert( int from, int type )
{
	EDIT_PARAM *buf = vib_work->buf;
	EDIT_PARAM temp;
	int ret=from,len = 0;

	while(buf->len){ 
		len += buf->len; 
		buf++;
	}
	
//	printf("len = %d\n",len);

	ParamShift( from, len, type );

	buf = vib_work->buf;
	while(buf->len){
		temp = *buf;
		temp.from+=from;
		if(!type){ if(temp.pow)temp.pow = 127; }
		
		AddTemp(&temp,type);
		ret = temp.from + temp.len;
		buf++;
	}

	return ret;
}

static int Paste_TrunOver_Pow( int from, int type )
{
	EDIT_PARAM *buf = vib_work->buf;
	EDIT_PARAM temp;
	int ret=from;

	while(buf->len){
		temp = *buf;
		temp.from+=from;
		if(!type){ if(temp.pow)temp.pow = 127; }
		
		if(type) temp.pow = (temp.pow ^ 0xff)+1;
		else    temp.pow ^= 127;
		temp.pow &= 0xff;
		AddTemp(&temp,type);
		ret = temp.from + temp.len;
		buf++;
	}

	return ret;
}

static int Paste_TrunOver_Len( int from, int type )
{
	EDIT_PARAM 		*buf = vib_work->buf;
	EDIT_PARAM 		temp;
	int 			ret=from,max_len = 0,len = 0;
	
	while(buf->len){
		max_len += buf->len;
		buf++;
	}

	buf = vib_work->buf;
	
	while(buf->len){
		temp = *buf;
		len += buf->len;
		temp.from = max_len - len + from;
		if(!type){ if(temp.pow)temp.pow = 127; }
		AddTemp(&temp,type);
		ret = temp.from + temp.len;
		buf++;
	}

	return max_len + from;
}

static int (*CommamdList1[])(int,int)={
	Paste,
	Paste_TrunOver_Len,
	Paste_TrunOver_Pow,
};


static void PramData_DebugPrint()
{
	int 		x = LOCATE_X+320;
	int 		y = LOCATE_Y;
	int 		i,time=0;
	VIB_PACK_WORK 	*edit_vib = &vib_work->vib_list[vib_work->file_num];
//	SCRIPT_PARAM 	*h_param = edit_vib->h_param;
//	SCRIPT_PARAM 	*l_param = edit_vib->l_param;
	SCRIPT_PARAM 	*param;

	if(vib_work->vib_type){
		param = edit_vib->l_param;
	}else{
	    param = edit_vib->h_param;
	}
	
	if(!param) return;

	for( i = 0; i < vib_work->debug_print_pos; i++ ){
		time+=param->len;
		param++;
	}
    DEBUG_Locate( x, y, 0 );
	//DEBUG_Printf( "PNUM   POW   LEN\n" );
	DEBUG_Printf( "NUM  POW  LEN  TIM\n" );
	for( i = vib_work->debug_print_pos; i < 20 + vib_work->debug_print_pos; i++ ){
      //DEBUG_Printf( "NUM  POW  LEN  TIME\n" );
		if( i >= MAX_PARAM )break;
		DEBUG_Printf( "%3d <%3d><%3d><%3d>\n",i,param->pow,param->len,time );
		time+=param->len;
	//	if(!param->len)break;
		param++;
	}

	if( vib_work->debug_print_pos <= vib_work->pos && vib_work->pos < vib_work->debug_print_pos+20 ){
		y = LOCATE_Y+17 + (vib_work->pos-vib_work->debug_print_pos)*17;
		DEBUG_Locate( x-16, y, 0 );
		DEBUG_Printf( ">>" );
	}
}

static void EditData_DebugPrint()
{
	int x = LOCATE_X+300;
	int y = LOCATE_Y;
	int i;
	EDIT_PACK 	*edit_pack = &vib_work->edit_temp;
	EDIT_PARAM 	*param;
	
	param = (vib_work->vib_type)?&edit_pack->l_param[vib_work->debug_print_pos]:&edit_pack->h_param[vib_work->debug_print_pos];
	
    DEBUG_Locate( x, y, 0 );
	//DEBUG_Printf( "PNUM   POW   LEN\n" );
	DEBUG_Printf( "NUM  POW  LEN  TIM\n" );
	for( i = vib_work->debug_print_pos; i < 20 + vib_work->debug_print_pos; i++ ){
		if( i >= MAX_PARAM )break;
      //DEBUG_Printf( "NUM  POW  LEN  TIME\n" );
		DEBUG_Printf( "%3d <%3d><%3d><%3d>\n",i,param->pow,param->len,param->from);
		//if(!param->len)return;
		param++;
	}

}

static void Edit1_Menu_Corsor()
{
	int pad = PreviewKey0.auto_status;
	int press = PreviewKey0.press;
	int status = PreviewKey0.status;

	if(vib_work->test == 1){
		if(!(status & PAD_A)){
			int temp = vib_work->temp.from + vib_work->temp.len;
			vib_work->test = 0;

			if(vib_work->temp.len<0){
				vib_work->temp.from += vib_work->temp.len;
				vib_work->temp.len *= -1;
			}
			if(vib_work->temp.len) AddTemp(&vib_work->temp,vib_work->vib_type);
			
			vib_work->temp.from = temp;
			vib_work->temp.len = 0;
			return;
		}

		//printf("left_dx = %d: dy = %d\n",GV_PadData[0].left_dx,GV_PadData[0].left_dy);
		if(GV_PadData[0].analog_input & GV_PAD_ANALOG_L_USE){
			vib_work->temp.len += (GV_PadData[0].left_dx-128)/32;
			if((long64)vib_work->temp.from+vib_work->temp.len < 0){
				vib_work->temp.len = (-vib_work->temp.from);
			}
		}
		if(pad & PAD_R)vib_work->temp.len++;
		if(pad & PAD_L){
			if(vib_work->temp.from+vib_work->temp.len)vib_work->temp.len--;
		}
	}else if(vib_work->test == 2){
		if(!(status & PAD_B)){
			int temp = vib_work->temp.from + vib_work->temp.len;
			vib_work->test = 0;

			if(vib_work->temp.len<0){
				vib_work->temp.from += vib_work->temp.len;
				vib_work->temp.len *= -1;
			}
			if(vib_work->temp.len) GetSelectSphere(&vib_work->temp,vib_work->vib_type);

			vib_work->temp.from = temp;
			vib_work->temp.len = 0;
			return;
		}

		//printf("left_dx = %d: dy = %d\n",GV_PadData[0].left_dx,GV_PadData[0].left_dy);
		if(GV_PadData[0].analog_input & GV_PAD_ANALOG_L_USE){
			vib_work->temp.len += (GV_PadData[0].left_dx-128)/32;
			if((long64)vib_work->temp.from+vib_work->temp.len < 0){
				vib_work->temp.len = (-vib_work->temp.from);
			}
		}
		if(pad & PAD_R)vib_work->temp.len++;
		if(pad & PAD_L){
			if(vib_work->temp.from+vib_work->temp.len)vib_work->temp.len--;
		}
	}else{
		if(vib_work->vib_type){
			if(pad & PAD_U)vib_work->temp.pow++;
			if(pad & PAD_D && vib_work->temp.pow)vib_work->temp.pow--;
			if(vib_work->temp.pow>=256)vib_work->temp.pow=255;
			if(GV_PadData[0].analog_input & GV_PAD_ANALOG_L_USE){
				int dy;
				dy = vib_work->temp.pow - (GV_PadData[0].left_dy-128)/32;
				if(dy<0)dy=0;
				if(dy>255)dy=255;
				vib_work->temp.pow = dy;
			}
		}else{
			if(pad & PAD_U)vib_work->temp.pow = 127;
			if(pad & PAD_D)vib_work->temp.pow = 0;
		}
		
		if(GV_PadData[0].analog_input & GV_PAD_ANALOG_L_USE){
			int dx;
			dx = vib_work->temp.from + (GV_PadData[0].left_dx-128)/32;
			if(dx<0)dx=0;
			vib_work->temp.from = dx;
		}
		
		if(pad & PAD_R)vib_work->temp.from++;
		if(pad & PAD_L && vib_work->temp.from)vib_work->temp.from--;
	
		if(press & PAD_X){
			vib_work->vib_type ^= 1;
			vib_work->temp.pow = 0;
		}
		if(press & PAD_A)vib_work->test = 1;
		if(press & PAD_B)vib_work->test = 2;

		//ペースと
		//if(press & PAD_AR)vib_work->temp.from = Paste( vib_work->temp.from,vib_work->vib_type);
		//pow反転ペースと
		//if(press & PAD_AR)vib_work->temp.from = Paste_TrunOver_Pow( vib_work->temp.from,vib_work->vib_type);
		//len反転ペースと
		//if(press & PAD_AR)vib_work->temp.from = Paste_TrunOver_Len( vib_work->temp.from,vib_work->vib_type);
		if(press & PAD_AR){
			vib_work->temp.from = CommamdList1[vib_work->com_num%3]( vib_work->temp.from,vib_work->vib_type);
			//ConvParam_Edit_Vib();
			//ConvParam_Vib_Edit();
		}
		if(press & PAD_AL) vib_work->temp.from = Insert(vib_work->temp.from,vib_work->vib_type);

		if(press & PAD_L2) vib_work->com_num++;
		vib_work->com_num%=3;
		if(status & PAD_L1){
			if(press & PAD_R1){
				if(vib_work->vib_type)	ReSetTemp(vib_work->edit_temp.l_param);
				else					ReSetTemp(vib_work->edit_temp.l_param);
				vib_work->temp.from = 0;
				vib_work->temp.len = 0;
				vib_work->temp.pow = 0;
			}
		}
	}
	
}

static void Edit1_Menu_DebugPrint()
{
	int x = LOCATE_X+27;
	int y = LOCATE_Y+(18<<1);
	char	file_name[12];
//	VIB_PACK *edit_vib = &vib_work->vib_list[vib_work->file_num];
//	SCRIPT_PARAM *h_param = &edit_vib->param[0][vib_work->pos];
//	SCRIPT_PARAM *l_param = &edit_vib->param[1][vib_work->pos];

	sprintf(file_name,"VIBDATA%02d", vib_work->file_num );

    DEBUG_Locate( x, y, 0 );
	
	DEBUG_Printf( "TYPE  <%s> [X: type change]\n",ftype[vib_work->vib_type]);
	DEBUG_Printf( "FILE_NAME   <%s>\n", file_name );
//	DEBUG_Printf( "PLAY\n");
	DEBUG_Printf( "POW <%03d> LEN <%03d>\n",vib_work->temp.pow,vib_work->temp.len);
	DEBUG_Printf( "TIME<%03d>\n",vib_work->temp.from+vib_work->temp.len);
	DEBUG_Printf( "COM<%s> [L2: change]\n",comstr[vib_work->com_num]);
//	DEBUG_Printf( "This mode isn't finished\n");

}

static void Edit_Menu_Corsor()
{
	int 			pad = PreviewKey0.auto_status;
	int 			press = PreviewKey0.press;
	VIB_PACK_WORK 	*edit_vib = &vib_work->vib_list[vib_work->file_num];
	SCRIPT_PARAM 	*h_param = edit_vib->h_param;
	SCRIPT_PARAM 	*l_param = edit_vib->l_param;
	SCRIPT_PARAM 	*edit_param;

	if(vib_work->vib_type){
		edit_param = &edit_vib->l_param[vib_work->pos];
	}else{
		edit_param = &edit_vib->h_param[vib_work->pos];
	}

	if(pad & PAD_U) vib_work->cursor--;
	if(pad & PAD_D) vib_work->cursor++;
	if(vib_work->cursor < 0) vib_work->cursor = EDIT_MAX-1;
	if(vib_work->cursor >= EDIT_MAX) vib_work->cursor = 0;
	
	switch( vib_work->cursor ){
	  case EDIT_FILE:
		if(pad & PAD_L) vib_work->file_num--;
		if(pad & PAD_R) vib_work->file_num++;
		if(vib_work->file_num < 0) vib_work->file_num = MAX_VIBFILE_NUM - 1;
		if(vib_work->file_num >= MAX_VIBFILE_NUM) vib_work->file_num = 0;
		break;
	  case EDIT_PLAY:
		if(press & PAD_A){
			NewPadVibration( (unsigned char*)h_param , 1 );
			NewPadVibration( (unsigned char*)l_param , 2 );
		}
		break;
	  case EDIT_PRAM:
		if(pad & PAD_L) vib_work->pos--;
		if(pad & PAD_R) vib_work->pos++;
		if(vib_work->pos < 0) vib_work->pos = 0;
		if(vib_work->pos >= MAX_PARAM) vib_work->pos = MAX_PARAM-1;
		if(pad & (PAD_L|PAD_R)){
			if( vib_work->pos < vib_work->debug_print_pos )vib_work->debug_print_pos = vib_work->pos;
			if( vib_work->pos > vib_work->debug_print_pos + 19 )vib_work->debug_print_pos = vib_work->pos - 19;
		}
		break;
	  case EDIT_POW:
		if(vib_work->vib_type){
			if((pad & PAD_R) && !(edit_param->pow == ((!vib_work->vib_type)?127:255))) edit_param->pow++;
			if((pad & PAD_L) && edit_param->pow ) edit_param->pow--;
		}else{
			if(pad & (PAD_R | PAD_L))edit_param->pow ^= 127;
		}

		if(press & PAD_A) vib_work->pos++;
		if(press & PAD_B) vib_work->pos--;
		if(vib_work->pos >= MAX_PARAM) vib_work->pos = MAX_PARAM - 1;
		if(vib_work->pos < 0) vib_work->pos = 0;
		if(pad & (PAD_A|PAD_B)){
			if( vib_work->pos < vib_work->debug_print_pos )vib_work->debug_print_pos = vib_work->pos;
			if( vib_work->pos > vib_work->debug_print_pos + 19 )vib_work->debug_print_pos = vib_work->pos - 19;
		}

		break;
	  case EDIT_LEN:
		if(pad & PAD_R && !(edit_param->len == 127)){
			edit_param->len++;
#if 0
			if(vib_work->pos<MAX_PARAM-1 && edit_param[1].len){
				edit_param[1].len--;
			}
#endif
		}
		if((pad & PAD_L) && edit_param->len ){
			edit_param->len--;
#if 0
			if(vib_work->pos<MAX_PARAM-1 && !(edit_param[1].len == 127) && edit_param[1].len != 0){
				edit_param[1].len++;
			}
#endif
		}
		
		if(press & PAD_A) vib_work->pos++;
		if(press & PAD_B) vib_work->pos--;
		if(vib_work->pos >= MAX_PARAM) vib_work->pos = MAX_PARAM - 1;
		if(vib_work->pos < 0) vib_work->pos = 0;
		if(pad & (PAD_A|PAD_B)){
			if( vib_work->pos < vib_work->debug_print_pos )vib_work->debug_print_pos = vib_work->pos;
			if( vib_work->pos > vib_work->debug_print_pos + 19 )vib_work->debug_print_pos = vib_work->pos - 19;
		}
		break;
	  case EDIT_COMMAND:
		if(pad & PAD_R)vib_work->com_num++;
		if(pad & PAD_L)vib_work->com_num--;
		if(vib_work->com_num >= COM_MAX) vib_work->com_num = 0;
		if(vib_work->com_num < 0) vib_work->com_num = COM_MAX-1;
		if(press & PAD_A) CommamdList0[vib_work->com_num]( ((vib_work->vib_type)?l_param:h_param),vib_work->pos);
		break;
	  default:
		break;
	}
	if(press & PAD_X){
		vib_work->vib_type ^= 1;
	}
	
	if(press & PAD_L1){
		vib_work->pos--;
		if(vib_work->pos < 0) vib_work->pos = 0;
		if( vib_work->pos < vib_work->debug_print_pos )vib_work->debug_print_pos = vib_work->pos;
		if( vib_work->pos > vib_work->debug_print_pos + 19 )vib_work->debug_print_pos = vib_work->pos - 19;
	}
	if(press & PAD_R1){
		vib_work->pos++;
		if(vib_work->pos >= MAX_PARAM) vib_work->pos = MAX_PARAM-1;
		if( vib_work->pos < vib_work->debug_print_pos )vib_work->debug_print_pos = vib_work->pos;
		if( vib_work->pos > vib_work->debug_print_pos + 19 )vib_work->debug_print_pos = vib_work->pos - 19;
	}
}

static void Edit_Menu_DebugPrint()
{
	int x = LOCATE_X+27;
	int y = LOCATE_Y+(18<<1);
	char	file_name[12];
	VIB_PACK_WORK *edit_vib = &vib_work->vib_list[vib_work->file_num];
	SCRIPT_PARAM *h_param = &edit_vib->h_param[vib_work->pos];
	SCRIPT_PARAM *l_param = &edit_vib->l_param[vib_work->pos];

	sprintf(file_name,"VIBDATA%02d", vib_work->file_num );

    DEBUG_Locate( x, y, 0 );
	
	DEBUG_Printf( "TYPE  <%s> [X: type change]\n",ftype[vib_work->vib_type]);
	DEBUG_Printf( "FILE_NAME   <%s>\n", file_name );
	DEBUG_Printf( "PLAY\n");
	DEBUG_Printf( "PARAM <%2d> Hi   Lo [L1,R1]\n",vib_work->pos);
	DEBUG_Printf( "POW      <%03d><%03d>\n",h_param->pow,l_param->pow);
	DEBUG_Printf( "LEN      <%03d><%03d>\n",h_param->len,l_param->len);
	DEBUG_Printf( "COMMAND  <%s>\n",menu_list_1[vib_work->com_num]);

	x = LOCATE_X + 18;
	y = LOCATE_Y + 17*3 + (vib_work->cursor)*17;
    DEBUG_Locate( x, y, 0 );
	DEBUG_Printf( ">");
}

#if 0
int PreviewSaveBinFile(char *filename,void *buf,int size)
{
    int wfd;
	int ans=1;

    /* ファイルの書き込みオープン */
    wfd = sceOpen(filename,SCE_WRONLY|SCE_TRUNC|SCE_CREAT);
    if (wfd < 0 ) {
		/* エラー処理 */
		printf("Can't open %s\n",filename);
		return 0;
    }

    if(sceWrite(wfd,(unsigned char *)buf,size)<0){
		printf("Can't write %s\n",filename);
		ans=0;
	}
    sceClose(wfd);

	return ans;
}

int PreviewLoadBinFile(char *filename,void *buf,int size)
{
    int rfd;
	int ans=1;

    /* ファイルの読み込みオープン */
    rfd = sceOpen(filename,SCE_RDONLY);
    if (rfd < 0 ) {
		/* エラー処理 */
		printf("Can't open  %s\n",filename);
		return 0;
    }

    len = sceLseek(rfd, 0, SCE_SEEK_END); /* サイズを知って*/
    sceLseek(rfd, 0, SCE_SEEK_SET); /* offset を戻しておく */

    if(sceWrite(wfd,(unsigned char *)buf,size)<0){
		printf("Can't write %s\n",filename);
		ans=0;
	}
	
    /* ファイルサイズの確認 */
    if(sceRead(rfd,(unsigned char *)buf,size)<0){
		printf("Can't read %s\n",filename);
		ans=0;
	}
	
    sceClose(rfd);

	return ans;
}
#endif

static int GetScriptDataNum( SCRIPT_PARAM *param )
{
	SCRIPT_PARAM	*data = param;
	int				i;

	for( i = 0; data->len; i++ ) data++;

	return (i);
}

static void FileSave( VIB_PACK_WORK *vib_data )
{
    int 		wfd;
//	char		*file_Data;
	short		*End_Code;
	ALIGN16_PRE short		Align_End_Code ALIGN16_POST;
	void		*temp;
	
	// End_Code = (void*)GV_Malloc( sizeof(short) );
	End_Code = &Align_End_Code;
	if(End_Code == NULL){
		printf("ERR!! MALLOC MVERTS!!\n");
		return;
	}
	*End_Code = 0;
	
	
	vib_data->n_h_param = GetScriptDataNum( vib_data->h_param );
	vib_data->n_l_param = GetScriptDataNum( vib_data->l_param );

	printf("nh = %d:nl = %d\n",vib_data->n_h_param,vib_data->n_l_param);

#if 1
	
    // ファイルの書き込みオープン
    wfd = sceOpen(FILE_NAME_VIB,SCE_WRONLY|SCE_TRUNC|SCE_CREAT);
    if (wfd < 0 ) {
		printf("Can't open %s\n",FILE_NAME_VIB);
		return;
    }
	if(vib_data->n_h_param){
		temp = (void*)GV_Malloc( sizeof(SCRIPT_PARAM)*vib_data->n_h_param );
		if(temp == NULL){
			printf("ERR!! MALLOC MVERTS!!\n");
			return;
		}

		memcpy(temp,vib_data->h_param,sizeof(SCRIPT_PARAM)*vib_data->n_h_param);

	    // 書き込み( h_param データ)
		if( sceWrite( wfd, (unsigned char *)temp, sizeof(SCRIPT_PARAM)*vib_data->n_h_param ) < 0 ){
			printf("Can't write %s\n",FILE_NAME_VIB);
			return;
		}
		if(temp) GV_DelayedFree(temp);

	}
	// 書き込み(終了データ)
	if(sceWrite(wfd,(unsigned char *)End_Code,sizeof(short))<0){
		printf("Can't write %s\n",FILE_NAME_VIB);
		return;
	}
	if(vib_data->n_l_param){
	    // 書き込み( h_param データ)
		temp = (void*)GV_Malloc( sizeof(SCRIPT_PARAM)*vib_data->n_l_param );
		if(temp == NULL){
			printf("ERR!! MALLOC MVERTS!!\n");
			return;
		}

		memcpy(temp,vib_data->l_param,sizeof(SCRIPT_PARAM)*vib_data->n_l_param);

		if( sceWrite( wfd, (unsigned char *)temp, sizeof(SCRIPT_PARAM)*vib_data->n_l_param ) < 0 ){
			printf("Can't write %s\n",FILE_NAME_VIB);
			return;
		}
		if(temp) GV_DelayedFree(temp);
	}
	// 書き込み(終了データ)
	if(sceWrite(wfd,(unsigned char *)End_Code,sizeof(short))<0){
		printf("Can't write %s\n",FILE_NAME_VIB);
		return;
	}
	
	// if(End_Code) GV_DelayedFree(End_Code);

    sceClose(wfd);
#endif
	printf("Save OK!!\n");
}

static void FileLoad( VIB_PACK_WORK *vib_data )
{
	SCRIPT_PARAM	*temp;
    int 			rfd;
	int				len = 0;

	memset(vib_data,0,sizeof(VIB_PACK_WORK));
	
    /* ファイルの読み込みオープン */
    rfd = sceOpen(FILE_NAME_VIB,SCE_RDONLY);
    if (rfd < 0 ) {
		/* エラー処理 */
		printf("Can't open  %s\n",FILE_NAME_VIB);
		return;
    }

    len = sceLseek(rfd, 0, SCE_SEEK_END); /* サイズを知って*/
    sceLseek(rfd, 0, SCE_SEEK_SET); /* offset を戻しておく */
	printf("file size = %d\n",len);
	if(len == 0){
		printf("No Data!!\n");
		return;
	}

	temp = (SCRIPT_PARAM*)GV_Malloc( len );
	if(temp == NULL){
		printf("ERR!! MALLOC MVERTS!!\n");
		return;
	}
    // ファイルサイズの確認
    if(sceRead(rfd,(unsigned char *)temp,len)<0){
		printf("Can't read %s\n",FILE_NAME_VIB);
		return;
	}
    sceClose(rfd);
	vib_data->n_h_param = GetScriptDataNum( &temp[0] );
	memcpy( vib_data->h_param, &temp[0], sizeof(SCRIPT_PARAM)*vib_data->n_h_param );
	vib_data->n_l_param = GetScriptDataNum( &temp[vib_data->n_h_param+1] );
	memcpy( vib_data->l_param, &temp[vib_data->n_h_param+1], sizeof(SCRIPT_PARAM)*vib_data->n_l_param );

	printf("nh = %d:nl = %d\n",vib_data->n_h_param,vib_data->n_l_param);
	
	if(temp) GV_DelayedFree(temp);

	
//	if(ret > 0) memcpy(vib_data,&temp,sizeof(VIB_PACK));
//	if(ret > 0) printf("Load OK!!\n");
	
}

static void FileCopy( VIB_PACK_WORK *dst_data, VIB_PACK_WORK *sor_data )
{
	memcpy(dst_data,sor_data,sizeof(VIB_PACK_WORK));
}

static void Main_Menu_Corsor()
{
	int pad = PreviewKey0.auto_status;
	int press = PreviewKey0.press;
	
	VIB_PACK_WORK 	*edit_vib = &vib_work->vib_list[vib_work->file_num];
	SCRIPT_PARAM 	*h_param = edit_vib->h_param;
	SCRIPT_PARAM 	*l_param = edit_vib->l_param;

	
	if(pad & PAD_U) vib_work->cursor--;
	if(pad & PAD_D) vib_work->cursor++;
	if(vib_work->cursor < 0) vib_work->cursor = MODE_MAX-2;
	if(vib_work->cursor >= MODE_MAX-1) vib_work->cursor = 0;

	switch( vib_work->cursor+1 ){
	  case MODE_FILE:
		if(pad & PAD_L) vib_work->file_num--;
		if(pad & PAD_R) vib_work->file_num++;
		if(vib_work->file_num < 0) vib_work->file_num = MAX_VIBFILE_NUM - 1;
		if(vib_work->file_num >= MAX_VIBFILE_NUM) vib_work->file_num = 0;
		break;
	  case MODE_PLAY:
		if(press & PAD_A){
			NewPadVibration( (unsigned char*)h_param , 1 );
			NewPadVibration( (unsigned char*)l_param , 2 );
		}
		break;
	  case MODE_COPY:
		if(pad & PAD_L) vib_work->copy_num--;
		if(pad & PAD_R) vib_work->copy_num++;
		if(vib_work->copy_num < 0) vib_work->copy_num = MAX_VIBFILE_NUM - 1;
		if(vib_work->copy_num >= MAX_VIBFILE_NUM) vib_work->copy_num = 0;

		if(press & PAD_A) FileCopy( edit_vib, &vib_work->vib_list[vib_work->copy_num] );
		break;
	  case MODE_LOAD:
		if(press & PAD_A) FileLoad( edit_vib );
		break;
	  case MODE_SAVE:
		if(press & PAD_A) FileSave( edit_vib );
		break;
	  default:
		break;
	}
	
}

static void Main_Menu_DebugPrint()
{
	int x = LOCATE_X+27;
	int y = LOCATE_Y+18*2;
	char	file_name[12];
//	char	copy_name[12];

	sprintf(file_name,"VIBDATA%02d", vib_work->file_num );
//	sprintf(copy_name,"VBEDIT%02d.DAT", vib_work->copy_num );
    DEBUG_Locate( x, y, 0 );
	DEBUG_Printf( "FILE_NAME   <%s>\n", file_name );
	DEBUG_Printf( "PLAY\n");
	DEBUG_Printf( "DATA_COPY   <%02d>-><%02d>\n",vib_work->copy_num,vib_work->file_num);
	DEBUG_Printf( "DATA_LOAD\n");
	DEBUG_Printf( "DATA_SAVE\n");

	x = LOCATE_X + 18;
	y = LOCATE_Y + 17*2 + (vib_work->cursor)*17;
    DEBUG_Locate( x, y, 0 );
	DEBUG_Printf( ">\n");
}

static char *StrMode[]={
	"FILE_MODE ",
	"EDIT1_MODE",
	"EDIT2_MODE",
};

extern int Vibration_Control()
{
	int x = LOCATE_X+9;
	int y = LOCATE_Y;
	int press = PreviewKey0.press;
	
	if(vib_work == NULL) {
		printf("NULL\n");
		return 1;
	}
    DEBUG_Locate( x, y, 0 );
	DEBUG_Printf( "VIBRATION EDITOR2\n");
	DEBUG_Printf( " <%s> [R2: mode_change]\n",(vib_work->mode)?StrMode[vib_work->mode-MODE_EDIT+1]:StrMode[0]);
	
	switch(vib_work->mode){
	  case MODE_MAIN:
		Main_Menu_DebugPrint();
		Main_Menu_Corsor();
		if(press & PAD_R2){
			vib_work->cursor = 0;
			vib_work->com_num = 0;
			vib_work->mode = MODE_EDIT;
		}
		break;
	  case MODE_EDIT:
		Edit_Menu_DebugPrint();
		Edit_Menu_Corsor();
		if(press & PAD_R2){
			vib_work->cursor = 0;
			vib_work->com_num = 0;
			vib_work->mode = MODE_EDIT2;
			ConvParam_Vib_Edit();
		}
		break;
	  case MODE_EDIT2:
		Edit1_Menu_DebugPrint();
		Edit1_Menu_Corsor();
		if(press & PAD_R2){
			vib_work->cursor = 0;
			vib_work->com_num = 0;
			vib_work->mode = MODE_MAIN;
			ConvParam_Edit_Vib();
		}
		break;
	  default:
		break;
	}
	
	if(vib_work->mode==MODE_EDIT2){
		if(vib_work->int_invi & 1) EditData_DebugPrint();
	}else{
		if( vib_work->int_invi & 1 ) PramData_DebugPrint();
	}

	if(GV_PadData[0].analog_input & GV_PAD_ANALOG_R_USE){
		if( GV_PadData[0].right_dy > 128+64 ) vib_work->debug_print_pos++;
		if( GV_PadData[0].right_dy < 128-64 ) vib_work->debug_print_pos--;

		if(vib_work->debug_print_pos < 0)vib_work->debug_print_pos = 0;
		if(vib_work->debug_print_pos > MAX_PARAM-1)vib_work->debug_print_pos = MAX_PARAM-1;
	}

	if(GV_PadData[0].analog_input & GV_PAD_ANALOG_R_USE){
		//if( GV_PadData[0].right_dx > 128+64 ) vib_work->line_prim_pos -= (float)(GV_PadData[0].right_dy-128)/256.0f;
		//if( GV_PadData[0].right_dx < 128-64 ) vib_work->line_prim_pos -= (float)(GV_PadData[0].right_dy-128)/256.0f;
		if(abs((short)GV_PadData[0].right_dx - 128) > 64) vib_work->line_prim_pos -= (float)(GV_PadData[0].right_dx - 128)/1024.0f;
		if(vib_work->line_prim_pos > -0.95f) vib_work->line_prim_pos = -0.95f;
	}
	
	if( press & PAD_Y )vib_work->int_invi++;
	if(vib_work->mode == MODE_EDIT2)vib_work->int_invi &=  1;
	vib_work->flag = 1;

    if ( PreviewKey.press & PAD_B ){
		static FVECTOR pos = { 0.0f, 2048.0f, 0.0f, 1.0f };
		FileCopy( &G_With_Motion_Vib, &vib_work->vib_list[vib_work->file_num] );

		GM_ResetControlPosition( GM_PlayerControl, &pos );
		
		return 1;
	}
	return 0;
	//printf("line_prim_pos = %f\n",vib_work->line_prim_pos);
}

static void InitPrim0( DG_PRIM2 *prim )
{
	FVECTOR 		*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB 	*uvrgb = MEM_SCR_UV;
	int 			i;

	for( i = 0; i < 4; i++ ){
		uvrgb->r = 0x01;
		uvrgb->g = 0x01;
		uvrgb->b = 0x01;
		uvrgb->a = 0x80;

		uvrgb->f = (i<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
		uvrgb->q = 4096;
		uvrgb->u = 0x00;
		uvrgb->v = 0x00;
		DG_COPY_VEC(pos,&DG_ZeroVector);
		//pos->vz = 51.0f;
		//printf("pos vx %f:vy %f:vz %f\n",pos->vx,pos->vy,pos->vz);
		
		pos++;
		uvrgb++;
	}

	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), 4 );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), 4 );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), 4 );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), 4 );

}

static void InitPrim1( DG_PRIM2 *prim )
{
	FVECTOR 		*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB 	*uvrgb = MEM_SCR_UV;
	int 			i,j;
	float			r,g,b,dr,dg,db;
	
	pos = prim->pos[0];
	uvrgb = prim->uvrgb[0];
	
	r = 128.0f;
	g = 64.0f;
	b = 64.0f;

	dr = (64.0f - 128.0f)/(float)WARU_Y;
	dg = (64.0f - 64.0f)/(float)WARU_Y;
	db = (128.0f - 64.0f)/(float)WARU_Y;
	
	for( i = 0; i < WARU_Y+1; i++ ){
		for( j = 0; j < 2; j++ ){
			uvrgb->r = (int)r;
			uvrgb->g = (int)g;
			uvrgb->b = (int)b;
			//printf("%x %x %x\n",uvrgb->r,uvrgb->g,uvrgb->b);
			uvrgb->a = 0x80;
			uvrgb->f = (j&1)?DRAW_KICK_CODE:VERT_KICK_CODE;
			uvrgb->q = 4096;
			uvrgb->u = 0x00;
			uvrgb->v = 0x00;
			DG_COPY_VEC(pos,&DG_ZeroVector);
			pos++;
			uvrgb++;
		}
		r += dr;
		g += dg;
		b += db;
	}
	
	r = 128.0f;
	g = 128.0f;
	b = 64.0f;

	dr = (128.0f - 128.0f)/(float)WARU_X;
	dg = (128.0f - 128.0f)/(float)WARU_X;
	db = (64.0f - 64.0f)/(float)WARU_X;
	
	for( i = 0; i < WARU_X+1; i++ ){
		for( j = 0; j < 2; j++ ){
			uvrgb->r = (int)r;
			uvrgb->g = (int)g;
			uvrgb->b = (int)b;
			uvrgb->a = 0x80;

			//printf("%x %x %x\n",uvrgb->r,uvrgb->g,uvrgb->b);
			uvrgb->f = (j&1)?DRAW_KICK_CODE:VERT_KICK_CODE;
			uvrgb->q = 4096;
			uvrgb->u = 0x00;
			uvrgb->v = 0x00;
			DG_COPY_VEC(pos,&DG_ZeroVector);
		
			pos++;
			uvrgb++;
		}
		r += dr;
		g += dg;
		b += db;
	}
	
	for(i = 0; i < 6; i++ ){
		uvrgb->r = 0xa0;
		uvrgb->g = 0xa0;
		uvrgb->b = 0xa0;
		uvrgb->a = 0x80;
		uvrgb->f = (i&1)?DRAW_KICK_CODE:VERT_KICK_CODE;
		uvrgb->q = 4096;
		uvrgb->u = 0x00;
		uvrgb->v = 0x00;
		DG_COPY_VEC(pos,&DG_ZeroVector);
		pos++;
		uvrgb++;
	}
	
	memcpy( prim->pos[1],prim->pos[0], sizeof(FVECTOR)* N_VERTS1);
	memcpy( prim->uvrgb[1],prim->uvrgb[0], sizeof(DG_PRIM2_UVRGB)* N_VERTS1);
	
//	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), N_VERTS1 );
//	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), N_VERTS1 );
//	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), N_VERTS1 );
//	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), N_VERTS1 );

}

static void InitPrim2( DG_PRIM2 *prim )
{
	FVECTOR 		*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB 	*uvrgb = MEM_SCR_UV;
	int 			i;

	pos = prim->pos[0];
	uvrgb = prim->uvrgb[0];
	
	for( i = 0; i < MAX_PARAM * 4; i++ ){
		if(i < MAX_PARAM * 2){//high
			uvrgb->r = 0x00;
			uvrgb->g = 0xff;
			uvrgb->b = 0xff;
		}else{//low
			uvrgb->r = 0x00;
			uvrgb->g = 0xff;
			uvrgb->b = 0x00;
		}
		uvrgb->a = 0x80;

		uvrgb->f = (i&1)?DRAW_KICK_CODE:VERT_KICK_CODE;
		uvrgb->q = 4096;
		uvrgb->u = 0x00;
		uvrgb->v = 0x00;
		DG_COPY_VEC(pos,&DG_ZeroVector);
		
		pos++;
		uvrgb++;
	}

	memcpy( prim->pos[1], prim->pos[0], sizeof(FVECTOR)*MAX_PARAM * 4 );
	memcpy( prim->uvrgb[1], prim->uvrgb[0], sizeof(DG_PRIM2_UVRGB)*MAX_PARAM * 4 );
	
//	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), MAX_PARAM * 4 );
//	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), MAX_PARAM * 4 );
//	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), MAX_PARAM * 4 );
//	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), MAX_PARAM * 4 );

}

static int InitPrims( Work *work )
{
	DG_PRIM2 *prim;
	DG_TEX	*tex;
	
	prim = work->prim0 = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ON_CAMERA,1,4 );
	if(!prim){
		printf("null prim\n");
		return (-1);
	}
	tex = DG_GetTexture(0);
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 2, 2, 0, 0x80 ) );	
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
//	prim->raise = 4090;
	InitPrim0( prim );
	DG_InvisiblePrim2(work->prim0);

	prim = work->prim1 = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_ON_CAMERA|DG_PRIM2_ANTIALIASING,13, 26);
	if(!prim){
		printf("null prim\n");
		return (-1);
	}
	tex = DG_GetTexture(0);
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 2, 1, 0x40 ) );
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	prim->raise = 4090;
	InitPrim1( prim );
	DG_InvisiblePrim2(work->prim1);


	prim = work->prim2 = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_ON_CAMERA|DG_PRIM2_ANTIALIASING,N_PRIMS2,N_VERTS2 );
	if(!prim){
		printf("null prim\n");
		return (-1);
	}
	tex = DG_GetTexture(0);
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 2, 1, 0x40 ) );
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	prim->raise = 4095;
	InitPrim2( prim );
	DG_InvisiblePrim2(work->prim2);
	
	return 0;
}

static	int	GetResources( Work *work )
{
	work->copy_num = 0;
	work->file_num = 0;
	work->mode = 0;
	work->cursor = 0;
	work->vib_type = 0;
	work->flag = 0;
	work->int_invi = 1;
	work->com_num = 0;
	work->test = 0;
//	work->pow = 0;
//	work->len = 0;
	memset(work->vib_list,0,sizeof(VIB_PACK)*MAX_VIBFILE_NUM);
	work->temp.pow = 0;
	work->temp.len = 0;
	work->temp.from = 0;

	work->line_prim_pos = -0.95f;
	
	work->debug_print_pos = 0;
	
	memset(work->buf,0,sizeof(EDIT_PARAM)*(255+3));
	memset(&work->edit_temp,0,sizeof(EDIT_PACK));	
//	memset(work->vib_list,0,sizeof(VIB_PACK)*MAX_VIBFILE_NUM);
	memset(work->vib_list,0,sizeof(VIB_PACK_WORK)*MAX_VIBFILE_NUM);
	
	if(InitPrims( work )<0){
		printf("null prim\n");
		return -1;
	}
	PreKeyInit0();

	memset(&G_With_Motion_Vib,0,sizeof(VIB_PACK_WORK));
	return 0 ;
}

//プレビュー呼び出し
void *NewVibrationEditorSet()
{
	Work		*work ;

	if(vib_work) return NULL;
	
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		vib_work = work;
	}
	printf("N_VERTS1 = %d\n",N_VERTS1);
	return (void *)work ;
}
