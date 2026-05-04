//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vmp_zone.c
	ヴァンプ　ゾーン関係
	2001/03/02

	$Id: vmp_zone.c,v 1.1.1.3 2002/11/19 11:48:58 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../../shibata/t_headmark/headmark.h"
#include	"../../kano/attachment/attachment_called.h"
#include	"../../takabe/other/puppetik.h"
#include	"../../kira/face/face.h"
#include	"../util/ts_util.h"

#include	"vamp.h"
#include	"vmp_inline.h"
#define N_ZONE_MAX_FLOOR	(14)


#define	ZONE_VY		(-6200.0f)

//　ゾーンからゾーンへの移動量（ゾーン数）
static char ZoneStepTableFloor[N_ZONE_MAX_FLOOR][N_ZONE_MAX_FLOOR] = {
	{   0,   6,   6,   5,   4,   3,   2,   1,   2,   3,   4,   5,   6,   7 },
	{   6,   0,   2,   1,   2,   3,   4,   5,   6,   7,   6,   5,   4,   3 },
	{   6,   2,   0,   1,   2,   3,   4,   5,   6,   5,   4,   3,   2,   1 },
	{   5,   1,   1,   0,   1,   2,   3,   4,   5,   6,   5,   4,   3,   2 },
	{   4,   2,   2,   1,   0,   1,   2,   3,   4,   5,   6,   5,   4,   3 },
	{   3,   3,   3,   2,   1,   0,   1,   2,   3,   4,   5,   6,   5,   4 },
	{   2,   4,   4,   3,   2,   1,   0,   1,   2,   3,   4,   5,   6,   5 },
	{   1,   5,   5,   4,   3,   2,   1,   0,   1,   2,   3,   4,   5,   6 },
	{   2,   6,   6,   5,   4,   3,   2,   1,   0,   1,   2,   3,   4,   5 },
	{   3,   7,   5,   6,   5,   4,   3,   2,   1,   0,   1,   2,   3,   4 },
	{   4,   6,   4,   5,   6,   5,   4,   3,   2,   1,   0,   1,   2,   3 },
	{   5,   5,   3,   4,   5,   6,   5,   4,   3,   2,   1,   0,   1,   2 },
	{   6,   4,   2,   3,   4,   5,   6,   5,   4,   3,   2,   1,   0,   1 },
	{   7,   3,   1,   2,   3,   4,   5,   6,   5,   4,   3,   2,   1,   0 },
};

//お隣さん情報
static char ZoneNextTableFloor[N_ZONE_MAX_FLOOR][4] = {
	{  7, -1, -1, -1, },
	{  3, -1, -1, -1, },
	{  3, 13, -1, -1, },
	{  2,  4,  1, -1, },
	{  3,  5, -1, -1, },
	{  4,  6, -1, -1, },
	{  5,  7, -1, -1, },
	{  6,  8,  0, -1, },
	{  7,  9, -1, -1, },
	{  8, 10, -1, -1, },
	{  9, 11, -1, -1, },
	{ 10, 12, -1, -1, },
	{ 11, 13, -1, -1, },
	{  2, 12, -1, -1, },
};

//　床ゾーン　→　床ゾーン情報
static char ZoneToZoneInfo[N_ZONE_MAX_FLOOR][N_ZONE_MAX_FLOOR] = {
	{ 0, 6, 4, 4, 3, 3, 2, 1, 2, 3, 3, 4, 4, 5 },
	{ 6, 0, 2, 1, 2, 3, 3, 4, 5, 4, 4, 4, 3, 3 },
	{ 4, 2, 0, 1, 2, 4, 4, 4, 5, 4, 4, 4, 2, 1 },
	{ 4, 1, 1, 0, 1, 2, 3, 4, 4, 5, 4, 4, 3, 2 },
	{ 3, 3, 2, 1, 0, 1, 2, 4, 4, 4, 5, 4, 4, 4 },
	{ 3, 3, 4, 2, 1, 0, 1, 2, 4, 4, 4, 5, 4, 4 },
	{ 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 4, 5, 4 },
	{ 1, 4, 4, 4, 4, 2, 1, 0, 1, 2, 4, 4, 4, 5 },
	{ 2, 4, 5, 4, 4, 4, 2, 1, 0, 1, 2, 4, 4, 4 },
	{ 3, 4, 4, 5, 4, 4, 3, 2, 1, 0, 1, 2, 3, 4 },
	{ 3, 4, 4, 4, 5, 4, 4, 4, 2, 1, 0, 1, 2, 4 },
	{ 4, 4, 4, 4, 4, 5, 4, 4, 4, 2, 1, 0, 1, 2 },
	{ 4, 3, 2, 3, 4, 4, 5, 4, 4, 3, 2, 1, 0, 1 },
	{ 4, 3, 1, 2, 4, 4, 4, 5, 4, 4, 4, 2, 1, 0 },
};

//ヴァンプ用ゾーンデータ(プレイヤーのいけるとこ) min max の順
static FVECTOR VmpZoneVertsFloor[N_ZONE_MAX_FLOOR][2] = {
	{ {  -9700.0f, ZONE_VY, -234500.0f, 1.0f }, {  -7200.0f, ZONE_VY, -232300.0f, 1.0f }, },
	{ {  -6800.0f, ZONE_VY, -246000.0f, 1.0f }, {  -4200.0f, ZONE_VY, -243800.0f, 1.0f }, },
	{ { -10000.0f, ZONE_VY, -243800.0f, 1.0f }, {  -7500.0f, ZONE_VY, -241000.0f, 1.0f }, },
	{ {  -7500.0f, ZONE_VY, -243800.0f, 1.0f }, {  -5500.0f, ZONE_VY, -241000.0f, 1.0f }, },
	{ {  -7500.0f, ZONE_VY, -241000.0f, 1.0f }, {  -5500.0f, ZONE_VY, -238750.0f, 1.0f }, },
	{ {  -7500.0f, ZONE_VY, -238750.0f, 1.0f }, {  -5500.0f, ZONE_VY, -236500.0f, 1.0f }, },
	{ {  -7500.0f, ZONE_VY, -236500.0f, 1.0f }, {  -5500.0f, ZONE_VY, -234500.0f, 1.0f }, },
	
	{ { -9700.0f, ZONE_VY, -236500.0f, 1.0f }, {  -7500.0f, ZONE_VY, -234500.0f, 1.0f }, },
	{ { -12500.0f, ZONE_VY, -236500.0f, 1.0f }, { -9700.0f, ZONE_VY, -234500.0f, 1.0f }, },
	
	{ { -14500.0f, ZONE_VY, -236100.0f, 1.0f }, { -12500.0f, ZONE_VY, -234500.0f, 1.0f }, },
	{ { -14500.0f, ZONE_VY, -238750.0f, 1.0f }, { -12500.0f, ZONE_VY, -236100.0f, 1.0f }, },

	{ { -14500.0f, ZONE_VY, -241000.0f, 1.0f }, { -12500.0f, ZONE_VY, -238750.0f, 1.0f }, },
	{ { -14500.0f, ZONE_VY, -243800.0f, 1.0f }, { -12500.0f, ZONE_VY, -241000.0f, 1.0f }, },
	{ { -12500.0f, ZONE_VY, -243800.0f, 1.0f }, { -10000.0f, ZONE_VY, -241000.0f, 1.0f }, },
};
#define WATER_POS_VY	(-8000.0f)
#define N_ZONE_MAX_WATER	(23)
//ヴァンプ用飛び出し込みデータ(水中)
static FVECTOR VmpZoneVertsWater[N_ZONE_MAX_WATER][2] = {
	{ { -10000.0f, WATER_POS_VY, -245200.0f, 1.0f }, {  -6800.0f, WATER_POS_VY, -243800.0f, 1.0f }, },
	{ { -12500.0f, WATER_POS_VY, -245200.0f, 1.0f }, { -10000.0f, WATER_POS_VY, -243800.0f, 1.0f }, },
	{ { -14500.0f, WATER_POS_VY, -245200.0f, 1.0f }, { -12500.0f, WATER_POS_VY, -243800.0f, 1.0f }, },

	{ { -16000.0f, WATER_POS_VY, -245200.0f, 1.0f }, { -14500.0f, WATER_POS_VY, -243800.0f, 1.0f }, },

	{ { -16000.0f, WATER_POS_VY, -243800.0f, 1.0f }, { -14500.0f, WATER_POS_VY, -241000.0f, 1.0f }, },
	{ { -16000.0f, WATER_POS_VY, -241000.0f, 1.0f }, { -14500.0f, WATER_POS_VY, -238750.0f, 1.0f }, },
	{ { -16000.0f, WATER_POS_VY, -238750.0f, 1.0f }, { -14500.0f, WATER_POS_VY, -236500.0f, 1.0f }, },
	{ { -16000.0f, WATER_POS_VY, -236500.0f, 1.0f }, { -14500.0f, WATER_POS_VY, -234500.0f, 1.0f }, },
	
	{ { -16000.0f, WATER_POS_VY, -234500.0f, 1.0f }, { -14500.0f, WATER_POS_VY, -233000.0f, 1.0f }, },
	
	{ { -14500.0f, WATER_POS_VY, -234500.0f, 1.0f }, { -12500.0f, WATER_POS_VY, -233000.0f, 1.0f }, },
	{ { -12500.0f, WATER_POS_VY, -234500.0f, 1.0f }, { -10000.0f, WATER_POS_VY, -233000.0f, 1.0f }, },
	{ { -10000.0f, WATER_POS_VY, -234500.0f, 1.0f }, {  -7500.0f, WATER_POS_VY, -233000.0f, 1.0f }, },
	{ {  -7500.0f, WATER_POS_VY, -234500.0f, 1.0f }, {  -5500.0f, WATER_POS_VY, -233000.0f, 1.0f }, },
	
	{ {  -5500.0f, WATER_POS_VY, -234500.0f, 1.0f }, {  -4000.0f, WATER_POS_VY, -233000.0f, 1.0f }, },

	{ {  -5500.0f, WATER_POS_VY, -236500.0f, 1.0f }, {  -4000.0f, WATER_POS_VY, -234500.0f, 1.0f }, },
	{ {  -5500.0f, WATER_POS_VY, -238750.0f, 1.0f }, {  -4000.0f, WATER_POS_VY, -236500.0f, 1.0f }, },
	{ {  -5500.0f, WATER_POS_VY, -241000.0f, 1.0f }, {  -4000.0f, WATER_POS_VY, -238750.0f, 1.0f }, },
	{ {  -5500.0f, WATER_POS_VY, -243800.0f, 1.0f }, {  -4000.0f, WATER_POS_VY, -241000.0f, 1.0f }, },

	{ { -11500.0f, WATER_POS_VY, -243800.0f, 1.0f }, {  -8500.0f, WATER_POS_VY, -241000.0f, 1.0f }, },
	{ { -11500.0f, WATER_POS_VY, -241000.0f, 1.0f }, {  -8500.0f, WATER_POS_VY, -236500.0f, 1.0f }, },
	{ { -11500.0f, WATER_POS_VY, -236500.0f, 1.0f }, {  -8500.0f, WATER_POS_VY, -234500.0f, 1.0f }, },
	{ { -14500.0f, WATER_POS_VY, -241000.0f, 1.0f }, { -11500.0f, WATER_POS_VY, -236500.0f, 1.0f }, },
	{ {  -8500.0f, WATER_POS_VY, -241000.0f, 1.0f }, {  -5500.0f, WATER_POS_VY, -236500.0f, 1.0f }, },
};

