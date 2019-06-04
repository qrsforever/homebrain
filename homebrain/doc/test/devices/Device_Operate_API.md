---

title: 设备操作API
date: 2018-12-29 17:51:48
tags: [Work]
categories: [Company]

---

<!-- vim-markdown-toc GFM -->

* [设备操作](#设备操作)
    * [环境变量](#环境变量)
    * [API测试](#api测试)
        * [搜索新设备](#搜索新设备)
        * [查询主机设备列表](#查询主机设备列表)
        * [添加新设备](#添加新设备)
        * [查询设备在线状态](#查询设备在线状态)
        * [查询设备属性Profile](#查询设备属性profile)
        * [设备状态信息获取](#设备状态信息获取)
        * [操作设备属性](#操作设备属性)

<!-- vim-markdown-toc -->

<!-- more -->

# 设备操作

## 环境变量

```sh
export ip=192.168.124.16
export port=8899
export token="token:1234567"

export devId="00124b00-19a8-9215-d0d0"
export typeId="oic.d.kk_lightctrl"
export prop1="{\"name\":\"value1\", \"value\":\"0\"}"
export prop2="{\"name\":\"value2\", \"value\":\"1\"}"
```


## API测试

### 搜索新设备

命令: ` curl -v -X POST http://$ip:$port/api/familydevice/discovery  -H $token `

输出:

```json
{
    "requestId": "abcddcba",
    "request": "/api/familydevice/discovery",
    "status": 1,
    "result": {
        "ret": "success",
        "maxDuration": "5"
    }
}
```

### 查询主机设备列表

命令:　` curl -v -X POST http://$ip:$port/api/familydevice/listall -H $token `

输出:

```json
{
    "requestId": "abcddcba",
    "request": "/api/familydevice/list",
    "status": 1,
    "result": [
        {
            "deviceId": "00124b00-12cc-8000-d0d0",
            "typeId": "oic.d.kk_envsensor",
            "ownedStatus": "false"
        },
        {
            "deviceId": "00124b00-12d2-676f-d0d0",
            "typeId": "oic.d.kk_envsensor",
            "ownedStatus": "false"
        },
        {
            "deviceId": "00124b00-1646-0823-d0d0",
            "typeId": "oic.d.kk_scenectrl",
            "ownedStatus": "false"
        },
        {
            "deviceId": "00124b00-16bd-69a5-d0d0",
            "typeId": "oic.d.kk_doorcontact",
            "ownedStatus": "true"
        },
        {
            "deviceId": "00124b00-16bd-9578-d0d0",
            "typeId": "oic.d.kk_sosalarm",
            "ownedStatus": "true"
        },
        {
            "deviceId": "00124b00-16bd-a7a8-d0d0",
            "typeId": "oic.d.kk_doorcontact",
            "ownedStatus": "true"
        },
        {
            "deviceId": "00124b00-1769-1549-d0d0",
            "typeId": "oic.d.kk_doorcontact",
            "ownedStatus": "false"
        },
        {
            "deviceId": "00124b00-1769-2c9b-d0d0",
            "typeId": "oic.d.kk_sosalarm",
            "ownedStatus": "true"
        },
        {
            "deviceId": "00124b00-176c-76cd-d0d0",
            "typeId": "oic.d.kk_sosalarm",
            "ownedStatus": "true"
        },
        {
            "deviceId": "00124b00-18dc-e960-d0d0",
            "typeId": "oic.d.kk_smartplug",
            "ownedStatus": "true"
        },
        {
            "deviceId": "00124b00-18dc-eaa1-d0d0",
            "typeId": "oic.d.kk_smartplug",
            "ownedStatus": "true"
        },
        {
            "deviceId": "00124b00-199f-5c97-d0d0",
            "typeId": "oic.d.kk_lightctrl",
            "ownedStatus": "true"
        },
        {
            "deviceId": "00124b00-19a8-9215-d0d0",
            "typeId": "oic.d.kk_lightctrl",
            "ownedStatus": "false"
        }
    ]
}
```

### 添加新设备

命令: ` curl -v -X POST http://$ip:$port/api/familydevice/bind -H $token -d "{\"deviceId\":\"$devId\", \"typeId\":\"$typeId\"}" `

输出：

```json
{
    "requestId": "abcddcba",
    "request": "/api/familydevice/bind",
    "status": 1,
    "result": {
        "typeId": "oic.d.kk_lightctrl",
        "deviceId": "00124b00-19a8-9215-d0d0",
        "ownedStatus": "true"
    }
}
```

### 查询设备在线状态

命令: ` curl -v -X POST http://$ip:$port/api/familydevice/isonline -H $token -d "{\"deviceId\":\"$devId\"}" `

输出:

```json
{
    "requestId": "abcddcba",
    "request": "/api/familydevice/isonline",
    "status": 1,
    "result": {
        "isonline": "true"
    }
}
```

### 查询设备属性Profile

命令: ` curl -v -X POST http://$ip:$port/api/familydevice/profile -H $token -d "{\"deviceId\":\"$devId\"}" `

输出:

```json
{
    "requestId": "abcddcba",
    "request": "/api/familydevice/profile",
    "status": 1,
    "result": {
        "profile": {
            "OnlineState": {
                "tag": "在线状态",
                "write": "F",
                "read": "T",
                "type": "enum",
                "range": {
                    "0": "离线",
                    "1": "在线"
                }
            },
            "value1": {
                "tag": "开关状态1",
                "read": "T",
                "write": "T",
                "type": "enum",
                "range": {
                    "0": "离线",
                    "1": "在线"
                }
            },
            "value2": {
                "tag": "开关状态2",
                "read": "T",
                "write": "T",
                "type": "enum",
                "range": {
                    "0": "离线",
                    "1": "在线"
                }
            },
            "value3": {
                "tag": "开关状态3",
                "read": "T",
                "write": "T",
                "type": "enum",
                "range": {
                    "0": "离线",
                    "1": "在线"
                }
            }
        },
        "description": "控客3T灯控",
        "manufacture": "Konke",
        "version": "0.9.2"
    }
}
```

### 设备状态信息获取

命令: ` curl -v -X POST http://$ip:$port/api/familydevice/status -H $token -d "{\"deviceId\":\"$devId\"}" `

输出:

```json
{
    "requestId": "abcddcba",
    "request": "/api/familydevice/status",
    "status": 1,
    "result": {
        "value1": "0",
        "value2": "0",
        "value3": "0"
    }
}
```

### 操作设备属性

命令: ` curl -v -X POST http://$ip:$port/api/familydevice/operate -H $token -d "{\"deviceId\":\"$devId\", \"propSet\":\"$propset\"}" `

输出:

```json
{
    "requestId": "abcddcba",
    "request": "/api/familydevice/operate",
    "status": 1,
    "result": {
        "ret": "SUCCESS"
    }
}
```
