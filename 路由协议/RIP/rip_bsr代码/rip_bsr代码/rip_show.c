char rip_show_version[] = "rip_show.c last changed at 2013.10.29 11:14:37 by yuguangcheng\n";
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
 *        FILE NAME       :   rip_show.c
 *        AUTHOR          :   HQ.Sun
 *        DATE            :   2006.01.08 11:15:52
 *        PURPOSE:        :   RIP模块显示信息处理模块
 *
**************************************************************************/
#include <rip/rip_include.h>
#include <rip/rip_main.h>
#include <rip/rip_show.h>

extern struct rip_glb_info_  rip_glb_info;
extern struct rip_intf_ *rip_intf_array[INTERFACE_DEVICE_MAX_NUMBER + 1];
extern struct rt_mem_list_  *rt_mem_list[RT_MEM_TYPE_MAX];
extern struct rip_process_list_  rip_process_list;

/*char           *vrf_GetNamebyID(uint32 vrf_id);
*/
/*show ip 下的RIP命令*/
struct cmds rip_cmds_show_ip_tab[] = 
{
	{ 
		"rip", 
		MATCH_AMB,
		0,
		0, 
		rip_cmd_show_ip_rip,
		NULL,
		NULL, 
		0,
		0, 
		"rip           -- Show RIP information",
		"rip           -- 显示RIP协议信息", 
		NULLCHAR,
		NULLCHAR
	},
	
	{NULLCHAR}
};

struct cmds rip_cmds_show_rip_process_tab[] = 
{
	{ 
		"0123456789", 
		MATCH_DIG, 
		0,
		0, 
		rip_cmd_show_process,
		NULL,
		NULL,
		0,
		0,
		"<1-65535>              -- Process ID number",
		"<1-65535>              -- 进程号", 
		NULLCHAR,
		NULLCHAR
	},
	    /*隐含命令*/
	{
		"version",
		MATCH_AMB,
		0,
		0, 
		rip_cmd_show_version, 
		NULL,
		NULL, 
		0, 
		0,
		NULLCHAR,
		NULLCHAR, 
		NULLCHAR,
		NULLCHAR
	},
	
	/*隐含命令*/
	{
		"memory", 
		MATCH_AMB,
		0,
		0, 
		rip_cmd_show_memory, 
		NULL,
		NULL, 
		0,
		0,
		NULLCHAR, 
		NULLCHAR, 
		NULLCHAR,
		NULLCHAR
	},

	{ 
		"<cr>",
		MATCH_AMB,
		0,
		0, 
		NULL,
		NULL,
		NULL, 
		0,
		0, 
		"<cr>",
		"<cr>",
		NULLCHAR,
		NULLCHAR
	},
	
	
	{NULLCHAR}
};

/*RIP show ip rip命令*/
struct cmds rip_cmds_show_ip_rip_process_tab[] = 
{
	{ 
		"database",
		MATCH_AMB,
		0,
		0, 
		rip_cmd_show_ip_rip_process_data,
		NULL,
		NULL, 
		0,
		0, 
		"database          -- Show RIP database information",
		"database          -- 显示RIP所有路由 ", 
		NULLCHAR,
		NULLCHAR
	},

	{ 
		"protocol",
		MATCH_AMB,
		0,
		0, 
		rip_cmd_show_ip_rip_process_proto,
		NULL,
		NULL, 
		0,
		0, 
		"protocol          -- Show RIP protocol information",
		"protocol          -- 显示RIP协议相关信息", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ 
		"summary",
		MATCH_AMB, 0, 0, 
		rip_cmd_show_ip_rip_process_summary,
		NULL, NULL, 0, 0, 
		"summary           -- Show RIP route summary information",
		"summary           -- 显示RIP路由汇总信息", 
		NULLCHAR, NULLCHAR
	},

	{ 
		"interface",
		MATCH_AMB, 0, 0, 
		rip_cmd_show_ip_rip_process_intf,
		NULL, NULL, 0, 0, 
		"interface         -- Show RIP interface information",
		"interface         -- 显示RIP端口信息", 
		NULLCHAR, NULLCHAR
	},


    	{ 
		"peer",
		MATCH_AMB, 0, 0, 
		rip_cmd_show_ip_rip_process_peer,
		NULL, NULL, 0, 0, 
		"peer              -- Show RIP peer information",
		"peer              -- 显示RIP peer 信息", 
		NULLCHAR, NULLCHAR
	},

	{ 
		"<cr>",
		MATCH_AMB,
		0,
		0, 
		NULL,
		NULL,
		NULL, 
		0,
		0, 
		"<cr>",
		"<cr>",
		NULLCHAR,
		NULLCHAR
	},

	{ NULLCHAR }
};

/*RIP show ip rip database命令*/
struct cmds rip_cmds_show_proto_tab[] = 
{
#if 0
	{ 
		"vrf",
		MATCH_AMB,
		0,
		0, 
		rip_cmd_show_proto_vrf,
		NULL,
		NULL, 
		0,
		0, 
		"vrf               -- VPN Routing/Forwarding instance",
		"vrf               -- VPN路由和转发表", 
		NULLCHAR,
		NULLCHAR
	},
#endif

	{ 
		"<cr>",
		MATCH_END,
		0,
		0, 
		NULL,
		NULL,
		NULL, 
		0,
		0,
		"<cr>", 
		"<cr>", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ NULLCHAR }
};
#if 0
/*RIP show ip rip database vrf命令*/
struct cmds rip_cmds_show_proto_vrf_tab[] = 
{
	{ 
		"1234567890",
		MATCH_STR,
		0,
		0, 
		rip_cmd_show_proto_vrf_name,
		NULL,
		NULL, 
		0,
		0, 
		"WORD              -- VPN Routing/Forwarding instance name",
		"WORD              -- VPN路由和转发表名字", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ NULLCHAR }
};

/*RIP show ip rip database命令*/
struct cmds rip_cmds_show_database_tab[] = 
{
#ifdef VRF_MODULE
	{ 
		"vrf",
		MATCH_AMB,
		0,
		0, 
		rip_cmd_show_data_vrf,
		NULL,
		NULL, 
		0,
		0, 
		"vrf               -- VPN Routing/Forwarding instance",
		"vrf               -- VPN路由和转发表", 
		NULLCHAR,
		NULLCHAR
	},
#endif

	{ 
		"<cr>",
		MATCH_END,
		0,
		0, 
		NULL,
		NULL,
		NULL, 
		0,
		0,
		"<cr>", 
		"<cr>", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ NULLCHAR }
};

