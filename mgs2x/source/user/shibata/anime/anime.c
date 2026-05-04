//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	anime.c
	２Ｄアニメスクリプトインタプリタ
	2000/01/21    H.TANAKA
	$Id: anime.c,v 1.1.1.3 2002/11/19 11:48:28 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef PSX2
#include <sifdev.h>
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include	"utl_dma.h"

#include	"mgs_type.h"
#include	"anime.h"
//#include	"fpu_pack.h"

/*----------------------------------------------------------------*/
                    /*
		       補助マクロ
		    */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define PRINT(c)

#define N_PRIMS         1
#define N_VERTS         1
#define	MAX_LOOP	4
#define MAX_SCRIPT	4
#define	PRIM_TYPE	(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)

#define SCRIPT_RANDOM	0x80

#define MAX_SCRIPTACT	(15)

typedef struct {
    long64 count;
    u_char *start;
} Loop;

typedef	struct	{
    int			count ;		    /* カウント */
    int			anime ;		    /* 現在のアニメパターン */
    FVECTOR		speed ;			    /* 移動スピード */
    u_char		r,g,b ;			    /* 輝度値 */
    u_char		now_loop ;		    /* 現在のループ番号 */
    u_char		*data ;			   /* バイナリデータ先頭アドレス */
    u_char		*dataptr ;		   /* バイナリデータアドレス */
    short		loop_count[ MAX_LOOP ];	   /* ループカウンタ */
    u_char		*loop_s[ MAX_LOOP ];
} Unit ;

typedef	struct	{
    GV_ACT_EX	actor ;
    
    DG_PRIM2	*prim ;
    FMATRIX	*world ;                      /* 座標 */
    DG_TEX      *tex ;

    int		off_x,off_y;		/* テクスチャー offset */
    int		add_x,add_y;		/* テクスチャー pix_add */
	short		tri_w,tri_h;		// ＴＲＩのＷＨ
    short		n_anime;		    /* アニメパターン数 */
	short		n_anime_x;		    /* アニメパターン数 */
    short       raise;                       /* 優先上げ */
    short		amb ;			     /* 半透明属性 */
    int         mode;                        /* モードチェック(田中) */
    short	flag;
    short	timer;
    Unit	unit;

    FVECTOR     pos ;         /* world != NULL のときのローカル座標 */

	FVECTOR		*size ;
	FVECTOR		*trg_pos;
} Work ;


/* スクリプト実行 -------------------------------------------------------*/
/* アニメパターン */

