# BFD



```cmd
Switch_config_v10#bfd ?
  authentication-mode   -- BFD authentication mode
  demand                -- BFD demand mode
  echo                  -- BFD echo enable
  enable                -- BFD enable
  nexthop               -- BFD packet nexthop
  min_echo_rx_interval  -- BFD required min echo rx interval
  neighbor              -- BFD neighbor config
  renotify_interval     -- BFD renotify interval
```



## bfd-enable

> 说明一下为什么是 BFD与RIP建立的是单跳邻居关系？
>
> 当neighbor取消或变动时，bfd与rip的绑定关系需要调整？
>
> rip和bfd的优先级考虑



**若RIP与BFD绑定的端口地址发生改变**

```c
typedef enum {
    PROTOCOL_BFD = 1,  // 高优先级
    PROTOCOL_RIP = 2   // 低优先级
} ProtocolPriority;

1.RIP收到rounting模块的删除地址消息(MSG_ROUTING_DEL_PRIMARY)、再收到添加地址消息(MSG_ROUTING_ADD_PRIMARY)，此时，向邻居设备发送请求整个路由表消息(peer链表中存在新的peer节点）
2.BFD检测到链路down，触发RIP注册的回调函数：
	- 从peer_list找到与当前接口处于直连网段的peer节点，向其发送keepalive报文
		若链路此时为UP情况：peer的rip2PeerLastUpdate会更新，获取当前时间与rip2PeerLastUpdate的间隔时间interval，若小于peer_timeout，切换为RIP模式（降低优先级） rip_intf_array[device_index]->ProtocolPriority = PROTOCOL_RIP;
		否则，认为链路DOWN了：将端口设置为不再接收RIP组播报文、删除该端口学习到的路由以及重分发直连路由。
		
3.地址又改回先前的neigbor指定的地址，此时BFD检测到链路又UP了，触发RIP注册的回调函数：
  - 将其从RIP模式切换为BFD模式（升级优先级）
    rip_intf_array[device_index]->proto_priority = PROTOCOL_BFD;
```

#### rip_intf_

```c
/*RIP端口信息*/
typedef struct rip_intf_
{
	uint32 vrf_id;/*端口对应的VRF*/
	uint32 process_id;/*被使能的进程号*/
	uint32 device_index;	/* 接口ID*/
	struct rip_process_info_ *pprocess;

/*	int ref_num;/*该端口被RIP网络覆盖的次数*/

	uint32 encap_type; /* 接口封装类型*/
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

	/* Commented by dangzw in 2009.03.04 19:10:01 */
	uint8 dynamicflag;
	uint32 sequence;/*序列号*/
	uint32 key_start_timer_id;/*key开始生效定时器*/
	uint32 key_lift_timer_id;/*key有效时间长度定时器*/
	struct dynamic_key_list_  key_list;/*邻居的key链表*/
	struct dynamic_key_list_  key_timeout_list;/*邻居间超时的key链表*/
	
	uint8 bfd_enable_flag;   	/* 端口enable bfd，0:disable 1:enable */
    uint8 rip_bfd_sess_up_num;	/* rip端口bfd会话up数 */
    struct rip_bfd_peer_list_ bfd_peer_list;/* rip端口bfd邻居链表 */
}rip_intf_t;

typedef struct rip_bfd_peer_list_ {
    struct rip_bfd_peer_list_ *forw;
    struct rip_bfd_peer_list_ *back;
    uint32 bfd_peer_addr;
    uint8 bfd_peer_up;
}rip_bfd_peer_list_t;
```

### RIP模块

#### rip_intf_create_bfd_peer

```c
/* 创建新的bfd_peer_list */
int rip_intf_create_bfd_peer(uint32 device_index, uint32 peer_addr){
    struct rip_intf_ *pintf;
	struct rip_bfd_peer_list_ *bfd_peer;
    
	if((!device_index) || (device_index > INTERFACE_DEVICE_MAX_NUMBER)) 
        return RIP_FAIL;
   
    if((pintf = rip_intf_array[device_index]) == NULL)
        return RIP_FAIL;
    
    bfd_peer = pintf->bfd_peer_list.forw;
    while(bfd_peer != &pintf->bfd_peer_list){
        if(bfd_peer->bfd_peer_addr == peer_addr){
            break;
        }
        bfd_peer = bfd_peer->forw;
    }
        
    if(bfd_peer == &pintf->bfd_peer_list){
        bfd_peer = rip_mem_malloc(sizeof(struct rip_bfd_peer_list_), RIP_NEIGHB_PEER_TYPE);
    	if (bfd_peer == NULL)
    	{
       		return RIP_FAIL;
    	}
        INSQUE(bfd_peer, pintf->bfd_peer_list.back);
    }
    bfd_peer->bfd_peer_up=0;
    return RIP_SUCCESS;
}
```



