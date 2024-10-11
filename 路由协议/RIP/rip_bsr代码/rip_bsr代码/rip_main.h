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

/*RIP���󷵻���*/
#define RIP_SUCCESS  0
#define RIP_FAIL -1

#define RIP_MEM_FAIL -2

#define RIP_MEM_STAT     1 /* routing module */

/*Fengsb add 2006-02-13*/
#define DEFAULT_EQUAL_RTNUM 4 /*��·�ɱ���ȱʡ�ȼ�·����Ŀ*/
#define MAX_EQUAL_RTNUM 8     /*��·�ɱ������ȼ�·����Ŀ*/

/*�꺯��*/
#if RIP_MEM_STAT
#define rip_mem_malloc( size, malloc_type ) mem_malloc( size, RT_RIP, malloc_type );
#define rip_mem_free( buffer, malloc_type ) mem_free( buffer, RT_RIP, malloc_type )
#else
#define rip_mem_malloc( size, malloc_type ) sys_mem_malloc( size );
#define rip_mem_free( buffer, malloc_type ) sys_mem_free( buffer )
#endif

/* �ڶ�rip intf lk���в�����ʱ��Ҫ����
 * Commented by dangzhw in 2010.02.01 10:46:40 */
/*#define rip_lock() \
{ \
} 
#define rip_unlock() \
{ \
}*/
#define RIP_LOCK mu_lock(rip_glb_info.sem_id , MU_WAIT , 0)
#define RIP_UNLOCK mu_unlock(rip_glb_info.sem_id)

/*�궨��*/
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

#define RIP_MAX_PACKET_SIZE         512  /*RIP������ֽ���      */

#define RIP_DEF_METRIC              1    /*RIPȱʡmetircֵ   */
#define RIP_MAX_METRIC              16   /*metric���ֵ   */
#define RIP_DIRECT_METRIC           0    /*RIPֱ��·��metircֵ     */

#define RIP_DEF_DISTANCE            120  /*RIPȱʡ�������      */
#define RIP_PORT                    520  /*RIP���Ķ˿ں�          */
#define RIP_MAX_DISTANCE            255  /*RIP����������ֵ*/

#define RIP_MULTICAST_ADDRESS       0xE0000009/*RIP�鲥��ַ224.0.0.9     */
#define RIP_BROADCAST_ADDRESS       0xFFFFFFFF/*RIP�㲥��ַ255.255.255.255*/

#define RIP_MAX_ROUTE_NUM           1024 /*ȱʡ·�ɱ�������*/
#define RIP_MAX_NH_NUM              4    /*ֻ��ѧϰ����distance��metricһ�µ�·����Ŀ����*/

#define MSG_RIP_UPDATE_TIMEOUT      0x10100001
#define MSG_RIP_EXPIRE_TIMEOUT      0x10100002
#define MSG_RIP_HOLDDOWN_TIMEOUT    0x10100003
#define MSG_RIP_TRIGGERED_TIMEOUT   0x10100004
#define MSG_RIP_GARBAGE_TIMEOUT		0x10100005
#define MSG_RIP_PEER_TIMEOUT		0x10100006
#define MSG_RIP_INTF_KEY_START      0x10100007
#define MSG_RIP_INTF_KEY_TIMEOUT	0x10100008        /*clakr add 2010-09-17*/
#define MSG_RIP_INTF_AUTH_COMMIT_TIMEOUT  0x10100009  /* ygc add 2011-12-29 */

#define RIP_INTF_REF_NUM_ADD_1	    1 /*ͳ�Ƽ�����1*/
#define RIP_INTF_REF_NUM_SUB_1	    2 /*ͳ�Ƽ�����1*/
#define RIP_INTF_REF_NUM_SUB_ALL	3 /*ͳ�Ƽ�����ȥ�ýӿ��µ�ȫ������*/
   
