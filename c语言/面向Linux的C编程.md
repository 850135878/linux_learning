# 面向Linux的C编程

## 1. 进程

### 1.1 常用库函数

#### 1.1.1 查看进程相关信息

```
#include <unistd>; 
#include <sys/types.h>; 
```

1. 查看进程ID

```c
pid_t getpid(void); 
pid_t getppid(void);
```

2. 查看进程用户相关信息

```c
uid_t getuid(void);   // 获取进程所有者的ID 
uid_t geteuid(void);  // 获取进程的有效用户ID
gid_t getgid(void);   // 获取进程的组ID
git_t getegid(void);  // 获取进程的有效组ID
```

```c
struct passwd { 
char *pw_name; /* 登录名称 */ 
char *pw_passwd; /* 登录口令 */ 
uid_t pw_uid; /* 用户 ID */ 
gid_t pw_gid; /* 用户组 ID */ 
char *pw_gecos; /* 用户的真名 */ 
char *pw_dir; /* 用户的目录 */ 
char *pw_shell; /* 用户的 SHELL */ 
};

#include <pwd.h>; 
// 获取进程所属用户的其他信息，如登录名称、登录口令等
struct passwd *getpwuid(uid_t uid);
```



#### 1.1.2 创建子进程

##### 1. fork

```
# 子进程和父进程不同的地方只有他的进程ID和父进程ID,其他的都是一样
pid_t fork();
```

fork调用一次，返回两次。

- 子进程返回0
- 父进程返回新建子进程的进程ID



##### 2. wait or waitpid

1. 进程阻塞函数

```c
// wait系统调用会使父进程阻塞直到一个子进程结束或者是父进程接受到了一个信号

// param:
//    -stat_loc: 子进程的退出状态
// return:
//    调用成功返回子进程ID，否则返回-1,并设置全局变量 errno.
//	  子进程通过调用exit，_exit 或者是 return 来设置这个值
pid_t wait(int *stat_loc); 


// param:
//    -pid: 指定的子进程（>0）
//			=0,则等待任何一个组ID和调用者的组ID相同的进程
// 			=-1 时等同于 wait 调用
//			<-1 时等待任何一个组ID等于pid绝对值的进程
//    -stat_loc: 子进程的退出状态
//    -options: 决定父进程的状态
//			WNOHANG：父进程立即返回当没有子进程存在时
//			WUNTACHED：当子进程结束时 waitpid 返回,但是子进程的退出状态不可得到.
// return:
pid_t waitpid(pid_t pid,int *stat_loc,int options);
```

> 定义了几个宏来测试这个返回值. 
>
> - WIFEXITED：判断子进程退出值是非 0 
>
> - WEXITSTATUS：判断子进程的退出值(当子进程退出时非 0). 
>
> - WIFSIGNALED：子进程由于有没有获得的信号而退出. 
>
> - WTERMSIG：子进程没有获得的信号号(在 WIFSIGNALED 为真时才有意义). 

```c
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
void main(void)
{
    pid_t child;
    int status;
    printf("This will demostrate how to get child status\n");
    if ((child = fork()) == -1)
    {
        printf("Fork Error ：%s\n", strerror(errno));
        exit(1);
    }
    else if (child == 0)
    {
        int i;
        printf("I am the child：%d\n", getpid());
        for (i = 0; i < 1000000; i++)
            sin(i);
        i = 5;
        printf("I exit with %d\n", i);
        exit(i);
    }
    while (((child = wait(&status)) == -1) & (errno == EINTR))
        ;
    if (child == -1)
        printf("Wait Error：%s\n", strerror(errno));
    else if (!status) // 子进程状态为0
        printf("Child %d terminated normally return status is zero\n", child);
    else if (WIFEXITED(status)) // 非0
        printf("Child %d terminated normally return status is %d\n", child, WEXITSTATUS(status));
    else if (WIFSIGNALED(status)) // 子进程由于有没有获得的信号而退出
        printf("Child %d terminated due to signal %d not caught\n", child, WTERMSIG(status));
}
```



##### 3. exec函数族

exec 族调用可以执行给定程序，man exec 

