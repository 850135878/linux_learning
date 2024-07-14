# Ensp实验

## 基础配置命令

```cmd
sys  					  # 进入管理视图（默认，用户视图）
sysname 名称              # 修改设备名称
undo info-center enable  # 关闭信息中心

vlan 10            # 创建单个vlan
vlan batch 10 20   # 创建多个vlan

display vlan int 10   # 查看指定vlan的设置
display port vlan active # 查看交换机连接配置 

display ip int brief     # 查看接口的ip地址
display ip routing-table # 查看路由表 
```



## DHCP实验

### 三种地址池模式区别

- 全局地址池

  - 指可以为网络中的<font color='red'>任意接口</font>提供DHCP服务的地址池。它允许创建多个全局地址池，每个地址池可以配置不同的IP地址范围和配置参数。   `创建完全局地址池后，可分配给某些指定接口`

   适用于大型网络或需要精细管理网络资源的场景。

- 接口地址池（仅限于特定接口下的客户端）   `只能直接在指定接口上创建地址池，并为接入其设备的客户端提供DHCP服务的地址池`

  - 是指将DHCP服务绑定到<font color='red'>指定接口</font>上，并为该接口下的客户端提供DHCP服务的地址池。

  - 通常与接口的IP地址和子网配置紧密相关，配置相对简单。

  适用于<font color='red'>小型网络或特定区域网络</font>。

- 中继代理模式













### 1. 全局地址池模式

#### 1.路由器DHCP服务器配置

1. 给路由器的接口配置网关  `ip address 192.168.10.254 24`

2. 进入全局配置模式，开启DHCP服务    `dhcp  enable`

3. 创建ip全局地址池，`ip pool 池名`

   在当前地址池中进行配置：

   - 可动态分配的IP地址范围：`network ip地址 [mask{mask|mask-length}]`		

   - 网关 `gateway-list 网关地址`       对应路由器某一接口的网关地址，如g0/0/0的。
   - 域名 `dns-list 客户端使用的dns服务器的IP地址`
   - 配置租期 `lease {day 天数 {hour 小时 [minute 分钟]} | unlimited}`
   - 配置排除分配的ip地址 `excluded-ip-address ip地址`

4. 给指定接口，设置全局地址池模式 `dhcp select global `    <font color='red'>  对于三层交换机来说，需给逻辑接口设置，即在指定vlan下开启</font>

```cmd
<Huawei>sys
# 1.给AR1的两个端口配置网关
[Huawei]int g0/0/1
[Huawei-GigabitEthernet0/0/0]ip address 192.168.10.254 24

[Huawei-GigabitEthernet0/0/0]int g0/0/1
[Huawei-GigabitEthernet0/0/1]ip address 192.168.20.254 24

# 2.AR1开启DHCP服务 全局配置
[Huawei-GigabitEthernet0/0/1]q	
[Huawei]dhcp enable 

# 3.创建两个地址池，tushuguan、jiaoxuelou
[Huawei]ip pool tushuguan
[Huawei-ip-pool-tushuguan]network 192.168.10.0
[Huawei-ip-pool-tushuguan]gateway-list 192.168.10.254
[Huawei-ip-pool-tushuguan]dns-list 8.8.8.8
[Huawei-ip-pool-tushuguan]excluded-ip-address 192.168.10.253	
[Huawei-ip-pool-tushuguan]lease day 0 hour 0 minute 20
[Huawei-ip-pool-tushuguan]display this
[V200R003C00]
##
#ip pool tushuguan
# gateway-list 192.168.10.254 
# network 192.168.10.0 mask 255.255.255.0 
# excluded-ip-address 192.168.10.253 
# dns-list 8.8.8.8 
##
#return

#  进入端口，开启全局地址池模式
[Huawei-ip-pool-tushuguan]int g0/0/0
[Huawei-GigabitEthernet0/0/0]dhcp select global 

[Huawei]ip pool jiaoxuelou
[Huawei-ip-pool-jiaoxuelou]network 192.168.20.0
[Huawei-ip-pool-jiaoxuelou]gateway-list 192.168.20.254
[Huawei-ip-pool-jiaoxuelou]dns-list 8.8.8.8
[Huawei-ip-pool-jiaoxuelou]excluded-ip-address 192.168.20.230 192.168.20.253
[Huawei-ip-pool-jiaoxuelou]int g0/0/1
[Huawei-GigabitEthernet0/0/1]dhcp select global
```

