#include "vxcodec.h"

void prefilter(WORD *x, ExD *y, ExD z)
{
	ExD a, b, c, d, e, f, g, h, m;
	register WORD i;
	if(z <= 2.0){
		b = 1.0;
		d = 0.0;
		c = 0.0;
		f = 0.0;
		e = 0.0;
	}
	else {
		a = tan(PI/z);
		h = (1.0+0.1*sqrt(2.0))*a;
		m = a*a;
		g = 1.0 + h + m;
		
		b = m/g;
		c = 2.0*m;
		d = m/g;
		e = -2.0*(1.0-m)/g;
		f = (1.0-h*a+m)/g;
	}
	
	for(i=0;i<0x001C;i++){
		if(z > 2.0){
			y[i] = b*(ExD)x[i] + c*(ExD)x1 +
			  ((d*(ExD)x2-e*Y1)*(ExD)x2) - e*Y1 - f*y2;
		}
		else {
			y[i] = b*(ExD)x[i];
		}
		
		if(y[i] >  32767.0*(60.0/64.0)){
			y[i] = 32767.0*(60.0/64.0);
		}
		
		if(y[i] < -32768.0*(60.0/64.0)){
			y[i] = -32768.0*(60.0/64.0);
		}
		
		y2=Y1;
		Y1=y[i];
		x2=x1;
		x1=x[i];
	}
	return;
}


void quantize(ExD *b, WORD *d, WORD flt, WORD rng)
{
	ExD a, e, f, g, h;
	register LONG i, j ,k;
	for(i=0;i<28;i++){
		h = q0*R[flt][0];
		g = b[i] + h;
		f = q1*R[flt][1];
		a = f +g;
		k = 1<<rng;
		f = (ExD)k;
		g = a*f;
		k = (LONG)g;
		k += 0x00000800L;
		k &= 0xFFFFF000L;
		k = max(min(k,32767L),-32768L);
		j = k>>rng;
		f = (ExD)j - a;
		e = f;
		d[i] = (WORD)k;
		q3 = q2;
		q2 = q1;
		q1 = q0;
		q0 = e;
	}
	return ;
}
