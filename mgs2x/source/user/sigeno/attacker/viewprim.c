/*
	viewprim.c
	デバッグ用表示

	1999/07/06 K.Sigeno
	$Id: viewprim.c,v 1.1.1.3 2002/11/19 11:49:05 Yoshizawa1 Exp $
*/



static inline void Prim_RGBA_GRAD( DG_POLY_G4 *poly ,int alpha){
	DG_SET_RGBA1( poly,   0,    0, 127, alpha );
	DG_SET_RGBA2( poly,   0,  127,   0, alpha );
	DG_SET_RGBA3( poly, 127,    0,   0, alpha );
	DG_SET_RGBA4( poly,   0,  127, 127, alpha );
}
static inline void Prim_RGBA_RED( DG_POLY_G4 *poly ,int alpha){
	DG_SET_RGBA1( poly, 127,    0,   0, alpha );
	DG_SET_RGBA2( poly, 127,    0,   0, alpha );
	DG_SET_RGBA3( poly, 127,    0,   0, alpha );
	DG_SET_RGBA4( poly, 127,    0,   0, alpha );
}
static inline void Prim_RGBA_BLUE( DG_POLY_G4 *poly ,int alpha){
	DG_SET_RGBA1( poly, 0,    0,  127, alpha );
	DG_SET_RGBA2( poly, 0,    0,  127, alpha );
	DG_SET_RGBA3( poly, 0,    0,  127, alpha );
	DG_SET_RGBA4( poly, 0,    0,  127, alpha );
}
static inline void Prim_RGBA_WHITE( DG_POLY_G4 *poly ,int alpha){
	DG_SET_RGBA1( poly, 127,  127,  127, alpha );
	DG_SET_RGBA2( poly, 127,  127,  127, alpha );
	DG_SET_RGBA3( poly, 127,  127,  127, alpha );
	DG_SET_RGBA4( poly, 127,  127,  127, alpha );
}
static inline void Prim_RGBA_YELLOW( DG_POLY_G4 *poly ,int alpha){
	DG_SET_RGBA1( poly, 127,  127,   0, alpha );
	DG_SET_RGBA2( poly, 127,  127,   0, alpha );
	DG_SET_RGBA3( poly, 127,  127,   0, alpha );
	DG_SET_RGBA4( poly, 127,  127,   0, alpha );
}
static inline void Prim_RGBA_BLACK( DG_POLY_G4 *poly ,int alpha){
	DG_SET_RGBA1( poly,  0,    0,   0, alpha );
	DG_SET_RGBA2( poly,  0,    0,   0, alpha );
	DG_SET_RGBA3( poly,  0,    0,   0, alpha );
	DG_SET_RGBA4( poly,  0,    0,   0, alpha );
}
static inline void Prim_RGBA_GREEN( DG_POLY_G4 *poly ,int alpha){
	DG_SET_RGBA1( poly,  0,  127,   0, alpha );
	DG_SET_RGBA2( poly,  0,    127,   0, alpha );
	DG_SET_RGBA3( poly,  0,    127,   0, alpha );
	DG_SET_RGBA4( poly,  0,    127,   0, alpha );
}
#define ZONE_H  1500.0F
#define ZONE_L  500.0F


