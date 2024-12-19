# EIGRP

对比RIPv2

- 通过多个条件对metric值进行计算，计算比RIPv2更严格；
- 最大跳数为255；
- 收敛速度更快，每5s发送一次Hello报文，确认邻居关系；
- 使用DUAL扩散更新算法，快速收敛路由；
- 使用增量更新和触发更新来取代周期性更新。



## 特点

- 高级距离矢量
- 快速收敛
- 支持VLSM（可变子网）和不连续子网CIDR（超网）
- 部分更新，带宽占用少
- 多播和单播，而不是广播地址
- 支持多种网络层协议
- 100%无循环无类路由
- 支持所有 LAN 和 WAN 数据链路协议和拓扑。
- 在相等和不相等成本路径上实现负载均衡



## 基本概念

​	EIGRP（Enhanced Interior Gateway Routing Protocol ）即增强型内部网关路由协议。

​	EIGRP结合了**链路状态**和**距离矢量**型路由选择协议的Cisco专用协议，采用扩散更新算法（DUAL）来实现快速收敛，可以**不发送定期的路由更新信息**以减少带宽的占用，支持Appletalk、IP、Novell和NetWare等多种网络层协议。

​	支持等开销负载均衡，默认4条，最大6条，默认ios12.4以上支持16条；同时支持非等开销负载均衡。

​	协议号：88



## 报文格式

### 1. EIGRP Packet Header

<img src="./EIGRP.assets/image-20241219135315120.png" alt="image-20241219135315120" style="zoom: 67%;" />

- Header Version：EIGRP数据包头部格式Version为2

- Option：消息的类型

  > EIGRP_OPC_UPDATE  	  1
  > EIGRP_OPC_REQUEST	  2
  > EIGRP_OPC_QUERY	      3
  > EIGRP_OPC_REPLY	       4
  > EIGRP_OPC_HELLO	      5
  >
  > Reserved			       6	   IPX SAP
  >
  > EIGRP_OPC_SIAQUERY	10	SIA查询
  > EIGRP_OPC_SIAREPLY	  11	SIA答复

- Checksum：标准的 IP 校验和。它是基于除了IP头部的整个EIGRP数据包来计算的。

- Flag：定义数据包的特殊处理。 目前有四个已定义的标志位。

  > INIT-Flag (0x01)：在发送给新发现邻居的初始UPDATE包中设置。 它指示向邻居通告其全部路由。
  >
  > CR-Flag (0x02)：该位指示接收器仅在处于条件接收模式时才应接受数据包。 
  >
  > ​			     当路由器接收并处理带有SEQUENCE TLV的HELLO 数据包时，路由器进入条件接收模式。  
  >
  > RS-Flag (0x04)：在重启期间，HELLO和UPDATE数据包中设置重启标志。 路由器查看 RSFlag 以检测邻居是否正在重新启动。 				从重启路由器的角度来看，如果相邻路由器检测到设置了RS-Flag，它将维持邻接关系，并在其UPDATE数据				包中设置RS-Flag以指示它正在进行软重启。  
  >
  > EOT-Flag (0x08)：End-of-Table 标志标志着与邻居的启动过程的结束。 如果设置了该标志，则表明邻居已完成发送所有 				UPDATE。 此时，路由器将删除在重启事件之前从邻居获知的所有陈旧路由。 陈旧路由是指在重新启动之前				存在且未被邻居通过 UPDATE 刷新的任何路由。

- Sequence Number：传输的每个数据包都将具有一个 32 位序列号，该序列号对于发送路由器而言是唯一的。

  ​				     值 0 表示不需要确认。

- Acknowledgment Number：数据包接收方正在确认的 32 位序列号。 

  - 如果该值为 0，则不存在确认。 

  - 非零值只能出现在单播寻址的数据包中。<font color='red'> 具有非零 ACK 字段的 HELLO 数据包应被解码为 ACK 数据包而不是 HELLO 数据包。</font>

    > 注意，如果数据包本身是单播的，这里的 ACK 字段只能是非零的，因为确认数据包从来都不是组播的。

