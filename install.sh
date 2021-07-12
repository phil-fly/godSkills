#!/bin/bash

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

install_akagent(){
    docker build -t bxsec/akagent:v1.0.1 .
}

install_akfs
install_akps
install_akfile
install_aknet
install_akagent