<img src="C:/Users/85013/AppData/Roaming/Typora/typora-user-images/image-20240714152726800.png" alt="image-20240714152726800" style="zoom:50%;" />

#### 2.交换机DHCP服务器配置

- 涉及二层交换机之间接入配置  `access`
- 三层交换机互联vlan配置          `trunk`
- DHCP配置

​		和路由器配置差不多，只不过对于三层交换机，最后是在指定的<font color='red'>逻辑接口，即vlan</font>进行设置开启地址池的模式。

```cmd
# 对于二层交换机，需通过vlan，设置接口的转发模式
<Huawei>sys
Enter system view, return user view with Ctrl+Z.
[Huawei]vlan 10
[Huawei]undo info-center enable 
Info: Information center is disabled.
[Huawei]int vlan 10
[Huawei-Vlanif10]int g0/0/2
[Huawei-GigabitEthernet0/0/2]port link-type access
[Huawei-GigabitEthernet0/0/2]display vlan 10
--------------------------------------------------------------------------------
U: Up;         D: Down;         TG: Tagged;         UT: Untagged;
MP: Vlan-mapping;               ST: Vlan-stacking;
#: ProtocolTransparent-vlan;    *: Management-vlan;
--------------------------------------------------------------------------------

VID  Type    Ports                                                          
--------------------------------------------------------------------------------
10   common  

VID  Status  Property      MAC-LRN Statistics Description      
--------------------------------------------------------------------------------
10   enable  default       enable  disable    VLAN 0010                         
[Huawei-GigabitEthernet0/0/2]port default vlan 10
[Huawei-GigabitEthernet0/0/2]display vlan 10
--------------------------------------------------------------------------------
U: Up;         D: Down;         TG: Tagged;         UT: Untagged;
MP: Vlan-mapping;               ST: Vlan-stacking;
#: ProtocolTransparent-vlan;    *: Management-vlan;
--------------------------------------------------------------------------------

VID  Type    Ports                                                          
--------------------------------------------------------------------------------
10   common  UT:GE0/0/2(U)                                                      

VID  Status  Property      MAC-LRN Statistics Description      
--------------------------------------------------------------------------------
10   enable  default       enable  disable    VLAN 0010                         
[Huawei-GigabitEthernet0/0/2]int g0/0/1
[Huawei-GigabitEthernet0/0/1]port link-type trunk 
[Huawei-GigabitEthernet0/0/1]port trunk allow-pass vlan 10
[Huawei-GigabitEthernet0/0/1]dis vlan 10
--------------------------------------------------------------------------------
U: Up;         D: Down;         TG: Tagged;         UT: Untagged;
MP: Vlan-mapping;               ST: Vlan-stacking;
#: ProtocolTransparent-vlan;    *: Management-vlan;
--------------------------------------------------------------------------------

VID  Type    Ports                                                          
--------------------------------------------------------------------------------
10   common  UT:GE0/0/2(U)                                                      
             TG:GE0/0/1(U)                                                      

VID  Status  Property      MAC-LRN Statistics Description      
--------------------------------------------------------------------------------
10   enable  default       enable  disable    VLAN 0010                         
[Huawei-GigabitEthernet0/0/1] User interface con0 is available
```

<img src="./Ensp%E5%AE%9E%E9%AA%8C.assets/image-20240714162448650.png" alt="image-20240714162448650" style="zoom:50%;" />

