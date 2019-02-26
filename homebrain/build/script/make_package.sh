#!/bin/bash

if [[ $# != 2 ]]
then
    echo "Error: parameters is invalid!"
    exit -1
fi

top_dir=$1
dst_dir=$top_dir/out/$2

if [[ ! -d $dst_dir ]]
then
    echo "Error: $dst_dir not found!"
    exit -1
fi

dt=`date +"%Y%m%d%H%M%S"`

cd $dst_dir

tar zcvf homebrain_$2_$dt.tar.gz root

mv homebrain_$2_$dt.tar.gz $top_dir

cd -
