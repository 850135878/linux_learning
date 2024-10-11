char rip_redis_version[] = "rip_redis.c last changed at 2012.10.12 10:12:59 by yuguangcheng\n";
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
 *        FILE NAME       :   rip_redis.c
 *        AUTHOR          :   Fengsb
 *        DATE            :   2006.05.17 16:58:21
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#include <rip/rip_include.h>
#include <rip/rip_main.h>
#include <rip/rip_redis.h>
#include <rip/rip_intf.h>
#include <rip/rip_timer.h>
#include <rip/rip_debug.h>
#include <rip/rip_redis.h>

extern struct rip_glb_info_  rip_glb_info;
extern struct rip_intf_ *rip_intf_array[INTERFACE_DEVICE_MAX_NUMBER + 1];
extern struct rip_process_list_  rip_process_list;

void rip_del_from_redis_list(struct rip_route_ *rip_route)
{
	struct rip_route_list_ *redis_list;
	if(!rip_route||rip_route->route_type!=RIP_REDIS_ROUTE)return;
	redis_list=rip_route->red_ptr;
	if(redis_list)
	{
		REMQUE(redis_list);
		rip_mem_free(redis_list,RIP_ROUTE_LIST_TYPE);
		rip_route->red_ptr=NULL;
	}
}

/*===========================================================
函数名:      rip_route_change
函数功能: 接收到routing路由发生变化时的处理函数
输入参数: rth : 路由发生的路由节点
输出参数: 无
返回值:      处理成功,返回RIP_SUCCESS
                       处理失败,返回RIP_FAIL
备注:Fengsb 2006-02-19考虑到创建的转发路由存在在rip_tbl_array[vrf_id]->rip_table
表中相应节点下的第一个entry中；
考虑到对相同的前缀，只有一个转发路由协议能将转发路由
添加进node的第一个entry。所以有以下算法的优化
=============================================================*/
int rip_route_change( struct _rt_head *rth )
{
	
	uint32 vrf_id;
	struct rip_redis_list_ *rip_redis_list = NULL;
	rt_entry *rt;
	struct prefix node_prefix;
	struct route_node *rip_rt_node = NULL;
	struct rip_route_ *rip_route_head = NULL;
    BOOL   b_redis_rt = FALSE;  /* there is no redistribute entry for rth */
	int ret;
	struct rip_process_list_ *pprocess_list;
	struct rip_process_info_ *pprocess;

	vrf_id = rth->vrf_id;

	/* 遍历每个rip进程，判断是否需要更新
	 * Commented by dangzhw in 2010.03.15 17:00:44 */
	pprocess_list = rip_process_list.forw;
	while(pprocess_list != &rip_process_list)
	{
		if(!(pprocess = pprocess_list->rip_process_info))
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}

		if(pprocess->vrf_id != vrf_id)
		{
			pprocess_list = pprocess_list->forw;
			continue;
		}

		b_redis_rt = FALSE;

		/*Fengsb 2006-02-19 检查接收到的路由条目是否存在对应的转发路由*/
		memset( &node_prefix, 0, sizeof(struct prefix) );
		node_prefix.family = AF_INET;
		node_prefix.safi = SAFI_UNICAST;
		node_prefix.prefixlen = mask_to_prefix(rth->rth_mask);
		node_prefix.u.prefix4.s_addr = rth->rth_dest;
		rip_rt_node = route_node_lookup( pprocess->rip_table, &node_prefix );

		if(rip_rt_node && rip_rt_node->info )
		{
			route_unlock_node( rip_rt_node );
			rip_route_head = rip_rt_node->info;
			if(rip_route_head->route_type == RIP_REDIS_ROUTE)
			{
	            b_redis_rt = TRUE;            
			}
			else if(rip_route_head->route_type == RIP_DEF_ROUTE)
			{
				rip_route_head = rip_route_head->forw;
				if(rip_route_head&&(rip_route_head->route_type==RIP_REDIS_ROUTE))
					b_redis_rt = TRUE;
			}
		}

		/*Fengsb 2006-05-17 add for patch : rip default route, we should withdraw the default route*/
        if((node_prefix.u.prefix4.s_addr == 0) && (RIP_DEF_ROUTE_ENABLE_SAFE == pprocess->default_route_flag))
        {
			rt = rth->rth_active;
			if(rt&&(rt->rt_proto != RTPROTO_RIP)) 
			{
				rip_create_orig_route(pprocess);
			}           
			else
			{
				rip_del_orig_route(pprocess);
			}
	    }

		/*fengsb 2006-0219 add these code instead of the above shield codes.*/
		if(b_redis_rt == TRUE)
		{
			rip_del_from_redis_list(rip_route_head);
			/*删除该路由*/
			ret = rip_del_route( rip_route_head );
			if( RIP_SUCCESS != ret )
			{
				pprocess_list = pprocess_list->forw;
				continue;
			}
		}

		if( rth->rth_active == NULL )
		{
			/*不作处理*/
			pprocess_list = pprocess_list->forw;/*徐海青2010-12-07修改bug号SWBUG00008946*/
			continue;
		}
		else
		{   /*apply for redistribute list, adjust for new redistribute policy*/
			rt = rth->rth_active;
			
			/*将rth_active加入转发路由列表*/
			QUE_LIST(rip_redis_list, &pprocess->redis_list)
			{
				if ((rip_redis_list->process == rt->process) && (rip_redis_list->proto == rt->rt_proto))
				{	
					rip_create_redis_route( rt, rip_redis_list, pprocess );
					break;
				}
			}QUE_LIST_END(rip_redis_list, &pprocess->redis_list);
		}

		pprocess_list = pprocess_list->forw;
	}

	return 0;
}

