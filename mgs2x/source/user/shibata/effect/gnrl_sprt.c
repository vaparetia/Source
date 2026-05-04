//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    gnrl_sprt.c
    汎用スプライト表示
	2000/09/28 T.Shibata
	
	$Id: gnrl_sprt.c,v 1.1.1.3 2002/11/19 11:48:37 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include	"gameheader.h"

#include	"../util/ts_util.h"

#define		N_PRIMS		(1)
#define		N_VERTS		(1)

#define		DEF_COL_R	(0x80)
#define		DEF_COL_G	(0x80)
#define		DEF_COL_B	(0x80)
#define		DEF_COL_A	(0x80)

#define		GET_COL_R(_rgba)	((_rgba)&0xff)
#define		GET_COL_G(_rgba)	(((_rgba)>>8)&0xff)
#define		GET_COL_B(_rgba)	(((_rgba)>>16)&0xff)
#define		GET_COL_A(_rgba)	(((_rgba)>>24)&0xff)

#define		DEF_SIZE	(400)

//内部フラグ
#define		GSPRT_FLAGS_MODE	(0x0001)
#define		GSPRT_FLAGS_LIFE	(0x0002)
#define		GSPRT_FLAGS_SHIFT	(0x0004)
#define		GSPRT_FLAGS_RGBA	(0x0008)

#define		GSPRT_FLAGS_MESG	(0x0010)

#define		GSPRT_FLAGS_INVSBL	(0x0100)


/*
//----------ここからコピーして使ってみたりして。

extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );

//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )

#define GSPRT_MODE_FIXCOR	(0x00100000)	// カラー固定起動時のみ
#define GSPRT_MODE_FOG		(0x00200000)	// フォグが掛かる
#define GSPRT_MODE_TEX		(0x00000000)	// 描画モード　テクスチャーそのまま　起動時のみ
#define GSPRT_MODE_ADD		(0x00010000)	// 描画モード　加算　起動時のみ
#define GSPRT_MODE_SUB		(0x00020000)	// 描画モード　減算　起動時のみ
#define GSPRT_MODE_ALPHA	(0x00030000)	// 描画モード　アルファ　起動時のみ

#define GSPRT_MODE_VSBL		(0x80000001)	// 表示にする
#define GSPRT_MODE_INVSBL	(0x80000002)	// 非表示にする
#define GSPRT_MODE_KILL		(0x80000010)	// 非表示にして殺す

//----------ここまで
*/

//外部モード
#define GSPRT_MODE_FIXCOR	(0x00100000)	// カラー固定起動時のみ
#define GSPRT_MODE_FOG		(0x00200000)	// フォグが掛かる
#define GSPRT_MODE_TEX		(0x00000000)	// 描画モード　テクスチャーそのまま　起動時のみ
#define GSPRT_MODE_ADD		(0x00010000)	// 描画モード　加算　起動時のみ
#define GSPRT_MODE_SUB		(0x00020000)	// 描画モード　減算　起動時のみ
#define GSPRT_MODE_ALPHA	(0x00030000)	// 描画モード　アルファ　起動時のみ

#define GSPRT_MODE_ALP_CK	(0x00030000)	// 描画モード　チェック用

#define GSPRT_MODE_VSBL		(0x80000001)	// 表示にする
#define GSPRT_MODE_INVSBL	(0x80000002)	// 非表示にする
#define GSPRT_MODE_KILL		(0x80000010)	// 非表示にして殺す

#define GSPRT_MODE_CALL		(0x80000000)	// コールチェック

typedef struct {

	GV_ACT_EX	actor ;
	DG_PRIM2	*prim;
	int			life;
	FVECTOR		*pos;
	float		shift;

	int			*rgba;
	int			*mode;
	int			flags;
	int			name;
} Work ;


