//----------------------------------------------------------------------------
// CStreamDriver_Demo.cpp
//----------------------------------------------------------------------------

#include "CStreamDriver_Demo.h"

//----------------------------------------------------------------------------

#include "string.h"
#include <algorithm>

//----------------------------------------------------------------------------

typedef unsigned __int64 u_int64;
typedef unsigned int u_int;
typedef unsigned __int16 u_short;
typedef unsigned char u_char;

#define ALIGN8_DECL(a) __declspec(align(8)) a

#define ALIGN16_DECL(a) __declspec(align(16)) a
#define ALIGN16_PRE __declspec( align(16) )
#define ALIGN16_POST

typedef u_int64 GV_HANDLE;
typedef GV_HANDLE (*DM_EFFECT_EXEC)( int id, void *param_ptr );

#define DEMO_PACKET_ID(p) ( ( (p)._id ) )
#define DEMO_PACKET_PACKET_TYPE(p) ( (p)._packet_type )
#define DEMO_PACKET_SIZE(p) ( ( (p)._size ) )
#define DEMO_PACKET_BASE_TYPE(p) ( (p)._base_type )

typedef ALIGN16_DECL(struct) FVECTOR_s { float vx, vy, vz, vw ; } FVECTOR;
typedef ALIGN8_DECL(struct) SVECTOR_s { short vx, vy, vz, pad ; } SVECTOR;

#define DM_MAX_FRAME (4) 
#define DM_MAX_OBJECT (256) 
#define DM_MAX_MOTION (64*4) 
#define DM_MAX_EFFECT (512) 
#define DM_MAX_PROC (64) 

typedef struct _GV_SIGNAL_TABLE {
   int signal;
   int ( *func )( void *work, u_int signal, int value, int param );
   int param;
} GV_SIGNAL_TABLE;

typedef ALIGN16_DECL(struct) _GV_ACT {
   struct _GV_ACT *next;
   void ( *act )( struct _GV_ACT * );
   unsigned int name;
   unsigned int _class;

   struct _GV_ACT *prev;
   int ( *die )( struct _GV_ACT * );
   GV_HANDLE handle;		// 自分自身のハンドル

   struct _GV_ACT *child;	/* 子ども */
   struct _GV_ACT *c_prev;	/* 子どものなかでのリンクリスト */
   struct _GV_ACT *c_next;	/* 子どものなかでのリンクリスト */
   GV_SIGNAL_TABLE *sig_table;

#ifdef DEBUG
   unsigned int runtime;		/* デバッグ用 */
   unsigned int count;			/* デバッグ用 */
   char *filename;				/* デバッグ用 */
   int res1;					/* デバッグ用(予約) */
#endif
} GV_ACT;

typedef struct _dm_effect {
   int			id ;
   int			life ;
   GV_HANDLE	handle ;
} DM_EFFECT;

typedef struct _dm_frame {
   short		id ;
   short		type ;
   short		no ;
   short		chanl ;
   float		screen ;
   float		z_rot ;
   FVECTOR		pos ;
   FVECTOR		trg ;
} DM_FRAME ;

typedef struct _dm_object {
   int			id ;
   int         flag;
   GV_HANDLE	handle ;
} DM_OBJECT;

typedef struct _dm_motion_data {
   FVECTOR		move ;
   SVECTOR		rot ;
   FVECTOR		rot_quat ;
   FVECTOR		*j_rots ;
   FVECTOR		*j_trans ;
} DM_MOTION_DATA ;

typedef struct _dm_motion {
   int			id ;
   int			object_id ;
   int			motion_type ;
   int			n_skeleton ;
   int			*skel_list ;
   int			time ;
   int			skip_count ;
   void		*mem ;
   DM_MOTION_DATA	motion ;
   DM_MOTION_DATA	trg_motion ;
   DM_MOTION_DATA	src_motion ;
} DM_MOTION ;

typedef struct _dm_work {
   GV_HANDLE	handle ;
   GV_ACT		*actor_ptr ;
   int			sequence_end ;
   int			frame_end ;
   int			n_frame ;
   int			n_object ;
   int			n_motion ;
   int			n_effect ;
   DM_FRAME	frm_list[DM_MAX_FRAME] ;
   DM_OBJECT	obj_list[DM_MAX_OBJECT] ;
   DM_MOTION	mtn_list[DM_MAX_MOTION] ;
   DM_EFFECT	eft_list[DM_MAX_EFFECT] ;
   int		proc_id[DM_MAX_PROC][2] ;
   int		low_poly_demo ;
   GV_HANDLE	camera_handle[2] ;
} DM_WORK ;

typedef struct _demo_packet {
   u_char	_base_type ;
   u_char	_packet_type ;
   u_short	_size ;
   int		_id ;
} DEMO_PACKET ;

typedef struct _demo_effect {
   DEMO_PACKET		header ;
   int				effect_id ;
   int				exec_type ;
   int				life ;
   int				parent ;
   int				data[0] ;
} DEMO_EFFECT ;
enum {
   DEMO_EFFECT_TYPE_NO_MANAGE	= 0x00,
   DEMO_EFFECT_TYPE_MANAGE		= 0x01,
};

