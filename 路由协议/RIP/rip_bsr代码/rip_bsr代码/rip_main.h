/**************************************************************************
 *                                                                        *
 *           Copyright 1998 - 2001, Baud Data Communication Ltd.          *
 *                      ALL RIGHTS RESERVED                               *
 *                                                                        *
 *   Permission is hereby granted to licensees of Baud Data               *
 *   Communication Ltd. products to use or abstract this computer         *
 *   program for the sole purpose of implementing a product based         *
 *   on Baud Data Communication Ltd. products.   No other rights to       *
 *   reproduce, use, or disseminate this computer program, whether        *
 *   in part or in whole, are granted.                                    *
 *                                                                        *
 *   Baud Data Communication, Ltd. makes no representation or warranties  *
 *   with respect to the performance of this computer program, and        *
 *   specifically disclaims any responsibility for any damages,           *
 *   special or consequential, connected with the use of this program.    *
 *                                                                        *
 *------------------------------------------------------------------------*
 *
 *        MODULE NAME     :   RIP
 *        FILE NAME       :   rip_main.h
 *        AUTHOR          :   Fengsb
 *        DATE            :   2007.07.12 13:51:34
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#ifndef RIP_MAIN_H
#define RIP_MAIN_H

#ifdef __cplusplus
	extern "C" {
#endif /* __cplusplus */
/******************* BEGIN HEADER FILE **********************************/
extern unsigned long rip_semaphore;

/*RIP错误返回码*/
#define RIP_SUCCESS  0
#define RIP_FAIL -1

#define RIP_MEM_FAIL -2

#define RIP_MEM_STAT     1 /* routing module */

/*Fengsb add 2006-02-13*/
#define DEFAULT_EQUAL_RTNUM 4 /*主路由表中缺省等价路由数目*/
#define MAX_EQUAL_RTNUM 8     /*主路由表中最大等价路由数目*/

/*宏函数*/
#if RIP_MEM_STAT
#define rip_mem_malloc( size, malloc_type ) mem_malloc( size, RT_RIP, malloc_type );
#define rip_mem_free( buffer, malloc_type ) mem_free( buffer, RT_RIP, malloc_type )
#else
#define rip_mem_malloc( size, malloc_type ) sys_mem_malloc( size );
#define rip_mem_free( buffer, malloc_type ) sys_mem_free( buffer )
#endif

/* 在对rip intf lk进行操作的时候，要加锁
 * Commented by dangzhw in 2010.02.01 10:46:40 */
/*#define rip_lock() \
{ \
} 
#define rip_unlock() \
{ \
}*/
#define RIP_LOCK mu_lock(rip_glb_info.sem_id , MU_WAIT , 0)
#define RIP_UNLOCK mu_unlock(rip_glb_info.sem_id)

/*宏定义*/
#define RIP_VRF_NAME_LEN    16
#define RIP_VRF_LIMIT       255
#define RIP_AUTH_AFI        0xffff
#define RIP_MD5_TAIL_TAG    0x01
#define RIP_AUTHEN_TAIL_TAG 0x0001

#define RIP_ACCLIST_NAME_LEN 64    /*access-list name len*/

#define RIP_DEF_INPUT_QUEUE 200

#define RIP_DEF_UPDATE_TIMEOUT      30     /*default update timer timeout*/
#define RIP_DEF_EXPIRE_TIMEOUT      180    /*default expire timer timeout*/
#define RIP_DEF_HOLDDOWN_TIMEOUT    120    /*default holddown timer timeout*/
#define RIP_DEF_TRIGGER_MAX_TIMEOUT  5 
#define RIP_DEF_TRIGGER_TIMEOUT RIP_DEF_TRIGGER_MAX_TIMEOUT /*default trigger timer timeout*/
#define RIP_AUTH_COMMIT_TIMEOUT      2
#define RIP_AUTH_COMMIT_MAX_TIMEOUT (RIP_AUTH_COMMIT_TIMEOUT*2+1)
#define RIP_DEF_PEER_TIMEOUT ((RIP_DEF_EXPIRE_TIMEOUT+RIP_DEF_HOLDDOWN_TIMEOUT)/3+2)/*default peer timer timeout,suggesting to be (expiretime+holddowntime)/3+2s*/
/*#define RIP_DEF_PEER_DELETING_TIMEOUT (RIP_DEF_PEER_TIMEOUT*3)*//*the third times timeout ,peer would be deleted*/

