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

echo "======> postbuild project top dir: [$1], platform: [$2], relase dir: [$3]"

pro_top_dir=$1
platform=$2
release_dir="$pro_top_dir/out/$3"
current_dir=`dirname ${BASH_SOURCE[0]}`
output_dir=$pro_top_dir/out/${platform}
hb_dir=$pro_top_dir/homebrain
build_dir=$hb_dir/build
script_dir=$build_dir/script

. $script_dir/common_utils.sh

if [[ ! -d $release_dir ]]
then
    echo "Error: build fails, see log file[${pro_top_dir}/build.log]"
    exit
fi

if [[ -d $output_dir/root ]]
then
    rm -rf $output_dir/root
fi

mkdir -p $output_dir/root/bin
mkdir -p $output_dir/root/lib
mkdir -p $output_dir/root/homebrain

__Check_and_Copy $release_dir/iotsystem $output_dir/root/bin
__Check_and_Copy $release_dir/smarthb   $output_dir/root/bin
__Check_and_Copy $build_dir/devices $output_dir/root/homebrain
__Check_and_Copy $build_dir/clips   $output_dir/root/homebrain
__Check_and_Copy $build_dir/www     $output_dir/root/homebrain
