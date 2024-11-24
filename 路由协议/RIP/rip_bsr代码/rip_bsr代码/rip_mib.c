char rip_mib_version[] = "rip_mib.c last changed at 2010.06.30 15:20:23 by dangzhw\n";
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
 *        FILE NAME       :   rip_mib.c
 *        AUTHOR          :   Fengsb
 *        DATE            :   2006.05.08 13:33:41
 *        PURPOSE:        :   xxxx
 *
**************************************************************************/
#include <Global.h>

#ifndef OS_VXWORKS
#include <psos.h>
#endif

#include <snmp/snmplib/asn1.h>

#include <snmp/snmpreg.h>
#include <snmp/snmp_vars.h>
#include <snmplib/snmp_api.h>
#include <snmp/var_struct.h>
#include <snmplib/snmp_impl.h>
#include <snmplib/snmp_debug.h>
#include <snmplib/snmp.h>
#include <snmp/agent_registry.h>

#include <routing/include.h>
#include <rip/rip_main.h>
#include <rip/rip_mib.h>


extern struct rip_glb_info_  rip_glb_info;
extern struct rip_intf_ *rip_intf_array[INTERFACE_DEVICE_MAX_NUMBER + 1];
extern struct rip_process_list_ rip_process_list;


/*rip2GlobalInfo*/
#define RIP2GLOBALROUTECHANGES	                1
#define RIP2GLOBALQUERIES		                2

/*rip2IfStatInfo*/
#define RIP2IFSTATADDRESS		                1
#define RIP2IFSTATRCVBADPKTS	                2
#define RIP2IFSTATRCVBADROUTES	                3
#define RIP2IFSTATSENTUPDATES	                4
#define RIP2IFSTATSTATUS		                5

/*rip2IfConfInfo*/
#define RIP2IFCONFADDRESS		                1
#define RIP2IFCONFDOMAIN		                2
#define RIP2IFCONFAUTHTYPE		                3
#define RIP2IFCONFAUTHKEY		                4
#define RIP2IFCONFSEND			                5
#define RIP2IFCONFRECEIVE		                6
#define RIP2IFCONFDEFAULTMETRIC	                7
#define RIP2IFCONFSTATUS		                8
#define RIP2IFCONFSRCADDRESS	                9

/*rip2PeerInfo*/
#define RIP2PEERADDRESS			                1
#define RIP2PEERDOMAIN			                2
#define RIP2PEERLASTUPDATE		                3
#define RIP2PEERVERSION			                4
#define RIP2PEERRCVBADPKTS		                5
#define RIP2PEERRCVBADROUTES	                6

struct variable13 rip2_variables[] =
{
	{RIP2GLOBALROUTECHANGES,ASN_COUNTER,RONLY,rip_mib_global,2,{1, 1}},
	{RIP2GLOBALQUERIES, ASN_COUNTER, RONLY, rip_mib_global, 2, {1, 2}},

	{RIP2IFSTATADDRESS, ASN_IPADDRESS, RONLY, rip_mib_intf_stat, 3, {2, 1, 1}},
	{RIP2IFSTATRCVBADPKTS, ASN_COUNTER, RONLY, rip_mib_intf_stat, 3, {2, 1, 2}},
	{RIP2IFSTATRCVBADROUTES, ASN_COUNTER, RONLY, rip_mib_intf_stat, 3, {2, 1, 3}},
	{RIP2IFSTATSENTUPDATES, ASN_COUNTER, RONLY, rip_mib_intf_stat, 3, {2, 1, 4}},
	{RIP2IFSTATSTATUS, ASN_INTEGER, RWRITE, rip_mib_intf_stat, 3, {2, 1, 5}},

	{RIP2IFCONFADDRESS, ASN_IPADDRESS, RONLY, rip_mib_intf_conf, 3, {3, 1, 1}},
	{RIP2IFCONFDOMAIN, ASN_OCTET_STR, RWRITE, rip_mib_intf_conf, 3, {3, 1, 2}},
	{RIP2IFCONFAUTHTYPE, ASN_INTEGER, RWRITE, rip_mib_intf_conf, 3, {3, 1, 3}},
	{RIP2IFCONFAUTHKEY, ASN_OCTET_STR, RWRITE, rip_mib_intf_conf, 3, {3, 1, 4}},
	{RIP2IFCONFSEND, ASN_INTEGER, RWRITE, rip_mib_intf_conf, 3, {3, 1, 5}},
	{RIP2IFCONFRECEIVE, ASN_INTEGER, RWRITE, rip_mib_intf_conf, 3, {3, 1, 6}},
	{RIP2IFCONFDEFAULTMETRIC,ASN_INTEGER, RWRITE, rip_mib_intf_conf, 3, {3, 1, 7}},
	{RIP2IFCONFSTATUS, ASN_INTEGER, RWRITE, rip_mib_intf_conf, 3, {3, 1, 8}},
	{RIP2IFCONFSRCADDRESS, ASN_IPADDRESS, RWRITE, rip_mib_intf_conf, 3, {3, 1, 9}},

