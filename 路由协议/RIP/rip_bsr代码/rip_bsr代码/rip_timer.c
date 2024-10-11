 char rip_timer_version[] = "rip_timer.c last changed at 2012.10.08 10:25:28 by yuguangcheng\n";
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
 *        FILE NAME       :   rip_timer.c
 *        AUTHOR          :   Fengsb
 *        DATE            :   2006.05.17 16:58:59
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#include <rip/rip_include.h>
#include <rip/rip_main.h>
#include <rip/rip_timer.h>
#include <rip/rip_packet.h>
#include <rip/rip_intf.h>
#include <rip/rip_debug.h>

extern struct rip_glb_info_  rip_glb_info;
extern struct rip_intf_ *rip_intf_array[INTERFACE_DEVICE_MAX_NUMBER + 1];
extern struct rip_tbl_info_ *rip_tbl_array[RIP_VRF_LIMIT];

/*===========================================================
������: rip_update_timeout
��������: update timer��ʱ����
�������: ��
�������: ��
����ֵ: ����ɹ�,����RIP_SUCCESS
        ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_update_timeout( uint32 processid )
{
	struct rip_neigh_list_ *rip_neigh_temp;
	uint32 dest_addr;
	uint32 ret;
	struct rip_process_info_ *pprocess;
	struct rip_process_intf_list_ *pintf_list;
	struct rip_intf_ *pintf;

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if( !ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	
	/*ֹͣtrigger timer*/
	sys_stop_timer( pprocess->trigger_timer_id );

	/*��մ��������б�
	rip_clear_trigger_list();
    */
    
    /*Fengsb shield 2005-10-3 ���������¶�ʱ��������,��ֹͣ�������¶�ʱ��
    ����ѧϰ����·�ɣ���Ϊ��ӵ���������б���ȥ����ˣ���ֹ
    �������¶�ʱ��û�����⣬���Ƕ���withdraw��·�ɣ���Ϊ�Ѿ�
    ��rip_tbl_array[vrf_id]->rip_table��ɾ�������ԣ�����ֹ�˴������£���û�а취��
    ����ͨ���ˣ���ˣ����ڱ���rip_tbl_array[vrf_id]->rip_table��û�У������������б�
    �д��ڵ�·�ɣ�����ɾ��*/
    rip_compress_trigger_list( pprocess );
#if 0
    if( pprocess->trigger_list.forw != &(pprocess->trigger_list))
	{ /*Fengsb modify 2005-10-03*/
		rip_trigger_timeout( pprocess->process_id);
	}
#endif
	if (( NULL != pprocess->trigger_table) && (NULL != pprocess->trigger_table->top ))
	{
		rip_trigger_timeout( pprocess->process_id);
	}

	/*RIP�˿ڷ���response����*/
	for( pintf_list = pprocess->intf_list.forw; pintf_list != &(pprocess->intf_list) ; pintf_list = pintf_list->forw)
	{
		if(!(pintf = pintf_list->pintf) || !BIT_TEST(pintf->state ,  RIP_INTF_PROCESS_ACTIVE))
			continue;

		/*���ھӷ��͵�������*/
		for( rip_neigh_temp = pprocess->neigh_list.forw; rip_neigh_temp != &(pprocess->neigh_list); rip_neigh_temp = rip_neigh_temp->forw )
		{
			if( (rip_neigh_temp->neigh_addr & pintf->mask)
				== (pintf->address & pintf->mask) )
			{
				rip_send_response( pintf->device_index, rip_neigh_temp->neigh_addr, htons(RIP_PORT));
			}
		}

		/*passive�˿�,�����ͱ���*/
		if( BIT_TEST(pintf->special_flag,RIP_PASSIVE_ENABLE))
		{
			continue;
		}

		/*��ȡ���ĵ�Ŀ�ĵ�ַ*/
		dest_addr = rip_get_pkt_dest_addr( pintf->device_index );
		/*����response����*/
		rip_send_response( pintf->device_index, dest_addr, htons(RIP_PORT) );
	}

	/*��������update timer*/
	ret = sys_start_timer( pprocess->update_timer_id, pprocess->update_timeout );
	if( SYS_NOERR != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	if ((pprocess->holddown_list.forw != &pprocess->holddown_list)
		&& (sys_timer_state(pprocess->holddown_timer_id) != PTIMER_RUNNING))
	{
		sys_start_timer(pprocess->holddown_timer_id, pprocess->holddown_timeout);
	}
	return RIP_SUCCESS;	
}

