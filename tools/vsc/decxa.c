#include "vxcodec.h"

static UBYTE vagbuf[8*16*18];

LONG decodeXA(UBYTE *s, WORD *d, LONG s_len)
{
	LONG blk,ret=0,g,u;
	UBYTE *vb = vagbuf;
	WORD  *db = d;
	do {
		blk = XAtoVAG(s+8, vb);
		switch(*(s+3)){
			case 0:
			case 4:
				setL();
				ret = decodeVAG(vb, db, blk);
				break;
			case 1:
			case 5:
				for(g=0;g<18;g++){
					for(u=0;u<8/2;u++){
						setL();
						ret += decodeVAG(vb, db           , 1);
						vb += 16;
						setR();
						ret += decodeVAG(vb, db+(blk/2)*28, 1);
						vb += 16;
						db += 28;
					}
				}
				break;
			default:
				break;
		}
		if(*(s+2)&0x80){
			resetWork();
		}
		s += 8*16*8 + 24;
	}while(--s_len>0);
	return ret;
}


LONG decodeXAf(UBYTE *s, WORD *d, LONG s_len)
{
	LONG blk,ret=0,g,u;
	UBYTE *vb = vagbuf;
	WORD  *db = d;
	do {
		blk = XAtoVAG(s+8, vb);
		switch(*(s+3)){
			case 0:
			case 4:
				setL();
				ret = decodeVAGf(vb, db, blk);
				break;
			case 1:
			case 5:
				for(g=0;g<18;g++){
					for(u=0;u<8/2;u++){
						setL();
						ret += decodeVAGf(vb, db          , 1);
						vb += 16;
						setR();
						ret += decodeVAGf(vb, db+(blk/2)*28, 1);
						vb += 16;
						db += 28;
					}
				}
				break;
			default:
				break;
		}
		if(*(s+2)&0x80){
			resetWork();
		}
		s += 8*16*8 + 24;
	}while(--s_len>0);
	return ret;
}



