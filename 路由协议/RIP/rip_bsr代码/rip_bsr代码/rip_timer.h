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
 *        FILE NAME       :   rip_timer.h
 *        AUTHOR          :   Fengsb
 *        DATE            :   2007.07.12 13:50:10
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#ifndef RIP_TIMER_H
#define RIP_TIMER_H

#ifdef __cplusplus
	extern "C" {
#endif /* __cplusplus */
/******************* BEGIN HEADER FILE **********************************/
int rip_update_timeout( uint32 processid );
int rip_expire_timeout( uint32 processid );
int rip_holddown_timeout( uint32 processid );
int rip_trigger_timeout( uint32 processid );

void rip_add_holddown( struct rip_route_ *rip_route );
void rip_add_trigger( struct rip_route_ *rip_route, struct rip_process_info_ *pprocess, BOOL b_DelFrmRipTbl );
void rip_del_holddown( struct rip_route_ *rip_route );
void rip_clear_trigger_list( struct rip_process_info_ *pprocess );
void rip_compress_trigger_list( struct rip_process_info_ *pprocess );

/******************* END OF HEADER FILE *********************************/

#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* RIP_TIMER_H */