static void Act(Work *work)
{	
	int					clock;
	FVECTOR				*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	//モードチェック
	if(work->flags & GSPRT_FLAGS_MODE){
		int		mode = *work->mode;
		
		if(mode & GSPRT_MODE_CALL){	
			//呼び出しあり
			
			if(mode & 0x0001){
				//表示
				work->flags &= ~(GSPRT_FLAGS_INVSBL);
				DG_VisiblePrim2(work->prim);
			}
			if(mode & 0x0002){
				//非表示
				work->flags |= GSPRT_FLAGS_INVSBL;
				DG_InvisiblePrim2(work->prim);
			}
			if(mode & 0x0010){
				//殺す
				work->flags |= GSPRT_FLAGS_INVSBL;
				DG_InvisiblePrim2(work->prim);
				GV_DestroyActor( work );
				//printf("mode kill\n");
			}
			*work->mode = 0;
		}
	}else if(work->flags & GSPRT_FLAGS_MESG){
		//
		GV_MSG *msg;
		int mes_num;
		int num;

		mes_num=GV_ReceiveMessage( work->name, &msg );
		msg += mes_num-1;

		while( --mes_num >= 0 ){
			num=msg->message[0];
			switch( num ){
			case 0:
				//表示
				work->flags &= ~(GSPRT_FLAGS_INVSBL);
				DG_VisiblePrim2(work->prim);				
				break;
			case 1:
				//非表示
				work->flags |= GSPRT_FLAGS_INVSBL;
				DG_InvisiblePrim2(work->prim);
				break;
			default:
				printf("gnrl_sprt.c Msg Err!![%d]\n",num);
				break;
			}
			msg--;
		}
	}

	//タイマー
	if((work->flags & GSPRT_FLAGS_LIFE) && (--work->life < 0)){
		work->flags |= GSPRT_FLAGS_INVSBL;
		DG_InvisiblePrim2(work->prim);
		GV_DestroyActor( work );
		//printf("life 0\n");
	}

	//非表示の時処理しない
	if(work->flags & GSPRT_FLAGS_INVSBL) return;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	pos = work->prim->pos[clock];
	uvrgbwh = work->prim->uvrgb[clock];

	if(work->pos){
		DG_COPY_VEC( pos, work->pos);
		//シフトあり
		if(work->flags & GSPRT_FLAGS_SHIFT){
			FVECTOR		shift;
		
			_sceVu0SubVector( &shift, (FVECTOR*)DG_Chanls[0].eye.m[3],pos );
			_sceVu0Normalize( &shift, &shift );
			_sceVu0ScaleVectorXYZ( &shift, &shift, work->shift );
			_sceVu0AddVector( pos, pos, &shift);	
		}
	}

	//カラー変更あり
	if(work->flags & GSPRT_FLAGS_RGBA){
		uvrgbwh->r = GET_COL_R((*work->rgba));
		uvrgbwh->g = GET_COL_G((*work->rgba));
		uvrgbwh->b = GET_COL_B((*work->rgba));
		uvrgbwh->a = GET_COL_A((*work->rgba));
	}

}

static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim);
	//printf("die\n");
}

static void InitPrimData( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int w, int h )
{
	FVECTOR				*pos0,*pos1;
	DG_PRIM2_UVRGBWH	*uvrgbwh0,*uvrgbwh1;

	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgbwh0 = prim->uvrgb[0];
	uvrgbwh1 = prim->uvrgb[1];

	DG_COPY_VEC( pos0, work->pos );
	DG_COPY_VEC( pos1, work->pos );

	uvrgbwh0->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	uvrgbwh0->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	uvrgbwh0->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	uvrgbwh0->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	uvrgbwh0->f0 = 0x0fff;
	uvrgbwh0->f1 = 0x0fff;
	uvrgbwh0->q0 = 4096;
	uvrgbwh0->q1 = 4096;

	uvrgbwh0->w = w;
	uvrgbwh0->h = h;
	if(work->rgba){
		uvrgbwh0->r = GET_COL_R((*work->rgba));
		uvrgbwh0->g = GET_COL_G((*work->rgba));
		uvrgbwh0->b = GET_COL_B((*work->rgba));
		uvrgbwh0->a = GET_COL_A((*work->rgba));
	}else{
		uvrgbwh0->r = DEF_COL_R;
		uvrgbwh0->g = DEF_COL_G;
		uvrgbwh0->b = DEF_COL_B;
		uvrgbwh0->a = DEF_COL_A;
	}
	
	*uvrgbwh1 = *uvrgbwh0;

}