static char ZoneStepTableWater[N_ZONE_MAX_WATER][N_ZONE_MAX_WATER] = {
	{ 0, 1, 2, 3,  4, 4, 4, 5,  6, 5, 4, 4,  5, 6, 5, 4,  4, 5, 1, 2,  3, 3, 3, },
	{ 1, 0, 1, 2,  3, 4, 4, 5,  6, 5, 4, 4,  5, 6, 5, 4,  4, 5, 1, 2,  3, 3, 3, },
	{ 2, 1, 0, 1,  2, 3, 4, 5,  6, 6, 5, 5,  6, 7, 6, 5,  5, 6, 2, 3,  4, 4, 4, },
	{ 3, 2, 1, 0,  1, 2, 3, 4,  5, 6, 6, 6,  7, 8, 7, 6,  6, 7, 3, 4,  5, 3, 5, },
	
	{ 4, 3, 2, 1,  0, 1, 2, 3,  4, 5, 5, 5,  6, 7, 6, 5,  5, 6, 4, 4,  4, 2, 4, },
	{ 4, 4, 3, 2,  1, 0, 1, 2,  3, 4, 4, 4,  5, 6, 5, 4,  4, 5, 3, 2,  3, 1, 3, },
	{ 4, 4, 4, 3,  2, 1, 0, 1,  2, 3, 4, 4,  5, 6, 5, 4,  4, 5, 3, 2,  3, 1, 3, },
	{ 5, 5, 5, 4,  3, 2, 1, 0,  1, 2, 3, 4,  5, 6, 6, 5,  5, 6, 4, 3,  4, 2, 4, },
	
	{ 6, 6, 6, 5,  4, 3, 2, 1,  0, 1, 2, 3,  4, 5, 6, 6,  6, 7, 5, 4,  3, 3, 5, },
	{ 5, 5, 6, 6,  5, 4, 3, 2,  1, 0, 1, 2,  3, 4, 5, 5,  5, 6, 4, 3,  2, 4, 4, },
	{ 4, 4, 5, 6,  5, 4, 4, 3,  2, 1, 0, 1,  2, 3, 4, 4,  4, 5, 3, 2,  1, 3, 3, },
	{ 4, 4, 5, 6,  5, 4, 4, 4,  3, 2, 1, 0,  1, 2, 3, 4,  4, 5, 3, 2,  1, 3, 3, },
	
	{ 5, 5, 6, 7,  6, 5, 5, 5,  4, 3, 2, 1,  0, 1, 2, 3,  4, 5, 4, 3,  2, 4, 4, },
	{ 6, 6, 7, 8,  7, 6, 6, 6,  5, 4, 3, 4,  1, 0, 1, 2,  3, 4, 5, 4,  3, 5, 3, },
	{ 5, 5, 6, 7,  6, 5, 5, 6,  6, 5, 4, 3,  2, 1, 0, 1,  2, 3, 4, 3,  4, 4, 2, },
	{ 4, 4, 5, 6,  5, 4, 4, 5,  6, 5, 4, 4,  3, 2, 1, 0,  1, 2, 3, 2,  3, 3, 1, },
	
	{ 4, 4, 5, 6,  5, 4, 4, 5,  6, 5, 4, 4,  4, 3, 2, 1,  0, 1, 3, 2,  3, 3, 1, },
	{ 5, 5, 6, 7,  6, 5, 5, 6,  7, 6, 5, 5,  5, 4, 3, 2,  1, 0, 4, 3,  4, 4, 2, },
	{ 1, 1, 2, 3,  4, 3, 3, 4,  5, 4, 3, 3,  4, 5, 4, 3,  3, 4, 0, 1,  2, 2, 2, },
	{ 2, 2, 3, 4,  4, 2, 2, 3,  4, 3, 2, 2,  3, 4, 3, 2,  2, 3, 1, 0,  1, 1, 1, },
	
	{ 3, 3, 4, 5,  4, 3, 3, 4,  3, 2, 1, 1,  2, 3, 4, 3,  3, 4, 2, 1,  0, 2, 2, },
	{ 3, 3, 4, 3,  2, 1, 1, 2,  3, 4, 3, 3,  4, 5, 4, 3,  3, 4, 2, 1,  2, 0, 2, },
	{ 3, 3, 4, 5,  4, 3, 3, 4,  5, 4, 3, 3,  4, 3, 2, 1,  1, 2, 2, 1,  2, 2, 0, },
};

static char ZoneNextTableWater[N_ZONE_MAX_WATER][4] = {
	{  1, 18, -1, -1, },
	{  0,  2, 18, -1, },
	{  1,  3, -1, -1, },
	{  2,  4, -1, -1, },
	
	{  3,  5, -1, -1, },
	{  4,  6, 21, -1, },
	{  5,  7, 21, -1, },
	{  6,  8, -1, -1, },
	
	{  7,  9, -1, -1, },
	{  8, 10, -1, -1, },
	{  9, 11, 20, -1, },
	{ 10, 12, 20, -1, },
	
	{ 11, 13, -1, -1, },
	{ 12, 14, -1, -1, },
	{ 13, 15, -1, -1, },
	{ 14, 16, 22, -1, },
	
	{ 15, 17, 22, -1, },
	{ 16, -1, -1, -1, },
	{  0,  1, 19, -1, },
	{ 18, 20, 21, 22, },
	
	{ 10, 11, 19, -1, },
	{  5,  6, 19, -1, },
	{ 15, 16, 19, -1, },
};

//　床ゾーンナンバー　→　水中ゾーンナンバー
static char VMP_DivePosNum[N_ZONE_MAX_FLOOR][4] = {	
	{ 10, 12, 10, 12, },
	{  0, 17,  0, 17, },
	{  0,  1,  0,  1, },
	{  0, 16, 17, 17, },
	{ 15, 16, 17, 16, },
	{ 14, 15, 16, 15, },
	{ 12, 14, 15, 14, },
	{ 10, 12, 10, 12, },
	{  9, 10,  9, 10, },
	{  6,  7,  9,  7, },
	{  5,  6,  7,  6, },
	{  4,  5,  6,  5, },
	{  2,  4,  5,  4, },
	{  0,  1,  2,  1, },
};

#define LOFT_POS_VY	(-1000.0f)
#define LOFT_LONG_POS_VY	(-1000.0f+500.0f)
static char VMP_LoftLineVertsNum[4] = { 2, 4, 2, 4 };

static FVECTOR VMP_LoftLineVerts0[2] = {
	{  -4750.0f, LOFT_POS_VY, -244500.0f, 1.0f, },
	{  -4750.0f, LOFT_POS_VY, -234000.0f, 1.0f, },
};

static FVECTOR VMP_LoftLineVerts1[4] = {
	{  -4750.0f, LOFT_POS_VY, -234000.0f, 1.0f, },
	{  -7200.0f, LOFT_POS_VY, -234000.0f, 1.0f, },
	{  -9800.0f, LOFT_POS_VY, -234000.0f, 1.0f, },
	{ -15350.0f, LOFT_POS_VY, -234000.0f, 1.0f, },
};

static FVECTOR VMP_LoftLineVerts2[2] = {
	{ -15350.0f, LOFT_POS_VY, -234000.0f, 1.0f, },
	{ -15350.0f, LOFT_POS_VY, -244500.0f, 1.0f, },
};

static FVECTOR VMP_LoftLineVerts3[4] = {
	{ -15350.0f, LOFT_POS_VY, -244500.0f, 1.0f, },
	{  -6800.0f, LOFT_POS_VY, -244500.0f, 1.0f, },
	{  -4750.0f, LOFT_POS_VY, -244500.0f, 1.0f, },
	{  -4750.0f, LOFT_POS_VY, -244500.0f, 1.0f, },
};

static FVECTOR *VMP_LoftLineVerts[4] = {
	VMP_LoftLineVerts0,
	VMP_LoftLineVerts1,
	VMP_LoftLineVerts2,
	VMP_LoftLineVerts3,
};


static FVECTOR VMP_LoftLineVerts1_debug[4] = {
	{  -4750.0f, LOFT_POS_VY+500.0f, -234000.0f, 1.0f, },
	{  -6800.0f, LOFT_POS_VY+500.0f, -234000.0f, 1.0f, },
	{ -10300.0f, LOFT_POS_VY+500.0f, -234000.0f, 1.0f, },
	{ -15350.0f, LOFT_POS_VY+500.0f, -234000.0f, 1.0f, },
};

static FVECTOR *VMP_LoftLineVerts_debug[4] = {
	VMP_LoftLineVerts0,
	VMP_LoftLineVerts1_debug,
	VMP_LoftLineVerts2,
	VMP_LoftLineVerts3,
};

static FVECTOR VMP_LoftLineVerts2_temp[2] = {
	{ -15350.0f, LOFT_POS_VY, -244500.0f, 1.0f, },
	{ -15350.0f, LOFT_POS_VY, -234000.0f, 1.0f, },
};
static FVECTOR *VMP_LoftLineVerts_temp[4] = {
	VMP_LoftLineVerts0,
	VMP_LoftLineVerts0,
	VMP_LoftLineVerts0,
	VMP_LoftLineVerts2_temp,
};

static FVECTOR VMP_LoftJumpOutPos[4] = {
	{  -4700.0f, LOFT_POS_VY, -239250.0f, 1.0f, },
	{ -10000.0f, LOFT_POS_VY, -233950.0f, 1.0f, },
	{ -15300.0f, LOFT_POS_VY, -239250.0f, 1.0f, },
	{ -10000.0f, LOFT_POS_VY, -244550.0f, 1.0f, },
};

static FVECTOR VMP_LoftJumpOutPos2[4] = {
	{  -9800.0f - 400.0f,	LOFT_POS_VY, -233950.0f,			1.0f, },		//右下
	{ -15300.0f,			LOFT_POS_VY, -233950.0f - 200.0f,	1.0f, },		//左下
	{ -15300.0f + 200.0f,	LOFT_POS_VY, -244550.0f,			1.0f, },		//左上
	{  -4700.0f,			LOFT_POS_VY, -244550.0f + 200.0f,	1.0f, },		//右上
};

