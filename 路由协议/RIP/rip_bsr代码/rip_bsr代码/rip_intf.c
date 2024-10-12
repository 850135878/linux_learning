char rip_intf_version[] = "rip_intf.c last changed at 2013.05.09 13:48:58 by yuguangcheng\n";
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
 *        FILE NAME       :   rip_intf.c
 *        AUTHOR          :   Fengsb
 *        DATE            :   2006.05.18 12:25:51
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#include <rip/rip_include.h>
#include <rip/rip_main.h>
#include <rip/rip_intf.h>
#include <rip/rip_debug.h>
#include <rip/rip_redis.h>
#include <rip/rip_packet.h>
#include <rip/rip_timer.h>
#include <rip/rip_init.h>

extern struct rip_glb_info_  rip_glb_info;
extern struct rip_intf_ *rip_intf_array[INTERFACE_DEVICE_MAX_NUMBER + 1];
extern int rip_intf_creat_timer(uint32 device_index);
extern struct rip_process_list_  rip_process_list;

/*extern int route_no_offset_list_cmd(unsigned long protocol,unsigned long process,DEVICE_ETERNAL_ID if_id,enum route_offset_type type,char *alist_name,unsigned long metric, uint32 vrfid);*/
extern int route_no_distribute_list_cmd(unsigned long protocol,unsigned long process,DEVICE_ETERNAL_ID if_id,enum route_distribute_type type,char *alist_name,char *gateway_name, char *plist_name, uint32 vrfid);
extern int route_no_offset_list_cmd(unsigned long protocol,unsigned long process,DEVICE_ETERNAL_ID if_id,enum route_offset_type type,char *alist_name,unsigned long metric, uint32 vrfid);

/*===========================================================
������: rip_task_callback
��������: RIP��Ϣ�ص�����
�������: procotol: Э���
          process: Э�����
          msg: �ص���Ϣ
�������:
����ֵ: ����ɹ�,����RIP_SUCCESS
                 ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_task_callback( uint32 procotol, uint32 process, struct rt_msg *msg )
{    
	struct MESSAGE_PARAM *rip_param = NULL;
	struct _rt_head *rth = NULL;
	uint32 device_index;
	uint32 address;
	uint32 mask;
	uint32 aid;
	uint8 addr_type = 0;
	uint32 vrf_id;    
	int ret = RIP_SUCCESS;
#if 0
	struct vrf_table_list_ *vrf_table = NULL;
#endif

	if( RTPROTO_RIP != procotol )
	{
		return RIP_FAIL;
	}

	switch ( msg->type )
	{
		case MSG_ROUTING_INTF_ADD:/*����RIP�ӿ�*/
			rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_INTF_ADD] message\n" );
			device_index = msg->uid;
			ret = rip_intf_add( device_index );
			break;

		case MSG_ROUTING_INTF_DEL:/*ɾ��RIP�ӿ�*/
			rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_INTF_DEL] message\n" );
			device_index = msg->uid;
			ret = rip_intf_del( device_index );
			break;

		case MSG_ROUTING_PROTO_UP:/*�˿���·״̬UP*/
			rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_PROTO_UP] message\n" );
			device_index = msg->uid;
			ret = rip_intf_link_up( device_index );
			break;

		case MSG_ROUTING_PROTO_DOWN:/*�˿���·״̬down*/
			rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_PROTO_DOWN] message\n" );
			device_index = msg->uid;
			ret = rip_intf_link_down( device_index);
			break;
	    
		case MSG_ROUTING_ADD_PPPLOCAL:	/*����PPPЭ���ַ*/			
		case MSG_ROUTING_ADD_PRIMARY:   /*������IP��ַ*/
		case MSG_ROUTING_ADD_UNNUMBERED:/*���Ӷ˿�unnumbered��ַ*/
			if( msg->type == MSG_ROUTING_ADD_PPPLOCAL )
			{
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_ADD_PPPLOCAL] message\n" );
				addr_type = RIP_PPPLOCAL_ADDRESS;
			}
			else if( msg->type == MSG_ROUTING_ADD_PRIMARY )
			{
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_ADD_PRIMARY] message\n" );
				addr_type = RIP_PRIMARY_ADDRESS;
			}
			else if( msg->type == MSG_ROUTING_ADD_UNNUMBERED )
			{
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_ADD_UNNUMBERED] message\n" );
				addr_type = RIP_UNNUMBERED_ADDRESS;
			}

			rip_param = (struct MESSAGE_PARAM *)msg->param;
			address = rip_param->data1.data_uint32;
			mask = rip_param->data2.data_uint32;
			device_index = rip_param->data3.data_uint32;
			aid = rip_param->data4.data_uint32;         /*Fengsb add 2007-07-12*/
			vrf_id = rip_param->data5.data_uint32;

			ret = rip_intf_addr_add( device_index, aid, address, mask, addr_type, vrf_id );
			break;	
	    
		case MSG_ROUTING_DEL_PRIMARY:
		case MSG_ROUTING_DEL_UNNUMBERED:
		case MSG_ROUTING_DEL_PPPLOCAL:
			if( msg->type == MSG_ROUTING_DEL_PRIMARY )
			{
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_DEL_PRIMARY] message\n" );
				addr_type = RIP_PRIMARY_ADDRESS;
			}
			else if( msg->type == MSG_ROUTING_DEL_UNNUMBERED )
			{
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_DEL_UNNUMBERED] message\n" );
				addr_type = RIP_UNNUMBERED_ADDRESS;
			}
			else if( msg->type == MSG_ROUTING_DEL_PPPLOCAL )
			{
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_DEL_PPPLOCAL] message\n" );
				addr_type = RIP_PPPLOCAL_ADDRESS;
			}
			rip_param = (struct MESSAGE_PARAM *)msg->param;
			address = rip_param->data1.data_uint32;
			mask = rip_param->data2.data_uint32;
			device_index = rip_param->data3.data_uint32;
			vrf_id = rip_param->data5.data_uint32;
			ret = rip_intf_addr_del( device_index, address, mask, addr_type, vrf_id );
			break;

		case MSG_VRF_DEL:
			rip_param = (struct MESSAGE_PARAM *)msg->param;
			vrf_id = rip_param->data1.data_uint32;
			rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_VRF_DEL] message(vrfid %d)\n",vrf_id );
			ret = rip_del_vrf_process( vrf_id );
			break;

		case MSG_VRF_INTF_ADD:
			rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_VRF_INTF_ADD] message\n" );
			rip_param = (struct MESSAGE_PARAM *)msg->param;
			device_index = rip_param->data1.data_uint32;
			vrf_id =rip_param->data2.data_uint32; 
			ret = rip_add_intf_vrf( vrf_id,device_index );
			break;

		case MSG_VRF_INTF_DEL:
			rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_VRF_INTF_DEL] message\n" );
			rip_param = (struct MESSAGE_PARAM *)msg->param;
			device_index = rip_param->data1.data_uint32;
			vrf_id = rip_param->data2.data_uint32;
			ret = rip_del_intf_vrf( vrf_id ,device_index);
			break;
	    
		case MSG_IPRT_TASKPOLICY_CHANGE:
			rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_IPRT_TASKPOLICY_CHANGE] message\n" );
			rip_param = (struct MESSAGE_PARAM *)(msg->param);
			rth = (struct _rt_head *)(rip_param->data1.data_uint32);
			ret = rip_route_change( rth );
			break;

		default:
			break;
	}
	if(ret != RIP_SUCCESS)return ret;
	return RIP_SUCCESS;
}
#if 0

/*******************************************************************************
 *
 * FUNCTION	:	rip_intf_ref_num_set
 *
 * PARAMS		:	device_index	- �߼��ӿ�����ֵ
 *					handle_type	- �������
 *
 * RETURN		:	��Ӧ�߼��ӿڱ�rip���ǵĴ���
 *
 * NOTE			:	��������ӿ���Ҫ��Ϊ��ͳ�ƽӿڱ�rip���ǵĴ�����
 					���е�һ���ӿڱ�rip���õ�����ǵ�ʱ�򣬽���ע��socket��
 					�ڽ������ϵı����ض���ͬʱ��û�нӿڱ�rip���ǵ�ʱ��ȡ�����ĵ��ض���
 					
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.02.01 16:42:19
 *
*******************************************************************************/
int rip_intf_ref_num_set(uint32 device_index, uint32 handle_type)
{
	switch(handle_type)
	{
		case RIP_INTF_REF_NUM_ADD_1:
			rip_intf_array[device_index]->ref_num ++;

			if(rip_glb_info.totle_intf_ref == 1)
				rip_socket_redirect_enable();
			/*
			if(rip_intf_array[device_index]->ref_num == 1)
				rip_intf_lk_set(device_index, RIP_INTF_LK_ADD);
			*/
			break;
		case RIP_INTF_REF_NUM_SUB_1:
			rip_intf_array[device_index]->ref_num --;
			rip_glb_info.totle_intf_ref --;

			#ifdef SWITCH
				if(rip_glb_info.totle_intf_ref == 0)
					rip_reset_pkt_redirect();
			#endif
			
			break;
		case RIP_INTF_REF_NUM_SUB_ALL:
			rip_glb_info.totle_intf_ref -=rip_intf_array[device_index]->ref_num;
			rip_intf_array[device_index]->ref_num = 0;

			#ifdef SWITCH
				if(rip_glb_info.totle_intf_ref == 0)
					rip_reset_pkt_redirect();
			#endif
			/*
			if(rip_intf_array[device_index]->ref_num == 0)
				rip_intf_lk_set(device_index, RIP_INTF_LK_DELETE);
			*/
			break;

		default:
			syslog(LOG_WARNING,"rip_intf_ref_num_set with error handle tye!\n");
			break;

	}

	return rip_intf_array[device_index]->ref_num;

}
#endif

