/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/geom/edgegeom.h"
#include "edge/geom/edgegeom_internal.h"
#include "edge/geom/edgegeom_masks.h"


#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38f
#endif


const qword s_AAaa = { 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13, 0x10, 0x11, 0x12, 0x13 };
const qword s_BbbB = { 0x04, 0x05, 0x06, 0x07, 0x14, 0x15, 0x16, 0x17, 0x14, 0x15, 0x16, 0x17, 0x04, 0x05, 0x06, 0x07 };
const qword s_AEIMaiemAEIMaiem = { 0x00, 0x04, 0x08, 0x0C, 0x10, 0x18, 0x14, 0x1C, 0x00, 0x04, 0x08, 0x0C, 0x10, 0x18, 0x14, 0x1C };

//const qword s_ABCa = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x10, 0x11, 0x12, 0x13 };
const qword s_ABCb = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x14, 0x15, 0x16, 0x17 };
const qword s_ABCc = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x18, 0x19, 0x1A, 0x1B };
const qword s_ABCd = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x1C, 0x1D, 0x1E, 0x1F };

//--------------------------------------------------------------------------------

#if CULLOCCLUDEDTRIANGLES_INTRINSICS
uint32_t cullOccludedTriangles(EdgeGeomSpuContext *ctx, const uint32_t indexCount, const uint32_t occluderCount, float occluderData[8*4*4], int32_t indexBias, EdgeGeomCullingResults *detailedResults)
{
    const uint32_t vertexCount = ctx->spuConfigInfo.numVertexes;
	const uint32_t iterationCount = ((vertexCount+3) & ~3) / 4;
	void * const pVertTemp = ctx->uniformTables[ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF];

	const qword sel_00FF = si_fsmbi(0x00FF);
	const qword s_AAAA = si_ila(0x10203);
	const qword s_BBBB = si_orbi(s_AAAA, 0x04);
	const qword s_CCCC = si_orbi(s_AAAA, 0x08);
	const qword s_DDDD = si_orbi(s_AAAA, 0x0C);
	const qword s_CcDd = si_rotqbyi((qword)s_DdCc, 8);
	const qword s_BbAa = si_rotqbyi((qword)s_AaBb, 8);
	const qword s_ABcd = si_rotqbyi((qword)s_cdAB, 8);
	const qword k1_0f    = si_ilhu(0x3F80);

	//
	// Loop #1: Compute the min max of the for each vertices...
	//                                                                                        
	qword pVertData  = si_from_ptr(pVertTemp);
	const qword sel_000F = si_fsmbi(0x000F);
	const qword k80      = si_ilh(0x8080);
	const qword s_AAA0   = si_selb((qword)s_AAAA, k80, sel_000F);
	const qword s_BBB0   = si_selb((qword)s_BBBB, k80, sel_000F);
	const qword s_CCC0   = si_selb((qword)s_CCCC, k80, sel_000F);
	const qword s_DDD0   = si_selb((qword)s_DDDD, k80, sel_000F);

	qword qmin0  = (qword)(vec_float4){  FLT_MAX,  FLT_MAX,  FLT_MAX,  FLT_MAX };
	qword qmax0  = (qword)(vec_float4){ -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };
	qword qmin1  = (qword)(vec_float4){  FLT_MAX,  FLT_MAX,  FLT_MAX,  FLT_MAX };
	qword qmax1  = (qword)(vec_float4){ -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };
	qword qmin2  = (qword)(vec_float4){  FLT_MAX,  FLT_MAX,  FLT_MAX,  FLT_MAX };
	qword qmax2  = (qword)(vec_float4){ -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };
	qword qmin3  = (qword)(vec_float4){  FLT_MAX,  FLT_MAX,  FLT_MAX,  FLT_MAX };
	qword qmax3  = (qword)(vec_float4){ -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };

	qword copy0 = si_lqd(pVertData, 0x00);
	qword copy1 = si_lqd(pVertData, 0x10);
	qword copy2 = si_lqd(pVertData, 0x20);
	qword copy3 = si_lqd(pVertData, 0x30);

	qword pVertOut   = pVertData;
	qword pVertDelay = pVertData;

	pVertData = si_ai(pVertData, 0x40);

	qword vertex0 = si_lqd(pVertData, 0x00);
	qword vertex1 = si_lqd(pVertData, 0x10);
	qword vertex2 = si_lqd(pVertData, 0x20);
	qword vertex3 = si_lqd(pVertData, 0x30);

	qword out0 = (qword)spu_splats(0);
	qword out1 = (qword)spu_splats(0);
	qword out2 = (qword)spu_splats(0);
	qword out3 = (qword)spu_splats(0);

	qword cmpMin0 = (qword)spu_splats(-1);
	qword cmpMax0 = (qword)spu_splats(-1);
	qword cmpMin1 = (qword)spu_splats(-1);
	qword cmpMax1 = (qword)spu_splats(-1);
	qword cmpMin2 = (qword)spu_splats(-1);
	qword cmpMax2 = (qword)spu_splats(-1);
	qword cmpMin3 = (qword)spu_splats(-1);
	qword cmpMax3 = (qword)spu_splats(-1);

	qword sgnMask = (qword)spu_splats(-1);
	sgnMask = si_rotmi(sgnMask, -1);


	// Preamble:
	const qword wval01 = si_shufb(copy0, copy1, (qword)s_DdCc);
	const qword wval23 = si_shufb(copy2, copy3, (qword)s_CcDd);
	const qword wval   = si_selb(wval01,  wval23,   (qword)sel_00FF);
	const qword recipA = si_frest(wval);
	const qword recipB = si_fi(wval, recipA);
	const qword recipC = si_fnms(recipB, wval, k1_0f);
	const qword ooWs   = si_fma(recipC, recipB, recipB);
	      qword ooW    = si_and(ooWs, sgnMask);

	// 32 cycles per loop iteration
	for ( uint32_t vndx = 0; vndx < iterationCount; vndx++ )
	{
		    qmin0 = si_selb(out0, qmin0, cmpMin0);
		    qmax0 = si_selb(out0, qmax0, cmpMax0);
		    qmin1 = si_selb(out1, qmin1, cmpMin1);
		    qmax1 = si_selb(out1, qmax1, cmpMax1);
		    qmin2 = si_selb(out2, qmin2, cmpMin2);
		    qmax2 = si_selb(out2, qmax2, cmpMax2);
		    qmin3 = si_selb(out3, qmin3, cmpMin3);
		    qmax3 = si_selb(out3, qmax3, cmpMax3);

		    si_stqd(out0, pVertOut, 0x00);
		    si_stqd(out1, pVertOut, 0x10);
		    si_stqd(out2, pVertOut, 0x20);
		    si_stqd(out3, pVertOut, 0x30);
		    pVertOut = pVertDelay;

		const qword wval01   = si_shufb(vertex0, vertex1, (qword)s_DdCc);
		const qword wval23   = si_shufb(vertex2, vertex3, (qword)s_CcDd);
		const qword wval     = si_selb(wval01,  wval23,   (qword)sel_00FF);

		  const qword ooW0   = si_shufb(ooW, ooW, s_AAA0);
		  const qword ooW1   = si_shufb(ooW, ooW, s_BBB0);
		  const qword ooW2   = si_shufb(ooW, ooW, s_CCC0);
		  const qword ooW3   = si_shufb(ooW, ooW, s_DDD0);

		  out0 = si_fm(copy0, ooW0);
		  out1 = si_fm(copy1, ooW1);
		  out2 = si_fm(copy2, ooW2);
		  out3 = si_fm(copy3, ooW3);
		  pVertDelay = pVertData;

		copy0 = vertex0;
		copy1 = vertex1;
		copy2 = vertex2;
		copy3 = vertex3;

		const qword recipA = si_frest(wval);
		const qword recipB = si_fi(wval, recipA);
		const qword recipC = si_fnms(recipB, wval, k1_0f);
		const qword ooWs   = si_fma(recipC, recipB, recipB);

		  cmpMin0 = si_fcgt( out0, qmin0);
		  cmpMax0 = si_fcgt(qmax0,  out0);
		  cmpMin1 = si_fcgt( out1, qmin1);
		  cmpMax1 = si_fcgt(qmax1,  out1);
		  cmpMin2 = si_fcgt( out2, qmin2);
		  cmpMax2 = si_fcgt(qmax2,  out2);
		  cmpMin3 = si_fcgt( out3, qmin3);
		  cmpMax3 = si_fcgt(qmax3,  out3);

	    ooW = si_and(ooWs, sgnMask);
		pVertData = si_ai(pVertData, 0x40);

		vertex0 = si_lqd(pVertData, 0x00);
		vertex1 = si_lqd(pVertData, 0x10);
		vertex2 = si_lqd(pVertData, 0x20);
		vertex3 = si_lqd(pVertData, 0x30);
	}

	// Postamble:
	switch ( vertexCount & 3 )
	{
	  case 0:
		  qmin3 = si_selb(out3, qmin3, cmpMin3);
		  qmax3 = si_selb(out3, qmax3, cmpMax3);
	  case 3:
		  qmin2 = si_selb(out2, qmin2, cmpMin2);
		  qmax2 = si_selb(out2, qmax2, cmpMax2);
	  case 2:
		  qmin1 = si_selb(out1, qmin1, cmpMin1);
		  qmax1 = si_selb(out1, qmax1, cmpMax1);
	  case 1:
		  qmin0 = si_selb(out0, qmin0, cmpMin0);
		  qmax0 = si_selb(out0, qmax0, cmpMax0);
	}

	const qword cmpMin01		= si_fcgt(qmin0, qmin1);
	const qword cmpMin23		= si_fcgt(qmin2, qmin3);
	const qword cmpMax01		= si_fcgt(qmax1, qmax0);
	const qword cmpMax23		= si_fcgt(qmax3, qmax2);

	const qword qmin01			= si_selb(qmin0, qmin1, cmpMin01);
	const qword qmin23			= si_selb(qmin2, qmin3, cmpMin23);
	const qword qmax01			= si_selb(qmax0, qmax1, cmpMax01);
	const qword qmax23			= si_selb(qmax2, qmax3, cmpMax23);

	const qword cmpQMin			= si_fcgt(qmin01, qmin23);
	const qword cmpQMax			= si_fcgt(qmax23, qmax01);
	const qword qmin			= si_selb(qmin01, qmin23, cmpQMin);
	const qword qmax			= si_selb(qmax01, qmax23, cmpQMax);

	si_stqd(out0, pVertOut, 0x00);
	si_stqd(out1, pVertOut, 0x10);
	si_stqd(out2, pVertOut, 0x20);
	si_stqd(out3, pVertOut, 0x30);
	pVertOut = pVertDelay;



	const qword zmin = si_shufb(qmin, qmin, (qword)s_CCCC);
	const qword zmax = si_shufb(qmax, qmax, (qword)s_CCCC);

	const qword boxx = si_shufb(qmin, qmax, (qword)s_AAaa);
	const qword boxy = si_shufb(qmin, qmax, (qword)s_BbbB);

	qword wndxMask = si_il(1);
	qword notOccluded = si_il(0);
	qword fullyHidden = { 0, 0, 0, (1<<occluderCount)-1,
						  0, 0, 0, 0,
						  0, 0, 0, 0,
						  0, 0, 0, 0 } ;

	//
	// Loop #2: Set a bit in the Nth bit of the W for each vertex if the vertex is behind the occluder
	//
	qword occluders = si_from_ptr(occluderData);
	for ( uint32_t ondx = 0; ondx < occluderCount; ondx++ )
	{
		// Load the occluders:
		const qword occluder0 = si_lqd(occluders, 0x00);
		const qword occluder1 = si_lqd(occluders, 0x10);
		const qword occluder2 = si_lqd(occluders, 0x20);
		const qword occluder3 = si_lqd(occluders, 0x30);

		const qword wv0 = si_fs(occluder1, occluder0);
		const qword wv1 = si_fs(occluder2, occluder1);
		const qword wv2 = si_fs(occluder3, occluder2);
		const qword wv3 = si_fs(occluder0, occluder3);

		const qword s_AAAA = si_ila(0x10203);
		const qword inc    = si_ilh(0x0404);
		const qword s_BBBB = si_a(s_AAAA, inc);
		const qword s_CCCC = si_a(s_BBBB, inc);

		//

		const qword wtmp0  = si_shufb(occluder0, occluder1, (qword)s_CcDd);
		const qword wtmp1  = si_shufb(occluder2, occluder3, (qword)s_DdCc);
		const qword ozzzz  = si_shufb(wtmp0, wtmp1, (qword)s_ABcd);

		//

		const qword wx0 = si_shufb(occluder0, occluder0, (qword)s_AAAA);
		const qword wx1 = si_shufb(occluder1, occluder1, (qword)s_AAAA);
		const qword wx2 = si_shufb(occluder2, occluder2, (qword)s_AAAA);
		const qword wx3 = si_shufb(occluder3, occluder3, (qword)s_AAAA);

		const qword wy0 = si_shufb(occluder0, occluder0, (qword)s_BBBB);
		const qword wy1 = si_shufb(occluder1, occluder1, (qword)s_BBBB);
		const qword wy2 = si_shufb(occluder2, occluder2, (qword)s_BBBB);
		const qword wy3 = si_shufb(occluder3, occluder3, (qword)s_BBBB);

		const qword wz0 = si_shufb(occluder0, occluder0, (qword)s_CCCC);

		//

		const qword pbX0 = si_fs(boxx, wx0); 
		const qword pbX1 = si_fs(boxx, wx1); 
		const qword pbX2 = si_fs(boxx, wx2); 
		const qword pbX3 = si_fs(boxx, wx3); 

		const qword pbY0 = si_fs(boxy, wy0); 
		const qword pbY1 = si_fs(boxy, wy1); 
		const qword pbY2 = si_fs(boxy, wy2); 
		const qword pbY3 = si_fs(boxy, wy3); 

		//

		const qword wvx0 = si_shufb(wv0, wv0, (qword)s_AAAA);
		const qword wvx1 = si_shufb(wv1, wv1, (qword)s_AAAA);
		const qword wvx2 = si_shufb(wv2, wv2, (qword)s_AAAA);
		const qword wvx3 = si_shufb(wv3, wv3, (qword)s_AAAA);

		const qword wvy0 = si_shufb(wv0, wv0, (qword)s_BBBB);
		const qword wvy1 = si_shufb(wv1, wv1, (qword)s_BBBB);
		const qword wvy2 = si_shufb(wv2, wv2, (qword)s_BBBB);
		const qword wvy3 = si_shufb(wv3, wv3, (qword)s_BBBB);

		const qword wvz0 = si_shufb(wv0, wv0, (qword)s_CCCC);
		const qword wvz1 = si_shufb(wv1, wv1, (qword)s_CCCC);

		//

		const qword c00  = si_fm(pbX0, wvy0);
		const qword c01  = si_fm(pbX1, wvy1);
		const qword c02  = si_fm(pbX2, wvy2);
		const qword c03  = si_fm(pbX3, wvy3);

		const qword cp0  = si_fnms(pbY0, wvx0, c00);
		const qword cp1  = si_fnms(pbY1, wvx1, c01);
		const qword cp2  = si_fnms(pbY2, wvx2, c02);
		const qword cp3  = si_fnms(pbY3, wvx3, c03);

		//

		// Use these lines if Z is into the screen
//~		const qword ztstif = si_fcgt(zmin, ozzzz);
//~		const qword ztstbh = si_fcgt(ozzzz, zmax);

		// Use these lines if Z is out of the screen
		const qword ztstif = si_fcgt(zmax, ozzzz);
		const qword ztstbh = si_fcgt(ozzzz, zmin);

		const qword notif  = si_orx(ztstif);
		const qword notbh  = si_orx(ztstbh);
	   
		//

		const qword cptmp0 = si_eqv(cp0, cp1);
		const qword cptmp1 = si_eqv(cp1, cp2);
		const qword cptmp2 = si_eqv(cp2, cp3);

		const qword cptmp3 = si_and(cptmp0, cptmp1);
		const qword cc     = si_and(cptmp3, cptmp2);
		const qword ncc    = si_xori(cc, -1);

		const qword icc    = si_orx(ncc);
		const qword inside  = si_xori(icc, -1);
		const qword inbf    = si_rotmai(inside, -31);

		// // 

		const qword skipocc = si_xori(notif, -1);
		
		//

		qword fullyHiddenBit = {0,0,0,0};

		if ( si_to_int(skipocc) == 0 )
		{
			const qword clearTriCount = si_andc(inbf, notbh);
			
			if ( si_to_int(clearTriCount) )
			{
				// The whole vertex set is occluded...
				return 0;
			}

			const qword wnxtmp = si_fm(wvy0, wvz1);
			const qword wnytmp = si_fm(wvz0, wvx1);
			const qword wnztmp = si_fm(wvx0, wvy1);

			const qword wnx = si_fnms(wvy1, wvz0, wnxtmp);
			const qword wny = si_fnms(wvz1, wvx0, wnytmp);
			const qword wnz = si_fnms(wvx1, wvy0, wnztmp);

			const qword zsat = si_rotmai(wnz, -31);
			fullyHiddenBit = si_ori(wndxMask, 0); 

			pVertData  = si_from_ptr(pVertTemp);
			pVertOut   = pVertData;

			qword xyz00 = si_lqd(pVertData, 0x00);
			qword xyz10 = si_lqd(pVertData, 0x10);
			qword xyz20 = si_lqd(pVertData, 0x20);
			qword xyz30 = si_lqd(pVertData, 0x30);
			qword delay = si_il(0);

			qword vx0 = (qword)spu_splats(0);
			qword vx1 = (qword)spu_splats(0);
			qword vx2 = (qword)spu_splats(0);
			qword vx3 = (qword)spu_splats(0);
			qword vy0 = (qword)spu_splats(0);
			qword vy1 = (qword)spu_splats(0);
			qword vy2 = (qword)spu_splats(0);
			qword vy3 = (qword)spu_splats(0);
			qword behind = (qword)spu_splats(0);
			qword xxxx0 = (qword)spu_splats(0);
			qword yyyy0 = (qword)spu_splats(0);
			qword zzzz0 = (qword)spu_splats(0);
			qword wwww0 = (qword)spu_splats(0);

			// 41 cycles per loop iteration
			for ( uint32_t vndx = 0; vndx < iterationCount + 1 ; vndx++ )
			{
				  const qword xyz0t          = si_lqd(pVertOut, 0x00);
				  const qword xyz1t          = si_lqd(pVertOut, 0x10);
				  const qword xyz2t          = si_lqd(pVertOut, 0x20);
				  const qword xyz3t          = si_lqd(pVertOut, 0x30);
				
				  const qword tmp01          = si_fm(vx0, wvy0);
				  const qword tmp11          = si_fm(vx1, wvy1);
				  const qword tmp21          = si_fm(vx2, wvy2);
				  const qword tmp31          = si_fm(vx3, wvy3);
                                             
				  const qword ins0           = si_fnms(vy0, wvx0, tmp01);
				  const qword ins1           = si_fnms(vy1, wvx1, tmp11);
				  const qword ins2           = si_fnms(vy2, wvx2, tmp21);
				  const qword ins3           = si_fnms(vy3, wvx3, tmp31);
                                             
				  const qword tmp02          = si_eqv(ins0, ins1);
				  const qword tmp12          = si_eqv(ins1, ins2);
				  const qword tmp22          = si_eqv(ins2, ins3);
                                             
				  const qword tmp32          = si_and(tmp02, tmp12);
				  const qword insideOcc      = si_and(tmp22, tmp32);
                                             
				  const qword hidden         = si_and(insideOcc, behind);
                                             
				  const qword hidesat        = si_rotmai(hidden, -31);
				  const qword hidebit        = si_and(hidesat, wndxMask);
				  const qword wwww1          = si_or(wwww0, hidebit);

				  const qword delayedhidebit = si_orc(hidebit, delay);
				  const qword delayedoccbit  = si_and(hidebit, delay);

				  delay = si_il(-1);
				  fullyHiddenBit = si_and(fullyHiddenBit, delayedhidebit);
				  notOccluded    = si_or(notOccluded, delayedoccbit);
                                             
				  const qword xyz01          = si_shufb(xyz0t, wwww1, (qword)s_ABCa);
				  const qword xyz11          = si_shufb(xyz1t, wwww1, (qword)s_ABCb);
				  const qword xyz21          = si_shufb(xyz2t, wwww1, (qword)s_ABCc);
				  const qword xyz31          = si_shufb(xyz3t, wwww1, (qword)s_ABCd);

				  si_stqd(xyz01, pVertOut, 0x00);
				  si_stqd(xyz11, pVertOut, 0x10);
				  si_stqd(xyz21, pVertOut, 0x20);
				  si_stqd(xyz31, pVertOut, 0x30);
 
				  pVertOut = pVertData;


			    pVertData = si_ai(pVertData, 0x40);

				const qword tmp00 = si_shufb(xyz00, xyz10, (qword)s_AaBb); // x0 x1 y0 y1
				const qword tmp10 = si_shufb(xyz20, xyz30, (qword)s_BbAa); // y2 y3 x2 x3
				const qword tmp20 = si_shufb(xyz00, xyz10, (qword)s_CcDd); // z0 z1 w0 w1
				const qword tmp30 = si_shufb(xyz20, xyz30, (qword)s_DdCc); // w2 w3 z2 z3
                                  
				xxxx0 = si_shufb(tmp00, tmp10, (qword)s_ABcd); // x0 x1 x2 x3
				yyyy0 = si_shufb(tmp10, tmp00, (qword)s_cdAB); // y0 y1 y2 y3
				zzzz0 = si_shufb(tmp20, tmp30, (qword)s_ABcd); // z0 z1 z2 z3
				wwww0 = si_shufb(tmp30, tmp20, (qword)s_cdAB); // w0 w1 w2 w3

				xyz00 = si_lqd(pVertData, 0x00);
				xyz10 = si_lqd(pVertData, 0x10);
				xyz20 = si_lqd(pVertData, 0x20);
				xyz30 = si_lqd(pVertData, 0x30);
                           
				vx0   = si_fs(xxxx0, wx0);
				vx1   = si_fs(xxxx0, wx1);
				vx2   = si_fs(xxxx0, wx2);
				vx3   = si_fs(xxxx0, wx3);
                      
				vy0   = si_fs(yyyy0, wy0);
				vy1   = si_fs(yyyy0, wy1);
				vy2   = si_fs(yyyy0, wy2);
				vy3   = si_fs(yyyy0, wy3);
                                  
				const qword vz0   = si_fs(zzzz0, wz0);



				const qword dot0   = si_fm(vx0, wnx);
				const qword dot1   = si_fma(vy0, wny, dot0);
				const qword dot2   = si_fma(vz0, wnz, dot1);
                                   
				behind = si_eqv(dot2, zsat);
			}
		}

		fullyHiddenBit = si_xori(fullyHiddenBit, -1);
		fullyHiddenBit = si_orx(fullyHiddenBit);
		fullyHiddenBit = si_xori(fullyHiddenBit, -1);

		fullyHiddenBit = si_orc(fullyHiddenBit, wndxMask); 

		fullyHidden = si_and(fullyHidden, fullyHiddenBit);

		wndxMask = si_shli(wndxMask, 1);
		occluders = si_ai(occluders, 64);
	}


	if ( si_to_uint(fullyHidden) )
	{
		return 0;
	}	

	const uint32_t numIndexes = indexCount;
	
	if ( si_to_uint(notOccluded) == 0)
	{
		//if no vertices are occluded, don't move the free pointer
		return numIndexes;
	}
	else if (numIndexes == 0)
	{
		return 0;
	}

	const uint32_t numTriangles = numIndexes / 3;
	const qword pTriData0  = si_from_uint(ctx->indexesLs);
	const qword pTriData1  = si_ai(pTriData0, 0x10);
	qword pTriOffset = si_il(0);

	const int   modTriangles = numTriangles & 0x3;
	const qword ignoreLast   = si_from_int((modTriangles == 0) ? 0 : (0xf >> modTriangles));

    pVertData  = si_from_ptr((float *)((uint32_t)pVertTemp + indexBias * sizeof(qword)));

	// Loops:
	//   a b c A B C d e | f D E F 
	//           a b c A | B C d e f D E F 

	// First loop  mask:  ABCD
	// Second loop mask: CDab = ABCD ^ s_inputMaskXor
	qword s_inputMask = (qword)s_ABCD;
	const qword s_inputMaskXor = { 0x08, 0x08, 0x08, 0x08,
								   0x08, 0x08, 0x08, 0x08,
								   0x18, 0x18, 0x18, 0x18,
								   0x18, 0x18, 0x18, 0x18};


	// Output masks:
	const qword pMasks00 = si_from_ptr(cullMaskTable00);
    const qword pMasks01 = si_from_ptr(cullMaskTable01);
    const qword pPrevMask = si_from_ptr(cullPreviousTable);

	qword loopCnt = si_from_uint((numTriangles+3)/4);
	loopCnt       = si_ai(loopCnt, 1);

	qword tri012 = si_lqx(pTriData0, pTriOffset);
	qword tri123 = si_lqx(pTriData1, pTriOffset);

	qword pOut = si_shufb(pTriData0, pTriData0, (qword)s_AAAA);

	// To make sure we don't write after the end of the buffer...
	qword dummyBuffer[3];
	const qword pDummy = si_shufb(si_from_ptr(dummyBuffer), (qword)s_AAAA, (qword)s_AAAA);
	const qword endOfBuffer = si_a(pOut, si_shufb(si_from_int( (numTriangles*6 + 0xF) & ~0xF ), (qword)s_AAAA, (qword)s_AAAA));
	const qword endOfBufferOffset = (qword)(vec_int4){-17, -33, 0, 0};
	const qword endOfBuffer0 = si_a(endOfBuffer, endOfBufferOffset);

    qword prevIndex = (qword)spu_splats(0);
    qword prevRotAmount = (qword)spu_splats(0);
    qword numBytes = (qword)spu_splats(0);
    qword prevIndexTmp = (qword)spu_splats(0);
	qword localNumRemainingIndexes = (qword)spu_splats(0x0);
    const qword negativeOne = (qword)spu_splats(0xffffffff);

	qword firstLoopMask =   {   0x0, 0x0, 0x0, 0xF,
								0x0, 0x0, 0x0, 0xF,
								0x0, 0x0, 0x0, 0x0,
								0x0, 0x0, 0x0, 0x0};

	qword keeperNdx = (qword)spu_splats(0);
	qword ndx012    = (qword)spu_splats(0);
	qword ndx_23     = (qword)spu_splats(0);

	qword xyz00 = (qword)spu_splats(0);
	qword xyz01 = (qword)spu_splats(0);
	qword xyz02 = (qword)spu_splats(0);
	qword xyz10 = (qword)spu_splats(0);
	qword xyz11 = (qword)spu_splats(0);
	qword xyz12 = (qword)spu_splats(0);
	qword xyz20 = (qword)spu_splats(0);
	qword xyz21 = (qword)spu_splats(0);
	qword xyz22 = (qword)spu_splats(0);
	qword xyz30 = (qword)spu_splats(0);
	qword xyz31 = (qword)spu_splats(0);
	qword xyz32 = (qword)spu_splats(0);

	qword ndx012c = (qword)spu_splats(0);
	qword ndx_23c = (qword)spu_splats(0);

	qword tri012n = (qword)spu_splats(0);
	qword tri123n = (qword)spu_splats(0);




	// 46 cycles per loop iteration
	for ( uint32_t tndx = 0; tndx < ((numTriangles+3)/4)+2 ; tndx++ )
	{
		// Read 2 quads => 4 triangles:
		pTriOffset = si_ai(pTriOffset, 0x18);
 
		    // Delay's done using the firstLoopMask
		    const qword keeperNdxDelayed    = si_or(keeperNdx, firstLoopMask);
		    firstLoopMask = si_shlqbyi(firstLoopMask, 4);
		    
		    // Compute the indices to keep:
		    const qword keeperOffset = si_shli(keeperNdxDelayed, 4);
		    const qword s_mask00 = si_lqx(pMasks00, keeperOffset);
		    const qword s_mask01 = si_lqx(pMasks01, keeperOffset);
		    
		    const qword keptIndexes0 = si_shufb(ndx012c, ndx_23c, s_mask00);
		    const qword keptIndexes1 = si_shufb(ndx_23c, ndx_23c, s_mask01);
		    
		    // Combine the indices with the previous ones:
		    const qword s_prevBase = si_lqx(pPrevMask, prevIndex); 
		    const qword sel_prev   = si_rotqmby(negativeOne, prevRotAmount);
            const qword s_prev     = si_selb((qword)s_ABCD, s_prevBase, sel_prev);
		    
		    pOut = si_a(pOut, numBytes); 
		    const qword prevOut = si_lqd(pOut, 0x00);
		    
		    const qword out0 = si_shufb(prevOut, keptIndexes0, s_prev);
		    const qword out1 = si_shufb(keptIndexes0, keptIndexes1, s_prevBase);
		    const qword out2 = si_shufb(keptIndexes1, keptIndexes1, s_prevBase);
		  
		    // Before to store, we have to load the next to indices to make sure we don't overwrite them :)
		    tri012n = si_lqx(pTriData0, pTriOffset);
		    tri123n = si_lqx(pTriData1, pTriOffset);
      	    
            // Compute the output values:
		    const qword pSafeOut2 = si_selb(pOut, pDummy, si_cgt(pOut, endOfBuffer0));
		    const qword pSafeOut3 = si_rotqbyi(pSafeOut2, 4);
		    
		    si_stqd(out0, pOut, 0x00);
		    si_stqd(out1, pSafeOut2, 0x10);
		    si_stqd(out2, pSafeOut3, 0x20);
            
		    const qword bitCount0 = si_cntb(keeperOffset);
		    const qword bitCount1 = si_shufb(bitCount0, (qword)s_AAAA, (qword)s_AAAA);
		    const qword numBytesTmp = si_sfi(bitCount1, 4);
		    numBytes = si_mpyui(numBytesTmp, 6);
            
		    localNumRemainingIndexes = si_a(localNumRemainingIndexes, numBytesTmp);
            
		    prevIndexTmp = si_andi(si_a(prevIndexTmp, numBytes), 0xf);
		    prevRotAmount = si_sfi(prevIndexTmp, 0);
		    prevIndex = si_shli(prevIndexTmp,4);
 		  
		  
		  // Extract the bitfield value from each 4 triangles:
		  const qword tmp20 = si_shufb(xyz00, xyz10, (qword)s_DdCc);
		  const qword tmp30 = si_shufb(xyz20, xyz30, (qword)s_CcDd);
		  const qword wwww0 = si_selb(tmp20, tmp30, sel_00FF);
		  
		  const qword tmp21 = si_shufb(xyz01, xyz11, (qword)s_DdCc);
		  const qword tmp31 = si_shufb(xyz21, xyz31, (qword)s_CcDd);
		  const qword wwww1 = si_selb(tmp21, tmp31, sel_00FF);
		  
		  const qword tmp22 = si_shufb(xyz02, xyz12, (qword)s_DdCc);
		  const qword tmp32 = si_shufb(xyz22, xyz32, (qword)s_CcDd);
		  const qword wwww2 = si_selb(tmp22, tmp32, sel_00FF);
 		  
		  // All three vertices must be occluded to have the triangle occluded:
		  const qword wfin0 = si_and(wwww0, wwww1);
		  const qword wfin1 = si_and(wwww2, wfin0);
		  const qword cfin = si_clgti(wfin1, 0);
		  const qword cgb  = si_gb(cfin);
		  
		  // Special case for the final iteration:
		  //     keeper.w[i:0..3] == -1 => keep the triangle i
		  loopCnt = si_ai(loopCnt, -1);
		  const qword isLastIteration     = si_ceqi(loopCnt, 0);
		  const qword lastIterationOffset = si_and(isLastIteration, ignoreLast);
		  
		  keeperNdx           = si_or(cgb, lastIterationOffset);

		  ndx012c = ndx012;
		  ndx_23c = ndx_23;


		ndx012 = si_shufb(tri012, tri123, s_inputMask);
		ndx_23 = si_shufb(tri123, tri123, s_inputMask);

		tri012 = tri012n;
		tri123 = tri123n;



		s_inputMask = si_xor(s_inputMask, s_inputMaskXor);


		//      ndx012       |    ndx_23
		//   a b c A B C d e | f D  E  F 
		//   0 1 2 3 4 5 6 7 | 8 9 10 11

		const qword ndx1  = si_shli(ndx012, 4);
		const qword ndx9  = si_shli(ndx_23, 4);

		const qword ndx3  = si_shlqbyi(ndx1, 4);
		const qword ndx5  = si_shlqbyi(ndx1, 8);
		const qword ndx7  = si_shlqbyi(ndx1, 12);
		const qword ndx11 = si_shlqbyi(ndx9, 4);

		const qword ndx0  = si_rotmi(ndx1, -16);
		const qword ndx2  = si_rotmi(ndx3, -16);
		const qword ndx4  = si_rotmi(ndx5, -16);
		const qword ndx6  = si_rotmi(ndx7, -16);

		const qword ndx8  = si_rotmi(ndx9,  -16);
		const qword ndx10 = si_rotmi(ndx11, -16);

		// Load each vertices:
		xyz00 = si_lqx(pVertData, ndx0);
		xyz01 = si_lqx(pVertData, ndx1);
		xyz02 = si_lqx(pVertData, ndx2);
		xyz10 = si_lqx(pVertData, ndx3);
		xyz11 = si_lqx(pVertData, ndx4);
		xyz12 = si_lqx(pVertData, ndx5);
		xyz20 = si_lqx(pVertData, ndx6);
		xyz21 = si_lqx(pVertData, ndx7);
		xyz22 = si_lqx(pVertData, ndx8);
		xyz30 = si_lqx(pVertData, ndx9);
		xyz31 = si_lqx(pVertData, ndx10);
		xyz32 = si_lqx(pVertData, ndx11);

   	}

	const uint32_t toRet = si_to_uint(localNumRemainingIndexes) * 3;
	return toRet;
}
#endif