static FVECTOR VMP_LoftAttMoveTo[4] = {
	{ -15300.0f + 1200.0f,	LOFT_POS_VY, -233950.0f,			1.0f, },		//右下
	{ -15300.0f,			LOFT_POS_VY, -244550.0f + 1200.0f,	1.0f, },		//左下
	{  -6800.0f - 1200.0f,	LOFT_POS_VY, -244550.0f,			1.0f, },		//左上
	{  -4700.0f,			LOFT_POS_VY, -234000.0f - 1200.0f,	1.0f, },		//右上
};

#define N_JOUTATT_POS		(5)
static FVECTOR VMP_JumpOutAttPos[] = {
	{ -10000.0f, WATER_POS_VY, -244500.0f, 1.0f },		//0-1
	{ -15250.0f, WATER_POS_VY, -238750.0f, 1.0f },		//5-6
	{ -11250.0f, WATER_POS_VY, -233750.0f, 1.0f },		//10
	{  -4750.0f, WATER_POS_VY, -238750.0f, 1.0f },		//15-16
	{ -10000.0f, WATER_POS_VY, -238750.0f, 1.0f },		//19
};


static FVECTOR VMP_LoftLineVerts_long0[2] = {
	{  -4700.0f, LOFT_LONG_POS_VY, -244550.0f, 1.0f, },
	{  -4700.0f, LOFT_LONG_POS_VY, -235950.0f, 1.0f, },
};

static FVECTOR VMP_LoftLineVerts_long1[2] = {
	{  -9800.0f, LOFT_LONG_POS_VY, -233950.0f, 1.0f, },
	{ -13300.0f, LOFT_LONG_POS_VY, -233950.0f, 1.0f, },
};

static FVECTOR VMP_LoftLineVerts_long2[2] = {
	{ -15300.0f, LOFT_LONG_POS_VY, -233950.0f, 1.0f, },
	{ -15300.0f, LOFT_LONG_POS_VY, -242550.0f, 1.0f, },
};

static FVECTOR VMP_LoftLineVerts_long3[2] = {
	{ -15300.0f, LOFT_LONG_POS_VY, -244550.0f, 1.0f, },
	{  -8800.0f, LOFT_LONG_POS_VY, -244550.0f, 1.0f, },
};

static FVECTOR *VMP_LoftLineVerts_long[4] = {
	VMP_LoftLineVerts_long0,
	VMP_LoftLineVerts_long1,
	VMP_LoftLineVerts_long2,
	VMP_LoftLineVerts_long3,
};


#define N_HNDRAIL_LINE		(4)
#define HANDRAIL_POS_VY		(-5200.0f)
static FVECTOR VMP_HandrailLineVerts[N_HNDRAIL_LINE][2] = {
	{ {  -7500.0f, HANDRAIL_POS_VY, -240500.0f, 1.0f, }, {  -7500.0f, HANDRAIL_POS_VY, -237000.0f, 1.0f }, },
	{ { -12500.0f, HANDRAIL_POS_VY, -240500.0f, 1.0f, }, { -12500.0f, HANDRAIL_POS_VY, -237000.0f, 1.0f }, },
	
	{ {  -5500.0f, HANDRAIL_POS_VY, -243250.0f, 1.0f, }, {  -5500.0f, HANDRAIL_POS_VY, -235000.0f, 1.0f }, },
	{ { -14500.0f, HANDRAIL_POS_VY, -243250.0f, 1.0f, }, { -14500.0f, HANDRAIL_POS_VY, -235000.0f, 1.0f }, },
};

#define N_TRIATT_POS	(2)
#define TRIATT_POS_VY0	(-4500.0f)

static FVECTOR VMP_TriAttPosVerts[N_TRIATT_POS] = {
	{  -10000.0f-100.0f, TRIATT_POS_VY0, -245250.0f+450.0f/*850.0f*/, 1.0f, },
	{  -10000.0f+100.0f, TRIATT_POS_VY0, -233250.0f-450.0f/*850.0f*/, 1.0f, },
};

static FVECTOR VMP_TriAttFloorZoneVerts[N_TRIATT_POS][2] = {
	{ { -11000.0f, ZONE_VY, -243700.0f, 1.0f, }, { -9000.0f, ZONE_VY, -241100.0f, 1.0f, }, },
	{ { -11000.0f, ZONE_VY, -236400.0f, 1.0f, }, { -9000.0f, ZONE_VY, -234500.0f, 1.0f, }, },
};
//デバッグ用ゾーン表示
void VMP_DebugZoneView( int p, int v, int dive_pos, int jumpout_pos )
{
	extern void *AN_Test_Eye2( FVECTOR*, int );
	int		i,j;
	DG_SetPos( &DG_Chanls[0].eye_pers );
#if 0
	for( i = 0; i < N_HNDRAIL_LINE; i++ ){
		NewLineView( &VMP_HandrailLineVerts[i][0], 1, 160, 32, 140 );
	}
#endif

#if 0
	for( i = 0; i < N_TRIATT_POS; i++ ){
		AN_Test_Eye2( &VMP_TriAttPosVerts[i], 2 );
		NewBoundingBoxView( &VMP_TriAttFloorZoneVerts[i][0], &VMP_TriAttFloorZoneVerts[i][1], 128, 64, 64 );
	}
#endif
	for( i = 0; i < N_JOUTATT_POS; i++ ){
		//AN_Test_Eye2( &VMP_JumpOutAttPos[i], 2 );
	}
#if 1
	for( i = 0; i < 4; i++ ){
		for( j = 0; j < VMP_LoftLineVertsNum[i]/2; j++ ){
			NewLineView( &VMP_LoftLineVerts_debug[i][j*2], 1, 160, 32, 140 );

			//AN_Test_Eye2( &VMP_LoftLineVerts[i][j*2+0], 2 );
			//AN_Test_Eye2( &VMP_LoftLineVerts[i][j*2+1], 2 );

		}
		AN_Test_Eye2( &VMP_LoftJumpOutPos2[i], 2 );
	}
	
	for( i = 0; i < 4; i++ ){
		for( j = 0; j < VMP_LoftLineVertsNum[i]/2; j++ ){
			NewLineView( &VMP_LoftLineVerts[i][j*2], 1, 160, 160, 140 );

			//AN_Test_Eye2( &VMP_LoftLineVerts[i][j*2+0], 2 );
			//AN_Test_Eye2( &VMP_LoftLineVerts[i][j*2+1], 2 );

		}
	}
	
#endif
#if 0
	for( i = 0; i < 4; i++ ){
		NewLineView( &VMP_LoftLineVerts_long[i][0], 1, 160, 32, 140 );
		//AN_Test_Eye2( &VMP_LoftLineVerts_long[i][0], 2 );
		//AN_Test_Eye2( &VMP_LoftLineVerts_long[i][1], 2 );
	}
#endif

#if 0
	for( i = 0; i < N_ZONE_MAX_WATER; i++ ){
		FVECTOR	center;
		int		r = 128, g = 128, b = 128;

		_sceVu0SubVector( &center, &VmpZoneVertsWater[i][1], &VmpZoneVertsWater[i][0] );
		_sceVu0ScaleVector( &center, &center, 0.5f );
		_sceVu0AddVector( &center, &center, &VmpZoneVertsWater[i][0] );
		DG_PutVector( &center, &center, 1 );
		if( center.vz < center.vw ){
			center.vx /= center.vw;
			center.vy /= center.vw;
			if( fabs(center.vx) < 1.0f && fabs(center.vy) < 1.0f ){
				int lx,ly;
				if( dive_pos == i ) DEBUG_SetColor( 160, 160, 100 );
				else DEBUG_SetColor( 32, 160, 100 );
				lx = (int)(center.vx * (float)(DRAW_WIDTH>>1)) + (DRAW_WIDTH>>1);
				ly = (int)(center.vy * (float)(DRAW_HEIGHT>>1)) + (DRAW_HEIGHT>>1);
				DEBUG_Locate( lx, ly, 0 );
				DEBUG_Printf( "%d", i );
			}
		}

		NewBoundingBoxView( &VmpZoneVertsWater[i][0], &VmpZoneVertsWater[i][1], r, g, b );
	}
	DEBUG_ResetColor();
#endif

#if 1
	for( i = 0; i < N_ZONE_MAX_FLOOR; i++ ){
		FVECTOR	center;
		int		r = 32, g = 32, b = 32;
		_sceVu0SubVector( &center, &VmpZoneVertsFloor[i][1], &VmpZoneVertsFloor[i][0] );
		_sceVu0ScaleVector( &center, &center, 0.5f );
		_sceVu0AddVector( &center, &center, &VmpZoneVertsFloor[i][0] );
		//AN_Test_Eye2( &center, 2 );
		DG_PutVector( &center, &center, 1 );
		if( center.vz < center.vw ){
			center.vx /= center.vw;
			center.vy /= center.vw;
			if( fabs(center.vx) < 1.0f && fabs(center.vy) < 1.0f ){
				int lx,ly;
				lx = (int)(center.vx * (float)(DRAW_WIDTH>>1)) + (DRAW_WIDTH>>1);
				ly = (int)(center.vy * (float)(DRAW_HEIGHT>>1)) + (DRAW_HEIGHT>>1);
				DEBUG_Locate( lx, ly, 0 );
				DEBUG_Printf( "%d", i );
				if( i == jumpout_pos ) DEBUG_Printf( "j" );
			}
		}
#if 0
		if( p == i ) r=255;
		if( v == i ) b=255;

		if( ZoneNextTableFloor[p][0] == i ) g+=64;
		if( ZoneNextTableFloor[p][1] == i ) g+=64;
		if( ZoneNextTableFloor[p][2] == i ) g+=64;
		if( ZoneNextTableFloor[p][3] == i ) g+=64;

		if( ZoneNextTableFloor[v][0] == i ) g+=64;
		if( ZoneNextTableFloor[v][1] == i ) g+=64;
		if( ZoneNextTableFloor[v][2] == i ) g+=64;
		if( ZoneNextTableFloor[v][3] == i ) g+=64;
#else
		switch(ZoneToZoneInfo[p][i]){
		  case ZONE_INFO_SAME:
			r = 128;
			g = 128;
			b = 128;
			break;
		  case ZONE_INFO_NEXT:
			r = 255;
			g = 128;
			b = 128;
			break;
		  case ZONE_INFO_NEAR:
			r = 255;
			g = 255;
			b = 128;
			break;
		  case ZONE_INFO_LINE:
			r = 128;
			g = 255;
			b = 255;
			break;
		  case ZONE_INFO_TOIMEN:
			r = 128;
			g = 255;
			b = 128;
			break;
		  case ZONE_INFO_CONTRAST:
			r = 128;
			g = 128;
			b = 255;
			break;
		  case ZONE_INFO_FAR:
			r = 0;
			g = 0;
			b = 0;
			break;
		}
#endif
		NewBoundingBoxView( &VmpZoneVertsFloor[i][0], &VmpZoneVertsFloor[i][1], r, g, b );
	}
#endif
};

