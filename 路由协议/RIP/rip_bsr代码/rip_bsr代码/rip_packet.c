char rip_packet_version[] = "rip_packet.c last changed at 2019.11.20 10:04:54 by leibang\n";
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
 *        FILE NAME       :   rip_packet.c
 *        AUTHOR          :   Fengsb
 *        DATE            :   2006.05.17 16:58:12
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#include <rip/rip_include.h>
#include <rip/rip_main.h>
#include <rip/rip_packet.h>
#include <rip/rip_debug.h>
#include <rip/rip_intf.h>
#include <rip/rip_timer.h>
#include <rip/rip_redis.h>

char rip_send_buffer[RIP_MAX_PACKET_SIZE];/*作为rip报文缓冲区*/
char rip_recv_buffer[RIP_MAX_PACKET_SIZE];/*作为rip报文缓冲区*/

int send_pkt_len;/*发送的报文长度*/
int send_item_len;/*发送的报文中除去认证信息的长度*/

int bfd_max_metric = 0;/*标记此时bfd 通知了down，需发送所有16跳*/

extern struct rip_glb_info_  rip_glb_info;
extern struct rip_intf_ *rip_intf_array[INTERFACE_DEVICE_MAX_NUMBER + 1];
void rip_sha1_cksum(void_t data, size_t datalen, size_t totallen, void_t digest, uint32 * init);
extern BOOL sys_timer_run(ULONG timer_id );

/*===========================================================
函数名: rip_recv_packet
函数功能: RIP接收到报文的处理函数
输入参数: pkt_len: 接收到的报文的长度
输出参数: 无
返回值: 处理成功,返回RIP_SUCCESS
                 处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_recv_packet( uint32 pkt_len )
{
	struct soaddr_in src_socket;
	struct soaddr_intf *src_socket_intf;
	uint32 device_index;
	int recv_pkt_len;
	int temp_len;
	struct rip_pkt_head_ *rip_pkt_head;
	struct rip_peer_list_  *peer;
	int ret;
	struct rip_process_info_ *pprocess;
	struct rip_intf_ *pintf;

	/*输入报文合法性检测*/
	if( pkt_len > RIP_MAX_PACKET_SIZE )
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: Ignored RIP packet(Illegal packet size).\n" );
		return RIP_FAIL;
	}
	
	/*清空接收缓冲区*/
	memset( rip_recv_buffer, 0, RIP_MAX_PACKET_SIZE );
	temp_len = sizeof(struct soaddr);

	/*从socket中接收数据*/
	recv_pkt_len = so_recvfrom( rip_glb_info.socket_id, rip_recv_buffer, pkt_len, 0, (struct soaddr *)(&src_socket), &temp_len );
	if( recv_pkt_len <= 0 )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d len %d.\n",__FILE__,__LINE__, recv_pkt_len);
		return RIP_FAIL;
	}

	src_socket_intf = (struct soaddr_intf *)(&src_socket);
	device_index = (uint32)(src_socket_intf->sin_ifno); 

    /*检测device_index有效性    add by fangqi 2006-10-10*/
    if( device_index > INTERFACE_DEVICE_MAX_NUMBER )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d ( %d).\n",__FILE__,__LINE__, device_index);
		return RIP_FAIL;
	}

	/* 若没有进程在端口上使能直接返回
	 * Commented by dangzhw in 2010.03.11 14:59:48 */
	 // 根据接口id获取其端口的结构体信息
	if(!(pintf = rip_intf_array[device_index]) || (pintf->process_id == 0) )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	// 检查其是否是不接受rip报文的接口
	if(BIT_TEST( rip_intf_array[device_index]->special_flag,RIP_NOTRECV_ENABLE))
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: Ignored RIP packet from %s(deaf interface).\n", ip_ntoa(src_socket_intf->sin_addr.s_addr));
		return RIP_FAIL;		
	}

	
	// 从端口配置中获取对应的RIP进程的信息
	if(!(pprocess = pintf->pprocess))
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: %s %d.\n",__FILE__,__LINE__ );
		return RIP_FAIL;
	}

	/*lookup peer list*/
	peer = pprocess->peer_list.forw;
	while(peer != &(pprocess->peer_list))
	{
		if(peer->peer_addr == src_socket_intf->sin_addr.s_addr)
			break;
		peer = peer->forw;
	}

	// 获取RIP报文的头部
	rip_pkt_head = (struct rip_pkt_head_ *)rip_recv_buffer;
	if(peer != &(pprocess->peer_list))peer->rip2PeerRcvPackets++;/*add 2019.8.30 peer收到的数据包数目*/
	pintf->rip2IfStatRecvUpdates++;/*add 2019.8.30intf收到的数据包数目*/

	/*接收报文端口链路状态必须是UP,否则不处理*/
	if(!BIT_TEST( pintf->state ,RIP_INTF_PROCESS_ACTIVE))
	{
		pintf->rip2IfStatRcvBadPackets++;
		if(peer != &(pprocess->peer_list))peer->rip2PeerRcvBadPackets++;
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	/*对接收到的报文的合法性进行检查*/
	ret = rip_packet_check( device_index, &src_socket );
	if( RIP_SUCCESS != ret )
	{
		pintf->rip2IfStatRcvBadPackets++;
		if(peer != &(pprocess->peer_list))peer->rip2PeerRcvBadPackets++;
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return ret ;
	}

	/*if not find the peer, then create a new one*/
	if(peer == &(pprocess->peer_list))
	{
		peer = rip_mem_malloc(sizeof(struct rip_peer_list_), RIP_PEER_LIST_TYPE);
		if(peer==NULL)return RIP_FAIL;
		memset((void *)peer, 0, sizeof(struct rip_peer_list_));
		peer->peer_addr = src_socket_intf->sin_addr.s_addr;
		peer->peer_port = src_socket_intf->sin_port;
		peer->peer_intf = device_index;
		INSQUE(peer, pprocess->peer_list.back);

		if(pprocess->peer_timer_id && (sys_timer_state(pprocess->peer_timer_id) != PTIMER_RUNNING))
		{
			sys_start_timer(pprocess->peer_timer_id, pprocess->peer_timeout);
		}
	}

	/*记录端口的peer信息*/
	peer->rip2PeerVersion = rip_pkt_head->version;
	peer->peer_domain = rip_pkt_head->zero;
	peer->rip2PeerLastUpdate = time_sec;

	switch( rip_pkt_head->cmd )
	{
		case RIP_REQUEST:
			ret = rip_recv_request( device_index, pkt_len, peer );
			if( RIP_SUCCESS != ret )
			{
				return ret;
			}
			break;
		case RIP_RESPONSE:
			ret = rip_recv_response( device_index , pkt_len, peer );
			if( RIP_SUCCESS != ret )
			{
				return ret;
			}
			break;
		default:
			break;				
	}

	return RIP_SUCCESS;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_peer_request
 *
 * PARAMS		:	peer		- 
 *
 * RETURN		:	
 *
 * NOTE			:	向peer的源地址发送request
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2009.12.05 10:26:31
 *
*******************************************************************************/
int rip_peer_request(struct rip_peer_list_ *peer ,struct rip_process_info_ *pprocess)
{
	uint32 device_index = peer->peer_intf;
	struct rip_pkt_head_ *rip_pkt_head = NULL;
	struct rip_route_item_ *rip_route_item = NULL;
	int ret = 0;

	if(device_index>INTERFACE_DEVICE_MAX_NUMBER)return RIP_FAIL;
	if(rip_intf_array[device_index]==NULL)return RIP_FAIL;
	
	if(BIT_TEST(rip_intf_array[device_index]->special_flag , RIP_PASSIVE_ENABLE))
		return RIP_SUCCESS;

	memset(rip_send_buffer, 0 ,RIP_MAX_PACKET_SIZE);
	send_pkt_len = 0;
	send_item_len = 0;

	/*创建报文头部*/
	rip_create_pkt_head(device_index, RIP_REQUEST);
	
	rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;
	rip_route_item = (struct rip_route_item_ *)(rip_send_buffer +send_pkt_len);
	rip_route_item->afi = 0;

	if(rip_pkt_head->version == RIP_VERSION_1)
	{
		rip_route_item->route_tag = 0;
	}
	else if(rip_pkt_head->version == RIP_VERSION_2)
	{
		rip_route_item->route_tag = htons(RIP_INTERNAL_ROUTE);
	}

	rip_route_item->network = 0;
	rip_route_item->mask = 0;
	rip_route_item->next_hop = 0;

	/* 当发送全部路由请求的时候,若对端设备中没有路由信息,将不做回应,
	这里应发送具体路由查询
	 * Commented by dangzhw in 2009.12.05 14:21:54 */
	/*rip_route_item->metric = htonl(RIP_MAX_METRIC);*/
	rip_route_item->metric = htonl(0);

	send_pkt_len += sizeof(struct rip_route_item_);
	send_item_len += sizeof(struct rip_route_item_);

	/*创建报文尾部*/
	rip_create_pkt_tail( device_index );

	/*将request报文发送出去*/
	ret = rip_send_packet( device_index, peer->peer_addr, htons(RIP_PORT), rip_send_buffer, send_pkt_len );
	if( RIP_SUCCESS != ret )
	{
		/*复位发送报文长度*/
		send_pkt_len = 0;
		send_item_len = 0;
		return ret;
	}

	/*复位发送报文长度*/
	send_pkt_len = 0;
	send_item_len = 0;

	pprocess->rip2GlobalQueries++;
	return RIP_SUCCESS;	
}

/*===========================================================
函数名: rip_recv_request
函数功能: RIP接收到request报文的处理函数
输入参数: device_index : 接收到的报文的端口
          pkt_len      : 接收到的报文长度
          src_address  : 报文源地址
          src_port     : 报文源端口
输出参数: 无
返回值: 处理成功,返回RIP_SUCCESS
        处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_recv_request( uint32 device_index , uint32 pkt_len, struct rip_peer_list_ *peer)
{
	struct rip_pkt_head_ *rip_pkt_head;
	struct rip_route_item_ *rip_route_item;
	uint32 packet_len;
	struct rip_route_ *rip_route;
	BOOL is_special_request;
	int request_entry_num;
	char string[100];
	int ret;
	struct rip_process_info_ *pprocess;

	/*输入参数合法性检查*/
	if( device_index > INTERFACE_DEVICE_MAX_NUMBER )
	{
		return RIP_FAIL;
	}
	
	/* add by fangqi 2006-10-10*/
    if(rip_intf_array[device_index]==NULL)
	{
		return RIP_FAIL;
	}

	pprocess= rip_intf_array[device_index]->pprocess;

	/*获取报文头部*/
	rip_pkt_head = (struct rip_pkt_head_ *)rip_recv_buffer;

	/*打印debug信息*/
	rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: recv RIP from %s on %s\n", ip_ntoa(peer->peer_addr), rip_intf_array[device_index]->intf_name );
	rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tvers %d, CMD_REQUEST, length %d\n", rip_pkt_head->version, pkt_len );

	/*对request报文的处理*/
	packet_len = sizeof( struct rip_pkt_head_ );
	is_special_request = FALSE;
	request_entry_num = 0;

	// 包头长度 + 
	while( (packet_len + sizeof(struct rip_route_item_)) <= pkt_len )
	{
		rip_route_item = (struct rip_route_item_ *)(rip_recv_buffer + packet_len);

		packet_len += sizeof(struct rip_route_item_);

		if( RIP_AUTH_AFI == htons(rip_route_item->afi)  )
		{
			/*对报文中的认证信息不处理*/
			continue;
		}

		/* 判断是否是special request报文*/    
		if( (0 == request_entry_num)   // 请求特定的路由条目
			&& (0 == rip_route_item->afi)
			&& (RIP_MAX_METRIC == htonl(rip_route_item->metric)) )
		{
			is_special_request = TRUE;
		}

		request_entry_num++;

		if( TRUE == is_special_request )  
		{
			/*special request报文的处理*/
			if( request_entry_num > 1)
			{
				/*special request报文只能有一个entry*/
				rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, 
					"RIP: ignored V%d request packet from %s (illegal request)\n", rip_pkt_head->version, ip_ntoa(peer->peer_addr) );

				return RIP_SUCCESS;
			}
			else
			{
				rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\taddress_family_identifier = %d\n", htons(rip_route_item->afi) );
				rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tmetric = %d\n",htonl(rip_route_item->metric) );
			}
		}
		else/*( FALSE == is_special )*/
		{
			/*非special request报文的处理*/
			if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_PROTO_RECV )
			{
				sprintf(string, "\t%s/%d", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
				sprintf(string, "%s via %s metric %lu", string, ip_ntoa(rip_route_item->next_hop), htonl(rip_route_item->metric));

				rip_debug(RIP_DEBUG_IP_RIP_PROTO_RECV, "%s\n", string );
			}

			/*本地是否有相应的路由*/
			rip_route = rip_get_route( rip_route_item, pprocess->rip_table);
			if( NULL == rip_route )
			{
				/*本地没有改路由,entry的metric值添16*/
				rip_route_item->metric = htonl(RIP_MAX_METRIC);
			}
			else
			{
				/*本地有改路由,entry的metric值为找到的改路由的metric值*/
				rip_route_item->metric = htonl(min((rip_route->metric + 1), RIP_MAX_METRIC));
			}
		}
	};

	/*request报文中no entry,不作回应*/
	if( 0 == request_entry_num )
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, 
			"RIP: ignored V%d request packet from %s (illegal request)\n", rip_pkt_head->version, ip_ntoa(peer->peer_addr) );

		return RIP_SUCCESS;
	}

	if(BIT_TEST(rip_intf_array[device_index]->special_flag ,RIP_PASSIVE_ENABLE)
		&& !rip_is_neigh_addr(pprocess , peer->peer_addr))
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, 
			"RIP: ignored V%d request packet from %s (interface is passive)\n", rip_pkt_head->version, ip_ntoa(peer->peer_addr) );

		return RIP_SUCCESS;
	}

	if( TRUE == is_special_request )
	{
		/*接收到special request报文,发送本地所有路由*/
		rip_send_response( device_index, peer->peer_addr, peer->peer_port);		
	}
	else if(rip_intf_array[device_index]->auth_type==RIP_AUTH_NONE
			||rip_intf_array[device_index]->auth_type==RIP_AUTH_SIMPLE)
	{	
		/*接收到entry by entry request报文,回应相应的response报文*/
		rip_pkt_head->cmd = RIP_RESPONSE;
		
		ret = rip_send_packet( device_index, peer->peer_addr, peer->peer_port, rip_recv_buffer, pkt_len );
		if( RIP_SUCCESS != ret )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return ret;
		}
	}
	else
	{
		/*修改发送响应报文时使用rip_send_buffer，考虑兼容认证报文
          原代码发送请求报文的响应时，忽略了认证报文的考虑
          modify by yangli 2012-5-18*/

        /*清除报文缓冲区*/
	    memset( (void *)rip_send_buffer, 0, RIP_MAX_PACKET_SIZE );

        /*复位发送报文长度*/
    	send_pkt_len = 0;
    	send_item_len = 0;

        /*填充报文头部*/
	    rip_create_pkt_head(device_index, RIP_RESPONSE );

        packet_len = sizeof( struct rip_pkt_head_ );
    	while( (packet_len + sizeof(struct rip_route_item_)) <= pkt_len )
    	{
    		rip_route_item = (struct rip_route_item_ *)(rip_recv_buffer + packet_len);

    		packet_len += sizeof(struct rip_route_item_);

			if( RIP_AUTH_AFI == htons(rip_route_item->afi)  )
			{
				/*对报文中的认证信息不处理*/
				continue;
			}

            /*将路由条目加入报文*/
	        rip_add_item_to_pkt(device_index, peer->peer_addr, peer->peer_port, rip_route_item );
    	}

        if( send_item_len > sizeof(struct rip_pkt_head_) )
    	{
    		/*将还没有发送出去的报文发送出去*/
    		rip_create_pkt_tail(device_index);

    		ret = rip_send_packet(device_index, peer->peer_addr, peer->peer_port, rip_send_buffer, send_pkt_len );
            if( RIP_SUCCESS != ret )
    		{
                /*复位发送报文长度*/
            	send_pkt_len = 0;
            	send_item_len = 0;

    			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
    			return ret;
    		}
    	}

    	/*复位发送报文长度*/
    	send_pkt_len = 0;
    	send_item_len = 0;
	}
	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_recv_response
函数功能: RIP接收到response报文的处理函数
输入参数: device_index : 接收到的报文的端口
          pkt_len      : 接收到的报文长度
          src_addr     : 报文源地址
