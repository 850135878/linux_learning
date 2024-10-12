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
 *        FILE NAME       :   rip_redis.h
 *        AUTHOR          :   Fengsb
 *        DATE            :   2007.07.12 13:56:31
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#ifndef RIP_REDIS_H
#define RIP_REDIS_H

#ifdef __cplusplus
	extern "C" {
#endif /* __cplusplus */
/******************* BEGIN HEADER FILE **********************************/

int rip_route_change( struct _rt_head *rth );
void rip_redis_callback( uint32 proto_fg, uint32 process, uint32 type, void * detail);

int rip_redis_add( struct rip_process_info_ *pprocess, uint32 proto, uint32 process, struct route_map *map_name );
int rip_redis_del( struct rip_process_info_ *pprocess, uint32 proto, uint32 process, struct route_map *map_name );

int rip_create_redis_route( rt_entry *rt, struct rip_redis_list_ *rip_redis_list, struct rip_process_info_ *pprocess );
int rip_del_redis_connect( struct rip_route_ *rip_route_head );

void rip_create_redis_connect(uint32 device_index);
void rip_del_from_redis_list(struct rip_route_ *rip_route);

/******************* END OF HEADER FILE *********************************/

#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* RIP_REDIS_H */
