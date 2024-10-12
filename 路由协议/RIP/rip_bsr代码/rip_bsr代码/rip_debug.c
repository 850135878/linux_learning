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
 *        PURPOSE:        :   RIPģ����ٴ���ģ��
 *
**************************************************************************/
#include <rip/rip_include.h>
#include <rip/rip_main.h>
#include <rip/rip_debug.h>

extern struct rip_glb_info_  rip_glb_info;

/*debug ip�µ�RIP����*/
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
		"rip                    -- ����RIP��Ϣ", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ NULLCHAR }
};

/*RIP debug ip rip ����*/
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
		"packet            -- ����RIP���ĵĽ��պͷ���", 
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
		"database          -- ����RIP·����Ϣ", 
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
		"message           -- ����RIP��Ϣ��Ϣ", 
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

/*RIP debug ip rip protocol [send | receive] ����*/
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
		"send              -- ����RIP���͵ı���", 
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
		"receive           -- ����RIP���յ��ı���", 
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
������:      rip_trace
��������: ��ӡRIP������Ϣ
�������: flag: debug��������
                       a   : ����ӡ�ĸ�����Ϣ
�������: ��
����ֵ:      ��
��ע:
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
������:      rip_cmd_debug_rip
��������: debug ip rip�������
�������: argc/argv :�������
                       u   : �����û�
�������: ��
����ֵ:      ����ɹ�,����RIP_CMD_SUCCESS
                       ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_debug_rip( int argc, char *argv[], struct user *u )
{
	return subcmd(rip_cmds_debug_rip_tab, NULL, argc--, argv++, u);
}

/*===========================================================
������:      rip_cmd_debug_data
��������: debug ip rip database�������
�������: argc/argv :�������
                       u   : �����û�
�������: ��
����ֵ:      ����ɹ�,����RIP_CMD_SUCCESS
                       ����ʧ��,����RIP_CMD_FAIL
��ע:
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
				vty_output( "RIP ���ݿ���Կ��ش�\n" );
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
				vty_output( "RIP ���ݿ���Կ��عر�\n" );
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
������:      rip_cmd_debug_packet
��������: debug ip rip packet�������
�������: argc/argv :�������
                       u   : �����û�
�������: ��
����ֵ:      ����ɹ�,����RIP_CMD_SUCCESS
                       ����ʧ��,����RIP_CMD_FAIL
��ע:
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
					vty_output("RIP Э����Կ��ش�\n");
				}
				else
				{
					vty_output("RIP protocol debugging is on\n");
				}
				break;
			case NOPREF_FUNC:
				/*ע��ص�����*/
				ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_PROTOCOL, rip_show_debug, rip_no_debug_all, 0 );
				if( ret < 0 )
				{
					return ret;
				}

				rip_glb_info.debug_flag &= ~(RIP_DEBUG_IP_RIP_PROTOCOL);

				if( IsChinese(u) )
				{
					vty_output("RIP Э����Կ��عر�\n");
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
������:      rip_cmd_debug_packet_send
��������: debug ip rip packet send�������
�������: argc/argv :�������
                       u   : �����û�
�������: ��
����ֵ:      ����ɹ�,����RIP_CMD_SUCCESS
                       ����ʧ��,����RIP_CMD_FAIL
��ע:
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
				vty_output("RIP Э�鷢�͵��Կ�����\n");
			}
			else
			{
				vty_output("RIP protocol send debugging is on\n");
			}
			break;
		case NOPREF_FUNC:
			/*ע��ص�����*/
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_PROTO_SEND, rip_show_debug, rip_no_debug_all, 0 );
			if( ret < 0 )
			{
				return ret;
			}

			rip_glb_info.debug_flag &= ~(RIP_DEBUG_IP_RIP_PROTO_SEND);

			if( IsChinese(u) )
			{
				vty_output("RIP Э�鷢�͵��Կ����ر�\n");
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
������:      rip_cmd_debug_packet_recv
��������: debug ip rip packet receive�������
�������: argc/argv :�������
                       u   : �����û�
�������: ��
����ֵ:      ����ɹ�,����RIP_CMD_SUCCESS
                       ����ʧ��,����RIP_CMD_FAIL
��ע:
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
				vty_output("RIP Э����յ��Կ��ش�\n");
			}
			else
			{
				vty_output("RIP protocol receive debugging is on\n");
			}
			break;
		case NOPREF_FUNC:
			/*ע��ص�����*/
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_PROTO_RECV, rip_show_debug, rip_no_debug_all, 0 );
			if( ret < 0 )
			{
				return ret;
			}

			rip_glb_info.debug_flag &= ~(RIP_DEBUG_IP_RIP_PROTO_RECV);
			
			if( IsChinese(u) )
			{
				vty_output("RIP Э����յ��Կ��عر�\n");
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
			/*ע��ص�����*/
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_RETURN, rip_show_debug, rip_no_debug_all, 1 );
			if( ret < 0 )
			{
				return ret;
			}

			rip_glb_info.debug_flag |= RIP_DEBUG_IP_RIP_RETURN;

			if( IsChinese(u) )
			{
				vty_output( "RIP ���ص��Կ��ش�\n" );
			}
			else
			{
				vty_output( "RIP return debugging is on\n" );
			}
			break;
		case NOPREF_FUNC:
			/*ע��ص�����*/
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_RETURN, rip_show_debug, rip_no_debug_all, 0 );
			if( ret < 0 )
			{
				return ret;
			}
			
			rip_glb_info.debug_flag &= ~(RIP_DEBUG_IP_RIP_RETURN);

			if( IsChinese(u) )
			{
				vty_output( "RIP ���ص��Կ��عر�\n" );
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
������:      rip_cmd_debug_msg
��������: debug ip rip message�������
�������: argc/argv :�������
                       u   : �����û�
�������: ��
����ֵ:      ����ɹ�,����RIP_CMD_SUCCESS
                       ����ʧ��,����RIP_CMD_FAIL
��ע:
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
			/*ע��ص�����*/
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_MSG, rip_show_debug, rip_no_debug_all, 1 );
			if( ret < 0 )
			{
				return ret;
			}

			rip_glb_info.debug_flag |= RIP_DEBUG_IP_RIP_MSG;

			if( IsChinese(u) )
			{
				vty_output( "RIP ��Ϣ���Կ��ش�\n" );
			}
			else
			{
				vty_output( "RIP message debugging is on\n" );
			}
			break;
		case NOPREF_FUNC:
			/*ע��ص�����*/
			ret = set_trace_flag( MODULE_TYPE_RIP, RIP_DEBUG_IP_RIP_MSG, rip_show_debug, rip_no_debug_all, 0 );
			if( ret < 0 )
			{
				return ret;
			}
			
			rip_glb_info.debug_flag &= ~(RIP_DEBUG_IP_RIP_MSG);

			if( IsChinese(u) )
			{
				vty_output( "RIP ��Ϣ���Կ��عر�\n" );
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
������:      rip_no_debug_all
��������: no debug all�������
�������: ��
�������: ��
����ֵ:      ��
��ע:
=============================================================*/
void rip_no_debug_all( void )
{
	rip_glb_info.debug_flag = 0;	
    	vty_output( "All RIP possible debugging has been turned off\n" );

	return ;
}

/*===========================================================
������:      rip_show_debug
��������: show debug�������
�������: ��
�������: ��
����ֵ:      ��
��ע:
=============================================================*/
void rip_show_debug( uint32 debug_flag )
{
	/*����rip_debug_flag��ʶ��ͬ����ͬ����*/
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