	{RIP2PEERADDRESS, ASN_IPADDRESS, RONLY, rip_mib_peer, 3, {4, 1, 1}},
	{RIP2PEERDOMAIN, ASN_OCTET_STR, RONLY, rip_mib_peer, 3, {4, 1, 2}},
	{RIP2PEERLASTUPDATE, ASN_TIMETICKS, RONLY, rip_mib_peer, 3, {4, 1, 3}},
	{RIP2PEERVERSION, ASN_INTEGER, RONLY, rip_mib_peer, 3, {4, 1, 4}},
	{RIP2PEERRCVBADPKTS, ASN_COUNTER, RONLY, rip_mib_peer, 3, {4, 1, 5}},
	{RIP2PEERRCVBADROUTES, ASN_COUNTER, RONLY, rip_mib_peer, 3, {4, 1, 6}}
};

oid rip_variables_oid[] = { 1,3,6,1,2,1,23 };


/*===========================================================
函数名:   rip_register_mib
函数功能: 注册MIB函数
输入参数: 无
输出参数: 无
返回值:   创建成功,返回RIP_SUCCESS
       创建失败,返回RIP_FAIL
备注:
=============================================================*/
int rip_register_mib( void )
{
	int ret;

	ret = register_snmp( 23, rip_init_mib );
	if( ret < 0 )
	{
		return RIP_FAIL;
	}
	
	return RIP_SUCCESS;
}

/*===========================================================
函数名: rip_init_mib
函数功能: register ourselves with the agent to handle our mib tree
输入参数: 无
输出参数: 无
返回值:      无
备注:
===========================================================*/
void rip_init_mib(void)
{
	REGISTER_MIB( "mibII/rip", rip2_variables, variable13, rip_variables_oid );
	return;
}

/*===========================================================
函数名: rip_mib_global
函数功能: MIB全局信息
输入参数: 
输出参数: 
返回值:      
备注:
===========================================================*/
u_char *rip_mib_global(struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int))
{
	static uint32 long_return;
	struct rip_process_info_ *pprocess;
	struct rip_process_list_ *pprocess_list;

	long_return = 0;
	
	if( MATCH_FAILED == header_rip(vp, name, length, exact, var_len, write_method) )
	{
		return NULL;
	}

	if((pprocess_list = rip_process_list.forw) == &rip_process_list)
		return NULL;
	pprocess = pprocess_list->rip_process_info;
	
	switch (vp->magic) 
	{
	case RIP2GLOBALROUTECHANGES:
		long_return = pprocess->rip2GlobalRouteChanges;
		return (u_char *)&long_return;
		break;
	case RIP2GLOBALQUERIES:
		long_return = pprocess->rip2GlobalQueries;
		return (u_char *)&long_return;
		break;
	default:
		assert(0);
		break;
	}
	
	return NULL;
}

/*===========================================================
函数名: rip_mib_intf_stat
函数功能: MIB端口信息
输入参数: 
输出参数: 
返回值:      
备注:
===========================================================*/
u_char *rip_mib_intf_stat( struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int) )
{
	uint32 device_index;
	static uint32 long_return;
	struct rip_intf_ *rip_intf;

	rip_intf = header_rip_index( vp, name, length, exact, var_len, write_method );
	if( rip_intf == NULL)
	{
		return NULL;
	}

	device_index = rip_intf->device_index;

	switch (vp->magic) 
	{
		case RIP2IFSTATADDRESS:
			if( rip_intf_array[device_index]->addr_type != RIP_UNNUMBERED_ADDRESS )
			{
				if(rip_intf_array[device_index]->address != 0 )
				{
					long_return = rip_intf_array[device_index]->address;
				}
				else
				{
					long_return = 0;
				}				
			}
			else
			{
				long_return = device_index;
			}
			return (u_char *)&long_return;
			break;
		case RIP2IFSTATRCVBADPKTS:
			long_return = rip_intf_array[device_index]->rip2IfStatRcvBadPackets;
			return (u_char *)&long_return;
			break;
		case RIP2IFSTATRCVBADROUTES:
			long_return = rip_intf_array[device_index]->rip2IfStatRcvBadRoutes;
			return (u_char *)&long_return;
			break;
		case RIP2IFSTATSENTUPDATES:
			long_return = rip_intf_array[device_index]->rip2IfStatSentUpdates;
			return (u_char *)&long_return;
			break;
		case RIP2IFSTATSTATUS:
			if( BIT_TEST( rip_intf_array[device_index]->state ,RIP_INTF_LINK_UP))
				long_return = Active;
			else
				long_return = 2;
			*write_method = rip_mib_write_intf_stat;
			return (u_char *)&long_return;
			break;
		default:
			assert(0);
			break;
	}
	
	return NULL;
}