/*===========================================================
������: rip_expire_timeout
��������: expire timer��ʱ����
�������: ��
�������: ��
����ֵ: ����ɹ�,����RIP_SUCCESS
                 ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_expire_timeout( uint32 processid )
{
	uint32 expire_timeout, current_sec, pass_time, left_time;
	struct rip_route_ *rip_route, *rip_route_head, *rip_route_forw;
	struct route_node *rip_route_node;
	int ret;
	uint32 count, num;
	struct rip_process_info_ *pprocess;

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if( !ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	expire_timeout = pprocess->expire_timeout;
	current_sec = time_sec;
	for( rip_route_node = route_top(pprocess->rip_table); rip_route_node != NULL; rip_route_node = route_next(rip_route_node) )
	{
		rip_route_head = rip_route_node->info;
		if( NULL == rip_route_head )
		{
			continue;
		}
		num = rip_route_head->equi_route_num;
		for( count = 1, rip_route = rip_route_head; count <= num; rip_route = rip_route_forw, count++ )
		{
			rip_route_forw = rip_route->forw;
			/*��ѧϰ·�ɲ�����expire timer*/
			if( RIP_NBR_ROUTE != rip_route->route_type )
			{
				continue;
			}

			/*holddown·�ɲ�����expire timer*/
			if( rip_route->metric >= RIP_MAX_METRIC )
			{
				if(!rip_route->hold_ptr)
				{
					rip_add_holddown( rip_route );
				}
				continue;
			}

			/*expire timer��ʱ�Ĵ���*/
			pass_time = current_sec - rip_route->refresh_time;
			if( pass_time >= pprocess->expire_timeout )
			{
				/*expire timer��ʱ,����holddown״̬*/
				if( TRUE == rip_exist_equi_route(rip_route) )
				{
					/*���еȼ�·��,��·�ɱ���ɾ��*/
					
					/*��·�ɵ�gw_index��ѧϰ�б���ɾ��*/
					rip_del_from_nbr_list( rip_route );

					/*��·�ɱ���ɾ��*/
					ret = rip_del_route( rip_route );
					if( RIP_SUCCESS != ret )
					{
						rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
						return ret;
					}
				}
				else
				{	    
					/*·�ɽ���holddown״̬*/
					rip_add_holddown( rip_route );	
					
					 /*֪ͨ��·�ɱ�·��״̬�ĸı�*/
					rip_change_notify( rip_route );
				}
			}
			else
			{
				left_time = pprocess->expire_timeout - pass_time;
				if(left_time < expire_timeout)
				{
					expire_timeout = left_time;
				}
			}
		}
	}
	
	/*��������expire timer*/
	sys_start_timer( pprocess->expire_timer_id, expire_timeout? expire_timeout: 1);
	return RIP_SUCCESS;
}

