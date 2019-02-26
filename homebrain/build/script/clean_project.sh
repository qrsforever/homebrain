#!/bin/bash
#=================================================================
# date: 2018-05-30 19:50:42
# title: clean_project
#=================================================================

#-----------------------------------------------------------------
#	Clean projects
#-----------------------------------------------------------------

scons -c

if [[ $# != 1 ]]
then
    echo "Error: clean_project project_top_dir"
    exit
fi

pro_top_dir=$1
script_dir=$pro_top_dir/homebrain/build/script

. $script_dir/record_extlibs_version.sh

if [[ -d $pro_top_dir/out ]]
then
    rm -rf $pro_top_dir/out
fi

#-----------------------------------------------------------------
#	Clean extlibs/boost
#-----------------------------------------------------------------

REV_RemoveVersionNumber $pro_top_dir/extlibs/boost
rm -rf $pro_top_dir/extlibs/boost/boost*
rm -rf $pro_top_dir/dep

#-----------------------------------------------------------------
#	Clean extlibs/mbedtls
#-----------------------------------------------------------------

REV_RemoveVersionNumber $pro_top_dir/extlibs/mbedtls
rm -rf $pro_top_dir/extlibs/mbedtls/mbedtls
rm -rf $pro_top_dir/extlibs/mbedtls/libmbed*.a

#-----------------------------------------------------------------
#	Clean extlibs/tinycbor
#-----------------------------------------------------------------

REV_RemoveVersionNumber $pro_top_dir/extlibs/tinycbor
rm -rf $pro_top_dir/extlibs/tinycbor/tinycbor

#-----------------------------------------------------------------
#	Clean extlibs/crow
#-----------------------------------------------------------------
rm -rf $pro_top_dir/extlibs/crow
