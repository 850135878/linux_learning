char rip_version_version[] = "rip_version.c last changed at 2021.03.01 10:20:54 by leibang\n";
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
 *        FILE NAME       :   rip_version.c
 *        AUTHOR          :   HQ.Sun
 *        DATE            :   2006.01.08 11:16:37
 *        PURPOSE:        :   RIP 版本控制信息
 *
**************************************************************************/
#if !defined(OS_VXWORKS)
#include <psos.h>
#include <pna.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <libsys/verctl.h>
#include <routing/rt_version.h>

#define RIP_MAJOR_VER		0x04
#define RIP_MINOR_VER		0x01
#define RIP_RELEASE_VER		18

extern char     rip_cmd_version[];
extern char     rip_debug_version[];
extern char     rip_init_version[];
extern char     rip_intf_version[];
extern char     rip_main_version[];
extern char     rip_mib_version[];
extern char     rip_packet_version[];
extern char     rip_redis_version[];
extern char     rip_show_version[];
extern char     rip_timer_version[];

static char *rip_file_list[] = {
	rip_cmd_version,
	rip_debug_version,
	rip_init_version,
	rip_intf_version,
	rip_main_version,
	rip_mib_version,
	rip_packet_version,
	rip_redis_version,
	rip_show_version,
	rip_timer_version,
	rip_version_version
};

void rip_version_register(void)
{
	struct version_list ver;
	char            rip_filedate[40];

	/*register module cmd version */
	ver.module_type = MODULE_TYPE_RIP;
	ver.version = (((RIP_MAJOR_VER & 0xFFFF) << 16) |
				   ((RIP_MINOR_VER & 0xFF) << 8) | (RIP_RELEASE_VER & 0xFF));

	GET_NEWESTFILEDATE(rip_file_list, sizeof(rip_file_list) / sizeof(char *), rip_filedate);
	sprintf(ver.module_name, "RIP");
	sprintf(ver.module_description, "RIP last update: %s", rip_filedate);
	ver.next = NULL;

	register_module_version(&ver);
}

extern int cmdend(int argc, char *argv[], struct user *u);
int rip_cmd_show_version(int argc, char **argv, struct user *u)
{
	int i = 0, ret = 0;
    
	ret = cmdend(argc - 1, argv + 1, u);
	if(ret != 0) 
	{
		return ret;
	}

    for(i = 0; i < sizeof(rip_file_list) / sizeof(char *); i++)
		VERSION_PRINT(rip_file_list[i]);
	return 0;
}

