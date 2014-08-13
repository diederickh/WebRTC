#!/bin/sh

d=${PWD}

if [ ! -d build.release ] ; then 
    mkdir build.release
fi

if [ "$(uname)" == "Darwin" ] ; then 
    if [ ! -d ${d}/../extern/mac-clang-x86_64 ] ; then
        ./build_mac_dependencies.sh
        cd build.release
    fi
else
    if [ ! -d ${d}/../extern/linux-gcc-x86_x64 ] ; then
        ./build_linux_dependencies.sh
    fi
fi

cd build.release
cmake -DCMAKE_BUILD_TYPE=Release ../ 
cmake --build . --target install

if [ "$(uname)" == "Darwin" ] ; then 
    export PATH=${d}/../install/mac-clang-x86_64/bin/:${PATH}
    cd ./../../install/mac-clang-x86_64/bin/
    if [ ! -f server-key.pem ] ; then
         openssl req -x509 -newkey rsa:2048 -days 3650 -nodes -subj "/C=/ST=/L=/O=/CN=roxlu.com" -keyout server-key.pem -out server-cert.pem
    fi
else
    export PATH=${d}/../install/linux-gcc-x86_64/bin/:${PATH}
    cd ./../../install/linux-gcc-x86_64/bin/
    if [ ! -f server-key.pem ] ; then
         openssl req -x509 -newkey rsa:2048 -days 3650 -nodes -subj "/C=/ST=/L=/O=/CN=roxlu.com" -keyout server-key.pem -out server-cert.pem
    fi
fi

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
#./test_video_encoder
#./test_mongoose
#./test_signaling