- Virtual Router Identifier (VRID)：一个 16 位数字，用于标识与该数据包关联的虚拟路由器。 
  - 0x0000  Unicast Address Family
  - 0x0001  Multicast Address Family
  - 0x0002-0x7FFF  Reserved
  - 0x8000  Unicast Service Family
  - 0x8001-0xFFFF  Reserved
- Autonomous System Number：发送系统的16位无符号数。 该字段间接用作认证值。 也就是说，接收并接受来自邻居的数据包的路由器必须具有相同的 AS 号，否则该数据包将被忽略。 有效 AS 编号的范围是 1 到 65,535。

### 2. EIGRP TLV Encoding Format

​	每一个 TLV字段都包含2个八位字节的类型号、一个指定 TLV 字段长度的2个八位组字节的字段和一个由类型决定其格式的可变字段。

<img src="./EIGRP.assets/image-20241219141112213.png" alt="image-20241219141112213" style="zoom: 67%;" />

- Type high：

  |   Protocol    |  ID  | Version |
  | :-----------: | :--: | :-----: |
  |    General    | 0x00 |   1.2   |
  |     IPv4      | 0x01 |   1.2   |
  |     IPv6      | 0x04 |   1.2   |
  |      SAF      | 0x05 |   3.0   |
  | Multiprotocol | 0x06 |   2.0   |

  <img src="./EIGRP.assets/image-20241219142957343.png" alt="image-20241219142957343" style="zoom: 50%;" />

- Length：是一个2个八位字节的无符号数，表示TLV的长度。 该值包括类型和长度字段。

- Value：一个多八位字节字段，包含 TLV 的有效负载

## 五种数据包

> 必须回复Ack的包（可靠的）：Update、Query、Reply
>
> 	- 若未收到Ack，将进行单播重传（最大16次）
> 	- 若依旧失败，则断开邻居关系；RTP在转发这三种包时，还会对其进行排序和流控；
>
> 而Hello和Ack包（不可靠的）是无需回复的。



### 1. Hello包

用于建立邻居关系，每5s向224.0.0.10组播地址发送Hello包，确认邻居关系；抑制时间为15s；

### 2. Update包

发送路由更新信息（发送Sequence_num）

### 3. Query包

当路由信息丢失并没有备用路由时，使用Query数据包向邻居查询，邻居必须回复确认。

### 4. Reply包

对邻居Query数据包的回复，也需要邻居回复确认。

### 5. ACK包

对收到的数据包的确认，告诉邻居自己已经收到数据包了，收到Ack后，不需要再对Ack做回复。

<img src="./EIGRP.assets/image-20241219094011124.png" alt="image-20241219094011124" style="zoom:50%;" />



## 维护的三张表

### 1. 邻居表

​	记录运行EIGRP的直连路由器的IP地址。

​	EIGRP在运行时需要首先跟周边的直连路由器建立EIGRP邻居关系。

<img src="./EIGRP.assets/image-20241219094109241.png" alt="image-20241219094109241" style="zoom: 67%;" />

> H ：记录了所学到的邻居路由器的顺序号。
>
> Address：邻居路由器的端口IP地址
>
> Interface：用于与该邻居通信的本地端口
>
> Hold：距收到上次邻居的 EIGRP 数据包以来，邻居还能维持多少时间（通常为 3 倍的 hello 时间）。
>
> Uptime：自与该邻居建立 EIGRP 邻居关系以来的时间
>
> SRRT（Smooth Round Trip Time）：发送EIGRP包并收到邻居确认的平均往返时间
>
> RTO（Retransmission Timeout）：等待邻居确认时的超时时间，通常与SRRT有关
>
> Q（Queue Count）：发送队列中的数据包数量。0表示没有待处理的数据包
>
> Seq：标识 EIGRP 消息的序列号，是由邻居生成并发送给本地设备的最新确认序列号

