//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

   メリケン粉 サラサラ落ちる      1
	2000/04/26 T.Shibata
	を拝借して
	2002/08/22 Y.Korekado改造

	$Id: gll_flour_sara.c,v 1.2 2002/11/25 01:54:46 Yoshizawa1 Exp $
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

#include	"def_dma.h"
#include	"utl_dma.h"

#define		N_PRIMS		(1)
#define		N_VERTS		(32)

#define		N_PRIMS1	(N_VERTS-8)
#define		N_VERTS1	(6)

#define		GRAVITY		(-2.0f)

#define		COLOR_R		(0x80)
#define		COLOR_G		(0x80)
#define		COLOR_B		(0x80)
#define		ALPHA		(0x80)

#define		S_TIMER_RATE	(4)
#define		M_TIMER_RATE	(40)

#define		SPEED_SCALE 	(8.0f)
#define		SIDE_LEN 		(32.0f)
#define		SIDE_WIDE_LEN 		(1024.0f)
#define		LIFE_TIME 		(8)

#define		SARA
/* パフパフ */
#define 	PAF
#define		PAF_SIZE			(64.0f)
#define		PAF_SPEED			(1.0f)
#define		PAF_TIME			(8)
#define		PAF_ALPHA			(0x20)
#define		PAF_GRAVITY			(-1.0f)
#define		PAF_WAIT			(16)
/* hxz    */
#define		HXZ_CHECK		(0x0003)	    /* 0:ない 1:床あり 2:天井あり 3:両方 */
#define		HXZ_FLR			(0x0001)		/* 床 */
#define		HXZ_CEL			(0x0002)		/* 天井  */
/* フラグ */
#define		FLAGS_FALL		(0x0001)	/* 落ちとりまっせ */
#define		FLAGS_FLRFIND	(0x0002)	/* 床発見 */

#define		FLAGS_END		(0x0004)

#define		PAF_FLAG_INIT	(0x0001)
/* check scr add over */
#ifdef BP_PSX2_GCC
#define		MEM_SCR0	((void *)( SCRPAD_ADDR))
#define		MEM_SCR1	((void *)( MEM_SCR0 ) + sizeof(FVECTOR)*N_VERTS)
#define		MEM_SCR2	((void *)( MEM_SCR1 ) + sizeof(DG_PRIM2_UVRGB)*N_VERTS)

#define		MEM_SCRx	((void *)( SCRPAD_ADDR))
#define		MEM_SCRy	((void *)( MEM_SCRx ) + sizeof(FVECTOR)*N_VERTS1*N_PRIMS1)
#define		MEM_SCRz	((void *)( MEM_SCRy ) + sizeof(DG_PRIM2_UVRGBWH)*N_VERTS1*N_PRIMS1)

#define		MEM_SCRa	((void *)( SCRPAD_ADDR))
#define		MEM_SCRb	((void *)( MEM_SCRa ) + sizeof(FVECTOR)*N_VERTS)
#define		MEM_SCRc	((void *)( MEM_SCRb ) + sizeof(FVECTOR)*N_VERTS)

#define		MEM_SCR_A	((void *)( SCRPAD_ADDR))												//m_pos
#define		MEM_SCR_B	((void *)( MEM_SCR_A ) + sizeof(FVECTOR)*N_VERTS)						//pos
#define		MEM_SCR_C	((void *)( MEM_SCR_B ) + sizeof(FVECTOR)*N_PRIMS1*N_VERTS1)				//before_pos
#define		MEM_SCR_D	((void *)( MEM_SCR_C ) + sizeof(FVECTOR)*N_PRIMS1*N_VERTS1)				//uvrgbwh
#define		MEM_SCR_E	((void *)( MEM_SCR_D ) + sizeof(DG_PRIM2_UVRGBWH)*N_VERTS1*N_PRIMS1)	//dvec
#define		MEM_SCR_F	((void *)( MEM_SCR_E ) + sizeof(FVECTOR)*N_PRIMS1*N_VERTS1)				//r_angle
#else
#define		MEM_SCR0	((void *)( SCRPAD_ADDR))
#define		MEM_SCR1	((void *)( (char *)MEM_SCR0 + sizeof(FVECTOR)*N_VERTS) )
#define		MEM_SCR2	((void *)( (char *)MEM_SCR1 + sizeof(DG_PRIM2_UVRGB)*N_VERTS) )

