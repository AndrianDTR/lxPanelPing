#!/bin/bash

#Edit these variables
LXDEBUILD_SRC_ROOTDIR="$HOME/random_stuff/src"
LXDEBUILD_LXDE_SRC_ROOTDIR="$LXDEBUILD_SRC_ROOTDIR/lxde"
LXDEBUILD_PREFIX="$HOME/opt/lxde"

#Environment variables
export PKG_CONFIG_PATH=$LXDEBUILD_PREFIX/lib/pkgconfig
export LD_LIBRARY_PATH=$LXDEBUILD_PREFIX/lib
export LD_RUN_PATH=$LXDEBUILD_PREFIX/lib
export CFLAGS="-I$LXDEBUILD_PREFIX/include"
export CPPFLAGS="-I$LXDEBUILD_PREFIX/include"

#Now execute the commands needed to build
./autogen.sh 
./configure --enable-man --prefix=$LXDEBUILD_PREFIX --oldincludedir=$LXDEBUILD_PREFIX/include --with-libdir=$LXDEBUILD_PREFIX/lib --sysconfdir=$LXDEBUILD_PREFIX/etc 
make