/*===========================================================
������: rip_holddown_timeout
��������: holddown timer��ʱ����
�������: ��
�������: ��
����ֵ: ����ɹ�,����RIP_SUCCESS
                 ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_holddown_timeout( uint32 processid )
{
	struct rip_route_list_ *rip_holddown_temp, *rip_holddown_forw;
	uint32 holddown_timeout;
	uint32 ret, current_sec, pass_time, left_time;
	struct rip_process_info_ *pprocess;

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if( !ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	holddown_timeout = pprocess->holddown_timeout;

	/*��holddown�б�����·����Ϣ,��������*/
	if( pprocess->holddown_list.forw == &(pprocess->holddown_list) )
	{
		return RIP_SUCCESS;
	}

	current_sec = time_sec;
	rip_holddown_temp = pprocess->holddown_list.forw;
	while( rip_holddown_temp != &(pprocess->holddown_list) )
	{
		rip_holddown_forw = rip_holddown_temp->forw;
		pass_time = current_sec - rip_holddown_temp->rip_route->refresh_time;
		if( pass_time >= pprocess->holddown_timeout )
		{		
			/*holddown��ʱ,��·�ɴ�gw_index��ѧϰ�б���ɾ��*/
			rip_del_from_nbr_list( rip_holddown_temp->rip_route );
			
			/*holddown��ʱ,��·�ɱ���ɾ��*/
			ret = rip_del_route( rip_holddown_temp->rip_route );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return ret;
			}

			/*�ͷ��ڴ�*/
			REMQUE( rip_holddown_temp );
			rip_mem_free( rip_holddown_temp, RIP_ROUTE_LIST_TYPE );

			pprocess->holddown_route_num--;
		}
		else
		{
			left_time = pprocess->holddown_timeout - pass_time;
			if( left_time < holddown_timeout )
			{
				holddown_timeout = left_time;
			}
		}

		rip_holddown_temp = rip_holddown_forw;
	};
	
	if( pprocess->holddown_list.forw != &(pprocess->holddown_list) )
	{
		/*��������holddown timer*/
		sys_start_timer(pprocess->holddown_timer_id, holddown_timeout? holddown_timeout: 1);
	}

	return RIP_SUCCESS;	
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_peer_interval
 *
 * PARAMS		:	peer		- 
 *
 * RETURN		:	
 *
 * NOTE			:	��ȡpeer�ϴθ��µ����ڵ�ʱ����
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2009.12.03 14:33:00
 *