/*===========================================================
������:      rip_intf_add
��������: �˿����Ӵ�����
�������: device_index: �߼��˿ں�
�������: ��
����ֵ:      ����ɹ�,����RIP_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_intf_add( uint32 device_index )
{
    char name_str[INTERFACE_DEVICE_MAX_NAME], *name = name_str;
	int ret;

    /*��������Ϸ��Լ��*/
    if( device_index > INTERFACE_DEVICE_MAX_NUMBER )
    {
        return RIP_FAIL;
    }

    if( NULL != rip_intf_array[device_index] )
    {
        /*�ö˿��Ѿ�����,���账��*/
        return RIP_SUCCESS;
    }

    /*�����ڴ�*/
    rip_intf_array[device_index] = (struct rip_intf_ *)rip_mem_malloc( sizeof(struct rip_intf_), RIP_INTF_TYPE );
    if( rip_intf_array[device_index] == NULL )
    {
        /*�ڴ����ʧ��*/
        fprintf(stderr, "RIP: Malloc memory failed.(%s, %d)\n", __FILE__, __LINE__ );
        return RIP_MEM_FAIL;
    }
    memset( rip_intf_array[device_index], 0, sizeof(struct rip_intf_) );

    /*��ʼ��*/
    rip_intf_array[device_index]->device_index = device_index;

    rip_intf_array[device_index]->send_version = RIP_SEND_VERSION_DEF;
    rip_intf_array[device_index]->recv_version = RIP_RECV_VERSION_DEF;

   /* rip_intf_array[device_index]->passive_flag = RIP_PASSIVE_DISABLE;*//*Ŀǰ���������λӦ�ã���Ӧλû����һ��ʾû��Ӧ�ø�ѡ��*/
    rip_intf_array[device_index]->auth_type = RIP_AUTH_NONE;

    rip_intf_array[device_index]->split_flag = RIP_SPLIT_HORIZON_DEF;

    /*��ʼ���˿�ѧϰ����·���б�*/
    rip_intf_array[device_index]->nbr_route_list.forw = &(rip_intf_array[device_index]->nbr_route_list);
    rip_intf_array[device_index]->nbr_route_list.back = &(rip_intf_array[device_index]->nbr_route_list);

	/* Commented by dangzw in 2009.03.04 19:48:18 */
	rip_intf_array[device_index]->key_list.forw = &( rip_intf_array[device_index]->key_list);
    rip_intf_array[device_index]->key_list.back = &( rip_intf_array[device_index]->key_list);

	rip_intf_array[device_index]->key_timeout_list.forw = &( rip_intf_array[device_index]->key_timeout_list);
    rip_intf_array[device_index]->key_timeout_list.back = &( rip_intf_array[device_index]->key_timeout_list);

	ret = rip_intf_creat_timer(device_index);
	if( RIP_SUCCESS != ret )
	{
		return RIP_FAIL;
	}

    name = rip_intf_array[device_index]->intf_name;
    get_if_namebyid( device_index, &name );

    rip_intf_array[device_index]->nbr_route_num = 0;    

    return RIP_SUCCESS;
}

int rip_intf_del_peers(uint32 device_index)
{
	struct rip_peer_list_ *peer,*next;
	struct rip_intf_ *pintf;
	struct rip_process_info_ *pprocess;
	int cnt=0;
	pintf=rip_intf_array[device_index];
	if(pintf==NULL)return 0;
	pprocess=pintf->pprocess;
	if(!pprocess)return 0;
	peer=pprocess->peer_list.forw;
	while(peer!=&(pprocess->peer_list))
	{
		next=peer->forw;
		if(peer->peer_intf==device_index)
		{
			cnt++;
			REMQUE(peer);
			rip_mem_free(peer,RIP_PEER_LIST_TYPE);
		}
		peer=next;
	}
	return cnt;
}

/*===========================================================
������:      rip_intf_del
��������: �˿�ɾ��������
�������: device_index: �߼��˿ں�
�������: ��
����ֵ:      ����ɹ�,����RIP_SUCCESS
                       ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_intf_del( uint32 device_index )
{
	int ret;
	struct dynamic_key_list_ *key, *keytemp;
	struct rip_route_list_ *rip_route_list_temp, *rip_route_list_forw;

	/*��������Ϸ��Լ��*/
	if( device_index > INTERFACE_DEVICE_MAX_NUMBER )
	{
		return RIP_FAIL;
	}

	if( NULL == rip_intf_array[device_index] )
	{
		/*�ö˿ڻ�û������,���账��*/
		return RIP_SUCCESS;
	}

	/*ɾ���ö˿�ѧϰ����·��*/
	rip_route_list_temp = rip_intf_array[device_index]->nbr_route_list.forw;
	while( rip_route_list_temp != &(rip_intf_array[device_index]->nbr_route_list))
	{
		rip_route_list_forw = rip_route_list_temp->forw;

		/*��holddown�б���ɾ��*/
		rip_del_holddown( rip_route_list_temp->rip_route );
		
		/*ɾ���Ӷ˿�ѧϰ����·��*/
		ret = rip_del_route( rip_route_list_temp->rip_route );
		if( RIP_SUCCESS != ret )
		{
			return ret;
		}

		/*��·��������ɾ��*/
		REMQUE( rip_route_list_temp );
		rip_mem_free( rip_route_list_temp, RIP_ROUTE_LIST_TYPE );

		rip_intf_array[device_index]->nbr_route_num--;

		rip_route_list_temp = rip_route_list_forw;
	};

	rip_relate_intf_to_process(device_index, rip_intf_array[device_index]->pprocess, RIP_INTF_DEL_FROM_PROCESS);

	/*ɾ���˿��ϵ�dynamic key*/
	key =rip_intf_array[device_index]->key_list.forw;
	while(key != &rip_intf_array[device_index]->key_list)
	{
		keytemp = key;
		key =key->forw;
		REMQUE( keytemp );
		rip_mem_free( keytemp, RIP_NEIGHB_KEY_TYPE );
	}

	key =rip_intf_array[device_index]->key_timeout_list.forw;
	while(key != &rip_intf_array[device_index]->key_timeout_list)
	{
		keytemp = key;
		key =key->forw;
		REMQUE( keytemp );
		rip_mem_free( keytemp, RIP_NEIGHB_KEY_TYPE );
	}

	sys_stop_timer(rip_intf_array[device_index]->key_start_timer_id);
	sys_delete_timer(rip_intf_array[device_index]->key_start_timer_id);

	sys_stop_timer(rip_intf_array[device_index]->key_lift_timer_id);
	sys_delete_timer(rip_intf_array[device_index]->key_lift_timer_id);

	sys_stop_timer(rip_intf_array[device_index]->auth_commit_timer_id);
	sys_delete_timer(rip_intf_array[device_index]->auth_commit_timer_id);

	rip_intf_del_peers(device_index);

	/*�ͷŶ˿�*/
	rip_mem_free( rip_intf_array[device_index], RIP_INTF_TYPE );
	rip_intf_array[device_index] = NULL;

	return RIP_SUCCESS;
}

/*===========================================================
������:      rip_intf_link_up
��������: �˿���·״̬UP������
�������: device_index: �߼��˿ں�
�������: ��
����ֵ:      ����ɹ�,����RIP_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_intf_link_up( uint32 device_index )
{
	int ret;
	unsigned long device_type;
	struct rip_route_activate_msg_ msg;
	struct rip_process_info_ *pprocess;
	/*��������Ϸ��Լ��*/
	if( device_index > INTERFACE_DEVICE_MAX_NUMBER )
	{
		return RIP_FAIL;
	}
	
	if( NULL == rip_intf_array[device_index] )
	{
		/*�ö˿ڻ�û������,���账��*/
		return RIP_SUCCESS;
	}

	if( BIT_TEST( rip_intf_array[device_index]->state ,RIP_INTF_LINK_UP))
	{
		/*�˿��Ѿ�UP,��������*/
		return RIP_SUCCESS;
	}

	/*��ȡ�˿�����*/
	ret =rt_get_if_type(device_index, &device_type);
	/*
	ret = interface_device_callback_gettype( device_index, &device_type);
	*/
	if( INTERFACE_GLOBAL_SUCCESS != ret )
	{
		return RIP_FAIL;
	}

	device_type &= INTERFACE_DEVICE_FLAGS_NET_MASK;

	rip_intf_array[device_index]->encap_type = device_type;

	/*��¼�˿�״̬*/
	BIT_SET(rip_intf_array[device_index]->state , RIP_INTF_LINK_UP);

	/* ��û�н����ڶ˿���enable������Ҫ������Ĵ���
	 * Commented by dangzhw in 2010.03.10 10:41:16 */
	if((rip_intf_array[device_index]->process_id == 0) || !(pprocess = rip_intf_array[device_index]->pprocess))
		return RIP_SUCCESS;

	memset(&msg , 0 ,sizeof(struct rip_route_activate_msg_));

	msg.type = RIP_ROUTE_INTF_PROTO_UP;
	msg.u.intf.device_index = device_index;
	msg.u.intf.warning = TRUE;

	ret = rip_process_route_activate( pprocess, &msg);
	
	return ret;
}

/*===========================================================
������:      rip_intf_link_down
��������: �˿���·״̬DOWN������
�������: device_index: �߼��˿ں�
�������: ��
����ֵ:      ����ɹ�,����RIP_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_intf_link_down( uint32 device_index )
{
	int ret;
	struct rip_route_activate_msg_ msg;
	struct rip_process_info_ *pprocess;
	
	/*��������Ϸ��Լ��*/
	if( device_index > INTERFACE_DEVICE_MAX_NUMBER )
	{
		return RIP_FAIL;
	}
	
	if( NULL == rip_intf_array[device_index] )
	{
		/*�ö˿ڻ�û������,���账��*/
		return RIP_SUCCESS;
	}
	
	if( !BIT_TEST( rip_intf_array[device_index]->state ,RIP_INTF_LINK_UP) )
	{
		/*�˿��Ѿ�DOWN,��������*/
		return RIP_SUCCESS;
	}
	else
		/*��¼�˿���·״̬*/
		BIT_RESET(rip_intf_array[device_index]->state ,RIP_INTF_LINK_UP);

	if((rip_intf_array[device_index]->process_id == 0) || !(pprocess = rip_intf_array[device_index]->pprocess))
		return RIP_SUCCESS;

	memset(&msg , 0 ,sizeof(struct rip_route_activate_msg_));

	msg.type = RIP_ROUTE_INTF_PROTO_DOWN;
	msg.u.intf.device_index = device_index;

	ret = rip_process_route_activate( pprocess, &msg);