#define RIP_MAX_PACKET_SIZE         512  /*RIP包最大字节数      */

#define RIP_DEF_METRIC              1    /*RIP缺省metirc值   */
#define RIP_MAX_METRIC              16   /*metric最大值   */
#define RIP_DIRECT_METRIC           0    /*RIP直连路由metirc值     */

#define RIP_DEF_DISTANCE            120  /*RIP缺省管理距离      */
#define RIP_PORT                    520  /*RIP报文端口号          */
#define RIP_MAX_DISTANCE            255  /*RIP管理距离最大值*/

#define RIP_MULTICAST_ADDRESS       0xE0000009/*RIP组播地址224.0.0.9     */
#define RIP_BROADCAST_ADDRESS       0xFFFFFFFF/*RIP广播地址255.255.255.255*/

#define RIP_MAX_ROUTE_NUM           1024 /*缺省路由表最大个数*/
#define RIP_MAX_NH_NUM              4    /*只对学习到的distance和metric一致的路由条目计数*/

#define MSG_RIP_UPDATE_TIMEOUT      0x10100001
#define MSG_RIP_EXPIRE_TIMEOUT      0x10100002
#define MSG_RIP_HOLDDOWN_TIMEOUT    0x10100003
#define MSG_RIP_TRIGGERED_TIMEOUT   0x10100004
#define MSG_RIP_GARBAGE_TIMEOUT		0x10100005
#define MSG_RIP_PEER_TIMEOUT		0x10100006
#define MSG_RIP_INTF_KEY_START      0x10100007
#define MSG_RIP_INTF_KEY_TIMEOUT	0x10100008        /*clakr add 2010-09-17*/
#define MSG_RIP_INTF_AUTH_COMMIT_TIMEOUT  0x10100009  /* ygc add 2011-12-29 */

#define RIP_INTF_REF_NUM_ADD_1	    1 /*统计计数加1*/
#define RIP_INTF_REF_NUM_SUB_1	    2 /*统计计数减1*/
#define RIP_INTF_REF_NUM_SUB_ALL	3 /*统计计数减去该接口下的全部计数*/
   
#define RIP_INTF_ADD_TO_PROCESS	    1 /*将端口添加到进程端口链表中*/
#define RIP_INTF_DEL_FROM_PROCESS	2 /*将端口从进程的端口链表中删除*/

#define RIP_ROUTE_INTF_PROCESS_ENABLE	1/*进程在端口上enable*/
#define RIP_ROUTE_INTF_PROCESS_DISABLE	2/*进程在端口上disable*/
#define RIP_ROUTE_INTF_ADDRESS_ADD		3/*端口添加地址*/
#define RIP_ROUTE_INTF_ADDRESS_DLE		4/*端口删除地址*/
#define RIP_ROUTE_INTF_PROTO_UP			5/*端口协议up*/
#define RIP_ROUTE_INTF_PROTO_DOWN		6/*端口协议down*/
/*#define RIP_ROUTE_NET_CMD_ADD			7配置network命令*/
/*#define RIP_ROUTE_NET_CMD_DEL			8取消配置network命令*/
#define RIP_ROUTE_INTF_VRF_ADD		    9
#define RIP_ROUTE_INTF_VRF_DEL			10

/*for intf 中lstate*/
#define RIP_INTF_LINK_UP				0x01/*链路协议UP*/
/*#define RIP_INTF_LINK_DOWN  			0x02*//*链路协议DOWN*/
#define RIP_INTF_PROCESS_ACTIVE	        0x04/*porcesss端口是激活的*/

#define RIP_CMD_MASKBIT                 1

typedef struct rip_route_activate_msg_
{
	uint8 type;

	union 
	{
		struct intf_
		{
			uint32 device_index;
			uint32 warning;
		}intf;
		
		struct network_
		{
			uint32 network;
			uint32 mask;
		}network;
	}u;
}rip_route_activate_msg_t;

/*枚举类型定义*/

/*RIP消息类型*/
enum RIP_MSG_TYPE
{
	RIP_MSG_UPDATE_TIMEOUT = 1,
	RIP_MSG_INVALID_TIMEOUT = 2,
	RIP_MSG_HOLDDOWN_TIMEOUT = 3,
	RIP_MSG_TRIGGER_TIMEOUT = 4,
	RIP_MSG_TYPE_BUTT
};

