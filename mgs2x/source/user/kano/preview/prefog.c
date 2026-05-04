/*
	prefog.c
		プレビュー用のフォグ管理機構

	2000/2/1 K.Kano
	$Id: prefog.c,v 1.1.1.3 2002/11/19 11:43:29 Yoshizawa1 Exp $
*/


#include "preview_def.h"


#ifndef M_PI
#define	M_PI		3.14159265358979323846	/* pi */
#endif

#define SAVEFILENAME	"host0:./save.dat"


static void Color_DebugCursole(float *c)
{
    int speed;

    if(PreviewLight.speed_mode){
		speed=0x10;
    }
    else{
		speed=0x01;
    }

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		*c=(float)(((int)*c-speed) & 0xff);
		break;
    case PAD_R:
		*c=(float)(((int)*c+speed) & 0xff);
		break;
    }
}

static void Color_DebugCursole2(FVECTOR *c)
{
    int speed;

    if(PreviewLight.speed_mode){
		speed=0x10;
    }
    else{
		speed=0x01;
    }

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		c->vx=(float)(((int)(c->vx)-speed) & 0xff);
		c->vz=c->vy=c->vx;
		break;
    case PAD_R:
		c->vx=(float)(((int)(c->vx)+speed) & 0xff);
		c->vz=c->vy=c->vx;
		break;
    }
}

static void Agree_DebugCursole(float *r)
{
    int speed;

    if(PreviewLight.speed_mode){
		speed=0x20;
    }
    else{
		speed=0x01;
    }

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		*r=(float)(((int)*r-speed) & 0xfff);
		break;
    case PAD_R:
		*r=(float)(((int)*r+speed) & 0xfff);
		break;
    }
}

static void Pos_DebugCursole(float *l)
{
    float speed;

    if(PreviewLight.speed_mode){
		speed=200;
    }
    else{
		speed=10;
    }

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		*l-=speed;
		break;
    case PAD_R:
		*l+=speed;
		break;
    }
}

/* フォグ操作 */
void Fog_DebugCursole(void)
{
    int	speed;

    if(PreviewKey.status & PAD_R2){
		PreviewLight.speed_mode=1;
		speed=0x10;
    }
    else{
		PreviewLight.speed_mode=0;
		speed=0x01;
    }

    switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
    case PAD_U:
		PreviewLight.fog_cursole_pos--;
		if(PreviewLight.fog_cursole_pos<0) PreviewLight.fog_cursole_pos=6-1;
		break;
    case PAD_D:
		PreviewLight.fog_cursole_pos++;
		if(PreviewLight.fog_cursole_pos>=6) PreviewLight.fog_cursole_pos=0;
		break;
    }

    switch(PreviewLight.fog_cursole_pos){
    case 0:
		switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
		case PAD_L:
			PreviewLight.fog_color.vx=(short)(((int)PreviewLight.fog_color.vx-speed)&255);
			break;
		case PAD_R:
			PreviewLight.fog_color.vx=(short)(((int)PreviewLight.fog_color.vx+speed)&255);
			break;
		}
		break;
    case 1:
		switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
		case PAD_L:
			PreviewLight.fog_color.vy=(short)(((int)PreviewLight.fog_color.vy-speed)&255);
			break;
		case PAD_R:
			PreviewLight.fog_color.vy=(short)(((int)PreviewLight.fog_color.vy+speed)&255);
			break;
		}
		break;
    case 2:
		switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
		case PAD_L:
			PreviewLight.fog_color.vz=(short)(((int)PreviewLight.fog_color.vz-speed)&255);
			break;
		case PAD_R:
			PreviewLight.fog_color.vz=(short)(((int)PreviewLight.fog_color.vz+speed)&255);
			break;
		}
		break;
    case 3:
		switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
		case PAD_L:
			PreviewLight.fog_color.vx=(short)(((int)PreviewLight.fog_color.vx-speed)&255);
			PreviewLight.fog_color.vz=PreviewLight.fog_color.vy=PreviewLight.fog_color.vx;
			break;
		case PAD_R:
			PreviewLight.fog_color.vx=(short)(((int)PreviewLight.fog_color.vx+speed)&255);
			PreviewLight.fog_color.vz=PreviewLight.fog_color.vy=PreviewLight.fog_color.vx;
			break;
		}
		break;
    case 4:
		Pos_DebugCursole(&(PreviewLight.fog_near));
		break;
    case 5:
		Pos_DebugCursole(&(PreviewLight.fog_far));
		break;
    }

    if(PreviewKey.press & PAD_SEL ){
		PreviewLight.fog_color.vx = FOG_DEFAULT_R ;
		PreviewLight.fog_color.vy = FOG_DEFAULT_G ;
		PreviewLight.fog_color.vz = FOG_DEFAULT_B ;
		PreviewLight.fog_near     = FOG_DEFAULT_N ;
		PreviewLight.fog_far      = FOG_DEFAULT_F ;
    }
}

