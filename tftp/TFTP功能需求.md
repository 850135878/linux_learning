git clone http://huangjiawei:huangjiawei@192.168.26.9/git/training.git

使用 copy 命令可以从TFTP或FTP 服务器读取文件到路由器 flash内，也可以将路由器 flash中的指定文件上传至TFTP或FTP服务器。

**copy tftp filename** *REMOTE_NAME* **host** *A.B.C.D* **flash [ filename** *LOCAL_NAME* **]**



向TFTP服务器上传设备flash中的指定文件。



向FTP服务器上传设备flash中的指定文件。



## Big300

### 1. 上传下载文件命令

#### **下载文件：**

从TFTP服务器下载文件到设备flash

```
cpoy tftp filename <远程文件名> host 远程ip<A.B.C.D> flash [filename <文件名>]
```

#### **上传文件：**

向TFTP服务器上传设备flash中的指定文件。

```
copy flash filename <本地文件名> tftp [ filename 远程文件名 ] host 远程ip<A.B.C.D>
```

/training_proj/training/packages/apps

## TFTP

### 功能需求

1) 实现基本的上传/下载操作；

2) 服务端可以并发接收客户端的请求；

3) 服务端实现如下命令：

   ```cmd
   tftp server [enable|disable]
   tftp server port <PORT>
   tftp server retransmit <TIMEOUT> <COUNT>
   ```

4) show running正常显示配置，并设备重启配置能正确施加；

5) 协议一致性，服务端能与系统自带tftp互通；

6) 设计测试用例：

- 自制服务端与系统tftp客户端互通无问题；
- 自制服务器并发性测试，>=20路；
- 上传/下载文件一致性检查；
- 自制软件与系统软件对比测试；
- 插拔网线，行为符合预期；
- 端口no ip address后，再次配置ip address，行为符合预期。
- 从服务端下载文件，如果文件名不存在，能正确返回错误值；
- 向服务器上传文件，如果文件不存在，则新建文件；如果文件存在，则进行覆盖；
- 多客户端向服务器同时上传同名文件，如何正确处理？
- 超长文件名，服务端无异常；





```
put rfc1350_1.pdf 127.0.0.1:rfc1350_1_1.pdf
```



申请设备环境编译参考下面这个网页 lsdk环境编译