/*RIP版本号*/
enum RIP_VERSION
{
	RIP_VERSION_DEF = 0,
	RIP_VERSION_1 = 1,
	RIP_VERSION_2 = 2,
	RIP_VERSION_BUTT
};

/*RIP 汇总功能*/
enum RIP_AUTO_SUMMARY
{
	RIP_AUTO_SUMMARY_ENABLE = 1,
	RIP_AUTO_SUMMARY_DISABLE = 2,
	RIP_AUTO_SUMMARY_BUTT
};

/*RIP 缺省路由功能*/
enum RIP_DEF_ROUTE_
{
	RIP_DEF_ROUTE_DISABLE = 0,
	RIP_DEF_ROUTE_ENABLE = 1,
	RIP_DEF_ROUTE_ENABLE_SAFE = 2,
	RIP_DEF_ROUTE_BUTT
};

/*RIP源地址验证*/
enum RIP_SRC_CHECK
{
	RIP_SRC_CHECK_ENABLE = 1,
	RIP_SRC_CHECK_DISABLE = 2,
	SRC_CHECK_BUTT
};

#define RIP_SRC_CHECK_ENABLE         0x01/*RIP源地址验证*/
#define RIP_ZERO_DOMAIN_CHECK_ENABLE 0x02 /*开启零域检查*/

/*RIP认证类型*/
enum RIP_AUTH_TYPE
{
	RIP_AUTH_NONE = 1,    /*不对报文进行认证*/
	RIP_AUTH_SIMPLE = 2,  /*明文认证*/
	RIP_AUTH_MD5 = 3,     /*MD5认证*/
	RIP_AUTH_DYNAMIC = 3, /*邻居间动态认证*/
	RIP_AUTH_TYPE_BUTT
};

/*md5 key 加密存储*/
#define RIP_AUTH_MD5_KEY_NORMAL       4
#define RIP_AUTH_MD5_KEY_HIDDEN       8

/*RIP进程状态*/
enum RIP_STATE
{
	RIP_DISABLE = 0, /*RIP进程未启动*/
	RIP_ENABLE   = 1, /*RIP进程已启动*/
	RIP_STATE_BUTT
};

/*RIP报文命令*/
enum RIP_PKT_TYPE
{
	RIP_REQUEST  = 1,/*RIP request报文*/
	RIP_RESPONSE = 2,/*RIP response报文*/
	RIP_PKT_TYPE_BUTT
};

/*水分分割方法*/
#define RIP_SPLIT_HORIZON_DEF                           0x01
#define RIP_SIMPLE_SPLIT_HORIZON_ENABLE                 0x01
#define RIP_POISONED_SPLIT_HORIZON_ENABLE               0x02
/*
#define RIP_SIMPLE_SPLIT_HORIZON_DISABLE                0x04
#define RIP_POISONED_SPLIT_HORIZON_DISABLE              0x08
*/
/*
enum RIP_SPLIT_HORIZON_FLAG
{
	RIP_SPLIT_HORIZON_DEF = 1,
	RIP_SIMPLE_SPLIT_HORIZON_ENABLE = 2,
	RIP_SIMPLE_SPLIT_HORIZON_DISABLE = 3,
	RIP_POISONED_SPLIT_HORIZON_ENABLE = 4,
	RIP_POISONED_SPLIT_HORIZON_DISABLE = 5,
	RIP_SPLIT_HORIZON_FLAG_BUTT
};*/

/*接口地址类新*/
enum RIP_ADDRESS_TYPE
{
	RIP_PRIMARY_ADDRESS    = 1,   /*主地址*/
	RIP_UNNUMBERED_ADDRESS = 2,   /*unnumbered地址*/
	RIP_PPPLOCAL_ADDRESS   = 3,   /*ppp地址*/
	ADDRESS_TYPE_BUTT
};

/*链路层协议状态*/
/*enum RIP_LINK_STATE
{
	RIP_LINK_UP    = 1,/*链路协议UP*/
/*	RIP_LINK_DOWN = 2,/*链路协议DOWN*/
/*	RIP_LINK_STATE_BUTT
};*/

/*RIP接口发送RIP报文的版本类型*/
enum RIP_SEND_VERSION
{
	RIP_SEND_VERSION_DEF         = 0,/*缺省发送RIP版本 newcode */
	RIP_SEND_VERSION_1           = 1,/*发送RIPv1报文*/
	RIP_SEND_VERSION_2_MULTICAST = 2,/*以组播形式发送RIPv2报文*/
	RIP_SEND_VERSION_2_BROADCAST = 3,/*以广播形式发送RIPv2报文*/
	RIP_SEND_VERSION_BUTT
};

