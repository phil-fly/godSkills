#!/bin/bash

version="v1.0.1"
akagentDockerPkg="akagent-docker-${version}.tar"


install_akfs(){
    insmod akfs.ko
    test -D /opt/mount||mkdir -p /opt/mount
    mount -t akfs none /opt/mount
}

install_akps(){
    insmod process.ko
}

install_akfile(){
    insmod file.ko
}

install_aknet(){
    insmod net.ko
}

install_akagent_docker(){
    docker load -i ${akagentDockerPkg}
}

install_akagent_service(){
    docker load -i ${akagentDockerPkg}
}


function person_menu(){  
cat << EOF
==================================================================
                        百晓安装服务
                                                by phil-fly
==================================================================
`echo -e "\033[35m 1. 安装docker模式\033[0m"`
`echo -e "\033[35m 2. 安装service模式\033[0m"`

                                                        
userspace program 安装方式选择: 1/2
EOF
read -p "输入选择：" num2
case $num2 in
 1)
  echo -e "\033[35m 开始docker模式安装.\033[0m"
  install_akagent_docker
  ;;
 2)
  echo -e "\033[35m 开始本地service模式安装.\033[0m"
  echo -e "\033[35m 暂不支持.\033[0m"
  ;;
 *)
  echo "the is Fail!!"
  person_menu
esac
}


person_menu
install_akfs
install_akps
install_akfile
install_aknet