/*===========================================================
函数名:      rip_redis_callback
函数功能: 转发的回调函数
输入参数: proto_fg  -- 进行转发路由的路由模块
			    process   -- 进行转发路由的路由模块进程
			    type      -- 转发操作类型
			    detail    -- 所转发路由的信息
输出参数: 无
返回值   : 处理成功,返回RIP_SUCCESS
                     处理失败,返回RIP_FAIL
备注:
=============================================================*/
void rip_redis_callback( uint32 proto_fg, uint32 process, uint32 type, void * detail)
{
	uint32 redis_protocol;
	uint32 redis_process;
	struct route_map *redis_route_map;
	struct redistribute_chg_infor *redis_info;
	int ret;
	uint32 vrf_id;
	struct rip_process_info_ *pprocess;

	/*进行转发的路由模块判断*/
	if( (RTPROTO_RIP != proto_fg))
	{
		return;
	}

	ret = rip_lookup_process_byprocessid(process, &pprocess);
	if(!ret)
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return;
	}
	
	/*获取转发信息*/
	redis_info = (struct redistribute_chg_infor *)detail;
	redis_protocol = redis_info->proto;
	redis_process = redis_info->process;
	redis_route_map = redis_info->rr_map;
	vrf_id = redis_info->vrf_id;

	if(pprocess->vrf_id != vrf_id)
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return;
	}
	
	/*根据转发操作类型做不同处理*/
	switch( type )
	{
		case INDEPEND_CHG_REDISTRIBUTE_ADD:
			rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [INDEPEND_CHG_REDISTRIBUTE_ADD] message\n" );

			ret = rip_redis_add( pprocess, redis_protocol, redis_process, redis_route_map );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return;
			}
			break;
		case INDEPEND_CHG_REDISTRIBUTE_DEL:
			rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [INDEPEND_CHG_REDISTRIBUTE_DEL] message\n" );

			ret = rip_redis_del( pprocess, redis_protocol, redis_process, redis_route_map );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return;
			}
			break;
		case INDEPEND_CHG_REDISTRIBUTE_CHG:
			rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [INDEPEND_CHG_REDISTRIBUTE_CHG] message\n" );

			/*先删除*/
			ret = rip_redis_del( pprocess, redis_protocol, redis_process, redis_route_map );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return;
			}
			
			/*再增加*/
			ret = rip_redis_add( pprocess, redis_protocol, redis_process, redis_route_map );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return;
			}
			break;
		default:
			break;
	}
	
	return;
}