```c
int execl(const char *path,const char *arg,...); 
int execlp(const char *file,const char *arg,...); 
int execle(const char *path,const char *arg,...); 
int execv(const char *path,char *const argv[]); 
int execvp(const char *file,char *const argv[])：
```



#### 1.1.3 创建守护进程

守护进程：是一种长期运行的进程，一般是操作系统启动的时候它就启动，操作系统关闭的时候它才关闭。

守护进程跟终端无关联，即它们没有控制终端，所以控制终端退出，也不会导致守护进程退出。

守护进程是在后台运行的，不会占着终端，终端可以执行其他命令。



比如，linux下的`init` 是系统守护进程，它负责启动各运行层次特定的系统服务，所以很多进程的 PPID 是 `init`，并且这个 `init` 也负责收养孤儿进程。

```
# e：显示所有进程
# l：以长格式显示信息，如各种字段信息
# f：全格式显示  TTY（控制终端）
ps -elf
```

![image-20240802102732078](./面向Linux的C编程.assets/image-20240802102732078.png)

![image-20240802102800584](./面向Linux的C编程.assets/image-20240802102800584.png)

大多数守护进程都是以超级用户特权运行的，守护进程没有控制终端，`TTY` 这列显示的是 `?`

- 内核守护进程以无控制终端方式启动
- 普通守护进程无控制终端可能是守护进程调用了 `setsid` 的结果



**守护进程的创建思想：**

- 父进程创建一个子进程  `fork()`
- 子进程杀死父进程  `kill(getppid(),SIGTERM)`
- 信号处理所有的工作由子进程来处理

```c
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <signal.h>  
#include <syslog.h>  
  
// 函数声明  
void daemonize(const char *cmd);  
  
int main(int argc, char *argv[]) {  
    const char *cmd = "mydaemon"; // 守护进程名，用于日志  
  
    // 守护化进程  
    daemonize(cmd);  
  
    // 守护进程主循环  
    while (1) {  
        // 这里可以添加守护进程的主要任务  
        syslog(LOG_INFO, "Daemon is running.");  
        sleep(10); // 休眠10秒  
    }  
  
    return 0;  
}  
  
void daemonize(const char *cmd) {  
    pid_t pid;  
    // 第一步：创建子进程，父进程退出  
    pid = fork();  
    if (pid < 0)  
        exit(EXIT_FAILURE);  
    if (pid > 0)  
        exit(EXIT_SUCCESS);  
    // 第二步：在子进程中创建新会话  
    if (setsid() < 0)  
        exit(EXIT_FAILURE);  
  
    // 第三步：改变当前工作目录  
    // 使用chdir()来改变工作目录到根目录，或者任何目录，但不能是挂载的文件系统  
    if (chdir("/") < 0)  
        exit(EXIT_FAILURE);  
  
    // 第四步：关闭标准文件描述符  
    close(STDIN_FILENO);  
    close(STDOUT_FILENO);  
    close(STDERR_FILENO);  
  
    // 第五步：可选：打开/dev/null，并重定向标准文件描述符  
    int fd = open("/dev/null", O_RDWR);  
  
    if (fd != STDIN_FILENO) {  
        dup2(fd, STDIN_FILENO);  
        close(fd);  
    }  
  
    if (fd != STDOUT_FILENO) {  
        dup2(fd, STDOUT_FILENO);  
        close(fd);  
    }  
  
    if (fd != STDERR_FILENO) {  
        dup2(fd, STDERR_FILENO);  
        close(fd);  
    }  
  
    // 第六步：处理SIGCHLD信号  
    signal(SIGCHLD, SIG_IGN);  
  
    // 第七步：可选：记录守护进程的PID  
    // 这里没有实现，但通常可以写入到/var/run/目录下的某个文件中  
  
    // 第八步：守护进程现在可以继续执行其任务  
    // 注意：使用syslog来记录日志，而不是标准输出或错误输出  
    openlog(cmd, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);  
    syslog(LOG_NOTICE, "Daemon started by %s", getlogin());  
}
```





## 2. 文件操作

### 2.1 打开/关闭文件