static float VampZoneLen( FVECTOR *z_verts, FVECTOR *pos ) {
	float ax, az ;

	if ( z_verts[0].vx > pos->vx ) {
		ax = z_verts[0].vx - pos->vx ;
	} else if ( z_verts[1].vx < pos->vx ) {
		ax = pos->vx - z_verts[1].vx ;
	} else {
		ax = 0.0 ;
	}

	if ( z_verts[0].vz > pos->vz ) {
		az = z_verts[0].vz - pos->vz ;
	} else if ( z_verts[1].vz < pos->vz ) {
		az = pos->vz - z_verts[1].vz ;
	} else {
		az = 0.0 ;
	}

	ax *= ax ;
	az *= az ;

	return ax+az ;
}

//　ポイントからゾーンナンバーを求める
int VMP_GetZoneNumFromPosFloor( FVECTOR *pos, int pre_num )
{
	int 		z_num = 0;
	int			i;
	float		len = FLOAT_MAX;
	FVECTOR		*z_verts = VmpZoneVertsFloor[pre_num];
//	FVECTOR		*z_verts_nxt0;
//	FVECTOR		*z_verts_nxt1;
//	FVECTOR		*z_verts_nxt2;

	//前いた所と隣を優先的に調べると・・・・
	//結果：余り変わらずでもちょっとは速くなったであろう・・・

	if( pre_num >= 0 && (z_verts[0].vx < pos->vx && pos->vx < z_verts[1].vx) &&
		(z_verts[0].vz < pos->vz && pos->vz < z_verts[1].vz) ){
		return pre_num;
	}

	z_verts = VmpZoneVertsFloor[0];

	for( i = 0; i < N_ZONE_MAX_FLOOR; i++ ){
		if( (z_verts[0].vx < pos->vx && pos->vx < z_verts[1].vx) &&
			(z_verts[0].vz < pos->vz && pos->vz < z_verts[1].vz) ){
			return i;
		}
		z_verts += 2;
	}

	z_verts = VmpZoneVertsFloor[0];

	for( i = 0; i < N_ZONE_MAX_FLOOR; i++ ){
		float	l;
//#ifdef PSX2
#if 0
		asm volatile ("
		   lqc2			vf4, 0x00(%1)
		   lqc2			vf5, 0x10(%1)
		   lqc2			vf6, 0x00(%2)
		   qmtc2.ni		%3,  vf7

		   vsub.xz		vf8,  vf5,  vf4
		   vmulx.xz		vf9,  vf8,  vf7
		   vadd.xz		vf10, vf4,  vf9
		   vsub.xz		vf11, vf6,  vf10

		   vmul.xz		vf8,  vf11, vf11
		   vmulax.w		ACC,  vf0,  vf8x
		   vmaddz.w		vf9,  vf0,  vf8z
		   vaddw.x		vf10, vf0,  vf9w
		   qmfc2.i		$8,vf10
		   sw			$8,0(%0)
		": : "r"(&l), "r"(z_verts), "r"(pos), "r"(0.5f) : "$8", "memory" );
#else
		l = VampZoneLen( z_verts, pos ) ;
#endif

		if( l < len ){
			len = l;
			z_num = i;
		}
		z_verts += 2;
	}
	return z_num | 0x8000;
}
//　ポイントからゾーンナンバーを求める
int VMP_GetZoneNumFromPosWater( FVECTOR *pos, int pre_num )
{
	int 		z_num = 0;
	int			i;
	float		len = FLOAT_MAX;
	FVECTOR		*z_verts = VmpZoneVertsWater[pre_num];

	//前いた所と隣を優先的に調べると・・・・
	//結果：余り変わらずでもちょっとは速くなったであろう・・・

	if( pre_num >= 0 && (z_verts[0].vx < pos->vx && pos->vx < z_verts[1].vx) &&
		(z_verts[0].vz < pos->vz && pos->vz < z_verts[1].vz) ){
		return pre_num;
	}

	z_verts = VmpZoneVertsWater[0];

	for( i = 0; i < N_ZONE_MAX_WATER; i++ ){
		if( (z_verts[0].vx < pos->vx && pos->vx < z_verts[1].vx) &&
			(z_verts[0].vz < pos->vz && pos->vz < z_verts[1].vz) ){
			return i;
		}
		z_verts += 2;
	}

	z_verts = VmpZoneVertsWater[0];

	for( i = 0; i < N_ZONE_MAX_WATER; i++ ){
		float	l;
//#ifdef PSX2
#if 0
		asm volatile ("
		   lqc2			vf4, 0x00(%1)
		   lqc2			vf5, 0x10(%1)
		   lqc2			vf6, 0x00(%2)
		   qmtc2.ni		%3,  vf7

		   vsub.xz		vf8,  vf5,  vf4
		   vmulx.xz		vf9,  vf8,  vf7
		   vadd.xz		vf10, vf4,  vf9
		   vsub.xz		vf11, vf6,  vf10

		   vmul.xz		vf8,  vf11, vf11
		   vmulax.w		ACC,  vf0,  vf8x
		   vmaddz.w		vf9,  vf0,  vf8z
		   vaddw.x		vf10, vf0,  vf9w
		   qmfc2.i		$8,vf10
		   sw			$8,0(%0)
		": : "r"(&l), "r"(z_verts), "r"(pos), "r"(0.5f) : "$8", "memory" );
#else
		l = VampZoneLen( z_verts, pos ) ;
#endif

		if( l < len ){
			len = l;
			z_num = i;
		}
		z_verts += 2;
	}
	return z_num;
}

//飛び出る先のゾーンナンバーから着地ポイントを求める（ちょっとランド）
void VMP_JumpOutPos( FVECTOR *out, int a )
{
	FVECTOR	fvtemp;
	_sceVu0SubVector( &fvtemp, &VmpZoneVertsFloor[a][1], &VmpZoneVertsFloor[a][0] );
	fvtemp.vx *= 0.5f;//(0.25f + rnd()*0.5f);
	fvtemp.vy = 1000.0f;
	fvtemp.vz *= 0.5f;//(0.25f + rnd()*0.5f);
	fvtemp.vw = 0.0f;
	_sceVu0AddVector( out, &fvtemp, &VmpZoneVertsFloor[a][0] );
}

//　ａから見たｂのゾーンの種類
int VMP_GetZoneInfo( int a, int b )
{
	return (int)ZoneToZoneInfo[a][b];
}

// aから見たinfoと同じゾーン情報を持つゾーンナンバーを返す（複数の時はランダム）
int VMP_GetRndZone( int a, int info )
{
	char	temp[N_ZONE_MAX_FLOOR] = "";
	int		i,num = 0;
	for( i = 2; i < N_ZONE_MAX_FLOOR; i++ ){
		if( ZoneToZoneInfo[a][i] == info ){
			temp[num] = i;
			num++;
		}
	}
	return (int)temp[(irnd()>>12)%num];
}

// aから見たinfoと同じゾーン情報を持つゾーンナンバーを返す（複数の時はランダム）
int VMP_GetRndZoneNoSame( int a, int info, int now )
{
	char	temp[N_ZONE_MAX_FLOOR] = "";
	int		i,num = 0;
	for( i = 2; i < N_ZONE_MAX_FLOOR; i++ ){
		if( ZoneToZoneInfo[a][i] == info && i != now ){
			temp[num] = i;
			num++;
		}
	}
	return (int)temp[(irnd()>>12)%num];
}

//vからfar 以上離れてpから見たinfo以上の場所（複数の時はランダム）
// yano /*'far'->'far_Y'に変更 */
int VMP_GetRndZoneFar( int p, int v, int info, int far_Y )
{
	char	temp[N_ZONE_MAX_FLOOR] = "";
	int		i,num = 0;
	for( i = 2; i < N_ZONE_MAX_FLOOR; i++ ){
		if( ZoneToZoneInfo[p][i] >= info &&
			ZoneStepTableFloor[v][i] >= 2 ){
			temp[num] = i;
			num++;
		}
	}
	return (int)temp[(irnd()>>12)%num];
}

int VMP_GetZoneLen( int a, int b )
{
	return ZoneStepTableFloor[a][b];
}


int VMP_GetZoneRndFromLen( int p, int len, int v )
{
	char	temp0[N_ZONE_MAX_FLOOR] = "";
	char	temp1[N_ZONE_MAX_FLOOR] = "";
	int		i,num0 = 0,num1 = 0;

	if( len < 2 ) len = 2;
	for( i = 2; i < N_ZONE_MAX_FLOOR; i++ ){
		if( ZoneStepTableFloor[p][i] == len ){
			temp1[num1] = i;
			num1++;
			if( v != i &&
				ZoneNextTableFloor[v][0] != i &&
				ZoneNextTableFloor[v][1] != i &&
				ZoneNextTableFloor[v][2] != i &&
				ZoneNextTableFloor[v][3] != i ){
				temp0[num0] = i;
				num0++;
			}
		}
	}
	if( num0 ) return (int)temp0[(irnd()>>12)%num0];
	if( !num1 ) VMP_PRINTF("ERR!! p[%d] v[%d] len[%d]\n",p,v,len);
	return (int)temp1[(irnd()>>12)%num1];
}


//　飛び込む場所（ゾーンナンバー、現在、床）から飛び込む場所（水、水中ゾーンナンバー）を求める
int VMP_GetDivePosNum( int a )
{
	int index = (irnd()>>16)&3;
	return	VMP_DivePosNum[a][index];
}

//　水中ゾーンナンバーからポイントを求める
void VMP_DivePos( FVECTOR *out, int a )
{
	FVECTOR fvtemp;
	_sceVu0SubVector( &fvtemp, &VmpZoneVertsWater[a][1], &VmpZoneVertsWater[a][0] );
	_sceVu0ScaleVector( &fvtemp, &fvtemp, 0.5f );
	_sceVu0AddVector( out, &fvtemp, &VmpZoneVertsWater[a][0] );
}

//　多分次行くゾーンであるだろうというゾーンナンバーを返す
int VMP_RaiMayBeNextZone( int now, int pre, int flag )
{
	int		a;
	if( flag ){
		return now;
	}
	if( now < 2 ){
		return (int)ZoneNextTableFloor[now][0];
	}else{
		a = (int)ZoneNextTableFloor[now][0];
		if( a == pre ) a = (int)ZoneNextTableFloor[now][1];
	}
	return a;
}

int VMP_GetNextIndexZone( int a, int index )
{
	return ZoneNextTableFloor[a][index];
}

static int VMP_GetNextNearZoneFloor( int from, int to )
{
	int i;
	int	next_index = 0;
	int	min_step = 0x7fffffff;

	for( i = 0; i < 4; i++ ){
		if( ZoneNextTableFloor[from][i] < 0 ) break;
		if( min_step > ZoneStepTableFloor[ (int)ZoneNextTableFloor[from][i] ][to] ){
			next_index = ZoneNextTableFloor[from][i];
			min_step = ZoneStepTableFloor[ (int)ZoneNextTableFloor[from][i] ][to];
		}
		//VMP_PRINTF("%4d: %4d\n",(int)ZoneNextTableFloor[from][i],ZoneStepTableFloor[ (int)ZoneNextTableFloor[from][i] ][to] );
	}
	return next_index;
}

//ｆｒｏｍからｔｏへ行く最短ゾーンに進むｄｉｒを返す
//-1の時もういい ０から４０９５？
int VMP_GetFromZoneToZoneDirFloor( FVECTOR *from_vec, int from, int to, float len, FVECTOR *to_pos )
{
	static FVECTOR to1pos = { -6100.0f, -5000.0f, -243900.0f, 1.0f };
	static FVECTOR from1pos = { -6100.0f, -5000.0f, -243500.0f, 1.0f };
	int			last_num = ZoneStepTableFloor[ from ][ to ];
	FVECTOR		to_vec;
	int			next_index = 0,dir = -1;
	SVECTOR		rot;

	//DG_COPY_VEC( &to_vec, &GM_PlayerPosition );

	switch( last_num ){
	  case 0:
		if( to_pos ){
			DG_COPY_VEC( &to_vec, to_pos );
		}else{
			_Vu0GetCenterVec2( &to_vec, &VmpZoneVertsFloor[from][0], &VmpZoneVertsFloor[from][1] );
		}
		//AN_Test_Eye2( &to_vec, 2 );
		_sceVu0SubVector( &to_vec, &to_vec, from_vec );

		if( len > _Vu0VecLenXZ( &to_vec ) ){
			return -1;
		}
		//_sceVu0Normalize( &to_pos, &to_pos );
		TS_VecToRot( &rot, &to_vec );
		dir = rot.vy;
		if( dir < 0 ) dir += 4096;
		break;
	  case 1:
		if( to == 1 ){
			DG_COPY_VEC( &to_vec, &to1pos );
		}else if( from == 1 ){
			DG_COPY_VEC( &to_vec, &from1pos );
		}else{
			//if(to_pos){
			//	DG_COPY_VEC( &to_vec, to_pos );
			//}else{
				next_index = VMP_GetNextNearZoneFloor( from, to );
				_Vu0GetCenterVec2( &to_vec, &VmpZoneVertsFloor[next_index][0], &VmpZoneVertsFloor[next_index][1] );
			//}
		}
		//AN_Test_Eye2( &to_vec, 2 );
		
		_sceVu0SubVector( &to_vec, &to_vec, from_vec );
		//_sceVu0Normalize( &to_pos, &to_pos );
		TS_VecToRot( &rot, &to_vec );
		dir = rot.vy;
		if( dir < 0 ) dir += 4096;
		break;
	  default:
		next_index = VMP_GetNextNearZoneFloor( from, to );

		_Vu0GetCenterVec2( &to_vec, &VmpZoneVertsFloor[next_index][0], &VmpZoneVertsFloor[next_index][1] );
		//AN_Test_Eye2( &to_vec, 2 );
		_sceVu0SubVector( &to_vec, &to_vec, from_vec );
		
		TS_VecToRot( &rot, &to_vec );
		dir = rot.vy;
		if( dir < 0 ) dir += 4096;
		break;
	}
	//VMP_PRINTF( "f %2d: t %2d:last %2d: next_index %d: dir %5d\n", from, to, last_num, next_index, dir );
	return dir;
}


static int VMP_GetNextNearZoneWater( int from, int to, int pre )
{
	int i;
	int	next_index = -1,temp = 0;
	int	min_step = 0x7fffffff;

	for( i = 0; i < 4; i++ ){
		if( ZoneNextTableWater[from][i] < 0 ) break;
		if( ZoneNextTableWater[from][i] == pre ){
			temp = ZoneNextTableWater[from][i];
			continue;
		}
		if( min_step > ZoneStepTableWater[ (int)ZoneNextTableWater[from][i] ][to] ){
			next_index = ZoneNextTableWater[from][i];
			min_step = ZoneStepTableWater[ (int)ZoneNextTableWater[from][i] ][to];
		}
		//VMP_PRINTF("%4d: %4d\n",(int)ZoneNextTableWater[from][i],ZoneStepTableWater[ (int)ZoneNextTableWater[from][i] ][to] );
	}
	if( next_index < 0 ) next_index = temp;
	return next_index;
}

//ｆｒｏｍからｔｏへ行く最短ゾーンに進むｄｉｒを返す
//-1の時もういい ０から４０９５？
int VMP_GetFromZoneToZoneDirWater( FVECTOR *from_vec, int from, int to, float len, int pre )
{
	int			last_num = ZoneStepTableWater[ from ][ to ];
	FVECTOR		to_vec;
	int			next_index = 0,dir = -1;
	SVECTOR		rot;

	//DG_COPY_VEC( &to_vec, &GM_PlayerPosition );
	switch( last_num ){
	  case 0:
		_Vu0GetCenterVec2( &to_vec, &VmpZoneVertsWater[from][0], &VmpZoneVertsWater[from][1] );
		_sceVu0SubVector( &to_vec, &to_vec, from_vec );
		//AN_Test_Eye2( &to_vec, 2 );
		if( len > _Vu0VecLenXZ( &to_vec ) ){
			return -1;
		}
		TS_VecToRot( &rot, &to_vec );
		dir = rot.vy;
		if( dir < 0 ) dir += 4096;
		break;
	  case 1:
		next_index = VMP_GetNextNearZoneWater( from, to, pre );
		_Vu0GetCenterVec2( &to_vec, &VmpZoneVertsWater[next_index][0], &VmpZoneVertsWater[next_index][1] );

		//AN_Test_Eye2( &to_vec, 2 );
		
		_sceVu0SubVector( &to_vec, &to_vec, from_vec );
		TS_VecToRot( &rot, &to_vec );
		dir = rot.vy;
		if( dir < 0 ) dir += 4096;
		break;
	  default:
		next_index = VMP_GetNextNearZoneWater( from, to, pre );

		_Vu0GetCenterVec2( &to_vec, &VmpZoneVertsWater[next_index][0], &VmpZoneVertsWater[next_index][1] );
		//AN_Test_Eye2( &to_vec, 2 );
		_sceVu0SubVector( &to_vec, &to_vec, from_vec );
		
		TS_VecToRot( &rot, &to_vec );
		dir = rot.vy;
		if( dir < 0 ) dir += 4096;
		break;
	}

	//VMP_PRINTF( "f %2d: t %2d:last %2d: next_index %d: dir %5d\n", from, to, last_num, next_index, dir );
	return dir;
}
//フロアーのゾーンから水ゾーンを求める
int VMP_GetNearWaterZoneFromFloor( int a, int pre )
{
	FVECTOR	center;

	_sceVu0SubVector( &center, &VmpZoneVertsFloor[a][1], &VmpZoneVertsFloor[a][0] );
	_sceVu0ScaleVector( &center, &center, 0.5f );
	_sceVu0AddVector( &center, &center, &VmpZoneVertsFloor[a][0] );

	return VMP_GetZoneNumFromPosWater( &center, pre );
}

int VMP_GetLoftJumpOutPos( FVECTOR *out, FVECTOR *pos, int wzn )
{
	int ret = 0;
	switch( wzn ){
	  case 21:
		ret = 0;
		DG_COPY_VEC( out, &VMP_LoftJumpOutPos[0] );
		break;
	  case 18:
		ret = 2;
		DG_COPY_VEC( out, &VMP_LoftJumpOutPos[1] );
		break;
	  case 22:
		ret = 3;
		DG_COPY_VEC( out, &VMP_LoftJumpOutPos[2] );
		break;
	  case 20:
		ret = 4;
		DG_COPY_VEC( out, &VMP_LoftJumpOutPos[3] );
		break;
	  default:
		VMP_PRINTF( "ERR!! WrongWzn[%2d]\n", wzn );
	}
	return ret;
}


int VMP_GetLoftJumpOutPos2( FVECTOR *out, FVECTOR *pos, int wzn )
{
	int ret = 0;
	switch( wzn ){
	  case 21:
		ret = 0;
		DG_COPY_VEC( out, &VMP_LoftJumpOutPos2[3] );
		break;
	  case 18:
		ret = 2;
		DG_COPY_VEC( out, &VMP_LoftJumpOutPos2[0] );
		break;
	  case 22:
		ret = 3;
		DG_COPY_VEC( out, &VMP_LoftJumpOutPos2[1] );
		break;
	  case 20:
		ret = 4;
		DG_COPY_VEC( out, &VMP_LoftJumpOutPos2[2] );
		break;
	  default:
		VMP_PRINTF( "ERR!! WrongWzn[%2d]\n", wzn );
	}
	return ret;
}

float VMP_GetSpinScale( FVECTOR *pos, int num )
{
	float ret = 0.0f;
	FVECTOR fvtemp;

	switch( num ){
	  case 0:
		DG_COPY_VEC( &fvtemp, &VMP_LoftAttMoveTo[3] );
		break;
	  case 2:
		DG_COPY_VEC( &fvtemp, &VMP_LoftAttMoveTo[0] );
		break;
	  case 3:
		DG_COPY_VEC( &fvtemp, &VMP_LoftAttMoveTo[1] );
		break;
	  case 4:
		DG_COPY_VEC( &fvtemp, &VMP_LoftAttMoveTo[2] );
		break;
	  default:
		VMP_PRINTF( "ERR!! Wrong num[%2d]\n", num );
		DG_COPY_VEC( &fvtemp, pos );
	}
	_sceVu0SubVector( &fvtemp, &fvtemp, pos );
	ret = _Vu0VecLenXZ( &fvtemp );

	return ret;
}

int VMP_GetLoftDir( short *pdir, short *pv, FVECTOR *pos, int mode )
{
	short			v = *pv;
	short			dir;
	int				ret = -1;
	FVECTOR			fvtemp,*check0,*check1;
	float			tmp = pos->vy;
	
	switch( v ){
	  case 0:
		check0 = &VMP_LoftLineVerts[0][0];
		check1 = &VMP_LoftLineVerts[0][1];
		if( check0->vz > pos->vz ){
			DG_COPY_VEC( pos, check0 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[3][0] );
			v = 5;
		}else if( check1->vz < pos->vz ){
			DG_COPY_VEC( pos, check1 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][3] );
			v = 1;
		}else{
			if( mode ){
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[0][0] );
			}else{
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[0][VMP_LoftLineVertsNum[0]-1] );
			}
		}
		pos->vx = VMP_LoftLineVerts[0][0].vx;
		break;
	  case 1:
		check0 = &VMP_LoftLineVerts[1][1];
		check1 = &VMP_LoftLineVerts[1][0];
		if( check0->vx > pos->vx ){
			DG_COPY_VEC( pos, check0 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][3] );
			v = 2;
			ret = 2;
		}else if( check1->vx < pos->vx ){
			DG_COPY_VEC( pos, check1 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[0][0] );
			v = 0;
		}else{
			if( mode ){
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][0] );
			}else{
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][VMP_LoftLineVertsNum[1]-1] );
			}
		}
		pos->vz = VMP_LoftLineVerts[1][0].vz;
		break;
	  case 2:
		check0 = &VMP_LoftLineVerts[1][3];
		check1 = &VMP_LoftLineVerts[1][2];
		if( check0->vx > pos->vx ){
			DG_COPY_VEC( pos, check0 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[2][1] );
			v = 3;
			//ret = 1;
		}else if( check1->vx < pos->vx ){
			DG_COPY_VEC( pos, check1 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[0][0] );
			v = 1;
			ret = 1;
		}else{
			if( mode ){
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][0] );
			}else{
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][VMP_LoftLineVertsNum[1]-1] );
			}
		}
		pos->vz = VMP_LoftLineVerts[1][0].vz;
		break;
	  case 3:
		check0 = &VMP_LoftLineVerts[2][1];
		check1 = &VMP_LoftLineVerts[2][0];
		if( check0->vz > pos->vz ){
			DG_COPY_VEC( pos, check0 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[3][3] );
			v = 4;
		}else if( check1->vz < pos->vz ){
			DG_COPY_VEC( pos, check1 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][0] );
			v = 2;
		}else{
			if( mode ){
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[2][0] );
			}else{
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[2][VMP_LoftLineVertsNum[2]-1] );
			}
		}
		pos->vx = VMP_LoftLineVerts[2][0].vx;
		break;
	  case 4:
		check0 = &VMP_LoftLineVerts[3][0];
		check1 = &VMP_LoftLineVerts[3][1];
		if( check0->vx > pos->vx ){
			DG_COPY_VEC( pos, check0 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[2][0] );
			v = 3;
		}else if( check1->vx < pos->vx ){
			DG_COPY_VEC( pos, check1 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[3][3] );
			v = 5;
			ret = 5;
		}else{
			if( mode ){
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[3][0] );
			}else{
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[3][VMP_LoftLineVertsNum[3]-1] );
			}
		}
		pos->vz = VMP_LoftLineVerts[3][0].vz;
		break;
	  case 5:
		DG_COPY_VEC( pos, &VMP_LoftLineVerts[0][0] );
		if(mode){
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[3][0] );
			v = 4;
			ret = 4;
		}else{
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[0][1] );
			v = 0;
		}
		pos->vx = VMP_LoftLineVerts[0][0].vx;
		break;
	  default:
		VMP_PRINTF( "ERR!! LOFTLINE ID[%d]\n", v );
	}

	*pv = v;
	_sceVu0SubVector( &fvtemp, &fvtemp, pos );
	dir = (int)(2048.0f * atan2f( fvtemp.vx , fvtemp.vz ) / PI);
	if( dir < 0 ) dir += 4096;
	*pdir = dir;

	pos->vy = tmp;
	return ret;
}

