//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

   メリケン粉 サラサラ落ちる      1
	2000/04/26 T.Shibata

	$Id: flour_sara.c,v 1.1.1.3 2002/11/19 11:48:51 Yoshizawa1 Exp $

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
#if 0 //BP
//#ifdef PSX2
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
	DG_PRIM2		*prim1;
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

} Work ;


static void RotPos(FVECTOR *out_fv,FVECTOR *offset,int sign)
{

	FVECTOR   	fvtemp;
	FVECTOR  	eye;
	float 		eye_ang,scale = sign * SIDE_LEN;
	
	DG_COPY_VEC(&eye,&DG_Chanls[DG_CHANL_MAIN].eye.m[3]);

	//_sceVu0SubVector(&eye,offset,&eye);
	fpu_SubVectors(&eye,offset,&eye);

	eye_ang = -atanf(fpu_Abs(eye.vz)/fpu_Abs(eye.vx));

	//printf("vx %f:vy %f:vz %f\n");

	fvtemp.vx = scale * cosf(eye_ang + PI/2);
	fvtemp.vy = 0.0f;
	fvtemp.vz = scale * -sinf(eye_ang + PI/2);

	//_sceVu0AddVector( &fvtemp, &fvtemp, offset);
	fpu_AddVectors( &fvtemp, &fvtemp, offset);
	
	DG_COPY_VEC(out_fv,&fvtemp);

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
		unsigned int 	init_flr_flag;
		HZX_FLR			flr[2];
		int				flr_atrs[2];
		float			flr_height[2];
				
		init_flr_flag = HZX_LevelHazardCheck( work->map_id,
											 &fvtemp,
											 HZX_CHK_F_FLOOR,
											 HZX_FLOOR_NO_PLAYER );
		//printf(" flr_Flag = %x\n",init_flr_flag);
		if( init_flr_flag & HXZ_FLR){
			HZX_GetLevelHazard( flr, flr_atrs );
            /* flr_height[2] 0:flr 1:cel*/
			HZX_GetLevelHeight( flr_height );
			work->m_flags |= FLAGS_FLRFIND;

			/* ここでモクモク生成 */
#if 1
			if( flr_height[1] == work->flr_h){
				//printf("ahoaho man\n");
				fvtemp.vy = work->flr_h;
				//AN_Test_Eye2( &fvtemp, 2 );
				NewFlour_Moku(&fvtemp);
				
//				GM_SeSetMode( SD_A_KOMFAL01, &fvtemp, GM_SEMODE_BOMB ) ;
			}
#endif
			work->flr_h = flr_height[0];
		}else{
			if(!(work->m_flags & FLAGS_FLRFIND)){
				DG_InvisiblePrim2(work->prim) ;
				GV_DestroyActor(work) ;
				printf("Kill flour_Fall\n");
				return;
			} else {
				work->m_flags &= ~(FLAGS_FALL);
			    /* 最後にモクモク生成 */
				fvtemp.vy = work->flr_h;
				//AN_Test_Eye2( &fvtemp, 2 );
				NewFlour_Moku(&fvtemp);
				work->sound_wait |= 0x80000000;
				
				DG_COPY_VEC( &work->sound_pos, &fvtemp );
				//GM_SeSetMode( SD_A_KOMFAL01, &fvtemp, GM_SEMODE_BOMB ) ;
			}
		}
	}
if( work->sound_wait & 0x80000000 ){
	work->sound_wait++;
	if( (work->sound_wait & 0xffff) > 4 ){
		GM_SeSetMode( SD_A_KOMFAL01, &work->sound_pos, GM_SEMODE_BOMB ) ;
		work->sound_wait = 0;
	}
		
}
	_BigScrCopy( MEM_SCRa,work->pos  ,sizeof(FVECTOR),N_VERTS);
	_BigScrCopy( MEM_SCRb,work->dvec ,sizeof(FVECTOR),N_VERTS);

	wpos = MEM_SCRa;
	dvec = MEM_SCRb;
	pos = MEM_SCRc;
	flags = work->flags;

	for( j = 0; j < N_VERTS ; j++ ){
		if(*flags & FLAGS_FALL && j < work->s_timer/S_TIMER_RATE){
			//_sceVu0AddVector( wpos, wpos, dvec );
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
		RotPos(pos,wpos,sign);

		sign *= -1;
	
		pos++;
		dvec++;
		wpos++;
		flags++;
	}
	_BigMemCopy( work->pos ,MEM_SCRa,sizeof(FVECTOR),N_VERTS);
	_BigMemCopy( work->dvec,MEM_SCRb,sizeof(FVECTOR),N_VERTS);
	_BigMemCopy( work->prim->pos[clock],MEM_SCRc,sizeof(FVECTOR),j);
}