输出参数: 无
返回值: 处理成功,返回RIP_SUCCESS
        处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_recv_response( uint32 device_index , uint32 pkt_len, struct rip_peer_list_ *peer)
{
	struct rip_route_item_ *rip_route_item;
	uint32 packet_len;
	struct rip_pkt_head_ *rip_pkt_head;

	/*获取报文头部*/
	rip_pkt_head = (struct rip_pkt_head_ *)rip_recv_buffer;

	/*打印debug信息*/
	rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: recv RIP from %s on %s\n", ip_ntoa(peer->peer_addr), rip_intf_array[device_index]->intf_name );
	rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tvers %d, CMD_RESPONSE, length %d\n", rip_pkt_head->version, pkt_len );

	packet_len = sizeof(struct rip_pkt_head_);
	while( (packet_len + sizeof(struct rip_route_item_)) <= pkt_len )
	{
		rip_route_item = (struct rip_route_item_ *)(rip_recv_buffer + packet_len);

		packet_len += sizeof(struct rip_route_item_);

		if(htons(RIP_AUTH_AFI) == rip_route_item->afi )
		{
			continue;
		}
		
		rip_recv_item( device_index, peer, rip_route_item );
	};

	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_send_request
函数功能: 发送request报文的处理函数
输入参数: device_index : 发送报文的端口
输出参数: 无
返回值: 处理成功,返回RIP_SUCCESS
                 处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_send_request( uint32 device_index )
{
	uint32 ret;
	struct rip_pkt_head_ *rip_pkt_head;
	struct rip_route_item_ *rip_route_item;
	uint32 dest_addr;

	/*输入参数合法性检验*/
	if( device_index > INTERFACE_DEVICE_MAX_NUMBER )
	{
		return RIP_FAIL;
	}

	/*检查该端口是否能发送RIP报文*/
	if(BIT_TEST( rip_intf_array[device_index]->special_flag,RIP_PASSIVE_ENABLE ))
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		/*passive端口不发送RIP报文*/
		return RIP_SUCCESS;
	}

	/*清除报文缓冲区*/
	memset( (void *)rip_send_buffer, 0, RIP_MAX_PACKET_SIZE );

	/*复位发送报文长度*/
	send_pkt_len = 0;
	send_item_len = 0;

	/*创建报文头部*/
	rip_create_pkt_head(device_index, RIP_REQUEST );

	/*填充报文内容*/
	rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;
	
	rip_route_item = (struct rip_route_item_ *)(rip_send_buffer + send_pkt_len );
	rip_route_item->afi = 0;
	if( RIP_VERSION_1 == rip_pkt_head->version )
	{
		rip_route_item->route_tag = 0;
	}
	else if( RIP_VERSION_2 == rip_pkt_head->version )
	{
		rip_route_item->route_tag = htons(RIP_INTERNAL_ROUTE);
	}

	rip_route_item->network = 0;
	rip_route_item->mask = 0;
	rip_route_item->next_hop = 0;
	
	rip_route_item->metric = htonl(RIP_MAX_METRIC);

	send_pkt_len += sizeof(struct rip_route_item_);
	send_item_len += sizeof(struct rip_route_item_);

	/*创建报文尾部*/
	rip_create_pkt_tail( device_index );

	/*获取报文的目的地址*/
	dest_addr = rip_get_pkt_dest_addr( device_index );

	/*将request报文发送出去*/
	ret = rip_send_packet( device_index, dest_addr, htons(RIP_PORT), rip_send_buffer, send_pkt_len );
	if( RIP_SUCCESS != ret )
	{
		/*复位发送报文长度*/
		send_pkt_len = 0;
		send_item_len = 0;
		
		return ret;
	}

	/*复位发送报文长度*/
	send_pkt_len = 0;
	send_item_len = 0;

	rip_intf_array[device_index]->pprocess->rip2GlobalQueries++;

	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_send_response
函数功能: 发送response报文的处理函数
输入参数: device_index : 发送报文的端口
          dest_addr : 发送报文的目的地址
          dest_port : 发送报文的目的端口
输出参数: 无
返回值:      无
备注:Fengsb 2006-02-17 
对于从某个端口学习到了路由，同时又转发了一条相同的路由进入
rip数据库，并且，端口上又配置了中毒逆转的处理，没有做很好的考虑
可能会存在问题。
=============================================================*/
void rip_send_response( uint32 device_index, uint32 dest_addr, uint16  dest_port)
{
	struct route_node *rip_route_node, *head_node;
	struct rip_route_ *rip_rthead;
	/*BOOL b_def_rt = FALSE;*/ /*the default route (not provide by rip) is exist int vrf_id table or not*/
	struct rip_process_info_ *pprocess;

	/*add by fang qi  on 2007-03-02*/
	u_char prefixlen;                         /*掩码长度*/
	int send_version;
	struct rip_route_item_ rip_route_item;
	/**************************************************/
	
	/*清除报文缓冲区*/
	memset( (void *)rip_send_buffer, 0, RIP_MAX_PACKET_SIZE );

	/*复位发送报文长度*/
	send_pkt_len = 0;
	send_item_len = 0;

      
	/*填充报文头部*/
	rip_create_pkt_head( device_index, RIP_RESPONSE );

	if(!(pprocess = rip_intf_array[device_index]->pprocess))
	{
		syslog(RIP_LOG_ERR, "RIP:%s %d error \n", __FILE__,__LINE__);
		return ;
	}

	/*获取是否发默认路由信息 add by fangqi*/
	/*if (pprocess->default_route_flag == RIP_DEF_ROUTE_ENABLE)
		b_def_rt=TRUE;*/

	/*获取sendversion add by fang qi */
	send_version=rip_set_send_version( device_index );

	// 若开启了自动汇总，在构建要发送的RIP数据包时，将合并特定网络的路由信息
	if(TRUE == rip_enable_auto_summary(pprocess,device_index))
	{		
		head_node = route_top( pprocess->rip_table);

		/*   fangqi modify this on 2007-03-02 */
		rip_route_node = head_node;
		label:while(NULL != rip_route_node)
		{
			rip_rthead = rip_route_node->info;

			if(NULL != rip_rthead )
			{
				prefixlen = rip_route_node->p.prefixlen;

				if (prefixlen == 0)  /*default route node */
				{
					rip_route_item.mask =0;
					goto ADDPACKET;
				}
				else
				{
					if (rip_get_natural_network (rip_route_node->p.u.prefix4.s_addr)
						== rip_get_natural_network (rip_intf_array[device_index]->address) ) /*主网相同*/
					{
						if( rip_rthead->route_type !=RIP_SUMMARY_ROUTE )
						{
							if(send_version == RIP_SEND_VERSION_2_BROADCAST || send_version == RIP_SEND_VERSION_2_MULTICAST)
							{
								/****版本2  下,只要自然网络相同均发送*/
								rip_route_item.mask = prefix_to_mask (rip_rthead->route_node->p.prefixlen);
								goto ADDPACKET;
							}
							else
							{
								if (prefixlen ==32 ||(prefix_to_mask(prefixlen)==(rip_intf_array[device_index]->mask)) )
								{
									rip_route_item.mask =0;
									goto ADDPACKET;
								}
								else
								{
									rip_route_node = route_next (rip_route_node);
									goto label;
								}							
							}
						}
						else   /*主网相同的summary 路由不发送*/
						{
							rip_route_node =route_next(rip_route_node);
							goto label;
							
						}
					}
					else
					{   	/*主网不相同*/

						goto ADDSUMPACKET;
					
					}
				}
			}
			else
			{
				/*当前结点为空*/
				rip_route_node = route_next (rip_route_node);
				goto label;
			}
		ADDPACKET:      /*添加路由信息*/
			rip_route_item.network = rip_rthead->route_node->p.u.prefix4.s_addr;
			rip_add_route_to_pkt(device_index, dest_addr, dest_port, rip_rthead, &rip_route_item);
			rip_route_node=route_next(rip_route_node);
			
			goto label;
		ADDSUMPACKET: /*添加汇聚路由信息*/
			rip_route_item.network=rip_get_natural_network(rip_rthead->route_node->p.u.prefix4.s_addr);
			if(send_version==RIP_SEND_VERSION_2_BROADCAST|| send_version==RIP_SEND_VERSION_2_MULTICAST)
			{
				rip_route_item.mask = rip_get_natural_mask(rip_route_item.network);
			}
			else rip_route_item.mask =0;
			rip_add_route_to_pkt(device_index,dest_addr, dest_port, rip_rthead, &rip_route_item);
			rip_route_node=route_next_skip_child(rip_route_node);
		 	goto label;
		}
	}
	/***modify over */
	else
	{
		for( rip_route_node = route_top( pprocess->rip_table); rip_route_node != NULL; )
		{
			rip_rthead = rip_route_node->info;
			if( NULL != rip_rthead )
			{
				rip_no_auto_sum_updt( device_index, dest_addr, dest_port, rip_rthead );
			}
			rip_route_node = route_next(rip_route_node);
		}
	}	

	if( send_item_len > sizeof(struct rip_pkt_head_) )
	{
		/*将还没有发送出去的报文发送出去*/
		rip_create_pkt_tail( device_index );

		rip_send_packet( device_index, dest_addr, dest_port, rip_send_buffer, send_pkt_len );
	}

	send_pkt_len = 0;
	send_item_len = 0;

	return;
}

/*===========================================================
函数名: rip_send_trigger_response
函数功能: 发送触发更新报文
输入参数: device_index : 发送报文的端口
                       dest_addr : 发送报文的目的地址
输出参数: 无
返回值:      无
备注:
=============================================================*/
void rip_send_trigger_response( uint32 device_index, uint32 dest_addr )
{
	struct rip_trigger_list_ *rip_trigger_temp;
	struct rip_route_info_ rip_route_info;
	struct rip_process_info_ *pprocess;
	struct rip_trigger_list_ * rip_trigger_head;
	struct route_node * rn;

	/*清除报文缓冲区*/
	memset( (void *)rip_send_buffer, 0, RIP_MAX_PACKET_SIZE );

	/*复位发送报文长度*/
	send_pkt_len = 0;
	send_item_len = 0;

	if(!(pprocess = rip_intf_array[device_index]->pprocess))
	{
		syslog(RIP_LOG_ERR, "RIP:%s,%d error\n", __FILE__,__LINE__);
		return ;
	}
	/*填充报文头部*/
	rip_create_pkt_head( device_index, RIP_RESPONSE );

#if 0
	for( rip_trigger_temp = pprocess->trigger_list.forw; rip_trigger_temp != &(pprocess->trigger_list); rip_trigger_temp = rip_trigger_temp->forw )
	{
		/*端口只发送具有相同VRF的路由*/
	/*	if( rip_trigger_temp->vrf_id != rip_intf_array[device_index]->vrf_id )
		{
			continue;
		}
	*/	
		/*加入路由信息加入路由表*/
		memset( &rip_route_info, 0, sizeof(struct rip_route_info_) );
		rip_route_info.gw_addr = rip_trigger_temp->gw_addr;
		rip_route_info.gw_index = rip_trigger_temp->gw_index;
		rip_route_info.mask = rip_trigger_temp->mask;
		rip_route_info.network = rip_trigger_temp->network;
		rip_route_info.next_hop = rip_trigger_temp->next_hop;
		rip_route_info.metric = rip_trigger_temp->metric;
		rip_route_info.route_tag = rip_trigger_temp->route_tag;
		rip_route_info.route_type = rip_trigger_temp->route_type;

		rip_add_trigger_to_pkt( device_index, dest_addr, htons(RIP_PORT), &rip_route_info );
	}
#endif

	for (rn = route_top(pprocess->trigger_table);rn;rn = route_next(rn))
	{
		if (NULL == rn->info)
		{
			continue;
		}

		rip_trigger_head = (struct rip_trigger_list_ *)rn->info;
		rip_trigger_temp = rip_trigger_head;

		while(NULL != rip_trigger_head)
		{
			/*加入路由信息加入路由表*/
			memset( &rip_route_info, 0, sizeof(struct rip_route_info_) );
			rip_route_info.gw_addr = rip_trigger_temp->gw_addr;
			rip_route_info.gw_index = rip_trigger_temp->gw_index;
			rip_route_info.mask = rip_trigger_temp->mask;
			rip_route_info.network = rip_trigger_temp->network;
			rip_route_info.next_hop = rip_trigger_temp->next_hop;
			rip_route_info.metric = rip_trigger_temp->metric;
			rip_route_info.route_tag = rip_trigger_temp->route_tag;
			rip_route_info.route_type = rip_trigger_temp->route_type;

			rip_add_trigger_to_pkt(device_index, dest_addr, htons(RIP_PORT), &rip_route_info);

			rip_trigger_temp  = rip_trigger_temp->forw;

			if (rip_trigger_temp == rip_trigger_head)
				break;
		}
	}


	/*还有没有发送的路由,填充报文尾部,发送出去*/
	if( send_item_len > sizeof(struct rip_pkt_head_) )
	{
		/*填充报文尾部*/
		rip_create_pkt_tail( device_index );

		/*发送出去*/
		rip_send_packet( device_index, dest_addr, htons(RIP_PORT), rip_send_buffer, send_pkt_len );
	}

	send_pkt_len = 0;
	send_item_len = 0;
	return;
}

/*===========================================================
函数名: rip_add_trigger_to_pkt
函数功能: 将触发更新路由加入报文
输入参数: device_index : 发送报文的端口
		           dest_addr : 报文目的地址
		           dest_port : 报文目的端口
		           rip_route_info : 路由信息
输出参数: 无
返回值:      无
备注:
===========================================================*/
void rip_add_trigger_to_pkt( uint32 device_index, uint32 dest_addr, uint16 dest_port, struct rip_route_info_ *rip_route_info )
{
	struct rip_route_item_ rip_route_item;
	uint32 ret;
	uint32 gateway = 0;
	struct rip_pkt_head_ *rip_pkt_head;

	/*对发送出去的路由进行水平分割过滤*/
	ret = rip_split_check( device_index, rip_route_info );
	if( RIP_SUCCESS != ret )
	{
		return;
	}

	memset( &rip_route_item, 0, sizeof(struct rip_route_item_) );

	/*对发送出去的路由进行auto-summary过滤, network, mask*/
	ret = rip_auto_summary_check( device_index, rip_route_info, &rip_route_item );
	if( RIP_SUCCESS != ret )
	{
		return;
	}

	if(rip_route_info->route_type==RIP_NBR_ROUTE)
		gateway = (rip_route_info->next_hop)?(rip_route_info->next_hop):(rip_route_info->gw_addr);

	/*对发送出去的路由进行filter out过滤*/
	ret = rip_filter_out_check( device_index, gateway, &rip_route_item );
	if( RIP_SUCCESS != ret )
	{
		return;
	}

	/*address_family_identifier*/
	rip_route_item.afi = htons(AF_INET);

	/*metric*/
	if( RIP_REDIS_ROUTE == rip_route_info->route_type )
	{
		/*Fengsb 2006-02-17 shielded Yuhuiping's code 
		转发路由的metric之前就已经设置。yuhuiping的代码只考虑了添加转发路由
		的情况，删除是metric应该为16
		rip_route_item.metric = rip_tbl_array[vrf_id]->default_metric;
		*/
		rip_route_item.metric = htonl(rip_route_info->metric);
	}
	else
	{
		rip_route_item.metric =htonl( min( (rip_route_info->metric + 1), RIP_MAX_METRIC ));
	}

	/*对发送出去的路由进行offset out信息控制, metric*/
	rip_offset_out_check( device_index, &rip_route_item );

	/*对路由条目的next-hop进行检查, next-hop*/
	rip_next_hop_check( device_index, rip_route_info, &rip_route_item );

	rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;
	
	/*route-tag*/
	if( RIP_VERSION_1 == rip_pkt_head->version )
	{
		rip_route_item.route_tag = 0;
	}
	else if( RIP_VERSION_2 == rip_pkt_head->version )
	{
		rip_route_item.route_tag = htons(rip_route_info->route_tag);
	}
 
	/*对于从邻居学习到的路由，如果是未被汇总的，而本地rip
	缺省为自动汇总，会导致每条路由都汇总一次，最后会有
	多条相同的汇总路由被通告出去而可能metric不一致，导致
	路由振荡。
	本处理可以进行部分限制，只发送一条汇总后的邻居路由*/
	ret = rip_nbrrt_check_to_pkt(device_index, &rip_route_item);
	if( RIP_SUCCESS != ret )
	{
		return;
	}

	/*将路由条目加入报文*/
	rip_add_item_to_pkt( device_index, dest_addr, dest_port, &rip_route_item );	

	return;
}

/*===========================================================
函数名: rip_auto_summary_check
函数功能: 对发送出去路由路由进行auto-summary过滤
输入参数: device_index : 发送报文端口
                       rip_route_info : 待发送出去的路由信息
输出参数: rip_route_item : 待发送出去的 路由条目
返回值:      路由通过auto-summary过滤,返回RIP_SUCCESS
                       路由不能通过auto-summary过滤,返回RIP_FAIL
备注:
===========================================================*/
int rip_auto_summary_check( uint32 device_index, struct rip_route_info_ *rip_route_info, struct rip_route_item_ *rip_route_item )
{
	uint32 natural_route_network;
	uint32 natural_route_mask;
	uint32 natural_intf_network;
	struct rip_pkt_head_ *rip_pkt_head;
	struct rip_process_info_ *pprocess;

	if(!(pprocess = rip_intf_array[device_index]->pprocess))
	{
		syslog(RIP_LOG_ERR, "RIP:%s,%d error\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;

	if( (0 == rip_route_info->mask) && (0 == rip_route_info->network) )
	{
		/*缺省路由*/
		rip_route_item->mask = 0;
		rip_route_item->network = 0;
		
		return RIP_SUCCESS;
	}

	/*network   mask*/
	if( TRUE == rip_enable_auto_summary(pprocess,device_index))
	{
		natural_route_network = rip_get_natural_network( rip_route_info->network );
		natural_route_mask = rip_get_natural_mask( rip_route_info->network );
		natural_intf_network = rip_get_natural_network( rip_intf_array[device_index]->address );
		
		/*汇总功能启动*/
		if( natural_route_network == natural_intf_network )
		{
			if( rip_route_info->mask == rip_intf_array[device_index]->mask )
			{
				/*汇总功能启动,与端口具有相同主网,
			    	   掩码相同的路由不被汇总发送*/
				rip_route_item->network = rip_route_info->network;
				if( RIP_VERSION_1 == rip_pkt_head->version )
				{
					rip_route_item->mask = 0;
				}
				else if( RIP_VERSION_2 == rip_pkt_head->version )
				{
					rip_route_item->mask = rip_route_info->mask;
				}
			}
			else/*( rip_route->mask != rip_intf_array[device_index]->address )*/
			{
				/*汇总功能启动,与发送端口具有相同主网,
				   掩码不相同的路由不被发送*/
				return RIP_FAIL;
			}
		}
		else
		{
			/*汇总功能启动,与发送端口具有不同主网的
			   路由被汇总发送*/
			rip_route_item->network = natural_route_network;
			if( RIP_VERSION_1 == rip_pkt_head->version )
			{
				rip_route_item->mask = 0;
			}
			else if( RIP_VERSION_2 == rip_pkt_head->version )
			{
				rip_route_item->mask = natural_route_mask;
			}
		}
	}
	else
	{
		/*汇总功能禁止*/
		rip_route_item->network = rip_route_info->network;
		if( RIP_VERSION_1 == rip_pkt_head->version )
		{
			rip_route_item->mask = 0;
		}
		else if( RIP_VERSION_2 == rip_pkt_head->version )
		{
			rip_route_item->mask = rip_route_info->mask;
		}
	}

	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_send_packet
函数功能: 发送RIP报文
输入参数: device_index : 发送报文的端口
          dest_addr : 发送报文的目的地址
          dest_port : 发送报文的目的端口
          packet :发送的RIP报文
          pkt_len : 发送报文的长度
输出参数: 无
返回值: 处理成功,返回RIP_SUCCESS
        处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_send_packet( uint32 device_index, uint32 dest_addr, uint16 dest_port, void *packet, uint32 pkt_len )
{
	uint32 value[2];
	int ret;
	struct soaddr_in socket;
	struct rip_pkt_head_ *rip_pkt_head;
	struct rip_route_item_ *rip_route_item;
	uint32 packet_len;
	char string[100];

	/*输入参数合法性检验*/
	if( ( device_index > INTERFACE_DEVICE_MAX_NUMBER )
    	|| ( 0 == dest_addr )
    	|| ( NULL == packet )
    	|| ( pkt_len > RIP_MAX_PACKET_SIZE ) )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
    
	/*定义变量初始化*/
	value[0] = dest_addr;
	value[1] = device_index;
	memset( (void *)&socket, 0, sizeof(struct soaddr_in) );

	/*IP不能向自己发送*/
	if( ( !BIT_TEST( rip_intf_array[device_index]->state ,RIP_INTF_PROCESS_ACTIVE))
		|| (dest_addr == rip_intf_array[device_index]->address))
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	if( dest_addr == htonl(RIP_MULTICAST_ADDRESS) ) 
	{
		ret = so_setsockopt( rip_glb_info.socket_id, IPPROTO_IP, IP_MULTICAST_PORT, (char *)value, sizeof( value ) );
		if( ret < 0 )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d (%d).\n",__FILE__,__LINE__,ret);
			return RIP_FAIL;
		}
	}
	else
	{
		ret = so_setsockopt( rip_glb_info.socket_id, IPPROTO_IP, IP_PORT, (char *)value, sizeof( value ) );
		if( ret <0 )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d (%d).\n",__FILE__,__LINE__,ret);
			return RIP_FAIL;
		}
	}

	/*设置目的地socket*/
	socket.sin_len    = sizeof( struct soaddr_in );
	socket.sin_family = AF_INET;
	socket.sin_addr.s_addr  = dest_addr;
	socket.sin_port  = dest_port;
	
	ret = so_sendto ( rip_glb_info.socket_id, packet, pkt_len, MSG_DONTROUTE, (struct soaddr *)&socket, sizeof(struct sockaddr_in) );
	if( ret != pkt_len )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d (%d).\n",__FILE__,__LINE__,ret);
		return RIP_FAIL;
	}

	rip_intf_array[device_index]->rip2IfStatSentUpdates++;

 	if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_PROTO_SEND )
 	{
		rip_pkt_head = (struct rip_pkt_head_ *)packet;
		
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_SEND, "RIP: send to %s via %s\n", ip_ntoa(dest_addr), rip_intf_array[device_index]->intf_name );

		if( RIP_RESPONSE == rip_pkt_head->cmd )
		{
			rip_debug( RIP_DEBUG_IP_RIP_PROTO_SEND, "\tvers %d, CMD_RESPONSE, length %d\n", rip_pkt_head->version, pkt_len );
		}
		else if( RIP_REQUEST == rip_pkt_head->cmd )
		{
			rip_debug( RIP_DEBUG_IP_RIP_PROTO_SEND, "\tvers %d, CMD_REQUEST, length %d\n", rip_pkt_head->version, pkt_len );
		}

		packet_len = sizeof(struct rip_pkt_head_);
		while( (packet_len + sizeof(struct rip_route_item_)) <= pkt_len )
		{
			rip_route_item = (struct rip_route_item_ *)((char *)packet + packet_len);
			
			packet_len += sizeof(struct rip_route_item_);
			
			if(rip_route_item->afi == htons(RIP_AUTH_AFI))
			{
				continue;
			}
			sprintf(string, "\t%s/%d", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
			sprintf(string, "%s via %s metric %lu", string, ip_ntoa(rip_route_item->next_hop), htonl(rip_route_item->metric) );
			rip_debug(RIP_DEBUG_IP_RIP_PROTO_SEND, "%s\n", string );				
		};
 	}

	return RIP_SUCCESS;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_loopup_peer_version
 *
 * PARAMS		:	device_index	- 
 *
 * RETURN		:	0:没有合适的version
 					非0:要填充到head中的version
 *
 * NOTE			:	当端口对应的peer为一个或者对应多个peer但多个peer的
 					的version一致，返回他们他们共同的version
 					否则不修改head中的version
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2009.10.12 10:30:11
 *
*******************************************************************************/
int rip_loopup_peer_version(uint32 device_index)
{
	uint8 version = 0;
	uint32 peer_num = 0;
	uint32 version_same =TRUE;
	struct rip_peer_list_ *peer = NULL;
	struct rip_process_info_ *pprocess;

	if(rip_intf_array[device_index] == NULL)
		return 0;

	if(!(pprocess = rip_intf_array[device_index]->pprocess))
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	peer = pprocess->peer_list.forw;

	while(peer != &pprocess->peer_list)
	{
		if(peer->peer_intf == device_index)
		{
			peer_num ++;

			if(peer_num > 1)
			{
				if(version != peer->rip2PeerVersion)
				{
					version_same = FALSE;
					break;
				}
			}
			else
				version = peer->rip2PeerVersion;
		}
		peer = peer->forw;
	}

	if(peer_num > 0 && version_same == TRUE )
		return version;
	else
		return 0;
}
/*===========================================================
函数名: rip_create_pkt_head
函数功能: 创建报文头部
输入参数: device_index : 逻辑端口号
          cmd_type: 发送报文类型
输出参数: 无
返回值:      无
备注:
===========================================================*/
void rip_create_pkt_head( uint32 device_index, uint8 cmd_type /*,uint8 request_version/*version in request packet */ )
{
	struct rip_pkt_head_ *rip_pkt_head;
	int send_version;
	struct rip_md5_head_ *rip_md5_head;
	struct rip_simple_head_ *rip_simple_head;

	uint16 keynum =0;
	uint16 keyid;
	struct rip_authen_head_ *authen_head;
	struct dynamic_key_list_ *key;

	rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;
	
	rip_pkt_head->cmd = cmd_type;

	send_version = rip_set_send_version( device_index );
	if( send_version == RIP_SEND_VERSION_1 )
	{
		/*版本1*/
		rip_pkt_head->version = RIP_VERSION_1;
	}
	else if( (send_version == RIP_SEND_VERSION_2_BROADCAST)
			|| (send_version == RIP_SEND_VERSION_2_MULTICAST))
	{
		/*版本2*/
		rip_pkt_head->version = RIP_VERSION_2;
	}
	else
	{
		/*其他情况,版本2(暂时)*/
		rip_pkt_head->version = RIP_VERSION_2;
	}	

	if(cmd_type == RIP_REQUEST)
	{
		if(BIT_TEST(rip_intf_array[device_index]->special_flag, RIP_SEND_V1DEMAND))
			rip_pkt_head->version = RIP_VERSION_1;
		else if(BIT_TEST(rip_intf_array[device_index]->special_flag, RIP_SEND_V2DEMAND))
			rip_pkt_head->version = RIP_VERSION_2;
	}
	/*零域*/
	rip_pkt_head->zero = 0;

	send_pkt_len += sizeof(struct rip_pkt_head_);
	send_item_len += sizeof(struct rip_pkt_head_);

	/*若是认证报文,需填充认证信息*/
	if( RIP_VERSION_2 == rip_pkt_head->version )
	{
		if( (RIP_AUTH_SIMPLE == rip_intf_array[device_index]->auth_type)
			&& (0 != strlen(rip_intf_array[device_index]->simple_key)) )
		{
			/*明文认证*/
			rip_simple_head = (struct rip_simple_head_ *)( rip_send_buffer + send_pkt_len );
			rip_simple_head->afi = htons(RIP_AUTH_AFI);
			rip_simple_head->authen_type = htons(RIP_AUTH_SIMPLE);
			/*strcpy(rip_simple_head->password, rip_intf_array[device_index]->simple_key);*/
			memcpy(rip_simple_head->password, rip_intf_array[device_index]->simple_key, 16);

			send_pkt_len += sizeof(struct rip_simple_head_);			
		}
		else if( (RIP_AUTH_MD5 == rip_intf_array[device_index]->auth_type)
			&& (0 != strlen(rip_intf_array[device_index]->md5_key)) &&(!rip_intf_array[device_index]->dynamicflag))
		{
			/*MD5认证*/
			rip_md5_head = (struct rip_md5_head_ *)( rip_send_buffer + send_pkt_len );
			rip_md5_head->afi = htons(RIP_AUTH_AFI);
			rip_md5_head->authen_type = htons(RIP_AUTH_MD5);
			rip_md5_head->md5_keyid = rip_intf_array[device_index]->md5_keyid;
			rip_md5_head->sequence_number = htonl(time_sec);

			send_pkt_len += sizeof(struct rip_md5_head_);
		}
		 else if(RIP_AUTH_DYNAMIC== rip_intf_array[device_index]->auth_type && rip_intf_array[device_index]->dynamicflag)
		 {
		 	key =rip_intf_array[device_index]->key_list.forw;
			// 获取邻居的key链表中随机获取一个有效key
			while(key != &rip_intf_array[device_index]->key_list)
			{
				if(key->key_state == RIP_NEIGHBOR_KEY_ACTIVE || key->key_state == RIP_NEIGHBOR_KEY_EXTENSIONUSE)
					keynum++;
				key = key->forw;
			}

			if(keynum == 0)
				return;
			else
			{
				/*选取随机的active的key*/
				srand( time(NULL));
     			keyid =rand()%keynum;
			}

			key = rip_intf_array[device_index]->key_list.forw;
			keynum =0;
			while(key != &rip_intf_array[device_index]->key_list)
			{
				if(key->key_state == RIP_NEIGHBOR_KEY_ACTIVE || key->key_state == RIP_NEIGHBOR_KEY_EXTENSIONUSE)
				{
					if(keynum == keyid)break;
					keynum++;
				}
				key = key->forw;
			}
			authen_head = (struct rip_authen_head_ *)( rip_send_buffer + send_pkt_len );
			authen_head->afi = htons(RIP_AUTH_AFI);
			authen_head->authen_type = htons(RIP_AUTH_MD5);
			authen_head->sequence_number = htonl(time_sec);
			authen_head->keyid = key->key_id;
		
			send_pkt_len += sizeof(struct rip_authen_head_);
			
		 }
		 	
	}

	return;
}

/*===========================================================
函数名: rip_create_pkt_tail
函数功能: 创建报文尾部
输入参数: device_index : 逻辑端口号
输出参数: 无
返回值:      无
备注:
===========================================================*/
void rip_create_pkt_tail( uint32 device_index )
{
	struct rip_pkt_head_ *rip_pkt_head;
	struct rip_md5_head_ *rip_md5_head;
	struct rip_md5_tail_ *rip_md5_tail;
	struct rip_authen_head_ *authen_head;
	struct rip_authen_tail_ *authen_tail;
	struct dynamic_key_list_ *key;

	rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;

	if( RIP_VERSION_2 == rip_pkt_head->version )
	{
		/*MD5报文需填充MD5信息的尾部信息*/
		if( (RIP_AUTH_MD5 == rip_intf_array[device_index]->auth_type)
			&& (0 != strlen(rip_intf_array[device_index]->md5_key)) && (!rip_intf_array[device_index]->dynamicflag))
		{
			rip_md5_head = (struct rip_md5_head_ *)( rip_send_buffer + sizeof(struct rip_pkt_head_) );
			rip_md5_head->packet_len = htons(send_pkt_len);

			rip_md5_tail = (struct rip_md5_tail_ *)( rip_send_buffer + send_pkt_len );
			rip_md5_tail->afi = htons(RIP_AUTH_AFI);
			rip_md5_tail->route_tag = htons(RIP_INTERNAL_ROUTE);
			/*strcpy( rip_md5_tail->password, rip_intf_array[device_index]->md5_key );*/
			memcpy( rip_md5_tail->password, rip_intf_array[device_index]->md5_key, 16);

			rip_md5_head->authen_data_len = sizeof(rip_md5_tail->afi) + sizeof(rip_md5_tail->route_tag) + sizeof(rip_md5_tail->password);

			send_pkt_len += rip_md5_head->authen_data_len;
			rt_md5_cksum( (byte *)rip_send_buffer, send_pkt_len, send_pkt_len, rip_md5_tail->password, (uint32 *)0 );
		}
		else if(RIP_AUTH_DYNAMIC== rip_intf_array[device_index]->auth_type && rip_intf_array[device_index]->dynamicflag)
		{
			authen_head = (struct rip_authen_head_ *)( rip_send_buffer + sizeof(struct rip_pkt_head_) );

			key = rip_intf_array[device_index]->key_list.forw;
			while(key != & rip_intf_array[device_index]->key_list)
			{
				if(((key->key_state ==RIP_NEIGHBOR_KEY_ACTIVE)
					 ||(key->key_state ==RIP_NEIGHBOR_KEY_EXTENSIONUSE))
					&&(key->key_id == authen_head->keyid))break;
				key = key->forw;
			}

			if(key == & rip_intf_array[device_index]->key_list )
				return;

			authen_head->packet_len = htons(send_pkt_len);
			authen_tail = (struct rip_authen_tail_ *)( rip_send_buffer + send_pkt_len );
			authen_tail->afi = htons(RIP_AUTH_AFI);
			authen_tail->route_tag = htons(RIP_INTERNAL_ROUTE);
			if(key->algorithms == RIP_NEIGHB_MD5)
			{
				memcpy(authen_tail->key.md5 ,key->key ,16);
				authen_head->authen_data_len = sizeof(authen_tail->afi) + sizeof(authen_tail->route_tag) + 16;
				send_pkt_len += authen_head->authen_data_len;

				rt_md5_cksum( (byte *)rip_send_buffer, send_pkt_len, send_pkt_len, authen_tail->key.md5, (uint32 *)0 );			
			}
			else if(key->algorithms == RIP_NEIGHB_SHA1)
			{
				memcpy(authen_tail->key.sha1 ,key->key ,20);
				authen_head->authen_data_len = sizeof(authen_tail->afi) + sizeof(authen_tail->route_tag) + 20;
				send_pkt_len += authen_head->authen_data_len;

				rip_sha1_cksum( (byte *)rip_send_buffer, send_pkt_len, send_pkt_len, authen_tail->key.sha1, (uint32 *)0 );			
			}
		}
	
	}

	return;
}

/*===========================================================
函数名: rip_add_route_to_pkt
函数功能: 将路由加入报文
输入参数: device_index : 发送报文的端口
		  dest_addr : 报文目的地址
		  dest_port : 报文目的端口
		  rip_route_info : 路由信息
输出参数: 无
返回值:      无
备注:
===========================================================*/
void rip_add_route_to_pkt( uint32 device_index, uint32 dest_addr, uint16 dest_port,
		struct rip_route_ * rip_route,  struct rip_route_item_ *rip_route_item)
{	
	uint32 ret, gateway = 0;
	struct rip_pkt_head_ *rip_pkt_head;
	struct rip_route_info_ rip_route_info;
	struct rip_process_info_ *pprocess;

	memset( &rip_route_info, 0, sizeof(struct rip_route_info_) );
	rip_route_info.gw_addr = rip_route->gw_addr;
	rip_route_info.gw_index = rip_route->gw_index;
	rip_route_info.mask = prefix_to_mask(rip_route->route_node->p.prefixlen);
	rip_route_info.metric = rip_route->metric;
	rip_route_info.network = rip_route->route_node->p.u.prefix4.s_addr;
	rip_route_info.next_hop = rip_route->next_hop;
	rip_route_info.route_tag = rip_route->route_tag;
	rip_route_info.route_type = rip_route->route_type;
	
	/*对发送出去的路由进行水平分割过滤*/
	ret = rip_split_check( device_index, &rip_route_info );
	if( RIP_SUCCESS != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return;
	}

	if(rip_route->route_type==RIP_NBR_ROUTE)
		gateway = (rip_route->next_hop)?(rip_route->next_hop):(rip_route->gw_addr);
	
	/*对发送出去的路由进行filter out过滤*/
	ret = rip_filter_out_check( device_index, gateway, rip_route_item );
	if( RIP_SUCCESS != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return;
	}

	/*address_family_identifier*/
	rip_route_item->afi = htons(AF_INET);

	if(!(pprocess = rip_intf_array[device_index]->pprocess))
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return ;
	}

	/*metric*/
	if( RIP_REDIS_ROUTE == rip_route_info.route_type )
	{
		/*Fengsb 2006-02-17 shielded Yuhuiping's code 
		转发路由的metric之前就已经设置。yuhuiping的代码只考虑了添加转发路由
		的情况，删除时metric应该为16
		rip_route_item.metric = rip_tbl_array[vrf_id]->default_metric;
		*/	
		/*fangqi modify this 2006-11-23  */
		/*转发路由的metric值应该为default-metric*/
        /*至于检测其metric值是否大于RIP_MAX_METRIC,则由rip_offset_out_check来完成*/
		rip_route_item->metric = htonl((rip_route->rmap_set_metric)?(rip_route_info.metric):((uint32)(pprocess->default_metric)));		
	}
	else
	{
		rip_route_item->metric = htonl(rip_route_info.metric + 1);
	}

	/*对发送出去的路由进行offset out信息控制, metric*/
	rip_offset_out_check( device_index, rip_route_item );

	/*对路由条目的next-hop进行检查, next-hop*/
	rip_next_hop_check( device_index, &rip_route_info, rip_route_item );

	rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;
	
	/*route-tag*/
	if( RIP_VERSION_1 == rip_pkt_head->version )
	{
		rip_route_item->route_tag = 0;
	}
	else if( RIP_VERSION_2 == rip_pkt_head->version )
	{
		rip_route_item->route_tag = htons(rip_route_info.route_tag);
	}
	
	/*对于从邻居学习到的路由，如果是未被汇总的，而本地rip
	缺省为自动汇总，会导致每条路由都汇总一次，最后会有
	多条相同的汇总路由被通告出去而可能metric不一致，导致
	路由振荡。
	本处理可以进行部分限制，只发送一条汇总后的邻居路由*/
	ret = rip_nbrrt_check_to_pkt(device_index, rip_route_item);
	if( RIP_SUCCESS != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return;
	}
	/*add bfd 联动rip，bfd检测端口down,rip 发送全部16跳条目*/
	if(bfd_max_metric == 1)
		rip_route_item->metric = RIP_MAX_METRIC;
	
	/*将路由条目加入报文*/
	rip_add_item_to_pkt( device_index, dest_addr, dest_port, rip_route_item );	

	return;
}

