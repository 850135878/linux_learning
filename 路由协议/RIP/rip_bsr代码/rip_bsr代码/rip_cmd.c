char rip_cmd_version[] = "rip_cmd.c last changed at 2013.10.29 14:04:19 by yuguangcheng\n";
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
 *        FILE NAME       :   rip_cmd.c
 *        AUTHOR          :   HQ.Sun
 *        DATE            :   2006.01.08 11:13:00
 *        PURPOSE:        :   RIPģ�������ģ��
 *
**************************************************************************/
#include <rip/rip_include.h>
#include <rip/rip_main.h>
#include <rip/rip_init.h>
#include <rip/rip_cmd.h>
#include <rip/rip_show.h>
#include <rip/rip_intf.h>
#include <rip/rip_packet.h>
#include <rip/rip_timer.h>
#include <rip/rip_debug.h>
#include <rip/rip_redis.h>

extern char rip_send_buffer[RIP_MAX_PACKET_SIZE];

extern struct cmds rip_cmds_show_ip_tab[];
extern struct cmds rip_cmds_debug_ip_tab[];
extern struct rip_glb_info_  rip_glb_info;
extern struct rip_intf_ *rip_intf_array[INTERFACE_DEVICE_MAX_NUMBER + 1];
#ifdef OS_VXWORKS
	void rtc_get(ulong *,ulong*);
#else 
	int tm_get(ulong*,ulong*,ulong*);
#endif

/*router rip�µ�����*/
struct topcmds  rip_cmd_topcmds[] =
{
#if 0
#ifdef VRF_MODULE
	{ 
		"address-family", cmdPref(PF_CMDNO, 0, 0), 
		IF_ANY, FG_NULL, IF_NULL,
		FG_RIP, rip_cmd_afi,
		NULL, NULL, 0, 0,
		"address-family         -- Enter Address Family command mode",
		"address-family         -- �����ַЭ��������ģʽ",
		NULLCHAR, NULLCHAR
	},
#endif
#endif

	{ 
 		"auto-summary", cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		IF_ANY, FG_NULL, IF_NULL,
		FG_RIP, rip_cmd_auto_summary,
		NULL, NULL, 0, 0,
		"auto-summary           -- Enable automatic summarization",
		"auto-summary           -- �����Զ�����",
		NULLCHAR, NULLCHAR
	},
	
	{
		"default-information", cmdPref(PF_CMDNO, PF_CMDDEF, 0),
		IF_ANY, FG_NULL, IF_NULL,
		FG_RIP, rip_cmd_def_info,
		NULL, NULL, 0, 0,
		"default-information    -- Set distribution of default information",
		"default-information    -- ����ȱʡ��Ϣ�ķ���",
		NULLCHAR, NULLCHAR
	},

	{
		"default-metric", cmdPref(PF_CMDNO, PF_CMDDEF, 0),
		IF_ANY, FG_NULL, IF_NULL,
		FG_RIP, rip_cmd_def_metric,
		NULL, NULL, 0, 0,
		"default-metric         -- Set metric of redistributed routes",
		"default-metric         -- ��������·�ɵ�·��Ȩֵ",
		NULLCHAR, NULLCHAR
	},

	{ 
		"distance", cmdPref(PF_CMDNO, 0, 0), 
		IF_ANY, FG_NULL, IF_NULL,
		FG_RIP, rip_cmd_distance,
		NULL, NULL, 0, 0,
		"distance               -- Set administrative distance",
		"distance               -- ���ù������",
		NULLCHAR, NULLCHAR
	},

	{ "filter", cmdPref(PF_CMDNO, 0, 0), 
		IF_ANY, FG_NULL, IF_NULL, 
		FG_RIP, rip_cmd_filter,
		NULL, NULL, 0, 0,
		"filter                 -- Set RIP route filter",
		"filter                 -- ����·�ɹ��˱�",
		NULLCHAR, NULLCHAR
	},
	
	{ 
		"input-queue", cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		IF_ANY, FG_NULL, IF_NULL,
		FG_RIP, rip_cmd_input_queue,
		NULL, NULL, 0,0, 
		"input-queue            -- Set input queue depth",
		"input-queue            -- ����RIP���ն������", 
		NULLCHAR, NULLCHAR
	},

	{ 
		"neighbor", cmdPref(PF_CMDNO, 0, 0), 
		IF_ANY, FG_NULL, IF_NULL,
		FG_RIP, rip_cmd_neigh,
		NULL, NULL, 0, 0,
		"neighbor               -- Set RIP neighbor",
		"neighbor               -- ����RIP�ھ�", 
		NULLCHAR, NULLCHAR
	},
/*
 	{
		"network",
		cmdPref(PF_CMDNO, 0, 0),
		IF_ANY,
		FG_NULL,
	    	IF_NULL,
		FG_RIP,
		rip_cmd_net,
		NULL,
		NULL,
		0,
		0,
		"network                -- Set RIP network",
		"network                -- ����RIP����", 
		NULLCHAR,
		NULLCHAR
	},
*/
	{ 
		"offset",
		cmdPref(PF_CMDNO, 0, 0), 
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP, 
		rip_cmd_offset,
		NULL,
		NULL,
		0,
		0,
		"offset                 -- Add offset for RIP routes",
		"offset                 -- ���RIP·��Ȩֵƫ����",
		NULLCHAR,
		NULLCHAR
	},

	{ 
		"redistribute",
		cmdPref(PF_CMDNO, 0, 0),
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP, 
		rip_cmd_redis,
		NULL,
		NULL, 
		0,
		0,
		"redistribute           -- Redistribute route",
		"redistribute           -- ת��·��", 
		NULLCHAR,
		NULLCHAR
	},

 	{
		"timers",
		cmdPref(PF_CMDNO, PF_CMDDEF, 0),
		IF_ANY,
		FG_NULL,
	    	IF_NULL,
		FG_RIP,
		rip_cmd_timer,
		NULL,
		NULL,
		0,
		0,
		"timers                 -- Set RIP timers",
		"timers                 -- ����RIP��ʱ��", 
		NULLCHAR,
		NULLCHAR
	},

	{
		"validate-update-source",
		cmdPref(PF_CMDNO, PF_CMDDEF, 0),
		IF_ANY,
		FG_NULL,
	    	IF_NULL,
		FG_RIP,
		rip_cmd_vali_source,
		NULL,
		NULL,
		0,
		0,
		"validate-update-source -- Enable validate update source",
		"validate-update-source -- ����·�ɸ��±��ĵ�Դ��ַ��֤",
		NULLCHAR,
		NULLCHAR
	},

	{ 
		"version", 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP, 
		rip_cmd_ver,
		NULL,
		NULL, 
		0,
		0, 
		"version                -- Set RIP version",
		"version                -- ����RIP�汾", 
		NULLCHAR,
		NULLCHAR
	},

	/*xuhaiqing 2010-12-21�Ž����·����Ŀ������ȡ��*/	
	/*{ 
		"maximum-routes", 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP, 
		rip_cmd_maxcount,
		NULL,
		NULL, 
		0,
		0, 
		"maximum-routes         -- Set maximum route count",
		"maximum-routes         -- ����RIP���·������", 
		NULLCHAR,
		NULLCHAR
	},*/

	{ 
		"maximum-nexthop", 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP, 
		rip_cmd_max_nh,
		NULL,
		NULL, 
		0,
		0, 
		"maximum-nexthop        -- Set maximum paths of one network",
		"maximum-nexthop        -- ����RIP���ȼ�·����", 
		NULLCHAR,
		NULLCHAR
	},

	{ 
		"check-zero-domain", 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP, 
		rip_cmd_zero_domain_check,
		NULL,
		NULL, 
		0,
		0, 
		"check-zero-domain      -- check route item zero domain",
		"check-zero-domain      -- ���·����Ŀ�����е�0�ֶκϷ���", 
		NULLCHAR,
		NULLCHAR
	},

	{ NULLCHAR }
};
#if 0
/*address-family �µ�����*/
struct topcmds  rip_cmd_vrf_topcmds[] =
{
	{ 
		"auto-summary", 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP_VRF,
		rip_cmd_auto_summary,
		NULL,
		NULL,
		0,
		0,
		"auto-summary           -- Enable automatic summarization",
		"auto-summary           -- �����Զ�����",
		NULLCHAR,
		NULLCHAR
	},
	
	{
		"default-information",
		cmdPref(PF_CMDNO, PF_CMDDEF, 0),
		IF_ANY,
		FG_NULL,
	    IF_NULL,
		FG_RIP_VRF,
		rip_cmd_def_info,
		NULL,
		NULL,
		0,
		0,
		"default-information    -- Set distribution of default information",
		"default-information    -- ����ȱʡ��Ϣ�ķ���",
		NULLCHAR,
		NULLCHAR
	},

	{
		"default-metric", 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0),
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP_VRF, 
		rip_cmd_def_metric,
		NULL,
		NULL,
		0,
		0,
		"default-metric         -- Set metric of redistributed routes",
		"default-metric         -- ��������·�ɵ�·��Ȩֵ",
		NULLCHAR,
		NULLCHAR
	},

	{ 
		"distance",
		cmdPref(PF_CMDNO, 0, 0), 
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP_VRF, 
		rip_cmd_distance,
		NULL,
		NULL,
		0,
		0,
		"distance               -- Set administrative distance",
		"distance               -- ���ù������",
		NULLCHAR,
		NULLCHAR
	},
#if 0	
	{ 
		"exit-address-family",
		0, 
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP_VRF, 
		rip_cmd_exit_afi,
		NULL,
		NULL,
		0,
		0,
		"exit-address-family    -- Exit from Address Family configuration mod",
		"exit-address-family    -- �˳���ַЭ��������ģʽ",
		NULLCHAR,
		NULLCHAR
	},
#endif
	{ "filter", cmdPref(PF_CMDNO, 0, 0), 
		IF_ANY, FG_NULL, IF_NULL, FG_RIP_VRF, 
		rip_cmd_filter, NULL, NULL, 0, 0,
		"filter                 -- Set RIP route filter",
		"filter                 -- ����·�ɹ��˱�",
		NULLCHAR, NULLCHAR
	},

 	{
		"network",
		cmdPref(PF_CMDNO, 0, 0),
		IF_ANY,
		FG_NULL,
	    	IF_NULL,
		FG_RIP_VRF,
		rip_cmd_net,
		NULL,
		NULL,
		0,
		0,
		"network                -- Set RIP network",
		"network                -- ����RIP����", 
		NULLCHAR,
		NULLCHAR
	},

	{ 
		"offset",
		cmdPref(PF_CMDNO, 0, 0), 
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP_VRF, 
		rip_cmd_offset,
		NULL,
		NULL,
		0,
		0,
		"offset                 -- Add offset for RIP routes",
		"offset                 -- ���RIP·��Ȩֵƫ����",
		NULLCHAR,
		NULLCHAR
	},

	{ 
		"redistribute",
		cmdPref(PF_CMDNO, 0, 0),
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP_VRF, 
		rip_cmd_redis,
		NULL,
		NULL, 
		0,
		0,
		"redistribute           -- Redistribute route",
		"redistribute           -- ת��·��", 
		NULLCHAR,
		NULLCHAR
	},

	{ 
		"version", 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP_VRF, 
		rip_cmd_ver,
		NULL,
		NULL, 
		0,
		0, 
		"version                -- Set RIP version",
		"version                -- ����RIP�汾", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ 
		"maximum-routes", 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		IF_ANY,
		FG_NULL,
		IF_NULL,
		FG_RIP_VRF, 
		rip_cmd_maxcount,
		NULL,
		NULL, 
		0,
		0, 
		"maximum-routes         -- Set maximum route count",
		"maximum-routes         -- ����RIP���·������", 
		NULLCHAR,
		NULLCHAR
	},

	{ NULLCHAR }
};
#endif

struct cmds rip_cmd_vrf_table[] = 
{
#ifdef VRF_MODULE
	{
		"vrf",
		MATCH_AMB,
		cmdPref(PF_CMDNO, 0, 0),
		0, 
		rip_cmd_rip_process_vrf,
		NULL,
		NULL, 
		0,
		0, 
		"vrf        -- VPN Routing/Forwarding Instance",
		"vrf        -- VPN·�ɺ�ת����", 
		NULLCHAR, 
		NULLCHAR
	},
#endif

	{ 
		"<cr>", MATCH_END, cmdPref(PF_CMDNO, 0, 0), 0, 
		NULL, NULL, NULL, 
		0, 0,
		"<cr>", 
		"<cr>", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ NULL }
};

struct cmds rip_cmd_rip_process_vrf_name_table[] = 
{
	{ 
		"WORD", 
		MATCH_STR,
		cmdPref(PF_CMDNO, 0, 0),
		0, 
		rip_cmd_rip_process_vrf_name,
		NULL,
		NULL, 
		0,
		0, 
		"WORD        -- VPN Routing/Forwarding Instance name",
		"WORD        -- VPN·�ɺ�ת��������", 
		NULLCHAR,
		NULLCHAR
	},

	{ NULL }
};
#if 0
struct cmds rip_cmds_afi_tab[] =
{
	{ 
		"ipv4",
		MATCH_AMB,
		cmdPref(PF_CMDNO, 0, 0),
		0,
		rip_cmd_afi_ipv4,
		NULL,
		NULL, 
		0,
		0,
		"ipv4              -- Address family",
		"ipv4              -- ��ַЭ����", 
		NULLCHAR,
		NULLCHAR
	},
	{ NULLCHAR }
};

struct cmds rip_cmds_afi_ipv4_tab[] =
{
	{ 
		"vrf",
		MATCH_AMB,
		cmdPref(PF_CMDNO, 0, 0),
		0,
		rip_cmd_afi_vrf,
		NULL,
		NULL, 
		0,
		0,
		"vrf               -- VPN Routing/Forwarding instance",
		"vrf               -- VPN·�ɺ�ת����", 
		NULLCHAR,
		NULLCHAR
	},
	{ NULLCHAR }
};

struct cmds rip_cmds_afi_vrf_tab[] =
{
	{ 
		"1234567890",
		MATCH_STR,
		cmdPref(PF_CMDNO, 0, 0),
		0,
		rip_cmd_afi_vrf_name,
		NULL,
		NULL, 
		0,
		0,
		"WORD              -- VPN Routing/Forwarding instance name(up to 16 chars)",
		"WORD              -- VPN·�ɺ�ת��������(���16���ַ�)", 
		NULLCHAR,
		NULLCHAR
	},
	{ NULLCHAR }
};
#endif
/*router rip������*/
struct cmds rip_cmd_router_tab[] =
{
	{
		"rip",
		MATCH_AMB,
		cmdPref(PF_CMDNO, 0, 0),
		0,
		rip_cmd_router_rip,
		NULL,
		NULL,
		0,
		0,
		"rip	--Enable Routing Information Protocol(RIP) ",
		"rip	--����RIP�����RIP����",
		NULLCHAR,
		NULLCHAR,
	},
	
	{ NULLCHAR }
};

struct cmds rip_cmd_process_tab[] =
{
	{
		"1234567890",
		MATCH_DIG,
		cmdPref(PF_CMDNO, 0, 0),
		0,
		rip_cmd_rip_process,
		NULL,
		NULL,
		0,
		0,
		"<1-65535>        -- RIP Process ID",
		"<1-65535>        -- RIPЭ����̺�", 
		NULLCHAR,
		NULLCHAR,
	},
	
	{ NULLCHAR }
};

/*input-queue����*/
struct cmds rip_cmds_input_queue_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_input_queue_val, NULL, NULL, 
		0, 0,
		"<1-61440>         -- Queue depth",
		"<1-61440>         -- �������", 
		NULLCHAR, NULLCHAR
	},
	
	{ "<cr>", MATCH_AMB, cmdPref(PF_CMDNO, PF_CMDDEF, PF_NOCMDNOR), 0,
		NULL, NULL, NULL, 
		0, 0, 
		"<cr>", 
		"<cr>", 
		NULLCHAR, NULLCHAR
	},

	{ NULLCHAR }
};

/*default-information originate����*/
struct cmds rip_cmds_def_origi_tab[] = 
{ 
	{ "originate",
		MATCH_AMB,
		cmdPref(0, 0, 0),
		0, 
		rip_cmd_def_info_origi,
		NULL,
		NULL, 
		0,
		0,
		"originate         -- Distribute a default route",
		"originate         -- ����ȱʡ·��", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ 	"originate-safe",
		MATCH_AMB,
		cmdPref(0, 0, 0),
		0, 
		rip_cmd_def_info_origi_safe,
		NULL,
		NULL, 
		0,
		0,
		"originate-safe    -- Distribute a default route when main route table has a non-RIP one",
		"originate-safe    -- ����·�ɱ��д��ڷ�RIP��ȱʡ·��ʱ����RIP����ȱʡ·��", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ "<cr>", MATCH_END, cmdPref(PF_CMDNO, PF_CMDDEF, PF_NOCMDNOR), 0,
		NULL, NULL, NULL, 
		0, 0,
		"<cr>",
		"<cr>",
		NULLCHAR, NULLCHAR
	},
	
	{ NULLCHAR }
};

/*default-metric <1-16>����*/
struct cmds rip_cmds_def_metric_val_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_def_metric_val, NULL, NULL, 
		0, 0,
		"<1-16>            -- Default metric",
		"<1-16>            -- ȱʡ·��Ȩֵ", 
		NULLCHAR, NULLCHAR
	},

	{ NULLCHAR }
};

/*version <1-2>����*/
struct cmds rip_cmds_ver_val_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_ver_val, NULL, NULL, 
		0, 0,
		"<1-2>             -- Version",
		"<1-2>             -- �汾",
		NULLCHAR, NULLCHAR
	},

	{ "<cr>", MATCH_END, cmdPref(PF_CMDNO, PF_CMDDEF, PF_NOCMDNOR), 0,
		NULL, NULL, NULL, 
		0, 0,
		"<cr>",
		"<cr>",
		NULLCHAR, NULLCHAR
	},
	
	{ NULLCHAR }
};

/*maximum-counts <1-8192>����*/
/*xuhaiqing 2010-12-21�Ž����·����Ŀ������ȡ��*/
/*struct cmds rip_cmds_max_count_val_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_maxcount_val, NULL, NULL, 
		0, 0,
		"<1-10240>         -- Route count",
		"<1-10240>         -- ·������", 
		NULLCHAR, NULLCHAR
	},

	{ NULLCHAR }
};*/

/*maximum-nhs <1-16>����*/
struct cmds rip_cmds_max_nh_val_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_maxnh_val, NULL, NULL, 
		0, 0,
		"<1-16>         -- nexthop count",
		"<1-16>         -- ��һ����Ŀ", 
		NULLCHAR, NULLCHAR
	},

	{ NULLCHAR }
};

/*neighbor A.B.C.D*/
struct cmds rip_cmds_neigh_addr_tab[] =
{
	{ 
		"1234567890",
		MATCH_ADD,
		cmdPref(PF_CMDNO, 0, 0),
		0, 
		rip_cmd_neigh_addr,
		NULL,
		NULL, 
		0,
		0,
		"A.B.C.D           -- Neighbor address",
		"A.B.C.D           -- �ھ�IP��ַ", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ NULLCHAR }
};
#if 0
/*network A.B.C.D���� */
struct cmds rip_cmds_net_addr_tab[] =
{
	{ 
		"1234567890",
		MATCH_ADD,
		cmdPref(PF_CMDNO, 0, 0),
		0, 
		rip_cmd_net_addr,
		NULL,
		NULL, 
		0,
		0,
		"A.B.C.D           -- Network number",
		"A.B.C.D           -- �����", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ NULLCHAR }
};