#define		MEM_SCRx	((void *)( SCRPAD_ADDR))
#define		MEM_SCRy	((void *)( (char *)MEM_SCRx + sizeof(FVECTOR)*N_VERTS1*N_PRIMS1) )
#define		MEM_SCRz	((void *)( (char *)MEM_SCRy + sizeof(DG_PRIM2_UVRGBWH)*N_VERTS1*N_PRIMS1) )

#define		MEM_SCRa	((void *)( SCRPAD_ADDR))
#define		MEM_SCRb	((void *)( (char *)MEM_SCRa + sizeof(FVECTOR)*N_VERTS) )
#define		MEM_SCRc	((void *)( (char *)MEM_SCRb + sizeof(FVECTOR)*N_VERTS) )

#define		MEM_SCR_A	((void *)( SCRPAD_ADDR))												//m_pos
#define		MEM_SCR_B	((void *)( (char *)MEM_SCR_A + sizeof(FVECTOR)*N_VERTS) )						//pos
#define		MEM_SCR_C	((void *)( (char *)MEM_SCR_B + sizeof(FVECTOR)*N_PRIMS1*N_VERTS1) )				//before_pos
#define		MEM_SCR_D	((void *)( (char *)MEM_SCR_C + sizeof(FVECTOR)*N_PRIMS1*N_VERTS1) )				//uvrgbwh
#define		MEM_SCR_E	((void *)( (char *)MEM_SCR_D + sizeof(DG_PRIM2_UVRGBWH)*N_VERTS1*N_PRIMS1) )	//dvec
#define		MEM_SCR_F	((void *)( (char *)MEM_SCR_E + sizeof(FVECTOR)*N_PRIMS1*N_VERTS1) )				//r_angle
#endif



/* extern */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
extern  void  _BigScrCopy( void *dat, void *src, int size, int num ) ;
extern  void  _BigMemCopy( void *dst, void *src, int size, int num ) ;
extern	void *NewFlour_Moku( FVECTOR *center );

typedef struct
{
	GV_ACT_EX			actor;
	
	DG_PRIM2		*prim;
	int				map;
	int				name;
	
	int				timer;
	int				m_flags;
	float			angle;
	float			flr_h;
	
	FVECTOR			center;
	FVECTOR			force;
	FVECTOR			sound_pos;
	
	FVECTOR			dvec[N_VERTS];
	FVECTOR			pos[N_VERTS];

	int				ver_cnt;
	int				s_timer;
	int				sound_wait;
	int				pad[1];
	
	FVECTOR			dvec1[N_PRIMS1*N_VERTS1];
	float			r_angle[N_PRIMS1*N_VERTS1];
	int				flags[N_VERTS];
	short			paflags[N_VERTS1*N_PRIMS1];

	
	HZX_GROUP_ID	map_id;

	int				mode ;

    TARGET			off ;
	FVECTOR			start_pos ;
	FVECTOR			flr_pos ;
	FVECTOR			target_pos ;
	FVECTOR			moku_pos ;
	int				trg_flag ;
	float			dirf ;
} Work ;


extern void  *NewSpreadFlour(FMATRIX *world ) ;


static	void	Hit( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;

    work = ( Work * )ptr ;
    work->trg_flag |= 1 ;
    DG_COPY_VEC( &work->target_pos, &off->hit ) ;

//printf("vx %f:vy %f:vz %f\n", off->hit.vx, off->hit.vy, off->hit.vz);
}