static int GetResources( Work *work, int tex_code, FVECTOR *pos, float shift,
						int *rgba, int w, int h, int life, int *mode )
{
	DG_PRIM2	*prim = NULL;
	DG_TEX		*tex = NULL;

	tex = DG_GetTexture(tex_code);
	if(!tex) { printf("ERR!! NO TEX!! <gnrl_sprt.c>\n"); return(-1); }

	prim = work->prim = GM_MakePrim2(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
									 N_PRIMS,
									 N_VERTS );
	if(!prim){ printf("ERR!! MAKE PRIM!! <gnrl_sprt.c>\n"); return (-1); }

	DG_ConfigPrim2Tex( prim, tex );

	if(mode){
		switch((*mode) & GSPRT_MODE_ALP_CK){
		  case GSPRT_MODE_ADD:
			DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
			break;
		  case GSPRT_MODE_SUB:
			DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
			break;
		  case GSPRT_MODE_ALPHA:
			DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
			break;
		  default:
			break;
		}
	}
	//DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	
	// ERR CHECK
	if( pos == NULL ) return -1;
	if( mode == NULL && life == 0 ) return -1;
	
	// DataInit
	work->pos = pos;
	work->shift = shift;
	work->mode = mode;
	work->life = life;
	work->rgba = rgba;

	//ACT FLAG
	work->flags = 0;
	if(mode) work->flags |= GSPRT_FLAGS_MODE;
	if(life) work->flags |= GSPRT_FLAGS_LIFE;
	if(rgba && !(*mode&GSPRT_MODE_FIXCOR)) work->flags |= GSPRT_FLAGS_RGBA;
	if(shift != 0.0f) work->flags |= GSPRT_FLAGS_SHIFT;

	InitPrimData( work, prim, tex, w, h );


	//printf("flags = %x\n",work->flags);
	return (0);
}

/*

extern void GSprite_TexChange( void *p, int tex_code, int alpha_mode );

#define GSPRT_MODE_TEX		(0x00000000)	// 描画モード　テクスチャーそのまま　起動時のみ
#define GSPRT_MODE_ADD		(0x00010000)	// 描画モード　加算　起動時のみ
#define GSPRT_MODE_SUB		(0x00020000)	// 描画モード　減算　起動時のみ
#define GSPRT_MODE_ALPHA	(0x00030000)	// 描画モード　アルファ　起動時のみ

*/