*******************************************************************************/
int rip_peer_interval(struct rip_peer_list_ *peer)
{
	int interval = 0;

	interval = time_sec - peer->rip2PeerLastUpdate;
	if(interval < 0)
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	return interval;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_peer_should_deleting
 *
 * PARAMS		:	peer		- 
 *
 * RETURN		:	1,should be deleted
 *
 * NOTE			:	�жϵ�ǰ��network��neighbor�Ƿ�ɾ��,����ɾ���򷵻�1,����0;
 					��peer������һnetwork���ǣ���������0��
 					�е�net list���������Ϊ0�����ʱ����peerԴ��ַ��network�ĵ�ַ��Ȼ����
 					һ��Ҳ������0��
 				
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2009.12.05 16:09:10
 *
*******************************************************************************/
int rip_peer_should_deleted(struct rip_peer_list_ *peer , struct rip_process_info_ *pprocess)
{
	struct rip_process_intf_list_ *pintf_list;
	struct rip_intf_ *pintf;
	uint32 natural_intf_network,natural_intf_mask;
		
	pintf=rip_intf_array[peer->peer_intf];
	if(!pintf||!BIT_TEST(pintf->state ,RIP_INTF_PROCESS_ACTIVE)||!BIT_TEST(pintf->state ,RIP_INTF_LINK_UP))return 1;
	for(pintf_list = pprocess->intf_list.forw ; pintf_list != & pprocess->intf_list ; pintf_list = pintf_list->forw)
	{
		if(!(pintf = pintf_list->pintf))
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}
		
		if(BIT_TEST(pintf->state ,RIP_INTF_PROCESS_ACTIVE)) 
		{
			if( (RIP_UNNUMBERED_ADDRESS != pintf->addr_type) &&  ((peer->peer_addr & pintf->mask ) ==  (pintf->address &  pintf->mask) )) 
				return 0;
			else 	if( pintf->encap_type == INTERFACE_DEVICE_FLAGS_NET_POINTTOPOINT )
				{
					/*�Ե�Ե�˿ڵ����⴦��*/
					natural_intf_network = rip_get_natural_network( pintf->address );
					natural_intf_mask = rip_get_natural_mask( pintf->address );
					if((peer->peer_addr& natural_intf_mask) ==  (natural_intf_network & natural_intf_mask) )
					{
						return 0;
					}
				}
		}
	}

	return 1;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_peer_timeout
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	��peer�����е����нڵ㶼����һ��,
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2009.12.03 13:28:32
 *
*******************************************************************************/
int rip_peer_timeout(uint32 processid)
{
	struct rip_peer_list_ *peer = NULL,*peer_temp = NULL;
	int interval = 0;
	int ret = 0;
	struct rip_process_info_ *pprocess;

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if( !ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	sys_stop_timer(pprocess->peer_timer_id );
	
	peer = pprocess->peer_list.forw;
	while(peer != & pprocess->peer_list)
	{
		peer_temp= peer->forw;
		
		interval= rip_peer_interval(peer);
		if(interval == RIP_FAIL)
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "rip_peer_timeout,peer %s interval  fail!\n",ip_ntoa( peer->peer_addr ));
			return RIP_FAIL;
		}

		/* ����no network,no neighbor,��Ӧ�Ľӿڱ�ɾ����״̬down��peer,Ҳ������ɾ��,��ֹ�ٷ�request
		 * Commented by dangzhw in 2009.12.05 15:47:38 */
		if(rip_peer_should_deleted(peer ,pprocess))
		{
			REMQUE(peer);
			rip_mem_free(peer, RIP_PEER_LIST_TYPE);
		}
		else if(interval >= pprocess->peer_timeout && interval < 3*pprocess->peer_timeout)
		{
			ret = rip_peer_request(peer ,pprocess);
			if(ret != RIP_SUCCESS)
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN,"rip_peer_timeout,peer %s sent request fail!\n",ip_ntoa( peer->peer_addr ));
				return RIP_FAIL;
			}
		}
		else if(interval >= 3*pprocess->peer_timeout )
		{
			REMQUE(peer);
			rip_mem_free(peer, RIP_PEER_LIST_TYPE);
		}
		
		peer = peer_temp;
	}

	if(pprocess->peer_list.forw != &pprocess->peer_list)
		sys_start_timer(pprocess->peer_timer_id , pprocess->peer_timeout);	

	return RIP_SUCCESS;
}
/*===========================================================
������: rip_trigger_timeout
��������: trigger timer��ʱ����
�������: ��
�������: ��
����ֵ: ����ɹ�,����RIP_SUCCESS
                 ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_trigger_timeout( uint32 processid )
{
	struct rip_neigh_list_ *rip_neigh_temp;
	uint32 dest_addr;
	int ret;
	struct rip_process_info_ *pprocess;
	struct rip_process_intf_list_ *pintf_list;
	struct rip_intf_ *pintf;

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if( !ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	/*ֹͣtrigger timer��ʱ��*/
	sys_stop_timer( pprocess->trigger_timer_id );

	/*�����������б�����·����Ϣ,��������*/
#if 0
	if( pprocess->trigger_list.forw == &(pprocess->trigger_list) )
	{
		return RIP_SUCCESS;
	}
#endif
	if (NULL == pprocess->trigger_table->top)
	{
		return RIP_SUCCESS;
	}

	/*RIP�˿ڷ��ʹ�������·��*/
	for( pintf_list= pprocess->intf_list.forw ; pintf_list != &(pprocess->intf_list) ; pintf_list = pintf_list->forw )
	{
		pintf = pintf_list->pintf;

		/* �˿�û�б�����������ҽ��̵���һ���˿�
		 * Commented by dangzhw in 2010.03.09 10:00:47 */
		if( !(pintf) || !BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
			continue;

		for( rip_neigh_temp = pprocess->neigh_list.forw; rip_neigh_temp != &(pprocess->neigh_list); rip_neigh_temp = rip_neigh_temp->forw )
		{
			if( (rip_neigh_temp->neigh_addr & pintf->mask)
				== (pintf->address &pintf->mask) )
			{
				rip_send_trigger_response( pintf->device_index, rip_neigh_temp->neigh_addr );
			}
		}

		/*passive�˿�,������response����*/
		if( BIT_TEST(pintf->special_flag,RIP_PASSIVE_ENABLE))
		{
			continue;
		}
		
		/*��ȡ���ĵ�Ŀ�ĵ�ַ*/
		dest_addr = rip_get_pkt_dest_addr( pintf->device_index );
		rip_send_trigger_response( pintf->device_index, dest_addr );
	}

	/*��մ��������б�*/
	rip_clear_trigger_list( pprocess);

	return RIP_SUCCESS;
}