/*===========================================================
函数名: rip_add_item_to_pkt
函数功能: 将路由条目加入报文
输入参数: device_index : 发送报文端口
	      dest_addr : 发送报文目的地址
	      dest_port : 发送报文的目的端口
		  rip_route_item : 待发送的路由条目
输出参数: 无
返回值:      无
备注:
===========================================================*/
void rip_add_item_to_pkt( uint32 device_index, uint32 dest_addr, uint16 dest_port, struct rip_route_item_ *rip_route_item )
{
	uint32 ret;
	struct rip_md5_head_ *rip_md5_head;
	struct rip_md5_tail_ *rip_md5_tail;
	struct rip_pkt_head_ *rip_pkt_head;
	uint16 offset;

	struct rip_authen_head_ *authen_head;
	struct rip_authen_tail_ *authen_tail;
	struct dynamic_key_list_ *key;
	uint16 keynum =0;
	
	rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;
	authen_head = (struct rip_authen_head_ *)(rip_send_buffer + sizeof(struct rip_pkt_head_));

	key =rip_intf_array[device_index]->key_list.forw;
	while(key !=&rip_intf_array[device_index]->key_list)
	{
		if(key->key_state == RIP_NEIGHBOR_KEY_ACTIVE || key->key_state == RIP_NEIGHBOR_KEY_EXTENSIONUSE)
		{
			keynum++;
			if(key->key_id == authen_head->keyid)
				break;
		}
		key = key->forw;
	}

		
	if( (RIP_VERSION_2 == rip_pkt_head->version)
		&& (RIP_AUTH_MD5 == rip_intf_array[device_index]->auth_type)
		&& (0 != strlen(rip_intf_array[device_index]->md5_key)) &&(!rip_intf_array[device_index]->dynamicflag))
	{
		/*MD5认证报文*/
		if( (send_pkt_len + sizeof(struct rip_route_item_)) >= (RIP_MAX_PACKET_SIZE - sizeof(struct rip_md5_tail_)) )
		{
			/*填充MD5报文尾部*/
			rip_md5_head = (struct rip_md5_head_ *)(rip_send_buffer + sizeof(struct rip_pkt_head_));
			rip_md5_head->packet_len = htons(send_pkt_len);
			
			rip_md5_tail = (struct rip_md5_tail_ *)( rip_send_buffer + send_pkt_len);
			rip_md5_tail->afi = htons(RIP_AUTH_AFI);
			rip_md5_tail->route_tag = htons(RIP_MD5_TAIL_TAG);
			/*strcpy( rip_md5_tail->password, rip_intf_array[device_index]->md5_key );*/
			memcpy( rip_md5_tail->password, rip_intf_array[device_index]->md5_key, 16);

			rip_md5_head->authen_data_len = sizeof(rip_md5_tail->afi) + sizeof(rip_md5_tail->route_tag) + sizeof(rip_md5_tail->password);

			send_pkt_len += rip_md5_head->authen_data_len;

			rt_md5_cksum( (byte *)rip_send_buffer, send_pkt_len, send_pkt_len, rip_md5_tail->password, (uint32 *)0 );

			/*发送报文*/
			ret = rip_send_packet( device_index, dest_addr, dest_port, rip_send_buffer, send_pkt_len );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return;
			}

			offset = sizeof(struct rip_pkt_head_) + sizeof(struct rip_md5_head_);
			memset( (rip_send_buffer + offset), 0, (RIP_MAX_PACKET_SIZE - offset) );
			memcpy((rip_send_buffer + offset ), rip_route_item, sizeof(struct rip_route_item_) );

			/*发送报文长度复位*/
			send_pkt_len = offset + sizeof(struct rip_route_item_);
			send_item_len = sizeof(struct rip_pkt_head_) + sizeof(struct rip_route_item_);
		}
		else
		{
			/*累加发送报文长度*/
			
			memcpy((rip_send_buffer + send_pkt_len ), rip_route_item, sizeof(struct rip_route_item_) );
			send_pkt_len += sizeof(struct rip_route_item_);
			send_item_len += sizeof(struct rip_route_item_);
		}
	}
	else if( (RIP_VERSION_2 == rip_pkt_head->version)
		&& (RIP_AUTH_DYNAMIC== rip_intf_array[device_index]->auth_type) && keynum &&rip_intf_array[device_index]->dynamicflag)
	{	
		/*dynamic authentication*/
		if( (send_pkt_len + sizeof(struct rip_route_item_)) >= (RIP_MAX_PACKET_SIZE - 20))
		{
				/*填充认证报文尾部*/
			authen_head->packet_len = htons(send_pkt_len);
			
			authen_tail = (struct rip_authen_tail_ *)( rip_send_buffer + send_pkt_len);
			authen_tail->afi = htons(RIP_AUTH_AFI);
			authen_tail->route_tag = htons(RIP_AUTHEN_TAIL_TAG);
			
			if(key->algorithms == RIP_NEIGHB_MD5)
			{
				memcpy(authen_tail->key.md5 ,key->key ,16);
				authen_head->authen_data_len = sizeof(authen_tail->afi) + sizeof(authen_tail->route_tag) + 16;
				send_pkt_len += authen_head->authen_data_len;

				rt_md5_cksum( (byte *)rip_send_buffer, send_pkt_len, send_pkt_len, authen_tail->key.md5, (uint32 *)0 );			
			}
			else if(key->algorithms == RIP_NEIGHB_SHA1)
			{
				memcpy(authen_tail->key.sha1 ,key->key ,20);
				authen_head->authen_data_len = sizeof(authen_tail->afi) + sizeof(authen_tail->route_tag) + 20;
				send_pkt_len += authen_head->authen_data_len;

				rip_sha1_cksum( (byte *)rip_send_buffer, send_pkt_len, send_pkt_len, authen_tail->key.sha1, (uint32 *)0 );			
			}
			
			/*发送报文*/
			ret = rip_send_packet( device_index, dest_addr, dest_port, rip_send_buffer, send_pkt_len );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return;
			}

			offset = sizeof(struct rip_pkt_head_) + sizeof(struct rip_md5_head_);
			memset( (rip_send_buffer + offset), 0, (RIP_MAX_PACKET_SIZE - offset) );
			memcpy((rip_send_buffer + offset ), rip_route_item, sizeof(struct rip_route_item_) );

			/*发送报文长度复位*/
			send_pkt_len = offset + sizeof(struct rip_route_item_);
			send_item_len = sizeof(struct rip_pkt_head_) + sizeof(struct rip_route_item_);
		}
		else
		{
			/*累加发送报文长度*/
			
			memcpy((rip_send_buffer + send_pkt_len ), rip_route_item, sizeof(struct rip_route_item_) );
			send_pkt_len += sizeof(struct rip_route_item_);
			send_item_len += sizeof(struct rip_route_item_);
		}
	}
	else if( (RIP_VERSION_2 == rip_pkt_head->version)
		    && (RIP_AUTH_SIMPLE == rip_intf_array[device_index]->auth_type)
		    && (0 != strlen(rip_intf_array[device_index]->simple_key) ) )
	{
		if( (send_pkt_len + sizeof(struct rip_route_item_)) >  RIP_MAX_PACKET_SIZE )
		{
			/*发送报文*/
			ret = rip_send_packet( device_index, dest_addr, dest_port, rip_send_buffer, send_pkt_len );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return;
			}
			
			offset = sizeof(struct rip_pkt_head_) + sizeof(struct rip_simple_head_);
			memset( (void *)(rip_send_buffer + offset), 0, (RIP_MAX_PACKET_SIZE - offset) );
			memcpy((rip_send_buffer + offset ), rip_route_item, sizeof(struct rip_route_item_) );

			/*发送报文长度复位*/
			send_pkt_len = offset + sizeof(struct rip_route_item_);
			send_item_len = sizeof(struct rip_pkt_head_) + sizeof(struct rip_route_item_);
		}
		else
		{
			memcpy((rip_send_buffer + send_pkt_len ), rip_route_item, sizeof(struct rip_route_item_) );

			/*累加发送报文长度*/
			send_pkt_len += sizeof(struct rip_route_item_);
			send_item_len += sizeof(struct rip_route_item_);
		}
	}
	else
	{
		if( (send_pkt_len + sizeof(struct rip_route_item_)) >  RIP_MAX_PACKET_SIZE )
		{
			/*发送报文*/
			ret = rip_send_packet( device_index, dest_addr, dest_port, rip_send_buffer, send_pkt_len );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return;
			}

			offset = sizeof(struct rip_pkt_head_);
			memset( (void *)(rip_send_buffer + offset), 0, (RIP_MAX_PACKET_SIZE - offset) );
			memcpy((rip_send_buffer + offset ), rip_route_item, sizeof(struct rip_route_item_) );

			/*发送报文长度复位*/
			send_pkt_len = offset + sizeof(struct rip_route_item_);
			send_item_len = sizeof(struct rip_pkt_head_) + sizeof(struct rip_route_item_);
		}
		else
		{
			memcpy((rip_send_buffer + send_pkt_len ), rip_route_item, sizeof(struct rip_route_item_) );

			/*累加发送报文长度*/
			send_pkt_len += sizeof(struct rip_route_item_);
			send_item_len += sizeof(struct rip_route_item_);
		}
	}
	
	return;
}
/*===========================================================
函数名: rip_recv_item
函数功能: 对接收到的路由条目进行处理
输入参数: device_index: 接收路由端口
		  src_addr : 路由源地址
          rip_route_item : 接收到的路由条目
输出参数: 无
返回值:      无
备注:
===========================================================*/
void rip_recv_item( uint32 device_index, struct rip_peer_list_ *peer, struct rip_route_item_ *rip_route_item )
{
	uint32 ret;
	struct prefix node_prefix;
	struct route_node *rip_route_node;
	struct rip_route_ *rip_route_head, *rip_route_old, *rip_route_cmp;
	uint32 distance;
	char string[100];
	struct rip_process_info_ *pprocess = rip_intf_array[device_index]->pprocess;
	
	if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_PROTO_RECV )
	{
		sprintf(string, "\t%s/%d", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
		sprintf(string, "%s via %s metric %lu", string, ip_ntoa(rip_route_item->next_hop), htonl(rip_route_item->metric));
		rip_debug(RIP_DEBUG_IP_RIP_PROTO_RECV, "%s\n", string );
	}

	ret = rip_item_recv_check(device_index, peer->peer_addr, rip_route_item, rip_route_node);
	if( RIP_SUCCESS != ret )
	{
		rip_intf_array[device_index]->rip2IfStatRcvBadRoutes++;
		peer->rip2PeerRcvBadRoutes++;
		return;
	}	
	peer->rip2PeerRcvRoutes++;/*add2019.8.30统计收到的路由条目数*/
	/*检查接收到的路由条目是否存在对应的路由*/
	memset( (void *)&node_prefix, 0, sizeof(struct prefix) );
	node_prefix.family = AF_INET;
	node_prefix.safi = SAFI_UNICAST;
	node_prefix.prefixlen = mask_to_prefix( rip_route_item->mask );
	node_prefix.u.prefix4.s_addr = rip_route_item->network;
	rip_route_node = route_node_lookup(pprocess->rip_table, &node_prefix);

	if(rip_route_node == NULL)
	{
		/*------------------------------------------------
		接收到的路由条目没有对应的路由:
		创建新的路由加入路由表
		---------------------------------------------------*/		
		rip_create_first_rt_frm_nbr( device_index, peer, rip_route_item );
	}
	else if(NULL != rip_route_node->info )
	{
		route_unlock_node( rip_route_node );
		rip_route_head = rip_route_node->info;

		/*获取路由条目对应的路由*/
		rip_route_old = rip_get_relevant_route( device_index, peer->peer_addr, rip_route_head );
		distance = rip_set_distance( pprocess, rip_route_item->network, rip_route_item->mask, peer->peer_addr );
		if( NULL == rip_route_old )
		{
			/*没有从该端口学习过该路由*/	
			/*Fengsb add 2006-02-17， 需要注意，若这之前通过redistribute转发了路由到
			rip本地数据库，这种情况下，不和转发进入的路由比较distance；学习到的路由应该
			添加进来; 注意，转发进来的路由一定放在rip_route_head(除非有默认路由);
			同样的，对于本地直连路由创建的汇总路由也这样处理*/
			if((rip_route_head->route_type == RIP_REDIS_ROUTE)
				||(rip_route_head->route_type == RIP_SUMMARY_ROUTE)
				||(rip_route_head->route_type == RIP_DEF_ROUTE))
			{
				if( rip_route_head->equi_route_num == 1 )
				{/*表示目前该节点只有转发路由,
					接收到的路由条目没有从邻居学习到的对应路由:
					创建新的路由加入路由表*/
					rip_create_first_rt_frm_nbr( device_index, peer, rip_route_item );
				}
				else if(rip_route_head->equi_route_num > 1 )
				{/*表示除了转发路由外，目前还有从新邻居学习到的路由*/
					rip_route_cmp = rip_route_head->forw;
					if(((rip_route_head->route_type==RIP_REDIS_ROUTE)&&(rip_route_cmp->route_type==RIP_SUMMARY_ROUTE))
						||((rip_route_head->route_type==RIP_DEF_ROUTE)&&(rip_route_cmp->route_type==RIP_REDIS_ROUTE)))
					{
						rip_route_cmp=rip_route_cmp->forw;
						if(rip_route_head->equi_route_num==2)
						{
							rip_create_first_rt_frm_nbr( device_index, peer, rip_route_item );
							return;
						}
					}
					if(rip_route_cmp->route_type != RIP_NBR_ROUTE)
					{
						syslog(RIP_LOG_ERR, "rip_recv_item: error for prepair to add nbr route.\n");
					}
					else
					{
						rip_create_equalrt_frm_new_nbr(device_index,peer,rip_route_item, rip_route_cmp);
					}
				}
			    
			}			
			else 
			{
				if( rip_route_head->equi_route_num > 0 )
				{
					rip_route_cmp = rip_route_head;
					rip_create_equalrt_frm_new_nbr(device_index,peer,rip_route_item, rip_route_cmp);
				}
				else
				{
					/*------------------------------------------------
					接收到的路由条目没有对应的路由:
					创建新的路由加入路由表
					---------------------------------------------------*/				
					rip_create_first_rt_frm_nbr( device_index, peer, rip_route_item );
				}
			}			
		}
		else/*( NULL != rip_route_old )*/
		{	/*Fengsb 2006-02-19 一定存在从邻居学习到的路由*/
			if((rip_route_head->route_type == RIP_REDIS_ROUTE)
				||(rip_route_head->route_type == RIP_SUMMARY_ROUTE)
				||(rip_route_head->route_type == RIP_DEF_ROUTE))
			{
				rip_route_cmp = rip_route_head->forw;
				ret=2;
				if(((rip_route_head->route_type==RIP_REDIS_ROUTE)&&(rip_route_cmp->route_type==RIP_SUMMARY_ROUTE))
					||((rip_route_head->route_type==RIP_DEF_ROUTE)&&(rip_route_cmp->route_type==RIP_REDIS_ROUTE)))
					ret++;
				if( rip_route_head->equi_route_num > ret )
				{/*存在从邻居学习到的等价路由*/
					rip_updt_equalrt_frm_nbr(device_index, peer,rip_route_item, rip_route_old);
				}
				else
				{
					rip_updt_singlert_frm_nbr( device_index, peer->peer_addr,rip_route_item, rip_route_old);
				}
			}
			else
			{
				if( rip_route_head->equi_route_num > 1 )
				{/*存在从邻居学习到的等价路由*/
					rip_updt_equalrt_frm_nbr(device_index, peer,rip_route_item, rip_route_old);
				}
				else
				{
					rip_updt_singlert_frm_nbr( device_index, peer->peer_addr,rip_route_item, rip_route_old);
				}
			}			
		}
	}	

	return;
}