void GSprite_TexChange( void *p, int tex_code )
{
	Work 	*work = (Work*)p;
	DG_TEX	*tex;
	if(!work){ printf("NULL Work [gnrl_sprite.c]\n"); return; }

	tex = DG_GetTexture(tex_code);
	if(!tex) { printf("ERR!! NO TEX!! <gnrl_sprt.c>\n"); return; }

	DG_ConfigPrim2Tex( work->prim, tex );

	switch((*work->mode) & GSPRT_MODE_ALP_CK){
	case GSPRT_MODE_ADD:
		DG_SetPrim2Alpha( work->prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		break;
	case GSPRT_MODE_SUB:
		DG_SetPrim2Alpha( work->prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
		break;
	case GSPRT_MODE_ALPHA:
		DG_SetPrim2Alpha( work->prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
		break;
	default:
		break;
	}
}

/*
extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );

	int		tex_code:		// テクスチャーコード
	FVECTOR	*pos:			// ワークにメモリ確保しません、参照のみ（NULLだめ）
	float	shift:			// ポジションからカメラ方向にシフト（0.0fの時計算しません。）
	int		*rgba:			// 0xaabbggrr 参照のみ(modeにより実体持ちます)（NULLだと固定0x80808080）
	int		w:				// スプライトのサイズ
	int		h:				// スプライトのサイズ
	int		life:			// 起動時間０だと下のフラグを見て死にます。
	int		*mode:			// これを見て表示、非表示、殺したり。参照のみ（NULLの時lifeはプラス入れて）

	modeは一回書き換えるだけでいいですこちらで０クリアーします。
*/



void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
						int *rgba, int w, int h, int life, int *mode )
{
	Work *work = NULL;

	work = (Work*) GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
				
		if(GetResources(work,tex_code,pos,shift,rgba,w,h,life,mode) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		//printf("new sprite\n");
	}

	return (void *)work ;
}

static int GetResources_Scn( Work *work )
{
	FVECTOR		pos;
	DG_PRIM2	*prim = NULL;
	DG_TEX		*tex = NULL;
	int			tex_code;
	int			mode,rgba,prim_mode = DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA;
	int			w,h;
	
	tex_code = GCL_GetOptionValue( 't', 0 );
	
	if(GCL_GetOption('p')){
		pos.vx = (float)GCL_GetNextInt();
		pos.vy = (float)GCL_GetNextInt();
		pos.vz = (float)GCL_GetNextInt();
	}else{
		pos.vx = 0.0f;
		pos.vy = 0.0f;
		pos.vz = 0.0f;
	}
	pos.vw = 1.0f;

	if(GCL_GetOption('s')){
		w = GCL_GetNextInt();
		h = GCL_GetNextInt();
	}else{
		w = 0;
		h = 0;
	}
	
	if(GCL_GetOption('c')){
		rgba = GCL_GetNextInt()&0xff;
		rgba |= (GCL_GetNextInt()&0xff)<<8;
		rgba |= (GCL_GetNextInt()&0xff)<<16;
		rgba |= (GCL_GetNextInt()&0xff)<<24;
	}else{
		rgba = 0x80808080;
	}
	
	mode = GCL_GetOptionValue( 'm', 0 );

	tex = DG_GetTexture(tex_code);
	if(!tex) { printf("ERR!! NO TEX!! <gnrl_sprt.c>\n"); return(-1); }

	if( mode & GSPRT_MODE_FOG ) prim_mode |= DG_PRIM2_FOG;

	prim = work->prim = GM_MakePrim2( prim_mode,
									  N_PRIMS,
									  N_VERTS );
	if(!prim){ printf("ERR!! MAKE PRIM!! <gnrl_sprt.c>\n"); return (-1); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	DG_ConfigPrim2Tex( prim, tex );

	switch(mode & GSPRT_MODE_ALP_CK){
	case GSPRT_MODE_ADD:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		break;
	case GSPRT_MODE_SUB:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
		break;
	case GSPRT_MODE_ALPHA:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
		break;
	default:
		break;
	}
	work->shift = (float)GCL_GetOptionValue( 'f', 0 );
	
	// DataInit
	work->pos = &pos;
	work->mode = NULL;
	work->life = 0;
	work->rgba = &rgba;

	//ACT FLAG
	work->flags = GSPRT_FLAGS_MESG;
	if(work->shift!=0.0f) work->flags |= GSPRT_FLAGS_SHIFT;

	InitPrimData( work, prim, tex, w, h );

	work->pos = NULL;
	work->rgba = NULL;

	//printf("flags = %x\n",work->flags);
	return (0);
}

void *NewGeneralSprite_Scn( int name, int map )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		work->name = name;
		if(GetResources_Scn(work) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		//printf("new sprite\n");
	}

	return (void *)work ;
}
