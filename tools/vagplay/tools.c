#include	"vxcodec.h"

BYTE verString[]="VXCODEC ver 0.04 1997.11.13 by KCET-SD :-)\x00";

BYTE *getVER()
{
	return verString;
}

void mix(WORD *pL, WORD *pR, WORD *pS, LONG l)
{
	LONG i;
	for(i=0;i<l;i++){
		*pS++=*pL++;
		*pS++=*pR++;
	}
	return;
}

void cat(WORD *pL, WORD *pR, WORD *pS, LONG l)
{
	LONG i;
	for(i=0;i<l;i++){
		*pS++ = *pL++;
	}
	for(i=0;i<l;i++){
		*pS++ = *pR++;
	}
	
	return;
}

void cpy(WORD *pL, WORD *pS, LONG l)
{
	LONG i;
	for(i=0;i<l;i++){
		 *pS++  =*pL++;
	}
	
	return;
}

/*	sort XA to VAG	*/

static UBYTE fr[] = {
	0,2,8,10,1,3,9,11,
};

LONG XAtoVAG(UBYTE *s, UBYTE *d)
{
	LONG group,unit,sample,block = 0;
	UBYTE *pL,*pH,*g;
	
	for(group=0;group<18;group++){
		g = getSoundData;
		for(unit=0;unit<8;unit++){
			*d++ = s[fr[unit]];
			*d++ = 0x02;
			pL = tL[unit];
			pH = tH[unit];
			for(sample=0;sample<28;sample+=2){
				*d++ = pL[s[*g]] + pH[s[*(g+1)]];
				g+=2;
			}
			block++;
		}
		s += 16 + 8*28/2;
	}
	return block;
}

/*	sort VAG to XA	*/

void VAGtoXA(UBYTE *s, UBYTE *d)
{
	LONG group,unit,sample;
	UBYTE *gs, *go, *pL, *pH;
	
	for(group=0;group<18;group++){
		for(unit=0;unit<8;unit++){
			for(sample=0;sample<28+4;sample+=2){
				d[(group*8 + unit)*16 + sample] = 0;
			}
		}
	}
	
	for(group=0;group<18;group++){
		gs = getSoundData;
		go = getSoundDataOffset;
		for(unit=0;unit<8;unit++){
			d[getFltRngA[unit]] = d[getFltRngB[unit]] = s[unit*16];
			pL = tLX[unit];
			pH = tHX[unit];
			for(sample=0;sample<28;sample+=2){
				d[16 + *gs++] += pL[s[*go  ]];
				d[16 + *gs++] += pH[s[*go++]];
			}
		}
		s +=  8 + 8 + (8*28/2);
		d += 16 +     (8*28/2);
	}
	
	return ;	
}

/*	for STEREO DATA ENCODING	*/

void pushEncEnvironment(UBYTE lr)
{
	if(lr == 0){
		q0L  = q0;
		q1L  = q1;
		q2L  = q2;
		q3L  = q3;
		q4L  = q4;
		x1L  = x1;
		x2L  = x2;
		Y1L  = Y1;
		y2L  = y2;
		xp1L = xp1;
		xp2L = xp2;
		WL[0] = W[0];
		WL[1] = W[1];
		WL[2] = W[2];
		WL[3] = W[3];
		WL[4] = W[4];
	}
	else {
		q0R  = q0;
		q1R  = q1;
		q2R  = q2;
		q3R  = q3;
		q4R  = q4;
		x1R  = x1;
		x2R  = x2;
		Y1R  = Y1;
		y2R  = y2;
		xp1R = xp1;
		xp2R = xp2;
		WR[0] = W[0];
		WR[1] = W[1];
		WR[2] = W[2];
		WR[3] = W[3];
		WR[4] = W[4];
	}
	return;
}

void popEncEnvironment(UBYTE lr)
{
	if(lr == 0){
		q0  = q0L;
		q1  = q1L;
		q2  = q2L;
		q3  = q3L;
		q4  = q4L;
		x1  = x1L;
		x2  = x2L;
		Y1  = Y1L;
		y2  = y2L;
		xp1 = xp1L;
		xp2 = xp2L;
		W[0] = WL[0];
		W[1] = WL[1];
		W[2] = WL[2];
		W[3] = WL[3];
		W[4] = WL[4];
	}
	else {
		q0  = q0R;
		q1  = q1R;
		q2  = q2R;
		q3  = q3R;
		q4  = q4R;
		x1  = x1R;
		x2  = x2R;
		Y1  = Y1R;
		y2  = y2R;
		xp1 = xp1R;
		xp2 = xp2R;
		W[0] = WR[0];
		W[1] = WR[1];
		W[2] = WR[2];
		W[3] = WR[3];
		W[4] = WR[4];
	}
	return;
}