/*network A.B.C.D A.B.C.D���� */
struct cmds rip_cmds_net_mask_tab[] =
{
	{ 
		"1234567890",
		MATCH_ADD,
		cmdPref(PF_CMDNO, 0, 0),
		0, 
		rip_cmd_net_addr_mask,
		NULL,
		NULL, 
		0, 
		0,
		"A.B.C.D           -- Network mask",
		"A.B.C.D           -- ��������", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ 
		"<cr>",
		MATCH_END,
		cmdPref(PF_CMDNO, 0, 0),
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
#endif
/*distance <2-255> ��Ϊֱ����distanceΪ0
��̬·�ɵ�distanceΪ1��rip��distance���ܱ����Ǹ�*/
struct cmds rip_cmds_distance_val_tab[] =
{
	{ 
		"1234567890",
		MATCH_DIG,
		cmdPref(PF_CMDNO, 0, 0),
		0,
		rip_cmd_distance_val,
		NULL,
		NULL, 
		0,
		0,
		"<1-255>           -- Administrative distance",
		"<1-255>           -- �������", 
		NULLCHAR,
		NULLCHAR
	},
	{ NULLCHAR }
};

/*timers {update | expire | holddown} ����*/
struct cmds rip_cmds_timer_tab[] =
{
	{ "holddown", MATCH_AMB, cmdPref(PF_CMDNO, PF_CMDDEF, 0), 0,
		rip_cmd_timer_hold, NULL, NULL, 
		0, 0,
		"holddown          -- Set holddown interval",
		"holddown          -- ���ñ��ֶ�ʱ����ʱʱ��", 
		NULLCHAR, NULLCHAR
	},
	{ "expire", MATCH_AMB, cmdPref(PF_CMDNO, PF_CMDDEF, 0), 0,
		rip_cmd_timer_expire, NULL, NULL, 
		0, 0,
		"expire            -- Set expire interval",
		"expire            -- ������Чʱ�䶨ʱ����ʱʱ��", 
		NULLCHAR, NULLCHAR
	},
	{ "update", MATCH_AMB, cmdPref(PF_CMDNO, PF_CMDDEF, 0), 0,
		rip_cmd_timer_update, NULL, NULL, 
		0, 0,
		"update            -- Set interval of routing updates",
		"update            -- ���ø��±��ķ���ʱ����", 
		NULLCHAR, NULLCHAR
	},
	{ "trigger", MATCH_AMB, cmdPref(PF_CMDNO, PF_CMDDEF, 0), 0,
		rip_cmd_timer_trigger, NULL, NULL, 
		0, 0,
		"trigger    -- Set interval of routing trigger",
		"trigger    -- ���ô������¶�ʱ��ʱ����", 
		NULLCHAR, NULLCHAR
	},	
	{ "peer", MATCH_AMB, cmdPref(PF_CMDNO, PF_CMDDEF, 0), 0,
		rip_cmd_timer_peer, NULL, NULL, 
		0, 0,
		"peer       -- Set interval of peer time out",
		"peer       -- ����peer��ʱ��ʱ��ʱ����", 
		NULLCHAR, NULLCHAR
	},
	{ NULLCHAR }
};

/*timers update <1-4294967295>����*/
struct cmds rip_cmds_timer_update_val_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_timer_update_val, NULL, NULL, 
		0, 0,
		"<1-2147483647>    -- Interval(in second)",
		"<1-2147483647>    -- ʱ����(��)", 
		NULLCHAR, NULLCHAR
	},
	{ "<cr>", MATCH_END, cmdPref(PF_CMDNO, 0, PF_NOCMDNOR), 0,
		NULL, NULL, NULL, 
		0, 0,
		"<cr>", 
		"<cr>", 
		NULLCHAR, NULLCHAR
	},
	{ NULLCHAR }
};

/*timers expire <1-4294967295>����*/
struct cmds rip_cmds_timer_expire_val_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_timer_expire_val, NULL, NULL, 
		0, 0,
		"<1-2147483647>    -- Interval(in second)",
		"<1-2147483647>    -- ʱ����(��)", 
		NULLCHAR, NULLCHAR
	},
	{ "<cr>", MATCH_END, cmdPref(PF_CMDNO, 0, PF_NOCMDNOR), 0,
		NULL, NULL, NULL, 
		0, 0,
		"<cr>", 
		"<cr>", 
		NULLCHAR, NULLCHAR
	},
	{ NULLCHAR }
};

/*timers holddown <1-4294967295>����*/
struct cmds rip_cmds_timer_hold_val_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_timer_hold_val, NULL, NULL, 
		0, 0,
		"<1-2147483647>    -- Interval(in second)",
		"<1-2147483647>    -- ʱ����(��)", 
		NULLCHAR, NULLCHAR
	},
	{ "<cr>", MATCH_END, cmdPref(PF_CMDNO, 0, PF_NOCMDNOR), 0,
		NULL, NULL, NULL, 
		0, 0,
		"<cr>", 
		"<cr>", 
		NULLCHAR, NULLCHAR
	},
	{ NULLCHAR }
};
struct cmds rip_cmds_peer_timer_val_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_timer_peer_val, NULL, NULL, 
		0, 0,
		"<0 - 1000>           -- Interval(in second)",
		"<0 - 1000>           -- ʱ����(��)", 
		NULLCHAR, NULLCHAR
	},
	{ "<cr>", MATCH_END, cmdPref(PF_CMDNO, 0, PF_NOCMDNOR), 0,
		NULL, NULL, NULL, 
		0, 0,
		"<cr>", 
		"<cr>", 
		NULLCHAR, NULLCHAR
	},
	{ NULLCHAR }
};
struct cmds rip_cmds_trigger_timer_val_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_timer_trigger_val, NULL, NULL, 
		0, 0,
		"<0 - 5>           -- Interval(in second)",
		"<0 - 5>           -- ʱ����(��)", 
		NULLCHAR, NULLCHAR
	},
	{ "<cr>", MATCH_END, cmdPref(PF_CMDNO, 0, PF_NOCMDNOR), 0,
		NULL, NULL, NULL, 
		0, 0,
		"<cr>", 
		"<cr>", 
		NULLCHAR, NULLCHAR
	},
	{ NULLCHAR }
};
/*�ӿ�����ģʽip rip����*/
struct cmds rip_cmds_intf_tab[] = 
{
	{ 
		"rip",
		MATCH_AMB,
		cmdPref(PF_CMDNO, PF_CMDDEF, 0),
		0,
		rip_cmd_intf_rip,
		NULL,
		NULL,
		0,
		0,
		"rip               -- Set RIP parameter on interface", 
		"rip               -- �ڶ˿�������RIP����", 
		NULLCHAR,
		NULLCHAR
	},

  {NULLCHAR}
};

int  rip_neigh_cmd_key(int argc, char **argv, struct user *u);
int rip_neigh_cmd_dynamic_authen(int argc, char **argv, struct user *u);

/*RIP�ӿ���������*/
struct cmds rip_cmds_intf_rip_tab[] = 
{
	{ 
		"0123456789", 
		MATCH_DIG, 
		cmdPref(PF_CMDNO, 0, 0),
		0,
		rip_cmd_intf_rip_process,
		NULL,
		NULL,
		0,
		0,
		"<1-65535>              -- Special the process id",
		"<1-65535>              -- ����RIP ���̺�",
		NULLCHAR, 
		NULLCHAR
	},
	{ 
		"authentication",
		MATCH_AMB,
		cmdPref(PF_CMDNO, 0, 0),
		0,
		rip_cmd_intf_auth,
		NULL,
		NULL,
		0,
		0,
		"authentication      -- Set authentication mode",
		"authentication      -- ������֤ģʽ ", 
		NULLCHAR,
		NULLCHAR
	},

	{
		"md5-key", 
		MATCH_AMB, 
		cmdPref(PF_CMDNO, 0, 0), 
		0,
		rip_cmd_intf_md5, 
		NULL, 
		NULL, 
		0, 
		0,
		"md5-key             -- Set md5 authentication key and key-id",
		"md5-key             -- ����MD5��֤����Կ����֤ID ", 
		NULLCHAR, 
		NULLCHAR
	},
	{ 
		"dynamic-key", 
		MATCH_AMB,
		cmdPref(PF_CMDNO, 0, 0), 
		0,
		rip_neigh_cmd_key, 
		NULL, 
		NULL, 
		0, 
		0,
		"dynamic-key         -- Set  dynamic authentication key",
		"dynamic-key         -- ���ö�̬��֤��Կ", 
		NULLCHAR, 
		NULLCHAR
	},

	{ 
		"passive", 
		MATCH_AMB, 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		0,
		rip_cmd_intf_passive, 
		NULL, 
		NULL, 
		0, 
		0,
		"passive             -- Only receive Update on the interface",
		"passive             -- �ڶ˿���ֻ����Update���� ", 
		NULLCHAR, 
		NULLCHAR
	},

	{ 
		"deaf", 
		MATCH_AMB, 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		0,
		rip_cmd_intf_deaf, 
		NULL, 
		NULL, 
		0, 
		0,
		"deaf                -- not receive any rip packet on the interface",
		"deaf                -- �˿��ϲ�����rip����", 
		NULLCHAR, 
		NULLCHAR
	},

	{ 
		"v1demand", 
		MATCH_AMB, 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		0,
		rip_cmd_intf_v1demand, 
		NULL, 
		NULL, 
		0, 
		0,
		"v1demand            -- use v1 format when send request on the interface",
		"v1demand            -- �ڷ���requestʱ��ֻ����v1��ʽ�ı���", 
		NULLCHAR, 
		NULLCHAR
	},

	{ 
		"v2demand", 
		MATCH_AMB, 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		0,
		rip_cmd_intf_v2demand, 
		NULL, 
		NULL, 
		0, 
		0,
		"v2demand            -- use v2 format when send request on the interface",
		"v2demand            -- �ڷ���requestʱ��ֻ����v2��ʽ�ı���", 
		NULLCHAR, 
		NULLCHAR
	},
		
	
	{ 
		"password", 
		MATCH_AMB, 
		cmdPref(PF_CMDNO, 0, 0), 
		0,
		rip_cmd_intf_simple,
		NULL, 
		NULL, 
		0, 
		0,
		"password            -- Set simple authentication password",
		"password            -- ����������֤��Կ ", 
		NULLCHAR,
		NULLCHAR
	},

	{ 
		"receive", 
		MATCH_AMB, 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		0,
		rip_cmd_intf_recv, 
		NULL, 
		NULL, 
		0, 
		0,
		"receive             -- Set receive version on the interface",
		"receive             -- ���ö˿ڽ��հ汾", 
		NULLCHAR, 
		NULLCHAR
	},

	{
		"send",
		MATCH_AMB, 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		0,
		rip_cmd_intf_send, 
		NULL, 
		NULL, 
		0, 
		0,
		"send                -- Set send version on the interface",
		"send                -- ���ö˿ڷ��Ͱ汾", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ 
		"split-horizon",
		MATCH_AMB,
		cmdPref(PF_CMDNO, PF_CMDDEF, 0),
		0,
		rip_cmd_intf_split,
		NULL,
		NULL,
		0,
		0,
		"split-horizon       -- Set split horizon on the interface",
		"split-horizon       -- ���ö˿�ˮƽ�ָ�",
		NULLCHAR,
		NULLCHAR
	},
	{ 
		"bfd-enable", 
		MATCH_AMB, 
		cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
		0,
		rip_cmd_intf_bfd_enable, 
		NULL, 
		NULL, 
		0, 
		0,
		"bfd-enable       -- Bind bfd on the interface",
		"bfd-enable       -- �ڶ˿��ϰ󶨹�עbfd ״̬", 
		NULLCHAR, 
		NULLCHAR
	},
	
  	{NULLCHAR}
};

struct cmds rip_cmd_intf_rip_process_tab[] =
{
	{
		 "enable",
		 MATCH_AMB,
		 cmdPref(PF_CMDNO, PF_CMDDEF, 0),
		 0,
		 rip_cmd_intf_rip_process_enable, 
		 NULL,
		 NULL,
		 0,
		 0,
		 "enable  -- enable/disable RIP process",
		 "enable  -- �ڶ˿�����/ֹͣRIP����",
		 NULLCHAR,
		 NULLCHAR
	},

	{NULLCHAR}
};

int rip_cmds_intf_authen_commit(int argc, char **argv, struct user *u);

/*�ӿ�����ģʽ��ip rip authen �µ�����*/
struct cmds rip_cmds_intf_auth_tab[] =
{
	{ 
		"md5",
		MATCH_AMB,
		0,
		0,
		rip_cmd_intf_auth_md5,
		NULL,
		NULL,
		0,
		0,
		"md5                 -- MD5 authentication",
		"md5                 -- MD5��֤", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ 
		"simple",
		MATCH_AMB,
		0,
		0,
		rip_cmd_intf_auth_simple,
		NULL,
		NULL,
		0,
		0,
		"simple              -- Simple authentication",
		"simple              -- ������֤", 
		NULLCHAR,
		NULLCHAR
	},
	{ 
		"dynamic", 
		MATCH_AMB,
		0,
		0,
		rip_neigh_cmd_dynamic_authen,
		NULL,
		NULL,
		0,
		0,
		"dynamic             -- Enable interface dynamic authentication",
		"dynamic             -- �����˿ڶ�̬��֤", 
		NULLCHAR, 
		NULLCHAR
	},
	{ 
		"commit", 
		MATCH_AMB,
		0,
		0,
		rip_cmds_intf_authen_commit,
		NULL,
		NULL,
		0,
		0,
		"commit             -- Finish and commit authentication",
		"commit             -- ��֤������ϲ��ύ", 
		NULLCHAR, 
		NULLCHAR
	},
	
	{ "<cr>", MATCH_END, cmdPref(PF_CMDNO, 0, PF_NOCMDNOR), 0,
		NULL, NULL, NULL, 
		0, 0,
		"<cr>",
		"<cr>",
		NULLCHAR, NULLCHAR
	},

  {NULLCHAR}
};



/*�ӿ�����ģʽ��ip rip password�µ�����*/
struct cmds rip_cmds_intf_pswd_tab[] =
{
	{ 
		"1234567890",
		MATCH_STR,
		0,
		0,
		rip_cmd_intf_simple_key,
		NULL,
		NULL,
		0,
		0,
		"WORD              -- Authentication key(16 char)", 
		"WORD              -- ��֤��Կ(16���ַ�)", 
		NULLCHAR,
		NULLCHAR
	},
	{ "<cr>", MATCH_END, cmdPref(PF_CMDNO, 0, PF_NOCMDNOR), 0,
		NULL, NULL, NULL, 
		0, 0,
		"<cr>",
		"<cr>",
		NULLCHAR, NULLCHAR
	},
  	{NULLCHAR}
};

/*�ӿ�����ģʽ��ip rip message_digest_key �µ�����*/
struct cmds rip_cmds_intf_md5_tab[] =
{
	{ 
		"01234567890",
		MATCH_DIG,
		cmdPref(PF_CMDNO, 0, 0),
		0,
		rip_cmd_intf_md5_keyid,
		NULL,
		NULL,
		0,
		0,
		"<0-255>           -- key-ID", 
		"<0-255>           -- ��֤ID", 
		NULLCHAR,
		NULLCHAR
	},
	{ "<cr>", MATCH_END, cmdPref(PF_CMDNO, 0, PF_NOCMDNOR), 0,
		NULL, NULL, NULL, 
		0, 0,
		"<cr>",
		"<cr>",
		NULLCHAR, NULLCHAR
	},
  	{NULLCHAR}
};

/*�ӿ�����ģʽ��ip rip message_digest_key <key_id>�µ�����*/
struct cmds rip_cmds_intf_md5_keyid_tab[] =
{
	{ 
		"md5",
		MATCH_AMB,
		cmdPref(PF_CMDNO, 0, 0),
		0,
		rip_cmd_intf_md5_keyid_md5,		
		NULL,
		NULL,
		0,
		0,
		"md5               -- Md5", 
		"md5               -- Md5�㷨", 
		NULLCHAR,
		NULLCHAR
	},

  	{NULLCHAR}
};

/*�ӿ�����ģʽ��ip rip message_digest_key <key_id> md5�µ�����*/
struct cmds rip_cmds_intf_md5_keyid_md5_tab[] =
{
	 {"0", MATCH_AMB, cmdPref(PF_CMDNO, 0, 0), RIP_CMD_MASKBIT,
		rip_cmd_intf_md5_keyid_md5_normal, NULL, NULL,
		0, 0,
		"0              -- Specifies an UNENCRYPTED password will follow",
		"0              -- δ����ֵ",
		NULLCHAR, NULLCHAR
	},
	{ "7", MATCH_AMB, cmdPref(PF_CMDNO, 0, 0),RIP_CMD_MASKBIT<<1,
		rip_cmd_intf_md5_keyid_md5_hidden, NULL, NULL,
		0, 0,
		"7              -- Specifies a HIDDEN password will follow",
		"7              -- ����ֵ",
		NULLCHAR, NULLCHAR
	},
	{ 
		"1234567890",
		MATCH_STR,
		cmdPref(PF_CMDNO, 0, 0),
		RIP_CMD_MASKBIT<<2,
		rip_cmd_intf_md5_keyid_md5_str,		
		NULL,
		NULL,
		0,
		0,
		"WORD              -- key(16 char)", 
		"WORD              -- ��֤��Կ(16���ַ�)", 
		NULLCHAR,
		NULLCHAR
	},

  	{NULLCHAR}
};

/*ip rip send version����*/
struct cmds rip_cmds_intf_send_tab[] =
{ 
	{ 
		"version",
		MATCH_AMB,
		cmdPref(PF_CMDNO, PF_CMDDEF, 0),
		0,
		rip_cmd_intf_send_ver,
		NULL,
		NULL, 
		0,
		0,
		"version           -- Send version control",
		"version           -- ���Ͱ汾����", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ NULLCHAR }
};


/*ip rip send version <1-2>����*/
struct cmds rip_cmds_intf_send_ver_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_intf_send_ver_val, NULL, NULL, 
		0, 0,
		"<1-2>             -- Send version",
		"<1-2>             -- ���Ͱ汾",
		NULLCHAR, NULLCHAR
	},
	
	{
		"compatibility",
		MATCH_AMB,
		0,
		0,
		rip_cmd_intf_send_comp,
		NULL,
		NULL, 
		0,
		0, 
		"compatibility     -- Send version 2 update broadcast",
		"compatibility     -- �㲥����RIP�汾2���±���", 
		NULLCHAR,
		NULLCHAR
	},
	
	{ "<cr>", MATCH_AMB, cmdPref(PF_CMDNO, PF_CMDDEF, PF_NOCMDNOR), 0,
		NULL, NULL, NULL, 
		0, 0,
		"<cr>", 
		"<cr>", 
		NULLCHAR, NULLCHAR
	},
	{ NULLCHAR }
};

/*ip rip receive version����*/
struct cmds rip_cmds_intf_recv_tab[] =
{ 
	{  
		"version",
		MATCH_AMB,
		cmdPref(PF_CMDNO, PF_CMDDEF, 0),
		0,
		rip_cmd_intf_recv_ver,
		NULL,
		NULL,
		0,
		0,
		"version           -- Receive version control",
		"version           -- ���հ汾����",
		NULLCHAR,
		NULLCHAR
	},
	{ NULLCHAR }
};

/*ip rip receive version <1-2>����*/
struct cmds rip_cmds_intf_recv_ver_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_intf_recv_ver_val, NULL, NULL, 
		0, 0,
		"<1-2>             -- Receive version",
		"<1-2>             -- ���հ汾",
		NULLCHAR, NULLCHAR
	},
	
	{ "<cr>", 
	  MATCH_END, 
	  cmdPref(PF_CMDNO, 0, PF_NOCMDNOR), 
	  0,NULL,NULL,NULL,0,0,
	  "<cr>", 
	  "<cr>", 
	  NULLCHAR, 
	  NULLCHAR
	},
	{ NULLCHAR }
};

/*ip rip receive version 1 2����*/
struct cmds rip_cmds_intf_recv_ver_1_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_intf_recv_ver_12, NULL, NULL, 
		0, 0,
		"2                 -- Receive version 2",
		"2                 -- ���հ汾2",
		NULLCHAR, NULLCHAR
	},
	
	{ "<cr>", 
	  MATCH_END, 
	  0, 
	  0,NULL,NULL,NULL,0,0,
	  "<cr>", 
	  "<cr>", 
	  NULLCHAR, 
	  NULLCHAR
	},
	{ NULLCHAR }
};

/*ip rip receive version 2 1����*/
struct cmds rip_cmds_intf_recv_ver_2_tab[] =
{
	{ "1234567890", MATCH_DIG, 0, 0,
		rip_cmd_intf_recv_ver_12, NULL, NULL, 
		0, 0,
		"1                 -- Receive version 1",
		"1                 -- ���հ汾1",
		NULLCHAR, NULLCHAR
	},
	
	{ "<cr>", 
	  MATCH_END, 
	  0, 
	  0,NULL,NULL,NULL,0,0,
	  "<cr>", 
	  "<cr>", 
	  NULLCHAR, 
	  NULLCHAR
	},
	{ NULLCHAR }
};

/*ip rip split-horizon {simple | poisoned}����*/
struct cmds rip_cmds_intf_split_tab[] =
{
	{ "simple", MATCH_AMB, cmdPref(PF_CMDNO, 0, 0), 0,
		rip_cmd_intf_split_simple, NULL, NULL, 
		0, 0,
		"simple            -- Simple split-horizon",
		"simple            -- ��ˮƽ�ָ�",
		NULLCHAR, NULLCHAR
	},
	
	{ "poisoned", MATCH_AMB, cmdPref(PF_CMDNO, 0, 0), 0,
		rip_cmd_intf_split_poisoned, NULL, NULL, 
		0, 0,
		"poisoned          -- Split horizon with poisoned reverse",
		"poisoned          -- ������תˮƽ�ָ�",
		NULLCHAR, NULLCHAR
	},
	
	{ "<cr>", 
	  MATCH_END, 
	  cmdPref(0, 0, PF_NOCMDNOR), 
	  0,NULL,NULL,NULL,0,0,
	  "<cr>", 
	  "<cr>", 
	  NULLCHAR, 
	  NULLCHAR
	},
	
	{ NULLCHAR }
};



