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
 *        FILE NAME       :   rip_mib.h
 *        AUTHOR          :   LiYongHui
 *        DATE            :   2008.12.06 11:02:35
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#ifndef RIP_MIB_H
#define RIP_MIB_H

#ifdef __cplusplus
	extern "C" {
#endif /* __cplusplus */
/******************* BEGIN HEADER FILE **********************************/

#ifndef __RIP_MIB_H
#define __RIP_MIB_H

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

#define RIP_INDEX_OID_LEN 14
#define RIP2_PEER_DOMAIN_NAME_LEN 2
#define destroy 6
#define Active			1

int rip_register_mib( void );


/*rip mib提供给snmp的init接口函数*/
void rip_init_mib( void );

/*处理来自snmp的mib get/getnext/set请求*/
u_char * rip_mib_global(struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int));
u_char *rip_mib_intf_stat( struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int) );
u_char * rip_mib_intf_conf(struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int));
u_char *rip_mib_peer(struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int));
int rip_mib_write_intf_stat( int action, u_char *var_val, u_char var_val_type, int var_val_len, u_char *statP, oid *name, int name_len );
int rip_mib_write_intf_conf(int action, u_char *var_val, u_char var_val_type, int var_val_len, u_char *statP, oid *name, int name_len);
static int header_rip(struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int));
static struct rip_intf_ * header_rip_index(struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int));
static struct rip_peer_list_  *header_rip_peer(struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int));



#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif



/******************* END OF HEADER FILE *********************************/

#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* RIP_MIB_H */