#### rip_intf_bfd_register

```c
int rip_intf_bfd_register(uint32 device_index, uint32 peer_addr, uint8 state)
{
    struct rip_intf_ *pintf;
    struct rip_peer_list_ *peer;
    struct rip_process_info_ *pprocess;
    struct rip_bfd_peer_list_ *bfd_peer;
    
	int ret;
	rip_debug(RIP_DEBUG_IP_RIP_MSG, "rip_intf_bfd_register device_index=%d,state = %d\n", device_index,state);
    
	if((!device_index) || (device_index > INTERFACE_DEVICE_MAX_NUMBER)) 
        return RIP_FAIL;
   
    if((pintf = rip_intf_array[device_index]) == NULL)
        return RIP_FAIL;

    pprocess = pintf->pprocess;
    if(!pprocess){
        /*该端口还没有绑定RIP进程,不需处理*/
		return RIP_SUCCESS;
    }
	
    /* 注册指定peer的bfd会话 */
    if(peer_addr!=0){
        ret = bfd_proto_announce(pintf->address, peer_addr, 0, RTPROTO_RIP, pintf->process_id, !state, rip_intf_bfd_callback);
        if(ret < 0){
             rip_debug(RIP_DEBUG_IP_RIP_RETURN, "rip_intf_bfd_register fail!\n");
             return RIP_FAIL;
        }  
        
        ret = rip_intf_create_bfd_peer(pintf->device_index, peer_addr);
        if(ret < 0){
             rip_debug(RIP_DEBUG_IP_RIP_RETURN, "rip_intf_bfd_register fail!\n");
             return RIP_FAIL;
        }  
        
        return RIP_SUCCESS;
    }
    
    peer = pprocess->peer_list.forw;
    while(peer != &(pprocess->peer_list))
    {
        if(peer->peer_intf == device_index)
        {
            ret = bfd_proto_announce(pintf->address,peer->peer_addr, 0, RTPROTO_RIP, pintf->process_id, !state, rip_intf_bfd_callback);
            if(ret < 0){
                rip_debug(RIP_DEBUG_IP_RIP_RETURN, "rip_intf_bfd_register fail!\n");
                return RIP_FAIL;
            }     
            
            ret = rip_intf_create_bfd_peer(pintf->device_index, peer_addr);
        	if(ret < 0){
            	rip_debug(RIP_DEBUG_IP_RIP_RETURN, "rip_intf_bfd_register fail!\n");
            	return RIP_FAIL;
        	}  
        }
        rip_peer = rip_peer->forw;
    }
    
	return RIP_SUCCESS;
}
```

#### rip_cmd_intf_bfd_enable

```c
int rip_cmd_intf_bfd_enable(int argc, char **argv, struct user *u)
{
    int ret;
    uint32 device_index;
    struct rip_intf_ *pintf = NULL;
    
    /*检测命令行参数是否多余*/
    ret = cmdend( argc - 1, argv + 1, u );
    if(ret != 0)
    {
        return ret;
    }

    device_index = u->struct_p[1];
    if((pintf=rip_intf_array[device_index])==NULL)
    {
        return RIP_FAIL;
    }

    /*根据命令类型做不同处理*/
    switch( TypeOfFunc(u) )
    {
    case NORMAL_FUNC:
		if(pintf->bfd_enable_flag == 1)
			break;
		/*往bfd 注册关注回调*/
		ret = rip_intf_bfd_register(device_index, 0, 1);
        if(ret != 0){
             return RIP_FAIL;
        }
        pintf->bfd_enable_flag = 1;
        break;
    case NOPREF_FUNC:
    case DEFPREF_FUNC:
		if(!pintf->bfd_enable_flag)
			break;
		/*取消 注册回调*/
		ret = rip_intf_bfd_register(device_index, 0, 0);
        if(ret != 0){
            return RIP_FAIL;
        }
		rip_intf_array[device_index]->bfd_enable_flag = 0;
		/*端口实际状态up，rip端口需up*/
		if(BIT_TEST(rt_get_if_status(device_index), RIP_INTF_LINK_UP)){
			rip_debug(RIP_DEBUG_IP_RIP_MSG, "rip intf RIP_INTF_LINK_UP no use bfd\n");
			if((pintf->address == 0) || (!BIT_TEST(pintf->state ,RIP_INTF_LINK_UP)))
				return RIP_FAIL;
        
            if(BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
				return RIP_SUCCESS;
        
        	/* 打上进程端口激活的标志*/
			BIT_SET(pintf->state , RIP_INTF_PROCESS_ACTIVE);
			ret = rip_process_intf_activate(device_index , TRUE);
		}
        break;
    default:
        break;
    }

    return RIP_SUCCESS;	
}
```

