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
 *        FILE NAME       :   rip_packet.h
 *        AUTHOR          :   HQ.Sun
 *        DATE            :   2006.01.08 10:59:18
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#ifndef RIP_PACKET_H
#define RIP_PACKET_H

#ifdef __cplusplus
	extern "C" {
#endif /* __cplusplus */
/******************* BEGIN HEADER FILE **********************************/

int rip_recv_packet( uint32 pkt_len );
int rip_peer_request(struct rip_peer_list_ *peer ,struct rip_process_info_ *pprocess);
int rip_recv_request( uint32 device_index , uint32 pkt_len, struct rip_peer_list_ *peer);
int rip_recv_response( uint32 device_index , uint32 pkt_len, struct rip_peer_list_ *peer);

int rip_send_request( uint32 device_index );
void rip_send_response( uint32 device_index, uint32 dest_addr, uint16  dest_port );
void rip_send_trigger_response( uint32 device_index, uint32 dest_addr );
int rip_send_packet( uint32 device_index, uint32 dest_addr, uint16 dest_port, void *packet, uint32 pkt_len );

void rip_create_pkt_head( uint32 device_index, uint8 cmd_type );
void rip_create_pkt_tail( uint32 device_index );
void rip_add_route_to_pkt( uint32 device_index, uint32 dest_addr, uint16 dest_port,
		struct rip_route_ * rip_route,  struct rip_route_item_ *rip_route_item);
void rip_add_item_to_pkt( uint32 device_index, uint32 dest_addr, uint16 dest_port, struct rip_route_item_ *rip_route_item );

void rip_recv_item( uint32 device_index, struct rip_peer_list_ *peer, struct rip_route_item_ *rip_route_item );
int rip_create_new_route( uint32 device_index, struct rip_peer_list_ *peer, struct rip_route_item_ *rip_route_item ); 
int rip_create_first_rt_frm_nbr( uint32 device_index, struct rip_peer_list_ *peer,struct rip_route_item_ *rip_route_item);
int rip_create_equalrt_frm_new_nbr( uint32 device_index, struct rip_peer_list_ *peer,struct rip_route_item_ *rip_route_item, struct rip_route_ *rip_route_cmp);
int rip_updt_equalrt_frm_nbr( uint32 device_index, struct rip_peer_list_ *peer,struct rip_route_item_ *rip_route_item, struct rip_route_ *rip_route_old);
int rip_updt_singlert_frm_nbr( uint32 device_index, uint32 src_addr,struct rip_route_item_ *rip_route_item, struct rip_route_ *rip_route_old);
void rip_del_route_node( struct rip_route_ *rip_route );
void rip_del_nbrrt_frm_node( struct rip_route_ *rip_route );
void rip_del_rn_callback( struct rip_route_ *rip_route );
void rip_del_trigger_tree_callback( struct rip_trigger_list_ *rtl);

uint32 rip_get_pkt_dest_addr( uint32 device_index );
int rip_split_check( uint32 device_index, struct rip_route_info_ *rip_route_info );
int rip_filter_in_check( uint32 device_index, uint32 src_addr, struct rip_route_item_ *rip_route_item );
int rip_filter_out_check( uint32 device_index, uint32 gateway, struct rip_route_item_ *rip_route_item );
int rip_offset_in_check( uint32 device_index, struct rip_route_item_ *rip_route_item );
int rip_offset_out_check( uint32 device_index, struct rip_route_item_ *rip_route_item );
int rip_auto_sum_updt( uint32 device_index, uint32 dest_addr, uint16 dest_port, struct rip_route_ * rip_rthead);
int rip_no_auto_sum_updt(uint32 device_index, uint32 dest_addr, uint16 dest_port, struct rip_route_ * rip_rthead);
void rip_add_trigger_to_pkt( uint32 device_index, uint32 dest_addr, uint16 dest_port, struct rip_route_info_ *rip_route_info );
int rip_auto_summary_check( uint32 device_index, struct rip_route_info_ *rip_route_info, struct rip_route_item_ *rip_route_item );
void rip_next_hop_check( uint32 device_index, struct rip_route_info_ *rip_route_info, struct rip_route_item_ *rip_route_item );
int rip_nbrrt_check_to_pkt( uint32 device_index, struct rip_route_item_ *rip_route_item );

struct rip_route_ *rip_get_route( struct rip_route_item_ *rip_route_item, struct route_table *table  );
BOOL rip_is_neigh_addr(struct rip_process_info_ *pprocess,uint32 dest_addr );
int rip_version_check( uint32 device_index, uint32 src_addr );
int rip_auth_check( uint32 device_index, uint32 src_addr );
int rip_packet_check( uint32 device_index, struct soaddr_in *src_socket );
int rip_auth_seq_check(uint32 device_index, uint32 src_address, uint32 seq_number );
int rip_item_recv_check(uint32 device_index, uint32 src_addr, struct rip_route_item_ *rip_route_item, struct route_node *rip_route_node);
BOOL rip_is_connected_network( uint32 vrf_id, uint32 dest, uint32 mask );
struct rip_route_ *rip_get_relevant_route( uint32 device_index, uint32 src_addr, rip_route_t* rip_route );

/******************* END OF HEADER FILE *********************************/

#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* RIP_PACKET_H */
