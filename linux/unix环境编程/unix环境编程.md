# UNIX环境编程





## 第3章 文件I/O

> 常用的5种I/O函数：open、read、write、lseek和close。

### 3.1 文件描述符

> include <unistd.h>

文件描述符：一个非负整数。  **0 ~ OPEN_MAX-1**

创建时机：打开一个现有文件或新建文件时，内核会向进程返回一个文件描述符。

三种标准输入、标准输出以及标准错误：**STDIN_FILENO（0）、STDOUT_FILENO（1）、STDERR_FIFENO（2）**



### 3.3 文件I/O相关的各种函数

#### 3.3.1 open和openat



#### 3.3.2 creat

#### 3.3.3 close

#### 3.3.4 lseek







## 第7章 进程环境

### 7.1 进程终止

#### 7.1.1 正常终止（5种）

1. 从main返回
2. 调用exit函数
3. 调用_exit函数或\_Exit
4. 最后一个线程从启动例程返回（11.5节）
5. 从最后一个线程调用pthread_exit（11.5节）

#### 7.1.2 异常终止（3种）

1. 调用abort
2. 接到一个信号
3. 最后一个线程对取消请求做出响应（11.5节和12.7节）

#### 7.1.3 退出函数

```c
#include <stdlib.h>
// 这两个终止一个程序，都会立即进入内核
void exit(int status);
void _Exit(int status);

#include <unistd.h>
// 先执行一些清理操作，然后返回进入内核
void _exit(int status);
```

status: 终止状态（退出状态）

> 若main函数终止时没有显示使用return语句或调用exit函数，则进程的终止状态是未定义的。
>
> ​     exit(0) 等价于 return (0)   // main函数返回一个整型值与该值调用exit是等价的。



**进程终止状态为未定义：**

1. main执行了一个不带返回值的return语句
2. main的返回值类型不为int
3. 调用3个退出函数不带终止状态



#### 7.1.4 atexit登记终止处理程序函数

一个进程可以登记至多32个函数（终止处理程序），这些函数被exit()自动调用，由atexit()来登记这些函数。(调用的顺序与登记顺序相反)

```c
int atexit(void (*func)(void));
```

```c
static void my_exit1(void){
    printf("first exit handler\n");
}

static void my_exit2(void){
    printf("second exit handler\n");
}

static void my_exit2(void);

int main(void){
  if(atexit(my_exit2) != 0)
  	err_sys("can't register my_exit2");
  	
  if(atexit(my_exit1) != 0)
  	err_sys("can't register my_exit1");
    
  if(atexit(my_exit1) != 0)
  	err_sys("can't register my_exit1");
    
  printf("main is done\n");
  return 0;
}

// main is done
// first exit handler  每登记一次就会被执行一次
// first exit handler
// second exit handler
```

![image-20240429234715800](.assets/image-20240429234715800.png)

exit首先会调用所有终止处理程序，然后通过fclose关闭所有打开流。

> 进程自愿终止的唯一方法：
>
> - 显示调用_exit或\_Exit。
> - 通过exit隐式调用_exit或\_Exit
>
> 
>
> 进程非自愿终止：由一个信号来终止







## 第8章 进程控制

### 8.1 进程标识

每个进程ID是一个非负整数的唯一ID。

> pid = 0: 调度进程（交换进程），是内核的一部分，但它不执行任何磁盘上的程序。   系统进程
>
> pid = 1: init进程（决不终止）   负责在自举过程结束时由内核调用。  用户进程       /sbin/init          
>
> pid = 2：页守护进程  负责支持虚拟存储器系统的分页操作

### 8.2 获取进程有关ID的函数

```c
pid_t getpid(void);    // 进程ID
pid_t getppid(void);   // 父进程ID
 
uid_t getuid(void);    // 进程实际用户ID
uid_t geteuid(void);   // 进程有效用户ID

gid_t getgid(void);    // 进程实际组ID
gid_t getegid(void);   // 进程有效组ID
```