/*===========================================================
������: rip_add_holddown
��������: ·�ɼ���holddown�����б���ʱ��û�д�rip_tbl_array[vrf_id]->rip_table
        ��ɾ��.
�������: rip_route : ������holddown��·��
�������: ��
����ֵ:   ��
��ע:
===========================================================*/
void rip_add_holddown( struct rip_route_ *rip_route )
{
	struct rip_route_list_ *rip_route_list;
	char string[100];
	struct rip_process_info_ *pprocess;

	/*��·���Ѵ���holddown״̬,���ټ���*/
	if( NULL != rip_route->hold_ptr )
	{
		return;
	}
	
	if( RIP_NBR_ROUTE != rip_route->route_type )
	{
		/*ֻ��ѧϰ����·�ɲŻ����·�ɱ�*/
		return;
	}

	if(!(pprocess = rip_route->pprocess))
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return ;
	}
	
	/*·�ɽ���holddown, metric��Ϊ16*/
	rip_route->metric = RIP_MAX_METRIC;
	rip_route->refresh_time = time_sec;

	rip_route_list = (struct rip_route_list_ *)rip_mem_malloc( sizeof(struct rip_route_list_), RIP_ROUTE_LIST_TYPE );
	if( rip_route_list == NULL )
	{
		/*�ڴ����ʧ��*/
		fprintf(stderr, "RIP: Malloc memory failed.(%s, %d)\n", __FILE__, __LINE__ );
		return;
	}
	memset( rip_route_list, 0, sizeof(struct rip_route_list_) );

	rip_route_list->rip_route = rip_route;

	if(pprocess->holddown_timer_id && (sys_timer_state(pprocess->holddown_timer_id) != PTIMER_RUNNING))
	{
		/*����holddown��ʱ��*/
		sys_start_timer( pprocess->holddown_timer_id, pprocess->holddown_timeout);
	}

	INSQUE( rip_route_list, pprocess->holddown_list.back );

	rip_route->hold_ptr = rip_route_list;

	/*�������¸�·��*/
	rip_add_trigger( rip_route, pprocess, FALSE );

	if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_DATABASE )
	{
		memset( (void *)string, 0, 100 );
		sprintf(string, "RIP-DB: %s/%d", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
		sprintf(string, "%s via %s becomes holddown", string, ip_ntoa( rip_route->gw_addr ) );
		rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%s\n", string );
	}

	pprocess->holddown_route_num++;

	return;
}

