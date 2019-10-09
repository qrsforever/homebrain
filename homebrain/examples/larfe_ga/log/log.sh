#!/bin/sh
#log file & elinkid for sdk
sdklog="$1"
elinkid="$2"
logid="$3"
#log
logdir="/tmp/hblog"
uploadlogfile="/tmp/hblog.tgz"
systemlogfile="$logdir/system.log"
rm -rf $logdir
mkdir -p $logdir
date >> $systemlogfile
dmesg >> $systemlogfile
ifconfig >> $systemlogfile
busybox ps >> $systemlogfile
busybox top -n 1 >> $systemlogfile
free >> $systemlogfile
cat /proc/cpuinfo >> $systemlogfile
if [ -f "$sdklog" ];then
	cp -rf $sdklog $logdir
fi
tar zcf $uploadlogfile $logdir 2>/dev/null

#router sk&ak
sk="skfileup_wChWwsmVOnqr6iaB"
ak="akfileup_YHoqMHgt"
systime=`date +%s`
#systime="156378456"
PWD=`pwd`
# get device mac address
deviceId=`ifconfig eth0|grep "HWaddr"|awk '{print $5}'|awk '{gsub(/:/,"");print}'`

# get currentVersion
versionCode=`grep versionCode ${PWD}/version 2>/dev/null|awk -F"=" '{print $2}'`
msg="$logid"
device_id="V$versionCode"

url_nosign="_ak=${ak}&_time=${systime}"
_sign=`echo -n "${url_nosign}${sk}" |md5sum |cut -d" " -f1`
url="${url_nosign}&_sign=${_sign}"

server="http://xfeedback.scloud.letv.com/api/v2/feedback?$url"
# upload log file
CURL="curl"
if [ -f "/mnt/nandflash/curl" ];then
	CURL="/mnt/nandflash/curl"
fi
respinfo=$($CURL -X POST -F 'data={"log_from":"router", "date":"'$systime'", "mac":"'${deviceId}'","message":"'$msg'","user_name":"", "uid":"'$elinkid'", "device_id":"'$device_id'"}' -F 'type=3' -F "log=@${uploadlogfile}"  $server 2>/dev/null)

json_ret=""
ota_json_get() {
	       key=$1
	       json_ret=`echo $respinfo|sed 's/[ |\"]//g'|sed 's/[,|{|}]/\n/g' |grep $key|awk -F":" '{print $2}'`
}
ota_json_get errno
errno=$json_ret
if [ "$errno" == "10000" ];then
	#upload file success
	echo "success"
else
	echo "failed"
fi

