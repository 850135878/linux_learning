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
 *        FILE NAME       :   rip_include.h
 *        AUTHOR          :   Fengsb
 *        DATE            :   2006.04.30 15:00:23
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#ifndef RIP_INCLUDE_H
#define RIP_INCLUDE_H

#ifdef __cplusplus
	extern "C" {
#endif /* __cplusplus */
/******************* BEGIN HEADER FILE **********************************/

/* the header file out of the rip module */
#include <global.h>
#include <libdev/modules.h>
#include <libdev/interface.h>
#ifndef OS_VXWORKS
#include <psos.h>
#include <libcmd/cmdiface.h>
#else
#include <libdev/interface.h>
#endif
#include <libcmd/cmdparse.h>
#include <libcmd/argparse.h>
#include <libcmd/cmderror.h>
#include <stdarg.h>

#include <libgpr/offset.h>
#include <libgpr/filter.h>
#include <libgpr/distance.h>
#include <rtlib/rtlib_table.h>
#include <libgpr/routemap.h>

#include <routing/include.h>

#include <ipacl/Ipacl.h>
#include <ip/socket.h>

#include <time.h>
#include <libsys/timer.h> 
#include <userm/Aaa.h>
/******************* END OF HEADER FILE *********************************/

#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* RIP_INCLUDE_H */