#define RIP_INTF_ADD_TO_PROCESS	    1 /*���˿���ӵ����̶˿�������*/
#define RIP_INTF_DEL_FROM_PROCESS	2 /*���˿ڴӽ��̵Ķ˿�������ɾ��*/

#define RIP_ROUTE_INTF_PROCESS_ENABLE	1/*�����ڶ˿���enable*/
#define RIP_ROUTE_INTF_PROCESS_DISABLE	2/*�����ڶ˿���disable*/
#define RIP_ROUTE_INTF_ADDRESS_ADD		3/*�˿���ӵ�ַ*/
#define RIP_ROUTE_INTF_ADDRESS_DLE		4/*�˿�ɾ����ַ*/
#define RIP_ROUTE_INTF_PROTO_UP			5/*�˿�Э��up*/
#define RIP_ROUTE_INTF_PROTO_DOWN		6/*�˿�Э��down*/
/*#define RIP_ROUTE_NET_CMD_ADD			7����network����*/
/*#define RIP_ROUTE_NET_CMD_DEL			8ȡ������network����*/
#define RIP_ROUTE_INTF_VRF_ADD		    9
#define RIP_ROUTE_INTF_VRF_DEL			10

/*for intf ��lstate*/
#define RIP_INTF_LINK_UP				0x01/*��·Э��UP*/
/*#define RIP_INTF_LINK_DOWN  			0x02*//*��·Э��DOWN*/
#define RIP_INTF_PROCESS_ACTIVE	        0x04/*porcesss�˿��Ǽ����*/

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

/*ö�����Ͷ���*/

/*RIP��Ϣ����*/
enum RIP_MSG_TYPE
{
	RIP_MSG_UPDATE_TIMEOUT = 1,
	RIP_MSG_INVALID_TIMEOUT = 2,
	RIP_MSG_HOLDDOWN_TIMEOUT = 3,
	RIP_MSG_TRIGGER_TIMEOUT = 4,
	RIP_MSG_TYPE_BUTT
};

/*RIP�汾��*/
enum RIP_VERSION
{
	RIP_VERSION_DEF = 0,
	RIP_VERSION_1 = 1,
	RIP_VERSION_2 = 2,
	RIP_VERSION_BUTT
};

/*RIP ���ܹ���*/
enum RIP_AUTO_SUMMARY
{
	RIP_AUTO_SUMMARY_ENABLE = 1,
	RIP_AUTO_SUMMARY_DISABLE = 2,
	RIP_AUTO_SUMMARY_BUTT
};

/*RIP ȱʡ·�ɹ���*/
enum RIP_DEF_ROUTE_
{
	RIP_DEF_ROUTE_DISABLE = 0,
	RIP_DEF_ROUTE_ENABLE = 1,
	RIP_DEF_ROUTE_ENABLE_SAFE = 2,
	RIP_DEF_ROUTE_BUTT
};

/*RIPԴ��ַ��֤*/
enum RIP_SRC_CHECK
{
	RIP_SRC_CHECK_ENABLE = 1,
	RIP_SRC_CHECK_DISABLE = 2,
	SRC_CHECK_BUTT
};

#define RIP_SRC_CHECK_ENABLE         0x01/*RIPԴ��ַ��֤*/
#define RIP_ZERO_DOMAIN_CHECK_ENABLE 0x02 /*����������*/

/*RIP��֤����*/
enum RIP_AUTH_TYPE
{
	RIP_AUTH_NONE = 1,    /*���Ա��Ľ�����֤*/
	RIP_AUTH_SIMPLE = 2,  /*������֤*/
	RIP_AUTH_MD5 = 3,     /*MD5��֤*/
	RIP_AUTH_DYNAMIC = 3, /*�ھӼ䶯̬��֤*/
	RIP_AUTH_TYPE_BUTT
};

/*md5 key ���ܴ洢*/
#define RIP_AUTH_MD5_KEY_NORMAL       4
#define RIP_AUTH_MD5_KEY_HIDDEN       8