/*===========================================================
函数名:      rip_redis_add
函数功能: redistribute命令的处理
输入参数: vrf_id : 路由表对应的VRF ID
                       proto : 转发的路由协议
                       process : 转发的路由协议进程号
                       map_name : 转发的route-map name策略
输出参数: 无
返回值:      处理成功,返回RIP_SUCCESS
                       处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_redis_add( struct rip_process_info_ *pprocess, uint32 proto, uint32 process, struct route_map *map_name )
{
	int ret;
	rt_head *rth;
	rt_list *rth_list;
	struct rip_redis_list_ *rip_redis_list;
#if 0/*MULTICORE*/
	struct vrf_table_list_ *vrf_table = NULL;
#endif

	QUE_LIST(rip_redis_list, &pprocess->redis_list){
		if( (proto == rip_redis_list->proto) && (process == rip_redis_list->process) )
		{
			/*已进行过redistribute, 不作处理*/
			return RIP_SUCCESS;
		}
	}QUE_LIST_END(rip_redis_list, &pprocess->redis_list);

	rip_redis_list = (struct rip_redis_list_ *)rip_mem_malloc( sizeof(struct rip_redis_list_), RIP_REDIS_LIST_TYPE );
	if( rip_redis_list == NULL )
	{
		/*内存分配失败*/
		fprintf(stderr, "RIP: Malloc memory failed.(%s, %d)\n", __FILE__, __LINE__ );
		return RIP_MEM_FAIL;
	}
	memset( rip_redis_list, 0, sizeof(struct rip_redis_list_) );
	rip_redis_list->proto = proto;
	rip_redis_list->process = process;
	rip_redis_list->redis_route_map = map_name;

	QUE_INIT(&rip_redis_list->rip_route_list);	
	INSQUE( rip_redis_list, pprocess->redis_list.back );

#if 0/*MULTICORE*/
            	vrf_table = vrf_table_enter(pprocess->vrf_id,RT_RD_LOCK);
	if(vrf_table == NULL)
	{
		return RIP_FAIL;
	}
#endif

	/*加入转发路由*/
	rth_list = rthlist_active( pprocess->vrf_id, TRUE );
	RT_LIST(rth, rth_list, rt_head)
	{
		if( NULL == rth->rth_active )
		{
			continue;
		}
		
		ret = rip_create_redis_route( rth->rth_active, rip_redis_list, pprocess );
		if( RIP_SUCCESS != ret )
		{
			continue;
		}
	}RT_LIST_END(rth, rth_list, rt_head);

	if( rth_list )
	{
		RTLIST_RESET( rth_list );
	}
#if 0/*MULTICORE*/
	vrf_table_leave(vrf_table,RT_RD_LOCK);
#endif
	return RIP_SUCCESS;	
}


