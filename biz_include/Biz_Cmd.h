/*
20141212-->to support user extended management
added: 
	1.CMD_USR_ACITVE_REQ
	2.CMD_USR_ACTIVE_REQ_ACK
	3.CMD_LGIN_EXT_ACK
	4.CMD_USR_FRIEND_ACCEPT/ACK

chang:
	1.CMD_USR_ACTIVE--->CMD_USR_ACTIVE_CONFIRM
	2.CMD_USR_ACTIVE_ACK--->CMD_USR_ACTIVE_CONFIRM_ACK
*/

/*
20141212-->to support msg-push with specified sound
added: 
	1.CMD_MSG_PUSH_EXT
	2.CMD_MSG_PUSH_EXT_ACK
*/


#ifndef _BIZ_CMD_H_
#define  _BIZ_CMD_H_

//safe delete define
#ifndef SAFE_FREE
#define SAFE_FREE(p)\
{\
	if((p) != NULL)\
	{\
		free(p) ;\
		(p) = NULL ;\
	}\
}
#endif

//command define--common
#define CMD_APP_LGN                 0x0a00
#define CMD_VER_URL_GET             0x0a02
#define CMD_SVC_LIST_GET            0x0a04
#define CMD_SVC_DETAIL_GET          0x0a06
#define CMD_BILLING_DETAIL_GET      0x0a08
#define CMD_SVR_STR_GET             0x0a0a

#define CMD_ALIVE      				0x0a0c
#define CMD_USR_REG             	0x0b00
#define CMD_USR_REG_EXT         	0x0b10
#define CMD_USR_ACTIVE_CONFIRM				0x0b12 //20141212 ACTIVE-->ACTIVE_CONFIRM
#define CMD_USR_RECOVERY_REQUEST		0x0b14
#define CMD_USR_RECOVERY_CONFIRM		0x0b16
#define CMD_USR_LGIN            0x0b02
#define CMD_USR_OLGIN           0x0b04
#define CMD_USR_LGOUT           0x0b06
#define CMD_USR_PWD_CHG     	0x0b08
#define CMD_USR_INF_SET     	0x0b0a
#define CMD_USR_INF_GET     	0x0b0c
#define CMD_USR_LGIN_EXT        0x0b0e
#define CMD_USR_DEVGRP_LIST_GET     	0x0c00
#define CMD_USR_DEVGRP_GET     			0x0c02
#define CMD_USR_DEVGRP_ADD     			0x0c04
#define CMD_USR_DEVGRP_SET     			0x0c06
#define CMD_USR_DEVGRP_DEL     			0x0c08
#define CMD_USR_DEV_LIST_GET     		0x0d00
#define CMD_USR_DEV_GET     			0x0d02
#define CMD_USR_DEV_ADD     			0x0d04
#define CMD_USR_DEV_SET      			0x0d06
#define CMD_USR_DEV_DEL     			0x0d08
#define CMD_FELLOW_LIST_SEARCH     		0x0e00
#define CMD_FELLOW_GET     				0x0e02
#define CMD_FELLOW_ADD     				0x0e04
#define CMD_FELLOW_DEL     				0x0e06
#define CMD_FELLOW_SET     				0x0e08
#define CMD_MSG_VIEW_GET     			0x0f00
#define CMD_MSG_IN_LIST_GET     		0x0f02
#define CMD_MSG_OUT_LIST_GET     		0x0f04
#define CMD_MSG_SET     				0x0f06
#define CMD_MSG_GET     				0x0f08
#define CMD_MSG_PUSH     				0x0f0a
#define CMD_MSG_TAG     				0x0f0c




#define CMD_SLR_LGIN                 	0x0100
#define CMD_SLR_LGOUT                 	0x0102
#define CMD_SLR_PWD_CHG                 0x0104
#define CMD_SLR_DEV_GET                 0x0106
#define CMD_SLR_DEV_SET               	0x0108
#define CMD_SLR_DEV_LIST_GET            0x010a


#define CMD_SYS_USR_ADMIN	0xfe00
#define CMD_SYS_USR_ADMIN_ACK	0xfe01


