# akfs

#### 介绍
>   akfs是百晓基础架构层，提供文件操作接口、hook接口以及获取基本信息的接口。该文件系统屏蔽了用户层和内核交互接口，使得抓捕数据的模块无需关心具体的实现，专注于各自的业务逻辑。
>   akfs秉承一切皆文件的理念，由内核抓捕数据输出到用户层以文件的形式存在。内置的接口调用验证机制，使得用户层非法程序不得轻易破坏内核模块数据。
>   在操作系统的运行的过程中，资源的使用是以进程为单元划分的，进程是资源的消耗主体，因此对进程的生命周期的监控可以有效的了解操作系统的资源分配。
>   因此百晓在akfs的基础上提供akfile、akps、aknet三个抓捕模块实现抓捕进程生命周期内容的数据。

#### 编译环境
|   操作系统 |  内核版本 |  gcc版本 |
|   --       |  --       |  ----    |
|   CentOS-7.0-1406   |   3.10.0-1160.15.2.el7.x86_64 |   4.8.5   |

>   yum update && yum install kernel kernel-devel kernel-headers

#### 编译命令
```
./configure && make
`__USE_FTRACE__`:表示hook采用ftrace机制
```


#### 加载命令
```
insmod akfs.ko
mount -t akfs none mount_point
```

#### 卸载命令
```
umount mount_point
rmmod akfs.ko
```

#### 时间戳相关设置
```
timedatectl set-local-rtc 1 
将硬件时钟调整为与本地时钟一致
0 为设置为 UTC 时间
timedatectl set-time "YYYY-MM-DD HH:MM:SS" 设置时间
timedatectl set-timezone Asia/Shanghai  设置时区
```
