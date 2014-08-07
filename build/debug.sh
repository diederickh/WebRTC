#!/bin/sh

if [ ! -d build.debug ] ; then 
    mkdir build.debug
fi

cd build.debug
cmake -DCMAKE_BUILD_TYPE=Debug ../ 
cmake --build . --target install
cd ./../../install/mac-clang-x86_64d/bin/
lldb ./test_extract_keying_info_for_srtpdebug
