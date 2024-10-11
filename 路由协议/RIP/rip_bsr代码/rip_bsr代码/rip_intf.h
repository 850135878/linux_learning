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
 *        FILE NAME       :   rip_intf.h
 *        AUTHOR          :   Fengsb
 *        DATE            :   2006.05.17 16:45:23
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#ifndef RIP_INTF_H
#define RIP_INTF_H

#ifdef __cplusplus
	extern "C" {
#endif /* __cplusplus */
/******************* BEGIN HEADER FILE **********************************/


int rip_task_callback( uint32 procotol, uint32 process, struct rt_msg *msg );

int rip_intf_add( uint32 device_index );
int rip_intf_del( uint32 device_index );
int rip_intf_link_up( uint32 device_index );
int rip_intf_link_down( uint32 device_index );
int rip_intf_addr_add( uint32 device_index, uint32 aid, uint32 addr, uint32 mask, uint8 addr_type, uint32 vrf_id );
int rip_intf_addr_del( uint32 device_index, uint32 addr, uint32 mask, uint8 addr_type, uint32 vrf_id );

int rip_del_vrf_process( uint32 vrf_id );

int rip_create_connect_route( uint32 device_index, uint warning);
int rip_del_connect_route( uint32 device_index );
ULONG rip_create_summary_route( ULONG device_index , struct rip_route_ *rip_intf_rt ,struct rip_process_info_ *pprocess);
ULONG rip_del_summary_route( ULONG device_index , struct rip_route_ *rip_intf_rt , struct rip_process_info_ *pprocess);

int rip_del_route( struct rip_route_ *rip_route );
int rip_enable_multicast_in( uint32 device_index );
int rip_disable_multicast_in( uint32 device_index );
BOOL rip_exist_equi_route( struct rip_route_ *rip_route );
void rip_del_from_main_tbl( struct rip_route_ *rip_route ,uint32 processid);
void rip_change_notify( struct rip_route_ *rip_route );
void rip_add_notify( struct rip_route_ *rip_route );

int rip_clear_peer_route_list(uint32 device_index,uint32 peer_addr);
int rip_clear_nbr_route_list( uint32 device_index );
void rip_del_from_nbr_list( struct rip_route_ *rip_route );
void rip_set_intf_ref_num( uint32 device_index );

uint16 rt_get_equal_rtnum_confg(void);

int rip_create_orig_route( struct rip_process_info_ *pprocess);
int rip_del_orig_route( struct rip_process_info_ *pprocess );
int rip_del_vrf_process( uint32 vrf_id );
int rip_del_intf_vrf(uint32 vrfid , uint32 device_index);
int rip_add_intf_vrf(uint32 vrfid , uint32 device_index);

int rip_intf_ref_num_set(uint32 device_index, uint32 handle_type);


/******************* END OF HEADER FILE *********************************/

#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* RIP_INTF_H */
