#!/bin/bash
#=================================================================
# date: 2019-04-28 10:55:56
# title: env
#=================================================================

# host port
ELINK_HOST=10.112.39.126
ELINK_POST=1883

# device auth
ELINK_ID=el.48f8c93eb2ca9e775649128f293b66d8
DEVICE_SECRET=ds.3567a01ae3f3a13c0f7760e623bdbf6e
PRODUCT_KEY=pk.be25f066ac9

# regist sub device
SUB_TOPIC_REGISTER_SUBDEV_REPLY="/sys/$PRODUCT_KEY/$ELINK_ID/thing/sub/registerReply"
PUB_TOPIC_REGISTER_SUBDEV="/sys/$PRODUCT_KEY/$ELINK_ID/thing/sub/register"


# topo
SUB_TOPIC_ADD_TOPO_REPLY="/sys/$PRODUCT_KEY/$ELINK_ID/thing/topo/addReply"
PUB_TOPIC_ADD_TOPO="/sys/$PRODUCT_KEY/$ELINK_ID/thing/topo/add"