/*===========================================================
������: rip_add_trigger
��������: ·�ɼ��봥�������б�
�������: rip_route : �����봥�������б��·��
�������: ��
����ֵ:      ��
��ע:
===========================================================*/
void rip_add_trigger( struct rip_route_ *rip_route, struct rip_process_info_ *pprocess, BOOL b_DelFrmRipTbl )
{
	char string[100];
	struct prefix route_prefix;
	struct route_table  *trigger_table = pprocess->trigger_table;
	struct route_node 	*rn = NULL;
	struct rip_trigger_list_ *rip_trigger_entry = NULL;
	struct rip_trigger_list_ *rip_trigger_temp = NULL;
	struct rip_trigger_list_ * rip_trigger_head = NULL;
	uint32 trigger_timeout;
	BOOL b_trigger_start = FALSE;
	
	struct rip_route_ *rip_route_head = rip_route->route_node->info;
	if(rip_route_head && (rip_route_head != rip_route) && (rip_route_head->metric != RIP_MAX_METRIC))
	{
		if((rip_route_head->route_type==RIP_DEF_ROUTE)
			||(rip_route_head->route_type==RIP_REDIS_ROUTE)
			||(rip_route_head->route_type==RIP_SUMMARY_ROUTE))
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return;
		}
	}
	
	if (NULL == pprocess->trigger_table->top) 
		b_trigger_start = TRUE;

	memset( &route_prefix, 0, sizeof(struct prefix) );
	route_prefix.family   = AF_INET;
	route_prefix.safi     = SAFI_UNICAST;
	route_prefix.prefixlen = rip_route->route_node->p.prefixlen;
	route_prefix.u.prefix4.s_addr = rip_route->route_node->p.u.prefix4.s_addr;

	rn = route_node_get(trigger_table,&route_prefix,NULL);
	if (NULL == rn) 
	{
		return;
	}

	if (NULL != rn->info)
	{
		rip_trigger_head = (struct rip_trigger_list_ *)rn->info;
		rip_trigger_temp = rip_trigger_head; 
		
		while(NULL != rip_trigger_temp)
		{		
			if ((rip_trigger_temp->network==rip_route->route_node->p.u.prefix4.s_addr)&&(rip_trigger_temp->next_hop==rip_route->next_hop)&&(rip_trigger_temp->gw_index==rip_route->gw_index))
			{
				rip_trigger_temp->metric = rip_route->metric;
				route_unlock_node(rn);
				return;
			}
			rip_trigger_temp = rip_trigger_temp->forw;
			
			if (rip_trigger_temp == rip_trigger_head) 
				break;
			
		}
	}
	
	/*������Ҫ�����µ�rip_trigger_list_�������������һ�����������*/
	rip_trigger_entry = (struct rip_trigger_list_ *)rip_mem_malloc( sizeof(struct rip_trigger_list_), RIP_TRIGGER_LIST_TYPE );
	if( NULL == rip_trigger_entry)
	{
		/*�ڴ����ʧ��*/
		route_unlock_node(rn);
		fprintf(stderr, "RIP: Malloc memory failed.(%s, %d)\n", __FILE__, __LINE__ );
		return;
	}
	memset( rip_trigger_entry, 0, sizeof(struct rip_trigger_list_) );

	/*Fengsb 2006-02-14 add for handle trigger compress for later*/
	if(b_DelFrmRipTbl == TRUE)
	{
	    /*��ʾ��·�ɽڵ��Ѿ��ӱ���ɾ����*/
	    rip_trigger_entry->route_node = NULL;
		rip_trigger_entry->metric=RIP_MAX_METRIC;
	}
	else
	{
	    rip_trigger_entry->route_node =  rip_route->route_node;
		rip_trigger_entry->metric = ((rip_route->route_type==RIP_REDIS_ROUTE)&&(rip_route->rmap_set_metric==0)&&(rip_route->metric<RIP_MAX_METRIC))?(uint32)(pprocess->default_metric):(rip_route->metric);
	}
	rip_trigger_entry->network = rip_route->route_node->p.u.prefix4.s_addr;
	rip_trigger_entry->mask = prefix_to_mask(rip_route->route_node->p.prefixlen);
	rip_trigger_entry->next_hop = rip_route->next_hop;
		
	rip_trigger_entry->gw_index = rip_route->gw_index;
	rip_trigger_entry->gw_addr = rip_route->gw_addr;
		
	rip_trigger_entry->route_type = rip_route->route_type;
	rip_trigger_entry->route_tag = rip_route->route_tag;	

	if((pprocess->trigger_timer_id != 0) && b_trigger_start )
	{
		/*����trigger��ʱ��*/
	    if(pprocess->trigger_timeout == 0)
		{
			sys_start_timer( pprocess->trigger_timer_id, 0x1 | TIMER_RESOLUTION_S100);
	    }
		else
		{
			trigger_timeout = rand()%pprocess->trigger_timeout + 1 ;
			sys_start_timer( pprocess->trigger_timer_id, trigger_timeout | TIMER_RESOLUTION_S);
		}
	}

	// ��Ϊͷ�ڵ�
	if (NULL == rn->info) 
	{
		QUE_INIT(rip_trigger_entry);
		rn->info = (void*)rip_trigger_entry;
	}
	else if (rip_trigger_temp == rip_trigger_head)
	{
		INSQUE(rip_trigger_entry,rip_trigger_head->back);
	}

	if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_DATABASE )
	{
		memset( (void *)string, 0, 100 );
		sprintf(string, "RIP-DB: %s/%d", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
		sprintf(string, "%s via %s triggered", string, ip_ntoa( rip_route->gw_addr ) );
		rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%s\n", string );
	}
/*	route_unlock_node(rn); ÿ������һ��trigger_list ��lock++,�˴�������*/
	
}