int VMP_GetLoftDirNoChangeLoftNum( short *pdir, short *pv, FVECTOR *pos, int mode )
{
	short			v = *pv;
	short			dir;
	int				ret = -1;
	FVECTOR			fvtemp,*check0,*check1;
	float			tmp = pos->vy;
	
	switch( v ){
	  case 0:
		check0 = &VMP_LoftLineVerts[0][0];
		check1 = &VMP_LoftLineVerts[0][1];
		if( check0->vz > pos->vz ){
			DG_COPY_VEC( pos, check0 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[3][0] );
		}else if( check1->vz < pos->vz ){
			DG_COPY_VEC( pos, check1 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][3] );
		}else{
			if( mode ){
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[0][0] );
			}else{
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[0][VMP_LoftLineVertsNum[0]-1] );
			}
		}
		pos->vx = VMP_LoftLineVerts[0][0].vx;
		break;
	  case 1:
		check0 = &VMP_LoftLineVerts[1][1];
		check1 = &VMP_LoftLineVerts[1][0];
		if( check0->vx > pos->vx ){
			DG_COPY_VEC( pos, check0 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][3] );
			v = 2;
			ret = 2;
		}else if( check1->vx < pos->vx ){
			DG_COPY_VEC( pos, check1 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[0][0] );
		}else{
			if( mode ){
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][0] );
			}else{
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][VMP_LoftLineVertsNum[1]-1] );
			}
		}
		pos->vz = VMP_LoftLineVerts[1][0].vz;
		break;
	  case 2:
		check0 = &VMP_LoftLineVerts[1][3];
		check1 = &VMP_LoftLineVerts[1][2];
		if( check0->vx > pos->vx ){
			DG_COPY_VEC( pos, check0 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[2][1] );
		}else if( check1->vx < pos->vx ){
			DG_COPY_VEC( pos, check1 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[0][0] );
			v = 1;
			ret = 1;
		}else{
			if( mode ){
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][0] );
			}else{
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][VMP_LoftLineVertsNum[1]-1] );
			}
		}
		pos->vz = VMP_LoftLineVerts[1][0].vz;
		break;
	  case 3:
		check0 = &VMP_LoftLineVerts[2][1];
		check1 = &VMP_LoftLineVerts[2][0];
		if( check0->vz > pos->vz ){
			DG_COPY_VEC( pos, check0 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[3][3] );
		}else if( check1->vz < pos->vz ){
			DG_COPY_VEC( pos, check1 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[1][0] );
		}else{
			if( mode ){
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[2][0] );
			}else{
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[2][VMP_LoftLineVertsNum[2]-1] );
			}
		}
		pos->vx = VMP_LoftLineVerts[2][0].vx;
		break;
	  case 4:
		check0 = &VMP_LoftLineVerts[3][0];
		check1 = &VMP_LoftLineVerts[3][1];
		if( check0->vx > pos->vx ){
			DG_COPY_VEC( pos, check0 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[2][0] );
		}else if( check1->vx < pos->vx ){
			DG_COPY_VEC( pos, check1 );
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[3][3] );
			v = 5;
			ret = 5;
		}else{
			if( mode ){
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[3][0] );
			}else{
				DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[3][VMP_LoftLineVertsNum[3]-1] );
			}
		}
		pos->vz = VMP_LoftLineVerts[3][0].vz;
		break;
	  case 5:
		DG_COPY_VEC( pos, &VMP_LoftLineVerts[0][0] );
		if(mode){
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[3][0] );
			v = 4;
			ret = 4;
		}else{
			DG_COPY_VEC( &fvtemp, &VMP_LoftLineVerts[0][1] );
		}
		pos->vx = VMP_LoftLineVerts[0][0].vx;
		break;
	  default:
		VMP_PRINTF( "ERR!! LOFTLINE ID[%d]\n", v );
	}

	*pv = v;
	_sceVu0SubVector( &fvtemp, &fvtemp, pos );
	dir = (int)(2048.0f * atan2f( fvtemp.vx , fvtemp.vz ) / PI);
	if( dir < 0 ) dir += 4096;
	*pdir = dir;

	pos->vy = tmp;
	return ret;
}