/*RIP接口接收RIP报文的版本类型*/
enum RIP_RECV_VERSION
{
	RIP_RECV_VERSION_DEF   = 0,/*缺省接收RIP版本 newcode*/        // 设置为默认，则查询全局RIP process设置的RIP版本
	RIP_RECV_VERSION_1     = 1,/*接收RIP版本1报文*/
	RIP_RECV_VERSION_2     = 2,/*接收RIP版本2报文*/
	RIP_RECV_VERSION_1_2   = 3,/*接收RIP版本1和版本2报文*/
	RIP_RECV_VERSION_BUTT
};

/*RIP路由类型*/
enum RIP_ROUTE_TYPE_
{
	RIP_NBR_ROUTE = 0x01,     /*邻居路由*/
	RIP_CONNECT_ROUTE = 0x02, /*直连路由*/
	RIP_REDIS_ROUTE = 0x04,   /*转发路由*/
	RIP_SUMMARY_ROUTE = 0x10, /*汇总路由*/
	RIP_DEF_ROUTE = 0x20,     /*缺省路由*/
	RIP_ROUTE_TYPE_BUTT
};

/*RIP路由标识*/
enum RIP_ROUTE_TAG
{
	RIP_INTERNAL_ROUTE = 1,/*域内路由*/
	RIP_EXTERNAL_ROUTE = 0,/*域外路由*/
	RIP_ROUTE_TAG_BUTT
};

/*RIP 被动接口设置*/
/*enum RIP_INTF_PASSIVE
{
	RIP_PASSIVE_ENABLE  = 1,/*被动接口*/
/*	RIP_PASSIVE_DISABLE = 2,/*非被动接口*/
	/*RIP_INTF_PASSIVE_BUTT
};*/


/* 为了配合mib中出现的多个选项，将passive_flag按位应用，并
增加如下选项
 * Commented by dangzhw in 2009.11.11 09:57:49 */
#define RIP_PASSIVE_ENABLE		0x01/*被动接口*/
#define RIP_NOTRECV_ENABLE		0x02/*不接受rip报文的接口*/
#define RIP_SEND_V1DEMAND		0x04/*在发送请求的时候，只发送v1的请求*/
#define RIP_SEND_V2DEMAND		0x08/*在发送请求的时候，只发送v2的请求*/

/*Offset direction*/
enum RIP_OFFSET_DIRECTION
{
	RIP_OFFSET_IN = 1,
	RIP_OFFSET_OUT = 2
};

/*RIP存储类型*/
enum RIP_MALLOC_TYPE
{
	RIP_INTF_TYPE = 1,
	RIP_NET_LIST_TYPE,
	RIP_NEIGH_LIST_TYPE,
	RIP_PEER_LIST_TYPE,
	RIP_DISTANCE_LIST_TYPE,
	RIP_NEIGHB_KEY_TYPE,

	RIP_ROUTE_TYPE,
	RIP_SUB_ROUTE_TYPE,
	RIP_RT_GATEWAY_TYPE,
	RIP_TRIGGER_LIST_TYPE,
	RIP_ROUTE_LIST_TYPE,

	RIP_MD5_RECORD_TYPE,

	RIP_TBL_INFO_TYPE,

	RIP_REDIS_LIST_TYPE,

	RIP_OFFSET_LIST_TYPE,


	RIP_PROCESS_LIST,
	RIP_PROCESS_INFO,
	RIP_PROCESS_INTF_LIST,

	RIP_MAX_MALLOC_TYPE
};

/*RIP NEIGHBOR KEY 状态*/
enum RIP_NEIGHBOR_KEY_STATE
{
	RIP_NEIGHBOR_KEY_VALID = 1,/*待生效的*/
	RIP_NEIGHBOR_KEY_ACTIVE = 2,/*正在生效的key*/
	RIP_NEIGHBOR_KEY_EXPIRED = 3,/*过期无效的key*/
	RIP_NEIGHBOR_KEY_EXTENSIONUSE = 4,/*超时延期使用的key*/
};