/*===========================================================
函数名: rip_mib_intf_conf
函数功能: MIB端口配置信息
输入参数: 
输出参数: 
返回值:      
备注:
===========================================================*/
u_char * rip_mib_intf_conf(struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int))
{
	static char Name[RIP2_PEER_DOMAIN_NAME_LEN];
	static char key[16];
	char *cp;
	uint32 device_index;
	static uint32 long_return;
	struct rip_intf_ *rip_interface;
	struct rip_process_info_ *pprocess;

	rip_interface = header_rip_index( vp, name, length, exact, var_len, write_method );
	if( rip_interface == NULL)
	{
		return NULL;
	}

	device_index = rip_interface->device_index;
	pprocess = rip_intf_array[device_index]->pprocess;
	if(!(pprocess))
		return NULL;

	switch (vp->magic) 
	{
		case RIP2IFCONFADDRESS:
			if( rip_intf_array[device_index]->addr_type != RIP_UNNUMBERED_ADDRESS )
			{
				if(rip_intf_array[device_index]->address != 0 )
				{
					long_return = rip_intf_array[device_index]->address;
				}
				else
				{
					long_return = 0;
				}				
			}
			else
			{
				long_return = device_index;
			}
			return (u_char *)&long_return;
			break;
		case RIP2IFCONFDOMAIN:
			memset( (void *) Name, 0, RIP2_PEER_DOMAIN_NAME_LEN );
			cp =(char *) &Name;
			*var_len = RIP2_PEER_DOMAIN_NAME_LEN;
			*write_method = rip_mib_write_intf_conf;
			return (u_char *)cp;
			break;
		case RIP2IFCONFAUTHTYPE:
			if( RIP_AUTH_SIMPLE == rip_intf_array[device_index]->auth_type )
			{
				long_return = 2;
			}
			else if( RIP_AUTH_MD5 == rip_intf_array[device_index]->auth_type )
			{
				long_return = 3;
			}
			else
			{
				long_return = 1;
			}
			*write_method = rip_mib_write_intf_conf;
			return (u_char *)&long_return;
			break;
		case RIP2IFCONFAUTHKEY:

			/* 在mib浏览的时候，key应该统一显示为全0的字符串
			 * Commented by dangzhw in 2009.11.10 10:42:38 */
			/*if( (rip_intf_array[device_index]->auth_type == RIP_AUTH_SIMPLE)
				&& (strlen(rip_intf_array[device_index]->simple_key) != 0))
			{
				cp = rip_intf_array[device_index]->simple_key;
			}
			else if( (rip_intf_array[device_index]->auth_type == RIP_AUTH_MD5)
				&& (strlen(rip_intf_array[device_index]->md5_key) != 0))
			{
				cp = rip_intf_array[device_index]->md5_key;
			}
			else
			{*/
				memset((void *)key, 0, 16 );
				cp = (char *) &key;
			/*}*/
				
			*var_len = 0;
			*write_method = rip_mib_write_intf_conf;
			return (u_char *)cp;
			break;
		case RIP2IFCONFSEND:		

			/* 只有在intf上没有配置version的时候才能显示v12demand
			 * Commented by dangzhw in 2009.11.11 14:56:35 */
			if(BIT_TEST( rip_intf_array[device_index]->special_flag,RIP_PASSIVE_ENABLE))
			{
				long_return = 1;
			}
			else if( (RIP_SEND_VERSION_1 == rip_intf_array[device_index]->send_version)
				/*|| (RIP_SEND_VERSION_DEF == rip_intf_array[device_index]->send_version)*/)
			{
				long_return = 2;
			}
			else if( RIP_SEND_VERSION_2_MULTICAST == rip_intf_array[device_index]->send_version )
			{
				long_return = 4;
			}
			else if( RIP_SEND_VERSION_2_BROADCAST == rip_intf_array[device_index]->send_version )
			{
				long_return = 3;
			}
			else if(BIT_TEST( rip_intf_array[device_index]->special_flag,RIP_SEND_V1DEMAND))
			{
				long_return = 5;
			}
			else if(BIT_TEST( rip_intf_array[device_index]->special_flag,RIP_SEND_V2DEMAND))
			{
				long_return = 6;
			}
			else if(RIP_SEND_VERSION_DEF == rip_intf_array[device_index]->send_version) 
			{
				if((RIP_VERSION_1 == pprocess->version) )
				{
					long_return = 2;
				}
				else if( RIP_VERSION_2 == pprocess->version )
				{
					long_return = 4;
				}
	 			else 
				{
					long_return = 4;
				}
			}
			else
			{
				long_return = 4;
			}
			*write_method = rip_mib_write_intf_conf;
			return (u_char *)&long_return;
			break;
		case RIP2IFCONFRECEIVE:

			/* 如果配置了deaf，则别的版本号不生效
			 * Commented by dangzhw in 2009.11.11 17:31:43 */
			if( BIT_TEST(rip_intf_array[device_index]->special_flag,RIP_NOTRECV_ENABLE))
			{
				long_return = 4;
			}
			else if( (RIP_RECV_VERSION_1_2 == rip_intf_array[device_index]->recv_version)
				/*|| (RIP_RECV_VERSION_DEF == rip_intf_array[device_index]->recv_version)*/ )
			{
				long_return = 3;
			}
			else if( RIP_RECV_VERSION_1 == rip_intf_array[device_index]->recv_version )
			{
				long_return = 1;
			}
			else if( RIP_RECV_VERSION_2 == rip_intf_array[device_index]->recv_version )
			{
				long_return = 2;
			}
			else
			{
				long_return = 3;
			}
			*write_method = rip_mib_write_intf_conf;
			return (u_char *)&long_return;
			break;
		case RIP2IFCONFSTATUS:
			if( BIT_TEST( rip_intf_array[device_index]->state ,RIP_INTF_LINK_UP))
				long_return = Active;
			else
				long_return = 2;
			*write_method = rip_mib_write_intf_conf;
			return (u_char *)&long_return;
			break;
		case RIP2IFCONFSRCADDRESS:
			if(rip_intf_array[device_index]->address != 0 )
			{
				long_return = rip_intf_array[device_index]->address;
			}
			else
			{
				long_return = 0;
			}				
			*write_method = rip_mib_write_intf_conf;
			return (u_char *)&long_return;
			break;
		case RIP2IFCONFDEFAULTMETRIC:
			long_return = pprocess->default_metric;
			*write_method = rip_mib_write_intf_conf;
			return (u_char *)&long_return;
			break;
		default:
			assert(0);
			break;
	}
	
	return NULL;
}