#if 0 
	/*ʹ�˿ڽ�ֹ�����鲥����*/

	/* ֻ�б�rip���Ǻ���ܼ���Ӧ��enable multicast in
	 * Commented by dangzhw in 2009.11.06 15:29:49 */
	if(rip_intf_array[device_index]->ref_num>0)
	{
		ret = rip_disable_multicast_in( device_index );
		if( RIP_SUCCESS != ret )
		{
			return ret;
		}
	}
	/*��RIP�˿�,��������*/
	if( rip_intf_array[device_index]->ref_num <= 0 )
	{
		return RIP_SUCCESS;
	}

       /*
       rip_del_summary_route(device_index , 
       	rip_intf_array[device_index]->connect_route);
      */

	/*ɾ���˿�ֱ��·��*/
	ret = rip_del_connect_route( device_index );
	if( RIP_SUCCESS != ret )
	{
		return ret;
	}

	/*ɾ���Ӷ˿�ѧϰ����·��*/
	ret = rip_clear_nbr_route_list( device_index );
	if( RIP_SUCCESS != ret )
	{
		return ret;
	}
#endif 
	return ret;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_intf_addr_add
 *
 * PARAMS		:	device_index	- �߼��˿ں�
 *					aid			- ip aid, we can only do with primary ip aid as yet
 *					addr		- �˿ڵ�ַ
 *					mask		- �˿ڵ�ַ����
 *					addr_type	- �˿ڵ�ַ����
 *                                vrf_id: �˿ڶ�Ӧ��VRF ID
 *
 * RETURN		:	����ɹ�,����RIP_SUCCESS
 *
 * NOTE			:	�˿ڵ�ַ���Ӵ�����
 *                               ����ʧ��,����RIP_FAIL
 *
 * AUTHOR		:	Fengsb
 *
 * DATE			:	2007.07.12 13:34:21
 *
*******************************************************************************/
int rip_intf_addr_add( uint32 device_index, uint32 aid, uint32 addr, uint32 mask, uint8 addr_type, uint32 vrf_id )
{
	int ret;
	struct rip_route_activate_msg_ msg;
	struct rip_process_info_ *pprocess;
	/*��������Ϸ��Լ��*/
	if( device_index > INTERFACE_DEVICE_MAX_NUMBER )
	{
		return RIP_FAIL;
	}

	if( NULL == rip_intf_array[device_index] )
	{
		/*�ö˿ڻ�û������,���账��*/
		return RIP_SUCCESS;
	}

	if( (rip_intf_array[device_index]->address == addr)
		&& (rip_intf_array[device_index]->mask == mask)
		&& (rip_intf_array[device_index]->addr_type == addr_type)
		&& (rip_intf_array[device_index]->vrf_id == vrf_id)
		&& (rip_intf_array[device_index]->aid ==  aid))/*xuhaiqing 2011-1-20 ���bug SWBUG00009229*/
	{
		/*��ַû�б仯,��������*/
		return RIP_SUCCESS; 
	}

	/*��¼�˿ڵ�ַ��Ϣ*/
	rip_intf_array[device_index]->address = addr;
	rip_intf_array[device_index]->mask = mask;
	rip_intf_array[device_index]->addr_type = addr_type;
	rip_intf_array[device_index]->vrf_id = vrf_id;

	/*fengsb 2007-07-12 add add primary ip aid for 76** */
	rip_intf_array[device_index]->aid =  aid; 

	/* ��û�н���enable�ö˿�ֱ�ӷ���
	 * Commented by dangzhw in 2010.03.10 14:23:01 */
	if(rip_intf_array[device_index]->process_id == 0 )
		return RIP_SUCCESS;

	pprocess = rip_intf_array[device_index]->pprocess;

	memset(&msg , 0 , sizeof(struct rip_route_activate_msg_));


	/* ���ڽ��̸��ǵĶ˿���Ҫ�ж��Ƿ�Ҫ���ֱ��·��
	 * Commented by dangzhw in 2010.03.10 14:36:08 */
	msg.type = RIP_ROUTE_INTF_ADDRESS_ADD;
	msg.u.intf.device_index = device_index;
	msg.u.intf.warning = TRUE;

	ret = rip_process_route_activate(pprocess, &msg);
#if 0		
	/*��RIP�˿ڲ�����*/
	if( rip_intf_array[device_index]->ref_num <= 0 )
	{
		return RIP_SUCCESS;
	}

	/*�˿�״̬��UP,��������*/
	if(! BIT_TEST( rip_intf_array[device_index]->link_state ,RIP_INTF_LINK_UP))
	{
		return RIP_SUCCESS;
	}

	/*�����˿�ֱ��·��*/
	ret = rip_create_connect_route( device_index ,TRUE);
	if( RIP_SUCCESS != ret )
	{
		rip_intf_ref_num_set(device_index, RIP_INTF_REF_NUM_SUB_1);
		return ret;
	}

	/*�˿ڷ���request����*/
	ret = rip_send_request( device_index );
	if( RIP_SUCCESS != ret )
	{
		return ret;
	}
#endif
	return ret;
}

/*===========================================================
������:      rip_intf_addr_del
��������: �˿ڵ�ַɾ��������
�������: device_index: �߼��˿ں�
                       addr: �˿ڵ�ַ
                       mask: �˿ڵ�ַ����
                       addr_type: �˿ڵ�ַ����
                       vrf_id: �˿ڶ�Ӧ��VRF ID
�������: ��
����ֵ:       ����ɹ�,����RIP_SUCCESS
                        ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_intf_addr_del( uint32 device_index, uint32 addr, uint32 mask, uint8 addr_type, uint32 vrf_id )
{
	int ret = 0;
	struct rip_process_info_ *pprocess;
	struct rip_route_activate_msg_ msg;
	
	/*��������Ϸ��Լ��*/
	if( device_index > INTERFACE_DEVICE_MAX_NUMBER )
	{
		return RIP_FAIL;
	}
	
	if( NULL == rip_intf_array[device_index] )
	{
		/*�ö˿ڻ�û������,���账��*/
		return RIP_SUCCESS;
	}

	/*���յ��Ķ˿ڵ�ַ��Ϣ������¼�Ķ˿ڵ�ַ��Ϣ��һ��,
	  ��������*/
	if( (addr != rip_intf_array[device_index]->address )
		|| (mask != rip_intf_array[device_index]->mask)
		|| (addr_type != rip_intf_array[device_index]->addr_type))
	{
		return RIP_SUCCESS;
	}

/*
	if(rip_intf_array[device_index]->process_id == 0 )
		return RIP_SUCCESS;
*/
	pprocess = rip_intf_array[device_index]->pprocess;
	if(pprocess)
	{
		memset(&msg , 0 , sizeof(struct rip_route_activate_msg_));
		msg.type = RIP_ROUTE_INTF_ADDRESS_DLE;
		msg.u.intf.device_index = device_index;
		ret = rip_process_route_activate( pprocess, &msg);
	}
	
	/*���Ķ˿ڵ�ַ��Ϣ*/
	rip_intf_array[device_index]->address = 0;
	rip_intf_array[device_index]->mask = 0;
	rip_intf_array[device_index]->addr_type = 0;
	rip_intf_array[device_index]->vrf_id = vrf_id;
	rip_intf_array[device_index]->aid = 0;
	return ret;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_add_intf_vrf
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	�˿����vrf
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.15 16:37:54
 *
*******************************************************************************/
int rip_add_intf_vrf(uint32 vrfid , uint32 device_index)
{
	int ret;
	struct rip_route_activate_msg_ msg;

	if(device_index >INTERFACE_DEVICE_MAX_NUMBER || !rip_intf_array[device_index])
		return RIP_FAIL;

	rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: interface %s Receive vrf (vrfid %d)add  message\n",rip_intf_array[device_index]->intf_name,vrfid);

	if(rip_intf_array[device_index]->vrf_id != 0)/*�Ѿ��󶨵�ĳ��vrf��*/ 
	{
		if(rip_intf_array[device_index]->vrf_id != vrfid)
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}
		else /*����ǰһ��ֱ�ӷ���*/
			return RIP_SUCCESS;
	}
	else
		rip_intf_array[device_index]->vrf_id = vrfid;


	/* ֻ�ж˿ڱ�����ָ����Ż�ȥ�ж����·�ɵ�database��
	 * Commented by dangzhw in 2010.03.15 16:25:46 */
	if(rip_intf_array[device_index]->process_id == 0)
		return RIP_SUCCESS;

	memset(&msg , 0 ,sizeof(struct rip_route_activate_msg_));
	msg.type = RIP_ROUTE_INTF_VRF_ADD;
	msg.u.intf.device_index =device_index;

	ret = rip_process_route_activate( rip_intf_array[device_index]->pprocess, & msg);

	return ret;
}


