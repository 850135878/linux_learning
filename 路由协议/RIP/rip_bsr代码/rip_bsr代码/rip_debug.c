char rip_debug_version[] = "rip_debug.c last changed at 2010.03.24 13:38:52 by dangzhw\n";
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
 *        FILE NAME       :   rip_debug.c
 *        AUTHOR          :   HQ.Sun
 *        DATE            :   2006.01.08 11:13:41
 *        PURPOSE:        :   RIP模块跟踪处理模块
 *
**************************************************************************/
#include <rip/rip_include.h>
#include <rip/rip_main.h>
#include <rip/rip_debug.h>

extern struct rip_glb_info_  rip_glb_info;

/*debug ip下的RIP命令*/
struct cmds rip_cmds_debug_ip_tab[] =
{
	{ 
		"rip", 
    	MATCH_AMB,
    	cmdPref(PF_CMDNO, 0, 0),
    	0, 
		rip_cmd_debug_rip,
		NULL,
		NULL,
		0,
		0, 
		"rip                    -- Debug RIP information",
		"rip                    -- 跟踪RIP信息", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ NULLCHAR }
};

/*RIP debug ip rip 命令*/
struct cmds rip_cmds_debug_rip_tab[] = 
{
	{ 
		"packet",
		MATCH_AMB,
		cmdPref(PF_CMDNO, 0, 0),
		0, 
		rip_cmd_debug_packet,
		NULL,
		NULL, 
		0,
		0, 
		"packet            -- Debug RIP packet information",
		"packet            -- 跟踪RIP报文的接收和发送", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ 
		"database",
		MATCH_AMB,
		cmdPref(PF_CMDNO, 0, 0),
		0, 
		rip_cmd_debug_data,
		NULL,
		NULL, 
		0,
		0, 
		"database          -- Debug RIP route information",
		"database          -- 跟踪RIP路由信息", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ 
		"message",
		MATCH_AMB,
		cmdPref(PF_CMDNO, 0, 0),
		0, 
		rip_cmd_debug_msg,
		NULL,
		NULL, 
		0,
		0, 
		"message           -- Debug RIP message information",
		"message           -- 跟踪RIP消息信息", 
		NULLCHAR,
		NULLCHAR
	},
	{ 
		"return",
		MATCH_AMB,
		cmdPref(PF_CMDNO, 0, 0),
		0, 
		rip_cmd_debug_return,
		NULL,
		NULL, 
		0,
		0, 
		NULL,
		NULL, 
		NULLCHAR,
		NULLCHAR
	},

	{ NULLCHAR }
};

/*RIP debug ip rip protocol [send | receive] 命令*/
struct cmds rip_cmds_debug_proto_tab[] = 
{
	{ 
		"send",
		MATCH_AMB,
		cmdPref(PF_CMDNO, 0, 0),
		0, 
		rip_cmd_debug_packet_send,
		NULL,
		NULL, 
		0,
		0, 
		"send              -- Debug RIP packet send",
		"send              -- 跟踪RIP发送的报文", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ 
		"receive",
		MATCH_AMB,
		cmdPref(PF_CMDNO, 0, 0),
		0, 
		rip_cmd_debug_packet_recv,
		NULL,
		NULL, 
		0,
		0, 
		"receive           -- Debug RIP packet receive",
		"receive           -- 跟踪RIP接收到的报文", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ 
		"<cr>", MATCH_AMB, 0, 0, 
		NULL, NULL, NULL, 
		0, 0, 
		"<cr>",
		"<cr>",
		NULLCHAR,
		NULLCHAR
	},

	{ NULLCHAR }
};

/*===========================================================
函数名:      rip_trace
函数功能: 打印RIP跟踪信息
输入参数: flag: debug跟踪类型
                       a   : 待打印的跟踪信息
输出参数: 无
返回值:      无
备注:
=============================================================*/
void rip_debug( uint32 flag, const char * a, ...)
{
	va_list varg;
    
	if( (rip_glb_info.debug_flag & flag) != 0 ) 
	{
		va_start( varg, a );
		trace_vprintf(MODULE_TYPE_RIP, flag, a, varg );
		va_end( varg );
	}

	return;
}


/*===========================================================
函数名:      rip_cmd_debug_rip
函数功能: debug ip rip命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_CMD_FAIL
备注:
=============================================================*/
int rip_cmd_debug_rip( int argc, char *argv[], struct user *u )
{
	return subcmd(rip_cmds_debug_rip_tab, NULL, argc--, argv++, u);
}

