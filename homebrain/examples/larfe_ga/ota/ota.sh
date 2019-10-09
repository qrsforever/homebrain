#!/bin/sh
cmd="$1"
systime=`date +%s`
deviceType="3rd"
PWD=`pwd`
# get device mac address
deviceId=`ifconfig eth0|grep "HWaddr"|awk '{print $5}'|awk '{gsub(/:/,"");print}'`

# get device model 
package="com.lerong.larfe"

project="leronghb"

# get currentVersion
versionCode=`grep versionCode ${PWD}/version|awk -F"=" '{print $2}'`

firmware_dest="/tmp/firmware_hb"
OTA_UPGRADE_FLAG="newupgrade"
### Local version
if [ "$cmd" == "local_version" ];then
    local_version=`grep displayVersion ${PWD}/version|awk -F"=" '{print $2}'`
    echo -n $local_version
    exit 0
fi
### UPGRADE
if [ "$cmd" == "upgrade" ];then
      cp -rf $firmware_dest/$project/* $PWD/
      touch $PWD/$OTA_UPGRADE_FLAG
      reboot
      exit 0
fi
if [ "$cmd" == "version" ];then
  if [ -f ${firmware_dest}/$project/version ];then
	  versionName=`grep displayVersion ${firmware_dest}/$project/version 2>/dev/null|awk -F"=" '{print $2}'`
          echo -n "$versionName"
	  exit 0
  fi
fi

param="package=$package&version=$versionCode&type=$deviceType&mac=$deviceId"
CURL="/mnt/nandflash/curl"
#CURL="curl"
# internet server
SERVER="http://ota-scloud.cp21.ott.cibntv.net"
# test server
#SERVER="http://test.tvapi.letv.com"
#echo $CURL "$SERVER/iptv/api/apk/getUpgradeProfile?$param"
ota_info=`$CURL "$SERVER/iptv/api/apk/getUpgradeProfile?$param" 2>/dev/null`
json_ret=""
ota_json_get() {
	key=$1
	json_ret=`echo $ota_info |sed 's/ //g'|sed 's/,/\n/g' |grep $key|awk -F":" '{print $2}'`
}

#update 0:already the latest version, 1: new version found
ota_json_get update
errno=$json_ret

if [ "$errno" != "1" ];then
  exit 0
fi

pkgUrl=$(echo $ota_info | sed 's/,/\n/g' |grep url|awk -F"\"" '{print $4}'|sed 's/\\u0026/\&/g')

# vesion(int)
#ota_json_get version
#versionName=$json_ret

if [ "$cmd" == "version" ];then
  rm -rf $firmware_dest
  mkdir -p $firmware_dest

  $CURL "$pkgUrl" -L -o $firmware_dest/firmware_hb.bin 2>/dev/null
  tar xf $firmware_dest/firmware_hb.bin -C $firmware_dest 2>/dev/null
  versionName=`grep displayVersion ${firmware_dest}/$project/version 2>/dev/null|awk -F"=" '{print $2}'`
  echo -n "$versionName"
  exit 0
fi

