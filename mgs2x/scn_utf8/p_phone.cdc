//
//	p_phone.cdc
//	ケータイめーるらしい。
//
//	2001/05/16	T.Fukushima
//	$Id: p_phone.cdc,v 1.6 2002/06/07 09:53:01 usr01475 Exp $


/* メッセージ定義 */
#include "story.h"


#include "cdc_proc_p.h"

#include "p_phone_e.cm"
#include "p_phone_e.ct"

#include "p_phone_f.cm"
#include "p_phone_f.ct"

#include "p_phone_g.cm"
#include "p_phone_g.ct"

#include "p_phone_s.cm"
#include "p_phone_s.ct"

#include "p_phone_i.cm"
#include "p_phone_i.ct"

#include "p_phone_j.cm"
#include "p_phone_j.ct"


block mobile PHONE_消火 {
	@rp_phone_消火
	eval($f:rfp_PHONE_消火聞いた = 1)
}

block mobile PHONE_フナムシ {
	@rp_phone_フナムシ
}

block mobile PHONE_地雷 {
	@rp_phone_地雷
}

block mobile PHONE_リモコン近道 {
	@rp_phone_リモコン近道
}