### 8.3 进程常用函数

```c
pid_t fork(void);
```

#### 8.3.1 fork

fork调用一次，返回两次。

- 子进程返回0
- 父进程返回新建子进程的进程ID



### 8.6  wait系列

#### 8.6.1 wait和waitpid

> 区别：
>
> - waitpid可等待一个特定的进程，而wait返回任一终止子进程的状态。
> - waitpid提供了一个wait的非阻塞版本。
> - waitpid通过WUNTRACED和WCONTINUED选项支持作业控制。

```c
pid_t wait(int *statloc); // 会阻塞调用者，直至子进程终止后获取到其终止状态后返回

pid_t waitpid(pid_t pid, int *statloc, int options); // 根据选项可设置不阻塞
/**
* - pid: 
*		=-1，等待任一子进程，等价于wait
*		>0, 等待·进程ID·与·pid·相等的子进程
*		=0，等待·组ID·等于·调用进程组ID·的任一子进程
*		<-1，等待·组ID·等于pid绝对值的任一子进程
* - statloc: 若不是空指针，则终止进程的终止状态就存放在它所指向的单元内。
* Return: 其返回值为终止子进程的进程ID，并将该子进程的终止状态存放在由statloc指向的存储单元中。
*/
```

当进程正常/异常终止时，内核就会向其父进程发送**SIGCHID信号**。父进程可以选择忽略该信号，或调用注册的SIGCHID信号处理程序。

调用wait和waitpid的进程可能会发生什么（3种）？

- 如果其所有子进程都还在运行，则调用后被阻塞。
- 如果一个子进程已终止，正等待父进程获取其终止状态，则获取到该子进程的终止状态后立即返回。
- 如果它没有任何子进程，则立即出错返回。

##### 1. 终止状态的宏

| 宏                   | 说明                                                         |
| -------------------- | ------------------------------------------------------------ |
| WIFEXITED(status)    | 若为**正常终止**子进程返回的状态，则为真。可执行WEXITSTATUS(status)，获取子进程传给exit或_exit参数的低8位。 |
| WIFSIGNALED(status)  | 若为**异常终止**子进程返回的状态，则为真。                   |
| WIFSTOPPED(status)   | 若为**当前暂停**子进程的返回的状态，则为真。                 |
| WIFCONTINUED(status) | 若在作业控制暂停后已经继续执行的子进程返回了状态，则为真。仅用于waitpid. |

##### 2. waitpid函数的options选项

```c
pid_t waitpid(pid_t pid, int *statloc, int options); // 根据选项可设置不阻塞
```

| 常量       | 说明                                                         |
| ---------- | ------------------------------------------------------------ |
| WCONTINUED | 若实现支持作业控制，那么由pid指定的任一子进程在停止后已经继续，但其状态尚未报告，则返回其状态。 |
| WNOHANG    | 若pid指定的子进程并不是立即可用的，则waitpid不阻塞，此时其返回值为0 |
| WUNTRACED  | 若某实现支持作用控制，而由pid指定的任一子进程已处于停止状态，并且其状态停止以来还未报告过，则返回其状态。 |



#### 8.6.2 waitid

与waitpid类似，获取进程终止状态的函数

```c
 int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
```

<img src="./unix环境编程.assets/image-20240918133856232.png" alt="image-20240918133856232" style="zoom: 80%;" />

> WCONTINUED、WEXITED和WSTOPPED三个常量之一必须在options参数中指定。



## 10. 进程间通信

​		进程间通信（IPC）是指在多个进程之间传输数据或共享信息的机制。

> 在OS中，每个进程都拥有独立的地址空间和资源。通过IPC机制可以实现进程间的数据交换和协作。

<img src="./unix环境编程.assets/image-20240808230915755.png" alt="image-20240808230915755" style="zoom:50%;" />