static void SetZonePrim(Work *work,int prim_num,
int address,int type)
{
	HZX_ZON		*zone,tmpzone ; 
	FVECTOR		trgpos;
	int i;
	float high;
	DG_PRIM_PACKET *packet ;
	DG_POLY_G4 *poly ;

//GM_PlayerBody

	work->prim->group_id = GM_PlayerBody->objs->group_id ;

	packet = (DG_PRIM_PACKET*)( (int)work->prim->packs[DG_Clock] );
	poly = (DG_POLY_G4*)packet->prim_top ;


	prim_num *= 4;
	for(i=0;i<prim_num;i++,poly++);
	high = ZONE_L;
	for(i=0;i<4;i++){
		switch(type){
			case 0 :
				Prim_RGBA_GRAD( poly ,127);
				break;
			case 1 :
				Prim_RGBA_RED( poly ,127);
				break;
			case 2 :
				Prim_RGBA_GREEN( poly ,127);
				break;
			case 3 :
				Prim_RGBA_BLUE( poly ,127);
				break;
			case 4 :
				Prim_RGBA_YELLOW( poly ,127);
				break;
		}
		poly++;
	}
#if 0
	switch(type){
		case 0:
			/*高い青*/
			high = ZONE_H;
			for(i=0;i<4;i++){
				Prim_RGBA_BLUE( poly ,127);
				poly++;
			}
			break;
		case 1:
			/*低い赤*/
			high = ZONE_H;
			for(i=0;i<4;i++){
				Prim_RGBA_RED( poly ,127);
				poly++;
			}
			break;
		default :
			/*その他黄*/
			high = ZONE_H;
			for(i=0;i<4;i++){
				Prim_RGBA_YELLOW( poly ,127);			
				poly++;
			}
			break;
	}
#endif

	DG_VisiblePrim( work->prim );

	if(HZX_Zone1(address) == 255){

		tmpzone.x = 0;
		tmpzone.y = 0;
		tmpzone.z = 0;

		tmpzone.w = 0;
		tmpzone.h = 0;

		zone = &tmpzone;

	}else {
		zone = ENE_HZX_GetZone( address ) ;
	}

	trgpos.vx = zone->x;
	trgpos.vy = zone->y;
	trgpos.vz = zone->z;

	/*4枚のポリゴンを直方体状に配置*/
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = trgpos;
		if(i&1)	work->primpos[prim_num][i].vx += (float)(zone->w);
		else	work->primpos[prim_num][i].vx -= (float)(zone->w);
		if(i<2)	work->primpos[prim_num][i].vz -= (float)(zone->h);
		else	work->primpos[prim_num][i].vz += (float)(zone->h);
		work->primpos[prim_num][i].vy  += high;
	}
	prim_num++;
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = trgpos;
		work->primpos[prim_num][i].vx -= (float)(zone->w);
		if(i&1)	work->primpos[prim_num][i].vz += (float)(zone->h);
		else	work->primpos[prim_num][i].vz -= (float)(zone->h);
		if(i<2)	work->primpos[prim_num][i].vy += high;
	}
	prim_num++;
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = trgpos;
		if(i&1)	work->primpos[prim_num][i].vx += (float)(zone->w);
		else	work->primpos[prim_num][i].vx -= (float)(zone->w);
		if(i<2)	work->primpos[prim_num][i].vy += high;
		work->primpos[prim_num][i].vz += (float)(zone->h);
	}
	prim_num++;
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = trgpos;
		work->primpos[prim_num][i].vx += (float)(zone->w);

		if(i&1)	work->primpos[prim_num][i].vz -= (float)(zone->h);
		else	work->primpos[prim_num][i].vz += (float)(zone->h);
		if(i<2)	work->primpos[prim_num][i].vy += high;
	}
}

static void FlatZonePrim(Work *work,int prim_num,
int address )
{
	HZX_ZON		*zone,tmpzone ; 
	FVECTOR		trgpos;
	int i;
	

	DG_VisiblePrim( work->prim );

	if(HZX_Zone1(address) == 255){

		tmpzone.x = 0;
		tmpzone.y = 0;
		tmpzone.z = 0;

		tmpzone.w = 0;
		tmpzone.h = 0;

		zone = &tmpzone;

	}else {
		zone = ENE_HZX_GetZone( address ) ;
	}

		trgpos.vx = zone->x;
		trgpos.vy = zone->y;
		trgpos.vz = zone->z;

	/*4枚のポリゴンを直方体状に配置*/
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = trgpos;
		if(i&1)	work->primpos[prim_num][i].vx += (float)(zone->w);
		else	work->primpos[prim_num][i].vx -= (float)(zone->w);
		if(i<2)	work->primpos[prim_num][i].vz -= (float)(zone->h);
		else	work->primpos[prim_num][i].vz += (float)(zone->h);
		work->primpos[prim_num][i].vy  += ZONE_H;
	}
}