/*RIP����״̬*/
enum RIP_STATE
{
	RIP_DISABLE = 0, /*RIP����δ����*/
	RIP_ENABLE   = 1, /*RIP����������*/
	RIP_STATE_BUTT
};

/*RIP��������*/
enum RIP_PKT_TYPE
{
	RIP_REQUEST  = 1,/*RIP request����*/
	RIP_RESPONSE = 2,/*RIP response����*/
	RIP_PKT_TYPE_BUTT
};

/*ˮ�ַָ��*/
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

/*�ӿڵ�ַ����*/
enum RIP_ADDRESS_TYPE
{
	RIP_PRIMARY_ADDRESS    = 1,   /*����ַ*/
	RIP_UNNUMBERED_ADDRESS = 2,   /*unnumbered��ַ*/
	RIP_PPPLOCAL_ADDRESS   = 3,   /*ppp��ַ*/
	ADDRESS_TYPE_BUTT
};

/*��·��Э��״̬*/
/*enum RIP_LINK_STATE
{
	RIP_LINK_UP    = 1,/*��·Э��UP*/
/*	RIP_LINK_DOWN = 2,/*��·Э��DOWN*/
/*	RIP_LINK_STATE_BUTT
};*/

/*RIP�ӿڷ���RIP���ĵİ汾����*/
enum RIP_SEND_VERSION
{
	RIP_SEND_VERSION_DEF         = 0,/*ȱʡ����RIP�汾 newcode */
	RIP_SEND_VERSION_1           = 1,/*����RIPv1����*/
	RIP_SEND_VERSION_2_MULTICAST = 2,/*���鲥��ʽ����RIPv2����*/
	RIP_SEND_VERSION_2_BROADCAST = 3,/*�Թ㲥��ʽ����RIPv2����*/
	RIP_SEND_VERSION_BUTT
};

/*RIP�ӿڽ���RIP���ĵİ汾����*/
enum RIP_RECV_VERSION
{
	RIP_RECV_VERSION_DEF   = 0,/*ȱʡ����RIP�汾 newcode*/        // ����ΪĬ�ϣ����ѯȫ��RIP process���õ�RIP�汾
	RIP_RECV_VERSION_1     = 1,/*����RIP�汾1����*/
	RIP_RECV_VERSION_2     = 2,/*����RIP�汾2����*/
	RIP_RECV_VERSION_1_2   = 3,/*����RIP�汾1�Ͱ汾2����*/
	RIP_RECV_VERSION_BUTT
};

/*RIP·������*/
enum RIP_ROUTE_TYPE_
{
	RIP_NBR_ROUTE = 0x01,     /*�ھ�·��*/
	RIP_CONNECT_ROUTE = 0x02, /*ֱ��·��*/
	RIP_REDIS_ROUTE = 0x04,   /*ת��·��*/
	RIP_SUMMARY_ROUTE = 0x10, /*����·��*/
	RIP_DEF_ROUTE = 0x20,     /*ȱʡ·��*/
	RIP_ROUTE_TYPE_BUTT
};

/*RIP·�ɱ�ʶ*/
enum RIP_ROUTE_TAG
{
	RIP_INTERNAL_ROUTE = 1,/*����·��*/
	RIP_EXTERNAL_ROUTE = 0,/*����·��*/
	RIP_ROUTE_TAG_BUTT
};

/*RIP �����ӿ�����*/
/*enum RIP_INTF_PASSIVE
{
	RIP_PASSIVE_ENABLE  = 1,/*�����ӿ�*/
/*	RIP_PASSIVE_DISABLE = 2,/*�Ǳ����ӿ�*/
	/*RIP_INTF_PASSIVE_BUTT
};*/


/* Ϊ�����mib�г��ֵĶ��ѡ���passive_flag��λӦ�ã���
��������ѡ��
 * Commented by dangzhw in 2009.11.11 09:57:49 */