static inline void _AddVec( FVECTOR *output, FVECTOR *v0, FVECTOR *v1 )
{

#ifdef BP_PSX2_ASM
	asm volatile ("
	lqc2		vf9 ,0x00(%1)
	lqc2		vf10,0x00(%2)
    vadd        vf8 ,vf9, vf10
	sqc2		vf8 ,0x00(%0)
	": : "r"(output), "r"(v0), "r"(v1) );
#else
	output->vx = v0->vx + v1->vx;
	output->vy = v0->vy + v1->vy;
	output->vz = v0->vz + v1->vz;
	output->vw = v0->vw + v1->vw;
#endif
}


static void Act_pafu(Work *work)
{
	int 				i,j;
	FVECTOR 			*pos,*before_pos,*dvec,*m_pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	int					clock;
	float				*r_angle,angle,scale;
	short				*flags;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim1->buffer_clock;
/*
	pos = work->prim1->pos[clock];
	before_pos = work->prim1->pos[1-clock];
	uvrgbwh = work->prim1->uvrgb[clock];
	m_pos = work->pos;
	r_angle = work->r_angle;
	dvec = work->dvec1;
*/
	m_pos = MEM_SCR_A;
	pos = MEM_SCR_B;
	before_pos = MEM_SCR_C;
	uvrgbwh = MEM_SCR_D;
	dvec = MEM_SCR_E;
	r_angle = MEM_SCR_F;
	
	flags = work->paflags;
	
	_BigScrCopy( MEM_SCR_B, work->prim1->pos[clock], sizeof(FVECTOR), N_PRIMS1*N_VERTS1 );
	_BigScrCopy( MEM_SCR_C, work->prim1->pos[1-clock], sizeof(FVECTOR), N_PRIMS1*N_VERTS1 );
	_BigScrCopy( MEM_SCR_D, work->prim1->uvrgb[clock], sizeof(DG_PRIM2_UVRGBWH), N_PRIMS1*N_VERTS1 );
	_BigScrCopy( MEM_SCR_E, work->dvec1, sizeof(FVECTOR), N_PRIMS1*N_VERTS1 );
    _BigScrCopy( MEM_SCR_F, work->r_angle, sizeof(float), N_PRIMS1*N_VERTS1 );


	m_pos += 2;

	for( i = 0; i < N_PRIMS1 ; i++){
		for( j = 0; j < N_VERTS1; j++ ){

			if(uvrgbwh->a){

//				_AddVec( pos, before_pos, dvec );
				
//				_sceVu0AddVector( pos, before_pos, dvec );
				fpu_AddVectors( pos, before_pos, dvec );
				dvec->vy += PAF_GRAVITY;
				//AN_Test_Eye2( pos, 2 );
				uvrgbwh->a -= 2;
				*r_angle += 0.089f;

				uvrgbwh->w = (PAF_ALPHA - uvrgbwh->a)*8.0f*cosf(*r_angle);
				uvrgbwh->h = (PAF_ALPHA - uvrgbwh->a)*8.0f*sinf(*r_angle);
			}
			if(!uvrgbwh->a){
				if(*flags & PAF_FLAG_INIT){
						uvrgbwh->a = PAF_ALPHA;
						*flags &= ~(PAF_FLAG_INIT);
						uvrgbwh->w = 0;
						uvrgbwh->h = 0;
						DG_COPY_VEC( pos ,m_pos);
				}else{
				
					if(!(irnd()%16) && !(work->m_flags & FLAGS_END)){
						DG_COPY_VEC( pos ,m_pos);
						
						angle = rnd()*2*PI;
						scale = 4.0f * rnd() + PAF_SPEED;

						dvec->vx = scale * cosf(angle);
						dvec->vy = 0.0f;
						dvec->vz = scale * sinf(angle);

						uvrgbwh->a = PAF_ALPHA;
						*flags |= PAF_FLAG_INIT;
						uvrgbwh->w = 0;
						uvrgbwh->h = 0;
					}
				}
			}

			r_angle++;
			pos++;
			before_pos++;
			uvrgbwh++;
			dvec++;
			flags++;
		}
		m_pos++;
	}

	_BigMemCopy( work->prim1->pos[clock],MEM_SCR_B,sizeof(FVECTOR),N_PRIMS1*N_VERTS1);
	_BigMemCopy( work->prim1->uvrgb[clock],MEM_SCR_D,sizeof(DG_PRIM2_UVRGBWH),N_PRIMS1*N_VERTS1);
	_BigMemCopy( work->dvec1,MEM_SCR_E,sizeof(FVECTOR),N_PRIMS1*N_VERTS1);
	_BigMemCopy( work->r_angle,MEM_SCR_F,sizeof(float),N_PRIMS1*N_VERTS1);
}

static void Act(Work *work)
{

	Act_sara(work);

	if( work->s_timer > PAF_WAIT )
		Act_pafu(work);


	if(++work->s_timer > LIFE_TIME + work->timer){
		DG_InvisiblePrim2(work->prim) ;
		DG_InvisiblePrim2(work->prim1) ;
		GV_DestroyActor( work );
	}

}

static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim);
	if(work->prim1) GM_FreePrim2(work->prim1);
}