/*===========================================================
函数名: rip_create_new_route
函数功能: 根据接收到的路由条目创建新路由
输入参数: device_index : 发送报文的端口
                       src_addr : 报文源地址
                       rip_route_item :接收到的路由条目
输出参数: 无
返回值:      创建新路由成功,返回RIP_SUCCESS
                       创建新路由失败,返回RIP_FAIL
备注:
===========================================================*/
int rip_create_new_route( uint32 device_index,struct rip_peer_list_ *peer,struct rip_route_item_ *rip_route_item)
{
	struct rip_route_ *rip_route, *rip_route_head =NULL;
	struct route_node *rip_route_node; 
	struct prefix route_node_prefix;
	char string[100];
	uint32 distance;
	struct rip_route_list_ *rip_route_list;
	struct rip_process_info_ *pprocess = rip_intf_array[device_index]->pprocess;

	uint32 src_addr = peer->peer_addr;

	memset( (void *)&route_node_prefix, 0, sizeof(struct prefix) );

	/*xuhaiqing 2010-12-21号将最大路由条目数限制取消*/
	/*路由表中路由条数超过最大值，不再添加*/
	/*if( pprocess->route_num >= pprocess->max_route_num )
	{
		rip_warning("%%Warning, Limit for rip routes exceeded, max # is %d.\n", pprocess->max_route_num );
		return RIP_SUCCESS;
	}*/

	distance = rip_set_distance( pprocess, rip_route_item->network, rip_route_item->mask, src_addr );
	if( RIP_MAX_DISTANCE == distance )
	{
		/*不可信源,*/
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	route_node_prefix.family = AF_INET;
	route_node_prefix.safi = SAFI_UNICAST;
	route_node_prefix.prefixlen = mask_to_prefix( rip_route_item->mask);
	route_node_prefix.u.prefix4.s_addr = rip_route_item->network;
	/*Fengsb 2006-02-16 shield this code for rip_recv_item has call route_node_get(),when create a new node 
	this will lock the node once, If the following code do, the lock will increase once more*/

	rip_route_node = route_node_get( pprocess->rip_table, &route_node_prefix ,NULL );
	if( NULL == rip_route_node )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	if( NULL != rip_route_node->info )
	{
		rip_route_head = rip_route_node->info;
        /* Fengsb 2006-02-17  every new entry route node lock++ 
        so we can't unlock */
		/* route_unlock_node( rip_route_node ); */


		/* 判断新增加的路由信息，是否会超过允许生成的最大等价邻居路由数目
		 * Commented by dangzhw in 2010.03.26 11:01:38 */
		if(rip_route_head->equi_nbr_num >= pprocess->max_nh_num)
		{
			rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%%Warning, Limit for rip routes exceeded, max # is %d.\n", pprocess->max_nh_num );
			route_unlock_node(rip_route_node);/*对应node get的lock，dangzhw2010326*/
			return RIP_SUCCESS;
		}
	}

	rip_route = (struct rip_route_ *)rip_mem_malloc( sizeof(struct rip_route_), RIP_ROUTE_TYPE );
	if( rip_route == NULL )
	{
		fprintf(stderr, "RIP: Malloc memory failed.'(%s, %d)\n", __FILE__, __LINE__ );
		return RIP_MEM_FAIL;
	}
	memset( (void *)rip_route, 0, sizeof(struct rip_route_) );
	rip_route->forw = rip_route;
	rip_route->back = rip_route;

	rip_route->metric = htonl(rip_route_item->metric);
	rip_route->route_tag = htons(rip_route_item->route_tag);
	rip_route->refresh_time = time_sec;
	rip_route->route_type = RIP_NBR_ROUTE;
	rip_route->next_hop = rip_route_item->next_hop;
	rip_route->pprocess = pprocess;
	rip_route->distance = distance;

	rip_route->gw_index = device_index;
	rip_route->gw_addr = src_addr;

	rip_route->hold_ptr = NULL;

	/*加入路由表*/
	rip_route->route_node = rip_route_node;

	if( NULL != rip_route_node->info )
	{
        /*Fengsb 2006-02-14 将学习到的邻居路由添加进节点的尾部*/
		INSQUE( rip_route, rip_route_head->back );
	}
	else
	{
		rip_route_node->info = rip_route;

		rip_route_head = rip_route;
	}
	
	rip_route_head->equi_route_num++;
	rip_route_head->equi_nbr_num++;
	pprocess->route_num++;
	pprocess->nbr_route_num++;
	peer->ref_num++;

	/*加入端口的学习路由列表中*/
	rip_route_list = (struct rip_route_list_ *)rip_mem_malloc( sizeof(struct rip_route_list_), RIP_ROUTE_LIST_TYPE );
	if( NULL == rip_route_list )
	{
		fprintf(stderr, "RIP: Malloc memory failed.'(%s, %d)\n", __FILE__, __LINE__ );
		return RIP_MEM_FAIL;
	}
	memset( rip_route_list, 0, sizeof(struct rip_route_list_) );

	rip_route_list->rip_route = rip_route;

	INSQUE( rip_route_list, rip_intf_array[device_index]->nbr_route_list.back );
    	rip_route->nbr_ptr = rip_route_list; /* added by sunhq */

	rip_intf_array[device_index]->nbr_route_num++;
	
	pprocess->rip2GlobalRouteChanges++;

	/*加入主路由表*/
	rip_add_notify( rip_route );
	
	/*触发更新*/
	rip_add_trigger( rip_route, pprocess ,FALSE );

	/*debug*/
	if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_DATABASE )
	{
		memset( (void *)string, 0, 100 );
		sprintf(string, "RIP-DB: Adding %s/%d <metric %lu>", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen, rip_route->metric );
		sprintf(string, "%s via %s to RIP database", string, ip_ntoa( rip_route->gw_addr ) );
		rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%s\n", string );
	}

	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_create_first_rt_frm_nbr
函数功能: 根据接收到的路由条目创建第一条邻居路由
输入参数: device_index : 发送报文的端口
                       src_addr : 报文源地址
                       rip_route_item :接收到的路由条目
输出参数: 无
返回值:      创建新路由成功,返回RIP_SUCCESS
                       创建新路由失败,返回RIP_FAIL
备注: Fengsb 2006-02-19
===========================================================*/
int rip_create_first_rt_frm_nbr( uint32 device_index, struct rip_peer_list_ *peer,struct rip_route_item_ *rip_route_item)
{
	uint32 distance;
	uint32 ret;
	struct rip_process_info_ *pprocess =rip_intf_array[device_index]->pprocess;
	
	distance = rip_set_distance( pprocess, rip_route_item->network, rip_route_item->mask, peer->peer_addr );
	if( RIP_MAX_DISTANCE == distance )
	{
		/*distance 为255的路由为不可信源,忽略该路由*/
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Unbelievable distance)\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
		rip_intf_array[device_index]->rip2IfStatRcvBadRoutes++;
		peer->rip2PeerRcvBadRoutes++;
		return RIP_FAIL;
	}

	if( RIP_MAX_METRIC == htonl(rip_route_item->metric) )
	{
		/*metric 为16的新路由,忽略*/
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Unreachble metric)\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
		rip_intf_array[device_index]->rip2IfStatRcvBadRoutes++;
		peer->rip2PeerRcvBadRoutes++;
		return RIP_FAIL;
	}
	
	ret = rip_create_new_route( device_index, peer, rip_route_item );
	if( RIP_SUCCESS != ret )
	{
		return RIP_FAIL;
	}

	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_create_equalrt_frm_new_nbr
函数功能: 从新的邻居接收到路由，并创建邻居等价路由
输入参数: device_index : 发送报文的端口
                       src_addr : 报文源地址
                       rip_route_item :接收到的路由条目
                       rip_route_cmp: route node下的参照邻居路由(从其他端口
                       学习到的邻居路由)
输出参数: 无
返回值:      创建新路由成功,返回RIP_SUCCESS
                       创建新路由失败,返回RIP_FAIL
备注: Fengsb 2006-02-19
===========================================================*/
int rip_create_equalrt_frm_new_nbr( uint32 device_index, struct rip_peer_list_ *peer,struct rip_route_item_ *rip_route_item, struct rip_route_ *rip_route_cmp)
{	
	uint32 distance;
	uint32 ret;
	struct rip_process_info_ *pprocess = rip_intf_array[device_index]->pprocess;
	
	distance = rip_set_distance( pprocess, rip_route_item->network, rip_route_item->mask, peer->peer_addr );

	if( distance > rip_route_cmp->distance )
	{
		/*忽略该路由*/
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Larger distance)\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
		rip_intf_array[device_index]->rip2IfStatRcvBadRoutes++;
		peer->rip2PeerRcvBadRoutes++;
		return RIP_FAIL;
	}
	else if( distance < rip_route_cmp->distance )
	{
		if( RIP_MAX_METRIC == htonl(rip_route_item->metric) )
		{
			/*忽略该路由*/
			rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Unreachble metric)\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
			rip_intf_array[device_index]->rip2IfStatRcvBadRoutes++;
			peer->rip2PeerRcvBadRoutes++;
			return RIP_FAIL;
		}
		else
		{
			/*将原有路由删除*/
			rip_del_nbrrt_frm_node( rip_route_cmp );

			/*将路由条目加入路由表*/
			ret = rip_create_new_route( device_index, peer , rip_route_item );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}
		}
	}
	else if( distance == rip_route_cmp->distance )
	{
		if( htonl(rip_route_item->metric) > rip_route_cmp->metric )
		{
			/*忽略该路由*/
			rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Larger metric)\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask));
			rip_intf_array[device_index]->rip2IfStatRcvBadRoutes++;
			peer->rip2PeerRcvBadRoutes++;
			return RIP_FAIL;
		}
		else if( htonl(rip_route_item->metric) < rip_route_cmp->metric )
		{
			/*将原有路由删除*/
			rip_del_nbrrt_frm_node( rip_route_cmp );

			/*将路由条目加入路由表*/
			ret = rip_create_new_route( device_index, peer , rip_route_item );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}
		}
		else if( htonl(rip_route_item->metric) == rip_route_cmp->metric )
		{
			if( RIP_MAX_METRIC == htonl(rip_route_item->metric) )
			{
				/*忽略该路由*/
				rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Unreachble metric)\n", \
				    ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask));
				rip_intf_array[device_index]->rip2IfStatRcvBadRoutes++;
				peer->rip2PeerRcvBadRoutes++;
				return RIP_FAIL;
			}
			else
			{
				/*将路由条目加入路由表*/
				ret = rip_create_new_route( device_index, peer , rip_route_item );
				if( RIP_SUCCESS != ret )
				{
					rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
					return RIP_FAIL;
				}
			}
		}
	}
	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_updt_equalrt_frm_nbr
