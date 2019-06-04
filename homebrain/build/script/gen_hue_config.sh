#!/bin/bash

INTERNAL_RESP=`curl -s https://www.meethue.com/api/nupnp`
INTERNAL_IP=`echo $INTERNAL_RESP | sed 's/.*internalipaddress":"\([0-9\.]*\)".*/\1/'`
INTERNAL_ID=`echo $INTERNAL_RESP | sed 's/.*id":"\(.*\)",.*/\1/'`

if [[ $# == 1 ]]
then
    read -p "Press the link button on Philips bridge, then resume with ENTER"
    ID_RESP=`curl -s -H "Content-Type: application/json" -X POST -d '{"devicetype":"lynxgateway#huebridge"}' http://$INTERNAL_IP/api`
    if [ "`echo $ID_RESP | grep success`" = "" ]; then
        echo $ID_RESP
        exit 1
    fi
    INTERNAL_USER=`echo $ID_RESP | sed 's/.*username":"\(.*\)".*/\1/'`
    CONFIG_FILE=hue_bridge.conf
else
    if [[ x$INTERNAL_ID == x001788fffe751108 ]]
    then
        INTERNAL_USER=2RU3MJROCn32Hn6OqJLg3IMzYTIUS9iruwNsH34C
    else
        INTERNAL_USER=sSJxR6KqKcctgaXQCGJRS0UxRNPkvQhF6GADztDp
    fi
    CONFIG_FILE=data/homebrain/hue_bridge.conf
fi

echo Configuration:
echo IP: $INTERNAL_IP
echo ID: $INTERNAL_ID
echo User: $INTERNAL_USER

echo -e "{\n \"id\":\"$INTERNAL_ID\",\n \"username\":\"$INTERNAL_USER\",\n \"internalipaddress\":\"$INTERNAL_IP\"\n}\n" > $CONFIG_FILE