/*===========================================================
函数名:      rip_redis_del
函数功能: no redistribute命令的处理
输入参数: vrf_id : 路由表对应的VRF ID
                       proto : 转发的路由协议
                       process : 转发的路由协议进程号
                       map_name : 转发的route-map name策略
输出参数: 无
返回值:      处理成功,返回RIP_SUCCESS
                       处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_redis_del( struct rip_process_info_ *pprocess, uint32 proto, uint32 process, struct route_map *map_name )
{
	int ret;
	struct rip_redis_list_ *rip_redis_list;
	struct rip_route_list_ *rip_route_list, *rip_route_list_forw;

	QUE_LIST(rip_redis_list, &pprocess->redis_list){
		if( (proto == rip_redis_list->proto) && (process == rip_redis_list->process) )
		{
			/*删除所有转发路由*/
			for( rip_route_list = rip_redis_list->rip_route_list.forw; rip_route_list != &(rip_redis_list->rip_route_list); rip_route_list = rip_route_list_forw )
			{
				rip_route_list_forw = rip_route_list->forw;
				
				/*删除该路由*/
				ret = rip_del_route( rip_route_list->rip_route );
				if( RIP_SUCCESS != ret )
				{
					rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
					return RIP_FAIL;
				}

				REMQUE( rip_route_list );
				rip_mem_free( rip_route_list, RIP_ROUTE_LIST_TYPE );
			}

			/*删除转发策略*/
			REMQUE( rip_redis_list );
			rip_mem_free( rip_redis_list, RIP_REDIS_LIST_TYPE );
			
			return RIP_SUCCESS;
		}
	}QUE_LIST_END(rip_redis_list, &pprocess->redis_list);
	
	return RIP_SUCCESS;	
}

