#!/bin/bash
#=================================================================
# date: 2018-05-30 17:00:30
# title: prebuild
#=================================================================

if [[ $# != 2 ]]
then
    echo "Error, Use: prebuild.sh project_top_dir"
    exit
fi

pro_top_dir=$1
platform=$2
build_dir=$pro_top_dir/homebrain/build
extlibs_dir=$build_dir/extlibs
script_dir=$build_dir/script

. $script_dir/record_extlibs_version.sh
. $script_dir/decompress_extlibs_tarball.sh

echo "======> prebuild project top dir: [$pro_top_dir], extlibs dir: [$extlibs_dir]"

output_dir=$pro_top_dir/out/${platform}
if [[ ! -d $output_dir ]]
then
    mkdir -p $output_dir
fi

#-----------------------------------------------------------------
#   Decompress the boost tarball
#-----------------------------------------------------------------

# DET_DecompressTarball $extlibs_dir/boost_1_60_0.zip "1.60.0" $pro_top_dir/extlibs/boost
# fix me, see extlibs/boost/SConscript: compiling when unzip this
if [[ ! -f  $pro_top_dir/extlibs/boost/boost_1_60_0.zip ]]
then
    cp -aprf $extlibs_dir/boost_1_60_0.zip  $pro_top_dir/extlibs/boost
fi

#-----------------------------------------------------------------
#	Decompress the mbedtls tarball
#-----------------------------------------------------------------

DET_DecompressTarball $extlibs_dir/mbedtls-2.4.2.tar.gz "2.4.2" $pro_top_dir/extlibs/mbedtls

#-----------------------------------------------------------------
#	Decompress the tinycbor
#-----------------------------------------------------------------

DET_DecompressTarball $extlibs_dir/tinycbor-v0.4.1.tar.gz "0.4.1" $pro_top_dir/extlibs/tinycbor

#-----------------------------------------------------------------
#	Decompress the hippomocks
#-----------------------------------------------------------------

DET_DecompressTarball $extlibs_dir/hippomocks-5.0.tar.gz "5.0" $pro_top_dir/extlibs/hippomocks

#-----------------------------------------------------------------
#	Decompress the rapadjson
#-----------------------------------------------------------------

DET_DecompressTarball $extlibs_dir/rapidjson-1.0.2.tar.gz "1.0.2" $pro_top_dir/extlibs/rapidjson

#-----------------------------------------------------------------
#	Decompress the crow
#-----------------------------------------------------------------

DET_DecompressTarball $extlibs_dir/crow-0.1.zip "0.1" $pro_top_dir/extlibs/crow

#-----------------------------------------------------------------
#	Decompress the mqtt
#-----------------------------------------------------------------

DET_DecompressTarball $extlibs_dir/mqtt/paho.mqtt.embedded-c-1.1.0.zip "1.1.0" $output_dir/extlibs/mqtt