static	void	SearchTarget( Work *work )
{
    TARGET	*t ;

    t = &work->off ;
    GM_SetTarget( t, TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN |
				 TARGET_POWER | TARGET_THROUGH, GM_CurrentStageMap, ENEMY_SIDE,
				 &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( t, WP_WEAPONCORE ) ;
    GM_SetTargetName( t, WP_Rgb6 ) ;
    GM_SetTargetCallBack( t, Hit, work ) ;

    GM_MoveOnlineTargetMap( t, &work->start_pos, &work->flr_pos, GM_StageMapAll ) ;
    GM_PutTarget( t ) ;
//NewTargetView( t, 200, 34, 184 ) ;
    work->trg_flag = 0 ;
}

static void RotPos(Work *work, FVECTOR *out_fv,FVECTOR *offset,int sign)
{

	FVECTOR   	fvtemp;
	FVECTOR  	eye;
	float 		eye_ang, scale ;

	if ( work->mode == 1 ) {
		scale = sign * SIDE_WIDE_LEN;
	} else {
		scale = sign * SIDE_LEN;
	}

#if 1
	fvtemp.vx = scale * cosf(work->dirf);
	fvtemp.vy = 0.0f;
	fvtemp.vz = scale * -sinf(work->dirf);
#else
	DG_COPY_VEC(&eye,&DG_Chanls[DG_CHANL_MAIN].eye.m[3]);

	//_sceVu0SubVector(&eye,offset,&eye);
	fpu_SubVectors(&eye,offset,&eye);

	eye_ang = -atanf(fpu_Abs(eye.vz)/fpu_Abs(eye.vx));

	//printf("vx %f:vy %f:vz %f\n");

	fvtemp.vx = scale * cosf(eye_ang + PI/2);
	fvtemp.vy = 0.0f;
	fvtemp.vz = scale * -sinf(eye_ang + PI/2);
#endif

	//_sceVu0AddVector( &fvtemp, &fvtemp, offset);
	fpu_AddVectors( &fvtemp, &fvtemp, offset);
	
	DG_COPY_VEC(out_fv,&fvtemp);
//printf("vx %f:vy %f:vz %f\n",out_fv->vx,out_fv->vy,out_fv->vz);

}

static void SetData( Work *work, FVECTOR *pos, FVECTOR *before_pos, FVECTOR *dvec)
{

	int 		i;
	FVECTOR		*wpos = work->pos;
//	FVECTOR		*scr_pos = MEM_SCR0;
//	FVECTOR		*scr_vec = MEM_SCR2;

	for( i = 0; i < N_VERTS; i++ ){
		work->flags[i] = FLAGS_FALL;

		DG_COPY_VEC(pos,&work->center);
		DG_COPY_VEC(wpos,&work->center);
		DG_COPY_VEC(before_pos,&work->center);
		DG_COPY_VEC(dvec,&work->force);		

		dvec++;
		pos++;
		before_pos++;
		wpos++;


	}

}

static void Act_sara(Work *work)
{
	DG_PRIM2_UVRGB		*uvrgb0, *uvrgb1;
	int 				j,sign = 1,clock,*flags;
	FVECTOR 			*pos,*dvec,*wpos;
	FVECTOR		  		fvtemp;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	wpos = work->pos;
	dvec = work->dvec;

	fpu_AddVectors( &fvtemp, wpos, dvec );
	//_sceVu0AddVector( &fvtemp, wpos, dvec );
	//AN_Test_Eye2( &fvtemp, 2 );

	if(work->m_flags & FLAGS_FALL){	/* ハザードチェック */		
		if ( !(work->s_timer%4) ){
			FMATRIX world ;

			world = DG_UnitMatrix ;
			DG_COPY_VEC( (FVECTOR *)world.m[3], &fvtemp );
			NewSpreadFlour(&world) ;
		}

		if( fvtemp.vy < work->flr_h){
			work->m_flags &= ~(FLAGS_FALL);
		}
	}

	_BigScrCopy( MEM_SCRa,work->pos  ,sizeof(FVECTOR),N_VERTS);
	_BigScrCopy( MEM_SCRb,work->dvec ,sizeof(FVECTOR),N_VERTS);

	wpos = MEM_SCRa;
	dvec = MEM_SCRb;
	pos = MEM_SCRc;
	flags = work->flags;

	uvrgb0 = work->prim->uvrgb[0] ;
	uvrgb1 = work->prim->uvrgb[1] ;

//printf("		uvrgb0->a = %d\n",		uvrgb0->a )  ;

	for( j = 0; j < N_VERTS ; j++ ){
		if(*flags & FLAGS_FALL && j < work->s_timer/S_TIMER_RATE){
			fpu_AddVectors( wpos, wpos, dvec );
			//AN_Test_Eye2( wpos, 2 );
			dvec->vy += GRAVITY;
			if(wpos->vy < work->flr_h){
				wpos->vy = work->flr_h;
				*flags &= ~(FLAGS_FALL);
				if(++work->ver_cnt >= N_VERTS){
				    /* 終了処理開始 */
					work->timer = work->s_timer;
					work->m_flags |= FLAGS_END;
					//printf("All Down\n");
				}
			}
		}

		if ( work->ver_cnt > 1 ) {
			if ( uvrgb0->a > 4 ) uvrgb0->a -= 4 ;
			else 				 uvrgb0->a =  0;
			uvrgb1->a = uvrgb0->a ;
		}

		RotPos(work,pos,wpos,sign);

		sign *= -1;

		pos++;
		dvec++;
		wpos++;
		flags++;
		uvrgb0++ ;
		uvrgb1++ ;
	}
	_BigMemCopy( work->pos ,MEM_SCRa,sizeof(FVECTOR),N_VERTS);
	_BigMemCopy( work->dvec,MEM_SCRb,sizeof(FVECTOR),N_VERTS);
	_BigMemCopy( work->prim->pos[clock],MEM_SCRc,sizeof(FVECTOR),j);

	SearchTarget( work ) ;
}

static void Act(Work *work)
{

	Act_sara(work);

	if(++work->s_timer > LIFE_TIME + work->timer){
		DG_InvisiblePrim2(work->prim) ;
		GV_DestroyActor( work );
	}

}

static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim);
}