/*debug信息*/
#define RIP_DEBUG_IP_RIP_DATABASE   0x01 	/*debug ip rip database*/
#define RIP_DEBUG_IP_RIP_PROTO_SEND 0x02 	/*debug ip rip protocol send*/
#define RIP_DEBUG_IP_RIP_PROTO_RECV 0x04 	/*debug ip rip protocol receive*/
#define RIP_DEBUG_IP_RIP_MSG        0x08    /*debug ip rip message*/
#define RIP_DEBUG_IP_RIP_PROTOCOL   RIP_DEBUG_IP_RIP_PROTO_SEND|RIP_DEBUG_IP_RIP_PROTO_RECV /*debug ip rip protocol*/
#define RIP_DEBUG_IP_RIP_RETURN	    0x10
#define RIP_DEBUG_ALL               0x0f    

/*Fengsb 2006It's used to store the summaried connect route*/
typedef struct rip_subrt_
{
	struct rip_subrt_  *forw, *back;
	struct rip_route_  *rip_route;
}rip_subrt_t;


/*存储RIP路由信息*/
typedef struct rip_route_
{
	struct rip_route_ *forw, *back;
	
	struct route_node *route_node;

	struct rip_process_info_ *pprocess; /*路由归属的rip进程*/
	/*uint32 process_id;*/
	uint32 gw_addr;/*网关地址*/
	uint32 gw_index;/*路由对应的端口*/	
	uint32 next_hop;	
	uint32 distance;/*RIP 路由管理距离*/
	uint32 metric;/*RIP 路由metric值*/
	uint16 route_tag;/*RIP路由标识*/
	uint16 route_type;/*路由类型:汇总, 直连, 转发, 学习*/

	int32 refresh_time;/*路由刷新时间*/
	uint32 equi_route_num;/*等价路由数*/	
	uint32 equi_nbr_num;/*从邻居学习到的等价路由数目，只针对邻居的同distance和metric的路由信息*/
	
	/*Fengsb add 2006-05-23  */
	rip_subrt_t rip_connect; /* list for subnet connect route */
	uint32 sum_cnt;/*本路由所汇总的直连(子网) 路由数目*/
	uint16 b_supress; /*是否被汇总，用于直连路由，目前暂不使用*/
	uint16 rmap_set_metric; /* metric值是否为route-map所设 */

	struct rip_route_list_ *hold_ptr;/*指向holddown list中的指针*/
    struct rip_route_list_ *nbr_ptr; /* point to the nbr list on the interface */
	struct rip_route_list_ *red_ptr;/*Fengsb 2006-02-19 指向redistribute list中entry的指针*/
}rip_route_t;

/*RIP路由链表*/
typedef struct rip_route_list_
{
	struct rip_route_list_ *forw, *back;
	
	struct rip_route_ *rip_route;
}rip_route_list_t;


/*RIP触发更新列表*/
typedef struct rip_trigger_list_
{
	struct rip_trigger_list_ *forw, *back;
    
	/*Fengsb 2006-02-13 add: if  route_node==NULL, means the route is rmv frm rip local table
	so update timeout, this entry can't be compressed */
	struct route_node *route_node; 
	uint32 network;
	uint32 mask;
	uint32 next_hop;

	uint32 metric;

	uint32 gw_index;
	uint32 gw_addr;

	uint16 route_type;
	uint16 route_tag;
}rip_trigger_list_t;

enum RIP_NEIGHBOR_AUTHEN
{
      RIP_NEIGHB_MD5 = 1,
      RIP_NEIGHB_SHA1 = 2
};

/*动态key有效时间长度*/
typedef struct key_lift_time_
{
	uint16 hour;/*小时*/
	uint16 minute;/*分*/
}key_lift_time;


/*动态key开始生效的时间结构*/
typedef struct key_start_time_
{
	uint16 year;  /*年*/
	uint16 month; /*月*/
	uint16 day;   /*日*/
	uint16 hour;  /*小时*/
	uint16 minute;/*分*/
	uint16 second;/*秒*/
}key_start_time;


/*邻居的动态key链表*/
typedef struct dynamic_key_list_
{
	struct dynamic_key_list_ *forw, *back;/*key链表*/
	uint32 key_state;/*key的状态[超时|激活|为使用|延期]*/
	uint8 key_id;/*key id*/
	uint8 algorithms;/*算法*/
	uint8 md5_type;/*md5 key值存储类型*/
	char key[20+1];/*key值*/
	struct key_start_time_ start_time;/*开始生效时间*/
	struct key_lift_time_ lift_time;/*有效时间长度*/
}key_list_t;