#### rip_intf_add

```c
int rip_intf_add(uint32 device_index){
	/* 初始化 */
    rip_intf_array[device_index]->bfd_enable_flag = 0；
    rip_intf_array[device_index]->rip_bfd_sess_up_num = 0；    
	rip_intf_array[device_index]->bfd_peer_list.forw = &(rip_intf_array[device_index]->bfd_peer_list);
    rip_intf_array[device_index]->bfd_peer_list.back = &(rip_intf_array[device_index]->bfd_peer_list);
}
```

#### rip_intf_del

```c
/*RIP存储类型*/
enum RIP_MALLOC_TYPE
{
	RIP_INTF_TYPE = 1,
	RIP_NET_LIST_TYPE,
	RIP_NEIGH_LIST_TYPE,
	RIP_PEER_LIST_TYPE,
	RIP_DISTANCE_LIST_TYPE,
	RIP_NEIGHB_KEY_TYPE,
    /* 添加RIP端口的bfd_peer类型 */
    RIP_NEIGHB_PEER_TYPE, 
    
    ....
}

int rip_intf_del(uint32 device_index){
	/* 清空内存 */
    struct rip_bfd_peer_list_ *bfd_peer, *bfd_peer_temp;
	bfd_peer = rip_intf_array[device_index]->bfd_peer_list.forw;
	while (bfd_peer != &rip_intf_array[device_index]->bfd_peer_list)
	{
		bfd_peer_temp = bfd_peer;
		bfd_peer = bfd_peer->forw;
		REMQUE(bfd_peer_temp);
		rip_mem_free(bfd_peer_temp, RIP_NEIGHB_PEER_TYPE);
	}
}
```



#### rip_recv_packet

```c
struct rip_bfd_peer_list_ *bfd_peer;

/*if not find the peer, then create a new one*/
if (peer == &(pprocess->peer_list))
{
    peer = rip_mem_malloc(sizeof(struct rip_peer_list_), RIP_PEER_LIST_TYPE);
    if (peer == NULL)
    {
        return RIP_FAIL;
    }

    memset((void *)peer, 0, sizeof(struct rip_peer_list_));
    peer->peer_addr = src_socket_intf->sin_addr.s_addr;
    peer->peer_port = src_socket_intf->sin_port;
    peer->peer_intf = device_index;
    INSQUE(peer, pprocess->peer_list.back);
	
    /* 若RIP与BFD联动 */
    if(pintf->bfd_enable_flag){
        ret = rip_intf_bfd_register(pintf->device_index, peer->peer_addr);
        if(ret < 0){
            rip_debug(RIP_DEBUG_IP_RIP_RETURN, "rip_intf_bfd_register fail !\n");
            return RIP_FAIL;
        }  
        
        bfd_peer = pintf->bfd_peer_list.forw;
        while(bfd_peer != &pintf->bfd_peer_list){
            if(bfd_peer->bfd_peer_addr == peer->peer_addr){
                break;
            }
            bfd_peer = bfd_peer->forw;
        }
        
        if(bfd_peer == &pintf->bfd_peer_list){
            bfd_peer = rip_mem_malloc(sizeof(struct rip_bfd_peer_list_), RIP_NEIGHB_PEER_TYPE);
    		if (bfd_peer == NULL)
    		{
       			return RIP_FAIL;
    		}
            INSQUE(bfd_peer, pintf->bfd_peer_list.back);
        }
        bfd_peer->bfd_peer_up=0;
    }
    
    if (!sys_timer_run(pprocess->peer_timer_id) && (pprocess->peer_timer_id != 0))
    {
        sys_start_timer(pprocess->peer_timer_id, pprocess->peer_timeout);
    }
}

```

#### rip_intf_bfd_callback