#define CMD_APP_LGN_ACK      		0x0a01
#define CMD_VER_URL_GET_ACK      	0x0a03
#define CMD_SVC_LIST_GET_ACK      	0x0a05
#define CMD_SVC_DETAIL_GET_ACK      0x0a07
#define CMD_BILLING_DETAIL_GET_ACK  0x0a09
#define CMD_SVR_STR_GET_ACK         0x0a0b
#define CMD_ALIVE_ACK      			0x0a0d 
#define CMD_USR_REG_ACK     		0x0b01
#define CMD_USR_REG_EXT_ACK         0x0b10
#define CMD_USR_ACTIVE_CONFIRM_ACK			0x0b13 //20141212 ACTIVE-->ACTIVE_CONFIRM
#define CMD_USR_RECOVERY_REQUEST_ACK	0x0b15
#define CMD_USR_RECOVERY_CONFIRM_ACK	0x0b17
#define CMD_USR_LGIN_ACK     	0x0b03
#define CMD_USR_OLGIN_ACK     	0x0b05
#define CMD_USR_LGOUT_ACK     	0x0b07
#define CMD_USR_PWD_CHG_ACK     0x0b09
#define CMD_USR_INF_SET_ACK     0x0b0b
#define CMD_USR_INF_GET_ACK     0x0b0d
#define CMD_USR_DEVGRP_LIST_GET_ACK     0x0c01
#define CMD_USR_DEVGRP_GET_ACK     		0x0c03
#define CMD_USR_DEVGRP_ADD_ACK     		0x0c05
#define CMD_USR_DEVGRP_SET_ACK     		0x0c07
#define CMD_USR_DEVGRP_DEL_ACK     		0x0c09
#define CMD_USR_DEV_LIST_GET_ACK     	0x0d01
#define CMD_USR_DEV_GET_ACK     		0x0d03
#define CMD_USR_DEV_ADD_ACK     		0x0d05
#define CMD_USR_DEV_SET_ACK    	 		0x0d07
#define CMD_USR_DEV_DEL_ACK     		0x0d09
#define CMD_FELLOW_LIST_SEARCH_ACK     	0x0e01
#define CMD_FELLOW_GET_ACK     			0x0e03
#define CMD_FELLOW_ADD_ACK     			0x0e05
#define CMD_FELLOW_DEL_ACK     			0x0e07
#define CMD_FELLOW_SET_ACK     			0x0e09
#define CMD_MSG_VIEW_GET_ACK     		0x0f01
#define CMD_MSG_IN_LIST_GET_ACK     	0x0f03
#define CMD_MSG_OUT_LIST_GET_ACK     	0x0f05
#define CMD_MSG_SET_ACK     			0x0f07
#define CMD_MSG_GET_ACK     			0x0f09
#define CMD_MSG_PUSH_ACK     			0x0f0b
#define CMD_MSG_TAG_ACK     			0x0f0d
#define CMD_SLR_LGIN_ACK                0x0101
#define CMD_SLR_LGOUT_ACK               0x0103 
#define CMD_SLR_PWD_CHG_ACK     		0x0105
#define CMD_SLR_DEV_GET_ACK             0x0107
#define CMD_SLR_DEV_SET_ACK             0x0109
#define CMD_SLR_DEV_LIST_GET_ACK        0x010b
#define CMD_SYS_NOTIFY	0xfffe


//20141212-->active request
#define CMD_USR_ACTIVE_REQ      				0x0200
#define CMD_USR_ACTIVE_REQ_ACK      			0x0201

#define CMD_USR_LGIN_EXT_ACK     	0x0203

#define CMD_FELLOW_ACCEPT     	0x0204
#define CMD_FELLOW_ACCEPT_ACK   0x0205

//20141213-->push extend, add sound param to push-end
#define CMD_MSG_PUSH_EXT     	0x0206
#define CMD_MSG_PUSH_EXT_ACK   0x0207

//special-control
#define CMD_SYS_MGM                             0xefef
#define CMD_SYS_MGM_ACK                 0xefef

#endif