/*RIP show ip rip database vrf命令*/
struct cmds rip_cmds_show_database_vrf_tab[] = 
{
	{ 
		"1234567890",
		MATCH_STR,
		0,
		0, 
		rip_cmd_show_data_vrf_name,
		NULL,
		NULL, 
		0,
		0, 
		"WORD              -- VPN Routing/Forwarding instance name",
		"WORD              -- VPN路由和转发表名字", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ NULLCHAR }
};
#endif
/*===========================================================
函数名:      rip_cmd_show_memory
函数功能: show memory rip命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_CMD_FAIL
备注:
=============================================================*/
int rip_cmd_show_memory( int argc, char *argv[], struct user *u )
{
	int ret;
	int malloc_type;
	int total_count;
	uint32 total_size;
	uint32 max_total_size;
	int proto_type;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	proto_type = RT_RIP;

	if( NULL == rt_mem_list[proto_type] )
	{
		/*该模块内存没有注册,不处理*/
		return RIP_SUCCESS;
	}

	total_count = 0;
	total_size = 0;
	max_total_size = 0;

	rt_show_printf( u, "*------------------------- RIP Memory Statistic ------------------------*\n" );

	rt_show_printf( u, "%-11s%-11s%-15s%-11s%s\n", "Size", "Malloc-Num", "Max-malloc-num", "Total-size", "Struct-info" );

	for( malloc_type = 0; malloc_type < RIP_MAX_MALLOC_TYPE; malloc_type++ )
	{
		if( rt_mem_list[proto_type][malloc_type].uint_size == 0 )
		{
			continue;
		}
		
		total_count++;

		total_size += (rt_mem_list[proto_type][malloc_type].alloc_num) * (rt_mem_list[proto_type][malloc_type].uint_size);
		max_total_size += (rt_mem_list[proto_type][malloc_type].max_alloc_num) * (rt_mem_list[proto_type][malloc_type].uint_size);

		rt_show_printf( u, "%-11d", rt_mem_list[proto_type][malloc_type].uint_size );
		rt_show_printf( u, "%-11d", rt_mem_list[proto_type][malloc_type].alloc_num );
		rt_show_printf( u, "%-15d", rt_mem_list[proto_type][malloc_type].max_alloc_num );
		rt_show_printf( u, "%-11d", rt_mem_list[proto_type][malloc_type].alloc_num * rt_mem_list[proto_type][malloc_type].uint_size );

		switch( malloc_type )
		{
			/* RIP */
			case RIP_INTF_TYPE:
				rt_show_printf( u, "rip_intf_t\n" );
				break;
			case RIP_NET_LIST_TYPE:
				rt_show_printf( u, "rip_net_list_t\n" );
				break;
			case RIP_NEIGH_LIST_TYPE:
				rt_show_printf( u, "rip_neigh_list_t\n" );
				break;
			case RIP_DISTANCE_LIST_TYPE:
				rt_show_printf( u, "rip_distance_list_t\n" );
				break;
			case RIP_ROUTE_TYPE:
				rt_show_printf( u, "rip_route_t\n" );
				break;
            		case RIP_RT_GATEWAY_TYPE:
                		rt_show_printf( u, "rt_gateway\n" );
				break;
			case RIP_TRIGGER_LIST_TYPE:
				rt_show_printf( u, "rip_trigger_list_t\n" );
				break;
			case RIP_ROUTE_LIST_TYPE:
				rt_show_printf( u, "rip_route_list_t\n" );
				break;
			case RIP_MD5_RECORD_TYPE:
				rt_show_printf( u, "rip_md5_record_t\n" );
				break;
			case RIP_TBL_INFO_TYPE:
				rt_show_printf( u, "rip_tbl_info_t\n" );
				break;
			case RIP_REDIS_LIST_TYPE:
				rt_show_printf( u, "rip_redis_list_t\n" );
				break;
			case RIP_OFFSET_LIST_TYPE:
				rt_show_printf( u, "rip_offset_list_t\n" );
			case RIP_PROCESS_LIST:
				rt_show_printf( u, "rip_process_list_t\n" );
			case RIP_PROCESS_INFO:
				rt_show_printf( u, "rip_process_info_t\n" );
			case RIP_PROCESS_INTF_LIST:
				rt_show_printf( u, "rip_process_intf_list_t\n" );
			default:
				break;
		}
	}

	rt_show_printf( u, "*-----------------------------------------------------------------------*\n" );

	rt_show_printf( u, "RIP Blocks: %u\n", total_count );
	rt_show_printf( u, "Current all of memory allocated for RIP task in bytes is %u\n", total_size );
	rt_show_printf( u, "Maximum all of memory allocated for RIP task in bytes is %u\n", max_total_size );
	
	rt_show_printf( u, "*-----------------------------------------------------------------------*\n" );
	
	rt_show_printf_end(u, 1);
	
	return RIP_SUCCESS;
}

/*===========================================================
函数名:      rip_cmd_show_ip_rip
函数功能: show ip rip命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_CMD_FAIL
备注:
=============================================================*/
int rip_cmd_show_ip_rip( int argc, char *argv[], struct user *u )
{
	int ret;
	struct rip_process_list_ *pprocess_list;
	
	if(argc == 1) 
	{
		ret = cmdend(argc - 1, argv + 1, u);
		if(ret != 0) 
		{
			return ret;
		}

		/*显示show ip rip命令信息*/
		for(pprocess_list = rip_process_list.forw ; pprocess_list != &(rip_process_list) ; pprocess_list = pprocess_list->forw)
		    rip_show_ip_rip_process( u , pprocess_list->rip_process_info);
	}
	else 
	{
		return subcmd(rip_cmds_show_rip_process_tab, NULL, argc, argv, u);
	}
	
	return RIP_SUCCESS;
}

/*===========================================================
函数名:      rip_cmd_show_data
函数功能: show ip rip database命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_CMD_FAIL
备注:
=============================================================*/
int rip_cmd_show_ip_rip_process_data( int argc, char *argv[], struct user *u )
{
	int ret;
	struct parameter param;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	param.type = ARG_UINT;
	param.min = 1;
	param.max = (unsigned long) 65535;
	param.flag = ARG_MIN | ARG_MAX ;

	ret = getparameter(argc + 2, argv - 2, u, &param);
	if (ret != 0) {
		return ret;
	}

	processid = param.value.v_int;

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret)
		return RIP_SUCCESS;

	rip_show_ip_rip_process_database( u, pprocess->rip_table);
	
	return RIP_SUCCESS;
}
int rip_cmd_show_process(int argc ,char **argv ,struct user *u)
{
	int rc;
	unsigned long processid;
	struct parameter param;
	struct rip_process_info_ *pprocess;
	

	if(argc == 1) 
	{
		param.type = ARG_UINT;
		param.min = 1;
		param.max = (unsigned long) 65535;
		param.flag = ARG_MIN | ARG_MAX ;

		rc = getparameter(argc + 1, argv - 1, u, &param);
		if (rc != 0) {
			return rc;
		}

		processid = param.value.v_int;

		rc = rip_lookup_process_byprocessid(processid, &pprocess);
		if(!rc)
			return RIP_SUCCESS;
		
		rip_show_ip_rip_process(u, pprocess);
	} 
	else 
	{
		return subcmd(rip_cmds_show_ip_rip_process_tab, NULL, argc, argv, u);
	}

	return 0;
}

#if 0
/*===========================================================
函数名:      rip_cmd_show_data_vrf
函数功能: show ip rip database vrf命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_CMD_FAIL
备注:
=============================================================*/
int rip_cmd_show_data_vrf( int argc, char *argv[], struct user *u )
{
	return subcmd(rip_cmds_show_database_vrf_tab, NULL, argc, argv, u);
}