static void InitWorkData( Work *work, DG_TEX **tex, FVECTOR *force )
{
	int 					j;
	FVECTOR					*pos;
	FVECTOR		  			*dvec;
	DG_PRIM2_UVRGB			*uvrgb;
	FVECTOR					fvtemp;
	float					f_len;
	float					u_off,v_off,u_scl,v_scl,k;

//printf("init\n");
	pos = MEM_SCR0;
	uvrgb = MEM_SCR1;
	dvec = MEM_SCR2;

	DG_COPY_VEC(&fvtemp, force);
	fvtemp.vy = 0.0f;
	/* fvtemp の大きさを１にしてから */
	f_len = _sceVu0InnerProduct( &fvtemp, &fvtemp );
	f_len = bp_sqrtf(f_len);   //BP_MATH - emulate PS2 sqrtf
	fvtemp.vx = -(SPEED_SCALE * fvtemp.vx)/f_len;
	fvtemp.vz = -(SPEED_SCALE * fvtemp.vz)/f_len;
	
	work->angle = atanf(force->vz/force->vx);

	DG_COPY_VEC(&work->force, &fvtemp);
	_sceVu0AddVector( &work->center, &work->center, &work->force );
#ifdef SARA
	u_off = tex[0]->u_offset;
	v_off = tex[0]->v_offset;
	u_scl = tex[0]->u_scale;
	v_scl = tex[0]->v_scale;
	k = 1.0f / (N_VERTS / 2 - 1);

	work->ver_cnt = 0;
	work->s_timer = 0;
		
	SetData( work, pos, pos, dvec);
		
	for(j = 0; j < N_VERTS ; j++ ){
			/* uvrgb init */
		uvrgb->u = FTOI12(k * (j/2) * u_scl + u_off);
		uvrgb->v = FTOI12((j % 2) * v_scl + v_off);
		uvrgb->q = 4096;
		    /*フラグ（描画キック：0x0fff、頂点キックのみ：0x8fff）*/
		if( j <= 1 )
			uvrgb->f = 0x8fff;
		else
			uvrgb->f = 0x0fff;
		
		uvrgb->a = ALPHA;
		uvrgb->r = COLOR_R;
		uvrgb->g = COLOR_G;
		uvrgb->b = COLOR_B;
		
		uvrgb++;
		
	}
	pos+=N_VERTS;
	dvec+=N_VERTS;

	
	_BigMemCopy(work->prim->pos[0],MEM_SCR0,sizeof(FVECTOR),N_PRIMS * N_VERTS);
	_BigMemCopy(work->prim->uvrgb[0],MEM_SCR1,sizeof(DG_PRIM2_UVRGB),N_PRIMS * N_VERTS);
	_BigMemCopy(work->prim->pos[1],MEM_SCR0,sizeof(FVECTOR),N_PRIMS * N_VERTS);
	_BigMemCopy(work->prim->uvrgb[1],MEM_SCR1,sizeof(DG_PRIM2_UVRGB),N_PRIMS * N_VERTS);
	_BigMemCopy(work->dvec,MEM_SCR2,sizeof(FVECTOR),N_PRIMS * N_VERTS );

#endif

}