void Fog_DebugPrint(void)
{
    int	x,y,yy;

    x=LOCATE_X;
    y=LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("FOG MENU");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(PreviewLight.speed_mode){
		DEBUG_Printf("<High Speed Mode :R2>");
    }
    else{
		DEBUG_Printf("<Low Speed Mode  :R2>");
    }

    x=LOCATE_X+0x08*4;
    y+=COLUMN_HEIGHT;
    yy=y;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("R    : %f[%4d]",PreviewLight.fog_color.vx/255.0f,(int)PreviewLight.fog_color.vx );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("G    : %f[%4d]",PreviewLight.fog_color.vy/255.0f,(int)PreviewLight.fog_color.vy );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("B    : %f[%4d]",PreviewLight.fog_color.vz/255.0f,(int)PreviewLight.fog_color.vz );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("<MODE: RGB>" );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("NEAR : %f",PreviewLight.fog_near );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("FAR  : %f",PreviewLight.fog_far );

    /* カーソル */
    x=LOCATE_X;
    y+=PreviewLight.fog_cursole_pos*COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf(">>>");
}


/* 環境光操作 */
void Ambient_DebugCursole(void)
{
    if(PreviewKey.status & PAD_R2){
		PreviewLight.speed_mode=1;
    }
    else{
		PreviewLight.speed_mode=0;
    }

    switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
    case PAD_U:
		PreviewLight.ambient_cursole_pos--;
		if(PreviewLight.ambient_cursole_pos<0) PreviewLight.ambient_cursole_pos=5-1;
		break;
    case PAD_D:
		PreviewLight.ambient_cursole_pos++;
		if(PreviewLight.ambient_cursole_pos>=5) PreviewLight.ambient_cursole_pos=0;
		break;
    }

    switch(PreviewLight.ambient_cursole_pos){
    case 0:
		Color_DebugCursole(&(PreviewLight.amb_lit.vx));
		break;
    case 1:
		Color_DebugCursole(&(PreviewLight.amb_lit.vy));
		break;
    case 2:
		Color_DebugCursole(&(PreviewLight.amb_lit.vz));
		break;
    case 3:
		Color_DebugCursole2(&(PreviewLight.amb_lit));
		break;
    case 4: /* Recalc Preshade */
		if(PreviewKey.press & PAD_A){
			ResetDataObjects();
		}
		break;
    }

    if(PreviewKey.press & PAD_SEL){
		PreviewLight.amb_lit.vx=AMB_DEFAULT_R;
		PreviewLight.amb_lit.vy=AMB_DEFAULT_G;
		PreviewLight.amb_lit.vz=AMB_DEFAULT_B;
    }
}

void Ambient_DebugPrint(void)
{
    int	x,y,yy;

    x=LOCATE_X;
    y=LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("AMBIENT MENU");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(PreviewLight.speed_mode){
		DEBUG_Printf("<High Speed Mode :R2>");
    }
    else{
		DEBUG_Printf("<Low Speed Mode  :R2>");
    }

    x=LOCATE_X+0x08*4;
    y+=COLUMN_HEIGHT;
    yy=y;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("R    : %f[%4d]\n",PreviewLight.amb_lit.vx/255.0f,(int)PreviewLight.amb_lit.vx );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("G    : %f[%4d]\n",PreviewLight.amb_lit.vy/255.0f,(int)PreviewLight.amb_lit.vy );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("B    : %f[%4d]\n",PreviewLight.amb_lit.vz/255.0f,(int)PreviewLight.amb_lit.vz );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("<MODE: RGB>\n" );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Recalc PRESHADE>\n" );

    /* カーソル */
    x=LOCATE_X;
    y+=PreviewLight.ambient_cursole_pos*COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf(">>>");
}