static	int		ScriptA(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{

    u_char	n ;
    Unit	*unit ;

    unit = &work->unit ;

PRINT(("a\n"));

	unit->dataptr++ ;
	n = *unit->dataptr++ ;

	if ( n == 0xff ) {
		unit->anime ++ ;
		if( unit->anime >= work->n_anime ) unit->anime = 0;
	} else if ( n == 0xfe ) {
		unit->anime = BP_PS2_rand() % work->n_anime;
	} else {
		unit->anime = n ;
	}
//	printf("a <%d>\n",n);
	return 0 ;
}

/*-----------------------------------------*/
/* 待ち時間 */
static	int		ScriptW(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{
	Unit	*unit ;
	unit = &work->unit ;
PRINT(("w\n"));

	unit->dataptr++;
	unit->count  = (*unit->dataptr++)*256;
	unit->count += (*unit->dataptr++);

//	printf("w <%d>\n",unit->count);
	return 1 ;
}

/*-----------------------------------------*/
/* 位置情報 */
static	int		ScriptP(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{
	Unit	*unit ;
	float	ftemp;

	unit = &work->unit ;
PRINT(("p\n"));
	unit->dataptr++;
	ftemp=(float)((char)(*unit->dataptr++)*256);
	ftemp+=(float)(*unit->dataptr++);
	pos->vx = ftemp;

	ftemp=(float)((char)(*unit->dataptr++)*256);
	ftemp+=(float)(*unit->dataptr++);
	pos->vy = ftemp;

	ftemp=(float)((char)(*unit->dataptr++)*256);
	ftemp+=(float)(*unit->dataptr++);
	pos->vz = ftemp;

//	printf("p <x:%f y:%f z:%f>\n",pos->vx,pos->vy,pos->vz);
	
	return 0 ;
}

/*-----------------------------------------*/
/* 位置移動 */
static	int		ScriptM(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{
	Unit	*unit ;
	float	ftemp;

	unit = &work->unit ;

PRINT(("m\n"));

//printf("a1:%f:%f:%f\n",work->pos.vx,work->pos.vy,work->pos.vz);
//printf("a1:%d\n",(int)*unit->dataptr);

	unit->dataptr++;
	ftemp=(float)((char)(*unit->dataptr++)*256);
	ftemp+=(float)(*unit->dataptr++);
	pos->vx += ftemp;

	ftemp=(float)((char)(*unit->dataptr++)*256);
	ftemp+=(float)(*unit->dataptr++);
	pos->vy += ftemp;

	ftemp=(float)((char)(*unit->dataptr++)*256);
	ftemp+=(float)(*unit->dataptr++);
	pos->vz += ftemp;

	return 0 ;
}

/*-----------------------------------------*/
/* スピード */
static	int		ScriptS(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{
	Unit	*unit ;
	float	ftemp;

	unit = &work->unit ;
PRINT(("s\n"));

	unit->dataptr++;
	ftemp=(float)((char)(*unit->dataptr++)*256);
	ftemp+=(float)(*unit->dataptr++);
	unit->speed.vx = ftemp;

	ftemp=(float)((char)(*unit->dataptr++)*256);
	ftemp+=(float)(*unit->dataptr++);
	unit->speed.vy = ftemp;

	ftemp=(float)((char)(*unit->dataptr++)*256);
	ftemp+=(float)(*unit->dataptr++);
	unit->speed.vz = ftemp;

	return 0 ;
}

/*-----------------------------------------*/
/* 重力 */
static	int		ScriptG(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{
	Unit	*unit ;
	float	ftemp;
	unit = &work->unit ;
PRINT(("g\n"));

	unit->dataptr++;
	ftemp=(float)((char)(*unit->dataptr++)*256);
	ftemp+=(float)(*unit->dataptr++);
	unit->speed.vx += ftemp;

	ftemp=(float)((char)(*unit->dataptr++)*256);
	ftemp+=(float)(*unit->dataptr++);
	unit->speed.vy += ftemp;

	ftemp=(float)((char)(*unit->dataptr++)*256);
	ftemp+=(float)(*unit->dataptr++);
	unit->speed.vz += ftemp;


	return 0 ;
}

/*-----------------------------------------*/
/* ? */
static	int		ScriptT(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{
	Unit	*unit ;
	unit = &work->unit ;
	
	unit->dataptr++;
	uvrgbwh->a += ((u_short)(*work->unit.dataptr++)) ;
//	printf("t [%d]\n",*unit->dataptr++);
PRINT(("t\n"));
	
	return 0 ;
}

/*-----------------------------------------*/
/* 色設定 */
static	int		ScriptV(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{
//    char	r,g,b;
PRINT(("v\n"));
	work->unit.dataptr++;
	uvrgbwh->r += ((u_short)(*work->unit.dataptr++)) ;
	uvrgbwh->g += ((u_short)(*work->unit.dataptr++)) ;
	uvrgbwh->b += ((u_short)(*work->unit.dataptr++)) ;

//	printf("v <r:%d g:%d b:%d>\n",r,g,b);
	return 0 ;
}

/*-----------------------------------------*/
/* 点光源 */
static	int		ScriptF(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{
	float	n;
	float   e;

PRINT(("f\n"));

	work->unit.dataptr++;
	n  = (float)(*work->unit.dataptr++)*256.0f;
	n += (float)(*work->unit.dataptr++);

//	printf("dir %5.1f\n",n) ;
	e = n / 2.0F ;
	DG_SetTmpLight2( pos, e, n,0x608090,LIT_FLAG_CHARAONLY|LIT_FLAG_BGONLY );
	
	return 0 ;
}

/*-----------------------------------------*/
/* サイズ */
static	int		ScriptZ(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{
	float	x,y;

PRINT(("z\n"));

	work->unit.dataptr++;
	x  = (float)((char)(*work->unit.dataptr++)*256) ;
	x += (float)( *work->unit.dataptr++) ;
	y  = (float)((char)(*work->unit.dataptr++)*256) ;
	y += (float)( *work->unit.dataptr++) ;
#if 0
	if(work->size){
		work->size->vx = x*2.0f;
		work->size->vy = y*2.0f;
		work->size->vz = 300.0f;
	}
#endif
	uvrgbwh->w += (short)(x / 2.0F) ;
	uvrgbwh->h += (short)(y / 2.0F) ;


	
	return 0 ;
}

/*-----------------------------------------*/
/* ループ開始 ? */
static	int		ScriptO(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{
	Unit	*unit ;
	short	n ;
//	int		now_loop ;

PRINT(("o\n"));

	unit = &work->unit ;

	unit->dataptr ++ ;
	n = (*unit->dataptr)*256 ;
	unit->dataptr ++ ;
	n += (*unit->dataptr) ;
	unit->dataptr ++ ;

	unit->now_loop ++ ;
	unit->loop_count[ unit->now_loop ]  = n;
	unit->loop_s[ unit->now_loop ] = unit->dataptr ;

	return 0 ;
}

/*-----------------------------------------*/
/* ループ終了 */
static	int		ScriptC(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{
	Unit	*unit ;
	int		now_loop, count ;

PRINT(("c\n"));
	unit = &work->unit ;


	now_loop = unit->now_loop ;
//printf("%d\n",now_loop);
	count = -- unit->loop_count[ now_loop ] ;

//printf("%d\n",count);

	if ( count > 0 ) {
//		unit->loop_count[ now_loop ] -- ;
		unit->dataptr = unit->loop_s[ now_loop ] ;
	} else if ( count == 0 ) {
		unit->now_loop -- ;
		unit->dataptr += 1 ;
	} else {
		unit->dataptr = unit->loop_s[ now_loop ] ;
	}

	return 0 ;
}

/*-----------------------------------------*/
static	int		ScriptL(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{
    int        addr ;
    GV_ACT_EX  *p ;

    addr = (int)work ;
    DG_InvisiblePrim2( work->prim );
    /* 死ぬ前にメッセージを送る */
    p = (GV_ACT_EX *)work ;
    if(!(p->actor.class & GV_CLASS_EX) || (p->actor.class & GV_CLASS_CHILD)){
		addr = (int)work ;
		GV_CallParentSignalFunc(work,GV_SIGNAL_DIE,addr) ;
    }
    GV_DestroyActor( work ) ;
    return 1 ;
}

/*-----------------------------------------*/
static	int		ScriptE(
Work      *work,
FVECTOR   *pos,
DG_PRIM2_UVRGBWH  *uvrgbwh
)
{
    int    addr ;
    GV_ACT_EX  *p ;
    
    addr = (int)work ;

    DG_InvisiblePrim2( work->prim );
    /* 死ぬ前にメッセージを送る */
    p = (GV_ACT_EX *)work ;
    if(!(p->actor.class & GV_CLASS_EX) || (p->actor.class & GV_CLASS_CHILD)){
		addr = (int)work ;
		GV_CallParentSignalFunc(work,GV_SIGNAL_DIE,addr) ;
    }
    GV_DestroyActor( work ) ;

//	printf("Kill Me\n");
    return 1 ;
}

/*----------------------------------------------------------------*/
static	int	( *ScriptAct[] )( Work *,FVECTOR *,DG_PRIM2_UVRGBWH * ) = {
	ScriptA,
	ScriptW,
	ScriptP,
	ScriptM,
	ScriptS,

	ScriptG,
	ScriptT,
	ScriptV,
	ScriptF,
	ScriptZ,

	ScriptO,
	ScriptC,
	ScriptL,
	ScriptE
} ;
/*----------------------------------------------------------------*/
 
#if 0

static  int  ReceiveSignal( void *workp, int signal, int value)
{
    Work *work = workp ;
    int  addr  ;

    addr = (int)work ;

    /* デストロイ */
    if(signal == GV_SIGNAL_KILL){
		if((addr == value) || (addr < 0))
			GV_DestroyActor(work) ;
    }
}

#endif

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	Unit	          *unit ;
	int	          n ;
	DG_PRIM2          *prim ;
	FVECTOR           tmp_pos, *pos ;
	DG_PRIM2_UVRGBWH  tmp_uvrgbwh, *uvrgbwh ;
	
	
	DG_TEX            *tex ;
	float              x,y ;
	FVECTOR            tex_uv ;
	int               addr ;
	GV_ACT_EX         *p ;


	unit = &work->unit ;
	tex  = work->tex ;

	/* 情報を取得 */
	prim = work->prim ;
	
	/* 常に現在のワールド */
	
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;

	/* ワールドが存在しているとき ローカル座標を渡す */
	if( work->world!=NULL) {
	    tmp_pos = work->pos ;
	}
	else
	{
	    tmp_pos  = *(FVECTOR *)prim->pos[prim->buffer_clock] ;
	}
	tmp_uvrgbwh = *(DG_PRIM2_UVRGBWH *)prim->uvrgb[prim->buffer_clock] ;

	DG_VisiblePrim2( prim );

	if ( unit->count <= 0 ) {
		do {
			n = ( *(unit->dataptr) & 0x7f );
			if( n <= 0 || n >= MAX_SCRIPTACT ){
			    printf( "SCRIPT ACT ERR!! %d\n", n ) ;
			    /* 死ぬ前にメッセージを送る */
			    p = (GV_ACT_EX *)work ;
			    if(!(p->actor.class & GV_CLASS_EX) || 
			       (p->actor.class & GV_CLASS_CHILD)){
					addr = (int)work ;
					GV_CallParentSignalFunc(work,GV_SIGNAL_DIE,addr) ;
			    }
			    GV_DestroyActor( work ) ;
			    break ;
			}
		} while ( !(( *ScriptAct[ n - 1 ] )( work,&tmp_pos,&tmp_uvrgbwh )) ) ;
	}
//	printf("finish time %d\n",work->timer++);
	unit->count -- ;

	/* ローカル位置データ作成 */
	tmp_pos.vx += unit->speed.vx ;
	tmp_pos.vy += unit->speed.vy ;
	tmp_pos.vz += unit->speed.vz ;
	tmp_pos.vw = 1.0F ;


	/* uvrgbwh 作成 */
#if 0
	x = (float)(work->unit.anime % work->dev_x);
	y = (float)(work->unit.anime / work->dev_x);

	tex_uv.vx = tex->u_scale *  x       / (float)work->dev_x + tex->u_offset;
	tex_uv.vy = tex->v_scale *  y       / (float)work->dev_y + tex->v_offset;
	tex_uv.vz = tex->u_scale * (x+1.0f) / (float)work->dev_x + tex->u_offset;
	tex_uv.vw = tex->v_scale * (y+1.0f) / (float)work->dev_y + tex->v_offset;
#else
	x = (float)((work->unit.anime % work->n_anime_x) * work->add_x);
	y = (float)((work->unit.anime / work->n_anime_x) * work->add_y);

	tex_uv.vx = ((float)work->off_x + x + 0.5f)/(float)work->tri_w;
	tex_uv.vy = ((float)work->off_y + y + 0.5f)/(float)work->tri_h;
	tex_uv.vz = ((float)work->off_x + x + (float)work->add_x - 0.5f)/(float)work->tri_w;
	tex_uv.vw = ((float)work->off_y + y + (float)work->add_y - 0.5f)/(float)work->tri_h;

//	printf("tex2 u0 %f:v0 %f:u1 %f:v1 %f:\n",tex_uv.vx,tex_uv.vy,tex_uv.vz,tex_uv.vw);
#endif

	tmp_uvrgbwh.u0 = FTOI12(tex_uv.vx ) ;/* 左上 */
	tmp_uvrgbwh.v0 = FTOI12(tex_uv.vy ) ;/* 左上 */
	tmp_uvrgbwh.u1 = FTOI12(tex_uv.vz ) ;/* 右下 */
	tmp_uvrgbwh.v1 = FTOI12(tex_uv.vw ) ;/* 右下 */
#if 0
	/*  テクスチャの設定 */
	if(work->amb == 0)
	{
	    //DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 )) ;
	    DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 )) ;

//	    prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	}
	else
	{
	    DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 )) ;
//	    prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 2,0,0,1,0x00 ) ;
	}
#endif
	
	/* 田中 追加 */
	if ( (GM_PlayerStatus &  PLAYER_WATCH) && work->mode & AN_MODE_PWATCHINV ){
	    DG_InvisiblePrim2(work->prim );
	}else{
	    DG_VisiblePrim2(work->prim);
	}
#if 0
	if ( work->flag==1 ){
	    if( work->world==NULL) {	/* 終了要求があった */
			DG_InvisiblePrim2( work->prim );

		/* 死ぬ前にメッセージを送る */
			p = (GV_ACT_EX *)work ;
			if(!(p->actor.class & GV_CLASS_EX) || 
			   (p->actor.class & GV_CLASS_CHILD))
				{
					addr = (int)work ;
					GV_CallParentSignalFunc(work,GV_SIGNAL_DIE,addr) ;
				}
			addr = (int)work ;
			GV_CallParentSignalFunc(work,GV_SIGNAL_DIE,addr) ;
			GV_DestroyActor( work ) ;
			printf("kill anime\n");
			return;
	    }else{
			work->pos = tmp_pos ;
			DG_SetPos( work->world ) ;
			DG_PutVector(&tmp_pos,&tmp_pos,1) ;
	    }
	}
#else

	if(work->world){
		if(work->mode & AN_MODE_POSONLY){
			work->pos = tmp_pos ;
			fpu_AddVectors(&tmp_pos,&tmp_pos,(FVECTOR*)work->world->m[3]);
		}else{
			work->pos = tmp_pos ;
			DG_SetPos( work->world ) ;
			DG_PutVector(&tmp_pos,&tmp_pos,1) ;
		}
	}
	if(work->trg_pos) *work->trg_pos = tmp_pos;
	
#endif
	

	DG_SwitchBuffPrim2(prim);
	pos = prim->pos[prim->buffer_clock] ;
	uvrgbwh = prim->uvrgb[prim->buffer_clock] ;

	*pos = tmp_pos ;
	*uvrgbwh = tmp_uvrgbwh ;

	
}
/*----------------------------------------------------------------*/

static	void	Die( Work *work )
{
    if(work->prim != NULL)
	GM_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static int MakeBinData ( Work *work, ANIMATION *anmform )
{
	int		i;
	u_char	*dataptr, *scriptptr[ MAX_SCRIPT ] ;
	PRESCRIPT		*pre ;
	int scriptnum;

	dataptr = anmform->script;
	scriptnum = (int)dataptr[ 2 ];

	dataptr += 3;
	for(i = 0; i < scriptnum; i++ ){
		long64 offset;
		offset = dataptr[ 0 ] * 256 + dataptr[ 1 ];
		scriptptr[ i ] = anmform->script + offset;
		dataptr += 2;
	}

	/* 初期情報 */
	pre = anmform->pre_script ;

	/* スクリプトナンバ毎のデータポインタ初期化 */
	work->unit.dataptr = work->unit.data = scriptptr[ pre->scr_num ] ;

	return 0 ;
}

/* ワークのデータ設定 */
static void Init_Work(Work *work, ANIMATION *anmform,FMATRIX *world, FVECTOR *pos, FVECTOR *size )
{

    PRESCRIPT	*pre ;
    Unit        *unit ;

    /* ワールドデータ */
    work->world = world ;


    /* テクスチャ分割数 */
    work->n_anime = (int)anmform->n_anime;       /* アニメ数 */
	work->n_anime_x = anmform->texdev_x;

//    work->dev_x = (int)anmform->texdev_x ;
//    work->dev_y = (int)anmform->texdev_y ;
//    work->n_anime = (int)anmform->n_anime ;       /* アニメ数 */

    work->raise = anmform->raise ;                /* 優先上げ */
    work->amb     = anmform->amb ;                /* テクスチャ属性 */
    /* 田中 追加 */
    work->mode = (int)anmform->mode ;             /* モード設定 */
	work->timer = 0;
    pre = anmform->pre_script ;                 
    unit = &work->unit ;
    unit->count = 0 ;
    unit->anime = pre->s_anim ;                   /* 現在のアニメパターン */
    unit->speed = pre->speed ;                    /* スピード */
    unit->now_loop = 255 ;

	work->size = size;
	work->trg_pos = pos;

}

static	int	GetResources( Work *work, int where, ANIMATION *anmform, FMATRIX *world, FVECTOR *pos, FVECTOR *size  )
{
	DG_TEX		   *tex ;
	DG_PRIM2           *prim ;
	FVECTOR            *pos_a,*pos_b ;
	DG_PRIM2_UVRGBWH   *uvrgbwh_a, *uvrgbwh_b ;
//	float              x,y ;
	FVECTOR            tex_uv ;
	int					prim_type = PRIM_TYPE;
	
	if( work->amb & AN_PRIMTYPE_FOG ) prim_type |= DG_PRIM2_FOG;
	/* プリミティブ作成 */
	work->prim = prim = GM_MakePrim2( prim_type, N_PRIMS, N_VERTS) ;
	if(prim==NULL){ printf("ANIME.C:ERROR  no memory :2\n"); return -1; }

	/* テクスチャ作成 */
	tex = work->tex = DG_GetTexture( (int)anmform->tex );
	if(tex == NULL){ printf("ERR!! NO TEX!! anim.c\n"); return -1; }

	DG_ConfigPrim2Tex( prim, tex ); 
	//printf("raise = %d\n",prim->raise);
	prim->raise = anmform->raise;
	/* ワークのデータ設定 */
	Init_Work(work,anmform,world,pos,size) ;
#if 0
	/* テクスチャタイプ設定 */
	if(work->amb == 0){
		//DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 2, 0x80 )) ;
		DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 )) ;
		//	    prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	}else{
		DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0 )) ;
		//	    prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 2,0,0,1,0 ) ;
	}
#else
	switch(work->amb & 0x000f){
	  case AN_ALPHA_ADD://加算
		DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 )) ;
		break;
	  case AN_ALPHA_SUB://減算
		DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0 )) ;
		break;
	  case AN_ALPHA_ALPHA://アルファ
		DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 )) ;
		break;
	  default://テクスチャーのをそまま
		break;
	}
#endif

	DG_GetTexelInfo( (int*)&work->add_x, (int*)&work->add_y, (int*)&work->off_x, (int*)&work->off_y, tex);
	work->add_x /= work->n_anime_x;
	work->add_y /= (work->n_anime/work->n_anime_x);

	work->tri_w = 1 << ((tex->tex_trans.tex0.data >> 26) & 0x0f);
	work->tri_h = 1 << ((tex->tex_trans.tex0.data >> 30) & 0x0f);

	/* 位置設定 */
	pos_a = prim->pos[0] ;
	pos_b = prim->pos[1] ;
	pos_a[0] = anmform->pre_script->pos ;
	pos_b[0] = pos_a[0] ;
	work->pos = pos_a[0] ;

	/* uvrgbwh 設定 */
	uvrgbwh_a = prim->uvrgb[0] ;
	uvrgbwh_b = prim->uvrgb[1] ;
	
	uvrgbwh_a->w = (short)(anmform->size_w / 2) ;
	uvrgbwh_a->h = (short)(anmform->size_h / 2) ;
	uvrgbwh_a->r = (u_short)anmform->v ;
	uvrgbwh_a->g = (u_short)anmform->v ;
	uvrgbwh_a->b = (u_short)anmform->v ;
	uvrgbwh_a->a = 128 ;


#if 0
//	x = (float)(work->unit.anime % work->dev_x);
//	y = (float)(work->unit.anime / work->dev_x);
	tex_uv.vx=tex->u_scale* x      /(float)work->dev_x + tex->u_offset;
	tex_uv.vy=tex->v_scale* y      /(float)work->dev_y + tex->v_offset;
	tex_uv.vz=tex->u_scale*(x+1.0f)/(float)work->dev_x + tex->u_offset;
	tex_uv.vw=tex->v_scale*(y+1.0f)/(float)work->dev_y + tex->v_offset;
#else
	tex_uv.vx = ((float)(work->off_x + 0) + 0.5f)/(float)work->tri_w;
	tex_uv.vy = ((float)(work->off_y + 0) + 0.5f)/(float)work->tri_h;
	tex_uv.vz = ((float)(work->off_x + work->add_x) - 0.5f)/(float)work->tri_w;
	tex_uv.vw = ((float)(work->off_y + work->add_y) - 0.5f)/(float)work->tri_h;
#endif

	uvrgbwh_a->u0 = FTOI12(tex_uv.vx ) ;/* 左上 */
	uvrgbwh_a->v0 = FTOI12(tex_uv.vy ) ;/* 左上 */
	uvrgbwh_a->u1 = FTOI12(tex_uv.vz ) ;/* 右下 */
	uvrgbwh_a->v1 = FTOI12(tex_uv.vw ) ;/* 右下 */
	uvrgbwh_a->q0 = 4096 ;
	uvrgbwh_a->q1 = 4096 ;
	uvrgbwh_a->f0 = 0x0fff ;
	uvrgbwh_a->f1 = 0x0fff ;
	uvrgbwh_b[0] = uvrgbwh_a[0] ;

	DG_InvisiblePrim2(prim);

	if( MakeBinData( work, anmform ) < 0 ){
		printf("ANIME.C:ERROR: SCRIPT_PART\n");
		return -1 ;
	}
	
	/* シグナル設定 */
//	GV_SetActorSignalFunc( work, ReceiveSignal ) ;
	return 0 ;
}

/* ----------------------------------------------------- */
	/*
		プログラム呼び出し
	*/
void* NewAnime( FMATRIX *world, int where, ANIMATION *anmform )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	
	if ( work != NULL ) {
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, where, anmform, world,NULL,NULL ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}


void* NewAnimeWithTrgt( FMATRIX *world, int where, ANIMATION *anmform, TARGET *trg, FVECTOR *pos, FVECTOR *size )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	
	if ( work != NULL ) {
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, where, anmform, world, pos, size ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