```c
// int (*callback)(uint32 src_ip, uint32 dst_ip, uint32 connected, uint32 down)
// down:0 up:1
// connect:0 直连
int rip_intf_bfd_callback(uint32 src_ip, uint32 dst_ip, uint32 connected, uint32 down)
{
	struct rip_intf_ *pintf = NULL;
    struct rip_process_info_ *pprocess;
	struct rip_peer_list_ *peer,*peer_forw;
    struct rip_process_intf_list_ *intf_list_temp;
	struct rip_process_list_ *pprocess_list_temp;
    struct rip_bfd_peer_list_ *bfd_peer;
    
	uint32 device_id;
    int ret;

	pprocess_list_temp = rip_process_list.forw;
	while(pprocess_list_temp != &rip_process_list)
	{
    	pprocess = pprocess_list_temp->rip_process_info;
		if(pprocess && pprocess->intf_list)
		{
        	intf_list_temp = pprocess->intf_list->forw;
        	while(intf_list_temp != &(pprocess->intf_list)){
            	pintf = intf_list_temp->pintf;
            	if(pintf && pintf->address == src_ip){
               		break;
            	}	
            	intf_list_temp = intf_list_temp->forw;
        	}
		}
		pprocess_list_temp = pprocess_list_temp->forw;
	}
    
    if(!pintf){
        return RIP_FAIL;
    }
    
    device_index = pintf->device_index;
    rip_debug(RIP_DEBUG_IP_RIP_MSG, "rip_intf_bfd_callback device_index=%d, up = %d\n", device_index, down);
    
    /* 查找端口下的bfd_peer_list */
    bfd_peer = pintf->bfd_peer_list.forw;
    while(bfd_peer != &pintf->bfd_peer_list){
        if(bfd_peer->bfd_peer_addr == dst_ip){
             break;
        }
        bfd_peer = bfd_peer->forw;
    }
        
    if(bfd_peer == &pintf->bfd_peer_list){
        return RIP_FAIL;
    }

    /*端口down*/
	if(!down){
		/* 若当前端口非进程激活端口，不需要操作*/
		if(BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
		{	
            if(bfd_peer->bfd_peer_up == 1){
                bfd_peer->bfd_peer_up == 0;
                if(pintf->rip_bfd_sess_up_num > 0){
                    pintf->rip_bfd_sess_up_num--;
                }
                if(!pintf->rip_bfd_sess_up_num){
					BIT_RESET(pintf->state ,RIP_INTF_PROCESS_ACTIVE);
					ret = rip_process_intf_deactivate(pintf->device_index);
                }else{
                    /*删除端口从peer学习到的路由*/
            		ret = rip_clear_peer_route_list(device_index, dst_ip);
            		if (RIP_SUCCESS != ret)
            		{
                		rip_debug(RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d.\n", __FILE__,__LINE__);
                		return RIP_FAIL;
           	 		}
                
                	/*发送request报文,请求其他peer的路由信息（可能次优路由被替换了）*/
           	 		ret = rip_send_request(device_index);
            		if (RIP_SUCCESS != ret)
            		{
                		rip_debug(RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d.\n", __FILE__,__LINE__);
                		return RIP_FAIL;
            		}
                }
                
            }    
		}
	}
	else{/*端口up*/
		if((!pintf) || (pintf->address == 0) || (!BIT_TEST(pintf->state ,RIP_INTF_LINK_UP)))
			return RIP_FAIL;
        
        if(!bfd_peer->bfd_peer_up){
            pintf->rip_bfd_sess_up_num++;
            if(pintf->rip_bfd_sess_up_num==1){
                 /* 打上进程端口激活的标志*/
				BIT_SET(pintf->state , RIP_INTF_PROCESS_ACTIVE);
                ret = rip_process_intf_activate(device_index , TRUE);
            }else{
				/*端口发送request单播报文*/
				ret = rip_send_response(device_index, dst_ip, htons(RIP_PORT));
				if( RIP_SUCCESS != ret )
				{
					rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
					return RIP_FAIL;
				}
            }
			bfd_peer->bfd_peer_up == 1;    
        }
        
	}
    return RIP_SUCCESS;
}
```

#### rip_inft_bfd_peer_del

```c
int rip_bfd_peer_del(uint32 device_index, uint32 peer_addr){
	struct rip_bfd_peer_list_ *bfd_peer, *bfd_peer_temp;
	bfd_peer = rip_intf_array[device_index]->bfd_peer_list.forw;
	while (bfd_peer != &rip_intf_array[device_index]->bfd_peer_list)
	{
		bfd_peer_temp = bfd_peer;
		bfd_peer = bfd_peer->forw;
		if(bfd_peer_temp->bfd_peer_addr == peer_addr){
			REMQUE(bfd_peer_temp);
			rip_mem_free(bfd_peer_temp, RIP_NEIGHB_PEER_TYPE);
			break;
		}
	}
	return RIP_SUCCESS;
}
```



