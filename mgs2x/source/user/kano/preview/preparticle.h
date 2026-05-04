/*
	preparticle.c
		パーティクルのシミュレーション

	2000/5/15 K.Kano
	$Id: preparticle.h,v 1.1.1.3 2002/11/19 11:43:31 Yoshizawa1 Exp $
*/


#ifndef _preparticle_h_
#define _preparticle_h_


#include "../particle/partsub2.h"


typedef struct {
	int menu_cursole_pos;
	void *sim_work;
} PreviewParticle_Param;


#ifdef _WORK_DECLARED_
PreviewParticle_Param PreviewParticle;
#else
extern PreviewParticle_Param PreviewParticle;
#endif


void Particle_DebugPrint(void);
void Particle_DebugCursole(void);

void *NewPreviewParticle(void);


#endif