/*===========================================================
函数名:      rip_create_redis_route
函数功能: 创建转发路由
输入参数: rt : 发生变化的路由
                       rip_redis_list: 转发策略列表
                       vrf_id :转发所在的VRF ID
输出参数: 无
返回值:      处理成功,返回RIP_SUCCESS
                       处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_create_redis_route( rt_entry *rt, struct rip_redis_list_ *rip_redis_list, struct rip_process_info_ *pprocess )
{	
	struct rip_route_ *rip_route, *rip_route_head = NULL;
	struct route_node *rip_route_node;
	struct prefix route_prefix;
	struct route_info route_info;
	rip_redistribute_struct *rr;	
	struct rip_route_list_ *rip_route_list;
	struct route_distance_list *rip_dis_lst = NULL;
	int i;
	int count;
	BOOL can_redis_ibgp;
	int ret;
	uint32     rt_nh_num;
	IPV4_RT_NH *ipv4_rt_nh;

	/*xuhaiqing 2010-12-21号将最大路由条目数限制取消*/
	/*路由表中路由条数超过最大值，不再添加*/
	/*if( pprocess->route_num >= pprocess->max_route_num )
	{
		rip_warning("%%Warning, Limit for rip routes exceeded, max # is %d.\n", pprocess->max_route_num );
		return RIP_SUCCESS;
	}*/

	/*应用route_map于转发的路由*/
	memset( (void *)&route_prefix, 0, sizeof(struct prefix) );
	route_prefix.family = AF_INET;
	route_prefix.safi = SAFI_UNICAST;
	route_prefix.prefixlen = mask_to_prefix( rt->rt_rth->rth_mask);
	route_prefix.u.prefix4.s_addr = rt->rt_rth->rth_dest;

	if( !BIT_TEST(rt->rt_state, RTS_ACTIVE) )
	{
		return RIP_SUCCESS;
	}

	can_redis_ibgp = FALSE;

	can_redis_ibgp = rt_redis_ibgp( pprocess->vrf_id );
	
	/*not to redistribute iBGP route */
	if( (can_redis_ibgp == FALSE) && BIT_TEST(rt->rt_state, RTS_EXTERIOR_2))
	{
		return RIP_SUCCESS;
	}

	rr = redistribute_check( pprocess->rip_redis_link, rt );
	if(!rr)
	{
		return RIP_SUCCESS;
	}
       
	memset( (void *)&route_info, 0, sizeof(struct route_info) );
	route_info.metric = rt->metric;
	route_info.tag = rt->rt_tag;
	rt_nh_num = rt->rt_nh_num;
	for(i = 0; i < rt_nh_num && i < RMAP_NEXTHOP_MAX; i++){
		ipv4_rt_nh = &(rt->rt_nh[i]);
		if(ipv4_rt_nh->nh_ptr){
			RT_NH_RWLOCK_RD_LOCK(&(ipv4_nh_hash_table[ipv4_rt_nh->nh_ptr->hash_key].nh_list_rwlock));
			route_info.nexthop[i] = ipv4_rt_nh->nh_ptr->nh_gateway;
			RT_NH_RWLOCK_RD_UNLOCK(&(ipv4_nh_hash_table[ipv4_rt_nh->nh_ptr->hash_key].nh_list_rwlock));
		}
	}

	ret = route_map_apply( rip_redis_list->redis_route_map, &route_prefix, RMAP_GEN, (void *)&route_info);
	if( RMAP_MATCH == ret )
	{
		/*获取route_node*/
		rip_route_node = route_node_get( pprocess->rip_table, &route_prefix ,NULL);
		if( NULL == rip_route_node )
		{
			return RIP_FAIL;
		}
		
		if( NULL != rip_route_node->info )
		{
			/* route_unlock_node( rip_route_node ); Fengsb shield 2006-02-17*/
			rip_route_head = rip_route_node->info;
			
			for( count = 1, rip_route = rip_route_head; count <= rip_route_head->equi_route_num; rip_route = rip_route->forw, count++ )
			{
				if(rip_route->route_type == RIP_REDIS_ROUTE || rip_route->route_type == RIP_CONNECT_ROUTE)
				{	
					/*Fengsb 2006-05-23 如果network已经把直连路由加入了，或者已经存在转发路由
					则不再添加*/
				    /* Fengsb 2006-02-17 the route is exist, for  route_node_get lock++ , so we unlock */
				    route_unlock_node( rip_route_node );
					return RIP_SUCCESS;
				}
			}
		}		
        
		/*将route_entry_temp加入本地路由表*/
		rip_route = (struct rip_route_ *)rip_mem_malloc( sizeof(struct rip_route_), RIP_ROUTE_TYPE );
		if( rip_route == NULL )
		{
			route_unlock_node( rip_route_node );
			fprintf(stderr, "RIP: Malloc memory failed'(%s, %d)\n", __FILE__, __LINE__ );
			return RIP_MEM_FAIL;
		}
		memset( (void *)rip_route, 0, sizeof(struct rip_route_) );
		rip_route->forw = rip_route;
		rip_route->back = rip_route;

		rip_route->rmap_set_metric=route_info.set_metric_flag;
		rip_route->metric = rip_route->rmap_set_metric?(route_info.metric):(uint32)(pprocess->default_metric);
		rip_route->route_tag = route_info.tag;
		rip_route->refresh_time = time_sec;
		rip_route->route_type = RIP_REDIS_ROUTE;
		rip_dis_lst = pprocess->distance_list;
		if(rip_dis_lst == NULL)
		{
		    rip_route->distance = RIP_DEF_DISTANCE;            
		}
		else
		{
		    rip_route->distance = rip_dis_lst->default_distance;
		}
		rip_route->gw_addr = 0;
		rip_route->gw_index = 0;
		
		rip_route->pprocess = pprocess;

		/*加入rip路由表，考虑到常规更新的时候，若存在等价路由，转发进rip路由表的路由优先
        	发送，所以，需要将该entry作为路由节点下的第一个entry，但存在默认路由时例外*/
		rip_route->route_node = rip_route_node;
		if( NULL != rip_route_node->info )
		{			          
			if(rip_route_head->route_type!=RIP_DEF_ROUTE)
			{
				rip_route->forw = rip_route_head;
				rip_route->back = rip_route_head->back;
				rip_route_head->back->forw = rip_route;
				rip_route_head->back = rip_route;
				rip_route->equi_route_num = rip_route_head->equi_route_num;
				rip_route->equi_nbr_num= rip_route_head->equi_nbr_num;
				rip_route_node->info = rip_route;
				rip_route_head = rip_route;
			}
			else
			{
				INSQUE(rip_route, rip_route_head);
			}
		}
		else
		{
			rip_route_node->info = rip_route;
			rip_route_head = rip_route;
		}
		
		/*将路由加入转发路由列表*/
		rip_route_list = (struct rip_route_list_ *)rip_mem_malloc( sizeof(struct rip_route_list_), RIP_ROUTE_LIST_TYPE );
		if( rip_route_list == NULL )
		{
			/*内存分配失败*/
			fprintf(stderr, "RIP: Malloc memory failed.(%s, %d)\n", __FILE__, __LINE__ );
			return RIP_MEM_FAIL;
		}
		memset( rip_route_list, 0, sizeof(struct rip_route_list_) );
		rip_route_list->rip_route = rip_route;
		rip_route->red_ptr = rip_route_list;	/*Fengsb add 2006-02-19*/

		INSQUE( rip_route_list, rip_redis_list->rip_route_list.back );

		pprocess->route_num++;
		pprocess->redis_route_num++;
		rip_route_head->equi_route_num++;

		rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "RIP-DB: Adding redistributed route %s/%d to RIP database\n", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );

		/*触发更新*/
		rip_add_trigger( rip_route, pprocess , FALSE );
	}

	return RIP_SUCCESS;	
}