```cmd
[Huawei]sysname LSW4
[LSW4]undo info-center enable 
Info: Information center is disabled.
[LSW4]int g0/0/1
[LSW4-GigabitEthernet0/0/1]port link-type trunk 
[LSW4-GigabitEthernet0/0/1]port trunk allow-pass vlan 10
[LSW4-GigabitEthernet0/0/1]int g0/0/2	
[LSW4-GigabitEthernet0/0/2]port link-type trunk 
[LSW4-GigabitEthernet0/0/2]port trunk allow-pass vlan 20
[LSW4-GigabitEthernet0/0/2]vlan batch 10 20
Info: This operation may take a few seconds. Please wait for a moment...done.
[LSW4]int vlan 10
[LSW4-Vlanif10]ip address 192.168.10.0 24
Error: The specified IP address is invalid.
[LSW4-Vlanif10]ip address 192.168.10.254 24
[LSW4-Vlanif10]int vlan 20
[LSW4-Vlanif20]ip address 192.168.20.254 24
[LSW4-Vlanif20]q
[LSW4]dhcp enable 
Info: The operation may take a few seconds. Please wait for a moment.done.
[LSW4]ip pool tushuguan
Info:It's successful to create an IP address pool.
[LSW4-ip-pool-tushuguan]network 192.168.10.0 
[LSW4-ip-pool-tushuguan]gateway-list 192.168.10.254
[LSW4-ip-pool-tushuguan]dns-list 8.8.8.8
[LSW4-ip-pool-tushuguan]excluded-ip-address 192.168.10.200 192.168.10.253
[LSW4-ip-pool-tushuguan]lease day 0 hour 0 minute 1
[LSW4-ip-pool-tushuguan]int g0/0/1
[LSW4-GigabitEthernet0/0/1]int vlan 10
[LSW4-Vlanif10]dhcp select global 
[LSW4-Vlanif10]q

[LSW4]dhcp enable 
[LSW4]ip pool jiaoxuelou
Info:It's successful to create an IP address pool.
[LSW4-ip-pool-jiaoxuelou]network 192.168.20.0	
[LSW4-ip-pool-jiaoxuelou]gateway-list 192.168.20.254
[LSW4-ip-pool-jiaoxuelou]excluded-ip-address 192.168.20.10 192.168.20.253
[LSW4-ip-pool-jiaoxuelou]dns-list 8.8.8.8
[LSW4-ip-pool-jiaoxuelou]int vlan 20
[LSW4-Vlanif20]dhcp select global 
```





### 2. 接口地址池模式

#### 1.路由器DHCP服务器配置

1. 给路由器的接口配置网关  如`ip address 192.168.10.254 24`

2. 进入全局配置模式，开启DHCP服务    `dhcp  enable`

3. 进入指定接口，为其创建接口IP地址池    

   设置接口地址池模式  `dhcp select interface`

   在当前接口的地址池中进行配置：

   - 域名 `dhcp server dns-list 客户端使用的dns服务器的IP地址`
   - 配置租期 `dhcp server lease {day 天数 {hour 小时 [minute 分钟]} | unlimited}`
   - 配置排除分配的ip地址 `dhcp server excluded-ip-address ip地址`