static void CalcLightVecToRot(FVECTOR *from,FVECTOR *to,FVECTOR *deg)
{
    FVECTOR tmp;
    float r;

    tmp.vx=to->vx-from->vx;
    tmp.vy=to->vy-from->vy;
    tmp.vz=to->vz-from->vz;

    r=fpu_Sqrt(tmp.vx*tmp.vx+tmp.vz*tmp.vz);
    deg->vx=atan2f(-tmp.vy,r);
    deg->vy=atan2f(tmp.vx,tmp.vz);
    deg->vz=0.0f;

    deg->vx*=2048.0f/M_PI;
    if(deg->vx<0.0f) deg->vx+=4096.0f;
    deg->vy*=2048.0f/M_PI;
    if(deg->vy<0.0f) deg->vy+=4096.0f;
}

/* 平行光源操作 */
void ForVRSlight(void)
{
    float ftemp_radius;
    float ftemp_r;

    //ftemp_radius = 10000.0f;
    ftemp_radius = 5000.0f;

    PreviewLight.forVRSlit.vy=-ftemp_radius*sinf(PreviewLight.col_dir.vx*M_PI/2048.0f);
    ftemp_r=ftemp_radius*cosf(PreviewLight.col_dir.vx*M_PI/2048.0f);

    PreviewLight.forVRSlit.vx=ftemp_r*sinf(PreviewLight.col_dir.vy*M_PI/2048.0f);
    PreviewLight.forVRSlit.vz=ftemp_r*cosf(PreviewLight.col_dir.vy*M_PI/2048.0f);
}

void VRSLight2Parallel(void)
{
    CalcLightVecToRot(&DG_ZeroVector,&(PreviewLight.forVRSlit),&(PreviewLight.col_dir));
}

static void CopyDirFromCamera(void)
{
    GM_CameraSet *now;

    /* 現行カメラから */
    now=GM_GetCurrentCameraSet( 0 ) ;
    CalcLightVecToRot(&(now->position),&(now->target),&(PreviewLight.col_dir));
    ForVRSlight();
}

void Parallel_DebugCursole(void)
{
    if(PreviewKey.status & PAD_R2){
		PreviewLight.speed_mode=1;
    }
    else{
		PreviewLight.speed_mode=0;
    }

    switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
    case PAD_U:
		PreviewLight.parallel_cursole_pos--;
		if(PreviewLight.parallel_cursole_pos<0) PreviewLight.parallel_cursole_pos=10-1;
		break;
    case PAD_D:
		PreviewLight.parallel_cursole_pos++;
		if(PreviewLight.parallel_cursole_pos>=11) PreviewLight.parallel_cursole_pos=0;
		break;
    }

    switch(PreviewLight.parallel_cursole_pos){
    case 0:
		Color_DebugCursole(&(PreviewLight.col_lit.vx));
		break;
    case 1:
		Color_DebugCursole(&(PreviewLight.col_lit.vy));
		break;
    case 2:
		Color_DebugCursole(&(PreviewLight.col_lit.vz));
		break;
    case 3:
		Color_DebugCursole2(&(PreviewLight.col_lit));
		break;
    case 4:
		Agree_DebugCursole(&(PreviewLight.col_dir.vx));
		ForVRSlight();
		break;
    case 5:
		Agree_DebugCursole(&(PreviewLight.col_dir.vy));
		ForVRSlight();
		break;
    case 6:
		Pos_DebugCursole(&(PreviewLight.forVRSlit.vx));
		VRSLight2Parallel();
		break;
    case 7:
		Pos_DebugCursole(&(PreviewLight.forVRSlit.vy));
		VRSLight2Parallel();
		break;
    case 8:
		Pos_DebugCursole(&(PreviewLight.forVRSlit.vz));
		VRSLight2Parallel();
		break;
    case 9: /* Recalc Preshade */
		if(PreviewKey.press & PAD_A){
			ResetDataObjects();
		}
		break;
    case 10: /* Copy Dir from Camera */
		if(PreviewKey.press & PAD_A){
			CopyDirFromCamera();
		}
		break;
    }

    if(PreviewKey.press & PAD_SEL){
		PreviewLight.col_dir.vx   = PLL_DEFAULT_X ;
		PreviewLight.col_dir.vy   = PLL_DEFAULT_Y ;
		PreviewLight.col_dir.vz   = PLL_DEFAULT_Z ;
		PreviewLight.col_lit.vx   = PLL_DEFAULT_R ;
		PreviewLight.col_lit.vy   = PLL_DEFAULT_G ;
		PreviewLight.col_lit.vz   = PLL_DEFAULT_B ;
		ForVRSlight();
    }
}