```c
int open(const char *pathname,int flags); 
// flags为O_CREAT时使用
int open(const char *pathname,int flags,mode_t mode); 
int close(int fd);
```

flags标识位，前三位只能用一个

> 1. O_RDONLY：以只读的方式打开文件. 
>
> 2. O_WRONLY：以只写的方式打开文件. 
>
> 3. O_RDWR：以读写的方式打开文件. 
>
> 4. O_APPEND：以追加的方式打开文件. 
>
> 5. O_CREAT：创建一个文件. 
>
> 6. O_EXEC：如果使用了 O_CREAT 而且文件已经存在,就会发生一个错误. 
>
> 7. O_NOBLOCK：以非阻塞的方式打开一个文件. 
>
> 8. O_TRUNC：如果文件已经存在,则删除文件的内容. 

mode标识位

> **S_IRUSR 用户**可以读
>
> **S_IWUSR** 用户可以写
>
> **S_IXUSR** 用户可以执行
>
> **S_IRWXU** 用户可以读写执行

> S_I**R**GRP 组可以读
>
> S_I**W**GRP 组可以写
>
> S_I**X**GRP 组可以执行 
>
> S_I**RWX**G 组可以读写执行

> S_I**R**OTH 其他人可以读
>
> S_I**W**OTH 其他人可以写
>
> S_I**X**OTH 其他人可以执行
>
> S_I**RWX**O 其他人可以读写执行

> S_ISUID 设置用户执行 ID S_ISGID 设置组的执行 ID 

或者使用数字代替：5 个数字来表示文件的各种权限. 	

> 00000：第一位表示设置用户 ID.第二位表示设置组 ID,第三位表示用户自己的权限位,第四位表示组的权限,最后一位表示其他人		的权限. 
>
> rwx：1(1执行权限)，2(10写权限)，4(读权限)

```
# 比如我们要创建一个用户读写执行,组没有权限,其他人读执行的文件.设置用户ID位那么
open("temp",O_CREAT,10705);
```



### 2.2 读/写文件

```c
//read读取正常：返回值 = count
//	  读取异常：读到了文件的结尾或者被一个信号所中断,返回值(0或-1) < count。
//			- 如果是由信号中断引起返回,而且没有返回数据,read 会返回-1,且设置 errno 为 EINTR
//			- 当程序读到了文件结尾的时候,read 会返回 0.
ssize_t read(int fd, void *buffer,size_t count); 
ssize_t write(int fd, const void *buffer,size_t count);
```



### 2.3 文件属性

#### 2.3.1 access 检测文件权限

- 判断文件是否可以进行某种操作(读,写等等)

```c
// R_OK 文件可以读,W_OK 文件可以写,X_OK 文件可以执行,F_OK 文件存在.
// 有权限，返回0；否则如果有一个条件不符时,返回-1.
int access(const char *pathname,int mode);
```

#### 2.3.2 stat和fstat 获取文件的其他属性

- 获取文件的其他属性

```c
// stat 用来判断没有打开的文件
// fstat 用来判断打开的文件，使用最多的属性是 st_mode，来判断给定的文件是一个普通文件还是一个目录,连接等等
int stat(const char *file_name,struct stat *buf); 
int fstat(int filedes,struct stat *buf);
```

```c
struct stat { 
    dev_t st_dev; /* 设备 */ 
    ino_t st_ino; /* 节点 */ 
    mode_t st_mode; /* 模式 */ 
    nlink_t st_nlink; /* 硬连接 */ 
    uid_t st_uid; /* 用户 ID */ 
    gid_t st_gid; /* 组 ID */ 
    dev_t st_rdev; /* 设备类型 */ 
    off_t st_off; /* 文件字节数 */ 
    unsigned long st_blksize; /* 块大小 */ 
    unsigned long st_blocks; /* 块数 */ 
    time_t st_atime; /* 最后一次访问时间 */ 
    time_t st_mtime; /* 最后一次修改时间 */ 
    time_t st_ctime; /* 最后一次改变时间(指属性) */ 
};

S_ISLNK(st_mode)//是否是一个连接
S_ISREG(st_mode)//是否是一个常规文件
S_ISDIR(st_mode)//是否是一个目录
S_ISCHR(st_mode)//是否是一个字符设备
S_ISBLK(st_mode)//是否是一个块设备 
S_ISFIFO(st_mode)//是否是一个FIFO文件
S_ISSOCK(st_mode)//是否是一个 SOCKET 文件
```