/*===========================================================
函数名:      rip_cmd_debug_data
函数功能: debug ip rip database命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_CMD_FAIL
备注:
=============================================================*/
int rip_cmd_debug_data( int argc, char *argv[], struct user *u )
{
	int ret;

	ret = cmdend(argc - 1, argv + 1, u);
	if( 0 != ret )
	{
		return ret;
	}
	
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_DATABASE, rip_show_debug, rip_no_debug_all, 1 );
			if( ret < 0 )
			{
				return ret;
			}
			
			rip_glb_info.debug_flag |= RIP_DEBUG_IP_RIP_DATABASE;

			if( IsChinese(u) )
			{
				vty_output( "RIP 数据库调试开关打开\n" );
			}
			else
			{
				vty_output( "RIP database debugging is on\n" );
			}
			break;
		case NOPREF_FUNC:
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_DATABASE, rip_show_debug, rip_no_debug_all, 0 );
			if( ret < 0 )
			{
				return ret;
			}
			
			rip_glb_info.debug_flag &= ~(RIP_DEBUG_IP_RIP_DATABASE);

			if( IsChinese(u) )
			{
				vty_output( "RIP 数据库调试开关关闭\n" );
			}
			else
			{
				vty_output( "RIP database debugging is off\n" );
			}
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
函数名:      rip_cmd_debug_packet
函数功能: debug ip rip packet命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_CMD_FAIL
备注:
=============================================================*/
int rip_cmd_debug_packet( int argc, char *argv[], struct user *u )
{
	int ret;

	if( argc == 1 )
	{
		ret = cmdend(argc - 1, argv + 1, u);
		if(ret != 0) 
		{
			return ret;
		}

		switch(TypeOfFunc(u)) 
		{
			case NORMAL_FUNC:
				ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_PROTOCOL, rip_show_debug, rip_no_debug_all, 1 );
				if( ret < 0 )
				{
					return ret;
				}

				rip_glb_info.debug_flag |= RIP_DEBUG_IP_RIP_PROTOCOL;

				if( IsChinese(u) )
				{
					vty_output("RIP 协议调试开关打开\n");
				}
				else
				{
					vty_output("RIP protocol debugging is on\n");
				}
				break;
			case NOPREF_FUNC:
				/*注册回调函数*/
				ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_PROTOCOL, rip_show_debug, rip_no_debug_all, 0 );
				if( ret < 0 )
				{
					return ret;
				}

				rip_glb_info.debug_flag &= ~(RIP_DEBUG_IP_RIP_PROTOCOL);

				if( IsChinese(u) )
				{
					vty_output("RIP 协议调试开关关闭\n");
				}
				else
				{
					vty_output("RIP protocol debugging is off\n");
				}
				break;
			default:
				break;
		}
	}
	else
	{
		return subcmd(rip_cmds_debug_proto_tab, NULL, argc--, argv++, u);
	}

	return RIP_SUCCESS;
}


/*===========================================================
函数名:      rip_cmd_debug_packet_send
函数功能: debug ip rip packet send命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_CMD_FAIL
备注:
=============================================================*/
int rip_cmd_debug_packet_send( int argc, char *argv[], struct user *u )
{
	int ret;

	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	switch(TypeOfFunc(u)) 
	{
		case NORMAL_FUNC:
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_PROTO_SEND, rip_show_debug, rip_no_debug_all, 1 );
			if( ret < 0 )
			{
				return ret;
			}

			rip_glb_info.debug_flag |= RIP_DEBUG_IP_RIP_PROTO_SEND;

			if( IsChinese(u) )
			{
				vty_output("RIP 协议发送调试开发打开\n");
			}
			else
			{
				vty_output("RIP protocol send debugging is on\n");
			}
			break;
		case NOPREF_FUNC:
			/*注册回调函数*/
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_PROTO_SEND, rip_show_debug, rip_no_debug_all, 0 );
			if( ret < 0 )
			{
				return ret;
			}

			rip_glb_info.debug_flag &= ~(RIP_DEBUG_IP_RIP_PROTO_SEND);

			if( IsChinese(u) )
			{
				vty_output("RIP 协议发送调试开发关闭\n");
			}
			else
			{
				vty_output("RIP protocol send debugging is off\n");
			}
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}