static void NaviZone(Work *work,ENETHINK *entk,int offset) {

	int i;
	int addr1,addr2,addr3;
	HZX_HDL	*hzx ;		
	FVECTOR		fpos;

	addr1 = (entk->znavi->this_addr) ; 
	addr2 = (entk->znavi->going_addr) ; 
//	addr2 = GM_PlayerAddress ; 

	fpos = entk->znavi->flore_pos;


#if 0
/*ＢＯＸ*/
	for(i=offset;i<(PRIM_NUM/4);i++){
		addr3 = HZX_Navigate( addr1, addr2, &fpos );
		if((addr3==255)||(addr3==addr1)) break;
		SetZonePrim(work,i,HZX_Zone1(addr3),entk->ctrl->hzx_id,0);
		addr1 = (addr3|(addr3<<8));
	}
#else
/*flat*/
	for(i=offset;i<PRIM_NUM;i++){
		addr3 = HZX_Navigate( addr1, addr2, &fpos );

//		if((addr3==255)||(addr3==addr1)) break;
		if(addr3==addr1) addr3 = (255|(255<<8));
		FlatZonePrim(work,i,HZX_Zone1(addr3));
//		addr1 = (addr3|(addr3<<8));
		addr1 = addr3;
	}
#endif
}
static void SetFarZone(Work *work,int prim_num,ENETHINK *entk){
	int rout,trgaddr,i;

	trgaddr = HZX_FarZoneNavigate( entk->ctrl->hzx_id,
	entk->ctrl->addr,GM_PlayerAddress, &rout );
	for(i=0;i<10;i++){
		trgaddr = HZX_FarZoneNavigate( entk->ctrl->hzx_id,
		trgaddr,GM_PlayerAddress, &rout );
	}
	SetZonePrim(work,0,trgaddr,0);
}

static void SetPosPrim(Work *work,FVECTOR *pos,float size,int prim_num,ENETHINK *entk)
{
	DG_PRIM_PACKET *packet ;
	DG_POLY_G4 *poly ;

	int i;

	packet = (DG_PRIM_PACKET*)( (int)work->prim->packs[DG_Clock] );
	poly = (DG_POLY_G4*)packet->prim_top ;


	DG_VisiblePrim( work->prim );

#if 1
	switch(entk->think3) {

//		case TH3_ZONE_CHASE:
//		case TH3_SAFEZONE_CHASE:
		case TH3_ROLL_OUT:
			/*白*/
			Prim_RGBA_WHITE( poly ,127);			
		break;
//		case TH3_SIDE_OUT:
		case TH3_DANGER :
			/*ダイレクト 赤*/
			Prim_RGBA_RED( poly ,127);			
		break;
		case TH3_WAIT_CHASE:
			/*待機 黄色*/
			Prim_RGBA_YELLOW( poly ,127);			
		break;
		case TH3_ATTACK_SIEGE :
			/* 横移動青 */
			Prim_RGBA_BLUE( poly ,127);			
		break;
//		case TH3_ATTACK_NEAR :
		case TH3_WAIT_SAFE :

			/*虹*/
			Prim_RGBA_GRAD( poly ,127);			
		break;
		default :
			DG_InvisiblePrim( work->prim );
		break;
	}
#endif
#if 0
	if(entk->think2 != TH2_CHASE) {
		/*attack*/
		/*攻撃 虹*/
		Prim_RGBA_GRAD( poly ,127);			
	}
#endif
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = *pos;
		if(i&1)	work->primpos[prim_num][i].vx += (size);
		else	work->primpos[prim_num][i].vx -= (size);
		if(i<2)	work->primpos[prim_num][i].vz -= (size);
		else	work->primpos[prim_num][i].vz += (size);

		work->primpos[prim_num][i].vy += (size);
	}
}
static void SetIDPrim(Work *work,FVECTOR *pos,float size,int prim_num,ENETHINK *entk)
{
	DG_PRIM_PACKET *packet ;
	DG_POLY_G4 *poly ;

	int i;

	packet = (DG_PRIM_PACKET*)( (int)work->prim->packs[DG_Clock] );
	poly = (DG_POLY_G4*)packet->prim_top ;


	DG_VisiblePrim( work->prim );

	switch(entk->id) {
		case 0 :
			/*ダイレクト 赤*/
			Prim_RGBA_RED( poly ,127);			
		break;
		case 1:
			/*待機 黄色*/
			Prim_RGBA_YELLOW( poly ,127);			
		break;
		case 2 :
			/* 横移動青 */
			Prim_RGBA_BLUE( poly ,127);			
		break;
		case 3 :
			/*虹*/
			Prim_RGBA_GRAD( poly ,127);			
		break;
		default :
			DG_InvisiblePrim( work->prim );
		break;
	}
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = *pos;
		if(i&1)	work->primpos[prim_num][i].vx += (size);
		else	work->primpos[prim_num][i].vx -= (size);
		if(i<2)	work->primpos[prim_num][i].vz -= (size);
		else	work->primpos[prim_num][i].vz += (size);

		work->primpos[prim_num][i].vy += (size);
	}
}