/*******************************************************************************
 *
 * FUNCTION	:	rip_del_intf_vrf
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	�˿�ɾ��vrf
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.15 16:38:21
 *
*******************************************************************************/
int rip_del_intf_vrf(uint32 vrfid , uint32 device_index)
{
	struct rip_route_activate_msg_ msg;
	int ret;

	if(device_index >INTERFACE_DEVICE_MAX_NUMBER || !rip_intf_array[device_index])
		return RIP_FAIL;

	rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: interface %s Receive vrf (vrfid %d)del  message\n",rip_intf_array[device_index]->intf_name,vrfid);

	if((vrfid == 0) ||(rip_intf_array[device_index]->vrf_id != vrfid) )
		return RIP_FAIL;

	rip_intf_array[device_index]->vrf_id = 0;

	if(rip_intf_array[device_index]->process_id == 0)
		return RIP_SUCCESS;

	memset(&msg , 0 ,sizeof(struct rip_route_activate_msg_));
	msg.type = RIP_ROUTE_INTF_VRF_DEL;
	msg.u.intf.device_index =device_index;

	ret = rip_process_route_activate( rip_intf_array[device_index]->pprocess, & msg);

	return RIP_SUCCESS;
}
/*===========================================================
������:      rip_vrf_del
��������: VRFɾ��������
�������: vrf_id: VRF ID
�������: ��
����ֵ:       ����ɹ�,����RIP_SUCCESS
                        ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_del_vrf_process( uint32 vrf_id )
{
	struct rip_process_list_ *pprocess_list;
	struct rip_process_info_ *pprocess;
	int ret;


	/* ��ͬһ��vrf��rip�������
	 * Commented by dangzhw in 2010.03.15 15:38:31 */
	pprocess_list = rip_process_list.forw;
	while(pprocess_list != &rip_process_list)
	{
		pprocess = pprocess_list->rip_process_info;
		if(!pprocess)
			return RIP_FAIL;

		pprocess_list = pprocess_list->forw;
		
		if(pprocess->vrf_id == vrf_id)
		{				/*ֹͣRIPģ�鶨ʱ��*/
			ret = rip_process_stop_timer(pprocess);
			if( RIP_SUCCESS != ret )
			{
				return ret;
			}

				/*ֹͣRIPģ�鶨ʱ��*/
			ret = rip_process_delete_timer(pprocess);
			if( RIP_SUCCESS != ret )
			{
				return ret;
			}
			
			assert(ret == 0);
			
			/*ɾ������������RIP·�ɱ�*/
			rip_clear_process_table(pprocess);

			/*ɾ��RIPȫ�ֱ����б������Ϣ*/
			rip_clear_process_info(pprocess);
		}
	}
	
	return RIP_SUCCESS;
}


/*===========================================================
������:      rip_create_connect_route
��������: ����RIP����ֱ��·��
�������: device_index: �߼��˿ں�
�������: ��
����ֵ:      ����ɹ�,����RIP_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_create_connect_route( uint32 device_index ,uint warning)
{
	struct prefix route_prefix;
	struct route_node *rip_route_node;
	struct rip_route_ *rip_route;
	struct rip_process_info_ *pprocess = NULL;
	
	/*��������Ϸ��Լ��*/
	if( (device_index > INTERFACE_DEVICE_MAX_NUMBER)
		|| (NULL == rip_intf_array[device_index]) )
	{
		return RIP_FAIL;
	}
/*
	vrf_id = rip_intf_array[device_index]->vrf_id;
	if( NULL == rip_tbl_array[vrf_id] )
	{
		return RIP_FAIL;
	}
	*/
	/*pprocess_list = rip_lookup_process_byintfid(device_index);*/
	if(!(pprocess = rip_intf_array[device_index]->pprocess))
		return RIP_FAIL;
	if(rip_intf_array[device_index]->connect_route)
		return RIP_SUCCESS;

	/*xuhaiqing 2010-12-21�Ž����·����Ŀ������ȡ��*/
    	/*·�ɱ���·�������������ֵ���������*/
	/*if( pprocess->route_num >= pprocess->max_route_num )
	{
		if(warning)
			vty_output( "%%Warning, Limit for rip routes exceeded, max # is %d.\n", pprocess->max_route_num );
		return RIP_FAIL;
	}*/
	
	/*���prefix��Ϣ*/
	memset( &route_prefix, 0, sizeof(struct prefix) );
	route_prefix.family   = AF_INET;
	route_prefix.safi     = SAFI_UNICAST;   // �������� IPv4 ����·��
	route_prefix.prefixlen = mask_to_prefix( rip_intf_array[device_index]->mask );
	route_prefix.u.prefix4.s_addr = rip_intf_array[device_index]->address & rip_intf_array[device_index]->mask;

	/*��ȡroute_node*/
	rip_route_node = route_node_get( pprocess->rip_table, &route_prefix, NULL );
	if( NULL == rip_route_node )
	{
		return RIP_FAIL;
	}
	
#if 0   /* modified by ygc ---- 2012.09.27 */   
	if( NULL != rip_route_node->info )
	{
        /* Fengsb 2006-02-17  every new entry route node lock++ , so we can't 
        	unlock here */
		/* route_unlock_node( rip_route_node ); */
	 	rip_route_head = (struct rip_route_ *)(rip_route_node->info);

		 /*fang qi add this, to modify the bug in 85 */
        while(NULL != rip_route_head && NULL!=rip_route_node->info)
        {
			rip_route_temp = rip_route_head;
			rip_route_head = rip_route_head->forw;			

			if (RIP_NBR_ROUTE == rip_route_temp->route_type)
			{
				/*���ھ�ѧϰ����·�� ��ɾ���ӿ��ھ�·������*/
				rip_route_list_temp =rip_route_temp->nbr_ptr;

				if (NULL!=rip_route_list_temp)
				{
					REMQUE( rip_route_list_temp );
					rip_mem_free( rip_route_list_temp, RIP_ROUTE_LIST_TYPE );
				
					rip_route_temp->nbr_ptr=NULL;
					rip_intf_array[rip_route_temp->gw_index]->nbr_route_num--;
				}
				
			}
			else if (RIP_REDIS_ROUTE == rip_route_temp->route_type)
			{
				/*��Ϊת��·�ɱ���Ϊ��·�ɱ���active·��
				������ֱ������ʱ,ת��·�ɱض���Ϊactive·��
				�ʿɴ�rip·�ɱ���ɾ��*/

				red_ptr = rip_route_temp->red_ptr;

				if (NULL!=red_ptr)
				{
					REMQUE( red_ptr );
					rip_mem_free( red_ptr, RIP_ROUTE_LIST_TYPE );  

					rip_route_temp->red_ptr=NULL;
				}
			
			}
			/*ע���ʱ·�ɲ�������connect��summary����*/
			/*���ݽӿ�ip��ַ���õ�����,��������������������ͬ
			�Ľӿ�,Ҳ�����ܴ���һ���ӿ����ڵ����Ÿ�����һ��
			�ӿ�����*/			
		
            /*ɾ��ѧϰ·���е�holdown·�� */
			rip_del_holddown(rip_route_temp);
			
            ret = rip_del_route(rip_route_temp);	
			if( RIP_SUCCESS != ret )
			{
				return ret;
			}
            /*����Ѿ�����һ��,���˳�*/
			if (rip_route_head==rip_route_node->info)
				break;

		}/* fang qi modify over */
	}
#endif	/* �����ε���δ��������⣬�����ڵȼ�·��ʱ����ȫɾ�� */

	rip_route = (struct rip_route_ *)(rip_route_node->info);
	if(rip_route)
	{
		rip_del_route_node(rip_route); /* ɾ��ѧ����·�ɺ��ط�����·�� */
		rip_route = (struct rip_route_ *)(rip_route_node->info);
		if(rip_route)
		{
			route_unlock_node(rip_route_node);
			syslog(LOG_ERR, "rip: create connect route fail!\n");
			return RIP_FAIL;
		}
	}  
	
	/*���rip_route������Ϣ*/
	rip_route = ( struct rip_route_ *)rip_mem_malloc( sizeof (struct rip_route_), RIP_ROUTE_TYPE );
	if( NULL == rip_route )
	{
		route_unlock_node(rip_route_node);
		fprintf(stderr, "RIP: Malloc memory failed.'(%s, %d)\n", __FILE__, __LINE__ );
		return RIP_MEM_FAIL;
	}
	memset( (void *)rip_route, 0, sizeof(struct rip_route_) );
	
	rip_route->forw = rip_route;
	rip_route->back = rip_route;

	rip_route->route_tag = RIP_INTERNAL_ROUTE;
	rip_route->route_type = RIP_CONNECT_ROUTE;
	
	rip_route->refresh_time = time_sec;	
	
	rip_route->pprocess= pprocess;
	
	rip_route->gw_index = device_index;
    /*Fengsb add 2006-04-30 : this distance is the distance that no consider network and mask*/
	rip_route->distance = pprocess->distance_list->default_distance;
	
	rip_route->gw_addr = 0;/*ֱ��·��,�������ַ*/
	rip_route->metric = RIP_DIRECT_METRIC;

	rip_route->hold_ptr = NULL;
	
	/*·�ɼ���route node */
	rip_route_node->info = (void *)rip_route;	
	rip_route->route_node = rip_route_node;
	
	pprocess->route_num++;
	pprocess->connect_route_num++;
	rip_route->equi_route_num++;

	rip_intf_array[device_index]->connect_route = rip_route;

    /*����·�ɼ��봥�������б�*/
	rip_add_trigger( rip_route, pprocess, FALSE );	
		
	rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "RIP-DB: Adding connected route %s/%d to RIP database\n", ip_ntoa(route_prefix.u.prefix4.s_addr), route_prefix.prefixlen );
	pprocess->rip2GlobalRouteChanges++;

	/*Fengsb 2006-05-23 ��������·��*/
	rip_create_summary_route(device_index , rip_route ,pprocess);

	return RIP_SUCCESS;
}