- **进程间通信的目的**
  - **数据交换**
    - 实现在不同进程之间传递数据。
  - **共享资源**
    - 实现多个进程共享资源，如共享内存、文件、设备等。
  - **进程同步**
    - 用于实现进程之间的同步操作。
  - **消息传递**
    - 可以通过消息传递的方式实现进程间的通信和协作。

### 10.1 进程间通信方法

分为四类：管道、System V IPC、Posix IPC、套接字。

<img src="./unix环境编程.assets/image-20240808231606165.png" alt="image-20240808231606165" style="zoom:80%;" />

#### 10.1.1 管道

##### 1. 无名管道

用于在具有亲缘关系的进程间通信。



**特点：**

- 半双工的通信方式，数据只能单向流动。 `一方负责发数据，另一方只能接收数据`
- 以字节流方式通信，数据格式由用户自定义。
- 多用于父子进程间通信，也可用于其他亲缘关系的进程间通信。 



**实现原理：**

1. 父进程调用pipe函数创建两个文件（**读管道文件、写管道文件**）。`对应的文件节点pipe inode(无名管道的)`

<img src="./unix环境编程.assets/image-20240808232725492.png" alt="image-20240808232725492" style="zoom: 50%;" />

2. 父进程调用fork()函数创建子进程，此时，子进程会拷贝父进程的文件表。即父子进程指向相同的文件（操作同一管道）

<img src="./unix环境编程.assets/image-20240808232937641.png" alt="image-20240808232937641" style="zoom: 50%;" />

3. 父进程调用close()函数关闭读文件描述符，子进程调用close()函数关闭写文件描述符。 

<img src="./unix环境编程.assets/image-20240808233145771.png" alt="image-20240808233145771" style="zoom:50%;" />

> 为什么匿名管道只能用在具有亲缘关系的进程间通信？
>
> ------  无亲缘关系的进程拥有的文件表不同，所以不能访问相同的文件，导致多个进程无法通过匿名管道来进行通信。





##### 2. 命名管道

通过FIFO文件（也称命名管道）来进行不同进程间的通信。

不同进程通过写入和读取相同FIFO文件进行通信。           `半双工通信，调用一次mkfifo函数，只生成一个管道（缓冲区）`



**特点：**

- FIFO文件位于文件系统中，可以像其他文件一样进行访问和管理。
- FIFO文件可以通过文件名进行识别和引用，而不仅仅依赖于文件描述符。
- FIFO文件可以在不同进程之间进行双向通信，允许同时进行读取和写入操作。



```c
// 1. 创建命名管道文件 FIFO文件
int mkfifo(const char* pathname, mode_t mode);
```

**实现原理：**

1.  进程通过调用`mkfifo函数`创建一个FIFO文件，FIFO文件对应一个fifo inode（底层和pipe inode实现相同）

   ![image-20240809135022984](./unix环境编程.assets/image-20240809135022984.png)

2. 进程调用`open函数`打开FIFO文件，由于每个文件都对应唯一的inode节点，所以每个进程打开的是相同的inode节点。
3. 进程调用`write或read函数`来进行读写管道，实现进程间通信。

<img src="./unix环境编程.assets/image-20240809135354503.png" alt="image-20240809135354503" style="zoom:80%;" />



#### 10.1.2 System V消息队列

它允许在同一系统上运行的不同进程之间传递消息。

**优点：**

- 可以实现独立的进程间通信，不受进程的启动和结束顺序的影响。
- 允许多个进程同时向消息队列中写入和读取消息，实现了并发处理。
- 通过消息优先级机制，可以优先处理重要的消息。



**实现原理：**

具有相同IPC命名空间的进程能**同时访问IPC命名空间相同内存空间**。

![image-20240809143004516](./unix环境编程.assets/image-20240809143004516.png)