static int GetResources( Work *work, int map, int name, FVECTOR *hit, FVECTOR *force, int mode )
{
	DG_PRIM2	*prim = NULL;
	DG_TEX		*tex[2] = { NULL , NULL };

//printf("get\n");
#ifdef SARA
	//tex[0] = DG_GetTexture(GV_StrCode("powder01_alp"));
	/* さらさら落ちる奴 */
//	tex[0] = DG_GetTexture(GV_StrCode("powder03_alp"));
	tex[0] = DG_GetTexture(11047070);
	if(!tex[0]){
		printf(" not texture int flour_Down.c \n");
		return(-1);
	}
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
									  N_PRIMS,
									  N_VERTS );
	if(!prim){
		printf("null prim\n");
		return (-1);
	}

	DG_ConfigPrim2Tex( prim, tex[0] );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	GM_GroupPrim2( prim, map ) ;
	/* まだチャンネル１～３は無いが取りあえず フラグ立て */
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
#endif
	
	/* DataInit */
	work->timer = 0xffffff;
	work->map = map;
	work->name = name;
	work->m_flags = FLAGS_FALL;
	work->center.vx = hit->vx;
	work->center.vy = hit->vy;
	work->center.vz = hit->vz;
	work->flr_h = 4000.0f;
	work->sound_wait = 0;
	work->mode = mode;
	DG_COPY_VEC( &work->sound_pos, &DG_ZeroVector );
	//work->map_id = GM_GetHzxGroupID( GM_CurrentStageMap );
	GM_GroupPrim2( prim, map ) ;
	work->map_id = GM_GetHzxGroupID( map );
	InitWorkData( work, tex ,force);

    work->trg_flag = 0 ;
    work->flr_pos = work->start_pos = work->center ;
    work->start_pos.vy -= 2000.0 ;
    work->flr_pos.vy = -10000.0f ;
    work->moku_pos = work->flr_pos ;
    {
		int dir ;
		
		dir = GV_VecDir2( force ) ;
		work->dirf = ( float )M_PI * (float)dir / 2048.0F ;
	}



//	NewGllFlourMoku(&work->moku_pos);

	GM_SeSetMode( SD_A_KOMFAL01, hit, GM_SEMODE_BOMB ) ;
	return (0);
}

/*
    map:
    name:
	hit:	当たった場所
	force:	力の向き
*/

void *NewGllFlourSara( int map, int name, FVECTOR *hit, FVECTOR *force, int mode )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(GetResources(work,map,name,hit,force,mode) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
