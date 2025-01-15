#!/bin/bash
cd ..
./0_Common/build.sh
cd 4_ThreadCalc
rm -fdr build_test
mkdir build_test
make
echo "------------------"
./build_test/my_program ../FileGenInput ../FileGenOutput -n 4