/*===========================================================
函数名: rip_mib_peer
函数功能: peer table "A list of RIP peers
输入参数: 
输出参数: 
返回值:      
备注:
===========================================================*/
u_char *rip_mib_peer(struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int))
{
	static char Name[RIP2_PEER_DOMAIN_NAME_LEN];
	char *cp;
	static uint32 long_return;
	struct rip_peer_list_ *peer;
	
	peer = header_rip_peer(vp, name, length, exact, var_len, write_method);
	if( peer == 0 )
	{
		return NULL;
	}

	switch (vp->magic)
	{
		case RIP2PEERADDRESS:
			long_return = peer->peer_addr;
			return (u_char *)&long_return;
			break;
		case RIP2PEERDOMAIN:
			memset((void *) Name, 0, RIP2_PEER_DOMAIN_NAME_LEN);
			cp =(char *) &Name;
			*var_len = RIP2_PEER_DOMAIN_NAME_LEN;
			if(peer->peer_domain != 0)
			{
				sprintf(Name,"%d",peer->peer_domain);
			}
			return (u_char *)cp;
			break;
		case RIP2PEERLASTUPDATE:
			long_return = peer->rip2PeerLastUpdate;
			return (u_char *)&long_return;
			break;
		case RIP2PEERVERSION:
			long_return = peer->rip2PeerVersion;
			return (u_char *)&long_return;
			break;
		case RIP2PEERRCVBADPKTS:
			long_return = peer->rip2PeerRcvBadPackets;
			return (u_char *)&long_return;
			break;
		case RIP2PEERRCVBADROUTES:
			long_return = peer->rip2PeerRcvBadRoutes;
			return (u_char *)&long_return;
			break;
		default:
			assert(0);
			break;
	}
	
	return NULL;
}