/*===========================================================
������:      rip_del_connect_route
��������: ɾ��RIP����ֱ��·��
�������: device_index: �߼��˿ں�
�������: ��
����ֵ:       ����ɹ�,����RIP_SUCCESS
                        ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_del_connect_route( uint32 device_index )
{
	int ret;
	struct rip_process_info_ *pprocess =NULL;
	
	/*��������Ϸ��Լ��*/
	if( (device_index > INTERFACE_DEVICE_MAX_NUMBER)
		|| (NULL == rip_intf_array[device_index]) )
	{
		return RIP_FAIL;
	}

	if( NULL != rip_intf_array[device_index]->connect_route )
	{
		pprocess= rip_intf_array[device_index]->pprocess;
		if(pprocess)
			rip_del_summary_route(device_index , rip_intf_array[device_index]->connect_route, pprocess);
		else
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}
		
		ret = rip_del_route( rip_intf_array[device_index]->connect_route);
		if( RIP_SUCCESS != ret )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return ret;
		}
	}
	
	rip_intf_array[device_index]->connect_route = NULL;	

	return RIP_SUCCESS;
}

/*-----------------------------------------------------------------
��������: rip_create_summary_route
��������: �������ػ���·��,��������뱾��·�ɱ�
�������: device_index: �ӿ�ID
			:  rip_intf_rt :�˿�ֱ��·��
�������: 
����ֵ   : ����ɹ�:RIP_SUCCESS
               ����ʧ��:���󷵻���
��ע:
------------------------------------------------------------------*/
ULONG rip_create_summary_route( ULONG device_index , struct rip_route_ *rip_intf_rt ,struct rip_process_info_ *pprocess)
{		
	struct route_node *rip_rtnode;
	struct rip_route_ *rip_route = NULL, *rip_rthead, *rip_rt_tmp;
	struct rip_subrt_ *rip_subrt;
	struct prefix route_prefix;
	ULONG natural_mask;
	ULONG natural_net;
	uint32  count;
	ULONG ret;

	ret = 0;
	natural_mask = 0;
	natural_net  = 0;
/*	vrf_id = rip_intf_array[device_index]->vrf_id;*/
	
	natural_net = rip_get_natural_network( rip_intf_array[device_index]->address );
	natural_mask = rip_get_natural_mask(  rip_intf_array[device_index]->address );
	if((natural_net == (rip_intf_array[device_index]->address & rip_intf_array[device_index]->mask))
		&&(natural_mask == rip_intf_array[device_index]->mask))
	{
		return RIP_SUCCESS;	
	}

	/*���prefix��Ϣ*/
	memset( &route_prefix, 0, sizeof(struct prefix) );
	route_prefix.family   = AF_INET;
	route_prefix.safi     = SAFI_UNICAST;
	route_prefix.prefixlen = mask_to_prefix( natural_mask );
	route_prefix.u.prefix4.s_addr = natural_net;
	
	/*��ȡroute_node*/
	rip_rtnode = route_node_get( pprocess->rip_table, &route_prefix ,NULL);
	if( NULL == rip_rtnode )
	{
		return RIP_FAIL;
	}

	rip_rthead = (struct rip_route_ *)(rip_rtnode->info);
	if(rip_rthead)
	{        
		/* route_unlock_node( rip_rtnode ); */
		if( rip_rthead->route_type == RIP_CONNECT_ROUTE )
		{
			/*������ڴ�ֱ��·�ɣ����ٴ�������·�ɣ���Ϊ
			��ʱ�����Ѿ���һ�µģ������������ֱ��·�ɣ���
			�����ڴ��ھ�ѧϰ������ͬ��·�ɡ�ͬʱ����Ϊֱ��·��
			�Ĵ��ڣ���Ȼû��redistribute connect�ܼ������*/
			route_unlock_node( rip_rtnode );
			return RIP_SUCCESS;			
		}

		/*query is there already exist summary route */
		for( count = 1, rip_route = rip_rthead; count <= rip_rthead->equi_route_num; rip_route = rip_rt_tmp, count++ )
		{
			rip_rt_tmp = rip_route->forw;
			if(RIP_SUMMARY_ROUTE == rip_route->route_type)
			{   
				route_unlock_node( rip_rtnode );				

				rip_subrt = (struct rip_subrt_ *)rip_mem_malloc( sizeof(struct rip_subrt_), RIP_SUB_ROUTE_TYPE );
				if( rip_subrt == NULL )
				{
					fprintf(stderr, "RIP: Malloc memory failed'(%s, %d)\n", __FILE__, __LINE__ );
					return RIP_MEM_FAIL;
				}
				memset( (void *)rip_subrt, 0, sizeof(struct rip_subrt_) );
				rip_subrt->forw = rip_subrt;
				rip_subrt->back = rip_subrt;
				rip_subrt->rip_route = rip_intf_rt;
		
				INSQUE(rip_subrt, rip_route->rip_connect.back);
				rip_route->sum_cnt++;
				return RIP_SUCCESS;	
			}
		}		
	}	
    
	/* create new summary rip_route entry ���rip_route������Ϣ*/
	rip_route = ( struct rip_route_ *)rip_mem_malloc( sizeof (struct rip_route_), RIP_ROUTE_TYPE );
	if( NULL == rip_route )
	{
		route_unlock_node( rip_rtnode );	
		fprintf(stderr, "RIP: Malloc memory failed.'(%s, %d)\n", __FILE__, __LINE__ );
		return RIP_MEM_FAIL;
	}
	memset( (void *)rip_route, 0, sizeof(struct rip_route_) );	
	QUE_INIT(rip_route);
	QUE_INIT(&(rip_route->rip_connect));
	
	rip_route->route_tag = RIP_INTERNAL_ROUTE;
	rip_route->route_type = RIP_SUMMARY_ROUTE;	
	rip_route->refresh_time = time_sec;		
	rip_route->pprocess= pprocess;	
	rip_route->gw_index = 0;
	rip_route->gw_addr = 0;/*ֱ��·��,�������ַ*/
  	/*Fengsb add 2006-04-30: this distance is the distance that no consider network and mask*/
	rip_route->distance = pprocess->distance_list->default_distance;	
	rip_route->metric    = RIP_DIRECT_METRIC;
	rip_route->hold_ptr = NULL;
	rip_route->route_node = rip_rtnode;

	rip_subrt = (struct rip_subrt_ *)rip_mem_malloc( sizeof(struct rip_subrt_), RIP_SUB_ROUTE_TYPE );
	if( rip_subrt == NULL )
	{
		route_unlock_node( rip_rtnode );	
		rip_mem_free(rip_route, RIP_ROUTE_TYPE);
		fprintf(stderr, "RIP: Malloc memory failed'(%s, %d)\n", __FILE__, __LINE__ );
		return RIP_MEM_FAIL;
	}
	memset( (void *)rip_subrt, 0, sizeof(struct rip_subrt_) );
	rip_subrt->forw = rip_subrt;
	rip_subrt->back = rip_subrt;
	rip_subrt->rip_route = rip_intf_rt;

	INSQUE(rip_subrt, rip_route->rip_connect.back);
	rip_route->sum_cnt++;
				
	if(rip_rthead)
	{	
		if(rip_rthead->route_type == RIP_REDIS_ROUTE)
		{/*If there is redistribute route, it must be the first entry, so we only need add the summary
			after the redistibute route*/			
			INSQUE( rip_route, rip_rthead );		
		}
		else
		{	/* summary route as the first entry under the route node*/
			rip_route->forw = rip_rthead;
			rip_route->back = rip_rthead->back;
			rip_rthead->back->forw = rip_route;
			rip_rthead->back = rip_route;
			rip_route->equi_route_num = rip_rthead->equi_route_num;
			rip_route->equi_nbr_num= rip_rthead->equi_nbr_num;
			rip_rtnode->info = rip_route;
			rip_rthead = rip_route;
		}
	}
	else
	{
		rip_rtnode->info = rip_route;
		rip_rthead = rip_route;
	}
	pprocess->route_num++;
	pprocess->sum_route_num++;
	rip_rthead->equi_route_num++;
	
	/*����·�ɼ��봥�������б�
	���ü��룬��Ϊֱ��·���Ѿ������˴��������б��ˡ��ڷ��͵�
	ʱ�����������auto summary ������Զ����ܵġ�
	rip_add_trigger( rip_route, FALSE );
	*/
	rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "RIP-DB: Adding summary route %s/%d to RIP database\n", \
		ip_ntoa(route_prefix.u.prefix4.s_addr), route_prefix.prefixlen );
	pprocess->rip2GlobalRouteChanges++;	
	return RIP_SUCCESS;
}

/*-----------------------------------------------------------------
��������: rip_del_summary_route
��������: ɾ�����ػ���·�ɣ�������ɾ��ֱ��·��ǰ����
			ɾ������·�ɡ���Ϊ�õ���rip_intf_rt
�������: device_index: �ӿ�ID
�������: 
����ֵ   : ����ɹ�:RIP_SUCCESS
               ����ʧ��:���󷵻���
��ע:
------------------------------------------------------------------*/
ULONG rip_del_summary_route( ULONG device_index , struct rip_route_ *rip_intf_rt , struct rip_process_info_ *pprocess)
{		
	struct route_node *rip_rtnode;
	struct rip_route_ *rip_route = NULL, *rip_rthead, *rip_rt_tmp;
	struct rip_subrt_ *rip_subrt;
	struct prefix route_prefix;
	ULONG natural_mask;
	ULONG natural_net;
	uint32  count;
	ULONG ret;
	
	natural_net = rip_get_natural_network( rip_intf_array[device_index]->address );
	natural_mask = rip_get_natural_mask(  rip_intf_array[device_index]->address );
	if((natural_net == (rip_intf_array[device_index]->address  & rip_intf_array[device_index]->mask))
		&&(natural_mask == rip_intf_array[device_index]->mask))
	{
		return RIP_SUCCESS;	
	}

	/*���prefix��Ϣ*/
	memset( &route_prefix, 0, sizeof(struct prefix) );
	route_prefix.family   = AF_INET;
	route_prefix.safi     = SAFI_UNICAST;
	route_prefix.prefixlen = mask_to_prefix( natural_mask );
	route_prefix.u.prefix4.s_addr = natural_net;
	
	/*��ȡroute_node*/
	rip_rtnode = route_node_lookup( pprocess->rip_table, &route_prefix );
	if( NULL == rip_rtnode )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	if( NULL != rip_rtnode->info )
	{        
		route_unlock_node( rip_rtnode ); 	
		rip_rthead = (struct rip_route_ *) rip_rtnode->info;
		/*query is there already exist summary route */
		for( count = 1, rip_route = rip_rthead; count <= rip_rthead->equi_route_num; rip_route = rip_rt_tmp, count++ )
		{
			rip_rt_tmp = rip_route->forw;
			if(RIP_SUMMARY_ROUTE == rip_route->route_type)
			{   
				QUE_LIST(rip_subrt, &rip_route->rip_connect){
					if(rip_subrt->rip_route == rip_intf_rt)
					{
						REMQUE(rip_subrt);
						rip_mem_free( rip_subrt, RIP_SUB_ROUTE_TYPE );
						rip_route->sum_cnt--;
						if(rip_route->sum_cnt == 0)
						{								
							/*ɾ����·��*/
							ret = rip_del_route( rip_route);
							if( RIP_SUCCESS != ret )
							{
								rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
								return RIP_FAIL;
							}
						}
						return RIP_SUCCESS;	
					}
				}QUE_LIST_END(rip_subrt, &rip_route->rip_connect);								
			}
		}		
	}	
		
	pprocess->rip2GlobalRouteChanges++;	
	return RIP_SUCCESS;
}