[http://192.168.26.9:8080/projects/training/wiki/Lsdk%E7%8E%AF%E5%A2%83%E7%BC%96%E8%AF%91](http://192.168.26.9:8080/projects/training/wiki/Lsdk环境编译)

登录新设备设备linux系统方式

admin/admin用户登录 然后diagnosis start 再bigxxx登录密码如下计算 http://192.168.26.99/bdpasswd/?mac=00:e1:f3:f1:3d:c8
升级版本 sysupgrade xxxx

tranining仓库生成的版本，和之前一样只是bigxxx账户是BigxXx123^IU%&或者zcWggo7T，都试试

[http://192.168.26.100](http://192.168.26.100/)



启动Docker

```
sudo systemctl start docker
sudo systemctl status docker   # 启动 Docker 守护进程
sudo systemctl enable docker   # 使 Docker 在启动时自动启动
sudo gpasswd -a hjw docker  # 将普通用户加入docker组
newgrp docker  # 更新docker组
```



**show running 问题：**

trainning的环境 show running方式是以前基于脚本实现的路径如下
/usr/share/config-afterif/    接口配置之前
/usr/share/config-beforeif/ 接口配置之后
/usr/share/config-interface/ 接口上配置



diagnosis start

http://192.168.26.99/bdpasswd/?mac=00:55:b1:1b:96:fc

fdGnGCQB5eSq / BigxXx123^IU%&



```
deb http://192.168.26.9/repos/ubuntu/ubuntu-ports/ bionic main restricted universe multiverse
deb http://192.168.26.9/repos/ubuntu/ubuntu-ports/ bionic-updates main restricted universe multiverse
deb http://192.168.85.220/repos/ubuntu/ubuntu-ports/ bionic-security main restricted universe multiverse
deb-src http://192.168.85.220/repos/ubuntu/ubuntu-ports/ bionic main restricted universe multiverse
deb-src http://192.168.85.220/repos/ubuntu/ubuntu-ports/ bionic-updates main restricted universe multiverse
deb-src http://192.168.85.220/repos/ubuntu/ubuntu-ports/ bionic-security main restricted universe multiverse
```





```
环境clone下来，可能需要切换分析（按照培训要求）
cd training
./m.sh switch big300
./m.sh all
把编译build/images目录下的版本刷到设备上

在后面只是模块开发，只需要替换模块可执行文件和xml命令行文件就行，不需要重新编译版本和刷版本（太费时间）。
```



- /etc/default/tftpd-hpa文件

```
# /etc/default/tftpd-hpa
TFTP_USERNAME="root"
TFTP_DIRECTORY="/etc/sysconfig"
TFTP_ADDRESS=":69"
TFTP_OPTIONS=" -l -c -s -4"
```

- /run/tftpd/tftpdenable

```
1
```



```bash
 # 检查 ${69} 是否有非空值
if test -n "${69}"; then 
	 # 从 /etc/default/tftpd-hpa 文件中提取 TFTP_ADDRESS 配置行中的端口号。
     myport=$(cat /etc/default/tftpd-hpa | grep "TFTP_ADDRESS" | awk -F"\"" '{print $2}' | awk -F":" '{print $2}')
     if [[ "$myport" -eq 69 || ! -n "$myport" ]]; then
         exit 0
     fi
		 # 检查端口 69 是否被占用，wc计算行数
         has_port=`netstat -alnp | grep ":69 " | wc -l`
     if [ "$has_port" != "0" ]; then
         echo "Port already in use."
             exit 1
         else
         	# 删除 /etc/default/tftpd-hpa 文件中包含 TFTP_ADDRESS 的行
            sed -i '/TFTP_ADDRESS/d' /etc/default/tftpd-hpa
            echo "TFTP_ADDRESS=\":69\"" >> /etc/default/tftpd-hpa
            # 检查 /run/tftpd/tftpdenable 文件是否存在，并读取其内容；为1则表明服务已启动，直接重启
            tftpdenable=$(cat /run/tftpd/tftpdenable 2>/dev/null)
            #if [ -n "$tftpenable" ]; then
            if [ "$tftpdenable" = "1" ]; then
                # 重启命令
             fi
         fi
    fi
    # 检查变量是否非空
    if test -n "${port_num}"; then
       myport=$(cat /etc/default/tftpd-hpa|grep "TFTP_ADDRESS"|awk -F"\"" '{print $2}'|awk -F":" '{print $2}')
    if [[ "$myport" -eq "${port_num}" ]]; then
       exit 0
    fi
        has_port=`netstat -alnp | grep ":${port_num} " | wc -l`
        if [ "$has_port" != "0" ]; then
            echo "Port already in use."
            exit 1
         else
            sed -i '/TFTP_ADDRESS/d' /etc/default/tftpd-hpa
            echo "TFTP_ADDRESS=\":${port_num}\"" >> /etc/default/tftpd-hpa
            #tftpenable=$(netstat -alnp|grep tftp)
            tftpdenable=$(cat /run/tftpd/tftpdenable 2>/dev/null)
            #if [ -n "$tftpenable" ]; then
            if [ "$tftpdenable" = "1" ]; then
              # systemctl restart tftpd-hpa
            fi
          fi
          if test -n "${enable}"; then
            # systemctl start tftpd-hpa
                  echo 1 > /run/tftpd/tftpdenable
          fi
fi
```



- /usr/bin/tftpserver_print.sh ${json}

```bash
# >> show tftp server
#!/bin/sh
tftpdenable=$(cat /run/tftpd/tftpdenable 2>/dev/null)
if [ "$tftpdenable" = "1" ]; then
    enablestr="enable"
else
    enablestr="disable"
fi
	myport=$(cat /etc/default/tftpd-hpa | grep "TFTP_ADDRESS"|awk -F"\"" '{print $2}'|awk -F":" '{print $2}')
if [ "$1" = "json" ]; then
    echo "{"\
    "\"status\": \"$enablestr\","\
    "\"port\": \"$myport\""\
    "}"
else
    echo "state:    ${enablestr}"
    echo "port:     ${myport}"
fi
```





/etc/init.d/mytftpd

`/lib/systemd/system/`mytftpd.service



### 执行时跳转顺序

1. 执行命令**systemctl start mytftpd**   

2. `systemctl` 命令会告诉`systemd` 启动名为 **tftpd-hpa.service服务**

3. 然后，`systemd`会去配置文件目录中查找，

   1. `/etc/systemd/system/`(优先级高)
   2. `/lib/systemd/system/`
   3. `/run/systemd/system/`

   最后，在 **/lib/systemd/system**目录下找到 tftpd-hpa.service文件，`/lib/systemd/system/tftpd-hpa.service `  

4. 且服务单元文件中定义了启动命令`ExecStart=/etc/init.d/tftpd-hpa start`，然后systemd会去执行`/etc/init.d/tftpd-hpa`的init脚本来启动命令

   - **Start命令**

   ```bash
   do_start()
   {
   	# Ensure --secure and multiple server directories are not used at the same time
   	if [ "$(echo $TFTP_DIRECTORY | wc -w)" -ge 2 ] && \
   	   echo $TFTP_OPTIONS | grep -qs secure
   	then
   		echo
   		echo "When --secure is specified, exactly one directory can be specified."
   		echo "Please correct your $DEFAULTS."
   		exit 1
   	fi
   
   	# Ensure server directories exist
   	for d in $TFTP_DIRECTORY
   	do
   		if [ ! -d "$d" ]
   		then
   			echo "$d missing, aborting."
   			exit 1
   		fi
   	done
   
   	start-stop-daemon --start --quiet --oknodo --exec $DAEMON -- \
   		--listen --user $TFTP_USERNAME --address $TFTP_ADDRESS \
   		$TFTP_OPTIONS $TFTP_DIRECTORY
   }
   
   case "$1" in
   	start)
   		init_is_upstart > /dev/null 2>&1 && exit 1   # 用于检测系统是否使用 Upstart 作为初始化系统。
   
   		log_daemon_msg "Starting $DESC" "$NAME"      # My tftpd  mytftpd
   		do_start
   		log_end_msg $?                               #
   		;;
   esac
   ```

   - **Stop命令**

   ```
   do_stop ()
   {
   	start-stop-daemon --stop --quiet --oknodo --name $NAME   # mytftpd
   }
   
   case "$1" in
   	stop)
   		init_is_upstart > /dev/null 2>&1 && exit 0
   
   		log_daemon_msg "Stopping $DESC" "$NAME"
   		do_stop
   		log_end_msg $?
   		;;
   esac
   ```

   - 

   ```bash
   
   # $1: 主要的消息文本，通常是服务的操作描述。
   # $2: 可选的第二个消息文本，通常是服务的名称或其他相关信息。
   log_daemon_msg () {
       if [ -z "${1:-}" ]; then   # 检查第一个参数 $1 是否为空。如果为空，返回状态码 1，表示错误或无效的输入。
           return 1               # ${1:-}：一种安全的参数扩展形式，防止未定义变量引起错误。
       fi
       log_daemon_msg_pre "$@"    # 预处理函数，通常用于在日志输出前执行一些准备工作。
   
       if [ -z "${2:-}" ]; then
           echo -n "$1:" || true  # 如果没有第二个参数，则输出第一个参数 $1 后面跟一个冒号。-n 选项表示不输出换行符。
           return                 # || true: 确保即使 echo 命令失败，也不会导致脚本中断
       fi
       
       echo -n "$1: $2" || true
       log_daemon_msg_post "$@"   # 调用后处理函数，通常用于在日志消息输出后执行一些额外的操作，如清理或附加日志信息。
   }
   
   # 用于记录守护进程状态的函数
   log_end_msg () {
       # If no arguments were passed, return
       if [ -z "${1:-}" ]; then
           return 1
       fi
   
       local retval
       retval=$1     # 保存退出状态码
   
       log_end_msg_pre "$@"
   
   	# 检查是否使用 fancy（花哨）输出的函数。其具体实现可能会检查终端是否支持颜色等功能。
       # Only do the fancy stuff if we have an appropriate terminal and if /usr is already mounted
       if log_use_fancy_output; then  
           RED=$( $TPUT setaf 1)
           YELLOW=$( $TPUT setaf 3)
           NORMAL=$( $TPUT op)
       else
           RED=''
           YELLOW=''
           NORMAL=''
       fi
   
       if [ $1 -eq 0 ]; then
           echo "." || true
       elif [ $1 -eq 255 ]; then
           /bin/echo -e " ${YELLOW}(warning).${NORMAL}" || true
       else
           /bin/echo -e " ${RED}failed!${NORMAL}" || true
       fi
       log_end_msg_post "$@"
       return $retval
   }
   
   # 用于判断当前终端是否支持“花哨的”输出（即彩色和其他格式化的输出）。这个函数会检查各种条件，以确定是否可以使用颜色和其他格式化功能。
   log_use_fancy_output () {
       TPUT=/usr/bin/tput  # 用于控制终端的工具，能够设置颜色和其他终端属性。
       EXPR=/usr/bin/expr  # 定义 expr 命令的路径。expr 用于执行算术运算和字符串处理。
       if  [ -t 1 ] &&
           [ "x${TERM:-}" != "x" ] &&
           [ "x${TERM:-}" != "xdumb" ] &&
           [ -x $TPUT ] && [ -x $EXPR ] &&
           $TPUT hpa 60 >/dev/null 2>&1 &&
           $TPUT setaf 1 >/dev/null 2>&1
       then
           [ -z $FANCYTTY ] && FANCYTTY=1 || true
       else
           FANCYTTY=0
       fi
       case "$FANCYTTY" in
           1|Y|yes|true)   true;;
           *)              false;;
       esac
   }
   
   
   # 初始化脚本中的占位符函数，用于在输出日志消息时调用。
   log_daemon_msg_pre () { :; }
   log_daemon_msg_post () { :; }
   log_begin_msg_pre () { :; }
   log_begin_msg_post () { :; }
   log_end_msg_pre () { :; }
   log_end_msg_post () { :; }
   log_action_msg_pre () { :; }
   log_action_msg_post () { :; }
   log_action_begin_msg_pre () { :; }
   log_action_begin_msg_post () { :; }
   log_action_end_msg_pre () { :; }
   log_action_end_msg_post () { :; }
   ```

   



- **/lib/systemd/system/tftpd-hpa.service**    
  - Unit：控制单元，表示启动顺序和依赖关系
  - Service：服务，表示服务的定义；
  - Install：安装，表示如何安装配置文件。

```
1.service 文件配置的服务常用systemd管理。然而，systemd有系统和用户区分；
-- 系统（/usr/lib/systemd/system/）、
-- 用户（/etc/lib/systemd/user/）。
	
一般系统管理员手工创建的单元文件建议存放在/etc/systemd/system/目录下面。
- 文件放到该目录下后 需要执行 systemctl daemon-reload
service jssj start
```

```cmd
[Unit] 
Description=LSB: HPA's tftp server     #当前配置文件的描述信息
After=network.target                   #表示当前服务是在那个服务后面启动，一般定义为网络服务启动后启动  
Before=klish.service

[Service]
Type=forking                           #定义启动类型  systemd认为当该服务进程fork，且父进程退出后服务启动成功
ExecStart=/etc/init.d/tftpd-hpa start  #定义启动进程时执行的命令或者脚本
ExecStop=/etc/init.d/tftpd-hpa stop    #定义关闭进程时执行的命令或者脚本
Restart=always                         # 这个选项如果被允许，服务重启的时候进程会退出，会通过systemctl命令执行清除并重启的操作。
RestartSec=5

[Install]
WantedBy=multi-user.target             #表示多用户命令行状态
```

```sh
#当新增或修改service单元文件时，需要系统重新加载所有修改过的配置文件
systemctl daemon-reload

#设置为开机自启动
systemctl enable ehr.service

#关掉开机启用
systemctl disable ehr.service

#启动服务
systemctl start ehr.service

#重启服务，相当于先stop，再start
systemctl restart ehr.service

#关闭服务,关闭服务后，进程会全部被kill掉
systemctl stop ehr.service

#查看服务的状态
systemctl status ehr.service

#查看服务是否开机启动
systemctl is-enabled ehr.service

#查看已启动的服务列表
systemctl list-unit-files|grep enabled
#
显示所有已启动的服务
systemctl list-units --type=service

#查看启动失败的服务列表
systemctl --failed

# 杀死一个服务的所有子进程
systemctl kill ehr.service

# 重新加载一个服务的配置文件
systemctl reload ehr.service

#移除标记为丢失的 Unit 文件。在删除 Unit 文件后，由于缓存的关系，
#即使通过 daemon-reload 更新了缓存，在 list-units 中依然会显示标记为 not-found 的 Unit
systemctl reset-failed ehr.service
```



```
# /etc/default/mytftp
TFTP_USERNAME="root"
TFTP_DIRECTORY="/etc/sysconfig"
TFTP_ADDRESS=":69"
TFTP_OPTIONS=" -l -c -s -4"

# tftp server retransmit <timeout> <count>
TFTP_TIMEOUT=3
TFTP_RETRANSMIT_COUNT=6
```



- /etc/init.d/tftpd-hpa

```bash
#!/bin/sh
### BEGIN INIT INFO
# Provides:             tftpd-hpa
# Required-Start:       $local_fs $remote_fs $syslog $network
# Required-Stop:        $local_fs $remote_fs $syslog $network
# Default-Start:        2 3 4 5 （默认启动运行级别，通常是多用户模式）
# Default-Stop:         0 1 6（默认停止运行级别: 分别是关机、单用户模式和重启）
# Short-Description:    HPA's tftp server
# Description:          Trivial File Transfer Protocol (TFTP) is a file transfer
#                       protocol, mainly to serve boot images over the network
#                       to other machines (PXE).
### END INIT INFO

# 指定可执行文件的路径
PATH="/sbin:/bin:/usr/sbin:/usr/bin" 
# 指定 TFTP 守护进程的路径
DAEMON="/usr/sbin/in.tftpd"

# 检查 TFTP 守护进程是否存在并且可执行。如果不可执行，则退出脚本。
test -x "$DAEMON" || exit 0

NAME="in.tftpd"                   # 服务的名称
DESC="HPA's tftpd"                # 服务的描述
PIDFILE="/var/run/tftpd-hpa.pid"  # 存放进程 ID 的文件
DEFAULTS="/etc/default/tftpd-hpa" # 存放默认配置的文件路径

# 如果脚本中的某个命令返回非零退出状态，脚本会停止执行并退出
set -e

# 如果默认配置文件存在且可读，则读取其中的配置。“.”的作用是执行指定文件中的命令，将其内容读入当前 shell 环境。
[ -r "$DEFAULTS" ] && . "$DEFAULTS"

# 加载 init-functions，这是一组提供 log_daemon_msg、log_end_msg 和其他函数的库，用于标准化服务日志记录和状态检查。
. /lib/lsb/init-functions
. /etc/default/tftpd-hpa
# 启动 TFTP 服务
do_start()
{
        # Ensure --secure and multiple server directories are not used at the same time
        # 如果指定了 --secure 选项但目录数大于 1，输出错误并退出

        if [ "$(echo $TFTP_DIRECTORY | wc -w)" -ge 2 ] && \
           echo $TFTP_OPTIONS | grep -qs secure
        then
                echo
                echo "When --secure is specified, exactly one directory can be specified."
                echo "Please correct your $DEFAULTS."
                exit 1
        fi

        # 确保 TFTP 目录存在
        for d in $TFTP_DIRECTORY
        do
                if [ ! -d "$d" ]
                then
                        echo "$d missing, aborting."
                        exit 1
                fi
        done
		
		#  使用 start-stop-daemon 启动守护进程，传递相关的参数（如用户名、地址、选项和目录）。
		#  --start：指定要启动一个新进程。如果省略此选项，start-stop-daemon 默认执行停止操作。
		#  --quiet: 使 start-stop-daemon 在执行过程中不输出额外的日志或信息（即安静模式）。
		#  --oknodo: 表示如果进程已经在运行，则不会尝试启动一个新的实例。该选项用于避免重复启动相同的守护进程。
		#  --exec <>: 指定要启动的程序或守护进程的可执行文件路径。
		#  --：表示命令行选项的结束。-- 之后的所有参数都将传递给守护进程，而不是 start-stop-daemon。
        start-stop-daemon --start --quiet --oknodo --exec $DAEMON -- \
                --listen --user $TFTP_USERNAME --address $TFTP_ADDRESS \
                $TFTP_OPTIONS $TFTP_DIRECTORY
}

# 停止 TFTP 服务
do_stop ()
{		
		# --name <>：指定要停止的进程的名称（而不是进程 ID 或路径）。
		#           start-stop-daemon 会根据这个名称查找并停止相关的进程。
        start-stop-daemon --stop --quiet --oknodo --name $NAME
}

# 重载 TFTP 服务配置
do_reload ()
{
		# --signal: 指定发送给进程的信号。1 是信号编号，表示 SIGHUP 信号。信号编号通常用于向进程发送控制信号。
		#           SIGHUP（Signal Hang Up）常用于通知进程重新加载配置文件或执行其他类似的操作。
        start-stop-daemon --stop --quiet --oknodo --name $NAME --signal 1
}

case "$1" in
        start)
                init_is_upstart > /dev/null 2>&1 && exit 1

                log_daemon_msg "Starting $DESC" "$NAME"
                do_start
                log_end_msg $?
                ;;

        stop)
                init_is_upstart > /dev/null 2>&1 && exit 0

                log_daemon_msg "Stopping $DESC" "$NAME"
                do_stop
                log_end_msg $?
                ;;

        restart|force-reload)
                init_is_upstart > /dev/null 2>&1 && exit 1

                log_daemon_msg "Restarting $DESC" "$NAME"
                do_stop
                sleep 1
                do_start
                log_end_msg $?
                ;;

        status)
                status_of_proc $DAEMON $NAME
                ;;

        *)
                echo "Usage: $0 {start|stop|restart|force-reload|status}" >&2
                exit 3
                ;;
esac
```

- /lib/lsb/init-functions

```bash
# 调用 start_daemon 函数并传递参数
start_daemon -f -n 10 -p /var/run/mydaemon.pid -- /path/to/mydaemon --arg1 value1 --arg2 value2
start_daemon () {
    # force：变量用来标识是否强制启动守护进程。
    # nice：用于设置进程的优先级（nice 值）
    # pidfile：存储进程ID的文件路径，默认为 /dev/null
    # exec：守护进程的可执行文件路径
    # args: 用于存储传递给 start-stop-daemon 的参数
    # OPTIND: 用于 getopts 的位置索引
    local force nice pidfile exec args OPTIND
    force=""
    nice=0
    pidfile=/dev/null

	# getopts 用于解析选项
    OPTIND=1
    # fn:p: 这是 optstring，定义了可接受的选项及其参数要求。
    #       f 和 n 是单个字符选项。f 不需要参数，n 需要一个参数。 p: 表示选项 p 需要一个参数。
    # opt: 是接收选项字符的变量
    while getopts fn:p: opt ; do 
        case "$opt" in
            f)  force="force";;     # -f: 如果指定了 -f 选项，将 force 变量设置为 "force"
            n)  nice="$OPTARG";;    # -n: 用于指定 nice 值，$OPTARG 是选项参数（即优先级）
            p)  pidfile="$OPTARG";; # -p: 用于指定 PID 文件路径，$OPTARG 是选项参数（即文件路径）。
        esac
    done
    
    # shift $(($OPTIND - 1)): 移动参数位置，以便 getopts 之后的参数能够正确处理。
    shift $(($OPTIND - 1))
    
    # 如果参数中有 --，则忽略它。
    if [ "$1" = '--' ]; then
        shift
    fi
 	
 	# exec="$1"; shift: 将第一个非选项参数作为守护进程的可执行文件路径，并移除该参数
    exec="$1"; shift
	
	# 构建 start-stop-daemon 命令
    args="--start --nicelevel $nice --quiet --oknodo"
    if [ "$force" ]; then
        /sbin/start-stop-daemon $args \
	    --chdir "$PWD" --startas $exec --pidfile /dev/null -- "$@"
    elif [ $pidfile ]; then
        /sbin/start-stop-daemon $args \
	    --chdir "$PWD" --exec $exec --oknodo --pidfile "$pidfile" -- "$@"  
	    # $PWD 总是返回当前目录的绝对路径，不需要进行额外的解析
    else
        /sbin/start-stop-daemon $args --chdir "$PWD" --exec $exec -- "$@"
    fi
}

pidofproc () {
    local pidfile base status specified pid OPTIND
    pidfile=
    specified=
    
    OPTIND=1
    while getopts p: opt ; do
        case "$opt" in
            p)  pidfile="$OPTARG"
                specified="specified"
		;;
        esac
    done
    shift $(($OPTIND - 1))
    if [ $# -ne 1 ]; then
        echo "$0: invalid arguments" >&2
        return 4
    fi

    base=${1##*/}
    if [ ! "$specified" ]; then
        pidfile="/var/run/$base.pid"
    fi

    if [ -n "${pidfile:-}" ]; then
     if [ -e "$pidfile" ]; then
      if [ -r "$pidfile" ]; then
        read pid < "$pidfile"
        if [ -n "${pid:-}" ]; then
            if $(kill -0 "${pid:-}" 2> /dev/null); then
                echo "$pid" || true
                return 0
            elif ps "${pid:-}" >/dev/null 2>&1; then
                echo "$pid" || true
                return 0 # program is running, but not owned by this user
            else
                return 1 # program is dead and /var/run pid file exists
            fi
        fi
      else
        return 4 # pid file not readable, hence status is unknown.
      fi
     else
       # pid file doesn't exist, try to find the pid nevertheless
       if [ -x /bin/pidof ] && [ ! "$specified" ]; then
         status="0"
         /bin/pidof -o %PPID -x $1 || status="$?"
         if [ "$status" = 1 ]; then
             return 3 # program is not running
         fi
         return 0
       fi
       return 3 # specified pid file doesn't exist, program probably stopped
     fi
    fi
    if [ "$specified" ]; then
        return 3 # almost certain it's not running
    fi
    return 4 # Unable to determine status
}

# start-stop-daemon uses the same algorithm as "pidofproc" above.
killproc () {
    local pidfile sig status base name_param is_term_sig OPTIND
    pidfile=
    name_param=
    is_term_sig=

    OPTIND=1
    while getopts p: opt ; do
        case "$opt" in
            p)  pidfile="$OPTARG";;
        esac
    done
    shift $(($OPTIND - 1))

    base=${1##*/}
    if [ ! $pidfile ]; then
        name_param="--name $base --pidfile /var/run/$base.pid"
    else
        name_param="--pidfile $pidfile"
    fi

    sig=$(echo ${2:-} | sed -e 's/^-\(.*\)/\1/')
    sig=$(echo $sig | sed -e 's/^SIG\(.*\)/\1/')
    if [ "$sig" = 15 ] || [ "$sig" = TERM ]; then
        is_term_sig="terminate_signal"
    fi
    status=0
    if [ ! "$is_term_sig" ]; then
        if [ -n "$sig" ]; then
            /sbin/start-stop-daemon --stop --signal "$sig" \
		--quiet $name_param || status="$?"
        else
            /sbin/start-stop-daemon --stop \
		--retry 5 \
		--quiet $name_param || status="$?"
        fi
    else
        /sbin/start-stop-daemon --stop --quiet \
	    --oknodo $name_param || status="$?"
    fi
    if [ "$status" = 1 ]; then
        if [ -z "$sig" ]; then
            return 0
        fi
        return 3 # program is not running
    fi

    if [ "$status" = 0 ] && [ "$is_term_sig" ] && [ "$pidfile" ]; then
        pidofproc -p "$pidfile" "$1" >/dev/null || rm -f "$pidfile"
    fi
    return 0
}

# Return LSB status
status_of_proc () {
    local pidfile daemon name status OPTIND

    pidfile=
    OPTIND=1
    while getopts p: opt ; do
        case "$opt" in
            p)  pidfile="$OPTARG";;
        esac
    done
    shift $(($OPTIND - 1))

    if [ -n "$pidfile" ]; then
        pidfile="-p $pidfile"
    fi
    daemon="$1"
    name="$2"

    status="0"
    pidofproc $pidfile $daemon >/dev/null || status="$?"
    if [ "$status" = 0 ]; then
        log_success_msg "$name is running"
        return 0
    elif [ "$status" = 4 ]; then
        log_failure_msg "could not access PID file for $name"
        return $status
    else
        log_failure_msg "$name is not running"
        return $status
    fi
}

log_use_fancy_output () {
    TPUT=/usr/bin/tput
    EXPR=/usr/bin/expr
    if  [ -t 1 ] &&
	[ "x${TERM:-}" != "x" ] &&
	[ "x${TERM:-}" != "xdumb" ] &&
	[ -x $TPUT ] && [ -x $EXPR ] &&
	$TPUT hpa 60 >/dev/null 2>&1 &&
	$TPUT setaf 1 >/dev/null 2>&1
    then
        [ -z $FANCYTTY ] && FANCYTTY=1 || true
    else
        FANCYTTY=0
    fi
    case "$FANCYTTY" in
        1|Y|yes|true)   true;;
        *)              false;;
    esac
}

log_success_msg () {
    if [ -n "${1:-}" ]; then
        log_begin_msg $@
    fi
    log_end_msg 0
}

log_failure_msg () {
    if [ -n "${1:-}" ]; then
        log_begin_msg $@ "..."
    fi
    log_end_msg 1 || true
}

log_warning_msg () {
    if [ -n "${1:-}" ]; then
        log_begin_msg $@ "..."
    fi
    log_end_msg 255 || true
}

#
# NON-LSB HELPER FUNCTIONS
#
# int get_lsb_header_val (char *scriptpathname, char *key)
get_lsb_header_val () {
        if [ ! -f "$1" ] || [ -z "${2:-}" ]; then
                return 1
        fi
        LSB_S="### BEGIN INIT INFO"
        LSB_E="### END INIT INFO"
        sed -n "/$LSB_S/,/$LSB_E/ s/# $2: \+\(.*\)/\1/p" "$1"
}

# If the currently running init daemon is upstart, return zero; if the
# calling init script belongs to a package which also provides a native
# upstart job, it should generally exit non-zero in this case.
init_is_upstart()
{
   if [ -x /sbin/initctl ] && /sbin/initctl version 2>/dev/null | /bin/grep -q upstart; then
       return 0
   fi
   return 1
}

# int log_begin_message (char *message)
log_begin_msg () {
    log_begin_msg_pre "$@"
    if [ -z "${1:-}" ]; then
        return 1
    fi
    echo -n "$@" || true
    log_begin_msg_post "$@"
}

# Sample usage:
# log_daemon_msg "Starting GNOME Login Manager" "gdm"
#
# On Debian, would output "Starting GNOME Login Manager: gdm"
# On Ubuntu, would output " * Starting GNOME Login Manager..."
#
# If the second argument is omitted, logging suitable for use with
# log_progress_msg() is used:
#
# log_daemon_msg "Starting remote filesystem services"
#
# On Debian, would output "Starting remote filesystem services:"
# On Ubuntu, would output " * Starting remote filesystem services..."

log_daemon_msg () {
    if [ -z "${1:-}" ]; then
        return 1
    fi
    log_daemon_msg_pre "$@"

    if [ -z "${2:-}" ]; then
        echo -n "$1:" || true
        return
    fi
    
    echo -n "$1: $2" || true
    log_daemon_msg_post "$@"
}

# #319739
#
# Per policy docs:
#
#     log_daemon_msg "Starting remote file system services"
#     log_progress_msg "nfsd"; start-stop-daemon --start --quiet nfsd
#     log_progress_msg "mountd"; start-stop-daemon --start --quiet mountd
#     log_progress_msg "ugidd"; start-stop-daemon --start --quiet ugidd
#     log_end_msg 0
#
# You could also do something fancy with log_end_msg here based on the
# return values of start-stop-daemon; this is left as an exercise for
# the reader...
#
# On Ubuntu, one would expect log_progress_msg to be a no-op.
log_progress_msg () {
    if [ -z "${1:-}" ]; then
        return 1
    fi
    echo -n " $@" || true
}


# int log_end_message (int exitstatus)
log_end_msg () {
    # If no arguments were passed, return
    if [ -z "${1:-}" ]; then
        return 1
    fi

    local retval
    retval=$1

    log_end_msg_pre "$@"

    # Only do the fancy stuff if we have an appropriate terminal
    # and if /usr is already mounted
    if log_use_fancy_output; then
        RED=$( $TPUT setaf 1)
        YELLOW=$( $TPUT setaf 3)
        NORMAL=$( $TPUT op)
    else
        RED=''
        YELLOW=''
        NORMAL=''
    fi

    if [ $1 -eq 0 ]; then
        echo "." || true
    elif [ $1 -eq 255 ]; then
        /bin/echo -e " ${YELLOW}(warning).${NORMAL}" || true
    else
        /bin/echo -e " ${RED}failed!${NORMAL}" || true
    fi
    log_end_msg_post "$@"
    return $retval
}

log_action_msg () {
    log_action_msg_pre "$@"
    echo "$@." || true
    log_action_msg_post "$@"
}

log_action_begin_msg () {
    log_action_begin_msg_pre "$@"
    echo -n "$@..." || true
    log_action_begin_msg_post "$@"
}

log_action_cont_msg () {
    echo -n "$@..." || true
}

log_action_end_msg () {
    local end
    log_action_end_msg_pre "$@"
    if [ -z "${2:-}" ]; then
        end="."
    else
        end=" ($2)."
    fi

    if [ $1 -eq 0 ]; then
        echo "done${end}" || true
    else
        if log_use_fancy_output; then
            RED=$( $TPUT setaf 1)
            NORMAL=$( $TPUT op)
            /bin/echo -e "${RED}failed${end}${NORMAL}" || true
        else
            echo "failed${end}" || true
        fi
    fi
    log_action_end_msg_post "$@"
}

# Pre&Post empty function declaration, to be overriden from /lib/lsb/init-functions.d/*
log_daemon_msg_pre () { :; }
log_daemon_msg_post () { :; }
log_begin_msg_pre () { :; }
log_begin_msg_post () { :; }
log_end_msg_pre () { :; }
log_end_msg_post () { :; }
log_action_msg_pre () { :; }
log_action_msg_post () { :; }
log_action_begin_msg_pre () { :; }
log_action_begin_msg_post () { :; }
log_action_end_msg_pre () { :; }
log_action_end_msg_post () { :; }

# Include hooks from other packages in /lib/lsb/init-functions.d
for hook in $(run-parts --lsbsysinit --list /lib/lsb/init-functions.d 2>/dev/null); do
    [ -r $hook ] && . $hook || true
done

FANCYTTY=
[ -e /etc/lsb-base-logging.sh ] && . /etc/lsb-base-logging.sh || true

```



scp -r ../mytftp huangjiawei@192.168.26.99:/home2/huangjiawei/training_proj/training/packages/apps

scp -r /home2/huangjiawei/training_proj/training/packages/apps/mytftp hjw@192.168.233.129:/home/hjw/vscode_workspace



scp packages/apps/mytftp/bin/server/server bigxxx@192.168.2.1:/usr/share/mytftp/bin





```
put rfc1350_1.pdf 192.168.233.1:rfc1350_1.pdf
put rfc1350.pdf 192.168.2.1:rfc1350_1.pdf

get 192.168.233.1:rfc1350_1.pdf

get 127.0.0.1:rfc1350_1.pdf
put rfc1350_1.pdf 127.0.0.1:rfc1350_1.pdf
```

![image-20240904104146610](./TFTP功能需求.assets/image-20240904104146610.png)

### /usr/share/config-afterif

```
#!/bin/sh
#tftpenable=$(netstat -alnp|grep tftp)
tftpdenable=$(cat /run/mytftp/tftpenable 2>/dev/null)
port=$(cat /etc/default/mytftpd | grep "TFTP_ADDRESS" |awk -F"\"" '{print $2}'|awk -F":" '{print $2}')

timeout=$(cat /etc/default/mytftpd | grep "TFTP_TIMEOUT" |awk -F"=" '{print $2}')
count=$(cat /etc/default/mytftpd | grep "TFTP_RETRANSMIT_COUNT" |awk -F"=" '{print $2}')

if [ "$tftpdenable" = "1" ]; then
    echo "tftp server enable"
        
    if [ -n "$port" ]; then
            echo "tftp server port $port "
        fi

        if [[ -n "$timeout" && -n "$count" ]]; then
            echo "tftp server retransmit timeout ${timeout} count ${count}"
        else
            if [[ -n "$timeout" ]]; then
                   echo "tftp server retransmit timeout ${timeout}"
            fi
        fi
else
    echo "tftp server disable"
fi 
```

- /etc/default/mytftp

```
# /etc/default/mytftp
TFTP_USERNAME="root"
TFTP_DIRECTORY="/etc/sysconfig/tftpdir"
TFTP_ADDRESS=":69"
# -l: 启用日志记录。
# -c: 允许客户端创建文件（写操作）。
# -s: 设置根目录。
# -4: 强制使用 IPv4
TFTP_OPTIONS=" -l -c -s -4"

# tftp server retransmit <timeout> <count>
TFTP_TIMEOUT=3
TFTP_RETRANSMIT_COUNT=6
```

- /usr/bin/mytftpserver_print.sh ${json}

```
#!/bin/sh
#tftpenable=$(netstat -alnp|grep tftp)
tftpenable=$(cat /run/mytftp/tftpenable 2>/dev/null)
#if [ -n "$tftpenable" ]; then
if [ "$tftpenable" = "1" ]; then
    enablestr="enable"
else
    enablestr="disable"
fi
myport=$(cat /etc/default/mytftp|grep "TFTP_ADDRESS"|awk -F"\"" '{print $2}'|awk -F":" '{print $2}')
timeout=$(cat /etc/default/mytftp |grep "TFTP_TIMEOUT" |awk -F"=" '{print $2}')
count=$(cat /etc/default/mytftp |grep "TFTP_RETRANSMIT_COUNT" |awk -F"=" '{print $2}')

if [ "$1" = "json" ]; then
    echo "{"\
    "\"status\": \"$enablestr\","\
    "\"port\": \"$myport\","\
        "\"timeout\": \"$timeout\","\
        "\"count\": \"$count\""\
    "}"
else
    echo "state:    ${enablestr}"
    echo "port:     ${myport}"
    echo "timeout:  ${timeout}"
    echo "count:    ${count}"
fi
```



### /user/share/klish/mytftpserver.xml

```shell
<?xml version="1.0" encoding="UTF-8"?>
<CLISH_MODULE xmlns="http://clish.sourceforge.net/XMLSchema" 
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
 xsi:schemaLocation="http://clish.sourceforge.net/XMLSchema
                     http://clish.sourceforge.net/XMLSchema/clish.xsd">
        <!--=======================================================-->

<PTYPE name="TFTP_NUMBER"
	method="integer"
	pattern="0..65535"
	help="tftp number"/>	

<VIEW name="config-view">
<COMMAND name="tftp" help="mytftp server">
	<PARAM name="server" help="tftp server" ptype="SUBCOMMAND" mode="subcommand" >
		<PARAM name="tftp_operate" help="Tftp operate" ptype="STRING" mode="switch">
                	<PARAM name="port" help="listen port" ptype="SUBCOMMAND" mode="subcommand" >
                        	<PARAM name="port_switch" help="Set tftp port" ptype="STRING" mode="switch">
                                	<PARAM name="69" help="Default port number" ptype="SUBCOMMAND" mode="subcommand"/>
                                	<PARAM name="port_num" help="tftp port" ptype="TFTP_PORT"/>
                        	</PARAM>
                	</PARAM>
                <PARAM name="enable" help="set tftp enable" ptype="SUBCOMMAND" mode="subcommand" />
                <PARAM name="disable" help="set tftp disable" ptype="SUBCOMMAND" mode="subcommand" />
                <PARAM name="retransmit" help="set tftp retransmit" ptype="SUBCOMMAND" mode="subcommand">
                     <PARAM name="timeout" help="set tftp timeout" optional="true" ptype="TFTP_NUMBER"/>
                     <PARAM name="count" help="set maximum retry count" optional="true" ptype="TFTP_NUMBER"/>
                </PARAM>
        </PARAM>
	</PARAM>
        
        <ACTION>
                if test -n "${69}"; then
                        myport=$(cat /etc/default/mytftpd | grep "TFTP_ADDRESS"|awk -F"\"" '{print $2}'|awk -F":" '{print $2}')
                        if [[ "$myport" -eq 69 || ! -n "$myport" ]]; then
                                exit 0
                        fi

                        has_port=`netstat -alnp | grep ":69 " | wc -l`
                        if [ "$has_port" != "0" ]; then
                                echo "Port already in use."
                                exit 1
                        else
                                sed -i '/TFTP_ADDRESS/d' /etc/default/mytftpd
                                echo "TFTP_ADDRESS=\":69\"" >> /etc/default/mytftpd
                                tftpenable=$(cat /run/mytftp/tftpenable 2>/dev/null)
         
                                if [ "$tftpenable" = "1" ]; then
                                        systemctl restart mytftpd	
                                fi
                        fi
                fi
                if test -n "${port_num}"; then
                        myport=$(cat /etc/default/mytftpd | grep "TFTP_ADDRESS"|awk -F"\"" '{print $2}'|awk -F":" '{print $2}')
                        if [[ "$myport" -eq "${port_num}" ]]; then
                                exit 0
                        fi
                        has_port=`netstat -alnp | grep ":${port_num} " | wc -l`
                        if [ "$has_port" != "0" ]; then
                                echo "Port already in use."
                                exit 1
                        else
                                sed -i '/TFTP_ADDRESS/d' /etc/default/mytftpd
                                echo "TFTP_ADDRESS=\":${port_num}\"" >> /etc/default/mytftpd
                                #tftpenable=$(netstat -alnp|grep tftp)
                                tftpenable=$(cat /run/mytftp/tftpenable 2>/dev/null)
                                #if [ -n "$tftpenable" ]; then
                                if [ "$tftpenable" = "1" ]; then
                                       systemctl restart mytftpd
                                fi
                        fi
                fi

                if test -n "${enable}"; then
                      systemctl start mytftpd
 		      echo 1 > /run/mytftp/tftpenable
                      
                fi

                if test -n "${disable}"; then
		      systemctl stop mytftpd
                      echo 0 > /run/mytftp/tftpenable
                      
                fi

                if test -n "${timeout}"; then
                        ori_timeout=$(cat /etc/default/mytftpd | grep "TFTP_TIMEOUT" |awk -F"=" '{print $2}')
                        if [[ "$ori_timeout" -eq "${timeout}"  ]] ; then
				if [ [! -n "${count}"]]; then
					exit 0
				fi	
                        else
                                sed -i '/TFTP_TIMEOUT/d' /etc/default/mytftpd
                                echo "TFTP_TIMEOUT=${timeout}" >> /etc/default/mytftpd
                                tftpenable=$(cat /run/mytftp/tftpenable 2>/dev/null)
                                if [ "$tftpenable" = "1" ]; then
                                         systemctl restart mytftpd	
                                fi
                        fi

			if test -n "${count}"; then
                        	ori_count=$(cat /etc/default/mytftpd |grep "TFTP_RETRANSMIT_COUNT" |awk -F"=" '{print $2}')
                        	if [[ "$ori_count" -eq "${count}" ]]; then
                                	exit 0
                       		else
                                	sed -i '/TFTP_RETRANSMIT_COUNT/d' /etc/default/mytftpd
                                	echo "TFTP_RETRANSMIT_COUNT=${count}" >> /etc/default/mytftpd
                                	tftpenable=$(cat /run/mytftp/tftpenable 2>/dev/null)
                                        if [ "$tftpenable" = "1" ]; then
                                       		systemctl restart mytftpd
                                	fi
                        	fi
                	fi
              fi
        </ACTION>
</COMMAND>
</VIEW>
<VIEW name="enable-view">
<COMMAND name="show tftp" help="show tftp server">
       <PARAM name="server" help="show tftp server" ptype="SUBCOMMAND" mode="subcommand" >
	      <PARAM name="json" help="JavaScript Object Notation" ptype="SUBCOMMAND" mode="subcommand" optional="true" hide="true"/>
    	      
       </PARAM>
	<ACTION>
            	/usr/bin/mytftpserver_print.sh ${json}
    	</ACTION>
</COMMAND>

</VIEW>

</CLISH_MODULE>
```



reboot后输入admin进入小bin

![image-20240920200217388](./TFTP功能需求.assets/image-20240920200217388.png)

执行run bootmmc_itb启动，输入root

```
ifconfig eth0 up
ifconfig eth0 192.168.2.1 
cd /
# 将bin拖入此目录
flex-installer -i pf -d /dev/mmcblk0
flex-installer -b bootpartition_ARMADA37XX_arm64_lts_4.14.tgz -d /dev/mmcblk0
flex-installer -r rootfs_lsdk1909_ARMADA37XX_arm64_main.tgz -d /dev/mmcblk0
sysupgrade BD_...bin

```