/*===========================================================
函数名:      rip_cmd_show_data_vrf_name
函数功能: show ip rip database vrf vrf-name命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_CMD_FAIL
备注:
=============================================================*/
int rip_cmd_show_data_vrf_name( int argc, char *argv[], struct user *u )
{
	int ret;
	uint32 vrf_id;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}
	
	if( strlen(*argv) > RIP_VRF_NAME_LEN )
	{
		cmderror( ERR_RANGE, *argv, u );
		return RIP_FAIL;
	}

	vrf_id = vrf_GetIDbyName( *argv );
	if( vrf_id == 0 )
	{
		if( IsChinese(u) )
		{
			vty_output( "%%错误, VRF '%s' 不存在或没有配置RD.\n", *argv );
		}
		else
		{
			vty_output( "%%Err, VRF '%s' does not exist or does not have a RD.\n", *argv );
		}
		
		return RIP_FAIL;
	}
	
	if( (NULL == rip_tbl_array[vrf_id]) || (RIP_ENABLE != rip_tbl_array[vrf_id]->enable) )
	{
		rt_show_printf(u, "RIP VRF %s protocol: Disabled \n", *argv);
		rt_show_printf_end(u, 1);
		
		return RIP_SUCCESS;
	}

	rip_show_database( u, rip_tbl_array[vrf_id] );
	
	return RIP_SUCCESS;
}

/*===========================================================
函数名:      rip_cmd_show_proto_vrf
函数功能: show ip rip protocol vrf 命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_cmd_show_proto_vrf( int argc, char *argv[], struct user *u )
{
	return subcmd(rip_cmds_show_proto_vrf_tab, NULL, argc, argv, u);
}

/*===========================================================
函数名:      rip_cmd_show_proto_vrf_name
函数功能: show ip rip protocol vrf vrf-name命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_cmd_show_proto_vrf_name( int argc, char *argv[], struct user *u )
{
	int ret;
	uint32 vrf_id;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}
	
	if( strlen(*argv) > RIP_VRF_NAME_LEN )
	{
		cmderror( ERR_RANGE, *argv, u );
		return RIP_FAIL;
	}
	
	vrf_id = vrf_GetIDbyName( *argv );
	if( vrf_id == 0 )
	{
		if( IsChinese(u) )
		{
			vty_output( "%%错误, VRF '%s' 不存在或没有RD.\n", *argv );
		}
		else
		{
			vty_output( "%%Err, VRF '%s' does not exist or does not have a RD.\n", *argv );
		}
		
		return RIP_FAIL;
	}
	
	if( (NULL == rip_tbl_array[vrf_id]) || (RIP_ENABLE != rip_tbl_array[vrf_id]->enable) )
	{
		rt_show_printf(u, "RIP VRF %s protocol: Disabled \n", *argv);
		rt_show_printf_end(u, 1);
		
		return RIP_SUCCESS;
	}

	rip_show_protocol( u, rip_tbl_array[vrf_id] );
	
	return RIP_SUCCESS;
}
#endif
/*===========================================================
函数名:      rip_cmd_show_proto
函数功能: show ip rip protocol命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_cmd_show_ip_rip_process_proto( int argc, char *argv[], struct user *u )
{
	int ret;	
	struct parameter param;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	param.type = ARG_UINT;
	param.min = 1;
	param.max = (unsigned long) 65535;
	param.flag = ARG_MIN | ARG_MAX ;

	ret = getparameter(argc + 2, argv - 2, u, &param);
	if (ret != 0) {
		return ret;
	}

	processid = param.value.v_int;

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret)
		return RIP_SUCCESS;
	
	rip_show_ip_rip_process_protocol(u , pprocess);

	return RIP_SUCCESS;
}


/*===========================================================
函数名:      rip_cmd_show_summary
函数功能: show ip rip summary命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_cmd_show_ip_rip_process_summary( int argc, char *argv[], struct user *u )
{
	int ret;
	struct parameter param;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	param.type = ARG_UINT;
	param.min = 1;
	param.max = (unsigned long) 65535;
	param.flag = ARG_MIN | ARG_MAX ;

	ret = getparameter(argc + 2, argv - 2, u, &param);
	if (ret != 0) {
		return ret;
	}

	processid = param.value.v_int;

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret)
		return RIP_SUCCESS;
	
	rt_show_printf( u, "*------------------- RIP Process %d Summary Statistic ------------------*\n" ,pprocess->process_id);

	if( 0 == pprocess->vrf_id)
	{
		rt_show_printf( u, "RIP route table:\n" );
	}
	else
	{
#ifdef MULTICORE
		char vrf_name[VRF_NAME_LEN] = {'\0'};
		vrf_GetNamebyID(pprocess->vrf_id,vrf_name);
		rt_show_printf( u, "RIP vrf %s route table:\n", vrf_name );
#else 
		rt_show_printf( u, "RIP vrf %s route table:\n", vrf_GetNamebyID(pprocess->vrf_id) );
#endif
	}
	/*xuhaiqing 2010-12-21号将最大路由条目数限制取消*/
	/*rt_show_printf( u, "%-2s%-22s:%d\n", " ", "Maximum route number", pprocess->max_route_num );*/
	rt_show_printf( u, "%-2s%-22s:%d\n", " ", "Total route number", pprocess->route_num );
	
	rt_show_printf( u, "%-4s%-27s:%d\n", " ", "Connect route number", pprocess->connect_route_num );
	rt_show_printf( u, "%-4s%-27s:%d\n", " ", "Summary route number", pprocess->sum_route_num);
	rt_show_printf( u, "%-4s%-27s:%d\n", " ", "Learn route number", pprocess->nbr_route_num );
	rt_show_printf( u, "%-4s%-27s:%d\n\n", " ", "Redistributed route number", pprocess->redis_route_num );

	rt_show_printf( u, "%-2s%-22s:%d\n", " ", "Holddown route number", pprocess->holddown_route_num );

	rt_show_printf( u, "*-----------------------------------------------------------*\n" );

	rt_show_printf_end(u, 1);
	
	return RIP_SUCCESS;
}

/*===========================================================
函数名:      rip_cmd_show_intf
函数功能: show ip rip interface命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_cmd_show_ip_rip_process_intf( int argc, char *argv[], struct user *u )
{
	int ret;
	uint32 processid;
	struct parameter param;
	struct rip_process_info_ *pprocess;

	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	param.type = ARG_UINT;
	param.min = 1;
	param.max = (unsigned long) 65535;
	param.flag = ARG_MIN | ARG_MAX ;

	ret = getparameter(argc + 2, argv - 2, u, &param);
	if (ret != 0) {
		return ret;
	}

	processid = param.value.v_int;

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret)
		return RIP_SUCCESS;
	
	rip_show_ip_rip_process_intf(u , pprocess);
	
	return RIP_SUCCESS;
}

int rip_cmd_show_ip_rip_process_peer( int argc, char *argv[], struct user *u )
{
	int ret;
	uint32 processid;
	struct parameter param;
	struct rip_process_info_ *pprocess;
	

	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	param.type = ARG_UINT;
	param.min = 1;
	param.max = (unsigned long) 65535;
	param.flag = ARG_MIN | ARG_MAX ;

	ret = getparameter(argc + 2, argv - 2, u, &param);
	if (ret != 0) {
		return ret;
	}

	processid = param.value.v_int;
	
	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret)
		return RIP_SUCCESS;
	
	rip_show_ip_rip_process_peer( u ,pprocess);
		
	return RIP_SUCCESS;
}

/*===========================================================
函数名:      rip_show_running
函数功能: show running命令解析
输入参数: device_index : 逻辑端口号
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_FAIL
备注:
=============================================================*/
INT32 rip_show_running( DEVICE_ID device_index )
{
	if( device_index == 0 )
	{
		/*show running-config*/
		rip_show_running_global();
	} 
	else
	{
		/*show running-config interface*/
		rip_show_running_intf(device_index);
	}
	
	return RIP_SUCCESS;
}