/*===========================================================
������:      rip_del_route
��������: ɾ����·��
�������: rip_route : ��ɾ����·��
�������: ��
����ֵ:      ����ɹ�,����RIP_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_del_route( struct rip_route_ *rip_route /*,struct rip_process_info_ *pprocess*/)
{
	uint32 device_index;
    BOOL   b_del_redis = FALSE;
	char string[100];
	struct rip_peer_list_ *rip_peer, *rip_peer_forw;
	struct rip_route_ *rip_route_head, *rip_new_rthead;
	struct rip_process_info_ *pprocess = NULL;

	rip_route_head = (struct rip_route_ *)(rip_route->route_node->info);
	if(rip_route_head == NULL )
		return RIP_SUCCESS;

	if(!(pprocess = rip_route_head->pprocess))
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	rip_route_head->equi_route_num--;
	if( rip_route_head != rip_route )
	{
		/*rip_route��route_node��ɾ��*/
		REMQUE( rip_route );
	}
	else // ���Ǵ�·�ɾ���ͷ�ڵ�
	{
		/*Fengsb add note 2006-02-14�˴�Ӧ�ÿ��ǵȼ�·�ɣ������ڵȼ�·�ɣ���Ҫ����һroute entry��
		��info�ϣ�ͬʱ��Ҫע��equi_route_num�Ĵ���*/
		// ɾ����·��ʱ�������������ȼ�·�ɣ�����·�ɽڵ����һ����Ϊhead�ڵ�
		if(rip_route_head->equi_route_num != 0)
		{
			rip_route->route_node->info = rip_route->forw;
			rip_new_rthead = (struct rip_route_ *)(rip_route->route_node->info);
			rip_new_rthead->equi_route_num = rip_route_head->equi_route_num;
			rip_new_rthead->equi_nbr_num = rip_route_head->equi_nbr_num;
			rip_route_head = rip_new_rthead;
			/*removeͷ�ڵ��Ժ󣬵��������ڵ�����ָ��*/
			rip_route->forw->back = rip_route->back;
			rip_route->back->forw = rip_route->forw;		
		}
		else
		{
			rip_route->route_node->info = NULL;
		}
	}	

	switch( rip_route->route_type )
	{
		case RIP_CONNECT_ROUTE:
			device_index = rip_route->gw_index;
			
			rip_intf_array[device_index]->connect_route = NULL;
			
			pprocess->connect_route_num--;
			pprocess->route_num--;
			
			rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "RIP-DB: Deleting connected route %s/%d from RIP database\n", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
			break;
		case RIP_NBR_ROUTE:
			/*ѧϰ·��,����·�ɱ���ɾ��*/

			/*before del route, del the peer first*/
			rip_peer = pprocess->peer_list.forw;
			while(rip_peer != &(pprocess->peer_list))
			{
				rip_peer_forw = rip_peer->forw;
				if(rip_peer->peer_addr == rip_route->gw_addr)
				{
					rip_peer->ref_num --;
				/*	if(rip_peer->ref_num == 0)
					{
						REMQUE(rip_peer);
						rip_mem_free(rip_peer, RIP_PEER_LIST_TYPE);
					}
				*/
					break;
				}
				rip_peer = rip_peer_forw;
			}
	
			rip_del_from_main_tbl( rip_route ,pprocess->process_id);

			pprocess->nbr_route_num--;
			pprocess->route_num--;
			rip_route_head->equi_nbr_num--;

			if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_DATABASE )
			{
				sprintf(string, "RIP-DB: Deleting learn route %s/%d <metric %ld>", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen, rip_route->metric );
				sprintf(string, "%s via %s from RIP database", string, ip_ntoa( rip_route->gw_addr ) );
				rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%s\n", string );
			}

			break;
		case RIP_REDIS_ROUTE:
			/*ת��·��*/
			pprocess->redis_route_num--;
			pprocess->route_num--;
            b_del_redis = TRUE; /*Fengsb 2006-04-27 add */
			rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "RIP-DB: Deleting redistributed route %s/%d from RIP database\n", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
			break;
	    case RIP_DEF_ROUTE:
			/*rip ȱʡ·��*/			
			pprocess->route_num--;  
			b_del_redis = TRUE; /* ygc add 2012-09-25 */
			rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "RIP-DB: Deleting rip default route %s/%d from RIP database\n", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
			break;
		case RIP_SUMMARY_ROUTE:
			/*rip ȱʡ·��*/			
			pprocess->route_num--; 
			pprocess->sum_route_num--;  
			rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "RIP-DB: Deleting rip summary route %s/%d from RIP database\n", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
			break;
		default:
			break;
	}

	/*Fengsb 2006-02-15 add the first condition to avoid route linar circle 
	��������,metricֵΪ16*/
	if( (rip_route_head->equi_route_num == 0) 
        	&& ( RIP_MAX_METRIC != rip_route->metric ) && (rip_route->route_type != RIP_SUMMARY_ROUTE))
	{
		rip_route->metric = RIP_MAX_METRIC;		
		rip_add_trigger(rip_route, pprocess, TRUE);
	}
	else if(b_del_redis == TRUE)
	{ /*����ɾ��ת��·�ɺ�ͬһ�ڵ㻹�����ھ�ѧϰ����·�ɵ����*/
		rip_add_trigger(rip_route, pprocess, TRUE);
	}
	
	/*Fengsb 2006-02-16 ��Ϊÿ�����·�ɣ�����route_node lock++������ɾ��·��
	��ʱ��Ҳ��Ҫ��route node lock--*/
	route_unlock_node( rip_route->route_node );		

	/*�ͷ�rip_route*/
	rip_mem_free( rip_route, RIP_ROUTE_TYPE );

	rip_route=NULL;  /* fangqi add this */

	pprocess->rip2GlobalRouteChanges++;

	return RIP_SUCCESS;
}


/*===========================================================
������:      rip_enable_multicast_in
��������: ʹ�ӿڷ����鲥����
�������: device_index : �߼��˿ں�
�������: ��
����ֵ:      ����ɹ�,����RIP_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_enable_multicast_in( uint32 device_index )
{
	int ret;
	struct ip_mreq mreq;

	/*��������Ϸ��Լ���*/
	if( device_index > INTERFACE_DEVICE_MAX_NUMBER )
	{
		return RIP_FAIL;
    }
#ifndef R1705
	memset( (void *)&mreq, 0, sizeof(struct ip_mreq) );

	mreq.imr_interface.s_addr = device_index;
	mreq.imr_mcastaddr.s_addr = RIP_MULTICAST_ADDRESS;
	ret = so_setsockopt( rip_glb_info.socket_id, IPPROTO_IP, IP_ADD_MEMBERSHIP_INTF, (char *)&mreq, sizeof(mreq) );
#else
	ret = ip_add_mcast(device_index, htonl(RIP_MULTICAST_ADDRESS));
#endif
	if( ret < 0 )
	{
		rip_debug(RIP_DEBUG_IP_RIP_PROTOCOL, "%%Warning, Interface %s failed to register multicast \n", rip_intf_array[device_index]->intf_name);
	}
	return RIP_SUCCESS;
}

/*===========================================================
������:      rip_disable_multicast_in
��������: ��ֹ�ӿڷ����鲥����
�������: device_index : �߼��˿ں�
�������: ��
����ֵ:      ����ɹ�,����RIP_SUCCESS
                       ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_disable_multicast_in( uint32 device_index )
{
	int ret;
	struct ip_mreq mreq;

	/*��������Ϸ��Լ���*/
   	if( device_index > INTERFACE_DEVICE_MAX_NUMBER )
    {
		return RIP_FAIL;
    }
#ifndef R1705 
	memset( (void *)&mreq, 0, sizeof(struct ip_mreq) );

	mreq.imr_interface.s_addr   = device_index;
	mreq.imr_mcastaddr.s_addr  = RIP_MULTICAST_ADDRESS;
	ret = so_setsockopt( rip_glb_info.socket_id, IPPROTO_IP, IP_DROP_MEMBERSHIP_INTF, (char *)&mreq, sizeof(mreq) );
#else
	ret = ip_del_mcast(device_index, htonl(RIP_MULTICAST_ADDRESS));
#endif
	if( ret < 0 )
    {
		rip_debug(RIP_DEBUG_IP_RIP_PROTOCOL, "%%Warning, Interface %s failed to unregister multicast \n", rip_intf_array[device_index]->intf_name);
    }
	
	return RIP_SUCCESS;
}