/*===========================================================
函数名:      rip_create_redis_connect
函数功能: 创建转发路由直连路由，当no network *.*.*.*的时候
			 :可能会从rip database中删除刚才的某些直连路由；但是
			 此时，因为配置了redistribute connect，所以需要再次加入。
输入参数: rt :待加入的转发路由 
                       vrf_id :转发所在的VRF ID
输出参数: 无
返回值:      处理成功,返回RIP_SUCCESS
                       处理失败,返回RIP_FAIL
备注:
=============================================================*/
void rip_create_redis_connect(uint32 device_index)	
{   	
	struct rip_redis_list_ *rip_redis_list = NULL;	
	struct _rt_entry *rt;
	struct rip_process_info_ *pprocess;
	struct rip_intf_ *pintf;

	if(device_index>INTERFACE_DEVICE_MAX_NUMBER)return;
	pintf=rip_intf_array[device_index];
	if(!pintf)return;
	if(pintf->address==0)return;
	pprocess=pintf->pprocess;
	if(!pprocess)return;
	rt = rt_locate(pprocess->vrf_id, RTS_ACTIVE, pintf->address, pintf->mask, RTPROTO_DIRECT, 0);
	if(!rt)return;
	
	/*将rth_active加入转发路由列表*/
	QUE_LIST(rip_redis_list, &pprocess->redis_list){
	   	if( (rip_redis_list->process == rt->process) && (rip_redis_list->proto == rt->rt_proto) )
	   	{
			rip_create_redis_route( rt, rip_redis_list, pprocess );
			return;
		}
	}QUE_LIST_END(rip_redis_list, &pprocess->redis_list);

	return;
}

/*===========================================================
函数名:      rip_del_redis_connect
函数功能: 创建转发路由直连路由，当no network *.*.*.*的时候
			 :可能会从rip database中删除刚才的某些直连路由；但是
			 此时，因为配置了redistribute connect，所以需要再次加入。
输入参数: rt :待加入的转发路由 
                       vrf_id :转发所在的VRF ID
输出参数: 无
返回值:      处理成功,返回RIP_SUCCESS
                       处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_del_redis_connect(struct rip_route_ *rip_route_head )	
{   	
	int ret;		
	struct rip_route_list_ *red_ptr;	

	red_ptr = rip_route_head->red_ptr ;
	/*删除该路由*/
	ret = rip_del_route( rip_route_head );
	if( RIP_SUCCESS != ret )
	{
		return RIP_FAIL;
	}
	if(red_ptr)
	{
		REMQUE( red_ptr );
		rip_mem_free( red_ptr, RIP_ROUTE_LIST_TYPE );  
	}
	return RIP_SUCCESS;
}