/*===========================================================
函数名:      rip_show_running_global
函数功能: show running命令显示的全局信息
输入参数: 无
输出参数: 无
返回值:      无
备注:
=============================================================*/
void rip_show_running_global(void)
{
	struct rip_neigh_list_ *rip_neigh_list;
	struct rip_process_list_ *pprocess_list;
	struct rip_process_info_ *pprocess;
    
    struct route_distance_list *rip_dis_lst = NULL;
	for( pprocess_list = rip_process_list.forw; pprocess_list != &rip_process_list; pprocess_list = pprocess_list->forw )
	{
		if( NULL == (pprocess = pprocess_list->rip_process_info))
		{
			continue;
		}

		rt_vty_printf("router rip %d ",pprocess->process_id);
		
		if( pprocess->vrf_id != 0 )
		{
#ifdef MULTICORE
			char vrf_name[VRF_NAME_LEN] = {'\0'};
			vrf_GetNamebyID(pprocess->vrf_id,vrf_name);
			rt_vty_printf(" vrf %s\n", vrf_name );
#else
			rt_vty_printf(" vrf %s\n", vrf_GetNamebyID(pprocess->vrf_id) );
#endif
		}
		else rt_vty_printf("\n");

		if( RIP_VERSION_1 == pprocess->version )
		{
			rt_vty_printf(" version 1\n");
		}
		else if( RIP_VERSION_2 == pprocess->version )
		{
			rt_vty_printf(" version 2\n");
		}
		
		if( RIP_AUTO_SUMMARY_DISABLE == pprocess->auto_summary_flag )
		{
			rt_vty_printf(" no auto-summary\n");
		}

		if( RIP_DEF_ROUTE_ENABLE == pprocess->default_route_flag )
		{
			rt_vty_printf(" default-information originate\n");
		}
		else if( RIP_DEF_ROUTE_ENABLE_SAFE == pprocess->default_route_flag )
		{
			rt_vty_printf(" default-information originate-safe\n");
		}
		
		if( RIP_DEF_METRIC != pprocess->default_metric )
		{
			rt_vty_printf( " default-metric %ld\n", pprocess->default_metric );
		}
        
		rip_dis_lst = pprocess->distance_list;        
		route_distance_config_write(rip_dis_lst);

		/*offset*/
		route_offset_list_config_write( pprocess->offset_list );

		/*filter */
		route_distribute_list_config_write( pprocess->filter_list );

		/*xuhaiqing 2010-12-21号将最大路由条目数限制取消*/	
		/*if( pprocess->max_route_num != RIP_MAX_ROUTE_NUM )
		{
			rt_vty_printf(" maximum-routes %u\n", pprocess->max_route_num );
		}*/

		if( pprocess->max_nh_num != RIP_MAX_NH_NUM)
		{
			rt_vty_printf(" maximum-nexthop %u\n", pprocess->max_nh_num );
		}
#if 0
		for( rip_net_list = pprocess->network_list.forw; rip_net_list != &(pprocess->network_list); rip_net_list = rip_net_list->forw )
		{
			rt_vty_printf(" network");
			if( 0 != rip_net_list->mask )
			{
				rt_vty_printf( " %s", ip_ntoa(rip_net_list->network) );
				rt_vty_printf( " %s\n", ip_ntoa(rip_net_list->mask) );
			}
			else
			{
				rt_vty_printf( " %s\n", ip_ntoa(rip_net_list->network) );
			}
		}
#endif
		/*redistribute*/
		show_redistribute_running_ptr(  pprocess->vrf_id,pprocess->rip_redis_link );

/*		if( 0 == pprocess->vrf_id )
		{*/
		if( RIP_DEF_INPUT_QUEUE != rip_glb_info.input_queue )
		{
			rt_vty_printf( " input-queue %u\n", rip_glb_info.input_queue );
		}
		
		for( rip_neigh_list = pprocess->neigh_list.forw; rip_neigh_list != &(pprocess->neigh_list); rip_neigh_list = rip_neigh_list->forw )
		{
			rt_vty_printf(" neighbor %s\n", ip_ntoa(rip_neigh_list->neigh_addr) );
		}

		if( RIP_DEF_UPDATE_TIMEOUT != pprocess->update_timeout )
		{
			rt_vty_printf( " timers update %u\n", pprocess->update_timeout );
		}
		if( RIP_DEF_EXPIRE_TIMEOUT != pprocess->expire_timeout )
		{
			rt_vty_printf(" timers expire %u\n", pprocess->expire_timeout);
		}
		if( RIP_DEF_HOLDDOWN_TIMEOUT != pprocess->holddown_timeout )
		{
			rt_vty_printf(" timers holddown %u\n", pprocess->holddown_timeout );
		}
		if(RIP_DEF_TRIGGER_TIMEOUT != pprocess->trigger_timeout)
		{
			rt_vty_printf(" timers trigger %u\n",pprocess->trigger_timeout );
		}
		if( RIP_DEF_PEER_TIMEOUT != pprocess->peer_timeout)
		{
			rt_vty_printf(" timers peer %u\n", pprocess->peer_timeout );
		}

		if( !BIT_TEST(pprocess->flags ,RIP_SRC_CHECK_ENABLE))
		{
			rt_vty_printf(" no validate-update-source\n");
		}
		
		if( !BIT_TEST(pprocess->flags ,RIP_ZERO_DOMAIN_CHECK_ENABLE))
		{
			rt_vty_printf(" no check-zero-domain\n");
		}			
/*		}*/
		
		rt_vty_printf(" \n");
	}
	
	return;
}
/*===========================================================
函数名:      rip_show_running_intf
函数功能: show running命令显示的端口下的RIP信息
输入参数: 无
输出参数: 无
返回值:      无
备注:
=============================================================*/
void rip_show_running_intf( uint32 device_index )
{
	struct dynamic_key_list_ *key;
	u_char md5_temp[34];
	int i;
	
	if( rip_intf_array[device_index] == NULL )
	{
		return;
	}
	
	/*interface authenticate */
	switch(rip_intf_array[device_index]->auth_type )
	{
		case RIP_AUTH_SIMPLE:
			rt_vty_printf(" ip rip authentication simple\n");
			break;
		case RIP_AUTH_MD5:
			if(!rip_intf_array[device_index]->dynamicflag)
				rt_vty_printf( " ip rip authentication md5\n" );
			else
				rt_vty_printf( " ip rip authentication dynamic\n" );
			break;
		default:
			break;
	}

	if(rip_intf_array[device_index]->process_id != 0)
		rt_vty_printf( " ip rip %d enable\n", rip_intf_array[device_index]->process_id);
	
	if( 0 != strlen(rip_intf_array[device_index]->simple_key) )
	{
		rt_vty_printf( " ip rip password %s\n", rip_intf_array[device_index]->simple_key );
	}
	
	if( 0 != strlen(rip_intf_array[device_index]->md5_key) )
	{
		if(um_PassEncryptFlags())
		{
			rt_vty_printf( " ip rip md5-key %d md5 ", rip_intf_array[device_index]->md5_keyid);
			memset(md5_temp, 0, 34);
			um_pass_encrypt((u_char *)rip_intf_array[device_index]->md5_key, (u_char *)md5_temp);
			rt_vty_printf("7 %02d", md5_temp[0]);
			for (i = 1; md5_temp[i] !=0; i++)
			{
				rt_vty_printf("%02x", md5_temp[i]);
			}
			rt_vty_printf("\n");		
		}
		else
			rt_vty_printf( " ip rip md5-key %d md5 0 %s\n", rip_intf_array[device_index]->md5_keyid, rip_intf_array[device_index]->md5_key );
	}

	key = rip_intf_array[device_index]->key_list.forw;
	while(key != &rip_intf_array[device_index]->key_list)
	{
		rt_vty_printf(" ip rip dynamic-key" );
		rt_vty_printf(" %-3d",key->key_id);
		if(key->algorithms == RIP_NEIGHB_MD5)
		{
			rt_vty_printf(" md5 ");
			if(um_PassEncryptFlags())
			{
				memset(md5_temp, 0, 34);
				um_pass_encrypt((u_char *)key->key, (u_char *)md5_temp);
				rt_vty_printf("7 %02d", md5_temp[0]);
				for (i = 1; md5_temp[i] !=0; i++)
				{
					rt_vty_printf("%02x", md5_temp[i]);
				}
			}
			else rt_vty_printf("0 %-19s",key->key);	
			rt_vty_printf(" ");
		}
		else
		{
			rt_vty_printf(" sha1");
			rt_vty_printf(" %-19s ",key->key);
		}
		rt_vty_printf("%4d-%d-%d-%d:%d",key->start_time.year,key->start_time.month,key->start_time.day,key->start_time.hour,key->start_time.minute);
		rt_vty_printf(" %d:%d\n",key->lift_time.hour,key->lift_time.minute);

		key = key->forw;
	}

	key = rip_intf_array[device_index]->key_timeout_list.forw;
	while(key != &rip_intf_array[device_index]->key_timeout_list)
	{
		if(key ==rip_intf_array[device_index]->key_timeout_list.forw)
			rt_vty_printf("\n");
		rt_vty_printf(" ip rip dynamic-key" );
		rt_vty_printf(" %-3d",key->key_id);
		if(key->algorithms == RIP_NEIGHB_MD5)
		{
			rt_vty_printf(" md5 ");
			if(um_PassEncryptFlags())
			{
				memset(md5_temp, 0, 34);
				um_pass_encrypt((u_char *)key->key, (u_char *)md5_temp);
				rt_vty_printf("7 %02d", md5_temp[0]);
				for (i = 1; md5_temp[i] !=0; i++)
				{
					rt_vty_printf("%02x", md5_temp[i]);
				}
			}
			else rt_vty_printf("0 %-19s",key->key);
			rt_vty_printf(" ");
		}
		else
		{
			rt_vty_printf(" sha1");
			rt_vty_printf(" %-19s ",key->key);
		}
		rt_vty_printf("%4d-%d-%d-%d:%d",key->start_time.year,key->start_time.month,key->start_time.day,key->start_time.hour,key->start_time.minute);
		rt_vty_printf(" %d:%d\n",key->lift_time.hour,key->lift_time.minute);

		key =key->forw;
	}			
	/* 端口enable bfd*/
	if( rip_intf_array[device_index]->bfd_enable_flag == 1)
	{
		rt_vty_printf(" ip rip bfd-enable\n");
	}
	/*interface passive */
	if( BIT_TEST(rip_intf_array[device_index]->special_flag,RIP_PASSIVE_ENABLE))
	{
		rt_vty_printf(" ip rip passive\n");
	}

	if( BIT_TEST(rip_intf_array[device_index]->special_flag,RIP_SEND_V1DEMAND))
	{
		rt_vty_printf(" ip rip v1demand\n");
	}

	if( BIT_TEST(rip_intf_array[device_index]->special_flag,RIP_SEND_V2DEMAND))
	{
		rt_vty_printf(" ip rip v2demand\n");
	}

	if( BIT_TEST(rip_intf_array[device_index]->special_flag,RIP_NOTRECV_ENABLE))
	{
		rt_vty_printf(" ip rip deaf\n");
	}

	/*interface receive version */
 	if( RIP_RECV_VERSION_1_2 == rip_intf_array[device_index]->recv_version )
	{
		rt_vty_printf(" ip rip receive version 1 2\n");
	}
 	else if( RIP_RECV_VERSION_1 == rip_intf_array[device_index]->recv_version )
 	{
		rt_vty_printf(" ip rip receive version 1\n");
 	}
 	else if( RIP_RECV_VERSION_2 == rip_intf_array[device_index]->recv_version )
 	{
		rt_vty_printf(" ip rip receive version 2\n");
	}

	/*interface send version */
 	if( RIP_SEND_VERSION_1 == rip_intf_array[device_index]->send_version )
	{
		rt_vty_printf(" ip rip send version 1\n");
	}
 	else if( RIP_SEND_VERSION_2_MULTICAST == rip_intf_array[device_index]->send_version )
 	{
		rt_vty_printf(" ip rip send version 2\n");
 	}
 	else if( RIP_SEND_VERSION_2_BROADCAST== rip_intf_array[device_index]->send_version )
 	{
		rt_vty_printf(" ip rip send version compatibility\n");
	}

	if(rip_intf_array[device_index]->split_flag & RIP_POISONED_SPLIT_HORIZON_ENABLE)
	{
		rt_vty_printf(" ip rip split-horizon poisoned\n");
	}

	if(!(rip_intf_array[device_index]->split_flag & RIP_SIMPLE_SPLIT_HORIZON_ENABLE))
	{
		rt_vty_printf( "  no ip rip split-horizon simple\n");
	}
 		
	/*
	if( RIP_SIMPLE_SPLIT_HORIZON_DISABLE == rip_intf_array[device_index]->split_flag )
	{
		rt_vty_printf(" no ip rip split-horizon simple\n");
	}
	else if( RIP_SIMPLE_SPLIT_HORIZON_ENABLE == rip_intf_array[device_index]->split_flag )
	{
		rt_vty_printf(" ip rip split-horizon simple\n");
	}
	else if( RIP_POISONED_SPLIT_HORIZON_DISABLE == rip_intf_array[device_index]->split_flag )
	{
		rt_vty_printf(" no ip rip split-horizon poisoned\n");
	}
	else if( RIP_POISONED_SPLIT_HORIZON_ENABLE == rip_intf_array[device_index]->split_flag )
	{
		rt_vty_printf(" ip rip split-horizon poisoned\n");
	}
	*/
	return;
}

