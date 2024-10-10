# RIP代码

```
  1. RIP RFC，包括MIB brower，读取mib节点
  2. rip和cisco模拟器做互通性实验；
  3. 走读代码，并在组内做rip模块介绍
  4. 对照入网测试文档，移动入围测试文档，自己尝试写测试用例，做测试设计，并进行测试。
```



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

### route_table

```c
struct route_table{
	struct route_node *top;
}
```

### route_node

```c
struct route_node{
	struct prefix p;
	struct route_table *table;
	struct route_node *parent;
	struct route_node *link[2];
#define l_left link[0];
#define r_right link[1];
	unsigned int lock;
	void *info;
	void *aggregate;
}
```



![image-20241008150321881](./Rip code.assets/image-20241008150321881.png)



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









###  rip_authen_head_

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

### rip_authen_tail_

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



#### rip_md5_head_

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

#### rip_md5_tail_

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

### 报文头部字段

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
    uint32 device_index;	            /* 端口ID*/
	uint32 vrf_id;                      /*端口对应的VRF*/     
    
	uint32 process_id; /*被使能的进程号*/
	struct rip_process_info_ *pprocess;    // 端口对应的RIP进程信息
    
	uint32 encap_type;  /*端口封装类型*/
	uint8 state;        /*端口链路状态*/

	uint8 addr_type;    /*端口地址类型*/
	uint32 address;     /* 接口IP地址*/	
	uint32 mask;        /* 接口IP地址掩码*/
	uint32 aid;  	    /* address id ，当在接口配置多个地址时，表示其id*/

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
	/* 端口enable bfd,just for show */
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

	uint32 gw_addr;   /*网关地址*/
	uint32 gw_index;  /*路由对应的端口*/	
	uint32 next_hop;	
	uint32 distance;  /*RIP路由管理距离*/
	uint32 metric;    /*RIP路由metric值*/
	uint16 route_tag; /*RIP路由标识（域内或域外）*/
	uint16 route_type;/*路由类型:邻居, 直连, 转发, 汇总，缺省*/

	int32 refresh_time;   /*路由刷新时间*/
	uint32 equi_route_num;/*等价路由数*/	
	uint32 equi_nbr_num;  /*从邻居学习到的等价路由数目，只针对邻居的同distance和metric的路由信息*/
	
	/*Fengsb add 2006-05-23  */
	rip_subrt_t rip_connect; /* list for subnet connect route */
	uint32 sum_cnt;/*本路由所汇总的直连(子网) 路由数目*/
	uint16 b_supress; /*是否被汇总，用于直连路由，目前暂不使用*/
	uint16 rmap_set_metric; /* metric值是否为route-map所设 */

	struct rip_route_list_ *hold_ptr;/*指向holddown list中的指针*/
    struct rip_route_list_ *nbr_ptr; /*point to the nbr list on the interface */
	struct rip_route_list_ *red_ptr; /*Fengsb 2006-02-19 指向redistribute list中entry的指针*/
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

	uint32 update_timer_id;   /*RIP update定时器ID*/
	uint32 expire_timer_id;   /*RIP invalid定时器ID*/
	uint32 holddown_timer_id; /*RIP holddown定时器ID*/
	uint32 trigger_timer_id;  /*RIP触发更新定时器ID*/
	uint32 peer_timer_id;     /*RIP peer超时定时器ID*/
	
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



## rip_trigger_list_