#### 1.1 相关字段

##### 1.1.1 SRTT 平均往返时间计时器

​	计算路由器发送EIGRP数据包到邻居和从邻居收到该数据包的确认（ACK）所花费的时间，毫秒（MS）为单位，取3次的平均时间。

##### 1.1.2 Queue count 队列数

​	未收到对端ACK，Qcnt=1，收到后，Qcnt=0

##### 1.1.3 RTO 重传超时定时器

​	SRTT和RTO时间是思科私有的



#### 1.1 邻居关系

​	跟OSPF一样，EIGRP使用邻居的概念。在路由器之间通过非常轻量级的EIGRP Hello报文检查邻居之间的可达性，维护邻居关系。

EIGRP邻居发现，邻居参数协商以及维护邻居关系都由EIGRP Hello报文完成。

> 使用组播地址224.0.0.10发送Hello报文。  
>
> 默认每5s发送一次Hello报文，如果连续15s没收到Hello报文，则断开邻居关系，删除从该邻居学到的所有路由。



EIGRP要求建立邻居关系的两台路由器，下列参数需要匹配：

- AS号一致,  `类似于进程号`
- K 值，默认1 0 1 0  0
- 处于同一子网
- 认证信息
- router-id必须不同



#### 1.2 影响EIGRP建立邻居的因素

##### 1.2.1 AS号

​	开启EIGRP进程的路由器之间，需要**AS进程号一致**才能建立邻居。

##### 1.2.2 Key值

​	key值是Metric计算的权重值，发送的Hello包中携带key值:

​	默认数值：K1=1，K2=0，K3=1，K4=0，K5=0；**如果Key值不一致，会导致邻居建立不了。**

##### 1.2.3 源检测

​	运行EIGRP的接口如果有主/secondary地址，发出的Hello包使用主地址为源，确保源检测通过。

##### 1.2.4 Hello包发送方式

​	hello包发送方式需要一致，单播对单播，组播对组播；

##### 1.2.5 **认证**

​	路由器开启了EIGRP认证，认证通过才能建立邻居。



#### 1.3 16次重传

​	数据包原本会以可靠的组播方式发出去，发送至224.0.0.10；MFT时间内没有从邻居收到ACK确认信息，那么数据包会以单播的形式重新向未回复ACK的邻居发送。若经过16次重传后，仍然没有收到ACK回复，则这个邻居被宣告无效，就会重置邻居关系。

> **从组播方式发送切换到单播方式之前，等待一个ACK时间，由组播统计流计时器Multicast flow timer指定。后续的单播之间由RTO（等待邻居确认时的超时时间）指定。MFT和RTO时间是通过SRTT计算出来的**





### 2. 拓扑表

​	本地直连路由和收到的邻居路由信息都会存到拓扑表中。

​	去往某个路径的所有集合。包括所有FS（可行后继路由器）

<img src="./EIGRP.assets/image-20241218175901985.png" alt="image-20241218175901985" style="zoom:50%;" />

> P (Passive/Active)：路径状态。 P表示路径稳定，未发生变化；A正在查询更好的路径或正在解决路由问题。
>
> ​	注意：<font color='red'> 仅在路径进入 `Active` 状态时触发查询，表明当前路径存在问题</font>

- 目的网络的可行距离FD
- 所有的可行后继路由器FS
- 每一个可行后继路由器所通告的到达目的网络的通告距离AD；

- 本地路由器所计算的经过每一个可行后继路由器到达目的网络的距离，也就是基于可行后继路由器所通告的到达目的子网的距离和本地路由器与该可行后继路由器之间相连链路的成本计算所得的距离；

### 3. 路由表

​	最优的路由会加入路由表，其他路由作为备选路径。

​	包括拓扑表中所有FS当中选出的S（后继路由器）所传递的路由器。

<img src="./EIGRP.assets/image-20241218160917499.png" alt="image-20241218160917499" style="zoom: 50%;" />

## 术语