#define RIP_PASSIVE_ENABLE		0x01/*�����ӿ�*/
#define RIP_NOTRECV_ENABLE		0x02/*������rip���ĵĽӿ�*/
#define RIP_SEND_V1DEMAND		0x04/*�ڷ��������ʱ��ֻ����v1������*/
#define RIP_SEND_V2DEMAND		0x08/*�ڷ��������ʱ��ֻ����v2������*/

/*Offset direction*/
enum RIP_OFFSET_DIRECTION
{
	RIP_OFFSET_IN = 1,
	RIP_OFFSET_OUT = 2
};

/*RIP�洢����*/
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

/*RIP NEIGHBOR KEY ״̬*/
enum RIP_NEIGHBOR_KEY_STATE
{
	RIP_NEIGHBOR_KEY_VALID = 1,/*����Ч��*/
	RIP_NEIGHBOR_KEY_ACTIVE = 2,/*������Ч��key*/
	RIP_NEIGHBOR_KEY_EXPIRED = 3,/*������Ч��key*/
	RIP_NEIGHBOR_KEY_EXTENSIONUSE = 4,/*��ʱ����ʹ�õ�key*/
};

/*debug��Ϣ*/
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


/*�洢RIP·����Ϣ*/
typedef struct rip_route_
{
	struct rip_route_ *forw, *back;
	
	struct route_node *route_node;

	struct rip_process_info_ *pprocess; /*·�ɹ�����rip����*/
	/*uint32 process_id;*/
	uint32 gw_addr;/*���ص�ַ*/
	uint32 gw_index;/*·�ɶ�Ӧ�Ķ˿�*/	
	uint32 next_hop;	
	uint32 distance;/*RIP ·�ɹ������*/
	uint32 metric;/*RIP ·��metricֵ*/
	uint16 route_tag;/*RIP·�ɱ�ʶ*/
	uint16 route_type;/*·������:����, ֱ��, ת��, ѧϰ*/

	int32 refresh_time;/*·��ˢ��ʱ��*/
	uint32 equi_route_num;/*�ȼ�·����*/	
	uint32 equi_nbr_num;/*���ھ�ѧϰ���ĵȼ�·����Ŀ��ֻ����ھӵ�ͬdistance��metric��·����Ϣ*/
	
	/*Fengsb add 2006-05-23  */
	rip_subrt_t rip_connect; /* list for subnet connect route */
	uint32 sum_cnt;/*��·�������ܵ�ֱ��(����) ·����Ŀ*/
	uint16 b_supress; /*�Ƿ񱻻��ܣ�����ֱ��·�ɣ�Ŀǰ�ݲ�ʹ��*/
	uint16 rmap_set_metric; /* metricֵ�Ƿ�Ϊroute-map���� */

	struct rip_route_list_ *hold_ptr;/*ָ��holddown list�е�ָ��*/
    struct rip_route_list_ *nbr_ptr; /* point to the nbr list on the interface */
	struct rip_route_list_ *red_ptr;/*Fengsb 2006-02-19 ָ��redistribute list��entry��ָ��*/
}rip_route_t;

/*RIP·������*/
typedef struct rip_route_list_
{
	struct rip_route_list_ *forw, *back;
	
	struct rip_route_ *rip_route;
}rip_route_list_t;


/*RIP���������б�*/
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

/*��̬key��Чʱ�䳤��*/
typedef struct key_lift_time_
{
	uint16 hour;/*Сʱ*/
	uint16 minute;/*��*/
}key_lift_time;


/*��̬key��ʼ��Ч��ʱ��ṹ*/
typedef struct key_start_time_
{
	uint16 year;  /*��*/
	uint16 month; /*��*/
	uint16 day;   /*��*/
	uint16 hour;  /*Сʱ*/
	uint16 minute;/*��*/
	uint16 second;/*��*/
}key_start_time;


