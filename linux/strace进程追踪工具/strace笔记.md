# strace笔记

## 1. 概念

strace常用来<font color='red'>跟踪进程执行</font>时的**`系统调用`**和**`所接收的信号`**。

>  在Linux世界，进程不能直接访问硬件设备，当进程需要访问硬件设备(比如读取磁盘文件，接收网络数据等等)时，必须由用户态模式切换至内核态模式，通过系统调用访问硬件设备。strace可以跟踪到一个进程产生的系统调用,包括参数，返回值，执行消耗的时间。



**【示例1】：**追踪`cat /dev/null`

```
# strace cat /dev/null
execve("/bin/cat", ["cat", "/dev/null"], [/* 22 vars */]) = 0
brk(0)                                  = 0xab1000
access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f29379a7000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
...
```

> 每一行都是一条系统调用，等号左边是系统调用的函数名及其参数，右边是该调用的返回值。



## 2. 命令行参数

|       参数       | 说明                                                         |
| :--------------: | :----------------------------------------------------------- |
|        -c        | 统计每一次系统调用所执行的时间，次数和出错次数等             |
|        -d        | 显示有关标准错误的strace本身的一些调试输出。                 |
|        -f        | 跟踪子进程，这些子进程是由于fork（2）系统调用而由当前跟踪的进程创建的。 |
|        -i        | 在系统调用时打印指令指针。                                   |
|        -t        | 跟踪的每一行都以时间为前缀。                                 |
|       -tt        | 如果给出两次，则打印时间将包括微秒。                         |
|       -ttt       | 如果给定三次，则打印时间将包括微秒，并且前导部分将打印为自该**以来的秒数。 |
|        -T        | 显示花费在系统调用上的时间。这将记录每个系统调用的开始和结束之间的时间差。 |
|        -v        | 打印环境，统计信息，termios等调用的未缩写版本。这些结构在调用中非常常见，因此默认行为显示了结构成员的合理子集。使用此选项可获取所有详细信息。 |
|        -V        | 打印strace的版本号。                                         |
|     -e expr      | 限定表达式，用于修改要跟踪的事件或如何跟踪它们：             |
|   -e trace=set   | 仅跟踪指定的系统调用集。该-c选项用于确定哪些系统调用可能是跟踪有用有用。例如，trace=open，close，read，write表示仅跟踪这四个系统调用。 |
|  -e trace=file   | 跟踪所有以文件名作为参数的系统调用。                         |
| -e trace=process | 跟踪涉及过程管理的所有系统调用。这对于观察进程的派生，等待和执行步骤很有用。 |
| -e trace=network | 跟踪所有与网络相关的系统调用。                               |
| -e trace=signal  | 跟踪所有与信号相关的系统调用。                               |
|   -e trace=ipc   | 跟踪所有与IPC相关的系统调用。                                |
|    -o 文件名     | 将跟踪输出写入文件而不是stderr。                             |
|      -p pid      | 使用进程ID pid附加到该进程并开始跟踪。跟踪可以随时通过键盘中断信号（CTRL -C）终止。 |
|        -S        | 按指定条件对-c选项打印的直方图输出进行排序。                 |

【实例二】

> 跟踪udp_client进程的所有系统调用（-e trace=all），并统计系统调用的花费时间，以及开始时间（并以可视化的时分秒格式显示），最后将记录结果存在output.txt文件里面

```
strace -o output.txt -T -tt -e trace=all  ./udp_client 127.0.0.1 8899
```

【实例三】：

> 追踪myserver进程，同时追踪它所fork出的子进程，并将输出记录到txt文件。

```
strace -f -o straceout.txt myserver
```

