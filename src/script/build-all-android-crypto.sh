#!/bin/bash

export ANDROID_NDK_ROOT=$1
NOWPATH=$(pwd)
cd ./../../3rd/cryptopp
. ./setenv-android.sh $2 $3
make -f ./GNUmakefile-cross distclean
make -f ./GNUmakefile-cross static dynamic -j4
make -f ./GNUmakefile-cross install PREFIX=$NOWPATH/cryptopp/$2/