函数功能: 从邻居接收到等价路由中某条路由的更新
输入参数: device_index : 发送报文的端口
                       src_addr : 报文源地址
                       rip_route_item :接收到的路由条目
                       rip_route_old: route node下的从device_index端口下邻居学习到
                       的邻居路由
输出参数: 无
返回值:      创建新路由成功,返回RIP_SUCCESS
                       创建新路由失败,返回RIP_FAIL
备注: Fengsb 2006-02-19
===========================================================*/
int rip_updt_equalrt_frm_nbr( uint32 device_index, struct rip_peer_list_ *peer,struct rip_route_item_ *rip_route_item, struct rip_route_ *rip_route_old)
{
	uint32 distance;
	uint32 ret;
	struct rip_process_info_ *pprocess = rip_intf_array[device_index]->pprocess;
	
	distance = rip_set_distance( pprocess, rip_route_item->network, rip_route_item->mask, peer->peer_addr  );

	if( (RIP_MAX_METRIC == htonl(rip_route_item->metric))
		&& (htonl(rip_route_item->metric) != rip_route_old->metric) )
	{
		/*将路由条目对应的路由从路由表中删除*/
		/*从holddown中删除*/
		rip_del_holddown( rip_route_old );

		/*从相应端口的学习列表中删除*/
		rip_del_from_nbr_list( rip_route_old );

		/*从路由表中删除*/
		ret = rip_del_route( rip_route_old );
		if( RIP_SUCCESS != ret )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}
	}
	else
	{
		/*有等价路由*/
		if( distance > rip_route_old->distance )
		{
			/*将路由条目对应的路由从路由表中删除*/
			/*从holddown中删除*/
			rip_del_holddown( rip_route_old );

			/*从相应端口的学习列表中删除*/
			rip_del_from_nbr_list( rip_route_old );

			/*从路由表中删除*/
			ret = rip_del_route( rip_route_old );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}
		}
		else if( distance < rip_route_old->distance )
		{
			if( RIP_MAX_METRIC == htonl(rip_route_item->metric) )
			{
				rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Unreachble metric)\n", \
                        							ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
				rip_intf_array[device_index]->rip2IfStatRcvBadRoutes++;
				peer->rip2PeerRcvBadRoutes++;
				return RIP_FAIL;
			}
			else
			{
				/*将route node中所有邻居路由删除*/
				rip_del_nbrrt_frm_node( rip_route_old );

				/*将路由条目加入路由表*/
				ret = rip_create_new_route( device_index, peer , rip_route_item );
				if( RIP_SUCCESS != ret )
				{
					rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
					return RIP_FAIL;
				}
			}
		}
		else/*( distance == rip_route_old->distance )*/
		{
			if( htonl(rip_route_item->metric) > rip_route_old->metric )
			{
				/*将路由条目对应的路由从路由表中删除*/
				/*从holddown中删除*/
				rip_del_holddown( rip_route_old );

				/*从相应端口的学习列表中删除*/
				rip_del_from_nbr_list( rip_route_old );

				/*从路由表中删除*/
				ret = rip_del_route( rip_route_old );
				if( RIP_SUCCESS != ret )
				{
					rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
					return RIP_FAIL;
				}
			}
			else if( htonl(rip_route_item->metric) < rip_route_old->metric )
			{
				/*将route node中所有邻居路由删除*/
				rip_del_nbrrt_frm_node( rip_route_old );

				/*将路由条目加入路由表*/
				ret = rip_create_new_route( device_index, peer , rip_route_item );
				if( RIP_SUCCESS != ret )
				{
					rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
					return RIP_FAIL;
				}
			}
			else/*( rip_route_item->metric == rip_route_old->metric )*/
			{
				/*更新路由条目对应路由的刷新时间*/
				rip_route_old->next_hop = rip_route_item->next_hop;
				rip_route_old->route_tag = htons(rip_route_item->route_tag);

				if( RIP_MAX_METRIC != htonl(rip_route_item->metric) )
				{
					rip_route_old->refresh_time = time_sec;
				}
			}
		}
	}
	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_updt_singlert_frm_nbr
函数功能: 从新的邻居接收到路由，并创建邻居等价路由
输入参数: device_index : 发送报文的端口
                       src_addr : 报文源地址
                       rip_route_item :接收到的路由条目
                       rip_route_old: route node下的从device_index端口下邻居学习到
输出参数: 无
返回值:      创建新路由成功,返回RIP_SUCCESS
                       创建新路由失败,返回RIP_FAIL
备注: Fengsb 2006-02-19
===========================================================*/
int rip_updt_singlert_frm_nbr( uint32 device_index, uint32 src_addr,struct rip_route_item_ *rip_route_item, struct rip_route_ *rip_route_old)
{
	uint32 distance;
	uint32 ret;
	struct rip_process_info_ *pprocess = rip_intf_array[device_index]->pprocess;
	
	distance = rip_set_distance( pprocess, rip_route_item->network, rip_route_item->mask, src_addr );
	
	/*没有等价路由*/
	if( RIP_MAX_DISTANCE == distance )
	{
		/*255为不可信源,将路由从路由表中删除*/

		/*从holddown中删除*/
		rip_del_holddown( rip_route_old );

		/*从相应端口的学习列表中删除*/
		rip_del_from_nbr_list( rip_route_old );

		/*从路由表中删除*/
		ret = rip_del_route( rip_route_old );
		if( RIP_SUCCESS != ret )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}
	}
	else if( (htonl(rip_route_item->metric) != rip_route_old->metric)
		&& (RIP_MAX_METRIC == htonl(rip_route_item->metric)) )
	{
		/*接收到metric为16的路由条目,进入holddown状态*/
		rip_route_old->distance = distance;
		rip_route_old->metric = htonl(rip_route_item->metric);
		rip_route_old->next_hop = rip_route_item->next_hop;
		rip_route_old->route_tag = htons(rip_route_item->route_tag);
		rip_route_old->refresh_time = time_sec;
		
		/*路由进入holddown状态*/
		rip_add_holddown( rip_route_old );

		/*从主路由表中删除*/
		rip_del_from_main_tbl( rip_route_old ,pprocess->process_id);
	}
	else  if( (distance != rip_route_old->distance) || (htonl(rip_route_item->metric) != rip_route_old->metric) )
	{
		ret = (RIP_MAX_METRIC == rip_route_old->metric)?1:0;
		
		/*通知主路由表distance和metric的变化*/
		rip_route_old->distance = distance;
		rip_route_old->metric = htonl(rip_route_item->metric);
		rip_route_old->next_hop = rip_route_item->next_hop;
		rip_route_old->route_tag = htons(rip_route_item->route_tag);

		if( RIP_MAX_METRIC != rip_route_old->metric ) /* 此时rip_route_old->metric已经更新了 */
		{
			rip_route_old->refresh_time = time_sec;
			if(ret) 
			{
				/*若rip_route_old原来是holddown路由,从holddown列表中删除*/
				rip_del_holddown( rip_route_old );
			}
			else ret=1;
		}
		else ret = 0; /* 此时新旧metric都是16，无需更新主路由表 */
		if(ret)
		{
			rip_change_notify( rip_route_old );
			/*Fengsb 2006-02-13 add the following policy, when route changed ,we should trigger*/
			rip_add_trigger( rip_route_old,pprocess , FALSE );     
		}               
	}
	else
	{
		/*更新刷新时间*/
		rip_route_old->next_hop = rip_route_item->next_hop;
		rip_route_old->route_tag = htons(rip_route_item->route_tag);

		if( RIP_MAX_METRIC != htonl(rip_route_item->metric) )
		{
			rip_route_old->refresh_time = time_sec;
		}
	}

	return RIP_SUCCESS;		
}

/*===========================================================
函数名:      rip_del_route_node
函数功能: 清空路由节点
输入参数: rip_route : 待删除的路由节点中的路由
输出参数: 无
返回值:      无
备注:
=============================================================*/
void rip_del_route_node( struct rip_route_ *rip_route )
{
	struct rip_route_ *rip_route_temp, *rip_route_forw;
	int ret;
	if(!rip_route)return;
	
	rip_route_temp = rip_route->forw;
	while( rip_route_temp != rip_route )
	{
		rip_route_forw = rip_route_temp->forw;

		/*从holddown列表中删除*/
		rip_del_holddown( rip_route_temp);

		/*从相应端口的学习路由列表中删除*/
		rip_del_from_nbr_list( rip_route_temp );

		/*从重发布列表中删除*/
		rip_del_from_redis_list( rip_route_temp );
		
		/*删除路由*/
		ret = rip_del_route( rip_route_temp );
		if( RIP_SUCCESS != ret )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return;
		}

		rip_route_temp = rip_route_forw;
	};

	/*free rip_route*/
	/*从holddown列表中删除*/
	rip_del_holddown( rip_route );

	/*从相应端口的学习路由列表中删除*/
	rip_del_from_nbr_list( rip_route );
	
	/*从重发布列表中删除*/
	rip_del_from_redis_list( rip_route );
	
	/*删除路由*/
	ret = rip_del_route( rip_route );
	if( RIP_SUCCESS != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return;
	}
#if 0	
    /*Fengsb 2006-02-16 move the code into rip_del_route*/
	/*route node中已无路由,将route node删除*/
	if( rip_route_node->info == NULL )
	{
		if( rip_route_node->lock > 1 )
		{
			route_unlock_node( rip_route_node );
		}
	}	
#endif

	return;
}


/*===========================================================
函数名:      rip_del_nbrrt_frm_node
函数功能: 清空路由节点下从邻居学习到的路由
输入参数: rip_route : 待删除的路由节点中的邻居路由
输出参数: 无
返回值:      无
备注:
=============================================================*/
void rip_del_nbrrt_frm_node( struct rip_route_ *rip_route )
{
	struct rip_route_ *rip_route_temp, *rip_route_head, *rip_route_forw;
	int ret;
	struct route_node *rip_route_node;

	rip_route_node = rip_route->route_node;
	rip_route_head = (struct rip_route_ *)(rip_route_node->info);

	rip_route_temp = rip_route->forw;
	while( rip_route_temp != rip_route )
	{
		rip_route_forw = rip_route_temp->forw;
		if(rip_route_temp->route_type != RIP_NBR_ROUTE)
		{
			rip_route_temp = rip_route_forw;
			continue;
		}

		/*从holddown列表中删除*/
		rip_del_holddown( rip_route_temp );

		/*从相应端口的学习路由列表中删除*/
		rip_del_from_nbr_list( rip_route_temp );

		/*删除路由*/
		ret = rip_del_route( rip_route_temp );
		if( RIP_SUCCESS != ret )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return;
		}

		rip_route_temp = rip_route_forw;
	};

	if(rip_route->route_type != RIP_NBR_ROUTE)
		return;
	
	/*free rip_route*/
	/*从holddown列表中删除*/
	rip_del_holddown( rip_route );

	/*从相应端口的学习路由列表中删除*/
	rip_del_from_nbr_list( rip_route );

	/*删除路由*/
	ret = rip_del_route( rip_route );
	if( RIP_SUCCESS != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return;
	}
#if 0	
    /*Fengsb 2006-02-16 move the code into rip_del_route*/
	/*route node中已无路由,将route node删除*/
	if( rip_route_node->info == NULL )
	{
		if( rip_route_node->lock > 1 )
		{
			route_unlock_node( rip_route_node );
		}
	}	
#endif

	return;
}

/*===========================================================
函数名:      rip_del_rn_callback
函数功能: 清空路由节点，用于route_table_destory回调
输入参数: rip_route : 待删除的路由节点中的路由
输出参数: 无
返回值:   无
备注:Fengsb add 2006-02-17 因为route_table_destory在调用rip_del_rn_callback
后会delete route node，所以，我们在rip_del_rn_callback的处理过程中不能delete
该节点，否则内存重复释放，出错。这就要求多lock一次
=============================================================*/
void rip_del_rn_callback( struct rip_route_ *rip_route )
{
	struct route_node *rip_route_node;

    if(rip_route && rip_route->route_node)
    {
        rip_route_node = rip_route->route_node;
        route_lock_node( rip_route_node );
        rip_del_route_node(rip_route);
    }
    
	return;
}

/*===========================================================
函数名:      rip_del_trigger_tree_callback
函数功能: 清空触发链表，用于route_table_destory回调
输入参数: rtl(rip_trigger_list) : 待删除的触发链表
输出参数: 无
返回值:   无
备注:lds 2011年5月23日 15:09:57
=============================================================*/
void rip_del_trigger_tree_callback( struct rip_trigger_list_ *rtl)
{
	struct rip_trigger_list_ * rip_trigger_forw;
	struct rip_trigger_list_ * rip_trigger_temp;
	char string[100];

	if (NULL == rtl)
		return;
	
	rip_trigger_forw = rtl->forw;

	/*因为这个是不带头结点的循环链表 从第二个结点开始遍历*/
	while(rip_trigger_forw != rtl)
	{
		rip_trigger_temp = rip_trigger_forw;
		rip_trigger_forw = rip_trigger_forw->forw;

		if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_DATABASE)
		{
			sprintf(string, "RIP-DB: route %s/%d <metric %ld>", ip_ntoa(rip_trigger_temp->network), mask_to_prefix(rip_trigger_temp->mask), rip_trigger_temp->metric );
			sprintf(string, "%s via %s un-triggered", string, ip_ntoa( rip_trigger_temp->gw_addr ) );
			rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%s\n", string );
		}
		rip_mem_free( rip_trigger_temp, RIP_TRIGGER_LIST_TYPE );
	}

	if (NULL != rtl ) /*清空最后一个结点，也就是第一个结点*/
	{
		rip_trigger_temp = rtl;
		if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_DATABASE)
		{
			sprintf(string, "RIP-DB: route %s/%d <metric %ld>", ip_ntoa(rip_trigger_temp->network), mask_to_prefix(rip_trigger_temp->mask), rip_trigger_temp->metric );
			sprintf(string, "%s via %s un-triggered", string, ip_ntoa( rip_trigger_temp->gw_addr ) );
			rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%s\n", string );
		}
		rip_mem_free( rip_trigger_temp, RIP_TRIGGER_LIST_TYPE );
	}
    
	return;
}

/*===========================================================
函数名: rip_get_pkt_dest_addr
函数功能: 获取报文目的地址
输入参数: device_index : 发送报文的端口
输出参数: 无
返回值:      报文的目的地址
备注:
===========================================================*/
uint32 rip_get_pkt_dest_addr( uint32 device_index )
{
	int send_version;

	send_version = rip_set_send_version( device_index );
	if( (send_version == RIP_SEND_VERSION_1)
		|| (send_version == RIP_SEND_VERSION_2_BROADCAST) )
	{
		/*广播地址255.255.255.255*/
		return RIP_BROADCAST_ADDRESS;
	}
	else	/* 配合将默认版本号设置为2
	 * Commented by dangzhw in 2010.02.20 15:09:43 */
	{
		/*组播地址224.0.0.9*/
		return htonl(RIP_MULTICAST_ADDRESS);
	}
}