void Parallel_DebugPrint()
{
    int	x,y,yy;

    x=LOCATE_X;
    y=LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("PARALLEL MENU");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(PreviewLight.speed_mode){
		DEBUG_Printf("<High Speed Mode :R2>");
    }
    else{
		DEBUG_Printf("<Low Speed Mode  :R2>");
    }

    x=LOCATE_X+0x08*4;
    y+=COLUMN_HEIGHT;
    yy=y;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("R     : %f[%4d]",PreviewLight.col_lit.vx/255.0f,(int)PreviewLight.col_lit.vx );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("G     : %f[%4d]",PreviewLight.col_lit.vy/255.0f,(int)PreviewLight.col_lit.vy );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("B     : %f[%4d]",PreviewLight.col_lit.vz/255.0f,(int)PreviewLight.col_lit.vz );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("<MODE: RGB>" );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("ROT X : %d/4096",(int)(PreviewLight.col_dir.vx));

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("ROT Y : %d/4096",(int)(PreviewLight.col_dir.vy));

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("for VRS X: %f",PreviewLight.forVRSlit.vx);

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("for VRS Y: %f",PreviewLight.forVRSlit.vy);

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("for VRS Z: %f",PreviewLight.forVRSlit.vz);

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Recalc PRESHADE>\n" );

    yy+=COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Copy DIR From Camera>\n" );

    /* カーソル */
    x=LOCATE_X;
    y+=PreviewLight.parallel_cursole_pos*COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf(">>>" );
}

void Preview_LightAct(void)
{
    FVECTOR	f_temp;
    FVECTOR	yajirushi_pos;
    SVECTOR	yajirushi_rot;
    FMATRIX	mat;


    /* 平行光 */
    f_temp.vx=0.0f;
    f_temp.vy=0.0f;
    f_temp.vz=4096.0f;
    {
		SVECTOR	sdir;
		sdir.vx = (short)PreviewLight.col_dir.vx;
		sdir.vy = (short)PreviewLight.col_dir.vy;
		sdir.vz = (short)PreviewLight.col_dir.vz;
		DG_SetPos2( &DG_ZeroVector, &sdir );
    }
    DG_PutVector(&f_temp,&f_temp,1);
    DG_SetMainLightDir((int)f_temp.vx,(int)f_temp.vy,(int)f_temp.vz);
    DG_SetMainLightCol((int)PreviewLight.col_lit.vx,
					   (int)PreviewLight.col_lit.vy,
					   (int)PreviewLight.col_lit.vz);

    /* 環境光 */
    DG_SetAmbient((int)PreviewLight.amb_lit.vx,
				  (int)PreviewLight.amb_lit.vy,
				  (int)PreviewLight.amb_lit.vz);

    /* フォグ */
    DG_SetFogColor((int)PreviewLight.fog_color.vx,
				   (int)PreviewLight.fog_color.vy,
				   (int)PreviewLight.fog_color.vz);
    DG_SetFogParam(PreviewLight.fog_near,PreviewLight.fog_far );


    /* 矢印 */
    if(PreviewLight.yajirushi_objs!=NULL){
#if 0
		PreviewLight.yajirushi_objs->flag &= ~DG_FLAG_INVISIBLE ;
#else
		PreviewLight.yajirushi_objs->flag |= DG_FLAG_INVISIBLE ;
#endif
    }
    yajirushi_pos.vx=35.0f;
    yajirushi_pos.vy=-25.0f;
    yajirushi_pos.vz=100.0f;
    DG_SetPos( &DG_Chanls->eye );
    DG_PutVector( &yajirushi_pos, &yajirushi_pos, 1 );
    yajirushi_rot.vx=(short)(PreviewLight.col_dir.vx + 1024.0f);
    yajirushi_rot.vy=(short)PreviewLight.col_dir.vy;
    yajirushi_rot.vz=(short)PreviewLight.col_dir.vz;
    DG_SetPos2( &yajirushi_pos, &yajirushi_rot ) ;
    DG_GetPos( &mat ) ;
    mat.m[3][0]/=0.003f;
    mat.m[3][1]/=0.003f;
    mat.m[3][2]/=0.003f;
    mat.m[3][3]/=0.003f;
    DG_SetPos( &mat ) ;
    if(PreviewLight.yajirushi_objs!=NULL){
		DG_PutObjs( PreviewLight.yajirushi_objs );
		DG_GetLightMatrix( (FVECTOR *)(PreviewLight.yajirushi_objs->world.m[3]),
						   PreviewLight.yajirushi_lights );
    }
}

