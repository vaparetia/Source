/*
	safecheck.c
	安全地帯判定
	1999/07/30 K.Sigeno
	$Id: safecheck.c,v 1.1.1.3 2002/11/19 11:49:04 Yoshizawa1 Exp $
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"korekado/conv/korekado.x"
#include	"korekado/enemy/enemy.h"

#ifdef OLD_SAFE
SAFEZONE Safezone ;
#else
SAFEZONE Safezone[256] ;
#endif

static int AddrSet(int addr){
	addr = HZX_Zone1(addr);
	addr = (addr|(addr<<8)|(GM_PlayerMap<<16));
	return addr;
}
#define	ONLINE_RATE 0.75F	/*四隅より少し内側で判定*/
#define	MIN_SEARCH	1	/*近すぎるゾーンは判定外*/
#define	MAX_SEARCH	10	/*遠すぎるゾーンも判定外*/

/*ゾーン中心近接壁 チェック半径*/
#define	S_SPHERE (500) 

void SafeZoneMake( hzx_id )
HZX_GROUP_ID hzx_id;
{
	HZX_ZON		*zone1,*zone2 ; 
	FVECTOR		vect_ptr[2]; /*壁判定*/
	FVECTOR		pos1,pos2,w_check,hitpoint;
	int addr1,addr2,hit_num,hzx_dir;
	int i,j,k,l,m,type,group;
	int	tmproute,num_zones,zone_dis;
	float	tmp;

	HZX_GRP *grp;

	grp = HZX_GetGroup( hzx_id );

//	num_zones = hzx->def->groups->n_zones;
	num_zones = grp->n_zones;

	for(addr1 = 0;addr1<num_zones;addr1++){
		if(addr1==0){
			printf("SAFEZONE CHECK START\n");
			printf("TOTAL ZONE NUM %d\n",num_zones);
			printf("NOW CHECKING. PLEASE WAIT\n");
		}
		/*仮想プレイヤゾーンセット*/
		zone1 = HZX_GetZone(hzx_id, addr1 ) ;
		pos1.vx = (float) zone1->x;
		pos1.vy = (float) (zone1->y+1500); /*目線の高さで判定*/
		pos1.vz = (float) zone1->z;
		/*格納先を初期化*/
		for(j=0;j<(SAFE_NUM);j++) {
#ifdef OLD_SAFE
			Safezone.zone[addr1][j] = 255;
			Safezone.reach[addr1][j] = 255;
			Safezone.type[addr1][j] = 0;
#else
			Safezone[addr1].zone[j] = 255;
			Safezone[addr1].reach[j] = 255;
			Safezone[addr1].type[j] = 0;
#endif
		}
		/** ゾーン数だけループ **/
		for(addr2=0;addr2<num_zones;addr2++){
		/*同一ゾーン同士は不要*/
			if(addr1==addr2) continue;
		/*リーチ数が規定内か？*/
			tmproute = 1+HZX_GetRoute( hzx_id, addr1, addr2 ) ;
			if((MIN_SEARCH>=tmproute)||(MAX_SEARCH <= tmproute)) continue;
		/*ゾーン間距離が規定内か？*/
//			zone_dis = ENE_GetRouteDis(NULL,NULL,
//			addr1,addr2,SAFE_MAX_DIS);

			zone_dis = ENE_GetRouteDis(NULL,NULL,
			AddrSet(addr1),AddrSet(addr2),SAFE_MAX_DIS);

			if(zone_dis >= SAFE_MAX_DIS) continue ;
		/*u_charに収める*/
			zone_dis = (zone_dis/500)+1;
			if(zone_dis>255) zone_dis = 255 ;
		/*チェック目標設定*/
			zone2 = HZX_GetZone(hzx_id, addr2 ) ;
			hit_num = 0;
			type = 0;
			pos2.vy = (float) (zone2->y+1500);
			/*めり込み判定用に中心セット*/

			w_check.vx = (float)(zone2->x);
			w_check.vy = (float)(zone2->y+1500);
			w_check.vz = (float)(zone2->z);

		/*高さが違い過ぎる場合 無効*/
			tmp = pos1.vy - pos2.vy;
			if((tmp>2000.0F)||(tmp< -2000.0F )) continue;

			/*最初に中心チェック*/
			pos2.vx = (float)zone2->x;
			pos2.vz = (float)zone2->z;

			if(HZX_OnlineHazardCheck(hzx_id,&pos1,&pos2,HZX_CHK_ALL,
			HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET ) ){
				hit_num++;
				type |= (SAFE_CENTER);
			}
			/*四隅をチェック*/
			for(i=0;i<4;i++){
				/*チェック用ベクトル生成*/
				if(i&1)	pos2.vx = (float)zone2->x+((float)zone2->w*ONLINE_RATE);
				else	pos2.vx = (float)zone2->x-((float)zone2->w*ONLINE_RATE);
				if(i<2)	pos2.vz = (float)zone2->z-((float)zone2->h*ONLINE_RATE);
				else	pos2.vz = (float)zone2->z+((float)zone2->h*ONLINE_RATE);
				/*壁めり込み判定*/
				/*ゾーンの中心からチェック*/
				if(HZX_OnlineHazardCheck(hzx_id,&w_check,&pos2,
				HZX_CHK_ALL,HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET ) ){
					/*ゾーン内部に壁あり*/
					HZX_GetOnlinePoint(&hitpoint);
					pos2 = hitpoint;
					if(pos2.vx > w_check.vx ) pos2.vx -= 5.0F;
					else  pos2.vx += 5.0F;
					if(pos2.vz > w_check.vz ) pos2.vz -= 5.0F;
					else  pos2.vz += 5.0F;
				}
				/*オンラインチェック*/
				if(HZX_OnlineHazardCheck(hzx_id,&pos1,&pos2,HZX_CHK_ALL,HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET ) )
				{
					hit_num++;
					switch (i) {
						case 0:
							type |= (SAFE_POINT1) ;
							break ;
						case 1:
							type |= (SAFE_POINT2) ;
							break ;
						case 2:
							type |= (SAFE_POINT3) ;
							break ;
						case 3:
							type |= (SAFE_POINT4) ;
							break ;
					}
				}
			}
			/* 高さ１５００で壁が見つからなかった場合低い位置で再チェック*/
			/* 3ゾーン以上離れているときのみ */
			if((hit_num==0)&&(3 < tmproute)){
				/*ゾーンの中心のみ*/
				pos2.vx = (float)zone2->x;
				pos2.vy = (float)(zone2->y+500);
				pos2.vz = (float)zone2->z;
				if(HZX_OnlineHazardCheck(hzx_id,&pos1,&pos2,
				HZX_CHK_FIX,HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET ) )
				{
					hit_num++;
					type |= (SAFE_LOW|SAFE_CENTER);
				}
			}
			/*安地検出*/
			if(hit_num > 0)
			{
				/*低いゾーンでなければゾーン近接壁チェック*/
				if(!(type&SAFE_LOW)){
					int hzx_check;
					w_check.vx = (float)(zone2->x);
					w_check.vy = (float)(zone2->y+1500);
					w_check.vz = (float)(zone2->z);
					hzx_check = HZX_NearHazardCheck(hzx_id,
					&w_check,S_SPHERE,HZX_CHK_F_SEGMENT,
					HZX_TYPE_BULLET,0 );

					/*壁検出 壁の方向によりフラグ立て*/
					if(hzx_check>0) {
						HZX_GetNearVector( vect_ptr );
						for(m=0;m<hzx_check;m++){
							hzx_dir = GV_VecDir2( &vect_ptr[i] );
							if( ( 3072+512 <= hzx_dir )||(hzx_dir < 512 )){
								type |= SAFE_BEHIND1;
							}else if( (  0+512 <=hzx_dir)&&(hzx_dir< 1024+512 )){
								type |= SAFE_BEHIND2;
							}else if( (1024+512 <= hzx_dir)&&(hzx_dir < 2048+512)){
								type |= SAFE_BEHIND3;
							}else if( (2048+512 <= hzx_dir)&&(hzx_dir < 3072+512)){ 
								type |= SAFE_BEHIND4;
							}
						}
					}
				}
				/*過去に検出したゾーンと距離比較*/
				for(k=0;k<(SAFE_NUM);k++){
#ifdef OLD_SAFE
					if(Safezone.reach[addr1][k] > zone_dis) {
#else
					if(Safezone[addr1].reach[k] > zone_dis) {
#endif
						/*安地更新された*/
						/*ソート格納準備*/
						for(l=(SAFE_NUM-2);l>=k;l--){
#ifdef OLD_SAFE
							Safezone.zone[addr1][l+1] = Safezone.zone[addr1][l];
							Safezone.reach[addr1][l+1] = Safezone.reach[addr1][l];
							Safezone.type[addr1][l+1] = Safezone.type[addr1][l];
#else
							Safezone[addr1].zone[l+1] = Safezone[addr1].zone[l];
							Safezone[addr1].reach[l+1] = Safezone[addr1].reach[l];
							Safezone[addr1].type[l+1] = Safezone[addr1].type[l];
#endif
						}
#ifdef OLD_SAFE
						Safezone.reach[addr1][k] = zone_dis;
						Safezone.zone[addr1][k] = addr2;
						Safezone.type[addr1][k] = type;
#else
						Safezone[addr1].reach[k] = zone_dis;
						Safezone[addr1].zone[k] = addr2;
						Safezone[addr1].type[k] = type;
#endif
						break;
					}
				}
			}
		}
	}
	printf("SAFEZONE CHECK END\n");
#if 0
	/**安地の内容 書き出し**/
	printf("******** SAFE ZONE ********\n");
	for(addr1 = 0;addr1<num_zones;addr1++){
		printf("PLAYER ADDR %d \n",addr1);
		printf("zone %d %d %d %d \n",Safezone[addr1].zone[0],Safezone[addr1].zone[1],Safezone[addr1].zone[2],Safezone[addr1].zone[3]);
		printf("reach %d %d %d %d\n",Safezone[addr1].reach[0],Safezone[addr1].reach[1],Safezone[addr1].reach[2],Safezone[addr1].reach[3]);
		printf("type %x %x %x %x \n",Safezone[addr1].type[0],Safezone[addr1].type[1],Safezone[addr1].type[2],Safezone[addr1].type[3]);
	}
#endif
}