/*===========================================================
������:      rip_exist_equi_route
��������: �ж�·���Ƿ��еȼ�·��
�������: rip_route : ·��
�������: ��
����ֵ:      ����ɹ�,����TRUE
                       ����ʧ��,����FALSE
��ע:
=============================================================*/
BOOL rip_exist_equi_route( struct rip_route_ *rip_route )
{
	struct rip_route_ *rip_route_head;

	rip_route_head = (struct rip_route_ *)(rip_route->route_node->info);

	if( rip_route_head->equi_route_num > 1 )
	{
		return TRUE;
	}

	return FALSE;
}

/*===========================================================
������:      rip_del_from_main_tbl
��������: ����·�ɱ���ɾ��·��
�������: rip_route : ·��
�������: ��
����ֵ:      ����ɹ�,����TRUE
          ����ʧ��,����FALSE
��ע:Fengsongbai add,2006-02-13 ��·�ɱ���ֻ����ȱʡ4�������8���ȼ�·�ɣ����
�˴��ľ���rip���ܴ��ڶ����ȼ�·�ɣ�Ҳֻ�����4����8��������·�ɱ�
=============================================================*/
void rip_del_from_main_tbl( struct rip_route_ *rip_route ,uint32 processid)
{
	struct route_node *rip_route_node;
	struct rip_route_ *rip_route_head, *rip_route_temp;	
	struct rt_gateway *rip_gateway_temp;
	struct rt_gateway *rip_gateway[MAX_EQUAL_RTNUM] = {NULL}; 
	/*    
	struct rt_gateway rip_gateway_temp[100];
	struct rt_gateway *rip_gateway[100];
	*/ 
	uint16 gateway_num;
	uint16 equal_rtnum;
	int count;    
  
	rip_route_node = rip_route->route_node;
	rip_route_head = rip_route_node->info;
	
	/*�ȴ���·�ɱ���ɾ��*/
	rt_delete_protort_notify( rip_route->pprocess->vrf_id,
							rip_route->route_node->p.u.prefix4.s_addr,
							prefix_to_mask(rip_route->route_node->p.prefixlen),
						      RTPROTO_RIP,
						      processid );

	if(( rip_route_head == NULL) || ( rip_route_head->equi_route_num == 0 ))
	{
		return;
	}
    
    /*����������·��,�ټ�����·�ɱ���*/
    equal_rtnum = rt_get_equal_rtnum_confg();    
    rip_gateway_temp = ( struct rt_gateway *)rip_mem_malloc( sizeof(struct rt_gateway)*equal_rtnum, RIP_RT_GATEWAY_TYPE );
	if(!rip_gateway_temp)return;
	bzero( rip_gateway_temp, sizeof(struct rt_gateway)*equal_rtnum);
 
/*	
	for( i = 0; i < equal_rtnum; i++ )
	{
		memset( (void *)&rip_gateway_temp[i], 0, sizeof(struct rt_gateway) );
		rip_gateway[i] = NULL;  
	}
*/
    /*����·�ɱ�����ӵȼ�·��*/	
	gateway_num = 0;	
	for( count = 1, rip_route_temp = rip_route_head; count <= rip_route_head->equi_route_num; rip_route_temp = rip_route_temp->forw, count++ )
	{	
		/*not add metric = 16 rip route to rt table*/
		if( (RIP_NBR_ROUTE == rip_route_temp->route_type)
			&& (rip_route_temp->distance >= RIP_MAX_DISTANCE || rip_route_temp->metric >= RIP_MAX_METRIC)) 
		{
			continue;
		}

		/*Fengsb 2006-02-19 ע�⣬ֻ������·�ɱ����ѧϰ����·�ɣ�
		��Ϊת��������·�ɷ���route node�ĵ�һ���ڵ㣬������Ҫ����
		�ж�*/
		if(RIP_REDIS_ROUTE == rip_route_temp->route_type)
		{
			continue;
		}
		if(RIP_SUMMARY_ROUTE == rip_route_temp->route_type)
		{
			continue;
		}
		/*fengsb add the following  if case 2006-05-18*/
		if(RIP_DEF_ROUTE == rip_route_temp->route_type)
		{
			continue;
		}
/*		memset( (void *)&rip_gateway_temp[gateway_num], 0, sizeof(struct rt_gateway) ); */

		if( rip_route_temp->next_hop != 0 )
		{
			rip_gateway_temp[gateway_num].gateway = rip_route_temp->next_hop;
		}
		else
		{
			rip_gateway_temp[gateway_num].gateway = rip_route_temp->gw_addr;
		}

		rip_gateway_temp[gateway_num].flags = 1;
		if(rip_intf_array[rip_route_temp->gw_index])
			rip_gateway_temp[gateway_num].aid = rip_intf_array[rip_route_temp->gw_index]->aid;
		rip_gateway_temp[gateway_num].index   = rip_route_temp->gw_index;
		rip_gateway[gateway_num] = &rip_gateway_temp[gateway_num];
        
		/*Fengsb 2006-02-13, ���ܳ�����·�ɱ�����ĵȼ�·����Ŀ*/
		gateway_num++;
		if(gateway_num >= equal_rtnum)
		{
			break;
		}
	}

	if(gateway_num > 0)
	{
		rt_add_protort_notify( rip_route->pprocess->vrf_id, 
							rip_route->route_node->p.u.prefix4.s_addr,
							prefix_to_mask(rip_route->route_node->p.prefixlen),
							RTPROTO_RIP,
							gateway_num,
							rip_gateway,
							0,
							NULL,
							RTS_INTERIOR|RTS_ELIGIBLE,
							rip_route->metric,
							rip_route->distance,
							rip_route->route_tag,
							processid, 0);
	}
	rip_mem_free(rip_gateway_temp, RIP_RT_GATEWAY_TYPE);
	return;	
}

/*===========================================================
������:      rip_change_notify
��������: ֪ͨ��·�ɱ���·�ɸı�
�������: rip_route : ·��
�������: ��
����ֵ:      ��
��ע:
=============================================================*/
void rip_change_notify( struct rip_route_ *rip_route )
{
	char string[100];
	struct rip_process_info_ *pprocess;

	if(!(pprocess = rip_route->pprocess))
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return ;
	}
		
	/*debug*/
	if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_DATABASE )
	{
		sprintf(string, "RIP-DB: route %s/%d", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
		sprintf(string, "%s via %s distance changed to %ld", string, ip_ntoa( rip_route->gw_addr ), rip_route->distance );
		rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%s\n", string );

		sprintf(string, "RIP-DB: route %s/%d", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
		sprintf(string, "%s via %s metric changed to %ld", string, ip_ntoa( rip_route->gw_addr ), rip_route->metric );
		rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%s\n", string );
	}
	
	rip_del_from_main_tbl(rip_route, pprocess->process_id);

	pprocess->rip2GlobalRouteChanges++;   
	
	return;
}

/*===========================================================
������:      rip_add_notify
��������: ����·�ɱ������·��
�������: rip_route : ·��
�������: ��
����ֵ:      ��
��ע:
=============================================================*/
void rip_add_notify( struct rip_route_ *rip_route )
{
	rip_del_from_main_tbl( rip_route ,rip_route->pprocess->process_id);
	return;
}

/*===========================================================
������:      rip_clear_peer_route_list
��������: ɾ����peerѧϰ����·��
�������: device_index : �߼��˿ں�
		  peer_addr: peer��ַ,0��ʾ����
�������: ��
����ֵ:      ����ɹ�,����RIP_SUCCESS
             ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_clear_peer_route_list(uint32 device_index,uint32 peer_addr)
{
	int ret;
	struct rip_route_list_ *rip_route_list_temp;
	struct rip_route_list_ *rip_route_list_forw;
	struct rip_route_ *rip_route;
	
	/*��������Ϸ��Լ��*/
	if( (device_index > INTERFACE_DEVICE_MAX_NUMBER)
		|| (NULL == rip_intf_array[device_index]) )
	{
		return RIP_FAIL;
	}
	
	/*�Զ˿�ѧϰ����·�ɵĴ���*/
	rip_route_list_temp = rip_intf_array[device_index]->nbr_route_list.forw;
	while( rip_route_list_temp != &(rip_intf_array[device_index]->nbr_route_list) )
	{
		rip_route_list_forw = rip_route_list_temp->forw;
		rip_route = rip_route_list_temp->rip_route;
		if(peer_addr&&(peer_addr!=rip_route->gw_addr))
		{
			rip_route_list_temp = rip_route_list_forw;
			continue;
		}
		if( TRUE == rip_exist_equi_route( rip_route) )
		{
			/*��holddown�б���ɾ��*/
			rip_del_holddown( rip_route );
			
			/*���еȼ�·��,��·�ɴ�·�ɱ���ɾ��*/
			ret = rip_del_route( rip_route );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return ret;
			}

			REMQUE( rip_route_list_temp );
			rip_mem_free( rip_route_list_temp, RIP_ROUTE_LIST_TYPE );

			rip_intf_array[device_index]->nbr_route_num--;
		}
		else
		{
			/*���޵ȼ�·��,����·�ɽ���holddown״̬,
			  ·�ɴ���·�ɱ���ɾ��*/
			rip_add_holddown( rip_route );

			rt_delete_protort_notify( rip_intf_array[device_index]->vrf_id,
			    					rip_route->route_node->p.u.prefix4.s_addr,
			    					prefix_to_mask(rip_route->route_node->p.prefixlen),
			    					RTPROTO_RIP, 0 );
		}

		rip_route_list_temp = rip_route_list_forw;
	};

	return RIP_SUCCESS;
}

