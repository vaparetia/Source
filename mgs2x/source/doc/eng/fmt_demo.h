/*
	fmt_demo.h
	DEMO STREAMMING DATA FORMAT

	2000/07/24
	$Id: fmt_demo.h,v 1.1.1.3 2002/11/19 11:41:45 Yoshizawa1 Exp $

*/

/*
   Demo Streamming 

   Demo data (DE2) consis of the PACKET-DATAs for each frame. The size
   of each packets(DEMO_PACKET) is unfixed, which makes easy for us to
   handle the datas. There is several types of packets, needed to play
   demos.  TERMINATE_FRAME must come the end of the each frames.
   Moreover, TERMINATE_SEQUENCE must come the end of the demo.

   ---beginning of 0th frame---
	+-----------+---------------------+
	|DEMO_PACKET|                     |
	+-----------+-------+-------------+
	|DEMO_PACKET|       |
	+-----------+-------+-----+
	|DEMO_PACKET|             |
	+-----------+-------------+-------+
	|DEMO_PACKET|                     |
	+-----------+-------------+-------+
	|DEMO_PACKET|             |
	+-----------+-------------+
	|DEMO_PACKET|
	+-----------+
	.
	.
	.
	.
	+-----------+
	|DEMO_PACKET|(=DEMO_PACKET_TERMINATE_FRAME)
	+-----------+
   ---end of 0th frame---
   ---beginning of 1st frame---
	+-----------+-----------------+
	|DEMO_PACKET|                 |
	+-----------+-----------------+
	.
	.


*/

#ifndef __FMT_DEMO_H__
#define __FMT_DEMO_H__


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __GNUC__
#define NULL_ARRAY 0
#else
#define NULL_ARRAY 
#endif


/* ---------------------------------------------------------------- */
/* Packet Header

   All of demo packets has this structure.
   Needed for DEMO-DAEMON to proceed the packet.

   The main purpose of this header is for
   handling the unfixed-length datas.
 */
typedef struct _demo_packet {
	int		packet_type ;
	int		size ;				/* packet size(Must be aligned to 16bytes)
	int		id ;				/* unique ID(USE: identfing each object)
	int		reserved ;			
} DEMO_PACKET ;

enum {
	/* Packet Pattern Definition */
	DEMO_PACKET_TERMINATE_SEQUENCE,		/* End of DEMO (No Extend-Infos) */
	DEMO_PACKET_TERMINATE_FRAME,		/* End of Frame (No Extend-Infos) */
	DEMO_PACKET_COMMAND,				/* Command packet       */
	DEMO_PACKET_FRAME,					/* Frame(Camera) packet     */
	DEMO_PACKET_OBJECT,					/* Object packet */
	DEMO_PACKET_MOTION,					/* Motion packet   */
	DEMO_PACKET_EFFECT,					/* Effect packet   */
	DEMO_PACKET_MOTION_PACK,			/* Motion-Pack packet */
	DEMO_PACKET_END,					/* End of Packet */

	/* MST file Definition */
	DEMO_PACKET_MST = 0x1000,			/* Motion Stream */

	/*  */
	DEMO_PACKET_NOSKIP = 0x80000000,	/* 'do NOT skip' flag */
} ;

/* ---------------------------------------------------------------- */
/* Command packet

   

 */
typedef struct _demo_command {
	DEMO_PACKET		header ;
	int				command ;		/* command type */
	int				id ;			/* OBJECT id */
	int				param1 ;		/* parameters were to be command type */
	int				param2 ;		/* parameters were to be command type */
} DEMO_COMMAND ;
/* Command Type */
enum {
	DEMO_COMMAND_OBJECT_VISIBLE,	/* change object visibility */
	DEMO_COMMAND_OBJECT_DESTROY,	/* destroy an object */
	DEMO_COMMAND_EFFECT_DESTROY,	/* destroy effect */
	DEMO_COMMAND_CAMERA_CREATE,		/* create a camera(needed only once) */
} ;

/* Frame Packet */
typedef struct _demo_frame {
	DEMO_PACKET		header ;
	int				no ;			/* camera number */
	float			screen ;		/* camera screen length ratio (DisplayWidth/Screen  default:2.0f) */
	float			z_rotate ;		/* Z-rotation */
	int				chanl ;			/* channel number(DG_Chanl) */
	FVECTOR			camera_pos ;	/* position of cam */
	FVECTOR			camera_trg ;	/* target of cam */
} DEMO_FRAME ;

/*Object Packet */
typedef struct _demo_object {
	DEMO_PACKET		header ;
	int				cache_id ;		/* EVM cache id(GV_CACHE) = Extension(8Bit)|ModelID(24bits) */
	int				flag ;			/* object init-flag */
	int				name ;			/* object name(for sending message) */
	int				kms_cache_id ;	/* KMS cache id(GV_CACHE) */
} DEMO_OBJECT ;

