# RIP代码

<img src="./Rip code.assets/image-20240926165142493.png" alt="image-20240926165142493" style="zoom: 50%;" />

## 预编译指令

`#pragma pack(1)` ：将结构体成员按 1 字节对齐

`#pragma align 1`：在某些编译器中可能用于设置对齐方式

`#pragma align 0`：通常用于恢复默认对齐设置

`#pragma pack ()`：取消自定义字节对齐方式

## RIP报文

<img src="./Rip code.assets/image-20240929162720719.png" alt="image-20240929162720719" style="zoom: 67%;" />

### RIP报文头部

<img src="./Rip code.assets/image-20240929172316948.png" alt="image-20240929172316948" style="zoom:50%;" />

```c
/*RIP报文头部数据结构*/
typedef struct rip_pkt_head_
{
	uint8  cmd;   /*报文类型       */
	uint8  version;/*报文版本类型*/
	uint16 zero;  /*零项               */
}rip_pkt_head_t;
```

### RIP路由条目

<img src="./Rip code.assets/image-20240929172337085.png" alt="image-20240929172337085" style="zoom:50%;" />

```c
/*RIP路由条目数据结构*/
typedef struct rip_route_item_
{
	 uint16  afi;			/*AFI*/
	 uint16  route_tag;     /*路由标记:域内路由/域外路由*/
	 uint32  network;       /*网络号*/
	 uint32  mask;          /*网络掩码*/
	 uint32  next_hop;      /*下一跳地址*/
	 uint32  metric;        /*Metric值*/
}rip_route_item_t;
```









###  RIPv2加密认证报文头部

<img src="./Rip code.assets/image-20240929163201100.png" alt="image-20240929163201100" style="zoom: 50%;" />

```c
/* rip authen 报文头部 */
typedef struct rip_authen_head_
{
	uint16 afi;
	uint16 authen_type;
	uint16 packet_len;    /*不包括报文尾部*/
	uint8 keyid;
	uint8 authen_data_len;/*尾部信息长度*/
	int32 sequence_number;
	uint32 zero1;
	uint32 zero2;
}rip_authen_head_t;
```

### RIPv2加密认证报文的尾部

```c
/*rip动态认证的尾部*/
typedef struct rip_authen_tail_
{
	uint16 afi;
	uint16 route_tag;
	union 
	{
		char md5[16];
		char sha1[20];
	}key;
}rip_authen_tail_t;
```



#### MD5报文头部

```c
/*
   |             0xFFFF            | AuType=Keyed Message Digest   |
   +-------------------------------+-------------------------------+
   |    RIP-2 Packet Length        |    Key ID    | Auth Data Len  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |               Sequence Number (non-decreasing)                |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |               reserved must be zero                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |               reserved must be zero                           |

*/
```

```c
/* MD5报文头部 */
typedef struct rip_md5_head_
{
	uint16 afi;
	uint16 authen_type;
	uint16 packet_len;     /* 不包括MD5报文尾部 */
	uint8 md5_keyid;
	uint8 authen_data_len; /* MD5尾部信息长度 */
	int32 sequence_number;
	uint32 zero1;
	uint32 zero2;
}rip_md5_head_t;
```

#### MD5报文尾部

```c
/*
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |             0xFFFF            |       0x01                    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /  Authentication Data (var. length; 16 bytes with Keyed MD5)   /
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
```

```
/*MD5报文尾部*/
typedef struct rip_md5_tail_
{
	uint16 afi;
	uint16 route_tag;
	char password[16];
}rip_md5_tail_t;
```

****

#### Command字段

```c
#define	RIPCMD_REQUEST		1	/* want info */             // 请求其他主机发送全部或部分路由表
#define	RIPCMD_RESPONSE		2	/* responding to request */ // 发送全部或部分路由表
#define	RIPCMD_TRACEON		3	/* turn tracing on */
#define	RIPCMD_TRACEOFF		4	/* turn it off */
#define	RIPCMD_POLL			5	/* like request, but anyone answers */
#define	RIPCMD_POLLENTRY	6	/* like poll, but for entire entry */
```

#### Version字段

```c
#define	RIP_VERSION_0	0
#define	RIP_VERSION_1	1
#define	RIP_VERSION_2	2
```

#### Address Family Identifier字段

