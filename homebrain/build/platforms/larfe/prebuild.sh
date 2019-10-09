#!/bin/bash
#=================================================================
# date: 2018-05-30 12:32:29
# title: prebuild
#=================================================================

if [[ $# != 3 ]]
then
    echo "Error, Use: prebuild.sh project_top_dir platform outdir"
    exit -1
fi

pro_top_dir=$1
platform=$2
release_dir="$pro_top_dir/out/$3"
current_dir=`dirname ${BASH_SOURCE[0]}`
hb_dir=$pro_top_dir/homebrain
build_dir=$hb_dir/build
extlibs_dir=$build_dir/extlibs
script_dir=$build_dir/script

. $script_dir/record_extlibs_version.sh
. $script_dir/decompress_extlibs_tarball.sh

echo "======> prebuild project top dir: [$pro_top_dir], platform: [$platform] extlibs dir: [$extlibs_dir]"

output_dir=$pro_top_dir/out/${platform}
if [[ ! -d $output_dir ]]
then
    mkdir -p $output_dir
fi

#-----------------------------------------------------------------
#   Decompress the sysroot tarball
#-----------------------------------------------------------------

if [[ -f $current_dir/arm-linux-gcc-4.3.2.tar.gz ]]
then
    if [[ ! -d $output_dir/arm-linux-gcc-4.3.2 ]]
    then
        tar zxvf $current_dir/arm-linux-gcc-4.3.2.tar.gz -C $output_dir
    fi
else
    echo "Warning: not found $extlibs_dir/arm-linux-gcc-4.3.2.tar.gz!"
    exit -1
fi

#-----------------------------------------------------------------
#	Decompress the rapadjson
#-----------------------------------------------------------------

DET_DecompressTarball $extlibs_dir/rapidjson-1.0.2.tar.gz "1.0.2" $pro_top_dir/extlibs/rapidjson

#-----------------------------------------------------------------
#	Decompress the mbedtls tarball
#-----------------------------------------------------------------

if [[ ! -f $pro_top_dir/extlibs/mbedtls/mbedtls-2.4.2.tar.gz ]]
then
    cp -aprf $extlibs_dir/mbedtls-2.4.2.tar.gz  $pro_top_dir/extlibs/mbedtls
fi
DET_DecompressTarball $extlibs_dir/mbedtls-2.4.2.tar.gz "2.4.2" $pro_top_dir/extlibs/mbedtls

#-----------------------------------------------------------------
#	Decompress the mqtt
#-----------------------------------------------------------------

DET_DecompressTarball $extlibs_dir/mqtt/paho.mqtt.embedded-c-1.1.0.zip "1.1.0" $output_dir/extlibs/mqtt
