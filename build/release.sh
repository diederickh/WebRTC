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
#./test_zlib_crc32
#./test_hmac_sha1
#./test_stun_message_integrity
#./test_stun_message_fingerprint
#./test_openssl_load_key_and_cert
#./test_libwebsockets
./test_ice_agent
#./test_extract_keying_info_for_srtp