static void SetLevelPrim(Work *work,FVECTOR *pos,float size,int prim_num,ENETHINK *entk)
{
	DG_PRIM_PACKET *packet ;
	DG_POLY_G4 *poly ;

	int i;

	packet = (DG_PRIM_PACKET*)( (int)work->prim->packs[DG_Clock] );
	poly = (DG_POLY_G4*)packet->prim_top ;


	DG_VisiblePrim( work->prim );

	switch(entk->at_com->level) {

		case 0 :
			/*ダイレクト 赤*/
			Prim_RGBA_RED( poly ,127);			
		break;
		case 1:
			/*待機 黄色*/
			Prim_RGBA_YELLOW( poly ,127);			
		break;
		case 2 :
			/* 横移動青 */
			Prim_RGBA_BLUE( poly ,127);			
		break;
		case 3 :
			/*虹*/
			Prim_RGBA_GRAD( poly ,127);			
		break;
		case 4 :
			/*ダイレクト 赤*/
			Prim_RGBA_WHITE( poly ,127);			
		break;
		default :
			DG_InvisiblePrim( work->prim );
		break;
	}
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = *pos;
		if(i&1)	work->primpos[prim_num][i].vx += (size);
		else	work->primpos[prim_num][i].vx -= (size);
		if(i<2)	work->primpos[prim_num][i].vz -= (size);
		else	work->primpos[prim_num][i].vz += (size);

		work->primpos[prim_num][i].vy += (size);
	}
}

static void SetRankPrim(Work *work,FVECTOR *pos,float size,int prim_num,ENETHINK *entk)
{
	DG_PRIM_PACKET *packet ;
	DG_POLY_G4 *poly ;

	int i;

	packet = (DG_PRIM_PACKET*)( (int)work->prim->packs[DG_Clock] );
	poly = (DG_POLY_G4*)packet->prim_top ;

	DG_VisiblePrim( work->prim );
	switch(at_thk->dis_rank) {

		case 0 :
			Prim_RGBA_RED( poly ,127);			
			break;
		case 1:
			Prim_RGBA_YELLOW( poly ,127);			
			break;
		case 2 :
			Prim_RGBA_BLUE( poly ,127);			
			break;
		case 3 :
			Prim_RGBA_WHITE( poly ,127);			
			break;
		default :
			DG_InvisiblePrim( work->prim );
		break;
	}
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = *pos;
		if(i&1)	work->primpos[prim_num][i].vx += (size);
		else	work->primpos[prim_num][i].vx -= (size);
		if(i<2)	work->primpos[prim_num][i].vz -= (size);
		else	work->primpos[prim_num][i].vz += (size);

		work->primpos[prim_num][i].vy += (size);
	}
}

