# 百晓降噪规则设计

### 哪些内容不需要输出？

已知良性命令或者操作不输出

白名单程序执行不输出

业务外联访问或者接受连接监控不输出

文件监控只监控固定目录以及其子目录，其它不关心。



### 如何控制？



百晓的日志都是序列化为json格式，规范为携带tag，通过tag查找到对应的字段判断其内容是否符合过滤条件。

不同系统输出的内容不一致，规则需要携带系统类型属性。

填充系统类型后，填充事件来源属性进一步细化规程，防止误处理。



### 规则的key从哪来？



百晓json定义默认全部携带json tag，使用tag作为规则key即可。



### 内容判断方式支持哪些?



目前整理了 string 字符串全匹配,regex 正则、path-ext 文件后缀 三种，后续继续新增。



### 规则设计



```json
[
    {
        "meta": {
            "author": "phil",
            "description": "docker 进程访问日志过滤",
            "name": "docker 进程写日志"
        },
        "and": true, // rules的逻辑，true为全部符合，false则只需要符合一个
        "enabled": true, // 规则启用开关

        "rules": {
            "exe_file":{ 
              "data": "/usr/local/bin/dockerd",
              "type": "string"
            },
            "chg_file":{ 
              "data": ".log", // 判断值
              "type": "path-ext" // 判断方式 (string 全匹配、regex 正则、path-ext 文件后缀)
            }
        },
        "source": "process", // 数据来源类型 (file,process,net)
        "system": "linux" // 操作系统类型 (windows,linux)
	}
]
```