/*===========================================================
������: rip_del_holddown
��������: ·�ɴ�holddown�����б���ɾ��
�������: rip_route : ·��
�������: ��
����ֵ:      ��
��ע:
===========================================================*/
void rip_del_holddown( struct rip_route_ *rip_route )
{
	char string[100];
	struct rip_process_info_ *pprocess;

	/*��·��û�д���holddown״̬,������*/
	if((!rip_route) || (NULL == rip_route->hold_ptr))
	{
		return;
	}

	if(RIP_NBR_ROUTE != rip_route->route_type)
	{
		/*ֻ��ѧϰ����·�ɲŻ����·�ɱ�*/
		return;
	}

	/*vrf_id = rip_route->vrf_id;*/
	if(!(pprocess = rip_route->pprocess))
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN,"RIP:%s,%d ,error pprocess %x\n",__FILE__,__LINE__,pprocess);
		return;
	}
	REMQUE( rip_route->hold_ptr );
	rip_mem_free( rip_route->hold_ptr, RIP_ROUTE_LIST_TYPE );
	rip_route->hold_ptr = NULL;

	/*debug*/
	if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_DATABASE)
	{
		sprintf(string, "RIP-DB: route %s/%d <metric %ld>", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen, rip_route->metric );
		sprintf(string, "%s via %s leave holddown state", string, ip_ntoa( rip_route->gw_addr ) );
		rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%s\n", string );
	}

	/*rip_tbl_array[vrf_id]->holddown_route_num--;*/
	pprocess->holddown_route_num--;
	
	if( (pprocess->holddown_route_num == 0) && (pprocess->holddown_timer_id != 0) )
	{
		/*ֹͣholddown��ʱ��*/
		sys_stop_timer( pprocess->holddown_timer_id );
	}

	return;
}

#if 0
/*===========================================================
������: rip_del_trigger
��������: ·�ɴӴ��������б���ɾ��
�������: rip_route : ·��
�������: ��
����ֵ:      ��
��ע:
===========================================================*/
void rip_del_trigger( struct rip_route_ *rip_route )
{
	struct rip_trigger_list_ *rip_trigger_temp;
	char string[100];

	for( rip_trigger_temp = rip_glb_info.trigger_list.forw; rip_trigger_temp != &(rip_glb_info.trigger_list); rip_trigger_temp = rip_trigger_temp->forw )
	{
		if( (rip_trigger_temp->network == rip_route->route_node->p.u.prefix4.s_addr)
			&& (mask_to_prefix(rip_trigger_temp->mask) == rip_route->route_node->p.prefixlen)
			&& (rip_trigger_temp->next_hop == rip_route->next_hop)
			&& (rip_trigger_temp->vrf_id == rip_route->vrf_id)
			&& (rip_trigger_temp->route_type == rip_route->route_type)
			&& (rip_trigger_temp->gw_index == rip_route->gw_index)
			&& (rip_trigger_temp->gw_addr == rip_route->gw_addr) )
		{
			REMQUE( rip_trigger_temp );
			rip_mem_free( rip_trigger_temp, RIP_TRIGGER_LIST_TYPE );
			
			if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_DATABASE)
			{
				sprintf(string, "RIP-DB: route %s/%d <metric %ld>", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen, rip_route->metric );
				sprintf(string, "%s via %s un-triggered", string, ip_ntoa( rip_route->gw_addr ) );
				rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%s\n", string );
			}
			
			return;
		}
	}

	return;
}
#endif
/*===========================================================
������: rip_clear_trigger_list
��������: ��մ��������б�
�������: ��
�������: ��
����ֵ:      ��
��ע:
===========================================================*/
void rip_clear_trigger_list( struct rip_process_info_ *pprocess )
{
	struct route_table  *trigger_table = pprocess->trigger_table;

	if (NULL == trigger_table)
		return;

	if ((NULL != trigger_table) && (NULL != trigger_table->top))
	{
		/* route_table_clear��ɾtrigger_table�������Բ����ٴ���� */
		route_table_clear(trigger_table,rip_del_trigger_tree_callback);
		/*route_table_destroy ��trigger_tableҲһ��ɾ���������Ҫ������һ��*/
		/*pprocess->trigger_table = route_table_init();*/
	}
}