static void SetCntPrim(Work *work,FVECTOR *pos,float size,int prim_num,ENETHINK *entk)
{
	DG_PRIM_PACKET *packet ;
	DG_POLY_G4 *poly ;

	int i;

	packet = (DG_PRIM_PACKET*)( (int)work->prim->packs[DG_Clock] );
	poly = (DG_POLY_G4*)packet->prim_top ;

	Prim_RGBA_RED( poly ,127);			
#if 1
	if(entk->debug > 0 ) {
		entk->debug--;
		DG_VisiblePrim( work->prim );
	}else {
		DG_InvisiblePrim( work->prim );
	}
#endif
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = *pos;
		if(i&1)	work->primpos[prim_num][i].vx += (size);
		else	work->primpos[prim_num][i].vx -= (size);
		if(i<2)	work->primpos[prim_num][i].vz -= (size);
		else	work->primpos[prim_num][i].vz += (size);

		work->primpos[prim_num][i].vy += (size);
	}
}

static void SetSight(Work *work,int prim_num,ENETHINK *entk){
	SENSEPARAM *sens ;
	FVECTOR *pos ;
	DG_PRIM_PACKET *packet ;
	DG_POLY_G4 *poly ;

	int i,size = 500;

	sens = &entk->sense ;
	pos = &entk->ctrl->mov ;

	packet = (DG_PRIM_PACKET*)( (int)work->prim->packs[DG_Clock] );
	poly = (DG_POLY_G4*)packet->prim_top ;

	DG_VisiblePrim( work->prim );
	Prim_RGBA_RED( poly ,127);			
	
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = *pos;
		if(i&1)	work->primpos[prim_num][i].vx += (size);
		else	work->primpos[prim_num][i].vx -= (size);
		if(i<2)	work->primpos[prim_num][i].vz -= (size);
		else	work->primpos[prim_num][i].vz += (size);

		work->primpos[prim_num][i].vy += (size);
	}

}

static void SetPosEye(Work *work,FVECTOR *pos,float size,int prim_num,ENETHINK *entk)
{
	DG_PRIM_PACKET *packet ;
	DG_POLY_G4 *poly ;

	int i;

	packet = (DG_PRIM_PACKET*)( (int)work->prim->packs[DG_Clock] );
	poly = (DG_POLY_G4*)packet->prim_top ;


	DG_VisiblePrim( work->prim );
	if(entk->pl_eyei.sight == EYE_INFO_SIGHT_IN) {
		Prim_RGBA_RED( poly ,127);			
	}else{
		Prim_RGBA_BLUE( poly ,127);			
	}

	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = *pos;
		if(i&1)	work->primpos[prim_num][i].vx += (size);
		else	work->primpos[prim_num][i].vx -= (size);
		if(i<2)	work->primpos[prim_num][i].vz -= (size);
		else	work->primpos[prim_num][i].vz += (size);

		work->primpos[prim_num][i].vy += (size);
	}
}

#if 0
static void SetTrapPrim(Work *work,int prim_num,int trap_num)
{
	int i;
	FVECTOR		pos1,pos2;
	pos1 = GM_CameraList[trap_num].vec1;
	pos2 = GM_CameraList[trap_num].vec2;

	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = pos1;
		if(i=1) {
			work->primpos[prim_num][i].vx = pos2.vx;
		}
		if(i=2) {
			work->primpos[prim_num][i].vz = pos2.vz;
		}
		if(i=3) {
			work->primpos[prim_num][i].vx = pos2.vx;
			work->primpos[prim_num][i].vz = pos2.vz;
		}
//		work->primpos[prim_num][i].vy = (size);
	}
}
#endif

static int AddrSet(int addr){
	addr = HZX_Zone1(addr);
	addr = (addr|(addr<<8)|(HZX_ZoneMapNo(GM_PlayerAddress)<<16));
	return addr;
}

static void ViewSafe(Work *work){
	HZX_ZON *pl_zone;
	int safezone,i;
	pl_zone = ENE_HZX_GetZone(GM_PlayerAddress);
	/*安全地帯表示*/
	for(i=0;i<(HZX_MAX_SAFEZONE_NUM);i++){
		safezone = pl_zone->safes[i];
		if(safezone != 255){
			SetZonePrim(work,i,AddrSet(safezone),0);
		}else {
			break ;
		}
	}
}


