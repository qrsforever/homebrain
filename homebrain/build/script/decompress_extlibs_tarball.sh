#!/bin/bash
#=================================================================
# date: 2018-05-30 17:38:40
# title: decompress_extlibs
# info: called by prebuild.sh
#=================================================================

__decompress_zip_file() {
    cmd=`which unzip`
    if [[ x$cmd == x ]]
    then
        echo "Error: not found unzip cmd"
        exit -1
    fi
    $cmd $1 -d $3
    if [[ x$? == x0 ]]
    then
        REV_WriteVersionNumber $2 $3
    fi
}

__decompress_tar_file() {
    cmd=`which tar`
    if [[ x$cmd == x ]]
    then
        echo "Error: not found unzip cmd"
        exit -1
    fi
    $cmd zxvf $1 -C $3
    if [[ x$? == x0 ]]
    then
        REV_WriteVersionNumber $2 $3
    fi
}

DET_DecompressTarball() {
    if [[ $# == 3 ]]
    then
        tarfile=$1
        version=$2
        destdir=$3
        ret=$(REV_CompareVersionNumber $version $destdir)
        if [[ x$ret != x1 ]]
        then
            if [[ -f $tarfile ]]
            then
                if [[ x${tarfile##*.} == x"zip" ]]
                then
                    __decompress_zip_file $tarfile $version $destdir
                elif [[ x${tarfile##*.} == x"gz" ]]
                then
                    __decompress_tar_file $tarfile $version $destdir
                else
                    echo "Waring: unkown file $tarfile!"
                    exit -1
                fi
            else
                echo "Warning: not found $tarfile!"
                exit -1
            fi
        fi
    else
        echo "Error: DET_DecomppressTarball tarfile version destdir!"
        exit -1
    fi
}