/*�ھӵĶ�̬key����*/
typedef struct dynamic_key_list_
{
	struct dynamic_key_list_ *forw, *back;/*key����*/
	uint32 key_state;/*key��״̬[��ʱ|����|Ϊʹ��|����]*/
	uint8 key_id;/*key id*/
	uint8 algorithms;/*�㷨*/
	uint8 md5_type;/*md5 keyֵ�洢����*/
	char key[20+1];/*keyֵ*/
	struct key_start_time_ start_time;/*��ʼ��Чʱ��*/
	struct key_lift_time_ lift_time;/*��Чʱ�䳤��*/
}key_list_t;


/*RIP �ھ��б�*/
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
	uint16 peer_domain;/*�����peer �յ�����һ�����ĵ�routing domain���ֵ*/
	uint32 peer_intf;   // ��¼�ھӷ��͸���ǰ�豸�Ķ˿�
	
	uint32 rip2PeerVersion;
	int32  rip2PeerLastUpdate;
	
	uint   rip2PeerRcvPackets;/*add 2019.8.30*/
	uint   rip2PeerRcvBadPackets;
	
	uint   rip2PeerRcvRoutes;/*add 2019.8.30*/
	uint   rip2PeerRcvBadRoutes;

	int	   ref_num; /*��peer���յ���·����Ŀ*/
	int    auth_state; /* 0:ͨ����֤, 1:�ȴ���֤ */
}rip_peer_list_t;
#if 0
/*RIP�����б�*/
typedef struct rip_net_list_
{
	struct rip_net_list_ *forw, *back;
	uint32 network;
	uint32 mask;
}rip_net_list_t;
#endif
/*RIP offset�б�*/
typedef struct rip_offset_list_
{
	struct rip_offset_list_ *forw, *back;

	uint32 decive_index;
	uint8 direct_flag;

	char acclist_name[RIP_ACCLIST_NAME_LEN];
	uint8 offset_val;
	
}rip_offset_list_t;

/*ת��·����Ϣ*/
typedef struct rip_redis_list_
{
	struct rip_redis_list_ *forw, *back;

	uint32 proto;
	uint32 process;
	struct route_map *redis_route_map;
	
	struct rip_route_list_ rip_route_list;/*ת����·���б�*/
}rip_redis_list_t;

/*����MD5��Ϣ*/
typedef struct rip_md5_record_
{
	struct rip_md5_record_ *forw, *back;
	uint32 src_addr;              /*���ص�ַ*/
	uint32 sequence_number;
}rip_md5_record_t;

/*rip���н��̵�����*/
typedef struct rip_process_list_
{
	struct rip_process_list_ *forw;
	struct rip_process_list_ *back;

	struct rip_process_info_ *rip_process_info;
}rip_process_list_t;

/*rip���̵Ľӿ�����*/
typedef struct rip_process_intf_list_
{
	struct rip_process_intf_list_ *forw;
	struct rip_process_intf_list_ *back;

	struct rip_intf_ *pintf;/*����ȫ�ֵ�rip�ӿ������е�Ԫ��*/
} rip_process_intf_list_t;