/*===========================================================
函数名: rip_split_check
函数功能: 检查端口发送出去的路由是否被水平分割禁止发送
输入参数: device_index : 发送报文的端口
          rip_route_info : 待检查的路由信息
输出参数: 无
返回值:      若报文能通过水平分割过滤,返回RIP_SUCCESS
          若报文不能通过水平分割过滤,返回RIP_FAIL
备注:
===========================================================*/
int rip_split_check( uint32 device_index, struct rip_route_info_ *rip_route_info )
{

	/***********fang qi modify on 2007-02-28 ******************************/
	if( INTERFACE_DEVICE_FLAGS_NET_POINTTOMULTI == rip_intf_array[device_index]->encap_type )
		return RIP_SUCCESS;

	if( (rip_route_info->gw_index == device_index)||
		( (RIP_REDIS_ROUTE == rip_route_info->route_type) &&
		   (rip_route_info->network == (rip_intf_array[device_index]->address & rip_intf_array[device_index]->mask ))
		   && (rip_route_info->mask == rip_intf_array[device_index]->mask) )  )  // 这条路由
	{
		if(rip_intf_array[device_index]->split_flag & RIP_POISONED_SPLIT_HORIZON_ENABLE)
		{
			/* 毒素分割启动，metric返回16      */
			rip_route_info->metric = RIP_MAX_METRIC;
            return RIP_SUCCESS;
		}
		else if ((rip_intf_array[device_index]->split_flag & RIP_SIMPLE_SPLIT_HORIZON_ENABLE) ==0)
		{
			/*毒素逆转没启动,水平分割禁止.可以发送相关路由*/
			return RIP_SUCCESS;
		}

		return RIP_FAIL;
	}

	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_filter_in_check
函数功能: 对接收到的路由进行filter in过滤检查
输入参数: device_index      : 接收报文的端口
          src_addr      : 报文源地址
          rip_route_item: 接收到的路由条目
输出参数: 无
返回值:      若报文能通过filter过滤,返回RIP_SUCCESS
          若报文不能通过filter过滤,返回RIP_FAIL
备注:
===========================================================*/
int rip_filter_in_check( uint32 device_index, uint32 src_addr, struct rip_route_item_ *rip_route_item )
{
	int ret;
	uint32 mask;
	DEVICE_ETERNAL_ID device_eternal_id;
	struct rip_process_info_ *pprocess;

	ret = interface_omnivorous_callback_makeeternal( device_index, &device_eternal_id );
	if( INTERFACE_GLOBAL_SUCCESS != ret )
	{
		return RIP_FAIL;
	}

	if( 0 == rip_route_item->mask )
	{/*Fengsb add 2006-05-08， 表示接收到的是version 1的rip路由*/
		mask = rip_get_mask( rip_route_item->network );
	}
	else
	{
		mask = rip_route_item->mask;
	}

	if(!(pprocess = rip_intf_array[device_index]->pprocess))
	{
		syslog(RIP_LOG_ERR, "RIP: %s,%d error\n" ,__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	ret = route_distribute_list_apply_in( pprocess->filter_list, device_eternal_id, rip_route_item->network, mask, src_addr );
	if( ROUTE_FILTER_DENY == ret )
	{
		return RIP_FAIL;
	}

	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_filter_out_check
函数功能: 对发送出去的路由进行filter out过滤检查
输入参数: device_index : 发送报文的端口
          dest_addr : 发送报文的目的地址
          rip_route_item:待发送出去的路由条目
输出参数: 无
返回值:      若报文能通过filter 过滤,返回RIP_SUCCESS
          若报文不能通过filter 过滤,返回RIP_FAIL
备注:
===========================================================*/
int rip_filter_out_check( uint32 device_index, uint32 gateway, struct rip_route_item_ *rip_route_item )
{
	int ret;
	uint32 mask;
	DEVICE_ETERNAL_ID device_eternal_id;
	struct rip_process_info_ *pprocess;
	
	ret = interface_omnivorous_callback_makeeternal( device_index, &device_eternal_id );
	if( INTERFACE_GLOBAL_SUCCESS != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	if( 0 == rip_route_item->mask )
	{   /*Fengsb add 2006-05-08， 表示接收到的是version 1的rip路由*/
		mask = rip_get_mask( rip_route_item->network );
	}
	else
	{
		mask = rip_route_item->mask;
	}

	if(!(pprocess = rip_intf_array[device_index]->pprocess))
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	ret = route_distribute_list_apply_out( pprocess->filter_list, device_eternal_id, rip_route_item->network, mask, gateway );
	if( ROUTE_FILTER_DENY == ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_offset_in_check
函数功能: 对接收到的路由路由进行offset信息控制
输入参数: device_index : 接收报文的端口
           rip_route_item : 接收到的路由条目
输出参数: 无
返回值:      无
备注:
===========================================================*/
int rip_offset_in_check( uint32 device_index,struct rip_route_item_ *rip_route_item )
{
	int ret;
	uint32 mask;
	DEVICE_ETERNAL_ID device_eternal_id;
	uint32 offset;
	struct rip_process_info_ *pprocess =rip_intf_array[device_index]->pprocess;

	ret = interface_omnivorous_callback_makeeternal( device_index, &device_eternal_id );
	if( INTERFACE_GLOBAL_SUCCESS != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	if( 0 == rip_route_item->mask )
	{/*Fengsb add 2006-05-08， 表示接收到的是version 1的rip路由*/
		mask = rip_get_mask( rip_route_item->network );
	}
	else
	{
		mask = rip_route_item->mask;
	}
    /*Fengsb end code here */

	offset = route_offset_list_apply_in( pprocess->offset_list, device_eternal_id, rip_route_item->network, mask);
	/*
	rip_route_item->metric += offset; 
	modify by guqinghua 2008-7-24      
	*/
	offset=offset+htonl(rip_route_item->metric);
	rip_route_item->metric = htonl(offset);

	if( htonl(rip_route_item->metric) > RIP_MAX_METRIC )
	{
		rip_route_item->metric = htonl(RIP_MAX_METRIC);
	}

	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_offset_out_check
函数功能: 对接收到的路由路由进行offset信息控制
输入参数: device_index : 接收报文的端口
           rip_route_item : 接收到的路由条目
输出参数: 无
返回值:      无
备注:
===========================================================*/
int rip_offset_out_check( uint32 device_index,struct rip_route_item_ *rip_route_item )
{
	int ret;
	uint32 mask;
	DEVICE_ETERNAL_ID device_eternal_id;
	struct rip_process_info_ *pprocess;
	uint32 offset;

	ret = interface_omnivorous_callback_makeeternal( device_index, &device_eternal_id );
	if( INTERFACE_GLOBAL_SUCCESS != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

    if( 0 == rip_route_item->mask )
	{/*Fengsb add 2006-05-08， 表示接收到的是version 1的rip路由*/
		mask = rip_get_mask( rip_route_item->network );
	}
	else
	{
		mask = rip_route_item->mask;
	}
    /*Fengsb end code here */
	if(!(pprocess = rip_intf_array[device_index]->pprocess))
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	offset = route_offset_list_apply_out( pprocess->offset_list, device_eternal_id, rip_route_item->network, mask);

	rip_route_item->metric=htonl(htonl( rip_route_item->metric) + offset);

	if( htonl(rip_route_item->metric) > RIP_MAX_METRIC )
	{
		rip_route_item->metric = htonl(RIP_MAX_METRIC);
	}

	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_auto_sum_updt
函数功能: 对发送出去路由路由进行auto-summary过滤
输入参数: device_index : 发送报文端口
                       rip_rthead : 待发送出去的路由信息
输出参数: rip_route_item : 待发送出去的 路由条目
返回值:      路由通过auto-summary过滤,返回RIP_SUCCESS
                       路由不能通过auto-summary过滤,返回RIP_FAIL
备注: 主动汇总常规更新
===========================================================*/
int rip_auto_sum_updt( uint32 device_index, uint32 dest_addr, uint16 dest_port, struct rip_route_ * rip_rthead)
{
	uint32 natural_net;
	uint32 natural_mask;
	uint32 natural_intf_net;
	uint32 count, vrf_id;
	uint32 mask; /* 接口IP地址掩码*/
	BOOL intf_exist =FALSE;
	struct rip_subrt_ *rip_subrt;
	struct rip_pkt_head_ *rip_pkt_head;
	struct rip_route_item_ rip_route_item;
	struct rip_route_ *rip_route;

	memset( &rip_route_item, 0, sizeof(struct rip_route_item_) );
	
	vrf_id = rip_intf_array[device_index]->vrf_id;
	rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;	

#if 0
	mask = prefix_to_mask(rip_rthead->route_node->p.prefixlen)

	if( (0 == mask) && (0 == rip_rthead->route_node->p.u.prefix4.s_addr) )
	{
		/*缺省路由*/
		rip_route_item.mask = 0;
		rip_route_item.network = 0;
	}
#endif

	natural_net = rip_get_natural_network(rip_rthead->route_node->p.u.prefix4.s_addr );
	natural_mask = rip_get_natural_mask( rip_rthead->route_node->p.u.prefix4.s_addr );
	natural_intf_net = rip_get_natural_network( rip_intf_array[device_index]->address );

	/*汇总功能启动,与发送端口具有不同主网的 路由被汇总发送*/
	rip_route_item.network = natural_net;
	if( RIP_VERSION_1 == rip_pkt_head->version )
	{
		rip_route_item.mask = 0;
	}
	else if( RIP_VERSION_2 == rip_pkt_head->version )
	{
		rip_route_item.mask = natural_mask;
	}

	if(rip_rthead->route_type == RIP_SUMMARY_ROUTE)
	{/*遍历该entry的端口，看是否有直连路由的device_index
		和本端口一致，如果有，则不发送该汇总路由，而发送相应的
		直连路由(不汇总)。*/
		QUE_LIST(rip_subrt, &rip_rthead->rip_connect){
			if(rip_subrt->rip_route->gw_index == device_index)
			{
				intf_exist = TRUE;
				break;				
			}
		}QUE_LIST_END(rip_subrt, &rip_rthead->rip_connect);	

		if(intf_exist == TRUE)
		{
			QUE_LIST(rip_subrt, &rip_rthead->rip_connect){
				if(rip_subrt->rip_route->gw_index == device_index)
					continue;	
				
				mask = prefix_to_mask(rip_subrt->rip_route->route_node->p.prefixlen);					
				if(rip_subrt->rip_route->gw_index != device_index  && 
					rip_intf_array[device_index]->mask == mask)
				{/*主网相同，子网号不同，掩码相同的可以被发送出去*/
					if( RIP_VERSION_1 == rip_pkt_head->version )
					{
						rip_route_item.mask = 0;
					}
					else if( RIP_VERSION_2 == rip_pkt_head->version )
					{
						rip_route_item.mask = mask;
					}
					rip_route_item.network = rip_subrt ->rip_route->route_node->p.u.prefix4.s_addr;
					/*rip_route_item.network = rip_rthead->route_node->p.u.prefix4.s_addr;*/
					rip_add_route_to_pkt(device_index,dest_addr,dest_port, rip_subrt->rip_route, &rip_route_item);
				}
			}QUE_LIST_END(rip_subrt, &rip_rthead->rip_connect);				
		}
		else
		{
			rip_add_route_to_pkt( device_index, dest_addr, dest_port, rip_rthead, &rip_route_item );
		}
		
	}
	else
	{			
		if(rip_rthead->route_type == RIP_REDIS_ROUTE)
		{
			if( natural_net != natural_intf_net )
				rip_add_route_to_pkt( device_index, dest_addr, dest_port, rip_rthead, &rip_route_item );
		}
		else
		{
			/*Fengsb 2006-02-17add code here, policy: 
			若存在等价路由C（出端口分别为1，2，3），intf－1配置了一般水平分割，intf－2上配置了中毒逆转
			intf－3上是缺省配置
			在常规更新时，发送得路由如下：
			对于intf-1，不发送针对路由C的更新
			对于intf-2，发送针对路由C的更新，metric = 16
			对于intf-3，发送针对路由C的更新，其中metric + 1
			所以，先遍历，看是否有rip_route_info.gw_index==device_index,
			若有，则只处理该rip_route_info, 然后跳出；
			若没有，则处理第一个rip_route_info，然后跳出内层循环*/
			if(rip_rthead->equi_route_num > 1)
			{
				rip_route = rip_rthead; 
				for( count = 1; count <= rip_rthead->equi_route_num; rip_route = rip_route->forw, count++ )
				{
					if(rip_route->gw_index == device_index)
					{
						rip_rthead = rip_route;
						break;
					}
				}                
			}
			else
			{
				;
			}
			if( natural_net != natural_intf_net )
				rip_add_route_to_pkt( device_index, dest_addr, dest_port, rip_rthead, &rip_route_item );
		}
	}
	
	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_no_auto_sum_updt
函数功能: 对发送出去路由路由进行auto-summary过滤
输入参数: device_index : 发送报文端口
          rip_rthead : 待发送出去的路由信息
输出参数: rip_route_item : 待发送出去的 路由条目
返回值:      路由通过auto-summary过滤,返回RIP_SUCCESS
          路由不能通过auto-summary过滤,返回RIP_FAIL
备注:		禁止自动汇总时的常规更新
===========================================================*/
int rip_no_auto_sum_updt(uint32 device_index, uint32 dest_addr, uint16 dest_port, struct rip_route_ * rip_rthead)
{
	uint32 count, vrf_id;
	struct rip_pkt_head_ *rip_pkt_head;
	struct rip_route_item_ rip_route_item;
	struct rip_route_ *rip_route;

	memset( &rip_route_item, 0, sizeof(struct rip_route_item_) );
	
	vrf_id = rip_intf_array[device_index]->vrf_id;
	rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;	

#if 0
	mask = prefix_to_mask(rip_rthead->route_node->p.prefixlen)

	if( (0 == mask) && (0 == rip_rthead->route_node->p.u.prefix4.s_addr) )
	{
		/*缺省路由*/
		rip_route_item.mask = 0;
		rip_route_item.network = 0;
	}
#endif
	rip_route_item.network = rip_rthead->route_node->p.u.prefix4.s_addr;

	if( RIP_VERSION_1 == rip_pkt_head->version )
	{
		rip_route_item.mask = 0;
	}
	else if( RIP_VERSION_2 == rip_pkt_head->version )
	{
		rip_route_item.mask = prefix_to_mask(rip_rthead->route_node->p.prefixlen);
	}
		
	if(rip_rthead->route_type == RIP_SUMMARY_ROUTE)
	{   /*在no auto sum 的时候，汇总路由不发，但是要发送从邻居学习到的路由*/
		if(rip_rthead->equi_route_num > 1)
		{
			rip_route = rip_rthead->forw; 
			for( count = 1; count < rip_rthead->equi_route_num; rip_route = rip_route->forw, count++ )
			{
				/*if(rip_route->route_type == RIP_SUMMARY_ROUTE)
					continue;*/
				if(rip_route->gw_index == device_index)
				{
					rip_rthead = rip_route;
					break;
				}
			} 
			if(rip_rthead->route_type == RIP_SUMMARY_ROUTE)rip_rthead=rip_rthead->forw;
			rip_add_route_to_pkt( device_index, dest_addr, dest_port, rip_rthead, &rip_route_item );
		}				
	}
	else if((rip_rthead->route_type==RIP_REDIS_ROUTE)||(rip_rthead->route_type==RIP_DEF_ROUTE))
	{		
		rip_add_route_to_pkt( device_index, dest_addr, dest_port, rip_rthead, &rip_route_item );
	}
	else
	{
		if(rip_rthead->equi_route_num > 1)
		{
			rip_route = rip_rthead; 
			for( count = 1; count <= rip_rthead->equi_route_num; rip_route = rip_route->forw, count++ )
			{
				if(rip_route->gw_index == device_index)
				{
					rip_rthead = rip_route;
					break;
				}
			}                
		}
		rip_add_route_to_pkt( device_index, dest_addr, dest_port, rip_rthead, &rip_route_item );
	}	
	
	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_next_hop_check
函数功能: 对发送出去路由的next-hop信息控制
输入参数: device_index : 发送报文端口
          rip_route_info : 待发送出去的路由信息
          rip_route_item : 待发送出去的路由条目
输出参数: 无
返回值:      无
备注:
===========================================================*/
void rip_next_hop_check( uint32 device_index, struct rip_route_info_ *rip_route_info, struct rip_route_item_ *rip_route_item )
{
	struct rip_pkt_head_ *rip_pkt_head;

	rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;

	if( RIP_VERSION_1 == rip_pkt_head->version )
	{
		rip_route_item->next_hop = 0;
	}
	else if( RIP_VERSION_2 == rip_pkt_head->version )
	{
		if( INTERFACE_DEVICE_FLAGS_NET_POINTTOPOINT != rip_intf_array[device_index]->encap_type )
		{
			/*非点对点端口*/
			if((rip_route_info->next_hop & rip_intf_array[device_index]->mask) == (rip_intf_array[device_index]->address & rip_intf_array[device_index]->mask) )
			{
				rip_route_item->next_hop = rip_route_info->next_hop;
			}
			else if( (rip_route_info->gw_addr & rip_intf_array[device_index]->mask ) == (rip_intf_array[device_index]->address & rip_intf_array[device_index]->mask) )
			{
				rip_route_item->next_hop = rip_route_info->gw_addr;
			}
			else
			{
				rip_route_item->next_hop = 0;
			}
		}
		else
		{
			/*点对点端口*/
			rip_route_item->next_hop = 0;
		}
	}

	return;
}

/*===========================================================
函数名: rip_nbrrt_check_to_pkt
函数功能: 从邻居学习到的路由汇总后发送出去前的检查,只加一条
输入参数: rip_route_item : 待发送的路由条目
输出参数: 无
返回值:      无
备注:  考虑到二叉树的左序遍历特性，其实如果最后一个
item不是认证尾部，就可以用最后一个item来进行比较。这样
可以大大节省CPU时间。因为没有遍历整个报文，所以，对于触发更新
条目，因为在触发更新列表中的存储方式不同
, 这个方式可能没有办法检查。t替换原来的函数rip_equi_check
===========================================================*/
int rip_nbrrt_check_to_pkt( uint32 device_index, struct rip_route_item_ *rip_route_item )
{	
	struct rip_pkt_head_ *rip_pkt_head;	
	struct rip_route_item_ *rip_last_item;
	uint16 offset;

	rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;

	if( (RIP_VERSION_2 == rip_pkt_head->version)
		&& (RIP_AUTH_MD5 == rip_intf_array[device_index]->auth_type)
		&& (0 != strlen(rip_intf_array[device_index]->md5_key)) )
	{
		offset = sizeof(struct rip_pkt_head_) + sizeof(struct rip_md5_head_);		
	}
	else if( (RIP_VERSION_2 == rip_pkt_head->version)
		    && (RIP_AUTH_SIMPLE == rip_intf_array[device_index]->auth_type)
		    && (0 != strlen(rip_intf_array[device_index]->simple_key) ) )
	{	
		offset = sizeof(struct rip_pkt_head_) + sizeof(struct rip_simple_head_);
	}
	else
	{
		offset = sizeof(struct rip_pkt_head_);			
	}
	
	if(send_pkt_len > offset )
	{
		rip_last_item = (struct rip_route_item_ *)(rip_send_buffer + send_pkt_len - sizeof(struct rip_route_item_));
		if( (rip_last_item->network == rip_route_item->network)
		&& (rip_last_item->mask == rip_route_item->mask) )
		{
	            /*发送metric值最小的． 山西运城农行测试，fengsb将最大改为最小2005-10-03
	            这是针对多条路由非等价时而言的，若端口存在等价路由，但是又有中毒逆转的配置
	            就只能用下面被屏蔽掉的代码*/
	            if( htonl(rip_last_item->metric) > htonl(rip_route_item->metric) )
			{
				/*发送metric值最大的*/
				rip_last_item->metric = rip_route_item->metric;
			}
            
            		#if 0 /*Fengsb shield 2006-02-17*/
			if( rip_route_item_temp->metric < rip_route_item->metric )
			{
				/*发送metric值最大的*/
				rip_route_item_temp->metric = rip_route_item->metric;
			}
			#endif
            
			return RIP_FAIL;
		}
	}		
	
	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_get_route
函数功能: 根据网络信息获取对应的路由
输入参数: rip_route_item : 网络信息
          rip_tbl_info: RIP路由表
输出参数: 无
返回值:      返回查找到的路由
备注:
===========================================================*/
struct rip_route_ *rip_get_route( struct rip_route_item_ *rip_route_item, struct route_table *table  )
{
	struct route_node *rip_route_node;
	struct rip_route_ *rip_route_head;
	struct prefix route_node_prefix;
	uint32 mask;	

	memset( (void *)&route_node_prefix, 0, sizeof(struct prefix) );
	
	if( 0 == rip_route_item->mask)
	{
		mask = rip_get_mask( rip_route_item->network );
	}
	else
	{
		mask = rip_route_item->mask;
	}

	route_node_prefix.family = AF_INET;     					   // IPv4
	route_node_prefix.safi = SAFI_UNICAST;  					   // 单播地址
	route_node_prefix.prefixlen = (u_char)mask_to_prefix( mask);   // 根据子网掩码获取前缀长度
	route_node_prefix.u.prefix4.s_addr = rip_route_item->network;  // 要查找的网络地址

	rip_route_node = route_node_lookup(table, &route_node_prefix ); // 在路由表中查找与构造的前缀匹配的路由节点
	if( rip_route_node != NULL )
	{
		route_unlock_node( rip_route_node );

		rip_route_head = rip_route_node->info;
		if( rip_route_head->equi_route_num >= 1 )  // 如果等价路由数量大于或等于 1，则返回该路由信息。
		{
			return rip_route_head;
		}
	}

	return NULL;
}

/*===========================================================
函数名: rip_is_neigh_addr
函数功能: 判断目的地址是否是neighbor地址
输入参数: dest_addr : 目的地址
输出参数: 无
返回值:      目的地址是neighbor地址,返回TRUE
          目的地址不是neighbor地址,返回FALSE
备注:
===========================================================*/
BOOL rip_is_neigh_addr(struct rip_process_info_ *pprocess, uint32 dest_addr )
{
	struct rip_neigh_list_ *rip_neigh_temp;

	for( rip_neigh_temp = pprocess->neigh_list.forw; rip_neigh_temp != &(pprocess->neigh_list); rip_neigh_temp = rip_neigh_temp->forw )
	{
		if( rip_neigh_temp->neigh_addr == dest_addr )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*===========================================================
函数名: rip_version_check
函数功能: 对接收到的报文的版本进行判断
输入参数: device_index : 接收报文的端口
          src_addr : 报文源地址
          
输出参数: 无
返回值:      通过对版本的判断,返回RIP_SUCCESS
          没有通过对版本的 判断,返回RIP_FAIL
备注:
===========================================================*/
int rip_version_check( uint32 device_index, uint32 src_addr )
{
	int recv_version;
	struct rip_pkt_head_ *rip_pkt_head;

	rip_pkt_head = (struct rip_pkt_head_ *)rip_recv_buffer;
	
	recv_version = rip_get_recv_version( device_index );

	switch( recv_version )
	{
		case RIP_RECV_VERSION_1:
			if( RIP_VERSION_1 != rip_pkt_head->version )
			{
				rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Illegal version).\n", rip_pkt_head->version, ip_ntoa(src_addr) );
				return RIP_FAIL;
			}
			break;
		case RIP_RECV_VERSION_2:
			if( RIP_VERSION_2 != rip_pkt_head->version )
			{
				rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Illegal version).\n", rip_pkt_head->version, ip_ntoa(src_addr) );
				return RIP_FAIL;
			}
			break;
		case RIP_RECV_VERSION_1_2:
		case RIP_RECV_VERSION_DEF:
			if( (RIP_VERSION_1 != rip_pkt_head->version) && (RIP_VERSION_2 != rip_pkt_head->version) )
			{
				rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Illegal version).\n", rip_pkt_head->version, ip_ntoa(src_addr) );
				return RIP_FAIL;
			}
			break;
		default:
			return RIP_FAIL;
	}

	return RIP_SUCCESS;
}


/*******************************************************************************
 *
 * FUNCTION	:	rip_neighb_auth_seq_check
 *
 * PARAMS		:	neighb		- 
 *					sequence	- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.03.02 11:30:56
 *
*******************************************************************************/
int rip_intf_auth_seq_check(uint32 device_index,uint32 sequence)
{
	
	if( rip_intf_array[device_index]->sequence <= sequence)
	{
		 rip_intf_array[device_index]->sequence = sequence;
		return RIP_SUCCESS;
	}
	else
	{
		 rip_intf_array[device_index]->sequence = sequence;
		return RIP_FAIL;
	}
}

/*===========================================================
函数名: rip_auth_check
函数功能: 对接收到的报文进行认证检测
输入参数: device_index : 接收报文的端口
          src_addr : 报文源地址
输出参数: 无
返回值:   认证通过,返回RIP_SUCCESS
       认证失败,返回RIP_FAIL
备注:
===========================================================*/
int rip_auth_check( uint32 device_index, uint32 src_addr )
{
	struct rip_pkt_head_ *rip_pkt_head;
	struct rip_route_item_ *rip_route_item;
	struct rip_simple_head_ *rip_simple_head;
	struct rip_md5_head_ *rip_md5_head;
	struct rip_md5_tail_ *rip_md5_tail;
	int ret;
	int activekey =0;
	uint32 packet_len =0;
	char password_md5[16];
	char password_sha1[20];

	struct dynamic_key_list_ *key;
	struct rip_authen_head_ *authen_head;
	struct rip_authen_tail_ *authen_tail;

	rip_pkt_head = (struct rip_pkt_head_ *)rip_recv_buffer;
	rip_route_item = (struct rip_route_item_ *)(rip_recv_buffer + sizeof(struct rip_pkt_head_) );

	if( htons(RIP_AUTH_AFI) != rip_route_item->afi )
	{
		/*接收到非认证报文*/
		// 但是端口开启了认证：
		// 1.如果是明文认证，并且simple_key不为NULL且未开启邻居动态认证
		// 2.如果是md5认证，并且md5_key不为NULL且未开启邻居动态认证
		// 3.如果是邻居间动态认证，并且开启了邻居动态认证
		if( ((rip_intf_array[device_index]->auth_type == RIP_AUTH_SIMPLE)
					&& (0 != strlen(rip_intf_array[device_index]->simple_key))&&(!rip_intf_array[device_index]->dynamicflag) )
				|| ( (rip_intf_array[device_index]->auth_type == RIP_AUTH_MD5)
					&& (0 != strlen(rip_intf_array[device_index]->md5_key))&&(!rip_intf_array[device_index]->dynamicflag))
					|| ( (rip_intf_array[device_index]->auth_type == RIP_AUTH_DYNAMIC) && rip_intf_array[device_index]->dynamicflag))
		{
			if(rip_intf_array[device_index]->dynamicflag)
			{
				struct dynamic_key_list_ *key=rip_intf_array[device_index]->key_list.forw;
				while(key!=&(rip_intf_array[device_index]->key_list))
				{
					if(key->key_state==RIP_NEIGHBOR_KEY_ACTIVE || key->key_state==RIP_NEIGHBOR_KEY_EXTENSIONUSE)
						break;
					key=key->forw;
				}
				if(key==&(rip_intf_array[device_index]->key_list))return RIP_SUCCESS;
			}
			
			if(RIP_VERSION_1 == rip_pkt_head->version)
			{
				ret = rip_get_recv_version(device_index);
				if (RIP_RECV_VERSION_1 == ret) return RIP_SUCCESS;
			}
			
			/*本地支持认证,接收到非认证报文,Ignored*/
			rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
			return RIP_FAIL;
		}
	}
	else
	{
		/*接收到认证报文*/
		if(RIP_VERSION_2 == rip_pkt_head->version)
		{
			/*认证类型不一致*/
			if( rip_intf_array[device_index]->auth_type != htons(rip_route_item->route_tag) )
			{
				rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
				return RIP_FAIL;
			}
			else
			{		
				/*认证类型一致,但密钥不一致*/
				if( RIP_AUTH_SIMPLE == rip_intf_array[device_index]->auth_type && (!rip_intf_array[device_index]->dynamicflag) )
				{
					rip_simple_head = (struct rip_simple_head_ *)( rip_recv_buffer +sizeof(struct rip_pkt_head_) );
					if( 0 != memcmp(rip_intf_array[device_index]->simple_key, rip_simple_head->password, 16 ) )
					{
						rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
						return RIP_FAIL;
					}					
				}
				else if( RIP_AUTH_MD5 == rip_intf_array[device_index]->auth_type &&(!rip_intf_array[device_index]->dynamicflag)  )
				{
					rip_md5_head = (struct rip_md5_head_ *)( rip_recv_buffer+sizeof(struct rip_pkt_head_) );
					rip_md5_tail = (struct rip_md5_tail_ *)( rip_recv_buffer + htons(rip_md5_head->packet_len) );

					/*check the time sequence */
					ret = rip_auth_seq_check(device_index , src_addr, htonl(rip_md5_head->sequence_number) );
                    if(RIP_SUCCESS != ret)
                    {
                        /*debug ip rip protocol*/
						rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
						return RIP_FAIL;
                    }
					
					packet_len = htons(rip_md5_head->packet_len) + rip_md5_head->authen_data_len;

					memcpy( password_md5, rip_md5_tail->password, 16 );
					
					memcpy( rip_md5_tail->password, rip_intf_array[device_index]->md5_key, 16);   /*fengsb add: it's no use 2006-01-25*/
					
					rt_md5_cksum( (byte *)rip_recv_buffer, packet_len, packet_len, rip_md5_tail->password, (uint32 *)0 );
					if( (memcmp(rip_md5_tail->password, password_md5, 16) != 0)
						|| (rip_intf_array[device_index]->md5_keyid != rip_md5_head->md5_keyid))
					{
						/*debug ip rip protocol*/
						rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
						return RIP_FAIL;
					}
				}
				else if( RIP_AUTH_DYNAMIC== rip_intf_array[device_index]->auth_type && rip_intf_array[device_index]->dynamicflag )
				{
					key =  rip_intf_array[device_index]->key_list.forw;
					while(key != & rip_intf_array[device_index]->key_list)
					{
						if(key->key_state == RIP_NEIGHBOR_KEY_ACTIVE || key->key_state == RIP_NEIGHBOR_KEY_EXTENSIONUSE)
						{
							activekey=1;
							break;
						}
						key =key->forw;
					}
					
					if( !activekey)
					{
						rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (dynamic authentication failed,no active key(%d)!)\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
						return RIP_FAIL;
					}
					authen_head = (struct rip_authen_head_ *)( rip_recv_buffer+sizeof(struct rip_pkt_head_));
					authen_tail = (struct rip_authen_tail_ *)( rip_recv_buffer + htons(authen_head->packet_len) );

					ret = rip_intf_auth_seq_check( device_index, htonl(authen_head->sequence_number) );
					if(RIP_SUCCESS != ret)
       				{
                 		/*debug ip rip protocol*/
						rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (dynamic authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
						return RIP_FAIL;
     				}

					packet_len = htons(authen_head->packet_len) + authen_head->authen_data_len;
									
					key =rip_intf_array[device_index]->key_list.forw;
					while(key != &rip_intf_array[device_index]->key_list)
					{
						if(key->key_id == authen_head->keyid)
							break;
						key =key->forw;
					}

					if(key == &rip_intf_array[device_index]->key_list)
       				{
                 		/*debug ip rip protocol*/
						rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (dynamic authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
						return RIP_FAIL;
     				}
					
					if(key->key_state != RIP_NEIGHBOR_KEY_ACTIVE && key->key_state != RIP_NEIGHBOR_KEY_EXTENSIONUSE)
					{
                 		/*debug ip rip protocol*/
						rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (dynamic authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
						return RIP_FAIL;
     				}

					
					if(key->algorithms ==RIP_NEIGHB_MD5)
					{
						memcpy( password_md5, authen_tail->key.md5,16);
						memcpy( authen_tail->key.md5, key->key, 16);
						rt_md5_cksum( (byte *)rip_recv_buffer, packet_len, packet_len, authen_tail->key.md5, (uint32 *)0 );
						if( (memcmp(authen_tail->key.md5, password_md5, 16) != 0))
						{
							/*debug ip rip protocol*/
							rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (dynamic authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
							return RIP_FAIL;
						}
					}
					else if(key->algorithms == RIP_NEIGHB_SHA1)
					{
						memcpy( password_sha1, authen_tail->key.sha1,20);
						memcpy( authen_tail->key.sha1, key->key, 20);
						rip_sha1_cksum( (byte *)rip_recv_buffer, packet_len, packet_len, authen_tail->key.sha1, (uint32 *)0 );
						if( (memcmp(authen_tail->key.sha1, password_sha1, 20) != 0))
						{
							/*debug ip rip protocol*/
							rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (dynamic authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
							return RIP_FAIL;
						}
					}
					else return RIP_FAIL;
				}
			}
		}
		else
		{
			/*非版本2的认证报文*/
			rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Illegal packet%d)\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
			return RIP_FAIL;
		}
	}

	return RIP_SUCCESS;
}


/*===========================================================
函数名: rip_packet_check
函数功能: 对接收到的报文的合法性进行检测
输入参数: device_index : 接收报文的端口
          src_socket : 报文源socket
          
输出参数: 无
返回值:      认证通过,返回RIP_SUCCESS
          认证失败,返回RIP_FAIL
备注:
===========================================================*/
int rip_packet_check( uint32 device_index, struct soaddr_in *src_socket )
{
	int ret;
	struct rip_pkt_head_ *rip_pkt_head;
	uint32 natural_intf_network;
	uint32 natural_intf_mask;
	
	// 获取指定端口的RIP的全局信息
	struct rip_process_info_ *pprocess = rip_intf_array[device_index]->pprocess;

	rip_pkt_head = (struct rip_pkt_head_ *)rip_recv_buffer;

	/*报文类型检测*/
	if( (RIP_REQUEST != rip_pkt_head->cmd) && (RIP_RESPONSE != rip_pkt_head->cmd) )
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Illegal packet type)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr) );
		return RIP_FAIL;
	}

	/* 报文版本检测*/
	if( (RIP_VERSION_1 != rip_pkt_head->version) && (RIP_VERSION_2 != rip_pkt_head->version) )
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (illegal version)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr) );
		return RIP_FAIL;
	}

	/*零域检测*/
	if( 0 != rip_pkt_head->zero )
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Illegal packet zero field)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr) );
		return RIP_FAIL;
	}

	/*报文源端口检测:520*/
	if( (RIP_RESPONSE == rip_pkt_head->cmd) && (htons(RIP_PORT) != src_socket->sin_port) )
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Illegal source port %d)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr), htons(src_socket->sin_port) );
		return RIP_FAIL;
	}

	/*报文源地址检测:报文源非本地地址*/
	if( rt_is_local_addr_vrf(src_socket->sin_addr.s_addr, rip_intf_array[device_index]->vrf_id, TRUE) )
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Source address is local address)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr) );
		return RIP_FAIL;
	}

	/*报文源地址检测:报文源地址与接收端口应是同一网段
	   若端口是unnumbered地址,则不进行源地址验证*/
	if( (BIT_TEST(pprocess->flags ,RIP_SRC_CHECK_ENABLE))
		&& (RIP_UNNUMBERED_ADDRESS != rip_intf_array[device_index]->addr_type) )
	{
		/*非同一网段*/
		if( (src_socket->sin_addr.s_addr & rip_intf_array[device_index]->mask ) !=  (rip_intf_array[device_index]->address &  rip_intf_array[device_index]->mask) )
		{
			/*点对点端口*/
			if( rip_intf_array[device_index]->encap_type == INTERFACE_DEVICE_FLAGS_NET_POINTTOPOINT )
			{
				/*对点对点端口的特殊处理*/
				natural_intf_network = rip_get_natural_network( rip_intf_array[device_index]->address );
				natural_intf_mask = rip_get_natural_mask( rip_intf_array[device_index]->address );
				if((src_socket->sin_addr.s_addr & natural_intf_mask) !=  (natural_intf_network & natural_intf_mask) )
				{
					rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Source validate failed)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr) );
					return RIP_FAIL;
				}
			}
			else
			{
				/*非点对点端口*/
				rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Source validate failed)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr) );
				return RIP_FAIL;
			}
		}
	}

	/*检测接收到的报文版本是否能被本地接受*/
	ret = rip_version_check( device_index, src_socket->sin_addr.s_addr );
	if( RIP_SUCCESS != ret )
	{
		return RIP_FAIL;
	}

	/*认证检测*/
	ret = rip_auth_check( device_index, src_socket->sin_addr.s_addr );
	if( RIP_SUCCESS != ret )
	{
		return RIP_FAIL; 
	}
	else if(rip_intf_array[device_index]->auth_commit_time)
	{
		struct rip_peer_list_ *peer=pprocess->peer_list.forw;
		int cnt=0;
		while(peer!=&(pprocess->peer_list))
		{
			if(peer->peer_addr==src_socket->sin_addr.s_addr)
				peer->auth_state=0;
			else if((peer->peer_intf==device_index)&&peer->auth_state)cnt++;
			peer=peer->forw;
		}
		if(cnt == 0)
		{
			rip_intf_array[device_index]->auth_commit_time=0;
			sys_stop_timer(rip_intf_array[device_index]->auth_commit_timer_id);
		}
	}
	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_auth_seq_check