/*===========================================================
函数名: rip_mib_write_intf_stat
函数功能: write interface status entry
输入参数: 
输出参数: 
返回值:      
备注:
===========================================================*/
int rip_mib_write_intf_stat( int action, u_char *var_val, u_char var_val_type, int var_val_len, u_char *statP, oid *name, int name_len )
{
	int bigsize = 4;
	long intval;
	u_char *cp;
	oid *op;
	static oid Current[RIP_INDEX_OID_LEN];
	uint32 intf_ip_address;
	struct rip_intf_ *rip_interface , *pintf;
	struct rip_process_list_ *pprocess_list;
	struct rip_process_info_ *pprocess;
	struct rip_process_intf_list_ *pintf_list;

	rip_interface = NULL;
	
	if (var_val_type != ASN_INTEGER	)
	{
		ERROR_MSG("not integer");
		return SNMP_ERR_WRONGTYPE;
	}

	if(name_len<11)
	{
		ERROR_MSG("name too short");
		return SNMP_ERR_WRONGTYPE;
	}
	
	memcpy( (char *)Current,(char *)name, (int)name_len * sizeof(oid));

	if((pprocess_list = rip_process_list.forw ) == &rip_process_list)
		return SNMP_ERR_NOSUCHNAME;

	pprocess = pprocess_list->rip_process_info;

	for( pintf_list  = pprocess->intf_list.forw ; pintf_list != &(pprocess->intf_list); pintf_list = pintf_list->forw )
	{
		if( ((pintf = pintf_list->pintf) == NULL)
			|| (pintf->address == 0)
			|| (pintf->process_id== 0))
		{
			continue;
		}

		intf_ip_address = pintf->address;

		op = Current + 10;
		cp = (u_char *)&intf_ip_address;
		*op++ = *cp++;
		*op++ = *cp++;
		*op++ = *cp++;
		*op++ = *cp++;
		
		if(snmp_oid_compare(name, name_len, Current, name_len) == 0)
		{
			rip_interface = pintf;
			break;	/* no need to search further */
		}
	}

	if( rip_interface == NULL )
	{
		return SNMP_ERR_NOSUCHNAME;
	}

	asn_parse_int(var_val, (size_t *) &bigsize, &var_val_type, &intval, sizeof (intval));

	if (intval < Active || intval > destroy )
	{
		ERROR_MSG(("mibII/rip, set status not valid\n"));
		return SNMP_ERR_WRONGVALUE;
	}

	if (action == COMMIT)
	{
		if (intval == Active)
		{
		}
		else
		{
		}
		ERROR_MSG(("mibII/rip, can not set status\n"));
		return SNMP_ERR_NOTWRITABLE;
	}
	
	return SNMP_ERR_NOERROR;
}

