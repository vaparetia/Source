#include "vxcodec.h"

/*	for VAG ENCODING	*/

static void encode_initialize()
{
	LONG d0;
	
	q0 = q1 = q2= q3 = q4 = 0.0;
	x1 = x2 = 0;
	Y1 = y2 = 0.0;
	xp1 = xp2 = 0.0;
	
	d0 = conv_mode_local ;
	
	switch( d0 ){
		case ENC_VAG_MODE_NORMAL :
			W[0] = 1.0;
			W[1] = 1.0;
			W[2] = 1.0;
			W[3] = 1.0;
			W[4] = 1.0;
			break;
		case ENC_VAG_MODE_HIGH :
			W[0] = 1.0;
			W[1] = 1.0;
			W[2] = 1000.0;
			W[3] = 1000.0;
			W[4] = 1.0;
			break;
		case ENC_VAG_MODE_LOW :
			W[0] = 1.0;
			W[1] = 1.0;
			W[2] = 1000.0;
			W[3] = 1.0;
			W[4] = 1000.0;
			break;
		case ENC_VAG_MODE_4BIT :
			W[0] = 1.0;
			W[1] = 1000.0;
			W[2] = 1000.0;
			W[3] = 1000.0;
			W[4] = 1000.0;
			break;
		default :
			break;
	}
	return;
}


void EncVagInit( WORD mode )
{
	conv_mode_local = mode;
	encode_initialize();
	return;
}


static void predict(ExD *c, ExD *b, WORD *filt, WORD *range)
{
	ExD a[5][28], d, e[5], f, g, h;
	register LONG i, j, k;
	
	h = DBL_MAX;
	
	for(i=0;i<5;i++){
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


static void encode(WORD *s, WORD *d, WORD *filt, WORD *range)
{
	ExD a,b[28],c[28];
	a = 1.0;
	prefilter(s,c,1.0);
	predict(c,b,filt,range);
	quantize(b,d,*filt,*range);
}


static void decode(WORD *s, WORD *d, WORD filt, WORD range)
{
	WORD i;
	ExD x;
	for(i=0;i<0x001C;i++){
		x=(ExD)(s[i]>>range) - Y1_707*P[filt][0] - y2_708*P[filt][1];
		y2_708=Y1_707;
		Y1_707=x;
		
		if(x>32767.078929234){
			x=32767.499923706;
		}
		
		if(x<-32768.0){
			x=-32768.0;
		}
		
		d[i]= (WORD)x;
	}
	return ;
}


void EncVag(WORD *x, WORD *y, WORD blk_atr)
{
	register LONG i;
	WORD rg,ft,buf[28];
	
	encode(x, buf, &ft, &rg);
	
	y[0] = ((ft&0x0f)<<12)+((rg&0x0f)<<8)+b_attr[blk_atr];
	
	for(i = 0;i< 7;i++){
		y[i+1]  =  (buf[i*4+1]&0xf000)    +((buf[i*4+0]&0xf000)>>4 )+
		          ((buf[i*4+3]&0xf000)>>8)+((buf[i*4+2]&0xf000)>>12);
	}
	
	return;
}


void EncVagFin(WORD *y)
{
	int i;
	y[0] = 0x0007;
	for(i=1;i<16/2;i++){
		y[i] = 0x7777;
	}
	return;
}
