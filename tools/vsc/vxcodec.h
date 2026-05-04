#ifndef _VXCODEC_H_
#define _VXCODEC_H_

#ifndef	INT

typedef	int		INT;
typedef	unsigned int	UINT;
typedef	short		WORD;
typedef	unsigned short	UWORD;
typedef	char		BYTE;
typedef	unsigned char	UBYTE;
typedef	void		VOID;
typedef	int		DEFAULT;
typedef	int		BOOLEAN;
typedef	long		LONG;
typedef	unsigned long	ULONG;

#endif

#include <math.h>
#include <stdlib.h>

/* conversion_mode */
#define ENC_VAG_MODE_NORMAL	1
#define ENC_VAG_MODE_HIGH	2
#define ENC_VAG_MODE_LOW	3
#define ENC_VAG_MODE_4BIT	4

/* block_attribute */
#define ENC_VAG_1_SHOT		0
#define ENC_VAG_1_SHOT_END	1
#define ENC_VAG_LOOP_START	2
#define ENC_VAG_LOOP_BODY	3
#define ENC_VAG_LOOP_END	4
	/*extend value*/
#define ENC_VAG_1_SHOT_1_BLOCK	5
#define ENC_VAG_1_SHOT_START	6
#define ENC_VAG_LOOP_1_BLOCK	7

#ifndef ExD
typedef long double ExD;
#endif

#ifndef VagHdr
#define VAGp 0x56414770L
#define pGAV 0x70474156L
typedef struct {
	LONG vagp;
	LONG dum0;
	LONG dum1;
	LONG size;
	LONG freq;
	LONG dum2;
	LONG dum3;
	LONG dum4;
	BYTE name[16];
} VagHdr;
#endif

#ifndef PI
#define	PI		3.1415926535898
#endif

#ifndef DBL_MAX
#define DBL_MAX 1.7976931348623158e+308
#endif

#ifndef XA_FILTER_TYPE
#define XA_FILTER_TYPE 4
#endif

#ifndef VAG_FILTER_TYPE
#define VAG_FILTER_TYPE 5
#endif

enum eLeftRight {
	eLeft,
	eRight,
};

enum eXaTypes {
	ENC_XA_MONO_LEVEL_B,
	ENC_XA_STEREO_LEVEL_B,
	ENC_XA_MONO_LEVEL_C,
	ENC_XA_STEREO_LEVEL_C,
};

#if 0

typedef struct {
	UBYTE fnum;
	UBYTE chno;
	UBYTE smod;
	UBYTE cinf;
} XaHdr;

typedef struct {
	UBYTE fltrng[16];
	UBYTE snddat[8*28/2];
} sndGrp;

typedef struct {
	XaHdr header;
	sndGrp group[18];
	UBYTE padding[24];
} sndSecotr;

#endif

typedef struct {
	LONG dH;
	LONG dL;
} Work;

typedef struct {
	double dH;
	double dL;
} WorkF;

#define min(a, b)  (((a) > (b)) ? (b) : (a))
#define max(a, b)  (((a) < (b)) ? (b) : (a))

/* function prototypes */

BYTE *getVER (void);
void mix (WORD *pL, WORD *pR, WORD *pS, LONG l);
void cat (WORD *pL, WORD *pR, WORD *pS, LONG l);
void cpy (WORD *pL, WORD *pS, LONG l);
LONG XAtoVAG (UBYTE *s, UBYTE *d);
void VAGtoXA (UBYTE *s, UBYTE *d);
void pushEncEnvironment (UBYTE lr);
void popEncEnvironment (UBYTE lr);
void prefilter (WORD *x, ExD *y, ExD z);
void quantize (ExD *b, WORD *d, WORD flt, WORD rng);

void initWork (void);
void resetWork (void);

LONG decodeVAG (UBYTE *s, WORD *d, LONG p_len);
LONG decodeVAGf (UBYTE *s, WORD *d, LONG p_len);

void setL (void);
void setR (void);

LONG decodeXA (UBYTE *s, WORD *d, LONG s_len);
LONG decodeXAf (UBYTE *s, WORD *d, LONG s_len);


void EncVagInit (WORD mode);
void EncVag (WORD *x, WORD *y, WORD blk_atr);
void EncVagFin (WORD *y);

void EncXaInit (void);
void EncXa (WORD *x, UBYTE *y, UBYTE fn, UBYTE ch, UBYTE sm, UBYTE ci);

#endif

#ifndef _ENCTBL_

extern ExD q0,q1,q2,q3,q4,Y1,y2,xp1,xp2,Y1_707,y2_708;
extern ExD q0L,q1L,q2L,q3L,q4L,Y1L,y2L,xp1L,xp2L;
extern ExD q0R,q1R,q2R,q3R,q4R,Y1R,y2R,xp1R,xp2R;

extern ExD P[5][4];
extern ExD R[5][4];

extern ExD    W[5];

extern ExD    M[5];

extern ExD Worg[5];


extern ExD    WL[5];

extern ExD    WR[5];


extern WORD x1,x2,conv_mode_local;
extern WORD x1L,x2L;
extern WORD x1R,x2R;

/*	for XA CODEC	*/

extern UBYTE getSoundData[];

extern UBYTE getSoundDataOffset[];

extern UBYTE getFltRngA[];

extern UBYTE getFltRngB[];

extern UBYTE nL[];

extern UBYTE nH[];

extern UBYTE *tL[];

extern UBYTE *tH[];

extern UBYTE *tLX[];

extern UBYTE *tHX[];

extern WORD b_attr[];

#endif

#ifndef _DECTBL_
/*	prediction filter table (16bit int shifted 6bits left)	*/

extern WORD fltA[];
extern WORD fltB[];

/*	prediction filter table (double)	*/

extern double dfltA[];
extern double dfltB[];

/*	multiple table (32bit int)	*/

extern LONG multbl[];

/*	multiple table (double)	*/

extern double multblF[];

/*	get 4bit of lower byte	*/

extern UBYTE nLow[];

/*	get 4bit of higher byte	*/

extern UBYTE nHigh[];

/*	feedback data work of prediction (for STEREO)	*/

extern Work work[2];
extern WorkF workf[2];

extern Work  *pWork;
extern WorkF *pWorkf;


#endif
