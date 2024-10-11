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
 *        FILE NAME       :   rip_init.h
 *        AUTHOR          :   Fengsb
 *        DATE            :   2006.04.30 11:01:45
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#ifndef RIP_INIT_H
#define RIP_INIT_H

#ifdef __cplusplus
	extern "C" {
#endif /* __cplusplus */
/******************* BEGIN HEADER FILE **********************************/

#ifndef __RIP_INIT_H
#define __RIP_INIT_H

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

int rip_socket_init( void );
int rip_create_timer( void );
int rip_start_timer( void );
int rip_delete_timer( void );
int rip_process_init(uint32 processid , uint32 vrfid);
int rip_process_stop_timer( struct rip_process_info_* pprocess );
int rip_process_delete_timer( struct rip_process_info_ *pprocess );
void rip_clear_process_table( struct rip_process_info_ *pprocess );
void rip_clear_process_info( struct rip_process_info_ *pprocess );
void rip_create_table( uint32 vrf_id );
void rip_delete_table( uint32 vrf_id );
void rip_clear_table( void );
void rip_socket_redirect_enable(void);
void rip_socket_redirect_disable(void);


void RipTask( void );

#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif



/******************* END OF HEADER FILE *********************************/

#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* RIP_INIT_H */