static void AT_ActPrimPos( Work *work,ENETHINK *entk)
{

	int mode;
	
	mode = 0 ;

	work->prim->group_id = entk->act->body->objs->group_id;
//	GM_PlayerBody->objs->group_id = work->prim->group_id ;
	ViewSafe(work);
	switch(mode){
		case 0 : break; 
		case 1 : /*現在地と思考を示す*/
			SetPosPrim( work, &entk->ctrl->mov,500,0,entk);
			break;
		case 2 : /*視界情報 見えてる 赤 見えない 青*/
			SetPosEye( work, &entk->ctrl->mov,500,0,entk);
			break;
		case 3 : /*時間限定表示*/
			SetCntPrim( work, &entk->ctrl->mov,500,0,entk);
			break;
		case 4: /*順位判定結果を示す*/
			SetRankPrim( work, &entk->ctrl->mov,500,0,entk);
			break;
		case 5: /*現在ゾーン*/
			SetZonePrim(work,0,entk->ctrl->addr,0);
			break;
		case 6: /*現在ゾーン*/
			SetZonePrim(work,0,*entk->at_com->teamaddr[0][entk->id],1);
			break;
		case 7: /*ＬＥＶＥＬ*/
			SetLevelPrim( work, &entk->ctrl->mov,500,0,entk);
			break;
		case 8: /*順位判定結果を示す*/
			SetIDPrim( work, &entk->ctrl->mov,500,0,entk);
			break;
		case 9: /*逃げるための一時目標*/
			SetFarZone(work,0,entk);
			break;
	}
}




static void InitPacket( DG_PRIM_PACKET *packet, int n, int which,int mode ,int color)
{
	int	i ;
	DG_POLY_G4 *poly ;


	poly = (DG_POLY_G4*)packet->prim_top ;
	/*加算半透明*/
	switch(mode){
		case 0: /*加算*/
			*(u_long64*)&packet->prim_init.alpha = SCE_GS_SET_ALPHA( 0,2,0,1,0 ) ;
			break;
		case 1: /*平均半透明*/
			*(u_long64*)&packet->prim_init.alpha = SCE_GS_SET_ALPHA( 0,1,0,1,0 ) ;
			break;
		case 2: /*反転半透明*/
			*(u_long64*)&packet->prim_init.alpha = SCE_GS_SET_ALPHA( 0,1,0,2,0 ) ;
			break;
	}

	packet->gif_tag.PRIM |= SCE_GS_PRIM_ABE ;
	packet->gif_tag.PRE = 1 ;
	packet->gif_tag.REGS0 = GS_REGS_PRIM ;


	for ( i = n ; i > 0 ; i-- ){
		DG_SET_POLYG4( poly, 1 );
		switch(color){
			case 0:
				Prim_RGBA_GRAD(poly , 127);
				break;
			case 1:
				Prim_RGBA_RED(poly , 127);
				break;
			case 2:
				Prim_RGBA_BLUE(poly , 127);
				break;
			case 3:
				Prim_RGBA_WHITE(poly , 127);
				break;
			case 4:
				Prim_RGBA_BLACK( poly ,0);
				break;
		}
		poly++ ;
	}
}
//#define POLY_PRIM (DG_PRIM_POLY_G4|DG_PRIM_VISIBLE|DG_PRIM_ON_WORLD|DG_PRIM_VERTICES)
#define POLY_PRIM (DG_PRIM_POLY_G4|DG_PRIM_INVISIBLE|DG_PRIM_ON_WORLD|DG_PRIM_VERTICES)

static void InitPrim( Work *work,int mode,int color)
{
	int i,j;
	DG_PRIM_PACKET *packet ;

	work->prim = GM_MakePrim( POLY_PRIM, 1,PRIM_NUM, work->primpos, NULL ) ;

	for ( j = 0 ; j < 2 ; j++ ){
		for ( i = 0 ; i < 1 ; i++ ){
			packet = (DG_PRIM_PACKET*)( (int)work->prim->packs[j] + work->prim->packet_size * i );
			InitPacket( packet, PRIM_NUM, j ,mode ,color);
		}
	}

}