```c
/*RIP触发更新列表*/
typedef struct rip_trigger_list_
{
	struct rip_trigger_list_ *forw, *back;
    
	/*Fengsb 2006-02-13 add: if  route_node==NULL, means the route is rmv frm rip local table
	so update timeout, this entry can't be compressed */
	struct route_node *route_node; 
	uint32 network;
	uint32 mask;
	uint32 next_hop;
	uint32 metric;

	uint32 gw_index;
	uint32 gw_addr;

	uint16 route_type;
	uint16 route_tag;
}rip_trigger_list_t;
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



## rip_init.c

### RIP模块初始化 RipTask

```c
void RipTask( void )
```

> 1.初始化rip_glb_info全局信息
>
> ```c
> memset(&rip_glb_info, 0, sizeof(struct rip_glb_info_));
> rip_glb_info.socket_id = -1;
> rip_glb_info.input_queue = RIP_DEF_INPUT_QUEUE;
> rip_process_list.back =rip_process_list.forw = &rip_process_list;
> ```
>
> 2.创建信号量
>
> ```c
> ret = sm_create((int8 *)"S_RIP", 1, SM_GLOBAL | SM_FIFO | SM_UNBOUNDED, &rip_semaphore);
> ```
>
> 3.创建消息队列
>
> ```c
> ret = q_create((uint32) "RIPD", 0, Q_FIFO|Q_NOLIMIT|Q_GLOBAL, &(rip_glb_info.queue_id) );
> ```
>
> 4.创建互斥量ID
>
> ```c
> ret = mu_create("RIPM", MU_FIFO | MU_RECURSIVE | MU_GLOBAL, 0, &(rip_glb_info.sem_id) );
> ```
>
> 5.向Routing注册端口和路由事件
>
> ```c
> rt_RegisterTask( RTPROTO_RIP, 0, rip_task_callback );
> ```
>
> 6.启动一个RipMain主任务
>
> ```c
> #ifndef OS_VXWORKS
> 	unsigned long tid;
> #endif
> ......
> #ifdef OS_VXWORKS	
> 	// 启动一个新的任务（线程）
> 	// ---- 任务名、优先级、调度选项（不可抢占）、为任务分配的栈大小、入口函数、`0,0`传给RipMain的参数
> 	if (sys_task_spawn("RIPT", SYS_TASK_PRI_NORMAL, T_OP_NOPREEMPT, 50*1024, RipMain, 0, 0)== (TASK_ID)SYS_ERROR) {
> 		perror("RIP: sys_task_spawn()\n");
> 	}
> #else
> 	// 其他操作系统环境下的任务创建
> 	ret = t_create("RIPT", 128, 40960, 4096, 0, &tid);
> 	if (ret != NOERR) {
> 		perror("RIP: t_create()\n");
> 	}
> 	ret = t_start(tid, T_SUPV | T_NOPREEMPT | T_NOTSLICE, RipMain, 0);
> 	if (ret != NOERR) {
> 		perror("RIP: t_start()\n");
> 	}
> #endif
> ```
>
> 7.注册RIP模块命令
>
> ```c
> ret = rip_register_cmd();
> ```
>
> 8.注册MIB
>
> ```c
> ret = rip_register_mib();
> ```
>
> 9.注册版本信息
>
> ```c
> rip_version_register();   
> ```

### socket初始化 rip_socket_init

> 初始化rip_glb_info中的sockfd

```c
int rip_socket_init( void )
```

> 1.创建socket
>
> 2.为socket设置参数选项
>
> - 设置为非阻塞：NONBLOCKING
> - 开启广播：BROADCAST
> - 设置发送/接收缓冲区的大小
>   - task_set_option(rip_glb_info.socket_id, TASKOPTION_SENDBUF, RIP_MAX_PACKET_SIZE)
>   - task_set_option(rip_glb_info.socket_id, TASKOPTION_RECVBUF, RIP_MAX_PACKET_SIZE*rip_glb_info.input_queue )
> - 启用 RCVDSTADDR 选项，获得数据包的目标地址
> - 设置数据包的TTL
> - 禁用多播环回
> - 设置多播数据包的TTL
>
> 3.注册socket
>
> ```c
> memset((void *) &sa, 0, sizeof(struct soaddr_in));
> sa.sin_len    = sizeof(struct soaddr_in);
> sa.sin_family = AF_INET;
> sa.sin_port   = htons(RIP_PORT); 
> 	
> if( task_addr_local( rip_glb_info.socket_id, &sa, rip_glb_info.queue_id ) ) 
> {
> 	return RIP_FAIL;
> }
> ```



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







## rip_main.c

### RipMain

```c
// 主要处理来自从Socket接口监听到和timer计时器触发的事件信息,
void RipMain( void );
```

> 1.从消息队列中取出消息
>
> ```c
> /*变量初始化*/
> memset( (void *)(&msg), 0, sizeof(struct rt_msg) );
> /*从消息队列接收消息*/
> ret = q_receive( rip_glb_info.queue_id, Q_WAIT, 0, (uint32 *)&msg);
> ```
>
> 2.根据消息的类型进行处理
>
> ```c
> switch(msg.type)
> {
>     case IP_SOCKET_RECV_PACKET:  // 接收到消息报文
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive packet(length = %d)\n", msg.len );
>         /*从socket接收数据*/
>         rip_recv_packet( msg.len );
>         break;
>     case MSG_RIP_UPDATE_TIMEOUT: /*周期更新定时器超时*/
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Update timer timeout(process %d)\n" ,msg.uid );
>         /*周期更新定时器超时*/
>         rip_update_timeout(msg.uid);
>         break;
>     case MSG_RIP_EXPIRE_TIMEOUT:
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Expire timer timeout(process %d)\n",msg.uid );
>         /*route-expire定时器超时*/
>         rip_expire_timeout(msg.uid);
>         break;
>     case MSG_RIP_HOLDDOWN_TIMEOUT:
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Holddown timer timeout(process %d)\n",msg.uid );
>         /*route-holddown 定时器超时*/
>         rip_holddown_timeout(msg.uid);
>         break;
>     case MSG_RIP_TRIGGERED_TIMEOUT:
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Trigger timer timeout(process %d)\n",msg.uid );
>         /*触发更新定时器超时*/
>         rip_trigger_timeout(msg.uid);
>         break;
>     /* peer超时定时器会在有peer的时候周期性的扫描peer链表；
>     扫描时，当距离上次收到该peer的时间间隔大于等于peer超时时间，但同时小于3*peer超时定时器时，
>     将会向该peer发送一次request(当peer的rip路由为空的时候，发送全部路由查询分组，bdcom和思科的设备
>     都不回应，所以发送一条网段为0metic也为0的特定路由查询--收到后因为AFI也不会加入到database里，实现同bdcom设备的peer维护)；
>     当距离上次收到该peer的时间间隔大于等于3*peer超时时间，则删除该peer;
>     同时去除del route时删除peer的操作，在删除rip进程的时候清除全部peer；
>     peer定时器默认周期为(RIP_DEF_EXPIRE_TIMEOUT +RIP_DEF_HOLDDOWN_TIMEOUT)/3 +2s,
>     超时的时候还将检查peer是否被network覆盖或是不是neighbor，若都不是将被删除(不然该peer还可能发送request分组)；	
>     注意，当系统一起来的时候，若对端没有路由信息，将不能将对端加到peer中来(有时间可以完善)
>      * Commented by dangzhw in 2009.12.05 09:01:56 */
>     case MSG_RIP_PEER_TIMEOUT:
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: peer timeout(process %d)\n",msg.uid );
> 
>         /*触发更新定时器超时*/
>         rip_peer_timeout(msg.uid);
>         break;
>     case MSG_RIP_INTF_KEY_START:
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: intf key timer start(device_index %d)\n",msg.uid );
>         rip_neighb_timer_key_start_check(msg.uid);
>         break;
>     case MSG_RIP_INTF_KEY_TIMEOUT:
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: intf key timer timeout(device_index %d)\n",msg.uid );
>         rip_neighb_key_life_out(msg.uid);
>         break;
>     case MSG_RIP_INTF_AUTH_COMMIT_TIMEOUT:
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: intf auth failure timer timeout(device_index %d)\n",msg.uid );
>         rip_auth_commit_timeout(msg.uid);
>         break;
>     default:
>         break;
> }
> ```
>
> 



### rip_get_recv_version

```c
// 获取端口上的RIP进程支持接收到报文的版本号
int rip_get_recv_version( uint32 device_index );
```

> 1.根据端口获取其开启的RIP进程
>
> ```c
> struct rip_process_info_ *pprocess = rip_intf_array[device_index]->pprocess;
> ```
>
> 2.返回端口设置的接收RIP报文的版本号
>
> ```c
> switch( rip_intf_array[device_index]->recv_version )
> {
>     case RIP_RECV_VERSION_DEF:  // 端口设置的RIP version设置为默认，则查询全局RIP信息
>         if( RIP_VERSION_DEF == pprocess->version )
>         {
>             return RIP_RECV_VERSION_1_2;/*接收版本1 和2*/
>         }
>         else if( RIP_VERSION_1 == pprocess->version )
>         {
>             return RIP_RECV_VERSION_1;/*接收版本1 */
>         }
>         else if( RIP_VERSION_2 == pprocess->version )
>         {
>             return RIP_RECV_VERSION_2;/*接收版本2*/
>         }
>         break;
>     case RIP_RECV_VERSION_1:
>         return RIP_RECV_VERSION_1;/*接收版本1 */
>         break;
>     case RIP_RECV_VERSION_2:
>         return RIP_RECV_VERSION_2;/*接收版本2*/
>         break;
>     case RIP_RECV_VERSION_1_2:
>         return RIP_RECV_VERSION_1_2;/*接收版本1 和2*/
>         break;
>     default:
>         break;
> }
> ```

### rip_set_send_version

```c
int rip_set_send_version( uint32 device_index );
```

> 1.返回端口设置的send_version
>
> ```c
> /*端口版本优先*/
> switch( rip_intf_array[device_index]->send_version )
> {
>     // 若端口指定为默认，需要根据全局rip进程的设置来返回send_version
>     case RIP_SEND_VERSION_DEF:
>         if( RIP_VERSION_1 == pprocess->version )
>         {
>             return RIP_SEND_VERSION_1;      /*发送版本1*/
>         }
>         else if( RIP_VERSION_2 == pprocess->version )
>         {
>             return RIP_SEND_VERSION_2_MULTICAST;/*发送版本2,目的地址224.0.0.9*/
>         }
>         /* 为了配合北京联通测试，增强rip的自适应功能，在缺省version的情况下(全局version和端口version都缺省)，
>         若目前该端口和对端是一对一的链接时，则按照对端的版本号发送；
>         若该端口对端是多个接口(一对多，如通过hub)，但这多个接口运行的版本号一致，
>         则发送他们共同的verision；
>         若这多个接口的version不一致，则按系统默认的version发送(目前是version 1)
>          * Commented by dangzhw in 2009.10.12 09:57:15 */
>         else if(RIP_VERSION_DEF == pprocess->version)
>         {
> 			// 若目前该端口和对端是一对一的链接时，则按照对端的版本号发送；
>             peer_version = rip_loopup_peer_version(device_index);
>             if( (peer_version == RIP_VERSION_1) || (peer_version == RIP_VERSION_2))
>                 return peer_version;
>             else
>             {
>                 /*return RIP_SEND_VERSION_1;*/
> 
>                 /* 为了规避收到rip报文后对verison1下的路由信息合法性耗时的检查,
>                 将默认版本号设置为version2的multicast方式
>                  * Commented by dangzhw in 2010.02.20 14:25:50 */
>                 return RIP_SEND_VERSION_2_MULTICAST;
>             }
>         }
>         break;
>     case RIP_SEND_VERSION_1:
>         return RIP_SEND_VERSION_1;           /*发送版本1*/
>         break;
>     case RIP_SEND_VERSION_2_BROADCAST:
>         return RIP_SEND_VERSION_2_BROADCAST; /*广播发送版本2*/
>         break;
>     case RIP_SEND_VERSION_2_MULTICAST:
>         return RIP_SEND_VERSION_2_MULTICAST; /*发送版本2,目的地址224.0.0.9*/
>         break;
>     default:
>         break;
> }
> ```
>
> 



### rip_enable_auto_summary

```c
/**
函数功能: 判断自动汇总功能是否启动
输入参数: rip_tbl_info: RIP路由表
输出参数: 无
返回值:    自动汇总功能启动,返回TRUE
          自动汇总功能禁止,返回FALSE
*/
BOOL rip_enable_auto_summary( struct rip_process_info_ *pprocess,uint32 device_index );
```

> 1.检查端口的send_version，若是RIPv1则返回TRUE；否则，查看全局RIP信息
>
> ```c
> sendversion = rip_set_send_version(device_index);
> if(/*RIP_VERSION_DEF == sendversion || */RIP_VERSION_1 == sendversion)
> {
>     return TRUE;
> }
> else
> {
>     if(RIP_AUTO_SUMMARY_DISABLE == pprocess->auto_summary_flag)
>         return FALSE;
>     else 
>         return TRUE;
> }
> ```
>
> 

## rip_intf.c

### rip_task_callback

```c
// RIP信息回调函数
// args: procotol: 协议号
//		 process: 协议进程
// 		 msg: 回调消息
int rip_task_callback( uint32 procotol, uint32 process, struct rt_msg *msg )
```

> 1.参数协议号检查
>
> 2.根据消息的类型执行相应逻辑
>
> ```c
> switch ( msg->type )
> {
>     case MSG_ROUTING_INTF_ADD:/*增加RIP接口*/
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_INTF_ADD] message\n" );
>         device_index = msg->uid;
>         ret = rip_intf_add( device_index );
>         break;
> 
>     case MSG_ROUTING_INTF_DEL:/*删除RIP接口*/
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_INTF_DEL] message\n" );
>         device_index = msg->uid;
>         ret = rip_intf_del( device_index );
>         break;
> 
>     case MSG_ROUTING_PROTO_UP:/*端口链路状态UP*/
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_PROTO_UP] message\n" );
>         device_index = msg->uid;
>         ret = rip_intf_link_up( device_index );
>         break;
> 
>     case MSG_ROUTING_PROTO_DOWN:/*端口链路状态down*/
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_PROTO_DOWN] message\n" );
>         device_index = msg->uid;
>         ret = rip_intf_link_down( device_index);
>         break;
> 
>     case MSG_ROUTING_ADD_PPPLOCAL:	/*增加PPP协议地址*/			
>     case MSG_ROUTING_ADD_PRIMARY:   /*增加主IP地址*/
>     case MSG_ROUTING_ADD_UNNUMBERED:/*增加端口unnumbered地址*/
>         if( msg->type == MSG_ROUTING_ADD_PPPLOCAL )
>         {
>             rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_ADD_PPPLOCAL] message\n" );
>             addr_type = RIP_PPPLOCAL_ADDRESS;
>         }
>         else if( msg->type == MSG_ROUTING_ADD_PRIMARY )
>         {
>             rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_ADD_PRIMARY] message\n" );
>             addr_type = RIP_PRIMARY_ADDRESS;
>         }
>         else if( msg->type == MSG_ROUTING_ADD_UNNUMBERED )
>         {
>             rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_ADD_UNNUMBERED] message\n" );
>             addr_type = RIP_UNNUMBERED_ADDRESS;
>         }
> 
>         rip_param = (struct MESSAGE_PARAM *)msg->param;
>         address = rip_param->data1.data_uint32;
>         mask = rip_param->data2.data_uint32;
>         device_index = rip_param->data3.data_uint32;
>         aid = rip_param->data4.data_uint32;         /*Fengsb add 2007-07-12*/
>         vrf_id = rip_param->data5.data_uint32;
> 
>         ret = rip_intf_addr_add( device_index, aid, address, mask, addr_type, vrf_id );
>         break;	
> 
>     case MSG_ROUTING_DEL_PRIMARY:
>     case MSG_ROUTING_DEL_UNNUMBERED:
>     case MSG_ROUTING_DEL_PPPLOCAL:
>         if( msg->type == MSG_ROUTING_DEL_PRIMARY )
>         {
>             rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_DEL_PRIMARY] message\n" );
>             addr_type = RIP_PRIMARY_ADDRESS;
>         }
>         else if( msg->type == MSG_ROUTING_DEL_UNNUMBERED )
>         {
>             rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_DEL_UNNUMBERED] message\n" );
>             addr_type = RIP_UNNUMBERED_ADDRESS;
>         }
>         else if( msg->type == MSG_ROUTING_DEL_PPPLOCAL )
>         {
>             rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_ROUTING_DEL_PPPLOCAL] message\n" );
>             addr_type = RIP_PPPLOCAL_ADDRESS;
>         }
>         rip_param = (struct MESSAGE_PARAM *)msg->param;
>         address = rip_param->data1.data_uint32;
>         mask = rip_param->data2.data_uint32;
>         device_index = rip_param->data3.data_uint32;
>         vrf_id = rip_param->data5.data_uint32;
>         ret = rip_intf_addr_del( device_index, address, mask, addr_type, vrf_id );
>         break;
> 
>     case MSG_VRF_DEL:
>         rip_param = (struct MESSAGE_PARAM *)msg->param;
>         vrf_id = rip_param->data1.data_uint32;
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_VRF_DEL] message(vrfid %d)\n",vrf_id );
>         ret = rip_del_vrf_process( vrf_id );
>         break;
> 
>     case MSG_VRF_INTF_ADD:
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_VRF_INTF_ADD] message\n" );
>         rip_param = (struct MESSAGE_PARAM *)msg->param;
>         device_index = rip_param->data1.data_uint32;
>         vrf_id =rip_param->data2.data_uint32; 
>         ret = rip_add_intf_vrf( vrf_id,device_index );
>         break;
> 
>     case MSG_VRF_INTF_DEL:
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_VRF_INTF_DEL] message\n" );
>         rip_param = (struct MESSAGE_PARAM *)msg->param;
>         device_index = rip_param->data1.data_uint32;
>         vrf_id = rip_param->data2.data_uint32;
>         ret = rip_del_intf_vrf( vrf_id ,device_index);
>         break;
> 
>     case MSG_IPRT_TASKPOLICY_CHANGE:
>         rip_debug( RIP_DEBUG_IP_RIP_MSG, "RIP: Receive [MSG_IPRT_TASKPOLICY_CHANGE] message\n" );
>         rip_param = (struct MESSAGE_PARAM *)(msg->param);
>         rth = (struct _rt_head *)(rip_param->data1.data_uint32);
>         ret = rip_route_change( rth );
>         break;
> 
>     default:
>         break;
> }
> ```
>
> 



### rip_intf_add

```c
// 端口开启Rip的处理函数
int rip_intf_add( uint32 device_index )
```

> 1.对端口号合法性检查
>
> 2.判断该端口是否已经开启了RIP，若已添加，则return
>
> 3.为端口分配内存，并初始化
>
> ```c
> rip_intf_array[device_index] = (struct rip_intf_ *)rip_mem_malloc( sizeof(struct rip_intf_), RIP_INTF_TYPE );
> if( rip_intf_array[device_index] == NULL )
> {
>     /*内存分配失败*/
>     fprintf(stderr, "RIP: Malloc memory failed.(%s, %d)\n", __FILE__, __LINE__ );
>     return RIP_MEM_FAIL;
> }
> memset( rip_intf_array[device_index], 0, sizeof(struct rip_intf_) );
> 
> /*初始化*/
> // 端口id
> rip_intf_array[device_index]->device_index = device_index;
> // 指的端口发送/接收RIP的版本号   （缺省发送/接收RIP报文版本）
> rip_intf_array[device_index]->send_version = RIP_SEND_VERSION_DEF;
> rip_intf_array[device_index]->recv_version = RIP_RECV_VERSION_DEF;
> 
> // 设置端口是否开启对报文认证
> rip_intf_array[device_index]->auth_type = RIP_AUTH_NONE;
> // 设置端口开启水平分割模式
> rip_intf_array[device_index]->split_flag = RIP_SPLIT_HORIZON_DEF;
> 
> /*初始化端口学习到的路由列表*/
> rip_intf_array[device_index]->nbr_route_list.forw = &(rip_intf_array[device_index]->nbr_route_list);
> rip_intf_array[device_index]->nbr_route_list.back = &(rip_intf_array[device_index]->nbr_route_list);
> 
> /* Commented by dangzw in 2009.03.04 19:48:18 */
> // 初始化邻居key链表  ？？？
> rip_intf_array[device_index]->key_list.forw = &( rip_intf_array[device_index]->key_list);
> rip_intf_array[device_index]->key_list.back = &( rip_intf_array[device_index]->key_list);
> // 初始化邻居间key的超时链表
> rip_intf_array[device_index]->key_timeout_list.forw = &( rip_intf_array[device_index]->key_timeout_list);
> rip_intf_array[device_index]->key_timeout_list.back = &( rip_intf_array[device_index]->key_timeout_list);
> ```
>
> 4.为端口创建定时器
>
> ```c
> ret = rip_intf_creat_timer(device_index);
> ```
>
> 5.根据端口id和端口名，检查是否准确
>
> ```
> name = rip_intf_array[device_index]->intf_name;
> get_if_namebyid( device_index, &name );
> ```
>
> 6.邻居路由数初始化为0
>
> ```c
> rip_intf_array[device_index]->nbr_route_num = 0;   
> ```



### rip_intf_del

```c
// 端口关闭RIP后的处理函数
int rip_intf_del( uint32 device_index )
```

> 1.device_index参数合法性检查
>
> 2.检查端口是否开启了RIP，若没开启，则无需操作
>
> ```c
> if( NULL == rip_intf_array[device_index] ){
> 	/*该端口还没有增加,不需处理*/
> 	return RIP_SUCCESS;
> }
> ```
>
> 3.删除端口学到的路由信息
>
> ```c
> rip_route_list_temp = rip_intf_array[device_index]->nbr_route_list.forw;
> while( rip_route_list_temp != &(rip_intf_array[device_index]->nbr_route_list))
> {
>     rip_route_list_forw = rip_route_list_temp->forw;
> 
>     /*从holddown列表中删除*/
>     rip_del_holddown( rip_route_list_temp->rip_route );
> 
>     /*删除从端口学习到的路由*/
>     ret = rip_del_route( rip_route_list_temp->rip_route );
>     if( RIP_SUCCESS != ret )
>     {
>         return ret;
>     }
> 
>     /*从路由链表中删除*/
>     REMQUE( rip_route_list_temp );
>     rip_mem_free( rip_route_list_temp, RIP_ROUTE_LIST_TYPE );
> 
>     rip_intf_array[device_index]->nbr_route_num--;
> 
>     rip_route_list_temp = rip_route_list_forw;
> };
> ```
>
> 4.删除端口的邻居key
>
> 5.关闭定时器
>
> 6.释放端口



### rip_create_connect_route

```c
// 创建直连路由
int rip_create_connect_route( uint32 device_index ,uint warning)
```

> 1.对device_index进行参数合法性验证
>
> ```c
> if( (device_index > INTERFACE_DEVICE_MAX_NUMBER)
> 	|| (NULL == rip_intf_array[device_index]) )
> {
> 	return RIP_FAIL;
> }
> ```
>
> 2.根据device_index检查端口对应的RIP进程是否为NULL
>
> ```c
> if(!(pprocess = rip_intf_array[device_index]->pprocess))
> 		return RIP_FAIL;
> ```
>
> 3.根据device_index检查该端口的直连路由是否已被创建
>
> ```c
> if(rip_intf_array[device_index]->connect_route)
> 	return RIP_SUCCESS;
> ```
>
> 4.填充route_prefix
>
> ```c
> memset( &route_prefix, 0, sizeof(struct prefix) );
> route_prefix.family   = AF_INET;
> route_prefix.safi     = SAFI_UNICAST;   // 交换的是 IPv4 单播路由
> route_prefix.prefixlen = mask_to_prefix( rip_intf_array[device_index]->mask );
> route_prefix.u.prefix4.s_addr = rip_intf_array[device_index]->address & rip_intf_array[device_index]->mask;
> ```
>
> 5.根据route_prefix获取端口对应的RIP进程下的路由表中相应的结点 `rip_route_node`
>
> ```c
> /*获取route_node*/
> rip_route_node = route_node_get( pprocess->rip_table, &route_prefix, NULL );
> if( NULL == rip_route_node ){
> 	return RIP_FAIL;
> }
> ```
>
> 6.检查路由节点中的info是否为NULL，若不为NULL，则删掉与直连路由等价的`学到和重发布的路由`
>
> ```c
> rip_route = (struct rip_route *)(rip_route_node->info);
> if(rip_route){
>     /* 删掉学到的路由和重发布的相同路由 */
> 	rip_del_route_node(rip_route); 
> 	rip_route = (struct rip_route_ *)(rip_route_node->info);
> 	if(rip_route){
> 		route_unlock_node(rip_route_node);
> 		syslog(LOG_ERR, "rip: create connect route fail!\n");
> 		return RIP_FAIL; 
> 	}
> } 
> ```
>
> 7.填充rip_route数据信息
>
> ```c
> rip_route = (struct rip_route_ *)rip_mem_malloc(sizeof(struct rip_route_), RIP_ROUTE_TYPE );
> if( NULL == rip_route ){
> 	route_unlock_node(rip_route_node);
> 	fprintf(stderr, "RIP: Malloc memory failed.'(%s, %d)\n", __FILE__, __LINE__ );
> 	return RIP_MEM_FAIL;
> }
> memset( (void *)rip_route, 0, sizeof(struct rip_route_) );
> rip_route->forw = rip_route;
> rip_route->back = rip_route;
> 
> rip_route->route_tag = RIP_INTERNAL_ROUTE;  // 路由标记：域内路由
> rip_route->route_type = RIP_CONNECT_ROUTE;  // 路由类型：直连
> 	
> rip_route->refresh_time = time_sec;	
> rip_route->pprocess= pprocess;
> 	
> rip_route->gw_index = device_index;
> /*Fengsb add 2006-04-30 : this distance is the distance that no consider network and mask*/
> rip_route->distance = pprocess->distance_list->default_distance;
> 	
> rip_route->gw_addr = 0;/*直连路由,无网络地址*/
> rip_route->metric = RIP_DIRECT_METRIC;
> 
> rip_route->hold_ptr = NULL;
> 	
> /*路由加入route node */
> rip_route_node->info = (void *)rip_route;	
> rip_route->route_node = rip_route_node;
> 	
> pprocess->route_num++;
> pprocess->connect_route_num++;
> rip_route->equi_route_num++;
> ```
>
> 8.后续的处理
>
> ```c
> // 添加接口的直连路由
> rip_intf_array[device_index]->connect_route = rip_route;
> /*将新路由加入触发更新列表*/
> rip_add_trigger( rip_route, pprocess, FALSE );	
> rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "RIP-DB: Adding connected route %s/%d to RIP database\n", ip_ntoa(route_prefix.u.prefix4.s_addr), route_prefix.prefixlen );
> 
> pprocess->rip2GlobalRouteChanges++;
> /*Fengsb 2006-05-23 创建汇总路由*/
> rip_create_summary_route(device_index , rip_route ,pprocess);
> ```
>
> 



### rip_del_route

```c
// 删除该路由
int rip_del_route( struct rip_route_ *rip_route )
```

> 1.将路由对应的节点下的info转为` rip_route_ *`类型，返回路由头节点
>
> ```c
> rip_route_head = (struct rip_route_ *)(rip_route->route_node->info);
> if(rip_route_head == NULL )
> 	return RIP_SUCCESS;
> ```
>
> 2.检查rip_route_head的pprocess进程是否为空
>
> ```c
> if(!(pprocess = rip_route_head->pprocess)){
> 	rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
> 	return RIP_FAIL;
> }
> ```
>
> 3.等价路由条目-1
>
> ```c
> rip_route_head->equi_route_num--;
> ```
>
> 4.检查删除的rip_route节点是否与头节点相同
>
> ```c
> if( rip_route_head != rip_route ){
> 	/*rip_route从route_node中删除*/
> 	REMQUE( rip_route );
> }
> else{
> /*Fengsb add note 2006-02-14此处应该考虑等价路由，若存在等价路由，需要把下一route entry挂在info上，同时需要注意equi_route_num的处理*/
>     // 若该删除的路由是头节点，则判断是否存在其他等价路由，若存在，则修改头节点信息
> 	if(rip_route_head->equi_route_num != 0){
> 		rip_route->route_node->info = rip_route->forw;
> 		rip_new_rthead = (struct rip_route_ *)(rip_route->route_node->info);
> 		rip_new_rthead->equi_route_num = rip_route_head->equi_route_num;
> 		rip_new_rthead->equi_nbr_num = rip_route_head->equi_nbr_num;
> 		rip_route_head = rip_new_rthead;
> 		/*remove头节点以后，调整后续节点链表指针*/
> 		rip_route->forw->back = rip_route->back;
> 		rip_route->back->forw = rip_route->forw;		
> 	}
> 	else{
> 		rip_route->route_node->info = NULL;
> 	}
> }
> ```
>
> 5.根据不同的路由类型做相应处理
>
> ```c
> switch( rip_route->route_type )
> {
> 	case RIP_CONNECT_ROUTE:
> 		device_index = rip_route->gw_index;
> 		rip_intf_array[device_index]->connect_route = NULL;
> 		pprocess->connect_route_num--;
> 		pprocess->route_num--;
> 		rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "RIP-DB: Deleting connected route %s/%d from RIP database\n", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
> 		break;
> 	case RIP_NBR_ROUTE:
> 		/*学习路由,从主路由表中删除*/
> 		/*before del route, del the peer first*/
> 		rip_peer = pprocess->peer_list.forw;
> 		while(rip_peer != &(pprocess->peer_list))
> 		{
> 			rip_peer_forw = rip_peer->forw;
> 			if(rip_peer->peer_addr == rip_route->gw_addr)
> 			{
> 				rip_peer->ref_num --;
> 				break;
> 			}
> 			rip_peer = rip_peer_forw;
> 		}
> 		rip_del_from_main_tbl( rip_route ,pprocess->process_id);
> 
> 		pprocess->nbr_route_num--;
> 		pprocess->route_num--;
> 		rip_route_head->equi_nbr_num--;
> 
> 		if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_DATABASE )
> 		{
> 			sprintf(string, "RIP-DB: Deleting learn route %s/%d <metric %ld>", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen, rip_route->metric );
> 			sprintf(string, "%s via %s from RIP database", string, ip_ntoa( rip_route->gw_addr ) );
> 			rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "%s\n", string );
> 		}
> 
> 		break;
> 	case RIP_REDIS_ROUTE:
> 		/*转发路由*/
> 		pprocess->redis_route_num--;
> 		pprocess->route_num--;
>         b_del_redis = TRUE; /*Fengsb 2006-04-27 add */
> 	    rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "RIP-DB: Deleting redistributed route %s/%d from RIP database\n", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
> 		break;
> 	case RIP_DEF_ROUTE:
> 		/*缺省路由*/			
> 		pprocess->route_num--;  
> 		b_del_redis = TRUE; /* ygc add 2012-09-25 */
> 		rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "RIP-DB: Deleting rip default route %s/%d from RIP database\n", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
> 		break;
> 	case RIP_SUMMARY_ROUTE:
> 		/*汇总路由*/			
> 		pprocess->route_num--; 
> 		pprocess->sum_route_num--;  
> 		rip_debug(RIP_DEBUG_IP_RIP_DATABASE, "RIP-DB: Deleting rip summary route %s/%d from RIP database\n", ip_ntoa(rip_route->route_node->p.u.prefix4.s_addr), rip_route->route_node->p.prefixlen );
> 		break;
> 	default:
> 		break;
> }
> ```
>
> 6.触发更新
>
> ```c
> /*Fengsb 2006-02-15 add the first condition to avoid route linar circle 
> 	触发更新,metric值为16*/
> if( (rip_route_head->equi_route_num == 0) 
>     && ( RIP_MAX_METRIC != rip_route->metric ) && (rip_route->route_type != RIP_SUMMARY_ROUTE))
> {
> 	rip_route->metric = RIP_MAX_METRIC;		
> 	rip_add_trigger(rip_route, pprocess, TRUE);
> }
> else if(b_del_redis == TRUE)
> { /*用于删除转发路由后，同一节点还存在邻居学习到的路由的情况*/
> 	rip_add_trigger(rip_route, pprocess, TRUE);
> }
> 	
> /*Fengsb 2006-02-16 因为每次添加路由，都对route_node lock++，所以删除路由
> 的时候，也需要对route node lock--*/
> 	route_unlock_node( rip_route->route_node );		
> 
> 	/*释放rip_route*/
> 	rip_mem_free( rip_route, RIP_ROUTE_TYPE );
> 	rip_route=NULL;  /* fangqi add this */
> 	pprocess->rip2GlobalRouteChanges++;
> ```
>
> 



### rip_del_from_main_tbl

```c
void rip_del_from_main_tbl( struct rip_route_ *rip_route ,uint32 processid)
```

```c
// 从主路由表中删除路由
// 主路由表中只允许缺省4条，最大8条等价路由，因此，此处的尽管rip可能存在多条等价路由，也只能添加4或者8条进入主路由表
void rip_del_from_main_tbl( struct rip_route_ *rip_route ,uint32 processid);
```

> 1.从等价路由表中删除路由信息
>
> ```c
> rt_delete_protort_notify( rip_route->pprocess->vrf_id,
> 							rip_route->route_node->p.u.prefix4.s_addr,
> 							prefix_to_mask(rip_route->route_node->p.prefixlen),
> 						      RTPROTO_RIP,
> 						      processid );
> ```
>
> 2.检查是否存在其他等价路由，若有，则再加入主路由表中
>
> ```c
> if(( rip_route_head == NULL) || ( rip_route_head->equi_route_num == 0 )){
> 	return;
> }
> equal_rtnum = rt_get_equal_rtnum_confg();    // 4
> rip_gateway_temp = ( struct rt_gateway *)rip_mem_malloc( sizeof(struct rt_gateway)*equal_rtnum, RIP_RT_GATEWAY_TYPE );
> if(!rip_gateway_temp)return;
> bzero( rip_gateway_temp, sizeof(struct rt_gateway)*equal_rtnum);
> ```
>
> 3.向路由表中添加等价路由
>
> ```c
> gateway_num = 0;	
> for( count = 1, rip_route_temp = rip_route_head; count <= rip_route_head->equi_route_num; rip_route_temp = rip_route_temp->forw, count++ )
> {	
>     /*not add metric = 16 rip route to rt table*/
>     if( (RIP_NBR_ROUTE == rip_route_temp->route_type)
>         && (rip_route_temp->distance >= RIP_MAX_DISTANCE || rip_route_temp->metric >= RIP_MAX_METRIC)) 
>     {
>         continue;
>     }
> 
>     /*Fengsb 2006-02-19 注意，只能向主路由表添加学习到的路由，
>     因为转发进来的路由放在route node的第一个节点，所以需要进行
>     判断*/
>     if(RIP_REDIS_ROUTE == rip_route_temp->route_type)
>     {
>         continue;
>     }
>     if(RIP_SUMMARY_ROUTE == rip_route_temp->route_type)
>     {
>         continue;
>     }
>     /*fengsb add the following  if case 2006-05-18*/
>     if(RIP_DEF_ROUTE == rip_route_temp->route_type)
>     {
>         continue;
>     }
> /*		memset( (void *)&rip_gateway_temp[gateway_num], 0, sizeof(struct rt_gateway) ); */
> 
>     if( rip_route_temp->next_hop != 0 )
>     {
>         rip_gateway_temp[gateway_num].gateway = rip_route_temp->next_hop;
>     }
>     else
>     {
>         rip_gateway_temp[gateway_num].gateway = rip_route_temp->gw_addr;
>     }
> 
>     rip_gateway_temp[gateway_num].flags = 1;
>     if(rip_intf_array[rip_route_temp->gw_index])
>         rip_gateway_temp[gateway_num].aid = rip_intf_array[rip_route_temp->gw_index]->aid;
>     rip_gateway_temp[gateway_num].index   = rip_route_temp->gw_index;
>     rip_gateway[gateway_num] = &rip_gateway_temp[gateway_num];
>         
>     /*Fengsb 2006-02-13, 不能超过主路由表允许的等价路由数目*/
>     gateway_num++;
>     if(gateway_num >= equal_rtnum)
>     {
>         break;
>     }
> }
> 
> if(gateway_num > 0)
> {
>     rt_add_protort_notify( rip_route->pprocess->vrf_id, 
>                         rip_route->route_node->p.u.prefix4.s_addr,
>                         prefix_to_mask(rip_route->route_node->p.prefixlen),
>                         RTPROTO_RIP,
>                         gateway_num,
>                         rip_gateway,
>                         0,
>                         NULL,
>                         RTS_INTERIOR|RTS_ELIGIBLE,
>                         rip_route->metric,
>                         rip_route->distance,
>                         rip_route->route_tag,
>                         processid, 0);
> }
> rip_mem_free(rip_gateway_temp, RIP_RT_GATEWAY_TYPE);
> ```
>
> 

## rip_packet.c

### rip_recv_packet

```c
int rip_recv_packet( uint32 pkt_len );
```

> 1.对报文长度参数进行合法性验证
>
> ```c
> if( pkt_len > RIP_MAX_PACKET_SIZE )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: Ignored RIP packet(Illegal packet size).\n" );
>     return RIP_FAIL;
> }
> ```
>
> 2.从socket的接收缓冲区中接收数据
>
> ```c
> struct soaddr_in src_socket;
> int temp_len;
> 
> memset( rip_recv_buffer, 0, RIP_MAX_PACKET_SIZE );
> temp_len = sizeof(struct soaddr);
> recv_pkt_len = so_recvfrom( rip_glb_info.socket_id, rip_recv_buffer, pkt_len, 0, (struct soaddr *)(&src_socket), &temp_len );
> ```
>
> 3.获取发送数据包的设备端口id，并检查合法性
>
> ```c
> src_socket_intf = (struct soaddr_intf *)(&src_socket);
> device_index = (uint32)(src_socket_intf->sin_ifno);
> if( device_index > INTERFACE_DEVICE_MAX_NUMBER )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d ( %d).\n",__FILE__,__LINE__, device_index);
>     return RIP_FAIL;
> }
> ```
>
> 4.检查端口上是否开启了RIP进程
>
> ```c
> if(!(pintf = rip_intf_array[device_index]) || (pintf->process_id == 0) )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>     return RIP_FAIL;
> }
> ```
>
> 5.检查端口是否接收RIP报文
>
> ```c
> // 检查其是否是不接受rip报文的接口
> if(BIT_TEST( rip_intf_array[device_index]->special_flag,RIP_NOTRECV_ENABLE))
> {
>     rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: Ignored RIP packet from %s(deaf interface).\n", ip_ntoa(src_socket_intf->sin_addr.s_addr));
>     return RIP_FAIL;		
> }
> 
> /*接收报文端口链路状态必须是UP,否则不处理*/
> if(!BIT_TEST( pintf->state ,RIP_INTF_PROCESS_ACTIVE))
> {
>     pintf->rip2IfStatRcvBadPackets++;
>     if(peer != &(pprocess->peer_list))peer->rip2PeerRcvBadPackets++;
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>     return RIP_FAIL;
> }
> ```
>
> 6.获取端口上的RIP进程
>
> ```c
> if(!(pprocess = pintf->pprocess))
> {
>     rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: %s %d.\n",__FILE__,__LINE__ );
>     return RIP_FAIL;
> }
> ```
>
> 7.根据报文的源ip地址吗，从端口RIP进程下的邻居节点链表查找相应节点
>
> ```c
> peer = pprocess->peer_list.forw;
> while(peer != &(pprocess->peer_list))
> {
>     if(peer->peer_addr == src_socket_intf->sin_addr.s_addr)
>         break;
>     peer = peer->forw;
> }
> if(peer != &(pprocess->peer_list))peer->rip2PeerRcvPackets++;/*add 2019.8.30 peer收到的数据包数目*/
> pintf->rip2IfStatRecvUpdates++;/*add 2019.8.30intf收到的数据包数目*/
> ```
>
> 8.对接收到的报文进行合法性验证
>
> ```c
> ret = rip_packet_check( device_index, &src_socket );
> if( RIP_SUCCESS != ret )
> {
>     pintf->rip2IfStatRcvBadPackets++;
>     if(peer != &(pprocess->peer_list))peer->rip2PeerRcvBadPackets++;
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>     return ret ;
> }
> ```
>
> 9.若没找的邻居节点，则新建一个
>
> ```c
> if(peer == &(pprocess->peer_list))  ？？？
> {
>     peer = rip_mem_malloc(sizeof(struct rip_peer_list_), RIP_PEER_LIST_TYPE);
>     if(peer==NULL)return RIP_FAIL;
>     memset((void *)peer, 0, sizeof(struct rip_peer_list_));
>     peer->peer_addr = src_socket_intf->sin_addr.s_addr;
>     peer->peer_port = src_socket_intf->sin_port;
>     peer->peer_intf = device_index;   // 
>     INSQUE(peer, pprocess->peer_list.back);
> 
>     if(pprocess->peer_timer_id && (sys_timer_state(pprocess->peer_timer_id) != PTIMER_RUNNING))
>     {
>         sys_start_timer(pprocess->peer_timer_id, pprocess->peer_timeout);
>     }
> }
> ```
>
> 10.存储邻居发送的更新路由信息
>
> ```
> /*记录端口的peer信息*/
> peer->rip2PeerVersion = rip_pkt_head->version;
> peer->peer_domain = rip_pkt_head->zero;
> peer->rip2PeerLastUpdate = time_sec;
> ```
>
> 11.根据command类型，处理请求或响应数据
>
> ```c
> switch( rip_pkt_head->cmd )
> {
>     case RIP_REQUEST:
>         ret = rip_recv_request( device_index, pkt_len, peer );
>         if( RIP_SUCCESS != ret )
>         {
>             return ret;
>         }
>         break;
>     case RIP_RESPONSE:
>         ret = rip_recv_response( device_index , pkt_len, peer );
>         if( RIP_SUCCESS != ret )
>         {
>             return ret;
>         }
>         break;
>     default:
>         break;				
> }
> ```
>
> 

#### rip_packet_check ？

```c
int rip_packet_check( uint32 device_index, struct soaddr_in *src_socket );
```

![image-20241009160721658](./Rip code.assets/image-20241009160721658.png)

> 1.获取报文头部
>
> rip_pkt_head = (struct rip_pkt_head_ *)rip_recv_buffer;
>
> 2.对报文头部字段进行检测
>
> ```c
> /*报文类型检测*/
> if( (RIP_REQUEST != rip_pkt_head->cmd) && (RIP_RESPONSE != rip_pkt_head->cmd) )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Illegal packet type)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr) );
>     return RIP_FAIL;
> }
> 
> /* 报文版本检测*/
> if( (RIP_VERSION_1 != rip_pkt_head->version) && (RIP_VERSION_2 != rip_pkt_head->version) )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (illegal version)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr) );
>     return RIP_FAIL;
> }
> 
> /*零域检测*/
> if( 0 != rip_pkt_head->zero )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Illegal packet zero field)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr) );
>     return RIP_FAIL;
> }
> ```
>
> 3.对发送RIP的报文源进行检测
>
> ```c
> /* RIP响应报文的源端口必须是520*/
> if( (RIP_RESPONSE == rip_pkt_head->cmd) && (htons(RIP_PORT) != src_socket->sin_port) )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Illegal source port %d)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr), htons(src_socket->sin_port) );
>     return RIP_FAIL;
> }
> 
> /*报文源地址检测:报文源非本地地址*/
> if( rt_is_local_addr_vrf(src_socket->sin_addr.s_addr, rip_intf_array[device_index]->vrf_id, TRUE) )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Source address is local address)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr) );
>     return RIP_FAIL;
> }
> 
> /*报文源地址检测:报文源地址与接收端口应是同一网段
> 	   若端口是unnumbered（无IP地址）地址,则不进行源地址验证*/
> if( (BIT_TEST(pprocess->flags ,RIP_SRC_CHECK_ENABLE))
>     && (RIP_UNNUMBERED_ADDRESS != rip_intf_array[device_index]->addr_type) )
> {
>     /*非同一网段*/
>     if( (src_socket->sin_addr.s_addr & rip_intf_array[device_index]->mask ) !=  (rip_intf_array[device_index]->address &  rip_intf_array[device_index]->mask) )
>     {
>         /*点对点端口*/
>         if( rip_intf_array[device_index]->encap_type == INTERFACE_DEVICE_FLAGS_NET_POINTTOPOINT )
>         {
>             /*对点对点端口的特殊处理*/
>             natural_intf_network = rip_get_natural_network( rip_intf_array[device_index]->address );
>             natural_intf_mask = rip_get_natural_mask( rip_intf_array[device_index]->address );
>             if((src_socket->sin_addr.s_addr & natural_intf_mask) !=  (natural_intf_network & natural_intf_mask) )
>             {
>                 rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Source validate failed)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr) );
>                 return RIP_FAIL;
>             }
>         }
>         else
>         {
>             /*非点对点端口*/
>             rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Source validate failed)\n", rip_pkt_head->version, ip_ntoa(src_socket->sin_addr.s_addr) );
>             return RIP_FAIL;
>         }
>     }
> }
> ```
>
> 4.对报文版本进行检测
>
> ```c
> ret = rip_version_check( device_index, src_socket->sin_addr.s_addr );
> if( RIP_SUCCESS != ret )
> {
>     return RIP_FAIL;
> }
> ```
>
> 5.认证检测，认证成功再判断
>
> ```c
> ret = rip_auth_check( device_index, src_socket->sin_addr.s_addr );
> if( RIP_SUCCESS != ret )
> {
>     return RIP_FAIL; 
> }//表示该接口是否处于“认证提交”状态。它可能是一个定时器，用于监控邻居认证状态
> else if(rip_intf_array[device_index]->auth_commit_time) 
> {
>     struct rip_peer_list_ *peer = pprocess->peer_list.forw;
>     int cnt = 0;
>     while(peer != &(pprocess->peer_list))
>     {
>         if(peer->peer_addr==src_socket->sin_addr.s_addr)
>             peer->auth_state=0;
>         // 记录未通过认证的数量
>         else if((peer->peer_intf==device_index)&&peer->auth_state)cnt++; 
>         peer=peer->forw;
>     }
>     if(cnt==0)  ？？？
>     {
>         rip_intf_array[device_index]->auth_commit_time=0;
>         sys_stop_timer(rip_intf_array[device_index]->auth_commit_timer_id);
>     }
> }
> ```

#### rip_version_check

```c
// 检测端口能否接收该rip报文的版本号
int rip_version_check( uint32 device_index, uint32 src_addr )
```

> 1.获取报文头部的RIP版本号
>
> ```c
> rip_pkt_head = (struct rip_pkt_head_ *)rip_recv_buffer;
> recv_version = rip_get_recv_version( device_index );
> ```
>
> 2.根据版本号来判断能否接收
>
> ```c
> switch( recv_version )
> {
>     case RIP_RECV_VERSION_1:
>         if( RIP_VERSION_1 != rip_pkt_head->version )
>         {
>             rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Illegal version).\n", rip_pkt_head->version, ip_ntoa(src_addr) );
>             return RIP_FAIL;
>         }
>         break;
>     case RIP_RECV_VERSION_2:
>         if( RIP_VERSION_2 != rip_pkt_head->version )
>         {
>             rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Illegal version).\n", rip_pkt_head->version, ip_ntoa(src_addr) );
>             return RIP_FAIL;
>         }
>         break;
>     case RIP_RECV_VERSION_1_2:
>     case RIP_RECV_VERSION_DEF:
>         if( (RIP_VERSION_1 != rip_pkt_head->version) && (RIP_VERSION_2 != rip_pkt_head->version) )
>         {
>             rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Illegal version).\n", rip_pkt_head->version, ip_ntoa(src_addr) );
>             return RIP_FAIL;
>         }
>         break;
>     default:
>         return RIP_FAIL;
> }
> ```

#### rip_auth_check ？

```c
// 对接收到的报文进行认证
int rip_auth_check( uint32 device_index, uint32 src_addr );
```

<img src="./Rip code.assets/image-20240929163201100.png" alt="image-20240929163201100" style="zoom: 50%;" />

> 1.获取报文头部
>
> ```c
> rip_pkt_head = (struct rip_pkt_head_ *)rip_recv_buffer;
> rip_route_item = (struct rip_route_item_ *)(rip_recv_buffer + sizeof(struct rip_pkt_head_) );
> ```
>
> 2.如果收到的报文不需要认证，但本地端口开启了认证功能
>
> ```c
> if( htons(RIP_AUTH_AFI) != rip_route_item->afi )
> {
>     /*接收到非认证报文*/
>     // 但是端口开启了认证：
>     // 1.如果是明文认证，并且simple_key不为NULL且未开启邻居动态认证
>     // 2.如果是md5认证，并且md5_key不为NULL且未开启邻居动态认证
>     // 3.如果是邻居间动态认证，并且开启了邻居动态认证
>     if( ((rip_intf_array[device_index]->auth_type == RIP_AUTH_SIMPLE)
>                 && (0 != strlen(rip_intf_array[device_index]->simple_key))&&(!rip_intf_array[device_index]->dynamicflag) )
>             || ( (rip_intf_array[device_index]->auth_type == RIP_AUTH_MD5)
>                 && (0 != strlen(rip_intf_array[device_index]->md5_key))&&(!rip_intf_array[device_index]->dynamicflag))
>                 || ( (rip_intf_array[device_index]->auth_type == RIP_AUTH_DYNAMIC) &&rip_intf_array[device_index]->dynamicflag))
>     {
>         // 第三种情况
>         if(rip_intf_array[device_index]->dynamicflag)
>         {
>             struct dynamic_key_list_ *key = rip_intf_array[device_index]->key_list.forw;
>             while(key != &(rip_intf_array[device_index]->key_list))
>             {
>                 // 若邻居key是正在生效的key或超时延期使用的key，则验证失败  ？？？
>                 if(key->key_state==RIP_NEIGHBOR_KEY_ACTIVE||key->key_state==RIP_NEIGHBOR_KEY_EXTENSIONUSE)
>                     break;
>                 key=key->forw;
>             }
>             if(key==&(rip_intf_array[device_index]->key_list))return RIP_SUCCESS;
>         }
> 		
>         // 若是RIPv1的报文，并且端口也是接收的RIPv1，则通过验证
>         if(RIP_VERSION_1 == rip_pkt_head->version)
>         {
>             ret = rip_get_recv_version(device_index);
>             if (RIP_RECV_VERSION_1 == ret) return RIP_SUCCESS;
>         }
> 
>         /*本地支持认证,接收到非认证报文,Ignored*/
>         rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
>         return RIP_FAIL;
>     }
> }
> ```
>
> 3.否则，若收到了认证报文，并且报文头部版本是RIPv2
>
> ```c
> if(RIP_VERSION_2 == rip_pkt_head->version)
> {
>     /*认证类型不一致*/
>     if( rip_intf_array[device_index]->auth_type != htons(rip_route_item->route_tag) )
>     {
>         rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
>         return RIP_FAIL;
>     }
>     else
>     {		
>         /*认证类型一致,但密钥不一致*/
>         if( RIP_AUTH_SIMPLE == rip_intf_array[device_index]->auth_type &&(!rip_intf_array[device_index]->dynamicflag) )
>         {
>             rip_simple_head = (struct rip_simple_head_ *)( rip_recv_buffer +sizeof(struct rip_pkt_head_) );
> 
>             if( 0 != memcmp(rip_intf_array[device_index]->simple_key, rip_simple_head->password, 16 ) )
>             {
>                 rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
>                 return RIP_FAIL;
>             }					
>         }
>         else if( RIP_AUTH_MD5 == rip_intf_array[device_index]->auth_type &&(!rip_intf_array[device_index]->dynamicflag)  )
>         {
>             rip_md5_head = (struct rip_md5_head_ *)( rip_recv_buffer+sizeof(struct rip_pkt_head_) );
>             rip_md5_tail = (struct rip_md5_tail_ *)( rip_recv_buffer + htons(rip_md5_head->packet_len) );
> 
>             /*check the time sequence */
>             ret = rip_auth_seq_check(device_index , src_addr, htonl(rip_md5_head->sequence_number) );
>             if(RIP_SUCCESS != ret)
>             {
>                 /*debug ip rip protocol*/
>                 rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
>                 return RIP_FAIL;
>             }
> 
>             packet_len = htons(rip_md5_head->packet_len) + rip_md5_head->authen_data_len;
> 
>             memcpy( password_md5, rip_md5_tail->password, 16 );
> 
>             memcpy( rip_md5_tail->password, rip_intf_array[device_index]->md5_key, 16);   /*fengsb add: it's no use 2006-01-25*/
> 
>             rt_md5_cksum( (byte *)rip_recv_buffer, packet_len, packet_len, rip_md5_tail->password, (uint32 *)0 );
> 
>             if( (memcmp(rip_md5_tail->password, password_md5, 16) != 0)
>                 || (rip_intf_array[device_index]->md5_keyid != rip_md5_head->md5_keyid))
>             {
>                 /*debug ip rip protocol*/
>                 rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (Authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
>                 return RIP_FAIL;
>             }
>         }
>         else if( RIP_AUTH_DYNAMIC== rip_intf_array[device_index]->auth_type  &&rip_intf_array[device_index]->dynamicflag )
>         {
>             key =  rip_intf_array[device_index]->key_list.forw;
>             while(key != & rip_intf_array[device_index]->key_list)
>             {
>                 if(key->key_state ==RIP_NEIGHBOR_KEY_ACTIVE||key->key_state ==RIP_NEIGHBOR_KEY_EXTENSIONUSE)
>                 {
>                     activekey=1;
>                     break;
>                 }
>                 key =key->forw;
>             }
> 
>             if( !activekey)
>             {
>                 rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (dynamic authentication failed,no active key(%d)!)\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
>                 return RIP_FAIL;
>             }
>             authen_head = (struct rip_authen_head_ *)( rip_recv_buffer+sizeof(struct rip_pkt_head_));
>             authen_tail = (struct rip_authen_tail_ *)( rip_recv_buffer + htons(authen_head->packet_len) );
> 
>             ret = rip_intf_auth_seq_check( device_index, htonl(authen_head->sequence_number) );
>             if(RIP_SUCCESS != ret)
>             {
>                 /*debug ip rip protocol*/
>                 rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (dynamic authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
>                 return RIP_FAIL;
>             }
> 
>             packet_len = htons(authen_head->packet_len) + authen_head->authen_data_len;
> 
>             key =rip_intf_array[device_index]->key_list.forw;
>             while(key != &rip_intf_array[device_index]->key_list)
>             {
>                 if(key->key_id ==authen_head->keyid)
>                 break;
>                 key =key->forw;
>             }
> 
>             if(key == &rip_intf_array[device_index]->key_list)
>             {
>                 /*debug ip rip protocol*/
>                 rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (dynamic authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
>                 return RIP_FAIL;
>             }
>             if(key->key_state != RIP_NEIGHBOR_KEY_ACTIVE && key->key_state != RIP_NEIGHBOR_KEY_EXTENSIONUSE)
>             {
>                 /*debug ip rip protocol*/
>                 rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (dynamic authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
>                 return RIP_FAIL;
>             }
> 
> 
>             if(key->algorithms ==RIP_NEIGHB_MD5)
>             {
>                 memcpy( password_md5, authen_tail->key.md5,16);
>                 memcpy( authen_tail->key.md5, key->key, 16);
>                 rt_md5_cksum( (byte *)rip_recv_buffer, packet_len, packet_len, authen_tail->key.md5, (uint32 *)0 );
>                 if( (memcmp(authen_tail->key.md5, password_md5, 16) != 0))
>                 {
>                     /*debug ip rip protocol*/
>                     rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (dynamic authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
>                     return RIP_FAIL;
>                 }
>             }
>             else if(key->algorithms == RIP_NEIGHB_SHA1)
>             {
>                 memcpy( password_sha1, authen_tail->key.sha1,20);
>                 memcpy( authen_tail->key.sha1, key->key, 20);
>                 rip_sha1_cksum( (byte *)rip_recv_buffer, packet_len, packet_len, authen_tail->key.sha1, (uint32 *)0 );
>                 if( (memcmp(authen_tail->key.sha1, password_sha1, 20) != 0))
>                 {
>                     /*debug ip rip protocol*/
>                     rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "RIP: ignored V%d packet from %s (dynamic authentication failed(%d))\n", rip_pkt_head->version, ip_ntoa(src_addr) ,__LINE__);
>                     return RIP_FAIL;
>                 }
>             }
>             else return RIP_FAIL;
>         }
>     }
> }
> ```
>
> 

#### rip_auth_seq_check

> 感觉pprocess->md5_record记录的是上次收到消息的邻居路由器的ip和seq_num

```c
// 对认证报文的sequence number进行检测
// - src_address: 报文源地址
// - seq_number: 认证报文中的源sequence number
int rip_auth_seq_check(uint32 device_index, uint32 src_address, uint32 seq_number );
```

> 1.获取端口的RIP进程信息，并遍历其下的md5_record链表，根据报文源IP检测是否有相应的md5_record
>
> ```c
> struct rip_process_info_ *pprocess = rip_intf_array[device_index]->pprocess;
> for( rip_md5_record_temp = pprocess->md5_record.forw; rip_md5_record_temp != &(pprocess->md5_record); rip_md5_record_temp = rip_md5_record_temp->forw )
> {
>     if( rip_md5_record_temp->src_addr == src_address )
>     {
>         // 防止重放攻击
> 		// seq_number的作用是确保每个消息的顺序是唯一且线性递增的。
> 	 	// 如果一个恶意攻击者捕获到之前的合法消息并尝试重放，接收方可以通过比较序列号来识别该消息是否为重复消息并拒绝处理它
>         if( seq_number >= rip_md5_record_temp->sequence_number )
>         {
>             rip_md5_record_temp->sequence_number = seq_number;
>             return RIP_SUCCESS;
>         }
>         else
>         {
>             rip_md5_record_temp->sequence_number = seq_number;
>             return RIP_FAIL;
>         }
>     }
> }
> ```



### rip_recv_request

```c
//函数功能: RIP接收到request报文的处理函数
//输入参数: device_index : 接收到的报文的端口
//          pkt_len      : 接收到的报文长度
//          src_address  : 报文源地址
//          src_port     : 报文源端口
int rip_recv_request( uint32 device_index , uint32 pkt_len, struct rip_peer_list_ *peer);
```

> 1.输入参数的合法性校验
>
> ```c
> if( device_index > INTERFACE_DEVICE_MAX_NUMBER )
> {
>     return RIP_FAIL;
> }
> 	
> /* add by fangqi 2006-10-10*/
> if(rip_intf_array[device_index]==NULL)
> {
>     return RIP_FAIL;
> }
> ```
>
> 2.对request报文的处理
>
> ```c
> pprocess= rip_intf_array[device_index]->pprocess;
> /*获取报文头部*/
> rip_pkt_head = (struct rip_pkt_head_ *)rip_recv_buffer;
> packet_len = sizeof( struct rip_pkt_head_ );
> is_special_request = FALSE;
> request_entry_num = 0;
> 
> // 包头长度 + 
> while( (packet_len + sizeof(struct rip_route_item_)) <= pkt_len )
> {
>     rip_route_item = (struct rip_route_item_ *)(rip_recv_buffer + packet_len);
>     packet_len += sizeof(struct rip_route_item_);
> 	// 前面已经认证过了
>     if( RIP_AUTH_AFI == htons(rip_route_item->afi)  )
>     {
>         /*对报文中的认证信息不处理*/
>         continue;
>     }
> 
>     /* 判断是否是special request报文*/    
>     if( (0 == request_entry_num)   // 请求特定的路由条目，即请求整个路由表
>         && (0 == rip_route_item->afi)
>         && (RIP_MAX_METRIC == htonl(rip_route_item->metric)) )
>     {
>         is_special_request = TRUE;
>     }
> 
>     request_entry_num++;
> 	
>     // 请求的是整个路由表
>     if( TRUE == is_special_request )  
>     {
>         /*special request报文的处理*/
>         if( request_entry_num > 1)
>         {
>             /*special request报文只能有一个entry*/
>             rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, 
>                 "RIP: ignored V%d request packet from %s (illegal request)\n", rip_pkt_head->version, ip_ntoa(peer->peer_addr) );
>             return RIP_SUCCESS;
>         }
>         else
>         {
>             rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\taddress_family_identifier = %d\n", htons(rip_route_item->afi) );
>             rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, "\tmetric = %d\n",htonl(rip_route_item->metric) );
>         }
>     }
>     else/*( FALSE == is_special )*/   
>     {
>         // 请求获取是某些路由信息，路由表中不存在，则metric设置为16，否则，设为相应的值
>         /*非special request报文的处理*/
>         if( rip_glb_info.debug_flag & RIP_DEBUG_IP_RIP_PROTO_RECV )
>         {
>             sprintf(string, "\t%s/%d", ip_ntoa(rip_route_item->network), mask_to_prefix(rip_route_item->mask) );
>             sprintf(string, "%s via %s metric %lu", string, ip_ntoa(rip_route_item->next_hop), htonl(rip_route_item->metric));
> 
>             rip_debug(RIP_DEBUG_IP_RIP_PROTO_RECV, "%s\n", string );
>         }
> 
>         /*本地是否有相应的路由*/
>         rip_route = rip_get_route( rip_route_item, pprocess->rip_table);
>         if( NULL == rip_route )
>         {
>             /*本地没有改路由,entry的metric值添16*/
>             rip_route_item->metric = htonl(RIP_MAX_METRIC);
>         }
>         else
>         {
>             /*本地有改路由,entry的metric值为找到的改路由的metric值*/
>             rip_route_item->metric = htonl(min((rip_route->metric + 1), RIP_MAX_METRIC));
>         }
>     }
> };
> ```
>
> 3.request报文中没有请求路由条目,则不作回应
>
> ```c
> if( 0 == request_entry_num )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, 
>         "RIP: ignored V%d request packet from %s (illegal request)\n", rip_pkt_head->version, ip_ntoa(peer->peer_addr) );
> 
>     return RIP_SUCCESS;
> }
> ```
>
> 4.检测是否是被动接口，即只接收请求，但不做响应
>
> ```c
> if(BIT_TEST(rip_intf_array[device_index]->special_flag ,RIP_PASSIVE_ENABLE)
>     && !rip_is_neigh_addr(pprocess , peer->peer_addr))
> {
>     rip_debug( RIP_DEBUG_IP_RIP_PROTO_RECV, 
>         "RIP: ignored V%d request packet from %s (interface is passive)\n", rip_pkt_head->version, ip_ntoa(peer->peer_addr) );
>     return RIP_SUCCESS;
> }
> ```
>
> 5.发送响应
>
> ```c
> if( TRUE == is_special_request )
> {
>     /*接收到special request报文,发送本地所有路由*/
>     rip_send_response( device_index, peer->peer_addr, peer->peer_port);		
> }
> else if(rip_intf_array[device_index]->auth_type==RIP_AUTH_NONE
> 			||rip_intf_array[device_index]->auth_type==RIP_AUTH_SIMPLE)
> {	
>     /*接收到entry by entry request报文,回应相应的response报文*/
>     rip_pkt_head->cmd = RIP_RESPONSE;
> 
>     ret = rip_send_packet( device_index, peer->peer_addr, peer->peer_port, rip_recv_buffer, pkt_len );
>     if( RIP_SUCCESS != ret )
>     {
>         rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>         return ret;
>     }
> }
> ```
>
> 





### rip_send_response

```c
//函数功能: 发送response报文的处理函数
//输入参数: device_index : 发送报文的端口
//         dest_addr : 发送报文的目的地址
//         dest_port : 发送报文的目的端口
void rip_send_response( uint32 device_index, uint32 dest_addr, uint16  dest_port);
```

> 1.填充响应报文头部
>
> rip_create_pkt_head( device_index, RIP_RESPONSE );
>
> 2.检测端口是否开启的路由汇总功能
>
> ```c
> // 若开启了自动汇总，在构建要发送的RIP数据包时，将合并特定网络的路由信息
> if(TRUE == rip_enable_auto_summary(pprocess,device_index))
> {		
>     head_node = route_top( pprocess->rip_table);
>     rip_route_node = head_node;
>     label:while(NULL != rip_route_node)
>     {
>         rip_rthead = rip_route_node->info;
> 
>         if(NULL != rip_rthead )
>         {
>             prefixlen = rip_route_node->p.prefixlen;
> 			// 如果是默认路由
>             if (prefixlen ==0)  /*default route node */
>             {
>                 rip_route_item.mask =0;
>                 goto ADDPACKET;
>             }
>             else
>             {
>                 if (rip_get_natural_network (rip_route_node->p.u.prefix4.s_addr)
>                     == rip_get_natural_network (rip_intf_array[device_index]->address) ) /*主网相同*/
>                 {
>                     if( rip_rthead->route_type !=RIP_SUMMARY_ROUTE )
>                     {
>                         if(send_version == RIP_SEND_VERSION_2_BROADCAST || send_version == RIP_SEND_VERSION_2_MULTICAST)
>                         {
>                             /****版本2  下,只要自然网络相同均发送*/
>                             rip_route_item.mask = prefix_to_mask (rip_rthead->route_node->p.prefixlen);
>                             goto ADDPACKET;
>                         }
>                         else
>                         {
>                             if (prefixlen ==32 ||(prefix_to_mask(prefixlen)==(rip_intf_array[device_index]->mask)) )
>                             {
>                                 rip_route_item.mask =0;
>                                 goto ADDPACKET;
>                             }
>                             else
>                             {
>                                 rip_route_node = route_next (rip_route_node);
>                                 goto label;
>                             }							
>                         }
>                     }
>                     else   /*主网相同的summary 路由不发送*/
>                     {
>                         rip_route_node =route_next(rip_route_node);
>                         goto label;
> 
>                     }
>                 }
>                 else
>                 {   	/*主网不相同*/
> 
>                     goto ADDSUMPACKET;
> 
>                 }
>             }
>         }
>         else
>         {
>             /*当前结点为空*/
>             rip_route_node = route_next (rip_route_node);
>             goto label;
>         }
>     ADDPACKET:      /*添加路由信息*/
>         rip_route_item.network = rip_rthead->route_node->p.u.prefix4.s_addr; // 路由的
>         rip_add_route_to_pkt(device_index, dest_addr, dest_port, rip_rthead, &rip_route_item);
>         rip_route_node=route_next(rip_route_node);
> 
>         goto label;
>     ADDSUMPACKET: /*添加汇聚路由信息*/
>         rip_route_item.network=rip_get_natural_network(rip_rthead->route_node->p.u.prefix4.s_addr);
>         if(send_version==RIP_SEND_VERSION_2_BROADCAST|| send_version==RIP_SEND_VERSION_2_MULTICAST)
>         {
>             rip_route_item.mask = rip_get_natural_mask(rip_route_item.network);
>         }
>         else rip_route_item.mask =0;
>         rip_add_route_to_pkt(device_index,dest_addr, dest_port, rip_rthead, &rip_route_item);
>         rip_route_node=route_next_skip_child(rip_route_node);
>         goto label;
>     }
> }
> ```
>
> 3.如果没有开启路由汇总
>
> ```c
> for( rip_route_node = route_top( pprocess->rip_table); rip_route_node != NULL; )
> {
>     rip_rthead = rip_route_node->info;
>     if( NULL != rip_rthead )
>     {
>         rip_no_auto_sum_updt( device_index, dest_addr, dest_port, rip_rthead );
>     }
>     rip_route_node = route_next(rip_route_node);
> }
> ```
>
> 

#### rip_create_pkt_head

```c
void rip_create_pkt_head( uint32 device_index, uint8 cmd_type )
```

> 1.构造rip_pkt_head结构体，并填充数据包头部字段
>
> ```c
> rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;
> rip_pkt_head->cmd = cmd_type;
> // 获取端口设置的send_version
> send_version = rip_set_send_version( device_index );
> if( send_version == RIP_SEND_VERSION_1 )
> {
>     /*版本1*/
>     rip_pkt_head->version = RIP_VERSION_1;
> }
> else if( (send_version == RIP_SEND_VERSION_2_BROADCAST)
>         || (send_version == RIP_SEND_VERSION_2_MULTICAST))
> {
>     /*版本2*/
>     rip_pkt_head->version = RIP_VERSION_2;
> }
> else
> {
>     /*其他情况,版本2(暂时)*/
>     rip_pkt_head->version = RIP_VERSION_2;
> }	
> ```
>
> 2.若是填充Request请求头，会根据端口设置的特殊标志，设置特定rip version版本
>
> ```c
> if(cmd_type == RIP_REQUEST)
> {
>     if(BIT_TEST(rip_intf_array[device_index]->special_flag, RIP_SEND_V1DEMAND))
>         rip_pkt_head->version = RIP_VERSION_1;
>     else if(BIT_TEST(rip_intf_array[device_index]->special_flag, RIP_SEND_V2DEMAND))
>         rip_pkt_head->version = RIP_VERSION_2;
> }
> 
> /*零域*/
> rip_pkt_head->zero = 0;
> ```
>
> 3.若是认证报文，需要填充认证信息
>
> ```c
> if( RIP_VERSION_2 == rip_pkt_head->version )
> {
>     if( (RIP_AUTH_SIMPLE == rip_intf_array[device_index]->auth_type)
>         && (0 != strlen(rip_intf_array[device_index]->simple_key)) )
>     {
>         /*明文认证*/
>         rip_simple_head = (struct rip_simple_head_ *)( rip_send_buffer + send_pkt_len );
>         rip_simple_head->afi = htons(RIP_AUTH_AFI);
>         rip_simple_head->authen_type = htons(RIP_AUTH_SIMPLE);
>         /*strcpy(rip_simple_head->password, rip_intf_array[device_index]->simple_key);*/
>         memcpy(rip_simple_head->password, rip_intf_array[device_index]->simple_key, 16);
> 
>         send_pkt_len += sizeof(struct rip_simple_head_);			
>     }
>     else if( (RIP_AUTH_MD5 == rip_intf_array[device_index]->auth_type)
>         && (0 != strlen(rip_intf_array[device_index]->md5_key)) &&(!rip_intf_array[device_index]->dynamicflag))
>     {
>         /*MD5认证*/
>         rip_md5_head = (struct rip_md5_head_ *)( rip_send_buffer + send_pkt_len );
>         rip_md5_head->afi = htons(RIP_AUTH_AFI);
>         rip_md5_head->authen_type = htons(RIP_AUTH_MD5);
>         rip_md5_head->md5_keyid = rip_intf_array[device_index]->md5_keyid;
>         rip_md5_head->sequence_number = htonl(time_sec);
> 
>         send_pkt_len += sizeof(struct rip_md5_head_);
>     }
>      else if(RIP_AUTH_DYNAMIC== rip_intf_array[device_index]->auth_type && rip_intf_array[device_index]->dynamicflag)
>      {
>         key =rip_intf_array[device_index]->key_list.forw;
>         while(key != &rip_intf_array[device_index]->key_list)
>         {
>             if(key->key_state == RIP_NEIGHBOR_KEY_ACTIVE ||key->key_state == RIP_NEIGHBOR_KEY_EXTENSIONUSE)
>                 keynum++;
>             key = key->forw;
>         }
> 
>         if(keynum == 0)
>             return;
>         else
>         {
>             /*选取随机的active的key*/
>             srand( time(NULL));
>             keyid =rand()%keynum;
>         }
> 
>         key =rip_intf_array[device_index]->key_list.forw;
>         keynum =0;
>         while(key != &rip_intf_array[device_index]->key_list)
>         {
>             if(key->key_state == RIP_NEIGHBOR_KEY_ACTIVE ||key->key_state == RIP_NEIGHBOR_KEY_EXTENSIONUSE)
>             {
>                 if(keynum == keyid)break;
>                 keynum++;
>             }
>             key = key->forw;
>         }
>         authen_head = (struct rip_authen_head_ *)( rip_send_buffer + send_pkt_len );
>         authen_head->afi = htons(RIP_AUTH_AFI);
>         authen_head->authen_type = htons(RIP_A UTH_MD5);
>         authen_head->sequence_number = htonl(time_sec);
>         authen_head->keyid = key->key_id;
> 
>         send_pkt_len += sizeof(struct rip_authen_head_);
> 
>      }
> 
> }
> ```
>
> 



##### rip_loopup_peer_version

```c
// 获取对端的send_version
// * PARAMS:	device_index
// * RETURN:	0:没有合适的version
// 				非0:要填充到head中的version
// *
// * NOTE:	当端口对应的peer为一个或者对应多个peer但多个peer的version一致，返回他们他们共同的version
// 			否则,不修改head中的version
int rip_loopup_peer_version(uint32 device_index);
```

> 1.获取接口的rip进程信息
>
> ```c
> if(rip_intf_array[device_index] == NULL)
> 		return 0;
> 		
> if(!(pprocess = rip_intf_array[device_index]->pprocess))
> {
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>     return RIP_FAIL;
> }
> ```
>
> 2.查询接口的对端的send_version
>
> ```c
> peer = pprocess->peer_list.forw;
> while(peer != &pprocess->peer_list)
> {
>     if(peer->peer_intf == device_index)
>     {
>         peer_num ++;
>         // 若出现多个邻居端口对一，并且版本存在不一致情况，需设置为默认版本
>         if(peer_num > 1)
>         {
>             if(version != peer->rip2PeerVersion)
>             {
>                 version_same = FALSE;
>                 break;
>             }
>         }
>         else
>             version = peer->rip2PeerVersion;
>     }
>     peer = peer->forw;
> }
> // 若版本都一致，需设置邻居的版本
> if(peer_num > 0 && version_same == TRUE )
> 	return version;
> else
>     return 0;
> ```

#### rip_add_route_to_pkt

```c
/*===========================================================
函数名: rip_add_route_to_pkt
函数功能: 将路由加入报文
输入参数: device_index : 发送报文的端口
		  dest_addr : 报文目的地址
		  dest_port : 报文目的端口
		  rip_route_info : 路由信息
备注:
===========================================================*/
void rip_add_route_to_pkt( uint32 device_index, uint32 dest_addr, uint16 dest_port, struct rip_route_ * rip_route,  struct rip_route_item_ *rip_route_item)
```

> 1.初始化路由信息
>
> ```c
> struct rip_route_info_ rip_route_info;
> memset( &rip_route_info, 0, sizeof(struct rip_route_info_) );
> 
> rip_route_info.gw_addr = rip_route->gw_addr;
> rip_route_info.gw_index = rip_route->gw_index;
> rip_route_info.mask = prefix_to_mask(rip_route->route_node->p.prefixlen);
> rip_route_info.metric = rip_route->metric;
> rip_route_info.network = rip_route->route_node->p.u.prefix4.s_addr;
> rip_route_info.next_hop = rip_route->next_hop;
> rip_route_info.route_tag = rip_route->route_tag;
> rip_route_info.route_type = rip_route->route_type;
> ```
>
> 2.对即将发送的路由进行水平分割
>
> ```
> /*对发送出去的路由进行水平分割过滤*/
> ret = rip_split_check( device_index, &rip_route_info );
> if( RIP_SUCCESS != ret )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>     return;
> }
> ```
>
> 3.对发送出去的路由进行filter out过滤
>
> ```c
> if(rip_route->route_type==RIP_NBR_ROUTE)
> 		gateway = (rip_route->next_hop)?(rip_route->next_hop):(rip_route->gw_addr);
> 
> /*对发送出去的路由进行filter out过滤*/
> ret = rip_filter_out_check( device_index, gateway, rip_route_item );
> if( RIP_SUCCESS != ret )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>     return;
> }
> ```
>
> 4.构造路由条目rip_route_item，并对字段进行检查
>
> ```c
> rip_route_item->afi = htons(AF_INET);
> /*metric*/
> if( RIP_REDIS_ROUTE == rip_route_info.route_type )
> {
>     /*Fengsb 2006-02-17 shielded Yuhuiping's code 
>     转发路由的metric之前就已经设置。yuhuiping的代码只考虑了添加转发路由
>     的情况，删除时metric应该为16
>     rip_route_item.metric = rip_tbl_array[vrf_id]->default_metric;
>     */	
>     /*fangqi modify this 2006-11-23  */
>     /*转发路由的metric值应该为default-metric*/
>     /*至于检测其metric值是否大于RIP_MAX_METRIC,则由rip_offset_out_check来完成*/
>     rip_route_item->metric = htonl((rip_route->rmap_set_metric)?(rip_route_info.metric):((uint32)(pprocess->default_metric)));		
> }
> else
> {
>     rip_route_item->metric = htonl(rip_route_info.metric + 1);
> }
> 
> /*对发送出去的路由进行offset out信息控制, metric*/
> rip_offset_out_check( device_index, rip_route_item );
> 
> /*对路由条目的next-hop进行检查, next-hop*/
> rip_next_hop_check( device_index, &rip_route_info, rip_route_item );
> 
> rip_pkt_head = (struct rip_pkt_head_ *)rip_send_buffer;
> /*route-tag*/
> if( RIP_VERSION_1 == rip_pkt_head->version )
> {
>     rip_route_item->route_tag = 0;
> }
> else if( RIP_VERSION_2 == rip_pkt_head->version )
> {
>     rip_route_item->route_tag = htons(rip_route_info.route_tag);
> }
> ```
>
> 5.对于从邻居学习到的路由，如果是未被汇总的，而本地rip缺省为自动汇总，会导致每条路由都汇总一次，最后会有多条相同的汇总路由被通告出去而可能metric不一致，导致路由振荡。
>
> ```c
> //本处理可以进行部分限制，只发送一条汇总后的邻居路由
> ret = rip_nbrrt_check_to_pkt(device_index, rip_route_item);
> if( RIP_SUCCESS != ret )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>     return;
> }
> ```
>
> 6.add bfd 联动rip，bfd检测端口down,rip 发送全部16跳条目
>
> ```c
> /*add bfd 联动rip，bfd检测端口down,rip 发送全部16跳条目*/
> if(bfd_max_metric == 1)
> 	rip_route_item->metric = RIP_MAX_METRIC;
> ```
>
> 7.将路由条目加入报文
>
> ```
> /*将路由条目加入报文*/
> rip_add_item_to_pkt( device_index, dest_addr, dest_port, rip_route_item );	
> ```
>
> 

##### rip_split_check

```c
/*===========================================================
函数功能: 检查端口发送出去的路由是否被水平分割禁止发送
输入参数: device_index : 发送报文的端口
         rip_route_info : 待检查的路由信息
输出参数: 无
返回值:    若报文能通过水平分割过滤,返回RIP_SUCCESS
          若报文不能通过水平分割过滤,返回RIP_FAIL
备注:
===========================================================*/
int rip_split_check( uint32 device_index, struct rip_route_info_ *rip_route_info );
```

> 1.如果端口类型为一对多，则通过；表示水平分割机制不适用，允许路由发送
>
> ```c
> if( INTERFACE_DEVICE_FLAGS_NET_POINTTOMULTI == rip_intf_array[device_index]->encap_type )
> 		return RIP_SUCCESS;
> ```
>
> 2.检查该路由信息是否是从该接口收到的
>
> ```
> if( (rip_route_info->gw_index == device_index)||  // 路由条目从该接口收到的
> 		( (RIP_REDIS_ROUTE == rip_route_info->route_type) &&  // 重分发路由
> 		   (rip_route_info->network == (rip_intf_array[device_index]->address & rip_intf_array[device_index]->mask ))
> 		   && (rip_route_info->mask == rip_intf_array[device_index]->mask) )  )  // 这条路由
> {
>     if(rip_intf_array[device_index]->split_flag & RIP_POISONED_SPLIT_HORIZON_ENABLE)
>     {
>         /* 毒素分割启动，metric返回16  */
>         rip_route_info->metric = RIP_MAX_METRIC;
>         return RIP_SUCCESS;
>     }
>     else if ((rip_intf_array[device_index]->split_flag & RIP_SIMPLE_SPLIT_HORIZON_ENABLE) ==0)
>     {
>         /*毒素逆转没启动,水平分割禁止.可以发送相关路由*/
>         return RIP_SUCCESS;
>     }
> 
>     return RIP_FAIL;
> }
> return RIP_SUCCESS;
> ```
>
> 

##### rip_filter_out_check

```c
/*===========================================================
函数名: rip_filter_out_check
函数功能: 对发送出去的路由进行filter out过滤检查
输入参数: device_index : 发送报文的端口
         dest_addr : 发送报文的目的地址
         rip_route_item:待发送出去的路由条目
输出参数: 无
返回值:   若报文能通过filter 过滤,返回RIP_SUCCESS
         若报文不能通过filter 过滤,返回RIP_FAIL
===========================================================*/
int rip_filter_out_check( uint32 device_index, uint32 gateway, struct rip_route_item_ *rip_route_item )
```

##### rip_offset_out_check

```c
/*===========================================================
函数名: rip_offset_out_check
函数功能: 对接收到的路由路由进行offset信息控制
输入参数: device_index : 发送报文的端口
         rip_route_item : 发送的路由条目
输出参数: 无
返回值:   无
备注:
===========================================================*/
int rip_offset_out_check( uint32 device_index,struct rip_route_item_ *rip_route_item )
```

##### rip_next_hop_check

```c
/*===========================================================
函数名: rip_next_hop_check
函数功能: 对发送出去路由的next-hop信息控制
输入参数:  device_index : 发送报文端口
          rip_route_info : 待发送出去的路由信息
          rip_route_item : 待发送出去的路由条目
输出参数: 无
返回值:   无
备注:
===========================================================*/
void rip_next_hop_check( uint32 device_index, struct rip_route_info_ *rip_route_info, struct rip_route_item_ *rip_route_item )
```

> ```c
> if( RIP_VERSION_1 == rip_pkt_head->version )
> {
>     rip_route_item->next_hop = 0;
> }
> else if( RIP_VERSION_2 == rip_pkt_head->version )
> {
>     if( INTERFACE_DEVICE_FLAGS_NET_POINTTOPOINT != rip_intf_array[device_index]->encap_type )
>     {
>         /*非点对点端口*/
>         // 首先检查下一跳是否在本地网络段，如果不在，则检查网关地址；如果两者都不在，则设置下一跳为 0。
>         if((rip_route_info->next_hop & rip_intf_array[device_index]->mask) == (rip_intf_array[device_index]->address & rip_intf_array[device_index]->mask) )
>         {
>             rip_route_item->next_hop = rip_route_info->next_hop;
>         }
>         else if( (rip_route_info->gw_addr & rip_intf_array[device_index]->mask ) == (rip_intf_array[device_index]->address & rip_intf_array[device_index]->mask) )
>         {
>             rip_route_item->next_hop = rip_route_info->gw_addr;
>         }
>         else
>         {
>             rip_route_item->next_hop = 0;
>         }
>     }
>     else
>     {
>         /*点对点端口*/
>         rip_route_item->next_hop = 0;
>     }
> }
> 
> ```



##### rip_add_item_to_pkt

```c
/*===========================================================
函数名: rip_add_item_to_pkt
函数功能: 将路由条目加入报文
输入参数: device_index : 发送报文端口
	     dest_addr : 发送报文目的地址
	     dest_port : 发送报文的目的端口
		 rip_route_item : 待发送的路由条目
输出参数: 无
返回值:   无
备注:
===========================================================*/
void rip_add_item_to_pkt( uint32 device_index, uint32 dest_addr, uint16 dest_port, struct rip_route_item_ *rip_route_item );
```

> 1.若是RIPv2
>
> - md5验证
>
> ```c
> if( (RIP_VERSION_2 == rip_pkt_head->version)
> 		&& (RIP_AUTH_MD5 == rip_intf_array[device_index]->auth_type)
> 		&& (0 != strlen(rip_intf_array[device_index]->md5_key)) && (!rip_intf_array[device_index]->dynamicflag))
> {
>     /*MD5认证报文*/
>     if( (send_pkt_len + sizeof(struct rip_route_item_)) >= (RIP_MAX_PACKET_SIZE - sizeof(struct rip_md5_tail_)) )
>     {
>         /*填充MD5报文尾部*/
>         rip_md5_head = (struct rip_md5_head_ *)(rip_send_buffer + sizeof(struct rip_pkt_head_));
>         rip_md5_head->packet_len = htons(send_pkt_len);
> 
>         rip_md5_tail = (struct rip_md5_tail_ *)( rip_send_buffer + send_pkt_len);
>         rip_md5_tail->afi = htons(RIP_AUTH_AFI);
>         rip_md5_tail->route_tag = htons(RIP_MD5_TAIL_TAG);
> 
>         memcpy( rip_md5_tail->password, rip_intf_array[device_index]->md5_key, 16);
>         // auth_data_len
>         rip_md5_head->authen_data_len = sizeof(rip_md5_tail->afi) + sizeof(rip_md5_tail->route_tag) + sizeof(rip_md5_tail->password);
> 
>         send_pkt_len += rip_md5_head->authen_data_len;
> 		// 计算md5校验和
>         rt_md5_cksum( (byte *)rip_send_buffer, send_pkt_len, send_pkt_len, rip_md5_tail->password, (uint32 *)0 );
> 
>         /*发送报文*/
>         ret = rip_send_packet( device_index, dest_addr, dest_port, rip_send_buffer, send_pkt_len );
>         if( RIP_SUCCESS != ret )
>         {
>             rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>             return;
>         }
> 
>         offset = sizeof(struct rip_pkt_head_) + sizeof(struct rip_md5_head_);
>         memset( (rip_send_buffer + offset), 0, (RIP_MAX_PACKET_SIZE - offset) );
>         memcpy((rip_send_buffer + offset ), rip_route_item, sizeof(struct rip_route_item_) );
> 
>         /*发送报文长度复位*/
>         send_pkt_len = offset + sizeof(struct rip_route_item_);
>         send_item_len = sizeof(struct rip_pkt_head_) + sizeof(struct rip_route_item_);
>     }
>     else
>     {
>         /*累加发送报文长度*/
>         memcpy((rip_send_buffer + send_pkt_len ), rip_route_item, sizeof(struct rip_route_item_) );
>         send_pkt_len += sizeof(struct rip_route_item_);
>         send_item_len += sizeof(struct rip_route_item_);
>     }
> ```
>
> - 明文验证
>
>   ```c
>   else if( (RIP_VERSION_2 == rip_pkt_head->version)
>   		    && (RIP_AUTH_SIMPLE == rip_intf_array[device_index]->auth_type)
>   		    && (0 != strlen(rip_intf_array[device_index]->simple_key) ) )
>   {
>       if( (send_pkt_len + sizeof(struct rip_route_item_)) >  RIP_MAX_PACKET_SIZE )
>       {
>           /*发送报文*/
>           ret = rip_send_packet( device_index, dest_addr, dest_port, rip_send_buffer, send_pkt_len );
>           if( RIP_SUCCESS != ret )
>           {
>               rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>               return;
>           }
>   
>           offset = sizeof(struct rip_pkt_head_) + sizeof(struct rip_simple_head_);
>           memset( (void *)(rip_send_buffer + offset), 0, (RIP_MAX_PACKET_SIZE - offset) );
>           memcpy((rip_send_buffer + offset ), rip_route_item, sizeof(struct rip_route_item_) );
>   
>           /*发送报文长度复位*/
>           send_pkt_len = offset + sizeof(struct rip_route_item_);
>           send_item_len = sizeof(struct rip_pkt_head_) + sizeof(struct rip_route_item_);
>       }
>       else
>       {
>           memcpy((rip_send_buffer + send_pkt_len ), rip_route_item, sizeof(struct rip_route_item_) );
>   
>           /*累加发送报文长度*/
>           send_pkt_len += sizeof(struct rip_route_item_);
>           send_item_len += sizeof(struct rip_route_item_);
>       }
>   }
>   ```
>
> - 邻居间动态认证
>
>   ```c
>   else if( (RIP_VERSION_2 == rip_pkt_head->version)
>   		    && (RIP_AUTH_SIMPLE == rip_intf_array[device_index]->auth_type)
>   		    && (0 != strlen(rip_intf_array[device_index]->simple_key) ) )
>   {
>       if( (send_pkt_len + sizeof(struct rip_route_item_)) >  RIP_MAX_PACKET_SIZE )
>       {
>           /*发送报文*/
>           ret = rip_send_packet( device_index, dest_addr, dest_port, rip_send_buffer, send_pkt_len );
>           if( RIP_SUCCESS != ret )
>           {
>               rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>               return;
>           }
>   
>           offset = sizeof(struct rip_pkt_head_) + sizeof(struct rip_simple_head_);
>           memset( (void *)(rip_send_buffer + offset), 0, (RIP_MAX_PACKET_SIZE - offset) );
>           memcpy((rip_send_buffer + offset ), rip_route_item, sizeof(struct rip_route_item_) );
>   
>           /*发送报文长度复位*/
>           send_pkt_len = offset + sizeof(struct rip_route_item_);
>           send_item_len = sizeof(struct rip_pkt_head_) + sizeof(struct rip_route_item_);
>       }
>       else
>       {
>           memcpy((rip_send_buffer + send_pkt_len ), rip_route_item, sizeof(struct rip_route_item_) );
>   
>           /*累加发送报文长度*/
>           send_pkt_len += sizeof(struct rip_route_item_);
>           send_item_len += sizeof(struct rip_route_item_);
>       }
>   }
>   ```
>
> 2.如果是版本1
>
> ```c
> if( (send_pkt_len + sizeof(struct rip_route_item_)) >  RIP_MAX_PACKET_SIZE )
> {
>     /*发送报文*/
>     ret = rip_send_packet( device_index, dest_addr, dest_port, rip_send_buffer, send_pkt_len );
>     if( RIP_SUCCESS != ret )
>     {
>         rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>         return;
>     }
> 
>     offset = sizeof(struct rip_pkt_head_);
>     memset( (void *)(rip_send_buffer + offset), 0, (RIP_MAX_PACKET_SIZE - offset) );
>     memcpy((rip_send_buffer + offset ), rip_route_item, sizeof(struct rip_route_item_) );
> 
>     /*发送报文长度复位*/
>     send_pkt_len = offset + sizeof(struct rip_route_item_);
>     send_item_len = sizeof(struct rip_pkt_head_) + sizeof(struct rip_route_item_);
> }
> else
> {
>     memcpy((rip_send_buffer + send_pkt_len ), rip_route_item, sizeof(struct rip_route_item_) );
> 
>     /*累加发送报文长度*/
>     send_pkt_len += sizeof(struct rip_route_item_);
>     send_item_len += sizeof(struct rip_route_item_);
> }
> ```
>
> 



#### rip_no_auto_sum_updt ？？

```c
/*===========================================================
函数名: rip_no_auto_sum_updt
函数功能: 对发送出去路由路由不进行auto-summary过滤
输入参数: device_index : 发送报文端口
         rip_rthead : 待发送出去的路由信息
输出参数: rip_route_item : 待发送出去的路由条目
返回值:    路由通过auto-summary过滤,返回RIP_SUCCESS
          路由不能通过auto-summary过滤,返回RIP_FAIL
备注:		禁止自动汇总时的常规更新
===========================================================*/
int rip_no_auto_sum_updt(uint32 device_index, uint32 dest_addr, uint16 dest_port, struct rip_route_ * rip_rthead)
```

> 1.填充数据包中ip address、mask字段
>
> ```c
> struct rip_route_item_ rip_route_item;
> memset( &rip_route_item, 0, sizeof(struct rip_route_item_) );
> rip_route_item.network = rip_rthead->route_node->p.u.prefix4.s_addr;
> if( RIP_VERSION_1 == rip_pkt_head->version )
> {
>     rip_route_item.mask = 0;
> }
> else if( RIP_VERSION_2 == rip_pkt_head->version )
> {
>     rip_route_item.mask = prefix_to_mask(rip_rthead->route_node->p.prefixlen);
> }	
> ```
>
> 2.根据路由信息的类型，进行填充
>
> ```c
> // 若是汇总路由，则不发送
> if(rip_rthead->route_type == RIP_SUMMARY_ROUTE)
> {   /*在no auto sum 的时候，汇总路由不发，但是要发送从邻居学习到的路由*/
>     if(rip_rthead->equi_route_num > 1)
>     {
>         rip_route = rip_rthead->forw; 
>         for( count = 1; count < rip_rthead->equi_route_num; rip_route = rip_route->forw, count++ )
>         {
>             // 寻找其中的一个路由，其下一跳通过当前设备
>             if(rip_route->gw_index == device_index)
>             {
>                 rip_rthead = rip_route;
>                 break;
>             }
>         } 
>         if(rip_rthead->route_type == RIP_SUMMARY_ROUTE)rip_rthead=rip_rthead->forw;
>         rip_add_route_to_pkt( device_index, dest_addr, dest_port, rip_rthead, &rip_route_item );
>     }				
> }
> ```
>
> 







#### rip_send_packet

```c
/*===========================================================
函数名: rip_send_packet
函数功能: 发送RIP报文
输入参数: device_index : 发送报文的端口
          dest_addr : 发送报文的目的地址
          dest_port : 发送报文的目的端口
          packet :发送的RIP报文
          pkt_len : 发送报文的长度
输出参数: 无
返回值: 处理成功,返回RIP_SUCCESS
        处理失败,返回RIP_FAIL
=============================================================*/
int rip_send_packet( uint32 device_index, uint32 dest_addr, uint16 dest_port, void *packet, uint32 pkt_len );
```

> 1.对输入参数进行合法性检查
>
> ```
> /*输入参数合法性检验*/
> if( ( device_index > INTERFACE_DEVICE_MAX_NUMBER )
>     || ( 0 == dest_addr )
>     || ( NULL == packet )
>     || ( pkt_len > RIP_MAX_PACKET_SIZE ) )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>     return RIP_FAIL;
> }
> ```
>
> 2.检查目的ip地址是否是自己端口的ip
>
> ```c
> /*IP不能向自己发送*/
> if( ( !BIT_TEST( rip_intf_array[device_index]->state ,RIP_INTF_PROCESS_ACTIVE))
>     || (dest_addr == rip_intf_array[device_index]->address))
> {
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>     return RIP_FAIL;
> }
> ```
>
> 3.查看目的ip是否是组播地址
>
> ```c
> /*定义变量初始化*/
> uint32 value[2];
> value[0] = dest_addr;    // 地址
> value[1] = device_index; // 端口
> if( dest_addr == htonl(RIP_MULTICAST_ADDRESS) ) 
> {
>     // 针对组播通信的端口配置，用于多对多的组播数据包传输
>     ret = so_setsockopt( rip_glb_info.socket_id, IPPROTO_IP, IP_MULTICAST_PORT, (char *)value, sizeof( value ) );
>     if( ret < 0 )
>     {
>         rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d (%d).\n",__FILE__,__LINE__,ret);
>         return RIP_FAIL;
>     }
> }else{
>     // 进行单播通信的端口配置，适用于一对一的数据包传输
>     ret = so_setsockopt( rip_glb_info.socket_id, IPPROTO_IP, IP_PORT, (char *)value, sizeof( value ) );
>     if( ret <0 )
>     {
>         rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d (%d).\n",__FILE__,__LINE__,ret);
>         return RIP_FAIL;
>     }
> }
> ```
>
> 4.设置目的地socket
>
> ```c
> memset( (void *)&socket, 0, sizeof(struct soaddr_in) );
> socket.sin_len = sizeof( struct soaddr_in );
> socket.sin_family = AF_INET;
> socket.sin_addr.s_addr = dest_addr;
> socket.sin_port = dest_port;
> ```
>
> 5.发送数据包
>
> ```c
> ret = so_sendto ( rip_glb_info.socket_id, packet, pkt_len, MSG_DONTROUTE, (struct soaddr *)&socket, sizeof(struct sockaddr_in) );
> if( ret != pkt_len )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d (%d).\n",__FILE__,__LINE__,ret);
>     return RIP_FAIL;
> }
> rip_intf_array[device_index]->rip2IfStatSentUpdates++;
> ```
>
> 

### rip_get_route

```c
//函数功能: 根据网络信息获取对应的路由
//输入参数: rip_route_item : 网络信息
//         rip_tbl_info: RIP路由表
struct rip_route_ *rip_get_route( struct rip_route_item_ *rip_route_item, struct route_table *table  );
```

> 1.获取路由条目的子网掩码
>
> ```c
> uint32 mask;
> if( 0 == rip_route_item->mask)
> {
>     mask = rip_get_mask( rip_route_item->network );
> }
> else
> {
>     mask = rip_route_item->mask;
> }
> ```
>
> 2.在路由表中查找与构造的前缀匹配的路由节点
>
> ```c
> struct route_node *rip_route_node;
> struct rip_route_ *rip_route_head;
> struct prefix route_node_prefix;
> 
> memset( (void *)&route_node_prefix, 0, sizeof(struct prefix) );
> route_node_prefix.family = AF_INET;     					   // IPv4
> route_node_prefix.safi = SAFI_UNICAST;  					   // 单播地址
> route_node_prefix.prefixlen = (u_char)mask_to_prefix( mask);   // 根据子网掩码获取前缀长度
> route_node_prefix.u.prefix4.s_addr = rip_route_item->network;  // 要查找的网络地址
> 
> rip_route_node = route_node_lookup(table, &route_node_prefix ); // 在路由表中查找与构造的前缀匹配的路由节点
> if( rip_route_node != NULL )
> {
>     route_unlock_node( rip_route_node );
> 
>     rip_route_head = rip_route_node->info;
>     if( rip_route_head->equi_route_num >= 1 )  // 如果等价路由数量大于或等于 1，则返回该路由信息。
>     {
>         return rip_route_head;
>     }
> }
> return NULL;
> ```

### rip_del_route_node

```c
// 清空路由节点
void rip_del_route_node( struct rip_route_ *rip_route );
```



> 1.对`rip_route`参数合法性检查
>
> 2.删除路由逻辑
>
> ```c
> rip_route_temp = rip_route->forw;
> while( rip_route_temp != rip_route )
> {
> 	rip_route_forw = rip_route_temp->forw;
> 	/*从holddown列表中删除*/
> 	rip_del_holddown( rip_route_temp);
> 	/*从相应端口的学习路由列表中删除*/
> 	rip_del_from_nbr_list( rip_route_temp );
> 	/*从重发布列表中删除*/
> 	rip_del_from_redis_list( rip_route_temp );
> 	/*删除路由*/
> 	ret = rip_del_route( rip_route_temp );
> 	if( RIP_SUCCESS != ret )
> 	{
> 		rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
> 		return;
> 	}
> 	rip_route_temp = rip_route_forw;
> };
> 
> /*free rip_route*/
> /*从holddown列表中删除*/
> rip_del_holddown( rip_route );
> /*从相应端口的学习路由列表中删除*/
> rip_del_from_nbr_list( rip_route );
> /*从重发布列表中删除*/
> rip_del_from_redis_list( rip_route );
> /*删除路由*/
> ret = rip_del_route( rip_route );
> if( RIP_SUCCESS != ret ){
> 	rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
> 	return;
> }
> ```



### 





## rip_timer.c

### rip_add_trigger

```c
// 将路由添加到触发更新列表中
void rip_add_trigger( struct rip_route_ *rip_route, struct rip_process_info_ *pprocess, BOOL b_DelFrmRipTbl )
```

> 1.
>
> ```c
> struct rip_route_ *rip_route_head = rip_route->route_node->info;
> // 默认路由、转发路由和汇总路由不会添加
> if(rip_route_head && (rip_route_head != rip_route) && (rip_route_head->metric != RIP_MAX_METRIC)){
> 	if((rip_route_head->route_type==RIP_DEF_ROUTE)
> 		||(rip_route_head->route_type==RIP_REDIS_ROUTE)
> 		||(rip_route_head->route_type==RIP_SUMMARY_ROUTE)){
> 			rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
> 			return;
> 		}
> }
> ```
>
> 



### rip_del_holddown

```c
// 从holddown更新列表中删除该路由
void rip_del_holddown( struct rip_route_ *rip_route )
```

> 1.检查路由是否处于holddown状态，若不处于则无需处理
>
> ```c
> if((!rip_route)||(NULL == rip_route->hold_ptr)){
> 	return;
> }
> ```
>
> 2.检查路由类型是否是从邻居节点学习到的
>
> ```c
> /*只有学习到的路由才会加入路由表*/
> if( RIP_NBR_ROUTE != rip_route->route_type ){
> 	return;
> }
> ```
>
> 3.检查管理路由的进程是否为NULL
>
> ```
> if(!(pprocess = rip_route->pprocess)){
> 	rip_debug( RIP_DEBUG_IP_RIP_RETURN,"RIP:%s,%d ,error pprocess %x\n",__FILE__,__LINE__,pprocess);
> 	return;
> }
> ```
>
> 4.删除路由下的hold_ptr
>
> ```c
> REMQUE( rip_route->hold_ptr ); //  ？？？？
> rip_mem_free( rip_route->hold_ptr, RIP_ROUTE_LIST_TYPE );
> rip_route->hold_ptr = NULL;
> ```
>
> 5.rip进程的holddown_route_num减一
>
> ```c
> pprocess->holddown_route_num--;
> ```
>
> 6.检查holddown_route_num是否为0，若为0，则需关闭holddown定时器
>
> ```c
> if( (pprocess->holddown_route_num == 0) && (pprocess->holddown_timer_id != 0) ){
> 	/*停止holddown定时器*/
> 	sys_stop_timer( pprocess->holddown_timer_id );
> }
> ```



### rip_del_from_nbr_list

```c
// 从相应端口的学习路由列表中删除指的路由
void rip_del_from_nbr_list( struct rip_route_ *rip_route )
```

> 1.检查路由类型是否是`从邻居节点学习到`的类型
>
> ```c
> /*只对学习到的路由进行处理*/
> if(!rip_route || RIP_NBR_ROUTE != rip_route->route_type ){
> 	return;
> }
> ```
>
> 2.从gw_index的学习列表中删除
>
> ```c
> device_index = rip_route->gw_index;
> rip_route_list_temp = rip_route->nbr_ptr;
> if(rip_route_list_temp){
>     REMQUE( rip_route_list_temp );
>     rip_mem_free( rip_route_list_temp, RIP_ROUTE_LIST_TYPE );
>     rip_intf_array[device_index]->nbr_route_num--;
>     rip_route->nbr_ptr = NULL;
> }
> ```



## rip_redis.c

### rip_del_from_redis_list

```c
void rip_del_from_redis_list(struct rip_route_ *rip_route)
```

> 1.检查是否是转发路由
>
> ​	if(!rip_route||rip_route->route_type!=RIP_REDIS_ROUTE) return;
>
> 2.从重发布列表中删除路由
>
> ```c
> redis_list=rip_route->red_ptr;
> if(redis_list){
> 	REMQUE(redis_list);
> 	rip_mem_free(redis_list,RIP_ROUTE_LIST_TYPE);
> 	rip_route->red_ptr=NULL;
> }
> ```



## rip_cmd.c

### rip_intf_creat_timer

```c
// 设置定时器，并在超时后通过消息队列向指定的设备发送超时消息
int rip_intf_creat_timer(uint32 device_index)
```

> ```c
> TIMER_USER_DATA timer_user_data;
> uint32 ret;
> 
> /*Register rip neighbor timer*/
> // 初始化定时器用户数据，包括消息队列id，具体的消息
> memset( &timer_user_data, 0, sizeof(TIMER_USER_DATA) );
> timer_user_data.msg.qid = (MSG_Q_ID)rip_glb_info.queue_id;
> timer_user_data.msg.msg_buf[0] = MSG_RIP_INTF_KEY_START;  // 第一个字节 消息类型
> timer_user_data.msg.msg_buf[1] = device_index;			  // 第二个字节 端口id
> // TIMER_MSG_METHOD：设置定时器的触发方式，表示超时后通过消息机制（消息队列）通知。
> // &timer_user_data：该数据将在定时器超时时作为消息发送。
> // key_start_timer_id：端口的key开始生效超时定时器ID
> ret = sys_add_timer(TIMER_MSG_METHOD, &timer_user_data, &(rip_intf_array[device_index]->key_start_timer_id));
> if( SYS_NOERR != ret )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>     return RIP_FAIL;
> }
> 
> memset( &timer_user_data, 0, sizeof(TIMER_USER_DATA) );
> timer_user_data.msg.qid = (MSG_Q_ID)rip_glb_info.queue_id;
> timer_user_data.msg.msg_buf[0] = MSG_RIP_INTF_KEY_TIMEOUT;
> timer_user_data.msg.msg_buf[1] = device_index;
> // key_lift_timer_id：端口的key有效时间超时定时器
> ret = sys_add_timer (TIMER_MSG_METHOD, &timer_user_data, &(rip_intf_array[device_index]->key_lift_timer_id));
> if( SYS_NOERR != ret )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>     return RIP_FAIL;
> }
> 
> memset( &timer_user_data, 0, sizeof(TIMER_USER_DATA) );
> timer_user_data.msg.qid = (MSG_Q_ID)rip_glb_info.queue_id;
> timer_user_data.msg.msg_buf[0] = MSG_RIP_INTF_AUTH_COMMIT_TIMEOUT;
> timer_user_data.msg.msg_buf[1] = device_index;
> //auth_commit_timer_id：端口的认证提交超时定时器
> ret = sys_add_timer (TIMER_MSG_METHOD, &timer_user_data, &(rip_intf_array[device_index]->auth_commit_timer_id));
> if( SYS_NOERR != ret )
> {
>     rip_debug( RIP_DEBUG_IP_RIP_RETURN, "RIP: %s %d .\n",__FILE__,__LINE__);
>     return RIP_FAIL;
> }
> ```