static void Preview_LightDie(void)
{
	if(PreviewLight.yajirushi_objs!=NULL){
		DG_DequeueObjs(PreviewLight.yajirushi_objs);
		DG_FreeObjs(PreviewLight.yajirushi_objs);
	}
}


void SetStageLight(void)
{
    char *filename=*(PreviewFile.stagelight_filenames+PreviewLight.lt2_select);
    extern void OK_DirVecXY( FVECTOR *from, FVECTOR *to, SVECTOR *rot );
    LIT_DEF		*lt2_def;
    FVECTOR		fvec;

    lt2_def=GV_GetCache(GV_CacheID(GV_StrCode(filename),'l'));

    DG_ResetFixedLight();
    // DG_SetFixedLight(lt2_def);
	DG_SetFixedLightMap(lt2_def,PreviewLight.where);

    /* 現行平行光から */
    fvec.vx=DG_LightMatrix.m[0][0];
    fvec.vy=DG_LightMatrix.m[1][0];
    fvec.vz=DG_LightMatrix.m[2][0];

#if 0
    OK_DirVecXY( &fvec, &DG_ZeroVector, &sdir );
    PreviewLight.col_dir.vx = (float)sdir.vx;
    PreviewLight.col_dir.vy = (float)sdir.vy;
    PreviewLight.col_dir.vz = (float)sdir.vz;
#else
    CalcLightVecToRot(&DG_ZeroVector,&fvec,&(PreviewLight.col_dir));
#endif

    PreviewLight.col_lit.vx=DG_ColorMatrix.m[0][0];
    PreviewLight.col_lit.vy=DG_ColorMatrix.m[0][1];
    PreviewLight.col_lit.vz=DG_ColorMatrix.m[0][2];

    /* 現行環境光から */
    PreviewLight.amb_lit.vx=DG_ColorMatrix.m[3][0];
    PreviewLight.amb_lit.vy=DG_ColorMatrix.m[3][1];
    PreviewLight.amb_lit.vz=DG_ColorMatrix.m[3][2];
}

