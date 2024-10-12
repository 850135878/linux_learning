char rip_init_version[] = "rip_init.c last changed at 2016.01.07 15:29:33 by yuguangcheng\n";
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
 *        FILE NAME       :   Rip_init.c
 *        AUTHOR          :   HQ.Sun
 *        DATE            :   2006.01.08 11:13:58
 *        PURPOSE:        :   RIPģ���ʼ������ģ��
 *
**************************************************************************/
#include <rip/rip_include.h>
#include <rip/rip_main.h>
#include <rip/rip_redis.h>
#include <rip/rip_packet.h>
#include <rip/rip_init.h>
#include <rip/rip_intf.h>
#include <rip/rip_timer.h>
#include <rip/rip_cmd.h>
#include <rip/rip_debug.h>

extern int rip_register_mib( void );
extern int rip_create_timer( void );
extern int rip_intf_creat_timer(uint32 device_index);
void rip_version_register(void);
void  rip_filter_callback(uint64 protocol,uint32 process,unsigned long long if_eterid, uint32 vrfid);
void  rip_offset_callback(uint64 protocol,uint32 process,unsigned long long if_eterid, uint32 vrfid);
void rip_distance_callback(uint64 protocol,uint32 process,uint32 vrf_id);
uint32 sm_create(int8 name[4], uint32 count, uint32 flags, uint32 *smid);
uint32 mu_create(int8 name[4], uint32 count, uint32 flags, uint32 *smid);

extern struct rip_glb_info_  rip_glb_info;
extern struct rip_intf_ *rip_intf_array[INTERFACE_DEVICE_MAX_NUMBER + 1];
extern struct rip_process_list_  rip_process_list;
#if 0
/*===========================================================
������:   rip_init_glb_info
��������: ��ʼ��RIPȫ����Ϣ
�������: ��
�������: ��
����ֵ:   ��
��ע:
=============================================================*/
int rip_init_glb_info( uint32 vrf_id )
{
	int ret;
	if(vrf_id == 0)
	{
		rip_glb_info.enable = RIP_ENABLE;

		rip_glb_info.src_check_flag = RIP_SRC_CHECK_ENABLE;
 		
 		rip_glb_info.update_timeout = RIP_DEF_UPDATE_TIMEOUT;        // ·�ɸ���ʱ�� 30s
		rip_glb_info.expire_timeout = RIP_DEF_EXPIRE_TIMEOUT;        // ��ʱʱ�� 180s
		rip_glb_info.holddown_timeout = RIP_DEF_HOLDDOWN_TIMEOUT;    // ����ʱ�� 120s
		rip_glb_info.trigger_timeout = RIP_DEF_TRIGGER_MAX_TIMEOUT;	 // �������·�����ȴ�������Ӧȷ�ϻ���Ӧ�����ʱ�� 5s
		
		rip_glb_info.peer_timeout = RIP_DEF_PEER_TIMEOUT;
		
		/*����RIPģ�鶨ʱ��*/
		ret = rip_start_timer();
		if( RIP_SUCCESS != ret )
		{
			return RIP_FAIL;
		}

		// ����˫�����ͷ�ڵ�
		memset( &(rip_glb_info.holddown_list), 0, sizeof(struct rip_route_list_) );
		rip_glb_info.holddown_list.forw = &(rip_glb_info.holddown_list);
		rip_glb_info.holddown_list.back = &(rip_glb_info.holddown_list);

		// ��ȡ����
		memset( &(rip_glb_info.trigger_list), 0, sizeof(struct rip_trigger_list_) );
		rip_glb_info.trigger_list.forw = &(rip_glb_info.trigger_list);
		rip_glb_info.trigger_list.back = &(rip_glb_info.trigger_list);
		
		memset( &(rip_glb_info.neigh_list), 0, sizeof(struct rip_neigh_list_) );
		rip_glb_info.neigh_list.forw = &(rip_glb_info.neigh_list);
		rip_glb_info.neigh_list.back = &(rip_glb_info.neigh_list);
		
		    /*////////////////////////////////////////////////*/
		rip_glb_info.rip_redis_link = init_proto_independent( RTPROTO_RIP, 0, rip_redis_callback );

		memset( &(rip_glb_info.md5_record), 0, sizeof(struct rip_md5_record_) );
		rip_glb_info.md5_record.forw = &(rip_glb_info.md5_record);
		rip_glb_info.md5_record.back = &(rip_glb_info.md5_record);

		rip_glb_info.rip2GlobalRouteChanges = 0;
		rip_glb_info.rip2GlobalQueries = 0; 
	   
	}

	/*����RIP·�ɱ�*/
	rip_create_table(vrf_id);

	return RIP_SUCCESS;
}
#endif
/*===========================================================
������:   rip_clear_glb_info
��������: ���RIPȫ����Ϣ
�������: ��
�������: ��
����ֵ:   ��
��ע:
=============================================================*/
void rip_clear_process_info( struct rip_process_info_ *pprocess )
{
	/*struct rip_route_list_ *rip_hold_list, *rip_hold_forw;*/
	struct rip_neigh_list_ *rip_neigh_list, *rip_neigh_forw;
	/*struct rip_trigger_list_ *rip_trigger_list, *rip_trigger_forw;*/
	struct rip_md5_record_ *rip_md5_record, *rip_md5_record_forw;
	struct rip_peer_list_ *peer ,*peer_temp;
	
	/*rip_hold_list = pprocess->holddown_list.forw;
	while( rip_hold_list != &(pprocess->holddown_list) )
	{
		rip_hold_forw = rip_hold_list->forw;
		if(rip_hold_list->rip_route)rip_hold_list->rip_route->hold_ptr=NULL;
		REMQUE( rip_hold_list );
		rip_mem_free( rip_hold_list, RIP_ROUTE_LIST_TYPE );
		rip_hold_list = rip_hold_forw;
	};*/
	/* clear process info֮ǰ��Ȼ�Ѿ�clear table�ˣ�hold_listӦ���Ѿ�Ϊ�գ������ٴ����� */

#if 0
	/*Clear trigger list*/
	rip_trigger_list = pprocess->trigger_list.forw;
	while( rip_trigger_list != &(pprocess->trigger_list) )
	{
		rip_trigger_forw = rip_trigger_list->forw;

		REMQUE( rip_trigger_list );
		rip_mem_free( rip_trigger_list, RIP_TRIGGER_LIST_TYPE );

		rip_trigger_list = rip_trigger_forw;
	};
#endif
	route_table_destroy( pprocess->trigger_table, rip_del_trigger_tree_callback );

	rip_neigh_list = pprocess->neigh_list.forw;
	while( rip_neigh_list != &(pprocess->neigh_list) )
	{
		rip_neigh_forw = rip_neigh_list->forw;

		REMQUE( rip_neigh_list );
		rip_mem_free( rip_neigh_list, RIP_NEIGH_LIST_TYPE );

		rip_neigh_list = rip_neigh_forw;
	};

	clear_proto_independent( pprocess->rip_redis_link );

	rip_md5_record = pprocess->md5_record.forw;
	while( rip_md5_record != &(pprocess->md5_record) )
	{
		rip_md5_record_forw = rip_md5_record->forw;

		REMQUE( rip_md5_record );
		rip_mem_free( rip_md5_record, RIP_MD5_RECORD_TYPE );

		rip_md5_record = rip_md5_record_forw;
	};


	/* 
	 * Commented by dangzhw in 2009.12.06 10:01:10 */
	peer = pprocess->peer_list.forw;
	while(peer != &pprocess->peer_list)
	{
		peer_temp = peer->forw ;

		REMQUE(peer);
		rip_mem_free(peer,RIP_PEER_LIST_TYPE);

		peer = peer_temp;
	}


	/* ������̽ڵ㣬����ʱû��rip���̴��ڣ�ȡ�����ĵ��ض���
	 * Commented by dangzhw in 2010.03.11 14:39:42 */
	REMQUE(pprocess->pprocess_list);
	rip_mem_free(pprocess->pprocess_list, RIP_PROCESS_LIST);
	rip_mem_free(pprocess, RIP_PROCESS_INFO);

	if(rip_process_list.back == rip_process_list.forw && rip_process_list.back == &rip_process_list)
		rip_socket_redirect_disable();
		
	return;
}