typedef struct _demo_effect_signal {
   DEMO_PACKET		header ;
   int				signal ;
   int				params[0] ;
} DEMO_EFFECT_SIGNAL ;


enum _demo_packet_effect {
   DEMO_PACKET_EFFECT				= 0x00,
   DEMO_PACKET_EFFECT_SIGNAL		= 0x01,
};

enum _demo_base_type {
   DEMO_BASETYPE_SYSTEM	= 0x00,
   DEMO_BASETYPE_FRAME		= 0x01,
   DEMO_BASETYPE_OBJECT	= 0x02,
   DEMO_BASETYPE_MOTION	= 0x03,
   DEMO_BASETYPE_EFFECT	= 0x04,
   DEMO_BASETYPE_COMMAND	= 0x05,
   DEMO_BASETYPE_MASK		= 0x0f,

   DEMO_BASETYPE_ENDIAN_SWAPPED = 0x40,
   DEMO_BASETYPE_NOSKIP	= 0x80,
};

enum _demo_packet_sytem {
   DEMO_PACKET_TERMINATE_SEQUENCE	= 0x00,
   DEMO_PACKET_TERMINATE_FRAME		= 0x01,
   DEMO_PACKET_MAXDEBUG			= 0xfe,
   DEMO_PACKET_END					= 0xff,
};

enum {

   SIG_VISIBLE,
   SIG_INVISIBLE,
   SIG_SET_PARAM,
   SIG_SET_CROSS_SPEED,

};

//----------------------------------------------------------------------------

static ALIGN16_PRE int effect_param_mem[64] ALIGN16_POST;
static DM_WORK work;

//----------------------------------------------------------------------------

GV_HANDLE NewDispTex2DEffect_Demo_0800Launch( int id, void *argv )
{
#if 1
   return 0;
#else
   GV_HANDLE  h_effect ;
   Work  work ;
   Data *data = (Data *)argv ;

   extern GV_HANDLE NewDispTex2DEffect_Demo( int  ) ;


   work.prio = data->d0 ;

   h_effect =  NewDispTex2DEffect_Demo( work.prio
      ) ;
   PRINT_FILE_NAME();
   return h_effect;
#endif
}

void *GM_GetCharaID( int nID )
{
   switch( nID )
   {
   case 0x01000800:
      return &NewDispTex2DEffect_Demo_0800Launch;
   }

   return NULL;
}

void CStreamDriver_Demo::DM_Packet_System( DM_WORK *work, DEMO_PACKET *packet )
{
   switch ( DEMO_PACKET_PACKET_TYPE( *packet ) ){
     case DEMO_PACKET_TERMINATE_SEQUENCE:
        work->sequence_end = 1 ;
        break ;
     case DEMO_PACKET_TERMINATE_FRAME:
        work->frame_end = 1 ;
        break ;
     case DEMO_PACKET_MAXDEBUG:
        break;
     case DEMO_PACKET_END:
        break ;
   }
}

void DM_Packet_Frame( DM_WORK *work, DEMO_PACKET *demo_packet )
{
}

void DM_Packet_Object( DM_WORK *work, DEMO_PACKET *demo_packet )
{
}

void DM_Packet_Motion( DM_WORK *work, DEMO_PACKET *demo_packet )
{
}