void Preview_GetNowLightData(void)
{
    extern void OK_DirVecXY( FVECTOR *from, FVECTOR *to, SVECTOR *rot );
    FVECTOR	fvec;

    printf("c1:%f %f %f \n",
		   DG_ColorMatrix.m[ 3 ][ 0 ],
		   DG_ColorMatrix.m[ 3 ][ 1 ],
		   DG_ColorMatrix.m[ 3 ][ 2 ]);
    printf("c2:%f %f %f \n",
		   DG_LightMatrix.m[ 0 ][ 0 ],
		   DG_LightMatrix.m[ 1 ][ 0 ],
		   DG_LightMatrix.m[ 2 ][ 0 ]);
    printf("c3:%f %f %f \n",
		   DG_ColorMatrix.m[ 0 ][ 0 ],
		   DG_ColorMatrix.m[ 0 ][ 1 ],
		   DG_ColorMatrix.m[ 0 ][ 2 ]);

    /* 現行平行光から */
    fvec.vx=DG_LightMatrix.m[0][0];
    fvec.vy=DG_LightMatrix.m[1][0];
    fvec.vz=DG_LightMatrix.m[2][0];

#if 0
    OK_DirVecXY( &fvec, &DG_ZeroVector, &sdir );
    PreviewLight.col_dir.vx = (float)sdir.vx;
    PreviewLight.col_dir.vy = (float)sdir.vy;
    PreviewLight.col_dir.vz = (float)sdir.vz;
#else
    CalcLightVecToRot(&DG_ZeroVector,&fvec,&(PreviewLight.col_dir));
#endif

    PreviewLight.col_lit.vx=DG_ColorMatrix.m[0][0];
    PreviewLight.col_lit.vy=DG_ColorMatrix.m[0][1];
    PreviewLight.col_lit.vz=DG_ColorMatrix.m[0][2];

    /* 現行環境光から */
    PreviewLight.amb_lit.vx=DG_ColorMatrix.m[3][0];
    PreviewLight.amb_lit.vy=DG_ColorMatrix.m[3][1];
    PreviewLight.amb_lit.vz=DG_ColorMatrix.m[3][2];

    /* 現行フォグから */
    PreviewLight.fog_color.vx=(short)DG_FogColor.r;
    PreviewLight.fog_color.vy=(short)DG_FogColor.g;
    PreviewLight.fog_color.vz=(short)DG_FogColor.b;
    // DG_FogParam1 = 255.0f / ( near - far ) ;
    // DG_FogParam2 = 255.0f - DG_FogParam1 * near ;

    if( DG_FogParam1==0.0f || DG_FogParam2==0.0f ){
		PreviewLight.fog_near = 0.0f;
		PreviewLight.fog_far  = 65536.0f;
    }else{
		PreviewLight.fog_near = (255.0f-DG_FogParam2)/DG_FogParam1;
		PreviewLight.fog_far  = -DG_FogParam2/DG_FogParam1;
    }

    //printf("%f %f\n",PreviewLight.fog_near,PreviewLight.fog_far);
}

void Preview_SetDefaultLightData(void)
{
    PreviewLight.col_dir.vx   = PLL_DEFAULT_X ;
    PreviewLight.col_dir.vy   = PLL_DEFAULT_Y ;
    PreviewLight.col_dir.vz   = PLL_DEFAULT_Z ;

    PreviewLight.col_lit.vx   = PLL_DEFAULT_R ;
    PreviewLight.col_lit.vy   = PLL_DEFAULT_G ;
    PreviewLight.col_lit.vz   = PLL_DEFAULT_B ;

    PreviewLight.amb_lit.vx   = AMB_DEFAULT_R ;
    PreviewLight.amb_lit.vy   = AMB_DEFAULT_G ;
    PreviewLight.amb_lit.vz   = AMB_DEFAULT_B ;

    PreviewLight.fog_color.vx = FOG_DEFAULT_R ;
    PreviewLight.fog_color.vy = FOG_DEFAULT_G ;
    PreviewLight.fog_color.vz = FOG_DEFAULT_B ;
    PreviewLight.fog_near     = FOG_DEFAULT_N;
    PreviewLight.fog_far      = FOG_DEFAULT_F;
}

void CopyLightToSavework(void)
{
	fpu_CopyVector(&(SaveData.light.col_dir),&(PreviewLight.col_dir));
    fpu_CopyVector(&(SaveData.light.col_lit),&(PreviewLight.col_lit));

    fpu_CopyVector(&(SaveData.light.amb_lit),&(PreviewLight.amb_lit));

    fpu_CopyVector(&(SaveData.light.fog_color),&(PreviewLight.fog_color));

    SaveData.light.fog_near=PreviewLight.fog_near;
    SaveData.light.fog_far=PreviewLight.fog_far;
}

