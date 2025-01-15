#!/bin/bash
current_folder=$(basename "$PWD")
if [ "$current_folder" = "4_ThreadCalc" ]; then
    cd ../0_Common
    ./build.sh
    cd ../4_ThreadCalc
    rm -fdr build
    mkdir build
    cd build 
    cmake ..
    make
else
    cd ./0_Common
    ./build.sh
    cd ../4_ThreadCalc
    rm -fdr build
    mkdir build
    cd build
    cmake ..
    make
fi
