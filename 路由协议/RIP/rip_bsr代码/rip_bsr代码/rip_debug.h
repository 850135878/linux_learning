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
 *        FILE NAME       :   rip_debug.h
 *        AUTHOR          :   dangzhw
 *        DATE            :   2010.03.24 14:05:52
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#ifndef RIP_DEBUG_H
#define RIP_DEBUG_H

#ifdef __cplusplus
	extern "C" {
#endif /* __cplusplus */
/******************* BEGIN HEADER FILE **********************************/

#ifndef	__RIP_DEBUG_H
#define __RIP_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */
#ifdef MULTICORE
#include <syslog/syslog.h>

#define RIP_LOG_INFO 		LOG_STD_PRI2(MODULE_TYPE_RIP, LOG_LEV_INFO)
#define RIP_LOG_NOTICE 		LOG_STD_PRI2(MODULE_TYPE_RIP, LOG_LEV_NOTICE)
#define RIP_LOG_WARNING 	LOG_STD_PRI2(MODULE_TYPE_RIP, LOG_LEV_WARNING)
#define RIP_LOG_ERR 			LOG_STD_PRI2(MODULE_TYPE_RIP, LOG_LEV_ERR)
#define RIP_LOG_CRIT 		LOG_STD_PRI2(MODULE_TYPE_RIP, LOG_LEV_CRIT)
#define RIP_LOG_ALERT 		LOG_STD_PRI2(MODULE_TYPE_RIP, LOG_LEV_ALERT)
#define RIP_LOG_EMERG 		LOG_STD_PRI2(MODULE_TYPE_RIP, LOG_LEV_EMERG)
#else
#define RIP_LOG_INFO 		LOG_WARNING
#define RIP_LOG_NOTICE 		LOG_WARNING
#define RIP_LOG_WARNING 	LOG_WARNING
#define RIP_LOG_ERR 			LOG_WARNING
#define RIP_LOG_CRIT 		LOG_WARNING
#define RIP_LOG_ALERT 		LOG_WARNING
#define RIP_LOG_EMERG 		LOG_WARNING
#endif
void rip_debug( uint32 flag, const char * a, ...);

int rip_cmd_debug_rip( int argc, char *argv[], struct user *u );
int rip_cmd_debug_data( int argc, char *argv[], struct user *u );
int rip_cmd_debug_packet( int argc, char *argv[], struct user *u );
int rip_cmd_debug_packet_send( int argc, char *argv[], struct user *u );
int rip_cmd_debug_packet_recv( int argc, char *argv[], struct user *u );
int rip_cmd_debug_msg( int argc, char *argv[], struct user *u );
int rip_cmd_debug_return( int argc, char *argv[], struct user *u );

void rip_no_debug_all( void );
void rip_show_debug( uint32 debug_flag );

#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif



/******************* END OF HEADER FILE *********************************/

#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* RIP_DEBUG_H */