/*===========================================================
函数名:      rip_show_rip
函数功能: show ip rip命令解析
输入参数: u   : 命令用户
输出参数: 无
返回值:      无
备注:
=============================================================*/
void rip_show_ip_rip_process( struct user *u ,struct rip_process_info_ *pprocess)
{
	struct rip_neigh_list_ *rip_neigh_list;
	struct rip_process_intf_list_ *pintf_list;
	struct rip_intf_ *pintf;

        if(! pprocess)
		return;

	rt_show_printf( u, "Process: %u", pprocess->process_id);
	if(pprocess->vrf_id)
	{
#ifdef MULTICORE
			char vrf_name[VRF_NAME_LEN] = {'\0'};
			vrf_GetNamebyID(pprocess->vrf_id,vrf_name);
			
			rt_show_printf(u," ,vrf %s(%d)\n", vrf_name ,pprocess->vrf_id);
#else
			rt_show_printf(u," ,vrf %s(%d)\n", vrf_GetNamebyID(pprocess->vrf_id) ,pprocess->vrf_id);
#endif
	}
	else
		rt_show_printf(u," \n");

	/*RIP定时器时间*/
	rt_show_printf( u, " Update: %u,", pprocess->update_timeout );
	rt_show_printf( u, "  Expire: %u,", pprocess->expire_timeout );
	rt_show_printf( u, "  Holddown: %u\n", pprocess->holddown_timeout );

	/*Input queue*/
	rt_show_printf( u, " Input-queue: %u\n", rip_glb_info.input_queue );

	/*源地址验证*/
	if( !BIT_TEST(pprocess->flags ,RIP_SRC_CHECK_ENABLE)) 
	{
		/*源地址验证禁止*/
		rt_show_printf( u, " Validate-update-source: Disable\n" );
	}
	else
	{
		/*源地址验证激活*/
		rt_show_printf( u, " Validate-update-source: Enable\n" );
	}

	if( !BIT_TEST(pprocess->flags ,RIP_ZERO_DOMAIN_CHECK_ENABLE)) 
	{
		/*源地址验证禁止*/
		rt_show_printf( u, " zero-domain-check: Disable\n" );
	}
	else
	{
		/*源地址验证激活*/
		rt_show_printf( u, " zero-domain-check: Enable\n" );
	}

	/*邻居信息*/
	rt_show_printf( u, " Neighbor List:\n" );
	if( pprocess->neigh_list.forw != &(pprocess->neigh_list) )
	{
		for( rip_neigh_list = pprocess->neigh_list.forw; rip_neigh_list != &(pprocess->neigh_list); rip_neigh_list = rip_neigh_list->forw )
		{
			rt_show_printf( u, "  neighbor %s\n", ip_ntoa(rip_neigh_list->neigh_addr) );
		}
	}

	/*进程使能的接口*/
	rt_show_printf( u, " interface List:\n" );
	for(pintf_list = pprocess->intf_list.forw ; pintf_list != &(pprocess->intf_list) ; pintf_list = pintf_list->forw)
	{
		pintf = pintf_list->pintf;
		rt_show_printf( u, "  interface %s\n",pintf->intf_name);
	}
	rt_show_printf( u, " \n" );
	rt_show_printf_end(u, 1);

	return;
}