/*******************************************************************************
 *
 * FUNCTION	:	rip_neighb_day_check
 *
 * PARAMS		:	key			- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.02.25 14:15:31
 *
*******************************************************************************/
int rip_neighb_day_check(struct dynamic_key_list_ *key)
{
	int ret =0;

	if((key->start_time.year %4 == 0 && key->start_time.year %100 !=0) ||key->start_time.year %400 ==0)
		/*����*/
		ret =1;

	switch(key->start_time.month)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			if(key->start_time.day >31)
				return RIP_FAIL;
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			if(key->start_time.day >30)
				return RIP_FAIL;
			break;
		case 2:
			if(ret)
			{
				if(key->start_time.day >29)
					return RIP_FAIL;
			}
			if(!ret)
			{
				if(key->start_time.day >28)
					return RIP_FAIL;
			}
			break;
	}

	return RIP_SUCCESS;
}



/*******************************************************************************
 *
 * FUNCTION	:	rip_intf_cmd_start_time_save
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.03.04 19:29:24
 *
*******************************************************************************/
int rip_intf_cmd_start_time_save(char *time_str , struct dynamic_key_list_ *key)
{
	int ret =0;
	int i = 1;
	int i_1 = 0;
	int i_2 = 0;
	int i_3 = 0;
	char a;
	char time[5];
	char temp[20];
	char *sp;
	char *time_ptr;

	memset(time , 0 , 5);
	memset(temp , 0 , 20 );
	strcpy(temp,time_str);

	sp = temp;

	/*ʱ���ʽ����*/
	while(*sp)
	{
		a = *sp;
		
		if(a == '-')
		{
			if(i_2||i)return RIP_FAIL;
			i_1++;
			i=1;
		}
		else if(a ==':')
		{
			if((i_1!=3)||(i!=0))return RIP_FAIL;
			i_2++;
			i=2;
		}
		else if((a<'0')||(a>'9'))return RIP_FAIL;
		else i=0;
		sp++;
	}

	if((i_1!=3)||(i_2!=1)||(i!=0))return RIP_FAIL;

	sp = temp;
	time_ptr = time;

	/*����key�Ŀ�ʼ��Чʱ��*/
	for(i=1 ; i<6 ; i++)
	{
		a = *sp;
		
		while(a != '-' && a !=':'  && a)
		{	
			*time_ptr = a;
			time_ptr++ ;
			i_3++;
							
			if(i_3>4)
			{	
				return RIP_FAIL;
			}
			sp++;
			a = *sp;
		}
		if(i_3==0)return RIP_FAIL;
		*time_ptr = '\0';
		
		switch(i)
		{
			case 1:
				if(strlen(time) <= 2)
					key->start_time.year = atoi(time) + 2000;
				else
					key->start_time.year = atoi(time);
				if(key->start_time.year>2111 ||key->start_time.year<2000)
					return RIP_FAIL;
				break;
			case 2:
				key->start_time.month = atoi(time);
				if(key->start_time.month>12||key->start_time.month==0)
					return RIP_FAIL;
				break;
			case 3:
				key->start_time.day = atoi(time);
				if(key->start_time.day==0)return RIP_FAIL;
				ret =rip_neighb_day_check(key);
				if(ret !=RIP_SUCCESS)return RIP_FAIL;
				break;
			case 4:
				key->start_time.hour = atoi(time);
				if(key->start_time.hour>=24)return RIP_FAIL;
				break;
			case 5:
				key->start_time.minute = atoi(time);
				if(key->start_time.minute>=60)return RIP_FAIL;
				break;
		}

		i_3 = 0;
		memset(time , 0 , 5 );
		time_ptr = time;
		sp++;

	}

	return RIP_SUCCESS;
}



/*******************************************************************************
 *
 * FUNCTION	:	rip_intf_cmd_lift_time_save
 *
 * PARAMS		:	time_str	- 
 *					key			- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.03.04 19:31:19
 *
*******************************************************************************/
int rip_intf_cmd_lift_time_save(char *time_str,struct dynamic_key_list_ *key)
{
	int lift_time = 0;
	char *temp = NULL;
	char time[4]={0};
	int i=0,j=0;
	char a;

	temp=time_str;
	a=*temp;
	while(a)
	{
		if(a==':')
		{
			if(j)return RIP_FAIL;
			j++;
			i=temp-time_str;
		}
		else if(a<'0'||a>'9')return RIP_FAIL;
		temp++;
		a=*temp;
	}
	if(i<=0||i>=4||j!=1)return RIP_FAIL;
	j=(temp-time_str)-i-1;
	if(j<=0||j>=3)return RIP_FAIL;
	memcpy(time,time_str,i);
	lift_time =atoi(time);
	if(lift_time>256 || lift_time<0)
	{
		printf("warning: key's lift time is too long. adding key fail.\n");
		return RIP_FAIL;
	}
	else key->lift_time.hour = lift_time;
	
	memset(time,0,4);
	memcpy(time,time_str+i+1,j);
	lift_time =atoi(time);
	if(lift_time>=60||lift_time<0)
	{
		printf("warning: key's lift time's minute's format is illegal. adding key fail.\n");
		return RIP_FAIL;
	}
	else if(lift_time==0&&key->lift_time.hour==0)
	{
		printf("warning: key's lift time can't be zero. adding key fail.\n");
		return RIP_FAIL;
	}
	else key->lift_time.minute= lift_time;
	
	return RIP_SUCCESS;	
}