/*RIP�˿���Ϣ*/
typedef struct rip_intf_
{
	uint32 vrf_id;                     /*�˿ڶ�Ӧ��VRF*/        
	uint32 process_id;                 /*��ʹ�ܵĽ��̺�*/
	uint32 device_index;	           /* �ӿ�ID*/
	struct rip_process_info_ *pprocess;

/*	int ref_num;/*�ö˿ڱ�RIP���縲�ǵĴ���*/

	uint32 encap_type;                 /* �ӿڷ�װ����*/
	uint8 state;/*�˿���·״̬*/

	uint8 addr_type;/*�˿ڵ�ַ����*/
	uint32 address;/* �ӿ�IP��ַ*/	
	uint32 mask; /* �ӿ�IP��ַ����*/
	uint32 aid;	/* address id �����ڽӿ����ö����ַʱ����ʾ��id*/

	char intf_name[ INTERFACE_DEVICE_MAX_NAME];/*�˿�����*/
	
	uint8 split_flag;/*��־λSPLIT_HORIZON_FLAG*/
	uint8 special_flag;/*passive,notreceive,v1demand,v2demand--dangzhw,20091111*/
	uint8  send_version;/*enum RIP_SEND_VERSION*/
	uint8  recv_version;/*enum RIP_RECV_VERSION*/

	uint8 auth_type;/*�˿���֤����*/
	uint8 md5_type;/*md5 key�Ĵ洢����*/
	char simple_key[16+1];
	char md5_key[16+1];
	uint8 md5_keyid;

	struct rip_route_ *connect_route;/*�˿ڶ�Ӧ��ֱ��·��*/

	struct rip_route_list_ nbr_route_list;/*�ö˿�ѧϰ����·������*/

	uint rip2IfStatRcvBadPackets;/*�˿ڽ��յ��Ĵ�������*/
	uint rip2IfStatRcvBadRoutes;/*�˿ڽ��յ��Ĵ���·����*/
	uint rip2IfStatSentUpdates;/*�˿ڷ��͵ĸ��±�����*/
	uint rip2IfStatRecvUpdates;/*�˿��յ��ĸ��±�����*/
	
	int nbr_route_num;
	uint32 auth_commit_timer_id;
	uint32 auth_commit_time;
	/* �˿�enable bfd,just for show*/
	uint8 bfd_enable_flag;

	/* Commented by dangzw in 2009.03.04 19:10:01 */
	uint8 dynamicflag;
	uint32 sequence;/*���к�*/
	uint32 key_start_timer_id;/*key��ʼ��Ч��ʱ��*/
	uint32 key_lift_timer_id;/*key��Чʱ�䳤�ȶ�ʱ��*/
	struct dynamic_key_list_  key_list;/*�ھӵ�key����*/
	struct dynamic_key_list_  key_timeout_list;/*�ھӼ䳬ʱ��key����*/
	
}rip_intf_t;

// RIP��ص�ȫ����Ϣ
typedef struct rip_glb_info_
{
	int socket_id;   /*RIP socket id,ȫ�ֹ���һ��socket*/

	uint32 queue_id;
	uint32 sem_id;   /*semphore id*/
	uint32 input_queue;

	uint8 debug_flag;
}rip_glb_info_t;

