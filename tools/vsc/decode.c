#include	"vxcodec.h"

/*	for VAG DECODING	*/

#define _FULLBIT_

/*#define _CONTINUE_*/


/*	initialization for decoding	*/

void resetWork()
{
	work[0].dH=work[0].dL=0;
	work[1].dH=work[1].dL=0;
	workf[0].dH=workf[0].dL=0.0;
	workf[1].dH=workf[1].dL=0.0;
	return;
}

void initWork()
{
	resetWork();
	setL();
	return;
}


/*	VAG DECODER integer version	*/


LONG decodeVAG(UBYTE *s, WORD *d, LONG p_len)
{
	register LONG d0,d1,d7,d2,d2L,d5,d6,*a3;
	register WORD d3,*a2;
	register UBYTE d4,*a0;
	
	if((s[0] == 'V')&&(s[1] == 'A')&&(s[2] == 'G')&&(s[3] == 'p')){
		p_len -= 4;
		s += 16*4;
	}
	
	d4 = 0;
	a0 = s;
	d7 = p_len;
	a2 = d;
	d2 = pWork->dL;
	d5 = d2;
	d6 = pWork->dH;
	
	while(d7-- > 0){
		d1 = (LONG)nHigh[*a0];
		d0 = (LONG)nLow[*a0];
		
		if(d0 > 0x0C){
			break;
		}
		
		#ifdef _FULLBIT_
		a3 = &multbl[d0<<4];
		#else
		a3 = &multbl[(d0+1)<<4];
		#endif
		
		a0++;
		
		if((d4=*a0++) == 0x07){
			a0 += 14;
#ifdef _CONTINUE_
			continue;
#else
			break;
#endif
		}
		
		d3 = 28/2-1;
		
		if(d1 < 0x04+1){
			register LONG d1L = d1;
			do {
				d2L  = (d2*fltA[d1L]) - (d6*fltB[d1L]);
				d2 = d2L>>6;
				d1 =  nLow[*a0  ];
				d2 += a3[d1];
				*a2++ = (WORD)d2;
				d6 = d2;
				
				d2L  = (d2*fltA[d1L]) - (d5*fltB[d1L]);
				d2 = d2L>>6;
				d1 =  nHigh[*a0++];
				d2 += a3[d1];
				*a2++ = (WORD)d2;
				d5 = d2;
			}while (--d3 != -1);
		}
	}
	
	if(d4&0x01){
		pWork->dL = 0;
		pWork->dH = 0;
	}
	else {
		pWork->dL = d5;
		pWork->dH = d6;
	}
	
	return (LONG)a2-(LONG)d;
}


/*	VAG DECODER floating point version	*/


LONG decodeVAGf(UBYTE *s, WORD *d, LONG p_len)
{
	register LONG d0,d1,d7;
	register WORD d3,*a2;
	double        d2,d5,d6,*a3;
	register UBYTE d4,*a0;
	
	if((s[0] == 'V')&&(s[1] == 'A')&&(s[2] == 'G')&&(s[3] == 'p')){
		p_len -= 4;
		s += 16*4;
	}
	
	d4 = 0;
	a0 = s;
	d7 = p_len;
	a2 = d;
	d2 = pWorkf->dL;
	d5 = d2;
	d6 = pWorkf->dH;
	
	while(d7-- > 0){
		d1 = (LONG)nHigh[*a0];
		d0 = (LONG)nLow[*a0];
		
		if(d0 > 0x0C){
			break;
		}
		
		#ifdef _FULLBIT_
		a3 = &multblF[d0<<4];
		#else
		a3 = &multblF[(d0+1)<<4];
		#endif
		
		a0++;
		
		if((d4 = *a0++) == 0x07){
			a0 += 14;
#ifdef _CONTINUE_
			continue;
#else
			break;
#endif
		}
		
		d3 = 28/2-1;
		
		if(d1 < 0x04+1){
			register LONG d1L = d1;
			do {
				d1 = nLow[*a0  ];
				d2 = a3[d1] - (d2*dfltA[d1L]) - (d6*dfltB[d1L]);
				*a2++ = (short)(((d2>32767.0)?32767.0:d2)<-32768.0)?-32768.0:d2;
				d6 = d2;
				
				d1 = nHigh[*a0++];
				d2 = a3[d1] - (d2*dfltA[d1L]) - (d5*dfltB[d1L]); 
				*a2++ = (short)(((d2>32767.0)?32767.0:d2)<-32768.0)?-32768.0:d2;
				d5 = d2;
			}while (--d3 != -1);
		}
	}
	
	if(d4&0x01){
		pWorkf->dL = 0.0;
		pWorkf->dH = 0.0;
	}
	else {
		pWorkf->dL = d5;
		pWorkf->dH = d6;
	}
	
	return (LONG)a2-(LONG)d;
}


/*	for STEREO DATA DECODING	*/

/*	set Left channel decoding enviroment	*/

void setL()
{
	pWork  = &work [0];
	pWorkf = &workf[0];
}


/*	set Right channel decoding enviroment	*/

void setR()
{
	pWork  = &work [1];
	pWorkf = &workf[1];
}


/*
//
//	VAG/XA adpcm enCOder/DECorder
//		ver 0.01 27/Sep/1997
//			NO RIGHTS RESERVED :-P
//
*/