/*===========================================================
������:   rip_socket_init
��������: RIPģ��socket��ʼ��
�������: ��
�������: ��
����ֵ:   �����ɹ�,����RIP_SUCCESS
       ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_socket_init( void )
{
    struct soaddr_in sa;
    
	/*����socket*/
	if ((rip_glb_info.socket_id = task_get_socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
 		return RIP_FAIL;
    }

	if (task_set_option(rip_glb_info.socket_id, TASKOPTION_NONBLOCKING, TRUE) < 0)
	{
		return RIP_FAIL;
	}

	/*����socketѡ�����*/
	// ���ù㲥ѡ��
	if (task_set_option(rip_glb_info.socket_id, TASKOPTION_BROADCAST, TRUE) < 0) 
	{
		return RIP_FAIL;
	}

	/*send buffer*/
	// ���÷��ͻ������Ĵ�С
	if (task_set_option(rip_glb_info.socket_id, TASKOPTION_SENDBUF, RIP_MAX_PACKET_SIZE) < 0) 
	{
		return RIP_FAIL;
	}

	/*receive buffer*/
	// ���ý��ջ������Ĵ�С
	if (task_set_option(rip_glb_info.socket_id, TASKOPTION_RECVBUF, RIP_MAX_PACKET_SIZE*rip_glb_info.input_queue ) < 0) 
	{
		return RIP_FAIL;
	}

	/*receive IP destination address w/dgram*/
	// ���� RCVDSTADDR ѡ�������ݰ���Ŀ���ַ
	if (task_set_option(rip_glb_info.socket_id, TASKOPTION_RCVDSTADDR, TRUE) < 0) 
	{
		return RIP_FAIL;
	}
	
	/*ip time to live*/
	// �������ݰ��������е�����ʱ��
	if (task_set_option(rip_glb_info.socket_id, TASKOPTION_TTL, TRUE) < 0) 
	{
		return RIP_FAIL;
	}
	
	/*set ip multicast loopback*/
	// ���öಥ����(����������ʱ���������͵Ķಥ���ݰ�Ҳ�ᱻͬһ��������)
	if (task_set_option(rip_glb_info.socket_id, TASKOPTION_MULTI_LOOP, FALSE) < 0) 
	{
		return RIP_FAIL;
	}

	/*set IP multicast ttl*/
	// ���öಥ���ݰ���TTLֵ
	if (task_set_option(rip_glb_info.socket_id, TASKOPTION_MULTI_TTL, TRUE) < 0) 
	{
		return RIP_FAIL;
	}

	/*--------------------ע��socket------------------*/
	memset((void *) &sa, 0, sizeof(struct soaddr_in));
	sa.sin_len    = sizeof(struct soaddr_in);
	sa.sin_family = AF_INET;
	sa.sin_port   = htons(RIP_PORT); 
	
	if( task_addr_local( rip_glb_info.socket_id, &sa, rip_glb_info.queue_id ) ) 
	{
		return RIP_FAIL;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_create_timer
��������: ����RIPģ�鶨ʱ��
�������: ��
�������: ��
����ֵ:   �����ɹ�,����RIP_SUCCESS
                     ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_process_create_timer( struct rip_process_info_ *pprocess )
{
	TIMER_USER_DATA timer_user_data;
	uint32 ret;
	
	/*Register rip timer*/
	
	memset( (void *)&timer_user_data, 0, sizeof(TIMER_USER_DATA) );	
	timer_user_data.msg.qid = (MSG_Q_ID)rip_glb_info.queue_id;
	timer_user_data.msg.msg_buf[0] = MSG_RIP_UPDATE_TIMEOUT;
	timer_user_data.msg.msg_buf[1] = pprocess->process_id;
	ret = sys_add_timer(TIMER_MSG_METHOD, &timer_user_data, &(pprocess->update_timer_id) );
	if( SYS_NOERR != ret )
	{
		return RIP_FAIL;
	}
	
	memset( (void *)&timer_user_data, 0, sizeof(TIMER_USER_DATA) );
	timer_user_data.msg.qid = (MSG_Q_ID)rip_glb_info.queue_id;
	timer_user_data.msg.msg_buf[0] = MSG_RIP_EXPIRE_TIMEOUT;
	timer_user_data.msg.msg_buf[1] = pprocess->process_id;
	ret = sys_add_timer( TIMER_MSG_METHOD, &timer_user_data, &(pprocess->expire_timer_id) );
	if( SYS_NOERR != ret )
	{
		return RIP_FAIL;
	}

	memset( (void *)&timer_user_data, 0, sizeof(TIMER_USER_DATA) );
	timer_user_data.msg.qid = (MSG_Q_ID)rip_glb_info.queue_id;
	timer_user_data.msg.msg_buf[0] = MSG_RIP_HOLDDOWN_TIMEOUT;
	timer_user_data.msg.msg_buf[1] = pprocess->process_id;
	ret = sys_add_timer( TIMER_MSG_METHOD, &timer_user_data, &(pprocess->holddown_timer_id) );
	if( SYS_NOERR != ret )
	{
		return RIP_FAIL;
	}

	memset( (void *)&timer_user_data, 0, sizeof(TIMER_USER_DATA) );
	timer_user_data.msg.qid = (MSG_Q_ID)rip_glb_info.queue_id;
	timer_user_data.msg.msg_buf[0] = MSG_RIP_TRIGGERED_TIMEOUT;
	timer_user_data.msg.msg_buf[1] = pprocess->process_id;
	ret = sys_add_timer( TIMER_MSG_METHOD, &timer_user_data, &(pprocess->trigger_timer_id) );
	if( SYS_NOERR != ret )
	{
		return RIP_FAIL;
	}

	/* ����rip peer��ʱ��ʱ��
	 * Commented by dangzhw in 2009.12.05 09:31:58 */
	memset( (void *)&timer_user_data, 0, sizeof(TIMER_USER_DATA) );
	timer_user_data.msg.qid = (MSG_Q_ID)rip_glb_info.queue_id;
	timer_user_data.msg.msg_buf[0] = MSG_RIP_PEER_TIMEOUT;
	timer_user_data.msg.msg_buf[1] = pprocess->process_id;
	ret = sys_add_timer( TIMER_MSG_METHOD, &timer_user_data, &(pprocess->peer_timer_id) );
	if( SYS_NOERR != ret )
	{
		return RIP_FAIL;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_start_timer
��������: ����RIPģ�鶨ʱ��
�������: ��
�������: ��
����ֵ:   �����ɹ�,����RIP_SUCCESS
                     ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_start_process_timer( struct rip_process_info_ *pprocess)
{
	int ret;
	
	ret = sys_start_timer( pprocess->update_timer_id, pprocess->update_timeout );
	if( SYS_NOERR != ret )
	{
		return RIP_FAIL;
	}
	
	ret = sys_start_timer( pprocess->expire_timer_id, pprocess->expire_timeout );
	if( SYS_NOERR != ret )
	{
		return RIP_FAIL;
	}
	
	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_stop_timer
��������: ֹͣRIPģ�鶨ʱ��
�������: ��
�������: ��
����ֵ:   �����ɹ�,����RIP_SUCCESS
                     ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_process_stop_timer( struct rip_process_info_* pprocess )
{
	uint32 ret;

	/*ֹͣ��ʱ��*/
	ret = sys_stop_timer( pprocess->update_timer_id );
	if( ret != SYS_NOERR )
	{
		return RIP_FAIL;
	}
	
	ret = sys_stop_timer( pprocess->expire_timer_id );
	if( ret != SYS_NOERR )
	{
		return RIP_FAIL;
	}
	
	ret = sys_stop_timer( pprocess->holddown_timer_id );
	if( ret != SYS_NOERR )
	{
		return RIP_FAIL;
	}
	
	ret = sys_stop_timer( pprocess->trigger_timer_id );
	if( ret != SYS_NOERR )
	{
		return RIP_FAIL;
	}

	ret = sys_stop_timer( pprocess->peer_timer_id );
	if( ret != SYS_NOERR )
	{
		return RIP_FAIL;
	}
	
	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_delete_timer
��������: ɾ��RIPģ�鶨ʱ��
�������: ��
�������: ��
����ֵ:   �����ɹ�,����RIP_SUCCESS
                     ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_process_delete_timer( struct rip_process_info_ *pprocess )
{
	uint32 ret;

	/*ֹͣ��ʱ��*/
	ret = sys_delete_timer( pprocess->update_timer_id );
	if( ret != SYS_NOERR )
	{
		return RIP_FAIL;
	}
	pprocess->update_timer_id = 0;
	
	ret = sys_delete_timer( pprocess->expire_timer_id );
	if( ret != SYS_NOERR )
	{
		return RIP_FAIL;
	}
	pprocess->expire_timer_id = 0;
	
	ret = sys_delete_timer( pprocess->holddown_timer_id );
	if( ret != SYS_NOERR )
	{
		return RIP_FAIL;
	}
	pprocess->holddown_timer_id = 0;
	
	ret = sys_delete_timer( pprocess->trigger_timer_id );
	if( ret != SYS_NOERR )
	{
		return RIP_FAIL;
	}
	pprocess->trigger_timer_id = 0;

	ret = sys_delete_timer( pprocess->peer_timer_id );
	if( ret != SYS_NOERR )
	{
		return RIP_FAIL;
	}
	pprocess->peer_timer_id= 0;
	
	return RIP_SUCCESS;
}


/*******************************************************************************
 *
 * FUNCTION	:	rip_process_create_table
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.26 10:25:21
 *
*******************************************************************************/
void rip_process_create_table( struct rip_process_info_ *pprocess )
{
#if 0	/*network*/
	pprocess->network_list.forw = &(pprocess->network_list);
	pprocess->network_list.back = &(pprocess->network_list);
#endif
	pprocess->distance_list = route_distance_list_init(FG_RIP, pprocess->process_id, \
			rip_distance_callback, pprocess->vrf_id);
	assert(pprocess->distance_list);
    
	/*filter */	
	pprocess->filter_list = route_distribute_list_init( FG_RIP, pprocess->process_id, \
				rip_filter_callback, pprocess->vrf_id);
	assert(pprocess->filter_list);

	/*offset*/
	pprocess->offset_list = route_offset_list_init(FG_RIP, pprocess->process_id, \
		RIP_DEF_METRIC, RIP_MAX_METRIC, rip_offset_callback, pprocess->vrf_id);
	assert(pprocess->offset_list);	
	
	/*initial redistribute list*/
	QUE_INIT(&(pprocess->redis_list));	
    
	pprocess->default_route_flag = RIP_DEF_ROUTE_DISABLE;

#if 0
	if( 0 == vrf_id )
	{
		pprocess->auto_summary_flag = RIP_AUTO_SUMMARY_ENABLE;
	}
	else
	{
		pprocess->auto_summary_flag = RIP_AUTO_SUMMARY_DISABLE;
	}
#endif
	pprocess->auto_summary_flag = RIP_AUTO_SUMMARY_ENABLE;
	
	pprocess->default_metric = RIP_DEF_METRIC;

	pprocess->rip_table = route_table_init();

	/*xuhaiqing 2010-12-21�Ž����·����Ŀ������ȡ��*/
	/*pprocess->max_route_num = RIP_MAX_ROUTE_NUM;*/
	pprocess->max_nh_num = RIP_MAX_NH_NUM;
	pprocess->route_num = 0;

	pprocess->connect_route_num = 0;
	pprocess->nbr_route_num = 0;
	pprocess->redis_route_num = 0;
	pprocess->holddown_route_num = 0;

	return;
}
#if 0
/*===========================================================
������:   rip_delete_table
��������: ɾ��RIP·�ɱ�
�������: vrf_id : VRF ID
�������: ��
����ֵ:   ��
��ע: Fengsb modified 2006-04-30
=============================================================*/
void rip_delete_table( uint32 vrf_id )
{
	struct rip_net_list_ *rip_net_list, *rip_net_forw;	
	struct rip_redis_list_ *rip_redis_list, *rip_redis_forw;
	uint32 device_index;
	
	/*ɾ��RIP·�ɱ��е�������Ϣ*/
	/*network*/
	rip_net_list = rip_tbl_array[vrf_id]->network_list.forw;
	while( rip_net_list != &(rip_tbl_array[vrf_id]->network_list) )
	{
		rip_net_forw = rip_net_list->forw;

		REMQUE( rip_net_list  );
		rip_mem_free( rip_net_list, RIP_NET_LIST_TYPE );
		
		rip_net_list = rip_net_forw;
	};	  
	
    /* destroy the callback functions */
    if( NULL != rip_tbl_array[vrf_id]->filter_list )
	{   /*filter*/
        route_distribute_list_destroy( rip_tbl_array[vrf_id]->filter_list );
    }	
    if( NULL != rip_tbl_array[vrf_id]->distance_list )
    {
	    route_distance_list_destroy(rip_tbl_array[vrf_id]->distance_list);
    }
    if( NULL != rip_tbl_array[vrf_id]->offset_list )
    {
	    route_offset_list_destroy(rip_tbl_array[vrf_id]->offset_list);
    }	
	/*clear_proto_independent(eigrp->redistribute_list);*/

	/*redistribute list*/
	rip_redis_list = rip_tbl_array[vrf_id]->redis_list.forw;
	while( rip_redis_list != &(rip_tbl_array[vrf_id]->redis_list) )
	{
		rip_redis_forw = rip_redis_list->forw;

		REMQUE( rip_redis_list  );
		rip_mem_free( rip_redis_list, RIP_REDIS_LIST_TYPE );
		
		rip_redis_list = rip_redis_forw;
	};

	/*��λRIP�˿�*/
	for(device_index = 0; device_index <= INTERFACE_DEVICE_MAX_NUMBER; device_index++ )
	{
		if( (NULL == rip_intf_array[device_index])
			|| (rip_intf_array[device_index]->vrf_id != vrf_id))
		{
			continue;
		}

		rip_intf_ref_num_set(device_index, RIP_INTF_REF_NUM_SUB_ALL);
		
		rip_disable_multicast_in( device_index);
	}
	
	/*table*/
	route_table_destroy( rip_tbl_array[vrf_id]->rip_table, rip_del_rn_callback );

	/*�ͷ��ڴ�*/
	rip_mem_free( rip_tbl_array[vrf_id], RIP_TBL_INFO_TYPE );
	rip_tbl_array[vrf_id] = NULL;

	return;	
}
#endif
/*===========================================================
������:   rip_clear_table
��������: ���RIP·�ɱ�
�������: ��
�������: ��
����ֵ:   ��
��ע:
=============================================================*/
void rip_clear_process_table( struct rip_process_info_ *pprocess )
{
	struct rip_redis_list_ *rip_redis_list, *rip_redis_forw;
	struct rip_process_intf_list_ *pprocess_intf, *pprocess_intf_temp;
#if 0	
	/*ɾ��RIP·�ɱ��е�������Ϣ*/
	/*network*/
	rip_net_list =pprocess->network_list.forw;
	while( rip_net_list != &(pprocess->network_list) )
	{
		rip_net_forw = rip_net_list->forw;

		REMQUE( rip_net_list  );
		rip_mem_free( rip_net_list, RIP_NET_LIST_TYPE );
		
		rip_net_list = rip_net_forw;
	};	  
#endif	
	/* destroy the callback functions */
	if( NULL !=pprocess->filter_list )
	{   /*filter*/
		route_distribute_list_destroy( pprocess->filter_list );
	}	
	
	if( NULL != pprocess->distance_list )
	{
		route_distance_list_destroy(pprocess->distance_list);
	}
	
	if( NULL != pprocess->offset_list )
	{
		route_offset_list_destroy(pprocess->offset_list);
	}	
	/*clear_proto_independent(eigrp->redistribute_list);*/

	/*������̶Զ˿ڵĹ���*/
	for(pprocess_intf = pprocess->intf_list.forw; pprocess_intf != &(pprocess->intf_list); pprocess_intf = pprocess_intf_temp)
	{
		pprocess_intf_temp = pprocess_intf->forw;

		rip_disable_multicast_in(pprocess_intf->pintf->device_index);
		rip_del_connect_route(pprocess_intf->pintf->device_index);
		pprocess_intf->pintf->process_id = 0;
		pprocess_intf->pintf->pprocess = NULL;
		BIT_RESET(pprocess_intf->pintf->state, RIP_INTF_PROCESS_ACTIVE);

		REMQUE(pprocess_intf);
		rip_mem_free(pprocess_intf ,RIP_PROCESS_INTF_LIST);
	}
	
	/*table*/
	route_table_destroy( pprocess->rip_table, rip_del_rn_callback );
	pprocess->rip_table = NULL;
	
	/*redistribute list*/
	rip_redis_list = pprocess->redis_list.forw;
	while( rip_redis_list != &(pprocess->redis_list) )
	{
		rip_redis_forw = rip_redis_list->forw;

		REMQUE( rip_redis_list  );
		rip_mem_free( rip_redis_list, RIP_REDIS_LIST_TYPE );
		
		rip_redis_list = rip_redis_forw;
	};
	return;	
}

#ifdef SWITCH
void rip_set_pkt_redirect(void)
{
	struct ni_service_sspctl_redirect_pkt_param rd;
	INTERFACE_DEVICE_GLOBAL_LINK_SERVICE_PARAM nisp;
	/*uint32 rip_mcast_addr = 0xE0000009;*/

	memset(&rd,0,sizeof(rd));

	nisp.sspctl.cmd = INTERFACE_SSPCTL_CMD_CREATE;
	nisp.sspctl.obj = INTERFACE_NETWORK_SSPCTL_REDIRECT_PKT;
	nisp.sspctl.arg = &rd;

	rd.module_type = MODULE_TYPE_RIP;
	rd.id = 0;
	rd.flags = SSPCTL_RD_FIELD_DIP;
	/*MAC_ADDR_SET_IPMULTICAST(&rip_mcast_addr, rd.dmac);*/
	rd.dip=0xE0000009ul; /* ntohl(0xE0000009ul); */
	rd.dip_mask=0xfffffffful;
#if 0
	/* module_id, id �����ֶν���������; */
	rd.module_type = MODULE_TYPE_RIP;
	rd.id = 0;

	rd.flags = SSPCTL_RD_FIELD_L4_DPORT;
	rd.protocol = IPPROTO_UDP;
	rd.d_port = RIP_PORT;
#endif
	/* VLAN�ӿڵ���·����L2ģ�� */
	interface_omnivorous_callback_global_link_service(DEVICE_TYPE_VLAN,
		INTERFACE_NETWORK_SERVICE_SSPCTL,
		&nisp);
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_reset_pkt_redirect
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.01.21 11:15:12
 *
*******************************************************************************/
void rip_reset_pkt_redirect(void)
{
	struct ni_service_sspctl_redirect_pkt_param rd;
	INTERFACE_DEVICE_GLOBAL_LINK_SERVICE_PARAM nisp;
	/*uint32 rip_mcast_addr = 0xE0000009;*/		

	memset(&rd,0,sizeof(rd));

	nisp.sspctl.cmd = INTERFACE_SSPCTL_CMD_DELETE;
	nisp.sspctl.obj = INTERFACE_NETWORK_SSPCTL_REDIRECT_PKT;
	nisp.sspctl.arg = &rd;

	rd.module_type = MODULE_TYPE_RIP;
	rd.id = 0;
	rd.flags = SSPCTL_RD_FIELD_DIP;
	/*MAC_ADDR_SET_IPMULTICAST(&rip_mcast_addr, rd.dmac);*/
	rd.dip=0xE0000009ul; /* ntohl(0xE0000009ul); */
	rd.dip_mask=0xfffffffful;
#if 0
	/* module_id, id �����ֶν���������; */
	rd.module_type = MODULE_TYPE_RIP;
	rd.id = 0;

	rd.flags = SSPCTL_RD_FIELD_L4_DPORT;
	rd.protocol = IPPROTO_UDP;
	rd.d_port = RIP_PORT;
#endif
	/* VLAN�ӿڵ���·����L2ģ�� */
	interface_omnivorous_callback_global_link_service(DEVICE_TYPE_VLAN,
		INTERFACE_NETWORK_SERVICE_SSPCTL,
		&nisp);
}
#endif

/*******************************************************************************
 *
 * FUNCTION	:	rip_socket_redirect_enable
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.01.21 11:24:59
 *
*******************************************************************************/
void rip_socket_redirect_enable(void)
{
	int ret;
	ret = rip_socket_init();
	if(ret != RIP_SUCCESS)
		syslog(RIP_LOG_CRIT, "rip_socket_init fail!\n");
	
#if defined(SWITCH)
	rip_set_pkt_redirect();
#endif
	bsp_protocol_priority_set(3, 0x208);
	return;
}

void rip_socket_redirect_disable(void )
{
	socket_unregister(rip_glb_info.socket_id);
	so_close(rip_glb_info.socket_id);	
	rip_glb_info.socket_id = -1;
#if defined(SWITCH)
	rip_reset_pkt_redirect();
#endif
	bsp_protocol_priority_unset(3, 0x208);
}

/*===========================================================
������:   RipTask
��������: RIPģ���ʼ��
�������: ��
�������: ��
����ֵ:   ��
��ע:
=============================================================*/
void RipTask( void )
{
	int ret;
#ifndef OS_VXWORKS
	unsigned long tid;
#endif

	memset(&rip_glb_info, 0, sizeof(struct rip_glb_info_));
	rip_glb_info.socket_id = -1;
	rip_glb_info.input_queue = RIP_DEF_INPUT_QUEUE;
	rip_process_list.back =rip_process_list.forw = &rip_process_list;
	
	/*ע���ڴ�ͳ��*/
#if RIP_MEM_STAT
	ret = rt_register_mem( RT_RIP, RIP_MAX_MALLOC_TYPE );
	assert( ret == 0 );
#endif

	ret = sm_create((int8 *)"S_RIP", 1, SM_GLOBAL | SM_FIFO | SM_UNBOUNDED, &rip_semaphore);
	assert(ret == 0);
	
	/*������Ϣ����ID*/
	ret = q_create((uint32) "RIPD", 0, Q_FIFO|Q_NOLIMIT|Q_GLOBAL, &(rip_glb_info.queue_id) );
	if( NOERR != ret )
	{
		return;
	}

	/*����������ID*/
	ret = mu_create("RIPM", MU_FIFO | MU_RECURSIVE | MU_GLOBAL, 0, &(rip_glb_info.sem_id) );
	assert(ret == 0);

	/*��Routingע��˿ں�·���¼� add by fangqi*/
  	rt_RegisterTask( RTPROTO_RIP, 0, rip_task_callback );

	/* end */
	
#ifdef OS_VXWORKS	
	if (sys_task_spawn("RIPT", SYS_TASK_PRI_NORMAL, T_OP_NOPREEMPT, 50*1024, RipMain, 0, 0)== (TASK_ID)SYS_ERROR) {
		perror("RIP: sys_task_spawn()\n");
	}
#else
	ret = t_create("RIPT", 128, 40960, 4096, 0, &tid);
	if (ret != NOERR) {
		perror("RIP: t_create()\n");
	}
	ret = t_start(tid, T_SUPV | T_NOPREEMPT | T_NOTSLICE, RipMain, 0);
	if (ret != NOERR) {
		perror("RIP: t_start()\n");
	}
#endif

	/*ע��RIPģ������*/
	ret = rip_register_cmd();
	if( RIP_SUCCESS != ret )
	{
		return ;
	}

	/*ע��MIB*/
	ret = rip_register_mib();
	if( RIP_SUCCESS != ret )
	{
		return ;
	}
	/* �Ƶ��ж˿ڱ����ǵ�ʱ����
	 * Commented by dangzhw in 2009.11.09 10:16:28 */
/*#if CTRL_CARD
	rip_set_pkt_redirect();
#endif
*/
    /*ע��汾��Ϣ*/
	rip_version_register();    
	return;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_process_init
 *
 * PARAMS		:	u.
 *
 * RETURN		:	
 *
 * NOTE			:	�ýӿں�����������Ϊ�˿���enableĳ�������ڵ�ʵ���������ģ�
 					���������£��˿�id�����浽user��struct_s�ṹ�У���rip����ʵ����
 					���ͻ��������u
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.06.13 15:28:23
 *
*******************************************************************************/
int rip_process_init(uint32 processid ,uint32 vrfid)
{
	/*uint32 processid = GetProcID(u);
	uint32 vrfid = GetVrfID(u);*/
	struct rip_process_list_ *pprocess_list =NULL;
	struct rip_process_info_ *pprocess =NULL ;
	int ret;

	/*���ɽ�������ڵ�*/
	pprocess_list = rip_mem_malloc(sizeof(struct rip_process_list_ ),RIP_PROCESS_LIST);
	if(!pprocess_list)
	{
		return RIP_FAIL;
	}
	memset(pprocess_list , 0 ,sizeof(struct rip_process_list_));

	/*���ɽ���ȫ�ֲ����ṹ*/
	pprocess = rip_mem_malloc(sizeof(struct rip_process_info_ ),RIP_PROCESS_INFO);
	if(!pprocess)
	{ 
		rip_mem_free(pprocess_list,RIP_PROCESS_LIST);
		return RIP_FAIL;
	}
	memset(pprocess , 0 ,sizeof(struct rip_process_info_));

	INSQUE(pprocess_list, rip_process_list.back);
	pprocess_list->rip_process_info = pprocess;

	/*�����̸���ֵ*/
	pprocess->process_id = processid;
	pprocess->vrf_id = vrfid;
	pprocess->pprocess_list = pprocess_list;
	
	pprocess->update_timeout = RIP_DEF_UPDATE_TIMEOUT;
	pprocess->expire_timeout = RIP_DEF_EXPIRE_TIMEOUT;
	pprocess->holddown_timeout = RIP_DEF_HOLDDOWN_TIMEOUT;
	pprocess->trigger_timeout = RIP_DEF_TRIGGER_TIMEOUT;
	pprocess->peer_timeout = RIP_DEF_PEER_TIMEOUT;

	pprocess->holddown_list.forw = &(pprocess->holddown_list);
	pprocess->holddown_list.back = &(pprocess->holddown_list);

	/*memset( &(pprocess->trigger_list), 0, sizeof(struct rip_trigger_list_) );*/
	/*pprocess->trigger_list.forw = &(pprocess->trigger_list);
	pprocess->trigger_list.back = &(pprocess->trigger_list);*/
	pprocess->trigger_table = route_table_init(); /*init trigger_table*/

	pprocess->neigh_list.forw = &(pprocess->neigh_list);
	pprocess->neigh_list.back = &(pprocess->neigh_list);

	pprocess->rip_redis_link = init_proto_independent( RTPROTO_RIP, processid, rip_redis_callback );

	pprocess->md5_record.forw = &(pprocess->md5_record);
	pprocess->md5_record.back = &(pprocess->md5_record);

	pprocess->intf_list.forw = &(pprocess->intf_list);
	pprocess->intf_list.back= &(pprocess->intf_list);

	pprocess->peer_list.forw = &(pprocess->peer_list);
	pprocess->peer_list.back = &(pprocess->peer_list);

	BIT_SET(pprocess->flags ,RIP_SRC_CHECK_ENABLE);
	BIT_SET(pprocess->flags ,RIP_ZERO_DOMAIN_CHECK_ENABLE);

	pprocess->rip2GlobalRouteChanges = 0;
    pprocess->rip2GlobalQueries = 0; 

	/*�������̵Ķ�ʱ�� 
	 * Commented by dangzhw in 2010.03.19 15:17:48 */
	ret = rip_process_create_timer( pprocess);
	if( RIP_SUCCESS != ret )
	{
		return RIP_FAIL;
	}

	ret = rip_start_process_timer(pprocess);
	if( RIP_SUCCESS != ret )
	{
		return RIP_FAIL;
	}

	/*����RIP·�ɱ�*/
	rip_process_create_table(pprocess);

	/* �����ɵ�һ��rip���̵�ʱ�򣬳�ʼ��socket��֪ͨ���������б����ض���
	 * Commented by dangzhw in 2010.03.11 14:31:25 */
	/* ����INSQUE���ڱ���ЧӦ����������Ϊrip_process_list.backֵû�з����仯��û�����´��ڴ��ж�ȡ��
	 ǿ��ת����volatile��ָ�룬ʹ���������¶�ȡ��
	  * Commented by dangzhw in 2010.09.06 09:07:53 */	
	/* �������溯���п����������л���������δ���ֻ�ܷ������ִ�У�
	   �������δ��ʼ����Ͼ��г�ȥ���ܵ����쳣�� ----by ygc 2012.10.31 */
	if(((struct rip_process_list_ * volatile)rip_process_list.back == pprocess_list) && ((struct rip_process_list_ * volatile)rip_process_list.forw == pprocess_list))
		rip_socket_redirect_enable();

	return RIP_SUCCESS;	
}

/*******************************************************************************
 * FUNCTION NAME: rip_filter_callback
 *
 * INPUTS	:	1. protocol -- Routing protocol flags defined by command module
 *          :  	2. process  -- Process number
 *       	:	3. if_eterid -- externed id of interface
 *
 * RETURN	:	none
 *
 * NOTE		:	Callback functions used when filter configuration changed
 *
 * AUTHOR	:	Fengsb
 *
 * DATE		:	2006.05.08
 * filter�����仯�Ժ�����·�ɱ�ȫ�����¸��£���Ҫ������in����out���Ա���ѧϰ����
 * ·�����½��й���
 ******************************************************************************/
void  rip_filter_callback(uint64 protocol,uint32 processid,unsigned long long if_eterid, uint32 vrf_id)
{      
	int ret;
	struct route_node *rip_route_node;
	struct rip_route_ *rip_route, *rip_route_forw, *rip_route_head;
	BOOL filter_chg;
	uint32 count, num;    
	struct rip_process_info_ *pprocess;
    DEVICE_ETERNAL_ID device_eternal_id = 0;
	
	if(FG_RIP != protocol )
		return;

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret)
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return ;
	}
	
	/*�����õ�filter in ������·��*/
	for( rip_route_node = route_top(pprocess->rip_table); rip_route_node != NULL; rip_route_node = route_next(rip_route_node) )
	{		
		rip_route_head = rip_route_node->info;
		if( NULL == rip_route_head )
		{
			continue;
		}
		filter_chg = FALSE;
        num = rip_route_head->equi_route_num;
		for( count = 1, rip_route = rip_route_head; count <= num; rip_route = rip_route_forw, count++ )
		{
			rip_route_forw = rip_route->forw;
			if(RIP_NBR_ROUTE != rip_route->route_type || 0 == rip_route->gw_index)
			{
				continue;
			}   
			ret = interface_omnivorous_callback_makeeternal(rip_route->gw_index, &device_eternal_id);
			if(INTERFACE_GLOBAL_SUCCESS != ret || device_eternal_id != if_eterid)
			{
				continue;
			}
			ret = route_distribute_list_apply_in( pprocess->filter_list, if_eterid, \
			                rip_route->route_node->p.u.prefix4.s_addr, \
			                prefix_to_mask(rip_route->route_node->p.prefixlen), \
			                rip_route->gw_addr );
			if( ROUTE_FILTER_DENY == ret )
			{
			    /*��holddown��ɾ��*/
				rip_del_holddown( rip_route );

				/*�Ӷ˿�ѧϰ·����ɾ��*/
				rip_del_from_nbr_list( rip_route );
				
				/*ɾ��·��*/
				ret = rip_del_route( rip_route );
				if( RIP_SUCCESS != ret )
				{
			        	syslog(RIP_LOG_ERR, "rip_distance_callback:del route %s/%d err.\n", \
						            inet_ntoa(rip_route->route_node->p.u.prefix4), rip_route->route_node->p.prefixlen);
					return;
				}
				filter_chg = TRUE;
			}
		}
		rip_route_head = rip_route_node->info;
		if( (TRUE == filter_chg) && (rip_route_head != NULL) && (rip_route_head->equi_route_num > 0 ) )
		{
			rip_change_notify( rip_route_head );
			/*Fengsb 2006-02-13 add the following policy, when route changed ,we should trigger*/
			rip_add_trigger( rip_route_head , pprocess ,  FALSE );
		}
	}   

    /*�����õ�filter out ������·��*/
    
    return;
}

/*******************************************************************************
 * FUNCTION NAME: rip_offset_callback
 *
 * INPUTS	:	1. protocol -- Routing protocol flags defined by command module
 *          :  	2. process  -- Process number
 *       	:	3. if_eterid -- externed id of interface
 *
 * RETURN	:	none
 *
 * NOTE		:	Callback functions used when filter configuration changed
 *
 * AUTHOR	:	Fengsb
 *
 * DATE		:	2006.05.08
 ******************************************************************************/
void  rip_offset_callback(uint64 protocol,uint32 process,unsigned long long if_eterid, uint32 vrfid)
{
    /*��Ϊripû�м�¼��NBRѧϰ����·�ɵĳ�ʼmetric������offset�仯�Ժ�
    ֻ�еȵ��ھ��´�ͨ�����ʱ������ЧӦ�á�������distance����filter��������
    �����Ա������ݿ����Ӧ�á��˴������������ݽṹ���иĽ�*/
    
    return;
}

/*******************************************************************************
 * FUNCTION NAME: rip_distance_callback
 *
 * INPUTS	:	1. protocol -- Routing protocol flags defined by command module
 *          :  	2. process  -- Process number
 *
 * RETURN	:	none
 *
 * NOTE		:	Callback functions used when distance configuration changed
 *
 * AUTHOR	:	Fengsb, RIP·�ɱ���·�ɵĹ�����뷢���仯
 *
 * DATE		:	2006.04.30
 *
 ******************************************************************************/
void rip_distance_callback(uint64 protocol,uint32 processid,uint32 vrf_id)
{	
	int ret;
	uint32 count, num;
	struct route_node *rip_route_node;
	struct rip_route_ *rip_route,  *rip_route_forw, *rip_route_head;
	uint32 min_distance;
	BOOL distance_change;
	uint32 distance;
	struct rip_process_info_ *pprocess;
    
	if(FG_RIP != protocol )
		return;

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret)
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return ;
	}
	
	/*�����õ�distance������·��*/
	for( rip_route_node = route_top(pprocess->rip_table); rip_route_node != NULL; rip_route_node = route_next(rip_route_node) )
	{
		min_distance = RIP_DEF_DISTANCE;
		
		rip_route_head = rip_route_node->info;
		if( NULL == rip_route_head )
		{
			continue;
		}

		distance_change = FALSE;
		num = rip_route_head->equi_route_num;
		/*����distance*/
		for( count = 1, rip_route = rip_route_head; count <= num; rip_route = rip_route->forw, count++ )
		{
			if( RIP_NBR_ROUTE != rip_route->route_type )
			{
				continue;
			}
			
			distance = rip_set_distance( pprocess, rip_route->route_node->p.u.prefix4.s_addr, prefix_to_mask(rip_route->route_node->p.prefixlen), rip_route->gw_addr );
			if( distance != rip_route->distance )
			{
				distance_change = TRUE;
			}
			rip_route->distance = distance;

			if( min_distance > rip_route->distance )
			{
				min_distance = rip_route->distance;
			}
		}

		/*��·�ɵ�distance���д���*/
		for( count = 1, rip_route = rip_route_head; count <= num; rip_route = rip_route_forw, count++ )
		{
			rip_route_forw = rip_route->forw;

			if( RIP_NBR_ROUTE != rip_route->route_type )
			{
				continue;
			}
			
			/*distance = 255Ϊ������Դ, ��С��distanceΪ����·��*/
			if( (rip_route->distance == 255) || ( rip_route->distance != min_distance) )
			{
				/*��holddown��ɾ��*/
				rip_del_holddown( rip_route );

				/*�Ӷ˿�ѧϰ·����ɾ��*/
				rip_del_from_nbr_list( rip_route );
				
				/*ɾ��·��*/
				ret = rip_del_route( rip_route );
				if( RIP_SUCCESS != ret )
				{
					syslog(RIP_LOG_ERR, "rip_distance_callback:del route %s/%d err.\n", \
						inet_ntoa(rip_route->route_node->p.u.prefix4), rip_route->route_node->p.prefixlen);
					return;
				}
			}
		}
		rip_route_head = rip_route_node->info;
		if( (TRUE == distance_change) && (rip_route_head != NULL) && (rip_route_head->equi_route_num > 0 ) )
		{
			rip_change_notify( rip_route_head );
			/*Fengsb 2006-02-13 add the following policy, when route changed ,we should trigger*/
			rip_add_trigger( rip_route_head,pprocess , FALSE );
		}
	}
    
	return;
}