函数功能: 对认证报文的sequence number进行检测
输入参数: src_address : 报文源地址
          seq_number : 认证报文中的源sequence number
输出参数: 无
返回值:      认证通过,返回RIP_SUCCESS
          认证失败,返回RIP_FAIL
备注:
===========================================================*/
int rip_auth_seq_check(uint32 device_index, uint32 src_address, uint32 seq_number )
{
	struct rip_md5_record_ *rip_md5_record_temp;
	struct rip_md5_record_ *rip_md5_record_new;
	
	struct rip_process_info_ *pprocess = rip_intf_array[device_index]->pprocess;
	for( rip_md5_record_temp = pprocess->md5_record.forw; rip_md5_record_temp != &(pprocess->md5_record); rip_md5_record_temp = rip_md5_record_temp->forw )
	{
		if( rip_md5_record_temp->src_addr == src_address )
		{
			
			if( seq_number >= rip_md5_record_temp->sequence_number )
			{
				rip_md5_record_temp->sequence_number = seq_number;
				return RIP_SUCCESS;
			}
			else
			{
				rip_md5_record_temp->sequence_number = seq_number;
				return RIP_FAIL;
			}
		}
	}

	rip_md5_record_new = (struct rip_md5_record_ *)rip_mem_malloc( sizeof(struct rip_md5_record_), RIP_MD5_RECORD_TYPE );
	if( rip_md5_record_new == NULL )
	{
		fprintf(stderr, "RIP: Malloc memory failed. (%s, %d)\n", __FILE__, __LINE__ );
		return RIP_MEM_FAIL;
	}
	memset( (void *)rip_md5_record_new, 0, sizeof(struct rip_md5_record_) );

	rip_md5_record_new->src_addr = src_address;
	rip_md5_record_new->sequence_number = seq_number;

	INSQUE( rip_md5_record_new, pprocess->md5_record.back );

	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_item_recv_check
函数功能: 对接收到的路由条目进行处理
输入参数: device_index: 接收路由端口
		  src_addr : 路由源地址
          rip_route_item : 接收到的路由条目
输出参数: 无
返回值:      路由条目通过检测,返回RIP_SUCCESS
          路由条目不通过检测,返回RIP_FAIL
备注:
===========================================================*/
int rip_item_recv_check(uint32 device_index, uint32 src_addr, struct rip_route_item_ *rip_route_item, struct route_node *rip_route_node)
{
	struct rip_pkt_head_ *rip_pkt_head;	
	struct rip_route_    *rip_route;
	struct rip_route_list_ *rip_route_list;//add 2019.6 leibang
	rt_entry *rt;
	uint32 network;
	uint32 natural_route_network;
	uint32 natural_intf_network;
	char string[100];
	uint32 device_index_temp;
	uint32 item_mask;
	int ret, count;
	struct rip_process_info_ *pprocess = rip_intf_array[device_index]->pprocess;
#if 0
	struct vrf_table_list_ *vrf_table = NULL;
#endif

	rip_pkt_head = (struct rip_pkt_head_ *)rip_recv_buffer;

	/*---------------------------------------------------
	item的address family identifier的检测:
	request: 0, 2, 0xffff
	response: 2, 0xffff
	---------------------------------------------------*/
	if( ((RIP_REQUEST == rip_pkt_head->cmd) && (rip_route_item->afi != 0) && (rip_route_item->afi != htons(AF_INET)) && (rip_route_item->afi != htons(RIP_AUTH_AFI)))
		|| ((RIP_RESPONSE == rip_pkt_head->cmd) && (rip_route_item->afi != htons(AF_INET)) && (rip_route_item->afi != htons(RIP_AUTH_AFI))) )
	{
		// Keep-Alive Response报文
		if((rip_route_item->afi==0)&&(rip_route_item->network==0)&&(rip_route_item->mask==0))
		{
			rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored peer keep-alive response packet\n");
			return RIP_FAIL;
		}
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Illegal address-family-identifier)\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
		return RIP_FAIL;
	}

	/*---------------------------------------------------
	版本1的零域检测:
	route-tag = 0
	mask = 0
	next-hop = 0
	---------------------------------------------------*/
	if( RIP_VERSION_1 == rip_pkt_head->version )
	{
		if( BIT_TEST(pprocess->flags , RIP_ZERO_DOMAIN_CHECK_ENABLE)/*在设置了零域不检查的时候，不做检查,dangzhw2010325*/
			&& ( (0 != rip_route_item->route_tag)
			|| (0 != rip_route_item->mask)
			|| (0 != rip_route_item->next_hop) ))
		{
			rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Illegal zero field)\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
			return RIP_FAIL;
		}
	}		
	
	/*---------------------------------------------------
	Item的metric检测:
	0 <= metric <= 16
	---------------------------------------------------*/
	if( htonl(rip_route_item->metric) > RIP_MAX_METRIC) 
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Illegal metric)\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
		return RIP_FAIL;
	}

	/*---------------------------------------------------
	Item的filter in检测:
	filiter * in access-list acclist-name
	---------------------------------------------------*/
	ret = rip_filter_in_check( device_index, src_addr, rip_route_item );
	if( RIP_SUCCESS != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Filtered).\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
		return ret;
	}
	
	/*---------------------------------------------------
	Item的network检测:
	不能为组播地址
	不能为127地址
	第一位不能为0(缺省路由除外)
	---------------------------------------------------*/
	if( ( *( (uint8 *)(&(rip_route_item->network)) ) >= 224) 
		|| ( 127 == *(  (uint8 *)(&(rip_route_item->network)) ) )
		|| ((0 == *(  (uint8 *)(&(rip_route_item->network)))) && (rip_route_item->network != 0)) )
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Illegal network).\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
		return RIP_FAIL;
	}
	
	/*---------------------------------------------------
	Item的network检测:
	不能为广播地址
	---------------------------------------------------*/
	network = htonl(rip_route_item->network);
	if( 0 != network )
	{
		item_mask = (RIP_VERSION_1 == rip_pkt_head->version) ? rip_get_natural_mask(rip_route_item->network): rip_route_item->mask;
		item_mask = ~(htonl(item_mask));		
		if(((item_mask!=0)&&((network & item_mask) == item_mask))
			||((item_mask==0)&&(network==0xfffffffful)))
		{
			rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Illegal network).\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
			return RIP_FAIL;
		}
	}
	else if(rip_route_item->mask != 0) // 缺省路由
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Illegal mask).\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
		return RIP_FAIL;
	}

	/*---------------------------------------------------
	Item的next-hop检测:
	next-hop不能为接收端口地址
	---------------------------------------------------*/
	if( rip_route_item->next_hop != 0 )
	{
		if( rip_route_item->next_hop == rip_intf_array[device_index]->address )
		{
			if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_PROTO_RECV )
			{
				memset( (void *)string, 0, 100 );
				sprintf(string, "\tIgnored route item %s/%d", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
				sprintf(string, "%s via %s metric %lu", string, ip_ntoa(rip_route_item->next_hop), htonl(rip_route_item->metric ));
				rip_debug(RIP_DEBUG_IP_RIP_PROTO_RECV, "%s(Illegal next-hop)\n", string );
			}

			return RIP_FAIL;
		}
		else if( (rip_route_item->next_hop & rip_intf_array[device_index]->mask) != (rip_intf_array[device_index]->address & rip_intf_array[device_index]->mask) )
		{
			// 下一跳地址必须与设备接口在同一子网内
			rip_route_item->next_hop = 0;
		}
	}
	
	/*---------------------------------------------------
	版本1 ,若接收到的路由条目与本地地址
	具有相同主网和不同子网掩码时,拒绝接收
	Fengsb modified according to cisco2691 2006-01-26
	---------------------------------------------------*/
	if( RIP_VERSION_1 == rip_pkt_head->version )
	{
		item_mask = rip_get_mask( rip_route_item->network );
		
		natural_route_network = rip_get_natural_network( rip_route_item->network );
	       natural_intf_network = rip_get_natural_network (rip_intf_array[device_index]->address);

		/* fang qi modify this on 2007-03-02  */
		if(natural_route_network == natural_intf_network )
		{
			/*与接口主网络号相同，采用本地掩码作为item的掩码，
			如果item掩码大于本地掩码，则采用主机路由掩码作为item的掩码*/

			if( item_mask <= rip_intf_array[device_index]->mask)
				rip_route_item->mask= rip_intf_array[device_index]->mask;
		       else rip_route_item->mask = 0xffffffff;    /*主机路由*/
		}
		else
		{
			rip_route_item->mask = rip_get_natural_mask( rip_route_item->network );
			for( device_index_temp = 0; device_index_temp <= INTERFACE_DEVICE_MAX_NUMBER; device_index_temp++ )
			{
				if( (rip_intf_array[device_index_temp] == NULL)
					|| ( !BIT_TEST( rip_intf_array[device_index_temp]->state ,RIP_INTF_LINK_UP)) 
					|| ( 0 == rip_intf_array[device_index_temp]->address)
					|| (rip_intf_array[device_index_temp]->vrf_id != rip_intf_array[device_index]->vrf_id))
				{
					continue;
				}

				natural_intf_network = rip_get_natural_network( rip_intf_array[device_index_temp]->address );
				if((natural_route_network == natural_intf_network) 
					&& ( item_mask <  rip_intf_array[device_index_temp]->mask) )
				{
					rip_debug(RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: Ignored route item %s/%d (Auto-summary reason).\n", 
							ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
					return RIP_FAIL;
				}
			}
		}
	}

	/*---------------------------------------------------
	版本2: 对接收到的网络号和网络掩码不一致时的处理
	---------------------------------------------------*/
	if( RIP_VERSION_2 == rip_pkt_head->version )
	{
		if( (rip_route_item->network & rip_route_item->mask) != rip_route_item->network )
		{
		#if 0
			item_mask = rip_get_mask( rip_route_item->network );

			if( htonl(item_mask) > htonl(rip_route_item->mask) )
			{
				rip_route_item->mask = 0xffffffff;
			}
			/*其他情况,以实际接收到的掩码为准*/
		#else
			rip_debug(RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: illegal route item %s/%d ", 
							ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask));
			rip_route_item->network &= rip_route_item->mask;
			rip_debug(RIP_DEBUG_IP_RIP_PROTO_RECV, "     has been converted to %s/%d\n", 
							ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask));
		#endif
		}
	}

    /*---------------------------------------------------
	接收到的路由条目item不能为本地直连路由
	---------------------------------------------------*/
	if(TRUE == rip_is_connected_network( rip_intf_array[device_index]->vrf_id, rip_route_item->network, rip_route_item->mask) )
	{
		rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Network is local).\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
		return RIP_FAIL;
	}
    
	/*---------------------------------------------------
	接收到的路由已处于holddown状态:
	若对应的路由在主路由表中存在,
	或接收到的路由条目的metric为16
	Ignored
	---------------------------------------------------*/
	for (rip_route_list = pprocess->holddown_list.forw; rip_route_list != &(pprocess->holddown_list); rip_route_list = rip_route_list->forw)
	{
		if ( (rip_route_list->rip_route->route_node->p.u.prefix4.s_addr == rip_route_item->network)
			&& (rip_route_list->rip_route->route_node->p.prefixlen == mask_to_prefix(rip_route_item->mask))
			/*&& (rip_route_list->rip_route->vrf_id == rip_intf_array[device_index]->vrf_id)*/ )
		{
			/*	
	if(rip_route_node && rip_route_node->info)
	{
		rip_route = rip_route_node->info;
		ret = rip_route->equi_route_num;
		for(count = 1; count <= ret; rip_route = rip_route->forw, count++)
		{
			if ((rip_route->route_type != RIP_NBR_ROUTE) || (rip_route->metric < RIP_MAX_METRIC))
				continue;
	*/

			
			/*----------------------------------------------------------
			端口down后，通过此端口的学习的路由从
			主路由表中删除，本地路由表超时后进入holddown,
			此时holddown状态，接收路由更新;
			
			而对于超时进入holddown的路由，不从主路由表
			删除，不接受路由更新
			------------------------------------------------------------*/
			if (htonl(rip_route_item->metric) == RIP_MAX_METRIC)
			{
				rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored unreachable route item %s/%d(Holddown state).\n", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
				return RIP_FAIL;
			}
			
			/*检查在主路由表中是否存在*/
#if 0/*MULTICORE*/
			vrf_table = vrf_table_enter(rip_intf_array[device_index]->vrf_id,RT_RD_LOCK);
			if(vrf_table == NULL)
			{
				return RIP_FAIL;
			}
#endif
			rt = rt_locate( pprocess->vrf_id,
						RTS_ELIGIBLE,
						rip_route_item->network,
						rip_route_item->mask,
						RTPROTO_RIP,
						pprocess->process_id);
			if(NULL != rt)
			{
#if 0/*MULTICORE*/
			vrf_table_leave(vrf_table,RT_RD_LOCK);
#endif
				rip_debug(RIP_DEBUG_IP_RIP_PROTO_RECV, "\tIgnored route item %s/%d(Holddown state, but exist in route table).\n", 
						ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask));
				return RIP_FAIL;
			}
#if 0/*MULTICORE*/
			vrf_table_leave(vrf_table,RT_RD_LOCK);
#endif
			break;
		}
	}

	/*---------------------------------------------------
	对接收到的路由条目进行offset in信息控制
	---------------------------------------------------*/
	rip_offset_in_check( device_index, rip_route_item );
	
	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_is_connected_network
