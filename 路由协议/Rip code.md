# RIP代码

<img src="./Rip code.assets/image-20240926165142493.png" alt="image-20240926165142493" style="zoom: 50%;" />

## 预编译指令

`#pragma pack(1)` ：将结构体成员按 1 字节对齐

`#pragma align 1`：在某些编译器中可能用于设置对齐方式

`#pragma align 0`：通常用于恢复默认对齐设置

`#pragma pack ()`：取消自定义字节对齐方式





## 外部数据结构

### rt_msg

```c
struct rt_msg 
{
	unsigned long type;
	unsigned long uid;
	unsigned long len;
	unsigned long param;
};
```

```c
struct MESSAGE_PARAM 
{
	union {
		unsigned long dest;
		unsigned long ifindex;

		unsigned long proto;
		/* DV_DEVICE_ENTRY * ifap; */
		unsigned long data_uint32;
		long data_int32;
		void * ptr;
	} data1;
	union {
		unsigned long mask;
		unsigned long ipaddr;
		
		void *ptr;
		unsigned long data_uint32;
	} data2;
	union {
		unsigned long dest;
		unsigned long next_hop;
		unsigned long ifindex;

		unsigned long data_uint32;
	} data3; 
	union {
		unsigned long mask;
		long metric;
		unsigned long data_uint32;
	} data4;
	union {
		unsigned long next_hop;
		long tag;
		unsigned long data_uint32;
	} data5;
	union {
		unsigned long metric;
		unsigned long data_uint32;
		void *ptr;
	} data6;
	union {
		unsigned long tag;
		unsigned long data_uint32;
	} data7;
	union {
		unsigned long data_uint32;
	} data8;
	union {
		unsigned long data_uint32;
	} data9;
	union {
		unsigned long data_uint32;
	} data10;
	union {
		unsigned long data_uint32;
	} data11;
	union {
		unsigned long data_uint32;
	} data12;
	union {
		unsigned long data_uint32;
	} data13;
	union {
		unsigned long data_uint32;
	} data14;
	union {
		unsigned long data_uint32;
	} data15;
};
```





## RIP报文

<img src="./Rip code.assets/image-20240929162720719.png" alt="image-20240929162720719" style="zoom: 67%;" />

### rip_pkt_head_

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

### rip_route_item_

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



## rip_glb_info_

```c
typedef struct rip_glb_info_
{
	int socket_id;/*RIP socket id,全局共用一个socket*/

	uint32 queue_id;
	uint32 sem_id;/*semphore id*/
	uint32 input_queue;

	uint8 debug_flag;
}rip_glb_info_t;
```



## rip_intf_

```c
/*接口的RIP配置信息*/
typedef struct rip_intf_
{   
    uint32 device_index;	                    /* 端口ID*/
	uint32 vrf_id;     /*端口对应的VRF*/     
    
	uint32 process_id; /*被使能的进程号*/
	struct rip_process_info_ *pprocess;    // 端口对应的RIP进程信息
    

/*	int ref_num;/*该端口被RIP网络覆盖的次数*/

	uint32 encap_type; /* 接口封装类型*/
	uint8 state;        /*端口链路状态*/

	uint8 addr_type;/*端口地址类型*/
	uint32 address; /* 接口IP地址*/	
	uint32 mask;    /* 接口IP地址掩码*/
	uint32 aid;  	/* address id ，当在接口配置多个地址时，表示其id*/

	
	uint8 split_flag;   /* 端口开启的分割类型（水平分割/毒性逆转水平分割） */
	uint8 special_flag; /*passive,notreceive,v1demand,v2demand--dangzhw,20091111*/
    
	uint8 send_version; /*端口发送RIP报文的版本类型*/
	uint8 recv_version; /*端口接收RIP报文的版本类型*/
	uint8 auth_type;	/*端口开启的Auth认证类型*/
    
	uint8 md5_type;     /*md5 key的存储类型*/
	char simple_key[16+1];
	char md5_key[16+1];
	uint8 md5_keyid;

	struct rip_route_ *connect_route;      /*端口对应的直连路由*/
	struct rip_route_list_ nbr_route_list; /*该端口学习到的路由链表*/

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
	uint8  dynamicflag;
	uint32 sequence;/*序列号*/
    
	uint32 key_start_timer_id;/*key开始生效定时器*/
	uint32 key_lift_timer_id; /*key有效时间长度定时器*/
    
	struct dynamic_key_list_  key_list;			/*邻居的key链表*/
	struct dynamic_key_list_  key_timeout_list; /*邻居间超时的key链表*/
	
}rip_intf_t;
```



### rip_route_list_

```c
/*RIP路由链表*/
typedef struct rip_route_list_
{
	struct rip_route_list_ *forw, *back;
	struct rip_route_ *rip_route;
}rip_route_list_t;
```

#### rip_route_

