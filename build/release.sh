#!/bin/sh

d=${PWD}

if [ ! -d build.release ] ; then 
    mkdir build.release
fi

cd build.release
cmake -DCMAKE_BUILD_TYPE=Release ../ 
cmake --build . --target install
cd ./../../install/mac-clang-x86_64/bin/
#./test_ice
#./test_ssl_fingerprint
#./test_hmac_sha1
./test_stun_message_integrity