/* motion packets */
typedef struct _demo_motion {
	DEMO_PACKET		header ;
	int				object_id ;		/* Object id */
	int				motion_type ;	/* motion type */
	int				start_joint ;	/* number of starting joints */
	int				n_joints ;		/* number of joints for applying object */
	FVECTOR			pos ;			/* center position of object */
	SVECTOR			rot ;			/* center rotation of object (CONTROL) */
	int				pad[2] ;		/* dummy */
	FVECTOR			motion[NULL_ARRAY] ;	/* joint motion data */
} DEMO_MOTION ;
enum {
	DEMO_MOTION_ROT			= 1,	/* Only joint rots(Quartanion) */
	DEMO_MOTION_TRANS		= 2,	/* Only joint moves( offset from center) */
	DEMO_MOTION_ROTTRANS	= 3,	/* joint rots & moves */
									/* (rot0+trans0),(rot1+trans1),(rot2+trans2),... */
	DEMO_MOTION_COMPRESS	= 4,	/* 16bit Quatanion compress */
	DEMO_MOTION_ABSROTTRANS	= 8,	/* absolute Quatanion (QUAT from center) */
};

/* motion compress definition */
enum {
	MOTIONDIET_ROT_X		= 0x0100,
	MOTIONDIET_ROT_Y		= 0x0200,
	MOTIONDIET_ROT_Z		= 0x0400,
	MOTIONDIET_ROT_W		= 0x0800,
	MOTIONDIET_TRANS_X		= 0x1000,
	MOTIONDIET_TRANS_Y		= 0x2000,
	MOTIONDIET_TRANS_Z		= 0x4000,
} ;

/* Effect Packet */
typedef struct _demo_effect {
	DEMO_PACKET		header ;
	int				effect_id ;		    /* Effect ID  */
	int				exec_type ;	        /* executing type(0:non-actor 1:create actor) */
	int				pad[2] ;		    /* dummy */
	int				data[NULL_ARRAY] ;	/* parameters */
} DEMO_EFFECT ;

/* motion pack */
typedef struct _demo_motion_info {
	int				object_id ;		/* object ID whose motion will be assigned */
	unsigned short	flag ;			/* data flag */
	unsigned char	start_joint ;	/* number of starting joints */
	unsigned char	n_joints ;		/* number of joints for applying object */
	unsigned short	offset ;		/* offset of motion-data  */
	unsigned short	frame ;			/* time(fps) of motion interpolartion */
} DEMO_MOTION_INFO ;
typedef struct _demo_motion2 {
	DEMO_PACKET			header ;
	int					n_lists ;		/* number of motions in the packet */
	int					data_offset ;	/* the offset of data-address from the packet header (not using now) */
	int					pad[2] ;		/*  */
	DEMO_MOTION_INFO	lists[NULL_ARRAY] ;
} DEMO_MOTION_PACK ;

/* data flag  */
enum {
	DEMO_MOTIONPACK_MOV			= 0x0001,	/* center move */
	DEMO_MOTIONPACK_ROT			= 0x0002,	/* center rot  */
	DEMO_MOTIONPACK_ROTS		= 0x0004,	/* joint local rot (local rotation from parent) */
	DEMO_MOTIONPACK_ABSROTS		= 0x0008,	/* joint absolute rot (rotation from center) */
	DEMO_MOTIONPACK_TRANS		= 0x0010,	/* joint move (rotation from center)各関節の移動量 */
};

/* ---------------------------------------------------------------- */
/*
   Motion Streamming Data (MST)

   This is designed to brush up the motion-data-format, compressing
   the data to minimize the size. There is one packet for each frame.
   And the content of the packet includes many motion-datas as MST_INFO,
   handling the real motion data and parameters.

	+-----------+-----------------------+
	|MST_PACKET |Content of Packet data |
	+-----------+-------+---------*-----+

	Content of Packet data
	+-----------+
	|MST_INFO   |(0)
	+-----------+
	|MST_INFO   |(1)
	+-----------+
	.
	.
	.
	+-----------+
	|MST_INFO   |(MST_PACKET->n_lists-1)
	+-----------+------------+
	|data area               |
	|                        |
	|                        |
	|                        |
	+------------------------+


	if <frame> is set more than 1, linear interporation will be done
	in MST-Driver(game/mtstream.c). So you can have less amount of datas.

*/
typedef struct _mst_info {
	int				object_id ;		/* Object-ID */
	unsigned short	flag ;			/* data flag */
	unsigned char	start_joint ;	/* number of starting joints */
	unsigned char	n_joints ;		/* number of joints for applying object */
	unsigned short	offset ;		/* offset of motion-data  */
	unsigned short	frame ;			/* time(fps) of motion interpolartion */
} MST_INFO ;

/* モーションパケットヘッダ */
typedef struct _mst_packet {
	int				packet_type ;
	int				size ;			/* packet size(Must be aligned to 16bytes)
	int				id ;			/* Object ID (not used) */
	int				n_lists ;		/* number of motions */
	MST_INFO		info_list[0] ;	/*  */
} MST_PACKET ;

/* data flag definition */
enum {
	MST_FLAG_POS		= 0x0001,	/* non-compressed center move(not used) */
	MST_FLAG_ROT		= 0x0002,	/* non-compressed center rot (not used) */
	MST_FLAG_ROTS		= 0x0004,	/* non-compressed joint move (not used) */
	MST_FLAG_TRANS		= 0x0008,	/* non-compressed joint rot  (not used) */
	MST_FLAG_COMPROTS	= 000040,	/* compressed joint rot  (default)*/
	MST_FLAG_COMPTRANS	= 0x0080,	/* compressed joint move (default)*/
};


/* ---------------------------------------------------------------- */


#ifdef __cplusplus
};
#endif

#endif	/* __FMT_DEMO_H__ */