void CopyLightFromSavework(void)
{
	fpu_CopyVector(&(PreviewLight.col_dir),&(SaveData.light.col_dir));
    fpu_CopyVector(&(PreviewLight.col_lit),&(SaveData.light.col_lit));

    fpu_CopyVector(&(PreviewLight.amb_lit),&(SaveData.light.amb_lit));

    fpu_CopyVector(&(PreviewLight.fog_color),&(SaveData.light.fog_color));

    PreviewLight.fog_near=SaveData.light.fog_near;
    PreviewLight.fog_far=SaveData.light.fog_far;

	ForVRSlight();
}


#if 0

void File_DebugPrint(Work *work)
{
    int x,y;

    x=LOCATE_X;
    y=LOCATE_Y;
    DEBUG_Locate( x,   y, 0 );
    DEBUG_Printf( "FILE MENU\n" );

    x=LOCATE_X+9;
    y=LOCATE_Y+9;
    DEBUG_Locate( x,   y, 0 );
    DEBUG_Printf( "DEFAULT\n"   );
    DEBUG_Printf( "FILE LOAD\n" );
    DEBUG_Printf( "FILE SAVE\n" );
    DEBUG_Printf( "LOAD stage.lt2\n" );

    /* カーソル */
    x=LOCATE_X;
    y=LOCATE_Y+9 + work->save.mode_1*9;
    DEBUG_Locate( x,   y, 0 );
    DEBUG_Printf( ">\n" );
}

#else

void File_DebugPrint(void)
{
    char *filename=*(PreviewFile.stagelight_filenames+PreviewLight.lt2_select);
    int x,y;

    x=LOCATE_X;
    y=LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("FILE MENU");

    x=LOCATE_X+0x08*4;
    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("FILE LOAD");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("FILE SAVE");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Select & Load %s.lt2",filename);

    /* カーソル */
    x=LOCATE_X;
    y=LOCATE_Y+COLUMN_HEIGHT+PreviewLight.file_cursole_pos*COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf( ">>>" );
}

#endif


static void LoadLight(void)
{
    if(PreviewKey.press & PAD_A){
		if(!PreviewLoadBinFile(SAVEFILENAME,&SaveData,sizeof(SaveData))) return;

		CopyCameraFromSavework();
		CopyLightFromSavework();
		CopyHumanFromSavework();
		CopyObjectFromSavework();
		CopyFobjFromSavework();

		Preview_LightAct();
		ResetDataObjects();
    }
}

static void SaveLight(void)
{
    if(PreviewKey.press & PAD_A){
		CopyCameraToSavework();
		CopyLightToSavework();
		CopyHumanToSavework();
		CopyObjectToSavework();
		CopyFobjToSavework();

		PreviewSaveBinFile(SAVEFILENAME,&SaveData,sizeof(SaveData));
    }
}

static void LT2Select(void)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		PreviewLight.lt2_select--;
		if(PreviewLight.lt2_select<0){
			PreviewLight.lt2_select=PreviewFile.stagelight_filenames_size-1;
		}
		break;
    case PAD_R:
		PreviewLight.lt2_select++;
		if(PreviewLight.lt2_select>=PreviewFile.stagelight_filenames_size){
			PreviewLight.lt2_select=0;
		}
		break;
    }
    if(PreviewKey.press & PAD_A){
		SetStageLight();
		ResetDataObjects();
    }
}

#if 0