/*===========================================================
函数名:      rip_show_protocol
函数功能: show ip rip protocol命令解析
输入参数: u   : 命令用户
                       rip_tbl_info : 待显示信息的RIP路由表
输出参数: 无
返回值:      无
备注:
=============================================================*/
void rip_show_ip_rip_process_protocol( struct user *u, struct rip_process_info_ *pprocess )
{
	int send_version, recv_version;
	struct route_distance_list *rip_dis_lst = NULL;
	struct rip_process_intf_list_ *pintf_list;
	struct rip_intf_ *pintf;
	
	if(pprocess->vrf_id)
	{
#ifdef MULTICORE
		char vrf_name[VRF_NAME_LEN] = {'\0'};
		vrf_GetNamebyID(pprocess->vrf_id,vrf_name);
		rt_show_printf(u, "RIP %d (vrf %s) is Active \n",pprocess->process_id, vrf_name);
#else
		rt_show_printf(u, "RIP %d (vrf %s) is Active \n", pprocess->process_id,vrf_GetNamebyID(pprocess->vrf_id) );
#endif
	}
	else
	{
		rt_show_printf(u, "RIP %d is Active \n", pprocess->process_id);
	}
	
	
	rt_show_printf(u, "update interval %d(s), Invalid interval %d(s)\n", \
	            pprocess->update_timeout, pprocess->expire_timeout);
	rt_show_printf(u, "Holddown interval %d(s), Trigger interval %d(s), peer interval %d(s) \n", \
	            pprocess->holddown_timeout, pprocess->trigger_timeout,pprocess->peer_timeout);    

	if(RIP_VERSION_2 ==pprocess->version && 
		RIP_AUTO_SUMMARY_DISABLE == pprocess->auto_summary_flag)
	{
		rt_show_printf(u, "Automatic network summarization: Disable\n");
	}
	else  rt_show_printf(u, "Automatic network summarization: Enable\n");
	
#if 0
	/*Network list*/
	rt_show_printf(u, "Network List:\n");
	for(rip_net_list = pprocess->network_list.forw; rip_net_list != &(pprocess->network_list); rip_net_list = rip_net_list->forw )
	{
		rt_show_printf(u, "%-2s", " " );
		
		rt_show_printf(u, "network %s", ip_ntoa(rip_net_list->network) );
		if( 0 == rip_net_list->mask )
		{
			rt_show_printf( u, "\n" );
		}
		else
		{
			rt_show_printf( u, " %s\n", ip_ntoa(rip_net_list->mask) );
		}
	}
#endif
	/*Filter */
	rt_show_printf(u, "Filter list:\n" );
	route_distribute_list_config_write( pprocess->filter_list );

	/*Offset*/
	rt_show_printf(u, "Offset list:\n" );
	route_offset_list_config_write( pprocess->offset_list );

	/*Redistribute*/
	rt_show_printf(u, "Redistribute policy:\n" );
	show_redistribute_running_ptr( pprocess->vrf_id, pprocess->rip_redis_link );

	/*Interface*/
	rt_show_printf(u, "Interface send version and receive version:\n" );
	if( RIP_VERSION_DEF == pprocess->version )
	{
		rt_show_printf(u, "Global version : default\n" );
	}
	else if( RIP_VERSION_1 == pprocess->version )
	{
		rt_show_printf(u, "Global version : V1\n" );
	}
	else if( RIP_VERSION_2 == pprocess->version )
	{
		rt_show_printf(u, "Global version : V2\n" );
	}

	rt_show_printf(u, "%-2s%-27s%-15s%-15s%s\n", " ", "Interface", "Send-version", "Recv-version", "Nbr_number" );
	for( pintf_list = pprocess->intf_list.forw; pintf_list != &(pprocess->intf_list); pintf_list = pintf_list->forw)
	{
		if(!(pintf = pintf_list->pintf))
		{
			/*syslog(LOG_WARNING,"RIP:%s %d error\n",__FILE__,__LINE__);*/
			return ;
		}
		
		rt_show_printf(u, "%-2s", " " );
		send_version = rip_set_send_version( pintf->device_index );
		switch( send_version )
		{
			case RIP_SEND_VERSION_1:
				rt_show_printf(u, "%-27s%-15s", pintf->intf_name, "V1" );
				break;
			case RIP_SEND_VERSION_2_BROADCAST:
			case RIP_SEND_VERSION_2_MULTICAST:
			case RIP_SEND_VERSION_DEF:
				rt_show_printf(u, "%-27s%-15s", pintf->intf_name, "V2" );
				break;
			default:
				rt_show_printf(u, "%-27s%-15s", pintf->intf_name, "*" );
				break;
		}

		recv_version = rip_get_recv_version( pintf->device_index  );
		switch( recv_version )
		{
			case RIP_RECV_VERSION_1:
				rt_show_printf(u, "%-15s", "V1" );
				break;
			case RIP_RECV_VERSION_DEF:
			case RIP_RECV_VERSION_1_2:
				rt_show_printf(u, "%-15s", "V1 V2" );
				break;
			case RIP_RECV_VERSION_2:
				rt_show_printf(u, "%-15s", "V2" );
				break;
			default:
				rt_show_printf(u, "%-15s", "*" );
				break;
		}

		rt_show_printf(u, "%d\n", pintf->nbr_route_num );
	}


	rt_show_printf(u, "Distance: %ld (default is 120):\n", pprocess->distance_list->default_distance );
	rip_dis_lst = pprocess->distance_list; 
	/*Fengsb 2006-04-30 distance configure output*/
	route_distance_config_write(rip_dis_lst);    

	/*xuhaiqing 2010-12-21号将最大路由条目数限制取消*/
	/*rt_show_printf( u, "Maximum route count: %ld,\tCurrent route count:%ld \n", pprocess->max_route_num, pprocess->route_num );*/
	rt_show_printf( u, "Current route count:%ld \n", pprocess->route_num );

	rt_show_printf_end(u, 1);

	return;
}

