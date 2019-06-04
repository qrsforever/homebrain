#!/bin/bash

# hue_bridge -c /system/etc/homebrain/hue_bridge.conf
if [[ $# == 2 ]]
then
    hue_bridge -c $2/$1
fi
