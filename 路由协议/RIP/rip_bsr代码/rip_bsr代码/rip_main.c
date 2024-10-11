char rip_main_version[] = "rip_main.c last changed at 2012.08.14 17:04:10 by yuguangcheng\n";
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
 *        FILE NAME       :   rip_main.c
 *        AUTHOR          :   Fengsb
 *        DATE            :   2006.05.16 14:26:40
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#include <rip/rip_include.h>
#include <rip/rip_main.h>
#include <rip/rip_timer.h>
#include <rip/rip_packet.h>
#include <rip/rip_debug.h>
#include <rip/rip_intf.h>
#include <rip/rip_redis.h>

struct rip_intf_ *rip_intf_array[INTERFACE_DEVICE_MAX_NUMBER + 1]={0};
struct rip_glb_info_  rip_glb_info;
struct rip_process_list_  rip_process_list;

unsigned long rip_semaphore;
uint32 rip_task;

extern int bfd_max_metric;

extern int rip_peer_timeout( uint32 processid );
extern int rip_loopup_peer_version(uint32 device_index);

/*******************************************************************************
 *
 * FUNCTION	:	rip_lookup_process_byid
 *
 * PARAMS		:	
 *
 * RETURN		:	true���ҵ���Ӧ��process�ڵ㣬���ѽڵ�ŵ�pprocess �����з���
 					false��û�ж�Ӧ�Ľ��̽ڵ�		
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.03 13:50:30
 *
*******************************************************************************/
int rip_lookup_process_byprocessid( uint32 processid ,struct rip_process_info_ **pprocess)
{
	struct rip_process_list_ *pprocess_list_temp = rip_process_list.forw;

	while(pprocess_list_temp != &rip_process_list)
	{
		if((pprocess_list_temp->rip_process_info) && (pprocess_list_temp->rip_process_info->process_id == processid))
		{
			*pprocess = pprocess_list_temp->rip_process_info;
			return TRUE;
		}
		pprocess_list_temp = pprocess_list_temp->forw;
	}

	return FALSE;
}
#if 0
/*******************************************************************************
 *
 * FUNCTION	:	rip_lookup_process_byintfid
 *
 * PARAMS		:	device_index	- �˿�����ֵ
 *
 * RETURN		:	�ҵ��ͷ��ض�Ӧ��process list�ڵ㣬����Ϊnull
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.03 17:40:34
 *
*******************************************************************************/
struct rip_process_list_  *rip_lookup_process_byintfid( uint32 device_index)
{
	struct rip_process_list_ *pprocess_list_temp = rip_process_list.forw;
	uint32 processid ;

	if(!(rip_intf_array[device_index]))
		return RIP_FAIL;

	processid = rip_intf_array[device_index]->process_id;

	while(pprocess_list_temp != &rip_process_list)
	{
		if((pprocess_list_temp->rip_process_info) && (pprocess_list_temp->rip_process_info->process_id == processid))
		{
			return pprocess_list_temp;
		}
		pprocess_list_temp = pprocess_list_temp->forw;
	}

	return NULL;
}
#endif
/*******************************************************************************
 *
 * FUNCTION	:	rip_ralate_intf_to_process
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	ÿ������ά��һ�����Լ�����ʹ�ܵĶ˿�������������������в���
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.03 14:13:32
 *
*******************************************************************************/
int rip_relate_intf_to_process(uint32 device_index , struct rip_process_info_ *pprocess ,uint8 handle_type)
{
	struct rip_process_intf_list_ *pintf_list = NULL;

	if(!pprocess)
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}	
	
	switch (handle_type)
	{
		case RIP_INTF_ADD_TO_PROCESS:

			pintf_list = pprocess->intf_list.forw;
			while( pintf_list != &(pprocess->intf_list))
			{/*���ö˿��Ѿ����뵽���̵Ķ˿������У�����*/
				if(pintf_list->pintf->device_index == device_index)
					return RIP_SUCCESS;
				pintf_list =pintf_list->forw;
			}


			/* ��ripȫ���µĽӿڽṹ���������̵Ķ˿�������
			 * Commented by dangzhw in 2010.03.03 14:07:43 */
			pintf_list = rip_mem_malloc(sizeof(struct rip_process_intf_list_ ),RIP_PROCESS_INTF_LIST);
			if(!pintf_list)
			{
				return RIP_FAIL;
			}

			pintf_list->pintf = rip_intf_array[device_index];

			INSQUE(pintf_list, pprocess->intf_list.back);
			break;
		case RIP_INTF_DEL_FROM_PROCESS:
			pintf_list = pprocess->intf_list.forw;
			while( pintf_list != &(pprocess->intf_list))
			{
				if(pintf_list->pintf->device_index == device_index)
					break;
				pintf_list =pintf_list->forw;
			}

			/* û���ҵ���Ӧ�Ķ˿ڣ�����
			 * Commented by dangzhw in 2010.03.03 14:11:18 */
			if(pintf_list == &(pprocess->intf_list))
				return RIP_SUCCESS;

			REMQUE(pintf_list);
			rip_mem_free(pintf_list , RIP_PROCESS_INTF_LIST);
			break;
		default:
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			break;		
	}

	return RIP_SUCCESS;	
}