/*===========================================================
������:      rip_clear_nbr_route_list
��������: ɾ���˿�ѧϰ����·��
�������: device_index : �߼��˿ں�
�������: ��
����ֵ:      ����ɹ�,����RIP_SUCCESS
                       ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_clear_nbr_route_list( uint32 device_index )
{
	return rip_clear_peer_route_list(device_index,0);
}

/*===========================================================
������:      rip_del_from_nbr_list
��������: ��·�ɴ�gw_index��ѧϰ�б���ɾ��
�������: rip_route : ·��
�������: ��
����ֵ:      ��
��ע:
=============================================================*/
void rip_del_from_nbr_list( struct rip_route_ *rip_route )
{
	struct rip_route_list_ *rip_route_list_temp;
	uint32 device_index;

	/*ֻ��ѧϰ����·�ɽ��д���*/
	if(!rip_route || RIP_NBR_ROUTE != rip_route->route_type )
	{
		return;
	}

    device_index = rip_route->gw_index;
    rip_route_list_temp = rip_route->nbr_ptr;
    if(rip_route_list_temp)
	{
        REMQUE( rip_route_list_temp );
        rip_mem_free( rip_route_list_temp, RIP_ROUTE_LIST_TYPE );
        rip_intf_array[device_index]->nbr_route_num--;
        rip_route->nbr_ptr = NULL;
    }
    
#if 0
	/*�Զ˿�ѧϰ����·�ɵĴ���*/
	for( rip_route_list_temp = rip_intf_array[device_index]->nbr_route_list.forw;
		rip_route_list_temp != &(rip_intf_array[device_index]->nbr_route_list);
		rip_route_list_temp = rip_route_list_temp->forw )
	{
		if( rip_route_list_temp->rip_route == rip_route )
		{
			REMQUE( rip_route_list_temp );
			rip_mem_free( rip_route_list_temp, RIP_ROUTE_LIST_TYPE );

			rip_intf_array[device_index]->nbr_route_num--;

			return;
		}
	}
#endif
	return;
}

#if 0
/*===========================================================
������:      rip_set_intf_ref_num
��������: ��ȡ�˿ڱ�RIP���縲�ǵĴ���
�������: device_index : �߼��˿ں�
�������: ��
����ֵ:      ��
��ע:
=============================================================*/
void rip_set_intf_ref_num( uint32 device_index )
{
	uint32 vrf_id;
	struct rip_net_list_ *rip_net_temp;
	uint32 mask;
	
	/*��������Ϸ��Լ��*/
	if( (device_index > INTERFACE_DEVICE_MAX_NUMBER)
		|| (NULL == rip_intf_array[device_index]) )
	{
		return;
	}

	vrf_id = rip_intf_array[device_index]->vrf_id;

	if( NULL == rip_tbl_array[vrf_id] )
	{
		return;
	}

	for( rip_net_temp = rip_tbl_array[vrf_id]->network_list.forw;
		rip_net_temp != &(rip_tbl_array[vrf_id]->network_list);
		rip_net_temp = rip_net_temp->forw )
	{
		if( 0 == rip_net_temp->mask )
		{
			/*network A.B.C.D*/
			mask = rip_get_mask( rip_net_temp->network ); 
		}
		else
		{
			mask = rip_net_temp->mask;
		}

		if( (rip_net_temp->network & mask) == (rip_intf_array[device_index]->address & mask))
		{
			rip_intf_ref_num_set(device_index, RIP_INTF_REF_NUM_ADD_1);
			
			/* ��Э��up��ʱ����intfû�б�rip���ǽ�����enable multicast��
			����enable
			 * Commented by dangzhw in 2009.11.06 15:34:15 */
			if(( BIT_TEST( rip_intf_array[device_index]->link_state ,RIP_INTF_LINK_UP)) && rip_intf_array[device_index]->ref_num == 1)
			{
				if( RIP_SUCCESS != rip_enable_multicast_in( device_index ) )
				{
					return ;
				}
			}
		}
	}

	return;
}
#endif 
/*===========================================================
������:      rt_get_equal_rtnum_confg
��������: ��ȡroutingģ�����õ����ȼ�·����Ŀ
�������: 
�������: ��
����ֵ:      ��
��ע:
=============================================================*/
uint16 rt_get_equal_rtnum_confg(void)
{
    return DEFAULT_EQUAL_RTNUM;
}

/*===========================================================
������:      rip_create_orig_route
��������: ����RIP����ȱʡ·��
�������: vrf_id: 
�������: ��
����ֵ:       ����ɹ�,����RIP_SUCCESS
                        ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_create_orig_route( struct rip_process_info_ *pprocess)
{	
    struct prefix node_prefix;
    struct route_node *rip_route_node;
    struct rip_route_ *rip_route, *rip_route_head;

    if(!pprocess)
    {
    	rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
        return RIP_FAIL;
    }

	/*xuhaiqing 2010-12-21�Ž����·����Ŀ������ȡ��*/
    /*·�ɱ���·�������������ֵ���������*/
    /*if(pprocess->route_num >= pprocess->max_route_num )
    {
        rip_warning("%%Warning, Limit for rip routes exceeded, max # is %d.\n", pprocess->max_route_num );
        return RIP_SUCCESS;
    }*/

    memset( &node_prefix, 0, sizeof(struct prefix) );
    node_prefix.family = AF_INET;
    node_prefix.safi = SAFI_UNICAST;
    node_prefix.prefixlen = mask_to_prefix( 0 );
    node_prefix.u.prefix4.s_addr = 0;
    rip_route_node = route_node_get( pprocess->rip_table, &node_prefix ,NULL );
    if(rip_route_node == NULL)
    {
        return RIP_FAIL;
    }
    rip_route_head = (struct rip_route_ *)(rip_route_node->info);
    if (rip_route_head)
    {            
        if(RIP_DEF_ROUTE == rip_route_head->route_type)
        {                       
            /* not need to create new node, so we must unlock once */
            route_unlock_node(rip_route_node);
            /*already exist default rip route, so we needn't do anything */
	        return RIP_SUCCESS;
        }          
    }

    /*���rip_route������Ϣ*/
    rip_route = (struct rip_route_ *)rip_mem_malloc(sizeof(struct rip_route_), RIP_ROUTE_TYPE);
	if(!rip_route)
	{
		route_unlock_node(rip_route_node);
		return RIP_FAIL;
	}
    memset( (void *)rip_route, 0, sizeof(struct rip_route_) );
    rip_route->forw = rip_route;
    rip_route->back = rip_route;

    rip_route->route_tag = RIP_INTERNAL_ROUTE;
    rip_route->route_type = RIP_DEF_ROUTE;
    rip_route->refresh_time = time_sec;
    rip_route->pprocess= pprocess;
    rip_route->gw_index = 0;
    rip_route->gw_addr = 0; /*ֱ��·��,�������ַ*/
    /*Fengsb add 2006-04-30 : this distance is the distance that no consider network and mask*/
    rip_route->distance = pprocess->distance_list->default_distance;            
    rip_route->metric    = RIP_DIRECT_METRIC;
    rip_route->hold_ptr = NULL;

    /*·�ɼ���route node */
    if(rip_route_head)
    {
        INSQUE(rip_route, rip_route_head->back);
		rip_route->equi_nbr_num = rip_route_head->equi_nbr_num;
		rip_route->equi_route_num = rip_route_head->equi_route_num;
    }

	rip_route_node->info = (void *)rip_route;
	rip_route_head = rip_route;
    rip_route->route_node = rip_route_node;
    pprocess->route_num++;            
    rip_route_head->equi_route_num++;        
    
    /* modify by fang qi in order to solve the double free problem on 2007-01-12 */
    /*rip default route is the hightest privilige, so remove the other kind of 0.0.0.0 route */
#if 0
    rip_route_head = rip_route_node->info;
    for( count = 1, rip_rt = rip_route_head; count <= rip_route_head->equi_route_num; rip_rt = rip_route_tmp, count++ )
    {
        rip_route_tmp = rip_rt->forw;
        if(RIP_DEF_ROUTE != rip_rt->route_type)
        {                       
            /*��holddown��ɾ��*/
            rip_del_holddown( rip_rt );
			
            /*�Ӷ˿�ѧϰ·����ɾ��*/
            rip_del_from_nbr_list( rip_rt );
			
            /*ɾ��·��*/
            ret = rip_del_route( rip_rt );
            if( RIP_SUCCESS != ret )
            {
                vty_output("rip_create_orig_route:del route %s/%d err.\n", \
                	ipv4_ntoa(rip_rt->route_node->p.u.prefix4.s_addr), rip_rt->route_node->p.prefixlen);
                return RIP_FAIL;
            }
        }               		
    }  
#endif
    /*����·�ɼ��봥�������б�*/
    rip_add_trigger( rip_route, pprocess, FALSE );
    rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "RIP-DB: Adding default route %s/%d to RIP database\n", ip_ntoa(node_prefix.u.prefix4.s_addr), node_prefix.prefixlen );
    pprocess->rip2GlobalRouteChanges++;
    
    return RIP_SUCCESS;
}

/*===========================================================
������:      rip_del_orig_route
��������: ɾ��RIP����ȱʡ·��
�������: vrf_id: 
�������: ��
����ֵ:      ����ɹ�,����RIP_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_del_orig_route( struct rip_process_info_ *pprocess )
{
	struct prefix node_prefix;
	struct route_node *rip_route_node;
	struct rip_route_ *rip_route;
	int ret;       
	
	if(!pprocess)
	{
		return RIP_FAIL;
	}

    memset( &node_prefix, 0, sizeof(struct prefix) );
	node_prefix.family = AF_INET;
	node_prefix.safi = SAFI_UNICAST;
	node_prefix.prefixlen = mask_to_prefix( 0 );
	node_prefix.u.prefix4.s_addr = 0;
    rip_route_node = route_node_lookup( pprocess->rip_table, &node_prefix );
	if(rip_route_node == NULL)
	{
		return RIP_SUCCESS;
	}
	rip_route = (struct rip_route_ *)(rip_route_node->info);
	route_unlock_node(rip_route_node);
	
	if(RIP_DEF_ROUTE == rip_route->route_type)
    {                    
        ret = rip_del_route( rip_route);
        if( RIP_SUCCESS != ret )
        {
            syslog(RIP_LOG_ERR, "rip_del_orig_route: delete rip default route error\n");
            return RIP_FAIL;
        }
    }
        
	return RIP_SUCCESS;
}
