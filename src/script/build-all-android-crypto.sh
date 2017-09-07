#!/bin/bash

export AOSP_API="android-14"
for arch in armeabi armeabi-v7a armeabi-v7a-hard arm64-v8a mips mips64 x86 x86_64
NOWPATH=$(pwd)
cd ./../../3rd/cryptopp
for arch in armeabi
do
	./setenv-android.sh $arch stlport-static
	if [ "$?" -eq "0" ]; then
		make -f ./GNUmakefile-cross distclean
		make -f ./GNUmakefile-cross static dynamic
		sudo make -f ./GNUmakefile-cross install PREFIX=$NOWPATH/cryptopp/android-$arch
	fi
done