### 邻接关系

> 邻接是指两个互相交换路由信息的邻居之间形成的一条逻辑的关联关系。

​	EIGRP刚启动时，路由器使用 Hello 数据包发现它的邻居并标识自己给邻居识别。

​	当邻居被发现时，EIGRP 协议将试图和它的邻居形成一个邻接关系。一旦邻接建立成功，路由器就可以从它们的邻居那里接收路由更新消息了。

### 通告距离（AD）

> 邻居（下一跳）路由器到达目的的Metric值

​	或称为RD（报告距离），邻居报告到达目标网络的metric



### 计算距离（CD）

​	AD + 本地路由器到达邻居的距离metric



### 可行性距离（Feasible Distance，FD）

> 本路由器到达目的的Metric值

​	**当前S的CD，**本地路由器到达每个目的网络的最小度量值metric就是该目的网络的可行距离



### 后继路由器（Successor）

​	到达目标网络的最佳路径**（CD/FD最小）**的下一跳路由器S



### 可行后继路由器（Feasible Successor，FS）

> 如果本地路由器的邻居路由器所通告的到达目的网络的距离AD < FD（满足了 FC），那么这个邻居就会成为该目的网络的一个可行后继路由器。

​	到达目标网络的备用路由的下一跳路由器。所有的FS都会存储在EIGRP拓扑表内。





<img src="./EIGRP.assets/image-20241219104857627.png" alt="image-20241219104857627" style="zoom:50%;" />

> 如上图，对于R1来说，R1去往R8有上中下三条链路，红色为Metric值。
>
> - 下面的链路FD为12，所以为Successor路由；
> - 中间的链路AD值为6，小于下面链路的FD值，所以为Feasible successor；
> - 而上面的链路既不是successor路由，也不是Feasible successor路由。



### 可行性条件（Feasibility Condition，FC）

​	需要满足本地路由器的一个邻居路由器所通告的到达一个目的网络的距离AD是否小于本地路由器到达相同目的网络的可行距离（FD）。    **FD  >  RD**   不满足当不了FS路由器

​	FC是EIGRP确保完全无环的关键。FC是一个路由没有环路的充分非必要条件。

​	只要有一条路由有环路风险，无论它是否有环，都不会被EIGRP采纳。可能导致一部分实际上无环的路径无法进入路由表。

<img src="./EIGRP.assets/image-20241218155850326.png" alt="image-20241218155850326" style="zoom: 50%;" />

## 如何实现快速收敛？

​	当S不再可用时，EIGRP搜索自己的拓扑表，如果有另一个FS，则不经过任何计算，直接通过另一个FS转发数据，同时该FS也就成为了S。如果FS全挂了，R1会发送请求给R4，如果R4有去往2.0网段路由，则直接发送给R1，并使用R4作为S。



## 四大组件

### 1. Hello机制

​	邻居发现和恢复

### 2. PDM

​	PDM（Protocol-Dependent Module,PDM) 协议相关模块：支持多种网络层协议。

### 3. RTP（Reliable Transport Protocol,RTP)

​	RTP（Reliable Transport Protocol,RTP) ：可靠传输协议，用来管理 EIGRP 数据包的发送和接收。

> - 有序的发送：
>   - 通过在每个数据包中包含两个序列号来实现的。每一个数据包都包含一个由发送该数据包的路由器分配的序列号，这个序列号在每台路由器发送一个新的数据包时递增 1。另外，发送路由器会把最近从目的路由器收到的数据包的序列号放在该数据包中。

​	借鉴TCP的4种可靠机制（确认、重传（默认最大16次）、排序、流控 （传递协议报文的流量不能超过链路带宽的50%）

### 4.扩散更新算法DUAL

​	DUAL（扩散更新算法）: 选择到每个目的网络的最小开销、loop-free的路径

​	为了随时能够打破路由环路，而使用扩散计算去执行一个分布式最短路径路由选择。

​	Loop-free：AD < FD 