/*RIP 邻居列表*/
typedef struct rip_neigh_list_
{
	struct rip_neigh_list_ *forw, *back;
	uint32 neigh_addr;
}rip_neigh_list_t;

typedef struct rip_peer_list_
{
	struct rip_peer_list_ *forw, *back;

	uint32 peer_addr;
	uint16 peer_port;
	uint16 peer_domain;/*保存该peer 收到的上一个报文的routing domain域的值*/
	uint32 peer_intf;   // 记录邻居发送给当前设备的端口
	
	uint32 rip2PeerVersion;
	int32  rip2PeerLastUpdate;
	
	uint   rip2PeerRcvPackets;/*add 2019.8.30*/
	uint   rip2PeerRcvBadPackets;
	
	uint   rip2PeerRcvRoutes;/*add 2019.8.30*/
	uint   rip2PeerRcvBadRoutes;

	int	   ref_num; /*该peer接收到的路由数目*/
	int    auth_state; /* 0:通过认证, 1:等待认证 */
}rip_peer_list_t;
#if 0
/*RIP网络列表*/
typedef struct rip_net_list_
{
	struct rip_net_list_ *forw, *back;
	uint32 network;
	uint32 mask;
}rip_net_list_t;
#endif
/*RIP offset列表*/
typedef struct rip_offset_list_
{
	struct rip_offset_list_ *forw, *back;

	uint32 decive_index;
	uint8 direct_flag;

	char acclist_name[RIP_ACCLIST_NAME_LEN];
	uint8 offset_val;
	
}rip_offset_list_t;

/*转发路由信息*/
typedef struct rip_redis_list_
{
	struct rip_redis_list_ *forw, *back;

	uint32 proto;
	uint32 process;
	struct route_map *redis_route_map;
	
	struct rip_route_list_ rip_route_list;/*转发的路由列表*/
}rip_redis_list_t;

/*保存MD5信息*/
typedef struct rip_md5_record_
{
	struct rip_md5_record_ *forw, *back;
	uint32 src_addr;              /*网关地址*/
	uint32 sequence_number;
}rip_md5_record_t;

/*rip所有进程的链表*/
typedef struct rip_process_list_
{
	struct rip_process_list_ *forw;
	struct rip_process_list_ *back;

	struct rip_process_info_ *rip_process_info;
}rip_process_list_t;

/*rip进程的接口链表*/
typedef struct rip_process_intf_list_
{
	struct rip_process_intf_list_ *forw;
	struct rip_process_intf_list_ *back;

	struct rip_intf_ *pintf;/*关联全局的rip接口数组中的元素*/
} rip_process_intf_list_t;

/*RIP端口信息*/
typedef struct rip_intf_
{
	uint32 vrf_id;                     /*端口对应的VRF*/        
	uint32 process_id;                 /*被使能的进程号*/
	uint32 device_index;	           /* 接口ID*/
	struct rip_process_info_ *pprocess;

/*	int ref_num;/*该端口被RIP网络覆盖的次数*/

	uint32 encap_type;                 /* 接口封装类型*/
	uint8 state;/*端口链路状态*/

	uint8 addr_type;/*端口地址类型*/
	uint32 address;/* 接口IP地址*/	
	uint32 mask; /* 接口IP地址掩码*/
	uint32 aid;	/* address id ，当在接口配置多个地址时，表示其id*/

	char intf_name[ INTERFACE_DEVICE_MAX_NAME];/*端口名字*/
	
	uint8 split_flag;/*标志位SPLIT_HORIZON_FLAG*/
	uint8 special_flag;/*passive,notreceive,v1demand,v2demand--dangzhw,20091111*/
	uint8  send_version;/*enum RIP_SEND_VERSION*/
	uint8  recv_version;/*enum RIP_RECV_VERSION*/

	uint8 auth_type;/*端口认证类型*/
	uint8 md5_type;/*md5 key的存储类型*/
	char simple_key[16+1];
	char md5_key[16+1];
	uint8 md5_keyid;

	struct rip_route_ *connect_route;/*端口对应的直连路由*/

	struct rip_route_list_ nbr_route_list;/*该端口学习到的路由链表*/

	uint rip2IfStatRcvBadPackets;/*端口接收到的错误报文数*/
	uint rip2IfStatRcvBadRoutes;/*端口接收到的错误路由数*/
	uint rip2IfStatSentUpdates;/*端口发送的更新报文数*/
	uint rip2IfStatRecvUpdates;/*端口收到的更新报文数*/
	
	int nbr_route_num;
	uint32 auth_commit_timer_id;
	uint32 auth_commit_time;
	/* 端口enable bfd,just for show*/
	uint8 bfd_enable_flag;

	/* Commented by dangzw in 2009.03.04 19:10:01 */
	uint8 dynamicflag;
	uint32 sequence;/*序列号*/
	uint32 key_start_timer_id;/*key开始生效定时器*/
	uint32 key_lift_timer_id;/*key有效时间长度定时器*/
	struct dynamic_key_list_  key_list;/*邻居的key链表*/
	struct dynamic_key_list_  key_timeout_list;/*邻居间超时的key链表*/
	
}rip_intf_t;