```c
#define	RIP_AF_UNSPEC	0
#define	RIP_AF_INET	2       // 
#define	RIP_AF_AUTH	0xffff
```





## 接口信息

```c
/*RIP端口信息*/
typedef struct rip_intf_
{
	uint32 vrf_id;/*端口对应的VRF*/     
	uint32 process_id;/*被使能的进程号*/
	uint32 device_index;	/* 接口ID*/
	struct rip_process_info_ *pprocess;

/*	int ref_num;/*该端口被RIP网络覆盖的次数*/

	uint32 encap_type; /* 接口封装类型*/
	uint8 state;/*端口链路状态*/

	uint8 addr_type;/*端口地址类型*/
	uint32 address;/* 接口IP地址*/	
	uint32 mask; /* 接口IP地址掩码*/
	uint32 aid;	/* address id ，当在接口配置多个地址时，表示其id*/

	char intf_name[ INTERFACE_DEVICE_MAX_NAME];/*端口名字*/
	
	uint8 split_flag;/*标志位SPLIT_HORIZON_FLAG*/
	uint8 special_flag;/*passive,notreceive,v1demand,v2demand--dangzhw,20091111*/
	uint8 send_version;/*enum RIP_SEND_VERSION*/
	uint8 recv_version;/*enum RIP_RECV_VERSION*/

	uint8 auth_type;/*端口认证类型*/
	uint8 md5_type;/*md5 key的存储类型*/
	char simple_key[16+1];
	char md5_key[16+1];
	uint8 md5_keyid;

	struct rip_route_ *connect_route;/*端口对应的直连路由*/

	struct rip_route_list_ nbr_route_list;/*该端口学习到的路由链表*/

	uint rip2IfStatRcvBadPackets;/*端口接收到的错误报文数*/
	uint rip2IfStatRcvBadRoutes;/*端口接收到的错误路由数*/
	uint rip2IfStatSentUpdates;/*端口发送的更新报文数*/
	uint rip2IfStatRecvUpdates;/*端口收到的更新报文数*/
	
	int nbr_route_num;
	uint32 auth_commit_timer_id;
	uint32 auth_commit_time;
	/* 端口enable bfd,just for show*/
	uint8 bfd_enable_flag;

	/* Commented by dangzw in 2009.03.04 19:10:01 */
	uint8 dynamicflag;
	uint32 sequence;/*序列号*/
	uint32 key_start_timer_id;/*key开始生效定时器*/
	uint32 key_lift_timer_id;/*key有效时间长度定时器*/
	struct dynamic_key_list_  key_list;/*邻居的key链表*/
	struct dynamic_key_list_  key_timeout_list;/*邻居间超时的key链表*/
	
}rip_intf_t;
```



## RIP全局信息

