# aknet

#### 简介
>   aknet模块旨在抓取进程使用网络资源的数据。

#### 编译环境
|   操作系统 |  内核版本 |  gcc版本 |
|   --       |  --       |  ----    |
|   CentOS-7.0-1406   |   3.10.0-1160.15.2.el7.x86_64 |   4.8.5   |

```
yum update && yum install kernel kernel-devel kernel-headers
```
#### 编译命令
```
./configure && make
net.lds为aknet模块的链接脚本
```

#### 依赖说明
```
aknet加载依赖akfs模块，需要先加载akfs模块
```
#### 加载命令
```
insmod net.ko
```

#### 卸载命令
```
rmmod net.ko
```
