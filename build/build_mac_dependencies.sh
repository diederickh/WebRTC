#!/bin/bash

d=${PWD}
sd=${d}/mac-sources
bd=${d}/../extern/mac-clang-x86_64

export PATH=${PATH}:${bd}/bin/
export CFLAGS="-I\"${bd}/include\""
export LDFLAGS="-L\"${bd}/lib\""

# ----------------------------------------------------------------------- #
#                D O W N L O A D   D E P E N D E N C I E S 
# ----------------------------------------------------------------------- #
if [ ! -d ${sd} ] ; then 
    mkdir -p ${sd}
fi

# Download mongoose (signaling)
if [ ! -d ${sd}/mongoose ] ; then 
    cd ${sd}
    git clone https://github.com/cesanta/mongoose.git mongoose
    
    if [ ! -d ${bd}/src ] ; then 
        mkdir ${bd}/src
    fi

    cp mongoose/mongoose.c ${bd}/src/
    cp mongoose/mongoose.h ${bd}/include/
fi

# Download net_skeleton (signaling)
if [ ! -d ${sd}/net_skeleton ] ; then 
    cd ${sd}

    if [ ! -d net_skeleton ] ; then
        git clone https://github.com/cesanta/net_skeleton.git net_skeleton
    fi

    if [ ! -d ${bd}/src ] ; then 
        mkdir ${bd}/src
    fi

    cp net_skeleton/net_skeleton.c ${bd}/src/
    cp net_skeleton/net_skeleton.h ${bd}/include/
fi

# Download ssl_wrapper (signaling)
if [ ! -d ${sd}/ssl_wrapper ] ; then 
    cd ${sd}
    git clone https://github.com/cesanta/ssl_wrapper.git ssl_wrapper

    if [ ! -d ${bd}/src ] ; then 
        mkdir ${bd}/src
    fi

    cp ssl_wrapper/ssl_wrapper.c ${bd}/src/
    cp ssl_wrapper/ssl_wrapper.h ${bd}/include/
fi