void VMP_GetLoftFixPos( short v, FVECTOR *pos  )
{
	switch( v ){
	  case 0:
	  case 5:
		pos->vx = VMP_LoftLineVerts[0][0].vx;
		break;
	  case 1:
	  case 2:
		pos->vz = VMP_LoftLineVerts[1][0].vz;
		break;
	  case 3:
		pos->vx = VMP_LoftLineVerts[2][0].vx;
		break;
	  case 4:
		pos->vz = VMP_LoftLineVerts[3][0].vz;
		break;
	  default:
		VMP_PRINTF( "ERR!! LOFTLINE ID[%d]\n", v );
	}
}

void VMP_GetLoftFixPosEx( short v, FVECTOR *pos  )
{
	switch( v ){
	  case 0:
		pos->vx = VMP_LoftLineVerts[0][0].vx;
		if( pos->vz < VMP_LoftLineVerts[0][0].vz ) pos->vz = VMP_LoftLineVerts[0][0].vz;
		else if( pos->vz > VMP_LoftLineVerts[0][1].vz ) pos->vz = VMP_LoftLineVerts[0][1].vz;
		break;
	  case 1:		
		pos->vz = VMP_LoftLineVerts[1][0].vz;
		if( pos->vx < VMP_LoftLineVerts[1][1].vx ) pos->vx = VMP_LoftLineVerts[1][1].vx;
		else if( pos->vx > VMP_LoftLineVerts[1][0].vx ) pos->vx = VMP_LoftLineVerts[1][0].vx;
		break;
	  case 2:
		pos->vz = VMP_LoftLineVerts[1][0].vz;
		if( pos->vx < VMP_LoftLineVerts[1][3].vx ) pos->vx = VMP_LoftLineVerts[1][3].vx;
		else if( pos->vx > VMP_LoftLineVerts[1][2].vx ) pos->vx = VMP_LoftLineVerts[1][2].vx;
		break;
	  case 3:
		pos->vx = VMP_LoftLineVerts[2][0].vx;
		if( pos->vz < VMP_LoftLineVerts[2][1].vz ) pos->vz = VMP_LoftLineVerts[2][1].vz;
		else if( pos->vz > VMP_LoftLineVerts[2][0].vz ) pos->vz = VMP_LoftLineVerts[2][0].vz;
		break;
	  case 4:
		pos->vz = VMP_LoftLineVerts[3][0].vz;
		if( pos->vx < VMP_LoftLineVerts[3][0].vx ) pos->vx = VMP_LoftLineVerts[3][0].vx;
		else if( pos->vx > VMP_LoftLineVerts[3][1].vx ) pos->vx = VMP_LoftLineVerts[3][1].vx;
		break;
	  case 5:
		pos->vx = VMP_LoftLineVerts[3][3].vx;
		pos->vz = VMP_LoftLineVerts[3][3].vz;

		break;
	  default:
		VMP_PRINTF( "ERR!! LOFTLINE ID[%d]\n", v );
	}
}