/*===========================================================
函数名:      rip_cmd_debug_packet_recv
函数功能: debug ip rip packet receive命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_CMD_FAIL
备注:
=============================================================*/
int rip_cmd_debug_packet_recv( int argc, char *argv[], struct user *u )
{
	int ret;

	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	switch(TypeOfFunc(u)) 
	{
		case NORMAL_FUNC:
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_PROTO_RECV, rip_show_debug, rip_no_debug_all, 1 );
			if( ret < 0 )
			{
				return ret;
			}

			rip_glb_info.debug_flag |= RIP_DEBUG_IP_RIP_PROTO_RECV;

			if( IsChinese(u) )
			{
				vty_output("RIP 协议接收调试开关打开\n");
			}
			else
			{
				vty_output("RIP protocol receive debugging is on\n");
			}
			break;
		case NOPREF_FUNC:
			/*注册回调函数*/
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_PROTO_RECV, rip_show_debug, rip_no_debug_all, 0 );
			if( ret < 0 )
			{
				return ret;
			}

			rip_glb_info.debug_flag &= ~(RIP_DEBUG_IP_RIP_PROTO_RECV);
			
			if( IsChinese(u) )
			{
				vty_output("RIP 协议接收调试开关关闭\n");
			}
			else
			{
				vty_output("RIP protocol receive debugging is off\n");
			}
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}
/*******************************************************************************
 *
 * FUNCTION	:	rip_cmd_debug_return
 *
 * PARAMS		:	argc		- 
 *					argv[]		- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.24 13:36:25
 *
*******************************************************************************/
int rip_cmd_debug_return( int argc, char *argv[], struct user *u )
{
	int ret;

	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	switch(TypeOfFunc(u))
	{
		case NORMAL_FUNC:
			/*注册回调函数*/
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_RETURN, rip_show_debug, rip_no_debug_all, 1 );
			if( ret < 0 )
			{
				return ret;
			}

			rip_glb_info.debug_flag |= RIP_DEBUG_IP_RIP_RETURN;

			if( IsChinese(u) )
			{
				vty_output( "RIP 返回调试开关打开\n" );
			}
			else
			{
				vty_output( "RIP return debugging is on\n" );
			}
			break;
		case NOPREF_FUNC:
			/*注册回调函数*/
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_RETURN, rip_show_debug, rip_no_debug_all, 0 );
			if( ret < 0 )
			{
				return ret;
			}
			
			rip_glb_info.debug_flag &= ~(RIP_DEBUG_IP_RIP_RETURN);

			if( IsChinese(u) )
			{
				vty_output( "RIP 返回调试开关关闭\n" );
			}
			else
			{
				vty_output( "RIP return debugging is off\n" );
			}
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
函数名:      rip_cmd_debug_msg
函数功能: debug ip rip message命令解析
输入参数: argc/argv :命令参数
                       u   : 命令用户
输出参数: 无
返回值:      处理成功,返回RIP_CMD_SUCCESS
                       处理失败,返回RIP_CMD_FAIL
备注:
=============================================================*/
int rip_cmd_debug_msg( int argc, char *argv[], struct user *u )
{
	int ret;

	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	switch(TypeOfFunc(u))
	{
		case NORMAL_FUNC:
			/*注册回调函数*/
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_MSG, rip_show_debug, rip_no_debug_all, 1 );
			if( ret < 0 )
			{
				return ret;
			}

			rip_glb_info.debug_flag |= RIP_DEBUG_IP_RIP_MSG;

			if( IsChinese(u) )
			{
				vty_output( "RIP 消息调试开关打开\n" );
			}
			else
			{
				vty_output( "RIP message debugging is on\n" );
			}
			break;
		case NOPREF_FUNC:
			/*注册回调函数*/
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_MSG, rip_show_debug, rip_no_debug_all, 0 );
			if( ret < 0 )
			{
				return ret;
			}
			
			rip_glb_info.debug_flag &= ~(RIP_DEBUG_IP_RIP_MSG);

			if( IsChinese(u) )
			{
				vty_output( "RIP 消息调试开关关闭\n" );
			}
			else
			{
				vty_output( "RIP message debugging is off\n" );
			}
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
函数名:      rip_no_debug_all
函数功能: no debug all命令解析
输入参数: 无
输出参数: 无
返回值:      无
备注:
=============================================================*/
void rip_no_debug_all( void )
{
	rip_glb_info.debug_flag = 0;	
    	vty_output( "All RIP possible debugging has been turned off\n" );

	return ;
}

/*===========================================================
函数名:      rip_show_debug
函数功能: show debug命令解析
输入参数: 无
输出参数: 无
返回值:      无
备注:
=============================================================*/
void rip_show_debug( uint32 debug_flag )
{
	/*根据rip_debug_flag标识不同做不同处理*/
	if( debug_flag != 0 )
	{
		vty_output( "RIP:\n" );
	}
	
	if( debug_flag & RIP_DEBUG_IP_RIP_DATABASE )
	{
		vty_output( " RIP database debugging is on\n" );
	}

	if( debug_flag & RIP_DEBUG_IP_RIP_PROTO_SEND )
	{
		vty_output(" RIP protocol send debugging is on\n");
	}

	if( debug_flag & RIP_DEBUG_IP_RIP_PROTO_RECV )
	{
		vty_output(" RIP protocol receive debugging is on\n");
	}

	if( debug_flag & RIP_DEBUG_IP_RIP_MSG )
	{
		vty_output(" RIP message debugging is on\n");
	}
	if( debug_flag & RIP_DEBUG_IP_RIP_RETURN)
	{
		vty_output(" RIP return debugging is on\n");
	}
	
	return;
}