/*===========================================================
������: rip_compress_trigger_list
��������: ��ֹ���������б�����rip_tbl_array[vrf_id]->rip_table�д��ڵ�
        route����Ӵ��������б���ɾ��
�������: ��
�������: ��
����ֵ:   ��
��ע:
===========================================================*/
void rip_compress_trigger_list( struct rip_process_info_ *pprocess )
{
	struct rip_trigger_list_ *rip_trigger_temp,*rip_trigger_forw;
	struct rip_trigger_list_ *rip_trigger_head;
	struct route_node *rn = NULL;
	/*struct route_table  *trigger_table= pprocess->trigger_table;*/
	char string[100];

	if (NULL == pprocess->trigger_table)
		return;

	for(rn = route_top(pprocess->trigger_table);rn;rn = route_next(rn))
	{
		rip_trigger_head = (struct rip_trigger_list_ *)rn->info;

		if (rip_trigger_head ==  NULL)
			continue;
		
		rip_trigger_forw = rip_trigger_head->forw;
		rip_trigger_temp = rip_trigger_forw;

		while(rip_trigger_forw != rip_trigger_head)
		{
			rip_trigger_temp = rip_trigger_forw;	
			rip_trigger_forw = rip_trigger_forw->forw;
			if (rip_trigger_temp->route_node == NULL)
				continue;

			if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_DATABASE)
			{
				sprintf(string, "RIP-DB: route %s/%d <metric %ld>", ip_ntoa(rip_trigger_temp->network), mask_to_prefix(rip_trigger_temp->mask), rip_trigger_temp->metric );
				sprintf(string, "%s via %s compress-triggered", string, ip_ntoa( rip_trigger_temp->gw_addr ) );
				rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%s\n", string );
			}
			REMQUE( rip_trigger_temp ); 
			rip_mem_free( rip_trigger_temp, RIP_TRIGGER_LIST_TYPE );
			route_unlock_node(rn);

		}

		if ((rip_trigger_forw == rip_trigger_head)&&(rip_trigger_head->route_node != NULL))
		{
			if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_DATABASE)
			{
				sprintf(string, "RIP-DB: route %s/%d <metric %ld>", ip_ntoa(rip_trigger_head->network), mask_to_prefix(rip_trigger_head->mask), rip_trigger_head->metric );
				sprintf(string, "%s via %s compress-triggered", string, ip_ntoa( rip_trigger_head->gw_addr ) );
				rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%s\n", string );
			}
			
			if (rip_trigger_head->forw == rip_trigger_head)
			{
				rip_mem_free( rip_trigger_head, RIP_TRIGGER_LIST_TYPE );
				rn->info = NULL;
			}
			else
			{
				rn->info = rip_trigger_head->forw;
				REMQUE( rip_trigger_head );
				rip_mem_free( rip_trigger_head, RIP_TRIGGER_LIST_TYPE );
			}
			route_unlock_node(rn);
		}
	}
}