// RIP相关的全局信息
typedef struct rip_glb_info_
{
	int socket_id;   /*RIP socket id,全局共用一个socket*/

	uint32 queue_id;
	uint32 sem_id;   /*semphore id*/
	uint32 input_queue;

	uint8 debug_flag;
}rip_glb_info_t;

/*RIP全局信息*/
typedef struct rip_process_info_
{
	uint32 process_id;
	uint32 vrf_id;

	struct rip_process_list_ *pprocess_list;

	uint8 version;            /*全局版本号*/
	uint8 flags;              /*源地址检查，零域检查等的标志*/
	uint8 auto_summary_flag;  /*自动汇总标志位*/

	uint8 default_route_flag; /*default-information originate标志位*/
	uint16 default_metric;    /*RIP缺省metric值*/

	uint32 update_timeout;    /*配置的RIP update超时时间*/
	uint32 expire_timeout;    /*配置的RIP invalid超时时间*/
	uint32 holddown_timeout;  /*配置的RIP holddown超时时间*/
	uint32 trigger_timeout;   /*Fensb add for configure trigger timeout*/
	uint32 peer_timeout;      /*peer timeout time --dangzhw*/

	uint32 update_timer_id;   /*RIP update定时器ID*/
	uint32 expire_timer_id;   /*RIP invalid定时器ID*/
	uint32 holddown_timer_id; /*RIP holddown定时器ID*/
	uint32 trigger_timer_id;  /*RIP触发更新定时器ID*/
	uint32 peer_timer_id;     /*RIP peer超时定时器ID*/
	
	struct rip_route_list_  holddown_list;  /*处于holddown状态的路由列表*/
	/*struct rip_trigger_list_ trigger_list; *//*RIP 触发更新路由列表*/
	struct route_table * trigger_table ;    /*用于建立二叉树的触发表*/

	struct rip_neigh_list_ neigh_list;      /*所配置的邻居信息列表*/
	struct rip_peer_list_ peer_list;        /*动态peer 信息列表*/
	struct rip_md5_record_ md5_record;
		
    uint rip2GlobalRouteChanges; /* init as 0*/
    uint rip2GlobalQueries ;     /* init as 0*/

	uint32 max_nh_num;/*这里只对从邻居学习到的路由生效，并且要求这些路由具有同样的distance和metric，dangzhw2010325*/
	/*xuhaiqing 2010-12-21号将最大路由条目数限制取消*/
	/*uint32 max_route_num;*//*RIP路由表最大路由数*/
	uint32 route_num;       /*RIP路由表中路由数*/
	
/*	uint32 totle_intf_ref;/*记录所有的intf被rip覆盖的次数，当不大于0时，不接受数据，dangzhw20091106*/

	int connect_route_num;  /*路由表直连路由数目*/
	int sum_route_num;      /*路由表汇总路由数目*/
	int nbr_route_num;      /*学习路由数目*/
	int redis_route_num;    /*转发路由数目*/
	int holddown_route_num; /*holddown路由数目*/	

	/*struct rip_net_list_ network_list;RIP网络列表*/
	struct route_distribute_list *filter_list;  /*RIP过滤列表*/
	struct route_distance_list *distance_list;  /* RIP管理距离列表*/
	struct route_offset_list *offset_list;      /*RIP offset-list列表*/

	struct link_protocol *rip_redis_link;       /*RIP转发信息*/
	struct rip_redis_list_ redis_list;          /*记录转发的信息*/

	struct route_table *rip_table;              /*RIP本地路由表*/

	struct rip_process_intf_list_ intf_list;    /*用来保存本进程有效的interface*/
}rip_process_info_t;

