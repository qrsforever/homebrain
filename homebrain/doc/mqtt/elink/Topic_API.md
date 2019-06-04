---

title: Elink Topic API Command Test

date: 2019-04-28 10:58:50
tags: [mqtt elink]
categories: [local]

---

[elink mqtt test](http://wiki.letv.cn/pages/viewpage.action?pageId=79233222)

**clientID目前只有一个,以下测试不可以**

## 准备

1. sudo apt-get install mosquitto-clients
2. source env.sh

## 直联设备连接认证


## 网关注册子设备

- 请求topic：/sys/$productKey/$elinkId/thing/sub/register  //发布，设备注册
- 响应topic：/sys/$productKe/$elinkId/thing/sub/registerReply //订阅，设备注册结果返回

例如:

```shell

mosquitto_sub -d -v -q 1 -h $ELINK_HOST -p $ELINK_POST \
    -i $ELINK_ID -u $ELINK_ID -P $DEVICE_SECRET \
    -t $SUB_TOPIC_REGISTER_SUBDEV_REPLY \


mosquitto_pub -d -h $ELINK_HOST -p $ELINK_POST \
    -i $ELINK_ID -u $ELINK_ID -P $DEVICE_SECRET \
    -t $PUB_TOPIC_REGISTER_SUBDEV \
    -f register_tv_device.json

```

## 控制指令


- 请求topic：/sys/property/$productKey/$elinkid/set  //订阅 接收云端控制指令
- 返回topic：/sys/property/$productKey/$elinkid/setReply //发布 反馈控制结果



## 获取设备状态



## 事件上报



## 网关设备添加拓扑

- 请求topic /sys/$productKey/$elinkId/thing/topo/add  //发布 添加拓扑关系
- 返回topic /sys/$productKey/$elinkId/thing/topo/addReply  //订阅

```shell

mosquitto_sub -d -v -q 1 -h $ELINK_HOST -p $ELINK_POST \
    -i $ELINK_ID -u $ELINK_ID -P $DEVICE_SECRET \
    -t $SUB_TOPIC_ADD_TOPO_REPLY


mosquitto_pub -d -q 1 -h $ELINK_HOST -p $ELINK_POST \
    -i $ELINK_ID -u $ELINK_ID -P $DEVICE_SECRET \
    -t $PUB_TOPIC_ADD_TOPO \
    -f add_topo.json

```


## 网关设备删除拓扑



# API

## gateway

接口名字暂时没想好

### 注册接口
int register(deviceId, deviceName, productKey)
    // 内部对应pub

int registerReply(callback1)
    // 内部对应sub
    int callback1(deviceId, deviceSecret, elinkId);

### 添加topo

int topoAdd(elinkId, productKey, deviceSecret, clientId)
    // 内部对应pub

int topoAddReply(callback2)
    // 内部对应sub
    int callback2(code, message)

### 删除topo

int topoDelete(elinkId, productKey, deviceSecret, clientId)
    // 内部对应pub

int topoDeleteReply(callback3)
    // 内部对应sub
    int callback3(code, message)






## subdevice

### 获取设备状态 (云主动获取)

int registerGetStatusCallback(callback4)
    // 内部对应sub
    int callback4(elinkId, attrsList/* 属性key数组 */)

int reportStatus(elinkId, keyvaluesMaps /* 属性key-values */)
    // 内部对应pub (被动reply)

### 上报事件

int reportEvent(eventId, type/*事件类型*/, message /* 消息体 */, value /*可选值*/)
    // 内部对应pub

## 控制指令
int registerAttrControlCallback(callback4)
    // 内部对应sub
    int callback4(elinkId, keyvaluesMaps/* 属性key-value*/)