```
1. Key: 整数型，用来索引IPC对象（根据哈希表），检索成功返回标识ID；失败则创建IPC对象并返回标识ID。
2. 标识ID：整数型，用来索引IPC对象（根据radix树），并不是由Key产生，而是由其他信息（如序号）通过算法产生。
3. IPC对象：消息队列继承IPC对象，提供一些通用信息
```

![image-20240809144057314](./unix环境编程.assets/image-20240809144057314.png)

3. Key的生成（三种）

   - **第一种：**通过msgget()函数，使用一个随机整数，如：

     ```c
     /**
     *	创建或获取一个System V消息队列描述符
     *   arg： key 唯一标识一个消息队列，可取由ftok创建的key值或指定的一个非负整数值
     *         msgflg 指定创建消息队列的权限和选项。
     *                当使用IPC_CREAT | IPC_EXCL时，若消息队列不存在则创建，否则返回失败并设置erron标识为EEXIST
     *   
     *   return: 标识ID（成功）  失败返回-1，并设置errno
     */
     int msgget(key_t key, int msgflg);
     ```

     **msgget作用：**

      	1. 通过Key查找IPC对象（消息队列）
      	 - 查找成功 ，直接返回标识ID。说明Key已由IPC对象映射。
      	 - 查找失败，说明Key对应的IPC对象未创建，需创建一个新的IPC对象。创建的新的IPC对象需根据序号信息生成一个标识ID，再根据标识ID插入radix树。
      	2. 返回一个标识ID（msgid）	
      	 - 后续进程间通信，通过标识ID查找消息队列进行通信

   |      msgflg参数      | IPC对象存在 |        IPC对象不存在        |
   | :------------------: | :---------: | :-------------------------: |
   |          0           | 返回标识ID  |           返回-1            |
   |      IPC_CREATE      | 返回标识ID  | 创建IPC对象，返回新的标识ID |
   | IPC_CREATE\|IPC_EXEC |   返回-1    | 创建IPC对象，返回新的标识ID |

   

   

   - **第二种：**通过msgget()函数，使用IPC_PRIVATE标识生成，只用于**亲缘关系的进程**

     ```c
     msgget(IPC_PRIVATE,0644)
     ```

     

   - **第三种：**使用ftok()函数生成**（推荐）**

     ```c
     // 通过将pathname的i-node号和proj_id进行组合来生成一个唯一键值
     // arg:    pathname：存在的路径名
     //         proj_id：一个整数
     // return: Key值（成功）  失败返回-1，并设置errno
     key_t ftok(const char *pathname, int proj_id);
     ```

     

##### 1. 发送消息 msgsnd

将一个消息添加到指定的消息队列中

```c
/**
*	arg: msqid 消息队列的标识ID
*		 msgp 指向要发送的消息的指针
* 		 msgsz 要发送的消息的大小。 是消息mtext数据长度
*		 msgflg 控制发送消息的行为
*   return:
*        成功：返回实际发送消息的大小
*        失败返回-1，并设置errno
*/
int msgsnd(int msqid, const void msgp[.msgsz], size_t msgsz, int msgflg);
```

```c
struct msgbuf {
      long mtype;       /* message type, must be > 0 */
      char mtext[1];    /* message data 柔性数组*/   
};
```

##### 2. 接收消息 msgrcv

```c
/**
*	arg: msqid 消息队列的标识ID
*		 msgp 指向接收消息的缓冲区
* 		 msgsz 接收消息的最大长度。    为0表示接收队列中的第一个消息
*		 msgflg 接收消息的行为，控制接收操作的行为
*   return:
*        成功：返回实际接收消息的长度
*        失败返回-1，并设置errno
*/
int msgsnd(int msqid, const void msgp[.msgsz], size_t msgsz, int msgflg);
```



##### 3. 控制操作 msgctl

