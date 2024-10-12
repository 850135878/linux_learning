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
 *        FILE NAME       :   rip_cmd.h
 *        AUTHOR          :   Fengsb
 *        DATE            :   2006.02.16 15:19:51
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#ifndef RIP_CMD_H
#define RIP_CMD_H

#ifdef __cplusplus
	extern "C" {
#endif /* __cplusplus */
/******************* BEGIN HEADER FILE **********************************/

#ifndef __RIP_CMD_H
#define __RIP_CMD_H

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

#define RIP_SET_DEVICE_INDEX(u)	(u->struct_p[1] = rt_if_index(u)) 
#define RIP_GET_DEVICE_INDEX(u)	(u->struct_p[1])

int rip_register_cmd( void );
int rip_cmd_router_rip(int argc, char **argv, struct user *u);
int rip_cmd_rip_process_vrf(int argc, char *argv[], struct user *u);
int rip_cmd_afi(int argc, char **argv, struct user *u);

int rip_cmd_input_queue(int argc, char **argv, struct user *u);
int rip_cmd_input_queue_val(int argc, char **argv, struct user *u);

int rip_cmd_auto_summary(int argc, char **argv, struct user *u);
int rip_cmd_zero_domain_check(int argc, char **argv, struct user *u);
int rip_cmd_vali_source(int argc, char **argv, struct user *u);
int rip_cmd_def_info(int argc, char **argv, struct user *u);
int rip_cmd_def_info_origi(int argc, char **argv, struct user *u);
int rip_cmd_def_info_origi_safe(int argc, char **argv, struct user *u);
int rip_cmd_def_metric(int argc, char **argv, struct user *u);
int rip_cmd_def_metric_val(int argc, char **argv, struct user *u);
int rip_cmd_ver(int argc, char **argv, struct user *u);
int rip_cmd_ver_val(int argc, char **argv, struct user *u);
int rip_cmd_maxcount(int argc, char **argv, struct user *u);
int rip_cmd_max_nh(int argc, char **argv, struct user *u);
int rip_cmd_maxcount_val(int argc, char **argv, struct user *u);
int rip_cmd_maxnh_val(int argc, char **argv, struct user *u);
int rip_cmd_neigh(int argc, char **argv, struct user *u);
int rip_cmd_neigh_addr(int argc, char **argv, struct user *u);
int rip_cmd_net(int argc, char **argv, struct user *u);
int rip_cmd_net_addr_mask(int argc, char **argv, struct user *u);
int rip_cmd_distance(int argc, char **argv, struct user *u);
int rip_cmd_distance_val(int argc, char **argv, struct user *u);
int rip_cmd_distance_src(int argc, char **argv, struct user *u);
int rip_cmd_distance_src_mask(int argc, char **argv, struct user *u);
int rip_cmd_distance_acclist(int argc, char **argv, struct user *u);
int rip_cmd_timer(int argc, char **argv, struct user *u);
int rip_cmd_timer_update(int argc, char **argv, struct user *u);
int rip_cmd_timer_update_val(int argc, char **argv, struct user *u);
int rip_cmd_timer_expire(int argc, char **argv, struct user *u);
int rip_cmd_timer_expire_val(int argc, char **argv, struct user *u);
int rip_cmd_timer_hold(int argc, char **argv, struct user *u);
int rip_cmd_timer_hold_val(int argc, char **argv, struct user *u);
int rip_cmd_timer_trigger(int argc, char **argv, struct user *u);
int rip_cmd_timer_peer(int argc, char **argv, struct user *u);
int rip_cmd_timer_trigger_val(int argc, char **argv, struct user *u);
int rip_cmd_timer_peer_val(int argc, char **argv, struct user *u);
int rip_cmd_redis(int argc, char **argv, struct user *u);

int rip_cmd_offset(int argc, char **argv, struct user *u);

int rip_cmd_filter(int argc, char **argv, struct user *u);
int rip_cmd_intf_rip(int argc, char **argv, struct user *u);
int rip_cmd_intf_auth(int argc, char **argv, struct user *u);
int rip_cmd_intf_auth_simple(int argc, char **argv, struct user *u);
int rip_cmd_intf_auth_md5(int argc, char **argv, struct user *u);
int rip_cmd_intf_simple(int argc, char **argv, struct user *u);
int rip_cmd_intf_simple_key(int argc, char **argv, struct user *u);
int rip_cmd_intf_md5(int argc, char **argv, struct user *u);
int rip_cmd_intf_md5_keyid(int argc, char **argv, struct user *u);
int rip_cmd_intf_md5_keyid_md5(int argc, char **argv, struct user *u);
int rip_cmd_intf_md5_keyid_md5_normal(int argc, char ** argv, struct user *u);
int rip_cmd_intf_md5_keyid_md5_hidden(int argc, char **argv, struct user *u);
int rip_cmd_intf_md5_keyid_md5_str(int argc, char **argv, struct user *u);
int rip_cmd_intf_passive(int argc, char **argv, struct user *u);
int rip_cmd_intf_deaf(int argc, char **argv, struct user *u);
int rip_cmd_intf_v1demand(int argc, char **argv, struct user *u);
int rip_cmd_intf_v2demand(int argc, char **argv, struct user *u);

/*add for bfd*/
int rip_cmd_intf_bfd_enable(int argc, char **argv, struct user *u);

int rip_cmd_intf_split( int argc, char **argv, struct user *u );
int rip_cmd_intf_split_simple( int argc, char **argv, struct user *u );
int rip_cmd_intf_split_poisoned( int argc, char **argv, struct user *u );

int rip_cmd_intf_send( int argc, char **argv, struct user *u );
int rip_cmd_intf_send_ver( int argc, char **argv, struct user *u );
int rip_cmd_intf_send_ver_val( int argc, char **argv, struct user *u );
int rip_cmd_intf_send_comp( int argc, char **argv, struct user *u );
int rip_cmd_intf_recv( int argc, char **argv, struct user *u );
int rip_cmd_intf_recv_ver( int argc, char **argv, struct user *u );
int rip_cmd_intf_recv_ver_val( int argc, char **argv, struct user *u );
int rip_cmd_intf_recv_ver_12( int argc, char **argv, struct user *u );
int rip_cmd_prompt( struct user *u, char *prompt_buffer );
int rip_cmd_exit( struct user *u, uint32 flags );

int rip_network_add( uint32 net_addr, uint32 net_mask, struct rip_process_info_ *pprocess);
int rip_network_delete( uint32 net_addr, uint32 net_mask, struct rip_process_info_ *pprocess );

int rip_enter_rip_mode( struct user *u, uint32 vrf_id );
int rip_exit_rip_mode( struct user *u ,struct rip_process_info_ *pprocess);

int rip_cmd_rip_process_vrf_name(int argc, char **argv, struct user *u);
int rip_cmd_rip_process(int argc, char **argv, struct user *u);
int rip_cmd_intf_rip_process(int argc , char **argv , struct user *u);
int rip_cmd_intf_rip_process_enable(int argc , char **argv , struct user *u);
int rip_cmd_rip_process_enter(struct user *u ,uint32 processid, uint32 vrfid, uint8 enter_process_mode);
int rip_enter_rip_process_mode( struct user *u, uint32 processid , uint32 vrfid ,uint8 enter_process_mode);

#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif

/******************* END OF HEADER FILE *********************************/

#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* RIP_CMD_H */