/*RIP报文头部数据结构*/
typedef struct rip_pkt_head_
{
	uint8  cmd;    /*报文类型    */
	uint8  version;/*报文版本类型*/
	uint16 zero;   /*零项  */
}rip_pkt_head_t;

/*RIP路由条目数据结构*/
typedef struct rip_route_item_
{
	 uint16  afi;          /*AFI*/
	 uint16  route_tag;    /*路由标记:域内路由/域外路由*/
	 uint32  network;      /* */
	 uint32  mask;         /*网络掩码*/
	 uint32  next_hop;     /*下一跳地址*/
	 uint32  metric;       /*Metric值*/
}rip_route_item_t;

/*
   |             0xFFFF            | AuType=Keyed Message Digest   |
   +-------------------------------+-------------------------------+
   |    RIP-2 Packet Length        |    Key ID    | Auth Data Len  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |               Sequence Number (non-decreasing)                |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |               reserved must be zero                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |               reserved must be zero                           |

*/

/*rip authen 报文头部*/
typedef struct rip_authen_head_
{
	uint16 afi;
	uint16 authen_type;
	uint16 packet_len;    /*不包括报文尾部*/
	uint8 keyid;
	uint8 authen_data_len;/*尾部信息长度*/
	int32 sequence_number;
	uint32 zero1;
	uint32 zero2;
}rip_authen_head_t;

/*MD5报文头部*/
typedef struct rip_md5_head_
{
	uint16 afi;
	uint16 authen_type;
	uint16 packet_len;    /*不包括MD5报文尾部*/
	uint8 md5_keyid;
	uint8 authen_data_len;/*MD5尾部信息长度*/
	int32 sequence_number;
	uint32 zero1;
	uint32 zero2;
}rip_md5_head_t;

/*
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |             0xFFFF            |       0x01                    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /  Authentication Data (var. length; 16 bytes with Keyed MD5)   /
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
/*MD5报文尾部*/
typedef struct rip_md5_tail_
{
	uint16 afi;
	uint16 route_tag;
	char password[16];
}rip_md5_tail_t;

/*rip动态认证的尾部*/
typedef struct rip_authen_tail_
{
	uint16 afi;
	uint16 route_tag;
	union 
	{
		char md5[16];
		char sha1[20];
	}key;
}rip_authen_tail_t;

/*simple报文头部*/
typedef struct rip_simple_head_
{
	uint16 afi;
	uint16 authen_type;
	char password[16];
}rip_simple_head_t;


/*RIP控制信息*/
typedef struct rip_route_info_
{
	uint32 network;
	uint32 mask;
	uint32 next_hop;
	
	uint32 gw_index;
	uint32 gw_addr;

	uint16 route_type;
	uint16 route_tag;
	uint32 metric;
}rip_route_info_t;

void RipMain( void );
uint32 rip_get_mask( uint32 network_num );
uint32 rip_get_natural_mask( uint32 network );
uint32 rip_get_natural_network( uint32 network );
uint32 rip_set_distance( struct rip_process_info_ *pprocess, uint32 network, uint32 mask, uint32 gw_addr );
int rip_set_send_version( uint32 device_index );
int rip_get_recv_version( uint32 device_index );
BOOL rip_enable_split( uint32 device_index );
BOOL rip_enable_simple_split( uint32 device_index );
BOOL rip_enable_poisoned_split( uint32 device_index );
int rip_auth_commit_timeout(uint32 device_index);

BOOL rip_enable_auto_summary( struct rip_process_info_ *pprocess,uint32 device_index );
int rip_lookup_process_byprocessid( uint32 processid ,struct rip_process_info_ **pprocess);
int rip_process_route_activate(struct rip_process_info_ *pprocess , struct rip_route_activate_msg_  *msg);
int rip_relate_intf_to_process(uint32 device_index , struct rip_process_info_ *pprocess ,uint8 handle_type);
extern void rip_neighb_timer_key_start_check(uint32 device_index);
extern void rip_neighb_key_life_out(uint32 device_index);

int rip_intf_bfd_callback(uint32 device_index,uint32 up);
int rip_intf_bfd_register(uint32 device_index,uint32 state);

/******************* END OF HEADER FILE *********************************/

#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* RIP_MAIN_H */
