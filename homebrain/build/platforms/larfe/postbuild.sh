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
output_dir=$pro_top_dir/out/${platform}/user/exec_larfe
hb_dir=$pro_top_dir/homebrain
build_dir=$hb_dir/build
script_dir=$build_dir/script

current_dir=`dirname ${BASH_SOURCE[0]}`

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
    rm -rf $output_dir
fi

mkdir -p $output_dir

#-----------------------------------------------------------------
#	copy bin files
#-----------------------------------------------------------------

__Check_and_Copy $release_dir/homebrain/examples/larfe_ga/larfe_ga $output_dir
__Check_and_Copy $hb_dir/examples/larfe_ga/hostdiscovery/hostdiscovery $output_dir
__Check_and_Copy $hb_dir/examples/larfe_ga/ota/ota.sh $output_dir
__Check_and_Copy $hb_dir/examples/larfe_ga/log/log.sh $output_dir
__Check_and_Copy $hb_dir/examples/larfe_ga/ota/version $output_dir
__Check_and_Copy $current_dir/startup.sh $output_dir
__Check_and_Copy $current_dir/start.sh $output_dir/..
__Check_and_Copy $current_dir/README.md $output_dir/..