/*===========================================================
函数名: rip_mib_write_intf_conf
函数功能: write interface configuration entry
输入参数: 
输出参数: 
返回值:      
备注:
===========================================================*/
int rip_mib_write_intf_conf(int action, u_char *var_val, u_char var_val_type, int var_val_len, u_char *statP, oid *name, int name_len)
{
	long intval;
	char magic = (char)name[9];
	u_char *cp;
	oid *op;
	static oid Current[RIP_INDEX_OID_LEN];
	uint32 intf_ip_address;
	struct rip_intf_ *rip_interface ,*pintf;
	struct rip_process_list_ *pprocess_list;
	struct rip_process_info_ *pprocess;
	struct rip_process_intf_list_ *pintf_list;
	
	rip_interface = NULL;

	memcpy( (char *)Current,(char *)name, (int)name_len * sizeof(oid));
	
	if((pprocess_list = rip_process_list.forw ) == &rip_process_list)
		return SNMP_ERR_NOSUCHNAME;

	pprocess = pprocess_list->rip_process_info;

	for( pintf_list  = pprocess->intf_list.forw ; pintf_list != &(pprocess->intf_list); pintf_list = pintf_list->forw )
	{
		if( ((pintf = pintf_list->pintf) == NULL)
			|| (pintf->address == 0)
			|| (pintf->process_id== 0))
		{
			continue;
		}

		intf_ip_address = pintf->address;

		op = Current + 10;
		cp = (u_char *)&intf_ip_address;
		*op++ = *cp++;
		*op++ = *cp++;
		*op++ = *cp++;
		*op++ = *cp++;
		
		if(snmp_oid_compare(name, name_len, Current, name_len) == 0)
		{
			rip_interface = pintf;
			break;/* no need to search further */
		}
	}
	if((!rip_interface) || !(pprocess = rip_interface->pprocess))
	{
		return SNMP_ERR_NOSUCHNAME;
	}

	switch ((char) name[9])
	{
		case RIP2IFCONFDOMAIN:
			/* obsolete */
			break;
		case RIP2IFCONFAUTHTYPE:
		case RIP2IFCONFAUTHKEY:
			if (var_val_len > 16)
			{
				ERROR_MSG("bad length for RIP auth key\n");
				return SNMP_ERR_WRONGLENGTH;
			}
			break;
		case RIP2IFCONFSEND:
			if (var_val_type != ASN_INTEGER)
			{
				ERROR_MSG("not integer");
				return SNMP_ERR_WRONGTYPE;
			}
			if(name_len<11)
			{
				ERROR_MSG("name too short");
				return SNMP_ERR_WRONGTYPE;
			}
			break;
		case RIP2IFCONFRECEIVE:
			if (var_val_type != ASN_INTEGER)
			{
				ERROR_MSG("not integer");
				return SNMP_ERR_WRONGTYPE;
			}
			if(name_len<11)
			{
				ERROR_MSG("name too short");
				return SNMP_ERR_WRONGTYPE;
			}
			break;
		case RIP2IFCONFDEFAULTMETRIC:
			if (var_val_type != ASN_INTEGER)
			{
				ERROR_MSG("not integer");
				return SNMP_ERR_WRONGTYPE;
			}
			if(name_len<11)
			{
				ERROR_MSG("name too short");
				return SNMP_ERR_WRONGTYPE;
			}
			break;
		case RIP2IFCONFSTATUS:
			if (var_val_type != ASN_INTEGER)
			{
				ERROR_MSG("not integer");
				return SNMP_ERR_WRONGTYPE;
			}
			if(name_len<11)
			{
				ERROR_MSG("name too short");
				return SNMP_ERR_WRONGTYPE;
			}
			break;
		case RIP2IFCONFSRCADDRESS:
			break;
		default:
			return SNMP_ERR_READONLY;
			break;
	}
	intval = *(int *)var_val;
	cp = var_val;
	if (action == COMMIT)
	{
		switch (magic)
		{
			case RIP2IFCONFDOMAIN:/* obsolete */
				return SNMP_ERR_NOTWRITABLE;
				break;
			case RIP2IFCONFAUTHTYPE:
				switch(intval)
				{
					case 1:
						rip_interface->auth_type = RIP_AUTH_NONE;
						rip_interface->dynamicflag = 0; /* ygc add */
						break;
					case 2:
						rip_interface->auth_type = RIP_AUTH_SIMPLE;
						rip_interface->dynamicflag = 0; /* ygc add */
						break;
					case 3:
						rip_interface->auth_type = RIP_AUTH_MD5;
						rip_interface->dynamicflag = 0; /* ygc add */
						break;
					default:
						return SNMP_ERR_BADVALUE;
				}
				break;
			case RIP2IFCONFAUTHKEY:
				if(rip_interface->auth_type == RIP_AUTH_SIMPLE)
				{
					memset(&rip_interface->simple_key, 0, 17);
					memcpy(&rip_interface->simple_key, cp, var_val_len);
				}
				else if (rip_interface->auth_type == RIP_AUTH_MD5)
				{
					memset(&rip_interface->md5_key, 0, 17);
					memcpy(&rip_interface->md5_key, cp, var_val_len);
				}
				break;
			case RIP2IFCONFSEND:
				switch(intval)
				{
					case 1:
						BIT_SET(rip_interface->special_flag, RIP_PASSIVE_ENABLE);
						break;
					case 2:
						rip_interface->send_version = RIP_SEND_VERSION_1;
						break;
					case 3:
						rip_interface->send_version = RIP_SEND_VERSION_2_BROADCAST;
						break;
					case 4:
						rip_interface->send_version = RIP_SEND_VERSION_2_MULTICAST;
						break;
					case 5:
						if(BIT_TEST(rip_interface->special_flag, RIP_SEND_V2DEMAND))
							BIT_RESET(rip_interface->special_flag, RIP_SEND_V2DEMAND);
						BIT_SET(rip_interface->special_flag, RIP_SEND_V1DEMAND);
						break;
					case 6:
						if(BIT_TEST(rip_interface->special_flag, RIP_SEND_V1DEMAND))
							BIT_RESET(rip_interface->special_flag, RIP_SEND_V1DEMAND);
						BIT_SET(rip_interface->special_flag, RIP_SEND_V2DEMAND);
						break;
					default:
						return SNMP_ERR_BADVALUE;
						break;
				}
				break;
			case RIP2IFCONFRECEIVE:
				switch(intval)
				{

					/* 在通过mib修改接收版本号的时候，若是设置成1或2或1、2的时候
					需要将notrecv标志去掉，否则设置了版本号也将不能生效，亦即通过mib
					不能够去除notrecv标志，在通过console口配置的时候不需要，那里可以通过命令
					实现no 掉notrecv标志(no ip rip deaf)
					 * Commented by dangzhw in 2009.11.11 17:04:40 */
					case 1:
						if(BIT_TEST(rip_interface->special_flag,RIP_NOTRECV_ENABLE))
							BIT_RESET(rip_interface->special_flag,RIP_NOTRECV_ENABLE);
						rip_interface->recv_version = RIP_RECV_VERSION_1;
						break;
					case 2:
						if(BIT_TEST(rip_interface->special_flag,RIP_NOTRECV_ENABLE))
							BIT_RESET(rip_interface->special_flag,RIP_NOTRECV_ENABLE);
						rip_interface->recv_version = RIP_RECV_VERSION_2;
						break;
					case 3:
						if(BIT_TEST(rip_interface->special_flag,RIP_NOTRECV_ENABLE))
							BIT_RESET(rip_interface->special_flag,RIP_NOTRECV_ENABLE);
						rip_interface->recv_version = RIP_RECV_VERSION_1_2;
						break;
					case 4:
						BIT_SET( rip_interface->special_flag,RIP_NOTRECV_ENABLE);
						break;
					default:
						return SNMP_ERR_BADVALUE;
			}
			break;
		case RIP2IFCONFDEFAULTMETRIC:
			pprocess->default_metric = intval;
			break;
		case RIP2IFCONFSTATUS:
			return SNMP_ERR_NOTWRITABLE;
		case RIP2IFCONFSRCADDRESS:
			rip_interface->address = intval;
			break;
		default:
			assert(0);
			break;
		}
	}
	return SNMP_ERR_NOERROR;
}