void File_DebugCursole(Work *work)
{
    if ( work->pad  & PAD_U ) work->save.mode_1--;
    if ( work->pad  & PAD_D ) work->save.mode_1++;
    if ( work->save.mode_1 >= 4 ) work->save.mode_1 = 0;
    if ( work->save.mode_1 <  0 ) work->save.mode_1 = 3;

    if ( GV_PadData[1].press  & PAD_A ){
		switch( work->save.mode_1 ){
		case 0:
			SetDefaultData( work );
			work->save.mode_0=1;
			work->save.mode_1=0;
			break;
		case 1:
			LoadFile( work );
			work->save.mode_0=1;
			work->save.mode_1=1;
#if 0
			ResetDataHuman( work );
			ResetDataObjects( work );
#else
			Preview_SetDefaultHumanData();
			ResetDataObjects();
#endif
			break;
		case 2:
			SaveFile( work );
			break;
		case 3:
	    {
			LIT_DEF		*lt2_def;

			lt2_def=GV_GetCache(  GV_CacheID( GV_StrCode( "stage" ), 'l' ) );

			DG_ResetFixedLight();
			DG_SetFixedLight( lt2_def );
			GetNowData( work );

			printf("a1:%f %f %f \n",
				   work->save.col_dir.vx,
				   work->save.col_dir.vy,
				   work->save.col_dir.vz);
			printf("a2:%f %f %f \n",
				   work->save.col_lit.vx,
				   work->save.col_lit.vy,
				   work->save.col_lit.vz);
			printf("a3:%f %f %f \n",
				   work->save.amb_lit.vx,
				   work->save.amb_lit.vy,
				   work->save.amb_lit.vz);
#if 0
			fvec.vx   = lt2_def->dir.vx ;
			fvec.vy   = lt2_def->dir.vy ;
			fvec.vz   =-lt2_def->dir.vz ;

			printf("aaa:%f %f %f \n",fvec.vx,fvec.vy,fvec.vz);

			OK_DirVecXY( &fvec, &DG_ZeroVector, &sdir );

			work->save.col_dir.vx = (float)sdir.vx;
			work->save.col_dir.vy = (float)sdir.vy;
			work->save.col_dir.vz = (float)sdir.vz;

			work->save.col_lit.vx   = (float)lt2_def->color.r ;
			work->save.col_lit.vy   = (float)lt2_def->color.g ;
			work->save.col_lit.vz   = (float)lt2_def->color.b ;

			work->save.amb_lit.vx   = (float)lt2_def->ambient.r ;
			work->save.amb_lit.vy   = (float)lt2_def->ambient.g ;
			work->save.amb_lit.vz   = (float)lt2_def->ambient.b ;
#endif
	    }
	    break;
		}
    }
}

#else

void File_DebugCursole(void)
{
    switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
    case PAD_U:
		PreviewLight.file_cursole_pos--;
		if(PreviewLight.file_cursole_pos<0) PreviewLight.file_cursole_pos=3-1;
		break;
    case PAD_D:
		PreviewLight.file_cursole_pos++;
		if(PreviewLight.file_cursole_pos>=3) PreviewLight.file_cursole_pos=0;
		break;
    }

    switch(PreviewLight.file_cursole_pos){
    case 0:
		LoadLight();
		break;
    case 1:
		SaveLight();
		break;
    case 2:
		LT2Select();
		break;
    }
}

#endif

#if 1

typedef struct {
    GV_ACT_EX	actor;
} Work;

static void GetResource(void)
{
    DG_DEF *def;

	/* 光源矢印 */
	PreviewLight.yajirushi_objs=NULL;
	def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode( "yajirushi" ), 'k' ) ) ;

	if(def!=NULL){
		PreviewLight.yajirushi_objs
			= DG_MakeObjs( def, DG_FLAG_FINISHCALC|DG_FLAG_NOFOG|DG_FLAG_ONEPIECE, 0 );

		if(PreviewLight.yajirushi_objs!=NULL){
			DG_QueueObjs( PreviewLight.yajirushi_objs );
			PreviewLight.yajirushi_objs->flag |= DG_FLAG_INVISIBLE ;
			DG_SetLightMatrix(PreviewLight.yajirushi_objs,
							  PreviewLight.yajirushi_lights);
		}
	}
	if(PreviewLight.yajirushi_objs!=NULL){
		printf("InitObject : yajirushi\n");
	}
	else{
		printf("InitObject : not found yajirushi\n");
	}

	PreviewLight.parallel_cursole_pos=0;
	PreviewLight.ambient_cursole_pos=0;
	PreviewLight.fog_cursole_pos=0;
	PreviewLight.file_cursole_pos=0;
	PreviewLight.lt2_select=0;
	PreviewLight.speed_mode=0;

	Preview_SetDefaultLightData();
	SetStageLight();
}

/* 初期化部メイン */
void *NewPreviewLight(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work)) ;
    if(work!=NULL) {
		GetResource();
		GV_SetActor(&(work->actor),Preview_LightAct,Preview_LightDie);
		GV_ActorEX(&(work->actor));
    }
    return (void *)work ;
}

#endif
