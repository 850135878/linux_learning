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
 *        FILE NAME       :   rip.h
 *        AUTHOR          :   Fengsb
 *        DATE            :   2006.04.30 16:25:37
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#ifndef RIP_H
#define RIP_H

#ifdef __cplusplus
	extern "C" {
#endif /* __cplusplus */
/******************* BEGIN HEADER FILE **********************************/

#ifndef	PROTO_RIP
#define PROTO_RIP

#include <routing/rt_table.h>
#include <routing/rt_intf.h>

/* modified by gaofeng in 2000/12/05 */
#include <routing/independent.h>

#include <rip/cmd_rip.h>

#ifndef	MIB_RIP
	#define MIB_RIP 1
#endif

/* Routing Information Protocol */
#define DEFAULT_METRIC 1

#define MinAgeInterval 5

/* 这些宏在rip main 的头文件中已被枚举定义为常数
#define	RIP_VERSION_0	0
#define	RIP_VERSION_1	1
#define	RIP_VERSION_2	2
*/

#define MIN_AUTH_HOLDTIME 3 /* wait 3 * rip_update_time to change auth */


/* #pragma noalign (rip_netinfo) */
#pragma pack(1)
#pragma align 1
struct rip_netinfo {
    uint16	rip_family;
    uint16	rip_tag;
    uint32	rip_dest;
	uint32	rip_mask;
	uint32	rip_router;
    uint32	rip_metric;			/* cost of route */
};
#pragma align 0
#pragma pack(0)
#pragma pack()

#define RIP_NETINFO_PIECE 20

#define RIP_METRIC_UNREACHABLE	16
#define	RIP_METRIC_SHUTDOWN	(RIP_METRIC_UNREACHABLE - 1)

/* #pragma noalign (rip_trailer) */
#pragma pack(1)
#pragma align 1
struct rip_trailer {
	uint16 family;
	uint16 authtype;
	uint16 length;
	uint8 authid;
	uint8 authlen;
    uint32 auth_sequence;
	uint32 data1;
	uint32 data2;
};
#pragma align 0
#pragma pack(0)
#pragma pack()

#define	RIP_AF_UNSPEC	0
#define	RIP_AF_INET	2
#define	RIP_AF_AUTH	0xffff


/* #pragma noalign (rip) */
#pragma pack(1)
#pragma align 1
struct rip {
    /* XXX - using bytes causes alignment problems */
    uint8	rip_cmd;		/* request/response */
    uint8	rip_vers;		/* protocol version # */
    uint16	rip_zero2;		/* unused */
};
#pragma align 0
#pragma pack(0)
#pragma pack()


/* Packet types. */
#define	RIPCMD_REQUEST		1	/* want info */
#define	RIPCMD_RESPONSE		2	/* responding to request */
#define	RIPCMD_TRACEON		3	/* turn tracing on */
#define	RIPCMD_TRACEOFF		4	/* turn it off */
#define	RIPCMD_POLL			5	/* like request, but anyone answers */
#define	RIPCMD_POLLENTRY	6	/* like poll, but for entire entry */

#define RIPCMD_TRIG_REQUEST		6
#define RIPCMD_TRIG_RESPONSE	7
#define RIPCMD_TRIG_ACK			8

#define RIPCMD_UPDATE_REQUEST	9
#define RIPCMD_UPDATE_RESPONSE	10
#define RIPCMD_UPDATE_ACK		11
#define	RIPCMD_MAX			12


#define RIP_PKTSIZE	512
#define	RIP_MAXSIZE(ifap)	MIN(RIP_PKTSIZE, ifap->dev_mtu - 8 - 20)

#define	RIP_ADDR_MC	0xe0000009 /* 224.0.0.9 */

#define RIP_T_UPDATE	(uint32) 30
#define	RIP_T_HOLDDOWN	(uint32) 120
#define	RIP_T_MAX	(uint32) 3
#define	RIP_T_FLASH	(uint32) (time_sec%((uint32)RIP_T_MAX) + 1)
#define	RIP_T_EXPIRE	(uint32) 180

#define	RIP_HOLDCOUNT		(RIP_T_HOLDDOWN/RIP_T_UPDATE)	/* Number of updates per holddown */

#define RIP_PORT	520
#define	RIP_HOP		1	/* Minimum hop count when passing through */

#define TR_RIP			0x01
#define TR_RIP_EVENTS	0x02
#define TR_RIP_PACKET	0x04
#define TR_RIP_DATABASE	0x08



/***************************************************************************
	Struct for save RIP
 ****************************************************************************/
/* Values for rip_flags */
#define	RIPF_ON				0x01 /* RIP is enabled */
#define	RIPF_BROADCAST		0x02 /* Broadcast to all interfaces */
#define	RIPF_SOURCE			0x04 /* Source packets to our peers */
#define	RIPF_CHOOSE			0x08 /* Broadcast if more than one interface */
#define	RIPF_NOCHECK		0x10 /* Don't check zero fields */
#define	RIPF_FLASHDUE		0x20 /* Flash update is due */
#define	RIPF_NOFLASH		0x40 /* Can not do a flash update until after the next normal update */
#define	RIPF_RECONFIG		0x80 /* Initial processing or reconfiguration */
#define RIPF_AUTO_SUMMARY	0x0200

#define RIPF_VALID_SOURCE	0x0400 /* validate update source */
#define RIPF_SPLIT_HORIZON	0x0800 /* split horizon */

#define RIPF_DEFAULT_INFOR	0x1000 /* default-information originate */

#define RIPF_VERSION_1		0X4000
#define RIPF_VERSION_2		0X8000


/* RIP const var */
struct RIP_Global {
	uint32 taskid;
	uint32 rt_queue;

	uint16 rip_flags;
	uint16 rip_port;
	uint16 rip_trace_types;

	struct route_redistribute rip_redis;

	int rip_socket;	/* Socket */

	uint32 rip_distance; /* Global Distance for RIP */
	uint32 rip_default_metric; /* Be used for redistribute */

	/* the route-map for default-information */
	struct route_map_struct rip_dfltinfo_map;

	struct soaddr_in rip_sock_addr;
	struct soaddr_in *rip_addr_v2;

	gw_entry rip_gw_list; /* List of RIP gateways */
	struct _distance_table rip_distance_table;
	rip_tsi_data rip_export_list;

	/* modified by gaofeng in 2000/10/24 */
	chain rip_intf;
	rip_target rip_targets;

	rip_filter rip_filter_list_in;
	rip_filter rip_filter_list_out;

	task_timer * rip_timer_update;	/* To send updates */
	task_timer * rip_timer_flash;		/* To send flash updates */
	task_timer * rip_timer_age;		/* To age routes */

	unsigned long rip_update_time;
	unsigned long rip_t_expire;
	unsigned long rip_t_holddown;

	int rip_unicast_ttl;

	unsigned char rip_send_buf[RIP_PKTSIZE];
	unsigned char rip_recv_buf[RIP_PKTSIZE];

	struct offset_list rip_offset_in;
	struct offset_list rip_offset_out;

	#ifdef	MIB_RIP
		uint32 rip_global_changes;
		uint32 rip_global_responses;
	#endif	/* MIB_RIP */

	/* modified by gaofeng in 2000/12/05 */
	struct link_protocol * rip_indep;
};

extern struct RIP_Global ripd;
extern uint32 ripd_rt_queue;

#ifdef	MIB_RIP
extern void rip_init_mib(void);
#endif	/* MIB_RIP */

extern void TRACE_RIP(uint32 flag, const char * a, ...);

extern int rip_mc_set(struct rt_tg_ifap * nifap, uint32 aid);
extern void rip_mc_reset(struct rt_tg_ifap * nifap, uint32 aid);

extern void rip_send_request(rip_target *, struct rt_tg_ifap *);

extern void rip_job(task_timer *tip);
extern void rip_need_flash (void);

extern void rip_policy(rt_head * rth, int flag);

extern int rip_export(rt_entry *, unsigned short, rip_redistribute_struct *, struct _adv_results *);


#endif	/* PROTO_RIP */


/******************* END OF HEADER FILE *********************************/

#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* RIP_H */
