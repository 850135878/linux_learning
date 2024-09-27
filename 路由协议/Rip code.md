# RIP代码

<img src="./Rip code.assets/image-20240926165142493.png" alt="image-20240926165142493" style="zoom: 50%;" />

## 预编译指令

`#pragma pack(1)` ：将结构体成员按 1 字节对齐

`#pragma align 1`：在某些编译器中可能用于设置对齐方式

`#pragma align 0`：通常用于恢复默认对齐设置

`#pragma pack ()`：取消自定义字节对齐方式

## RIP报文

### 消息头部

```c
// Rip消息头部
struct rip {
    uint8	rip_cmd;		/* request/response */  
    uint8	rip_vers;		/* protocol version # */
    uint16	rip_zero2;		/* unused */
};
```

```c
struct rip_netinfo {
    uint16	rip_family;
    uint16	rip_tag;
    uint32	rip_dest;
	uint32	rip_mask;
	uint32	rip_router;
    uint32	rip_metric;			/* cost of route */
};
```

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

