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
 *        FILE NAME       :   rip_show.h
 *        AUTHOR          :   HQ.Sun
 *        DATE            :   2006.01.08 11:27:56
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#ifndef RIP_SHOW_H
#define RIP_SHOW_H

#ifdef __cplusplus
	extern "C" {
#endif /* __cplusplus */
/******************* BEGIN HEADER FILE **********************************/
int rip_cmd_show_memory( int argc, char *argv[], struct user *u );
int rip_cmd_show_version(int argc, char **argv, struct user *u);

int rip_cmd_show_ip_rip( int argc, char *argv[], struct user *u );

int rip_cmd_show_data( int argc, char *argv[], struct user *u );
/*int rip_cmd_show_data_vrf( int argc, char *argv[], struct user *u );
int rip_cmd_show_data_vrf_name( int argc, char *argv[], struct user *u );
*/
int rip_cmd_show_proto( int argc, char *argv[], struct user *u );
/*int rip_cmd_show_proto_vrf( int argc, char *argv[], struct user *u );
int rip_cmd_show_proto_vrf_name( int argc, char *argv[], struct user *u );
*/
int rip_cmd_show_summary( int argc, char *argv[], struct user *u );
int rip_cmd_show_intf( int argc, char *argv[], struct user *u );
int rip_cmd_show_intf_vrf( int argc, char *argv[], struct user *u );
int rip_cmd_show_intf_vrf_name( int argc, char *argv[], struct user *u );

int rip_cmd_show_peer( int argc, char *argv[], struct user *u );
void rip_show_peer( struct user *u);

INT32 rip_show_running( DEVICE_ID device_index );
void rip_show_running_global(void);
void rip_show_running_intf( uint32 device_index );

void rip_show_rip( struct user *u );
/*void rip_show_protocol( struct user *u, struct rip_tbl_info_ *rip_tbl_info );
void rip_show_database( struct user *u, struct rip_tbl_info_ *rip_tbl_info );
void rip_show_intf( struct user *u, struct rip_tbl_info_ *rip_tbl_info );
*/
int rip_cmd_show_process(int argc ,char **argv ,struct user *u);
int rip_cmd_show_ip_rip_process_data( int argc, char *argv[], struct user *u );
int rip_cmd_show_ip_rip_process_proto( int argc, char *argv[], struct user *u );
int rip_cmd_show_ip_rip_process_summary( int argc, char *argv[], struct user *u );
int rip_cmd_show_ip_rip_process_intf( int argc, char *argv[], struct user *u );
int rip_cmd_show_ip_rip_process_peer( int argc, char *argv[], struct user *u );
void rip_show_ip_rip_process( struct user *u ,struct rip_process_info_ *pprocess);
void rip_show_ip_rip_process_database( struct user *u, struct route_table *rip_table );
void rip_show_ip_rip_process_protocol( struct user *u, struct rip_process_info_ *pprocess );
void rip_show_ip_rip_process_intf( struct user *u, struct rip_process_info_ *pprocess );
void rip_show_ip_rip_process_peer( struct user *u , struct rip_process_info_ *pprocess);

char *rip_print_time( long time );

/******************* END OF HEADER FILE *********************************/

#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* RIP_SHOW_H */