```
<Huawei>sys
Enter system view, return user view with Ctrl+Z.
[Huawei]sysname R1
[R1]undo info-center ena
Info: Information center is disabled.

[R1-GigabitEthernet0/0/0]q
[R1]dhcp enable 
Info: The operation may take a few seconds. Please wait for a moment.done.
[R1]int g0/0/1
[R1-GigabitEthernet0/0/1]ip address 192.168.20.25 244
[R1-GigabitEthernet0/0/1]int g0/0/0

[R1-GigabitEthernet0/0/0]dhcp select interface 
[R1-GigabitEthernet0/0/0]ip address 192.168.10.254 24
[R1-GigabitEthernet0/0/0]dhcp select interface 
[R1-GigabitEthernet0/0/0]dhcp server ?
  dns-list             Configure DNS servers
  domain-name          Configure domain name 
  excluded-ip-address  Mark disable IP addresses 
  import               Imports the following network configuration parameters   
                       from a central server into local ip pool database: domain
                       name, dns server and netbios server.
  lease                Configure the lease of the IP pool
  nbns-list            Configure the windows's netbios name servers 
  netbios-type         Netbios node type
  next-server          The address of the server to use in the next step of the 
                       client's bootstrap process.
  option               Configure the DHCP options
  option121            DHCP option 121 
  option184            DHCP option 184
  recycle              Recycle IP address
  static-bind          Static bind
[R1-GigabitEthernet0/0/0]dhcp server dns-list 8.8.8.8
[R1-GigabitEthernet0/0/0]dhcp server lease day 0 hour 0 minute 20
[R1-GigabitEthernet0/0/0]dhcp server excluded-ip-address 192.168.10.20 192.168.1
0.253
[R1-GigabitEthernet0/0/0]int g0/0/1
[R1-GigabitEthernet0/0/1]dhcp select interface 
[R1-GigabitEthernet0/0/1]dhcp server dns-list 8.8.8.8
```



#### 2.交换机DHCP服务器配置

- 涉及二层交换机之间接入配置  `access`
- 三层交换机互联vlan配置          `trunk`
- DHCP配置

​		和路由器配置差不多，只不过对于三层交换机，最后是在指定的<font color='red'>逻辑接口，即vlan</font>进行设置开启地址池的模式。

<img src="./Ensp%E5%AE%9E%E9%AA%8C.assets/image-20240714195736710.png" alt="image-20240714195736710" style="zoom:50%;" />

```
<Huawei>sys
Enter system view, return user view with Ctrl+Z.
[Huawei]sysname sw1
[sw1]undo info-center enable 
Info: Information center is disabled.
[sw1]vlan batch 10 20
Info: This operation may take a few seconds. Please wait for a moment...done.
[sw1]int vlan 10
[sw1-Vlanif10]int g0/0/2
[sw1-GigabitEthernet0/0/2]port link-type access
[sw1-GigabitEthernet0/0/2]port default vlan 10
[sw1-GigabitEthernet0/0/2]int g0/0/3
[sw1-GigabitEthernet0/0/3]port link-type access
[sw1-GigabitEthernet0/0/3]port default vlan 20
[sw1-GigabitEthernet0/0/3]int g0/0/1
[sw1-GigabitEthernet0/0/1]port link-type trunk
[sw1-GigabitEthernet0/0/1]port trunk allow-pass vlan 10 20
[sw1-GigabitEthernet0/0/1]dis vlan 10
--------------------------------------------------------------------------------
U: Up;         D: Down;         TG: Tagged;         UT: Untagged;
MP: Vlan-mapping;               ST: Vlan-stacking;
#: ProtocolTransparent-vlan;    *: Management-vlan;
--------------------------------------------------------------------------------

VID  Type    Ports                                                          
--------------------------------------------------------------------------------
10   common  UT:GE0/0/2(U)                                                      
             TG:GE0/0/1(U)                                                      

VID  Status  Property      MAC-LRN Statistics Description      
--------------------------------------------------------------------------------
10   enable  default       enable  disable    VLAN 0010                         
[sw1-GigabitEthernet0/0/1]dis vlan 20
--------------------------------------------------------------------------------
U: Up;         D: Down;         TG: Tagged;         UT: Untagged;
MP: Vlan-mapping;               ST: Vlan-stacking;
#: ProtocolTransparent-vlan;    *: Management-vlan;
--------------------------------------------------------------------------------

VID  Type    Ports                                                          
--------------------------------------------------------------------------------
20   common  UT:GE0/0/3(U)                                                      
             TG:GE0/0/1(U)                                                      

VID  Status  Property      MAC-LRN Statistics Description      
--------------------------------------------------------------------------------
20   enable  default       enable  disable    VLAN 0020 
```

