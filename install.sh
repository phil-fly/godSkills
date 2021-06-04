#!/bin/bash

install_akfs(){
    echo "编译安装akfs"

    cd akfs
    chmod +x configure
    ./configure
    make
    insmod akfs.ko
    mkdir /opt/mount
    mount -t akfs none /opt/mount
    cd ../
}

install_akps(){
    echo "编译安装akps"

    cd akps
    chmod +x configure
    ./configure
    make
    insmod process.ko
    cd ../
}

install_akfile(){
    echo "编译安装akfile"

    cd akfile
    chmod +x configure
    ./configure
    make
    insmod file.ko
    cd ../
}

install_aknet(){
    echo "编译安装aknet"

    cd aknet
    chmod +x configure
    ./configure
    make
    insmod net.ko
    cd ../
}

install_akagent(){
    echo "编译安装akagent"

    cd akagent
    go build
}

install_akfs
install_akps
install_akfile
install_aknet
install_akagent
