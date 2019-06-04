#!/bin/bash

if [[ $# < 2 ]]
then
    echo "Error, Use: prebuild.sh project_top_dir platform [build_dir]"
    exit -1
fi

pro_top_dir=$1
platform=$2
build_dir=$pro_top_dir"/"$3
current_dir=`dirname ${BASH_SOURCE[0]}`

echo "======> Build top dir:[$pro_top_dir] platform:[$platform] current dir:[$current_dir]"

__initEnv() {

    export TARGET_OS=linux
    export TARGET_ARCH=arm64

    export CROSS_PREFIX=aarch64-linux-gnu-

    export SYSROOT_DIR=${pro_top_dir}/out/${platform}/sysroot_aarch64-linux-gnu
    export PROJECT_PLATFORM=$platform
    export PATH=$SYSROOT_DIR/bin:$PATH

    echo "======> SYSROOT_DIR: $SYSROOT_DIR"
    echo "======> CROSS_PREFIX: $CROSS_PREFIX"
}

__main() {

    __initEnv

    L=INFO
    R=True
    P=False
    DIR=release
    if [[ x$DEBUG == x1 ]]
    then
        L=DEBUG
        R=False
        P=True
        DIR=debug
    fi

    $pro_top_dir/homebrain/build/script/gen_build_info.sh $pro_top_dir

    if [[ -f $current_dir/prebuild.sh ]]
    then
        $current_dir/prebuild.sh $pro_top_dir $platform "${TARGET_OS}/${TARGET_ARCH}/${DIR}"
    fi

    if [[ x$? != x0 ]]
    then
        echo -e "\033[31m ****** Prebuild Error[$?]\033[0m"
        exit -1
    fi

    scons VERBOSE=1 \
        TARGET_OS=${TARGET_OS}\
        TARGET_ARCH=${TARGET_ARCH} \
        ANDROID_BUILD_JAVA=0 \
        TEST=0 \
        LOG_LEVEL=$L \
        ERROR_ON_WARN=$R \
        BUILD_JAVA=0 \
        TARGET_TRANSPORT=IP \
        SECURED=0 \
        SECURE=0 \
        IPV4=1 \
        DYNAMIC=1 \
        LOGGING=$P \
        RELEASE=$R $build_dir -j$JJ | tee $pro_top_dir/build.log

    if [[ x$? != x0 || x`tail -n1 build.log  | grep -o errors` == x"errors" ]]
    then
        echo -e "\033[31m******Build Error[$?]:\033[0m See $pro_top_dir/build.log"
        exit -1
    fi

    if [[ -f $current_dir/postbuild.sh ]]
    then
        $current_dir/postbuild.sh $pro_top_dir $platform "${TARGET_OS}/${TARGET_ARCH}/${DIR}"
    fi
}

__main
