#ifndef _NEW_IRS_H_
#define _NEW_IRS_H_

#define	BODY_FLAG	( DG_FLAG_TEXT | DG_FLAG_SHADE | DG_FLAG_ONEPIECE | DG_FLAG_IRREACTION )
#define	BODY_FLAG2	( DG_FLAG_TEXT | DG_FLAG_SHADE | DG_FLAG_ONEPIECE )
#define LAMP_FLAG   ( DG_PRIM2_SPRT | DG_PRIM2_TEX | DG_PRIM2_ALPHA )

#define MAX_BRIGHT      (64.0f)
#define INIT_COUNT       (60*1)
#define MAX_COUNT       (60*60)
#define FINISH_COUNT    (60*55)

#define N_PRIMS	    (1)
#define	N_VERTS	    (4)

#define SCR_POS SCRPAD_ADDR
#define SCR_UVS (SCRPAD_ADDR+0x2000)
#define SCR_WORK (SCRPAD_ADDR+0x2200)

#define TOP_INTERVAL  (5)
#define INTERVAL      (6)

#define N_IRS_SENSOR      (2)
#define N_IRS_SENSOR_RAY (10)
#define N_RAY_VERTS       (2)
#define PRIM_TYPE      ( DG_PRIM2_LINE | DG_PRIM2_SHADE | DG_PRIM2_TEX | DG_PRIM2_ANTIALIASING )
#define SENSE_LENGTH   (5000.0f)
#define MODEL_MAX         (3) // 通常 , 壊れ , センサー
#define PROC_MAX          (2)
#define SAIFA_DIE_FLG      (3)
#define ALL_DIE_FLG        (4)
#define SINGLE_DIE_FLG     (8)

enum 
{
ModelBox = 0 ,
ModelBreakBox , 
ModelSenser 
};

enum
{
    IRS_NORMAL_FLAG   = 0x0000,
    IRS_VISIBLE_FLAG  = 0x0001
};

enum
{
    ATTACH_ON   = 0x0000,
    ATTACH_OFF  = 0x0001,
    ATTACH_FREE = 0x0002
};

#endif
