#!/bin/bash

d=${PWD}
sd=${d}/linux-sources
bd=${d}/../extern/linux-gcc-x86_64

export PATH=${PATH}:${bd}/bin/
export CFLAGS="-I\"${bd}/include\""
export LDFLAGS="-L\"${bd}/lib\""

# ----------------------------------------------------------------------- #
#                D O W N L O A D   D E P E N D E N C I E S 
# ----------------------------------------------------------------------- #

# Create source dir
if [ ! -d ${sd} ] ; then 
    mkdir -p ${sd}
fi

# Download openssl
if [ ! -d ${sd}/openssl ] ; then
    cd ${sd}
    if [ ! -f openssl.tar.gz ] ; then 
        curl -o openssl.tar.gz http://www.openssl.org/source/openssl-1.0.1i.tar.gz
        tar -zxvf openssl.tar.gz
    fi
    mv openssl-1.0.1i openssl
fi

# Download libuv
if [ ! -d ${sd}/libuv ] ; then
    cd ${sd}
    git clone https://github.com/joyent/libuv.git libuv
fi

# Download libz
if [ ! -d ${sd}/zlib ] ; then
    cd ${sd}
    if [ ! -f libz.tar.gz ] ; then
        curl -o libz.tar.gz http://zlib.net/zlib-1.2.8.tar.gz
        tar -zxvf libz.tar.gz
    fi
    mv zlib-1.2.8 zlib
fi

# Download libsrtp 
if [ ! -d ${sd}/libsrtp ] ; then
    cd ${sd}
    git clone https://github.com/cisco/libsrtp.git libsrtp
fi

# Download libvpx
if [ ! -d ${sd}/libvpx ] ; then 
    cd ${sd}
    git clone https://chromium.googlesource.com/webm/libvpx libvpx
fi

# Download libvideogenerator 
if [ ! -d ${sd}/libvideogenerator ] ; then
    cd ${sd}
    git clone git@github.com:roxlu/video_generator.git libvideogenerator
fi 

# Download yasm, needed for libvpx
if [ ! -d ${sd}/yasm ] ; then
    cd ${sd}
    curl -o yasm.tar.gz http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz
    tar -zxvf yasm.tar.gz
    mv yasm-1.3.0 yasm
fi

# Download mongoose (signaling)
if [ ! -d ${sd}/mongoose ] ; then 
    cd ${sd}
    git clone https://github.com/cesanta/mongoose.git mongoose
fi    
  
if [ ! -f ${bd}/src/mongoose.c ] ; then
    cp ${sd}/mongoose/mongoose.c ${bd}/src/
    cp ${sd}/mongoose/mongoose.h ${bd}/include/
fi


# Download net_skeleton (signaling)
if [ ! -d ${sd}/net_skeleton ] ; then 
    cd ${sd}
    git clone https://github.com/cesanta/net_skeleton.git net_skeleton
fi

if [ ! -f ${bd}/src/net_skeleton.c ] ; then
    cp ${sd}/net_skeleton/net_skeleton.c ${bd}/src/
    cp ${sd}/net_skeleton/net_skeleton.h ${bd}/include/
fi


# Download ssl_wrapper (signaling)
if [ ! -d ${sd}/ssl_wrapper ] ; then 
    cd ${sd}
    git clone https://github.com/cesanta/ssl_wrapper.git ssl_wrapper
fi

if [ ! -f ${bd}/src/ssl_wrapper.c ] ; then
    cp ${sd}/ssl_wrapper/ssl_wrapper.c ${bd}/src/
    cp ${sd}/ssl_wrapper/ssl_wrapper.h ${bd}/include/
fi

# ----------------------------------------------------------------------- #
#                C O M P I L E   D E P E N D E N C I E S 
# ----------------------------------------------------------------------- #

# Compile openSSL
if [ ! -f ${bd}/lib/libssl.a ] ; then 
    cd ${sd}/openssl
    ./Configure --prefix=${bd} linux-x86_64
    make clean
    make
    make install
fi

# Compile libuv
if [ ! -f ${bd}/lib/libuv.a ] ; then
    cd ${sd}/libuv
    ./autogen.sh
    ./configure --prefix=${bd} --enable-static
    make
    make install
fi

# Compile zlib
if [ ! -f ${bd}/lib/libz.a ] ; then 
    cd ${sd}/zlib
    ./configure --prefix=${bd} --static --64
    make
    make install
fi

# Compile libsrtp
if [ ! -f ${bd}/lib/libsrtp.a ] ; then
    cd ${sd}/libsrtp
    ./configure --prefix=${bd}
    make
    make install
fi

# Compile yasm
if [ ! -f ${bd}/bin/yasm ] ; then
    cd ${sd}/yasm
    ./configure --prefix=${bd}
    make
    make install
fi

# Compile libvpx
if [ ! -f ${bd}/lib/libvpx.a ] ; then 
    cd ${sd}/libvpx
    ./configure --prefix=${bd} --as=yasm --disable-shared --enable-static
    make
    make install
fi

# Compile libvideogenerator
if [ ! -f ${bd}/lib/libvideogenerator.a ] ; then 
    cd ${sd}/libvideogenerator/build
    cmake -DCMAKE_INSTALL_PREFIX=${bd}
    cmake --build . --target install
fi