int rip_neighb_get_currect_time(struct key_start_time_ *currect_time)
{
	uint32 date = 0;
	uint32 time = 0;
	
#ifdef OS_VXWORKS
	rtc_get(&date,&time);
#else 
	uint32 ticks;
	if(tm_get(&date, &time,  &ticks))return RIP_FAIL;
#endif

	currect_time->day = date & 0x000000ff;
	
	currect_time->month = (date>>8) & 0x000000ff;

	currect_time->year = (date>>16) & 0x0000ffff;

	currect_time->second =time & 0x000000ff;

	currect_time->minute = (time>>8) & 0x000000ff;

	currect_time->hour = (time>>16) & 0x0000ffff;

	return RIP_SUCCESS;
	
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_neighb_key_time_switch
 *
 * PARAMS		:	key_forw	- 
 *
 * RETURN		:	
 *
 * NOTE			:	����¼��ʱ��ת��Ϊ��2000�꿪ʼ������Ϊ��λ������
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.02.25 15:26:10
 *
*******************************************************************************/
uint32 rip_neighb_key_time_switch(struct key_start_time_ *key_forw)
{
	uint16 year = 2000;
	uint32 sum_forw =0;
	

	while(key_forw->year - year )
	{
		if((year %4 == 0 && year %100 !=0) ||year %400 ==0)
			sum_forw =sum_forw +366*24*60;
		else
			sum_forw = sum_forw +365*24*60;
		year++;
	}

	switch(key_forw->month)
	{
		case 1:
			sum_forw =sum_forw + (key_forw->day -1) *24*60 +key_forw->hour*60 +key_forw->minute;
			break;
		case 2:
			sum_forw =sum_forw + 31*24*60 +(key_forw->day -1) *24*60 +key_forw->hour*60 +key_forw->minute;
			break;
		case 3:
			sum_forw =sum_forw + (31+28)*24*60 +(key_forw->day -1) *24*60 +key_forw->hour*60 +key_forw->minute;
			break;
		case 4:
			sum_forw =sum_forw + (31+28+31)*24*60 +(key_forw->day -1) *24*60 +key_forw->hour*60 +key_forw->minute;
			break;
		case 5:
			sum_forw =sum_forw + (31+28+31+30)*24*60 +(key_forw->day -1) *24*60 +key_forw->hour*60 +key_forw->minute;
			break;
		case 6:
			sum_forw =sum_forw + (31+28+31+30+31)*24*60 +(key_forw->day -1) *24*60 +key_forw->hour*60 +key_forw->minute;
			break;
		case 7:
			sum_forw =sum_forw + (31+28+31+30+31+30)*24*60 +(key_forw->day -1) *24*60 +key_forw->hour*60 +key_forw->minute;
			break;
		case 8:
			sum_forw =sum_forw + (31 +28+31+30+31+30+31)*24*60 +(key_forw->day -1) *24*60 +key_forw->hour*60 +key_forw->minute;
			break;
		case 9:
			sum_forw =sum_forw + (31 +28+31+30+31+30+31+31)*24*60 +(key_forw->day -1) *24*60 +key_forw->hour*60 +key_forw->minute;
			break;
		case 10:
			sum_forw =sum_forw + (31 +28+31+30+31+30+31+31+30)*24*60 +(key_forw->day -1) *24*60 +key_forw->hour*60 +key_forw->minute;
			break;
		case 11:
			sum_forw =sum_forw + (31 +28+31+30+31+30+31+31+30+31)*24*60 +(key_forw->day -1) *24*60 +key_forw->hour*60 +key_forw->minute;
			break;
		case 12:
			sum_forw =sum_forw + (31 +28+31+30+31+30+31+31+30+31+30)*24*60 +(key_forw->day -1) *24*60 +key_forw->hour*60 +key_forw->minute;
			break;
	}

	if(((key_forw->year %4 == 0 && key_forw->year %100 !=0) 
			||key_forw->year %400 ==0) && key_forw->month >2)
		sum_forw =sum_forw+24*60;

	sum_forw =sum_forw*60 + key_forw->second;
	return sum_forw;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_neighb_time_cmp
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	����Ϊ��λ�������߼�Ĳ�ֵ
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.02.25 10:01:52
 *
*******************************************************************************/
int rip_neighb_time_cmp(struct key_start_time_*key_forw ,struct key_start_time_ *key_back)
{
	int32 dispersion;
	uint32 sumforw;
	uint32 sumback;

	sumforw = rip_neighb_key_time_switch(key_forw);
	sumback = rip_neighb_key_time_switch( key_back);

	dispersion =sumforw -sumback;

	return dispersion;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_neighb_add_key
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.02.24 10:53:29
 *
*******************************************************************************/
int rip_neighb_add_key(struct dynamic_key_list_ *key ,uint32 device_index)
{
	int32 ret;
	int32 lifttime;
	struct dynamic_key_list_ *key_temp;
	struct key_start_time_ currect_time;

	memset(&currect_time ,0 ,sizeof(struct key_start_time_));	
	ret =rip_neighb_get_currect_time(&currect_time);
	if(ret !=RIP_SUCCESS)return RIP_FAIL;

	ret =rip_neighb_time_cmp(&key->start_time ,&currect_time);
	if(ret>0)key->key_state =RIP_NEIGHBOR_KEY_VALID;
	else
	{
		lifttime = key->lift_time.hour * 60*60 + key->lift_time.minute*60;

		if( lifttime > abs( ret ) )
			key->key_state =RIP_NEIGHBOR_KEY_ACTIVE;
		else
		{
			key->key_state=RIP_NEIGHBOR_KEY_EXPIRED;
			INSQUE(key , rip_intf_array[device_index]->key_timeout_list.forw);
			return RIP_SUCCESS;
		}
	}
		
	key_temp =  rip_intf_array[device_index]->key_list.forw;
	while(key_temp != &( rip_intf_array[device_index]->key_list))
	{
		if(rip_neighb_time_cmp(&key_temp->start_time ,&key->start_time) >= 0)
			break;
		key_temp = key_temp->forw;
	}

	if(key_temp == &( rip_intf_array[device_index]->key_list))
	{
		INSQUE(key,  rip_intf_array[device_index]->key_list.back);
	}
	else
	{
		INSQUE(key, key_temp->back);
	}
	return RIP_SUCCESS;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_intf_add_keyid_check
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.03.04 19:25:56
 *
*******************************************************************************/
int rip_intf_add_keyid_check(struct dynamic_key_list_ *key , uint32 device_index)
{	
	struct dynamic_key_list_ *key_temp;

	for(key_temp = rip_intf_array[device_index]->key_list.forw;
		key_temp !=&rip_intf_array[device_index]->key_list; 
		key_temp =key_temp->forw)
	{
		if(key_temp->key_id == key->key_id)
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}
	}

	for(key_temp=rip_intf_array[device_index]->key_timeout_list.forw;
		key_temp!=&(rip_intf_array[device_index]->key_timeout_list);
		key_temp=key_temp->forw)
	{
		if(key->key_id==key_temp->key_id)
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}
	}
		
	return RIP_SUCCESS;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_neigh_cmd_do_key_lift_time
 *
 * PARAMS		:	argc		- 
 *					argv		- 
 *					u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.02.23 17:00:31
 *
*******************************************************************************/
int  rip_intf_cmd_do_key_lift_time(int argc, char **argv, struct user *u)
{
	int ret,i;
	char *keytype;
	char time_str[20] = {'\0'};
	struct dynamic_key_list_ *key;
	struct parameter param;
	uint32 u4Flags;
	u_char md5_temp[20+1];

	uint32 device_index;

	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)return ret;

	u4Flags = u->struct_p[2];
	device_index = u->struct_p[1];
	if(rip_intf_array[device_index]==NULL)
	{
       	return RIP_FAIL;
	}

	/*�����µ� key*/
	key = (struct dynamic_key_list_ *)rip_mem_malloc( sizeof(struct dynamic_key_list_), RIP_NEIGHB_KEY_TYPE );
	if( key == NULL )
	{
		/*�ڴ����ʧ��*/
		fprintf(stderr, "RIP: Malloc memory failed.(%s, %d)\n", __FILE__, __LINE__ );
		return RIP_MEM_FAIL;
	}
	memset( key, 0, sizeof(struct dynamic_key_list_) );
	QUE_INIT(key);
		
	/*Get  key_id*/
	param.type = ARG_UINT;
	param.min = 1;
	param.max = 255;
	param.flag = ARG_MIN | ARG_MAX;
	if(BIT_TEST(u4Flags, RIP_AUTH_MD5_KEY_HIDDEN)
		|| BIT_TEST(u4Flags, RIP_AUTH_MD5_KEY_NORMAL))
		ret = getparameter(argc + 6, argv - 6, u, &param);
	else
		ret = getparameter(argc + 5, argv - 5, u, &param);
	if(ret != 0)
	{
		rip_mem_free( key, RIP_NEIGHB_KEY_TYPE );
		return ret;
	}
	key->key_id = param.value.v_int;
	ret = rip_intf_add_keyid_check(key, device_index);
	if(ret != RIP_SUCCESS)
	{
		printf("warning: there has been a key with this keyid,adding fail!\n");
		rip_mem_free( key, RIP_NEIGHB_KEY_TYPE );
		return RIP_FAIL;
	}

	/*get keyid's algorithems */
	param.type = ARG_STR;
	if(BIT_TEST(u4Flags, RIP_AUTH_MD5_KEY_HIDDEN)
		|| BIT_TEST(u4Flags, RIP_AUTH_MD5_KEY_NORMAL))
		ret = getparameter(argc + 5, argv - 5, u, &param);
	else
		ret = getparameter(argc + 4, argv - 4, u, &param);
	if(ret != 0)
	{
		rip_mem_free( key, RIP_NEIGHB_KEY_TYPE );
		return ret;
	}
	keytype = param.value.v_string;
	if( !strcmp(keytype,"md5") ||!strcmp(keytype,"m") ||!strcmp(keytype,"md") )
		key->algorithms = RIP_NEIGHB_MD5;
	else if(!strcmp(keytype,"sha1")||!strcmp(keytype,"s")||!strcmp(keytype,"sh")||!strcmp(keytype,"sha"))
		key->algorithms = RIP_NEIGHB_SHA1;

	/*get key value */
	param.type = ARG_STR;
	ret = getparameter(argc + 3, argv - 3, u, &param);
	if(ret != 0)
	{
		rip_mem_free( key, RIP_NEIGHB_KEY_TYPE );
		return ret;
	}
	
	if(key->algorithms == RIP_NEIGHB_MD5)
	{
		if(BIT_TEST(u4Flags, RIP_AUTH_MD5_KEY_HIDDEN))
		{
			i=strlen(param.value.v_string);
			if(i > 34 || i <= 0)ret=RIP_FAIL;
		}
		else
		{
			i=strlen(param.value.v_string);
			if(i > 16 || i <= 0)ret=RIP_FAIL;
		}
	}
	else if(key->algorithms == RIP_NEIGHB_SHA1)
	{
		i=strlen(param.value.v_string);
		if(i > 20 || i <= 0)ret=RIP_FAIL;
	}
	if(ret!=RIP_SUCCESS)
	{
		printf("warning:the  key is too long,adding fail!\n");
		rip_mem_free( key, RIP_NEIGHB_KEY_TYPE );
		return RIP_FAIL;
	}
	
	BIT_SET(key->md5_type, u4Flags);
	memset(md5_temp, 0, 20+1);
	if(BIT_TEST(u4Flags, RIP_AUTH_MD5_KEY_HIDDEN))
	{
		if(um_pass_string_crypt((u_char *)param.value.v_string,  md5_temp))
		{	
			vty_output("%%rip md5 invalid encrypted key.\n");
			rip_mem_free( key, RIP_NEIGHB_KEY_TYPE );
			return RIP_FAIL;
		}
	}
	else
	{	
		memcpy(md5_temp,  param.value.v_string, strlen(param.value.v_string));
	}
	memset(key->key , 0 , 20);
	strcpy(key->key , (char *)md5_temp);
	
	/*get key start time */
	param.type = ARG_STR;
	ret = getparameter(argc + 2, argv - 2, u, &param);
	if(ret != 0)
	{
		rip_mem_free( key, RIP_NEIGHB_KEY_TYPE );
		return ret;
	}
	i=strlen(param.value.v_string);
	if(i>18||i<=0)
	{
		rip_mem_free( key, RIP_NEIGHB_KEY_TYPE );
		return RIP_FAIL;
	}
	strcpy(time_str , param.value.v_string);
	ret = rip_intf_cmd_start_time_save(time_str , key);
	if(ret != RIP_SUCCESS)
	{
		printf("error: fault key start_time format!adding fail!\n");
		rip_mem_free( key, RIP_NEIGHB_KEY_TYPE );
		return RIP_FAIL;
	}

	/*get key's lift time*/
	param.type = ARG_STR;
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if(ret != 0)
	{
		rip_mem_free( key, RIP_NEIGHB_KEY_TYPE );
		return ret;
	}
	i=strlen(param.value.v_string);
	if(i>6||i<=0)
	{
		rip_mem_free( key, RIP_NEIGHB_KEY_TYPE );
		return RIP_FAIL;
	}
	memset(time_str,0,20);
	strcpy(time_str , param.value.v_string);
	ret = rip_intf_cmd_lift_time_save(time_str , key);
	if(ret != RIP_SUCCESS)
	{
		rip_mem_free( key, RIP_NEIGHB_KEY_TYPE );
		return RIP_FAIL;
	}

	/*��key��ӵ�neighbor ��key������*/
	ret = rip_neighb_add_key(key,device_index);
	if(ret != RIP_SUCCESS)
	{
		rip_mem_free( key, RIP_NEIGHB_KEY_TYPE );
		return RIP_FAIL;
	}

	sys_start_timer( rip_intf_array[device_index]->key_start_timer_id,60);
	sys_start_timer( rip_intf_array[device_index]->key_lift_timer_id, 60);

	return RIP_SUCCESS;
}

/*����dynamic_key�µ�key��Чʱ�䳤��������*/
struct cmds rip_neighbor_cmds_key_lift_time_tab[]  = 
{

	{ 
		"1234567890",
		MATCH_STR, 
		0, 
		0,
		rip_intf_cmd_do_key_lift_time,
		NULL,
		NULL,
		0,
		0,
		"xx:xx            -- hour:minute  (key's effective time length )", 
		"xx:xx             -- Сʱ:���� ��key����Чʱ�䳤�ȣ�",
		NULLCHAR,
		NULLCHAR
	},
	{
		"<cr>", 
	  	MATCH_END, 
	  	cmdPref(0, 0, PF_NOCMDNOR), 
	  	0,NULL,NULL,NULL,0,0,
	  	"<cr>", 
	  	"<cr>", 
	  	NULLCHAR, 
	 	 NULLCHAR
	},
		{ NULLCHAR }
};


int  rip_neigh_cmd_do_key_start_time(int argc, char **argv, struct user *u)
{
	return subcmd( rip_neighbor_cmds_key_lift_time_tab , NULL, argc, argv, u );
}

/*����dynamic_key�µ�key��Чʱ��������*/
struct cmds rip_neighbor_cmds_key_start_time_tab[]  = 
{
	{ 
		"1234567890",
		MATCH_STR, 
		0, 
		0,
		rip_neigh_cmd_do_key_start_time,
		NULL,
		NULL,
		0,
		0,
		"xxxx-xx-xx-xx:xx              -- year-month-day-hour:minute  (24h)", 
		"xxxx-xx-xx-xx:xx              -- ��-��-��-ʱ:�� ��24Сʱ�ƣ�",
		NULLCHAR,
		NULLCHAR
	},
	
	{
		"<cr>", 
	  	MATCH_END, 
	  	cmdPref(0, 0, PF_NOCMDNOR), 
	  	0,NULL,NULL,NULL,0,0,
	  	"<cr>", 
	  	"<cr>", 
	  	NULLCHAR, 
	 	NULLCHAR
	},
	
	{ NULLCHAR }
};

int  rip_neigh_cmd_do_key_value(int argc, char **argv, struct user *u)
{
	return subcmd( rip_neighbor_cmds_key_start_time_tab , NULL, argc, argv, u );
}

int rip_neigh_cmd_do_md5_key_normal(int argc, char * * argv, struct user * u);
int rip_neigh_cmd_do_md5_key_hidden(int argc, char * * argv, struct user * u);
/*����dynamic_key�µ�keyֵ������*/
struct cmds rip_neighbor_cmds_key_md5_tab[]  = 
{
	{"0", MATCH_AMB, 0,RIP_CMD_MASKBIT,
		rip_neigh_cmd_do_md5_key_normal, NULL, NULL,
		0, 0,
		"0              -- Specifies an UNENCRYPTED password will follow",
		"0              -- δ����ֵ",
		NULLCHAR, NULLCHAR
	},
	{ "7", MATCH_AMB, 0, RIP_CMD_MASKBIT<<1,
		rip_neigh_cmd_do_md5_key_hidden, NULL, NULL,
		0, 0,
		"7              -- Specifies a HIDDEN password will follow",
		"7              -- ����ֵ",
		NULLCHAR, NULLCHAR
	},
	{ 
		"1234567890abcde",
		MATCH_STR, 
		0, 
		RIP_CMD_MASKBIT<<2,
		rip_neigh_cmd_do_key_value,
		NULL,
		NULL,
		0,
		0,
		"WORD              -- key(16 char)", 
		"WORD              --��֤��Կ(md5�16���ַ�)",
		NULLCHAR,
		NULLCHAR
	},
	{ NULLCHAR }
};

int rip_neigh_cmd_do_md5_key_normal(int argc, char **argv, struct user *u)
{
	u->struct_p[2] = (unsigned long)RIP_AUTH_MD5_KEY_NORMAL;
	u->cmd_mskbits = RIP_CMD_MASKBIT<<2;
	return subcmd( rip_neighbor_cmds_key_md5_tab, &u->cmd_mskbits, argc, argv, u );
}

int rip_neigh_cmd_do_md5_key_hidden(int argc, char **argv, struct user *u)
{
	u->struct_p[2] = (unsigned long)RIP_AUTH_MD5_KEY_HIDDEN;
	u->cmd_mskbits = RIP_CMD_MASKBIT<<2;
	return subcmd( rip_neighbor_cmds_key_md5_tab, &u->cmd_mskbits, argc, argv, u );
}

/*����dynamic_key�µ�keyֵ������*/
struct cmds rip_neighbor_cmds_key_sha1_tab[]  = 
{
	{ 
		"1234567890abcdefg",
		MATCH_STR, 
		0, 
		0,
		rip_neigh_cmd_do_key_value,
		NULL,
		NULL,
		0,
		0,
		"WORD              -- key(20 char for sha1)", 
		"WORD              --��֤��Կ(sha1�20���ַ�)",
		NULLCHAR,
		NULLCHAR
	},
	{ NULLCHAR }
};

/*******************************************************************************
 *
 * FUNCTION	:	rip_neighb_get_key
 *
 * PARAMS		:	argc		- 
 *					argv		- 
 *					u			- 
 *					key			- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.02.25 16:58:57
 *
*******************************************************************************/
int rip_neighb_get_key(int argc, char **argv, struct user *u,struct dynamic_key_list_ *key)
{
	int ret;
	struct parameter param;
	char *keytype;

	/*Get  key_id*/
	param.type = ARG_UINT;
	param.min = 1;
	param.max = 255;
	param.flag = ARG_MIN | ARG_MAX;
	ret = getparameter(argc + 2, argv - 2, u, &param);
	if(ret != 0)
	{
		return ret;
	}
	key->key_id = param.value.v_int;

	/*get keyid's algorithems */
	param.type = ARG_STR;
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if(ret != 0)
	{
		return ret;
	}
	keytype = param.value.v_string;
	if( !strcmp(keytype,"md5")  || !strcmp(keytype,"m")||!strcmp(keytype,"md"))
		key->algorithms = RIP_NEIGHB_MD5;
	else if(!strcmp(keytype,"sha1")||!strcmp(keytype,"s")||!strcmp(keytype,"sh")||!strcmp(keytype,"sha"))
			key->algorithms = RIP_NEIGHB_SHA1;
	else return RIP_FAIL;

	return RIP_SUCCESS;
}

struct dynamic_key_list_ *rip_neighb_lookup_key(uint32 device_index,uint8 key_id,uint8 algorithms)
{
	struct dynamic_key_list_ *key_temp;
	if(rip_intf_array[device_index]== NULL)return NULL;
	for(key_temp = rip_intf_array[device_index]->key_list.forw ;key_temp !=&(rip_intf_array[device_index]->key_list) ; key_temp =key_temp->forw)
	{
		if(key_temp->key_id == key_id)
		{
			if(key_temp->algorithms == algorithms)
			{					
				return key_temp;
			}
			else break;
		}
	}
	for(key_temp = rip_intf_array[device_index]->key_timeout_list.forw ;key_temp !=&(rip_intf_array[device_index]->key_timeout_list) ; key_temp =key_temp->forw)
	{
		if(key_temp->key_id == key_id)
		{
			if(key_temp->algorithms == algorithms)
			{	
			    return key_temp;
			}
			else break;
		}
	}	
	return NULL;
}

int rip_neighb_key_del(uint32 device_index,uint8 key_id,uint8 algorithms)
{
	struct dynamic_key_list_ *key_temp;
	key_temp=rip_neighb_lookup_key(device_index,key_id,algorithms);
	if(!key_temp)return RIP_FAIL;
	
	/*ɾ��key��*/
	REMQUE( key_temp );
	rip_mem_free( key_temp, RIP_NEIGHB_KEY_TYPE );	
	return RIP_SUCCESS;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_neighb_delte_key
 *
 * PARAMS		:	u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.02.24 15:22:40
 *
*******************************************************************************/
int rip_neighb_delte_key(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 device_index;
	struct dynamic_key_list_ key;

	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	device_index = u->struct_p[1];
	if(rip_intf_array[device_index]== NULL)
	{
       	return RIP_FAIL;
	}

	/*GET KEY which will be deleted*/
	ret  = rip_neighb_get_key(argc,argv,u,&key);
	if(ret != RIP_SUCCESS)return RIP_FAIL;

	ret=rip_neighb_key_del(device_index,key.key_id,key.algorithms);
	if(ret!=RIP_SUCCESS)return RIP_FAIL;
	
	return RIP_SUCCESS;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_neigh_cmd_do_md5
 *
 * PARAMS		:	argc		- 
 *					argv		- 
 *					u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.02.24 15:21:23
 *
*******************************************************************************/
int  rip_neigh_cmd_do_md5(int argc, char **argv, struct user *u)
{
	int ret;
	
	switch(TypeOfFunc(u)) 
	{
		case NORMAL_FUNC:
			u->struct_p[2] = (unsigned long)0;
			return subcmd( rip_neighbor_cmds_key_md5_tab, NULL, argc, argv, u);
			break;
		case NOPREF_FUNC:
			ret = cmdend(argc - 1, argv + 1, u);
			if(ret != 0)
			{
				return ret;
		    }
			ret = rip_neighb_delte_key(argc,argv,u);
			if(ret != RIP_SUCCESS)
				return RIP_FAIL;
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_neigh_cmd_do_sha1
 *
 * PARAMS		:	argc		- 
 *					argv		- 
 *					u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.02.25 17:03:29
 *
*******************************************************************************/
int  rip_neigh_cmd_do_sha1(int argc, char **argv, struct user *u)
{
	int ret;
	
	switch(TypeOfFunc(u)) 
	{
		case NORMAL_FUNC:
			return subcmd( rip_neighbor_cmds_key_sha1_tab , NULL, argc, argv, u);
			break;
		case NOPREF_FUNC:
			ret = cmdend(argc - 1, argv + 1, u);
			if(ret != 0)
			{
				return ret;
		    	}
			ret = rip_neighb_delte_key(argc,argv,u);
			if(ret != RIP_SUCCESS)
				return RIP_FAIL;
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

 /*����dynamic_key�µ�key�㷨��������*/
struct cmds rip_neighbor_cmds_key_algorithems_tab[]  = 
{
	{ 
		"md5",
		MATCH_AMB, 
		cmdPref(PF_CMDNO, 0, 0), 
		0,
		rip_neigh_cmd_do_md5,
		NULL,
		NULL,
		0,
		0,
		"md5			-- md5",
		"md5			-- md5�㷨",
		NULLCHAR,
		NULLCHAR
	},
	
	{ 
		"sha1",
		MATCH_AMB, 
		cmdPref(PF_CMDNO, 0, 0), 
		0,
		rip_neigh_cmd_do_sha1,
		NULL,
		NULL,
		0,
		0,
		"sha1			-- sha1",
		"sha1			-- sha1�㷨",
		NULLCHAR,
		NULLCHAR
	},
	
	{
		"<cr>", 
	  	MATCH_END, 
	  	cmdPref(0, 0, PF_NOCMDNOR), 
	  	0,NULL,NULL,NULL,0,0,
	  	"<cr>", 
	  	"<cr>", 
	  	NULLCHAR, 
	 	 NULLCHAR
	},
	
	{ NULLCHAR }
};

int  rip_neigh_cmd_do_keyid(int argc, char **argv, struct user *u)
{
	return subcmd( rip_neighbor_cmds_key_algorithems_tab , NULL, argc, argv, u );
}

 /*����dynamic_key�µ�key id������*/
struct cmds rip_neighbor_cmds_key_id_tab[]  = 
{
	{ 
		"0123456789",
		MATCH_DIG, 
		cmdPref(PF_CMDNO, 0, 0), 
		0,
		rip_neigh_cmd_do_keyid,
		NULL,
		NULL,
		0,
		0,
		"<1-255>           -- key-ID", 
		"<1-255>           -- ��֤ID", 
		NULLCHAR,
		NULLCHAR
	},
	{ NULLCHAR }
};


int  rip_neigh_cmd_key(int argc, char **argv, struct user *u)
{
	return subcmd(rip_neighbor_cmds_key_id_tab, NULL, argc, argv, u );
}

/*===========================================================
������:      rip_register_cmd
��������: ע��RIPģ������
�������: ��
�������: ��
����ֵ:      ע��ɹ�,����RIP_CMD_SUCCESS
                       ע��ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_register_cmd( void )
{
	int ret;

	/*-------------------------------------------------------------------
	ע��route rip�����µ�������
	--------------------------------------------------------------------*/
	ret = registerncmd( rip_cmd_topcmds, (sizeof(rip_cmd_topcmds)/sizeof(struct topcmds)) - 1 );
	if( ret != ((sizeof(rip_cmd_topcmds)/sizeof(struct topcmds)) - 1) )
	{
		return RIP_FAIL;
	}
#if 0/*rip�������չ�������ʽ����ospf���У�dangzhw20103.22*/
	/*-------------------------------------------------------------------
	ע��route rip address family�����µ�������
	--------------------------------------------------------------------*/
#ifdef VRF_MODULE
	ret = registerncmd( rip_cmd_vrf_topcmds, (sizeof(rip_cmd_vrf_topcmds)/sizeof(struct topcmds)) - 1 );
	if( ret != ((sizeof(rip_cmd_vrf_topcmds)/sizeof(struct topcmds)) - 1) )
	{
		return RIP_FAIL;
	}
#endif
#endif
	/*-------------------------------------------------------------------
	ע��route rip����
	--------------------------------------------------------------------*/
	ret = register_subcmd_tab( "router", 
							   cmdPref(PF_CMDNO, 0, 0), 
							   IF_NULL,
							   FG_CONFIG, 
							   rip_cmd_router_tab,
							   1 );
	if( ret != 1 )
	{
		return RIP_FAIL;
	}
	
	/*-------------------------------------------------------------------
	ע��ӿ�����ģʽ�µ�RIP����������
	--------------------------------------------------------------------*/
	ret = register_subcmd_tab ( "ip", 
				                cmdPref(PF_CMDNO, PF_CMDDEF, 0), 
#ifdef OS_VXWORKS
				                IF_ROUTING & ~IF_VIRT, 
#else
				                IF_ANY & ~IF_VIRT, 
#endif
				                FG_CONFIG,
				                rip_cmds_intf_tab,
							    1 );
	if( ret != 1 )
	{
		return RIP_FAIL;
	}

	/*show ip �����RIP����*/
	ret = register_subcmd_tab("show ip", 0, IF_NULL, FG_ENABLE,
							  rip_cmds_show_ip_tab, 1);
	if( ret != 1 )
	{
		return RIP_FAIL;
	}

	/*debug ip �����RIP����*/
	ret = register_subcmd_tab("debug ip", cmdPref(PF_CMDNO, 0, 0), IF_NULL, FG_ENABLE,
							  rip_cmds_debug_ip_tab, 1);
	if( ret != 1 )
	{
		return RIP_FAIL;
	}
	
	/* RIPģ��show running���� */
	ret = interface_set_showrunning_service( MODULE_TYPE_RIP, rip_show_running );	
	if( ret < 0 )
	{
		return RIP_FAIL;
	}
	
	return RIP_SUCCESS;
	
}

/*===========================================================
������:   rip_cmd_router_rip
��������: router rip�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_router_rip(int argc, char **argv, struct user *u)
{
	return subcmd(rip_cmd_process_tab , NULL,argc , argv , u);
}

int rip_cmd_rip_process_vrf(int argc, char *argv[], struct user *u)
{
	return subcmd(rip_cmd_rip_process_vrf_name_table, NULL, argc, argv, u);
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_cmd_rip_process_vrf_name
 *
 * PARAMS		:	argc		- 
 *					argv[]		- 
 *					u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	router rip process vrf vrf-name����
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.05 14:18:34
 *
*******************************************************************************/
int rip_cmd_rip_process_vrf_name(int argc, char **argv, struct user *u)
{
	int rc;
	uint32 vrfid;
	uint32 processid;
	struct parameter param;
	
	rc = cmdend(argc - 1, argv + 1, u);
	if (rc != 0) 
	{
		return rc;
	}

	param.type = ARG_UINT;
	param.min = 1;
	param.max = 65535;
	param.flag = ARG_MIN | ARG_MAX;

	rc = getparameter(argc + 3, argv - 3, u, &param);
	if (rc != 0) {
		return rc;
	}

	processid = param.value.v_int;
	vrfid = vrf_GetIDbyName( *argv );

	if( vrfid == 0)
	{
		if( IsChinese(u) )
		{
			vty_output( "%%����, VRF '%s' �����ڻ�û������RD.\n", *argv );
		}
		else
		{
			vty_output( "%%Err, VRF '%s' does not exist or does not have a RD.\n", *argv );
		}
		
		return RIP_FAIL;
	}
		
	if( vrfid == 0 )
	{
		vty_output( "%%RIP: VRF '%s' does not exist or does not have a RD.\n", *argv );
		return -1;
	}
	
	rip_cmd_rip_process_enter(u , processid ,vrfid, TRUE);

	return 0;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_cmd_rip_process_enter
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.05 13:54:27
 *
*******************************************************************************/
int rip_cmd_rip_process_enter(struct user *u ,uint32 processid, uint32 vrfid,uint8 enter_process_mode)
{
	int ret;
	struct rip_process_info_ *pprocess = NULL;

	/*����������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			/*����RIP����ģʽ*/
			ret = rip_enter_rip_process_mode( u, processid ,vrfid, enter_process_mode);
			if( RIP_SUCCESS != ret )
			{
				vty_output( "RIP:rip_cmd_rip_process_enter,enter mode fail\n");
				return RIP_FAIL;
			}
			
			ret = rip_lookup_process_byprocessid(processid , &pprocess);
			if(!ret)
			{/*��һ�����ý��̵�ʱ�����ɽ���*/
				rip_process_init(processid , vrfid)	;		
			}
			else
			{/*�������Ѿ����ڣ���Ҫ����ģʽ��������ʱ�򣬰󶨵�vrf�µĽ��̿��Բ�����vrf����
				�����ٴν��룬����û�а󶨵�vrf�Ľ��̣��ٴν������ģʽ��ʱ��������vrf����ʾ����dangzhw20103.25*/
				if( pprocess->vrf_id != vrfid )
				{
					if( vrfid != 0 )
					{
						vty_output("RIP:process %d VRF specified does not match existing router\n",pprocess->process_id);
						PrepareSwitch(u, FG_CONFIG);
						modifyCmdFlag(u, FG_CFG_ST, FG_ANY);
					}
					else
					{
						SetVrfID(u, pprocess->vrf_id);/*������ʵ��vrfid*/
						break;
					}
					return RIP_FAIL;
				}
			}
			break;
		case NOPREF_FUNC:
			ret = rip_lookup_process_byprocessid(processid , &pprocess);
			if(!ret)
			{	
				/*rip��û������,��������*/
				return RIP_SUCCESS;
			}

			/*�˳�RIP����ģʽ*/
			ret = rip_exit_rip_mode( u ,pprocess);
			if( RIP_SUCCESS != ret )
			{
				return RIP_FAIL;
			}
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_cmd_rip_process
 *
 * PARAMS		:	argc		- 
 *					argv[]		- 
 *					u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	router rip processid�������
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.05 13:47:29
 *
*******************************************************************************/
int rip_cmd_rip_process(int argc, char **argv, struct user *u)
{
	int rc;
	uint32 processid,vrfid;
	struct parameter param;

	if( argc == 1 )
	{
		param.type = ARG_UINT;
		param.min = 1;
		param.max = 65535;
		param.flag = ARG_MIN | ARG_MAX;

		rc = getparameter(argc+1, argv-1 , u, &param);
		if (rc != 0) {
			return rc;
		}

		processid = param.value.v_int;
		vrfid = 0;
	
		rc = cmdend(argc - 1, argv + 1, u);
		if (rc != 0) {
			return rc;
		}

		rip_cmd_rip_process_enter( u ,processid ,vrfid,TRUE);
	}
	else
	{
		return subcmd(rip_cmd_vrf_table, NULL, argc, argv, u);
	}
				
	return 0;
}
#if 0
/*===========================================================
������:   rip_cmd_router_rip
��������: router rip�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_router_rip(int argc, char **argv, struct user *u)
{
	int ret;
       uint32 vrf_id;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	/*����������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			/*����RIP����ģʽ*/
			ret = rip_enter_rip_mode( u, 0 );
			if( RIP_SUCCESS != ret )
			{
				return RIP_FAIL;
			}
            
                     if( RIP_ENABLE != rip_glb_info.enable )
			{
				/*��ʼ��ȫ�ֱ���*/
                            vrf_id  = GetVrfID(u);
				ret = rip_init_glb_info(vrf_id);	
                            if( RIP_SUCCESS != ret )
				{
					return RIP_FAIL;
				}
			}
			break;
		case NOPREF_FUNC:
			if( RIP_ENABLE != rip_glb_info.enable )
			{	
				/*rip��û������,��������*/
				return RIP_SUCCESS;
			}

			/*�˳�RIP����ģʽ*/
			ret = rip_exit_rip_mode( u );
			if( RIP_SUCCESS != ret )
			{
				return RIP_FAIL;
			}
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_afi
��������: address-family �������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_afi(int argc, char **argv, struct user *u)
{
	return subcmd(rip_cmds_afi_tab, NULL, argc, argv, u);
}

/*===========================================================
������:   rip_cmd_afi_ipv4
��������: address-family ipv4�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_afi_ipv4(int argc, char **argv, struct user *u)
{
	return subcmd(rip_cmds_afi_ipv4_tab, NULL, argc, argv, u);
}

/*===========================================================
������:   rip_cmd_afi_vrf
��������: address-family ipv4_vrf�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_afi_vrf(int argc, char **argv, struct user *u)
{
	return subcmd(rip_cmds_afi_vrf_tab, NULL, argc, argv, u);
}

/*===========================================================
������:   rip_cmd_afi_vrf_name
��������: address-family ipv4_vrf vrf-name�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_afi_vrf_name(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 vrf_id;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	/*RIP���̲�û������,��������*/
	if( TRUE != rip_is_enable(u, 0) )
	{
		return RIP_FAIL;
	}

	if( strlen(*argv) > RIP_VRF_NAME_LEN )
	{
		cmderror( ERR_RANGE, *argv, u );
		return RIP_FAIL;
	}
	
	vrf_id = vrf_GetIDbyName( *argv );
	if( vrf_id == 0)
	{
		if( IsChinese(u) )
		{
			vty_output( "%%����, VRF '%s' �����ڻ�û������RD.\n", *argv );
		}
		else
		{
			vty_output( "%%Err, VRF '%s' does not exist or does not have a RD.\n", *argv );
		}
		
		return RIP_FAIL;
	}

	/*����������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			/*����RIP����ģʽ*/
			ret = rip_enter_rip_mode( u, vrf_id );
			if( RIP_SUCCESS != ret )
			{
				return RIP_FAIL;
			}
            if( NULL == rip_tbl_array[vrf_id] )
			{
				/*����RIP·�ɱ�*/
                            rip_init_glb_info(vrf_id);				
			}
			break;
		case NOPREF_FUNC:
			if( NULL == rip_tbl_array[vrf_id] )
			{
				/*address-family��û������,��������*/
				return RIP_SUCCESS;
			}

			/*�˳�RIP����ģʽ*/
			ret = rip_exit_afi_mode( u, vrf_id );
			if( RIP_SUCCESS != ret )
			{
				return RIP_FAIL;
			}
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_exit_afi
��������: exit address-family�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_exit_afi(int argc, char **argv, struct user *u)
{
	int ret;

	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	SetVrfID( u, 0 );
	
	PrepareSwitch(u, FG_RIP_VRF);
	modifyCmdFlag(u, FG_CFG_ST | FG_RIP, FG_ANY);
	
	return RIP_SUCCESS;
}
#endif

/*===========================================================
������:   rip_cmd_input_queue
��������: input-queue <1-1024>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_input_queue(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;

	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			return subcmd(rip_cmds_input_queue_tab, NULL, argc, argv, u);
			break;
		case NOPREF_FUNC:
		case DEFPREF_FUNC:
			ret = cmdend(argc - 1, argv + 1, u);
			if(ret != 0) 
			{
				return ret;
			}

			processid = GetProcID( u );

			ret = rip_lookup_process_byprocessid(processid, & pprocess);
			if(!ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}
			
			if( RIP_DEF_INPUT_QUEUE != rip_glb_info.input_queue )
			{
				rip_glb_info.input_queue = RIP_DEF_INPUT_QUEUE;

				if (task_set_option(rip_glb_info.socket_id, TASKOPTION_RECVBUF, RIP_MAX_PACKET_SIZE*rip_glb_info.input_queue) < 0) 
				{
					rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
					return RIP_FAIL;
				}
			}
			break;
		default:
			break;
	}
	return RIP_SUCCESS;	
}

/*===========================================================
������:   rip_cmd_input_queue_val
��������: input-queue <>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_input_queue_val(int argc, char **argv, struct user *u)
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

	processid = GetProcID(u);

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	param.type = ARG_UINT;
	param.min = 1;
	param.max = 61440;
	param.flag = ARG_MIN | ARG_MAX ;
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if(ret != 0)
	{
		return ret;
	}

	if( rip_glb_info.input_queue != param.value.v_int )
	{
		rip_glb_info.input_queue = param.value.v_int;

		if (task_set_option(rip_glb_info.socket_id, TASKOPTION_RECVBUF, RIP_MAX_PACKET_SIZE*rip_glb_info.input_queue) < 0) 
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}
	}
	
	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_auto_summary
��������: auto-summary�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_auto_summary(int argc, char **argv, struct user *u)
{
	int ret;
	/*uint32 vrf_id;
	*/
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	processid = GetProcID( u );

	ret = rip_lookup_process_byprocessid(processid, & pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
		
	/*����������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			pprocess->auto_summary_flag = RIP_AUTO_SUMMARY_ENABLE;
			break;
		case NOPREF_FUNC:
			pprocess->auto_summary_flag = RIP_AUTO_SUMMARY_DISABLE;
			break;
		case DEFPREF_FUNC:
			pprocess->auto_summary_flag = RIP_AUTO_SUMMARY_ENABLE;
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_cmd_zero_domain_check
 *
 * PARAMS		:	argc		- 
 *					argv		- 
 *					u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.25 16:22:27
 *
*******************************************************************************/
int rip_cmd_zero_domain_check(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	processid = GetProcID( u );

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	/*����������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			BIT_SET(pprocess->flags ,RIP_ZERO_DOMAIN_CHECK_ENABLE);
			break;
		case NOPREF_FUNC:
			BIT_RESET(pprocess->flags ,RIP_ZERO_DOMAIN_CHECK_ENABLE);
			break;
		case DEFPREF_FUNC:
			BIT_SET(pprocess->flags ,RIP_ZERO_DOMAIN_CHECK_ENABLE);
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_vali_source
��������: validate-update-source�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_vali_source(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	processid = GetProcID( u );

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	/*����������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			BIT_SET(pprocess->flags ,RIP_SRC_CHECK_ENABLE);
			break;
		case NOPREF_FUNC:
			BIT_RESET(pprocess->flags ,RIP_SRC_CHECK_ENABLE);
			break;
		case DEFPREF_FUNC:
			BIT_SET(pprocess->flags ,RIP_SRC_CHECK_ENABLE);
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_def_info
��������: default-information�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_def_info(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	switch(TypeOfFunc(u))
	{
	case NORMAL_FUNC:
		return subcmd(rip_cmds_def_origi_tab, NULL, argc, argv, u);
		break;
	case NOPREF_FUNC:
	case DEFPREF_FUNC:
		ret = cmdend(argc - 1, argv + 1, u);
		if(ret != 0) return ret;
		
		processid = GetProcID(u);	
		ret = rip_lookup_process_byprocessid(processid, &pprocess);
		if(!ret )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}
		pprocess->default_route_flag = RIP_DEF_ROUTE_DISABLE;
		rip_del_orig_route(pprocess);
		break;
	default:
		break;
	}
	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_def_info_origi_safe
��������: default-information originate-safe�������
�������: argc/argv: �������
          u: �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_def_info_origi_safe(int argc, char **argv, struct user *u)
{
	register rt_entry *rt = (rt_entry *)NULL;
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;
#if 0
	struct vrf_table_list_ *vrf_table = NULL;
#endif
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	processid = GetProcID(u);
	
	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	pprocess->default_route_flag = RIP_DEF_ROUTE_ENABLE_SAFE;
#if 0/*MULTICORE*/
	vrf_table = vrf_table_enter(pprocess->vrf_id,RT_RD_LOCK);
	if(vrf_table == NULL)
	{
		return RIP_FAIL;
	}
#endif
    /*Fengsb 2006-05-09 ����cisco������������Ҫ������·�ɱ����Ƿ����ȱʡ·��
	(cisco�����õ�ȱʡ·�ɣ������޸�ΪֻҪ�Ƿ�rip�ṩ��ȱʡ·�ɾͿ���)��
	������ڣ��Ž��з���*/
	rt=rt_locate(pprocess->vrf_id, RTS_ACTIVE, 0, 0, RTPROTO_ANY, 0);
	if(rt&&(rt->rt_proto != RTPROTO_RIP))rip_create_orig_route(pprocess);
	else rip_del_orig_route(pprocess);
	
#if 0/*MULTICORE*/
	vrf_table_leave(vrf_table,RT_RD_LOCK);
#endif
	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_def_info_origi
��������: default-information originate�������
�������: argc/argv: �������
          u: �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_def_info_origi(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;
#if 0
	struct vrf_table_list_ *vrf_table = NULL;
#endif
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	processid = GetProcID(u);
	
	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
		
	pprocess->default_route_flag = RIP_DEF_ROUTE_ENABLE;
#if 0/*MULTICORE*/
	vrf_table = vrf_table_enter(pprocess->vrf_id,RT_RD_LOCK);
	if(vrf_table == NULL)
	{
		return RIP_FAIL;
	}
#endif
    rip_create_orig_route(pprocess);	
#if 0/*MULTICORE*/
	vrf_table_leave(vrf_table,RT_RD_LOCK);
#endif
	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_def_metric
��������: default-metric�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_def_metric(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	switch(TypeOfFunc(u)) 
	{
		case NORMAL_FUNC:
			return subcmd(rip_cmds_def_metric_val_tab, NULL, argc, argv, u);
			break;
		case NOPREF_FUNC:
		case DEFPREF_FUNC:
			ret = cmdend(argc - 1, argv + 1, u);
			if(ret != 0) 
			{
				return ret;
			}

			processid = GetProcID(u);
			ret = rip_lookup_process_byprocessid(processid, & pprocess);
			if(!ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}		
			pprocess->default_metric = RIP_DEF_METRIC;
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_def_metric_val
��������: default-metric <1-16>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_def_metric_val(int argc, char **argv, struct user *u)
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
	
	processid = GetProcID( u );

	ret = rip_lookup_process_byprocessid(processid, & pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	param.type = ARG_UINT;
	param.min = 1;
	param.max = 16;
	param.flag = ARG_MIN | ARG_MAX;
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if(ret != 0) 
	{
		return ret;
	}
	
	pprocess->default_metric = param.value.v_int;

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_ver
��������: version�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_ver(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	/*���������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			return subcmd( rip_cmds_ver_val_tab, NULL, argc, argv, u );
			break;
		case NOPREF_FUNC:
		case DEFPREF_FUNC:
			/*��������в����Ƿ����*/
			ret = cmdend(argc - 1, argv + 1, u);
			if(ret != 0)
			{
				return ret;
			}
			
			processid = GetProcID(u);
			ret = rip_lookup_process_byprocessid(processid, & pprocess);
			if(!ret)
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}

			pprocess->version = RIP_VERSION_DEF;
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_ver_val
��������: version <1-2>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_ver_val(int argc, char **argv, struct user *u)
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
	
	processid = GetProcID( u );

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	param.type = ARG_UINT;
	param.min = 1;
	param.max = 2;
	param.flag = ARG_MIN | ARG_MAX ;

	ret = getparameter(argc + 1, argv - 1, u, &param);
	if( ret != 0 )
	{
		return ret;
	}
	
	if( 1 == param.value.v_int )
	{
		pprocess->version = RIP_VERSION_1;
	}
	else if( 2 == param.value.v_int )
	{
		pprocess->version = RIP_VERSION_2;
	}

	return RIP_SUCCESS;	
}

#if 0
int rip_check_connect_route(struct rip_process_info_ *pprocess,uint warning)
{
	struct rip_process_intf_list_ *pintf_list;
	struct rip_intf_ *pintf;
	struct rip_route_activate_msg_ msg;
	
	for(pintf_list = pprocess->intf_list.forw ; pintf_list != &(pprocess->intf_list) ; pintf_list = pintf_list->forw )
	{
		if( !(pintf = pintf_list->pintf) )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}

		/* ����������ǰ�����·����Ŀ�����ƣ�ʹ�ò���ֱ��·��û����ӽ�����
		�������±������еĽ��̶˿�
		 * Commented by dangzhw in 2010.03.09 15:47:30 */
		memset( &msg , 0 ,sizeof(struct rip_route_activate_msg_));

		msg.type = RIP_ROUTE_INTF_PROCESS_ENABLE;
		msg.u.intf.device_index = pintf->device_index;
		msg.u.intf.warning = warning;

		rip_process_route_activate(pprocess, &msg);
	}
	return RIP_SUCCESS;
}
#endif

void rip_apply_max_nh(struct rip_process_info_ *pprocess)
{
	struct rip_route_ *rip_route, *rip_route_head, *rip_route_forw;
	struct route_node *rip_route_node;
	uint32 count,num,k;

	if(!pprocess)return;
	for(rip_route_node=route_top(pprocess->rip_table);rip_route_node;rip_route_node=route_next(rip_route_node))
	{
		rip_route_head = rip_route_node->info;
		if(NULL == rip_route_head)continue;
		num = rip_route_head->equi_nbr_num;
		if(num<=pprocess->max_nh_num)continue;
		k = num- pprocess->max_nh_num;
		for( count = 1, rip_route = rip_route_head; count <= num; rip_route = rip_route_forw, count++ )
		{
			rip_route_forw = rip_route->forw;
			/*��ѧϰ·�ɲ���ɾ��*/
			if( RIP_NBR_ROUTE != rip_route->route_type )
			{
				continue;
			}
			
			/* ��holddown�б���ɾ�� */
			rip_del_holddown(rip_route);
			
			/*��·�ɵ�ѧϰ�б���ɾ��*/
			rip_del_from_nbr_list( rip_route );

			/*��·�ɱ���ɾ��*/
			rip_del_route( rip_route );
			
			k--;
			if(k==0)break;
		}
	}
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_cmd_maxnh
 *
 * PARAMS		:	argc		- 
 *					argv		- 
 *					u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.25 16:55:26
 *
*******************************************************************************/
int rip_cmd_max_nh(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid, pre_val;
	struct rip_process_info_ *pprocess;

	switch(TypeOfFunc(u)) 
	{
		case NORMAL_FUNC:
			return subcmd(rip_cmds_max_nh_val_tab, NULL, argc, argv, u);
			break;
		case NOPREF_FUNC:
			/*if there are any connet route add fail because the max count limited, add again*/
			ret = cmdend(argc - 1, argv + 1, u);
			if(ret != 0) 
			{
				return ret;
			}
			processid = GetProcID(u);
			ret = rip_lookup_process_byprocessid(processid, &pprocess);
			if(!ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}
			pre_val=pprocess->max_nh_num;
			pprocess->max_nh_num= RIP_MAX_NH_NUM;
			if(RIP_MAX_NH_NUM < pre_val) rip_apply_max_nh(pprocess);
			break;
		case DEFPREF_FUNC:
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}
/*xuhaiqing 2010-12-21�Ž����·����Ŀ������ȡ��*/
/*===========================================================
������:   rip_cmd_maxcount
��������: maximum-routes�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
/*int rip_cmd_maxcount(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;

	switch(TypeOfFunc(u)) 
	{
		case NORMAL_FUNC:
			return subcmd(rip_cmds_max_count_val_tab, NULL, argc, argv, u);
			break;
		case NOPREF_FUNC:
			/*if there are any connet route add fail because the max count limited, add again*/
			/*ret = cmdend(argc - 1, argv + 1, u);
			if(ret != 0) 
			{
				return ret;
			}
			processid = GetProcID(u);
			ret = rip_lookup_process_byprocessid(processid, &pprocess);
			if(!ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}
			
			pprocess->max_route_num = RIP_MAX_ROUTE_NUM;

			rip_check_connect_route(pprocess, FALSE);
			
		case DEFPREF_FUNC:
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}*/

/*******************************************************************************
 *
 * FUNCTION	:	rip_cmd_maxnh_val
 *
 * PARAMS		:	argc		- 
 *					argv		- 
 *					u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.25 17:06:46
 *
*******************************************************************************/
int rip_cmd_maxnh_val(int argc, char **argv, struct user *u)
{
	int ret;
	struct parameter param;
	uint32 processid, pre_val;
	struct rip_process_info_ *pprocess;

	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	processid = GetProcID( u );

	ret = rip_lookup_process_byprocessid(processid, & pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	param.type = ARG_UINT;
	param.min = 1;
	param.max = 16;
	param.flag = ARG_MIN | ARG_MAX ;
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if(ret != 0) 
	{
		return ret;
	}
	pre_val=pprocess->max_nh_num;
	pprocess->max_nh_num= param.value.v_int;
	if(pprocess->max_nh_num<pre_val)rip_apply_max_nh(pprocess);
	return RIP_SUCCESS;
}

/*xuhaiqing 2010-12-21�Ž����·����Ŀ������ȡ��*/
/*===========================================================
������:   rip_cmd_maxcount_val
��������: maximum-routes <1-8192>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
/*int rip_cmd_maxcount_val(int argc, char **argv, struct user *u)
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

	processid = GetProcID( u );

	ret = rip_lookup_process_byprocessid(processid, & pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	param.type = ARG_UINT;
	param.min = 1;
	param.max = 10240;
	param.flag = ARG_MIN | ARG_MAX ;
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if(ret != 0) 
	{
		return ret;
	}
	
	pprocess->max_route_num = param.value.v_int;

	rip_check_connect_route(pprocess , FALSE);

	return RIP_SUCCESS;
}*/

/*===========================================================
������:   rip_cmd_neigh
��������: neighbor�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_neigh(int argc, char **argv, struct user *u)
{
	return subcmd( rip_cmds_neigh_addr_tab, NULL, argc, argv, u );
}

/*===========================================================
������:   rip_cmd_neigh_addr
��������: neighbor A.B.C.D�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_neigh_addr(int argc, char **argv, struct user *u)
{
	int ret;
	struct parameter param;
	uint32 neigh_addr;
	uint32 processid;
	struct rip_neigh_list_ *rip_neigh_list;
	struct rip_process_info_ *pprocess;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	processid = GetProcID(u);

	ret = rip_lookup_process_byprocessid(processid, & pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	param.type = ARG_ADDR;
	param.flag = 0;
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if(ret != 0)
	{
		return ret;
	}

	neigh_addr = param.value.v_addr;

	switch(TypeOfFunc(u))
	{
		case NORMAL_FUNC:
			if(rt_is_local_addr(neigh_addr,TRUE) )
			{
				if( IsChinese(u) )
				{
					vty_output("%%����, �Ǳ��ص�ַ\n");
				}
				else
				{
					vty_output("%%Err,  it is local addr\n");
				}
				
				return RIP_FAIL;
			}

			if( (htonl(neigh_addr) & 0xff ) == 0 )
			{
				if( IsChinese(u) )
				{
					vty_output( "%%����, ��Ч�ھ�IP��ַ.\n" );
				}
				else
				{
					vty_output( "%%Err, Invalid neighbor ip address.\n" );
				}
				
				return RIP_FAIL;
			}

			for( rip_neigh_list = pprocess->neigh_list.forw; rip_neigh_list != &(pprocess->neigh_list); rip_neigh_list = rip_neigh_list->forw )
			{
				if( rip_neigh_list->neigh_addr == neigh_addr )
				{
					/*�����ù�����*/
					return RIP_SUCCESS;
				}
			}

			/*�����µ�neighbor address*/
			rip_neigh_list = (struct rip_neigh_list_ *)rip_mem_malloc( sizeof(struct rip_neigh_list_), RIP_NEIGH_LIST_TYPE );
			if( rip_neigh_list == NULL )
			{
				/*�ڴ����ʧ��*/
				fprintf(stderr, "RIP: Malloc memory failed.(%s, %d)\n", __FILE__, __LINE__ );
				return RIP_MEM_FAIL;
			}
			memset( rip_neigh_list, 0, sizeof(struct rip_neigh_list_) );
			rip_neigh_list->neigh_addr = neigh_addr;
			INSQUE( rip_neigh_list, pprocess->neigh_list.back );
			break;
		case NOPREF_FUNC:
			for( rip_neigh_list = pprocess->neigh_list.forw; rip_neigh_list != &(pprocess->neigh_list); rip_neigh_list = rip_neigh_list->forw )
			{
				if( rip_neigh_list->neigh_addr == neigh_addr )
				{
					/*ɾ��neighbor address��*/
					REMQUE( rip_neigh_list );
					rip_mem_free( rip_neigh_list, RIP_NEIGH_LIST_TYPE );
					return RIP_SUCCESS;
				}
			}
			break;
		default:
			break;
	}
	
	return RIP_SUCCESS;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_neighb_key_start_check
 *
 * PARAMS		:	neighb		- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.02.26 11:25:12
 *
*******************************************************************************/
void rip_neighb_timer_key_start_check(uint32 device_index)
{
	int32 ret;
	uint found_exten =0;
	uint found_valid =0;
	int32 timeout;
	struct key_start_time_ currect_time;
	struct dynamic_key_list_ *key;

	memset(&currect_time, 0, sizeof(struct key_start_time_));
	ret =rip_neighb_get_currect_time(& currect_time);
	if(ret !=RIP_SUCCESS)return;
	
	key= rip_intf_array[device_index]->key_list.forw;
	while(key != & rip_intf_array[device_index]->key_list)
	{	
		if(key->key_state == RIP_NEIGHBOR_KEY_EXTENSIONUSE)
			found_exten=1;
		if(key->key_state == RIP_NEIGHBOR_KEY_VALID)
		{
			ret =rip_neighb_time_cmp(&key->start_time,&currect_time);
			timeout = key->lift_time.hour*60*60+key->lift_time.minute*60;
			if(ret <=60 )
			{
				if(timeout >abs(ret))
				{
					key->key_state = RIP_NEIGHBOR_KEY_ACTIVE;
					found_valid =1;
				}
				else key->key_state = RIP_NEIGHBOR_KEY_EXPIRED;
			}
			else break;
		}
		key =key->forw;
	}	

	/*������ʹ�õ�key����������key��ʼ��Ч����ɾ�����ڵ�key*/
	if(found_exten && found_valid)
	{
		key= rip_intf_array[device_index]->key_list.forw;
		while(key->key_state !=RIP_NEIGHBOR_KEY_EXTENSIONUSE)
		{
			key=key->forw;
		}
		key->key_state =RIP_NEIGHBOR_KEY_EXPIRED;
		REMQUE(key);
		INSQUE(key,rip_intf_array[device_index]->key_timeout_list.forw);
	}
	sys_start_timer(rip_intf_array[device_index]->key_start_timer_id, 60);
	return ;	
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_neighb_key_life_out
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.02.26 11:25:48
 *
*******************************************************************************/
void rip_neighb_key_life_out(uint32 device_index)
{
	int32 ret;
	uint16 keyid =0;
	uint found_active =0;
	uint found_timeout =0;
	int32 time_out; 
	struct key_start_time_ currect_time;
	struct dynamic_key_list_ *key;
	struct dynamic_key_list_ *timeout_key = NULL;
	struct dynamic_key_list_ *keytemp;

	memset(&currect_time, 0, sizeof(struct key_start_time_));
	ret =rip_neighb_get_currect_time(&currect_time);
	if(ret !=RIP_SUCCESS)return;
	
	key = rip_intf_array[device_index]->key_list.forw;
	while(key != &rip_intf_array[device_index]->key_list)
	{
		/*valid ������Ŀ����λ�ã�������valid��key���˳�*/
		if(key->key_state == RIP_NEIGHBOR_KEY_VALID)break;
		if(key->key_state == RIP_NEIGHBOR_KEY_EXPIRED)
		{
			keytemp=key;
			key = key->forw;
			REMQUE(keytemp);
			INSQUE(keytemp,rip_intf_array[device_index]->key_timeout_list.forw);
			continue;
		}
		if(key->key_state == RIP_NEIGHBOR_KEY_ACTIVE)
		{	
			found_active++;
			ret =rip_neighb_time_cmp(&currect_time, &key->start_time);
			time_out = key->lift_time.hour*60*60+key->lift_time.minute*60;
			if(ret >=time_out)
			{	/*��ͬʱ�ж��key��ʱ��ɾ��keyidС��*/
				found_timeout++;
				if(keyid ==0)
				{
					keyid = key->key_id;
					timeout_key =key;
				}
				else
				{
					if(keyid > key->key_id)
					{
						keyid = key->key_id;
						timeout_key =key;
					}
				}
			}
		}
		key =key->forw;
	}

	if(found_active && found_timeout)
	{
		if(found_active==1)
			timeout_key->key_state = RIP_NEIGHBOR_KEY_EXTENSIONUSE;
		
		if(found_active >1)
		{
			timeout_key->key_state = RIP_NEIGHBOR_KEY_EXPIRED;
			REMQUE(timeout_key);
			INSQUE(timeout_key,rip_intf_array[device_index]->key_timeout_list.forw);
		}		
	}
	ret =sys_start_timer(rip_intf_array[device_index]->key_lift_timer_id,60);
	return;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_neighb_creat_timer
 *
 * PARAMS		:	neighb		- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.02.26 11:23:45
 *
*******************************************************************************/
int rip_intf_creat_timer(uint32 device_index)
{
	TIMER_USER_DATA timer_user_data;
	uint32 ret;
	
	/*Register rip neighbor timer*/
	
	memset( &timer_user_data, 0, sizeof(TIMER_USER_DATA) );
	timer_user_data.msg.qid = (MSG_Q_ID)rip_glb_info.queue_id;
	timer_user_data.msg.msg_buf[0] = MSG_RIP_INTF_KEY_START;
	timer_user_data.msg.msg_buf[1] = device_index;
	ret = sys_add_timer(TIMER_MSG_METHOD, &timer_user_data, &(rip_intf_array[device_index]->key_start_timer_id));
	if( SYS_NOERR != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	memset( &timer_user_data, 0, sizeof(TIMER_USER_DATA) );
	timer_user_data.msg.qid = (MSG_Q_ID)rip_glb_info.queue_id;
	timer_user_data.msg.msg_buf[0] = MSG_RIP_INTF_KEY_TIMEOUT;
	timer_user_data.msg.msg_buf[1] = device_index;
	ret = sys_add_timer (TIMER_MSG_METHOD, &timer_user_data, &(rip_intf_array[device_index]->key_lift_timer_id));
	if( SYS_NOERR != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	memset( &timer_user_data, 0, sizeof(TIMER_USER_DATA) );
	timer_user_data.msg.qid = (MSG_Q_ID)rip_glb_info.queue_id;
	timer_user_data.msg.msg_buf[0] = MSG_RIP_INTF_AUTH_COMMIT_TIMEOUT;
	timer_user_data.msg.msg_buf[1] = device_index;
	ret = sys_add_timer (TIMER_MSG_METHOD, &timer_user_data, &(rip_intf_array[device_index]->auth_commit_timer_id));
	if( SYS_NOERR != ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}
	
	return RIP_SUCCESS;
}

#if 0

/*===========================================================
������:   rip_cmd_net
��������: network�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_net(int argc, char **argv, struct user *u)
{
	return subcmd(rip_cmds_net_addr_tab, NULL, argc, argv, u);
}

/*===========================================================
������:   rip_cmd_net_addr
��������: network A.B.C.D�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_net_addr(int argc, char **argv, struct user *u)
{
	int ret;
	struct parameter param; 
	uint32 net_addr;

	if( argc == 1 )
	{
		uint32 processid;
		struct rip_process_info_ *pprocess;


		ret = cmdend(argc - 1, argv + 1, u);
		if(ret != 0) 
		{
			return ret;
		}

		processid = GetProcID(u);

		ret = rip_lookup_process_byprocessid(processid, & pprocess);
		if(!ret )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}

		param.type = ARG_ADDR;
		ret = getparameter( argc + 1, argv - 1, u, &param );
		if(ret != 0) 
		{
			return ret;
		}
		
		net_addr = rip_get_natural_network( param.value.v_addr );
		
		switch(TypeOfFunc(u))
		{
			case NORMAL_FUNC:
				/*�����ҪС��E��*/
				ret = rip_network_add( net_addr, 0, pprocess);
				if( RIP_SUCCESS != ret )
				{
					rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
					return RIP_FAIL;
				}
				break;
			case NOPREF_FUNC:
				ret = rip_network_delete( net_addr, 0, pprocess );
				if( RIP_SUCCESS != ret )
				{
					rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
					return RIP_FAIL;
				}
				break;
			default:
				break;
		}
	}
	else
	{
		return subcmd(rip_cmds_net_mask_tab, NULL, argc, argv, u);
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_net_addr_mask
��������: network A.B.C.D A.B.C.D�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_net_addr_mask(int argc, char **argv, struct user *u)
{
	int ret;
	struct parameter param;
	uint32 net_addr;
	uint32 net_mask;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	processid = GetProcID(u);

	ret = rip_lookup_process_byprocessid(processid, & pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	/*network net_addr*/
	param.type = ARG_ADDR;
	ret = getparameter(argc + 2, argv - 2, u, &param);
	if(ret != 0) 
	{
		return ret;
	}
	net_addr = param.value.v_addr;

	/*network net_mask*/
	param.type = ARG_ADDR;
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if(ret != 0) 
	{
		return ret;
	}
	net_mask = param.value.v_addr;
	
	/*������*/
	if(TRUE != check_mask(net_mask) )
	{
		if( IsChinese(u) )
		{
			vty_output( "%%����, ��Ч���� %s\n", ip_ntoa(net_mask) );
		}
		else
		{
			vty_output( "%%Err, Invalid mask %s\n", ip_ntoa(net_mask) );
		}
		
		return RIP_FAIL;
	}

	net_addr = net_addr & net_mask;
	
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			ret = rip_network_add( net_addr, net_mask, pprocess );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}
			break;
		case NOPREF_FUNC:
			ret = rip_network_delete( net_addr, net_mask, pprocess );
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}
#endif
/*===========================================================
������:   rip_cmd_distance
��������: distance�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_distance(int argc, char **argv, struct user *u)
{
	return subcmd( rip_cmds_distance_val_tab, NULL, argc, argv, u );
}

/*===========================================================
������:   rip_cmd_distance_val
��������: distance <1-255>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_distance_val(int argc, char **argv, struct user *u)
{
/*    uint32 vrf_id;
    vrf_id = GetVrfID( u );
	if( TRUE != rip_is_enable(u, vrf_id) )
	{
		return RIP_FAIL;
	}*/
	return do_route_distance_value( argc, argv, u );
}

/*===========================================================
������:   rip_cmd_timer
��������: timer�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_CMD_FAIL
��ע:
=============================================================*/
int rip_cmd_timer(int argc, char **argv, struct user *u)
{
	return subcmd( rip_cmds_timer_tab, NULL, argc, argv, u );
}

/*===========================================================
������:   rip_cmd_timer_update
��������: timer update�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_timer_update(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;

	/*���������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			return subcmd( rip_cmds_timer_update_val_tab, NULL, argc, argv, u );
			break;			
		case NOPREF_FUNC:
		case DEFPREF_FUNC:
			ret = cmdend(argc - 1, argv + 1, u);
			if(ret != 0)
			{
				return ret;
			}
			
			processid = GetProcID( u );

			ret = rip_lookup_process_byprocessid(processid, &pprocess);
			if(!ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}

			if( pprocess->update_timeout > RIP_DEF_UPDATE_TIMEOUT )
			{
				pprocess->update_timeout = RIP_DEF_UPDATE_TIMEOUT;
				
				/*����update��ʱ��*/
				sys_stop_timer( pprocess->update_timer_id );

				ret = sys_start_timer( pprocess->update_timer_id, pprocess->update_timeout );
				if( SYS_NOERR != ret )
				{
					rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
					return RIP_FAIL;
				}
			}
			else
			{
				pprocess->update_timeout = RIP_DEF_UPDATE_TIMEOUT;
			}
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_timer_update_val
��������: timer update <1-4294967295>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_timer_update_val(int argc, char **argv, struct user *u)
{
	int ret;
	struct parameter param;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	/*��������в����Ƿ����*/
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	processid = GetProcID( u );

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	param.type = ARG_UINT;
	param.min = 1;
	param.max = 0x7fffffff;
	param.flag = ARG_MIN | ARG_MAX ;
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if( ret != 0 )
	{
		return ret;
	}

	if( pprocess->update_timeout > param.value.v_int )
	{
		pprocess->update_timeout = param.value.v_int;
		
		/*����update��ʱ��*/
		sys_stop_timer( pprocess->update_timer_id );

		ret = sys_start_timer( pprocess->update_timer_id, pprocess->update_timeout );
		if( SYS_NOERR != ret )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}
	}
	else
	{
		pprocess->update_timeout = param.value.v_int;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_timer_expire
��������: timer expire�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_timer_expire(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;

	/*���������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			return subcmd( rip_cmds_timer_expire_val_tab, NULL, argc, argv, u );
			break;			
		case NOPREF_FUNC:
		case DEFPREF_FUNC:
			ret = cmdend(argc - 1, argv + 1, u);
			if(ret != 0)
			{
				return ret;
			}
			
			processid = GetProcID( u );

			ret = rip_lookup_process_byprocessid(processid, & pprocess);
			if(!ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}


			if( pprocess->expire_timeout != RIP_DEF_EXPIRE_TIMEOUT )
			{
				pprocess->expire_timeout = RIP_DEF_EXPIRE_TIMEOUT;
				
				/*����expire��ʱ��*/
				sys_stop_timer( pprocess->expire_timer_id );

				ret = rip_expire_timeout(pprocess->process_id);
				if( RIP_SUCCESS != ret )
				{
					rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
					return RIP_FAIL;
				}
			}
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_timer_expire_val
��������: timer expire <1-4294967295>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_timer_expire_val(int argc, char **argv, struct user *u)
{
	int ret;
	struct parameter param;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	/*��������в����Ƿ����*/
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	processid = GetProcID( u );

	ret = rip_lookup_process_byprocessid(processid, & pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	param.type = ARG_UINT;
	param.min = 1;
	param.max = 0x7fffffff;
	param.flag = ARG_MIN | ARG_MAX ;
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if( ret != 0 )
	{
		return ret;
	}

	if( param.value.v_int != pprocess->expire_timeout )
	{
		pprocess->expire_timeout = param.value.v_int;
		
		/*����holddown��ʱ��*/
		sys_stop_timer( pprocess->expire_timer_id );

		ret = rip_expire_timeout(pprocess->process_id);
		if( RIP_SUCCESS != ret )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_timer_hold
��������: timer holddown�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_timer_hold(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;

	/*���������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			return subcmd( rip_cmds_timer_hold_val_tab, NULL, argc, argv, u );
			break;			
		case NOPREF_FUNC:
		case DEFPREF_FUNC:
			ret = cmdend(argc - 1, argv + 1, u);
			if(ret != 0)
			{
				return ret;
			}
			
			processid = GetProcID( u );

			ret = rip_lookup_process_byprocessid(processid, & pprocess);
			if(!ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}

			if( RIP_DEF_HOLDDOWN_TIMEOUT != pprocess->holddown_timeout )
			{
				pprocess->holddown_timeout = RIP_DEF_HOLDDOWN_TIMEOUT;
				
				/*����holddown��ʱ��*/
				sys_stop_timer( pprocess->holddown_timer_id  );

				ret = rip_holddown_timeout(pprocess->process_id);
				if( RIP_SUCCESS != ret )
				{
					rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
					return RIP_FAIL;
				}
			}
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_timer_hold_val
��������: timer holddown <1-4294967295>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_timer_hold_val(int argc, char **argv, struct user *u)
{
	int ret;
	struct parameter param;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	/*��������в����Ƿ����*/
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	processid = GetProcID( u );

	ret = rip_lookup_process_byprocessid(processid, & pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}


	param.type = ARG_UINT;
	param.min = 1;
	param.max = 0x7fffffff;
	param.flag = ARG_MIN | ARG_MAX ;
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if( ret != 0 )
	{
		return ret;
	}

	if( param.value.v_int != pprocess->holddown_timeout )
	{
		pprocess->holddown_timeout = param.value.v_int;
		
		/*����holddown��ʱ��*/
		sys_stop_timer( pprocess->holddown_timer_id  );

		ret = rip_holddown_timeout( pprocess->process_id);
		if( RIP_SUCCESS != ret )
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}
	}
	
	return RIP_SUCCESS;
}
/*******************************************************************************
 *
 * FUNCTION	:	rip_cmd_timer_peer
 *
 * PARAMS		:	argc		- 
 *					argv		- 
 *					u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2009.12.05 10:43:41
 *
*******************************************************************************/
int rip_cmd_timer_peer(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;

	/*���������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			return subcmd( rip_cmds_peer_timer_val_tab, NULL, argc, argv, u );
			break;			
		case NOPREF_FUNC:
		case DEFPREF_FUNC:
			ret = cmdend(argc - 1, argv + 1, u);
			if(ret != 0)
			{
				return ret;
			}
			
			processid = GetProcID( u );

			ret = rip_lookup_process_byprocessid(processid, &pprocess);
			if(!ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}

			pprocess->peer_timeout = RIP_DEF_PEER_TIMEOUT;	
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

int rip_cmd_timer_peer_val(int argc, char **argv, struct user *u)
{
	int ret;
	struct parameter param;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	/*��������в����Ƿ����*/
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	processid = GetProcID( u );

	ret = rip_lookup_process_byprocessid(processid, & pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	param.type = ARG_UINT;
	param.min = 0;
	param.max = 1000;
	param.flag = ARG_MIN | ARG_MAX ;
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if( ret != 0 )
	{
		return ret;
	}

	if( param.value.v_int != pprocess->peer_timeout)
	{        
       	pprocess->peer_timeout = param.value.v_int;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_timer_trigger
��������: timer trigger�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_timer_trigger(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 processid;
	struct rip_process_info_ *pprocess;

	/*���������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			return subcmd( rip_cmds_trigger_timer_val_tab, NULL, argc, argv, u );
			break;			
		case NOPREF_FUNC:
		case DEFPREF_FUNC:
			ret = cmdend(argc - 1, argv + 1, u);
			if(ret != 0)
			{
				return ret;
			}
			
			processid = GetProcID( u );
			ret = rip_lookup_process_byprocessid(processid, &pprocess);
			if(!ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}

			if(pprocess->trigger_timeout == RIP_DEF_TRIGGER_TIMEOUT)break;
			pprocess->trigger_timeout = RIP_DEF_TRIGGER_TIMEOUT;
			ret = rip_trigger_timeout(pprocess->process_id);
			if( RIP_SUCCESS != ret )
			{
				rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
				return RIP_FAIL;
			}	
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_timer_trigger_val
��������: timer trigger <0-5>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_timer_trigger_val(int argc, char **argv, struct user *u)
{
	int ret;
	struct parameter param;
	uint32 processid;
	struct rip_process_info_ *pprocess;
	
	/*��������в����Ƿ����*/
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	processid = GetProcID( u );

	ret = rip_lookup_process_byprocessid(processid, &pprocess);
	if(!ret )
	{
		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
		return RIP_FAIL;
	}

	param.type = ARG_UINT;
	param.min = 0;
	param.max = 5;
	param.flag = ARG_MIN | ARG_MAX ;
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if( ret != 0 )
	{
		return ret;
	}

	if( param.value.v_int != pprocess->trigger_timeout )
	{        
	    pprocess->trigger_timeout = param.value.v_int;
		/*����trigger��ʱ��*/	
		ret = rip_trigger_timeout( pprocess->process_id);
		if( RIP_SUCCESS != ret )
		{
			return RIP_FAIL;
		}
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_redis
��������: redistribute�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_redis(int argc, char **argv, struct user *u)
{
/*	uint32 vrf_id;

	vrf_id = GetVrfID( u );
	
	if( TRUE != rip_is_enable(u, vrf_id) )
	{
		return RIP_FAIL;
	}*/

	return cmd_redistribute( argc, argv, u );
}


/*===========================================================
������:   rip_cmd_offset
��������: offset�������
�������: argc/argv :�������
                        u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_offset(int argc, char **argv, struct user *u)
{
 /*   uint32 vrf_id;
    vrf_id = GetVrfID( u );
	if( TRUE != rip_is_enable(u, vrf_id) )
	{
		return RIP_FAIL;
	}   
*/
	return do_route_offset_list( argc, argv, u );
}

/*===========================================================
������:   rip_cmd_filter
��������: filter�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_filter(int argc, char **argv, struct user *u)
{
	/*uint32 vrf_id;

	vrf_id = GetVrfID( u );
	if( TRUE != rip_is_enable(u, vrf_id) )
	{
		return RIP_FAIL;
	}
*/
	return do_route_distribute_list( argc, argv, u );
}


/*===========================================================
������:   rip_cmd_intf_rip
��������: ip rip�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_rip(int argc, char **argv, struct user *u)
{
	/*fengsb ���bug 5970: ��76�ϵ�POS������rip�������޷���Ч*/
	/*Fengsb add 20070810 ���ڱ���˿�����״̬�¶�Ӧ�Ķ˿�
	diid�����������ڲ������ж�ε���GetMomentaryDevId( u, (DEVICE_ID *)&device_index );
	�ŵ���:1.�������Ч��
	                2. ����Ҫ���ǲ�����ͳһ�ķ�װ����rt_if_index,������76**����
	                �ϵ���ֲ����,CPOS��ת��̫�ڣ����߿�����ʾ��G�ڣ���
	                ����������ʾ��������s1/1�Ĵ���*/
	/*u->struct_p[1] = rt_if_index(u);*/
	RIP_SET_DEVICE_INDEX(u);
	return subcmd( rip_cmds_intf_rip_tab, NULL, argc, argv, u);
}


int rip_cmd_intf_rip_process(int argc , char **argv , struct user *u)
{	
	return subcmd(rip_cmd_intf_rip_process_tab, NULL, argc, argv, u);
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_cmd_intf_rip_process_enable
 *
 * PARAMS		:	None.
 *
 * RETURN		:	
 *
 * NOTE			:	�Զ˿���ip rip processid enable����Ĵ�����ӱ���·�ɺͽ��̶˿�
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2010.03.05 11:19:26
 *
*******************************************************************************/
int rip_cmd_intf_rip_process_enable(int argc , char **argv , struct user *u)
{
	int ret;
	uint32 processid = 0;
	uint32 device_index;
	struct rip_intf_ *pintf;
	struct rip_process_info_ *pprocess;
	struct rip_route_activate_msg_ msg;

	/*��������в����Ƿ����*/
	ret = cmdend( argc - 1, argv + 1, u );
	if( ret != 0 ) 
	{
		return ret;
	}

	ret = getuintrc(&processid, argc+2, argv-2, 1, 65535, u);
	if (ARG_NOERR != ret) {
		return -1;
	}

	device_index = RIP_GET_DEVICE_INDEX(u);
	if(!(pintf = rip_intf_array[device_index]))
		return RIP_FAIL;
	
	switch( TypeOfFunc(u) )
	{
	case NORMAL_FUNC: 
		/* һ���˿�ֻ�ܰ󶨵�һ������
		 * Commented by dangzhw in 2010.03.03 11:12:35 */
		if(pintf->process_id != processid)
		{
			if(pintf->process_id !=0)/*�Ѿ����ڱ�Ľ���*/
			{
				/*syslog(LOG_WARNING,"RIP:%s has been bind to process %d\n",pintf->intf_name , pintf->process_id);*/
				vty_output("RIP:interface %s has been bind to process %d\n",pintf->intf_name , pintf->process_id);/*���ٴ�ӡ��ֱ�ӷ���*/
				return RIP_FAIL;
			}
		}
		else/*�Ѿ����ù���������*/
		{
			return RIP_SUCCESS;
		}
		
		/* ���˿ڰ󶨵�ĳ������ǰ��Ҫ����ȫ�������ɸý���
		 * Commented by dangzhw in 2010.03.03 11:17:50 */
		ret = rip_lookup_process_byprocessid(processid, &pprocess);
		if(!ret)
		{
#if 0
			vty_output("RIP:please creat process %d first\n", processid);
			return RIP_FAIL;
#else
			/* �����������У�����ip rip n enable�����������ڵ�ǰ��ʾ���������������Ч,
			�޶�Ϊ����ǰ�����ڸ�ʵ�������Ըö˿��ϵ�vrf����ʵ��
			 * Commented by dangzhw in 2010.06.07 16:23:13 */
			ret = rip_cmd_rip_process_enter(u,processid,pintf->vrf_id,FALSE);
			if(ret != RIP_SUCCESS)
				return ret;	

			ret = rip_lookup_process_byprocessid(processid, &pprocess);
			if(!ret)
			{
				vty_output("RIP:process %d not exsit\n", processid);
				return RIP_FAIL;
			}
#endif
		}

		/* �ڽӿںͽ��̵�vrf��һ�µĸ�����ʾ�����������ã��ȴ��ӿ����õ�vrf�ı��һ��ʱ��Ч
		 * Commented by dangzhw in 2010.03.24 10:20:15 */
		if(pprocess->vrf_id != pintf->vrf_id)
			vty_output( "Rip warning:interface %s has a different vrf wirh process %d(%d vs %d)" , pintf->intf_name , pprocess->process_id,pintf->vrf_id,pprocess->vrf_id);

		/* �����˿ڵ���Ӧ�Ľ���
		 * Commented by dangzhw in 2010.03.03 11:23:37 */
		pintf->process_id = processid;
		pintf->pprocess = pprocess;

		/* ���˿���ӵ�rip���̵Ķ˿�������
		 * Commented by dangzhw in 2010.03.03 11:23:50 */
		rip_relate_intf_to_process(device_index , pprocess , RIP_INTF_ADD_TO_PROCESS);

		memset(&msg , 0 ,sizeof(struct rip_route_activate_msg_));
		msg.u.intf.device_index = device_index;
		msg.u.intf.warning = TRUE;
		msg.type= RIP_ROUTE_INTF_PROCESS_ENABLE;

		rip_process_route_activate(pprocess , &msg);

		break;			
	case NOPREF_FUNC:
		if(pintf->process_id != processid)
		{
			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
			return RIP_FAIL;
		}

		ret = rip_lookup_process_byprocessid(processid, &pprocess);
		if(!ret)
		{
			vty_output("RIP:process %d not exist\n", processid);
			return RIP_FAIL;
		}
		
		memset(&msg , 0 ,sizeof(struct rip_route_activate_msg_));
		msg.u.intf.device_index = device_index;
		msg.type= RIP_ROUTE_INTF_PROCESS_DISABLE;

		/* ��ֱ��·�ɺͻ���·�ɴӽ��̵�database��ɾ��
		 * Commented by dangzhw in 2010.03.05 11:17:11 */
		rip_process_route_activate(pprocess , &msg);

		/* ���˿ڴӽ��̵Ķ˿����������
		 * Commented by dangzhw in 2010.03.05 11:17:35 */
		rip_relate_intf_to_process(device_index,  pprocess, RIP_INTF_DEL_FROM_PROCESS);

		pintf->pprocess = NULL;
		pintf->process_id = 0;
		
		break;
	default:
		break;
	}

	return RIP_SUCCESS;
}
/*===========================================================
������:   rip_cmd_intf_auth
��������: ip rip authentication�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_auth(int argc, char **argv, struct user *u)
{
    int ret;
    uint32 device_index;

    switch( TypeOfFunc(u) )
    {
    case NORMAL_FUNC:
		return subcmd(rip_cmds_intf_auth_tab, NULL, argc, argv, u);
        break;
    case NOPREF_FUNC:
        ret = cmdend(argc - 1, argv + 1, u);
        if(ret != 0) 
        {
            return ret;
        }
        device_index = u->struct_p[1];
        /*check the device index add by fangqi  2006-10-11*/
        if(rip_intf_array[device_index]==NULL)
        {
            return RIP_FAIL;
        }

        rip_intf_array[device_index]->auth_type = RIP_AUTH_NONE;
		rip_intf_array[device_index]->dynamicflag =0;
        break;
    default:
        break;
    }

    return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_intf_auth_simple
��������: ip rip authentication simple�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_auth_simple(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 device_index;

	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	device_index = u->struct_p[1];
	/*check the device index add by fangqi  2006-10-11*/
	if(rip_intf_array[device_index]==NULL)
	{
       	return RIP_FAIL;
	}
	rip_intf_array[device_index]->auth_type = RIP_AUTH_SIMPLE;
	rip_intf_array[device_index]->dynamicflag = 0;

	return RIP_SUCCESS;
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_neigh_cmd_dynamic_authen
 *
 * PARAMS		:	argc		- 
 *					argv		- 
 *					u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	
 *
 * AUTHOR		:	dangzw
 *
 * DATE			:	2009.03.04 18:53:43
 *
*******************************************************************************/
int rip_neigh_cmd_dynamic_authen(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 device_index;

	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	device_index = u->struct_p[1];
	/*check the device index add by fangqi  2006-10-11*/
	if(rip_intf_array[device_index]==NULL)
	{
       	return RIP_FAIL;
	}
	rip_intf_array[device_index]->auth_type = RIP_AUTH_DYNAMIC;
	rip_intf_array[device_index]->dynamicflag =1;

	return RIP_SUCCESS;
}

int rip_cmds_intf_authen_commit(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 device_index;
	struct rip_peer_list_ *peer;
	struct rip_process_info_ *pprocess;
	struct rip_intf_ *rintf;
	
	/*CMD_REDN_SYNC_CLEAR(u);*/
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	device_index = u->struct_p[1];
	rintf=rip_intf_array[device_index];
	if(rintf==NULL)return RIP_FAIL;
	
	pprocess=rintf->pprocess;
	if(!pprocess)return RIP_FAIL;
	rintf->auth_commit_time=time_sec;

	peer=pprocess->peer_list.forw;
	while(peer!=&(pprocess->peer_list))
	{
		if(peer->peer_intf==device_index)
		{
			peer->auth_state=1;
			rip_peer_request(peer,pprocess);
		}
		peer=peer->forw;
	}
	sys_start_timer(rintf->auth_commit_timer_id,RIP_AUTH_COMMIT_TIMEOUT);
	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_intf_auth_md5
��������: ip rip authentication message-digest�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_auth_md5(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 device_index;

	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

	device_index = u->struct_p[1];
	/*check the device index add by fangqi  2006-10-11*/
	if(rip_intf_array[device_index]==NULL)
	{
       	return RIP_FAIL;
	}

	rip_intf_array[device_index]->auth_type = RIP_AUTH_MD5;
	rip_intf_array[device_index]->dynamicflag = 0;

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_intf_simple
��������: ip rip password�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_simple(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 device_index;

	/*����������������ͬ����*/
	switch(TypeOfFunc(u)) 
	{
		case NORMAL_FUNC:
			return subcmd( rip_cmds_intf_pswd_tab, NULL, argc, argv, u);
			break;			
		case NOPREF_FUNC:
			/*��������в����Ƿ����*/
			ret = cmdend( argc - 1, argv + 1, u );
			if( ret != 0 ) 
			{
				return ret;
			}

			device_index = u->struct_p[1];
		    /*check the device index add by fangqi  2006-10-11*/
	        if(rip_intf_array[device_index]==NULL)
	        {
       	       	return RIP_FAIL;
	        } 
			memset( rip_intf_array[device_index]->simple_key, 0, 16 );
			break;
 		default:
 			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_intf_simple_key
��������: ip rip password string�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_simple_key(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 device_index;

	/*��������в����Ƿ����*/
	ret = cmdend( argc - 1, argv + 1, u );
	if(ret != 0) 
	{
		return ret;
	}

	device_index = u->struct_p[1];
    /*check the device index add by fangqi  2006-10-11*/
	if(rip_intf_array[device_index]==NULL)
	{
       	return RIP_FAIL;
	}
	
	if( strlen( *argv ) > 16 )
	{
		cmderror(ERR_RANGE, *argv, u);
		return RIP_FAIL;
	}

	/* add by fang qi 2006-11-24 */
	memset( rip_intf_array[device_index]->simple_key, 0, 16 );

	/* modify over */
	strcpy( rip_intf_array[device_index]->simple_key, *argv );

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_intf_md5
��������: ip rip message-digest-key�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_md5(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 device_index;
	
	switch(TypeOfFunc(u)) 
	{
		case NORMAL_FUNC:
			return subcmd( rip_cmds_intf_md5_tab, NULL, argc, argv, u);
			break;
		case NOPREF_FUNC:
			ret = cmdend(argc - 1, argv + 1, u);
			if(ret != 0)
			{
				return ret;
		    }

			device_index = u->struct_p[1];
			/*check the device index add by fangqi  2006-10-11*/
	        if(rip_intf_array[device_index]==NULL)
	        {
		        return RIP_FAIL;
	        }

			BIT_RESET(rip_intf_array[device_index]->md5_type, RIP_AUTH_MD5_KEY_NORMAL);
			BIT_RESET(rip_intf_array[device_index]->md5_type, RIP_AUTH_MD5_KEY_HIDDEN);
			memset( rip_intf_array[device_index]->md5_key, 0, 16 );
			rip_intf_array[device_index]->md5_keyid = 0;
			break;
		default:
			break;
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_intf_md5_keyid
��������: ip rip message-digest-key <0-255>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_md5_keyid(int argc, char **argv, struct user *u)
{
	int ret;
	struct parameter param;
	
	param.type = ARG_UINT;
	param.min = 0;
	param.max = 255;
	param.flag = ARG_MIN | ARG_MAX;
	
	ret = getparameter(argc + 1, argv - 1, u, &param);
	if(ret != 0)
	{
		vty_output("%%rip: error keyid!\n");
		return ret;
	}
	u->struct_p[3] = param.value.v_int;
	return subcmd(rip_cmds_intf_md5_keyid_tab, NULL, argc, argv, u); 
}

/*===========================================================
������:   rip_cmd_intf_md5_keyid_md5
��������: ip rip message-digest-key <0-255> md5�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_md5_keyid_md5(int argc, char **argv, struct user *u)
{
	u->struct_p[2] = (unsigned long)RIP_AUTH_MD5_KEY_NORMAL;
	return subcmd(rip_cmds_intf_md5_keyid_md5_tab, NULL, argc, argv, u);
}

/*===========================================================
������:   rip_cmd_intf_md5_keyid_md5_normal
��������: ip rip message-digest-key <0-255> md5 0�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_md5_keyid_md5_normal(int argc, char **argv, struct user *u)
{
	u->struct_p[2] = (unsigned long)RIP_AUTH_MD5_KEY_NORMAL;
	u->cmd_mskbits = RIP_CMD_MASKBIT<<2;
	return subcmd( rip_cmds_intf_md5_keyid_md5_tab, &u->cmd_mskbits, argc, argv, u );
}

/*===========================================================
������:   rip_cmd_intf_md5_keyid_md5_hidden
��������: ip rip message-digest-key <0-255> md5 7 �������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_md5_keyid_md5_hidden(int argc, char **argv, struct user *u)
{
	u->struct_p[2] = (unsigned long)RIP_AUTH_MD5_KEY_HIDDEN;
	u->cmd_mskbits = RIP_CMD_MASKBIT<<2;
	return subcmd( rip_cmds_intf_md5_keyid_md5_tab, &u->cmd_mskbits, argc, argv, u );
}
/*===========================================================
������:   rip_cmd_intf_md5_keyid_md5_str
��������: ip rip message-digest-key <0-255> md5 string�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_md5_keyid_md5_str(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 device_index;
	uint32 u4Flags;
	u_char md5_temp[16+1];
	
	device_index = u->struct_p[1];
	u4Flags = u->struct_p[2];
	/*check the device index add by fangqi  2006-10-11*/
	if(rip_intf_array[device_index]==NULL)
	{
       	return RIP_FAIL;
	}
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	memset(md5_temp, 0, 16+1);
	if(BIT_TEST(u4Flags, RIP_AUTH_MD5_KEY_HIDDEN))
	{
		if( strlen( *argv ) > 34 )
		{
			cmderror(ERR_RANGE, *argv, u);
			return RIP_FAIL;
		}
		if(um_pass_string_crypt((u_char *)*argv,  md5_temp))
		{	
			vty_output("%%rip md5 invalid encrypted key.\n");
			return RIP_FAIL;
		}
	}
	else
	{
		if( strlen( *argv ) > 16 )
		{
			cmderror(ERR_RANGE, *argv, u);
			return RIP_FAIL;
		}
		memcpy(md5_temp,  *argv, strlen(*argv));
	}
	
	/*Get md5 key_id*/
	rip_intf_array[device_index]->md5_keyid = u->struct_p[3];
	BIT_SET(rip_intf_array[device_index]->md5_type, u4Flags);
    /* modify by fangqi   2006-11-24 */
	memset( rip_intf_array[device_index]->md5_key, 0, 16 );
    /*modify over */	   
	strcpy( rip_intf_array[device_index]->md5_key, (char*)md5_temp );

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_intf_passive
��������: ip rip passive�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_passive(int argc, char **argv, struct user *u)
{
    int ret;
    uint32 device_index;

    /*��������в����Ƿ����*/
    ret = cmdend( argc - 1, argv + 1, u );
    if(ret != 0)
    {
        return ret;
    }

    device_index = u->struct_p[1];
    /*check the device index add by fangqi  2006-10-11*/
    if(rip_intf_array[device_index]==NULL)
    {
        return RIP_FAIL;
    }

    /*����������������ͬ����*/
    switch( TypeOfFunc(u) )
    {
    case NORMAL_FUNC:
	BIT_SET(rip_intf_array[device_index]->special_flag, RIP_PASSIVE_ENABLE);
        break;
    case NOPREF_FUNC:
    case DEFPREF_FUNC:
        BIT_RESET(rip_intf_array[device_index]->special_flag, RIP_PASSIVE_ENABLE);
        break;
    default:
        break;
    }

    return RIP_SUCCESS;	
}

/*===========================================================
������:   rip_cmd_intf_bfd_enable
��������: ip rip bfd-enable�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_bfd_enable(int argc, char **argv, struct user *u)
{
    int ret;
    uint32 device_index;
    /*��������в����Ƿ����*/
    ret = cmdend( argc - 1, argv + 1, u );
    if(ret != 0)
    {
        return ret;
    }

    device_index = u->struct_p[1];

    if(rip_intf_array[device_index]==NULL)
    {
        return RIP_FAIL;
    }

    /*����������������ͬ����*/
    switch( TypeOfFunc(u) )
    {
    case NORMAL_FUNC:
		if(rip_intf_array[device_index]->bfd_enable_flag == 1)
			break;
		/*��bfd ע���ע�ص�*/
		rip_intf_bfd_register(device_index,1);
		rip_intf_array[device_index]->bfd_enable_flag = 1;
        break;
    case NOPREF_FUNC:
    case DEFPREF_FUNC:
		if(!rip_intf_array[device_index]->bfd_enable_flag)
			break;
		/*ȡ�� ע��ص�*/
		rip_intf_bfd_register(device_index,0);
		rip_intf_array[device_index]->bfd_enable_flag = 0;
		/*�˿�ʵ��״̬up��rip�˿���up*/
		if(BIT_TEST(rt_get_if_status(device_index), RIP_INTF_LINK_UP)){
			rip_debug(RIP_DEBUG_IP_RIP_MSG, "rip intf RIP_INTF_LINK_UP no use bfd\n");
			rip_intf_bfd_callback(device_index,1);
		}

        break;
    default:
        break;
    }

    return RIP_SUCCESS;	
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_cmd_intf_deaf
 *
 * PARAMS		:	argc		- 
 *					argv		- 
 *					u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	�����������ѡ��󣬸ö˿ڽ����ٽ����κ�rip����
 					�����Է���update����
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2009.11.11 13:29:11
 *
*******************************************************************************/
int rip_cmd_intf_deaf(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 device_index;

	/*��������в����Ƿ����*/
	ret = cmdend( argc - 1, argv + 1, u );
	if(ret != 0)
	{
		return ret;
	}

	device_index = u->struct_p[1];
	if(rip_intf_array[device_index]==NULL)
	{
		return RIP_FAIL;
	}

	/*����������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			BIT_SET(rip_intf_array[device_index]->special_flag, RIP_NOTRECV_ENABLE);
			break;
		case NOPREF_FUNC:
		case DEFPREF_FUNC:
			BIT_RESET(rip_intf_array[device_index]->special_flag, RIP_NOTRECV_ENABLE);
			break;
		default:
			break;
	}

	return RIP_SUCCESS;	
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_cmd_intf_v1demand
 *
 * PARAMS		:	argc		- 
 *					argv		- 
 *					u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	ֻ��rip����request��ʱ����v1�ĸ�ʽ���ͣ����Ǳ��ʱ���򲻹ܸ�ѡ�
 					��update�ȶ���intf���趨�ĸ�ʽ���У���û���趨����ȫ���趨��version��
 					��ȫ��Ҳ��ȱʡ������rip������Ӧ����
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2009.11.11 13:34:23
 *
*******************************************************************************/
int rip_cmd_intf_v1demand(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 device_index;

	/*��������в����Ƿ����*/
	ret = cmdend( argc - 1, argv + 1, u );
	if(ret != 0)
	{
		return ret;
	}

	device_index = u->struct_p[1];
	if(rip_intf_array[device_index]==NULL)
	{
		return RIP_FAIL;
	}

	/*����������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			if(BIT_TEST(rip_intf_array[device_index]->special_flag, RIP_SEND_V2DEMAND))
				BIT_RESET(rip_intf_array[device_index]->special_flag, RIP_SEND_V2DEMAND);
			BIT_SET(rip_intf_array[device_index]->special_flag, RIP_SEND_V1DEMAND);
			break;
		case NOPREF_FUNC:
		case DEFPREF_FUNC:
			BIT_RESET(rip_intf_array[device_index]->special_flag, RIP_SEND_V1DEMAND);
			break;
		default:
			break;
	}

	return RIP_SUCCESS;	
}

/*******************************************************************************
 *
 * FUNCTION	:	rip_cmd_intf_v2demand
 *
 * PARAMS		:	argc		- 
 *					argv		- 
 *					u			- 
 *
 * RETURN		:	
 *
 * NOTE			:	ֻ��rip����request��ʱ����v2�ĸ�ʽ���ͣ����Ǳ��ʱ���򲻹ܸ�ѡ�
 					��update�ȶ���intf���趨�ĸ�ʽ���У���û���趨����ȫ���趨��version��
 					��ȫ��Ҳ��ȱʡ������rip������Ӧ����
 *
 * AUTHOR		:	dangzhw
 *
 * DATE			:	2009.11.11 13:38:30
 *
*******************************************************************************/
int rip_cmd_intf_v2demand(int argc, char **argv, struct user *u)
{
	int ret;
	uint32 device_index;

	/*��������в����Ƿ����*/
	ret = cmdend( argc - 1, argv + 1, u );
	if(ret != 0)
	{
		return ret;
	}

	device_index = u->struct_p[1];
	if(rip_intf_array[device_index]==NULL)
	{
		return RIP_FAIL;
	}

	/*����������������ͬ����*/
	switch( TypeOfFunc(u) )
	{
		case NORMAL_FUNC:
			if(BIT_TEST(rip_intf_array[device_index]->special_flag, RIP_SEND_V1DEMAND))
				BIT_RESET(rip_intf_array[device_index]->special_flag, RIP_SEND_V1DEMAND);
			BIT_SET(rip_intf_array[device_index]->special_flag, RIP_SEND_V2DEMAND);
			break;
		case NOPREF_FUNC:
		case DEFPREF_FUNC:
			BIT_RESET(rip_intf_array[device_index]->special_flag, RIP_SEND_V2DEMAND);
			break;
		default:
			break;
	}

	return RIP_SUCCESS;	
}
/*===========================================================
������:   rip_cmd_intf_split
��������: ip rip split-horizon�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_split( int argc, char **argv, struct user *u )
{
    int ret;
    uint32 device_index;

    /*����������������ͬ����*/
    switch( TypeOfFunc(u) )
    {
    case NORMAL_FUNC:
    case NOPREF_FUNC:
        return subcmd( rip_cmds_intf_split_tab, NULL, argc, argv, u );
        break;
    case DEFPREF_FUNC:
        ret = cmdend( argc - 1, argv + 1, u );
        if(ret != 0)
        {
            return ret;
        }			
        device_index = u->struct_p[1];
        /*check the device index add by fangqi  2006-10-11*/
        if(rip_intf_array[device_index]==NULL)
        {
            return RIP_FAIL;
        }
        rip_intf_array[device_index]->split_flag = RIP_SPLIT_HORIZON_DEF;
        break;
    default:
        break;
    }

    return RIP_SUCCESS;	
}

/*===========================================================
������:   rip_cmd_intf_split_simple
��������: ip rip split-horizon simple�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_split_simple( int argc, char **argv, struct user *u )
{
    int ret;
    uint32 device_index;

    ret = cmdend( argc - 1, argv + 1, u );
    if(ret != 0)
    {
        return ret;
    }	
    device_index = u->struct_p[1];
    /*check the device index add by fangqi  2006-10-11*/
    if(rip_intf_array[device_index]==NULL)
    {
        return RIP_FAIL;
    }

    /*attention:  when split_flag's first bit is 1, then RIP_SIMPLE_SPLIT_HORIZON is ENABLE */ 
    switch( TypeOfFunc(u) )
    {
    case NORMAL_FUNC:
        /*modify by fangqi on 2007-06-01  */
        rip_intf_array[device_index]->split_flag |= RIP_SIMPLE_SPLIT_HORIZON_ENABLE;
        break;
    case NOPREF_FUNC:
        /*modify by fangqi on 2007-06-01  */
        rip_intf_array[device_index]->split_flag &=0xFE;  /*����1λ��Ϊ0*/
        break;
    default:
        break;
    }
    return RIP_SUCCESS;	
}

/*===========================================================
������:   rip_cmd_intf_split_poisoned
��������: ip rip split-horizon poisoned�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_split_poisoned( int argc, char **argv, struct user *u )
{
    int ret;
    uint32 device_index;

    ret = cmdend( argc - 1, argv + 1, u );
    if(ret != 0)
    {
        return ret;
    }

    device_index = u->struct_p[1];
    /*check the device index add by fangqi  2006-10-11*/
    if(rip_intf_array[device_index]==NULL)
    {
        return RIP_FAIL;
    }
    /* addtion : when split_flag's second bit is set 1, RIP_POISONED_SPLIT_HORIZON is ENABLE 
    or, it is disable */
    switch( TypeOfFunc(u) )
    {
    case NORMAL_FUNC:
        /*modify by fang qi on 2007-06-01   */
        rip_intf_array[device_index]->split_flag |= RIP_POISONED_SPLIT_HORIZON_ENABLE;  /*����2λ��Ϊ1*/
        break;
    case NOPREF_FUNC:
        /* modify by fangqi on 2007-02-28*/
        rip_intf_array[device_index]->split_flag &= 0xFD;  /*����2λ��Ϊ0*/
        break;
    default:
        break;
    }
    return RIP_SUCCESS;	
}

/*===========================================================
������:   rip_cmd_intf_send
��������: ip rip send�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_send( int argc, char **argv, struct user *u )
{
	return subcmd( rip_cmds_intf_send_tab, NULL, argc, argv, u );
}

/*===========================================================
������:   rip_cmd_intf_send_ver
��������: ip rip send version�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_send_ver( int argc, char **argv, struct user *u )
{
    int ret;
    uint32 device_index;

    switch(TypeOfFunc(u)) 
    {
    case NORMAL_FUNC:/*ip rip send version ?*/
        return subcmd( rip_cmds_intf_send_ver_tab, NULL, argc, argv, u);
        break;			
    case NOPREF_FUNC:/*no ip rip send version*/
    case DEFPREF_FUNC:/*default ip rip send version*/
        ret = cmdend(argc - 1, argv + 1, u);
        if( ret != 0 )
        {
            return ret;
        }
        device_index = u->struct_p[1];
        /*check the device index add by fangqi  2006-10-11*/
        if(rip_intf_array[device_index]==NULL)
        {
            return RIP_FAIL;
        }
        rip_intf_array[device_index]->send_version = RIP_SEND_VERSION_DEF;
        break;
    default:
        break;
    }

    return RIP_SUCCESS;
}

/*===========================================================
������:   rip_cmd_intf_send_ver_val
��������: ip rip send version <1-2>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_send_ver_val( int argc, char **argv, struct user *u )
{
	int ret;
	uint32 device_index;
	struct parameter param;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}

	param.type = ARG_UINT;
	param.min = 1;
	param.max = 2;
	param.flag = ARG_MIN | ARG_MAX ;

	ret = getparameter(argc + 1, argv - 1, u, &param);
	if( ret != 0 )
	{
		return ret;
	}
	device_index = u->struct_p[1];
	/*check the device index add by fangqi  2006-10-11*/
       if(rip_intf_array[device_index]==NULL)
	{
        	return RIP_FAIL;
	}
	
	if( 1 == param.value.v_int )
	{
		rip_intf_array[device_index]->send_version = RIP_SEND_VERSION_1;
	}
	else if( 2 == param.value.v_int )
	{
		rip_intf_array[device_index]->send_version = RIP_SEND_VERSION_2_MULTICAST;
	}

	return RIP_SUCCESS;	
}

/*===========================================================
������:   rip_cmd_intf_send_comp
��������: ip rip send version compatibility�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_send_comp( int argc, char **argv, struct user *u )
{
	int ret;
	uint32 device_index;
	
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0)
	{
		return ret;
	}
	device_index = u->struct_p[1];
       /*check the device index add by fangqi  2006-10-11*/
	if(rip_intf_array[device_index]==NULL)
       {
       	return RIP_FAIL;
	}	
	rip_intf_array[device_index]->send_version = RIP_SEND_VERSION_2_BROADCAST;

	return RIP_SUCCESS;	
}

/*===========================================================
������:   rip_cmd_intf_recv
��������: ip rip receive�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_recv( int argc, char **argv, struct user *u )
{
	return subcmd( rip_cmds_intf_recv_tab, NULL, argc, argv, u);
}

/*===========================================================
������:   rip_cmd_intf_recv_ver
��������: ip rip receive versioin�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_recv_ver( int argc, char **argv, struct user *u )
{
    int ret;
    uint32 device_index;

    switch(TypeOfFunc(u)) 
    {
    case NORMAL_FUNC:/*ip rip receive version ?*/
        return subcmd( rip_cmds_intf_recv_ver_tab, NULL, argc, argv, u);
        break;			
    case NOPREF_FUNC:/*no ip rip receive version*/
    case DEFPREF_FUNC:/*default ip rip receive version*/
        ret = cmdend(argc - 1, argv + 1, u);
        if( ret != 0 )
        {
        	return ret;
        }			
        device_index = u->struct_p[1];
        /*check the device index add by fangqi  2006-10-11*/
        if(rip_intf_array[device_index]==NULL)
        {
        	return RIP_FAIL;
        }
        rip_intf_array[device_index]->recv_version = RIP_RECV_VERSION_DEF;
        break;
    default:
        break;
    }

    return RIP_SUCCESS;
}


/*===========================================================
������:   rip_cmd_intf_recv_ver_val
��������: ip rip receive versioin <1-2>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_recv_ver_val( int argc, char **argv, struct user *u )
{
    int ret;
    uint32 device_index;
    struct parameter param;

    param.type = ARG_UINT;
    param.min = 1;
    param.max = 2;
    param.flag = ARG_MIN | ARG_MAX ;

    ret = getparameter(argc + 1, argv - 1, u, &param);
    if( ret != 0 )
    {
        return ret;
    }

    device_index = u->struct_p[1];
    /*check the device index add by fangqi  2006-10-11*/
    if(rip_intf_array[device_index]==NULL)
    {
        return RIP_FAIL;
    }

    if( argc == 1 )
    {
        ret = cmdend(argc - 1, argv + 1, u);
        if(ret != 0)
        {
            return ret;
        }

        if( 1 == param.value.v_int )
        {
            rip_intf_array[device_index]->recv_version = RIP_RECV_VERSION_1;
        }
        else if( 2 == param.value.v_int )
        {
            rip_intf_array[device_index]->recv_version = RIP_RECV_VERSION_2;
        }
    }
    else
    {
        if( 1 == param.value.v_int )
        {
            return subcmd( rip_cmds_intf_recv_ver_1_tab, NULL, argc, argv, u);
        }
        else if( 2 == param.value.v_int )
        {
            return subcmd( rip_cmds_intf_recv_ver_2_tab, NULL, argc, argv, u);
        }
    }

    return RIP_SUCCESS;	
}



/*===========================================================
������:   rip_cmd_intf_recv_ver_12
��������: ip rip receive versioin <1-2>�������
�������: argc/argv :�������
          u   : �����û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_intf_recv_ver_12( int argc, char **argv, struct user *u )
{
    int ret;
    uint32 device_index;

    ret = cmdend(argc - 1, argv + 1, u);
    if(ret != 0)
    {
        return ret;
    }
    device_index = u->struct_p[1];
    /*check the device index add by fangqi  2006-10-11*/
    if(rip_intf_array[device_index]==NULL)
    {
        return RIP_FAIL;
    }
    rip_intf_array[device_index]->recv_version = RIP_RECV_VERSION_1_2;
    
    return RIP_SUCCESS;	
}

/*===========================================================
������:   rip_cmd_prompt
��������: ����router rip����̬����ʾ����
�������: u   : �����û�
          prompt_buffer :��ʾ����Ϣ
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_prompt( struct user *u, char *prompt_buffer )
{
	char *p;
	uint32 vrf_id;
	int processid;

	vrf_id = GetVrfID(u);
	processid = GetProcID(u);
	
	p = prompt_buffer;
	
/*	if( vrf_id == 0 )
	{
		p += sprintf( p, "_config_rip_%d" ,processid );
	}
	else
	{	
	*/	p += sprintf( p, "_config_rip_%d" , processid );
/*	}	*/
	
	return (p - prompt_buffer);
}

/*===========================================================
������:   rip_cmd_exit
��������: �˳�router rip����̬�Ĵ�����
�������: u   : �����û�
          flags :����ģʽ
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_cmd_exit( struct user *u, uint32 flags )
{
/*	uint32 vrf_id;

	vrf_id = GetVrfID(u);

	if( vrf_id == 0 )
	{*/
		SetPromptFunc( u, NULL );
		SetExitFunc( u, NULL );
		modifyCmdFlag( u, FG_CFG_ST, FG_ANY );
		
		u->cmd_if = IF_NULL;
	/*}
	else
	{
		SetVrfID( u, 0 );
		
		PrepareSwitch(u, FG_RIP_VRF);
		modifyCmdFlag(u, FG_CFG_ST | FG_RIP, FG_ANY);
	}
*/
	return RIP_SUCCESS;
}
#if 0
/*===========================================================
������:   rip_is_enable
��������: �ж�RIP�����Ƿ�����
�������: u: ִ����������û�
                       vrf_id :RIP���̶�Ӧ��VRF ID
�������: ��
����ֵ:   RIP����������,����TRUE
          RIP����û������,����FALSE
��ע:
=============================================================*/
BOOL rip_is_enable( struct user *u, uint32 vrf_id )
{
	if( NULL != rip_tbl_array[vrf_id] )
	{
		return TRUE;
	}
	else
	{
		if( 0 == vrf_id )
		{
			if( IsChinese(u) )
			{
				vty_output( "%%RIP�����Ѿ��������û�ɾ��.\n" );
			}
			else
			{
				vty_output( "%%Router rip has deleted by other user.\n" );
			}
			
			PrepareSwitch(u, FG_CONFIG);
			return FALSE;
		}
		else
		{
#ifdef MULTICORE
			char vrf_name[VRF_NAME_LEN] = {'\0'};
		
			vrf_GetNamebyID(vrf_id,vrf_name);
			if( IsChinese(u) )
			{
				vty_output( "%%RIP vrf %s �����Ѿ��������û�ɾ��.\n", vrf_name );
			}
			else
			{
				vty_output( "%%Address-family ipv4 vrf %s has deleted by other user.\n", vrf_name );
			}
#else
			if( IsChinese(u) )
			{
				vty_output( "%%RIP vrf %s �����Ѿ��������û�ɾ��.\n", vrf_GetNamebyID(vrf_id) );
			}
			else
			{
				vty_output( "%%Address-family ipv4 vrf %s has deleted by other user.\n", vrf_GetNamebyID(vrf_id) );
			}
			
#endif
			PrepareSwitch(u, FG_CONFIG);
			return FALSE;
		}
	}
}

/*===========================================================
������:   rip_network_add
��������: �������õ�network addr mask������Ӧ��network list
�������: net_addr : �����õ�network addr
          net_mask : �����õ�network mask
          vrf_id   : ��Ӧ��VRF ID
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_network_add( uint32 net_addr, uint32 net_mask, struct rip_process_info_ *pprocess)
{
	struct rip_net_list_ *rip_net_list;
	uint32 mask;
	struct rip_route_activate_msg_ msg;

	/*�ж��������Ƿ��Ѵ���*/
	for( rip_net_list = pprocess->network_list.forw; rip_net_list != &(pprocess->network_list); rip_net_list = rip_net_list->forw )
	{
		if( (rip_net_list->network == net_addr) && (rip_net_list->mask == net_mask) )
		{
			/*�����ù�����*/
			return RIP_SUCCESS;
		}
	}

	/*�����µ�network list����*/
	rip_net_list = (struct rip_net_list_ *)rip_mem_malloc( sizeof(struct rip_net_list_), RIP_NET_LIST_TYPE );
	if( rip_net_list == NULL )
	{
		/*�ڴ����ʧ��*/
		fprintf(stderr, "RIP: Malloc memory failed.(%s, %d)\n", __FILE__, __LINE__ );
		return RIP_MEM_FAIL;
	}
	memset( rip_net_list, 0, sizeof(struct rip_net_list_) );
	rip_net_list->network = net_addr;
	rip_net_list->mask = net_mask;
	INSQUE( rip_net_list, pprocess->network_list.back );

	/*����Ƿ��и��ǵĶ˿�*/
	if( net_mask == 0 )
	{
		mask = rip_get_mask( net_addr );
	}
	else
	{
		mask = net_mask;
	}
	
	memset(&msg , 0 ,sizeof(struct rip_route_activate_msg_));

	msg.type = RIP_ROUTE_NET_CMD_ADD;
	msg.u.network.network = net_addr;
	msg.u.network.mask = mask;

	rip_process_route_activate(pprocess , &msg);		

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_network_delete
��������: �����õ�network addr mask����Ӧ��network list��ɾ��
�������: net_addr : �����õ�network addr
          net_mask : �����õ�network mask
          vrf_id   : ��Ӧ��VRF ID
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_network_delete( uint32 net_addr, uint32 net_mask, struct rip_process_info_ *pprocess )
{	
	struct rip_net_list_ *rip_net_list;
	uint32 mask;
	struct rip_route_activate_msg_ msg;

	/*�ж��������Ƿ��Ѵ���*/
	for( rip_net_list = pprocess->network_list.forw; rip_net_list != &(pprocess->network_list); rip_net_list = rip_net_list->forw )
	{
		if( (rip_net_list->network == net_addr) && (rip_net_list->mask == net_mask) )
		{
			/*ɾ������*/
			REMQUE( rip_net_list );
			rip_mem_free( rip_net_list, RIP_NET_LIST_TYPE );
			rip_net_list = NULL;    /*Fengsb add 2006-02-14*/
			
			/*����Ƿ��и��ǵĶ˿�*/
			if( net_mask == 0 )
			{
				mask = rip_get_mask( net_addr );
			}
			else
			{
				mask = net_mask;
			}

			memset(&msg , 0 ,sizeof(struct rip_route_activate_msg_));

			msg.type = RIP_ROUTE_NET_CMD_DEL;
			msg.u.network.network = net_addr;
			msg.u.network.mask = mask;

			rip_process_route_activate( pprocess, & msg);
#if 0
			for( device_index = 0; device_index  <= INTERFACE_DEVICE_MAX_NUMBER; device_index++ )
			{
				if( (NULL == rip_intf_array[device_index])
					|| (0 == rip_intf_array[device_index]->address)
					|| (rip_intf_array[device_index]->vrf_id != vrf_id))
				{
					continue;
				}
				
				if((rip_intf_array[device_index]->address & mask) == (net_addr & mask))
				{

					ret = rip_intf_ref_num_set(device_index, RIP_INTF_REF_NUM_SUB_1);
					if(ret != 0)
						continue;
					
					if( BIT_TEST( rip_intf_array[device_index]->link_state ,RIP_INTF_LINK_UP))
					{


						/* ��û��rip���Ǹ�intf��disable multicast in
						 * Commented by dangzhw in 2009.11.06 15:54:44 */
						rip_disable_multicast_in( device_index);
						
						/*������ص�����ź�����*/
						if( NULL != rip_intf_array[device_index]->connect_route )
						{
							intf_net = rip_intf_array[device_index]->connect_route->route_node->p.u.prefix4.s_addr;
							intf_mask =  prefix_to_mask(rip_intf_array[device_index]->connect_route->route_node->p.prefixlen);
						}

                                           /*
                                       	 rip_del_summary_route(device_index , 
       	                                   rip_intf_array[device_index]->connect_route);
                                          */
					       					
						/*ɾ���˿�ֱ��·��*/
						ret = rip_del_connect_route( device_index );
						if( RIP_SUCCESS != ret )
						{
							return RIP_FAIL;
						}

						/*ɾ���Ӷ˿�ѧϰ����·��*/
						ret = rip_clear_nbr_route_list( device_index );
						if( RIP_SUCCESS != ret )
						{
							return RIP_FAIL;
						}
#ifdef MULTICORE
						vrf_table = vrf_table_enter(vrf_id,RT_RD_LOCK);
						if(vrf_table == NULL)
						{
							return RT_FAIL;
						}
#endif
						/*����redistribute connect ������*/
						rth = rt_table_locate(vrf_id, intf_net, intf_mask);		
						if (NULL != rth &&  NULL != rth->rth_active)
						{
							 rip_create_redis_connect( rth->rth_active, vrf_id );
						}
#ifdef MULTICORE
						vrf_table_leave(vrf_table,RT_RD_LOCK);
#endif
					}
				}
			}
#endif
            break; /*Fengsb add 2006-02-14*/
		}
	}

	return RIP_SUCCESS;
}
#endif
/*===========================================================
������:   rip_enter_rip_mode
��������: ����RIP����ģʽ
�������: u:����������û�
                       vrf_id : VRF ID
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_enter_rip_process_mode( struct user *u, uint32 processid , uint32 vrfid ,uint8 enter_process_mode)
{
	int ret;

	if(!enter_process_mode)
	{
		/*tProcID(u, processid);
		SetVrfID(u, vrfid);*/
	}
	else
	{
		/*�л���ʶ��*/
		PrepareSwitch( u, FG_CONFIG );

		u->cmd_if = IF_NULL;

		ret = modifyCmdFlag( u, FG_CFG_ST | FG_RIP, FG_ANY );
		if( 0 != ret )
		{
			return ret;
		}

		SetProcID(u, processid);
		SetVrfID(u, vrfid);
		
		SetPromptFunc( u, rip_cmd_prompt );
		SetExitFunc( u, rip_cmd_exit );
	}

	return RIP_SUCCESS;
}

/*===========================================================
������:   rip_exit_rip_mode
��������: �˳�RIP����ģʽ
�������: u:����������û�
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_exit_rip_mode( struct user *u ,struct rip_process_info_ *pprocess)
{
	int ret;
	
	if(!pprocess)
		return RIP_FAIL;
	
	/*ֹͣRIPģ�鶨ʱ��*/
	ret = rip_process_stop_timer(pprocess);
	if( RIP_SUCCESS != ret )
	{
		return ret;
	}

	/*ɾ��RIPģ�鶨ʱ��*/
	ret = rip_process_delete_timer(pprocess);
	if( RIP_SUCCESS != ret )
	{
		return ret;
	}
	
	ret = sm_p(rip_semaphore, SM_WAIT, 0);
	assert(ret == 0);
	
	/*ɾ������������RIP·�ɱ�*/
	rip_clear_process_table(pprocess);

	/*ɾ��RIPȫ�ֱ����б������Ϣ*/
	rip_clear_process_info(pprocess);
	ret = sm_v(rip_semaphore);
	assert(ret == 0);
	
	PrepareSwitch(u, FG_CONFIG);
	modifyCmdFlag(u, FG_CFG_ST, FG_ANY);

	return RIP_SUCCESS;
}
#if 0
/*===========================================================
������:   rip_exit_afi_mode
��������: �˳�address-family ����ģʽ
�������: u:����������û�
                       vrf_id : VRF ID
�������: ��
����ֵ:   ����ɹ�,����RIP_CMD_SUCCESS
          ����ʧ��,����RIP_FAIL
��ע:
=============================================================*/
int rip_exit_afi_mode( struct user *u, uint32 vrf_id )
{
	/*ɾ��VRF����*/
	rip_delete_table( vrf_id );
	
	PrepareSwitch(u, FG_CONFIG);
	modifyCmdFlag(u, FG_CFG_ST | FG_RIP, FG_ANY);

	return RIP_SUCCESS;
}
#endif