/*===========================================================
函数名: header_rip
函数功能: found and return rip information
输入参数: 
输出参数: 
返回值:      
备注:
===========================================================*/
static int header_rip(struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int))
{
    oid newname[MAX_OID_LEN];
    int result;
    
    memcpy( (char *)newname,(char *)vp->name, (int)vp->namelen * sizeof(oid));
    newname[9] = 0;
    
    result = snmp_oid_compare(name, *length, newname, (int)vp->namelen + 1);
    if ((exact && (result != 0)) || (!exact && (result >= 0)))
    {
        return(MATCH_FAILED);
    }
    
    memcpy( (char *)name,(char *)newname, ((int)vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;

    *write_method = 0;
    *var_len = sizeof(long);/* default to 'long' results */
    
    return(MATCH_SUCCEEDED);
}

/*===========================================================
函数名: header_rip_index
函数功能: found and return the interface device index
输入参数: 
输出参数: 
返回值:      
备注:
===========================================================*/
static struct rip_intf_ * header_rip_index(struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int))
{
	int found = FALSE;
	u_char *cp;
	oid *op;
	oid lowest[RIP_INDEX_OID_LEN];
	uint32 intf_ip_address;
	struct rip_intf_ *rip_interface , *pintf;
	static oid Current[RIP_INDEX_OID_LEN];
	struct rip_process_list_ *pprocess_list;
	struct rip_process_info_ *pprocess;
	struct rip_process_intf_list_ *pintf_list;
	
	rip_interface = NULL;

	memcpy( (char *)Current,(char *)vp->name, (int)vp->namelen * sizeof(oid));

	if((pprocess_list = rip_process_list.forw ) == &rip_process_list)
		return 0;

	pprocess = pprocess_list->rip_process_info;

	for( pintf_list  = pprocess->intf_list.forw ; pintf_list != &(pprocess->intf_list); pintf_list = pintf_list->forw )
	{
		if( ((pintf = pintf_list->pintf) == NULL)
			|| (pintf->address == 0)
			|| (pintf->process_id== 0))
		{
			continue;
		}

		intf_ip_address = pintf->address;

		op = Current + 10;
		cp = (u_char *)&intf_ip_address;
		*op++ = *cp++;
		*op++ = *cp++;
		*op++ = *cp++;
		*op++ = *cp++;
		
		if (exact)
		{
			if (snmp_oid_compare(name, *length, Current, RIP_INDEX_OID_LEN) == 0)
			{
				memcpy( (char *)lowest,(char *)Current, RIP_INDEX_OID_LEN * sizeof(oid));
				rip_interface = pintf;
				found = TRUE;
				break;	/* no need to search further */
			}
		}
		else
		{
	    	if ((snmp_oid_compare(Current, RIP_INDEX_OID_LEN, name, *length) > 0)
	    		&& (!rip_interface || (snmp_oid_compare(Current, RIP_INDEX_OID_LEN, lowest, RIP_INDEX_OID_LEN) < 0)))
			{
				rip_interface = pintf;
				memcpy( (char *)lowest,(char *)Current, RIP_INDEX_OID_LEN * sizeof(oid));
			}
		}
	}

    if (!rip_interface)
    {
        return 0;
    }
        
    memcpy( (char *)name,(char *)lowest, RIP_INDEX_OID_LEN * sizeof(oid));
    *length = RIP_INDEX_OID_LEN;

    *write_method = 0;
    *var_len = sizeof(long);	/* default to 'long' results */
    
    return rip_interface;
}

/*===========================================================
函数名: header_rip_peer
函数功能: found and return the peer
输入参数: 
输出参数: 
返回值:      
备注:
===========================================================*/
static struct rip_peer_list_ *header_rip_peer(struct variable *vp, oid *name, int *length, int exact, int *var_len, int (**write_method) (int, u_char *, u_char, int, u_char *, oid *, int))
{
	u_char *cp;
	oid *op;
	oid lowest[RIP_INDEX_OID_LEN];
	oid Current[RIP_INDEX_OID_LEN];
	BOOL break_true;
	struct rip_peer_list_ *rip_peer;
	struct rip_peer_list_ *peer_header;
	struct rip_process_list_ *pprocess_list;
	struct rip_process_info_ *pprocess;
	
	peer_header = NULL;
	break_true = FALSE;

	memcpy( (char *)Current,(char *)vp->name, (int)vp->namelen * sizeof(oid));

	if((pprocess_list = rip_process_list.forw ) == &rip_process_list)
		return NULL;

	pprocess = pprocess_list->rip_process_info;

	rip_peer = pprocess->peer_list.forw;

	/* 解决walk的时候peer不能显示的bug，原来在exact为0
	的时候，将不能显示peer
	 * Commented by dangzhw in 2009.10.13 17:43:24 */
	if(rip_peer == & pprocess->peer_list)/**/
		return NULL;
	while(rip_peer != &(pprocess->peer_list))
	{
		op = Current + 10;
		cp = (u_char *)&(rip_peer->peer_addr);
		*op++ = *cp++;
		*op++ = *cp++;
		*op++ = *cp++;
		*op++ = *cp++;
		if (exact)
		{
			if (snmp_oid_compare(name, *length, Current, RIP_INDEX_OID_LEN) == 0)
			{
				memcpy( (char *)lowest,(char *)Current, RIP_INDEX_OID_LEN * sizeof(oid));
				peer_header = rip_peer;
				break_true = TRUE;
				break;
	    	}
		}
		else
		{
		    if ((snmp_oid_compare(Current, RIP_INDEX_OID_LEN, name, *length) > 0)
		    	&& (!peer_header || (snmp_oid_compare(Current, RIP_INDEX_OID_LEN, lowest, RIP_INDEX_OID_LEN) < 0)))
		    {
				peer_header = rip_peer;
				memcpy( (char *)lowest,(char *)Current, RIP_INDEX_OID_LEN * sizeof(oid));
			}
		}
		rip_peer = rip_peer->forw;
	}
/*
	if (rip_peer == &(rip_glb_info.peer_list))
		return NULL;
*/
	memcpy( (char *)name,(char *)lowest, RIP_INDEX_OID_LEN * sizeof(oid));
	*length = RIP_INDEX_OID_LEN;
	*write_method = 0;
	*var_len = sizeof(long);

	return peer_header;
}