void VMP_GetLinePos( FVECTOR *out, int a )
{
	switch( a ){
	  case 1:
		DG_COPY_VEC( out, &VMP_LoftLineVerts[1][1] );
		break;
	  case 2:
		DG_COPY_VEC( out, &VMP_LoftLineVerts[1][2] );
		break;
	  case 4:
		DG_COPY_VEC( out, &VMP_LoftLineVerts[3][1] );
		break;
	  case 5:
		DG_COPY_VEC( out, &VMP_LoftLineVerts[3][2] );
		break;
	  default:
		VMP_PRINTF( "ERR!! Wrong line pos[%2d]\n", a );
	}
}

int VMP_GetLinePosFromFloorLoft( FVECTOR *out, int pf, int vl, FVECTOR *now )
{
	int			loft_num;
	FVECTOR		fvtemp0,fvtemp1;
	
	if( pf == 1 || (2 <= pf && pf < 5) ){
		//1,2,3
		if( vl < 0 ){
			loft_num = (irnd()>>16)%3 + 1;
		}else{
			loft_num = (irnd()>>16)%2;
			if( vl == 1 ){
				loft_num += 2;
			}else if( vl == 3 ){
				loft_num += 1;
			}else{
				loft_num = (loft_num)?1:3;
			}
		}
	}else if( pf == 0 || (5 <= pf && pf < 8) ){
		//3,4
		if( vl < 0 ){
			loft_num = (irnd()>>16)%2 + 3;
		}else{
			loft_num = (vl == 3)?4:3;
		}
	}else if( 8 <= pf && pf < 11 ){
		//0,4
		if( vl < 0 ){
			loft_num = (irnd()>>16)%2 * 4;
		}else{
			loft_num = (vl == 0)?4:0;
		}
	}else if( 11 <= pf && pf < 14 ){
		//0,1,2
		if( vl < 0 ){
			loft_num = (irnd()>>16)%3;
		}else{
			loft_num = (irnd()>>16)%2;
			if( vl == 1 ){
				loft_num = (loft_num)?0:2;
			}else if( vl == 0 ){
				loft_num += 1;
			}
		}
	}else{
		VMP_PRINTF( "ERR Wrong floor num[%d]\n", pf );
		loft_num = 0;
	}		

	if( loft_num < 2 ){
		DG_COPY_VEC( &fvtemp0, &VMP_LoftLineVerts_debug[loft_num][0] );
		DG_COPY_VEC( &fvtemp1, &VMP_LoftLineVerts_debug[loft_num][1] );
	}else if( loft_num == 2 ){
		DG_COPY_VEC( &fvtemp0, &VMP_LoftLineVerts_debug[1][2] );
		DG_COPY_VEC( &fvtemp1, &VMP_LoftLineVerts_debug[1][3] );
	}else{
		DG_COPY_VEC( &fvtemp0, &VMP_LoftLineVerts_debug[loft_num-1][0] );
		DG_COPY_VEC( &fvtemp1, &VMP_LoftLineVerts_debug[loft_num-1][1] );
	}

	if( vl >= 0 ){
		FVECTOR fvtemp;
		float len0,len1;
		_sceVu0SubVector( &fvtemp, &fvtemp0, now );
		len0 = _Vu0VecLenXZ( &fvtemp );		
		_sceVu0SubVector( &fvtemp, &fvtemp1, now );
		len1 = _Vu0VecLenXZ( &fvtemp );
		if( len0 < 2000.0f || len1 < 2000.0f ){
			if( len0 > len1 ){
				//スワップ
				DG_COPY_VEC( &fvtemp, &fvtemp0 );
				DG_COPY_VEC( &fvtemp0, &fvtemp1 );
				DG_COPY_VEC( &fvtemp1, &fvtemp );
				len0 = len1;
			}
			{
				FVECTOR a,d;
				float	len = bp_sqrtf( 2000.0f*2000.0f-len0*len0 ); //BP_MATH - emulate PS2 sqrtf
				float	inner;

				_sceVu0SubVector( &a, &fvtemp1, &fvtemp0 );
				_sceVu0Normalize( &a, &a );
				_sceVu0SubVector( &d, now, &fvtemp0 );
				inner = _sceVu0InnerProduct( &a, &d );
				
				_sceVu0ScaleVector( &d, &a, inner );
				_sceVu0AddVector( &d, &d, &fvtemp0 );
				_sceVu0ScaleVector( &a, &a, len );
				_sceVu0AddVector( &fvtemp0, &a, &d );
				//AN_Test_Eye2( &fvtemp0, 2 );
			}
		}
		
			
	}
	
	_sceVu0SubVector( &fvtemp1, &fvtemp1, &fvtemp0 );
	_sceVu0ScaleVector( &fvtemp1, &fvtemp1, rnd() );
	_sceVu0AddVector( out, &fvtemp1, &fvtemp0 );

	return loft_num;
}


int VMP_GetLinePosFromFloorLoft_long( FVECTOR *out, int pf, int vl )
{
	int			loft_num = 0;
	FVECTOR		fvtemp0,fvtemp1;
	
	if( pf == 1 || (2 <= pf && pf < 5) ){
		//2,3
		if( vl < 0 ){
			loft_num = (irnd()>>16)%2 + 2;
		}else{
			loft_num = (vl == 2)?3:2;
		}
	}else if( pf == 0 || (5 <= pf && pf < 8) ){
		//3,4
		if( vl < 0 ){
			loft_num = (irnd()>>16)%2 + 3;
		}else{
			loft_num = (vl == 3)?4:3;
		}
	}else if( 8 <= pf && pf < 11 ){
		//0,4
		if( vl < 0 ){
			loft_num = (irnd()>>16)%2 * 4;
		}else{
			loft_num = (vl)?0:4;
		}
	}else if( 11 <= pf && pf < 14 ){
		//0,2
		if( vl < 0 ){
			loft_num = (irnd()>>16)%2 * 2;
		}else{
			loft_num = (loft_num)?0:2;
		}
	}else{
		VMP_PRINTF( "ERR Wrong floor num[%d]\n", pf );
		loft_num = 0;
	}

	if( loft_num < 1 ){
		DG_COPY_VEC( &fvtemp0, &VMP_LoftLineVerts_long[loft_num][0] );
		DG_COPY_VEC( &fvtemp1, &VMP_LoftLineVerts_long[loft_num][1] );
	}else if( loft_num > 1 ){
		DG_COPY_VEC( &fvtemp0, &VMP_LoftLineVerts_long[loft_num-1][0] );
		DG_COPY_VEC( &fvtemp1, &VMP_LoftLineVerts_long[loft_num-1][1] );
	}else{
		VMP_PRINTF( "ERR Wrong loft num[%d]\n", pf );
		
		DG_COPY_VEC( &fvtemp0, &VMP_LoftLineVerts_long[0][0] );
		DG_COPY_VEC( &fvtemp1, &VMP_LoftLineVerts_long[0][1] );
	}

	
	_sceVu0SubVector( &fvtemp1, &fvtemp1, &fvtemp0 );
	_sceVu0ScaleVector( &fvtemp1, &fvtemp1, rnd() );
	_sceVu0AddVector( out, &fvtemp1, &fvtemp0 );

	return loft_num;
}

int VMP_GetWaterJumpOutPos( FVECTOR *out, int wzn )
{
	int ret = 0;
	switch( wzn ){
	  case 20:
		DG_COPY_VEC( out, &VMP_JumpOutAttPos[0] );
		break;
	  case 22:
		DG_COPY_VEC( out, &VMP_JumpOutAttPos[1] );
		break;
	  case 18:
		DG_COPY_VEC( out, &VMP_JumpOutAttPos[2] );
		break;
	  case 21:
		DG_COPY_VEC( out, &VMP_JumpOutAttPos[3] );
		break;
	  default:
		VMP_PRINTF( "ERR!! Wrong JmpOut Water[%2d]\n", wzn );
	}
	return ret;
}

int VMP_GetToShdwAttNum( int wzn )
{
	int ret;

	switch( wzn ){
	  case 2:
		ret = 1;
		break;
	  case 3:
		ret = 0;
		break;
	  case 12:
		ret = 0;
		break;
	  case 13:
		ret = 1;
		break;
	  case 8:
		ret = 10;
		break;
	  case 9:
		ret = 10;
		break;
	  case 6:
		ret = 12;
		break;
	  case 7:
		ret = 12;
		break;
		
	  default:
		ret = -1;
	}
	return ret;
}

int GetNearAttRndPos( FVECTOR *out, FVECTOR *player, int map )
{
	
#define CHECK_OFFSET	(1800.0f)
#define GO_OFFSET		(1300.0f)
#define	ROOT_2			(1.41421f)

	FVECTOR		check_offset[8] = {
		{  CHECK_OFFSET, 0.0f, 0.0f, 0.0f },
		{ -CHECK_OFFSET, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f,  CHECK_OFFSET, 0.0f },
		{ 0.0f, 0.0f, -CHECK_OFFSET, 0.0f },
		
		{  CHECK_OFFSET/ROOT_2, 0.0f,  CHECK_OFFSET/ROOT_2, 0.0f },
		{ -CHECK_OFFSET/ROOT_2, 0.0f,  CHECK_OFFSET/ROOT_2, 0.0f },
		{  CHECK_OFFSET/ROOT_2, 0.0f, -CHECK_OFFSET/ROOT_2, 0.0f },
		{ -CHECK_OFFSET/ROOT_2, 0.0f, -CHECK_OFFSET/ROOT_2, 0.0f },
	};
	
	FVECTOR		go_offset[8] = {
		{  GO_OFFSET, 0.0f, 0.0f, 0.0f },
		{ -GO_OFFSET, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f,  GO_OFFSET, 0.0f },
		{ 0.0f, 0.0f, -GO_OFFSET, 0.0f },
		
		{  GO_OFFSET/ROOT_2, 0.0f,  GO_OFFSET/ROOT_2, 0.0f },
		{ -GO_OFFSET/ROOT_2, 0.0f,  GO_OFFSET/ROOT_2, 0.0f },
		{  GO_OFFSET/ROOT_2, 0.0f, -GO_OFFSET/ROOT_2, 0.0f },
		{ -GO_OFFSET/ROOT_2, 0.0f, -GO_OFFSET/ROOT_2, 0.0f },
	};
	FVECTOR		from;
	FVECTOR		to;
	int			hzx_result,i,ok[8],ok_cnt = 0;

	DG_COPY_VEC( &from, player );
	from.vy = -6000.0f;
	
	//AN_Test_Eye2( &from, 2 );
	for( i = 0; i < 8; i++ ){
		_sceVu0AddVector( &to, &from, &check_offset[i] );
		hzx_result = HZX_OnlineHazardCheck( GM_GetHzxGroupID( map ), &from, &to,
											HZX_CHK_SEGMENT, HZX_SEG_NO_ENEMY, HZX_FLOOR_NO_ENEMY );
#if 0
		{
			FVECTOR	line[2];
			DG_COPY_VEC( &line[0], &from );
			DG_COPY_VEC( &line[1], &to );
			
			NewLineView( line, 1, 160, 32, 140 );
		}
#endif
		if(!(hzx_result)){
			FVECTOR		temp;
			int			seg_num;
			_sceVu0AddVector( &temp, &from, &go_offset[i] );
			seg_num = HZX_NearHazardCheck( GM_GetHzxGroupID( map ), &temp, 400,
										   HZX_CHK_SEGMENT, HZX_TYPE_ENEMY,
										   400 );
			if( !seg_num ){
				//AN_Test_Eye2( &temp, 2 );
				ok[ok_cnt] = i;
				ok_cnt++;
			}
		}
		hzx_result = 0;
	}

	VMP_PRINTF("ok_cnt [%d]\n",ok_cnt);
	if( ok_cnt ){
		_sceVu0AddVector( out, &from, &go_offset[ok[(irnd()>>6)%ok_cnt]] );
		return 0;
	} else {
		return 1;
	}
	//AN_Test_Eye2( out, 2 );
	
}