```cmd
<Huawei>sys
Enter system view, return user view with Ctrl+Z.
[Huawei]sysname sw3
[sw3]undo info-center enable 
[sw3]dhcp enable
Info: Information center is disabled.
[sw3]vlan batch 10 20
Info: This operation may take a few seconds. Please wait for a moment...done.
[sw3]int vlan 10
[sw3-Vlanif10]ip address 192.168.10.254 24
[sw3-Vlanif10]int vlan 20
[sw3-Vlanif20]ip address 192.168.20.254 24
[sw3-Vlanif20]int g0/0/1
[sw3-GigabitEthernet0/0/1]port link-type trunk
[sw3-GigabitEthernet0/0/1]port trunk allow-pass vlan 10 20
[sw3-GigabitEthernet0/0/1]int vlan 10
[sw3-Vlanif10]dhcp select interface
[sw3-Vlanif10]dhcp server dns-list 8.8.8.8

[sw3-Vlanif10]int vlan 20
[sw3-Vlanif20]dhcp select interface
[sw3-Vlanif20]dhcp server dns-list 8.8.8.8
```



#### 3.单臂路由DHCP服务器配置

- 涉及二层交换机之间接入配置  `access` `trunk`
- 进入终结子接口（路由只有一个接口，分离出2个逻辑接口） `dot1q termination vid 10` 配置子接口dot1q终结的单层VLAN ID 
- 开启子接口主动发送ARP请求报文，ARP广播  `arp broadcast enable`
- DHCP配置

​		。

```
<Huawei>sys
Enter system view, return user view with Ctrl+Z.
[Huawei]sysname R1
[R1]undo info-center enable 
Info: Information center is disabled.
[R1]dhcp enable 
Info: The operation may take a few seconds. Please wait for a moment.done.
[R1]int g0/0/0.10
[R1-GigabitEthernet0/0/0.10]ip address 192.168.10.254 24
[R1-GigabitEthernet0/0/0.10]dhcp select interface
[R1-GigabitEthernet0/0/0.10]dhcp server dns-list 8.8.8.8
[R1-GigabitEthernet0/0/0.10]dot1q termination vid 10
[R1-GigabitEthernet0/0/0.10]arp broadcast enable 
```

<img src="./Ensp%E5%AE%9E%E9%AA%8C.assets/image-20240714204943080.png" alt="image-20240714204943080" style="zoom:50%;" />





## 静态路由配置

目的网段/掩码 出接口 下一跳

```cmd
[AR1]ip route-static 192.168.2.0 24 10.0.12.254 
```

```cmd
[AR2]ip route-static 192.168.1.0 24 10.0.12.1
```

![image-20240714224251145](./Ensp%E5%AE%9E%E9%AA%8C.assets/image-20240714224251145.png)

```
[AR2]dis ip routing-table 
Route Flags: R - relay, D - download to fib
------------------------------------------------------------------------------
Routing Tables: Public
         Destinations : 11       Routes : 11       

Destination/Mask    Proto   Pre  Cost      Flags NextHop         Interface

      10.0.12.0/24  Direct  0    0           D   10.0.12.1       GigabitEthernet
0/0/1
      10.0.12.1/32  Direct  0    0           D   127.0.0.1       GigabitEthernet
0/0/1
    10.0.12.255/32  Direct  0    0           D   127.0.0.1       GigabitEthernet
0/0/1
      127.0.0.0/8   Direct  0    0           D   127.0.0.1       InLoopBack0
      127.0.0.1/32  Direct  0    0           D   127.0.0.1       InLoopBack0
127.255.255.255/32  Direct  0    0           D   127.0.0.1       InLoopBack0
    192.168.1.0/24  Direct  0    0           D   192.168.1.254   GigabitEthernet
0/0/0
  192.168.1.254/32  Direct  0    0           D   127.0.0.1       GigabitEthernet
0/0/0
  192.168.1.255/32  Direct  0    0           D   127.0.0.1       GigabitEthernet
0/0/0
    192.168.2.0/24  Static  60   0          RD   10.0.12.254     GigabitEthernet
0/0/1
255.255.255.255/32  Direct  0    0           D   127.0.0.1       InLoopBack0
```