```c
/*存储RIP路由信息*/
typedef struct rip_route_
{
	struct rip_route_ *forw, *back;
	struct route_node *route_node;

	struct rip_process_info_ *pprocess; /*路由归属的rip进程*/
	/*uint32 process_id;*/
	uint32 gw_addr;   /*网关地址*/
	uint32 gw_index;  /*路由对应的端口*/	
	uint32 next_hop;	
	uint32 distance;  /*RIP 路由管理距离*/
	uint32 metric;    /*RIP 路由metric值*/
	uint16 route_tag; /*RIP 路由标识*/
	uint16 route_type;/*路由类型:汇总, 直连, 转发, 学习*/

	int32 refresh_time;/*路由刷新时间*/
	uint32 equi_route_num;/*等价路由数*/	
	uint32 equi_nbr_num;/*从邻居学习到的等价路由数目，只针对邻居的同distance和metric的路由信息*/
	
	/*Fengsb add 2006-05-23  */
	rip_subrt_t rip_connect; /* list for subnet connect route */
	uint32 sum_cnt;/*本路由所汇总的直连(子网) 路由数目*/
	uint16 b_supress; /*是否被汇总，用于直连路由，目前暂不使用*/
	uint16 rmap_set_metric; /* metric值是否为route-map所设 */

	struct rip_route_list_ *hold_ptr;/*指向holddown list中的指针*/
    struct rip_route_list_ *nbr_ptr; /* point to the nbr list on the interface */
	struct rip_route_list_ *red_ptr;/*Fengsb 2006-02-19 指向redistribute list中entry的指针*/
}rip_route_t;
```

#### rip_route_info_

```c
/*RIP控制信息*/
typedef struct rip_route_info_
{
	uint32 network;
	uint32 mask;
	uint32 next_hop;
	
	uint32 gw_index; /*路由对应的端口*/
	uint32 gw_addr;  /*网关地址*/

	uint16 route_type;
	uint16 route_tag;
	uint32 metric;
}rip_route_info_t;
```





### dynamic_key_list_

```c
/*邻居的动态key链表*/
typedef struct dynamic_key_list_
{
	struct dynamic_key_list_ *forw, *back;/*key链表*/
	uint32 key_state;                     /*key的状态[超时|激活|为使用|延期]*/
	uint8 key_id;                         /*key id*/
	uint8 algorithms;                     /*算法*/
	uint8 md5_type;                       /*md5 key值存储类型*/
	char key[20+1];                       /*key值*/
	struct key_start_time_ start_time;    /*开始生效时间*/
	struct key_lift_time_ lift_time;      /*有效时间长度*/
}key_list_t;
```

#### key_start_time_

```c
/*动态key开始生效的时间结构*/
typedef struct key_start_time_
{
	uint16 year;/*年*/
	uint16 month;/*月*/
	uint16 day;/*日*/
	uint16 hour;/*小时*/
	uint16 minute;/*分*/
	uint16 second;/*秒*/
}key_start_time;
```

#### key_lift_time_

```c
/*动态key有效时间长度*/
typedef struct key_lift_time_
{
	uint16 hour;  /*小时*/
	uint16 minute;/*分*/
}key_lift_time;
```



## 

## rip_process_list_

```c
/*rip所有进程的链表*/
typedef struct rip_process_list_
{
	struct rip_process_list_ *forw;
	struct rip_process_list_ *back;

	struct rip_process_info_ *rip_process_info;
}rip_process_list_t;
```

### rip_process_info_

```c
/*RIP进程的配置信息*/
typedef struct rip_process_info_
{
	uint32 process_id;    // 进程id
	uint32 vrf_id;

	struct rip_process_list_ *pprocess_list;   // 进程链表

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

	struct rip_neigh_list_ neigh_list; /*所配置的邻居信息列表*/
	struct rip_peer_list_ peer_list;   /*动态peer信息列表*/
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

	struct route_table *rip_table;     /*RIP本地路由表*/

	struct rip_process_intf_list_ intf_list;/*用来保存本进程有效的interface*/
}rip_process_info_t;
```

#### rip_peer_list_

```c
typedef struct rip_peer_list_
{
	struct rip_peer_list_ *forw, *back;

	uint32 peer_addr;      // 邻居使用的IP地址
	uint16 peer_port;	   //
	uint16 peer_domain;     /*保存该peer收到的上一个报文的routing domain域的值*/
	uint32 peer_intf;       
	
	uint32 rip2PeerVersion;
	int32  rip2PeerLastUpdate;
	
	uint   rip2PeerRcvPackets;/*add 2019.8.30*/   
	uint   rip2PeerRcvBadPackets;			      // 从RIP邻居接收到的无效RIP数据包的数量。
	
	uint   rip2PeerRcvRoutes;/*add 2019.8.30*/
	uint   rip2PeerRcvBadRoutes;                  // 从某个RIP邻居接收到的**无效路由条目**的数量。

	int  ref_num; /*该peer接收到的路由数目*/
	int  auth_state; /* 0:通过认证, 1:等待认证 */
}rip_peer_list_t;
```

#### rip_neigh_list_

```c
/*RIP 邻居列表*/
typedef struct rip_neigh_list_
{
	struct rip_neigh_list_ *forw, *back;
	uint32 neigh_addr;
}rip_neigh_list_t;
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



## 函数

### 初始化函数

#### 端口初始化

##### rip_intf_add

> 端口开启RIP后，初始化端口的RIP配置信息

1. 检查参数端口号是否合法

2. 查看此端口是否已经设置过RIP了

3. 分配内存并初始化端口的RIP配置信息

   - 设置端口发送/接收RIP报文的版本类型
   - 设置端口开启的Auth认证类型
   - 设置端口的分割类型
   - 初始化端口学习到的路由列表
   - 初始化key_list和key_timeout_list
   - 为端口设置定时器
     - key开始生效定时器
     - key有效时间长度定时器
     - 邻居认证定时器

   - 获取并设置端口名称
   - 初始化邻居的路由器数量

   

##### rip_intf_del

> 删除端口上设置的RIP配置

1. 检查参数端口号是否合法
2. 查看此端口是否已经设置过RIP了
3. 删除该端口学习到的路由







## RipMain

### rip_recv_packet(uint32 pkt_len)

- 检查传入的数据包长度参数是否合法` <= RIP_MAX_PACKET_SIZE`
- 从socket中接收数据