int GetGoSplashWaterNum( int fzn )
{
	static int splash_num_list[] = {
		0,17,16,15,13,12,10,8,6,5,3,1
	};
	int		ret;

	if( fzn == 0 ){
		ret = (irnd()&0x00400)? 10: 12;
		
	}else if( fzn == 1 ){
		ret = (irnd()&0x00400)? 0: 17;
	}else{
		ret = splash_num_list[ fzn - 2 ];
	}

	return ret;
}

int GetGoSplashWaterNum_1st( int to_wzn )
{
	int ret = 0;
	switch( to_wzn ){
	  case 0:
	  case 17:
		ret = 7;
		break;
	  case 16:
		ret = 9;
		break;
	  case 15:
	  case 13:
		ret = 2;
		break;
	  case 12:
	  case 10:
	  case 8:
		ret = 4;
		break;
	  case 6:
	  case 5:
		ret = 12;
		break;
	  case 3:
	  case 1:
		ret = 14;
		break;
	  default:
		break;
	}

	return ret;
}

int GetGoSplashWaterNum_2nd( int to_wzn )
{
	int ret = 0;
	switch( to_wzn ){
	  case 0:
	  case 17:
		ret = 9;
		break;
	  case 16:
		ret = 7;
		break;
	  case 15:
	  case 13:
		ret = 4;
		break;
	  case 12:
	  case 10:
	  case 8:
		ret = 2;
		break;
	  case 6:
	  case 5:
		ret = 14;
		break;
	  case 1:
	  case 3:
		ret = 12;
		break;
	  default:
		break;
	}

	return ret;
}

int GetTriAttNum( FVECTOR *pos )
{
	int i;
	FVECTOR		*z_verts = VMP_TriAttFloorZoneVerts[0];

	for( i = 0; i < N_TRIATT_POS; i++ ){
		if( (z_verts[0].vx < pos->vx && pos->vx < z_verts[1].vx) &&
			(z_verts[0].vz < pos->vz && pos->vz < z_verts[1].vz) ){
			return i;
		}
		z_verts += 2;
	}
	return -1;
}

int GetTriAttDammyNum( int tri_att_num )
{
	int num_list[] = { 1, 10 };
	return num_list[tri_att_num];
}

void GetTriAttPosition( FVECTOR *out, int tri_att_num )
{
	if( tri_att_num < 0 ) { printf("いやーん\n"); return; }
	DG_COPY_VEC( out, &VMP_TriAttPosVerts[tri_att_num] );
}


void GetHndRailPosition( FVECTOR *out, int hndrl_num )
{
	static FVECTOR VMP_HndRailPos[] = {
		{  -7500.0f, HANDRAIL_POS_VY, -238000.0f, 1.0f },
		{ -12500.0f, HANDRAIL_POS_VY, -238000.0f, 1.0f },
		{  -5500.0f, HANDRAIL_POS_VY, -238000.0f, 1.0f },
		{ -14500.0f, HANDRAIL_POS_VY, -238000.0f, 1.0f },
	};
	if( hndrl_num < 0 ) { printf("いやーん\n"); return; }
	DG_COPY_VEC( out, &VMP_HndRailPos[hndrl_num] );
}

int GetHndRailPositionRnd( FVECTOR *out, int hndrl_num )
{
	int num;
	FVECTOR	*verts;
	char shift[] = { 3, 2, 1, 0 };

	if( hndrl_num < 0 ) num = (irnd()>>8)%4;
	else num = (irnd()>>8)%3;
	if( num == hndrl_num ) num += shift[hndrl_num];

	verts = VMP_HandrailLineVerts[num];
	_Vu0GetScaleVec2( out, &verts[0], &verts[1], rnd() );

	return num;
}


int GetHandRailDammyNum( int hndrl_num )
{
	int num_list[] = { 15, 6, 15, 6 };
	return num_list[hndrl_num];
}

int VMP_GetHndRailDir( FVECTOR *pos, int hndrl_num, int turn )
{
	FVECTOR *hndrl_pos = VMP_HandrailLineVerts[hndrl_num];
	FVECTOR	fvtemp,verts[2];
	int		tmp = turn, dir;
	
	pos->vx = hndrl_pos->vx;

	if( pos->vz < hndrl_pos[0].vz ){
		tmp = 0;
		//pos->vz = hndrl_pos[0].vz;
	}else if( hndrl_pos[1].vz < pos->vz ){
		tmp = 1;
		//pos->vz = hndrl_pos[1].vz;
	}
	DG_COPY_VEC( &verts[0], &hndrl_pos[tmp] );
	DG_COPY_VEC( &verts[1], &hndrl_pos[1-tmp] );
	_sceVu0SubVector( &fvtemp, &verts[1], &verts[0] );
	dir = (int)(2048.0f * atan2f( fvtemp.vx , fvtemp.vz ) / PI);
	if( dir < 0 ) dir += 4096;

	return dir;
}


void GetLoftPosition_Temp( FVECTOR *out, int loft_num )
{
	static FVECTOR VMP_LoftPos_temp[] = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{  -4700.0f, LOFT_POS_VY, -239250.0f, 1.0f, },
		{ -15300.0f, LOFT_POS_VY, -239250.0f, 1.0f, },
	};
	if( loft_num < 0 ) { printf("いやーん\n"); return; }
	DG_COPY_VEC( out, &VMP_LoftPos_temp[loft_num] );
}

int VMP_GetLoftDir_Temp( FVECTOR *pos, int loft_num, int turn )
{
	FVECTOR *loft_pos = VMP_LoftLineVerts_temp[loft_num];
	FVECTOR	fvtemp,verts[2];
	int		tmp = turn, dir;
	
	pos->vx = loft_pos->vx;

	if( pos->vz < loft_pos[0].vz ){
		tmp = 0;
		//pos->vz = hndrl_pos[0].vz;
	}else if( loft_pos[1].vz < pos->vz ){
		tmp = 1;
		//pos->vz = hndrl_pos[1].vz;
	}
	DG_COPY_VEC( &verts[0], &loft_pos[tmp] );
	DG_COPY_VEC( &verts[1], &loft_pos[1-tmp] );
	_sceVu0SubVector( &fvtemp, &verts[1], &verts[0] );
	dir = (int)(2048.0f * atan2f( fvtemp.vx , fvtemp.vz ) / PI);
	if( dir < 0 ) dir += 4096;

	return dir;
}

void VMP_GetLoftFixPos_Temp( int loft_num, FVECTOR *pos )
{
	FVECTOR *loft_pos = VMP_LoftLineVerts_temp[loft_num];
	pos->vx = loft_pos->vx;
	if( pos->vz < loft_pos[0].vz ){
		pos->vz = loft_pos[0].vz;
	}else if( loft_pos[1].vz < pos->vz ){
		pos->vz = loft_pos[1].vz;
	}
}

void VMP_GetBlowWaterPos( FVECTOR *out, FVECTOR *pos, FVECTOR *pforce )
{
	//0<->10 12<->17
	FVECTOR		force,fvtemp;
	int			i,ret = 0;
//	float		len = FLOAT_MAX;
	float		inner = -1.0f;
	float		tmp_inner;//,tmp_len
	
	_sceVu0Normalize( &force, pforce );
	for( i = 0; i < 18; i++ ){
		if( i == 11 ) continue;
		_Vu0GetCenterVec2( &fvtemp, &VmpZoneVertsWater[i][0], &VmpZoneVertsWater[i][1] );
		_sceVu0SubVector( &fvtemp, &fvtemp, pos );
		//tmp_len = _Vu0VecLenXZ( &fvtemp );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		tmp_inner = _sceVu0InnerProduct( &force, &fvtemp );

		if( tmp_inner > inner ){
			ret = i;
			inner = tmp_inner;
		}
	}

	_Vu0GetCenterVec2( out, &VmpZoneVertsWater[ret][0], &VmpZoneVertsWater[ret][1] );
}


int VMP_GetZoneNumFromForce( FVECTOR *out, FVECTOR *pos, FVECTOR *pforce )
{
	//0<->10 12<->17
	FVECTOR		force,fvtemp,center;
	int			i,ret = 0;
	float		len = FLOAT_MAX;
	float		inner = -1.0f;
	float		tmp_inner,tmp_len;
	
	_sceVu0Normalize( &force, pforce );
	for( i = 0; i < N_ZONE_MAX_FLOOR; i++ ){
		_Vu0GetCenterVec2( &center, &VmpZoneVertsFloor[i][0], &VmpZoneVertsFloor[i][1] );
		_sceVu0SubVector( &fvtemp, &center, pos );
		tmp_len = _Vu0VecLenXZ( &fvtemp );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		tmp_inner = _sceVu0InnerProduct( &force, &fvtemp );
		if( tmp_len < len && tmp_inner > inner ){
			ret = i;
			len = tmp_len;
			inner = tmp_inner;
			DG_COPY_VEC( out, &center );
		}
	}
	VMP_PRINTF("near att special step\n");
	return ret;
}


int VMP_GetTurnToPos( FVECTOR *pos, FVECTOR *to, int vl )
{
	int	ret = 0;
	switch( vl ){
	  case 0:
		if( pos->vz < to->vz ) ret = 0;
		else ret = 1;
		break;
	  case 1:
		if( pos->vx > to->vx ) ret = 0;
		else ret = 1;
		break;
	  case 2:
		if( pos->vx > to->vx ) ret = 0;
		else ret = 1;
		break;
	  case 3:
		if( pos->vz > to->vz ) ret = 0;
		else ret = 1;
		break;
	  case 4:
		if( pos->vx < to->vx ) ret = 0;
		else ret = 1;
		break;
	  case 5:
		if( pos->vx < to->vx ) ret = 0;
		else ret = 1;
		break;
	}
	return ret;
}	
