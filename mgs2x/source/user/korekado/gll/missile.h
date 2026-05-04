/*
	missile.h
	
	2002/7/16 Y.Korekado
	$Id: missile.h,v 1.1.1.3 2002/11/19 11:44:14 Yoshizawa1 Exp $
*/

typedef	struct{
	u_int 	point_num ;
	FVECTOR	*route ;
} MOVE_ROUTE ;

#define MAX_MOVE_ROUTE	(4)

#define MOVE_POINT_NUM_0	(6)
static FVECTOR MovePoint_0[] = {
	{-7000.0, 0.0, -12000.0},
	{2000.0, 0.0, -8000.0},
	{0.0, 0.0, -2000.0},
	{7000.0, 0.0, -1000.0},
	{7000.0, 0.0, -5000.0},
	{-7000.0, 0.0, -7000.0},
} ;

#define MOVE_POINT_NUM_1	(5)
static FVECTOR MovePoint_1[] = {
	{-2000.0, 0.0, -12000.0},
	{-6000.0, 0.0, -10000.0},
	{-7000.0, 0.0, -1000.0},
	{3500.0, 0.0, -1000.0},
	{3000.0, 0.0, -9000.0},
} ;
#define MOVE_POINT_NUM_2	(8)
static FVECTOR MovePoint_2[] = {
	{2000.0, 0.0, -12000.0},
	{3000.0, 0.0, -9000.0},
	{-6000.0, 0.0, -10000.0},
	{-3000.0, 0.0, -6000.0},
	{7000.0, 0.0, -5000.0},
	{3500.0, 0.0, -1000.0},
	{3000.0, 0.0, -7000.0},
	{0.0, 0.0, -10000.0},
} ;

#define MOVE_POINT_NUM_3	(5)
static FVECTOR MovePoint_3[] = {
	{7000.0, 0.0, -12000.0},
	{0.0, 0.0, -1000.0},
	{-4000.0, 0.0, -4500.0},
	{-4000.0, 0.0, -10000.0},
	{2000.0, 0.0, -12000.0},
} ;

static FVECTOR	ParaRoute[MAX_MOVE_ROUTE] = {
	{-7000.0, 10000.0, -15000.0},
	{-2000.0, 10000.0, -15000.0},
	{2000.0, 10000.0, -15000.0},
	{7000.0, 10000.0, -15000.0},
} ;

static FVECTOR	StartRoute[MAX_MOVE_ROUTE] = {
	{-7000.0, 500.0, -15000.0},
	{-2000.0, 500.0, -15000.0},
	{2000.0, 500.0, -15000.0},
	{7000.0, 500.0, -15000.0},
} ;

static MOVE_ROUTE	MoveRoute[MAX_MOVE_ROUTE] = {
	{ MOVE_POINT_NUM_0, MovePoint_0 },
	{ MOVE_POINT_NUM_1, MovePoint_1 },
	{ MOVE_POINT_NUM_2, MovePoint_2 },
	{ MOVE_POINT_NUM_3, MovePoint_3 },
} ;