/*RIPȫ����Ϣ*/
typedef struct rip_process_info_
{
	uint32 process_id;
	uint32 vrf_id;

	struct rip_process_list_ *pprocess_list;

	uint8 version;            /*ȫ�ְ汾��*/
	uint8 flags;              /*Դ��ַ��飬������ȵı�־*/
	uint8 auto_summary_flag;  /*�Զ����ܱ�־λ*/

	uint8 default_route_flag; /*default-information originate��־λ*/
	uint16 default_metric;    /*RIPȱʡmetricֵ*/

	uint32 update_timeout;    /*���õ�RIP update��ʱʱ��*/
	uint32 expire_timeout;    /*���õ�RIP invalid��ʱʱ��*/
	uint32 holddown_timeout;  /*���õ�RIP holddown��ʱʱ��*/
	uint32 trigger_timeout;   /*Fensb add for configure trigger timeout*/
	uint32 peer_timeout;      /*peer timeout time --dangzhw*/

	uint32 update_timer_id;   /*RIP update��ʱ��ID*/
	uint32 expire_timer_id;   /*RIP invalid��ʱ��ID*/
	uint32 holddown_timer_id; /*RIP holddown��ʱ��ID*/
	uint32 trigger_timer_id;  /*RIP�������¶�ʱ��ID*/
	uint32 peer_timer_id;     /*RIP peer��ʱ��ʱ��ID*/
	
	struct rip_route_list_  holddown_list;  /*����holddown״̬��·���б�*/
	/*struct rip_trigger_list_ trigger_list; *//*RIP ��������·���б�*/
	struct route_table * trigger_table ;    /*���ڽ����������Ĵ�����*/

	struct rip_neigh_list_ neigh_list;      /*�����õ��ھ���Ϣ�б�*/
	struct rip_peer_list_ peer_list;        /*��̬peer ��Ϣ�б�*/
	struct rip_md5_record_ md5_record;
		
    uint rip2GlobalRouteChanges; /* init as 0*/
    uint rip2GlobalQueries ;     /* init as 0*/

	uint32 max_nh_num;/*����ֻ�Դ��ھ�ѧϰ����·����Ч������Ҫ����Щ·�ɾ���ͬ����distance��metric��dangzhw2010325*/
	/*xuhaiqing 2010-12-21�Ž����·����Ŀ������ȡ��*/
	/*uint32 max_route_num;*//*RIP·�ɱ����·����*/
	uint32 route_num;       /*RIP·�ɱ���·����*/
	
/*	uint32 totle_intf_ref;/*��¼���е�intf��rip���ǵĴ�������������0ʱ�����������ݣ�dangzhw20091106*/

	int connect_route_num;  /*·�ɱ�ֱ��·����Ŀ*/
	int sum_route_num;      /*·�ɱ����·����Ŀ*/
	int nbr_route_num;      /*ѧϰ·����Ŀ*/
	int redis_route_num;    /*ת��·����Ŀ*/
	int holddown_route_num; /*holddown·����Ŀ*/	

	/*struct rip_net_list_ network_list;RIP�����б�*/
	struct route_distribute_list *filter_list;  /*RIP�����б�*/
	struct route_distance_list *distance_list;  /* RIP��������б�*/
	struct route_offset_list *offset_list;      /*RIP offset-list�б�*/

	struct link_protocol *rip_redis_link;       /*RIPת����Ϣ*/
	struct rip_redis_list_ redis_list;          /*��¼ת������Ϣ*/

	struct route_table *rip_table;              /*RIP����·�ɱ�*/

	struct rip_process_intf_list_ intf_list;    /*�������汾������Ч��interface*/
}rip_process_info_t;

/*RIP����ͷ�����ݽṹ*/
typedef struct rip_pkt_head_
{
	uint8  cmd;    /*��������    */
	uint8  version;/*���İ汾����*/
	uint16 zero;   /*����  */
}rip_pkt_head_t;

/*RIP·����Ŀ���ݽṹ*/
typedef struct rip_route_item_
{
	 uint16  afi;          /*AFI*/
	 uint16  route_tag;    /*·�ɱ��:����·��/����·��*/
	 uint32  network;      /* */
	 uint32  mask;         /*��������*/
	 uint32  next_hop;     /*��һ����ַ*/
	 uint32  metric;       /*Metricֵ*/
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

/*rip authen ����ͷ��*/
typedef struct rip_authen_head_
{
	uint16 afi;
	uint16 authen_type;
	uint16 packet_len;    /*����������β��*/
	uint8 keyid;
	uint8 authen_data_len;/*β����Ϣ����*/
	int32 sequence_number;
	uint32 zero1;
	uint32 zero2;
}rip_authen_head_t;

/*MD5����ͷ��*/
typedef struct rip_md5_head_
{
	uint16 afi;
	uint16 authen_type;
	uint16 packet_len;    /*������MD5����β��*/
	uint8 md5_keyid;
	uint8 authen_data_len;/*MD5β����Ϣ����*/
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
/*MD5����β��*/
typedef struct rip_md5_tail_
{
	uint16 afi;
	uint16 route_tag;
	char password[16];
}rip_md5_tail_t;

/*rip��̬��֤��β��*/
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

/*simple����ͷ��*/
typedef struct rip_simple_head_
{
	uint16 afi;
	uint16 authen_type;
	char password[16];
}rip_simple_head_t;


/*RIP������Ϣ*/
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
