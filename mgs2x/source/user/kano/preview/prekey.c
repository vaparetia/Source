/*
	prekey.c
		キーリピート対応

	1999/12/13 K.Kano
	$Id: prekey.c,v 1.1.1.3 2002/11/19 11:43:30 Yoshizawa1 Exp $
*/


#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"

#include "prekey.h"


void PreKeyInit(void)
{
    PreviewKey.status=PreviewKey.auto_status
		=PreviewKey.press=0;
    PreviewKey.repeat_count=0;
}

void PreKeyAct(void)
{
    PreviewKey.press=GV_PadData[1].press;

    if(GV_PadData[1].status && PreviewKey.status==GV_PadData[1].status){
		if(PreviewKey.repeat_count>PreviewKey.repeat_next_count){
			if(PreviewKey.repeat_next_count>REPEAT_SECOND){
				PreviewKey.repeat_count=PreviewKey.repeat_next_count;
			}
			else{
				PreviewKey.repeat_next_count+=REPEAT_NEXT;
			}
			PreviewKey.auto_status=PreviewKey.status;
		}
		else PreviewKey.auto_status=0;

		PreviewKey.repeat_count++;
    }
    else{
		PreviewKey.auto_status
			=PreviewKey.status=GV_PadData[1].status;
		PreviewKey.repeat_count=0;
		PreviewKey.repeat_next_count=REPEAT_FIRST;
    }
}



#define KEYBRD_BUF_SIZE	0x40

#define REPEAT_FIRST	20
#define REPEAT_NEXT		3
#define REPEAT_SECOND	(REPEAT_FIRST+30)


typedef struct {
	unsigned char keybrd_code[4];
	int keybrd_repeat_count[4];
	int keybrd_repeat_next_count[4];

	int keybrd_buf_in,keybrd_buf_out;
	unsigned char keybrd_buf[KEYBRD_BUF_SIZE];
} Keybrd_Work;

Keybrd_Work KeybrdW;

int KeybrdGetChar(Keybrd_Work *KeybrdW)
{
	int ans;

	if(KeybrdW->keybrd_buf_in==KeybrdW->keybrd_buf_out) return -1;

	ans=KeybrdW->keybrd_buf[KeybrdW->keybrd_buf_out];

	KeybrdW->keybrd_buf_out++;
	if(KeybrdW->keybrd_buf_out>=KEYBRD_BUF_SIZE) KeybrdW->keybrd_buf_out=0;

	return ans;
}


void KeybrdPutChar(Keybrd_Work *KeybrdW,int mask,int key)
{
	static const short normal_code[]={
		/* 0x0? */
		0  ,0  ,0  ,0  ,   'a','b','c','d',   'e','f','g','h',   'i','j','k','l',
		/* 0x1? */
		'm','n','o','p',   'q','r','s','t',   'u','v','w','x',   'y','z','1','2',
		/* 0x2? */
		'3','4','5','6',   '7','8','9','0',   0x0d,0x1b,0x09,0x09,   ' ',0  ,0  ,0  ,
		/* 0x3? */
		0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,
		/* 0x4? */
		0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,
		/* 0x5? */
		0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,
		/* 0x6? */
		0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,
		/* 0x7? */
		0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,
	};
	static const short shift_code[]={
		/* 0x0? */
		0  ,0  ,0  ,0  ,   'A','B','C','D',   'E','F','G','H',   'I','J','K','L',
		/* 0x1? */
		'M','N','O','P',   'Q','R','S','T',   'U','V','W','X',   'Y','Z','!','\"',
		/* 0x2? */
		'#','$','%','&',   '\'','(',')',0  ,   0x0d,0x1b,0x09,0x09,   ' ',0  ,0  ,0  ,
		/* 0x3? */
		0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,
		/* 0x4? */
		0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,
		/* 0x5? */
		0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,
		/* 0x6? */
		0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,
		/* 0x7? */
		0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,   0  ,0  ,0  ,0  ,
	};
	int code;

	if(mask & KBD_MASK_CTRL){
		code=0;
	}
	else if(mask & KBD_MASK_ALT){
		code=0;
	}
	else if(mask & KBD_MASK_SHIFT){
		code=shift_code[key];
	}
	else{
		code=normal_code[key];
	}

	if(code==0) return;

	KeybrdW->keybrd_buf[KeybrdW->keybrd_buf_in]=code;

	KeybrdW->keybrd_buf_in++;
	if(KeybrdW->keybrd_buf_in>=KEYBRD_BUF_SIZE) KeybrdW->keybrd_buf_in=0;

	if(KeybrdW->keybrd_buf_in==KeybrdW->keybrd_buf_out){
		KeybrdW->keybrd_buf_out++;
		if(KeybrdW->keybrd_buf_out>=KEYBRD_BUF_SIZE){
			KeybrdW->keybrd_buf_out=0;
		}
	}
}

void KeybrdAct(Keybrd_Work *KeybrdW)
{
	int i,j;

	for(i=0;i<MAX_KEYCODE;i++){
		if(KeybrdW->keybrd_code[i]==0) continue;

		for(j=0;j<MAX_KEYCODE;j++){
			if(GV_KeyData.status[j]==0) continue;

			if(KeybrdW->keybrd_code[i]==GV_KeyData.status[j]){
				/* 既存の入力 */
				if(KeybrdW->keybrd_repeat_count[i]>KeybrdW->keybrd_repeat_next_count[i]){

					if(KeybrdW->keybrd_repeat_next_count[i]>REPEAT_SECOND){
						KeybrdW->keybrd_repeat_count[i]
							=KeybrdW->keybrd_repeat_next_count[i];
					}
					else{
						KeybrdW->keybrd_repeat_next_count[i]
							+=REPEAT_NEXT;
					}

					KeybrdPutChar(KeybrdW,GV_KeyData.mask,GV_KeyData.status[j]);
				}

				KeybrdW->keybrd_repeat_count[i]++;

				break;
			}
		}
		if(j==MAX_KEYCODE){
			/* 入力が終了 */
			KeybrdW->keybrd_code[i]=0;
		}
	}

	for(i=0;i<MAX_KEYCODE;i++){
		if(GV_KeyData.status[i]!=0) continue;

		for(j=0;j<MAX_KEYCODE;j++){
			if(KeybrdW->keybrd_code[j]==0) continue;
			if(KeybrdW->keybrd_code[j]==GV_KeyData.status[i]) break;
		}
		if(j==MAX_KEYCODE){
			/* 新規の入力 */
			for(j=0;j<MAX_KEYCODE;j++) if(KeybrdW->keybrd_code[j]==0) break;

			KeybrdW->keybrd_code[j]=GV_KeyData.status[i];
			KeybrdW->keybrd_repeat_count[j]=0;
			KeybrdW->keybrd_repeat_next_count[j]=REPEAT_FIRST;

			KeybrdPutChar(KeybrdW,GV_KeyData.mask,GV_KeyData.status[i]);
		}
	}
}
