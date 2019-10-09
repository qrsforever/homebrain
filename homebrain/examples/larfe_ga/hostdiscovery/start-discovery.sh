#!/bin/sh
ip=`ifconfig eth0 | grep "inet addr" | awk '{ print $2}' | awk -F: '{print $2}'`
sn="$1"
killall hostdiscovery
./hostdiscovery $ip $sn &