```c
/*RIP全局信息*/
typedef struct rip_process_info_
{
	uint32 process_id;
	uint32 vrf_id;

	struct rip_process_list_ *pprocess_list;

	uint8 version;/*全局版本号*/
	uint8 flags; /*源地址检查，零域检查等的标志*/
	uint8 auto_summary_flag;/*自动汇总标志位*/

	uint8 default_route_flag;/*default-information originate标志位*/
	uint16 default_metric; /*RIP缺省metric值*/

	uint32 update_timeout; /*配置的RIP update超时时间*/
	uint32 expire_timeout; /*配置的RIP invalid超时时间*/
	uint32 holddown_timeout; /*配置的RIP holddown超时时间*/
	uint32 trigger_timeout; /*Fensb add for configure trigger timeout*/
	uint32 peer_timeout;/*peer timeout time --dangzhw*/

	uint32 update_timer_id;  /*RIP update定时器ID*/
	uint32 expire_timer_id;  /*RIP invalid定时器ID*/
	uint32 holddown_timer_id;  /*RIP holddown定时器ID*/
	uint32 trigger_timer_id; /*RIP触发更新定时器ID*/
	uint32 peer_timer_id; /*RIP peer超时定时器ID*/
	
	struct rip_route_list_  holddown_list; /*处于holddown状态的路由列表*/
	/*struct rip_trigger_list_ trigger_list; *//*RIP 触发更新路由列表*/
	struct route_table * trigger_table ; /*用于建立二叉树的触发表*/

	struct rip_neigh_list_ neigh_list;/*所配置的邻居信息列表*/
	struct rip_peer_list_ peer_list;/*动态peer 信息列表*/
	struct rip_md5_record_ md5_record;
		
    uint rip2GlobalRouteChanges; /* init as 0*/
    uint rip2GlobalQueries ; /* init as 0*/

	uint32 max_nh_num;/*这里只对从邻居学习到的路由生效，并且要求这些路由具有同样的distance和metric，dangzhw2010325*/
	/*xuhaiqing 2010-12-21号将最大路由条目数限制取消*/
	/*uint32 max_route_num;*//*RIP路由表最大路由数*/
	uint32 route_num;/*RIP路由表中路由数*/
/*	uint32 totle_intf_ref;/*记录所有的intf被rip覆盖的次数，当不大于0时，不接受数据，dangzhw20091106*/

	int connect_route_num;/*路由表直连路由数目*/
	int sum_route_num;/*路由表汇总路由数目*/
	int nbr_route_num;/*学习路由数目*/
	int redis_route_num;/*转发路由数目*/
	int holddown_route_num;/*holddown路由数目*/	

	/*struct rip_net_list_ network_list;RIP网络列表*/
	struct route_distribute_list *filter_list;/*RIP过滤列表*/
	struct route_distance_list *distance_list;/* RIP管理距离列表*/
	struct route_offset_list *offset_list;/*RIP offset-list列表*/

	struct link_protocol *rip_redis_link; /*RIP转发信息*/
	struct rip_redis_list_ redis_list;/*记录转发的信息*/

	struct route_table *rip_table;/*RIP本地路由表*/

	struct rip_process_intf_list_ intf_list;/*用来保存本进程有效的interface*/
}rip_process_info_t;
```





```c
typedef struct rip_peer_list_
{
	struct rip_peer_list_ *forw, *back;

	uint32 peer_addr;
	uint16 peer_port;
	uint16 peer_domain;/*保存该peer收到的上一个报文的routing domain域的值*/
	uint32 peer_intf;
	
	uint32 rip2PeerVersion;
	int32  rip2PeerLastUpdate;
	
	uint   rip2PeerRcvPackets;/*add 2019.8.30*/
	uint   rip2PeerRcvBadPackets;
	
	uint   rip2PeerRcvRoutes;/*add 2019.8.30*/
	uint   rip2PeerRcvBadRoutes;

	int  ref_num; /*该peer接收到的路由数目*/
	int  auth_state; /* 0:通过认证, 1:等待认证 */
}rip_peer_list_t;
```





## MBI扩展

### 根节点

so(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).rip2(23)

```c
oid rip_variables_oid[] = { 1,3,6,1,2,1,23 };
```

### 全局节点

```c
// 如OID对象标识符：1.3.6.1.2.1.23.1.(1/2)
#define RIP2GLOBALROUTECHANGES	                1
#define RIP2GLOBALQUERIES		                2
```

### 表节点

#### 接口状态表

```c
// 接口状态表 2.1
// 如OID对象标识符：1.3.6.1.2.1.23.2.1.(1/2/3/4/5)
#define RIP2IFSTATADDRESS		                1
#define RIP2IFSTATRCVBADPKTS	                2
#define RIP2IFSTATRCVBADROUTES	                3
#define RIP2IFSTATSENTUPDATES	                4
#define RIP2IFSTATSTATUS		                5
```

#### 接口配置表

```c
// 接口配置表 3.1
// 如OID对象标识符：1.3.6.1.2.1.23.3.1.(1/2/3/4/5/6/...)
#define RIP2IFCONFADDRESS		                1
#define RIP2IFCONFDOMAIN		                2
#define RIP2IFCONFAUTHTYPE		                3
#define RIP2IFCONFAUTHKEY		                4
#define RIP2IFCONFSEND			                5
#define RIP2IFCONFRECEIVE		                6
#define RIP2IFCONFDEFAULTMETRIC	                7
#define RIP2IFCONFSTATUS		                8
#define RIP2IFCONFSRCADDRESS	                9
```

#### 邻居表

```c
// 邻居表 4.1
#define RIP2PEERADDRESS			                1
#define RIP2PEERDOMAIN			                2
#define RIP2PEERLASTUPDATE		                3
#define RIP2PEERVERSION			                4
#define RIP2PEERRCVBADPKTS		                5
#define RIP2PEERRCVBADROUTES	                6
```

