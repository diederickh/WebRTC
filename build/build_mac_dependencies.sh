#!/bin/bash
set -x
d=${PWD}
sd=${d}/mac-sources
bd=${d}/../extern/mac-clang-x86_64

export PATH=${PATH}:${bd}/bin/:${sd}/gyp/
export CFLAGS="-I\"${bd}/include\""
export LDFLAGS="-L\"${bd}/lib\""

# ----------------------------------------------------------------------- #
#                D O W N L O A D   D E P E N D E N C I E S 
# ----------------------------------------------------------------------- #
if [ ! -d ${sd} ] ; then 
    mkdir -p ${sd}
fi

if [ ! -d ${bd}/src ] ; then 
    mkdir ${bd}/src
fi

# Download autoconf and friends (for libuv)
if [ ! -d ${sd}/autoconf ] ; then 
    cd ${sd}
    curl -o autoconf.tar.gz http://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz
    tar -zxvf autoconf.tar.gz
    mv autoconf-2.69 autoconf
fi 

# Download libtool
if [ ! -d ${sd}/libtool ] ; then
    cd ${sd}
    curl -o libtool.tar.gz http://ftp.gnu.org/gnu/libtool/libtool-2.4.2.tar.gz
    tar -zxvf libtool.tar.gz
    mv libtool-2.4.2 libtool
fi

# Download automake
if [ ! -d ${sd}/automake ] ; then
    cd ${sd}
    curl -o automake.tar.gz http://ftp.gnu.org/gnu/automake/automake-1.14.tar.gz
    tar -zxvf automake.tar.gz
    mv automake-1.14 automake
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

# Download gyp for libuv
if [ ! -d ${sd}/libuv/build/gyp ] ; then 
    cd ${sd}/libuv
    git clone https://git.chromium.org/external/gyp.git build/gyp
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

# Cleanup some files we don't need anymore.
if [ -f ${sd}/autoconf.tar.gz ] ; then
    rm ${sd}/autoconf.tar.gz
fi
if [ -f ${sd}/automake.tar.gz ] ; then
    rm ${sd}/automake.tar.gz
fi
if [ -f ${sd}/libtool.tar.gz ] ; then
    rm ${sd}/libtool.tar.gz 
fi
if [ -f ${sd}/libz.tar.gz ] ; then
    rm ${sd}/libz.tar.gz 
fi
if [ -f ${sd}/openssl.tar.gz ] ; then
    rm ${sd}/openssl.tar.gz
fi
if [ -f ${sd}/yasm.tar.gz ] ; then
    rm ${sd}/yasm.tar.gz
fi

# ----------------------------------------------------------------------- #
#                C O M P I L E   D E P E N D E N C I E S 
# ----------------------------------------------------------------------- #

# Compile autoconf
if [ ! -f ${bd}/bin/autoconf ] ; then
    cd ${sd}/autoconf
    ./configure --prefix=${bd}
    make
    make install
fi

# Compile libtool
if [ ! -f ${bd}/bin/libtool ] ; then
    cd ${sd}/libtool
    ./configure --prefix=${bd}
    make
    make install
fi

if [ ! -f ${bd}/bin/automake ] ; then 
    cd ${sd}/automake
    ./configure --prefix=${bd}
    make
    make install
fi

# Compile openSSL
if [ ! -f ${bd}/lib/libssl.a ] ; then 
    cd ${sd}/openssl
    ./Configure --prefix=${bd} darwin64-x86_64-cc
    make clean
    make
    make install
fi

# Compile libuv
if [ ! -f ${bd}/lib/libuv.a ] ; then
    cd ${sd}/libuv
    ./gyp_uv.py -f xcode
    xcodebuild -ARCHS="x86_64" -project uv.xcodeproj -configuration Release -target All
    cp ${sd}/libuv/build/Release/libuv.a ${bd}/lib/
    cp ${sd}/libuv/include/*.h ${bd}/include/
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