#### rip_intf_del_peers

```c
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
            if(pintf->bfd_enable_flag){
                /* 将bfd_peer删除 */
                rip_bfd_peer_del(device_index, peer_addr);
                /* 注销bfd */
        		ret = ret = rip_intf_bfd_register(pintf->device_index, peer->peer_addr, 0);
                if(ret < 0){
                	rip_debug(RIP_DEBUG_IP_RIP_RETURN, "rip_intf_del_peers failed!\n");
                	return 0;
            	}   
            }
            
			REMQUE(peer);
			rip_mem_free(peer,RIP_PEER_LIST_TYPE);
		}
		peer=next;
	}
    
	return cnt;
}
```



#### rip_peer_timeout

```c
int rip_peer_timeout(uint32 processid)
{
	struct rip_peer_list_ *peer = NULL,*peer_temp = NULL;
	int interval = 0;
	int ret = 0;
	struct rip_process_info_ *pprocess;
	struct rip_process_intf_list_ *intf_list; 
    struct rip_intf_ *pintf; 
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

		/* 对于no network,no neighbor,对应的接口被删除或状态down的peer,也在这里删除,防止再发request
		 * Commented by dangzhw in 2009.12.05 15:47:38 */
		if(rip_peer_should_deleted(peer, pprocess))
		{
            /* 遍历rip_process_intf_list_ intf_list，如果端口开启了bfd-enable，需要注销对应peer的会话 */
            for(intf_list = pprocess->intf_list->forw; intf_list != &(pprocess->intf_list); intf_list = intf_list->forw){
                pintf = intf_list->pintf;
                if(!pintf && pintf->bfd_enable_flag && peer->peer_intf == pintf->device_index ){
                    /* 将bfd_peer删除 */
                	rip_bfd_peer_del(pintf->device_index, peer_addr);
                    /* 注销bfd */
        			ret = rip_intf_bfd_register(pintf->device_index, peer->peer_addr, 0);
               	 	if(ret < 0){
                		rip_debug(RIP_DEBUG_IP_RIP_RETURN, "rip_peer_timeout, bfd_proto_announce failed!\n");
                		return RIP_FAIL;
            		}  
                    break;
                }
            }
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
            /* 遍历rip_process_intf_list_ intf_list，如果端口开启了bfd-enable，需要注销对应peer的会话 */
            for(intf_list = pprocess->intf_list->forw; intf_list != &(pprocess->intf_list); intf_list = intf_list->forw){
                pintf = intf_list->pintf;
                if(!pintf && pintf->bfd_enable_flag){
                    /* 将bfd_peer删除 */
                	rip_bfd_peer_del(pintf->device_index, peer_addr);
                    /* 注销bfd */
        			ret = rip_intf_bfd_register(pintf->device_index, peer->peer_addr, 0);
               	 	if(ret < 0){
                		rip_debug(RIP_DEBUG_IP_RIP_RETURN, "rip_peer_timeout, bfd_proto_announce failed!\n");
                		return RIP_FAIL;
            		}  
                    break;
                }
            }
			REMQUE(peer);
			rip_mem_free(peer, RIP_PEER_LIST_TYPE);
		}
		
		peer = peer_temp;
	}

	if(pprocess->peer_list.forw != &pprocess->peer_list)
		sys_start_timer(pprocess->peer_timer_id , pprocess->peer_timeout);	

	return RIP_SUCCESS;
}
```





### BFD模块

#### bfd_proto_announce

```c
/**
*int bfd_proto_announce(uint32 src_ip, uint32 dst_ip, uint8 connected, uint32 proto,
					   uint32 process, uint32 nbr_state, int (*callback)(uint32 src_ip, uint32 dst_ip, uint32 connected, uint32 down))
*/
    
int bfd_proto_announce(uint32 if_index, uint32 neigh_addr, uint32 proto, uint32 state, int (*callback)(uint32 if_index, uint32 down));

#define BFD_PROTO_RIP RTPROTO_RIP
(proto==BFD_PROTO_RIP)? "RIP" : (proto==BFD_PROTO_RIP)? "RIP" : \
```