<img src="C:\Users\huang\AppData\Roaming\Typora\typora-user-images\image-20241218141244216.png" alt="image-20241218141244216" style="zoom:50%;" />

操作DUAL的前提条件   `使用邻居发现/恢复和可靠传输协议（RTP）来确定这些前提条件`

- 一个节点需要在有限的时间内检测到一个新邻居的存在或一个相连邻居的丢失。
- 在一个正在运行的链路上传送的所有消息应该在一个有限的时间内正确地收到，并且包含正确的序列号。
- 所有的消息，包括改变链路的代价、链路失败和发现新邻居的通告，都应该在一个有限的时间内一次一个地处理，并且应该被有顺序地检测到。



## Metric计算

​	默认，EIGRP使用**带宽和延迟**参与Metric计算，可选的使用可靠性和负载（不推荐）参与Metric计算

> 计算延迟时，使用的是根据端口带宽得到一个参考延迟，而非实际延迟。实际上对Metric影响最大的还是带宽

```c
1. 静态度量值：
	BandWidth = 10^7/链路的最小带宽  kbps
	Delay = 路径延迟delay之和/10（以10us为单位）
2. 动态度量值，它们会随着时间而不断变化。
    可靠性
    负载
    MTU

Formula with default K values (带宽K1 = 1, 负载K2 = 0, 延迟K3 = 1, 可靠性K4 = 0, MTUK5 = 0): 
	Metric = [K1 * BW + ((K2 * BW) / (256 – load)) + K3 * delay]

(1)默认：
    Metric = (Bandwidth + Delay) * 256;
(2)K5>0:
	Metric = [K1 * BW + ((K2 * BW) / (256 – load)) + K3 * delay] * [K5 / (reliability + K4)] * 256;
```

> 1）带宽（BW）取路由传递方向**最小带宽**
>
> 2）延时（Delay）取数据传递方向出向接口**延时之和**

<img src="./EIGRP.assets/image-20241219104021755.png" alt="image-20241219104021755" style="zoom:50%;" />



## 工作过程

1. 启动配置后，运行EIGRP协议的路由器，通过组播224.0.0.10向所有的邻居发送hello包；
2. 收到邻居的hello包后及标识认识了邻居，生成邻居表；
3. 邻居关系建立后，邻居间使用更新包进行路由信息的共享；
   1. 收集到所有的路由条目后，本地生成拓扑表；拓扑表装载所有的最佳和备份路径；
   2. 本地默认将拓扑表中的最佳路径全部加载到路由表内；收敛完成。之后仅Hello包保活即可。

结构突变：

（1）新增网段：直连新增网段的设备，使用更新包告知所有邻居即可

（2）断开网段：直连断开网段的设备，发出查询包，通过DUAL，收集应答包来重新收敛；完成全网同步。

（3）设备无法沟通：hello time 5s hold time 15s

​		hold time （采取的是倒计时方式）计时器结束断开邻居关系，同时产生删除所有从邻居出学习到的信息



## 配置命令

```cmd
router eigrp <AS号>
network <net-number>   # 宣告网络

router eigrp 100                     #100为进程号（AS），范围1-65535
 network 192.168.12.1 0.0.0.0        #精准匹配
 network 192.168.12.0 0.0.0.255      #反掩码匹配
 network 192.168.12.0 255.255.255.0  #正掩码匹配
 network 0.0.0.0 0.0.0.0             #激活全部接口
 network 0.0.0.0 255.255.255.255     #激活全部接口

# show 命令
show ip route eigrp
show ip protocols
show ip eigrp interfaces
show ip eigrp neighbors [detail]
show ip eigrp topology [all]
show ip eigrp traffic
debug ip eigrp
```

<img src="./EIGRP.assets/image-20241218141443795.png" alt="image-20241218141443795" style="zoom:50%;" />

### network的两层含义

1、network后面的网段所在范围的端口开启EIGRP进程；

2、该端口所在直连网段路由放进EIGRP的拓扑表中。