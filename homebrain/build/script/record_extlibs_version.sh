#!/bin/bash
#=================================================================
# date: 2018-05-30 17:09:08
# title: record_extlibs_version
# info: called by prebuild.sh
#=================================================================

REV_WriteVersionNumber() {
    if [[ $# == 2 ]]
    then
        echo "$1" > $2/version.info
    else
        echo "REV_WriteVersionNumber version libdir"
    fi
}

REV_ReadVersionNumber() {
    if [[ $# == 1 ]]
    then
        if [[ -f $1/version.info ]]
        then
            result=`cat $1/version.info`
        else
            result="-1"
        fi
        echo $result
    else
        echo "REV_ReadVersionNumber libdir"
    fi
}

REV_CompareVersionNumber() {
    if [[ $# == 2 ]]
    then
        result=$(REV_ReadVersionNumber $2)
        if [[ x$result == x$1 ]]
        then
            echo "1"
        else
            echo "0"
        fi
    else
        echo "REV_CompareVersionNumber version libdir"
    fi
}

REV_RemoveVersionNumber() {
    if [[ $# == 1 ]]
    then
        if [[ -d $1 ]]
        then
            rm -f $1/version.info
        fi
    fi
}
