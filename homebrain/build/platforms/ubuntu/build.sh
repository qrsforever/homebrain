#!/bin/bash
#=================================================================
# date: 2018-05-30 16:57:21
# title: build
#=================================================================

if [[ $# < 1 ]]
then
    echo "Error, Use: prebuild.sh project_top_dir [buil_dir]"
    exit -1
fi

pro_top_dir=$1
platform=$2
build_dir=$pro_top_dir"/"$3
current_dir=`dirname ${BASH_SOURCE[0]}`

echo "======> Build top dir:[$pro_top_dir] current dir:[$current_dir]"

__main() {

    $pro_top_dir/homebrain/build/script/gen_build_info.sh $pro_top_dir

    export TARGET_OS=linux
    export TARGET_ARCH=x86_64
    export PROJECT_PLATFORM=$platform

    if [[ -f $current_dir/prebuild.sh ]]
    then
        $current_dir/prebuild.sh $pro_top_dir $platform
    fi

    if [[ x$? != x0 ]]
    then
        echo -e "\033[31m ****** Prebuild Error[$?]\033[0m"
        exit -1
    fi

    L=INFO
    R=True
    P=False
    if [[ x$DEBUG == x1 ]]
    then
        L=DEBUG
        R=False
        P=True
    fi
    scons VERBOSE=1 \
        TEST=0 \
        LOG_LEVEL=$L \
        ERROR_ON_WARN=False \
        WITH_TCP=yes \
        TARGET_TRANSPORT=IP \
        SECURED=0 \
        LOGGING=$P \
        RELEASE=$R -j$JJ $build_dir | tee $pro_top_dir/build.log

    if [[ -f $current_dir/postbuild.sh ]]
    then
        if [[ x$R == xTrue ]]
        then
            $current_dir/postbuild.sh $pro_top_dir $platform "${TARGET_OS}/${TARGET_ARCH}/release"
        else
            $current_dir/postbuild.sh $pro_top_dir $platform "${TARGET_OS}/${TARGET_ARCH}/debug"
        fi
    fi
}

__main
