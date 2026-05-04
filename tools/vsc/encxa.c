#include "vxcodec.h"

/*	for XA ENCODING	*/

void EncXaInit()
{
	q0 = q1 = q2= q3 = q4 = 0.0;
	x1 = x2 = 0;
	Y1 = y2 = 0.0;
	xp1 = xp2 = 0.0;
	W[0] = 1.0;
	W[1] = 1.0;
	W[2] = 1.0;
	W[3] = 1.0;
	W[4] = 1.0;
	pushEncEnvironment(0);
	pushEncEnvironment(1);	
	return;
}


static void predict_xa(ExD *c, ExD *b, WORD *filt, WORD *range)
{
	ExD a[5][28], d, e[5], f, g, h;
	register LONG i, j, k;
	
	h = DBL_MAX;
	
	for(i=0;i<XA_FILTER_TYPE;i++){
		d = xp1;
		g = xp2;
		j = 0;
		e[i] = 0.0;
		for(j=0;j<0x001C;j++){
			
			a[i][j] = c[j]+d*P[i][0]+g*P[i][1];
			
			f = (a[i][j]>0.0)?a[i][j]:-a[i][j];
			
			if(e[i] < f){
				e[i] = f;
			}
			
			g = d;
			d = c[j];
		}
		
		if(h > e[i]*W[i]){
			h  = e[i]*W[i];
			*filt = (WORD)i;
		}
		
		if(i == 0){
			
			if(e[i] < 7.0){
				*filt = 0;
				break;
			}
		}
		
	}
	
	xp1 = d;
	xp2 = g;
	
	for(k=0;k<28;k++){
		b[k]=a[*filt][k];
	}
	
	j = max(min(((LONG)e[*filt]*M[*filt]),32767L),-32768L);
	i = 16384;
	for(*range=0;*range<12;(*range)++){
		k=(i>>3)+j;
		if((k&i)!=0){
			break;
		}
		i>>=1;
	}
	return;
}


static void encode_xa(WORD *s, WORD *d, WORD *filt, WORD *range)
{
	ExD a,b[28],c[28];
	a = 1.0;
	prefilter(s,c,1.0);
	predict_xa(c,b,filt,range);
	quantize(b,d,*filt,*range);
}


void EncXa(WORD *x, UBYTE *y, UBYTE fn, UBYTE ch, UBYTE sm, UBYTE ci)
{
	register LONG i,j,k;
	WORD rg,ft,buf[28];
	UBYTE buf2[18*8*16];
	for(k=0;k<18;k++){
		for(j=0;j<8;j+=2){
			if(ci&0x01){
				popEncEnvironment(eLeft);
			}
			encode_xa(x, buf, &ft, &rg);
			buf2[(k*8+j)*16+0] = nL[ft]+nLow[rg];
			buf2[(k*8+j)*16+1] = 0x02;
			for(i = 0;i< 14;i++){
				buf2[(k*8+j  )*16+i+2]  = (((UWORD)(buf[(i<<1)+1]))>>8 )&0xF0;
				buf2[(k*8+j  )*16+i+2] += (((UWORD)(buf[(i<<1)  ]))>>12)&0x0F;
			}
			x += 28;
			if(ci&0x01){
				pushEncEnvironment(eLeft);
				popEncEnvironment(eRight);
			}
			encode_xa(x, buf, &ft, &rg);
			buf2[(k*8+j+1)*16+0] = nL[ft]+nLow[rg];
			buf2[(k*8+j+1)*16+1] = 0x02;
			for(i = 0;i< 14;i++){
				buf2[(k*8+j+1)*16+i+2]  = (((UWORD)(buf[(i<<1)+1]))>>8 )&0xF0;
				buf2[(k*8+j+1)*16+i+2] += (((UWORD)(buf[(i<<1)  ]))>>12)&0x0F;
			}
			x += 28;
			if(ci&0x01){
				pushEncEnvironment(eRight);
			}
		}
	}
	y[0] = y[4] = fn;
	y[1] = y[5] = ch;
	y[2] = y[6] = sm;
	y[3] = y[7] = ci;
	VAGtoXA(buf2, y+8);
	return;
}
