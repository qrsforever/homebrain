#!/bin/sh

export PATH=/mnt/nandflash:$PATH

cur_dir=`pwd`

UPTIME=30
MAXCNT=10
DCOUNT=0

PROGRAM="$cur_dir/larfe_ga"
DBNAME="$cur_dir/lf.db"
SYSCMD="$cur_dir/system.cmd"

__reset_program() {
    if [ -f ${PROGRAM}.bk ]
    then
        cp ${PROGRAM}.bk ${PROGRAM}
        rm ${DBNAME}
    fi
}

__check_systemcmd() {
    if [ ! -f $SYSCMD ]
    then
        return
    fi
    cmd=`cat $SYSCMD`
    if [[ x$cmd == x"recovery" ]]
    then
        __reset_program
    fi
    rm -f $SYSCMD
}

$cur_dir/hostdiscovery &

while :
do
    mv homebrain.log homebrain.log.pre
    st=`cat /proc/uptime | cut -d\  -f1 | cut -d. -f1`
    $PROGRAM -d $cur_dir -f
    ed=`cat /proc/uptime | cut -d\  -f1 | cut -d. -f1`
    if [[ $(expr $ed - $st) -lt $UPTIME ]]
    then
        DCOUNT=`expr $DCOUNT + 1`
        if [[ $DCOUNT -gt $MAXCNT ]]
        then
            __reset_program
            DCOUNT=0
        fi
    else
        __check_systemcmd
    fi
    sleep 5
done