### 2.4 目录文件

#### 2.4.1 getcwd 查看当前的工作路径

```c
char *getcwd(char *buffer,size_t size);
```

#### 2.4.2 mkdir 创建文件夹

```c
int mkdir(const char *path,mode_t mode); 
```

#### 2.4.3 opendir

```c
DIR *opendir(const char *path); 
```

#### 2.4.4 readdir

读一个打开的目录

```c
struct dirent *readdir(DIR *dir); 
```

#### 2.4.5 rewinddir

重读目录的和学的 rewind 函数一样

```c
void rewinddir(DIR *dir); 
```

#### 2.4.6 telldir

telldir 和 seekdir 类似与 ftee 和 fseek 函数. 

```c
off_t telldir(DIR *dir); 
```

#### 2.4.7 seekdir

```c
void seekdir(DIR *dir,off_t off); 
```

#### 2.4.8 closedir

```c
int closedir(DIR *dir); 
```



### 2.5 管道文件

Linux提供了许多的过滤和重定向程序,比如 more、cat 等等。还提供了< >; | <<等等重定向操作符

#### 2.5.1 pipe 创建管道

```c
// fildes[0]是用来读的文件描述符,而fildes[1]是用来写的文件描述符
int pipe(int fildes[2]);
```

#### 2.5.2 dup2 重定向

```c
// 用oldfd文件描述符来代替newfd文件描述符,同时关闭newfd文件描述符
// 即所有向 newfd 操作都转到 oldfd 上面
int dup2(int oldfd,int newfd);
```





## 3. 系统时间操作

### 3.1 time和ctime输出系统时间

​	输出系统当前的时间,比如我们使用 date 命令

```c
// 返回从1970年1月1日0点以来的秒数。
time_t time(time_t *tloc); 
// 将秒数转化为长度固定为26的字符串  可能值为Thu Dec 7 14：58：59 2000 
char *ctime(const time_t *clock);
```

### 3.2 gettimeofday 获取时间

​	将时间保存在结构 tv 之中，tz 一般我们使用 NULL 来代替。

```c
int gettimeofday(struct timeval *tv,struct timezone *tz); 
```

```c
strut timeval { 
    long tv_sec; /* 秒数 */ 
    long tv_usec; /* 微秒数 */ 
};
```

[案例]：计算程序执行的时间

```c
struct timeval start;
struct timeval end;
float timeuse;

gettimeofday(&start,NULL);
// 执行自定义程序
gettimeofday(&end,NULL);
timeuse=1000000*(end.tv_sec-start.tv_sec)+end.tv_usec-start.tv_usec;
timeuse/=1000000; 
printf("Used Time：%f\n",timeuse);
```

### 3.3 计时器使用

linux为每一个进程提供了 3 个内部间隔计时器

（1）ITIMER_REAL：减少实际时间，一到时就发出 SIGALRM 信号. 

（2）ITIMER_VIRTUAL：减少有效时间(进程执行的时间)，产生 SIGVTALRM 信号. 

（3）ITIMER_PROF：减少进程的有效时间和系统时间(为进程调度用的时间)。这个经常和上面一个使用，用来计算系统内核时间和用户时间.产生 SIGPROF 信号. 

```c
struct itimerval { 
	struct timeval it_interval;  
	struct timeval it_value;  // it_value减少的时间，当值为0时，就发出相应的信号，然后设置为 it_interval 值
}
```

```c
//  which：表示上面使用三个计时器中的哪一个

// getitimer：获取间隔计时器的时间值
int getitimer(int which, struct itimerval *value); 

// setitimer：设置间隔计时器的时间值为newval，并将旧值保存在oldval中
int setitimer(int which, struct itimerval *newval, struct itimerval *oldval);
```