```c
/**
*	arg: msqid 消息队列的标识ID
*		 op 指定要执行的操作  
*			- IPC_STAT：获取消息队列的状态
*			- IPC_SET：设置消息队列的状态
*			- IPC_RMID：删除消息队列
*
* 		 buf 指向一个结构体msqid_ds指针，用于传递或获取消息队列的属性信息
*
*   return:
*        成功：IPC_STAT,IPC_SET
*        失败：返回-1，并设置errno
*/
int msgctl(int msqid, int op, struct msqid_ds *buf);
```

【使用示例】

```v
// 1. 获取消息队列状态
struct msqid_ds buf = {0};
int msgctl(msqid, IPC_STAT, &buf);

// 2. 设置消息队列的状态
struct msqid_ds buf = {0};
int msgctl(msqid, IPC_SET, &buf);

// 3. 删除消息队列
int msgctl(msqid, IPC_RMID, 0);
```





### 11. 线程

#### 11.1 创建线程 pthread_create

```c
typedef unsigned long int pthread_t;
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);
```

#### 11.2 获取线程ID pthread_self

```c
// 获取自身的线程ID
pthread_t pthread_self(void);
// 比较两个线程ID是否相等
int pthread_equal(pthread_t t1, pthread_t t2);
```

#### 11.3 终止线程 pthread_exit

如果线程中调用了exit、\_Exit或_exit，那么整个进程都会终止。

单个线程可以通过3种方式在终止整个进程的情况下退出：

- 线程可以简单的从启动例程中返回，返回值是线程的退出码 `return`
- 线程可以被同一进程的其他线程取消 `pthread_cancel`
- 线程调用pthread_exit函数 `pthread_exit`

```c
void pthread_exit(void *retval);
```

> 当某个线程调用pthread_exit方法后，该线程会被关闭（相当于return）。
>
> 并通过retval向其他线程传递信息，retval指向的区域不可以放在线程函数的栈内。
>
> 其他线程（如主线程）如果想要获取这个返回值，则需要调用pthread_join方法。



#### 11.4 取消线程 pthread_cancel

向目标线程发送取消请求，请求取消同一进程中的其他线程。

```
int pthread_cancel(pthread_t tid);
```



#### 11.5 等待线程退出 pthread_join

> 等待指定线程结束，获取目标线程的返回值，并在目标线程结束后回收它的资源

```c
int pthread_join(pthread_t thread, void **retval);
```

调用线程将一直阻塞，直到指定的线程调用pthread_exit、从启动例程中返回或被取消。如果线程简单的从它的启动例程中返回，retval就包含线程的返回值。如果线程被取消，由retval指定的内存单元就设置为PTHREAD_CANCELED。



#### 11.6 分离线程 pthread_detach（不阻塞等待线程退出）

将线程标记为detached状态。POSIX线程终止后，如果没有调用pthread_detach或pthread_join，其资源会继续占用内存，类似于僵尸进程的未回收状态，默认情况下，它处于可join状态。此时可以调用pthread_join等待线程终止并回收资源。

如果主线程不需要等待线程终止，可以将其标记为detached状态，即表示线程终止后，其资源会被系统自动回收。

```c
// 使用detach标记会等待线程完成后回收相关资源
int pthread_detach(pthread_t thread);
```



### 12. 线程同步

#### 12.1 互斥锁

```c
// 初始化锁  静态分配，使用完无需释放
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// 动态方式初始化锁，需显示销毁
pthread_mutex_init(pthread_mutex_t &mutex);
```

```c
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

#### 12.2 读写锁

```c
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
int pthread_rwlock_init(pthread_rwlock_t *restrict rwlock,
           const pthread_rwlockattr_t *restrict attr);
```

```c
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
```

避免写饥饿

```c
pthread_rwlockattr_t attr;
pthread_rwlockattr_init(&attr);
// 修改参数，设置写优先
pthread_rwlockattr_setkind_np(&attr,PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
// 初始化读写锁
pthread_rwlock_init(&rwlock, &attr);
```