/*===========================================================
函数名:      rip_show_database
函数功能: show ip rip database命令解析
输入参数: u   : 命令用户
                       rip_tbl_info : 待显示信息的RIP路由表
输出参数: 无
返回值:      无
备注:
=============================================================*/
void rip_show_ip_rip_process_database( struct user *u, struct route_table *rip_table )
{
	struct route_node *rip_route_node;
	struct rip_route_ *rip_route, *rip_route_head;
	char string[100];
	int count;
	
	for( rip_route_node = route_top(rip_table); rip_route_node != NULL; rip_route_node = route_next(rip_route_node))
	{
		rip_route_head = rip_route_node->info;
		if( NULL == rip_route_head )
		{
			continue;
		}
		
		for( count = 1, rip_route = rip_route_head; count <= rip_route_head->equi_route_num; rip_route = rip_route->forw, count++ )
		{
			if( RIP_CONNECT_ROUTE == rip_route->route_type )
			{
				sprintf(string, "%s/%d", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
				rt_show_printf( u, " %-19sdirectly connected  %s\n", string, rip_intf_array[rip_route->gw_index]->intf_name );
			}
			else if( RIP_REDIS_ROUTE == rip_route->route_type )
			{
				sprintf(string, "%s/%d", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
				rt_show_printf(u, " %-19sredistributed\n", string );
			}
			else if( RIP_NBR_ROUTE == rip_route->route_type )
			{
				sprintf(string, "%s/%d", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
				rt_show_printf(u, " %-19s[%d,%d] ", string, rip_route->distance, rip_route->metric );

				if(rip_route->next_hop != 0)
				{
					rt_show_printf(u, " via %s", ip_ntoa(rip_route->next_hop) );
				}
				else
				{
					rt_show_printf(u, " via %s", ip_ntoa(rip_route->gw_addr) );
				}
				
				if( rip_route->metric == RIP_MAX_METRIC )
				{
					rt_show_printf(u, " holddown", ip_ntoa(rip_route->next_hop) );
				}

				rt_show_printf(u, " (on %s) ", rip_intf_array[rip_route->gw_index]->intf_name );

				rt_show_printf( u, " %-9s\n", rip_print_time( time_sec - rip_route->refresh_time) );
			}
			else if(RIP_SUMMARY_ROUTE == rip_route->route_type)
			{
				sprintf(string, "%s/%d", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
				rt_show_printf( u, " %-19sauto-summary\n", string );
			}
			else if(RIP_DEF_ROUTE == rip_route->route_type)
			{
				sprintf(string, "%s/%d", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
				rt_show_printf( u, " %-19sredistributed(D)\n", string );
			}
		}
	}
	
	rt_show_printf_end(u, 1);

	return ;
}

/*===========================================================
函数名:      rip_show_intf
函数功能: show ip rip interface命令解析
输入参数: u   : 命令用户
                       rip_tbl_info : 待显示信息的RIP路由表
输出参数: 无
返回值:      无
备注:
=============================================================*/
void rip_show_ip_rip_process_intf( struct user *u, struct rip_process_info_ *pprocess )
{
	struct dynamic_key_list_ *key;
	struct rip_process_intf_list_ *pintf_list;
	struct rip_intf_ *pintf;
	

	for( pintf_list = pprocess->intf_list.forw; pintf_list != &(pprocess->intf_list) ; pintf_list = pintf_list->forw )
	{
		pintf = pintf_list->pintf;
		rt_show_printf( u, "Interface %s", pintf->intf_name );
		if(pintf->vrf_id==0)
			rt_show_printf( u, "\n" );
		else
		{
#ifdef MULTICORE
			char vrf_name[VRF_NAME_LEN] = {'\0'};
			vrf_GetNamebyID(pintf->vrf_id,vrf_name);
			
			rt_show_printf(u," ,vrf %s(%d)\n", vrf_name ,pintf->vrf_id);
#else
			rt_show_printf(u," ,vrf %s(%d)\n", vrf_GetNamebyID(pintf->vrf_id) ,pintf->vrf_id);
#endif
		}

		rt_show_printf( u, "%-2sAddress:%s", " ", ip_ntoa(pintf->address) );
		rt_show_printf( u, ", mask:%s\n", ip_ntoa(pintf->mask) );
	
		if(BIT_TEST(pintf->state , RIP_INTF_PROCESS_ACTIVE))
			rt_show_printf( u, "%-2sstate:active\n"," ");
		else
		{
			rt_show_printf( u, "%-2svrfid %d,status %s,\n"," ",pintf->vrf_id ,(BIT_TEST(pintf->state, RIP_INTF_LINK_UP)?"up":"down"));
		}

		switch(pintf->send_version )
		{
			case RIP_SEND_VERSION_1:
				rt_show_printf( u, "%-2sSend version: V1\n", " " );
				break;
			case RIP_SEND_VERSION_2_BROADCAST:
				rt_show_printf( u, "%-2sSend version: V2 broadcast\n", " " );
				break;
			case RIP_SEND_VERSION_2_MULTICAST:
				rt_show_printf( u, "%-2sSend version: V2 multicast\n", " " );
				break;
			case RIP_SEND_VERSION_DEF:
				rt_show_printf( u, "%-2sSend version: V1(default)\n", " " );
				break;
			default:
				break;
		}

		switch( pintf->recv_version )
		{
			case RIP_RECV_VERSION_1:
				rt_show_printf( u, "%-2sReceive version: V1\n", " " );
				break;
			case RIP_RECV_VERSION_2:
				rt_show_printf( u, "%-2sReceive version: V2\n", " " );
				break;
			case RIP_RECV_VERSION_1_2:
				rt_show_printf( u, "%-2sReceive version: V1 and V2\n", " " );
				break;
			case RIP_RECV_VERSION_DEF:
				rt_show_printf( u, "%-2sReceive version: V1 and V2(default)\n", " " );
				break;
			default:
				break;
		}

		if( BIT_TEST(pintf->special_flag,RIP_PASSIVE_ENABLE))
		{
			rt_show_printf( u, "%-2sPassive: Enable\n", " " );
		}
		else
		{
			rt_show_printf( u, "%-2sPassive: Disable\n", " " );
		}

		if( BIT_TEST(pintf->special_flag,RIP_SEND_V1DEMAND))
		{
			rt_show_printf( u, "%-2sv1demand: Enable\n", " " );
		}
		else
		{
			rt_show_printf( u, "%-2sv1demand: Disable\n", " " );
		}

		if( BIT_TEST(pintf->special_flag,RIP_SEND_V2DEMAND))
		{
			rt_show_printf( u, "%-2sv2demand: Enable\n", " " );
		}
		else
		{
			rt_show_printf( u, "%-2sv2demand: Disable\n", " " );
		}

		if( BIT_TEST(pintf->special_flag,RIP_NOTRECV_ENABLE))
		{
			rt_show_printf( u, "%-2sdeaf: Enable\n", " " );
		}
		else
		{
			rt_show_printf( u, "%-2sdeaf: Disable\n", " " );
		}
		

		rt_show_printf( u, "%-2sAuthentication type: ", " " );
		switch( pintf->auth_type )
		{
			case RIP_AUTH_MD5:
				if(!pintf->dynamicflag)
					rt_show_printf( u, "MD5\n" );
				else
					rt_show_printf( u, "dynamic\n" );
				break;
			case RIP_AUTH_SIMPLE:
				rt_show_printf( u, "simple\n" );
				break;
			default:
				rt_show_printf( u, "NULL\n" );
				break;
		}

		if( 0 != strlen(pintf->md5_key) )
		{
			rt_show_printf( u, "%-2sMD5 authentication key-id:%d, key: %s\n", " ", pintf->md5_keyid,pintf->md5_key );
		}
		else
		{
			rt_show_printf( u, "%-2sMD5 authentication key: NULL\n", " " );
		}
		if( 0 != strlen(pintf->simple_key) )
		{
			rt_show_printf( u, "%-2sSimple password: %s\n", " ", pintf->simple_key );
		}
		else
		{
			rt_show_printf( u, "%-2sSimple password: NULL\n", " " );
		}

	key = pintf->key_list.forw;
	while(key != &pintf->key_list)
	{
		if(key ==pintf->key_list.forw)
			rt_show_printf(u, " ip rip dynamic-key:\n" );
		rt_show_printf(u, "  %-3d",key->key_id);
		if(key->algorithms == RIP_NEIGHB_MD5)
			rt_show_printf(u, "  md5");
		else
			rt_show_printf(u, " sha1");
		rt_show_printf(u, "  %-18s",key->key);
		rt_show_printf(u, "%4d-%d-%d-%d:%d",key->start_time.year,key->start_time.month,key->start_time.day,key->start_time.hour,key->start_time.minute);
		rt_show_printf(u, "  %d:%d\n",key->lift_time.hour,key->lift_time.minute);

		key = key->forw;
	}

	key = pintf->key_timeout_list.forw;
	while(key != &pintf->key_timeout_list)
	{
		if(key ==pintf->key_timeout_list.forw)
			rt_show_printf(u, "\n");
		rt_show_printf(u, "  %-3d",key->key_id);
		if(key->algorithms == RIP_NEIGHB_MD5)
			rt_show_printf(u, " md5");
		else
			rt_show_printf(u, " sha1");
		rt_show_printf(u, "  %-18s",key->key);
		rt_show_printf(u, "%4d-%d-%d-%d:%d",key->start_time.year,key->start_time.month,key->start_time.day,key->start_time.hour,key->start_time.minute);
		rt_show_printf(u, "  %d:%d\n",key->lift_time.hour,key->lift_time.minute);

		key =key->forw;
	}			


		if(pintf->split_flag & RIP_POISONED_SPLIT_HORIZON_ENABLE)
		{
			rt_show_printf( u, "%-2sSplit-horizon-poisoned:enable\n", " " );
		}
		
		if(!(pintf->split_flag & RIP_SIMPLE_SPLIT_HORIZON_ENABLE))
		{
			rt_show_printf( u, "%-2sSplit-horizon-simple:disable\n", " " );
		}

	/* 端口enable bfd*/
	if(pintf->bfd_enable_flag == 1)
	{
		rt_vty_printf(" ip rip bfd-enable\n");
	}

		
              /*
		if( TRUE == rip_enable_simple_split(device_index) )
		{
			rt_show_printf( u, "%-2sSplit-horizon: simple\n", " " );
		}
		else if( TRUE == rip_enable_poisoned_split(device_index) )
		{
			rt_show_printf( u, "%-2sSplit-horizon: poisoned\n", " " );
		}
		else
		{
			rt_show_printf( u, "%-2sSplit-horizon: disabled\n", " " );
		}
		*/
	}

	rt_show_printf_end(u, 1);

	return;
}


/*******************************************************************************
 *
 * FUNCTION	:	rip_show_peer
 *
 * PARAMS		:	u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2009.10.13 10:40:52
 *
*******************************************************************************/
void rip_show_ip_rip_process_peer( struct user *u , struct rip_process_info_ *pprocess)
{
	struct rip_peer_list_ *rip_peer;

	rip_peer = pprocess->peer_list.forw;
	while(rip_peer != & pprocess->peer_list)
	{	
		rt_show_printf(u,"peer %s\n",ip_ntoa(rip_peer->peer_addr));
		rt_show_printf(u,"  rip2PeerLastUpdate :%d\n",rip_peer->rip2PeerLastUpdate);
		rt_show_printf(u,"  rip2PeerRcvPackets :%d\n",rip_peer->rip2PeerRcvPackets);//
		rt_show_printf(u,"  rip2PeerRcvBadPackets :%d\n",rip_peer->rip2PeerRcvBadPackets);
		rt_show_printf(u,"  rip2PeerRcvRoutes :%d\n",rip_peer->rip2PeerRcvRoutes);//
		rt_show_printf(u,"  rip2PeerRcvBadRoutes :%d\n",rip_peer->rip2PeerRcvBadRoutes);
		rt_show_printf(u,"  rip2PeerVersion :%d\n",rip_peer->rip2PeerVersion);
		rt_show_printf(u,"  total route number :%d\n",rip_peer->ref_num);

		rip_peer = rip_peer->forw;
	}
	
	/*add 端口收发报文打印*/
		struct rip_process_intf_list_ *rip_intf_list;
		struct rip_intf_ *rip_intf; //add
		rip_intf_list = pprocess->intf_list.forw;//add
		while((rip_intf_list != &pprocess->intf_list) && rip_intf_list->pintf)
		{
			rip_intf = rip_intf_list->pintf;
			rt_show_printf(u,"index %d name %s\n",rip_intf->device_index,rip_intf->intf_name);
		
			rt_show_printf(u,"	rip2IfStatRcvBadPackets :%d\n",rip_intf->rip2IfStatRcvBadPackets);
			rt_show_printf(u,"	rip2IfStatRcvBadRoutes :%d\n",rip_intf->rip2IfStatRcvBadRoutes);
			rt_show_printf(u,"	rip2IfStatSentUpdatePackets :%d\n",rip_intf->rip2IfStatSentUpdates);
			rt_show_printf(u,"	rip2IfStatRecvUpdatePackets :%d\n",rip_intf->rip2IfStatRecvUpdates);
			rip_intf_list = rip_intf_list->forw;
		}
	/*add end*/
		
	rt_show_printf_end(u, 1);

	return ;
}

/*===========================================================
函数名:      rip_print_time
函数功能: 打印时间
输入参数: time   : 时间
输出参数: 无
返回值:      返回打印的信息
备注:
=============================================================*/
char *rip_print_time( long time )
{
	time_t uptime = time;
	struct tm *tm;
	static char buf[25];

	tm = gmtime(&uptime);
	/* Making formatted timer strings. */
#define ONE_DAY_SECOND 60*60*24
#define ONE_WEEK_SECOND 60*60*24*7

	if (uptime < ONE_DAY_SECOND)
		snprintf(buf, 25, "%02d:%02d:%02d",
				 tm->tm_hour, tm->tm_min, tm->tm_sec);
	else if (uptime < ONE_WEEK_SECOND)
		snprintf(buf, 25, "%dd%02dh%02dm",
				 tm->tm_yday, tm->tm_hour, tm->tm_min);
	else
		snprintf(buf, 25, "%02dw%dd%02dh",
				 tm->tm_yday / 7, tm->tm_yday - ((tm->tm_yday / 7) * 7),
				 tm->tm_hour);
	return buf;
}

