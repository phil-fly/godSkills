#!/bin/bash

version="v1.0.1"
akagentDockerPkg="akagent-docker-${version}.tar"


check_module_exist(){
	targetMod=$1
	# shellcheck disable=SC2006
	result=`lsmod | grep "$targetMod" | awk -F " " '{print $1}'`
	if [ "$result" == "$targetMod" ]
	then
	  if [ "$result" == "akfs" ];then
		rmmod file
		rmmod process
		rmmod net
		echo "akfs branch go"
		umount /opt/mount
	  fi
	  echo "$targetMod.ko is already existed!rmmod first"
	  rmmod $targetMod
	  
	  return 1
	fi
	return 0
}

install_akfs(){
	check_module_exist akfs
    insmod akfs/akfs.ko
    # test -D /opt/mount||mkdir -p /opt/mount
	# -d 参数判断 $folder 是否存在
	folder=/opt/mount
	if [ ! -d "/opt/mount" ]
	then
	  mkdir -p "/opt/mount"
	fi
    mount -t akfs none /opt/mount
	echo "install akfs/akfs.ko success"
}

install_akps(){
	check_module_exist process
    insmod akps/process.ko
	echo "install akps/process.ko success"
}

install_akfile(){
	check_module_exist file
    insmod akfile/file.ko
	echo "install akfile/file.ko success"
}

install_aknet(){
	check_module_exist net
    insmod aknet/net.ko
	echo "install aknet/net.ko success"
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
  #install_akagent_docker
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


#person_menu
install_akfs
install_akps
install_akfile
install_aknet