int rip_process_intf_activate(uint32 device_index , uint32 warning)
{
	int ret;

	ret = rip_enable_multicast_in( device_index );
	if( RIP_SUCCESS != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return ret;
	}

	/*�����˿�ֱ��·��*/
	ret = rip_create_connect_route( device_index , warning );
	if( RIP_SUCCESS != ret )
	{
		BIT_RESET(rip_intf_array[device_index]->state , RIP_INTF_PROCESS_ACTIVE);
		rip_disable_multicast_in( device_index);
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

		/*�˿ڷ���request����*/
	ret = rip_send_request( device_index );
	if( RIP_SUCCESS != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	return RIP_SUCCESS;
}

int rip_process_intf_deactivate(uint32 device_index)
{
	int ret ;
	
	/* �˿ڲ��ٽ���ripЭ����鲥����
	   Commented by dangzhw in 2010.03.08 16:45:19 */
	rip_disable_multicast_in( device_index);

	/*ɾ���˿�ֱ��·��*/
	ret = rip_del_connect_route(device_index );
	if( RIP_SUCCESS != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	/*ɾ���Ӷ˿�ѧϰ����·��*/
	ret = rip_clear_nbr_route_list( device_index );
	if( RIP_SUCCESS != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	rip_create_redis_connect(device_index);
	return RIP_SUCCESS;	
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_process_route_activate
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.25 16:29:21
 *
*******************************************************************************/
int rip_process_route_activate(struct rip_process_info_ *pprocess , struct rip_route_activate_msg_  *msg)
{
	int ret = 0;
	uint32 device_index ;
	struct rip_intf_ *pintf;

#if 0
	struct vrf_table_list_ *vrf_table;
#endif

	if(!pprocess)
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	switch (msg->type)
	{
		case RIP_ROUTE_INTF_PROCESS_ENABLE:
			/* �������ж˿���enable ��ʱ����Ҫ�ж��Ƿ���ֱ��·����Ҫ��ӵ����̵�database��
			 * Commented by dangzhw in 2010.03.03 16:43:48 */
			device_index = msg->u.intf.device_index;
			pintf = rip_intf_array[device_index];

			if(BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
				return RIP_SUCCESS;

			/* �˿���û��ip ��ַ���˿ڵ�vrf�ͽ��̵Ĳ�һ�¡��˿ڵ�proto����up�ģ��������ֱ��·��
			 * Commented by dangzhw in 2010.03.03 16:44:52 */
			if((!pintf) || (pintf->address == 0) || (pprocess->vrf_id != pintf->vrf_id) || (!BIT_TEST(pintf->state ,RIP_INTF_LINK_UP)))
				return RIP_FAIL;
#if 0	
			pnet_list = pprocess->network_list.forw;
			while(pnet_list != &(pprocess->network_list))
			{	
				/* �ö˿ڱ�network���ǣ����ֱ��·��
				 * Commented by dangzhw in 2010.03.03 17:02:07 */
				 if( pnet_list->mask == 0 )
					mask = rip_get_mask( pnet_list->network );
				else
					mask = pnet_list->mask;
				
				if((pnet_list->network & mask) ==(pintf->address & mask))
				{	
					/* ���Ͻ��̶˿ڼ���ı�־
					 * Commented by dangzhw in 2010.03.05 10:30:18 */
					BIT_SET(pintf->state , RIP_INTF_PROCESS_ACTIVE);
						
					ret = rip_process_intf_activate(device_index , msg->u.intf.warning);
				}

				pnet_list = pnet_list->forw;
			}
#endif
			/* ���Ͻ��̶˿ڼ���ı�־
			* Commented by dangzhw in 2010.03.05 10:30:18 */
			BIT_SET(pintf->state , RIP_INTF_PROCESS_ACTIVE);
				
			ret = rip_process_intf_activate(device_index , msg->u.intf.warning);
			break;
		case RIP_ROUTE_INTF_PROCESS_DISABLE:
			device_index = msg->u.intf.device_index;
			pintf = rip_intf_array[device_index];
			/* ����ǰ�˿ڷǽ��̼���˿ڣ�����Ҫ���ֱ��·��
			 * Commented by dangzhw in 2010.03.05 10:32:58 */
			if(BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
			{				
				BIT_RESET(pintf->state ,RIP_INTF_PROCESS_ACTIVE);

				ret = rip_process_intf_deactivate(pintf->device_index);
			}

			break;
		case RIP_ROUTE_INTF_PROTO_UP:

			/* �����̸��ǵĶ˿�Э��up��ʱ��������network���ǡ�vrfһ�µ�����������������ֱ��·��
			 * Commented by dangzhw in 2010.03.10 10:53:09 */	
			device_index = msg->u.intf.device_index;
			pintf = rip_intf_array[device_index];
			if((!pintf) ||(pintf->address == 0) || (pprocess->vrf_id != pintf->vrf_id))
				return RIP_FAIL;
#if 0
			pnet_list = pprocess->network_list.forw;
			while(pnet_list != &(pprocess->network_list))
			{

				if( pnet_list->mask == 0 )
					mask = rip_get_mask( pnet_list->network );
				else
					mask = pnet_list->mask;
				
				if((pnet_list->network & mask) ==(pintf->address & mask))
				{
					BIT_SET(pintf->state , RIP_INTF_PROCESS_ACTIVE);

					rip_process_intf_activate(pintf->device_index, FALSE);
				}

				pnet_list = pnet_list->forw;
			}
#endif	
			BIT_SET(pintf->state , RIP_INTF_PROCESS_ACTIVE);

			rip_process_intf_activate(pintf->device_index, FALSE);
			break;
		case RIP_ROUTE_INTF_PROTO_DOWN:
			device_index = msg->u.intf.device_index;
			pintf = rip_intf_array[device_index];
			if(BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
			{				
				BIT_RESET(pintf->state ,RIP_INTF_PROCESS_ACTIVE);

				ret = rip_process_intf_deactivate(pintf->device_index);
			}

			break;
		case RIP_ROUTE_INTF_ADDRESS_ADD:
			device_index = msg->u.intf.device_index;
			pintf = rip_intf_array[device_index];

			if(BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
				return RIP_SUCCESS;

			/* �˿���û��ip ��ַ���˿ڵ�vrf�ͽ��̵Ĳ�һ�¡��˿ڵ�proto����up�ģ��������ֱ��·��
			 * Commented by dangzhw in 2010.03.03 16:44:52 */
			if((!pintf) || (pintf->address == 0) || (pprocess->vrf_id != pintf->vrf_id) || (!BIT_TEST(pintf->state ,RIP_INTF_LINK_UP)))
				return RIP_FAIL;

			if(pprocess->process_id != pintf->process_id)
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}

#if 0
			pnet_list = pprocess->network_list.forw;
			while(pnet_list != &(pprocess->network_list))
			{
				/* �ö˿ڱ�network���ǣ����ֱ��·��
				 * Commented by dangzhw in 2010.03.03 17:02:07 */
				 if( pnet_list->mask == 0 )
					mask = rip_get_mask( pnet_list->network );
				else
					mask = pnet_list->mask;
				
				if((pnet_list->network & mask) ==(pintf->address & mask))
				{
					/* ���Ͻ��̶˿ڼ���ı�־
					 * Commented by dangzhw in 2010.03.05 10:30:18 */
					BIT_SET(pintf->state , RIP_INTF_PROCESS_ACTIVE);

					ret = rip_process_intf_activate(device_index , msg->u.intf.warning);
				}

				pnet_list = pnet_list->forw;
			}
#endif
						/* ���Ͻ��̶˿ڼ���ı�־
			 * Commented by dangzhw in 2010.03.05 10:30:18 */
			BIT_SET(pintf->state , RIP_INTF_PROCESS_ACTIVE);

			ret = rip_process_intf_activate(device_index , msg->u.intf.warning);
			
			break;
		case RIP_ROUTE_INTF_ADDRESS_DLE:
			device_index = msg->u.intf.device_index;
			pintf = rip_intf_array[device_index];
			if(!BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
				return RIP_SUCCESS;

			if(pprocess->process_id != pintf->process_id)
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}
			BIT_RESET(pintf->state ,RIP_INTF_PROCESS_ACTIVE);

			ret = rip_process_intf_deactivate(device_index);

			break;
#if 0
		case RIP_ROUTE_NET_CMD_ADD:
			network = msg->u.network.network;
			mask = msg->u.network.mask;

			for(pintf_list = pprocess->intf_list.forw; pintf_list != &(pprocess->intf_list) ; pintf_list = pintf_list->forw)
			{
				/* һ������network��Ҫ���������̸��ǵ�ȫ���˿ڣ����˿���������ʱ�����ɶ�Ӧ��·��
				 * Commented by dangzhw in 2010.03.08 16:06:01 */
				if((!(pintf = pintf_list->pintf)) || (!BIT_TEST(pintf->state , RIP_INTF_LINK_UP)) ||(pintf->address == 0) ||(pprocess->vrf_id != pintf->vrf_id))
					continue ;

				if((pintf->address & mask) == (network & mask ))
				{
					BIT_SET(pintf->state , RIP_INTF_PROCESS_ACTIVE);

					ret = rip_process_intf_activate(pintf->device_index, TRUE);
				}				
			}
			break;
		case RIP_ROUTE_NET_CMD_DEL:
			network = msg->u.network.network;
			mask = msg->u.network.mask;

			for(pintf_list = pprocess->intf_list.forw ; pintf_list != &(pprocess->intf_list) ; pintf_list = pintf_list->forw)
			{
				if(!( pintf = pintf_list->pintf ) || !BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
					continue;

				if((pintf->address & mask) != (network & mask ))/*����network���ǣ�������һ���˿�*/
					continue;

				/* ������б��network�����ö˿��ϵ�ip��ַ��Ҳ���Ըö˿ڴ���
				 * Commented by dangzhw in 2010.07.02 15:01:31 */
				pnet_list = pprocess->network_list.forw;
				while(pnet_list != &(pprocess->network_list))
				{
					 if( pnet_list->mask == 0 )
						mask = rip_get_mask( pnet_list->network );
					else
						mask = pnet_list->mask;
					
					if((pnet_list->network & mask) ==(pintf->address & mask))
					{	
						flag_cover = 1;
						break;
					}

					pnet_list = pnet_list->forw;
				}

				if(flag_cover)
				{
					flag_cover = 0;
					continue;
				}
				
				/*������ص�����ź�����*/
				intf_net = pintf->connect_route->route_node->p.u.prefix4.s_addr;
				intf_mask =  prefix_to_mask(pintf->connect_route->route_node->p.prefixlen);
			
				BIT_RESET(pintf->state , RIP_INTF_PROCESS_ACTIVE);
				
				/* ɾ���ö˿ڶ�Ӧ��ֱ��·���Լ��ö˿�ѧϰ����·��
				 * Commented by dangzhw in 2010.03.10 14:04:41 */
				rip_process_intf_deactivate(pintf->device_index);
				
#if 0
				vrf_table = vrf_table_enter(pprocess->vrf_id,RT_RD_LOCK);
				if(vrf_table == NULL)
				{
					return RT_FAIL;
				}
#endif

				/*����redistribute connect ������*/
				rth = rt_table_locate(pprocess->vrf_id, intf_net, intf_mask);		
				if (NULL != rth &&  NULL != rth->rth_active)
				{
					 rip_create_redis_connect( rth->rth_active, pprocess );
				}
#if 0
				vrf_table_leave(vrf_table,RT_RD_LOCK);
#endif			
			}
			break;
#endif
		case RIP_ROUTE_INTF_VRF_ADD:
			device_index = msg->u.intf.device_index;
			pintf = rip_intf_array[device_index];
			if(BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
				return RIP_SUCCESS;

			if((!pintf) || (pintf->address == 0) || (pprocess->vrf_id != pintf->vrf_id) || (!BIT_TEST(pintf->state ,RIP_INTF_LINK_UP)) || (pintf->process_id != pprocess->process_id))
				return RIP_FAIL;
#if 0
			pnet_list = pprocess->network_list.forw;
			while(pnet_list != &(pprocess->network_list))
			{
				/* �ö˿ڱ�network���ǣ����ֱ��·��
				 * Commented by dangzhw in 2010.03.03 17:02:07 */
				 if( pnet_list->mask == 0 )
					mask = rip_get_mask( pnet_list->network );
				else
					mask = pnet_list->mask;
				
				if((pnet_list->network & mask) ==(pintf->address & mask))
				{
					/* ���Ͻ��̶˿ڼ���ı�־
					 * Commented by dangzhw in 2010.03.05 10:30:18 */
					BIT_SET(pintf->state , RIP_INTF_PROCESS_ACTIVE);

					ret = rip_process_intf_activate(device_index , msg->u.intf.warning);
				}

				pnet_list = pnet_list->forw;
			}
#endif
			/* ���Ͻ��̶˿ڼ���ı�־
			 * Commented by dangzhw in 2010.03.05 10:30:18 */
			BIT_SET(pintf->state , RIP_INTF_PROCESS_ACTIVE);

			ret = rip_process_intf_activate(device_index , msg->u.intf.warning);
			break;
		case RIP_ROUTE_INTF_VRF_DEL:
			device_index = msg->u.intf.device_index;
			pintf = rip_intf_array[device_index];
			/* ����ǰ�˿ڷǽ��̼���˿ڣ�����Ҫ���ֱ��·��
			 * Commented by dangzhw in 2010.03.05 10:32:58 */
			if(BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
			{				
				BIT_RESET(pintf->state ,RIP_INTF_PROCESS_ACTIVE);

				ret = rip_process_intf_deactivate(pintf->device_index);
			}

			break;
		default:
			break;
	}

	return ret;
}

int rip_auth_commit_timeout(uint32 device_index)
{
	struct rip_intf_ *rintf;
	struct rip_peer_list_ *peer,*next;
	struct rip_process_info_ *pprocess;
	int timeout,cnt=0;
	if(device_index>INTERFACE_DEVICE_MAX_NUMBER)return RIP_FAIL;
	rintf=rip_intf_array[device_index];
	if(NULL==rintf)return RIP_FAIL;
	sys_stop_timer(rintf->auth_commit_timer_id);
	pprocess=rintf->pprocess;
	if(NULL==pprocess)return RIP_FAIL;
	timeout=((time_sec-rintf->auth_commit_time)>RIP_AUTH_COMMIT_MAX_TIMEOUT);
	peer=pprocess->peer_list.forw;
	while(peer!=&(pprocess->peer_list))
	{
		next=peer->forw;
		if((peer->peer_intf==device_index)&&peer->auth_state)
		{
			if(timeout)
			{
				rip_clear_peer_route_list(device_index,peer->peer_addr);
				REMQUE(peer);
				rip_mem_free(peer,RIP_PEER_LIST_TYPE);
			}
			else 
			{
				rip_peer_request(peer,pprocess);
				cnt++;
			}
		}
		peer=next;
	}
	if(timeout)rintf->auth_commit_time=0;
	else if(cnt)sys_start_timer(rintf->auth_commit_timer_id,RIP_AUTH_COMMIT_TIMEOUT);
	return RIP_SUCCESS;
}

/*===========================================================
������:      RipMain
��������: RIPģ�鴦���ⲿ�¼��ķַ�������Ҫ�������Դ�Socket�ӿڼ�������timer��ʱ���������¼���Ϣ,
�������: ��
�������: ��
����ֵ:      ��
��ע:  
=============================================================*/
void RipMain( void )
{
	int ret;
	struct rt_msg msg;     
	
	/*��Routingע��˿ں�·���¼�*/
 	/*  modify by fang qi , in order to solve the NULL interface problem 
	rt_RegisterTask( RTPROTO_RIP, 0, rip_task_callback );
	
	*/
	/*--------------�Դ�socket���յ�����Ϣ�Ĵ���-------------------*/
	while( 1 )
	{
		/*������ʼ��*/
		memset( (void *)(&msg), 0, sizeof(struct rt_msg) );
		
		/*����Ϣ���н�����Ϣ*/
		ret = q_receive( rip_glb_info.queue_id, Q_WAIT, 0, (uint32 *)&msg);
		if( ret != NOERR )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return ;
		}
		ret = sm_p(rip_semaphore, SM_WAIT, 0);
		assert(ret == 0);

		/*���ݴ���Ϣ�����н��յ�����Ϣ��������ͬ����*/
		switch(msg.type)
		{
			case IP_SOCKET_RECV_PACKET:  // ���յ���Ϣ����
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive packet(length = %d)\n", msg.len );
					/*��socket��������*/
				rip_recv_packet( msg.len );
	
				break;
			case MSG_RIP_UPDATE_TIMEOUT: /*���ڸ��¶�ʱ����ʱ*/
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Update timer timeout(process %d)\n" ,msg.uid );
				
				/*���ڸ��¶�ʱ����ʱ*/
				rip_update_timeout(msg.uid);
				break;
			case MSG_RIP_EXPIRE_TIMEOUT:
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Expire timer timeout(process %d)\n",msg.uid );
				
				/*route-expire��ʱ����ʱ*/
				rip_expire_timeout(msg.uid);
				break;
			case MSG_RIP_HOLDDOWN_TIMEOUT:
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Holddown timer timeout(process %d)\n",msg.uid );
				
				/*route-holddown ��ʱ����ʱ*/
				rip_holddown_timeout(msg.uid);
				break;
			case MSG_RIP_TRIGGERED_TIMEOUT:
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Trigger timer timeout(process %d)\n",msg.uid );
				
				/*�������¶�ʱ����ʱ*/
				rip_trigger_timeout(msg.uid);
				break;
			/* peer��ʱ��ʱ��������peer��ʱ�������Ե�ɨ��peer����
			ɨ��ʱ���������ϴ��յ���peer��ʱ�������ڵ���peer��ʱʱ�䣬��ͬʱС��3*peer��ʱ��ʱ��ʱ��
			�������peer����һ��request(��peer��rip·��Ϊ�յ�ʱ�򣬷���ȫ��·�ɲ�ѯ���飬bdcom��˼�Ƶ��豸
			������Ӧ�����Է���һ������Ϊ0meticҲΪ0���ض�·�ɲ�ѯ--�յ�����ΪAFIҲ������뵽database�ʵ��ͬbdcom�豸��peerά��)��
			�������ϴ��յ���peer��ʱ�������ڵ���3*peer��ʱʱ�䣬��ɾ����peer;
			ͬʱȥ��del routeʱɾ��peer�Ĳ�������ɾ��rip���̵�ʱ�����ȫ��peer��
			peer��ʱ��Ĭ������Ϊ(RIP_DEF_EXPIRE_TIMEOUT +RIP_DEF_HOLDDOWN_TIMEOUT)/3 +2s,
			��ʱ��ʱ�򻹽����peer�Ƿ�network���ǻ��ǲ���neighbor���������ǽ���ɾ��(��Ȼ��peer�����ܷ���request����)��	
			ע�⣬��ϵͳһ������ʱ�����Զ�û��·����Ϣ�������ܽ��Զ˼ӵ�peer����(��ʱ���������)
			 * Commented by dangzhw in 2009.12.05 09:01:56 */
			case MSG_RIP_PEER_TIMEOUT:
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: peer timeout(process %d)\n",msg.uid );
				
				/*�������¶�ʱ����ʱ*/
				rip_peer_timeout(msg.uid);
				break;
			case MSG_RIP_INTF_KEY_START:
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: intf key timer start(device_index %d)\n",msg.uid );
				rip_neighb_timer_key_start_check(msg.uid);
				break;
			case MSG_RIP_INTF_KEY_TIMEOUT:
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: intf key timer timeout(device_index %d)\n",msg.uid );
				rip_neighb_key_life_out(msg.uid);
				break;
			case MSG_RIP_INTF_AUTH_COMMIT_TIMEOUT:
				rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: intf auth failure timer timeout(device_index %d)\n",msg.uid );
				rip_auth_commit_timeout(msg.uid);
				break;
			default:
				break;
		}
		ret = sm_v(rip_semaphore);
		assert(ret == 0);
		tm_wkafter(0);
	};

	return ;
}

/*===========================================================
������:      rip_get_mask
��������: ���������ַ��ȡ��������
�������: network: �����
�������: ��
����ֵ: ����ɹ�, ���ظ�������Ż�ȡ��������
                  ����ʧ��, ����0
��ע:  
=============================================================*/
uint32 rip_get_mask( uint32 network_num )
{
	if( network_num == 0 )
	{
		return 0x00000000;
	}
	else if( (network_num & htonl(0xff000000)) == network_num )
	{
		return htonl(0xff000000);
	}
	else if( ((network_num & htonl(0xffff0000)) == network_num ) && ((network_num & htonl(0xff000000)) != network_num) )
	{
		return htonl(0xffff0000);
	}
	else if( ((network_num & htonl(0xffffff00)) == network_num ) && ((network_num & htonl(0xffff0000)) != network_num) )
	{
		return htonl(0xffffff00);
	}
	else if( ((network_num & htonl(0xffffffff)) == network_num ) && ((network_num & htonl(0xffffff00)) != network_num) )
	{
		return 0xffffffff;
	}
	
	return 0;
}

/*===========================================================
������:      rip_get_natural_mask
��������: ��ȡ����Ȼ�����ʾ����������
�������: network: �����
�������: ��
����ֵ: ����ɹ�, ���ظ�������Ż�ȡ������Ȼ����
                  ����ʧ��,����0
��ע:
=============================================================*/
uint32 rip_get_natural_mask( uint32 network )
{
	uint8 *ip_address_identifier;

	ip_address_identifier = (uint8 *)&network;

	if( (*ip_address_identifier > 0) && (*ip_address_identifier <= 127) )
	{
		return prefix_to_mask(8);
	}
	else if( (*ip_address_identifier > 127) && (*ip_address_identifier <= 191) )
	{
		return prefix_to_mask(16);
	}
	else if( (*ip_address_identifier > 191) && (*ip_address_identifier <= 223) )
	{
		return prefix_to_mask(24);
	}

	return 0;
}

/*===========================================================
������:      rip_get_natural_network
��������: ��ȡIP��ַ����Ȼ�����ʾ�������
�������: network: �����
�������: ��
����ֵ: ����ɹ�, ���ػ�ȡ������Ȼ�����
                  ����ʧ��, ����0
��ע:
=============================================================*/
uint32 rip_get_natural_network( uint32 network )
{
	uint8 *ip_address_identifier;

	ip_address_identifier = (uint8 *)&network;

	if( (*ip_address_identifier > 0) && (*ip_address_identifier <= 127) )
	{
		return network & prefix_to_mask(8);
	}
	else if( (*ip_address_identifier > 127) && (*ip_address_identifier <= 191) )
	{
		return network & prefix_to_mask(16);
	}
	else if( (*ip_address_identifier > 191) && (*ip_address_identifier <= 223) )
	{
		return network & prefix_to_mask(24);
	}

	return 0;
}

/*===========================================================
������:      rip_set_distance
��������:    ��ȡ·�ɵĹ������
�������:    vrf_id : ��Ӧ·�ɱ�ID
			 network: ·�ɵ�ַǰ׺
             mask : ·�ɵ�ַ����
             gw_addr :·�����ص�ַ
�������: ��
����ֵ:      ���ػ�ȡ����·�ɵĹ������
��ע:  
=============================================================*/
uint32 rip_set_distance( struct rip_process_info_ *pprocess, uint32 network, uint32 mask, uint32 gw_addr )
{   
	struct route_distance_list *rip_dis_lst = NULL;
	uint32 distance;
	
	rip_dis_lst = pprocess->distance_list;
	if(rip_dis_lst == NULL)
	{
		distance = RIP_DEF_DISTANCE;       
	}
	else
	{
		/* apply: interface distance metric */
		distance = route_distance_apply( rip_dis_lst, network, mask, gw_addr);
		if (0 == distance)
			distance = RIP_DEF_DISTANCE;
	}
   	return distance;
}

/*===========================================================
������:      rip_set_send_version
��������: ���ö˿ڷ��͵ı��ĵİ汾��
�������: device_index: ���ͱ��ĵ��߼��˿ں�
�������: ��
����ֵ:      �˿ڷ��ͱ��ĵİ汾
��ע:  
=============================================================*/
int rip_set_send_version( uint32 device_index )
{
	struct rip_process_info_ *pprocess;
	uint8 peer_version = 0;

	if(!(pprocess = rip_intf_array[device_index]->pprocess))
	{
		syslog(RIP_LOG_ERR, "RIP:%s %d error\n", __FILE__, __LINE__);
		return RIP_FAIL;
	}
	
	/*�˿ڰ汾����*/
	switch( rip_intf_array[device_index]->send_version )
	{
		case RIP_SEND_VERSION_DEF:
			if( RIP_VERSION_1 == pprocess->version )
			{
				return RIP_SEND_VERSION_1;      /*���Ͱ汾1*/
			}
			else if( RIP_VERSION_2 == pprocess->version )
			{
				return RIP_SEND_VERSION_2_MULTICAST;/*���Ͱ汾2,Ŀ�ĵ�ַ224.0.0.9*/
			}
			/* Ϊ����ϱ�����ͨ���ԣ���ǿrip������Ӧ���ܣ���ȱʡversion�������(ȫ��version�Ͷ˿�version��ȱʡ)��
			��Ŀǰ�ö˿ںͶԶ���һ��һ������ʱ�����նԶ˵İ汾�ŷ��ͣ�
			���ö˿ڶԶ��Ƕ���ӿ�(һ�Զ࣬��ͨ��hub)���������ӿ����еİ汾��һ�£�
			�������ǹ�ͬ��verision��
			�������ӿڵ�version��һ�£���ϵͳĬ�ϵ�version����(Ŀǰ��version 1)
			 * Commented by dangzhw in 2009.10.12 09:57:15 */
	 		else if(RIP_VERSION_DEF == pprocess->version)
			{
				peer_version = rip_loopup_peer_version(device_index);
				if( (peer_version == RIP_VERSION_1) || (peer_version == RIP_VERSION_2))
					return peer_version;
				else
				{
					/*return RIP_SEND_VERSION_1;*/

					/* Ϊ�˹���յ�rip���ĺ��verison1�µ�·����Ϣ�Ϸ��Ժ�ʱ�ļ��,
					��Ĭ�ϰ汾������Ϊversion2��multicast��ʽ
					 * Commented by dangzhw in 2010.02.20 14:25:50 */
					return RIP_SEND_VERSION_2_MULTICAST;
				}
			}
			break;
		case RIP_SEND_VERSION_1:
			return RIP_SEND_VERSION_1;           /*���Ͱ汾1*/
			break;
		case RIP_SEND_VERSION_2_BROADCAST:
			return RIP_SEND_VERSION_2_BROADCAST; /*�㲥���Ͱ汾2*/
			break;
		case RIP_SEND_VERSION_2_MULTICAST:
			return RIP_SEND_VERSION_2_MULTICAST; /*���Ͱ汾2,Ŀ�ĵ�ַ224.0.0.9*/
			break;
		default:
			break;
	}

	return RIP_SEND_VERSION_2_MULTICAST;
}

/*===========================================================
������:      rip_get_recv_version
��������: ��ȡ�˿ڽ��յı��ĵİ汾��
�������: device_index: ���ձ��ĵ��߼��˿ں�
�������: ��
����ֵ:      �˿ڿ��Խ��ձ��ĵİ汾
��ע:  
=============================================================*/
int rip_get_recv_version( uint32 device_index )
{
	struct rip_process_info_ *pprocess = rip_intf_array[device_index]->pprocess;
	
	switch( rip_intf_array[device_index]->recv_version )
	{
		case RIP_RECV_VERSION_DEF:  // �˿����õ�RIP version����ΪĬ�ϣ����ѯȫ��RIP��Ϣ
			if( RIP_VERSION_DEF == pprocess->version )
			{
				return RIP_RECV_VERSION_1_2;/*���հ汾1 ��2*/
			}
			else if( RIP_VERSION_1 == pprocess->version )
			{
				return RIP_RECV_VERSION_1;/*���հ汾1 */
			}
			else if( RIP_VERSION_2 == pprocess->version )
			{
				return RIP_RECV_VERSION_2;/*���հ汾2*/
			}
			break;
		case RIP_RECV_VERSION_1:
			return RIP_RECV_VERSION_1;/*���հ汾1 */
			break;
		case RIP_RECV_VERSION_2:
			return RIP_RECV_VERSION_2;/*���հ汾2*/
			break;
		case RIP_RECV_VERSION_1_2:
			return RIP_RECV_VERSION_1_2;/*���հ汾1 ��2*/
			break;
		default:
			break;
			
	}

	return RIP_RECV_VERSION_DEF;
}

/*===========================================================
������:      rip_enable_split
��������: �ж϶˿�ˮƽ�ָ���Ƿ�����
�������: device_index: �߼��˿ں�
�������: ��
����ֵ:      �˿�ˮƽ�ָ�ܼ���,����TRUE
                       �˿�ˮƽ�ָ�ܽ�ֹ,����FALSE
��ע:  
=============================================================*/
/*
BOOL rip_enable_split( uint32 device_index )
{
	switch( rip_intf_array[device_index]->split_flag )
	{
		case RIP_SPLIT_HORIZON_DEF:
			if( INTERFACE_DEVICE_FLAGS_NET_POINTTOMULTI == rip_intf_array[device_index]->encap_type )
			{
		
				return FALSE;
			}
			else
			{
	
				return TRUE;
			}
			break;
		case RIP_SIMPLE_SPLIT_HORIZON_ENABLE:
		case RIP_POISONED_SPLIT_HORIZON_ENABLE:
			return TRUE;
			break;
		case RIP_SIMPLE_SPLIT_HORIZON_DISABLE:
		case RIP_POISONED_SPLIT_HORIZON_DISABLE:
			return FALSE;
			break;
		default:
			break;
	}

	return TRUE;
}
*/
/*===========================================================
������:      rip_enable_simple_split
��������: �ж϶˿ڼ�ˮƽ�ָ���Ƿ�����
�������: device_index: �߼��˿ں�
�������: ��
����ֵ:      �˿ڼ�ˮƽ�ָ�ܼ���,����TRUE
                       �˿ڼ�ˮƽ�ָ�ܽ�ֹ,����FALSE
��ע:  
=============================================================*/
/*
BOOL rip_enable_simple_split( uint32 device_index )
{
	switch( rip_intf_array[device_index]->split_flag )
	{
		case RIP_SPLIT_HORIZON_DEF:
			if( INTERFACE_DEVICE_FLAGS_NET_POINTTOMULTI == rip_intf_array[device_index]->encap_type )
			{
			
				return FALSE;
			}
			else
			{
		
				return TRUE;
			}
			break;
		case RIP_SIMPLE_SPLIT_HORIZON_ENABLE:
			return TRUE;
			break;
		default:
			break;
	}

	return FALSE;
}
*/
/*===========================================================
������:      rip_enable_poisoned_split
��������: �ж϶˿ڶ���ˮƽ�ָ���Ƿ�����
�������: device_index: �߼��˿ں�
�������: ��
����ֵ:      �˿ڶ���ˮƽ�ָ�ܼ���,����TRUE
                       �˿ڶ���ˮƽ�ָ�ܽ�ֹ,����FALSE
��ע:  
=============================================================*/
/*
BOOL rip_enable_poisoned_split( uint32 device_index )
{
	if( RIP_POISONED_SPLIT_HORIZON_ENABLE == rip_intf_array[device_index]->split_flag )
	{
		return TRUE;
	}

	return FALSE;
}
*/

/*===========================================================
������:      rip_enable_auto_summary
��������: �ж��Զ����ܹ����Ƿ�����
�������: rip_tbl_info: RIP·�ɱ�
�������: ��
����ֵ:      �Զ����ܹ�������,����TRUE
          �Զ����ܹ��ܽ�ֹ,����FALSE
��ע:  
=============================================================*/
BOOL rip_enable_auto_summary( struct rip_process_info_ *pprocess,uint32 device_index )
{
	int sendversion;
	
	sendversion = rip_set_send_version(device_index);
	if(/*RIP_VERSION_DEF == sendversion || */RIP_VERSION_1 == sendversion)
	{
		return TRUE;
	}
	else
	{
		if(RIP_AUTO_SUMMARY_DISABLE == pprocess->auto_summary_flag)
			return FALSE;
		else 
			return TRUE;
	}
	/*
	switch( rip_tbl->version )
	{
		case RIP_VERSION_DEF:
		case RIP_VERSION_1:

			return TRUE;
			break;
		case RIP_VERSION_2:
			if( RIP_AUTO_SUMMARY_DISABLE == rip_tbl->auto_summary_flag )
			{

				return FALSE;
			}
			break;
		default:
			break;
	}
	return TRUE;*/
}

/*add for register bfd */
int rip_intf_bfd_register(uint32 device_index,uint32 state)
{
	int ret;
	rip_debug(RIP_DEBUG_IP_RIP_MSG, "rip_intf_bfd_register device_index=%d,state = %d\n", device_index,state);
#ifdef HAVE_BFD
	if((!device_index) || (device_index > INTERFACE_DEVICE_MAX_NUMBER))return 0;
	
	ret = bfd_rip_announce(device_index, RTPROTO_RIP, state, rip_intf_bfd_callback);
	/*bfd have all down need metric==16 when register*/
	if((ret == 1) && state)
		rip_intf_bfd_callback(device_index,0);
	else if(ret < 0)
		rip_debug(RIP_DEBUG_IP_RIP_RETURN, "rip_intf_bfd_register  fail !\n");
#else
	return 0;
#endif
}
int rip_intf_bfd_callback(uint32 device_index,uint32 up)
{
	int ret = 0;
	struct rip_intf_ *pintf;
	uint32 dest_addr;

	rip_debug(RIP_DEBUG_IP_RIP_MSG, "rip_intf_bfd_callback device_index=%d,up = %d\n", device_index,up);
	/*�˿�down*/
	if(!up){
		pintf = rip_intf_array[device_index];
		if(!pintf) return RIP_FAIL;
		/* ����ǰ�˿ڷǽ��̼���˿ڣ�����Ҫ����*/
		if(BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
		{			
		/*��ǰ���Զ˷���·��16��*/
			bfd_max_metric=1;
			/*��ȡ���ĵ�Ŀ�ĵ�ַ*/
			dest_addr = rip_get_pkt_dest_addr( pintf->device_index );
			/*����response����,ǿ��16��*/
			rip_send_response( pintf->device_index, dest_addr, htons(RIP_PORT) );
			bfd_max_metric=0;
			
			BIT_RESET(pintf->state ,RIP_INTF_PROCESS_ACTIVE);

			ret = rip_process_intf_deactivate(pintf->device_index);
		}
	}
	else{/*�˿�up*/
		pintf = rip_intf_array[device_index];

		if((!pintf) || (pintf->address == 0) || (!BIT_TEST(pintf->state ,RIP_INTF_LINK_UP)))
			return RIP_FAIL;

		if(BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
			return RIP_SUCCESS;

		/* ���Ͻ��̶˿ڼ���ı�־*/
		BIT_SET(pintf->state , RIP_INTF_PROCESS_ACTIVE);
			
		ret = rip_process_intf_activate(device_index , TRUE);
	}

}