static void InitWorkData( Work *work, DG_TEX **tex, FVECTOR *force )
{
	int 					i,j;
	FVECTOR					*pos;
	FVECTOR		  			*dvec;
	DG_PRIM2_UVRGB			*uvrgb;
	DG_PRIM2_UVRGBWH		*uvrgbwh;
	FVECTOR					fvtemp;
	float					f_len;
	float					u_off,v_off,u_scl,v_scl,k;
	short					u0,v0,u1,v1,*flags;

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
	
#ifdef PAF
	pos = MEM_SCRx;
	uvrgbwh = MEM_SCRy;
	dvec = MEM_SCRz;
	flags = work->paflags;
	u0 = FTOI12( 0.0f * tex[1]->u_scale + tex[1]->u_offset );
	v0 = FTOI12( 0.0f * tex[1]->v_scale + tex[1]->v_offset );
	u1 = FTOI12( 1.0f * tex[1]->u_scale + tex[1]->u_offset );
	v1 = FTOI12( 1.0f * tex[1]->v_scale + tex[1]->v_offset );

	for(i = 0; i < N_PRIMS1; i++ ){
		for( j = 0; j < N_VERTS1; j++ ){
			//DG_COPY_VEC( pos ,&work->center);
			//Paf_Set_dvec( work ,dvec );
			*flags = 0;

			uvrgbwh->u0 = u0;
			uvrgbwh->v0 = v0;
			uvrgbwh->u1 = u1;
			uvrgbwh->v1 = v1;
			uvrgbwh->q0 = 4096;
			uvrgbwh->q1 = 4096;
			uvrgbwh->f0 = 0x0fff;
			uvrgbwh->f1 = 0x0fff;
			
			uvrgbwh->r = COLOR_R;
			uvrgbwh->g = COLOR_G;
			uvrgbwh->b = COLOR_B;
			
			uvrgbwh->a = 0;//128;

			f_len = work->r_angle[i*N_VERTS1+j] = 2*PI * rnd();
			uvrgbwh->w = (short)(PAF_SIZE * cosf(f_len));
			uvrgbwh->h = (short)(PAF_SIZE * sinf(f_len));

			uvrgbwh++;
			pos++;
			dvec++;
			flags++;
		}
	}	
	_BigMemCopy( work->prim1->pos[0]  , MEM_SCRx, sizeof(FVECTOR)		  , N_PRIMS1 * N_VERTS1);
	_BigMemCopy( work->prim1->uvrgb[0], MEM_SCRy, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS1 * N_VERTS1);
	_BigMemCopy( work->prim1->pos[1]  , MEM_SCRx, sizeof(FVECTOR)		  , N_PRIMS1 * N_VERTS1);
	_BigMemCopy( work->prim1->uvrgb[1], MEM_SCRy, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS1 * N_VERTS1);
	_BigMemCopy( work->dvec1		  , MEM_SCRz, sizeof(FVECTOR)		  , N_PRIMS1 * N_VERTS1);

#endif
//	printf("init2\n\n");
}


static int GetResources( Work *work, int map, int name, FVECTOR *hit, FVECTOR *force )
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
	/* パフパフ */
#ifdef PAF
//	tex[1] = DG_GetTexture(GV_StrCode("powder04_alp"));
	tex[1] = DG_GetTexture(12095646);
	if(!tex[1]){
		printf(" not texture int flour_Down.c \n");
		return(-1);
	}
	prim = work->prim1 = GM_MakePrim2(DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
									  N_PRIMS1,
									  N_VERTS1 );
	if(!prim){
		printf("null prim\n");
		return (-1);
	}

	DG_ConfigPrim2Tex( prim, tex[1] );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
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
	work->flr_h = 0.0f;
	work->sound_wait = 0;
	DG_COPY_VEC( &work->sound_pos, &DG_ZeroVector );
	//work->map_id = GM_GetHzxGroupID( GM_CurrentStageMap );
	GM_GroupPrim2( prim, map ) ;
	work->map_id = GM_GetHzxGroupID( map );
	InitWorkData( work, tex ,force);

	
	//GM_SeSetMode( SD_A_KOMFAL01, hit, GM_SEMODE_BOMB ) ;
	return (0);
}

/*
    map:
    name:
	hit:	当たった場所
	force:	力の向き
*/

void *NewFlour_Sara( int map, int name, FVECTOR *hit, FVECTOR *force )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(GetResources(work,map,name,hit,force) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
