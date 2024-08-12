

   本 RFC 为互联网社区指定了一个 IAB 标准跟踪协议，并请求讨论和提出改进建议。
   社区的 IAB 标准跟踪协议，并请求讨论和提出改进建议。
   请参阅当前版本的 "IAB 官方协议标准
   标准"，了解该协议的标准化状态和现状。
   本备忘录的分发不受限制。

https://datatracker.ietf.org/doc/html/rfc1350

### TFTP包

操作码（Opcode operation）操作
            1 读取请求 (RRQ)
            2 写入请求 (WRQ)
            3 数据 (DATA)
            4 确认 (ACK)
            5 错误 (ERROR)

数据包的 TFTP 头包含与该数据包相关的操作码。



![image-20240812173326074](C:\Users\huang\AppData\Roaming\Typora\typora-user-images\image-20240812173326074.png)

Opcode操作码:

- 1为读请求包，Read Request（RRQ）
- 2为写请求包，Write Request（WRQ）
- 3为文件数据包，Data（DATA）
- 4为回应包：Acknowledgement（ACK）
- 5为错误信息包：Error（ERROR）



####  TFTP协议支持三种传输模式

- netascii：ASCII文本模式     `接收到该类型数据需要转换为自定义格式数据`
- octet：二进制模式，每字节8位  `用于传输8位格式的文件`
- mail：现在已经不使用



#### RRQ和WRQ请求包字段介绍

- FileName: 以 netascii 表示的以 0 字节结束字节序列。 。



 假设假定每种类型的机器都有一种比较通用的 8 位格式。
   格式，并选择该格式。 例如，在
   DEC-20（36 位机器）上，一个字有 4 个 8 位字节，4 位断点。
   四位断点。 如果主机收到一个八进制文件，然后
   返回，则返回的文件必须与原始文件完全相同。
   邮件模式使用邮件收件人的名称代替文件，必须以 WRQ 开始。
   必须以 WRQ 开始。 否则与 netascii 模式相同。
   邮件收件人字符串应为 "username "或
   "username@hostname"。 如果使用第二种形式，则允许
   选择由中继计算机转发邮件。

![image-20240812180419402](C:\Users\huang\AppData\Roaming\Typora\typora-user-images\image-20240812180419402.png)

![image-20240812180426615](C:\Users\huang\AppData\Roaming\Typora\typora-user-images\image-20240812180426615.png)

![image-20240812180510270](C:\Users\huang\AppData\Roaming\Typora\typora-user-images\image-20240812180510270.png)