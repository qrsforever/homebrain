#!/bin/bash
#=================================================================
# date: 2018-05-30 13:47:20
# title: postbuild
#=================================================================

if [[ $# != 3 ]]
then
    echo "Error, Use: prebuild.sh project_top_dir platform release_dir"
    exit
fi

echo "======> postbuild project top dir: [$1], platform: [$2], out dir: [$3]"

pro_top_dir=$1
platform=$2
release_dir="$pro_top_dir/out/$3"
current_dir=`dirname ${BASH_SOURCE[0]}`
output_dir=$pro_top_dir/out/${platform}/root
hb_dir=$pro_top_dir/homebrain
build_dir=$hb_dir/build
script_dir=$build_dir/script

. $script_dir/common_utils.sh

if [[ ! -d $release_dir ]]
then
    echo "Error: build fails, see log file[${pro_top_dir}/build.log]"
    exit
fi

#-----------------------------------------------------------------
#   prepare install dir
#-----------------------------------------------------------------

if [[ -d $output_dir ]]
then
    rm -rf $output_dir/usr
    rm -rf $output_dir/data
fi

mkdir -p $output_dir/data/homebrain
mkdir -p $output_dir/usr/bin
mkdir -p $output_dir/usr/lib

dst_lib_dir=$output_dir/usr/lib


#-----------------------------------------------------------------
#	copy includes
#-----------------------------------------------------------------

# if [[ ! -d $release_dir/include ]]
# then
#     echo "Error: not found $release_dir/include dir!"
#     exit
# fi
# 
# cp -aprf $release_dir/include  $output_dir/system/


#-----------------------------------------------------------------
#	copy libs
#-----------------------------------------------------------------

__Check_and_Copy $release_dir/liboc.so $dst_lib_dir
__Check_and_Copy $release_dir/liboc_logger.so $dst_lib_dir
__Check_and_Copy $release_dir/liboctbstack.so $dst_lib_dir
__Check_and_Copy $release_dir/libconnectivity_abstraction.so $dst_lib_dir
__Check_and_Copy $release_dir/libHJ_KKSDK_Arm_64_Linux.so $dst_lib_dir
__Check_and_Copy $release_dir/libdevice_manager.so $dst_lib_dir
__Check_and_Copy $release_dir/libipca.so $dst_lib_dir
__Check_and_Copy $release_dir/libHB_RuleEngine.so $dst_lib_dir
__Check_and_Copy $release_dir/libHB_Utils.so $dst_lib_dir
__Check_and_Copy $release_dir/libHB_GWAgent.so $dst_lib_dir
__Check_and_Copy $release_dir/libHB_StorageTable.so $dst_lib_dir
__Check_and_Copy $release_dir/libiotivity-constrained-server.so $dst_lib_dir
__Check_and_Copy $release_dir/libuuid.so $dst_lib_dir

#-----------------------------------------------------------------
#  copy pkgconfg file
#-----------------------------------------------------------------

# if [[ -f $release_dir/lib/pkgconfig/iotivity.pc ]]
# then
#     cp $release_dir/lib/pkgconfig/iotivity.pc $output_dir/system
# fi

#-----------------------------------------------------------------
#	copy bin files
#-----------------------------------------------------------------

__Check_and_Copy $release_dir/iotsystem $output_dir/usr/bin
__Check_and_Copy $release_dir/smarthb $output_dir/usr/bin
__Check_and_Copy $release_dir/kk_bridge $output_dir/usr/bin
__Check_and_Copy $current_dir/hue_bridge $output_dir/usr/bin
__Check_and_Copy $current_dir/kk_bridge.sh $output_dir/usr/bin
__Check_and_Copy $current_dir/hue_bridge.sh $output_dir/usr/bin
__Check_and_Copy $current_dir/iot_server $output_dir/usr/bin

#-----------------------------------------------------------------
#	copy data files
#-----------------------------------------------------------------

__Check_and_Copy $build_dir/devices $output_dir/data/homebrain
__Check_and_Copy $build_dir/clips   $output_dir/data/homebrain
__Check_and_Copy $build_dir/www     $output_dir/data/homebrain