函数功能: 判断路由条目是否是本地路由
输入参数: vrf_id: VRF ID
			    dest : 路由条目地址前缀
                       mask : 路由条目地址前缀掩码
输出参数: 无
返回值:      是本地直连路由,返回TRUE
                       不是本地直连路由,返回FALSE
备注:Fengsb modified: if the mask length is not match the local mask length
we think the announced route from neighbor is no the local connect route
and can install in local rip database and main route table 
do it as cisco2691 rip policy 2006-01-26
===========================================================*/
BOOL rip_is_connected_network( uint32 vrf_id, uint32 dest, uint32 mask )
{	
	struct _rt_entry *rt;
#if 0/*MULTICORE*/
	struct vrf_table_list_ *vrf_table= NULL;

	vrf_table = vrf_table_enter(vrf_id,RT_RD_LOCK);
	if(vrf_table == NULL)
	{
		return FALSE;
	}
#endif
	rt = rt_locate(vrf_id, RTS_ACTIVE, dest, mask, RTPROTO_DIRECT, 0);
	if (rt)
	{
#if 0/*MULTICORE*/
		vrf_table_leave(vrf_table,RT_RD_LOCK);
#endif
		return TRUE;
	}
	else
	{
#if 0/*MULTICORE*/
		vrf_table_leave(vrf_table,RT_RD_LOCK);
#endif
		return FALSE;
	}
	
#if 0
	uint32 device_index;
	uint32 network;
	network = dest & mask;
	for( device_index = 0; device_index <= INTERFACE_DEVICE_MAX_NUMBER; device_index++ )
	{
		if( (rip_intf_array[device_index] == NULL)
			|| ( RIP_LINK_UP != rip_intf_array[device_index]->link_state) 
			|| ( NULL == rip_intf_array[device_index]->address)
			|| (rip_intf_array[device_index]->vrf_id != vrf_id))
		{
			continue;
		}

        /*Fengsb shield this code 2006-01-16*/
		if( (rip_intf_array[device_index]->address & rip_intf_array[device_index]->mask)
			== (network & rip_intf_array[device_index]->mask) )
		{
			return TRUE;
		}
        /*Fengsb change the connect route judgement policy, do it as cisco 2691, 2006-01-26*/
        if( (rip_intf_array[device_index]->address & rip_intf_array[device_index]->mask)
			== network)
		{
			return TRUE;
		}
	}
	return FALSE;
#endif
}

/*===========================================================
函数名: rip_get_relevant_route
函数功能: 获取路由条目对应的路由
输入参数: device_index: 接收路由端口
			    src_addr : 路由源地址
                       rip_route_item : 接收到的路由条目
输出参数: 无
返回值:      路由条目有对应路由,返回该路由
                       否则,返回NULL
Author:            fengsb
备注:   this function has been modified to improve the performance by sunhq.
===========================================================*/
struct rip_route_ *rip_get_relevant_route( uint32 device_index, uint32 src_addr, rip_route_t* rip_route )
{
    rip_route_t *route_tmp = rip_route;
    
    if(route_tmp->route_type == RIP_NBR_ROUTE && route_tmp->gw_addr == src_addr && route_tmp->gw_index == device_index)
        return rip_route;

    for(route_tmp = rip_route->forw; route_tmp != rip_route; route_tmp = route_tmp->forw)
        if(route_tmp->route_type == RIP_NBR_ROUTE && route_tmp->gw_addr == src_addr && route_tmp->gw_index == device_index)
            return route_tmp;

	return NULL;
}

#ifndef _SHA1_H_
#define _SHA1_H_

#ifndef _SHA_enum_
#define _SHA_enum_
enum
{
    shaSuccess = 0,
    shaNull,            /* Null pointer parameter */
    shaInputTooLong,    /* input data too long */
    shaStateError       /* called Input after Result */
};
#endif
#define SHA1HashSize 20

/*
 *  This structure will hold context information for the SHA-1
 *  hashing operation
 */
typedef struct SHA1Context
{
    uint32 Intermediate_Hash[SHA1HashSize/4]; /* Message Digest  */

    uint32 Length_Low;            /* Message length in bits      */
    uint32 Length_High;           /* Message length in bits      */

                               /* Index into message block array   */
    uint16 Message_Block_Index;
    uint8 Message_Block[64];      /* 512-bit message blocks      */

    int Computed;               /* Is the digest computed?         */
    int Corrupted;             /* Is the message digest corrupted? */
} SHA1Context;

/*
 *  Function Prototypes
 */

#endif

/*
 *  sha1.c
 *
 *  Description:
 *      This file implements the Secure Hashing Algorithm 1 as
 *      defined in FIPS PUB 180-1 published April 17, 1995.
 *
 *      The SHA-1, produces a 160-bit message digest for a given
 *      data stream.  It should take about 2**n steps to find a
 *      message with the same digest as a given message and
 *      2**(n/2) to find any two messages with the same digest,
 *      when n is the digest size in bits.  Therefore, this
 *      algorithm can serve as a means of providing a
 *      "fingerprint" for a message.
 *
 *  Portability Issues:
 *      SHA-1 is defined in terms of 32-bit "words".  This code
 *      uses <stdint.h> (included via "sha1.h" to define 32 and 8
 *      bit unsigned integer types.  If your C compiler does not
 *      support 32 bit unsigned integers, this code is not
 *      appropriate.
 *
 *  Caveats:
 *      SHA-1 is designed to work with messages less than 2^64 bits
 *      long.  Although SHA-1 allows a message digest to be generated
 *      for messages of any number of bits less than 2^64, this
 *      implementation only works with messages with a length that is
 *      a multiple of the size of an 8-bit character.
 *
 */


/*
 *  Define the SHA1 circular left shift macro
 */
#define SHA1CircularShift(bits,word) \
                (((word) << (bits)) | ((word) >> (32-(bits))))

/* Local Function Prototyptes */
void SHA1PadMessage(SHA1Context *);
void SHA1ProcessMessageBlock(SHA1Context *);

/*
 *  SHA1Reset
 *
 *  Description:
 *      This function will initialize the SHA1Context in preparation
 *      for computing a new SHA1 message digest.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to reset.
 *
 *  Returns:
 *      sha Error Code.
 *
 */
int sha1init(SHA1Context *context)
{
    if (!context)
    {
        return shaNull;
    }

    context->Length_Low = 0;
    context->Length_High = 0;
    context->Message_Block_Index = 0;

    context->Intermediate_Hash[0]  = 0x67452301;
    context->Intermediate_Hash[1]  = 0xEFCDAB89;
    context->Intermediate_Hash[2]  = 0x98BADCFE;
    context->Intermediate_Hash[3]  = 0x10325476;
    context->Intermediate_Hash[4]  = 0xC3D2E1F0;

    context->Computed = 0;
    context->Corrupted = 0;

    return shaSuccess;
}

/*
 *  SHA1Result
 *
 *  Description:
 *      This function will return the 160-bit message digest into the
 *      Message_Digest array  provided by the caller.
 *      NOTE: The first octet of hash is stored in the 0th element,
 *            the last octet of hash in the 19th element.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to use to calculate the SHA-1 hash.
 *      Message_Digest: [out]
 *          Where the digest is returned.
 *
 *  Returns:
 *      sha Error Code.
 *
 */
int SHA1Result( SHA1Context *context,
                uint8 Message_Digest[SHA1HashSize])
{
    int i;

    if (!context || !Message_Digest)
    {
        return shaNull;
    }

    if (context->Corrupted)
    {
        return context->Corrupted;
    }

    if (!context->Computed)
    {
        SHA1PadMessage(context);
        for(i=0; i<64; ++i)
        {
            /* message may be sensitive, clear it out */
            context->Message_Block[i] = 0;
        }
        context->Length_Low = 0;    /* and clear length */
        context->Length_High = 0;
        context->Computed = 1;

    }

    for(i = 0; i < SHA1HashSize; ++i)
    {
        Message_Digest[i] = context->Intermediate_Hash[i>>2]
                            >> 8 * ( 3 - ( i & 0x03 ) );
    }

    return shaSuccess;
}

/*
 *  SHA1Input
 *
 *  Description:
 *      This function accepts an array of octets as the next portion
 *      of the message.
 *
 *  Parameters:
 *      context: [in/out]
 *          The SHA context to update
 *      message_array: [in]
 *          An array of characters representing the next portion of
 *          the message.
 *      length: [in]
 *          The length of the message in message_array
 *
 *  Returns:
 *      sha Error Code.
 *
 */
int SHA1Input(    SHA1Context    *context,
                  const uint8  *message_array,
                  unsigned       length)
{
    if (!length)
    {
        return shaSuccess;
    }

    if (!context || !message_array)
    {
        return shaNull;
    }

    if (context->Computed)
    {
        context->Corrupted = shaStateError;

        return shaStateError;
    }

    if (context->Corrupted)
    {
         return context->Corrupted;
    }
    while(length-- && !context->Corrupted)
    {
    context->Message_Block[context->Message_Block_Index++] =
                    (*message_array & 0xFF);

    context->Length_Low += 8;
    if (context->Length_Low == 0)
    {
        context->Length_High++;
        if (context->Length_High == 0)
        {
            /* Message is too long */
            context->Corrupted = 1;
        }
    }

    if (context->Message_Block_Index == 64)
    {
        SHA1ProcessMessageBlock(context);
    }

    message_array++;
    }

    return shaSuccess;
}

/*
 *  SHA1ProcessMessageBlock
 *
 *  Description:
 *      This function will process the next 512 bits of the message
 *      stored in the Message_Block array.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:

 *      Many of the variable names in this code, especially the
 *      single character names, were used because those were the
 *      names used in the publication.
 *
 *
 */
void SHA1ProcessMessageBlock(SHA1Context *context)
{
    const uint32 K[] = {       /* Constants defined in SHA-1   */
                            0x5A827999,
                            0x6ED9EBA1,
                            0x8F1BBCDC,
                            0xCA62C1D6
                            };
    int           t;                 /* Loop counter                */
    uint32      temp;              /* Temporary word value        */
    uint32      W[80];             /* Word sequence               */
    uint32      A, B, C, D, E;     /* Word buffers                */

    /*
     *  Initialize the first 16 words in the array W
     */
    for(t = 0; t < 16; t++)
    {
        W[t] = context->Message_Block[t * 4] << 24;
        W[t] |= context->Message_Block[t * 4 + 1] << 16;
        W[t] |= context->Message_Block[t * 4 + 2] << 8;
        W[t] |= context->Message_Block[t * 4 + 3];
    }

    for(t = 16; t < 80; t++)
    {
       W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }

    A = context->Intermediate_Hash[0];
    B = context->Intermediate_Hash[1];
    C = context->Intermediate_Hash[2];
    D = context->Intermediate_Hash[3];
    E = context->Intermediate_Hash[4];

    for(t = 0; t < 20; t++)
    {
        temp =  SHA1CircularShift(5,A) +
                ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);

        B = A;
        A = temp;
    }

    for(t = 20; t < 40; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 40; t < 60; t++)
    {
        temp = SHA1CircularShift(5,A) +
               ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 60; t < 80; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    context->Intermediate_Hash[0] += A;
    context->Intermediate_Hash[1] += B;
    context->Intermediate_Hash[2] += C;
    context->Intermediate_Hash[3] += D;
    context->Intermediate_Hash[4] += E;

    context->Message_Block_Index = 0;
}

/*
 *  SHA1PadMessage
 *

 *  Description:
 *      According to the standard, the message must be padded to an even
 *      512 bits.  The first padding bit must be a '1'.  The last 64
 *      bits represent the length of the original message.  All bits in
 *      between should be 0.  This function will pad the message
 *      according to those rules by filling the Message_Block array
 *      accordingly.  It will also call the ProcessMessageBlock function
 *      provided appropriately.  When it returns, it can be assumed that
 *      the message digest has been computed.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to pad
 *      ProcessMessageBlock: [in]
 *          The appropriate SHA*ProcessMessageBlock function
 *  Returns:
 *      Nothing.
 *
 */

void SHA1PadMessage(SHA1Context *context)
{
    /*
     *  Check to see if the current message block is too small to hold
     *  the initial padding bits and length.  If so, we will pad the
     *  block, process it, and then continue padding into a second
     *  block.
     */
    if (context->Message_Block_Index > 55)
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 64)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }

        SHA1ProcessMessageBlock(context);

        while(context->Message_Block_Index < 56)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    }
    else
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 56)
        {

            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    }

    /*
     *  Store the message length as the last 8 octets
     */
    context->Message_Block[56] = context->Length_High >> 24;
    context->Message_Block[57] = context->Length_High >> 16;
    context->Message_Block[58] = context->Length_High >> 8;
    context->Message_Block[59] = context->Length_High;
    context->Message_Block[60] = context->Length_Low >> 24;
    context->Message_Block[61] = context->Length_Low >> 16;
    context->Message_Block[62] = context->Length_Low >> 8;
    context->Message_Block[63] = context->Length_Low;

    SHA1ProcessMessageBlock(context);
}


/*
 *  sha1test.c
 *
 *  Description:
 *      This file will exercise the SHA-1 code performing the three
 *      tests documented in FIPS PUB 180-1 plus one which calls
 *      SHA1Input with an exact multiple of 512 bits, plus a few
 *      error test checks.
 *
 *  Portability Issues:
 *      None.
 *
 */

/*******************************************************************************
 *
 * FUNCTION	:	rip_sha1_cksum
 *
 * PARAMS		:	data		- 
 *					datalen		- 
 *					totallen	- 
 *					digest		- 
 *					init		- 
 *
 * RETURN		:	
 *
 * NOTE			:	为了保持和rt md5 chsum的一致性，保留了init和datalen两参数。
 					这个函数是对传递过来的整个数据进行加密，然后将
 					得出的摘要值替换掉数据的最后20位，即密钥位。
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.03.04 10:19:43
 *
*******************************************************************************/
void rip_sha1_cksum(void_t data, size_t datalen, size_t totallen, void_t digest, uint32 * init)			
{
	SHA1Context sha1;
    	int  ret;
    	char Message_Digest[20];

	ret =sha1init(&sha1);
	if(ret != 0)
		return;

	ret = SHA1Input(&sha1,data,totallen);
	if(ret != 0)
		return;

	ret = SHA1Result(&sha1, (uint8 *)Message_Digest);
       if(ret != 0)
		return;	

	memcpy((char *)digest,Message_Digest,20);
	return;
	   
}