void CStreamDriver_Demo::DM_Packet_Effect( DM_WORK *work, DEMO_PACKET *demo_packet )
{
   DM_EFFECT	*effect ;
   int			id, size ;
   DM_EFFECT_EXEC exec ;

   id = DEMO_PACKET_ID( *demo_packet ) ;

   switch ( DEMO_PACKET_PACKET_TYPE( *demo_packet ) ){
     case DEMO_PACKET_EFFECT:
        {
           DEMO_EFFECT	*packet = (DEMO_EFFECT*)demo_packet ;
           size = DEMO_PACKET_SIZE( packet->header ) - sizeof(DEMO_EFFECT) ;

           exec = (DM_EFFECT_EXEC)GM_GetCharaID( packet->effect_id ) ;

           memcpy( effect_param_mem, packet->data, size );
#if 1
           if( exec )
           {
              mEffectIds.push_back(id);
           }
#else
           if ( packet->exec_type == DEMO_EFFECT_TYPE_NO_MANAGE ){
              (* exec)( id, (int *)effect_param_mem ) ;
           } else {
              effect = MakeEffect( work );
              effect->id = id ;
              effect->life = packet->life ;
              effect->handle = (* exec)( id, (int *)effect_param_mem ) ;
              if ( effect->handle != GV_HANDLE_ERROR ){
                 GV_actorSetChild( work->actor_ptr, effect->handle );
                 printf( "%dth effect create[0x%08x] header%d type%d hndl <%ld>\n" ,
                    work->n_effect, packet->effect_id, DEMO_PACKET_ID( packet->header ), packet->exec_type, effect->handle ) ;
              } else {
                 FreeEffect( work, id );
              }
           }
#endif
           memset( effect_param_mem, 0, sizeof(int)*64 );
        }
        break ;
     case DEMO_PACKET_EFFECT_SIGNAL:
        {
           DEMO_EFFECT_SIGNAL	*packet = (DEMO_EFFECT_SIGNAL*)demo_packet ;
#if 1
           //Hardcoded to handle NewDispTex2DEffect_Demo_0800Launch types
           if( std::find( mEffectIds.begin(), mEffectIds.end(), id ) != mEffectIds.end() )
           {
              //Interpret and dump the info from this signal.
              switch( packet->signal )
              {
              case SIG_SET_PARAM:               
                 {
                    int	 *pData = (int*)packet->params;
                    unsigned int u32TexStrCodeF = pData[ 0 ];
                    unsigned int u32TexStrCodeB = pData[ 1 ];
                    int s32BasePosType = pData[ 2 ];
                    int s32PosX 		  = pData[ 3 ];
                    int s32PosY 		  = pData[ 4 ];
                    float fScale         = (float)pData[ 5 ] * 0.01f;
                    float fTargetAlpha   = (float)pData[ 6 ];
                    float fShiftTime     = (float)pData[ 7 ];

                    printf("//EffectId %d stream %s/%s:\n", id, gOutputFolder, gCurrStreamName);
                    printf("// u32TexStrCodeF 0x%08x\n", u32TexStrCodeF );
                    printf("// u32TexStrCodeB 0x%08x\n", u32TexStrCodeB );
                    printf("// s32BasePosType %d\n", s32BasePosType );
                    printf("// s32PosX %d\n", s32PosX );
                    printf("// s32PosY %d\n", s32PosY );
                    printf("// fScale %f\n", fScale );
                    printf("// fTargetAlpha %f\n", fTargetAlpha );
                    printf("// fShiftTime %f\n\n", fShiftTime);
                    //Print out handy binary data source code that can be copied into disp_tex_2d.c
                    printf("static const int skNewDispTex2DEffect_Demo_Param_%s_%s_%d[] =\n", gCurrStreamName, gRegionString, mCurrStreamTag._time );
                    printf("{\n");
                    printf("   %8d, %8d, %8d, %8d, %8d, %8d, %8d, %8d\n", pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6], pData[7] );
                    printf("};\n\n");
                 }
                 break;
              }
           }
#else
           effect = SearchEffect( work, id );

           if( effect == NULL ) break;

           size = DEMO_PACKET_SIZE( packet->header ) - sizeof(DEMO_EFFECT_SIGNAL) ;

           memcpy( effect_param_mem, packet->params, size );
           GV_ActorSendSignal( effect->handle, packet->signal, (int)effect_param_mem );
#endif
        }
        break ;
   }
}

void CStreamDriver_Demo::DM_ExecDemoStream( DM_WORK *work, void *packet_stream_ptr, int exec_flag )
{
   DEMO_PACKET		*packet ;
   int				base_type, packet_type ;

   work->frame_end = 0 ;
   while ( work->frame_end == 0 ){
      packet = (DEMO_PACKET*)packet_stream_ptr ;
      packet_stream_ptr = (void*)((int)packet_stream_ptr + DEMO_PACKET_SIZE(*packet)) ;

      if ( exec_flag == 0 && !( DEMO_PACKET_BASE_TYPE( *packet ) & DEMO_BASETYPE_NOSKIP ) ) continue ;

      base_type = DEMO_PACKET_BASE_TYPE( *packet ) & DEMO_BASETYPE_MASK ;
      packet_type = DEMO_PACKET_PACKET_TYPE(*packet) ;

      if ( packet_type < DEMO_PACKET_END ){
         if( base_type > DEMO_BASETYPE_COMMAND )
         {
            printf( "Error: Packet exec function out of range" );
            throw false;
         }

         switch( base_type )
         {
         case DEMO_BASETYPE_SYSTEM:
            DM_Packet_System( work, packet );
            break;
         case DEMO_BASETYPE_EFFECT:
            DM_Packet_Effect( work, packet );
            break;
         }
      }
   }
}

//----------------------------------------------------------------------------

CStreamDriver_Demo::CStreamDriver_Demo()
: mbInitialized( false )
{
}

//----------------------------------------------------------------------------

CStreamDriver_Demo::~CStreamDriver_Demo()
{
}

//----------------------------------------------------------------------------

void CStreamDriver_Demo::ProcessPacket(const STREAM_TAG& packet, const void * const pBody)
{
   mCurrStreamTag = packet;
   if( !mbInitialized )
   {
      mbInitialized = true;
      BeginStream();
   }
   DM_ExecDemoStream( &work, (void*)pBody, 0 );
}

//----------------------------------------------------------------------------

void CStreamDriver_Demo::BeginStream()
{
   memset( &work, 0, sizeof( work ) );
   mEffectIds.clear();
}

//----------------------------------------------------------------------------

void CStreamDriver_Demo::EndStream()
{
   mbInitialized = false;
}